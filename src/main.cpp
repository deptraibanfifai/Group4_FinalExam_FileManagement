#include "FileInfo.hpp"
#include "DirScanner.hpp"
#include "FileOps.hpp"
#include "ReportExporter.hpp"
#include "TerminalUI.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

void printBanner() {
    std::cout << Terminal::FG_BRIGHT_CYAN << Terminal::BOLD;
    std::cout << R"(
  ███████╗██╗██╗     ███████╗    ███╗   ███╗ ██████╗ ████████╗
  ██╔════╝██║██║     ██╔════╝    ████╗ ████║██╔════╝ ╚══██╔══╝
  █████╗  ██║██║     █████╗      ██╔████╔██║██║  ███╗   ██║   
  ██╔══╝  ██║██║     ██╔══╝      ██║╚██╔╝██║██║   ██║   ██║   
  ██║     ██║███████╗███████╗    ██║ ╚═╝ ██║╚██████╔╝   ██║   
  ╚═╝     ╚═╝╚══════╝╚══════╝    ╚═╝     ╚═╝ ╚═════╝    ╚═╝   
)" << Terminal::RESET;
    std::cout << Terminal::DIM << "  Professional C++ Terminal File Manager & Metadata Reporter v2.0\n" << Terminal::RESET;
}

void printUsage(const std::string& prog) {
    std::cout << Terminal::FG_YELLOW << Terminal::BOLD << "Usage:\n" << Terminal::RESET;
    std::cout << "  " << prog << " info [-s|--sha256] <PATH> [PATH...]\n";
    std::cout << "  " << prog << " list [-R] <DIRECTORY> [NAME_FILTER]\n";
    std::cout << "  " << prog << " tree <DIRECTORY> [MAX_DEPTH=3]\n";
    std::cout << "  " << prog << " search [-R] <DIRECTORY> <KEYWORD> [--ext .ext] [--min-size bytes] [--max-size bytes]\n";
    std::cout << "  " << prog << " du|analyze <DIRECTORY>\n";
    std::cout << "  " << prog << " compare|diff <PATH1> <PATH2> [--no-content]\n";
    std::cout << "  " << prog << " export <txt|csv|json|md> <OUTPUT_FILE> <PATH> [PATH...]\n";
    std::cout << "  " << prog << " cp|copy [-r] <SRC> <DST>\n";
    std::cout << "  " << prog << " mv|move <SRC> <DST>\n";
    std::cout << "  " << prog << " rm|delete [-r] <TARGET>\n";
    std::cout << "  " << prog << " touch <FILE>\n";
    std::cout << "  " << prog << " mkdir <DIR>\n";
    std::cout << "  " << prog << " head|tail <FILE> [LINES=20]\n";
    std::cout << "  " << prog << " interactive              (Start interactive terminal shell)\n";
    std::cout << "\n" << Terminal::DIM << "Note: If run without arguments, interactive shell starts automatically.\n" << Terminal::RESET;
}

