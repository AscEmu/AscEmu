/*
Copyright(c) 2014 - 2026 AscEmu Team < http://www.ascemu.org>
This file is released under the MIT license.See README - MIT for more information.
*/

#include "CascExtractor.hpp"
#include "Db2TableDefinitions.hpp"
#include "Db2Registry.hpp"
#include <CascLib.h>
#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>

#pragma pack(push, 1)
struct Db2HeaderCommon
{
    char magic[4]; // 'WDB5', 'WDB6', 'WDC1', ...
    uint32_t recordCount;
    uint32_t fieldCount;
    uint32_t recordSize;
    uint32_t stringTableSize;
    uint32_t tableHash; // 0x14: Jenkins hash of the table name (lowercase)
};
#pragma pack(pop)

namespace
{
    struct DiscoveredDb2
    {
        uint32_t fileDataId;
        uint32_t tableHash;
        std::string fileName;
    };

    bool isDb2Magic(const char* magic)
    {
        return (memcmp(magic, "WDBC", 4) == 0 ||
            memcmp(magic, "WDB2", 4) == 0 ||
            memcmp(magic, "WDB5", 4) == 0 ||
            memcmp(magic, "WDB6", 4) == 0 ||
            memcmp(magic, "WDC1", 4) == 0 ||
            memcmp(magic, "WDC2", 4) == 0 ||
            memcmp(magic, "WDC3", 4) == 0 ||
            memcmp(magic, "WDC4", 4) == 0 ||
            memcmp(magic, "WDC5", 4) == 0);
    }

    void printDefinitionBlock(const std::vector<DiscoveredDb2>& files, uint32_t buildNumber)
    {
        printf("\n==================== COPY CODE BELOW ====================\n\n");
        printf("#pragma once\n\n");
        printf("#include \"Db2Registry.hpp\"\n");
        printf("#include <array>\n\n");
        printf("namespace MapExtractor::Build_%u\n{\n", buildNumber);
        printf("    inline constexpr std::array<DB2::Db2Entry, %zu> DB2_FILES =\n    {\n", files.size());

        for (const auto& item : files)
        {
            printf("        DB2::Db2Entry{ %-8u, 0x%08Xu, \"%s\" },\n",
                   item.fileDataId, item.tableHash, item.fileName.c_str());
        }

        printf("    };\n}\n");
        printf("\n==================== COPY CODE ABOVE ====================\n\n");
    }

    size_t performFullScan(HANDLE storageHandle, uint32_t buildNumber, const fs::path& outputDir)
    {
        CASC_FIND_DATA findData{};
        HANDLE findHandle = CascFindFirstFile(storageHandle, "*", &findData, nullptr);

        if (findHandle == nullptr || findHandle == INVALID_HANDLE_VALUE)
        {
            printf("Fatal Error: CascFindFirstFile failed (Error: %lu)\n", GetLastError());
            return 0;
        }

        std::vector<DiscoveredDb2> discovered;
        size_t scannedCount = 0;

        printf("Scanning CASC archive for DB2 files...\n");

        do
        {
            ++scannedCount;
            if (scannedCount % 50000 == 0)
            {
                printf("Progress: %zu files scanned (%zu DB2s identified)\n", scannedCount, discovered.size());
            }

            if (findData.FileSize != CASC_INVALID_SIZE && findData.FileSize < sizeof(Db2HeaderCommon))
                continue;

            HANDLE fileHandle = nullptr;
            bool opened = false;

            // Attempt to open the file by its data ID first, if available
            if (findData.dwFileDataId != CASC_INVALID_ID && findData.dwFileDataId != 0)
            {
                opened = CascOpenFile(storageHandle, CASC_FILE_DATA_ID(findData.dwFileDataId), findData.dwLocaleFlags, CASC_OPEN_BY_FILEID, &fileHandle);
                if (!opened)
                    opened = CascOpenFile(storageHandle, CASC_FILE_DATA_ID(findData.dwFileDataId), CASC_LOCALE_ALL, CASC_OPEN_BY_FILEID, &fileHandle);
            }

            // Fallback to opening by name if the file data ID is invalid or the file couldn't be opened by ID
            if (!opened && findData.szFileName[0] != '\0')
            {
                opened = CascOpenFile(storageHandle, findData.szFileName, findData.dwLocaleFlags, CASC_OPEN_BY_NAME, &fileHandle);
                if (!opened)
                    opened = CascOpenFile(storageHandle, findData.szFileName, CASC_LOCALE_ALL, CASC_OPEN_BY_NAME, &fileHandle);
            }

            if (!opened)
                continue;

            DWORD fileSize = CascGetFileSize(fileHandle, nullptr);
            if (fileSize < sizeof(Db2HeaderCommon) || fileSize == CASC_INVALID_SIZE)
            {
                CascCloseFile(fileHandle);
                continue;
            }

            Db2HeaderCommon header{};
            DWORD bytesRead = 0;
            if (!CascReadFile(fileHandle, &header, sizeof(Db2HeaderCommon), &bytesRead) || bytesRead != sizeof(Db2HeaderCommon))
            {
                CascCloseFile(fileHandle);
                continue;
            }

            if (!isDb2Magic(header.magic))
            {
                CascCloseFile(fileHandle);
                continue;
            }

            std::vector<uint8_t> buffer(fileSize);
            CascSetFilePointer(fileHandle, 0, nullptr, FILE_BEGIN);
            if (!CascReadFile(fileHandle, buffer.data(), fileSize, &bytesRead) || bytesRead != fileSize)
            {
                CascCloseFile(fileHandle);
                continue;
            }
            CascCloseFile(fileHandle);

            std::string_view resolved = MapExtractor::DB2::getDb2FileNameByHash(header.tableHash);
            std::string fileName;

            if (!resolved.empty())
            {
                fileName = std::string(resolved);
            }
            else if (findData.szFileName[0] != '\0')
            {
                // TableHash is unknown, but we have a file name. Use the file name as-is.
                std::filesystem::path p(findData.szFileName);
                fileName = p.filename().string();
            }
            else
            {
                fileName = "Unknown_0x" + std::to_string(header.tableHash) + ".db2";
            }

            fs::path targetPath = outputDir / fileName;
            std::ofstream outFile(targetPath, std::ios::binary);
            if (outFile.is_open())
            {
                outFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
                discovered.push_back({findData.dwFileDataId, header.tableHash, fileName});
            }
        } while (CascFindNextFile(findHandle, &findData));

        CascFindClose(findHandle);

        printf("\nScan completed: %zu files checked, %zu DB2s extracted.\n", scannedCount, discovered.size());


        if (discovered.empty())
        {
            printf("\nWarning: No DB2 files discovered in this archive!\n");
        }
        else
        {
            // Sort alphabetically by file name for consistent output
            std::sort(discovered.begin(), discovered.end(), [](const DiscoveredDb2& a, const DiscoveredDb2& b) {
                return a.fileName < b.fileName;
            });

            // Print the definition block for the discovered DB2 files
            printDefinitionBlock(discovered, buildNumber);
        }

        // Wait for user input before exiting
        printf("Scan finished. Press ENTER to continue...\n");
        std::cin.clear();
        std::cin.sync();
        std::cin.get();

        return discovered.size();
    }

