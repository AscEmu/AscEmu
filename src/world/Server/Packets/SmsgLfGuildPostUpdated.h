/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <string>
#include <utility>

namespace AscEmu::Packets
{
    class SmsgLfGuildPostUpdated : public ManagedPacket
    {
    public:
        bool isGuildMaster;
        bool isListed;
        uint32_t level;
        std::string comment;
        uint32_t availability;
        uint32_t classRoles;
        uint32_t interests;

        SmsgLfGuildPostUpdated() : SmsgLfGuildPostUpdated(false, false, 0, "", 0, 0, 0)
        {
        }

        SmsgLfGuildPostUpdated(bool isGuildMaster, bool isListed, uint32_t level, std::string comment,
            uint32_t availability, uint32_t classRoles, uint32_t interests) :
            ManagedPacket(SMSG_LF_GUILD_POST_UPDATED, 1 + 4 + 4 + 4 + 4 + comment.size()),
            isGuildMaster(isGuildMaster), isListed(isListed), level(level), comment(std::move(comment)),
            availability(availability), classRoles(classRoles), interests(interests)
        {
        }

    protected:
        size_t expectedSize() const override { return 1 + 4 + 4 + 4 + 4 + comment.size(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.writeBit(isGuildMaster);

                if (isGuildMaster)
                {
                    packet.writeBits(comment.size(), 11);

                    packet.writeBit(isListed);

                    packet << uint32_t(level);

                    packet.writeString(comment);

                    packet << uint32_t(0);

                    packet << uint32_t(availability);
                    packet << uint32_t(classRoles);
                    packet << uint32_t(interests);
                }
                else
                {
                    packet.flushBits();
                }

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(isGuildMaster);

                if (isGuildMaster)
                {
                    packet.writeBits(comment.size(), 11);

                    packet.writeBit(isListed);

                    packet.writeString(comment);

                    packet << uint32_t(level);

                    packet << uint32_t(0);

                    packet << uint32_t(availability);
                    packet << uint32_t(classRoles);
                    packet << uint32_t(interests);
                }
                else
                {
                    packet.flushBits();
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
