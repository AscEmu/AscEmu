/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
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

#if VERSION_STRING < Cata
        uint64_t gobjGuid;
        uint32_t money{};
        uint32_t cod{};
#else
        ObjectGuid gobjGuid;
        uint64_t money{};
        uint64_t cod{};
#endif

        CmsgSendMail() : CmsgSendMail(0)
        {
        }

        CmsgSendMail(uint64_t gobjGuid) :
            ManagedPacket(CMSG_SEND_MAIL, 0),
            gobjGuid(gobjGuid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet >> gobjGuid;
            packet >> receiverName;
            packet >> subject;
            packet >> body;
            packet >> stationery;
            packet.read_skip<uint32_t>();
            packet >> itemCount;

            if (itemCount > 12)
                return false;

            for (uint8_t i = 0; i < itemCount; ++i)
            {
                packet >> itemSlot;
                packet >> itemGuid[i];
            }

            packet >> money;
            packet >> cod;

            return true;
#else
            packet.read_skip<uint32_t>();
            packet.read_skip<uint32_t>();

            packet >> cod;
            packet >> money;

            uint32_t bodyLength = packet.readBits(12);
            uint32_t subjectLength = packet.readBits(9);

            itemCount = static_cast<uint8_t>(packet.readBits(5));

            if (itemCount > 12)
                return false;

            gobjGuid[0] = packet.readBit();

            ObjectGuid itemGUIDs[12];

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

            packet.ReadByteSeq(gobjGuid[4]);

            for (uint8_t i = 0; i < itemCount; ++i)
            {
                packet.ReadByteSeq(itemGUIDs[i][6]);
                packet.ReadByteSeq(itemGUIDs[i][1]);
                packet.ReadByteSeq(itemGUIDs[i][7]);
                packet.ReadByteSeq(itemGUIDs[i][2]);
                packet >> itemSlot;
                packet.ReadByteSeq(itemGUIDs[i][3]);
                packet.ReadByteSeq(itemGUIDs[i][0]);
                packet.ReadByteSeq(itemGUIDs[i][4]);
                packet.ReadByteSeq(itemGUIDs[i][5]);
            }

            packet.ReadByteSeq(gobjGuid[7]);
            packet.ReadByteSeq(gobjGuid[3]);
            packet.ReadByteSeq(gobjGuid[6]);
            packet.ReadByteSeq(gobjGuid[5]);

            subject = packet.ReadString(subjectLength);
            receiverName = packet.ReadString(receiverLength);

            packet.ReadByteSeq(gobjGuid[2]);
            packet.ReadByteSeq(gobjGuid[0]);

            body = packet.ReadString(bodyLength);

            packet.ReadByteSeq(gobjGuid[1]);

            return true;
#endif
        }
    };
}}