void runInteractiveShell(const std::string& progName) {
    Terminal::printHeader("INTERACTIVE TERMINAL FILE MANAGER");
    std::cout << "Type 'help' for commands, 'exit' or 'quit' to leave.\n\n";

    fs::path currentDir = fs::current_path();

    while (true) {
        std::cout << Terminal::FG_BRIGHT_BLUE << Terminal::BOLD << "[" << currentDir.string() << "]# " << Terminal::RESET;
        std::string line;
        if (!std::getline(std::cin, line)) break;

        // Trim
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) line.erase(line.begin());
        while (!line.empty() && (line.back() == ' ' || line.back() == '\t')) line.pop_back();

        if (line.empty()) continue;
        if (line == "exit" || line == "quit" || line == "q") {
            std::cout << "Exiting File Manager. Goodbye!\n";
            break;
        }

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;
        std::vector<std::string> args;
        std::string arg;
        while (ss >> arg) {
            args.push_back(arg);
        }

        if (cmd == "help") {
            printUsage(progName);
        } else if (cmd == "pwd") {
            std::cout << currentDir.string() << "\n";
        } else if (cmd == "cd") {
            if (args.empty()) {
                currentDir = fs::current_path();
            } else {
                fs::path target = currentDir / args[0];
                std::error_code ec;
                if (fs::is_directory(target, ec)) {
                    currentDir = fs::canonical(target, ec);
                } else {
                    Terminal::printError("Directory not found: " + args[0]);
                }
            }
        } else if (cmd == "ls" || cmd == "list") {
            bool rec = false;
            std::string filter = "";
            for (const auto& a : args) {
                if (a == "-R" || a == "-r") rec = true;
                else filter = a;
            }
            auto files = DirScanner::listDirectory(currentDir, rec, filter);
            for (const auto& f : files) {
                std::cout << Terminal::colorPath(f.pathStr, f.isDirectory, f.isSymlink, false)
                          << "  " << std::setw(10) << f.permissionsStr
                          << "  " << std::setw(12) << f.humanReadableSize
                          << "  " << f.lastWriteTime << "\n";
            }
        } else if (cmd == "info" || cmd == "stat") {
            if (args.empty()) {
                Terminal::printWarning("Usage: info [-s] <PATH>");
            } else {
                bool hash = false;
                for (const auto& a : args) {
                    if (a == "-s" || a == "--sha256") hash = true;
                    else {
                        fs::path p = currentDir / a;
                        FileMetadata meta = FileMetadata::getInfo(p, hash);
                        meta.printDetailed(std::cout, true);
                    }
                }
            }
        } else if (cmd == "tree") {
            int depth = 3;
            if (!args.empty()) depth = std::stoi(args[0]);
            DirScanner::printTree(currentDir, depth);
        } else if (cmd == "du" || cmd == "analyze") {
            auto summary = DirScanner::analyzeDiskUsage(currentDir);
            DirScanner::printDiskUsage(summary);
        } else if (cmd == "compare" || cmd == "diff") {
            if (args.size() < 2) {
                Terminal::printWarning("Usage: compare <PATH1> <PATH2>");
            } else {
                auto res = FileOps::compareFiles(currentDir / args[0], currentDir / args[1], true);
                FileOps::printComparison(res);
            }
        } else if (cmd == "touch") {
            if (args.empty()) Terminal::printWarning("Usage: touch <filename>");
            else FileOps::createFile(currentDir / args[0]);
        } else if (cmd == "mkdir") {
            if (args.empty()) Terminal::printWarning("Usage: mkdir <dirname>");
            else FileOps::createDir(currentDir / args[0]);
        } else if (cmd == "rm") {
            bool rec = false;
            std::string target;
            for (const auto& a : args) {
                if (a == "-r" || a == "-R") rec = true;
                else target = a;
            }
            if (target.empty()) Terminal::printWarning("Usage: rm [-r] <target>");
            else FileOps::deletePath(currentDir / target, rec);
        } else if (cmd == "cp") {
            bool rec = false;
            std::vector<std::string> paths;
            for (const auto& a : args) {
                if (a == "-r" || a == "-R") rec = true;
                else paths.push_back(a);
            }
            if (paths.size() < 2) Terminal::printWarning("Usage: cp [-r] <src> <dst>");
            else FileOps::copyPath(currentDir / paths[0], currentDir / paths[1], rec, true);
        } else if (cmd == "mv") {
            if (args.size() < 2) Terminal::printWarning("Usage: mv <src> <dst>");
            else FileOps::movePath(currentDir / args[0], currentDir / args[1]);
        } else if (cmd == "head" || cmd == "tail") {
            if (args.empty()) Terminal::printWarning("Usage: head/tail <file> [lines]");
            else {
                size_t l = 20;
                if (args.size() >= 2) l = std::stoul(args[1]);
                FileOps::viewHeadTail(currentDir / args[0], l, (cmd == "head"));
            }
        } else {
            Terminal::printError("Unknown command: " + cmd + ". Type 'help' for available commands.");
        }
    }
}

