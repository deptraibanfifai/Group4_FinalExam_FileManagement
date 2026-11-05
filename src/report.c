#include "filemanager.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static void csv_field(FILE *out, const char *value)
{
    const char *p;

    fputc('"', out);
    for (p = value; *p; p++) {
        if (*p == '"')
            fputc('"', out);
        fputc(*p, out);
    }
    fputc('"', out);
}

static void format_time_value(time_t value, char output[64])
{
    struct tm tm_value;

    if (localtime_r(&value, &tm_value) == NULL ||
        strftime(output, 64, "%Y-%m-%d %H:%M:%S %Z", &tm_value) == 0)
        strcpy(output, "unavailable");
}

int fm_export(const char *output_path, const char *format,
              int path_count, char **paths)
{
    FILE *out;
    int i;
    int csv;

    if (output_path == NULL || format == NULL || path_count <= 0 || paths == NULL)
        return -1;

    csv = strcmp(format, "csv") == 0;
    out = fopen(output_path, "w");
    if (out == NULL) {
        fprintf(stderr, "Cannot write %s: %s\n", output_path, strerror(errno));
        return -1;
    }

    if (csv)
        fprintf(out, "path,type,permissions,mode,size_bytes,inode,links,uid,gid,access,modified,status_changed\n");

    for (i = 0; i < path_count; i++) {
        FileInfo file;
        char perm[11], atime[64], mtime[64], ctime_buf[64];

        if (fm_get_info(paths[i], 0, &file) == -1) {
            fprintf(stderr, "%s: %s\n", paths[i], strerror(errno));
            continue;
        }

        fm_permissions(file.info.st_mode, perm);
        format_time_value(file.info.st_atime, atime);
        format_time_value(file.info.st_mtime, mtime);
        format_time_value(file.info.st_ctime, ctime_buf);

        if (csv) {
            csv_field(out, file.path);
            fputc(',', out);
            csv_field(out, fm_type(file.info.st_mode));
            fprintf(out, ",%s,%04o,%lld,%llu,%llu,%u,%u,",
                    perm, (unsigned int)(file.info.st_mode & 07777),
                    (long long)file.info.st_size,
                    (unsigned long long)file.info.st_ino,
                    (unsigned long long)file.info.st_nlink,
                    (unsigned int)file.info.st_uid,
                    (unsigned int)file.info.st_gid);
            csv_field(out, atime);
            fputc(',', out);
            csv_field(out, mtime);
            fputc(',', out);
            csv_field(out, ctime_buf);
            fputc('\n', out);
        } else {
            fm_print_info(out, &file);
        }
    }

    if (fclose(out) != 0)
        return -1;

    printf("Report saved: %s\n", output_path);
    return 0;
}
