/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

#ifndef POSSESSEDSUMMON_H
#define POSSESSEDSUMMON_H

#include "Units/Summons/Summon.h"
#include "Units/Creatures/CreatureDefines.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Class that implements summons that are possessed by the player after spawning.
/// They despawn when killed or dismissed
//////////////////////////////////////////////////////////////////////////////////////////
class PossessedSummon : public Summon
{
    public:

        PossessedSummon(uint64 GUID);
        ~PossessedSummon();

        void Load(CreatureProperties const* properties_, Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);

        void OnPushToWorld();
        void OnPreRemoveFromWorld();
};

#endif // _POSSESSEDSUMMON_H
