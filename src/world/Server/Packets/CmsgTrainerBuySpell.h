/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgTrainerBuySpell : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t trainerId = 0;
        uint32_t spellId;

        CmsgTrainerBuySpell() : CmsgTrainerBuySpell(0, 0)
        {
        }

        CmsgTrainerBuySpell(uint64_t guid, uint32_t spellId) :
            ManagedPacket(CMSG_TRAINER_BUY_SPELL, 12),
            guid(guid),
            spellId(spellId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet >> spellId;
                packet >> trainerId;

                WoWGuid unpackedGuid;
                unpackedGuid[1] = packet.readBit();
                unpackedGuid[4] = packet.readBit();
                unpackedGuid[0] = packet.readBit();
                unpackedGuid[6] = packet.readBit();
                unpackedGuid[3] = packet.readBit();
                unpackedGuid[2] = packet.readBit();
                unpackedGuid[5] = packet.readBit();
                unpackedGuid[7] = packet.readBit();

                packet.readByteSeq(unpackedGuid[3]);
                packet.readByteSeq(unpackedGuid[1]);
                packet.readByteSeq(unpackedGuid[4]);
                packet.readByteSeq(unpackedGuid[7]);
                packet.readByteSeq(unpackedGuid[0]);
                packet.readByteSeq(unpackedGuid[5]);
                packet.readByteSeq(unpackedGuid[6]);
                packet.readByteSeq(unpackedGuid[2]);

                guid = unpackedGuid;
                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;

                if (m_protocol.expansion > WoW::Expansion::_WotLK)
                {
                    packet >> trainerId;
                }

                packet >> spellId;
                guid.init(unpackedGuid);
                return true;
            }

            return false;
        }
    };
}
