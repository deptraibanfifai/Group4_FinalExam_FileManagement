#include "DirScanner.hpp"
#include "TerminalUI.hpp"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>

namespace {
    bool matchPattern(const std::string& str, const std::string& pattern) {
        if (pattern.empty()) return true;
        // Simple case-insensitive substring search or basic wildcard
        auto toLower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        };
        std::string sLower = toLower(str);
        std::string pLower = toLower(pattern);

        return sLower.find(pLower) != std::string::npos;
    }
}

std::vector<FileMetadata> DirScanner::listDirectory(const fs::path& dir, bool recursive, const std::string& nameFilter) {
    std::vector<FileMetadata> results;
    std::error_code ec;

    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
        return results;
    }

    auto processEntry = [&](const fs::path& path) {
        std::string filename = path.filename().string();
        if (matchPattern(filename, nameFilter)) {
            results.push_back(FileMetadata::getInfo(path, false, false));
        }
    };

    if (recursive) {
        for (const auto& entry : fs::recursive_directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {
            processEntry(entry.path());
        }
    } else {
        for (const auto& entry : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {
            processEntry(entry.path());
        }
    }

    return results;
}

std::vector<FileMetadata> DirScanner::search(const fs::path& dir, const SearchFilter& filter, bool recursive) {
    std::vector<FileMetadata> results;
    std::error_code ec;

    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
        return results;
    }

    auto checkAndAdd = [&](const fs::path& p) {
        std::string fname = p.filename().string();

        if (!filter.namePattern.empty() && !matchPattern(fname, filter.namePattern)) {
            return;
        }

        if (!filter.extension.empty()) {
            std::string ext = p.extension().string();
            if (!matchPattern(ext, filter.extension)) return;
        }

        FileMetadata meta = FileMetadata::getInfo(p, false, false);
        if (!meta.exists) return;

        if (filter.onlyDirs && !meta.isDirectory) return;
        if (filter.onlyFiles && !meta.isRegularFile) return;

        if (filter.minSize >= 0 && static_cast<int64_t>(meta.fileSize) < filter.minSize) return;
        if (filter.maxSize >= 0 && static_cast<int64_t>(meta.fileSize) > filter.maxSize) return;

        results.push_back(meta);
    };

    if (recursive) {
        for (const auto& entry : fs::recursive_directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {
            checkAndAdd(entry.path());
        }
    } else {
        for (const auto& entry : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {
            checkAndAdd(entry.path());
        }
    }

    return results;
}

static void printTreeRecursive(const fs::path& currentDir, const std::string& prefix, int currentDepth, int maxDepth, bool showHidden, std::ostream& os) {
    if (currentDepth > maxDepth) return;

    std::error_code ec;
    std::vector<fs::directory_entry> entries;
    for (const auto& entry : fs::directory_iterator(currentDir, fs::directory_options::skip_permission_denied, ec)) {
        std::string fname = entry.path().filename().string();
        if (!showHidden && !fname.empty() && fname[0] == '.') continue;
        entries.push_back(entry);
    }

    // Sort: directories first, then alphabetically
    std::sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
        bool aDir = a.is_directory();
        bool bDir = b.is_directory();
        if (aDir != bDir) return aDir > bDir;
        return a.path().filename().string() < b.path().filename().string();
    });

    for (size_t i = 0; i < entries.size(); ++i) {
        bool isLast = (i == entries.size() - 1);
        const auto& entry = entries[i];
        std::string name = entry.path().filename().string();

        os << prefix << (isLast ? "└── " : "├── ");

        bool isDir = entry.is_directory();
        bool isSymlink = entry.is_symlink();
        os << Terminal::colorPath(name, isDir, isSymlink, false);

        if (!isDir && entry.is_regular_file()) {
            uintmax_t sz = entry.file_size(ec);
            os << " " << Terminal::DIM << "(" << FileMetadata::formatBytes(sz) << ")" << Terminal::RESET;
        }
        os << "\n";

        if (isDir) {
            std::string nextPrefix = prefix + (isLast ? "    " : "│   ");
            printTreeRecursive(entry.path(), nextPrefix, currentDepth + 1, maxDepth, showHidden, os);
        }
    }
}

