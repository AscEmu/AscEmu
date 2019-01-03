/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"
#include "Units/Creatures/Creature.h"

namespace AscEmu { namespace Packets
{
    class SmsgCreatureQueryResponse : public ManagedPacket
    {
    public:
        CreatureProperties info;
        uint32_t entry;
        const char* name;
        const char* subName;

        SmsgCreatureQueryResponse() : SmsgCreatureQueryResponse(info, 0, "", "")
        {
        }

        SmsgCreatureQueryResponse(CreatureProperties info, uint32_t entry, const char* name, const char* subName) :
            ManagedPacket(SMSG_CREATURE_QUERY_RESPONSE, 250),
            info(info),
            entry(entry),
            name(name),
            subName(subName)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (entry == 300000)
            {
                packet << entry << "WayPoint" << uint8_t(0) << uint8_t(0) << uint8_t(0) << "Level is WayPoint ID";
                for (uint8_t i = 0; i < 8; ++i)
                    packet << uint32_t(0);

                packet << uint8_t(0);
            }
            else
            {
                packet << entry << name;

#if VERSION_STRING <= WotLK
                for (uint8_t i = 0; i < 3; ++i)
                    packet << uint8_t(0);
#else
                for (uint8_t i = 0; i < 7; ++i)
                    packet << uint8_t(0);
#endif
                packet << subName;
                packet << info.info_str << info.typeFlags;
#if VERSION_STRING > WotLK
                packet << uint32_t(0);
#endif
                packet << info.Type << info.Family << info.Rank << info.killcredit[0]
                    << info.killcredit[1] << info.Male_DisplayID << info.Female_DisplayID << info.Male_DisplayID2
                    << info.Female_DisplayID2 << info.baseAttackMod << info.rangeAttackMod << info.Leader;

#if VERSION_STRING >= WotLK
                for (uint8_t i = 0; i < 6; ++i)
                    packet << uint32_t(info.QuestItems[i]);

                packet << info.waypointid;
#endif

#if VERSION_STRING > WotLK
                packet << uint32_t(0);
#endif
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
