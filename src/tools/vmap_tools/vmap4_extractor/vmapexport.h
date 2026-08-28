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

#ifndef VMAPEXPORT_H
#define VMAPEXPORT_H

#include "mpqlib/ClientVersion.hpp"

#include <string>

enum ModelFlags
{
    MOD_M2 = 1,
    MOD_WORLDSPAWN = 1<<1,
    MOD_HAS_BOUND = 1<<2
};

extern const char * szWorkDirWmo;
extern const char * szRawVMAPMagic;             // vmap magic string for extracted raw vmap data

// Detected once at startup (see main()). Classic/TBC/WotLK vs Cata/Mop steer
// which MPQ-discovery strategy runs and a few per-family output quirks;
// Classic/TBC vs everything else additionally steers the M2 header layout
// (see modelheaders.h) - two different boundaries, not one, so two
// predicates rather than a single "legacy" flag.
extern mpqlib::ClientVersion gClientVersion;
bool IsLegacyVmapArchiveLayout();
bool IsPreWotLKModelFormat();

bool FileExists(const char * file);
void strToLower(char* str);

bool ExtractSingleWmo(std::string& fname);
bool ExtractSingleModel(std::string& fname);

void ExtractGameobjectModels();

#endif  //VMAPEXPORT_H
