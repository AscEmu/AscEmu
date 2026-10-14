/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgCloseInteraction : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgCloseInteraction() : ManagedPacket(CMSG_CLOSE_INTERACTION, 0) {}

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            WoWGuid modernGuid;
            std::size_t consumed = 0;
            if (!WoWGuid::unpackModern(packet.contents() + packet.rpos(), packet.remaining(), modernGuid, consumed))
                return false;

            packet.rpos(packet.rpos() + consumed);
            if (packet.remaining() != 0)
                return false;

            guid.init(modernGuid.toLegacyRaw());
            return true;
        }
    };
}
