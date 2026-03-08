/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgTextEmote : public ManagedPacket
    {
    public:
        uint32_t nameLength;
        std::string name;
        uint32_t textEmote;
        uint64_t guid;
        uint32_t numEmote;
        WoWGuid targetGuid;

        SmsgTextEmote() : SmsgTextEmote(0, "", 0, 0, 0, WoWGuid())
        {
        }

        SmsgTextEmote(uint32_t nameLength, std::string name, uint32_t textEmote, uint64_t guid, uint32_t unk, WoWGuid target) :
            ManagedPacket(SMSG_TEXT_EMOTE, 28 + nameLength),
            nameLength(nameLength),
            name(name),
            textEmote(textEmote),
            guid(guid),
            numEmote(unk),
            targetGuid(target)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= Cata
            packet << guid << textEmote << numEmote << nameLength;
            if (nameLength > 1)
                packet.append(name.c_str(), nameLength);
            else
                packet << uint8_t(0);
#else // Mop
            WoWGuid playerGuid(guid);

            packet.writeBit(playerGuid[1]);

            packet.writeBit(targetGuid[7]);

            packet.writeBit(playerGuid[6]);

            packet.writeBit(targetGuid[5]);

            packet.writeBit(playerGuid[3]);

            packet.writeBit(targetGuid[6]);
            packet.writeBit(targetGuid[2]);

            packet.writeBit(playerGuid[7]);

            packet.writeBit(targetGuid[0]);
            packet.writeBit(targetGuid[1]);

            packet.writeBit(playerGuid[4]);
            packet.writeBit(playerGuid[2]);

            packet.writeBit(targetGuid[3]);
            packet.writeBit(targetGuid[4]);

            packet.writeBit(playerGuid[0]);
            packet.writeBit(playerGuid[5]);

            packet.WriteByteSeq(targetGuid[2]);
            packet.WriteByteSeq(targetGuid[1]);

            packet.WriteByteSeq(playerGuid[7]);
            packet.WriteByteSeq(playerGuid[4]);

            packet.WriteByteSeq(targetGuid[7]);

            packet.WriteByteSeq(playerGuid[5]);
            packet.WriteByteSeq(playerGuid[2]);

            packet << textEmote;

            packet.WriteByteSeq(playerGuid[6]);

            packet.WriteByteSeq(targetGuid[0]);

            packet.WriteByteSeq(playerGuid[3]);
            packet.WriteByteSeq(playerGuid[1]);

            packet.WriteByteSeq(targetGuid[6]);

            packet.WriteByteSeq(playerGuid[0]);

            packet.WriteByteSeq(targetGuid[3]);
            packet.WriteByteSeq(targetGuid[5]);
            packet.WriteByteSeq(targetGuid[4]);

            packet << numEmote;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
