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
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 6.x and later (CMSG_LOOT_ITEM): a list of {looted object, loot list id}; the server tracks one loot
                // session, so only the first entry counts. The loot list id is the slot index plus one, 0 is never sent.
                uint32_t count = 0;
                packet >> count;
                if (count == 0 || count > 100)
                    return false;

                for (uint32_t i = 0; i < count; ++i)
                {
                    // the guid comes packed (two mask bytes plus the non-zero bytes), so the entry length varies
                    if (packet.remaining() < 3)
                        return false;

                    WoWGuid128 object;
                    packet >> object;
                    uint8_t lootListId = 0;
                    packet >> lootListId;
                    if (i == 0)
                        slot = lootListId > 0 ? lootListId - 1 : 0;
                }
                // 10.x and later append a soft interact bit, not needed
                return true;
            }

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
