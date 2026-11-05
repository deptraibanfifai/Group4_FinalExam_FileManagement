#define _DEFAULT_SOURCE
#include "filemanager.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

const char *fm_type(mode_t mode)
{
    if (S_ISREG(mode)) return "Regular file";
    if (S_ISDIR(mode)) return "Directory";
    if (S_ISLNK(mode)) return "Symbolic link";
    if (S_ISCHR(mode)) return "Character device";
    if (S_ISBLK(mode)) return "Block device";
    if (S_ISFIFO(mode)) return "FIFO / pipe";
    if (S_ISSOCK(mode)) return "Socket";
    return "Unknown";
}

void fm_permissions(mode_t mode, char output[11])
{
    output[0] = S_ISDIR(mode) ? 'd' : S_ISLNK(mode) ? 'l' :
        S_ISCHR(mode) ? 'c' : S_ISBLK(mode) ? 'b' :
        S_ISFIFO(mode) ? 'p' : S_ISSOCK(mode) ? 's' : '-';
    output[1] = (mode & S_IRUSR) ? 'r' : '-';
    output[2] = (mode & S_IWUSR) ? 'w' : '-';
    output[3] = (mode & S_IXUSR) ? 'x' : '-';
    output[4] = (mode & S_IRGRP) ? 'r' : '-';
    output[5] = (mode & S_IWGRP) ? 'w' : '-';
    output[6] = (mode & S_IXGRP) ? 'x' : '-';
    output[7] = (mode & S_IROTH) ? 'r' : '-';
    output[8] = (mode & S_IWOTH) ? 'w' : '-';
    output[9] = (mode & S_IXOTH) ? 'x' : '-';
    output[10] = '\0';

    if (mode & S_ISUID) output[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID) output[6] = (mode & S_IXGRP) ? 's' : 'S';
    if (mode & S_ISVTX) output[9] = (mode & S_IXOTH) ? 't' : 'T';
}

int fm_get_info(const char *path, int follow_link, FileInfo *result)
{
    int status;

    if (path == NULL || result == NULL) {
        errno = EINVAL;
        return -1;
    }
    memset(result, 0, sizeof(*result));
    if (strlen(path) >= sizeof(result->path)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    strcpy(result->path, path);
    status = follow_link ? stat(path, &result->info) : lstat(path, &result->info);
    if (status == -1) {
        result->error_number = errno;
        return -1;
    }
    result->valid = 1;
    return 0;
}

static void print_time(FILE *out, const char *label, time_t value)
{
    char buffer[64];
    struct tm tm_value;

    if (localtime_r(&value, &tm_value) == NULL ||
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %Z", &tm_value) == 0)
        strcpy(buffer, "unavailable");

    fprintf(out, "%-27s %s\n", label, buffer);
}

void fm_print_info(void *stream, const FileInfo *file)
{
    FILE *out = (FILE *)stream;
    char permissions[11];
    const struct stat *st;

    if (out == NULL || file == NULL)
        return;

    if (!file->valid) {
        fprintf(out, "\nPath: %s\nError: %s\n",
                file->path, strerror(file->error_number));
        return;
    }

    st = &file->info;
    fm_permissions(st->st_mode, permissions);

    fprintf(out, "\nPath:                       %s\n", file->path);
    fprintf(out, "Type:                       %s\n", fm_type(st->st_mode));
    fprintf(out, "Inode:                      %llu\n", (unsigned long long)st->st_ino);
    fprintf(out, "Number of links:            %llu\n", (unsigned long long)st->st_nlink);
    fprintf(out, "Permissions:                %s (octal %04o)\n",
            permissions, (unsigned int)(st->st_mode & 07777));
    fprintf(out, "Size:                       %lld bytes\n", (long long)st->st_size);
    fprintf(out, "Block size:                 %lld bytes\n", (long long)st->st_blksize);
    fprintf(out, "Blocks allocated:           %lld\n", (long long)st->st_blocks);
    fprintf(out, "Owner UID:                  %u\n", (unsigned int)st->st_uid);
    fprintf(out, "Owner GID:                  %u\n", (unsigned int)st->st_gid);
    print_time(out, "Last access:", st->st_atime);
    print_time(out, "Last modification:", st->st_mtime);
    print_time(out, "Last status change:", st->st_ctime);
}
