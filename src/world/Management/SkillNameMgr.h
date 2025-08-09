/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#ifndef SKILLNAMEMGR_H
#define SKILLNAMEMGR_H

#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

#if VERSION_STRING < Cata
#include "Server/World.h"
#endif

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
        uint32_t maxskill;

        SkillNameMgr()
        {
            /// This will become the size of the skill name lookup table
            maxskill = sSkillLineStore.getNumRows();

            /// SkillNames = (char **) malloc(maxskill * sizeof(char *));
            SkillNames = std::make_unique<std::unique_ptr<char[]>[]>(maxskill + 1); //(+1, arrays count from 0.. not 1.)
            std::fill(SkillNames.get(), SkillNames.get(), nullptr);

            for (uint32_t i = 0; i < sSkillLineStore.getNumRows(); ++i)
            {
                auto skill_line = sSkillLineStore.lookupEntry(i);
                if (skill_line == nullptr)
                    continue;

                uint32_t SkillID = skill_line->id;
#if VERSION_STRING < Cata
                char* SkillName = skill_line->Name[sWorld.getDbcLocaleLanguageId()];
#else
                char* SkillName = skill_line->Name[0];
#endif

                SkillNames[SkillID] = std::make_unique<char[]>(strlen(SkillName) + 1);
                //When the DBCFile gets cleaned up, so does the record data, so make a copy of it..
                std::strcpy(SkillNames[SkillID].get(), SkillName);
            }
        }

        ~SkillNameMgr() = default;
};

#endif // SKILLNAMEMGR_H
