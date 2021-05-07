/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "Instance.h"
#include "InstanceDefines.hpp"
#include "WorldCreatorDefines.hpp"


bool Instance::isPersistent() const
{
    return this->m_mapInfo->type == INSTANCE_MULTIMODE && this->m_difficulty >= InstanceDifficulty::DUNGEON_HEROIC || this->m_mapInfo->type == INSTANCE_RAID;
}

bool Instance::isResetable() const
{
    return !this->m_persistent && (this->m_mapInfo->type == INSTANCE_NONRAID || this->m_mapInfo->type == INSTANCE_MULTIMODE && this->m_difficulty == InstanceDifficulty::DUNGEON_NORMAL);
}
