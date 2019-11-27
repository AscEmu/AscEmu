/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include "MapMgr.h"

class SERVER_DECL Instance
{
    public:

        uint32_t m_instanceId {0};
        uint32_t m_mapId {0};
        MapMgr* m_mapMgr {nullptr};
        uint32_t m_creatorGuid {0};
        uint32_t m_creatorGroup {0};
        bool m_persistent {false};
        uint8_t m_difficulty {0};
        std::set<uint32_t> m_killedNpcs {};
        uint32_t m_creation {0};
        time_t m_expiration {0};
        MySQLStructure::MapInfo const* m_mapInfo {nullptr};
        bool m_isBattleground {false};

        [[nodiscard]] bool isPersistent() const;
        [[nodiscard]] bool isResetable() const;
};
