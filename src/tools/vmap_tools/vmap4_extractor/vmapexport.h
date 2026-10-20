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

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

class MPQFile;

enum ModelFlags
{
    MOD_M2 = 1,
    MOD_WORLDSPAWN = 1<<1,
    MOD_HAS_BOUND = 1<<2
};

extern const char * szWorkDirWmo;
extern const char * szRawVMAPMagic;             // vmap magic string for extracted raw vmap data

// Guards the append-mode "dir_bin" instance-directory file, which every
// ADT tile's ModelInstance/WMOInstance writes to via its own independently
// fopen()'d FILE* - parallel ADT tiles would otherwise risk interleaving or
// splitting each other's multi-fwrite instance records. Held only around
// each instance's own fwrite burst (plus a flush before unlocking), not
// around the surrounding geometry/MPQ work, so it doesn't serialize the
// expensive part of tile processing.
extern std::mutex g_dirFileMutex;

// Detected once at startup (see main()). Classic/TBC/WotLK vs Cata/Mop steer
// which MPQ-discovery strategy runs and a few per-family output quirks;
// Classic/TBC vs everything else additionally steers the M2 header layout
// (see modelheaders.h) - two different boundaries, not one, so two
// predicates rather than a single "legacy" flag.
extern mpqlib::ClientVersion gClientVersion;
bool IsLegacyVmapArchiveLayout();
bool IsPreWotLKModelFormat();
// 6.x and later read the CASC storage instead of MPQ archives.
bool IsCascClient();

// Opens a client file by path on whichever source the client has (MPQ chain
// or CASC storage); a file data id name (see FileDataIdName) is resolved by id.
std::unique_ptr<MPQFile> OpenWorldFile(std::string const& fileName, bool warnNoExist = true);

// "FILE%08X.xxx": the name the tool gives a file that 8.1 and later only
// know by file data id; used for the output files and the instance directory alike.
std::string FileDataIdName(uint32_t fileDataId);
std::optional<uint32_t> FileDataIdFromName(std::string_view name);

// One extraction per output file at a time, the ADT tiles run in parallel.
std::mutex& LockForModelOutput(std::string const& outputPath);

bool FileExists(const char * file);
void strToLower(char* str);

bool ExtractSingleWmo(std::string& fname);
bool ExtractSingleModel(std::string& fname);

void ExtractGameobjectModels();

#endif  //VMAPEXPORT_H
