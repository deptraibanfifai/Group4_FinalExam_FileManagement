#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

class FileMetadata {
public:
    std::string pathStr;
    bool exists = false;
    bool isDirectory = false;
    bool isRegularFile = false;
    bool isSymlink = false;
    bool isOther = false;
    std::string fileTypeStr;

    uintmax_t fileSize = 0;
    std::string humanReadableSize;
    uint32_t hardLinksCount = 0;

    std::string permissionsStr;
    std::string octalPermissions;

    std::string lastAccessTime;
    std::string lastWriteTime;
    std::string creationTime;

    uint64_t inode = 0;
    uint32_t uid = 0;
    uint32_t gid = 0;
    std::string ownerName;
    std::string groupName;

    std::string sha256Checksum;
    std::string errorMessage;

    static FileMetadata getInfo(const fs::path& p, bool computeHash = false, bool followSymlinks = true);
    void printDetailed(std::ostream& os = std::cout, bool colored = true) const;

    static std::string formatBytes(uintmax_t bytes);
    static std::string calculateSHA256(const fs::path& p);
};
