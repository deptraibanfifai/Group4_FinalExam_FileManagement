#include "TerminalUI.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace Terminal {

void initColorSupport() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
    // Set UTF-8 code page on Windows
    SetConsoleOutputCP(CP_UTF8);
#endif
}

std::string colorPath(const std::string& path, bool isDir, bool isSymlink, bool isExec) {
    if (isDir) {
        return FG_BRIGHT_BLUE + BOLD + path + "/" + RESET;
    }
    if (isSymlink) {
        return FG_BRIGHT_CYAN + path + "@" + RESET;
    }
    if (isExec) {
        return FG_BRIGHT_GREEN + BOLD + path + "*" + RESET;
    }
    return FG_WHITE + path + RESET;
}

void printHeader(const std::string& title) {
    std::cout << "\n" << FG_BRIGHT_CYAN << BOLD << "=== " << title << " ===" << RESET << "\n";
}

void printSuccess(const std::string& msg) {
    std::cout << FG_BRIGHT_GREEN << "[SUCCESS] " << RESET << msg << "\n";
}

void printWarning(const std::string& msg) {
    std::cout << FG_BRIGHT_YELLOW << "[WARNING] " << RESET << msg << "\n";
}

void printError(const std::string& msg) {
    std::cerr << FG_BRIGHT_RED << "[ERROR] " << RESET << msg << "\n";
}

} // namespace Terminal
