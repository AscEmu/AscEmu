/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "shared/WoWGuid.hpp"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class CmsgCharacterListAck : public ManagedPacket
    {
    public:
        uint32_t characterCount{0};
        std::vector<WoWGuid> characters;

        CmsgCharacterListAck() : ManagedPacket(CMSG_CHARACTER_LIST_ACK, sizeof(uint32_t)) {}

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet >> characterCount;
            characters.clear();
            if (characterCount > packet.remaining() / 2U)
                return false;

            characters.reserve(characterCount);

            for (uint32_t i = 0; i < characterCount; ++i)
            {
                WoWGuid guid;
                std::size_t consumed = 0;
                if (!WoWGuid::unpackModern(packet.contents() + packet.rpos(), packet.remaining(), guid, consumed) || consumed == 0 || consumed > packet.remaining())
                    return false;

                packet.rpos(packet.rpos() + consumed);
                characters.emplace_back(guid);
            }

            return packet.remaining() == 0;
        }
    };
}
