#include "ReportExporter.hpp"
#include "TerminalUI.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>

ExportFormat ReportExporter::parseFormat(const std::string& fmtStr) {
    if (fmtStr == "csv" || fmtStr == "CSV") return ExportFormat::CSV;
    if (fmtStr == "json" || fmtStr == "JSON") return ExportFormat::JSON;
    if (fmtStr == "md" || fmtStr == "markdown" || fmtStr == "MARKDOWN") return ExportFormat::MARKDOWN;
    return ExportFormat::TXT;
}

static std::string escapeCsv(const std::string& val) {
    std::string out = "\"";
    for (char c : val) {
        if (c == '"') out += "\"\"";
        else out += c;
    }
    out += "\"";
    return out;
}

static std::string escapeJson(const std::string& val) {
    std::string out;
    for (char c : val) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\b') out += "\\b";
        else if (c == '\f') out += "\\f";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

bool ReportExporter::exportReport(const std::string& outputPath,
                                 ExportFormat format,
                                 const std::vector<FileMetadata>& files) {
    std::ofstream ofs(outputPath);
    if (!ofs.is_open()) {
        Terminal::printError("Unable to open output file for writing: " + outputPath);
        return false;
    }

    switch (format) {
        case ExportFormat::CSV: {
            ofs << "Path,Type,Permissions,Octal,Size_Bytes,Size_Human,Links,Owner,Group,Last_Access,Last_Modified,Status_Creation,SHA256\n";
            for (const auto& f : files) {
                ofs << escapeCsv(f.pathStr) << ","
                    << escapeCsv(f.fileTypeStr) << ","
                    << escapeCsv(f.permissionsStr) << ","
                    << escapeCsv(f.octalPermissions) << ","
                    << f.fileSize << ","
                    << escapeCsv(f.humanReadableSize) << ","
                    << f.hardLinksCount << ","
                    << escapeCsv(f.ownerName) << ","
                    << escapeCsv(f.groupName) << ","
                    << escapeCsv(f.lastAccessTime) << ","
                    << escapeCsv(f.lastWriteTime) << ","
                    << escapeCsv(f.creationTime) << ","
                    << escapeCsv(f.sha256Checksum) << "\n";
            }
            break;
        }

        case ExportFormat::JSON: {
            ofs << "[\n";
            for (size_t i = 0; i < files.size(); ++i) {
                const auto& f = files[i];
                ofs << "  {\n";
                ofs << "    \"path\": \"" << escapeJson(f.pathStr) << "\",\n";
                ofs << "    \"type\": \"" << escapeJson(f.fileTypeStr) << "\",\n";
                ofs << "    \"permissions\": \"" << escapeJson(f.permissionsStr) << "\",\n";
                ofs << "    \"octal\": \"" << escapeJson(f.octalPermissions) << "\",\n";
                ofs << "    \"size_bytes\": " << f.fileSize << ",\n";
                ofs << "    \"size_human\": \"" << escapeJson(f.humanReadableSize) << "\",\n";
                ofs << "    \"hard_links\": " << f.hardLinksCount << ",\n";
                ofs << "    \"owner\": \"" << escapeJson(f.ownerName) << "\",\n";
                ofs << "    \"group\": \"" << escapeJson(f.groupName) << "\",\n";
                ofs << "    \"last_access\": \"" << escapeJson(f.lastAccessTime) << "\",\n";
                ofs << "    \"last_modified\": \"" << escapeJson(f.lastWriteTime) << "\",\n";
                ofs << "    \"creation_time\": \"" << escapeJson(f.creationTime) << "\",\n";
                ofs << "    \"sha256\": \"" << escapeJson(f.sha256Checksum) << "\"\n";
                ofs << "  }" << (i + 1 < files.size() ? "," : "") << "\n";
            }
            ofs << "]\n";
            break;
        }

        case ExportFormat::MARKDOWN: {
            ofs << "# File Management Report\n\n";
            ofs << "| Path | Type | Permissions | Size | Modified | SHA-256 |\n";
            ofs << "|------|------|-------------|------|----------|---------|\n";
            for (const auto& f : files) {
                std::string hashBrief = f.sha256Checksum.empty() ? "-" : f.sha256Checksum.substr(0, 10) + "...";
                ofs << "| `" << f.pathStr << "` | "
                    << f.fileTypeStr << " | `"
                    << f.permissionsStr << "` | "
                    << f.humanReadableSize << " | "
                    << f.lastWriteTime << " | `"
                    << hashBrief << "` |\n";
            }
            break;
        }

        case ExportFormat::TXT:
        default: {
            ofs << "================================================================================\n";
            ofs << "                         FILE MANAGEMENT REPORT                                 \n";
            ofs << "================================================================================\n\n";
            for (const auto& f : files) {
                f.printDetailed(ofs, false);
                ofs << "\n--------------------------------------------------------------------------------\n";
            }
            break;
        }
    }

    Terminal::printSuccess("Report exported successfully (" + std::to_string(files.size()) + " items) -> " + outputPath);
    return true;
}
