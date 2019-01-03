/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgGuildEvent : public ManagedPacket
    {
    public:
        uint8_t guildEvent;
        std::vector<std::string> text;
        uint8_t textCount = 0;
        uint64_t guid;

        SmsgGuildEvent() : SmsgGuildEvent(0, {}, 0)
        {
        }

        SmsgGuildEvent(uint8_t guildEvent, std::vector<std::string> text, uint64_t guid) :
            ManagedPacket(SMSG_GUILD_EVENT, 0),
            guildEvent(guildEvent),
            text(text),
            textCount(static_cast<uint8_t>(text.size())),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 1 + 1 + textCount + (guid ? 8 : 0);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guildEvent << textCount;

            for (const auto& stringVars : text)
                packet << stringVars.c_str();

            if (guid)
                packet << guid;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
