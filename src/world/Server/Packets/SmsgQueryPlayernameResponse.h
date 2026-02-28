/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgQueryPlayernameResponse : public ManagedPacket
    {
    public:
        WoWGuid guid;
        std::string player_name;
        uint8_t race;
        uint8_t gender;
        uint8_t class_;
        uint8_t level;
        std::vector<std::string> declinedNames;

        SmsgQueryPlayernameResponse() : SmsgQueryPlayernameResponse(0, "", 0, 0, 0, 0)
        {
        }

        SmsgQueryPlayernameResponse(uint64_t guid, std::string playerName, uint8_t race, uint8_t gender, uint8_t class_, uint8_t level = 0, std::vector<std::string> declinedNames = {}) :
            ManagedPacket(SMSG_QUERY_PLAYER_NAME_RESPONSE, 0),
            guid(guid),
            player_name(playerName),
            race(race),
            gender(gender),
            class_(class_),
            level(level),
            declinedNames(declinedNames)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING >= WotLK && VERSION_STRING < Mop
            packet << guid << uint8_t(0) << player_name << uint8_t(0) << race << gender << class_ << uint8_t(0);
#elif VERSION_STRING == Mop
            // MoP Implementation
            bool hasData = true; // We always have data if we're responding
            packet.writeBit(hasData);
            if (hasData)
            {
                 bool hasDeclined = !declinedNames.empty();
                 packet.writeBit(hasDeclined);
                 packet.writeBits(player_name.length(), 6);
                 packet.writeBits(0, 6); // Realm name length (0 for now)
                 if (hasDeclined)
                 {
                     for(const auto& name : declinedNames)
                         packet.writeBits(name.length(), 7);
                 }
                 
                 // GUID Packing for MoP SMSG_QUERY_PLAYER_NAME_RESPONSE (Based on 5.4.8 reference)
                 // Common packing: 1, 4, 7, 3, 2, 6, 5, 0 (Bits)
                 packet.writeBit(guid[1]);
                 packet.writeBit(guid[4]);
                 packet.writeBit(guid[7]);
                 packet.writeBit(guid[3]);
                 packet.writeBit(guid[2]);
                 packet.writeBit(guid[6]);
                 packet.writeBit(guid[5]);
                 packet.writeBit(guid[0]);
                 
                 packet.flushBits();

                 // GUID bytes first (typical MoP order after bits)
                 packet.WriteByteSeq(guid[5]);
                 packet.WriteByteSeq(guid[1]);
                 packet.WriteByteSeq(guid[0]);
                 packet.WriteByteSeq(guid[6]);
                 packet.WriteByteSeq(guid[2]);
                 packet.WriteByteSeq(guid[4]);
                 packet.WriteByteSeq(guid[7]);
                 packet.WriteByteSeq(guid[3]);

                 packet << uint8_t(race);
                 packet << uint8_t(gender);
                 packet << uint8_t(class_);
                 packet << uint8_t(level);

                 packet.WriteString(player_name);
                 packet.WriteString(""); // Realm name (empty)
                 if (hasDeclined)
                 {
                      for(const auto& name : declinedNames)
                          packet.WriteString(name);
                 }
            }
            else
            {
                 // Write GUID only?
                 // Usually only if player not found, but we check !info before sending.
            }
#else
            // Classic/TBC
            packet << guid.getGuidLow() << uint32_t(0) << player_name << uint8_t(0) << uint32_t(race) << uint32_t(gender) << uint32_t(class_) <<
                    uint8_t(0);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
