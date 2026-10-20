/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "model.h"
#include "mpqlib/CascStorage.hpp"
#include "mpqlib/ClientDb2Layouts.hpp"
#include "mpqlib/DBCFile.hpp"
#include "mpqlib/Db2File.hpp"
#include "ADTFile.hpp"
#include "vmapexport.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <mutex>
#include <stdio.h>
#include <unordered_map>

extern std::unique_ptr<mpqlib::MpqPatchChain> WorldMpq;
extern std::unique_ptr<mpqlib::MpqPatchChain> LocaleMpq;
extern std::unique_ptr<mpqlib::CascStorage> gCascStorage;

namespace
{
    // ExtractSingleModel's FileExists()-then-create check is only race-free
    // when calls are sequential. With ADT tiles processed in parallel, many
    // tiles commonly reference the very same model (a shared tree/rock
    // doodad) - sometimes via genuinely different source paths (case
    // variants, or a distinct MPQ entry) that normalize to the same output
    // filename, where one variant can fail to open while another succeeds.
    // A per-output-path mutex (rather than std::call_once) preserves the
    // original sequential fallback behavior under concurrency: only one
    // thread attempts extraction for a given filename at a time, but a
    // later caller still retries with its own input if the file is still
    // missing, instead of being permanently skipped because some earlier,
    // differently-sourced attempt already ran and failed.
    std::mutex g_modelExtractionRegistryMutex;
    std::unordered_map<std::string, std::unique_ptr<std::mutex>> g_modelExtractionLocks;
}

std::mutex& LockForModelOutput(std::string const& outputPath)
{
    std::lock_guard<std::mutex> lock(g_modelExtractionRegistryMutex);
    auto& slot = g_modelExtractionLocks[outputPath];
    if (!slot)
        slot = std::make_unique<std::mutex>();
    return *slot;
}

bool ExtractSingleModel(std::string& fname)
{
    // < 3.1.0 ADT MMDX section stores filename.mdx for the corresponding .m2 file.
    if (fname.length() >= 4 && fname.substr(fname.length() - 4, 4) == ".mdx")
    {
        fname.erase(fname.length()-2,2);
        fname.append("2");
    }
    // >= 3.1.0 ADT MMDX section stores filename.m2 directly - nothing to do.

    // The modern (Cata+) ADT MMDX path intentionally passes the raw,
    // un-normalized MMDX string here (see ADTFile::init()) - case/space-fix
    // the plain-name portion for the on-disk output filename while still
    // opening the model from the MPQ under its original path. A no-op
    // re-application for callers (legacy ADT MMDX, ExtractGameobjectModels)
    // that already normalized their path before calling.
    std::string originalName = fname;
    char* name = getPlainName(const_cast<char*>(fname.c_str()));
    fixNameCase(name, strlen(name));
    fixNameSpaces(name, strlen(name));

    std::string output(szWorkDirWmo);
    output += "/";
    output += name;

    if (FileExists(output.c_str()))
        return true;

    std::lock_guard<std::mutex> lock(LockForModelOutput(output));

    // Re-check: another thread may have finished extracting this exact
    // output while we were waiting for the lock.
    if (FileExists(output.c_str()))
        return true;

    Model mdl(originalName);
    if (!mdl.open())
        return false;

    return mdl.convertToVMapModel(output.c_str());
}

namespace
{
    // Writes one line of the game object model list: display id, kind, name.
    void writeGameObjectModel(FILE* modelList, uint32_t displayId, bool isWmo, char const* name)
    {
        uint8_t const kind = isWmo ? 1 : 0;
        uint32_t const nameLength = static_cast<uint32_t>(strlen(name));
        fwrite(&displayId, sizeof(uint32_t), 1, modelList);
        fwrite(&kind, sizeof(uint8_t), 1, modelList);
        fwrite(&nameLength, sizeof(uint32_t), 1, modelList);
        fwrite(name, sizeof(char), nameLength, modelList);
    }

    // The MPQ clients and 6.x name the model file in the table.
    bool extractGameObjectModelByPath(FILE* modelList, uint32_t displayId, std::string path)
    {
        if (path.length() < 4)
            return false;

        fixNameCase(path.data(), path.size());
        char* name = getPlainName(path.data());
        fixNameSpaces(name, strlen(name));

        char* ch_ext = getExtension(name);
        if (!ch_ext)
            return false;

        strToLower(ch_ext);

        bool result = false;
        bool isWmo = false;
        if (!strcmp(ch_ext, ".wmo"))
        {
            isWmo = true;
            result = ExtractSingleWmo(path);
        }
        else if (!strcmp(ch_ext, ".mdl"))
        {
            // TODO: extract .mdl files, if needed
            return false;
        }
        else //if (!strcmp(ch_ext, ".mdx") || !strcmp(ch_ext, ".m2"))
        {
            result = ExtractSingleModel(path);
        }

        if (result)
            writeGameObjectModel(modelList, displayId, isWmo, name);
        return result;
    }

