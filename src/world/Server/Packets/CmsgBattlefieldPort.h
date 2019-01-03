/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgBattlefieldPort : public ManagedPacket
    {
    public:
        uint16_t unknown;
        uint32_t bgType;
        uint16_t mapInfo;
        uint8_t action;


        CmsgBattlefieldPort() : CmsgBattlefieldPort(0, 0, 0, 0)
        {
        }

        CmsgBattlefieldPort(uint16_t unknown, uint32_t bgType, uint16_t mapInfo, uint8_t action) :
            ManagedPacket(CMSG_BATTLEFIELD_PORT, 0),
            unknown(unknown),
            bgType(bgType),
            mapInfo(mapInfo),
            action(action)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> unknown >> bgType >> mapInfo >> action;
            return true;
        }
    };
}}
