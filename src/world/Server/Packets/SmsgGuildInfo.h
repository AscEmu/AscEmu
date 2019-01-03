/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgGuildInfo : public ManagedPacket
    {
    public:
        std::string guildName;
        time_t createDate;
        uint32_t membersCount;
        uint32_t accountsCount;

        SmsgGuildInfo() : SmsgGuildInfo("", 0, 0, 0)
        {
        }

        SmsgGuildInfo(std::string guildName, time_t createDate, uint32_t membersCount, uint32_t accountsCount) :
            ManagedPacket(SMSG_GUILD_INFO, 0),
            guildName(guildName),
            createDate(createDate),
            membersCount(membersCount),
            accountsCount(accountsCount)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return guildName.size() + 4 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guildName;
            packet.appendPackedTime(createDate);
            packet << membersCount << accountsCount;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
