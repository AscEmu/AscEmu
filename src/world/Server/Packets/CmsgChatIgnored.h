/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgChatIgnored : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t unk1;

        CmsgChatIgnored() : CmsgChatIgnored(0, 0)
        {
        }

        CmsgChatIgnored(uint64_t guid, uint8_t unk1) :
            ManagedPacket(CMSG_CHAT_IGNORED, 0),
            guid(guid),
            unk1(unk1)
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
            uint64_t unpacked_guid;
            packet >> unpacked_guid >> unk1;
            guid.Init(unpacked_guid);
#else
            ObjectGuid playerGuid;

            packet >> unk1;

            playerGuid[5] = packet.readBit();
            playerGuid[2] = packet.readBit();
            playerGuid[6] = packet.readBit();
            playerGuid[4] = packet.readBit();
            playerGuid[7] = packet.readBit();
            playerGuid[0] = packet.readBit();
            playerGuid[1] = packet.readBit();
            playerGuid[3] = packet.readBit();

            packet.ReadByteSeq(playerGuid[0]);
            packet.ReadByteSeq(playerGuid[6]);
            packet.ReadByteSeq(playerGuid[5]);
            packet.ReadByteSeq(playerGuid[1]);
            packet.ReadByteSeq(playerGuid[4]);
            packet.ReadByteSeq(playerGuid[3]);
            packet.ReadByteSeq(playerGuid[7]);
            packet.ReadByteSeq(playerGuid[2]);

            guid.Init(playerGuid);
#endif
            return true;
        }
    };
}