void DirScanner::printTree(const fs::path& dir, int maxDepth, bool showHidden, std::ostream& os) {
    std::error_code ec;
    if (!fs::exists(dir, ec)) {
        os << "Directory does not exist: " << dir << "\n";
        return;
    }

    os << Terminal::BOLD << Terminal::FG_BRIGHT_BLUE << dir.string() << Terminal::RESET << "\n";
    printTreeRecursive(dir, "", 1, maxDepth, showHidden, os);
}

DiskUsageSummary DirScanner::analyzeDiskUsage(const fs::path& dir, size_t topCount) {
    DiskUsageSummary summary;
    std::error_code ec;

    std::vector<std::pair<std::string, uintmax_t>> allFiles;

    for (const auto& entry : fs::recursive_directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {
        if (entry.is_directory()) {
            summary.totalDirs++;
        } else if (entry.is_regular_file()) {
            summary.totalFiles++;
            uintmax_t sz = entry.file_size(ec);
            if (ec) sz = 0;

            summary.totalBytes += sz;

            std::string ext = entry.path().extension().string();
            if (ext.empty()) ext = "[no ext]";
            summary.extensionSizes[ext] += sz;
            summary.extensionCounts[ext]++;

            allFiles.push_back({entry.path().string(), sz});
        }
    }

    std::sort(allFiles.begin(), allFiles.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    for (size_t i = 0; i < std::min(topCount, allFiles.size()); ++i) {
        summary.largestFiles.push_back(allFiles[i]);
    }

    return summary;
}

void DirScanner::printDiskUsage(const DiskUsageSummary& summary, std::ostream& os) {
    Terminal::printHeader("DISK USAGE ANALYSIS SUMMARY");
    os << "Total Size:        " << Terminal::BOLD << FileMetadata::formatBytes(summary.totalBytes) << Terminal::RESET << " (" << summary.totalBytes << " bytes)\n";
    os << "Total Files:       " << summary.totalFiles << "\n";
    os << "Total Directories: " << summary.totalDirs << "\n\n";

    os << Terminal::FG_YELLOW << Terminal::BOLD << "Top Largest Files:" << Terminal::RESET << "\n";
    if (summary.largestFiles.empty()) {
        os << "  (none)\n";
    } else {
        for (size_t i = 0; i < summary.largestFiles.size(); ++i) {
            os << "  " << (i + 1) << ". " << std::left << std::setw(50) << summary.largestFiles[i].first
               << " " << Terminal::FG_GREEN << FileMetadata::formatBytes(summary.largestFiles[i].second) << Terminal::RESET << "\n";
        }
    }

    os << "\n" << Terminal::FG_CYAN << Terminal::BOLD << "Usage by Extension:" << Terminal::RESET << "\n";
    os << "  " << std::left << std::setw(15) << "Extension"
       << std::setw(12) << "Count"
       << std::setw(16) << "Total Size"
       << "Percentage\n";
    os << "  " << std::string(55, '-') << "\n";

    // Sort extensions by size
    std::vector<std::pair<std::string, uintmax_t>> extVec(summary.extensionSizes.begin(), summary.extensionSizes.end());
    std::sort(extVec.begin(), extVec.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    for (const auto& item : extVec) {
        double pct = (summary.totalBytes > 0) ? (static_cast<double>(item.second) * 100.0 / summary.totalBytes) : 0.0;
        os << "  " << std::left << std::setw(15) << item.first
           << std::setw(12) << summary.extensionCounts.at(item.first)
           << std::setw(16) << FileMetadata::formatBytes(item.second)
           << std::fixed << std::setprecision(1) << pct << " %\n";
    }
}
