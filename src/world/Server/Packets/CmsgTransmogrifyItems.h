/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/ItemDefines.hpp"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class CmsgTransmogrifyItems : public ManagedPacket
    {
    public:
        WoWGuid npcGuid;
        std::vector<WoWGuid> itemGuids;
        std::vector<uint32_t> newEntries;
        std::vector<uint32_t> slots;

        CmsgTransmogrifyItems() :
            ManagedPacket(CMSG_TRANSMOGRIFY_ITEMS, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                npcGuid[5] = packet.readBit();
                npcGuid[6] = packet.readBit();
                npcGuid[1] = packet.readBit();
                npcGuid[2] = packet.readBit();
                npcGuid[3] = packet.readBit();
                npcGuid[4] = packet.readBit();

                const uint32_t count = packet.readBits(21);

                if (count >= EQUIPMENT_SLOT_END)
                {
                    packet.rfinish();
                    return false;
                }

                itemGuids.assign(count, WoWGuid());
                newEntries.assign(count, 0);
                slots.assign(count, 0);

                std::vector<bool> hasItemBonus(count, false);
                std::vector<bool> hasModifications(count, false);

                for (uint32_t i = 0; i < count; ++i)
                {
                    hasItemBonus[i] = packet.readBit();
                    hasModifications[i] = packet.readBit();
                }

                npcGuid[0] = packet.readBit();
                npcGuid[7] = packet.readBit();

                for (uint32_t i = 0; i < count; ++i)
                {
                    if (hasModifications[i])
                    {
                        itemGuids[i][5] = packet.readBit();
                        itemGuids[i][6] = packet.readBit();
                        itemGuids[i][1] = packet.readBit();
                        itemGuids[i][3] = packet.readBit();
                        itemGuids[i][0] = packet.readBit();
                        itemGuids[i][4] = packet.readBit();
                        itemGuids[i][7] = packet.readBit();
                        itemGuids[i][2] = packet.readBit();
                    }

                    if (hasItemBonus[i])
                    {
                        itemGuids[i][4] = packet.readBit();
                        itemGuids[i][1] = packet.readBit();
                        itemGuids[i][0] = packet.readBit();
                        itemGuids[i][6] = packet.readBit();
                        itemGuids[i][5] = packet.readBit();
                        itemGuids[i][2] = packet.readBit();
                        itemGuids[i][7] = packet.readBit();
                        itemGuids[i][3] = packet.readBit();
                    }
                }

                for (uint32_t i = 0; i < count; ++i)
                {
                    packet >> slots[i];
                    packet >> newEntries[i];
                }

                packet.readByteSeq(npcGuid[5]);
                packet.readByteSeq(npcGuid[0]);
                packet.readByteSeq(npcGuid[1]);
                packet.readByteSeq(npcGuid[2]);
                packet.readByteSeq(npcGuid[3]);
                packet.readByteSeq(npcGuid[4]);
                packet.readByteSeq(npcGuid[6]);
                packet.readByteSeq(npcGuid[7]);

                for (uint32_t i = 0; i < count; ++i)
                {
                    if (hasModifications[i])
                    {
                        packet.readByteSeq(itemGuids[i][2]);
                        packet.readByteSeq(itemGuids[i][5]);
                        packet.readByteSeq(itemGuids[i][4]);
                        packet.readByteSeq(itemGuids[i][3]);
                        packet.readByteSeq(itemGuids[i][6]);
                        packet.readByteSeq(itemGuids[i][0]);
                        packet.readByteSeq(itemGuids[i][7]);
                        packet.readByteSeq(itemGuids[i][1]);
                    }

                    if (hasItemBonus[i])
                    {
                        packet.readByteSeq(itemGuids[i][7]);
                        packet.readByteSeq(itemGuids[i][1]);
                        packet.readByteSeq(itemGuids[i][6]);
                        packet.readByteSeq(itemGuids[i][5]);
                        packet.readByteSeq(itemGuids[i][4]);
                        packet.readByteSeq(itemGuids[i][3]);
                        packet.readByteSeq(itemGuids[i][0]);
                        packet.readByteSeq(itemGuids[i][2]);
                    }
                }

                return true;
            }
            else if (m_protocol.isCata())
            {
                const uint32_t count = packet.readBits(22);

                if (count >= EQUIPMENT_SLOT_END)
                {
                    packet.rfinish();
                    return false;
                }

                itemGuids.assign(count, WoWGuid());
                newEntries.assign(count, 0);
                slots.assign(count, 0);

                for (uint32_t i = 0; i < count; ++i)
                {
                    itemGuids[i][0] = packet.readBit();
                    itemGuids[i][5] = packet.readBit();
                    itemGuids[i][6] = packet.readBit();
                    itemGuids[i][2] = packet.readBit();
                    itemGuids[i][3] = packet.readBit();
                    itemGuids[i][7] = packet.readBit();
                    itemGuids[i][4] = packet.readBit();
                    itemGuids[i][1] = packet.readBit();
                }

                npcGuid[7] = packet.readBit();
                npcGuid[3] = packet.readBit();
                npcGuid[5] = packet.readBit();
                npcGuid[6] = packet.readBit();
                npcGuid[1] = packet.readBit();
                npcGuid[4] = packet.readBit();
                npcGuid[0] = packet.readBit();
                npcGuid[2] = packet.readBit();

                packet.flushBits();

                for (uint32_t i = 0; i < count; ++i)
                {
                    packet >> newEntries[i];

                    packet.readByteSeq(itemGuids[i][1]);
                    packet.readByteSeq(itemGuids[i][5]);
                    packet.readByteSeq(itemGuids[i][0]);
                    packet.readByteSeq(itemGuids[i][4]);
                    packet.readByteSeq(itemGuids[i][6]);
                    packet.readByteSeq(itemGuids[i][7]);
                    packet.readByteSeq(itemGuids[i][3]);
                    packet.readByteSeq(itemGuids[i][2]);

                    packet >> slots[i];
                }

                packet.readByteSeq(npcGuid[7]);
                packet.readByteSeq(npcGuid[2]);
                packet.readByteSeq(npcGuid[5]);
                packet.readByteSeq(npcGuid[4]);
                packet.readByteSeq(npcGuid[3]);
                packet.readByteSeq(npcGuid[1]);
                packet.readByteSeq(npcGuid[6]);
                packet.readByteSeq(npcGuid[0]);

                return true;
            }

            return false;
        }
    };
}