    bool extractSingleFileById(HANDLE storageHandle, uint32_t fileDataId, const fs::path& destinationPath)
    {
        HANDLE fileHandle = nullptr;
        bool opened = CascOpenFile(storageHandle, CASC_FILE_DATA_ID(fileDataId), CASC_LOCALE_ALL_WOW, CASC_OPEN_BY_FILEID, &fileHandle);
        if (!opened)
            opened = CascOpenFile(storageHandle, CASC_FILE_DATA_ID(fileDataId), CASC_LOCALE_NONE, CASC_OPEN_BY_FILEID, &fileHandle);
        if (!opened)
            opened = CascOpenFile(storageHandle, CASC_FILE_DATA_ID(fileDataId), CASC_LOCALE_ALL, CASC_OPEN_BY_FILEID, &fileHandle);

        if (!opened)
            return false;

        DWORD fileSize = CascGetFileSize(fileHandle, nullptr);
        if (fileSize == CASC_INVALID_SIZE || fileSize == 0)
        {
            CascCloseFile(fileHandle);
            return false;
        }

        std::vector<uint8_t> buffer(fileSize);
        DWORD bytesRead = 0;
        if (!CascReadFile(fileHandle, buffer.data(), fileSize, &bytesRead) || bytesRead != fileSize)
        {
            CascCloseFile(fileHandle);
            return false;
        }

        CascCloseFile(fileHandle);

        std::ofstream outFile(destinationPath, std::ios::binary);
        if (!outFile.is_open())
            return false;

        outFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        return true;
    }

    size_t extractPreindexedDb2(HANDLE storageHandle, const MapExtractor::DB2::BuildDefinition& def, const fs::path& outputDir)
    {
        printf("Extracting indexed DB2 files for %.*s (Builds %u-%u)...\n",
               static_cast<int>(def.expansionName.length()), def.expansionName.data(),
               def.minBuild, def.maxBuild);

        size_t successCount = 0;
        for (const auto& entry : def.files)
        {
            fs::path targetPath = outputDir / entry.fileName;
            if (extractSingleFileById(storageHandle, entry.fileDataId, targetPath))
            {
                ++successCount;
            }
            else
            {
                printf("Warning: Failed to extract %.*s (ID: %u)\n",
                       static_cast<int>(entry.fileName.length()), entry.fileName.data(), entry.fileDataId);
            }
        }
        return successCount;
    }
}

bool CascExtractor::run(const fs::path& clientPath, std::string_view expansionName, uint32_t buildNumber)
{
    printf("Initializing CASC storage for %.*s (Build: %u)...\n",
           static_cast<int>(expansionName.length()), expansionName.data(), buildNumber);

    HANDLE storageHandle = nullptr;
    if (!CascOpenStorage(clientPath.string().c_str(), CASC_LOCALE_ALL, &storageHandle))
    {
        printf("Fatal Error: Could not open CASC storage at '%s'\n", clientPath.string().c_str());
        return false;
    }

    printf("CASC Storage opened successfully!\n");

    fs::path dbcOutputDir = fs::current_path() / "dbc";
    fs::create_directories(dbcOutputDir);

    // Check if there's a pre-indexed definition for the given build number
    const auto* buildDef = MapExtractor::DB2::findBuildDefinition(buildNumber);

    if (buildDef != nullptr)
    {
        size_t count = extractPreindexedDb2(storageHandle, *buildDef, dbcOutputDir);
        printf("Instant extraction complete: %zu / %zu DB2 files written to '%s'.\n",
               count, buildDef->files.size(), dbcOutputDir.string().c_str());
    }
    else
    {
        // Unknown build, prompt user for full scan
        std::cout << "No indexed table for Build " << buildNumber << " found. Run full CASC scan (~1-2 min)? [y/N]: ";
        char choice = 'n';
        std::cin >> choice;

        if (choice == 'y' || choice == 'Y')
        {
            performFullScan(storageHandle, buildNumber, dbcOutputDir);
        }
        else
        {
            printf("Extraction skipped by user.\n");
        }
    }

    CascCloseStorage(storageHandle);
    return true;
}
