/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/LFG/LFG.hpp"

namespace AscEmu::Packets
{
    class CmsgLfgJoin : public ManagedPacket
    {
    public:
        uint32_t roles = 0;
        LfgDungeonSet dungeons;
        std::string comment;

        CmsgLfgJoin() : ManagedPacket(CMSG_LFG_JOIN, 0)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.readSkip<uint8_t>();

                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();

                packet >> roles;

                const uint32_t numDungeons = packet.readBits(22);
                const uint32_t commentLength = packet.readBits(8);
                packet.readBit();

                if (numDungeons == 0)
                    return true;

                for (uint32_t i = 0; i < numDungeons; ++i)
                {
                    uint32_t dungeon;
                    packet >> dungeon;
                    dungeons.insert(dungeon & 0x00FFFFFF);
                }

                comment = packet.readString(commentLength);

                return true;
            }
            else if (m_protocol.isCata())
            {
                packet >> roles;

                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();

                const uint32_t commentLength = packet.readBits(9);
                const uint32_t numDungeons = packet.readBits(24);

                if (numDungeons == 0)
                    return true;

                comment = packet.readString(commentLength);

                for (uint32_t i = 0; i < numDungeons; ++i)
                {
                    uint32_t dungeon;
                    packet >> dungeon;
                    dungeons.insert(dungeon & 0x00FFFFFF);
                }

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet >> roles;
                packet.readSkip<uint16_t>();

                uint8_t numDungeons;
                packet >> numDungeons;

                if (numDungeons == 0)
                    return true;

                for (uint8_t i = 0; i < numDungeons; ++i)
                {
                    uint32_t dungeon;
                    packet >> dungeon;
                    dungeons.insert(dungeon & 0x00FFFFFF);
                }

                packet.readSkip<uint32_t>();

                packet >> comment;

                return true;
            }

            return false;
        }
    };
}
