/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include "dbcfile.h"
#include "adtfile.h"
#include "vmapexport.h"

#include <algorithm>
#include <stdio.h>

bool ExtractSingleModel(std::string& fname, std::string& fixed_name, StringSet& failed_path_names)
{
    char const* ext = GetExtension(GetPlainName(fname.c_str()));

    // < 3.1.0 ADT MMDX section store filename.mdx filenames for corresponded .m2 file
    if (!strcmp(ext, ".mdx"))
    {
        // replace .mdx -> .m2
        fname.erase(fname.length() - 2, 2);
        fname.append("2");
    }
    // >= 3.1.0 ADT MMDX section store filename.m2 filenames for corresponded .m2 file
    // nothing do

    fixed_name = GetPlainName(fname.c_str());

    std::string output(szWorkDirWmo);
    output += "/";
    output += fixed_name;

    if (FileExists(output.c_str()))
        return true;

    Model mdl(fname);
    if (!mdl.open(failed_path_names))
        return false;

    return mdl.ConvertToVMAPModel(output.c_str());
}

void ExtractGameobjectModels()
{
    printf("Extracting GameObject models...");
    DBCFile dbc("DBFilesClient\\GameObjectDisplayInfo.dbc");
    if(!dbc.open())
    {
        printf("Fatal error: Invalid GameObjectDisplayInfo.dbc file format!\n");
        exit(1);
    }

    std::string basepath = szWorkDirWmo;
    basepath += "/";
    std::string path;
    StringSet failedPaths;

    std::string modelListPath = basepath + "temp_gameobject_models";
    FILE* model_list = fopen(modelListPath.c_str(), "wb");
    if (!model_list)
    {
        printf("Fatal error: Could not open file %s\n", modelListPath.c_str());
        return;
    }

    for (DBCFile::Iterator it = dbc.begin(); it != dbc.end(); ++it)
    {
        path = it->getString(1);

        if (path.length() < 4)
            continue;

        fixnamen((char*)path.c_str(), path.size());
        char* name = GetPlainName((char*)path.c_str());
        fixname2(name, strlen(name));

        char const* ch_ext = GetExtension(name);
        if (!ch_ext)
            continue;

        //strToLower(ch_ext);

        bool result = false;
        if (!strcmp(ch_ext, ".wmo"))
        {
            result = ExtractSingleWmo(path);
        }
        else if (!strcmp(ch_ext, ".mdl"))
        {
            // TODO: extract .mdl files, if needed
            continue;
        }
        else //if (!strcmp(ch_ext, ".mdx") || !strcmp(ch_ext, ".m2"))
        {
            std::string fixedName;
            result = ExtractSingleModel(path, fixedName, failedPaths);
        }

        if (result)
        {
            uint32 displayId = it->getUInt(0);
            uint32 path_length = strlen(name);
            fwrite(&displayId, sizeof(uint32), 1, model_list);
            fwrite(&path_length, sizeof(uint32), 1, model_list);
            fwrite(name, sizeof(char), path_length, model_list);
        }
    }

    fclose(model_list);

    printf("Done!\n");
}
