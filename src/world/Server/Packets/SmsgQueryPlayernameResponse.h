/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include "AEVersion.hpp"
#include "Server/Opcodes.hpp"
#include "WorldPacket.h"
#include "WoWGuid.hpp"

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgQueryPlayerNameResponse : public ManagedPacket
    {
    public:
        static constexpr uint8_t MAX_DECLINED_NAME_CASES = 5;

        std::array<std::string, MAX_DECLINED_NAME_CASES> declinedNames;
        std::string player_name;
        WoWGuid guid;

        uint32_t realmId = 0;
        uint32_t accountId = 0;

        uint8_t race = 0;
        uint8_t gender = 0;
        uint8_t class_ = 0;
        uint8_t level = 0;
        
        bool hasData = false;
        bool hasDeclinedNames = false;

        SmsgQueryPlayerNameResponse() : ManagedPacket(SMSG_QUERY_PLAYER_NAME_RESPONSE, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING >= WotLK && VERSION_STRING < Mop
            packet << guid << uint8_t(0) << player_name << uint8_t(0) << race << gender << class_ << uint8_t(0);

#elif VERSION_STRING == Mop
            WoWGuid guid2 = 0;

            packet.writeBit(guid[3]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[1]);

            packet.flushBits();

            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[6]);
            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[2]);

            packet << static_cast<uint8_t>(hasData ? 0 : 1);

            if (hasData)
            {
                packet << realmId;
                packet << accountId;
                packet << class_;
                packet << race;
                packet << level;
                packet << gender;
            }

            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[3]);

            if (!hasData)
                return true;

            packet.writeBit(guid2[2]);
            packet.writeBit(guid2[7]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[0]);
            packet.writeBit(0); // isDeleted Flag
            packet.writeBit(guid2[4]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid2[1]);
            packet.writeBit(guid2[3]);
            packet.writeBit(guid2[0]);

            for (uint8_t i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            {
                size_t len = hasDeclinedNames ? declinedNames[i].size() : 0;
                packet.writeBits(len, 7);
            }

            packet.writeBit(guid[6]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid2[5]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[4]);

            packet.writeBits(player_name.size(), 6);

            packet.writeBit(guid2[6]);

            packet.flushBits();

            packet.WriteByteSeq(guid[6]);
            packet.WriteByteSeq(guid[0]);

            packet.WriteString(player_name);

            packet.WriteByteSeq(guid2[5]);
            packet.WriteByteSeq(guid2[2]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid2[4]);
            packet.WriteByteSeq(guid2[3]);
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid2[7]);

            if (hasDeclinedNames)
            {
                for (uint8_t i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
                    packet.WriteString(declinedNames[i]);
            }

            packet.WriteByteSeq(guid2[6]);
            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid2[1]);
            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid2[0]);

#else
            // Classic/TBC
            packet << guid.getGuidLow() << uint32_t(0) << player_name << uint8_t(0) << uint32_t(race) << uint32_t(gender) << uint32_t(class_) << uint8_t(0);
#endif
            
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
