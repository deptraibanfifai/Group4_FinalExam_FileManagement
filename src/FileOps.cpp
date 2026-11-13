#include "FileOps.hpp"
#include "TerminalUI.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cstring>

CompareResult FileOps::compareFiles(const fs::path& a, const fs::path& b, bool checkContent) {
    CompareResult res;
    res.pathA = a.string();
    res.pathB = b.string();

    FileMetadata metaA = FileMetadata::getInfo(a, checkContent, false);
    FileMetadata metaB = FileMetadata::getInfo(b, checkContent, false);

    res.existsA = metaA.exists;
    res.existsB = metaB.exists;

    if (!res.existsA || !res.existsB) {
        res.diffDetails = "One or both files do not exist or cannot be accessed.";
        return res;
    }

    res.sameType = (metaA.fileTypeStr == metaB.fileTypeStr);
    res.sameSize = (metaA.fileSize == metaB.fileSize);
    res.samePermissions = (metaA.octalPermissions == metaB.octalPermissions);
    res.sameModTime = (metaA.lastWriteTime == metaB.lastWriteTime);

    if (checkContent && metaA.isRegularFile && metaB.isRegularFile) {
        if (!res.sameSize) {
            res.contentIdentical = false;
            res.diffDetails = "File sizes differ (" + std::to_string(metaA.fileSize) + " vs " + std::to_string(metaB.fileSize) + " bytes)";
        } else {
            // Compare bytes or SHA-256
            if (!metaA.sha256Checksum.empty() && !metaB.sha256Checksum.empty()) {
                res.contentIdentical = (metaA.sha256Checksum == metaB.sha256Checksum);
            } else {
                std::ifstream f1(a, std::ios::binary);
                std::ifstream f2(b, std::ios::binary);
                std::vector<char> buf1(16384), buf2(16384);
                bool same = true;

                while (f1 && f2) {
                    f1.read(buf1.data(), buf1.size());
                    f2.read(buf2.data(), buf2.size());
                    if (f1.gcount() != f2.gcount() || std::memcmp(buf1.data(), buf2.data(), f1.gcount()) != 0) {
                        same = false;
                        break;
                    }
                }
                res.contentIdentical = same;
            }
            res.diffDetails = res.contentIdentical ? "Byte-by-byte content is identical." : "Content differs despite identical size.";
        }
    }

    return res;
}

void FileOps::printComparison(const CompareResult& res, std::ostream& os) {
    Terminal::printHeader("COMPARISON: " + res.pathA + " <===> " + res.pathB);

    if (!res.existsA || !res.existsB) {
        Terminal::printError(res.diffDetails);
        return;
    }

    auto printCmp = [&](const std::string& field, bool isSame) {
        os << std::left << std::setw(22) << field << ": "
           << (isSame ? (Terminal::FG_GREEN + "MATCH (identical)") : (Terminal::FG_RED + "DIFFERENT"))
           << Terminal::RESET << "\n";
    };

    printCmp("File Type", res.sameType);
    printCmp("File Size", res.sameSize);
    printCmp("Permissions", res.samePermissions);
    printCmp("Modification Time", res.sameModTime);
    printCmp("Content Hash", res.contentIdentical);

    if (!res.diffDetails.empty()) {
        os << "\n" << Terminal::DIM << "Note: " << res.diffDetails << Terminal::RESET << "\n";
    }
}

bool FileOps::copyPath(const fs::path& src, const fs::path& dst, bool recursive, bool overwrite) {
    std::error_code ec;
    auto options = fs::copy_options::none;
    if (overwrite) options |= fs::copy_options::overwrite_existing;
    if (recursive) options |= fs::copy_options::recursive;

    fs::copy(src, dst, options, ec);
    if (ec) {
        Terminal::printError("Failed to copy: " + ec.message());
        return false;
    }
    Terminal::printSuccess("Copied " + src.string() + " -> " + dst.string());
    return true;
}

bool FileOps::movePath(const fs::path& src, const fs::path& dst) {
    std::error_code ec;
    fs::rename(src, dst, ec);
    if (ec) {
        Terminal::printError("Failed to move: " + ec.message());
        return false;
    }
    Terminal::printSuccess("Moved " + src.string() + " -> " + dst.string());
    return true;
}

bool FileOps::deletePath(const fs::path& target, bool recursive) {
    std::error_code ec;
    if (recursive) {
        uintmax_t count = fs::remove_all(target, ec);
        if (ec) {
            Terminal::printError("Failed to remove: " + ec.message());
            return false;
        }
        Terminal::printSuccess("Removed " + std::to_string(count) + " item(s) from " + target.string());
        return true;
    } else {
        bool ok = fs::remove(target, ec);
        if (!ok || ec) {
            Terminal::printError("Failed to remove: " + (ec ? ec.message() : "Unknown error"));
            return false;
        }
        Terminal::printSuccess("Removed file " + target.string());
        return true;
    }
}

bool FileOps::createDir(const fs::path& p) {
    std::error_code ec;
    if (fs::create_directories(p, ec)) {
        Terminal::printSuccess("Directory created: " + p.string());
        return true;
    }
    if (fs::exists(p)) {
        Terminal::printWarning("Directory already exists: " + p.string());
        return true;
    }
    Terminal::printError("Cannot create directory: " + ec.message());
    return false;
}

bool FileOps::createFile(const fs::path& p) {
    std::ofstream ofs(p, std::ios::app);
    if (ofs.is_open()) {
        Terminal::printSuccess("File touched/created: " + p.string());
        return true;
    }
    Terminal::printError("Failed to create file: " + p.string());
    return false;
}

bool FileOps::viewHeadTail(const fs::path& p, size_t lines, bool head, std::ostream& os) {
    std::ifstream ifs(p);
    if (!ifs.is_open()) {
        Terminal::printError("Cannot open file: " + p.string());
        return false;
    }

    std::vector<std::string> allLines;
    std::string line;
    while (std::getline(ifs, line)) {
        allLines.push_back(line);
    }

    Terminal::printHeader(std::string(head ? "HEAD " : "TAIL ") + std::to_string(lines) + " lines of " + p.string());

    if (head) {
        for (size_t i = 0; i < std::min(lines, allLines.size()); ++i) {
            os << Terminal::DIM << std::setw(4) << (i + 1) << " | " << Terminal::RESET << allLines[i] << "\n";
        }
    } else {
        size_t start = (allLines.size() > lines) ? (allLines.size() - lines) : 0;
        for (size_t i = start; i < allLines.size(); ++i) {
            os << Terminal::DIM << std::setw(4) << (i + 1) << " | " << Terminal::RESET << allLines[i] << "\n";
        }
    }
    return true;
}
