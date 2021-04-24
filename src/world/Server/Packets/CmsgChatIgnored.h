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

            packet >> unk1;

            guid[5] = packet.readBit();
            guid[2] = packet.readBit();
            guid[6] = packet.readBit();
            guid[4] = packet.readBit();
            guid[7] = packet.readBit();
            guid[0] = packet.readBit();
            guid[1] = packet.readBit();
            guid[3] = packet.readBit();

            packet.ReadByteSeq(guid[0]);
            packet.ReadByteSeq(guid[6]);
            packet.ReadByteSeq(guid[5]);
            packet.ReadByteSeq(guid[1]);
            packet.ReadByteSeq(guid[4]);
            packet.ReadByteSeq(guid[3]);
            packet.ReadByteSeq(guid[7]);
            packet.ReadByteSeq(guid[2]);

#endif
            return true;
        }
    };
}
