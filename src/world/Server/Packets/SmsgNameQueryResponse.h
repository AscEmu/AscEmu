/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgNameQueryResponse : public ManagedPacket
    {
    public:
        WoWGuid guid;
        std::string player_name;
        uint8_t race;
        uint8_t gender;
        uint8_t class_;

        SmsgNameQueryResponse() : SmsgNameQueryResponse(0, "", 0, 0, 0)
        {
        }

        SmsgNameQueryResponse(uint64_t guid, std::string playerName, uint8_t race, uint8_t gender, uint8_t class_) :
            ManagedPacket(SMSG_NAME_QUERY_RESPONSE, 0),
            guid(guid),
            player_name(playerName),
            race(race),
            gender(gender),
            class_(class_)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING >= WotLK
            packet << guid << uint8_t(0) << player_name << uint8_t(0) << race << gender << class_ << uint8_t(0);
#else
            packet << guid.getGuidLow() << uint32_t(0) << player_name << uint8_t(0) << uint32_t(race) << uint32_t(gender) << uint32_t(class_) <<
                    uint8_t(0);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
