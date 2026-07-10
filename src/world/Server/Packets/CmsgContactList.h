/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgContactList : public ManagedPacket
    {
    public:
        uint32_t list_flag;

        CmsgContactList() : CmsgContactList(0)
        {
        }

        CmsgContactList(uint32_t list_flag) :
            ManagedPacket(CMSG_CONTACT_LIST, 0),
            list_flag(list_flag)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> list_flag;

            if (m_protocol.expansion == WoW::Expansion::_Mop)
                packet.readSkip<uint8_t>();

            return true;
        }
    };
}
