#pragma once

#include "FileInfo.hpp"
#include <iostream>
#include <string>
#include <vector>

struct CompareResult {
    std::string pathA;
    std::string pathB;
    bool existsA = false;
    bool existsB = false;
    bool sameType = false;
    bool sameSize = false;
    bool samePermissions = false;
    bool sameModTime = false;
    bool contentIdentical = false;
    std::string diffDetails;
};

class FileOps {
public:
    static CompareResult compareFiles(const fs::path& a, const fs::path& b, bool checkContent = true);
    static void printComparison(const CompareResult& res, std::ostream& os = std::cout);

    static bool copyPath(const fs::path& src, const fs::path& dst, bool recursive = true, bool overwrite = false);
    static bool movePath(const fs::path& src, const fs::path& dst);
    static bool deletePath(const fs::path& target, bool recursive = true);
    static bool createDir(const fs::path& p);
    static bool createFile(const fs::path& p);
    static bool viewHeadTail(const fs::path& p, size_t lines = 20, bool head = true, std::ostream& os = std::cout);
};
