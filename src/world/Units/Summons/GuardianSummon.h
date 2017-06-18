/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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

#ifndef GUARDIANSUMMON_H
#define GUARDIANSUMMON_H

#include "Units/Summons/Summon.h"
#include "Units/Creatures/CreatureDefines.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Class that implements guardians
/// Guardians are summons that follow and protect their owner
//////////////////////////////////////////////////////////////////////////////////////////
class GuardianSummon : public Summon
{
    public:

        GuardianSummon(uint64 GUID);
        ~GuardianSummon();

        void Load(CreatureProperties const* properties_, Unit* owner, LocationVector & position, uint32 spellid, int32 summonslot);
        void OnPushToWorld();
        void OnPreRemoveFromWorld();
};

#endif // _GUARDIANSUMMON_H
