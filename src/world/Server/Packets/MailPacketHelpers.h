/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/WorldPacket.hpp"
#include "Server/ClientProtocol.hpp"
#include "WoWGuid.hpp"

#include <cstdint>

namespace AscEmu::Packets::MailWire
{
    // the ids of mails and attachments: 32 bit up to 9.x, 64 bit from 10.x on
    inline bool wideIds(WoW::ClientProtocol const& protocol)
    {
        return protocol.expansion >= WoW::Expansion::_Dragonflight;
    }

    inline uint32_t readId(WorldPacket& packet, WoW::ClientProtocol const& protocol)
    {
        if (wideIds(protocol))
        {
            uint64_t id;
            packet >> id;
            return static_cast<uint32_t>(id);
        }

        uint32_t id;
        packet >> id;
        return id;
    }

    inline void writeId(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t id)
    {
        if (wideIds(protocol))
            packet << uint64_t(id);
        else
            packet << id;
    }

    inline uint64_t readGuid(WorldPacket& packet)
    {
        WoWGuid128 guid;
        packet >> guid;
        return WoWGuid::fromGuid128(guid).getRawGuid();
    }
}
