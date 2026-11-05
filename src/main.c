#define _DEFAULT_SOURCE
#include "filemanager.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static void usage(const char *program)
{
    fprintf(stderr,
        "Usage:\n"
        "  %s info [-L] PATH...\n"
        "  %s list [-R] DIRECTORY [NAME_FILTER]\n"
        "  %s search [-R] DIRECTORY KEYWORD\n"
        "  %s compare PATH1 PATH2\n"
        "  %s export txt|csv OUTPUT PATH...\n",
        program, program, program, program, program);
}

int main(int argc, char **argv)
{
    int i;

    if (argc < 3) {
        usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "info") == 0) {
        int follow = 0;
        int first = 2;

        if (strcmp(argv[first], "-L") == 0) {
            follow = 1;
            first++;
        }
        if (first >= argc) {
            usage(argv[0]);
            return 1;
        }

        for (i = first; i < argc; i++) {
            FileInfo file;

            if (fm_get_info(argv[i], follow, &file) == -1) {
                file.valid = 0;
                file.error_number = errno;
                snprintf(file.path, sizeof(file.path), "%s", argv[i]);
            }
            fm_print_info(stdout, &file);
        }
        return 0;
    }

    if (strcmp(argv[1], "list") == 0) {
        int recursive = 0;
        int index = 2;
        const char *filter = "";

        if (strcmp(argv[index], "-R") == 0) {
            recursive = 1;
            index++;
        }
        if (index >= argc) {
            usage(argv[0]);
            return 1;
        }
        if (index + 1 < argc)
            filter = argv[index + 1];

        return fm_list_directory(argv[index], filter, recursive, stdout) < 0;
    }

    if (strcmp(argv[1], "search") == 0) {
        int recursive = 0;
        int index = 2;

        if (strcmp(argv[index], "-R") == 0) {
            recursive = 1;
            index++;
        }
        if (argc - index < 2) {
            usage(argv[0]);
            return 1;
        }
        return fm_search(argv[index], argv[index + 1], recursive, stdout) < 0;
    }

    if (strcmp(argv[1], "compare") == 0) {
        if (argc != 4) {
            usage(argv[0]);
            return 1;
        }
        return fm_compare(argv[2], argv[3], stdout) != 0;
    }

    if (strcmp(argv[1], "export") == 0) {
        if (argc < 5 ||
            (strcmp(argv[2], "txt") != 0 && strcmp(argv[2], "csv") != 0)) {
            usage(argv[0]);
            return 1;
        }
        return fm_export(argv[3], argv[2], argc - 4, &argv[4]) != 0;
    }

    usage(argv[0]);
    return 1;
}
