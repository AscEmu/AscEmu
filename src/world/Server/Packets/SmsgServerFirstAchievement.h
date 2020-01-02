/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgServerFirstAchievement : public ManagedPacket
    {
    public:
        std::string playerName;
        uint64_t guid;
        uint32_t achivementId;
        uint32_t nameClickable;

        SmsgServerFirstAchievement() : SmsgServerFirstAchievement("", 0, 0, 0)
        {
        }

        SmsgServerFirstAchievement(std::string playerName, uint64_t guid, uint32_t achivementId, uint32_t nameClickable) :
            ManagedPacket(SMSG_SERVER_FIRST_ACHIEVEMENT, 0),
            playerName(std::move(playerName)), guid(guid), achivementId(achivementId), nameClickable(nameClickable)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return playerName.size() + 1 + 8 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << playerName << guid << achivementId << nameClickable;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
