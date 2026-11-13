#pragma once

#include "FileInfo.hpp"
#include <string>
#include <vector>

enum class ExportFormat {
    TXT,
    CSV,
    JSON,
    MARKDOWN
};

class ReportExporter {
public:
    static bool exportReport(const std::string& outputPath,
                             ExportFormat format,
                             const std::vector<FileMetadata>& files);

    static ExportFormat parseFormat(const std::string& fmtStr);
};
