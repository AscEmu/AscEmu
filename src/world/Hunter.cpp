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

void World::InitHunterSpells()
{
    SpellEntry * sp = NULL;
    if (sp = dbcSpell.LookupEntryForced(6991))
        sp->EffectImplicitTargetA[0] = EFF_TARGET_GAMEOBJECT_ITEM;

    if (sp = dbcSpell.LookupEntryForced(61847))
    {
        sp->Effect[2] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[2] = SPELL_AURA_MOD_DODGE_PERCENT;
    }

    if (sp = dbcSpell.LookupEntryForced(19552))
        sp->custom_BGR_one_buff_on_target = 0;
    if (sp = dbcSpell.LookupEntryForced(19553))
        sp->custom_BGR_one_buff_on_target = 0;
    if (sp = dbcSpell.LookupEntryForced(19554))
        sp->custom_BGR_one_buff_on_target = 0;
    if (sp = dbcSpell.LookupEntryForced(19555))
        sp->custom_BGR_one_buff_on_target = 0;
    if (sp = dbcSpell.LookupEntryForced(19556))
        sp->custom_BGR_one_buff_on_target = 0;

    if (sp = dbcSpell.LookupEntryForced(53252))
    {
        sp->custom_c_is_flags |= SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_ON_PET;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->EffectTriggerSpell[0] = 53398;
        sp->procChance = 50;
        sp->procFlags = PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT | PROC_TARGET_SELF;
    }

    if (sp = dbcSpell.LookupEntryForced(53253))
    {
        sp->custom_c_is_flags |= SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_ON_PET;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->EffectTriggerSpell[0] = 53398;
        sp->procChance = 100;
        sp->procFlags = PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT | PROC_TARGET_SELF;
    }

    if (sp = dbcSpell.LookupEntryForced(53398))
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET_MASTER;
    }

    if (sp = dbcSpell.LookupEntryForced(34456))
    {
        sp->EffectBasePoints[0] = 3;
    }

    if (sp = dbcSpell.LookupEntryForced(53257))
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    }

    if (sp = dbcSpell.LookupEntryForced(56314))
    {
        sp->custom_c_is_flags |= SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_PET_OWNER | SPELL_FLAG_IS_EXPIREING_WITH_PET;
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 57447;
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 57475;
    }

    if (sp = dbcSpell.LookupEntryForced(56315))
    {
        sp->custom_c_is_flags |= SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_PET_OWNER | SPELL_FLAG_IS_EXPIREING_WITH_PET;
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 57452;
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 57482;
    }

    if (sp = dbcSpell.LookupEntryForced(56316))
    {
        sp->custom_c_is_flags |= SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_PET_OWNER | SPELL_FLAG_IS_EXPIREING_WITH_PET;
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 57453;
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 57483;
    }

    if (sp = dbcSpell.LookupEntryForced(56317))
    {
        sp->custom_c_is_flags |= SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_PET_OWNER | SPELL_FLAG_IS_EXPIREING_WITH_PET;
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 57457;
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 57484;
    }

    if (sp = dbcSpell.LookupEntryForced(56318))
    {
        sp->custom_c_is_flags |= SPELL_FLAG_IS_CASTED_ON_PET_SUMMON_PET_OWNER | SPELL_FLAG_IS_EXPIREING_WITH_PET;
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 57458;
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 57485;
    }

    if (sp = dbcSpell.LookupEntryForced(53220))
    {
        sp->NameHash += 1;
    }

    if (sp = dbcSpell.LookupEntryForced(34720))
        if (sp = dbcSpell.LookupEntryForced(781))
            sp->Effect[0] = SPELL_EFFECT_DUMMY;
}
