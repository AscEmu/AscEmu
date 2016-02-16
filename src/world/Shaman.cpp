/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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
 *
 */

#include "StdAfx.h"


void World::InitShamanSpells()
{
    SpellEntry * sp = NULL;

    if (sp = dbcSpell.LookupEntryForced(58583))
    {
        sp->EffectTriggerSpell[0] = 58586;
    }

    if (sp = dbcSpell.LookupEntryForced(58584))
    {
        sp->EffectTriggerSpell[0] = 58587;
    }

    if (sp = dbcSpell.LookupEntryForced(58585))
    {
        sp->EffectTriggerSpell[0] = 58588;
    }

    if (sp = dbcSpell.LookupEntryForced(16086))
    {
        sp->EffectApplyAuraName[2] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[2] = 5648;
        sp->procChance = 50;
        sp->procFlags = PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT | PROC_ON_SPELL_HIT;
        //sp->custom_c_is_flags |= SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_ON_TOTEM;
    }
    if (sp = dbcSpell.LookupEntryForced(16544))
    {
        sp->EffectApplyAuraName[2] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[2] = 5648;
        sp->procChance = 100;
        sp->procFlags = PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT | PROC_ON_SPELL_HIT;
        //sp->custom_c_is_flags |= SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_ON_TOTEM;
    }
    if (sp = dbcSpell.LookupEntryForced(16166))
    {
        auto source_spell = dbcSpell.LookupEntryForced(64701);
        if (source_spell != nullptr)
            sp->DurationIndex = source_spell->DurationIndex;
    }

}
