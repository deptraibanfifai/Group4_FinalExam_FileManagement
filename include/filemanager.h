#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <sys/types.h>
#include <sys/stat.h>

#define FM_PATH_SIZE 4096

typedef struct {
    char path[FM_PATH_SIZE];
    struct stat info;
    int valid;
    int error_number;
} FileInfo;

int fm_get_info(const char *path, int follow_link, FileInfo *result);
void fm_print_info(void *stream, const FileInfo *file);
int fm_list_directory(const char *directory, const char *keyword,
                      int recursive, void *stream);
int fm_search(const char *directory, const char *keyword,
              int recursive, void *stream);
int fm_compare(const char *path_a, const char *path_b, void *stream);
int fm_export(const char *output_path, const char *format,
              int path_count, char **paths);
const char *fm_type(mode_t mode);
void fm_permissions(mode_t mode, char output[11]);

#endif
