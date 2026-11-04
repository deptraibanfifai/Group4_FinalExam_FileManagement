#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static const char *
file_type(mode_t mode)
{
    if (S_ISREG(mode))  return "Regular file";
    if (S_ISDIR(mode))  return "Directory";
    if (S_ISLNK(mode))  return "Symbolic link";
    if (S_ISCHR(mode))  return "Character device";
    if (S_ISBLK(mode))  return "Block device";
    if (S_ISFIFO(mode)) return "FIFO / pipe";
    if (S_ISSOCK(mode)) return "Socket";
    return "Unknown";
}

static void
print_permissions(mode_t mode)
{
    char permissions[11];

    permissions[0] = S_ISDIR(mode) ? 'd' :
                     S_ISLNK(mode) ? 'l' :
                     S_ISCHR(mode) ? 'c' :
                     S_ISBLK(mode) ? 'b' :
                     S_ISFIFO(mode) ? 'p' :
                     S_ISSOCK(mode) ? 's' : '-';

    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';
    permissions[10] = '\0';

    /* Hiển thị các bit set-user-ID, set-group-ID và sticky. */
    if (mode & S_ISUID)
        permissions[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID)
        permissions[6] = (mode & S_IXGRP) ? 's' : 'S';
    if (mode & S_ISVTX)
        permissions[9] = (mode & S_IXOTH) ? 't' : 'T';

    printf("Permissions:       %s (octal %04o)\n",
           permissions, (unsigned int)(mode & 07777));
}

static void
print_time(const char *label, time_t value)
{
    char buffer[64];
    struct tm *local_time = localtime(&value);

    if (local_time == NULL ||
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %Z",
                 local_time) == 0) {
        printf("%-18s (cannot format time)\n", label);
        return;
    }

    printf("%-18s %s\n", label, buffer);
}

static void
show_file_info(const char *path)
{
    struct stat info;

    printf("\nPath:              %s\n", path);

    /*
     * lstat() lấy thông tin của chính symbolic link.
     * Nếu dùng stat(), chương trình sẽ lấy thông tin đích link.
     */
    if (lstat(path, &info) == -1) {
        printf("Error:             %s\n", strerror(errno));
        return;
    }

    printf("File type:         %s\n", file_type(info.st_mode));
    printf("Inode number:      %llu\n",
           (unsigned long long)info.st_ino);
    printf("Number of links:   %llu\n",
           (unsigned long long)info.st_nlink);
    printf("Owner user ID:     %u\n", (unsigned int)info.st_uid);
    printf("Owner group ID:    %u\n", (unsigned int)info.st_gid);
    printf("Size:              %lld bytes\n",
           (long long)info.st_size);
    printf("Block size:        %lld bytes\n",
           (long long)info.st_blksize);
    printf("Blocks allocated:  %lld\n",
           (long long)info.st_blocks);

    print_permissions(info.st_mode);
    print_time("Last access:", info.st_atime);
    print_time("Last modification:", info.st_mtime);
    print_time("Last status change:", info.st_ctime);
}

int
main(int argc, char *argv[])
{
    int i;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s file_or_directory [more_paths ...]\n",
                argv[0]);
        return 1;
    }

    for (i = 1; i < argc; i++)
        show_file_info(argv[i]);

    return 0;
}
