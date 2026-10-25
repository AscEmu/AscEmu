/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WoWGuid.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    // a pet as the journal packets describe it
    struct BattlePetData
    {
        uint32_t id = 0;
        uint32_t species = 0;
        uint32_t creatureId = 0;
        uint32_t displayId = 0;
        uint16_t breed = 0;
        uint16_t level = 1;
        uint16_t experience = 0;
        uint16_t flags = 0;
        uint32_t power = 0;
        uint32_t health = 0;
        uint32_t maxHealth = 0;
        uint32_t speed = 0;
        uint8_t quality = 0;
        std::string name;
        bool noRename = false;
    };

    struct BattlePetSlotData
    {
        uint8_t index = 0;
        uint32_t petId = 0;                 // 0 for an empty slot
        bool locked = true;
    };

    // the encodings the battle pet packets share
    namespace BattlePetWire
    {
        // the pet guid of the 6.x and later clients, an empty slot names the pet type with counter 0
        inline void writeGuid(WorldPacket& packet, uint32_t petId)
        {
            packet << WoWGuid128::global(HighGuid128::BattlePet, petId);
        }

        inline uint32_t readGuid(WorldPacket& packet)
        {
            WoWGuid128 guid;
            packet >> guid;
            return static_cast<uint32_t>(guid.getCounter());
        }

        // a guid a 6.x or later client sends, as server guid
        inline uint64_t readObjectGuid(WorldPacket& packet)
        {
            WoWGuid128 guid;
            packet >> guid;
            return WoWGuid::fromGuid128(guid).getRawGuid();
        }

        // the pet of a 6.x and later journal, the same from 6.x to 12.x for a pet without an owner
        inline void writePet(WorldPacket& packet, BattlePetData const& pet)
        {
            writeGuid(packet, pet.id);
            packet << pet.species << pet.creatureId << pet.displayId;
            packet << pet.breed << pet.level << pet.experience << pet.flags;
            packet << pet.power << pet.health << pet.maxHealth << pet.speed << pet.quality;
            packet.writeBits(static_cast<uint32_t>(pet.name.size()), 7);
            packet.writeBit(false);                     // owner of a pet bound to one character
            packet.writeBit(pet.noRename);
            packet.flushBits();
            packet.writeString(pet.name);
        }

        inline void writeSlot(WorldPacket& packet, BattlePetSlotData const& slot)
        {
            writeGuid(packet, slot.petId);
            packet << uint32_t(0);                      // collar
            packet << slot.index;
            packet.writeBit(slot.locked);
            packet.flushBits();
        }

        // 7.x to 11.x announce the bits of the lists before their entries, 6.x and 12.x after them
        inline bool bitsBeforeLists(WoW::ClientProtocol const& protocol)
        {
            return protocol.expansion >= WoW::Expansion::_Legion && protocol.expansion <= WoW::Expansion::_TheWarWithin;
        }
    }
}
