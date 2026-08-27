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
#include "mpqlib/DBCFile.hpp"
#include "ADTFile.hpp"
#include "vmapexport.h"

#include <algorithm>
#include <memory>
#include <stdio.h>

extern std::unique_ptr<mpqlib::MpqPatchChain> WorldMpq;

bool ExtractSingleModel(std::string& fname)
{
    char* name = getPlainName((char*)fname.c_str());
    char* ext = getExtension(name);

    // < 3.1.0 ADT MMDX section store filename.mdx filenames for corresponded .m2 file
    if (!strcmp(ext, ".mdx"))
    {
        // replace .mdx -> .m2
        fname.erase(fname.length()-2,2);
        fname.append("2");
    }
    // >= 3.1.0 ADT MMDX section store filename.m2 filenames for corresponded .m2 file
    // nothing do

    std::string output(szWorkDirWmo);
    output += "/";
    output += name;

    if (FileExists(output.c_str()))
        return true;

    Model mdl(fname);
    if (!mdl.open())
        return false;

    return mdl.convertToVMapModel(output.c_str());
}

void ExtractGameobjectModels()
{
    printf("Extracting GameObject models...");
    DBCFile dbc(*WorldMpq, "DBFilesClient\\GameObjectDisplayInfo.dbc");
    if(!dbc.open())
    {
        printf("Fatal error: Invalid GameObjectDisplayInfo.dbc file format!\n");
        exit(1);
    }

    std::string basepath = szWorkDirWmo;
    basepath += "/";

    std::string modelListPath = basepath + "temp_gameobject_models";
    FILE* model_list = fopen(modelListPath.c_str(), "wb");
    if (!model_list)
    {
        printf("Fatal error: Could not open file %s\n", modelListPath.c_str());
        return;
    }

    for (auto const& record : dbc)
    {
        std::string path = record.getString(1);

        if (path.length() < 4)
            continue;

        fixNameCase((char*)path.c_str(), path.size());
        char* name = getPlainName((char*)path.c_str());
        fixNameSpaces(name, strlen(name));

        char* ch_ext = getExtension(name);
        if (!ch_ext)
            continue;

        strToLower(ch_ext);

        bool result = false;
        uint8_t isWmo = 0;
        if (!strcmp(ch_ext, ".wmo"))
        {
            isWmo = 1;
            result = ExtractSingleWmo(path);
        }
        else if (!strcmp(ch_ext, ".mdl"))
        {
            // TODO: extract .mdl files, if needed
            continue;
        }
        else //if (!strcmp(ch_ext, ".mdx") || !strcmp(ch_ext, ".m2"))
        {
            result = ExtractSingleModel(path);
        }

        if (result)
        {
            uint32_t displayId = record.getUInt(0);
            uint32_t path_length = static_cast<uint32_t>(strlen(name));
            fwrite(&displayId, sizeof(uint32_t), 1, model_list);
            fwrite(&isWmo, sizeof(uint8_t), 1, model_list);
            fwrite(&path_length, sizeof(uint32_t), 1, model_list);
            fwrite(name, sizeof(char), path_length, model_list);
        }
    }

    fclose(model_list);

    printf("Done!\n");
}
