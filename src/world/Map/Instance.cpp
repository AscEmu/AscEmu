/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "Instance.h"
#include "InstanceDefines.hpp"
#include "WorldCreatorDefines.hpp"


bool Instance::isPersistent() const
{
    return (this->m_mapInfo->isMultimodeDungeon() && this->m_difficulty >= InstanceDifficulty::DUNGEON_HEROIC) || this->m_mapInfo->isRaid();
}

bool Instance::isResetable() const
{
    return !this->m_persistent && (this->m_mapInfo->isDungeon() || (this->m_mapInfo->isMultimodeDungeon() && this->m_difficulty == InstanceDifficulty::DUNGEON_NORMAL));
}
