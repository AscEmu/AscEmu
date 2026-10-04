/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgCreatureQueryResponse : public ManagedPacket
    {
    public:
        CreatureProperties const* info = nullptr;
        uint32_t entry;
        const char* name;
        const char* subName;

        SmsgCreatureQueryResponse() : SmsgCreatureQueryResponse(nullptr, 0, "", "")
        {
        }

        SmsgCreatureQueryResponse(CreatureProperties const* info, uint32_t entry, const char* name, const char* subName) :
            ManagedPacket(SMSG_CREATURE_QUERY_RESPONSE, 250),
            info(info),
            entry(entry),
            name(name),
            subName(subName)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool serialiseLegion(WorldPacket& packet)
        {
            // entry, allow, then the string lengths (with terminator), names, flags, type, family, rank, kill
            // credits, displays, modifiers, quest items and the strings
            std::string creatureName = name;
            std::string title = subName;

            if (entry == 300000)
            {
                creatureName = "WayPoint";
                title = "Level is WayPoint ID";
            }

            const bool allow = info != nullptr || entry == 300000;

            packet << entry;
            packet.writeBit(allow);
            packet.flushBits();

            if (!allow)
                return true;

            const std::string cursorName = info != nullptr ? info->icon_name : std::string();

            std::vector<uint32_t> questItems;
            if (info != nullptr)
            {
                for (uint8_t i = 0; i < 6; ++i)
                {
                    if (info->QuestItems[i] != 0)
                        questItems.push_back(info->QuestItems[i]);
                }
            }

            packet.writeBits(static_cast<uint32_t>(title.length() + 1), 11);
            packet.writeBits(1, 11);                                    // alternative title
            packet.writeBits(static_cast<uint32_t>(cursorName.length() + 1), 6);
            packet.writeBit(info != nullptr && info->Leader);

            // four names with an alternative each, only the first one is used
            packet.writeBits(static_cast<uint32_t>(creatureName.length() + 1), 11);
            packet.writeBits(1, 11);
            for (uint8_t i = 1; i < 4; ++i)
            {
                packet.writeBits(1, 11);
                packet.writeBits(1, 11);
            }
            packet.flushBits();

            if (!creatureName.empty())
                packet << creatureName;

            packet << uint32_t(info != nullptr ? info->typeFlags : 0);
            packet << uint32_t(0);                                      // flags 2
            packet << uint32_t(info != nullptr ? info->Type : 0);
            packet << uint32_t(info != nullptr ? info->Family : 0);
            packet << uint32_t(info != nullptr ? info->Rank : 0);
            packet << uint32_t(info != nullptr ? info->killcredit[0] : 0);
            packet << uint32_t(info != nullptr ? info->killcredit[1] : 0);
            packet << uint32_t(info != nullptr ? info->Male_DisplayID : 0);
            packet << uint32_t(info != nullptr ? info->Female_DisplayID : 0);
            packet << uint32_t(info != nullptr ? info->Male_DisplayID2 : 0);
            packet << uint32_t(info != nullptr ? info->Female_DisplayID2 : 0);
            packet << float(1.0f);                                      // health modifier
            packet << float(1.0f);                                      // power modifier
            packet << uint32_t(questItems.size());
            packet << uint32_t(info != nullptr ? info->waypointid : 0); // movement info
            if (m_protocol.isLegion())
                packet << uint32_t(0);                                  // health scaling expansion
            packet << uint32_t(0);                                      // required expansion
            packet << uint32_t(0);                                      // 6.x: quest flag, 7.x: vignette

            if (!title.empty())
                packet << title;

            if (!cursorName.empty())
                packet << cursorName;

            for (const uint32_t questItem : questItems)
                packet << questItem;

            return true;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion())
                return serialiseLegion(packet);

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
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

                    if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                    {
                        for (uint8_t i = 0; i < 3; ++i)
                            packet << uint8_t(0);
                    }
                    else
                    {
                        for (uint8_t i = 0; i < 7; ++i)
                            packet << uint8_t(0);
                    }
                    packet << subName;
                    packet << info->icon_name << info->typeFlags;
                    if (m_protocol.expansion > WoW::Expansion::_WotLK)
                    {
                        packet << uint32_t(0);
                    }
                    packet << info->Type << info->Family << info->Rank;
                    if (m_protocol.expansion > WoW::Expansion::_TBC)
                    {
                        packet << info->killcredit[0] << info->killcredit[1];
                    }
                    else
                    {
                        packet << uint32_t(0) << info->spelldataid;
                    }
                    packet << info->Male_DisplayID << info->Female_DisplayID << info->Male_DisplayID2 << info->Female_DisplayID2;
                    if (m_protocol.expansion > WoW::Expansion::_TBC)
                    {
                        packet << info->baseAttackMod << info->rangeAttackMod;
                    }
                    else
                    {
                        packet << float(0) << float(0); //health and power multiplier.
                    }
                    packet << info->Leader;

                    if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                    {
                        for (uint8_t i = 0; i < 6; ++i)
                            packet << uint32_t(info->QuestItems[i]);

                        packet << info->waypointid;
                    }

                    if (m_protocol.expansion > WoW::Expansion::_WotLK)
                    {
                        packet << uint32_t(0);
                    }
                }
            }
            else
            {
                std::string _name = name;
                std::string _subName = subName;

                bool hasSubname = _subName.length() ? true : false;
                bool hasIcon = false;

                if (entry == 300000)
                {
                    _name = "WayPoint";
                    _subName = "Level is WayPoint ID";

                    packet << entry;
                    packet.writeBit(info ? 1 : 0);

                    packet.writeBits(hasSubname ? _subName.length() + 1 : 0, 11);
                    packet.writeBits(0, 22); // max quest items
                    packet.writeBits(0, 11); // unknown string
                    packet.writeBits(_name.length() + 1, 11);

                    for (uint8_t i = 0; i < 7; ++i)
                        packet.writeBits(0, 11); // name2-name8

                    packet.writeBit(false);
                    packet.writeBits(0, 6);

                    packet.flushBits();

                    packet << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0);
                    packet << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0);

                    packet << _name;
                    if (hasSubname)
                        packet << _subName;

                    packet << uint32_t(0) << uint32_t(0);

                    for (uint8_t i = 0; i < 6; ++i)
                        packet << uint32_t(0);

                    packet << uint32_t(0) << uint32_t(0) << uint32_t(0);
                }
                else
                {
                    packet << entry;
                    packet.writeBit(info ? 1 : 0);

                    if (info)
                    {
                        hasIcon = info->icon_name.length() ? true : false;

                        packet.writeBits(hasSubname ? _subName.length() + 1 : 0, 11);

                        packet.writeBits(6, 22); // max quest items

                        packet.writeBits(0, 11); // unknown string

                        packet.writeBits(_name.length() + 1, 11);

                        for (uint8_t i = 0; i < 7; ++i)
                            packet.writeBits(0, 11); // name2-name8

                        packet.writeBit(info->Leader);
                        packet.writeBits(hasIcon ? info->icon_name.length() + 1 : 0, 6);

                        packet.flushBits();

                        packet << info->killcredit[0];

                        packet << info->Female_DisplayID;
                        packet << info->Female_DisplayID2;
                        packet << uint32_t(0); // version
                        packet << info->Type;
                        packet << float(0); // hp mod
                        packet << info->typeFlags;
                        packet << uint32_t(0); // flags 2
                        packet << info->Rank;
                        packet << info->waypointid;
                        packet << _name;

                        if (hasSubname)
                            packet << _subName;

                        packet << info->Male_DisplayID;
                        packet << info->Male_DisplayID2;

                        if (hasIcon)
                            packet << info->icon_name;

                        for (uint8_t i = 0; i < 6; ++i)
                            packet << uint32_t(info->QuestItems[i]);

                        packet << info->killcredit[1];

                        packet << float(0); // mana mod

                        packet << info->Family;

                        //packet << info->baseAttackMod << info->rangeAttackMod; Zyres not used
                    }
                }
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
