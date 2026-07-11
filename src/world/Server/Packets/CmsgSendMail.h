/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSendMail : public ManagedPacket
    {
    public:
        std::string receiverName;
        std::string subject;
        std::string body;
        uint32_t stationery{};
        uint8_t itemCount{};

        uint8_t itemSlot{};
        uint64_t itemGuid[12]{};

        WoWGuid gobjGuid;
        uint64_t money{};
        uint64_t cod{};

        CmsgSendMail() : CmsgSendMail(0)
        {
        }

        CmsgSendMail(uint64_t gobjGuid) :
            ManagedPacket(CMSG_SEND_MAIL, 0),
            gobjGuid(gobjGuid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                uint64_t rawGobjGuid;
                uint32_t money32;
                uint32_t cod32;
                packet >> rawGobjGuid;
                gobjGuid.init(rawGobjGuid);
                packet >> receiverName;
                packet >> subject;
                packet >> body;
                packet >> stationery;
                packet.readSkip<uint32_t>();
                packet >> itemCount;

                if (itemCount > 12)
                    return false;

                for (uint8_t i = 0; i < itemCount; ++i)
                {
                    packet >> itemSlot;
                    packet >> itemGuid[i];
                }

                packet >> money32;
                packet >> cod32;
                money = money32;
                cod = cod32;

                return true;
            }
            else
            {
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();

                packet >> cod;
                packet >> money;

                uint32_t bodyLength = packet.readBits(12);
                uint32_t subjectLength = packet.readBits(9);

                itemCount = static_cast<uint8_t>(packet.readBits(5));

                if (itemCount > 12)
                    return false;

                gobjGuid[0] = packet.readBit();

                WoWGuid itemGUIDs[12];

                for (uint8_t i = 0; i < itemCount; ++i)
                {
                    itemGUIDs[i][2] = packet.readBit();
                    itemGUIDs[i][6] = packet.readBit();
                    itemGUIDs[i][3] = packet.readBit();
                    itemGUIDs[i][7] = packet.readBit();
                    itemGUIDs[i][1] = packet.readBit();
                    itemGUIDs[i][0] = packet.readBit();
                    itemGUIDs[i][4] = packet.readBit();
                    itemGUIDs[i][5] = packet.readBit();
                }

                gobjGuid[3] = packet.readBit();
                gobjGuid[4] = packet.readBit();
                uint32_t receiverLength = packet.readBits(7);
                gobjGuid[2] = packet.readBit();
                gobjGuid[6] = packet.readBit();
                gobjGuid[1] = packet.readBit();
                gobjGuid[7] = packet.readBit();
                gobjGuid[5] = packet.readBit();

                packet.readByteSeq(gobjGuid[4]);

                for (uint8_t i = 0; i < itemCount; ++i)
                {
                    packet.readByteSeq(itemGUIDs[i][6]);
                    packet.readByteSeq(itemGUIDs[i][1]);
                    packet.readByteSeq(itemGUIDs[i][7]);
                    packet.readByteSeq(itemGUIDs[i][2]);
                    packet >> itemSlot;
                    packet.readByteSeq(itemGUIDs[i][3]);
                    packet.readByteSeq(itemGUIDs[i][0]);
                    packet.readByteSeq(itemGUIDs[i][4]);
                    packet.readByteSeq(itemGUIDs[i][5]);
                }

                packet.readByteSeq(gobjGuid[7]);
                packet.readByteSeq(gobjGuid[3]);
                packet.readByteSeq(gobjGuid[6]);
                packet.readByteSeq(gobjGuid[5]);

                subject = packet.readString(subjectLength);
                receiverName = packet.readString(receiverLength);

                packet.readByteSeq(gobjGuid[2]);
                packet.readByteSeq(gobjGuid[0]);

                body = packet.readString(bodyLength);

                packet.readByteSeq(gobjGuid[1]);

                return true;
            }
        }
    };
}
