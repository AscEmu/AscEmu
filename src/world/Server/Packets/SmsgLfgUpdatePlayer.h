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
    class SmsgLfgUpdatePlayer : public ManagedPacket
    {
    public:
        uint8_t updateType;
        uint8_t extraInfo;
        uint8_t queued;
        LfgDungeonSet dungeons;
        std::string comment;

        SmsgLfgUpdatePlayer() : SmsgLfgUpdatePlayer(0, 0, 0, {}, "")
        {
        }

        SmsgLfgUpdatePlayer(uint8_t updateType, uint8_t extraInfo, uint8_t queued, LfgDungeonSet dungeons, std::string comment) :
            ManagedPacket(SMSG_LFG_UPDATE_PLAYER, 0),
            updateType(updateType),
            extraInfo(extraInfo),
            queued(queued),
            dungeons(std::move(dungeons)),
            comment(std::move(comment))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 1 + 1 + (extraInfo ? 1 : 0) * (1 + 1 + 1 + 1 + dungeons.size() * 4 + comment.length());
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion == WoW::Expansion::_WotLK)
            {
                packet << uint8_t(updateType);       // Lfg Update type
                packet << uint8_t(extraInfo);         // Extra info
                if (extraInfo)
                {
                    packet << uint8_t(queued);        // Join the queue
                    packet << uint8_t(0);             // unk - Always 0
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
