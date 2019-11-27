/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "Instance.h"
#include "WorldCreatorDefines.hpp"


bool Instance::isPersistent() const
{
    return this->m_mapInfo->type == INSTANCE_MULTIMODE && this->m_difficulty >= MODE_HEROIC || this->m_mapInfo->type == INSTANCE_RAID;
}

bool Instance::isResetable() const
{
    return !this->m_persistent && (this->m_mapInfo->type == INSTANCE_NONRAID || this->m_mapInfo->type == INSTANCE_MULTIMODE && this->m_difficulty == MODE_NORMAL);
}
