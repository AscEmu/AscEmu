/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <algorithm>
#include <cstring>
#include <memory>
#include <ranges>

#include "Server/ClientProtocol.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Server/World.h"

enum SpellTreeName
{
    SPELLTREE_MAGE_ARCANE               = 237,
    SPELLTREE_MAGE_FIRE                 = 8,
    SPELLTREE_MAGE_FROST                = 6,

    SPELLTREE_ROGUE_ASSASSINATION       = 253,
    SPELLTREE_ROGUE_COMBAT              = 38,
    SPELLTREE_ROGUE_SUBTLETY            = 39,

    SPELLTREE_WARLOCK_AFFLICTION        = 355,
    SPELLTREE_WARLOCK_DEMONOLOGY        = 354,
    SPELLTREE_WARLOCK_DESTRUCTION       = 593,

    SPELLTREE_WARRIOR_ARMS              = 26,
    SPELLTREE_WARRIOR_FURY              = 256,
    SPELLTREE_WARRIOR_PROTECTION        = 257,

    SPELLTREE_SHAMAN_ELEMENTAL          = 375,
    SPELLTREE_SHAMAN_ENHANCEMENT        = 373,
    SPELLTREE_SHAMAN_RESTORATION        = 374,

    SPELLTREE_PALADIN_HOLY              = 594,
    SPELLTREE_PALADIN_PROTECTION        = 267,
    SPELLTREE_PALADIN_RETRIBUTION       = 184,

    SPELLTREE_DEATHKNIGHT_BLOOD         = 770,
    SPELLTREE_DEATHKNIGHT_FROST         = 771,
    SPELLTREE_DEATHKNIGHT_UNHOLY        = 772,

    SPELLTREE_PRIEST_DISCIPLINE         = 613,
    SPELLTREE_PRIEST_HOLY               = 56,
    SPELLTREE_PRIEST_SHADOW             = 78,

    SPELLTREE_HUNTER_BEASTMASTERY       = 50,
    SPELLTREE_HUNTER_MARKSMANSHIP       = 163,
    SPELLTREE_HUNTER_SURVIVAL           = 51,

    SPELLTREE_DRUID_BALANCE             = 574,
    SPELLTREE_DRUID_FERAL_COMBAT        = 134,
    SPELLTREE_DRUID_RESTORATION         = 573,
};

class SkillNameMgr
{
public:
    std::unique_ptr<std::unique_ptr<char[]>[]> SkillNames;
    uint32_t maxSkill{0};

    SkillNameMgr()
    {
        /// This will become the size of the skill name lookup table
        for (auto const& skillLine : sSkillLineStore | std::views::values)
            maxSkill = std::max(maxSkill, skillLine.id);

        /// SkillNames
        SkillNames = std::make_unique<std::unique_ptr<char[]>[]>(maxSkill + 1);

        const uint32_t locale = sWorld.getDbcLocaleLanguageId();
        const bool isPreCata = !WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata);

        for (auto const& skillLine : sSkillLineStore | std::views::values)
        {
            const uint32_t skillId = skillLine.id;
            std::string const& skillName = (isPreCata && locale < skillLine.name.size())
                                               ? skillLine.name[locale]
                                               : skillLine.name[0];

            SkillNames[skillId] = std::make_unique<char[]>(skillName.size() + 1);

            // When the DBCFile gets cleaned up, so does the record data, so make a copy of it
            std::memcpy(SkillNames[skillId].get(), skillName.c_str(), skillName.size() + 1);
        }
    }

    ~SkillNameMgr() = default;
};
