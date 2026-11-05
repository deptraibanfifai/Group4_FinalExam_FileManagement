#include "filemanager.h"

#include <stdio.h>

int fm_compare(const char *path_a, const char *path_b, void *stream)
{
    FileInfo a, b;
    FILE *out = (FILE *)stream;

    if (out == NULL ||
        fm_get_info(path_a, 0, &a) == -1 ||
        fm_get_info(path_b, 0, &b) == -1) {
        fprintf(stderr, "Cannot read one or both paths.\n");
        return -1;
    }

    fprintf(out, "Comparison: %s <-> %s\n", path_a, path_b);
#define COMPARE_FIELD(label, expression) \
    fprintf(out, "%-20s %s\n", label, (expression) ? "same" : "different")
    COMPARE_FIELD("File type:",
        ((a.info.st_mode & S_IFMT) == (b.info.st_mode & S_IFMT)));
    COMPARE_FIELD("Size:", a.info.st_size == b.info.st_size);
    COMPARE_FIELD("Permissions:",
        (a.info.st_mode & 07777) == (b.info.st_mode & 07777));
    COMPARE_FIELD("Link count:", a.info.st_nlink == b.info.st_nlink);
    COMPARE_FIELD("Owner UID:", a.info.st_uid == b.info.st_uid);
    COMPARE_FIELD("Owner GID:", a.info.st_gid == b.info.st_gid);
    COMPARE_FIELD("Modified time:", a.info.st_mtime == b.info.st_mtime);
#undef COMPARE_FIELD

    return 0;
}
