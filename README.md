# FILE MANAGEMENT SYSTEM (C++ Terminal Edition v2.0)

Hệ thống quản lý, tra cứu thông tin tệp tin & thư mục chuyên nghiệp chạy hoàn toàn trên Terminal (CLI & Interactive Shell), được nâng cấp từ C lên **Modern C++ (C++17)** với khả năng tương thích đa nền tảng (Linux, macOS, Windows).

---

## 🌟 1. Tính năng nổi bật

### 📌 Yêu cầu cốt lõi (Core Requirements)
- **Tra cứu Metadata chi tiết** cho một hoặc nhiều tệp/thư mục cùng lúc:
  - Loại tệp (Regular File, Directory, Symbolic Link, FIFO, Socket, Block/Char Device,...)
  - Quyền truy cập (Permissions: dạng chuỗi `rwxrwxrwx` và dạng bát phân `octal 0755`)
  - Kích thước tệp (Bytes thuần & định dạng dễ đọc: B, KB, MB, GB, TB)
  - Số lượng liên kết cứng (Number of hard links)
  - Inode / File ID, UID/GID, Owner & Group
  - Mốc thời gian: Last Access Time, Last Modification Time, Status / Creation Time
  - Tính toàn vẹn dữ liệu: Mã băm **SHA-256 Checksum**

### 🚀 Tính năng mở rộng nâng cao (Advanced Features)
- 🖥️ **Interactive Terminal Shell (`interactive` / `shell` / `chạy không tham số`)**: Giao diện dòng lệnh tương tác trực tiếp với dấu nhắc prompt, lệnh điều hướng `cd`, `pwd`, `ls`, `info`, `tree`, `du`, `touch`, `mkdir`, `rm`, `cp`, `mv`,...
- 🌳 **Directory Tree View (`tree`)**: Hiển thị cây thư mục phân nhánh trực quan với biểu tượng màu sắc ANSI, hiển thị kích thước từng tệp.
- 📊 **Disk Usage Analysis (`du` / `analyze`)**: Phân tích dung lượng thư mục, thống kê tổng số file/thư mục, tỷ lệ % dung lượng theo từng định dạng mở rộng (.cpp, .exe, .png,...) và top file chiếm nhiều dung lượng nhất.
- 🔍 **Tìm kiếm đa tiêu chí (`search` / `find`)**: Tìm kiếm theo tên (substring/pattern), đuôi mở rộng (`--ext`), kích thước tối thiểu/tối đa (`--min-size`, `--max-size`), chỉ định dạng file hoặc thư mục.
- ⚖️ **So sánh tệp sâu (`compare` / `diff`)**: So sánh song song metadata và kiểm tra tính toàn vẹn nội dung tệp (Byte-by-byte & SHA-256 hash).
- 📄 **Xuất báo cáo đa định dạng (`export`)**: Hỗ trợ xuất dữ liệu ra `txt`, `csv`, `json`, và `markdown` table.
- ⚡ **Thao tác tệp tiện ích**: Hỗ trợ các lệnh tiện ích nhanh ngay trên terminal: `cp`, `mv`, `rm`, `touch`, `mkdir`, `head`, `tail`.
- 🎨 **Terminal ANSI Colors**: Phân loại màu sắc rõ nét theo loại tệp (Thư mục màu xanh dương, Executable màu xanh lá, Symlink màu lục lam Cyan, Cảnh báo màu vàng,...).

---

## 📂 2. Cấu trúc mã nguồn

```text
FileManagerment/
├── include/
│   ├── FileInfo.hpp          # Class FileMetadata, phân tích thuộc tính & SHA-256
│   ├── DirScanner.hpp        # Liệt kê, tìm kiếm, vẽ Tree View, Disk Usage
│   ├── FileOps.hpp           # So sánh file (diff), sao chép, di chuyển, xóa, head/tail
│   ├── ReportExporter.hpp    # Xuất báo cáo TXT, CSV, JSON, Markdown
│   ├── TerminalUI.hpp        # Quản lý mã màu ANSI, format hiển thị terminal
│   └── filemanager.h         # Header C legacy (tương thích ngược)
├── src/
│   ├── main.cpp              # CLI parser & Interactive Shell REPL
│   ├── FileInfo.cpp          # Hiện thực lấy metadata & SHA-256
│   ├── DirScanner.cpp        # Hiện thực Tree, Search, Disk Analyzer
│   ├── FileOps.cpp           # Hiện thực so sánh diff & thao tác tệp
│   ├── ReportExporter.cpp    # Hiện thực xuất báo cáo đa định dạng
│   ├── TerminalUI.cpp        # Hiện thực ANSI console Windows/Linux
│   └── [legacy C files]      # fileinfo.c, directory.c, compare.c, report.c, main.c
├── Makefile                  # Makefile đa nền tảng (MinGW/GCC/Clang)
└── README.md                 # Tài liệu hướng dẫn sử dụng
```

---

## 🛠️ 3. Hướng dẫn biên dịch

Yêu cầu trình biên dịch hỗ trợ **C++17** (như `g++`, `clang++`, hoặc `MSVC`).

### Biên dịch bằng `g++` trực tiếp:
```bash
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude src/main.cpp src/FileInfo.cpp src/DirScanner.cpp src/FileOps.cpp src/ReportExporter.cpp src/TerminalUI.cpp -o filemanager
```
*(Trên Windows sẽ tạo ra file `filemanager.exe`)*

### Biên dịch bằng `make`:
```bash
make
```

---

## 💻 4. Hướng dẫn sử dụng

### 4.1. Xem thông tin chi tiết một hoặc nhiều file / thư mục
```bash
./filemanager info README.md Makefile src/main.cpp
```
Thêm cờ `-s` hoặc `--sha256` để tính mã băm kiểm tra tính toàn vẹn:
```bash
./filemanager info -s README.md Makefile
```

### 4.2. Khởi động Interactive Terminal Shell (Chế độ tương tác)
Chạy chương trình mà không truyền đối số, hoặc truyền `interactive`:
```bash
./filemanager
# hoặc
./filemanager interactive
```
Trong môi trường tương tác:
```text
[D:\FileManagerment]# ls
[D:\FileManagerment]# tree 2
[D:\FileManagerment]# du
[D:\FileManagerment]# info -s README.md
[D:\FileManagerment]# help
[D:\FileManagerment]# exit
```

### 4.3. Xem cấu trúc cây thư mục (Tree View)
```bash
./filemanager tree . 3
```

### 4.4. Phân tích dung lượng đĩa (Disk Usage Analysis)
```bash
./filemanager du .
```

### 4.5. Liệt kê & Tìm kiếm tệp nâng cao
- Liệt kê thư mục (đệ quy với `-R`):
  ```bash
  ./filemanager list -R src
  ```
- Tìm kiếm theo từ khóa và bộ lọc:
  ```bash
  ./filemanager search -R . "main" --ext .cpp
  ./filemanager search -R . "" --min-size 1024 --max-size 1048576
  ```

### 4.6. So sánh 2 tệp (Diff / Compare)
```bash
./filemanager compare fileA.txt fileB.txt
```

### 4.7. Xuất báo cáo (TXT, CSV, JSON, Markdown)
```bash
# Xuất JSON
./filemanager export json report.json README.md Makefile src/main.cpp

# Xuất CSV
./filemanager export csv report.csv README.md Makefile

# Xuất Markdown
./filemanager export md report.md README.md Makefile
```

### 4.8. Thao tác tệp tiện ích
```bash
./filemanager touch test.txt
./filemanager cp test.txt test_copy.txt
./filemanager head src/main.cpp 15
./filemanager rm test.txt test_copy.txt
```
