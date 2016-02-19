/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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

void World::InitDruidSpells()
{
    SpellEntry* sp = NULL;

    /* Zyres: Genius... Delete this!
    if (sp = dbcSpell.LookupEntryForced(34123))
        sp->custom_NameHash = 1;*/

    if (sp = dbcSpell.LookupEntryForced(34297))
    {
        sp->custom_apply_on_shapeshift_change = true;
        sp->RequiredShapeShift = 255;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    }
    if (sp = dbcSpell.LookupEntryForced(34300))
    {
        sp->custom_apply_on_shapeshift_change = true;
        sp->RequiredShapeShift = 255;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    }
    if (sp = dbcSpell.LookupEntryForced(17007))
    {
        sp->custom_apply_on_shapeshift_change = true;
        sp->RequiredShapeShift = 255;
    }

    /*Zyres: Delete this!
    if (sp = dbcSpell.LookupEntryForced(34299))
        sp->custom_NameHash += 1;*/

    if (sp = dbcSpell.LookupEntryForced(48483))
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }

    if (sp = dbcSpell.LookupEntryForced(48484))
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }

    if (sp = dbcSpell.LookupEntryForced(48485))
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }

    if (sp = dbcSpell.LookupEntryForced(48389))
    {
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;
    }

    if (sp = dbcSpell.LookupEntryForced(48392))
    {
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;
    }

    if (sp = dbcSpell.LookupEntryForced(48393))
    {
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;
    }

    if (sp = dbcSpell.LookupEntryForced(48516))
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    if (sp = dbcSpell.LookupEntryForced(48521))
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    if (sp = dbcSpell.LookupEntryForced(48525))
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    /*Zyres: Delete this!
    if (sp = dbcSpell.LookupEntryForced(48517))
    {
        sp->custom_NameHash += 1;
    }

    if (sp = dbcSpell.LookupEntryForced(48518))
    {
        sp->custom_NameHash += 2;
    }*/

    if (sp = dbcSpell.LookupEntryForced(48506))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(48510))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(48511))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }


    if (sp = dbcSpell.LookupEntryForced(48384))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 50170;
    }

    if (sp = dbcSpell.LookupEntryForced(48395))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 50171;
    }

    if (sp = dbcSpell.LookupEntryForced(48396))
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 50172;
    }
    if (sp = dbcSpell.LookupEntryForced(51185))
    {
        sp->RequiredShapeShift = 144;
    }
    if (sp = dbcSpell.LookupEntryForced(51178))
    {
        sp->custom_NameHash += 1;
    }
    if (sp = dbcSpell.LookupEntryForced(50334))
    {
        sp->EffectSpellClassMask[2][0] = 256;
        sp->EffectSpellClassMask[2][1] = 0;
        sp->EffectSpellClassMask[2][2] = 0;
    }
    /*if (sp = dbcSpell.LookupEntryForced(17002))
        sp->RequiredShapeShift = 1 | 16 | 128;
        if (sp = dbcSpell.LookupEntryForced(24866))
        sp->RequiredShapeShift = 1 | 16 | 128; */
}
