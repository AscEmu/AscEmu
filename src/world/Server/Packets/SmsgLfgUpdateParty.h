/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/LFG/LFG.hpp"
#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgLfgUpdateParty : public ManagedPacket
    {
    public:
        uint8_t updateType;
        uint8_t hasExtraInfo;
        uint8_t isJoining;
        uint8_t isQueued;
        LfgDungeonSet dungeons;
        std::string comment;

        SmsgLfgUpdateParty() : SmsgLfgUpdateParty(0, 0, 0, 0, {}, "")
        {
        }

        SmsgLfgUpdateParty(uint8_t updateType, uint8_t hasExtraInfo, uint8_t isJoining, uint8_t isQueued,
            LfgDungeonSet dungeons, std::string comment) :
            ManagedPacket(SMSG_LFG_UPDATE_PARTY, 0),
            updateType(updateType),
            hasExtraInfo(hasExtraInfo),
            isJoining(isJoining),
            isQueued(isQueued),
            dungeons(std::move(dungeons)),
            comment(std::move(comment))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 1 + 1 + (hasExtraInfo ? 1 : 0) * (1 + 1 + 1 + 1 + 1 + dungeons.size() * 4 + comment.length());
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion == WoW::Expansion::_WotLK)
            {
                packet << uint8_t(updateType);          // Lfg Update type
                packet << uint8_t(hasExtraInfo);          // Extra info
                if (hasExtraInfo)
                {
                    packet << uint8_t(isJoining);         // LFG Join
                    packet << uint8_t(isQueued);          // Join the queue
                    packet << uint8_t(0);                 // unk - Always 0
                    packet << uint8_t(0);                 // unk - Always 0
                    for (uint8_t i = 0; i < 3; ++i)
                        packet << uint8_t(0);             // unk - Always 0

                    packet << uint8_t(dungeons.size());

                    if (!dungeons.empty())
                    {
                        for (auto dungeonEntry : dungeons)
                            packet << uint32_t(dungeonEntry);
                    }

                    packet << comment;
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
