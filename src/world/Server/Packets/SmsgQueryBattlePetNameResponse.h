/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BattlePetPacketData.hpp"

#include <cstdint>
#include <ctime>
#include <string>

namespace AscEmu::Packets
{
    // the name of a summoned pet for every client that sees it
    class SmsgQueryBattlePetNameResponse : public ManagedPacket
    {
    public:
        uint32_t petId = 0;
        uint32_t creatureId = 0;
        time_t timestamp = 0;
        std::string name;
        bool allow = false;

        SmsgQueryBattlePetNameResponse(uint32_t petId, uint32_t creatureId, time_t timestamp, std::string name, bool allow) :
            ManagedPacket(SMSG_QUERY_BATTLE_PET_NAME_RESPONSE, 0),
            petId(petId),
            creatureId(creatureId),
            timestamp(timestamp),
            name(std::move(name)),
            allow(allow)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Shadowlands)
            {
                BattlePetWire::writeGuid(packet, petId);
                packet << int32_t(creatureId) << int64_t(timestamp);
                packet.writeBit(allow);
                if (allow)
                {
                    packet.writeBits(static_cast<uint32_t>(name.size()), 8);
                    packet.writeBit(false);             // declined names
                    for (uint8_t i = 0; i < 5; ++i)
                        packet.writeBits(0, 7);
                    packet.flushBits();
                    packet.writeString(name);
                    return true;
                }

                packet.flushBits();
                return true;
            }

            // 7.x and 8.x: the time is packed, the declined names follow in their own bytes
            if (m_protocol.isLegion() || m_protocol.isBfA())
            {
                BattlePetWire::writeGuid(packet, petId);
                packet << creatureId;
                packet.appendPackedTime(timestamp);
                packet.writeBit(allow);
                if (!allow)
                {
                    packet.flushBits();
                    return true;
                }

                packet.writeBits(static_cast<uint32_t>(name.size()), 8);
                packet.writeBit(false);                 // declined names
                packet.flushBits();
                for (uint8_t i = 0; i < 5; ++i)
                    packet.writeBits(0, 7);
                packet.flushBits();
                packet.writeString(name);
                return true;
            }

            if (m_protocol.isMop())
            {
                packet << uint64_t(petId) << uint32_t(timestamp) << creatureId;
                packet.writeBit(true);
                packet.writeBits(static_cast<uint32_t>(name.size()), 8);
                for (uint8_t i = 0; i < 5; ++i)
                    packet.writeBits(0, 7);
                packet.writeBit(false);
                packet.flushBits();
                packet.writeString(name);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