int main(int argc, char* argv[]) {
    Terminal::initColorSupport();

    if (argc < 2) {
        printBanner();
        runInteractiveShell(argv[0]);
        return 0;
    }

    std::string command = argv[1];

    if (command == "--help" || command == "-h" || command == "help") {
        printBanner();
        printUsage(argv[0]);
        return 0;
    }

    if (command == "interactive" || command == "shell" || command == "-i") {
        printBanner();
        runInteractiveShell(argv[0]);
        return 0;
    }

    // Command: info [-s|--sha256] <PATH...>
    if (command == "info" || command == "stat") {
        bool computeHash = false;
        std::vector<fs::path> paths;

        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-s" || arg == "--sha256") {
                computeHash = true;
            } else {
                paths.push_back(arg);
            }
        }

        if (paths.empty()) {
            Terminal::printError("No files or directories specified.");
            printUsage(argv[0]);
            return 1;
        }

        for (const auto& p : paths) {
            FileMetadata meta = FileMetadata::getInfo(p, computeHash);
            meta.printDetailed(std::cout, true);
        }
        return 0;
    }

    // Command: list [-R] <DIRECTORY> [NAME_FILTER]
    if (command == "list" || command == "ls") {
        bool recursive = false;
        int idx = 2;
        if (idx < argc && (std::string(argv[idx]) == "-R" || std::string(argv[idx]) == "-r")) {
            recursive = true;
            idx++;
        }

        fs::path dir = (idx < argc) ? argv[idx++] : ".";
        std::string filter = (idx < argc) ? argv[idx] : "";

        auto files = DirScanner::listDirectory(dir, recursive, filter);
        Terminal::printHeader("LISTING " + dir.string() + (recursive ? " (Recursive)" : "") + (filter.empty() ? "" : " [Filter: " + filter + "]"));
        
        std::cout << std::left << std::setw(12) << "Permissions"
                  << std::setw(14) << "Size"
                  << std::setw(22) << "Last Modified"
                  << "Path\n";
        std::cout << std::string(75, '-') << "\n";

        for (const auto& f : files) {
            std::cout << std::left << std::setw(12) << f.permissionsStr
                      << std::setw(14) << f.humanReadableSize
                      << std::setw(22) << f.lastWriteTime << " "
                      << Terminal::colorPath(f.pathStr, f.isDirectory, f.isSymlink, false) << "\n";
        }
        std::cout << "\nTotal found: " << files.size() << " item(s)\n";
        return 0;
    }

    // Command: tree <DIRECTORY> [MAX_DEPTH]
    if (command == "tree") {
        fs::path dir = (argc > 2) ? argv[2] : ".";
        int maxDepth = (argc > 3) ? std::stoi(argv[3]) : 3;
        DirScanner::printTree(dir, maxDepth);
        return 0;
    }

    // Command: search [-R] <DIRECTORY> <KEYWORD> [--ext .ext] [--min-size sz] [--max-size sz]
    if (command == "search" || command == "find") {
        bool recursive = false;
        int idx = 2;
        if (idx < argc && (std::string(argv[idx]) == "-R" || std::string(argv[idx]) == "-r")) {
            recursive = true;
            idx++;
        }

        if (idx >= argc) {
            Terminal::printError("Directory and keyword required.");
            printUsage(argv[0]);
            return 1;
        }

        fs::path dir = argv[idx++];
        SearchFilter filter;
        if (idx < argc) {
            filter.namePattern = argv[idx++];
        }

        while (idx < argc) {
            std::string flag = argv[idx++];
            if (flag == "--ext" && idx < argc) {
                filter.extension = argv[idx++];
            } else if (flag == "--min-size" && idx < argc) {
                filter.minSize = std::stoll(argv[idx++]);
            } else if (flag == "--max-size" && idx < argc) {
                filter.maxSize = std::stoll(argv[idx++]);
            } else if (flag == "--dir-only") {
                filter.onlyDirs = true;
            } else if (flag == "--file-only") {
                filter.onlyFiles = true;
            }
        }

        auto results = DirScanner::search(dir, filter, recursive);
        Terminal::printHeader("SEARCH RESULTS in " + dir.string());
        for (const auto& item : results) {
            std::cout << Terminal::colorPath(item.pathStr, item.isDirectory, item.isSymlink, false)
                      << " (" << item.humanReadableSize << ", " << item.permissionsStr << ")\n";
        }
        std::cout << "\nFound " << results.size() << " match(es).\n";
        return 0;
    }

    // Command: du|analyze <DIRECTORY>
    if (command == "du" || command == "analyze") {
        fs::path dir = (argc > 2) ? argv[2] : ".";
        auto summary = DirScanner::analyzeDiskUsage(dir);
        DirScanner::printDiskUsage(summary);
        return 0;
    }

    // Command: compare|diff <PATH1> <PATH2> [--no-content]
    if (command == "compare" || command == "diff") {
        if (argc < 4) {
            Terminal::printError("Please provide two paths to compare.");
            printUsage(argv[0]);
            return 1;
        }
        bool checkContent = true;
        if (argc > 4 && std::string(argv[4]) == "--no-content") {
            checkContent = false;
        }
        auto res = FileOps::compareFiles(argv[2], argv[3], checkContent);
        FileOps::printComparison(res);
        return 0;
    }

    // Command: export <format> <output> <PATH...>
    if (command == "export") {
        if (argc < 5) {
            Terminal::printError("Usage: export <txt|csv|json|md> <OUTPUT_FILE> <PATH...>");
            return 1;
        }
        std::string fmtStr = argv[2];
        std::string outPath = argv[3];
        ExportFormat format = ReportExporter::parseFormat(fmtStr);

        std::vector<FileMetadata> metadataList;
        for (int i = 4; i < argc; ++i) {
            metadataList.push_back(FileMetadata::getInfo(argv[i], true));
        }

        return ReportExporter::exportReport(outPath, format, metadataList) ? 0 : 1;
    }

    // File operation utilities: cp, mv, rm, touch, mkdir, head, tail
    if (command == "cp" || command == "copy") {
        bool rec = false;
        int idx = 2;
        if (idx < argc && (std::string(argv[idx]) == "-r" || std::string(argv[idx]) == "-R")) {
            rec = true;
            idx++;
        }
        if (argc - idx < 2) {
            Terminal::printError("Usage: cp [-r] <SRC> <DST>");
            return 1;
        }
        return FileOps::copyPath(argv[idx], argv[idx + 1], rec, true) ? 0 : 1;
    }

    if (command == "mv" || command == "move") {
        if (argc < 4) {
            Terminal::printError("Usage: mv <SRC> <DST>");
            return 1;
        }
        return FileOps::movePath(argv[2], argv[3]) ? 0 : 1;
    }

    if (command == "rm" || command == "delete") {
        bool rec = false;
        int idx = 2;
        if (idx < argc && (std::string(argv[idx]) == "-r" || std::string(argv[idx]) == "-R")) {
            rec = true;
            idx++;
        }
        if (idx >= argc) {
            Terminal::printError("Usage: rm [-r] <TARGET>");
            return 1;
        }
        return FileOps::deletePath(argv[idx], rec) ? 0 : 1;
    }

    if (command == "touch") {
        if (argc < 3) {
            Terminal::printError("Usage: touch <FILE>");
            return 1;
        }
        return FileOps::createFile(argv[2]) ? 0 : 1;
    }

    if (command == "mkdir") {
        if (argc < 3) {
            Terminal::printError("Usage: mkdir <DIR>");
            return 1;
        }
        return FileOps::createDir(argv[2]) ? 0 : 1;
    }

    if (command == "head" || command == "tail") {
        if (argc < 3) {
            Terminal::printError("Usage: head/tail <FILE> [LINES]");
            return 1;
        }
        size_t lines = (argc > 3) ? std::stoul(argv[3]) : 20;
        return FileOps::viewHeadTail(argv[2], lines, (command == "head")) ? 0 : 1;
    }

    Terminal::printError("Unknown command: " + command);
    printUsage(argv[0]);
    return 1;
}
