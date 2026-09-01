/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "DbcReader.hpp"
#include "WDBFormat.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using dbc2sql::DbcReader;

namespace
{
    // Comfortably above the longest known format string (234 characters,
    // WotLK Spell.dbc) - used only to learn the file's true field
    // count/record size (see DbcReader::load()'s doc comment), never to
    // actually type a column.
    std::string const kProbeFormat(1024, 'i');

    struct Column
    {
        std::string name;
        std::string sqlType;
        char formatChar;   // 'i', 'f', 'b', 's' - decides how to print a value
        uint32_t fieldIndex; // index into the on-disk record (skipped format
                              // characters do not get a column, but still
                              // occupy a field index)
    };

    // The on-disk record size a format string implies. This is deliberately
    // NOT WDB::WDBLoader::getFormatRecordSize(): that function sizes the
    // *in-memory* struct produced after string fields are resolved to real
    // pointers (8 bytes on a 64-bit build) - on disk a string field is
    // always a 4-byte offset into the trailing string table, regardless of
    // pointer width. Returns false for any character it doesn't recognize,
    // since a candidate we can't size can't be trusted to match.
    bool onDiskRecordSize(std::string const& format, uint32_t& outSize)
    {
        outSize = 0;
        for (char c : format)
        {
            switch (c)
            {
                case 'i': case 'n': case 'f': case 's':
                    outSize += 4;
                    break;
                case 'b': case 'X':
                    outSize += 1;
                    break;
                case 'x': case 'd':
                    break;
                default:
                    return false;
            }
        }
        return true;
    }

    // Finds which of a DBC/DB2's (up to 5) per-version format strings in
    // WDBFormat.hpp's dbcFieldDefines matches this specific file, by
    // comparing each candidate's implied on-disk record size (and field
    // count) against what was actually read from the file's own header.
    // Returns "" if the filename isn't in the table, or none of its
    // non-empty candidates match.
    //
    // 115 of the table's 116 keys are spelled with a ".dbc" extension even
    // though the real Cata/Mop client ships many of those same tables as
    // ".db2" - mirrors WDB::WDBLoader::hasFormat()/getFormat()'s own
    // "force the last character to 'c'" lookup so a genuine "Foo.db2" still
    // finds the "Foo.dbc" entry.
    std::string findMatchingFormat(std::string const& filename, uint32_t realFieldCount, uint32_t realRecordSize)
    {
        std::string dbcName = filename;
        if (!dbcName.empty())
            dbcName.back() = 'c';

        auto it = dbcFieldDefines.find(filename);
        if (it == dbcFieldDefines.end())
            it = dbcFieldDefines.find(dbcName);
        if (it == dbcFieldDefines.end())
            return {};

        for (std::string const& candidate : it->second.format)
        {
            if (candidate.empty() || candidate.size() != realFieldCount)
                continue;

            uint32_t candidateSize = 0;
            if (onDiskRecordSize(candidate, candidateSize) && candidateSize == realRecordSize)
                return candidate;
        }

        return {};
    }

    std::string escapeSql(std::string const& s)
    {
        std::string out;
        out.reserve(s.size());
        for (char c : s)
        {
            switch (c)
            {
                case '\\': out += "\\\\"; break;
                case '\'': out += "\\'"; break;
                case '\0': out += "\\0"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                default: out += c; break;
            }
        }
        return out;
    }

    // Builds the emitted column list for a recognized format string,
    // matching the same character meanings WDB::WDBLoader::autoProduceData()/
    // autoProduceStrings() use (src/world/Storage/WDB/WDBLoader.cpp) - 'x'/
    // 'X'/'d' are not stored, matching that loader's own semantics.
    std::vector<Column> columnsFromFormat(std::string const& format)
    {
        std::vector<Column> columns;
        uint32_t fieldIndex = 0;
        for (char c : format)
        {
            switch (c)
            {
                case 'i': case 'n':
                    columns.push_back({"Field" + std::to_string(columns.size() + 1), "INT UNSIGNED", 'i', fieldIndex});
                    break;
                case 'f':
                    columns.push_back({"Field" + std::to_string(columns.size() + 1), "FLOAT", 'f', fieldIndex});
                    break;
                case 'b':
                    columns.push_back({"Field" + std::to_string(columns.size() + 1), "TINYINT UNSIGNED", 'b', fieldIndex});
                    break;
                case 's':
                    columns.push_back({"Field" + std::to_string(columns.size() + 1), "TEXT", 's', fieldIndex});
                    break;
                case 'x': case 'X': case 'd':
                    break;
                default:
                    fprintf(stderr, "  warning: unexpected format character '%c' at field %u, treating as raw uint32\n", c, fieldIndex);
                    columns.push_back({"Field" + std::to_string(columns.size() + 1), "INT UNSIGNED", 'i', fieldIndex});
                    break;
            }
            ++fieldIndex;
        }
        return columns;
    }

