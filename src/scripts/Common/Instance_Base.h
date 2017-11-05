/*
 * Moon++ Scripts for Ascent MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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

#ifndef _INSTANCE_BASE_H
#define _INSTANCE_BASE_H

#include "Map/WorldCreatorDefines.hpp"

#define MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(ClassName, ParentClassName) \
public:\
    ADD_INSTANCE_FACTORY_FUNCTION(ClassName);\
    typedef ParentClassName ParentClass;

enum EncounterState
{
    State_NotStarted    = 0,
    State_InProgress    = 1,
    State_Finished      = 2,
    State_Performed     = 3,
    State_PreProgress   = 4,        // for example: violet hold
    State_InvalidState  = 0xff
};

enum InstanceType
{
    Type_Default    = INSTANCE_NULL,
    Type_Raid       = INSTANCE_RAID,
    Type_NonRaid    = INSTANCE_NONRAID,
    Type_PvP        = INSTANCE_BATTLEGROUND,    //pvp
    Type_MultiMode  = INSTANCE_MULTIMODE
};

enum InstanceMode
{
    Mode_Normal     = MODE_NORMAL_10MEN,
    Mode_Heroic     = MODE_NORMAL_25MEN,
    Mode_Normal_10  = MODE_NORMAL_10MEN,
    Mode_Normal_25  = MODE_NORMAL_25MEN,
    Mode_Heroic_10  = MODE_HEROIC_10MEN,
    Mode_Heroic_25  = MODE_HEROIC_25MEN
};

enum DataType
{
    Data_EncounterState     = 0,
    Data_EncounterProgress  = 1,
    Data_UnspecifiedType    = 2
};

enum GameObjectState
{
    State_Active    = 0,    // Door: open
    State_Inactive  = 1
};

//////////////////////////////////////////////////////////////////////////////////////////
//Class MoonInstanceScript
class MoonInstanceScript : public InstanceScript
{
    public:
        MoonInstanceScript(MapMgr* pMapMgr);
        virtual ~MoonInstanceScript();

        // Cells
        void SetCellForcedStates(float pMinX, float pMaxX, float pMinY, float pMaxY, bool pActivate = true);

        virtual void Destroy();
};

#endif      // _INSTANCE_BASE_H
