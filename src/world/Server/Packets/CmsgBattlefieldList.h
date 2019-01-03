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
            packet >> bgType >> fromType;
            return true;
        }
    };
}}
