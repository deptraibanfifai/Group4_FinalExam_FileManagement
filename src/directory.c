#define _DEFAULT_SOURCE
#include "filemanager.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

static int visit_directory(const char *directory, const char *keyword,
                           int recursive, FILE *out, int search_mode, int depth)
{
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    if (depth > 64)
        return 0;

    dir = opendir(directory);
    if (dir == NULL) {
        perror(directory);
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        char child[FM_PATH_SIZE];
        FileInfo info;
        int written;

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        written = snprintf(child, sizeof(child), "%s/%s", directory, entry->d_name);
        if (written < 0 || (size_t)written >= sizeof(child))
            continue;
        if (fm_get_info(child, 0, &info) == -1)
            continue;

        if (keyword == NULL || *keyword == '\0' ||
            strstr(entry->d_name, keyword) != NULL) {
            if (search_mode)
                fprintf(out, "%s\n", child);
            else
                fprintf(out, "%-40s %-20s %12lld bytes\n",
                        child, fm_type(info.info.st_mode),
                        (long long)info.info.st_size);
            count++;
        }

        /* Ch i vo th mc tht, khng i theo symbolic link. */
        if (recursive && S_ISDIR(info.info.st_mode)) {
            int nested = visit_directory(child, keyword, recursive,
                                         out, search_mode, depth + 1);
            if (nested > 0)
                count += nested;
        }
    }

    closedir(dir);
    return count;
}

int fm_list_directory(const char *directory, const char *keyword,
                      int recursive, void *stream)
{
    if (directory == NULL || stream == NULL)
        return -1;
    return visit_directory(directory, keyword, recursive, (FILE *)stream, 0, 0);
}

int fm_search(const char *directory, const char *keyword,
              int recursive, void *stream)
{
    if (directory == NULL || keyword == NULL || stream == NULL)
        return -1;
    return visit_directory(directory, keyword, recursive, (FILE *)stream, 1, 0);
}
