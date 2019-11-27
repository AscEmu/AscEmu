/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include <cstdint>
#include "WorldCreator.h"

uint32_t InstanceMgr::getNextInstanceId()
{
    if (m_InstanceHigh == 0)
    {
        if (QueryResult* result = CharacterDatabase.Query("SELECT MAX(id) FROM instances"))
            return result->Fetch()[0].GetUInt32() + 1;

        return 1;
    }

    m_mapLock.Acquire();
    const auto nextInstanceId = m_InstanceHigh++;
    m_mapLock.Release();

    return nextInstanceId;
}

