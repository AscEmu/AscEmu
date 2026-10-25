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
    // battle slots that changed; 5.x takes one slot per packet
    class SmsgPetBattleSlotUpdates : public ManagedPacket
    {
    public:
        std::vector<BattlePetSlotData> slots;
        bool newSlot = false;                   // the client shows the unlocked slot
        bool autoSlotted = false;

        SmsgPetBattleSlotUpdates(std::vector<BattlePetSlotData> slots, bool newSlot, bool autoSlotted) :
            ManagedPacket(SMSG_PET_BATTLE_SLOT_UPDATES, 0),
            slots(std::move(slots)),
            newSlot(newSlot),
            autoSlotted(autoSlotted)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet << uint32_t(slots.size());

                // 9.x to 11.x send the bits first
                const bool bitsFirst = m_protocol.expansion >= WoW::Expansion::_Shadowlands && m_protocol.expansion <= WoW::Expansion::_TheWarWithin;
                if (bitsFirst)
                    writeBits(packet);

                for (const auto& slot : slots)
                    BattlePetWire::writeSlot(packet, slot);

                if (!bitsFirst)
                    writeBits(packet);
                return true;
            }

            if (m_protocol.isMop() && slots.size() == 1)
            {
                const BattlePetSlotData& slot = slots.front();
                const WoWGuid guid(static_cast<uint64_t>(slot.petId));

                packet.writeBits(1, 25);
                packet.writeBit(autoSlotted);
                packet.writeBit(false);
                packet.writeBit(true);
                packet.writeBit(false);
                packet.writeBit(false);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[6]);
                packet.writeBit(newSlot);
                packet.flushBits();

                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[7]);
                packet << slot.index;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }

    private:
        void writeBits(WorldPacket& packet) const
        {
            packet.writeBit(newSlot);
            packet.writeBit(autoSlotted);
            packet.flushBits();
        }
    };
}
