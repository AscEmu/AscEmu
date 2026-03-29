/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgContactList : public ManagedPacket
    {
    public:
#if VERSION_STRING < Mop
        static constexpr uint16_t packetSize = 4;
#else
        static constexpr uint16_t packetSize = 5;
#endif
        uint32_t list_flag;

        CmsgContactList() : CmsgContactList(0)
        {
        }

        CmsgContactList(uint32_t list_flag) :
            ManagedPacket(CMSG_CONTACT_LIST, packetSize),
            list_flag(list_flag)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << list_flag;
#if VERSION_STRING >= Mop
            packet << uint8_t(0);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> list_flag;
#if VERSION_STRING >= Mop
            packet.read_skip<uint8_t>();
#endif
            return true;
        }
    };
}
