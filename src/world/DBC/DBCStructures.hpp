/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2011 <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DBC_STRUCTURES_H
#define _DBC_STRUCTURES_H

#include "Common.h"

namespace DBC
{
    namespace Structures
    {
        namespace
        {
            char const area_table_entry_format[] = "iiinixxxxxissssssssssssssssxiiiiixxx";
			const char LFGDungeonEntryformat[] = "nssssssssssssssssxiiiiiiiiixxixixxxxxxxxxxxxxxxxx";
        }

        #pragma pack(push, 1)
        struct AreaTableEntry
        {
            uint32 id;                                             // 0
            uint32 map_id;                                          // 1
            uint32 zone;                                           // 2 if 0 then it's zone, else it's zone id of this area
            uint32 explore_flag;                                    // 3, main index
            uint32 flags;                                          // 4, unknown value but 312 for all cities
            // 5-9 unused
            int32   area_level;                                     // 10
            char*   area_name[16];                                  // 11-26
            // 27, string flags, unused
            uint32  team;                                           // 28
            uint32  liquid_type_override[4];                          // 29-32 liquid override by type
        };

		struct LFGDungeonEntry
		{
			uint32  ID;                                             // 0
			char*   name[16];                                       // 1-17 Name lang
			uint32  minlevel;                                       // 18
			uint32  maxlevel;                                       // 19
			uint32  reclevel;                                       // 20
			uint32  recminlevel;                                    // 21
			uint32  recmaxlevel;                                    // 22
			int32   map;                                            // 23
			uint32  difficulty;                                     // 24
			uint32  flags;                                          // 25
			uint32  type;                                           // 26
			//uint32  unk;                                          // 27
			//char*   iconname;                                     // 28
			uint32  expansion;                                      // 29
			//uint32  unk4;                                         // 30
			uint32  grouptype;                                      // 31
			//char*   desc[16];                                     // 32-47 Description
			// Helpers
			uint32 Entry() const { return ID + (type << 24); }
		};
        #pragma pack(pop)
    }
}

#endif // _DBC_GLOBALS_H