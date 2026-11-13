#pragma once

#include "FileInfo.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>

struct SearchFilter {
    std::string namePattern;     // substring or wildcard/extension
    std::string extension;       // e.g. ".cpp"
    int64_t minSize = -1;        // in bytes (-1 for ignore)
    int64_t maxSize = -1;        // in bytes (-1 for ignore)
    bool onlyDirs = false;
    bool onlyFiles = false;
    int modifiedWithinDays = -1; // -1 for ignore
};

struct DiskUsageSummary {
    uintmax_t totalBytes = 0;
    size_t totalFiles = 0;
    size_t totalDirs = 0;
    std::map<std::string, uintmax_t> extensionSizes;
    std::map<std::string, size_t> extensionCounts;
    std::vector<std::pair<std::string, uintmax_t>> largestFiles;
};

class DirScanner {
public:
    static std::vector<FileMetadata> listDirectory(const fs::path& dir, bool recursive = false, const std::string& nameFilter = "");
    
    static std::vector<FileMetadata> search(const fs::path& dir, const SearchFilter& filter, bool recursive = true);
    
    static void printTree(const fs::path& dir, int maxDepth = 4, bool showHidden = false, std::ostream& os = std::cout);

    static DiskUsageSummary analyzeDiskUsage(const fs::path& dir, size_t topCount = 5);
    static void printDiskUsage(const DiskUsageSummary& summary, std::ostream& os = std::cout);
};
