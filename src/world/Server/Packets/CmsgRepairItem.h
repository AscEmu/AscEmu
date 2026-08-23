/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgRepairItem : public ManagedPacket
    {
    public:

        WoWGuid creatureGuid;
        WoWGuid itemGuid;
        bool isInGuild;

        CmsgRepairItem() : CmsgRepairItem(0, 0, false)
        {
        }

        CmsgRepairItem(uint64_t creatureGuid, uint64_t itemGuid, bool isInGuild) :
            ManagedPacket(CMSG_REPAIR_ITEM, 17),
            creatureGuid(creatureGuid),
            itemGuid(itemGuid),
            isInGuild(isInGuild)
        {
        }

    protected:

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                uint64_t unpackedItemGuid;
                packet >> unpackedGuid >> unpackedItemGuid >> isInGuild;
                creatureGuid.init(unpackedGuid);
                itemGuid.init(unpackedItemGuid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                itemGuid[2] = packet.readBit();
                itemGuid[5] = packet.readBit();
                creatureGuid[3] = packet.readBit();
                isInGuild = packet.readBit();
                creatureGuid[7] = packet.readBit();
                itemGuid[4] = packet.readBit();
                creatureGuid[2] = packet.readBit();
                itemGuid[0] = packet.readBit();
                itemGuid[3] = packet.readBit();
                creatureGuid[6] = packet.readBit();
                creatureGuid[1] = packet.readBit();
                creatureGuid[4] = packet.readBit();
                itemGuid[6] = packet.readBit();
                creatureGuid[5] = packet.readBit();
                creatureGuid[0] = packet.readBit();
                itemGuid[7] = packet.readBit();
                itemGuid[1] = packet.readBit();

                packet.readByteSeq(itemGuid[2]);
                packet.readByteSeq(creatureGuid[1]);
                packet.readByteSeq(itemGuid[1]);
                packet.readByteSeq(creatureGuid[4]);
                packet.readByteSeq(creatureGuid[7]);
                packet.readByteSeq(creatureGuid[3]);
                packet.readByteSeq(creatureGuid[2]);
                packet.readByteSeq(itemGuid[7]);
                packet.readByteSeq(creatureGuid[5]);
                packet.readByteSeq(creatureGuid[0]);
                packet.readByteSeq(itemGuid[5]);
                packet.readByteSeq(itemGuid[3]);
                packet.readByteSeq(itemGuid[4]);
                packet.readByteSeq(itemGuid[6]);
                packet.readByteSeq(creatureGuid[6]);
                packet.readByteSeq(itemGuid[0]);
                return true;
            }

            return false;
        }
    };
}
