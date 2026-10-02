/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgSellAllJunkItems : public ManagedPacket
    {
    public:
        WoWGuid vendorGuid;

        CmsgSellAllJunkItems() : ManagedPacket(CMSG_SELL_ALL_JUNK_ITEMS, 1)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_protocol.isForever() ? 1 : 0;
        }

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

            vendorGuid.init(modernGuid.toLegacyRaw());
            return true;
        }
    };
}
