/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= WotLK
            packet >> bgType >> fromType;
#else
            packet >> bgType;
#endif
            return true;
        }
    };
}
