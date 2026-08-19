/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgLootRoll : public ManagedPacket
    {
    public:
        WoWGuid objectGuid;
        uint32_t slot;
        uint8_t choice;

        CmsgLootRoll() : CmsgLootRoll(0, 0, 0)
        {
        }

        CmsgLootRoll(uint64_t creatureGuid, uint32_t slot, uint8_t choice) :
            ManagedPacket(CMSG_LOOT_ROLL, 0),
            objectGuid(creatureGuid),
            slot(slot),
            choice(choice)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                uint8_t itemSlot = 0;
                packet >> itemSlot;
                slot = itemSlot;
                packet >> choice;

                objectGuid[7] = packet.readBit();
                objectGuid[1] = packet.readBit();
                objectGuid[2] = packet.readBit();
                objectGuid[0] = packet.readBit();
                objectGuid[6] = packet.readBit();
                objectGuid[3] = packet.readBit();
                objectGuid[4] = packet.readBit();
                objectGuid[5] = packet.readBit();

                packet.readByteSeq(objectGuid[0]);
                packet.readByteSeq(objectGuid[2]);
                packet.readByteSeq(objectGuid[7]);
                packet.readByteSeq(objectGuid[3]);
                packet.readByteSeq(objectGuid[1]);
                packet.readByteSeq(objectGuid[5]);
                packet.readByteSeq(objectGuid[4]);
                packet.readByteSeq(objectGuid[6]);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpackedCreatureGuid;

                packet >> unpackedCreatureGuid >> slot >> choice;

                objectGuid = WoWGuid(unpackedCreatureGuid);

                return true;
            }

            return false;
        }
    };
}
