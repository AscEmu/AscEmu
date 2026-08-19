/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WoWGuid.hpp"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class CmsgAutostoreLootItem : public ManagedPacket
    {
    public:
        uint8_t slot;

        CmsgAutostoreLootItem() : CmsgAutostoreLootItem(0)
        {
        }

        CmsgAutostoreLootItem(uint8_t slot) :
            ManagedPacket(CMSG_AUTOSTORE_LOOT_ITEM, 0),
            slot(slot)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // Mop supports AoE looting (multiple corpses at once), so the client sends a list of
                // {guid, slot} pairs. AscEmu's loot system only tracks a single active loot session
                // (via player->getLootGuid()), so the per-item guid isn't used - only the slot from
                // the first (and in practice, only) entry matters.
                const auto lootCount = packet.readBits(23);
                std::vector<WoWGuid> guids(lootCount);

                for (uint32_t i = 0; i < lootCount; ++i)
                {
                    guids[i][2] = packet.readBit();
                    guids[i][7] = packet.readBit();
                    guids[i][0] = packet.readBit();
                    guids[i][6] = packet.readBit();
                    guids[i][5] = packet.readBit();
                    guids[i][3] = packet.readBit();
                    guids[i][1] = packet.readBit();
                    guids[i][4] = packet.readBit();
                }

                for (uint32_t i = 0; i < lootCount; ++i)
                {
                    packet.readByteSeq(guids[i][0]);
                    packet.readByteSeq(guids[i][4]);
                    packet.readByteSeq(guids[i][1]);
                    packet.readByteSeq(guids[i][7]);
                    packet.readByteSeq(guids[i][6]);
                    packet.readByteSeq(guids[i][5]);
                    packet.readByteSeq(guids[i][3]);
                    packet.readByteSeq(guids[i][2]);

                    uint8_t itemSlot = 0;
                    packet >> itemSlot;

                    if (i == 0)
                        slot = itemSlot;
                }

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> slot;

                return true;
            }

            return false;
        }
    };
}
