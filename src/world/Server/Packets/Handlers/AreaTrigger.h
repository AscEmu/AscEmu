/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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
#pragma once

#include <string>

enum AreaTriggerType
{
    ATTYPE_NULL         = 0,
    ATTYPE_INSTANCE     = 1,
    ATTYPE_QUESTTRIGGER = 2,
    ATTYPE_INN          = 3,
    ATTYPE_TELEPORT     = 4,
    ATTYPE_SPELL        = 5,
    ATTYPE_BATTLEGROUND = 6
};

#pragma pack(push,1)
typedef struct AreaTrigger
{
    uint32_t AreaTriggerID;
    uint8_t Type;
    uint32_t Mapid;
    uint32_t PendingScreen;
    std::string Name;
    float x;
    float y;
    float z;
    float o;
    uint32_t required_honor_rank;
    uint32_t required_level;
} AreaTrigger;
#pragma pack(pop)
