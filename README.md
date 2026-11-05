# FILE MANAGEMENT SYSTEM

## 1. Gii thiu

File Management System l chng trnh C chy trn NetBSD, h tr xem v phn tch thng tin file/th mc trong mi trng UNIX.

Chng trnh hin chy trn terminal, s dng cc hm h thng POSIX nh `stat()`, `lstat()`, `opendir()` v `readdir()`. Phn x l c tch thnh cc module ring  c th mm rng thm giao din  ha sau ny.

## 2. Mi trng pht trin

- H iu hnh: NetBSD 10.1 amd64
- Ngn ng: C
- Chun bin dch: C99
- Trnh bin dch: `cc`
- Cng c qun l m ngun: Git
- Khng ph thuc Ubuntu
- Phin bn hin ti cha cn GTK hoc X11

## 3. Cu trc d n

```text
FileManagement/
 include/
 filemanager.h       # Khai b   o kiu d liu v hm dng chung
 src/
 main.c              # c lnh v iu phi chc nng
 fileinfo.c          # Ly v    hin th metadata
 directory.c         # Lit k    v tm kim trong th mc
 compare.c           # So s   nh metadata hai ng dn
 report.c            # Xut b   o co TXT/CSV
 fileinfo.c              # Phin bn ban u ca chng trnh
 fileinfo                # Chng trnh bn u  bin dch
 Makefile
 README.md
