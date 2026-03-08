/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgTextEmote : public ManagedPacket
    {
    public:
        uint32_t text_emote;
        uint32_t numEmote;
        uint64_t guid;

        CmsgTextEmote() : CmsgTextEmote(0, 0, 0)
        {
        }

        CmsgTextEmote(uint32_t text_emote, uint32_t unk, uint64_t guid) :
            ManagedPacket(CMSG_TEXT_EMOTE, 16),
            text_emote(text_emote),
            numEmote(unk),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << text_emote << numEmote << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= Cata
            packet >> text_emote >> numEmote >> guid;
#else // Mop
            WoWGuid packedGuid;

            packet >> text_emote;
            packet >> numEmote;

            packedGuid[6] = packet.readBit();
            packedGuid[7] = packet.readBit();
            packedGuid[3] = packet.readBit();
            packedGuid[2] = packet.readBit();
            packedGuid[0] = packet.readBit();
            packedGuid[5] = packet.readBit();
            packedGuid[1] = packet.readBit();
            packedGuid[4] = packet.readBit();

            packet.ReadByteSeq(packedGuid[0]);
            packet.ReadByteSeq(packedGuid[5]);
            packet.ReadByteSeq(packedGuid[1]);
            packet.ReadByteSeq(packedGuid[4]);
            packet.ReadByteSeq(packedGuid[2]);
            packet.ReadByteSeq(packedGuid[3]);
            packet.ReadByteSeq(packedGuid[7]);
            packet.ReadByteSeq(packedGuid[6]);

            guid = packedGuid.getRawGuid();
#endif
            return true;
        }
    };
}
