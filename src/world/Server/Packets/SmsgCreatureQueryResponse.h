/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <array>
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

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                packet << entry;
                packet.writeBit(info != nullptr);
                packet.flushBits();

                if (info == nullptr)
                    return true;

                const std::string creatureName = name != nullptr ? name : "";
                const std::string creatureTitle = subName != nullptr ? subName : "";

                packet.writeBits(creatureTitle.empty() ? 0U : static_cast<uint32_t>(creatureTitle.size() + 1U), 11);
                packet.writeBits(0U, 11); // TitleAlt
                packet.writeBits(info->icon_name.empty() ? 0U : static_cast<uint32_t>(info->icon_name.size() + 1U), 6);
                packet.writeBit(info->Leader != 0);

                packet.writeBits(creatureName.empty() ? 0U : static_cast<uint32_t>(creatureName.size() + 1U), 11);
                packet.writeBits(0U, 11); // NameAlt[0]
                for (uint8_t i = 1; i < 4; ++i)
                {
                    packet.writeBits(0U, 11);
                    packet.writeBits(0U, 11);
                }
                packet.flushBits();

                if (!creatureName.empty())
                    packet << creatureName;

                packet << info->typeFlags << uint32_t(0) << uint32_t(0);
                packet << static_cast<uint8_t>(info->Type);
                packet << static_cast<int32_t>(info->Family);
                packet << static_cast<int8_t>(info->Rank);
                packet << info->killcredit[0] << info->killcredit[1];

                struct CreatureDisplay
                {
                    uint32_t id;
                    float scale;
                };

                const float displayScale = info->Scale > 0.0f ? info->Scale : 1.0f;
                const std::array<CreatureDisplay, 4> displays{{
                    { info->Male_DisplayID, displayScale },
                    { info->Female_DisplayID, displayScale },
                    { info->Male_DisplayID2, displayScale },
                    { info->Female_DisplayID2, displayScale }
                }};

                uint32_t displayCount = 0;
                for (auto const& display : displays)
                    if (display.id != 0)
                        ++displayCount;

                packet << displayCount;
                packet << (displayCount != 0 ? 1.0f : 0.0f);

                const float probability = displayCount != 0 ? 1.0f / static_cast<float>(displayCount) : 0.0f;
                for (auto const& display : displays)
                {
                    if (display.id == 0)
                        continue;

                    packet << display.id << display.scale << probability;
                }

                packet << float(1.0f) << float(1.0f);

                uint32_t questItemCount = 0;
                for (uint32_t questItem : info->QuestItems)
                    if (questItem != 0)
                        ++questItemCount;

                packet << questItemCount;
                packet << uint32_t(0); // QuestCurrencies count
                packet << int32_t(0);  // CreatureMovementInfoID
                packet << int32_t(0);  // HealthScalingExpansion
                packet << int32_t(0);  // RequiredExpansion
                packet << int32_t(0);  // VignetteID
                packet << int32_t(0);  // Class
                packet << int32_t(0);  // CreatureDifficultyID
                packet << int32_t(0);  // WidgetSetID
                packet << int32_t(0);  // WidgetSetUnitConditionID

                if (!creatureTitle.empty())
                    packet << creatureTitle;
                if (!info->icon_name.empty())
                    packet << info->icon_name;

                for (uint32_t questItem : info->QuestItems)
                    if (questItem != 0)
                        packet << static_cast<int32_t>(questItem);

                return true;
            }

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