    // Fallback for a file not found in dbcFieldDefines: every on-disk field
    // dumped as a raw uint32, numbered in order. Intentionally simple - no
    // attempt to guess int vs. float vs. string offset.
    std::vector<Column> genericColumns(uint32_t fieldCount)
    {
        std::vector<Column> columns;
        columns.reserve(fieldCount);
        for (uint32_t i = 0; i < fieldCount; ++i)
            columns.push_back({"Field" + std::to_string(i + 1), "INT UNSIGNED", 'i', i});
        return columns;
    }

    bool writeSql(std::string const& outPath, std::string const& tableName,
        DbcReader const& reader, std::vector<Column> const& columns)
    {
        std::ofstream out(outPath, std::ios::binary);
        if (!out)
            return false;

        out << "DROP TABLE IF EXISTS `" << tableName << "`;\n";
        out << "CREATE TABLE `" << tableName << "` (\n";
        for (size_t i = 0; i < columns.size(); ++i)
            out << "    `" << columns[i].name << "` " << columns[i].sqlType << (i + 1 < columns.size() ? ",\n" : "\n");
        out << ");\n\n";

        if (reader.recordCount() == 0 || columns.empty())
            return true;

        constexpr uint32_t kBatchSize = 500;
        for (uint32_t start = 0; start < reader.recordCount(); start += kBatchSize)
        {
            uint32_t const end = std::min(start + kBatchSize, reader.recordCount());

            out << "INSERT INTO `" << tableName << "` (";
            for (size_t i = 0; i < columns.size(); ++i)
                out << "`" << columns[i].name << "`" << (i + 1 < columns.size() ? ", " : "");
            out << ") VALUES\n";

            for (uint32_t record = start; record < end; ++record)
            {
                out << "(";
                for (size_t i = 0; i < columns.size(); ++i)
                {
                    Column const& col = columns[i];
                    switch (col.formatChar)
                    {
                        case 'f':
                            out << reader.getFloat(record, col.fieldIndex);
                            break;
                        case 'b':
                            out << static_cast<int>(reader.getUInt8(record, col.fieldIndex));
                            break;
                        case 's':
                            out << "'" << escapeSql(reader.getString(record, col.fieldIndex)) << "'";
                            break;
                        default:
                            out << reader.getUInt32(record, col.fieldIndex);
                            break;
                    }
                    if (i + 1 < columns.size())
                        out << ", ";
                }
                out << ")" << (record + 1 < end ? ",\n" : ";\n");
            }
            out << "\n";
        }

        return true;
    }

    void processFile(std::string const& path)
    {
        fs::path const p(path);
        std::string const filename = p.filename().string();
        printf("Processing %s...\n", filename.c_str());

        DbcReader probe;
        if (!probe.load(path, kProbeFormat))
        {
            fprintf(stderr, "  error: unrecognized/unsupported file format (not WDBC or simple WDB2)\n");
            return;
        }

        std::string const matchedFormat = findMatchingFormat(filename, probe.fieldCount(), probe.recordSize());

        DbcReader typedReader;
        DbcReader const* reader = &probe;
        std::vector<Column> columns;

        if (!matchedFormat.empty())
        {
            if (!typedReader.load(path, matchedFormat))
            {
                fprintf(stderr, "  error: matched a known format but failed to reload the file\n");
                return;
            }
            reader = &typedReader;
            columns = columnsFromFormat(matchedFormat);
            printf("  recognized format (%zu columns)\n", columns.size());
        }
        else
        {
            columns = genericColumns(probe.fieldCount());
            printf("  unrecognized file - falling back to %u generic numbered columns\n", probe.fieldCount());
        }

        std::string const tableName = p.stem().string();
        std::string const outPath = (p.parent_path() / (tableName + ".sql")).string();

        if (!writeSql(outPath, tableName, *reader, columns))
        {
            fprintf(stderr, "  error: couldn't write %s\n", outPath.c_str());
            return;
        }

        printf("  wrote %s (%u rows)\n", outPath.c_str(), reader->recordCount());
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Usage: dbc2sql <file1.dbc|file1.db2> [file2 ...]\n");
        printf("Drop one or more .dbc/.db2 files onto this executable, or pass paths on the command line.\n");
        return 1;
    }

    for (int i = 1; i < argc; ++i)
        processFile(argv[i]);

    return 0;
}
