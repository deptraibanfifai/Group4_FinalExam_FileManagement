# File Management - NetBSD

C/POSIX tool for reading file and directory metadata. The core modules are separate from the user interface so a GUI can be added later.

## Functions

- Show type, permissions, inode, hard-link count, size, UID/GID and timestamps.
- List a directory and optionally search recursively or filter names.
- Compare metadata for two paths.
- Export reports as TXT or CSV.
- Use `lstat()` by default; `info -L` follows symbolic links.

## Build

    make

## Examples

    ./filemanager info fileinfo.c /etc /dev/null
    ./filemanager info -L linkname
    ./filemanager list /etc
    ./filemanager list -R /home Nguyen
    ./filemanager search -R /home report
    ./filemanager compare file-a file-b
    ./filemanager export csv report.csv fileinfo.c /etc/passwd
    ./filemanager export txt report.txt fileinfo.c /etc/passwd
