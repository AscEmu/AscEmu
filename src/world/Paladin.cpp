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


void World::InitPaladinSpells()
{
    SpellEntry * sp = NULL;
    sp = dbcSpell.LookupEntryForced(32746);
    if (sp != NULL)
    {
        sp->Category = 1500;
        sp->CategoryRecoveryTime = 1500;
        sp->StartRecoveryTime = 1500;
    }

    if (sp = dbcSpell.LookupEntryForced(20271))
    {
        auto source_spell = dbcSpell.LookupEntryForced(20185);
        if (source_spell != nullptr)
        {
            sp->Rank = source_spell->Rank;
            sp->custom_RankNumber = source_spell->custom_RankNumber;
        }
    }

    if (sp = dbcSpell.LookupEntryForced(59295))
    {
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
        sp->custom_BGR_one_buff_on_target = 0;
    }
    if (sp = dbcSpell.LookupEntryForced(59296))
    {
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
        sp->custom_BGR_one_buff_on_target = 0;
    }
    if (sp = dbcSpell.LookupEntryForced(59297))
    {
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
        sp->custom_BGR_one_buff_on_target = 0;
    }
    if (sp = dbcSpell.LookupEntryForced(59298))
    {
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
        sp->custom_BGR_one_buff_on_target = 0;
    }
    if (sp = dbcSpell.LookupEntryForced(53486))
    {
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
        sp->EffectSpellClassMask[0][0] |= 8388608 | 32768 | 131072;
        sp->procFlags = PROC_ON_RANGED_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT | PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT;
    }
    if (sp = dbcSpell.LookupEntryForced(53488))
    {
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
        sp->EffectSpellClassMask[0][0] |= 8388608 | 32768 | 131072;
        sp->procFlags = PROC_ON_RANGED_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT | PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT;
    }

    if (sp = dbcSpell.LookupEntryForced(53489))
    {
        sp->custom_NameHash += 1;
        sp->custom_RankNumber += 1;
    }
    if (sp = dbcSpell.LookupEntryForced(59578))
    {
        sp->custom_NameHash += 1;
        sp->custom_RankNumber += 2;
    }

    if (sp = dbcSpell.LookupEntryForced(53720))
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }

    if (sp = dbcSpell.LookupEntryForced(31892))
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }

    if (sp = dbcSpell.LookupEntryForced(53695))
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
    }
    if (sp = dbcSpell.LookupEntryForced(53696))
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
    }

    if (sp = dbcSpell.LookupEntryForced(53569))
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    if (sp = dbcSpell.LookupEntryForced(53576))
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    if (sp = dbcSpell.LookupEntryForced(53672))
    {
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(54149))
    {
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;
    }

    if (sp = dbcSpell.LookupEntryForced(20911))             //??????
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN;
    }

    if (sp = dbcSpell.LookupEntryForced(31850))
    {
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN;
        sp->EffectBasePoints[1] = -11;
    }

    if (sp = dbcSpell.LookupEntryForced(31851))
    {
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN;
        sp->EffectBasePoints[1] = -21;
    }

    if (sp = dbcSpell.LookupEntryForced(31852))
    {
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN;
        sp->EffectBasePoints[1] = -31;
    }
}