    // 7.x and later name the model by file data id; the kind comes from the file's magic.
    bool extractGameObjectModelByFileDataId(FILE* modelList, uint32_t displayId, uint32_t fileDataId)
    {
        if (fileDataId == 0)
            return false;

        auto const magic = gCascStorage->readFilePrefix(fileDataId, 4);
        if (!magic || magic->size() < 4)
            return false;

        std::string path = FileDataIdName(fileDataId);
        bool result = false;
        bool isWmo = false;
        if (std::memcmp(magic->data(), "REVM", 4) == 0)
        {
            isWmo = true;
            result = ExtractSingleWmo(path);
            // a WMO without collision was not written
            result = result && FileExists((std::string(szWorkDirWmo) + "/" + path).c_str());
        }
        else if (std::memcmp(magic->data(), "MD20", 4) == 0 || std::memcmp(magic->data(), "MD21", 4) == 0)
            result = ExtractSingleModel(path);
        else
            return false;       // textures and other odd references of the table

        if (result)
            writeGameObjectModel(modelList, displayId, isWmo, path.c_str());
        return result;
    }

    void extractGameObjectModelsFromCasc(FILE* modelList)
    {
        auto const layouts = mpqlib::clientDb2LayoutsFor(gClientVersion);
        if (!layouts)
        {
            printf("No GameObjectDisplayInfo layout is known for this client, game object models are not extracted.\n");
            return;
        }

        mpqlib::GameObjectDisplayInfoColumns const& columns = layouts->gameObjectDisplayInfo;
        std::optional<std::vector<std::byte>> bytes;
        if (columns.table.fileDataId != 0)
            bytes = gCascStorage->readFile(columns.table.fileDataId);
        else
            bytes = gCascStorage->readFile(columns.table.fileName);
        if (!bytes)
        {
            printf("Fatal error: cannot find GameObjectDisplayInfo in the client!\n");
            return;
        }

        std::string error;
        auto table = mpqlib::Db2File::load(std::move(*bytes), error);
        if (!table)
        {
            printf("Fatal error: cannot read GameObjectDisplayInfo: %s\n", error.c_str());
            return;
        }

        if (columns.table.layoutHash != 0 && table->layoutHash() != columns.table.layoutHash)
            printf("Warning: GameObjectDisplayInfo has layout 0x%08X, the column indices were taken from layout 0x%08X\n", table->layoutHash(), columns.table.layoutHash);

        for (uint32_t i = 0; i < table->recordCount(); ++i)
        {
            if (!table->isReadable(i))
                continue;

            auto const record = table->record(i);
            if (columns.fileDataId >= 0)
                extractGameObjectModelByFileDataId(modelList, record.id(), record.getUInt32(static_cast<uint32_t>(columns.fileDataId)));
            else
                extractGameObjectModelByPath(modelList, record.id(), std::string(record.getString(columns.modelName)));
        }
    }
}

void ExtractGameobjectModels()
{
    printf("Extracting GameObject models...");

    std::string basepath = szWorkDirWmo;
    basepath += "/";

    std::string modelListPath = basepath + "temp_gameobject_models";
    FILE* model_list = fopen(modelListPath.c_str(), "wb");
    if (!model_list)
    {
        printf("Fatal error: Could not open file %s\n", modelListPath.c_str());
        return;
    }

    if (IsCascClient())
    {
        extractGameObjectModelsFromCasc(model_list);
        fclose(model_list);
        printf("Done!\n");
        return;
    }

    DBCFile dbc(IsLegacyVmapArchiveLayout() ? *WorldMpq : *LocaleMpq, "DBFilesClient\\GameObjectDisplayInfo.dbc");
    if(!dbc.open())
    {
        printf("Fatal error: Invalid GameObjectDisplayInfo.dbc file format!\n");
        exit(1);
    }

    for (auto const& record : dbc)
        extractGameObjectModelByPath(model_list, record.getUInt(0), record.getString(1));

    fclose(model_list);

    printf("Done!\n");
}
