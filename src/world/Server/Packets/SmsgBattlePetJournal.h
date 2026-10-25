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
    // the whole journal: the battle slots and every pet of the account
    class SmsgBattlePetJournal : public ManagedPacket
    {
    public:
        std::vector<BattlePetSlotData> slots;
        std::vector<BattlePetData> pets;
        bool hasJournalLock = true;
        uint16_t trapLevel = 0;
        uint32_t maxPets = 1000;

        SmsgBattlePetJournal() : SmsgBattlePetJournal({}, {}, true)
        {
        }

        SmsgBattlePetJournal(std::vector<BattlePetSlotData> slots, std::vector<BattlePetData> pets, bool hasJournalLock) :
            ManagedPacket(SMSG_BATTLE_PET_JOURNAL, 0),
            slots(std::move(slots)),
            pets(std::move(pets)),
            hasJournalLock(hasJournalLock)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet << trapLevel << uint32_t(slots.size()) << uint32_t(pets.size());

                // 7.x and 8.x name the size of the journal
                if (m_protocol.isLegion() || m_protocol.isBfA())
                    packet << maxPets;

                const bool lockFirst = BattlePetWire::bitsBeforeLists(m_protocol);
                if (lockFirst)
                {
                    packet.writeBit(hasJournalLock);
                    packet.flushBits();
                }

                for (const auto& slot : slots)
                    BattlePetWire::writeSlot(packet, slot);
                for (const auto& pet : pets)
                    BattlePetWire::writePet(packet, pet);

                if (!lockFirst)
                {
                    packet.writeBit(hasJournalLock);
                    packet.flushBits();
                }
                return true;
            }

            if (m_protocol.isMop())
                return writeMop(packet);

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }

    private:
        // 5.x: the bits of every pet, the bits of the slots, then the slot data and the pet data
        bool writeMop(WorldPacket& packet) const
        {
            ByteBuffer petData;
            ByteBuffer slotData;

            packet.writeBits(static_cast<uint32_t>(pets.size()), 19);
            for (const auto& pet : pets)
            {
                const WoWGuid guid(static_cast<uint64_t>(pet.id));
                packet.writeBit(pet.flags == 0);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[7]);
                packet.writeBits(static_cast<uint32_t>(pet.name.size()), 7);
                packet.writeBit(false);                 // bound to one character
                packet.writeBit(guid[0]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[6]);
                packet.writeBit(false);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[5]);
                packet.writeBit(pet.breed == 0);
                packet.writeBit(guid[4]);
                packet.writeBit(pet.quality == 0);

                if (pet.quality != 0)
                    petData << pet.quality;
                petData << pet.power;
                petData.writeByteSeq(guid[7]);
                petData << pet.level << pet.health;
                if (pet.breed != 0)
                    petData << pet.breed;
                petData << pet.species;
                petData.writeByteSeq(guid[2]);
                if (pet.flags != 0)
                    petData << pet.flags;
                petData << pet.creatureId << pet.displayId << pet.speed;
                petData.writeString(pet.name);
                petData.writeByteSeq(guid[6]);
                petData.writeByteSeq(guid[5]);
                petData << pet.maxHealth;
                petData.writeByteSeq(guid[4]);
                petData << pet.experience;
                petData.writeByteSeq(guid[0]);
                petData.writeByteSeq(guid[1]);
                petData.writeByteSeq(guid[3]);
            }

            packet.writeBit(true);                      // battle slots
            packet.writeBits(static_cast<uint32_t>(slots.size()), 25);
            for (const auto& slot : slots)
            {
                const WoWGuid guid(static_cast<uint64_t>(slot.petId));
                packet.writeBit(slot.locked);
                packet.writeBit(true);
                packet.writeBit(false);
                packet.writeBit(false);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[3]);

                slotData.writeByteSeq(guid[5]);
                slotData.writeByteSeq(guid[1]);
                slotData.writeByteSeq(guid[7]);
                slotData.writeByteSeq(guid[2]);
                slotData.writeByteSeq(guid[3]);
                slotData.writeByteSeq(guid[0]);
                slotData.writeByteSeq(guid[4]);
                slotData.writeByteSeq(guid[6]);
                slotData << slot.index;
            }

            packet.flushBits();
            packet.append(slotData);
            packet.append(petData);
            packet << trapLevel;
            return true;
        }
    };
}
