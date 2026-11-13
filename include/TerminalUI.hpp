#pragma once

#include <iostream>
#include <string>

namespace Terminal {
    // ANSI Color codes
    const std::string RESET       = "\033[0m";
    const std::string BOLD        = "\033[1m";
    const std::string DIM         = "\033[2m";
    const std::string UNDERLINE   = "\033[4m";

    const std::string FG_BLACK    = "\033[30m";
    const std::string FG_RED      = "\033[31m";
    const std::string FG_GREEN    = "\033[32m";
    const std::string FG_YELLOW   = "\033[33m";
    const std::string FG_BLUE     = "\033[34m";
    const std::string FG_MAGENTA  = "\033[35m";
    const std::string FG_CYAN     = "\033[36m";
    const std::string FG_WHITE    = "\033[37m";

    const std::string FG_BRIGHT_BLUE   = "\033[94m";
    const std::string FG_BRIGHT_GREEN  = "\033[92m";
    const std::string FG_BRIGHT_CYAN   = "\033[96m";
    const std::string FG_BRIGHT_YELLOW = "\033[93m";
    const std::string FG_BRIGHT_RED    = "\033[91m";
    const std::string FG_BRIGHT_WHITE  = "\033[97m";

    const std::string BG_BLUE     = "\033[44m";
    const std::string BG_DARK     = "\033[40m";

    void initColorSupport();
    std::string colorPath(const std::string& path, bool isDir, bool isSymlink, bool isExec);
    void printHeader(const std::string& title);
    void printSuccess(const std::string& msg);
    void printWarning(const std::string& msg);
    void printError(const std::string& msg);
}
