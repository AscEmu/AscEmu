/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgBattlefieldList : public ManagedPacket
    {
    public:
        uint32_t bgType;
        uint8_t fromType;

        CmsgBattlefieldList() : CmsgBattlefieldList(0, 0)
        {
        }

        CmsgBattlefieldList(uint32_t bgType, uint8_t fromType) :
            ManagedPacket(CMSG_BATTLEFIELD_LIST, 0),
            bgType(bgType),
            fromType(fromType)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                packet >> bgType >> fromType;
            else
                packet >> bgType;

            return true;
        }
    };
}
