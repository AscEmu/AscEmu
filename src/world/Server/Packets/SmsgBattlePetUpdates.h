/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BattlePetPacketData.hpp"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    // pets that were learned or changed; 5.x takes one pet per packet
    class SmsgBattlePetUpdates : public ManagedPacket
    {
    public:
        std::vector<BattlePetData> pets;
        bool petAdded = false;

        SmsgBattlePetUpdates(std::vector<BattlePetData> pets, bool petAdded) :
            ManagedPacket(SMSG_BATTLE_PET_UPDATES, 0),
            pets(std::move(pets)),
            petAdded(petAdded)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet << uint32_t(pets.size());

                const bool addedFirst = BattlePetWire::bitsBeforeLists(m_protocol);
                if (addedFirst)
                {
                    packet.writeBit(petAdded);
                    packet.flushBits();
                }

                for (const auto& pet : pets)
                    BattlePetWire::writePet(packet, pet);

                if (!addedFirst)
                {
                    packet.writeBit(petAdded);
                    packet.flushBits();
                }
                return true;
            }

            if (m_protocol.isMop() && pets.size() == 1)
            {
                const BattlePetData& pet = pets.front();
                const WoWGuid guid(static_cast<uint64_t>(pet.id));

                packet.writeBits(1, 19);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[7]);
                packet.writeBit(pet.quality == 0);
                packet.writeBit(pet.breed == 0);
                packet.writeBit(guid[5]);
                packet.writeBit(false);
                packet.writeBit(guid[2]);
                packet.writeBit(pet.flags == 0);
                packet.writeBit(false);                 // bound to one character
                packet.writeBit(guid[6]);
                packet.writeBit(guid[3]);
                packet.writeBits(static_cast<uint32_t>(pet.name.size()), 7);
                packet.writeBit(guid[0]);
                packet.writeBit(petAdded);
                packet.flushBits();

                packet.writeByteSeq(guid[1]);
                packet.writeString(pet.name);
                packet << pet.health << pet.species;
                packet.writeByteSeq(guid[0]);
                packet << pet.power << pet.speed;
                if (pet.breed != 0)
                    packet << pet.breed;
                packet.writeByteSeq(guid[4]);
                packet << pet.creatureId << pet.maxHealth;
                packet.writeByteSeq(guid[6]);
                if (pet.quality != 0)
                    packet << pet.quality;
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[3]);
                packet << pet.experience;
                packet.writeByteSeq(guid[7]);
                if (pet.flags != 0)
                    packet << pet.flags;
                packet.writeByteSeq(guid[5]);
                packet << pet.displayId << pet.level;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
