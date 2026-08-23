/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgMailTakeMoney : public ManagedPacket
    {
    public:
        WoWGuid gobjGuid;
        uint32_t messageId;

        // Not used for actual crediting (the server trusts its own stored mail value, not
        // the client-sent one) - kept only so the packet is consumed correctly off the wire.
        uint64_t money = 0;

        CmsgMailTakeMoney() : CmsgMailTakeMoney(0, 0)
        {
        }

        CmsgMailTakeMoney(uint64_t gobjGuid, uint32_t messageId) :
            ManagedPacket(CMSG_MAIL_TAKE_MONEY, 12),
            gobjGuid(gobjGuid),
            messageId(messageId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet >> messageId;
                packet >> money;

                gobjGuid[7] = packet.readBit();
                gobjGuid[6] = packet.readBit();
                gobjGuid[3] = packet.readBit();
                gobjGuid[2] = packet.readBit();
                gobjGuid[4] = packet.readBit();
                gobjGuid[5] = packet.readBit();
                gobjGuid[0] = packet.readBit();
                gobjGuid[1] = packet.readBit();

                packet.readByteSeq(gobjGuid[7]);
                packet.readByteSeq(gobjGuid[1]);
                packet.readByteSeq(gobjGuid[4]);
                packet.readByteSeq(gobjGuid[0]);
                packet.readByteSeq(gobjGuid[3]);
                packet.readByteSeq(gobjGuid[2]);
                packet.readByteSeq(gobjGuid[6]);
                packet.readByteSeq(gobjGuid[5]);

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> messageId;
                gobjGuid = WoWGuid(unpackedGuid);
                packet >> money;

                return true;
            }

            return false;
        }
    };
}
