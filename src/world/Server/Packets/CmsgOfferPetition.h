/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgOfferPetition : public ManagedPacket
    {
    public:
        WoWGuid itemGuid;
        WoWGuid playerGuid;

        CmsgOfferPetition() : CmsgOfferPetition(0, 0)
        {
        }

        CmsgOfferPetition(uint64_t itemGuid, uint64_t playerGuid) :
            ManagedPacket(CMSG_OFFER_PETITION, 20),
            itemGuid(itemGuid),
            playerGuid(playerGuid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedItemGuid;
                uint64_t unpackedGuid;
                packet.readSkip<uint32_t>();
                packet >> unpackedItemGuid >> unpackedGuid;
                itemGuid.init(unpackedItemGuid);
                playerGuid.init(unpackedGuid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.readSkip<uint32_t>();

                playerGuid[4] = packet.readBit();
                playerGuid[1] = packet.readBit();
                itemGuid[2] = packet.readBit();
                playerGuid[6] = packet.readBit();
                itemGuid[1] = packet.readBit();
                playerGuid[2] = packet.readBit();
                itemGuid[4] = packet.readBit();
                playerGuid[3] = packet.readBit();
                playerGuid[7] = packet.readBit();
                itemGuid[0] = packet.readBit();
                itemGuid[6] = packet.readBit();
                playerGuid[5] = packet.readBit();
                playerGuid[0] = packet.readBit();
                itemGuid[3] = packet.readBit();
                itemGuid[5] = packet.readBit();
                itemGuid[7] = packet.readBit();

                packet.readByteSeq(playerGuid[7]);
                packet.readByteSeq(itemGuid[1]);
                packet.readByteSeq(itemGuid[4]);
                packet.readByteSeq(itemGuid[2]);
                packet.readByteSeq(playerGuid[6]);
                packet.readByteSeq(itemGuid[3]);
                packet.readByteSeq(itemGuid[0]);
                packet.readByteSeq(itemGuid[5]);
                packet.readByteSeq(playerGuid[0]);
                packet.readByteSeq(playerGuid[2]);
                packet.readByteSeq(playerGuid[5]);
                packet.readByteSeq(playerGuid[3]);
                packet.readByteSeq(playerGuid[4]);
                packet.readByteSeq(itemGuid[7]);
                packet.readByteSeq(playerGuid[1]);
                packet.readByteSeq(itemGuid[6]);
                return true;
            }

            return false;
        }
    };
}
