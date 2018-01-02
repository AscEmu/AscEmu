/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2011 <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

#ifndef TOTEMSUMMON_HPP_
#define TOTEMSUMMON_HPP_

#include "Units/Creatures/Creature.h"
#include "Units/Summons/Summon.h"

//////////////////////////////////////////////////////////////////////////////////////////
/// Class that implements Totems. Totems are stationary, and don't attack with melee,
/// however they can cast spells
//////////////////////////////////////////////////////////////////////////////////////////
class TotemSummon : public Summon
{
    public:

        TotemSummon(uint64 GUID);
        ~TotemSummon();

        void Load(CreatureProperties const* properties_, Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);

        void OnPushToWorld();

        void OnPreRemoveFromWorld();

        bool IsTotem() { return true; }

        Group* GetGroup();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Sets up the spells the totem will cast. This code was almost directly copied
        /// from SpellEffects.cpp, it requires further refactoring!
        /// For example totems should cast like other units..
        /// \param none      \returns none
        //////////////////////////////////////////////////////////////////////////////////////////
        void SetupSpells();

        void Die(Unit* pAttacker, uint32 damage, uint32 spellid);
};

#endif      //TOTEMSUMMON_HPP_
