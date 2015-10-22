/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org/>
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


void World::InitRogueSpells()
{
    SpellEntry * sp = NULL;
    sp = dbcSpell.LookupEntryForced(45182);
    if (sp)
    {
        sp->Category = 60000;
        sp->CategoryRecoveryTime = 60000;
        sp->StartRecoveryTime = 60000;
    }

    //effect = 6 auraname = 226
    sp = dbcSpell.LookupEntryForced(31221);
    if (sp)
    {
        sp->procChance = 100;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectTriggerSpell[1] = 31666;
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
    }
    sp = dbcSpell.LookupEntryForced(31222);
    if (sp)
    {
        sp->procChance = 100;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectTriggerSpell[1] = 31666;
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
    }
    sp = dbcSpell.LookupEntryForced(31223);
    if (sp)
    {
        sp->procChance = 100;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectTriggerSpell[1] = 31666;
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
    }
    sp = dbcSpell.LookupEntryForced(31665);
    if (sp)
    {
        auto source_spell = dbcSpell.LookupEntryForced(31666);
        if (source_spell != nullptr)
        {
            sp->DurationIndex = source_spell->DurationIndex;
            sp->RankNumber = 4;
        }
    }
    sp = dbcSpell.LookupEntryForced(31666);
    if (sp)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_DAMAGE_PERCENT_DONE;
        sp->EffectMiscValue[0] = 127;
        sp->RankNumber = 4;
    }

    if (sp = dbcSpell.LookupEntryForced(14156))
        sp->procFlags = 87376;

    if (sp = dbcSpell.LookupEntryForced(40477))
        //sp->ProcsPerMinute = 2.0f;

        if (sp = dbcSpell.LookupEntryForced(58426))
        {
            auto source_spell = dbcSpell.LookupEntryForced(58428);
            auto target_spell = dbcSpell.LookupEntryForced(58427);
            if (source_spell != nullptr && target_spell != nullptr)
            {
                sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
                sp->EffectTriggerSpell[0] = 58427;
                sp->procFlags = PROC_ON_CAST_SPELL;
                sp->procChance = 100;
                target_spell->DurationIndex = source_spell->DurationIndex;
            }
        }

    if (sp = dbcSpell.LookupEntryForced(14177))
    {
        sp->EffectSpellClassMask[0][0] = 637665798;
        sp->EffectSpellClassMask[0][1] = 262415;
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;
    }
}
