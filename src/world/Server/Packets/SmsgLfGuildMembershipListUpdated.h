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
    struct SmsgLfGuildMembershipEntry
    {
        WoWGuid guildGuid;
        std::string name;
        std::string comment;
        uint32_t availability = 0;
        uint32_t classRoles = 0;
        uint32_t interests = 0;
        time_t submitTime = 0;
        time_t expiryTime = 0;
    };

    class SmsgLfGuildMembershipListUpdated : public ManagedPacket
    {
    public:
        std::vector<SmsgLfGuildMembershipEntry> applications;
        uint32_t remainingRequests;

        SmsgLfGuildMembershipListUpdated() : SmsgLfGuildMembershipListUpdated({}, 0)
        {
        }

        SmsgLfGuildMembershipListUpdated(std::vector<SmsgLfGuildMembershipEntry> applications, uint32_t remainingRequests) :
            ManagedPacket(SMSG_LF_GUILD_MEMBERSHIP_LIST_UPDATED, 7 + 54 * applications.size()),
            applications(std::move(applications)),
            remainingRequests(remainingRequests)
        {
        }

    protected:
        size_t expectedSize() const override { return 7 + 54 * applications.size(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.writeBits(applications.size(), 20);

                if (!applications.empty())
                {
                    ByteBuffer bufferData(54 * applications.size());
                    for (const auto& app : applications)
                    {
                        const WoWGuid& guildGuid = app.guildGuid;

                        packet.writeBit(guildGuid[1]);
                        packet.writeBit(guildGuid[0]);
                        packet.writeBit(guildGuid[5]);

                        packet.writeBits(app.comment.size(), 11);

                        packet.writeBit(guildGuid[3]);
                        packet.writeBit(guildGuid[7]);
                        packet.writeBit(guildGuid[4]);
                        packet.writeBit(guildGuid[6]);
                        packet.writeBit(guildGuid[2]);

                        packet.writeBits(app.name.size(), 8);

                        bufferData.writeByteSeq(guildGuid[2]);

                        bufferData.writeString(app.comment);

                        bufferData.writeByteSeq(guildGuid[5]);

                        bufferData.writeString(app.name);

                        bufferData << uint32_t(app.availability);
                        bufferData << uint32_t(app.expiryTime - time(nullptr));

                        bufferData.writeByteSeq(guildGuid[0]);
                        bufferData.writeByteSeq(guildGuid[6]);
                        bufferData.writeByteSeq(guildGuid[3]);
                        bufferData.writeByteSeq(guildGuid[7]);

                        bufferData << uint32_t(app.classRoles);

                        bufferData.writeByteSeq(guildGuid[4]);
                        bufferData.writeByteSeq(guildGuid[1]);

                        bufferData << uint32_t(time(nullptr) - app.submitTime);

                        bufferData << uint32_t(app.interests);
                    }

                    packet.flushBits();
                    packet.append(bufferData);
                }

                packet << uint32_t(remainingRequests);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBits(applications.size(), 20);

                if (!applications.empty())
                {
                    ByteBuffer bufferData(54 * applications.size());
                    for (const auto& app : applications)
                    {
                        const WoWGuid& guildGuid = app.guildGuid;

                        packet.writeBit(guildGuid[0]);
                        packet.writeBit(guildGuid[4]);
                        packet.writeBit(guildGuid[2]);
                        packet.writeBit(guildGuid[7]);

                        packet.writeBits(app.name.size(), 7);

                        // NOTE: guildGuid[2] mask bit twice here and never writes guildGuid[1]
                        packet.writeBit(guildGuid[2]);
                        packet.writeBit(guildGuid[3]);

                        packet.writeBits(app.comment.size(), 10);

                        packet.writeBit(guildGuid[6]);
                        packet.writeBit(guildGuid[5]);

                        bufferData << uint32_t(app.interests);
                        bufferData << uint32_t(0); // Unk

                        bufferData.writeString(app.name);

                        bufferData.writeByteSeq(guildGuid[4]);

                        bufferData << uint32_t(app.classRoles);

                        bufferData.writeByteSeq(guildGuid[6]);
                        bufferData.writeByteSeq(guildGuid[5]);

                        bufferData << uint32_t(time(nullptr) - app.submitTime);

                        bufferData.writeByteSeq(guildGuid[1]);
                        bufferData.writeByteSeq(guildGuid[3]);
                        bufferData.writeByteSeq(guildGuid[0]);
                        bufferData.writeByteSeq(guildGuid[7]);
                        bufferData.writeByteSeq(guildGuid[2]);

                        bufferData << uint32_t(app.expiryTime - time(nullptr));
                        bufferData << uint32_t(app.availability);

                        bufferData.writeString(app.comment);
                    }

                    packet.flushBits();
                    packet.append(bufferData);
                }

                packet << uint32_t(remainingRequests);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
