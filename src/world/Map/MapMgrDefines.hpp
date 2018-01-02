/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

#define IS_PERSISTENT_INSTANCE(p) (((p)->m_mapInfo->type == INSTANCE_MULTIMODE && (p)->m_difficulty >= MODE_HEROIC) || (p)->m_mapInfo->type == INSTANCE_RAID)
#define IS_RESETABLE_INSTANCE(p) (!(p)->m_persistent && ((p)->m_mapInfo->type == INSTANCE_NONRAID || ((p)->m_mapInfo->type == INSTANCE_MULTIMODE && (p)->m_difficulty == MODE_NORMAL)))
#define CHECK_INSTANCE_GROUP(p, g) ((p)->m_creatorGroup == 0 || ((g) && (p)->m_creatorGroup == (g)->GetID()))

#define GO_GUID_RECYCLE_INTERVAL 2048       /// client will cache GO positions. Using same guid for same client will make GO appear at wrong possition so we try to avoid assigning same guid

#define ZONE_MASK_ALL -1
/// MapId -1 doesn't exist (0 is Eastern Kingdoms)
#define MAPID_NOT_IN_WORLD 0xFFFFFFFF
/// Instance Id 0 doesn't exist (-1 is World Instance)
#define INSTANCEID_NOT_IN_WORLD 0

enum MapMgrTimers
{
    MMUPDATE_OBJECTS        = 0,
    MMUPDATE_SESSIONS       = 1,
    MMUPDATE_FIELDS         = 2,
    MMUPDATE_IDLE_OBJECTS   = 3,
    MMUPDATE_ACTIVE_OBJECTS = 4,
    MMUPDATE_COUNT          = 5
};

enum ObjectActiveState
{
    OBJECT_STATE_NONE		= 0,
    OBJECT_STATE_INACTIVE	= 1,
    OBJECT_STATE_ACTIVE		= 2
};

#define MAX_TRANSPORTERS_PER_MAP 25
#define RESERVE_EXPAND_SIZE 1024
#define CALL_INSTANCE_SCRIPT_EVENT(Mgr, Func) if (Mgr != NULL && Mgr->GetScript() != NULL) Mgr->GetScript()->Func
