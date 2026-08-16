/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    struct SmsgLfGuildRecruitEntry
    {
        WoWGuid playerGuid;
        std::string name;
        std::string comment;
        int32_t level = 0;
        int32_t availability = 0;
        int32_t classRoles = 0;
        int32_t interests = 0;
        int32_t playerClass = 0;
        time_t submitTime = 0;
        time_t expiryTime = 0;
    };

    class SmsgLfGuildRecruitListUpdated : public ManagedPacket
    {
    public:
        std::vector<SmsgLfGuildRecruitEntry> recruits;

        SmsgLfGuildRecruitListUpdated() : SmsgLfGuildRecruitListUpdated(std::vector<SmsgLfGuildRecruitEntry>{})
        {
        }

        SmsgLfGuildRecruitListUpdated(std::vector<SmsgLfGuildRecruitEntry> recruits) :
            ManagedPacket(SMSG_LF_GUILD_RECRUIT_LIST_UPDATED, 7 + 26 * recruits.size() + 53 * recruits.size()),
            recruits(std::move(recruits))
        {
        }

    protected:
        size_t expectedSize() const override { return 7 + 26 * recruits.size() + 53 * recruits.size(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata() || m_protocol.isMop())
            {
                ByteBuffer dataBuffer(53 * recruits.size());
                packet.writeBits(recruits.size(), 20);

                for (const auto& recruit : recruits)
                {
                    const WoWGuid& playerGuid = recruit.playerGuid;

                    packet.writeBits(recruit.comment.size(), 11);

                    packet.writeBit(playerGuid[2]);
                    packet.writeBit(playerGuid[4]);
                    packet.writeBit(playerGuid[3]);
                    packet.writeBit(playerGuid[7]);
                    packet.writeBit(playerGuid[0]);

                    packet.writeBits(recruit.name.size(), 7);

                    packet.writeBit(playerGuid[5]);
                    packet.writeBit(playerGuid[1]);
                    packet.writeBit(playerGuid[6]);

                    dataBuffer.writeByteSeq(playerGuid[4]);

                    dataBuffer << int32_t(time(nullptr) <= recruit.expiryTime);

                    dataBuffer.writeByteSeq(playerGuid[3]);
                    dataBuffer.writeByteSeq(playerGuid[0]);
                    dataBuffer.writeByteSeq(playerGuid[1]);

                    dataBuffer << int32_t(recruit.level);

                    dataBuffer.writeByteSeq(playerGuid[6]);
                    dataBuffer.writeByteSeq(playerGuid[7]);
                    dataBuffer.writeByteSeq(playerGuid[2]);

                    dataBuffer << int32_t(time(nullptr) - recruit.submitTime);
                    dataBuffer << int32_t(recruit.availability);
                    dataBuffer << int32_t(recruit.classRoles);
                    dataBuffer << int32_t(recruit.interests);
                    dataBuffer << int32_t(recruit.expiryTime - time(nullptr));

                    dataBuffer.writeString(recruit.name);
                    dataBuffer.writeString(recruit.comment);

                    dataBuffer << int32_t(recruit.playerClass);

                    dataBuffer.writeByteSeq(playerGuid[5]);
                }

                packet.flushBits();
                packet.append(dataBuffer);
                packet << uint32_t(time(nullptr));

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
