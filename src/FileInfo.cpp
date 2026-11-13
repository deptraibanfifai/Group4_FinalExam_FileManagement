#include "FileInfo.hpp"
#include "TerminalUI.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#endif

// Simple, self-contained standard SHA-256 implementation
namespace SHA256Impl {
    typedef uint32_t uint32;
    typedef uint64_t uint64;
    typedef uint8_t  uint8;

    struct Context {
        uint32 state[8];
        uint64 count;
        uint8  buffer[64];
    };

    static inline uint32 rotr(uint32 x, uint32 n) { return (x >> n) | (x << (32 - n)); }
    static inline uint32 ch(uint32 x, uint32 y, uint32 z) { return (x & y) ^ (~x & z); }
    static inline uint32 maj(uint32 x, uint32 y, uint32 z) { return (x & y) ^ (x & z) ^ (y & z); }
    static inline uint32 sigma0(uint32 x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    static inline uint32 sigma1(uint32 x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
    static inline uint32 gamma0(uint32 x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    static inline uint32 gamma1(uint32 x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

    static const uint32 K[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };

    static void transform(Context *ctx, const uint8 data[64]) {
        uint32 a, b, c, d, e, f, g, h, w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = ((uint32)data[i * 4] << 24) |
                   ((uint32)data[i * 4 + 1] << 16) |
                   ((uint32)data[i * 4 + 2] << 8) |
                   ((uint32)data[i * 4 + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            w[i] = gamma1(w[i - 2]) + w[i - 7] + gamma0(w[i - 15]) + w[i - 16];
        }

        a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
        e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

        for (int i = 0; i < 64; ++i) {
            uint32 t1 = h + sigma1(e) + ch(e, f, g) + K[i] + w[i];
            uint32 t2 = sigma0(a) + maj(a, b, c);
            h = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }

        ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
        ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
    }

    void init(Context *ctx) {
        ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85;
        ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
        ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
        ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
        ctx->count = 0;
    }

    void update(Context *ctx, const uint8 *data, size_t len) {
        size_t buffer_len = (size_t)(ctx->count & 63);
        ctx->count += len;
        size_t space = 64 - buffer_len;

        if (len >= space) {
            for (size_t i = 0; i < space; ++i) ctx->buffer[buffer_len + i] = data[i];
            transform(ctx, ctx->buffer);
            data += space;
            len -= space;
            while (len >= 64) {
                transform(ctx, data);
                data += 64;
                len -= 64;
            }
            buffer_len = 0;
        }
        for (size_t i = 0; i < len; ++i) ctx->buffer[buffer_len + i] = data[i];
    }

    void final(Context *ctx, uint8 hash[32]) {
        uint64 total_bits = ctx->count * 8;
        size_t buffer_len = (size_t)(ctx->count & 63);
        ctx->buffer[buffer_len++] = 0x80;

        if (buffer_len > 56) {
            while (buffer_len < 64) ctx->buffer[buffer_len++] = 0;
            transform(ctx, ctx->buffer);
            buffer_len = 0;
        }
        while (buffer_len < 56) ctx->buffer[buffer_len++] = 0;

        for (int i = 7; i >= 0; --i) {
            ctx->buffer[56 + (7 - i)] = (uint8)(total_bits >> (i * 8));
        }
        transform(ctx, ctx->buffer);

        for (int i = 0; i < 8; ++i) {
            hash[i * 4]     = (uint8)(ctx->state[i] >> 24);
            hash[i * 4 + 1] = (uint8)(ctx->state[i] >> 16);
            hash[i * 4 + 2] = (uint8)(ctx->state[i] >> 8);
            hash[i * 4 + 3] = (uint8)(ctx->state[i]);
        }
    }
}

std::string FileMetadata::formatBytes(uintmax_t bytes) {
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int s = 0;
    double count = static_cast<double>(bytes);
    while (count >= 1024.0 && s < 5) {
        s++;
        count /= 1024.0;
    }
    std::ostringstream ss;
    if (s == 0) {
        ss << bytes << " " << suffixes[s];
    } else {
        ss << std::fixed << std::setprecision(2) << count << " " << suffixes[s];
    }
    return ss.str();
}

std::string FileMetadata::calculateSHA256(const fs::path& p) {
    std::ifstream file(p, std::ios::binary);
    if (!file.is_open()) return "Unable to open file";

    SHA256Impl::Context ctx;
    SHA256Impl::init(&ctx);

    char buffer[32768];
    while (file.read(buffer, sizeof(buffer))) {
        SHA256Impl::update(&ctx, reinterpret_cast<const uint8_t*>(buffer), file.gcount());
    }
    if (file.gcount() > 0) {
        SHA256Impl::update(&ctx, reinterpret_cast<const uint8_t*>(buffer), file.gcount());
    }

    uint8_t hash[32];
    SHA256Impl::final(&ctx, hash);

    std::ostringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

static std::string formatTimePoint(std::chrono::system_clock::time_point tp) {
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    char buf[64];
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
    return std::string(buf);
}

FileMetadata FileMetadata::getInfo(const fs::path& p, bool computeHash, bool followSymlinks) {
    FileMetadata meta;
    meta.pathStr = p.string();

    std::error_code ec;
    fs::file_status status;

    if (followSymlinks) {
        status = fs::status(p, ec);
    } else {
        status = fs::symlink_status(p, ec);
    }

    if (ec) {
        meta.exists = false;
        meta.errorMessage = ec.message();
        return meta;
    }

    meta.exists = fs::exists(status);
    if (!meta.exists) {
        meta.errorMessage = "File not found or broken symlink";
        return meta;
    }

    // Determine type
    if (fs::is_directory(status)) {
        meta.isDirectory = true;
        meta.fileTypeStr = "Directory";
    } else if (fs::is_regular_file(status)) {
        meta.isRegularFile = true;
        meta.fileTypeStr = "Regular File";
    } else if (fs::is_symlink(status)) {
        meta.isSymlink = true;
        meta.fileTypeStr = "Symbolic Link";
    } else if (fs::is_block_file(status)) {
        meta.fileTypeStr = "Block Device";
    } else if (fs::is_character_file(status)) {
        meta.fileTypeStr = "Character Device";
    } else if (fs::is_fifo(status)) {
        meta.fileTypeStr = "FIFO / Pipe";
    } else if (fs::is_socket(status)) {
        meta.fileTypeStr = "Socket";
    } else {
        meta.isOther = true;
        meta.fileTypeStr = "Other / Unknown";
    }

    // Hard links
    meta.hardLinksCount = fs::hard_link_count(p, ec);
    if (ec) meta.hardLinksCount = 1;

    // Permissions
    auto perms = status.permissions();
    std::string pStr = "----------";
    if (meta.isDirectory) pStr[0] = 'd';
    else if (meta.isSymlink) pStr[0] = 'l';
    else if (meta.fileTypeStr == "FIFO / Pipe") pStr[0] = 'p';
    else if (meta.fileTypeStr == "Character Device") pStr[0] = 'c';
    else if (meta.fileTypeStr == "Block Device") pStr[0] = 'b';
    else if (meta.fileTypeStr == "Socket") pStr[0] = 's';

    pStr[1] = ((perms & fs::perms::owner_read) != fs::perms::none) ? 'r' : '-';
    pStr[2] = ((perms & fs::perms::owner_write) != fs::perms::none) ? 'w' : '-';
    pStr[3] = ((perms & fs::perms::owner_exec) != fs::perms::none) ? 'x' : '-';
    pStr[4] = ((perms & fs::perms::group_read) != fs::perms::none) ? 'r' : '-';
    pStr[5] = ((perms & fs::perms::group_write) != fs::perms::none) ? 'w' : '-';
    pStr[6] = ((perms & fs::perms::group_exec) != fs::perms::none) ? 'x' : '-';
    pStr[7] = ((perms & fs::perms::others_read) != fs::perms::none) ? 'r' : '-';
    pStr[8] = ((perms & fs::perms::others_write) != fs::perms::none) ? 'w' : '-';
    pStr[9] = ((perms & fs::perms::others_exec) != fs::perms::none) ? 'x' : '-';
    meta.permissionsStr = pStr;

    // Octal representation
    unsigned int octVal = static_cast<unsigned int>(perms) & 0777;
    std::ostringstream octSs;
    octSs << "0" << std::oct << octVal;
    meta.octalPermissions = octSs.str();

    // Size
    if (meta.isRegularFile) {
        meta.fileSize = fs::file_size(p, ec);
        if (ec) meta.fileSize = 0;
    } else {
        meta.fileSize = 0;
    }
    meta.humanReadableSize = formatBytes(meta.fileSize);

    // Timestamps and OS specific attributes
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
    std::wstring wpath = p.wstring();
    if (GetFileAttributesExW(wpath.c_str(), GetFileExInfoStandard, &fileAttrData)) {
        auto filetimeToSystemTime = [](const FILETIME& ft) -> std::chrono::system_clock::time_point {
            ULARGE_INTEGER ull;
            ull.LowPart = ft.dwLowDateTime;
            ull.HighPart = ft.dwHighDateTime;
            // 100-nanoseconds intervals since Jan 1, 1601 to Jan 1, 1970
            const uint64_t EPOCH_DIFF = 116444736000000000ULL;
            if (ull.QuadPart >= EPOCH_DIFF) {
                int64_t diff = ull.QuadPart - EPOCH_DIFF;
                return std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(
                    std::chrono::nanoseconds(diff * 100)));
            }
            return std::chrono::system_clock::time_point();
        };

        meta.creationTime = formatTimePoint(filetimeToSystemTime(fileAttrData.ftCreationTime));
        meta.lastAccessTime = formatTimePoint(filetimeToSystemTime(fileAttrData.ftLastAccessTime));
        meta.lastWriteTime = formatTimePoint(filetimeToSystemTime(fileAttrData.ftLastWriteTime));

        if (!meta.isRegularFile && !meta.isDirectory) {
            uint64_t sz = ((uint64_t)fileAttrData.nFileSizeHigh << 32) | fileAttrData.nFileSizeLow;
            meta.fileSize = sz;
            meta.humanReadableSize = formatBytes(sz);
        }
    } else {
        meta.lastWriteTime = "Unavailable";
        meta.lastAccessTime = "Unavailable";
        meta.creationTime = "Unavailable";
    }

    meta.ownerName = "Windows User";
    meta.groupName = "Windows Group";
#else
    struct stat st;
    int res = followSymlinks ? stat(p.c_str(), &st) : lstat(p.c_str(), &st);
    if (res == 0) {
        meta.inode = st.st_ino;
        meta.hardLinksCount = st.st_nlink;
        meta.uid = st.st_uid;
        meta.gid = st.st_gid;
        meta.fileSize = st.st_size;
        meta.humanReadableSize = formatBytes(meta.fileSize);

        struct passwd *pw = getpwuid(st.st_uid);
        meta.ownerName = (pw ? pw->pw_name : std::to_string(st.st_uid));
        struct group  *gr = getgrgid(st.st_gid);
        meta.groupName = (gr ? gr->gr_name : std::to_string(st.st_gid));

        auto toTp = [](time_t t) { return std::chrono::system_clock::from_time_t(t); };
        meta.lastAccessTime = formatTimePoint(toTp(st.st_atime));
        meta.lastWriteTime = formatTimePoint(toTp(st.st_mtime));
        meta.creationTime = formatTimePoint(toTp(st.st_ctime));
    }
#endif

    if (computeHash && meta.isRegularFile) {
        meta.sha256Checksum = calculateSHA256(p);
    }

    return meta;
}

void FileMetadata::printDetailed(std::ostream& os, bool colored) const {
    const std::string& C_KEY = colored ? (Terminal::FG_CYAN + Terminal::BOLD) : "";
    const std::string& C_VAL = colored ? Terminal::FG_WHITE : "";
    const std::string& C_RESET = colored ? Terminal::RESET : "";

    if (!exists) {
        os << "\n" << (colored ? Terminal::FG_RED : "") << "Path:  " << pathStr << "\n"
           << "Error: " << errorMessage << C_RESET << "\n";
        return;
    }

    os << "\n" << (colored ? (Terminal::BG_BLUE + Terminal::FG_BRIGHT_WHITE + Terminal::BOLD) : "")
       << " [" << pathStr << "] " << C_RESET << "\n";

    auto printLine = [&](const std::string& label, const std::string& val) {
        os << C_KEY << std::left << std::setw(24) << label << C_RESET << ": "
           << C_VAL << val << C_RESET << "\n";
    };

    printLine("Type", fileTypeStr);
    printLine("Permissions", permissionsStr + " (octal " + octalPermissions + ")");
    printLine("Size", std::to_string(fileSize) + " bytes (" + humanReadableSize + ")");
    printLine("Number of links", std::to_string(hardLinksCount));
    if (inode != 0) {
        printLine("Inode", std::to_string(inode));
    }
    if (!ownerName.empty()) {
        printLine("Owner / Group", ownerName + " / " + groupName);
    }
    printLine("Last Access Time", lastAccessTime);
    printLine("Last Modification Time", lastWriteTime);
    printLine("Status / Creation Time", creationTime);

    if (!sha256Checksum.empty()) {
        printLine("SHA-256 Checksum", sha256Checksum);
    }
}
