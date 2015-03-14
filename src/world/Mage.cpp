/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org/>
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
 *
 */

#include "StdAfx.h"


void World::InitMageSpells()
{
    SpellEntry * sp = NULL;
    sp = dbcSpell.LookupEntryForced(130);
    if (sp != NULL)
        sp->EffectApplyAuraName[0] = SPELL_AURA_SAFE_FALL;

    if (sp = dbcSpell.LookupEntryForced(57761))
    {
        sp->procCharges = 1;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->AuraInterruptFlags |= AURA_INTERRUPT_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(54646))
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    if (sp = dbcSpell.LookupEntryForced(44404))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    if (sp = dbcSpell.LookupEntryForced(54486))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    if (sp = dbcSpell.LookupEntryForced(54488))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    if (sp = dbcSpell.LookupEntryForced(54489))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    if (sp = dbcSpell.LookupEntryForced(54490))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(44546))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(44548))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(44549))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(44543))
    {
        sp->procChance = 7;
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(44545))
    {
        sp->procChance = 15;
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(33395))
    {
        sp->CategoryRecoveryTime = 24000;
    }

    if (sp = dbcSpell.LookupEntryForced(55080))
    {
        sp->MechanicsType = MECHANIC_FROZEN;
    }

    if (sp = dbcSpell.LookupEntryForced(12494))
    {
        sp->MechanicsType = MECHANIC_FROZEN;
    }

    if (sp = dbcSpell.LookupEntryForced(29077))
    {
        //sp->logsId = 29077;
    }

    if (sp = dbcSpell.LookupEntryForced(57531))
    {
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;
    }
    if (sp = dbcSpell.LookupEntryForced(36032))
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;

    if (sp = dbcSpell.LookupEntryForced(31687))
    {
        sp->Effect[0] = SPELL_EFFECT_SUMMON;
        sp->Effect[1] = 0;
        sp->EffectApplyAuraName[0] = sp->EffectApplyAuraName[1] = 0;
        sp->EffectMiscValueB[0] = 0;
        sp->EffectMiscValue[0] = 510;
    }
}
