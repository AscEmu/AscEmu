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

void CreateDummySpell(uint32 id)
{
    const char* name = "Dummy Trigger";
    SpellEntry* sp = new SpellEntry;
    memset(sp, 0, sizeof(SpellEntry));
    sp->Id = id;
    sp->Attributes = 384;
    sp->AttributesEx = 268435456;
    sp->AttributesExB = 4;
    sp->CastingTimeIndex = 1;
    sp->procChance = 75;
    sp->rangeIndex = 13;
    sp->EquippedItemClass = uint32(-1);
    sp->Effect[0] = SPELL_EFFECT_DUMMY;
    sp->EffectImplicitTargetA[0] = 25;
    sp->custom_NameHash = crc32((const unsigned char*)name, (unsigned int)strlen(name));
    sp->dmg_multiplier[0] = 1.0f;
    sp->StanceBarOrder = -1;
    dbcSpell.SetRow(id, sp);
    sWorld.dummyspells.push_back(sp);
}

void Modify_EffectBasePoints(SpellEntry* sp)
{
    if (sp == nullptr)
    {
        Log.Error("Modify_EffectBasePoints", "Something tried to call with an invalid spell pointer!");
        return;
    }

    //Rogue: Poison time fix for 2.3
    if (sp->Id == 3408)                 // Crippling Poison && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->EffectBasePoints[0] = 3599;
    if (sp->Id == 5761)                 // Mind-numbing Poison && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->EffectBasePoints[0] = 3599;
    if (sp->Id == 8679)                 // Instant Poison && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->EffectBasePoints[0] = 3599;
    if (sp->Id == 2823)                 // Deadly Poison && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->EffectBasePoints[0] = 3599;
    if (sp->Id == 13219)                // Wound Poison && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->EffectBasePoints[0] = 3599;
    if (sp->Id == 26785)                // Anesthetic Poison && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->EffectBasePoints[0] = 3599;

    // Zyres: According to the description the weapon damage gets increased from 2 to 12 (depends on the different spell ids)
    if (sp->Id == 2828 || sp->Id == 29452 || sp->Id == 29453 || sp->Id == 56308) //Sharpen Blade && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->EffectBasePoints[0] = 3599;

    // Set the diff. EffectBasePoint from description.
    if (sp->Id == 11119)     // Ignite Rank 1
        sp->EffectBasePoints[0] = 8;
    if (sp->Id == 11120)     // Ignite Rank 2
        sp->EffectBasePoints[0] = 16;
    if (sp->Id == 12846)     // Ignite Rank 3
        sp->EffectBasePoints[0] = 24;
    if (sp->Id == 12847)     // Ignite Rank 4
        sp->EffectBasePoints[0] = 32;
    if (sp->Id == 12848)     // Ignite Rank 5
        sp->EffectBasePoints[0] = 40;
}

void Set_missing_spellLevel(SpellEntry* sp)
{
    if (sp == nullptr)
    {
        Log.Error("Set_missing_spellLevel", "Something tried to call with an invalid spell pointer!");
        return;
    }

    //stupid spell ranking problem
    if (sp->spellLevel == 0)
    {
        uint32 new_level = 0;

        if (strstr(sp->Name, "Apprentice "))
            new_level = 1;
        else if (strstr(sp->Name, "Journeyman "))
            new_level = 2;
        else if (strstr(sp->Name, "Expert "))
            new_level = 3;
        else if (strstr(sp->Name, "Artisan "))
            new_level = 4;
        else if (strstr(sp->Name, "Master "))
            new_level = 5;
        else if (strstr(sp->Name, "Grand Master "))
            new_level = 6;

        if (new_level != 0)
        {
            uint32 teachspell = 0;
            if (sp->Effect[0] == SPELL_EFFECT_LEARN_SPELL)
                teachspell = sp->EffectTriggerSpell[0];
            else if (sp->Effect[1] == SPELL_EFFECT_LEARN_SPELL)
                teachspell = sp->EffectTriggerSpell[1];
            else if (sp->Effect[2] == SPELL_EFFECT_LEARN_SPELL)
                teachspell = sp->EffectTriggerSpell[2];

            if (teachspell)
            {
                SpellEntry* spellInfo;
                spellInfo = CheckAndReturnSpellEntry(teachspell);
                spellInfo->spellLevel = new_level;
                sp->spellLevel = new_level;
            }
        }
    }
}

void Set_Custom_apply_on_shapeshift_change(SpellEntry* sp)
{
    if (sp == nullptr)
    {
        Log.Error("Set_Custom_apply_on_shapeshift_change", "Something tried to call with an invalid spell pointer!");
        return;
    }

    if (sp->Id == 5225 || sp->Id == 19883)
        sp->custom_apply_on_shapeshift_change = true;   // apply on shapeshift change
}

void Set_Custom_always_apply(SpellEntry* sp)
{
    if (sp == nullptr)
    {
        Log.Error("Set_Custom_always_apply", "Something tried to call with an invalid spell pointer!");
        return;
    }

    switch (sp->Id)
    {
        // SPELL_HASH_BLOOD_FURY
        case 20572:
        case 23230:
        case 24571:
        case 33697:
        case 33702:
        // SPELL_HASH_SHADOWSTEP
        case 36554:
        case 36563:
        case 41176:
        case 44373:
        case 45273:
        case 46463:
        case 55965:
        case 55966:
        case 63790:
        case 63793:
        case 66178:
        case 68759:
        case 68760:
        case 68761:
        case 69087:
        case 70431:
        case 72326:
        case 72327:

        // SPELL_HASH_PSYCHIC_HORROR
        case 34984:
        case 65545:     // Psychic Horror
        {
            sp->custom_always_apply = true;
        } break;
        default:
            break;
    }
}

void Modify_AuraInterruptFlags(SpellEntry* sp)
{
    if (sp == nullptr)
    {
        Log.Error("Modify_AuraInterruptFlags", "Something tried to call with an invalid spell pointer!");
        return;
    }

    // HACK FIX: Break roots/fear on damage.. this needs to be fixed properly!
    if (!(sp->AuraInterruptFlags & AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN))
    {
        for (uint32 z = 0; z < 3; ++z)
        {
            if (sp->EffectApplyAuraName[z] == SPELL_AURA_MOD_FEAR || sp->EffectApplyAuraName[z] == SPELL_AURA_MOD_ROOT)
            {
                sp->AuraInterruptFlags |= AURA_INTERRUPT_ON_UNUSED2;
                break;
            }
        }
    }
}

void Modify_RecoveryTime(SpellEntry* sp)
{
    if (sp == nullptr)
    {
        Log.Error("Modify_RecoveryTime", "Something tried to call with an invalid spell pointer!");
        return;
    }

    // Description includes
    switch (sp->Id)
    {
        // "Must remain seated" 154 rows o.O
        case 430:
        case 431:
        case 432:
        case 433:
        case 434:
        case 435:
        case 833:
        case 1127:
        case 1129:
        case 1131:
        case 1133:
        case 1135:
        case 1137:
        case 2639:
        case 5004:
        case 5005:
        case 5006:
        case 5007:
        case 7737:
        case 9177:
        case 10250:
        case 10256:
        case 10257:
        case 18071:
        case 18124:
        case 18140:
        case 18229:
        case 18230:
        case 18231:
        case 18232:
        case 18233:
        case 18234:
        case 21149:
        case 22731:
        case 22734:
        case 23540:
        case 23541:
        case 23542:
        case 23692:
        case 23698:
        case 24005:
        case 24355:
        case 24384:
        case 24409:
        case 24410:
        case 24411:
        case 24707:
        case 24800:
        case 24869:
        case 25660:
        case 25690:
        case 25691:
        case 25692:
        case 25693:
        case 25697:
        case 25700:
        case 25701:
        case 25886:
        case 25887:
        case 25990:
        case 26030:
        case 26263:
        case 27089:
        case 27094:
        case 28616:
        case 29007:
        case 29008:
        case 29029:
        case 29055:
        case 29073:
        case 30024:
        case 32112:
        case 33253:
        case 33255:
        case 33258:
        case 33260:
        case 33262:
        case 33264:
        case 33266:
        case 33269:
        case 33725:
        case 33772:
        case 34291:
        case 35270:
        case 35271:
        case 40543:
        case 40745:
        case 40768:
        case 41030:
        case 41031:
        case 42207:
        case 42308:
        case 42309:
        case 42311:
        case 42312:
        case 43154:
        case 43180:
        case 43182:
        case 43183:
        case 43706:
        case 43763:
        case 44107:
        case 44109:
        case 44110:
        case 44111:
        case 44112:
        case 44113:
        case 44114:
        case 44115:
        case 44116:
        case 44166:
        case 45019:
        case 45020:
        case 45548:
        case 45618:
        case 46683:
        case 46755:
        case 46812:
        case 46898:
        case 49472:
        case 52911:
        case 53283:
        case 53373:
        case 56439:
        case 57069:
        case 57070:
        case 57073:
        case 57084:
        case 57649:
        case 58645:
        case 58648:
        case 58886:
        case 61827:
        case 61828:
        case 61829:
        case 61830:
        case 61874:
        case 64056:
        case 64354:
        case 64355:
        case 64356:
        case 65363:
        case 65418:
        case 65419:
        case 65420:
        case 65421:
        case 65422:
        case 69560:
        case 69561:
        case 71068:
        case 71071:
        case 71073:
        case 71074:
        case 72623:
        {
            sp->RecoveryTime = 1000;
            sp->CategoryRecoveryTime = 1000;
        } break;
        default:
            break;
    }
}

void ApplyNormalFixes()
{
    //Updating spell.dbc

    Log.Success("World", "Processing %u spells...", dbcSpell.GetNumRows());

    //checking if the DBCs have been extracted from an english client, based on namehash of spell 4, the first with a different name in non-english DBCs
    SpellEntry* sp = dbcSpell.LookupEntry(4);
    if (crc32((const unsigned char*)sp->Name, (unsigned int)strlen(sp->Name)) != SPELL_HASH_WORD_OF_RECALL_OTHER)
    {
        Log.LargeErrorMessage("You are using DBCs extracted from an unsupported client.", "ArcEmu supports only enUS and enGB!!!", NULL);
        abort();
    }

    uint32 cnt = dbcSpell.GetNumRows();

    for (uint32 x = 0; x < cnt; x++)
    {
        // Read every SpellEntry row
        sp = dbcSpell.LookupRow(x);

        uint32 namehash = 0;

        sp->custom_apply_on_shapeshift_change = false;

        // hash the name
        //!!!!!!! representing all strings on 32 bits is dangerous. There is a chance to get same hash for a lot of strings ;)
        namehash = crc32((const unsigned char*)sp->Name, (unsigned int)strlen(sp->Name));
        sp->custom_NameHash = namehash; //need these set before we start processing spells

        float radius = std::max(::GetRadius(sSpellRadiusStore.LookupEntry(sp->EffectRadiusIndex[0])), ::GetRadius(sSpellRadiusStore.LookupEntry(sp->EffectRadiusIndex[1])));
        radius = std::max(::GetRadius(sSpellRadiusStore.LookupEntry(sp->EffectRadiusIndex[2])), radius);
        radius = std::max(GetMaxRange(sSpellRangeStore.LookupEntry(sp->rangeIndex)), radius);
        sp->custom_base_range_or_radius_sqr = radius * radius;

        sp->ai_target_type = GetAiTargetType(sp);
        // NEW SCHOOLS AS OF 2.4.0:
        // (bitwise)
        //SCHOOL_NORMAL = 1,
        //SCHOOL_HOLY   = 2,
        //SCHOOL_FIRE   = 4,
        //SCHOOL_NATURE = 8,
        //SCHOOL_FROST  = 16,
        //SCHOOL_SHADOW = 32,
        //SCHOOL_ARCANE = 64

        // Save School as custom_SchoolMask, and set School as an index
        sp->custom_SchoolMask = sp->School;
        for (uint8 i = 0; i < SCHOOL_COUNT; i++)
        {
            if (sp->School & (1 << i))
            {
                sp->School = i;
                break;
            }
        }

        ARCEMU_ASSERT(sp->School < SCHOOL_COUNT);

        // correct caster/target aura states
        if (sp->CasterAuraState > 1)
            sp->CasterAuraState = 1 << (sp->CasterAuraState - 1);

        if (sp->TargetAuraState > 1)
            sp->TargetAuraState = 1 << (sp->TargetAuraState - 1);


        //there are some spells that change the "damage" value of 1 effect to another : devastate = bonus first then damage
        //this is a total bullshit so remove it when spell system supports effect overwriting
        for (uint32 col1_swap = 0; col1_swap < 2; col1_swap++)
            for (uint32 col2_swap = col1_swap; col2_swap < 3; col2_swap++)
                if (sp->Effect[col1_swap] == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE && sp->Effect[col2_swap] == SPELL_EFFECT_DUMMYMELEE)
                {
                    uint32 temp;
                    float ftemp;
                    temp = sp->Effect[col1_swap];
                    sp->Effect[col1_swap] = sp->Effect[col2_swap];
                    sp->Effect[col2_swap] = temp;
                    temp = sp->EffectDieSides[col1_swap];
                    sp->EffectDieSides[col1_swap] = sp->EffectDieSides[col2_swap];
                    sp->EffectDieSides[col2_swap] = temp;
                    //temp = sp->EffectBaseDice[col1_swap];    sp->EffectBaseDice[col1_swap] = sp->EffectBaseDice[col2_swap] ;        sp->EffectBaseDice[col2_swap] = temp;
                    //ftemp = sp->EffectDicePerLevel[col1_swap];            sp->EffectDicePerLevel[col1_swap] = sp->EffectDicePerLevel[col2_swap] ;                sp->EffectDicePerLevel[col2_swap] = ftemp;
                    ftemp = sp->EffectRealPointsPerLevel[col1_swap];
                    sp->EffectRealPointsPerLevel[col1_swap] = sp->EffectRealPointsPerLevel[col2_swap];
                    sp->EffectRealPointsPerLevel[col2_swap] = ftemp;
                    temp = sp->EffectBasePoints[col1_swap];
                    sp->EffectBasePoints[col1_swap] = sp->EffectBasePoints[col2_swap];
                    sp->EffectBasePoints[col2_swap] = temp;
                    temp = sp->EffectMechanic[col1_swap];
                    sp->EffectMechanic[col1_swap] = sp->EffectMechanic[col2_swap];
                    sp->EffectMechanic[col2_swap] = temp;
                    temp = sp->EffectImplicitTargetA[col1_swap];
                    sp->EffectImplicitTargetA[col1_swap] = sp->EffectImplicitTargetA[col2_swap];
                    sp->EffectImplicitTargetA[col2_swap] = temp;
                    temp = sp->EffectImplicitTargetB[col1_swap];
                    sp->EffectImplicitTargetB[col1_swap] = sp->EffectImplicitTargetB[col2_swap];
                    sp->EffectImplicitTargetB[col2_swap] = temp;
                    temp = sp->EffectRadiusIndex[col1_swap];
                    sp->EffectRadiusIndex[col1_swap] = sp->EffectRadiusIndex[col2_swap];
                    sp->EffectRadiusIndex[col2_swap] = temp;
                    temp = sp->EffectApplyAuraName[col1_swap];
                    sp->EffectApplyAuraName[col1_swap] = sp->EffectApplyAuraName[col2_swap];
                    sp->EffectApplyAuraName[col2_swap] = temp;
                    temp = sp->EffectAmplitude[col1_swap];
                    sp->EffectAmplitude[col1_swap] = sp->EffectAmplitude[col2_swap];
                    sp->EffectAmplitude[col2_swap] = temp;
                    ftemp = sp->EffectMultipleValue[col1_swap];
                    sp->EffectMultipleValue[col1_swap] = sp->EffectMultipleValue[col2_swap];
                    sp->EffectMultipleValue[col2_swap] = ftemp;
                    temp = sp->EffectChainTarget[col1_swap];
                    sp->EffectChainTarget[col1_swap] = sp->EffectChainTarget[col2_swap];
                    sp->EffectChainTarget[col2_swap] = temp;
                    temp = sp->EffectMiscValue[col1_swap];
                    sp->EffectMiscValue[col1_swap] = sp->EffectMiscValue[col2_swap];
                    sp->EffectMiscValue[col2_swap] = temp;
                    temp = sp->EffectTriggerSpell[col1_swap];
                    sp->EffectTriggerSpell[col1_swap] = sp->EffectTriggerSpell[col2_swap];
                    sp->EffectTriggerSpell[col2_swap] = temp;
                    ftemp = sp->EffectPointsPerComboPoint[col1_swap];
                    sp->EffectPointsPerComboPoint[col1_swap] = sp->EffectPointsPerComboPoint[col2_swap];
                    sp->EffectPointsPerComboPoint[col2_swap] = ftemp;
                }

        for (uint32 b = 0; b < 3; ++b)
        {
            if (sp->EffectTriggerSpell[b] != 0 && dbcSpell.LookupEntryForced(sp->EffectTriggerSpell[b]) == NULL)
            {
                // proc spell referencing non-existent spell. create a dummy spell for use w/ it.
                CreateDummySpell(sp->EffectTriggerSpell[b]);
            }

            if (sp->Attributes & ATTRIBUTES_ONLY_OUTDOORS && sp->EffectApplyAuraName[b] == SPELL_AURA_MOUNTED)
            {
                sp->Attributes &= ~ATTRIBUTES_ONLY_OUTDOORS;
            }

            if (sp->EffectApplyAuraName[b] == SPELL_AURA_PREVENT_RESURRECTION)
			{
				sp->Attributes |= ATTRIBUTES_NEGATIVE;
				sp->AttributesExC |= CAN_PERSIST_AND_CASTED_WHILE_DEAD;
			}
        }

        sp->custom_spell_coef_flags = 0;
        sp->Dspell_coef_override = -1;
        sp->OTspell_coef_override = -1;
        sp->casttime_coef = 0;
        sp->fixed_dddhcoef = -1;
        sp->fixed_hotdotcoef = -1;

        
        // DankoDJ: Refactoring session 16/02/2016 new functions
        Modify_EffectBasePoints(sp);
        Set_missing_spellLevel(sp);
        Modify_AuraInterruptFlags(sp);
        Modify_RecoveryTime(sp);

        // DankoDJ: Refactoring session 16/02/2016 set up custom spell fields
        Set_Custom_apply_on_shapeshift_change(sp);
        Set_Custom_always_apply(sp);

        // find diminishing status
        sp->custom_DiminishStatus = GetDiminishingGroup(sp->custom_NameHash);

        // various flight spells
        // these make vehicles and other charmed stuff fliable
        if (sp->activeIconID == 2158)
            sp->Attributes |= ATTRIBUTES_PASSIVE;


        //Name includes "" overwrites
        switch (sp->Id)
        {
            // Name includes "Ignite" (and "an additional" % in description)
            // mage ignite talent should proc only on some chances
            case 11119:     // Ignite Rank 1
            case 11120:     // Ignite Rank 2
            case 12846:     // Ignite Rank 3
            case 12847:     // Ignite Rank 4
            case 12848:     // Ignite Rank 5
            {
                sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL; //force him to use procspell effect
                sp->EffectTriggerSpell[0] = 12654;          //evil , but this is good for us :D
                sp->procFlags = PROC_ON_SPELL_CRIT_HIT;     //add procflag here since this was not processed with the others !
            } break;

            // Name includes "Winter's Chill"
            // Winter's Chill handled by frost school
            case 11180:     // Winter's Chill Rank 1
            case 12579:
            case 28592:     // Winter's Chill Rank 2
            case 28593:     // Winter's Chill Rank 3
            case 63094:
            {
                sp->School = SCHOOL_FROST;
            } break;

            // Name includes "Chain Heal"
            // more triggered spell ids are wrong. I think blizz is trying to outsmart us :S
            // Chain Heal all ranks %50 heal value (49 + 1)
            case 1064:
            case 10622:
            case 10623:
            case 14900:
            case 15799:
            case 16367:
            case 21899:
            case 23573:
            case 25422:
            case 25423:
            case 26122:
            case 30872:
            case 30873:
            case 33642:
            case 38322:
            case 38434:
            case 38435:
            case 41114:
            case 42027:
            case 42477:
            case 43527:
            case 43752:
            case 48894:
            case 54481:
            case 55437:
            case 55458:
            case 55459:
            case 55537:
            case 57232:
            case 59473:
            case 60167:
            case 61321:
            case 67226:
            case 67389:
            case 69923:
            case 70425:
            case 71120:
            case 75370:
            {
                sp->EffectDieSides[0] = 49;
            } break;

            //some procs trigger at intervals
            // Name includes "Water Shield"
            case 16180:
            case 16196:
            case 16198:
            case 23575:
            case 33737:
            case 34318:
            case 34827:
            case 34828:
            case 36816:
            case 36817:
            case 37209:
            case 37432:
            case 38105:
            case 38106:
            case 52128:
            case 52130:
            case 52132:
            case 52133:
            case 52135:
            case 52137:
            case 55535:
            case 57961:
            case 58063:
            case 58266:
            case 58332:
            case 60166:
            {
                sp->custom_proc_interval = 3000;  //3 seconds
                sp->procFlags |= PROC_TARGET_SELF;
            } break;

            // Name includes "Earth Shield"
            case 32734:
            case 37204:
            case 38101:
            case 38102:
            case 38590:
            case 51560:
            case 51561:
            case 54479:
            case 54480:
            case 55599:
            case 55600:
            case 56451:
            case 57802:
            case 57803:
            case 58981:
            case 58982:
            case 59471:
            case 59472:
            case 60013:
            case 60014:
            case 63279:
            case 63925:
            case 64261:
            case 66063:
            case 66064:
            case 67530:
            case 67537:
            case 68320:
            case 68592:
            case 68593:
            case 68594:
            case 69568:
            case 69569:
            case 69925:
            case 69926:
            {
                sp->custom_proc_interval = 3000;  //3 seconds
            } break;

            default:
                break;
        }

        // Set default mechanics if we don't already have one
        if (!sp->MechanicsType)
        {
            //Set Silencing spells mechanic.
            if (sp->EffectApplyAuraName[0] == SPELL_AURA_MOD_SILENCE ||
                sp->EffectApplyAuraName[1] == SPELL_AURA_MOD_SILENCE ||
                sp->EffectApplyAuraName[2] == SPELL_AURA_MOD_SILENCE)
                sp->MechanicsType = MECHANIC_SILENCED;

            //Set Stunning spells mechanic.
            if (sp->EffectApplyAuraName[0] == SPELL_AURA_MOD_STUN ||
                sp->EffectApplyAuraName[1] == SPELL_AURA_MOD_STUN ||
                sp->EffectApplyAuraName[2] == SPELL_AURA_MOD_STUN)
                sp->MechanicsType = MECHANIC_STUNNED;

            //Set Fearing spells mechanic
            if (sp->EffectApplyAuraName[0] == SPELL_AURA_MOD_FEAR ||
                sp->EffectApplyAuraName[1] == SPELL_AURA_MOD_FEAR ||
                sp->EffectApplyAuraName[2] == SPELL_AURA_MOD_FEAR)
                sp->MechanicsType = MECHANIC_FLEEING;

            //Set Interrupted spells mech
            if (sp->Effect[0] == SPELL_EFFECT_INTERRUPT_CAST ||
                sp->Effect[1] == SPELL_EFFECT_INTERRUPT_CAST ||
                sp->Effect[2] == SPELL_EFFECT_INTERRUPT_CAST)
                sp->MechanicsType = MECHANIC_INTERRUPTED;
        }

        if (sp->custom_proc_interval != 0)
            sp->procFlags |= PROC_REMOVEONUSE;

        //shaman - shock, has no spellgroup.very dangerous move !

        //mage - fireball. Only some of the spell has the flags

        switch (sp->Id)
        {
            // SPELL_HASH_SEAL_OF_COMMAND
            case 20375:     // Seal of Command - Proc Chance
            case 20424:
            case 29385:
            case 33127:
            case 41469:
            case 42058:
            case 57769:
            case 57770:
            case 66004:
            case 68020:
            case 68021:
            case 68022:
            case 69403:
            {
                sp->procChance = 25;
                sp->School = SCHOOL_HOLY; //the procspells of the original seal of command have physical school instead of holy
                sp->Spell_Dmg_Type = SPELL_DMG_TYPE_MAGIC; //heh, crazy spell uses melee/ranged/magic dmg type for 1 spell. Now which one is correct ?
            } break;

            // SPELL_HASH_JUDGEMENT_OF_COMMAND
            case 20425:
            case 20467:
            case 29386:
            case 32778:
            case 33554:
            case 41368:
            case 41470:
            case 66005:
            case 68017:
            case 68018:
            case 68019:
            case 71551:
            {
                sp->Spell_Dmg_Type = SPELL_DMG_TYPE_MAGIC;
            } break;


            // SPELL_HASH_BLESSING_OF_PROTECTION
            case 41450:
            // SPELL_HASH_DIVINE_PROTECTION
            case 498:
            case 13007:
            case 27778:
            case 27779:
            // SPELL_HASH_DIVINE_SHIELD
            case 642:
            case 13874:
            case 29382:
            case 33581:
            case 40733:
            case 41367:
            case 54322:
            case 63148:
            case 66010:
            case 67251:
            case 71550:
            {
                sp->MechanicsType = MECHANIC_INVULNARABLE;
            } break;
            // SPELL_HASH_SHRED
            case 3252:
            case 5221:      // Shred Rank 1
            case 6800:      // Shred Rank 2
            case 8992:      // Shred Rank 3
            case 9829:      // Shred Rank 4
            case 9830:      // Shred Rank 5
            case 27001:     // Shred Rank 6
            case 27002:     // Shred Rank 7
            case 27555:
            case 48571:     // Shred Rank 8
            case 48572:     // Shred Rank 9
            case 49121:
            case 49165:
            case 61548:
            case 61549:
            // SPELL_HASH_BACKSTAB
            case 53:        // Backstab Rank 1
            case 2589:      // Backstab Rank 2
            case 2590:      // Backstab Rank 3
            case 2591:      // Backstab Rank 4
            case 7159:
            case 8721:      // Backstab Rank 5
            case 11279:     // Backstab Rank 6
            case 11280:     // Backstab Rank 7
            case 11281:     // Backstab Rank 8
            case 15582:
            case 15657:
            case 22416:
            case 25300:     // Backstab Rank 9
            case 26863:     // Backstab Rank 10
            case 30992:
            case 34614:
            case 37685:
            case 48656:     // Backstab Rank 11
            case 48657:     // Backstab Rank 12
            case 52540:
            case 58471:
            case 63754:
            case 71410:
            case 72427:
            // SPELL_HASH_AMBUSH
            case 8676:      // Ambush Rank 1
            case 8724:      // Ambush Rank 2
            case 8725:      // Ambush Rank 3
            case 11267:     // Ambush Rank 4
            case 11268:     // Ambush Rank 5
            case 11269:     // Ambush Rank 6
            case 24337:
            case 27441:     // Ambush Rank 7
            case 39668:
            case 39669:
            case 41390:
            case 48689:     // Ambush Rank 8
            case 48690:     // Ambush Rank 9
            case 48691:     // Ambush Rank 10
            case 56239:
            // SPELL_HASH_GARROTE
            case 703:       // Garrote Rank 1
            case 8631:      // Garrote Rank 2
            case 8632:      // Garrote Rank 3
            case 8633:      // Garrote Rank 4
            case 8818:      // Garrote Rank 4
            case 11289:     // Garrote Rank 5
            case 11290:     // Garrote Rank 6
            case 26839:     // Garrote Rank 7
            case 26884:     // Garrote Rank 8
            case 37066:
            case 48675:     // Garrote Rank 9
            case 48676:     // Garrote Rank 10
            // SPELL_HASH_RAVAGE
            case 3242:
            case 3446:
            case 6785:      // Ravage Rank 1
            case 6787:      // Ravage Rank 2
            case 8391:
            case 9866:      // Ravage Rank 3
            case 9867:      // Ravage Rank 4
            case 24213:
            case 24333:
            case 27005:     // Ravage Rank 5
            case 29906:
            case 33781:
            case 48578:     // Ravage Rank 6
            case 48579:     // Ravage Rank 7
            case 50518:     // Ravage Rank 1
            case 53558:     // Ravage Rank 2
            case 53559:     // Ravage Rank 3
            case 53560:     // Ravage Rank 4
            case 53561:     // Ravage Rank 5
            case 53562:     // Ravage Rank 6
            {
                // FIX ME: needs different flag check
                sp->FacingCasterFlags = SPELL_INFRONT_STATUS_REQUIRE_INBACK;
            } break;
        }
    }

    /////////////////////////////////////////////////////////////////
    //SPELL COEFFICIENT SETTINGS START
    //////////////////////////////////////////////////////////////////

    for (uint32 x = 0; x < cnt; x++)
    {
        // get spellentry
        sp = dbcSpell.LookupRow(x);

        //Setting Cast Time Coefficient
        auto spell_cast_time = sSpellCastTimesStore.LookupEntry(sp->CastingTimeIndex);
        float castaff = float(GetCastTime(spell_cast_time));
        if (castaff < 1500)
            castaff = 1500;
        else if (castaff > 7000)
            castaff = 7000;

        sp->casttime_coef = castaff / 3500;

        SpellEntry* spz;
        bool spcheck = false;

        //Flag for DoT and HoT
        for (uint8 i = 0; i < 3; i++)
        {
            if (sp->EffectApplyAuraName[i] == SPELL_AURA_PERIODIC_DAMAGE ||
                sp->EffectApplyAuraName[i] == SPELL_AURA_PERIODIC_HEAL ||
                sp->EffectApplyAuraName[i] == SPELL_AURA_PERIODIC_LEECH)
            {
                sp->custom_spell_coef_flags |= SPELL_FLAG_IS_DOT_OR_HOT_SPELL;
                break;
            }
        }

        //Flag for DD or DH
        for (uint8 i = 0; i < 3; i++)
        {
            if (sp->EffectApplyAuraName[i] == SPELL_AURA_PERIODIC_TRIGGER_SPELL && sp->EffectTriggerSpell[i])
            {
                spz = dbcSpell.LookupEntryForced(sp->EffectTriggerSpell[i]);
                if (spz && (spz->Effect[i] == SPELL_EFFECT_SCHOOL_DAMAGE || spz->Effect[i] == SPELL_EFFECT_HEAL))
                    spcheck = true;
            }
            if (sp->Effect[i] == SPELL_EFFECT_SCHOOL_DAMAGE || sp->Effect[i] == SPELL_EFFECT_HEAL || spcheck)
            {
                sp->custom_spell_coef_flags |= SPELL_FLAG_IS_DD_OR_DH_SPELL;
                break;
            }
        }

        for (uint8 i = 0; i < 3; i++)
        {
            switch (sp->EffectImplicitTargetA[i])
            {
                //AoE
                case EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS:
                case EFF_TARGET_ALL_ENEMY_IN_AREA:
                case EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT:
                case EFF_TARGET_ALL_PARTY_AROUND_CASTER:
                case EFF_TARGET_ALL_ENEMIES_AROUND_CASTER:
                case EFF_TARGET_IN_FRONT_OF_CASTER:
                case EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
                case EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED:
                case EFF_TARGET_ALL_FRIENDLY_IN_AREA:
                case EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME:
                case EFF_TARGET_ALL_PARTY:
                case EFF_TARGET_LOCATION_INFRONT_CASTER:
                case EFF_TARGET_BEHIND_TARGET_LOCATION:
                case EFF_TARGET_LOCATION_INFRONT_CASTER_AT_RANGE:
                {
                    sp->custom_spell_coef_flags |= SPELL_FLAG_AOE_SPELL;
                    break;
                }
            }
        }

        for (uint8 i = 0; i < 3; i++)
        {
            switch (sp->EffectImplicitTargetB[i])
            {
                //AoE
                case EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS:
                case EFF_TARGET_ALL_ENEMY_IN_AREA:
                case EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT:
                case EFF_TARGET_ALL_PARTY_AROUND_CASTER:
                case EFF_TARGET_ALL_ENEMIES_AROUND_CASTER:
                case EFF_TARGET_IN_FRONT_OF_CASTER:
                case EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
                case EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED:
                case EFF_TARGET_ALL_FRIENDLY_IN_AREA:
                case EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME:
                case EFF_TARGET_ALL_PARTY:
                case EFF_TARGET_LOCATION_INFRONT_CASTER:
                case EFF_TARGET_BEHIND_TARGET_LOCATION:
                case EFF_TARGET_LOCATION_INFRONT_CASTER_AT_RANGE:
                {
                    sp->custom_spell_coef_flags |= SPELL_FLAG_AOE_SPELL;
                    break;
                }
            }
        }

        //Special Cases
        //Holy Light & Flash of Light
        switch (sp->Id)
        {
            // SPELL_HASH_HOLY_LIGHT
            case 635:
            case 639:
            case 647:
            case 1026:
            case 1042:
            case 3472:
            case 10328:
            case 10329:
            case 13952:
            case 15493:
            case 25263:
            case 25292:
            case 27135:
            case 27136:
            case 29383:
            case 29427:
            case 29562:
            case 31713:
            case 32769:
            case 37979:
            case 43451:
            case 44479:
            case 46029:
            case 48781:
            case 48782:
            case 52444:
            case 56539:
            case 58053:
            case 66112:
            case 68011:
            case 68012:
            case 68013:
            // SPELL_HASH_FLASH_OF_LIGHT
            case 19750:
            case 19939:
            case 19940:
            case 19941:
            case 19942:
            case 19943:
            case 25514:
            case 27137:
            case 33641:
            case 37249:
            case 37254:
            case 37257:
            case 48784:
            case 48785:
            case 57766:
            case 59997:
            case 66113:
            case 66922:
            case 68008:
            case 68009:
            case 68010:
            case 71930:
            {
                sp->custom_spell_coef_flags |= SPELL_FLAG_IS_DD_OR_DH_SPELL;
            } break;
            default:
                break;
        }

        //Additional Effect (not healing or damaging)
        for (uint8 i = 0; i < 3; i++)
        {
            if (sp->Effect[i] == SPELL_EFFECT_NULL)
                continue;

            switch (sp->Effect[i])
            {
                case SPELL_EFFECT_SCHOOL_DAMAGE:
                case SPELL_EFFECT_ENVIRONMENTAL_DAMAGE:
                case SPELL_EFFECT_HEALTH_LEECH:
                case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
                case SPELL_EFFECT_ADD_EXTRA_ATTACKS:
                case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
                case SPELL_EFFECT_POWER_BURN:
                case SPELL_EFFECT_ATTACK:
                case SPELL_EFFECT_HEAL:
                case SPELL_EFFECT_HEAL_MAX_HEALTH:
                case SPELL_EFFECT_DUMMY:
                    continue;
            }

            switch (sp->EffectApplyAuraName[i])
            {
                case SPELL_AURA_PERIODIC_DAMAGE:
                case SPELL_AURA_PROC_TRIGGER_DAMAGE:
                case SPELL_AURA_PERIODIC_DAMAGE_PERCENT:
                case SPELL_AURA_POWER_BURN:
                case SPELL_AURA_PERIODIC_HEAL:
                case SPELL_AURA_MOD_INCREASE_HEALTH:
                case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
                case SPELL_AURA_DUMMY:
                    continue;
            }

            sp->custom_spell_coef_flags |= SPELL_FLAG_ADITIONAL_EFFECT;
            break;

        }

        //Calculating fixed coeficients
        //Channeled spells
        if (sp->ChannelInterruptFlags != 0)
        {
            float Duration = float(GetDuration(sSpellDurationStore.LookupEntry(sp->DurationIndex)));
            if (Duration < 1500)
                Duration = 1500;
            else if (Duration > 7000)
                Duration = 7000;

            sp->fixed_hotdotcoef = (Duration / 3500.0f);

            if (sp->custom_spell_coef_flags & SPELL_FLAG_ADITIONAL_EFFECT)
                sp->fixed_hotdotcoef *= 0.95f;
            if (sp->custom_spell_coef_flags & SPELL_FLAG_AOE_SPELL)
                sp->fixed_hotdotcoef *= 0.5f;
        }

        //Standard spells
        else if ((sp->custom_spell_coef_flags & SPELL_FLAG_IS_DD_OR_DH_SPELL) && !(sp->custom_spell_coef_flags & SPELL_FLAG_IS_DOT_OR_HOT_SPELL))
        {
            sp->fixed_dddhcoef = sp->casttime_coef;
            if (sp->custom_spell_coef_flags & SPELL_FLAG_ADITIONAL_EFFECT)
                sp->fixed_dddhcoef *= 0.95f;
            if (sp->custom_spell_coef_flags & SPELL_FLAG_AOE_SPELL)
                sp->fixed_dddhcoef *= 0.5f;
        }

        //Over-time spells
        else if (!(sp->custom_spell_coef_flags & SPELL_FLAG_IS_DD_OR_DH_SPELL) && (sp->custom_spell_coef_flags & SPELL_FLAG_IS_DOT_OR_HOT_SPELL))
        {
            float Duration = float(GetDuration(sSpellDurationStore.LookupEntry(sp->DurationIndex)));
            sp->fixed_hotdotcoef = (Duration / 15000.0f);

            if (sp->custom_spell_coef_flags & SPELL_FLAG_ADITIONAL_EFFECT)
                sp->fixed_hotdotcoef *= 0.95f;
            if (sp->custom_spell_coef_flags & SPELL_FLAG_AOE_SPELL)
                sp->fixed_hotdotcoef *= 0.5f;

        }

        //Combined standard and over-time spells
        else if (sp->custom_spell_coef_flags & SPELL_FLAG_IS_DD_DH_DOT_SPELL)
        {
            float Duration = float(GetDuration(sSpellDurationStore.LookupEntry(sp->DurationIndex)));
            float Portion_to_Over_Time = (Duration / 15000.0f) / ((Duration / 15000.0f) + sp->casttime_coef);
            float Portion_to_Standard = 1.0f - Portion_to_Over_Time;

            sp->fixed_dddhcoef = sp->casttime_coef * Portion_to_Standard;
            sp->fixed_hotdotcoef = (Duration / 15000.0f) * Portion_to_Over_Time;

            if (sp->custom_spell_coef_flags & SPELL_FLAG_ADITIONAL_EFFECT)
            {
                sp->fixed_dddhcoef *= 0.95f;
                sp->fixed_hotdotcoef *= 0.95f;
            }
            if (sp->custom_spell_coef_flags & SPELL_FLAG_AOE_SPELL)
            {
                sp->fixed_dddhcoef *= 0.5f;
                sp->fixed_hotdotcoef *= 0.5f;
            }
        }

        // DankoDJ: This switch replaces the old NameHash overwrites
        switch (sp->Id)
        {
            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_SHIELD_OF_RIGHTEOUSNESS
            case 53600:     // Shield of Righteousness Rank 1
            case 61411:     // Shield of Righteousness Rank 2
            {
                sp->School = SCHOOL_HOLY;
                sp->Effect[0] = SPELL_EFFECT_DUMMY;
                sp->Effect[1] = SPELL_EFFECT_NULL;          //hacks, handling it in Spell::SpellEffectSchoolDMG(uint32 i)
                sp->Effect[2] = SPELL_EFFECT_SCHOOL_DAMAGE; //hack
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_CONSECRATION
            case 20116:     // Consecration Rank 2
            case 20922:     // Consecration Rank 3
            case 20923:     // Consecration Rank 4
            case 20924:     // Consecration Rank 5
            case 26573:     // Consecration Rank 1
            case 27173:     // Consecration Rank 6
            case 32773:
            case 33559:
            case 36946:
            case 37553:
            case 38385:
            case 41541:
            case 43429:
            case 48818:     // Consecration Rank 7
            case 48819:     // Consecration Rank 8
            case 57798:
            case 59998:
            case 69930:
            case 71122:
            {
                sp->School = SCHOOL_HOLY; //Consecration is a holy redirected spell.
                sp->Spell_Dmg_Type = SPELL_DMG_TYPE_MAGIC; //Speaks for itself.
            } break;
            
            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_SEALS_OF_THE_PURE
            case 20224:     // Seals of the Pure Rank 1
            case 20225:     // Seals of the Pure Rank 2
            case 20330:     // Seals of the Pure Rank 3
            case 20331:     // Seals of the Pure Rank 4
            case 20332:     // Seals of the Pure Rank 5
            {
                sp->EffectSpellClassMask[0][0] = 0x08000400;
                sp->EffectSpellClassMask[0][1] = 0x20000000;
                sp->EffectSpellClassMask[1][1] = 0x800;
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_MEND_PET
            case 136:       // Mend Pet Rank 1
            case 3111:      // Mend Pet Rank 2
            case 3661:      // Mend Pet Rank 3
            case 3662:      // Mend Pet Rank 4
            case 13542:     // Mend Pet Rank 5
            case 13543:     // Mend Pet Rank 6
            case 13544:     // Mend Pet Rank 7
            case 27046:     // Mend Pet Rank 8
            case 33976:     // Mend Pet
            case 48989:     // Mend Pet Rank 9
            case 48990:     // Mend Pet Rank 10
            {
                sp->ChannelInterruptFlags = 0;
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_BORROWED_TIME
            case 52795:     // Borrowed Time Rank 1
            case 52797:     // Borrowed Time Rank 2
            case 52798:     // Borrowed Time Rank 3
            case 52799:     // Borrowed Time Rank 4
            case 52800:     // Borrowed Time Rank 5
            case 59887:     // Borrowed Time
            case 59888:     // Borrowed Time
            case 59889:     // Borrowed Time
            case 59890:     // Borrowed Time
            case 59891:     // Borrowed Time
            {
                sp->procFlags = PROC_ON_CAST_SPELL;
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_GRACE
            case 47516:    // Grace Rank 1
            case 47517:    // Grace Rank 2
            {
                sp->procFlags = PROC_ON_CAST_SPELL;
            } break;
            case 47930:     // Grace
            {
                sp->rangeIndex = 4;
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_FLAMETONGUE_ATTACK
            case 10444:     // Flametongue Attack
            case 65978:
            case 68109:
            case 68110:
            case 68111:
            {
                //sp->Effect[1] = SPELL_EFFECT_DUMMY;
                sp->AttributesExC |= FLAGS4_NO_DONE_BONUS;
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_FLAMETONGUE_TOTEM
            case 8227:      // Flametongue Totem
            case 8249:
            case 10526:
            case 16387:
            case 25557:
            case 52109:
            case 52110:
            case 52111:
            case 52112:
            case 52113:
            case 58649:
            case 58651:
            case 58652:
            case 58654:
            case 58655:
            case 58656:
            {
                // Flametongue Totem passive target fix
                sp->EffectImplicitTargetA[0] = EFF_TARGET_SELF;
                sp->EffectImplicitTargetB[0] = 0;
                sp->EffectImplicitTargetA[1] = EFF_TARGET_SELF;
                sp->EffectImplicitTargetB[1] = 0;
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_FROSTBRAND_ATTACK
            case 8034:      // Frostbrand Attack
            case 8037:
            case 10458:
            case 16352:
            case 16353:
            case 25501:
            case 38617:
            case 54609:
            case 58797:
            case 58798:
            case 58799:
            case 64186:
            {
                // Frostbrand Weapon - 10% spd coefficient
                sp->fixed_dddhcoef = 0.1f;
                // Attributes addition
                sp->AttributesExC |= FLAGS4_NO_DONE_BONUS;
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_FIRE_NOVA
            case 8349:      // Fire Nova
            case 8502:
            case 8503:
            case 11306:
            case 11307:
            case 11969:
            case 11970:
            case 12470:
            case 16079:
            case 16635:
            case 17366:
            case 18432:
            case 20203:
            case 20602:
            case 23462:
            case 25535:
            case 25537:
            case 26073:
            case 30941:
            case 32167:
            case 33132:
            case 33775:
            case 37371:
            case 38728:
            case 43464:
            case 46551:
            case 61163:
            case 61650:
            case 61654:
            case 61655:
            case 68969:
            case 69667:
            case 78723:
            case 78724:
            {
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_FIRE_NOVA (ranks)
            case 1535:      //Fire Nova Rank 1
            {
                sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
                sp->EffectTriggerSpell[1] = 8349;
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 8498:      //Fire Nova Rank 2
            {
                sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
                sp->EffectTriggerSpell[1] = 8502;
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 8499:      //Fire Nova Rank 3
            {
                sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
                sp->EffectTriggerSpell[1] = 8503;
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 11314:     //Fire Nova Rank 4
            {
                sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
                sp->EffectTriggerSpell[1] = 11306;
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 11315:     //Fire Nova Rank 5
            {
                sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
                sp->EffectTriggerSpell[1] = 11307;
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 25546:     //Fire Nova Rank 6
            {
                sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
                sp->EffectTriggerSpell[1] = 25535;
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 25547:     //Fire Nova Rank 7
            {
                sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
                sp->EffectTriggerSpell[1] = 25537;
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 61649:     //Fire Nova Rank 8
            {
                sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
                sp->EffectTriggerSpell[1] = 61650;
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 61657:     //Fire Nova Rank 9
            {
                sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
                sp->EffectTriggerSpell[1] = 61654;
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_ATTACK
            case 3606:      // Attack
            case 6350:
            case 6351:
            case 6352:
            case 7389:
            case 10435:
            case 10436:
            case 15037:
            case 22048:
            case 25530:
            case 31992:
            case 32969:
            case 38296:
            case 38584:
            case 39592:
            case 39593:
            case 58700:
            case 58701:
            case 58702:
            case 65998:
            case 68106:
            case 68107:
            case 68108:
            case 68866:
            case 74413:
            case 75100:
            {
                // Searing Totem - 8% spd coefficient
                sp->fixed_dddhcoef = 0.08f;
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_HEALING_STREAM
            case 5672:      // Healing Stream
            case 6371:      // Healing Stream
            case 6372:      // Healing Stream
            case 10460:     // Healing Stream
            case 10461:     // Healing Stream
            case 25566:     // Healing Stream
            case 58763:     // Healing Stream
            case 58764:     // Healing Stream
            case 58765:     // Healing Stream
            case 65994:     // Healing Stream
            case 68882:     // Healing Stream
            {
                // 8% healing coefficient
                sp->OTspell_coef_override = 0.08f;
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_NATURE_S_GUARDIAN
            case 30881:     // Nature's Guardian
            case 30883:     // Nature's Guardian
            case 30884:     // Nature's Guardian
            case 30885:     // Nature's Guardian
            case 30886:     // Nature's Guardian
            case 31616:     // Nature's Guardian
            {
                sp->procFlags = PROC_ON_SPELL_HIT_VICTIM | PROC_ON_MELEE_ATTACK_VICTIM | PROC_ON_RANGED_ATTACK_VICTIM | PROC_ON_ANY_DAMAGE_VICTIM;
                sp->custom_proc_interval = 5000;
                sp->EffectTriggerSpell[0] = 31616;      // DankoDJ: sp->EffectTriggerSpell[0] is alread 18350 in spell.dbc !?
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_HEX
            case 11641:     // Hex
            case 16097:     // Hex
            case 16707:     // Hex
            case 16708:     // Hex
            case 16709:     // Hex
            case 17172:     // Hex
            case 18503:     // Hex
            case 22566:     // Hex
            case 24053:     // Hex
            case 29044:     // Hex
            case 36700:     // Hex
            case 40400:     // Hex
            case 46295:     // Hex
            case 51514:     // Hex
            case 53439:     // Hex
            case 66054:     // Hex
            {
                sp->AuraInterruptFlags |= AURA_INTERRUPT_ON_UNUSED2;
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_DASH
            case 1850:      // Dash
            case 9821:      // Dash
            case 33357:     // Dash
            case 36589:     // Dash
            case 43317:     // Dash
            case 44029:     // Dash
            case 44531:     // Dash
            case 61684:     // Dash
            {
                // mask for FORM_CAT(1) = 1 << (1 - 1), which is 1
                sp->RequiredShapeShift = 1;
            } break;
            default:
                break;
        }
    }
    // END OF LOOP

    //Settings for special cases
    QueryResult* resultx = WorldDatabase.Query("SELECT * FROM spell_coef_override");
    if (resultx != NULL)
    {
        do
        {
            Field* f;
            f = resultx->Fetch();
            sp = dbcSpell.LookupEntryForced(f[0].GetUInt32());
            if (sp != NULL)
            {
                sp->Dspell_coef_override = f[2].GetFloat();
                sp->OTspell_coef_override = f[3].GetFloat();
            }
            else
                Log.Error("SpellCoefOverride", "Has nonexistent spell %u.", f[0].GetUInt32());
        }
        while (resultx->NextRow());
        delete resultx;
    }

    //Fully loaded coefficients, we must share channeled coefficient to its triggered spells
    for (uint32 x = 0; x < cnt; x++)
    {
        // get spellentry
        sp = dbcSpell.LookupRow(x);
        SpellEntry* spz;

        //Case SPELL_AURA_PERIODIC_TRIGGER_SPELL
        for (uint8 i = 0; i < 3; i++)
        {
            if (sp->EffectApplyAuraName[i] == SPELL_AURA_PERIODIC_TRIGGER_SPELL)
            {
                spz = CheckAndReturnSpellEntry(sp->EffectTriggerSpell[i]);
                if (spz != NULL)
                {
                    if (sp->Dspell_coef_override >= 0)
                        spz->Dspell_coef_override = sp->Dspell_coef_override;
                    else
                    {
                        //we must set bonus per tick on triggered spells now (i.e. Arcane Missiles)
                        if (sp->ChannelInterruptFlags != 0)
                        {
                            float Duration = float(GetDuration(sSpellDurationStore.LookupEntry(sp->DurationIndex)));
                            float amp = float(sp->EffectAmplitude[i]);
                            sp->fixed_dddhcoef = sp->fixed_hotdotcoef * amp / Duration;
                        }
                        spz->fixed_dddhcoef = sp->fixed_dddhcoef;
                    }

                    if (sp->OTspell_coef_override >= 0)
                        spz->OTspell_coef_override = sp->OTspell_coef_override;
                    else
                    {
                        //we must set bonus per tick on triggered spells now (i.e. Arcane Missiles)
                        if (sp->ChannelInterruptFlags != 0)
                        {
                            float Duration = float(GetDuration(sSpellDurationStore.LookupEntry(sp->DurationIndex)));
                            float amp = float(sp->EffectAmplitude[i]);
                            sp->fixed_hotdotcoef *= amp / Duration;
                        }
                        spz->fixed_hotdotcoef = sp->fixed_hotdotcoef;
                    }
                    break;
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////
    //SPELL COEFFICIENT SETTINGS END
    /////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    // thrown - add a 1.6 second cooldown
    const static uint32 thrown_spells[] = { SPELL_RANGED_GENERAL, SPELL_RANGED_THROW, SPELL_RANGED_WAND, 26679, 29436, 37074, 41182, 41346, 0 };
    for (uint32 i = 0; thrown_spells[i] != 0; ++i)
    {
        sp = CheckAndReturnSpellEntry(thrown_spells[i]);
        if (sp != NULL && sp->RecoveryTime == 0 && sp->StartRecoveryTime == 0)
        {
            if (sp->Id == SPELL_RANGED_GENERAL)
                sp->RecoveryTime = 500;    // cebernic: hunter general with 0.5s
            else
                sp->RecoveryTime = 1500; // 1.5cd
        }
    }

    ////////////////////////////////////////////////////////////
    // Wands
    sp = CheckAndReturnSpellEntry(SPELL_RANGED_WAND);
    if (sp != NULL)
        sp->Spell_Dmg_Type = SPELL_DMG_TYPE_RANGED;

    
    //////////////////////////////////////////////////////
    // CLASS-SPECIFIC SPELL FIXES                        //
    //////////////////////////////////////////////////////

    // Note: when applying spell hackfixes, please follow a template

    //////////////////////////////////////////
    // WARRIOR                                //
    //////////////////////////////////////////

    // Insert warrior spell fixes here
    //Warrior: Death Wish
    sp = dbcSpell.LookupEntryForced(12292);
    if (sp != NULL)
    {
        sp->procChance = 100;
    }
    ////////////////////////////////////////////////////////////
    // Arms

    // Juggernaut
    sp = CheckAndReturnSpellEntry(65156);
    if (sp != NULL)
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;

    // Taste for Blood Rank 1
    sp = CheckAndReturnSpellEntry(56636);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL | PROC_ON_ANY_HOSTILE_ACTION;
        sp->custom_proc_interval = 6000;
    }

    // Taste for Blood Rank 2
    sp = CheckAndReturnSpellEntry(56637);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL | PROC_ON_ANY_HOSTILE_ACTION;
        sp->custom_proc_interval = 6000;
    }

    // Taste for Blood Rank 3
    sp = CheckAndReturnSpellEntry(56638);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL | PROC_ON_ANY_HOSTILE_ACTION;
        sp->custom_proc_interval = 6000;
    }

    // Wrecking Crew Rank 1
    sp = CheckAndReturnSpellEntry(46867);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }

    // Wrecking Crew Rank 2
    sp = CheckAndReturnSpellEntry(56611);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }

    // Wrecking Crew Rank 3
    sp = CheckAndReturnSpellEntry(56612);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }

    // Wrecking Crew Rank 4
    sp = CheckAndReturnSpellEntry(56613);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }

    // Wrecking Crew Rank 5
    sp = CheckAndReturnSpellEntry(56614);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }

    // Warrior - Deep Wounds
    sp = CheckAndReturnSpellEntry(12834);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[0] = 12721;          // DankoDJ: sp->EffectTriggerSpell[0] is alread 12162 in spell.dbc !?
        sp->procFlags |= PROC_ON_SPELL_CRIT_HIT;
    }
    sp = CheckAndReturnSpellEntry(12849);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[0] = 12721;          // DankoDJ: sp->EffectTriggerSpell[0] is alread 12850 in spell.dbc !?
        sp->procFlags |= PROC_ON_SPELL_CRIT_HIT;
    }
    sp = CheckAndReturnSpellEntry(12867);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[0] = 12721;          // DankoDJ: sp->EffectTriggerSpell[0] is alread 12868 in spell.dbc !?
        sp->procFlags |= PROC_ON_SPELL_CRIT_HIT;
    }

    // Warrior - Improved Hamstring Rank 1
    sp = CheckAndReturnSpellEntry(12289);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 23694;          // DankoDJ: sp->EffectTriggerSpell[0] is alread 23694 in spell.dbc !?
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
    }
    // Warrior - Improved Hamstring Rank 2
    sp = CheckAndReturnSpellEntry(12668);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 23694;          // DankoDJ: sp->EffectTriggerSpell[0] is alread 23694 in spell.dbc !?
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
    }
    // Warrior - Improved Hamstring Rank 3
    sp = CheckAndReturnSpellEntry(23695);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 23694;          // DankoDJ: sp->EffectTriggerSpell[0] is alread 23694 in spell.dbc !?
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
    }

    // Warrior - Retaliation
    sp = CheckAndReturnSpellEntry(20230);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 22858; //evil , but this is good for us :D
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM; //add procflag here since this was not processed with the others !
    }

    // Warrior - Second Wind should trigger on self
    sp = CheckAndReturnSpellEntry(29841);
    if (sp != NULL)
        sp->procFlags |= PROC_TARGET_SELF;

    sp = CheckAndReturnSpellEntry(29842);
    if (sp != NULL)
        sp->procFlags |= PROC_TARGET_SELF;

    // Warrior - Sudden Death Rank 1
    sp = CheckAndReturnSpellEntry(29723);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }

    // Warrior - Sudden Death Rank 2
    sp = CheckAndReturnSpellEntry(29725);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }

    // Warrior - Sudden Death Rank 3
    sp = CheckAndReturnSpellEntry(29724);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }

    // Warrior - Overpower Rank 1
    sp = CheckAndReturnSpellEntry(7384);
    if (sp != NULL)
        sp->Attributes |= ATTRIBUTES_CANT_BE_DPB;
    // Warrior - Overpower Rank 2
    sp = CheckAndReturnSpellEntry(7887);
    if (sp != NULL)
        sp->Attributes |= ATTRIBUTES_CANT_BE_DPB;
    // Warrior - Overpower Rank 3
    sp = CheckAndReturnSpellEntry(11584);
    if (sp != NULL)
        sp->Attributes |= ATTRIBUTES_CANT_BE_DPB;
    // Warrior - Overpower Rank 4
    sp = CheckAndReturnSpellEntry(11585);
    if (sp != NULL)
        sp->Attributes |= ATTRIBUTES_CANT_BE_DPB;

    // Warrior - Tactical Mastery Rank 1
    sp = CheckAndReturnSpellEntry(12295);
    if (sp != NULL)
        sp->RequiredShapeShift = 0x00070000;
    // Warrior - Tactical Mastery Rank 2
    sp = CheckAndReturnSpellEntry(12676);
    if (sp != NULL)
        sp->RequiredShapeShift = 0x00070000;
    // Warrior - Tactical Mastery Rank 3
    sp = CheckAndReturnSpellEntry(12677);
    if (sp != NULL)
        sp->RequiredShapeShift = 0x00070000;

    // Warrior - Heroic Throw
    sp = CheckAndReturnSpellEntry(57755);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_SCHOOL_DAMAGE;
    }

    // Warrior - Rend
    sp = CheckAndReturnSpellEntry(772);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(6546);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(6547);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(6548);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(11572);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(11573);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(11574);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(25208);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;

    ////////////////////////////////////////////////////////////
    // Fury

    // Warrior - Slam
    sp = CheckAndReturnSpellEntry(1464);
    if (sp != NULL)
        sp->Effect[0] = SPELL_EFFECT_SCHOOL_DAMAGE;

    sp = CheckAndReturnSpellEntry(8820);
    if (sp != NULL)
        sp->Effect[0] = SPELL_EFFECT_SCHOOL_DAMAGE;

    sp = CheckAndReturnSpellEntry(11604);
    if (sp != NULL)
        sp->Effect[0] = SPELL_EFFECT_SCHOOL_DAMAGE;

    sp = CheckAndReturnSpellEntry(11605);
    if (sp != NULL)
        sp->Effect[0] = SPELL_EFFECT_SCHOOL_DAMAGE;

    sp = CheckAndReturnSpellEntry(25241);
    if (sp != NULL)
        sp->Effect[0] = SPELL_EFFECT_SCHOOL_DAMAGE;

    sp = CheckAndReturnSpellEntry(25242);
    if (sp != NULL)
        sp->Effect[0] = SPELL_EFFECT_SCHOOL_DAMAGE;

    sp = CheckAndReturnSpellEntry(47474);
    if (sp != NULL)
        sp->Effect[0] = SPELL_EFFECT_SCHOOL_DAMAGE;

    sp = CheckAndReturnSpellEntry(47475);
    if (sp != NULL)
        sp->Effect[0] = SPELL_EFFECT_SCHOOL_DAMAGE;

    // Warrior - Bloodsurge
    sp = CheckAndReturnSpellEntry(46913);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_CAST_SPELL;  // DankoDJ: original just PROC_ON_CAST_SPELL
    }
    sp = CheckAndReturnSpellEntry(46914);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_CAST_SPELL;  // DankoDJ: original just PROC_ON_CAST_SPELL
    }
    sp = CheckAndReturnSpellEntry(46915);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_CAST_SPELL;  // DankoDJ: original just PROC_ON_CAST_SPELL
    }

    // Warrior - Furious Attacks
    sp = CheckAndReturnSpellEntry(46910);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
        sp->custom_proc_interval = 7000;
    }
    sp = CheckAndReturnSpellEntry(46911);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
        sp->custom_proc_interval = 5000;
    }

    // Warrior - Enrage Procflags
    sp = CheckAndReturnSpellEntry(12317);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_ON_RANGED_ATTACK_VICTIM | PROC_ON_SPELL_HIT_VICTIM;
    sp = CheckAndReturnSpellEntry(13045);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_ON_RANGED_ATTACK_VICTIM | PROC_ON_SPELL_HIT_VICTIM;
    sp = CheckAndReturnSpellEntry(13046);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_ON_RANGED_ATTACK_VICTIM | PROC_ON_SPELL_HIT_VICTIM;
    sp = CheckAndReturnSpellEntry(13047);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_ON_RANGED_ATTACK_VICTIM | PROC_ON_SPELL_HIT_VICTIM;
    sp = CheckAndReturnSpellEntry(13048);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_ON_RANGED_ATTACK_VICTIM | PROC_ON_SPELL_HIT_VICTIM;

    // Remove the charges only on melee attacks
    sp = CheckAndReturnSpellEntry(12880);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    sp = CheckAndReturnSpellEntry(14201);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    sp = CheckAndReturnSpellEntry(14202);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    sp = CheckAndReturnSpellEntry(14203);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    sp = CheckAndReturnSpellEntry(14204);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK;

    // Warrior - Blood Craze Procflags
    sp = CheckAndReturnSpellEntry(16487);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM;
    sp = CheckAndReturnSpellEntry(16489);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM;
    sp = CheckAndReturnSpellEntry(16492);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM;

    // Warrior - Bloodthirst new version is ok but old version is wrong from now on :(
    sp = CheckAndReturnSpellEntry(23881);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL; //cast on us, it is good
        sp->EffectTriggerSpell[1] = 23885; //evil , but this is good for us :D
    }
    sp = CheckAndReturnSpellEntry(23892);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 23886; //evil , but this is good for us :D  // DankoDJ: Is there a reason to trigger an non existing spell?
    }
    sp = CheckAndReturnSpellEntry(23893);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL; //
        sp->EffectTriggerSpell[1] = 23887; //evil , but this is good for us :D // DankoDJ: Is there a reason to trigger an non existing spell?
    }
    sp = CheckAndReturnSpellEntry(23894);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL; //
        sp->EffectTriggerSpell[1] = 23888; //evil , but this is good for us :D // DankoDJ: Is there a reason to trigger an non existing spell?
    }
    sp = CheckAndReturnSpellEntry(25251);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL; //aura
        sp->EffectTriggerSpell[1] = 25252; //evil , but this is good for us :D // DankoDJ: Is there a reason to trigger an non existing spell?
    }
    sp = CheckAndReturnSpellEntry(30335);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL; //aura
        sp->EffectTriggerSpell[1] = 30339; //evil , but this is good for us :D // DankoDJ: Is there a reason to trigger an non existing spell?
    }

    // Warrior - Berserker Rage
    sp = CheckAndReturnSpellEntry(18499);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;//Forcing a dummy aura, so we can add the missing 4th effect.
        sp->Effect[1] = SPELL_EFFECT_NULL;
        sp->Effect[2] = SPELL_EFFECT_NULL;
    }

    // Warrior - Improved Berserker Rage
    sp = CheckAndReturnSpellEntry(20500);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(20501);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL | static_cast<uint32>(PROC_TARGET_SELF);

    // Warrior - Unbridled Wrath
    sp = CheckAndReturnSpellEntry(12322);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(12999);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(13000);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(13001);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(13002);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);

    // Warrior - Heroic Fury
    sp = CheckAndReturnSpellEntry(60970);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_DUMMY;
    }

    ////////////////////////////////////////////////////////////
    // Protection

    // Sword and Board Rank 1
    sp = CheckAndReturnSpellEntry(46951);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    // Sword and Board Rank 2
    sp = CheckAndReturnSpellEntry(46952);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    // Sword and Board Rank 3
    sp = CheckAndReturnSpellEntry(46953);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    // Safeguard Rank 1
    sp = CheckAndReturnSpellEntry(46945);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    // Safeguard Rank 2
    sp = CheckAndReturnSpellEntry(46949);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    // Improved Defensive Stance Rank 1
    sp = CheckAndReturnSpellEntry(29593);
    if (sp != NULL)
        sp->procFlags = PROC_ON_DODGE_VICTIM | PROC_ON_BLOCK_VICTIM;

    // Improved Defensive Stance Rank 2
    sp = CheckAndReturnSpellEntry(29594);
    if (sp != NULL)
        sp->procFlags = PROC_ON_DODGE_VICTIM | PROC_ON_BLOCK_VICTIM;

    // Improved Revenge Rank 1
    sp = CheckAndReturnSpellEntry(12797);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 12798;
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
    }

    // Improved Revenge Rank 2
    sp = CheckAndReturnSpellEntry(12799);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 12798;
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
    }

    // Intervene  Ranger: stop attack
    sp = CheckAndReturnSpellEntry(3411);
    if (sp != NULL)
    {
        sp->Attributes |= ATTRIBUTES_STOP_ATTACK;
    }

    // Gag Order Rank 1
    sp = CheckAndReturnSpellEntry(12311);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 18498;      // DankoDJ: sp->EffectTriggerSpell[0] is alread 18498 in spell.dbc !?
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    // Gag Order Rank 2
    sp = CheckAndReturnSpellEntry(12958);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 18498;      // DankoDJ: sp->EffectTriggerSpell[0] is alread 18498 in spell.dbc !?
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    //////////////////////////////////////////
    // PALADIN                                //
    //////////////////////////////////////////

    // Insert paladin spell fixes here

    //Paladin - Judgement of Command
    sp = CheckAndReturnSpellEntry(20467);
    if (sp != NULL)
        sp->procChance = 50;

    //Paladin - Judgement of Corruption
    sp = CheckAndReturnSpellEntry(53733);
    if (sp != NULL)
        sp->procChance = 50;

    //Paladin - Judgement of Light
    sp = CheckAndReturnSpellEntry(20185);
    if (sp != NULL)
        sp->procChance = 50;

    //Paladin - Judgement of Justice
    sp = CheckAndReturnSpellEntry(20184);
    if (sp != NULL)
        sp->procChance = 50;

    //Paladin - Judgement of Righteousness
    sp = CheckAndReturnSpellEntry(20187);
    if (sp != NULL)
        sp->procChance = 50;

    //Paladin - Judgement of Vengeance
    sp = CheckAndReturnSpellEntry(31804);
    if (sp != NULL)
        sp->procChance = 50;

    //Paladin - Judgement of Wisdom
    sp = CheckAndReturnSpellEntry(20186);
    if (sp != NULL)
        sp->procChance = 50;

    //Paladin - Seal of Command
    sp = CheckAndReturnSpellEntry(20375);
    if (sp != NULL)
        sp->custom_proc_interval = 3000;

    //Paladin - Seal of Corruption
    sp = CheckAndReturnSpellEntry(53736);
    if (sp != NULL)
        sp->custom_proc_interval = 3000;

    //Paladin - Seal of Light
    sp = CheckAndReturnSpellEntry(20165);
    if (sp != NULL)
        sp->custom_proc_interval = 3000;

    //Paladin - Seal of Justice
    sp = CheckAndReturnSpellEntry(20164);
    if (sp != NULL)
        sp->custom_proc_interval = 3000;

    //Paladin - Seal of Righteousness
    sp = CheckAndReturnSpellEntry(21084);
    if (sp != NULL)
        sp->custom_proc_interval = 3000;

    //Paladin - Seal of Vengeance
    sp = CheckAndReturnSpellEntry(31801);
    if (sp != NULL)
        sp->custom_proc_interval = 3000;

    //Paladin - Seal of Wisdom
    sp = CheckAndReturnSpellEntry(20166);
    if (sp != NULL)
        sp->custom_proc_interval = 3000;

    sp = CheckAndReturnSpellEntry(20056);   //Rank 2
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT;
    }

    sp = CheckAndReturnSpellEntry(20057);   //Rank 3
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT;
    }

    //Paladin - Seal of Command - Holy damage, but melee mechanics (crit damage, chance, etc)
    sp = CheckAndReturnSpellEntry(20424);
    if (sp != NULL)
        sp->custom_is_melee_spell = true;

    //Paladin - Hammer of the Righteous
    sp = CheckAndReturnSpellEntry(53595);
    if (sp != NULL)
    {
        sp->speed = 0;    //without, no damage is done
    }

    sp = CheckAndReturnSpellEntry(38008);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 31893;
    }

    //Paladin - Seal of Martyr
    sp = CheckAndReturnSpellEntry(53720);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 53719;
        sp->School = SCHOOL_HOLY;
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }
    //Paladin - seal of blood
    sp = CheckAndReturnSpellEntry(31892);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 31893;
        sp->School = SCHOOL_HOLY;
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }
    sp = CheckAndReturnSpellEntry(53719);
    if (sp != NULL)
    {
        sp->School = SCHOOL_HOLY;
        sp->Spell_Dmg_Type = SPELL_DMG_TYPE_MAGIC;
    }
    sp = CheckAndReturnSpellEntry(31893);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_PHYSICAL_ATTACK;
        sp->School = SCHOOL_HOLY;
        sp->Spell_Dmg_Type = SPELL_DMG_TYPE_MAGIC;
    }

    //Paladin - Divine Storm
    sp = CheckAndReturnSpellEntry(53385);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectRadiusIndex[0] = 43; //16 yards
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 54172;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 100;
        sp->MaxTargets = 4;
    }

    //Paladin - Sacred Shield - bonus to flash is not working
    sp = CheckAndReturnSpellEntry(53601);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM | static_cast<uint32>(PROC_TARGET_SELF);
        sp->custom_proc_interval = 6000;
        sp->EffectTriggerSpell[0] = 58597;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
    }

    //Paladin - Vindication
    sp = CheckAndReturnSpellEntry(26016);
    if (sp != NULL)
    {
        sp->procChance = 30;
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }
    sp = CheckAndReturnSpellEntry(9452);
    if (sp != NULL)
    {
        sp->procChance = 30;
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }

    //Paladin - Reckoning
    sp = CheckAndReturnSpellEntry(20177);
    if (sp != NULL)
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM | static_cast<uint32>(PROC_TARGET_SELF);

    sp = CheckAndReturnSpellEntry(20179);
    if (sp != NULL)
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM | static_cast<uint32>(PROC_TARGET_SELF);

    sp = CheckAndReturnSpellEntry(20180);
    if (sp != NULL)
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM | static_cast<uint32>(PROC_TARGET_SELF);

    sp = CheckAndReturnSpellEntry(20181);
    if (sp != NULL)
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM | static_cast<uint32>(PROC_TARGET_SELF);

    sp = CheckAndReturnSpellEntry(20182);
    if (sp != NULL)
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM | static_cast<uint32>(PROC_TARGET_SELF);

    //Paladin - Reckoning Effect
    sp = CheckAndReturnSpellEntry(20178);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);
    }

    //Paladin - Judgements of the Wise
    sp = CheckAndReturnSpellEntry(31930);
    if (sp != NULL)
    {
        sp->SpellFamilyName = 0;
        sp->SpellGroupType[0] = 0;
        sp->SpellGroupType[1] = 0;
        sp->SpellGroupType[2] = 0;
    }

    sp = CheckAndReturnSpellEntry(54180);
    if (sp != NULL)
    {
        sp->SpellFamilyName = 0;
        sp->SpellGroupType[0] = 0;
        sp->SpellGroupType[1] = 0;
        sp->SpellGroupType[2] = 0;
        sp->custom_proc_interval = 4000;
    }

    sp = CheckAndReturnSpellEntry(31876);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 54180;
    }
    sp = CheckAndReturnSpellEntry(31877);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 54180;
    }
    sp = CheckAndReturnSpellEntry(31878);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 54180;
    }

    //Paladin - Blessed Life ranks 1-3
    sp = CheckAndReturnSpellEntry(31828);
    if (sp != NULL)
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;

    sp = CheckAndReturnSpellEntry(31829);
    if (sp != NULL)
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;

    sp = CheckAndReturnSpellEntry(31830);
    if (sp != NULL)
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;

    //Palarin - Light's Grace
    sp = CheckAndReturnSpellEntry(31833);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    sp = CheckAndReturnSpellEntry(31835);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    sp = CheckAndReturnSpellEntry(31836);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    //Paladin - Spiritual Attunement
    sp = CheckAndReturnSpellEntry(31785);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_SPELL_LAND_VICTIM | static_cast<uint32>(PROC_TARGET_SELF);
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 31786;
    }
    sp = CheckAndReturnSpellEntry(33776);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_SPELL_LAND_VICTIM | static_cast<uint32>(PROC_TARGET_SELF);
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 31786;
    }

    //Paladin - Improved Lay on Hands
    sp = CheckAndReturnSpellEntry(20234);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    sp = CheckAndReturnSpellEntry(20235);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    //Paladin - Avenging Wrath marker - Is forced debuff
    sp = CheckAndReturnSpellEntry(61987);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_IGNORE_INVULNERABILITY;
    }

    //Paladin - Forbearance - Is forced debuff
    sp = CheckAndReturnSpellEntry(25771);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_IGNORE_INVULNERABILITY;
    }

    //Divine Protection
    sp = CheckAndReturnSpellEntry(498);
    if (sp != NULL)
        sp->targetAuraSpellNot = 25771;

    //Divine Shield
    sp = CheckAndReturnSpellEntry(642);
    if (sp != NULL)
        sp->targetAuraSpellNot = 25771;

    //Hand of Protection Rank 1
    sp = CheckAndReturnSpellEntry(1022);
    if (sp != NULL)
        sp->targetAuraSpellNot = 25771;

    //Hand of Protection Rank 2
    sp = CheckAndReturnSpellEntry(5599);
    if (sp != NULL)
        sp->targetAuraSpellNot = 25771;

    //Hand of Protection Rank 3
    sp = CheckAndReturnSpellEntry(10278);
    if (sp != NULL)
        sp->targetAuraSpellNot = 25771;

    //Paladin - Infusion of Light
    sp = CheckAndReturnSpellEntry(53569);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }
    sp = CheckAndReturnSpellEntry(53576);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    //Paladin - Sacred Cleansing
    sp = CheckAndReturnSpellEntry(53551);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(53552);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(53553);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    //Paladin - Judgements of the Pure
    sp = CheckAndReturnSpellEntry(53671);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(53673);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(54151);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(54154);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(54155);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    //Paladin -  Heart of the Crusader
    sp = CheckAndReturnSpellEntry(20335);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 21183;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 100;
    }
    sp = CheckAndReturnSpellEntry(20336);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 54498;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 100;
    }
    sp = CheckAndReturnSpellEntry(20337);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 54499;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 100;
    }

    //Paladin - Art of War
    sp = CheckAndReturnSpellEntry(53486);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_DAMAGE_DONE;
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }
    sp = CheckAndReturnSpellEntry(53489);
    if (sp != NULL)
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;

    sp = CheckAndReturnSpellEntry(53488);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_DAMAGE_DONE;
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }
    sp = CheckAndReturnSpellEntry(59578);
    if (sp != NULL)
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;

    //Paladin - Hammer of Justice - Interrupt effect
    sp = CheckAndReturnSpellEntry(853);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 32747;
    }
    sp = CheckAndReturnSpellEntry(5588);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 32747;
    }
    sp = CheckAndReturnSpellEntry(5589);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 32747;
    }
    sp = CheckAndReturnSpellEntry(10308);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 32747;
    }

    // Paladin - Sheath of Light
    sp = CheckAndReturnSpellEntry(53501);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 54203;
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    sp = CheckAndReturnSpellEntry(53502);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 54203;
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    sp = CheckAndReturnSpellEntry(53503);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 54203;
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    //////////////////////////////////////////
    // HUNTER                                //
    //////////////////////////////////////////
    //Wild quiver rank 1
    sp = CheckAndReturnSpellEntry(53215);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_RANGED_ATTACK;
    }
    sp = CheckAndReturnSpellEntry(53216);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_RANGED_ATTACK;
    }
    sp = CheckAndReturnSpellEntry(53217);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_RANGED_ATTACK;
    }

    // Insert hunter spell fixes here

    // Pashtet: Lock'n'Load, only for traps. Need something more for Serpent Sting DoT.
    sp = CheckAndReturnSpellEntry(56342);
    if (sp != NULL)
    {
        sp->procFlags |= PROC_ON_TRAP_TRIGGER;
        sp->custom_proc_interval = 30000;
    }
    sp = CheckAndReturnSpellEntry(56343);
    if (sp != NULL)
    {
        sp->procFlags |= PROC_ON_TRAP_TRIGGER;
        sp->custom_proc_interval = 30000;
    }
    sp = CheckAndReturnSpellEntry(56344);
    if (sp != NULL)
    {
        sp->procFlags |= PROC_ON_TRAP_TRIGGER;
        sp->custom_proc_interval = 30000;
    }

    //Hunter - Bestial Wrath
    sp = CheckAndReturnSpellEntry(19574);
    if (sp != NULL)
        sp->EffectApplyAuraName[2] = SPELL_AURA_DUMMY;

    //Hunter - The Beast Within
    sp = CheckAndReturnSpellEntry(34692);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 100;
        sp->EffectTriggerSpell[0] = 34471;
    }
    sp = CheckAndReturnSpellEntry(34471);
    if (sp != NULL)
        sp->EffectApplyAuraName[2] = SPELL_AURA_DUMMY;

    //Hunter - Go for the Throat
    sp = CheckAndReturnSpellEntry(34950);
    if (sp != NULL)
        sp->procFlags = PROC_ON_RANGED_CRIT_ATTACK;
    sp = CheckAndReturnSpellEntry(34954);
    if (sp != NULL)
        sp->procFlags = PROC_ON_RANGED_CRIT_ATTACK;
    sp = CheckAndReturnSpellEntry(34952);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    sp = CheckAndReturnSpellEntry(34953);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;

    // Hunter - Master Tactician
    sp = CheckAndReturnSpellEntry(34506);
    if (sp != NULL)
        sp->procFlags = PROC_ON_RANGED_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(34507);
    if (sp != NULL)
        sp->procFlags = PROC_ON_RANGED_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(34508);
    if (sp != NULL)
        sp->procFlags = PROC_ON_RANGED_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(34838);
    if (sp != NULL)
        sp->procFlags = PROC_ON_RANGED_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(34839);
    if (sp != NULL)
        sp->procFlags = PROC_ON_RANGED_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);

    // Hunter - Spirit Bond
    sp = CheckAndReturnSpellEntry(19578);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 19579;
    }
    sp = CheckAndReturnSpellEntry(20895);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 24529;
    }
    sp = CheckAndReturnSpellEntry(19579);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA; //we should do the same for player too as we did for pet
        sp->EffectApplyAuraName[1] = sp->EffectApplyAuraName[0];
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
        sp->EffectBasePoints[1] = sp->EffectBasePoints[0];
        sp->EffectAmplitude[1] = sp->EffectAmplitude[0];
        sp->EffectDieSides[1] = sp->EffectDieSides[0];
    }
    sp = CheckAndReturnSpellEntry(24529);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA; //we should do the same for player too as we did for pet
        sp->EffectApplyAuraName[1] = sp->EffectApplyAuraName[0];
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
        sp->EffectBasePoints[1] = sp->EffectBasePoints[0];
        sp->EffectAmplitude[1] = sp->EffectAmplitude[0];
        sp->EffectDieSides[1] = sp->EffectDieSides[0];
    }

    //Hunter Silencing Shot
    sp = CheckAndReturnSpellEntry(34490);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_SILENCE;
    }

    // Hunter - Ferocious Inspiration
    sp = CheckAndReturnSpellEntry(34455);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->EffectTriggerSpell[0] = 34456;
        sp->procFlags = PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF); //maybe target master ?
        sp->Effect[1] = SPELL_EFFECT_NULL; //remove this
    }
    sp = CheckAndReturnSpellEntry(34459);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->EffectTriggerSpell[0] = 34456;
        sp->procFlags = PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF);
        sp->Effect[1] = SPELL_EFFECT_NULL; //remove this
    }
    sp = CheckAndReturnSpellEntry(34460);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->EffectTriggerSpell[0] = 34456;
        sp->procFlags = PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF);
        sp->Effect[1] = SPELL_EFFECT_NULL; //remove this
    }

    // Hunter - Focused Fire
    sp = CheckAndReturnSpellEntry(35029);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 35060;
    }
    sp = CheckAndReturnSpellEntry(35030);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 35061;
    }

    // Hunter - Thrill of the Hunt
    sp = CheckAndReturnSpellEntry(34497);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF);
        sp->procChance = sp->EffectBasePoints[0] + 1;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 34720;
    }
    sp = CheckAndReturnSpellEntry(34498);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF);
        sp->procChance = sp->EffectBasePoints[0] + 1;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 34720;
    }
    sp = CheckAndReturnSpellEntry(34499);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF);
        sp->procChance = sp->EffectBasePoints[0] + 1;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 34720;
    }

    // Hunter - Expose Weakness
    sp = CheckAndReturnSpellEntry(34500);
    if (sp != NULL)
        sp->procFlags = PROC_ON_RANGED_CRIT_ATTACK;
    sp = CheckAndReturnSpellEntry(34502);
    if (sp != NULL)
        sp->procFlags = PROC_ON_RANGED_CRIT_ATTACK;
    sp = CheckAndReturnSpellEntry(34503);
    if (sp != NULL)
        sp->procFlags = PROC_ON_RANGED_CRIT_ATTACK;

    //Hunter - Frenzy
    sp = CheckAndReturnSpellEntry(19621);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 19615;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->procChance = sp->EffectBasePoints[0];
        sp->procFlags = PROC_ON_CRIT_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);        //Zyres: moved from custom_c_is_flag
    }
    sp = CheckAndReturnSpellEntry(19622);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 19615;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->procChance = sp->EffectBasePoints[0];
        sp->procFlags = PROC_ON_CRIT_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);        //Zyres: moved from custom_c_is_flag
    }
    sp = CheckAndReturnSpellEntry(19623);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 19615;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->procChance = sp->EffectBasePoints[0];
        sp->procFlags = PROC_ON_CRIT_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);        //Zyres: moved from custom_c_is_flag
    }
    sp = CheckAndReturnSpellEntry(19624);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 19615;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->procChance = sp->EffectBasePoints[0];
        sp->procFlags = PROC_ON_CRIT_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);        //Zyres: moved from custom_c_is_flag
    }
    sp = CheckAndReturnSpellEntry(19625);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 19615;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->procChance = sp->EffectBasePoints[0];
        sp->procFlags = PROC_ON_CRIT_ATTACK | static_cast<uint32>(PROC_TARGET_SELF);        //Zyres: moved from custom_c_is_flag
    }

    //Hunter : Pathfinding
    sp = CheckAndReturnSpellEntry(19559);
    if (sp != NULL)
    {
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
    }
    sp = CheckAndReturnSpellEntry(19560);
    if (sp != NULL)
    {
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
    }

    //Hunter : Rapid Killing - might need to add honor trigger too here. I'm guessing you receive Xp too so I'm avoiding double proc
    sp = CheckAndReturnSpellEntry(34948);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_GAIN_EXPIERIENCE | static_cast<uint32>(PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(34949);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_GAIN_EXPIERIENCE | static_cast<uint32>(PROC_TARGET_SELF);
    }

    //Hunter : Entrapment
    sp = CheckAndReturnSpellEntry(19184);
    if (sp != NULL)
        sp->procFlags = PROC_ON_TRAP_TRIGGER;
    sp = CheckAndReturnSpellEntry(19387);
    if (sp != NULL)
        sp->procFlags = PROC_ON_TRAP_TRIGGER;
    sp = CheckAndReturnSpellEntry(19388);
    if (sp != NULL)
        sp->procFlags = PROC_ON_TRAP_TRIGGER;

    // aspect of the pack - change to AA
    sp = CheckAndReturnSpellEntry(13159);
    if (sp != NULL)
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;

    // aspect of the cheetah - add proc flags
    sp = CheckAndReturnSpellEntry(5118);
    if (sp != NULL)
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;

    // Feed pet
    sp = CheckAndReturnSpellEntry(6991);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = 0;
    }

    // MesoX: Serendipity http://www.wowhead.com/?spell=63730
    sp = CheckAndReturnSpellEntry(63730);   // Rank 1
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_BINDING_HEAL;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_FLASH_HEAL;
    }
    sp = CheckAndReturnSpellEntry(63733);   // Rank 2
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_BINDING_HEAL;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_FLASH_HEAL;
    }
    sp = CheckAndReturnSpellEntry(63737);   // Rank 3
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_BINDING_HEAL;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_FLASH_HEAL;
    }


    //////////////////////////////////////////
    // ROGUE                                //
    //////////////////////////////////////////

    // Insert rogue spell fixes here

    //Rogue - Blade Twisting Rank 1
    sp = CheckAndReturnSpellEntry(31124);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 31125;          // DankoDJ: sp->EffectTriggerSpell[0] is alread 31125 in spell.dbc !?
        sp->procFlags = PROC_ON_MELEE_ATTACK;       // DankoDJ: original 20
    }

    //Rogue - Blade Twisting Rank 2
    sp = CheckAndReturnSpellEntry(31126);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;       // DankoDJ: original 20
    }

    // Garrote - this is used?
    sp = CheckAndReturnSpellEntry(37066);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_ENEMY;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_SINGLE_ENEMY;
    }

    //rogue - Camouflage.
    sp = CheckAndReturnSpellEntry(13975);
    if (sp != NULL)
    {
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
    }
    sp = CheckAndReturnSpellEntry(14062);
    if (sp != NULL)
    {
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
    }
    sp = CheckAndReturnSpellEntry(14063);
    if (sp != NULL)
    {
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
    }

    //rogue - Vanish : Second Trigger Spell
    sp = CheckAndReturnSpellEntry(18461);
    if (sp != NULL)
        sp->AttributesEx |= ATTRIBUTESEX_NOT_BREAK_STEALTH;

    // rogue - Blind (Make it able to miss!)
    sp = CheckAndReturnSpellEntry(2094);
    if (sp != NULL)
    {
        sp->Spell_Dmg_Type = SPELL_DMG_TYPE_RANGED;
        sp->custom_is_ranged_spell = true;
    }

    //rogue - Shadowstep
    sp = CheckAndReturnSpellEntry(36563);
    if (sp != NULL)
    {
        sp->EffectMiscValue[2] = SMT_MISC_EFFECT;
    }
    // Still related to shadowstep - prevent the trigger spells from breaking stealth.
    sp = CheckAndReturnSpellEntry(44373);
    if (sp != NULL)
        sp->AttributesEx |= ATTRIBUTESEX_NOT_BREAK_STEALTH;
    sp = CheckAndReturnSpellEntry(36563);
    if (sp != NULL)
        sp->AttributesEx |= ATTRIBUTESEX_NOT_BREAK_STEALTH;
    sp = CheckAndReturnSpellEntry(36554);
    if (sp != NULL)
        sp->AttributesEx |= ATTRIBUTESEX_NOT_BREAK_STEALTH;

    //rogue - Seal Fate
    sp = CheckAndReturnSpellEntry(14186);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }
    sp = CheckAndReturnSpellEntry(14190);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }
    sp = CheckAndReturnSpellEntry(14193);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }
    sp = CheckAndReturnSpellEntry(14194);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }
    sp = CheckAndReturnSpellEntry(14195);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }
    //garrot
    sp = CheckAndReturnSpellEntry(703);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(8631);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(8632);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(8633);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(11289);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(11290);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(26839);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(26884);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;

    //rupture
    sp = CheckAndReturnSpellEntry(1943);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(8639);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(8640);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(11273);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(11274);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(11275);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;
    sp = CheckAndReturnSpellEntry(26867);
    if (sp != NULL)
        sp->MechanicsType = MECHANIC_BLEEDING;

    //Rogue - Killing Spree Stealth fix
    sp = CheckAndReturnSpellEntry(51690);
    if (sp != NULL)
        sp->AttributesEx |= ATTRIBUTESEX_NOT_BREAK_STEALTH;


    //////////////////////////////////////////
    // PRIEST                                //
    //////////////////////////////////////////

    // Insert priest spell fixes here

    // Prayer of mending
    sp = CheckAndReturnSpellEntry(41635);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_FRIEND;
    }
    sp = CheckAndReturnSpellEntry(48110);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_FRIEND;
    }
    sp = CheckAndReturnSpellEntry(48111);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_FRIEND;
    }
    sp = CheckAndReturnSpellEntry(33110);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_FRIEND;

    // Vampiric Embrace heal spell
    sp = CheckAndReturnSpellEntry(15290);
    if (sp != NULL)
    {
        sp->EffectBasePoints[0] = 2;
        sp->EffectBasePoints[1] = 14;
    }

    // Improved Mind Blast
    sp = CheckAndReturnSpellEntry(15273);   //rank 1
    if (sp != NULL)
    {
        sp->procChance = 20;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    }
    sp = CheckAndReturnSpellEntry(15312);   //rank 2
    if (sp != NULL)
    {
        sp->procChance = 40;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    }
    sp = CheckAndReturnSpellEntry(15313);   //rank 3
    if (sp != NULL)
    {
        sp->procChance = 60;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    }
    sp = CheckAndReturnSpellEntry(15314);   //rank 4
    if (sp != NULL)
    {
        sp->procChance = 80;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    }
    sp = CheckAndReturnSpellEntry(15316);   //rank 5
    if (sp != NULL)
    {
        sp->procChance = 100;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    }

    // Body and soul - fix duration of cleanse poison
    sp = CheckAndReturnSpellEntry(64134);
    if (sp != NULL)
        sp->DurationIndex = 29;

    // Spirit of Redemption - required spells can be casted while dead
    sp = CheckAndReturnSpellEntry(27795);   // This is casted by shape shift
    if (sp != NULL)
        sp->AttributesExC |= CAN_PERSIST_AND_CASTED_WHILE_DEAD;
    sp = CheckAndReturnSpellEntry(27792);   // This is casted by Apply Aura: Spirit of Redemption
    if (sp != NULL)
        sp->AttributesExC |= CAN_PERSIST_AND_CASTED_WHILE_DEAD;

    //Priest: Blessed Recovery
    // DankoDJ: Trigger the current EffectTrigger* defines through the originall EffectTriggerSpell from spell.dbc
    sp = CheckAndReturnSpellEntry(27811);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[0] = 27813;
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
    }
    sp = CheckAndReturnSpellEntry(27815);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[0] = 27817;
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
    }
    sp = CheckAndReturnSpellEntry(27816);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[0] = 27818;
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
    }
    //priest- Blessed Resilience
    sp = CheckAndReturnSpellEntry(33142);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
    }
    sp = CheckAndReturnSpellEntry(33145);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
    }
    sp = CheckAndReturnSpellEntry(33146);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
    }

    //priest- Focused Will
    sp = CheckAndReturnSpellEntry(45234);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
    sp = CheckAndReturnSpellEntry(45243);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
    sp = CheckAndReturnSpellEntry(45244);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM;

    //Priest - Wand Specialization
    sp = CheckAndReturnSpellEntry(14524);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
    }
    sp = CheckAndReturnSpellEntry(14525);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
    }
    sp = CheckAndReturnSpellEntry(14526);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
    }
    sp = CheckAndReturnSpellEntry(14527);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
    }
    sp = CheckAndReturnSpellEntry(14528);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
    }

    //Priest - Inspiration proc spell
    sp = CheckAndReturnSpellEntry(14893);
    if (sp != NULL)
        sp->rangeIndex = 4;
    sp = CheckAndReturnSpellEntry(15357);
    if (sp != NULL)
        sp->rangeIndex = 4;
    sp = CheckAndReturnSpellEntry(15359);
    if (sp != NULL)
        sp->rangeIndex = 4;

    //priest - surge of light
    sp = CheckAndReturnSpellEntry(33150);
    if (sp != NULL)
        sp->procFlags = uint32(PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF));
    sp = CheckAndReturnSpellEntry(33154);
    if (sp != NULL)
        sp->procFlags = uint32(PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF));
    sp = CheckAndReturnSpellEntry(33151);
    if (sp != NULL)
    {
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;
    }
    // priest - Reflective Shield
    sp = CheckAndReturnSpellEntry(33201);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_ABSORB;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 33619; //!! WRONG spell, we will make direct dmg here
    }
    sp = CheckAndReturnSpellEntry(33202);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_ABSORB;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 33619; //!! WRONG spell, we will make direct dmg here
    }
    // Weakened Soul - Is debuff
    sp = CheckAndReturnSpellEntry(6788);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_IGNORE_INVULNERABILITY;
    }

    // Penance
    sp = CheckAndReturnSpellEntry(47540);
    if (sp != NULL)
    {
        sp->DurationIndex = 566; // Change to instant cast as script will cast the real channeled spell.
        sp->ChannelInterruptFlags = 0; // Remove channeling behavior.
    }

    sp = CheckAndReturnSpellEntry(53005);
    if (sp != NULL)
    {
        sp->DurationIndex = 566;
        sp->ChannelInterruptFlags = 0;
    }

    sp = CheckAndReturnSpellEntry(53006);
    if (sp != NULL)
    {
        sp->DurationIndex = 566;
        sp->ChannelInterruptFlags = 0;
    }

    sp = CheckAndReturnSpellEntry(53007);
    if (sp != NULL)
    {
        sp->DurationIndex = 566;
        sp->ChannelInterruptFlags = 0;
    }

    // Penance triggered healing spells have wrong targets.
    sp = CheckAndReturnSpellEntry(47750);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_FRIEND;
    }

    sp = CheckAndReturnSpellEntry(52983);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_FRIEND;
    }

    sp = CheckAndReturnSpellEntry(52984);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_FRIEND;
    }

    sp = CheckAndReturnSpellEntry(52985);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_FRIEND;
    }

    //Grace Rank 1
    sp = CheckAndReturnSpellEntry(47516);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 100;
    }

    //Grace Rank 2
    sp = CheckAndReturnSpellEntry(47517);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 100;
    }

    //////////////////////////////////////////
    // SHAMAN                                //
    //////////////////////////////////////////

    // Insert shaman spell fixes here
    //shaman - Healing Way
    sp = CheckAndReturnSpellEntry(29202);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;     // DankoDJ: No triggered Spell! We override SPELL_AURA_ADD_PCT_MODIFIER with this crap?
    }
    sp = CheckAndReturnSpellEntry(29205);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;     // DankoDJ: No triggered Spell! We override SPELL_AURA_ADD_PCT_MODIFIER with this crap?
    }
    sp = CheckAndReturnSpellEntry(29206);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;     // DankoDJ: No triggered Spell! We override SPELL_AURA_ADD_PCT_MODIFIER with this crap?
    }

    // Elemental Mastery
    sp = CheckAndReturnSpellEntry(16166);
    if (sp != NULL)
    {
        sp->EffectMiscValue[0] = SMT_CRITICAL;
        sp->EffectMiscValue[1] = SMT_COST;
        // sp->AuraInterruptFlags = AURA_INTERRUPT_ON_AFTER_CAST_SPELL;
    }

    ////////////////////////////////////////////////////////////
    // Shamanistic Rage
    SpellEntry*  parentsp = CheckAndReturnSpellEntry(30823);
    SpellEntry* triggersp = CheckAndReturnSpellEntry(30824);
    if (parentsp != NULL && triggersp != NULL)
        triggersp->EffectBasePoints[0] = parentsp->EffectBasePoints[0];

    //summon only 1 elemental totem
    sp = CheckAndReturnSpellEntry(2894);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_TOTEM_FIRE; //remove this targeting. it is enough to get 1 target

    //summon only 1 elemental totem
    sp = CheckAndReturnSpellEntry(2062);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_TOTEM_EARTH; //remove this targeting. it is enough to get 1 target

    // Elemental Focus
    sp = CheckAndReturnSpellEntry(16164);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;

    // Stormstrike
    sp = CheckAndReturnSpellEntry(17364);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_SPELL_HIT_VICTIM;
    }

    ////////////////////////////////////////////////////////////
    // Bloodlust
    //Bloodlust
    sp = CheckAndReturnSpellEntry(2825);
    if (sp != NULL)
        sp->casterAuraSpellNot = 57724; //sated debuff

    // Sated - is debuff
    sp = CheckAndReturnSpellEntry(57724);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_IGNORE_INVULNERABILITY;
    }

    ////////////////////////////////////////////////////////////
    // Heroism
    //Heroism
    sp = CheckAndReturnSpellEntry(32182);
    if (sp != NULL)
        sp->casterAuraSpellNot = 57723; //sated debuff

    // Sated - is debuff
    sp = CheckAndReturnSpellEntry(57723);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_IGNORE_INVULNERABILITY;
    }

    ////////////////////////////////////////////////////////////
    // Lightning Overload
    sp = CheckAndReturnSpellEntry(30675);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 39805;//proc something (we will override this)
        sp->procFlags = PROC_ON_SPELL_HIT;
    }
    sp = CheckAndReturnSpellEntry(30678);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 39805;//proc something (we will override this)
        sp->procFlags = PROC_ON_SPELL_HIT;
    }
    sp = CheckAndReturnSpellEntry(30679);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 39805;//proc something (we will override this)
        sp->procFlags = PROC_ON_SPELL_HIT;
    }
    ////////////////////////////////////////////////////////////
    // Purge
    sp = CheckAndReturnSpellEntry(370);
    if (sp != NULL)
        sp->DispelType = DISPEL_MAGIC;
    sp = CheckAndReturnSpellEntry(8012);
    if (sp != NULL)
        sp->DispelType = DISPEL_MAGIC;
    sp = CheckAndReturnSpellEntry(27626);
    if (sp != NULL)
        sp->DispelType = DISPEL_MAGIC;
    sp = CheckAndReturnSpellEntry(33625);
    if (sp != NULL)
        sp->DispelType = DISPEL_MAGIC;

    ////////////////////////////////////////////////////////////
    // Eye of the Storm
    sp = CheckAndReturnSpellEntry(29062);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM;
    sp = CheckAndReturnSpellEntry(29064);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM;
    sp = CheckAndReturnSpellEntry(29065);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM;

    //Shaman - Shamanistic Focus
    // needs to be fixed (doesn't need to proc, it now just reduces mana cost always by %)
    sp = CheckAndReturnSpellEntry(43338);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[0] = 43339;
    }

    sp = CheckAndReturnSpellEntry(43339);
    if (sp != NULL)
    {
        sp->EffectMiscValue[0] = SMT_COST;
    }

    //shaman - Improved Chain Heal
    sp = CheckAndReturnSpellEntry(30873);
    if (sp != NULL)
    {
        sp->EffectDieSides[0] = 0;
    }
    sp = CheckAndReturnSpellEntry(30872);
    if (sp != NULL)
    {
        sp->EffectDieSides[0] = 0;
    }

    //shaman - Improved Weapon Totems
    sp = CheckAndReturnSpellEntry(29193);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectApplyAuraName[1] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
        sp->EffectMiscValue[1] = SMT_MISC_EFFECT;
    }
    sp = CheckAndReturnSpellEntry(29192);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectApplyAuraName[1] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectMiscValue[0] = SMT_MISC_EFFECT;
        sp->EffectMiscValue[1] = SMT_MISC_EFFECT;
    }

    // Shaman - Improved Fire Totems
    sp = CheckAndReturnSpellEntry(16544);
    if (sp != NULL)
    {
        sp->EffectMiscValue[0] = SMT_DURATION;
    }
    sp = CheckAndReturnSpellEntry(16086);
    if (sp != NULL)
    {
        sp->EffectMiscValue[0] = SMT_DURATION;
    }

    // Shaman Arena totems fix
    // Totem of Third WInd
    sp = CheckAndReturnSpellEntry(46098);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(34138);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(42370);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(43728);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    //shaman - Elemental Weapons
    sp = CheckAndReturnSpellEntry(29080);
    if (sp != NULL)
    {
        sp->EffectMiscValue[1] = SMT_DAMAGE_DONE;
        sp->EffectMiscValue[2] = SMT_DAMAGE_DONE;
    }
    sp = CheckAndReturnSpellEntry(29079);
    if (sp != NULL)
    {
        sp->EffectMiscValue[1] = SMT_DAMAGE_DONE;
        sp->EffectMiscValue[2] = SMT_DAMAGE_DONE;
    }
    sp = CheckAndReturnSpellEntry(16266);
    if (sp != NULL)
    {
        sp->EffectMiscValue[1] = SMT_DAMAGE_DONE;
        sp->EffectMiscValue[2] = SMT_DAMAGE_DONE;
    }

    // Magma Totem - 0% spd coefficient
    sp = CheckAndReturnSpellEntry(25550);
    if (sp != NULL)
        sp->fixed_dddhcoef = 0.0f;
    sp = CheckAndReturnSpellEntry(10581);
    if (sp != NULL)
        sp->fixed_dddhcoef = 0.0f;
    sp = CheckAndReturnSpellEntry(10580);
    if (sp != NULL)
        sp->fixed_dddhcoef = 0.0f;
    sp = CheckAndReturnSpellEntry(10579);
    if (sp != NULL)
        sp->fixed_dddhcoef = 0.0f;
    sp = CheckAndReturnSpellEntry(8187);
    if (sp != NULL)
        sp->fixed_dddhcoef = 0.0f;

    //Tidal Waves
    sp = CheckAndReturnSpellEntry(51562);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(51563);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(51564);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(51565);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(51566);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    //Earthliving Weapon
    sp = CheckAndReturnSpellEntry(51940);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 20;
    }
    sp = CheckAndReturnSpellEntry(51989);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 20;
    }
    sp = CheckAndReturnSpellEntry(52004);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 20;
    }
    sp = CheckAndReturnSpellEntry(52005);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 20;
    }
    sp = CheckAndReturnSpellEntry(52007);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 20;
    }
    sp = CheckAndReturnSpellEntry(52008);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 20;
    }

    //Maelstrom Weapon
    sp = CheckAndReturnSpellEntry(51528);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
        sp->custom_proc_interval = 24000;
    }
    sp = CheckAndReturnSpellEntry(51529);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
        sp->custom_proc_interval = 12000;
    }
    sp = CheckAndReturnSpellEntry(51530);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
        sp->custom_proc_interval = 8000;
    }
    sp = CheckAndReturnSpellEntry(51531);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
        sp->custom_proc_interval = 6000;
    }
    sp = CheckAndReturnSpellEntry(51532);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
        sp->custom_proc_interval = 5000;
    }
    sp = CheckAndReturnSpellEntry(53817);
    if (sp != NULL)
        sp->procCharges = 0;

    ////////////////////////////////////////////////////////////
    //  Unleashed Rage - LordLeeCH
    sp = CheckAndReturnSpellEntry(30802);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
        sp->Effect[0] = SPELL_EFFECT_APPLY_GROUP_AREA_AURA;
    }
    sp = CheckAndReturnSpellEntry(30808);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
        sp->Effect[0] = SPELL_EFFECT_APPLY_GROUP_AREA_AURA;
    }
    sp = CheckAndReturnSpellEntry(30809);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CRIT_ATTACK;
        sp->Effect[0] = SPELL_EFFECT_APPLY_GROUP_AREA_AURA;
    }

    ////////////////////////////////////////////////////////////
    // Elemental Devastation
    sp = CheckAndReturnSpellEntry(29179);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    sp = CheckAndReturnSpellEntry(29180);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    sp = CheckAndReturnSpellEntry(30160);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;

    ////////////////////////////////////////////////////////////
    // Ancestral healing
    sp = CheckAndReturnSpellEntry(16176);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    sp = CheckAndReturnSpellEntry(16235);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    sp = CheckAndReturnSpellEntry(16240);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;

    ////////////////////////////////////////////////////////////
    // Ancestral healing proc spell
    sp = CheckAndReturnSpellEntry(16177);
    if (sp != NULL)
        sp->rangeIndex = 4;
    sp = CheckAndReturnSpellEntry(16236);
    if (sp != NULL)
        sp->rangeIndex = 4;
    sp = CheckAndReturnSpellEntry(16237);
    if (sp != NULL)
        sp->rangeIndex = 4;

    sp = CheckAndReturnSpellEntry(20608);   //Reincarnation
    if (sp != NULL)
    {
        for (uint8 i = 0; i < 8; i++)
        {
            if (sp->Reagent[i])
            {
                sp->Reagent[i] = 0;
                sp->ReagentCount[i] = 0;
            }
        }
    }

    //////////////////////////////////////////
    // SHAMAN WRATH OF AIR TOTEM            //
    //////////////////////////////////////////
    sp = CheckAndReturnSpellEntry(2895);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SELF;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_SELF;
        sp->EffectImplicitTargetA[2] = 0;
        sp->EffectImplicitTargetB[0] = 0;
        sp->EffectImplicitTargetB[1] = 0;
        sp->EffectImplicitTargetB[2] = 0;
    }

    // Rogue - Master of Subtlety
    sp = CheckAndReturnSpellEntry(31665);
    if (sp != NULL)
        sp->AttributesEx |= ATTRIBUTESEX_NOT_BREAK_STEALTH;

    //////////////////////////////////////////
    // MAGE                                    //
    //////////////////////////////////////////

    // Insert mage spell fixes here

    //Missile Barrage
    sp = CheckAndReturnSpellEntry(44404);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(54486);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(54488);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(54489);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(54490);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    // Brain Freeze rank 1
    sp = CheckAndReturnSpellEntry(44546);
    if (sp != NULL)
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;

    // Brain Freeze rank 2
    sp = CheckAndReturnSpellEntry(44548);
    if (sp != NULL)
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;

    // Brain Freeze rank 3
    sp = CheckAndReturnSpellEntry(44549);
    if (sp != NULL)
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;

    // Fingers of Frost rank 1
    sp = CheckAndReturnSpellEntry(44543);
    if (sp != NULL)
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;


    // Fingers of Frost rank 2
    sp = CheckAndReturnSpellEntry(44545);
    if (sp != NULL)
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;

    ////////////////////////////////////////////////////////////
    // Improved Blink by Alice

    //Improved Blink - *Rank 1*
    sp = CheckAndReturnSpellEntry(31569);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
    }
    //Improved Blink - *Rank 2*
    sp = CheckAndReturnSpellEntry(31570);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
    }
    ////////////////////////////////////////////////////////////
    // Arcane Concentration

    sp = CheckAndReturnSpellEntry(11213);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_HIT | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(12574);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_HIT | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(12575);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_HIT | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(12576);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_HIT | static_cast<uint32>(PROC_TARGET_SELF);
    sp = CheckAndReturnSpellEntry(12577);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_HIT | static_cast<uint32>(PROC_TARGET_SELF);

    //Mage - Arcane Concentration proc
    sp = CheckAndReturnSpellEntry(12536);
    if (sp != NULL)
    {
        sp->custom_BGR_one_buff_on_target = 0;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procCharges = 2;
    }

    //Mage - Spell Power
    sp = CheckAndReturnSpellEntry(35578);
    if (sp != NULL)
    {
        sp->EffectMiscValue[0] = SMT_CRITICAL_DAMAGE;
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
    }
    sp = CheckAndReturnSpellEntry(35581);
    if (sp != NULL)
    {
        sp->EffectMiscValue[0] = SMT_CRITICAL_DAMAGE;
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
    }

    //Mage - Elemental Precision
    sp = CheckAndReturnSpellEntry(29438);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectMiscValue[0] = SMT_COST;
    }
    sp = CheckAndReturnSpellEntry(29439);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectMiscValue[0] = SMT_COST;
    }
    sp = CheckAndReturnSpellEntry(29440);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectMiscValue[0] = SMT_COST;
    }

    //Mage - Arcane Blast
    sp = CheckAndReturnSpellEntry(30451);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_ARCANE_BLAST;
    }

    // Arcane Blast
    sp = CheckAndReturnSpellEntry(42894);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_ARCANE_BLAST;
    }

    sp = CheckAndReturnSpellEntry(42896);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
    }

    sp = CheckAndReturnSpellEntry(42897);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
    }

    //mage : Empowered Arcane Missiles
    //heh B thinks he is smart by adding this to description ? If it doesn't work std then it still needs to made by hand
    sp = CheckAndReturnSpellEntry(31579);
    if (sp != NULL)
    {
        sp->EffectBasePoints[0] = 5 * (sp->EffectBasePoints[0] + 1);
    }
    sp = CheckAndReturnSpellEntry(31582);
    if (sp != NULL)
    {
        sp->EffectBasePoints[0] = 5 * (sp->EffectBasePoints[0] + 1);
    }
    sp = CheckAndReturnSpellEntry(31583);
    if (sp != NULL)
    {
        sp->EffectBasePoints[0] = 5 * (sp->EffectBasePoints[0] + 1);
    }

    //Mage - Improved Blizzard
    sp = CheckAndReturnSpellEntry(11185);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 12484;
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(12487);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 12485;
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(12488);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 12486;
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    // cebernic: not for self?
    // impact
    sp = CheckAndReturnSpellEntry(12355);
    if (sp != NULL)
    {
        // passive rank: 11103, 12357, 12358 ,12359,12360 :D
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM | PROC_ON_SPELL_CRIT_HIT | PROC_ON_SPELL_HIT;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_ALL_ENEMIES_AROUND_CASTER;
        sp->EffectImplicitTargetB[0] = EFF_TARGET_ALL_ENEMIES_AROUND_CASTER;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_ALL_ENEMIES_AROUND_CASTER;
        sp->EffectImplicitTargetB[1] = EFF_TARGET_ALL_ENEMIES_AROUND_CASTER;
        sp->EffectImplicitTargetA[2] = EFF_TARGET_ALL_ENEMIES_AROUND_CASTER;
        sp->EffectImplicitTargetB[2] = EFF_TARGET_ALL_ENEMIES_AROUND_CASTER;
    }

    //Mage - Invisibility
    sp = CheckAndReturnSpellEntry(66);
    if (sp != NULL)
    {
        sp->AuraInterruptFlags |= AURA_INTERRUPT_ON_CAST_SPELL;
        sp->Effect[1] = SPELL_EFFECT_NULL;
        sp->EffectApplyAuraName[2] = SPELL_AURA_PERIODIC_TRIGGER_SPELL;
        sp->Effect[2] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectAmplitude[2] = 3000;
        sp->EffectImplicitTargetA[2] = EFF_TARGET_SELF;
        //sp->EffectBaseDice[2] = 1;
        sp->EffectDieSides[2] = 1;
        sp->EffectTriggerSpell[2] = 32612;
        sp->EffectBasePoints[2] = -1;
    }

    //Invisibility triggered spell, should be removed on cast
    sp = CheckAndReturnSpellEntry(32612);
    if (sp != NULL)
    {
        sp->AuraInterruptFlags |= AURA_INTERRUPT_ON_CAST_SPELL;
    }

    //Fingers of frost proc
    sp = CheckAndReturnSpellEntry(44544);
    if (sp != NULL)
    {
        sp->procCharges = 2;
        sp->procFlags = PROC_ON_SPELL_HIT;
    }

    //Brain Freeze proc (Fireball!)
    sp = CheckAndReturnSpellEntry(57761);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procCharges = 1;
    }

    //Arcane Potency procs
    sp = CheckAndReturnSpellEntry(57529);
    if (sp != NULL)
    {
        sp->procFlags = 0;
        // sp->custom_RankNumber = 100;            DankoDJ: Why?
        sp->AuraInterruptFlags = 0;
    }

    sp = CheckAndReturnSpellEntry(57531);
    if (sp != NULL)
    {
        sp->procFlags = 0;
        // sp->custom_RankNumber = 101;            DankoDJ: Why?
        sp->AuraInterruptFlags = 0;
    }

    //Hot Streak proc
    sp = CheckAndReturnSpellEntry(48108);
    if (sp != NULL)
    {
        sp->AuraInterruptFlags |= AURA_INTERRUPT_ON_CAST_SPELL;
    }

    //Ice Lances
    sp = CheckAndReturnSpellEntry(42914);
    if (sp != NULL)
        sp->Dspell_coef_override = 0.1429f;

    sp = CheckAndReturnSpellEntry(42913);
    if (sp != NULL)
        sp->Dspell_coef_override = 0.1429f;

    sp = CheckAndReturnSpellEntry(30455);
    if (sp != NULL)
        sp->Dspell_coef_override = 0.1429f;

    // Frostfire Bolts
    sp = CheckAndReturnSpellEntry(47610);
    if (sp != NULL)
        sp->fixed_dddhcoef = 0.8571f;

    sp = CheckAndReturnSpellEntry(44614);
    if (sp != NULL)
        sp->fixed_dddhcoef = 0.8571f;


    //mage - Combustion
    sp = CheckAndReturnSpellEntry(11129);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_NULL;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 28682;
        sp->procFlags = PROC_ON_SPELL_HIT | PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF);
        sp->procCharges = 0;
    }

    //mage - Master of Elements
    sp = CheckAndReturnSpellEntry(29074);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 29077;
        sp->procFlags = uint32(PROC_ON_SPELL_CRIT_HIT | PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(29075);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 29077;
        sp->procFlags = uint32(PROC_ON_SPELL_CRIT_HIT | PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(29076);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 29077;
        sp->procFlags = uint32(PROC_ON_SPELL_CRIT_HIT | PROC_TARGET_SELF);
    }

    //mage: Blazing Speed
    sp = CheckAndReturnSpellEntry(31641);
    if (sp != NULL)
        sp->EffectTriggerSpell[0] = 31643;
    sp = CheckAndReturnSpellEntry(31642);
    if (sp != NULL)
        sp->EffectTriggerSpell[0] = 31643;

    //Mage - Improved Scorch
    sp = CheckAndReturnSpellEntry(11095);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(12872);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(12873);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    // mage - Frost Warding
    sp = CheckAndReturnSpellEntry(11189);
    if (sp != NULL)
    {
        sp->procChance = 10;
    }
    sp = CheckAndReturnSpellEntry(28332);
    if (sp != NULL)
    {
        sp->procChance = 20;
    }

    // mage - Conjure Refreshment Table
    sp = CheckAndReturnSpellEntry(43985);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_DYNAMIC_OBJECT;

    // Hypothermia - forced debuff
    sp = CheckAndReturnSpellEntry(41425);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_IGNORE_INVULNERABILITY;
    }

    // Mage - Permafrost Rank 1
    sp = CheckAndReturnSpellEntry(11175);
    if (sp != NULL)
    {
        sp->EffectMiscValue[1] = SMT_MISC_EFFECT;
    }

    // Mage - Permafrost Rank 2
    sp = CheckAndReturnSpellEntry(12569);
    if (sp != NULL)
    {
        sp->EffectMiscValue[1] = SMT_MISC_EFFECT;
    }

    // Mage - Permafrost Rank 3
    sp = CheckAndReturnSpellEntry(12571);
    if (sp != NULL)
    {
        sp->EffectMiscValue[1] = SMT_MISC_EFFECT;
    }

    //Improved Counterspell rank 1
    sp = CheckAndReturnSpellEntry(11255);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_COUNTERSPELL;
    }

    //Improved Counterspell rank 2
    sp = CheckAndReturnSpellEntry(12598);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_COUNTERSPELL;
    }
    //////////////////////////////////////////
    // WARLOCK                                //
    //////////////////////////////////////////

    // Insert warlock spell fixes here

    //Dummy for Demonic Circle
    sp = CheckAndReturnSpellEntry(48018);
    if (sp != NULL)
    {

        sp->EffectImplicitTargetA[1] = 1;
        CreateDummySpell(62388);
        sp = CheckAndReturnSpellEntry(62388);
        if (sp != NULL)
        {
            sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
            sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
        }
    }

    //megai2: Immolation Aura
    sp = CheckAndReturnSpellEntry(50589);
    if (sp != NULL)
    {
        sp->ChannelInterruptFlags = 0; // Remove channeling behaviour.
    }

    //megai2: Everlasting Affliction
    sp = CheckAndReturnSpellEntry(47205);
    if (sp != NULL)
    {
        sp->EffectSpellClassMask[1][0] = 0x111;
        sp->EffectSpellClassMask[1][1] = 0;
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION;
    }

    sp = CheckAndReturnSpellEntry(47204);
    if (sp != NULL)
    {
        sp->EffectSpellClassMask[1][0] = 0x111;
        sp->EffectSpellClassMask[1][1] = 0;
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION;
    }

    sp = CheckAndReturnSpellEntry(47203);
    if (sp != NULL)
    {
        sp->EffectSpellClassMask[1][0] = 0x111;
        sp->EffectSpellClassMask[1][1] = 0;
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION;
    }

    sp = CheckAndReturnSpellEntry(47202);
    if (sp != NULL)
    {
        sp->EffectSpellClassMask[1][0] = 0x111;
        sp->EffectSpellClassMask[1][1] = 0;
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION;
    }

    sp = CheckAndReturnSpellEntry(47201);
    if (sp != NULL)
    {
        sp->EffectSpellClassMask[1][0] = 0x111;
        sp->EffectSpellClassMask[1][1] = 0;
    }

    //warlock: Eradication
    sp = CheckAndReturnSpellEntry(47195);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION;
    }

    sp = CheckAndReturnSpellEntry(47196);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION;
    }

    sp = CheckAndReturnSpellEntry(47197);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION;
    }

    //Warlock Molten Core
    sp = CheckAndReturnSpellEntry(47245);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION;
    }

    sp = CheckAndReturnSpellEntry(47246);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION;
    }

    sp = CheckAndReturnSpellEntry(47247);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION;
    }

    ////////////////////////////////////////////////////////////
    // Nether Protection
    sp = CheckAndReturnSpellEntry(30299);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 13000;
    }
    sp = CheckAndReturnSpellEntry(30301);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 13000;
    }
    sp = CheckAndReturnSpellEntry(30302);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 13000;
    }
    ////////////////////////////////////////////////////////////
    // Backlash
    sp = CheckAndReturnSpellEntry(34935);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 8000;
        sp->procFlags |= PROC_ON_MELEE_ATTACK_VICTIM | static_cast<uint32>(PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(34938);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 8000;
        sp->procFlags |= PROC_ON_MELEE_ATTACK_VICTIM | static_cast<uint32>(PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(34939);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 8000;
        sp->procFlags |= PROC_ON_MELEE_ATTACK_VICTIM | static_cast<uint32>(PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(34936);
    if (sp != NULL)
    {
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;
    }

    ////////////////////////////////////////////////////////////
    // Demonic Knowledge
    sp = CheckAndReturnSpellEntry(35691);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_DAMAGE_DONE;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SELF;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_DONE;
        sp->EffectBasePoints[1] = sp->EffectBasePoints[0];
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
        sp->Effect[2] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[2] = 35696;
        sp->EffectImplicitTargetA[2] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(35692);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_DAMAGE_DONE;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SELF;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_DONE;
        sp->EffectBasePoints[1] = sp->EffectBasePoints[0];
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
        sp->Effect[2] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[2] = 35696;
        sp->EffectImplicitTargetA[2] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(35693);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_DAMAGE_DONE;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SELF;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_DONE;
        sp->EffectBasePoints[1] = sp->EffectBasePoints[0];
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
        sp->Effect[2] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[2] = 35696;
        sp->EffectImplicitTargetA[2] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(35696);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA; //making this only for the visible effect
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY; //no effect here
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    }

    //warlock -  Seed of Corruption
    sp = CheckAndReturnSpellEntry(27243);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 27285;
        sp->procFlags = PROC_ON_SPELL_HIT_VICTIM | PROC_ON_DIE;
    }

    //warlock: Nightfall
    sp = CheckAndReturnSpellEntry(18094);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 17941;
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION | static_cast<uint32>(PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(18095);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 17941;
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION | static_cast<uint32>(PROC_TARGET_SELF);
    }
    //Shadow Trance should be removed on the first SB
    sp = CheckAndReturnSpellEntry(17941);
    if (sp != NULL)
    {
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;
    }

    //warlock: Empowered Corruption
    sp = CheckAndReturnSpellEntry(32381);
    if (sp != NULL)
    {
        sp->EffectBasePoints[0] *= 6;
    }
    sp = CheckAndReturnSpellEntry(32382);
    if (sp != NULL)
    {
        sp->EffectBasePoints[0] *= 6;
    }
    sp = CheckAndReturnSpellEntry(32383);
    if (sp != NULL)
    {
        sp->EffectBasePoints[0] *= 6;
    }

    //warlock - Demonic Tactics
    sp = CheckAndReturnSpellEntry(30242);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_NULL; //disable this. This is just blizz crap. Pure proof that they suck :P
        sp->EffectImplicitTargetB[1] = EFF_TARGET_PET;
        //sp->EffectApplyAuraName[2] = SPELL_AURA_MOD_SPELL_CRIT_CHANCE; //lvl 1 has it fucked up :O 
                                                                        // Zyres: No you fukced it up. This spell was defined few lines below.
        sp->EffectImplicitTargetB[2] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(30245);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_NULL; //disable this. This is just blizz crap. Pure proof that they suck :P
        sp->EffectImplicitTargetB[1] = EFF_TARGET_PET;
        sp->EffectImplicitTargetB[2] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(30246);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_NULL; //disable this. This is just blizz crap. Pure proof that they suck :P
        sp->EffectImplicitTargetB[1] = EFF_TARGET_PET;
        sp->EffectImplicitTargetB[2] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(30247);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_NULL; //disable this. This is just blizz crap. Pure proof that they suck :P
        sp->EffectImplicitTargetB[1] = EFF_TARGET_PET;
        sp->EffectImplicitTargetB[2] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(30248);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_NULL; //disable this. This is just blizz crap. Pure proof that they suck :P
        sp->EffectImplicitTargetB[1] = EFF_TARGET_PET;
        sp->EffectImplicitTargetB[2] = EFF_TARGET_PET;
    }

    //warlock - Demonic Resilience
    sp = CheckAndReturnSpellEntry(30319);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(30320);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(30321);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
    }

    //warlock - Improved Imp
    sp = CheckAndReturnSpellEntry(18694);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(18695);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(18696);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    }

    //warlock - Demonic Brutality
    sp = CheckAndReturnSpellEntry(18705);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(18706);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(18707);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    }

    //warlock - Improved Succubus
    sp = CheckAndReturnSpellEntry(18754);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(18755);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(18756);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
    }

    //warlock - Fel Vitality
    sp = CheckAndReturnSpellEntry(18731);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_PERCENT_STAT;
        sp->EffectMiscValue[0] = 3;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(18743);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_PERCENT_STAT;
        sp->EffectMiscValue[0] = 3;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    }
    sp = CheckAndReturnSpellEntry(18744);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_PERCENT_STAT;
        sp->EffectMiscValue[0] = 3;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    }

    //warlock - Demonic Tactics
    /* Zyres: Disabled this spell has already some changes few lines above!
    sp = CheckAndReturnSpellEntry(30242);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        //this is required since blizz uses spells for melee attacks while we use fixed functions
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_PERCENT_DONE;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
        sp->EffectMiscValue[1] = SCHOOL_NORMAL;
        sp->EffectBasePoints[1] = sp->EffectBasePoints[0];
    }*/

    //warlock - Unholy Power
    sp = CheckAndReturnSpellEntry(18769);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        //this is required since blizz uses spells for melee attacks while we use fixed functions
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_PERCENT_DONE;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
        sp->EffectMiscValue[1] = SCHOOL_NORMAL;
        sp->EffectBasePoints[1] = sp->EffectBasePoints[0];
    }
    sp = CheckAndReturnSpellEntry(18770);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        //this is required since blizz uses spells for melee attacks while we use fixed functions
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_PERCENT_DONE;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
        sp->EffectMiscValue[1] = SCHOOL_NORMAL;
        sp->EffectBasePoints[1] = sp->EffectBasePoints[0];
    }
    sp = CheckAndReturnSpellEntry(18771);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        //this is required since blizz uses spells for melee attacks while we use fixed functions
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_PERCENT_DONE;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
        sp->EffectMiscValue[1] = SCHOOL_NORMAL;
        sp->EffectBasePoints[1] = sp->EffectBasePoints[0];
    }
    sp = CheckAndReturnSpellEntry(18772);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        //this is required since blizz uses spells for melee attacks while we use fixed functions
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_PERCENT_DONE;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
        sp->EffectMiscValue[1] = SCHOOL_NORMAL;
        sp->EffectBasePoints[1] = sp->EffectBasePoints[0];
    }
    sp = CheckAndReturnSpellEntry(18773);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_ADD_PCT_MODIFIER;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
        //this is required since blizz uses spells for melee attacks while we use fixed functions
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_DAMAGE_PERCENT_DONE;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_PET;
        sp->EffectMiscValue[1] = SCHOOL_NORMAL;
        sp->EffectBasePoints[1] = sp->EffectBasePoints[0];
    }

    //warlock - Master Demonologist - 25 spells here
    sp = CheckAndReturnSpellEntry(23785);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 23784;
    }
    sp = CheckAndReturnSpellEntry(23822);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 23830;
    }
    sp = CheckAndReturnSpellEntry(23823);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 23831;
    }
    sp = CheckAndReturnSpellEntry(23824);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 23832;
    }
    sp = CheckAndReturnSpellEntry(23825);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 35708;
    }
    //and the rest
    sp = CheckAndReturnSpellEntry(23784);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    sp = CheckAndReturnSpellEntry(23830);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    sp = CheckAndReturnSpellEntry(23831);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    sp = CheckAndReturnSpellEntry(23832);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    sp = CheckAndReturnSpellEntry(35708);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    sp = CheckAndReturnSpellEntry(23759);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
    }
    sp = CheckAndReturnSpellEntry(23760);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
    }
    sp = CheckAndReturnSpellEntry(23761);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
    }
    sp = CheckAndReturnSpellEntry(23762);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
    }
    sp = CheckAndReturnSpellEntry(23826);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
    }
    sp = CheckAndReturnSpellEntry(23827);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
    }
    sp = CheckAndReturnSpellEntry(23828);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
    }
    sp = CheckAndReturnSpellEntry(23829);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
    }
    // Zyres: eeek
    for (uint32 i = 23833; i <= 23844; i++)
    {
        sp = CheckAndReturnSpellEntry(i);
        if (sp != NULL)
        {
            sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        }
    }
    sp = CheckAndReturnSpellEntry(35702);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->Effect[1] = SPELL_EFFECT_NULL; //hacks, we are handling this in another way
    }
    sp = CheckAndReturnSpellEntry(35703);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->Effect[1] = SPELL_EFFECT_NULL; //hacks, we are handling this in another way
    }
    sp = CheckAndReturnSpellEntry(35704);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->Effect[1] = SPELL_EFFECT_NULL; //hacks, we are handling this in another way
    }
    sp = CheckAndReturnSpellEntry(35705);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->Effect[1] = SPELL_EFFECT_NULL; //hacks, we are handling this in another way
    }
    sp = CheckAndReturnSpellEntry(35706);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->Effect[1] = SPELL_EFFECT_NULL; //hacks, we are handling this in another way
    }

    //warlock - Improved Drain Soul
    sp = CheckAndReturnSpellEntry(18213);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_TARGET_DIE | static_cast<uint32>(PROC_TARGET_SELF);
        sp->procChance = 100;
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 18371;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SELF;
        sp->Effect[2] = SPELL_EFFECT_NULL; //remove this effect
    }
    sp = CheckAndReturnSpellEntry(18372);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_TARGET_DIE | static_cast<uint32>(PROC_TARGET_SELF);
        sp->procChance = 100;
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 18371;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SELF;
        sp->Effect[2] = SPELL_EFFECT_NULL; //remove this effect
    }

    //warlock - Shadow Embrace
    sp = CheckAndReturnSpellEntry(32385);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->Effect[1] = SPELL_EFFECT_NULL; //remove this effect ? Maybe remove the other one :P xD
    }
    sp = CheckAndReturnSpellEntry(32387);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->Effect[1] = SPELL_EFFECT_NULL; //remove this effect ? Maybe remove the other one :P xD
    }
    sp = CheckAndReturnSpellEntry(32392);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->Effect[1] = SPELL_EFFECT_NULL; //remove this effect ? Maybe remove the other one :P xD
    }
    sp = CheckAndReturnSpellEntry(32393);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->Effect[1] = SPELL_EFFECT_NULL; //remove this effect ? Maybe remove the other one :P xD
    }
    sp = CheckAndReturnSpellEntry(32394);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->Effect[1] = SPELL_EFFECT_NULL; //remove this effect ? Maybe remove the other one :P xD
    }

    //warlock - soul leech
    sp = CheckAndReturnSpellEntry(30293);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA; //aura
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 30294;
        sp->procFlags = uint32(PROC_ON_CAST_SPELL | PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(30295);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA; //aura
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 30294;
        sp->procFlags = uint32(PROC_ON_CAST_SPELL | PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(30296);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA; //aura
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 30294;
        sp->procFlags = uint32(PROC_ON_CAST_SPELL | PROC_TARGET_SELF);
    }

    //warlock - Pyroclasm
    sp = CheckAndReturnSpellEntry(18073);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_NULL; //delete this override effect :P
        sp->EffectTriggerSpell[1] = 18093; //trigger spell was wrong :P
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION;
        sp->procChance = 13; //god, save us from fixed values !
    }
    sp = CheckAndReturnSpellEntry(18096);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_NULL; //delete this override effect :P
        sp->EffectTriggerSpell[1] = 18093; //trigger spell was wrong :P
        sp->procFlags = PROC_ON_ANY_HOSTILE_ACTION;
        sp->procChance = 26; //god, save us from fixed values !
    }
    //Warlock Chaos bolt
    sp = CheckAndReturnSpellEntry(50796);
    if (sp != NULL)
    {
        sp->Attributes |= ATTRIBUTES_IGNORE_INVULNERABILITY;
        sp->School = SCHOOL_FIRE;
    }

    sp = CheckAndReturnSpellEntry(59170);
    if (sp != NULL)
    {
        sp->Attributes |= ATTRIBUTES_IGNORE_INVULNERABILITY;
        sp->School = SCHOOL_FIRE;
    }

    sp = CheckAndReturnSpellEntry(59171);
    if (sp != NULL)
    {
        sp->Attributes |= ATTRIBUTES_IGNORE_INVULNERABILITY;
        sp->School = SCHOOL_FIRE;
    }

    sp = CheckAndReturnSpellEntry(59172);
    if (sp != NULL)
    {
        sp->Attributes |= ATTRIBUTES_IGNORE_INVULNERABILITY;
        sp->School = SCHOOL_FIRE;
    }
    // End Warlock chaos bolt

    //Warlock Healthstones
    int HealthStoneID[8] = { 6201, 6202, 5699, 11729, 11730, 27230, 47871, 47878 };
    for (uint8 i = 0; i < 8; i++)
    {
        sp = CheckAndReturnSpellEntry(HealthStoneID[i]);
        if (sp != NULL)
        {
            sp->Reagent[1] = 0;
        }
    }

    //Backdraft Rank 1
    sp = CheckAndReturnSpellEntry(47258);
    if (sp != NULL)
    {
        sp->procFlags = uint32(PROC_TARGET_SELF | PROC_ON_ANY_HOSTILE_ACTION);
    }

    //Backdraft Rank 2
    sp = CheckAndReturnSpellEntry(47259);
    if (sp != NULL)
    {
        sp->procFlags = uint32(PROC_TARGET_SELF | PROC_ON_ANY_HOSTILE_ACTION);
    }

    //Backdraft Rank 3
    sp = CheckAndReturnSpellEntry(47260);
    if (sp != NULL)
    {
        sp->procFlags = uint32(PROC_TARGET_SELF | PROC_ON_ANY_HOSTILE_ACTION);
    }

    //////////////////////////////////////////
    // DRUID                                //
    //////////////////////////////////////////

    // Insert druid spell fixes here

    ////////////////////////////////////////////////////////////
    // Balance
    ////////////////////////////////////////////////////////////

    // Druid - Nature's Grace
    sp = CheckAndReturnSpellEntry(16880);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;

    sp = CheckAndReturnSpellEntry(61345);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;

    sp = CheckAndReturnSpellEntry(61346);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;

    // Druid - Earth and Moon
    sp = CheckAndReturnSpellEntry(48506);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    sp = CheckAndReturnSpellEntry(48510);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    sp = CheckAndReturnSpellEntry(48511);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    // Druid - Nature's Grasp //sp->AuraInterruptFlags = 0; //we remove it on proc or timeout
    sp = CheckAndReturnSpellEntry(16689);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_REMOVEONUSE;

    sp = CheckAndReturnSpellEntry(16810);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_REMOVEONUSE;

    sp = CheckAndReturnSpellEntry(16811);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_REMOVEONUSE;

    sp = CheckAndReturnSpellEntry(16812);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_REMOVEONUSE;

    sp = CheckAndReturnSpellEntry(16813);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_REMOVEONUSE;

    sp = CheckAndReturnSpellEntry(17329);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_REMOVEONUSE;

    sp = CheckAndReturnSpellEntry(27009);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_REMOVEONUSE;

    sp = CheckAndReturnSpellEntry(53312);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_REMOVEONUSE;

    // Druid - Force of Nature
    sp = CheckAndReturnSpellEntry(33831);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SELF; //some land under target is used that gathers multiple targets ...
        sp->EffectImplicitTargetA[1] = EFF_TARGET_NONE;
    }

    ////////////////////////////////////////////////////////////
    //    Feral Combat
    ////////////////////////////////////////////////////////////

    // Druid - Natural Reaction
    sp = CheckAndReturnSpellEntry(57878);
    if (sp != NULL)
        sp->procFlags = PROC_ON_DODGE_VICTIM;

    sp = CheckAndReturnSpellEntry(57880);
    if (sp != NULL)
        sp->procFlags = PROC_ON_DODGE_VICTIM;

    sp = CheckAndReturnSpellEntry(57881);
    if (sp != NULL)
        sp->procFlags = PROC_ON_DODGE_VICTIM;

    // Druid - Infected Wounds
    sp = CheckAndReturnSpellEntry(48483);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_SHRED;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_MAUL;
        sp->custom_ProcOnNameHash[2] = SPELL_HASH_MANGLE;
    }

    sp = CheckAndReturnSpellEntry(48484);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_SHRED;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_MAUL;
        sp->custom_ProcOnNameHash[2] = SPELL_HASH_MANGLE;
    }

    sp = CheckAndReturnSpellEntry(48485);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_SHRED;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_MAUL;
        sp->custom_ProcOnNameHash[2] = SPELL_HASH_MANGLE;
    }

    // Druid - Bash - Interrupt effect
    sp = CheckAndReturnSpellEntry(5211);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 32747;
    }
    sp = CheckAndReturnSpellEntry(6798);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 32747;
    }
    sp = CheckAndReturnSpellEntry(8983);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 32747;
    }

    //Druid - Feral Swiftness
    sp = CheckAndReturnSpellEntry(17002);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 24867;
    }
    sp = CheckAndReturnSpellEntry(24866);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 24864;
    }

    // Druid - Maim
    sp = CheckAndReturnSpellEntry(22570);
    if (sp != NULL)
    {
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_UNUSED2;
        sp->custom_is_melee_spell = true;
    }
    sp = CheckAndReturnSpellEntry(49802);
    if (sp != NULL)
    {
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_UNUSED2;
        sp->custom_is_melee_spell = true;
    }

    sp = CheckAndReturnSpellEntry(20719); //feline grace
    if (sp != NULL)
        sp->Effect[0] = SPELL_EFFECT_NULL;

    // Druid - Feral Swiftness
    sp = CheckAndReturnSpellEntry(17002);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 24867;
    }
    sp = CheckAndReturnSpellEntry(24866);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 24864;
    }

    // Druid - Frenzied Regeneration
    sp = CheckAndReturnSpellEntry(22842);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 22845;
    }

    // Druid - Improved Leader of the Pack
    sp = CheckAndReturnSpellEntry(34297);
    if (sp != NULL)
        sp->custom_proc_interval = 6000;//6 secs

    sp = CheckAndReturnSpellEntry(34300);
    if (sp != NULL)
        sp->custom_proc_interval = 6000;//6 secs

    // Druid - Primal Fury (talent)
    sp = CheckAndReturnSpellEntry(37116);
    if (sp != NULL)
        sp->RequiredShapeShift = 0;

    sp = CheckAndReturnSpellEntry(37117);
    if (sp != NULL)
        sp->RequiredShapeShift = 0;

    // Druid - Blood Frenzy (proc)
    sp = CheckAndReturnSpellEntry(16954);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_ATTACK;

    sp = CheckAndReturnSpellEntry(16952);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_ATTACK;

    // Druid - Primal Fury (proc)
    sp = CheckAndReturnSpellEntry(16961);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_ATTACK;

    sp = CheckAndReturnSpellEntry(16958);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_ATTACK;

    // Druid - Predatory Strikes
    uint32 mm = DecimalToMask(FORM_BEAR) | DecimalToMask(FORM_DIREBEAR) | DecimalToMask(FORM_MOONKIN) | DecimalToMask(FORM_CAT);

    sp = CheckAndReturnSpellEntry(16972);
    if (sp != NULL)
        sp->RequiredShapeShift = mm;
    sp = CheckAndReturnSpellEntry(16974);
    if (sp != NULL)
        sp->RequiredShapeShift = mm;
    sp = CheckAndReturnSpellEntry(16975);
    if (sp != NULL)
        sp->RequiredShapeShift = mm;

    ////////////////////////////////////////////////////////////
    // Restoration
    ////////////////////////////////////////////////////////////

    // Druid - Tree Form Aura
    /* Zyres: Genius... Delete this! I'm not familiar with this technique, looks awesome. Unfortunately I don't understand the effect of this. SPELL_HASH_TREE_OF_LIFE is not used in any statement...
    sp = CheckAndReturnSpellEntry(34123);
    if (sp != NULL)
        sp->custom_NameHash = 0;*/

    // Druid - Omen of Clarity
    sp = CheckAndReturnSpellEntry(16864);
    if (sp != NULL)
    {
        sp->procChance = 6; //procchance dynamic. 3ppm
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_CAST_SPELL;
    }

    // Druid - Natural Perfection
    sp = CheckAndReturnSpellEntry(33881);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
    sp = CheckAndReturnSpellEntry(33882);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
    sp = CheckAndReturnSpellEntry(33883);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CRIT_HIT_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM;

    // Druid - Intensity
    sp = CheckAndReturnSpellEntry(17106);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    sp = CheckAndReturnSpellEntry(17107);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    sp = CheckAndReturnSpellEntry(17108);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    // Druid - Natural Shapeshifter
    sp = CheckAndReturnSpellEntry(16833);
    if (sp != NULL)
        sp->DurationIndex = 0;
    sp = CheckAndReturnSpellEntry(16834);
    if (sp != NULL)
        sp->DurationIndex = 0;
    sp = CheckAndReturnSpellEntry(16835);
    if (sp != NULL)
        sp->DurationIndex = 0;

    sp = CheckAndReturnSpellEntry(61177); // Northrend Inscription Research
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_NULL;
        //sp->EffectBaseDice[1] = 0;
        sp->EffectBasePoints[1] = 0;
        sp->EffectImplicitTargetA[1] = 0;
        sp->EffectDieSides[1] = 0;
    }
    sp = CheckAndReturnSpellEntry(61288); // Minor Inscription Research
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_NULL;
        //sp->EffectBaseDice[1] = 0;
        sp->EffectBasePoints[1] = 0;
        sp->EffectImplicitTargetA[1] = 0;
        sp->EffectDieSides[1] = 0;
    }
    sp = CheckAndReturnSpellEntry(60893); // Northrend Alchemy Research
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_NULL;
        //sp->EffectBaseDice[1] = 0;
        sp->EffectBasePoints[1] = 0;
        sp->EffectImplicitTargetA[1] = 0;
        sp->EffectDieSides[1] = 0;
    }
    sp = CheckAndReturnSpellEntry(46097); // Brutal Totem of Survival
    if (sp != NULL)
    {
        sp->EffectSpellClassMask[0][0] = 0x00100000 | 0x10000000 | 0x80000000;
        sp->EffectSpellClassMask[0][1] = 0x08000000;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_SELF;
    }
    sp = CheckAndReturnSpellEntry(43860); // Totem of Survival
    if (sp != NULL)
    {
        sp->EffectSpellClassMask[0][0] = 0x00100000 | 0x10000000 | 0x80000000;
        sp->EffectSpellClassMask[0][1] = 0x08000000;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_SELF;
    }
    sp = CheckAndReturnSpellEntry(43861); // Merciless Totem of Survival
    if (sp != NULL)
    {
        sp->EffectSpellClassMask[0][0] = 0x00100000 | 0x10000000 | 0x80000000;
        sp->EffectSpellClassMask[0][1] = 0x08000000;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_SELF;
    }
    sp = CheckAndReturnSpellEntry(43862); // Vengeful Totem of Survival
    if (sp != NULL)
    {
        sp->EffectSpellClassMask[0][0] = 0x00100000 | 0x10000000 | 0x80000000;
        sp->EffectSpellClassMask[0][1] = 0x08000000;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_SELF;
    }
    sp = CheckAndReturnSpellEntry(60564); // Savage Gladiator's Totem of Survival
    if (sp != NULL)
    {
        sp->EffectSpellClassMask[0][0] = 0x00100000 | 0x10000000 | 0x80000000;
        sp->EffectSpellClassMask[0][1] = 0x08000000;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_SELF;
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 60565; // Savage Magic
    }
    sp = CheckAndReturnSpellEntry(60571); // Hateful Gladiator's Totem of Survival
    if (sp != NULL)
    {
        sp->EffectSpellClassMask[0][0] = 0x00100000 | 0x10000000 | 0x80000000;
        sp->EffectSpellClassMask[0][1] = 0x08000000;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_SELF;
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 60566; // Hateful Magic
    }
    sp = CheckAndReturnSpellEntry(60572); // Deadly Gladiator's Totem of Survival
    if (sp != NULL)
    {
        sp->EffectSpellClassMask[0][0] = 0x00100000 | 0x10000000 | 0x80000000;
        sp->EffectSpellClassMask[0][1] = 0x08000000;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_SELF;
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 60567; // Deadly Magic
    }
    sp = CheckAndReturnSpellEntry(60567); // Deadly Magic
    if (sp != NULL)
        sp->EffectImplicitTargetA[1] = EFF_TARGET_SELF;
    sp = CheckAndReturnSpellEntry(46098); // Brutal Totem of Third WInd
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectSpellClassMask[0][0] = 0x00000080;
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 46099; // Brutal Gladiator's Totem of the Third Wind
    }
    sp = CheckAndReturnSpellEntry(34138); // Totem of the Third Wind
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectSpellClassMask[0][0] = 0x00000080;
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 34132; // Gladiator's Totem of the Third Wind
    }
    sp = CheckAndReturnSpellEntry(42370); // Merciless Totem of the Third WInd
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectSpellClassMask[0][0] = 0x00000080;
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 42371; // Merciless Gladiator's Totem of the Third Wind
    }
    sp = CheckAndReturnSpellEntry(43728); // Vengeful Totem of Third WInd
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectSpellClassMask[0][0] = 0x00000080;
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 43729; // Vengeful Gladiator's Totem of the Third Wind
    }
    //////////////////////////////////////////
    // ITEMS                                //
    //////////////////////////////////////////

    // Insert items spell fixes here

    //Compact Harvest Reaper
    sp = CheckAndReturnSpellEntry(4078);
    if (sp != NULL)
    {
        sp->DurationIndex = 6;
    }

    //Graccu's Mince Meat Fruitcake
    sp = CheckAndReturnSpellEntry(25990);
    if (sp != NULL)
    {
        sp->EffectAmplitude[1] = 1000;
    }

    //Extract Gas
    sp = CheckAndReturnSpellEntry(30427);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_DUMMY;
    }

    //Relic - Idol of the Unseen Moon
    sp = CheckAndReturnSpellEntry(43739);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    //Lunar Grace - Idol of the Unseen Moon proc
    sp = CheckAndReturnSpellEntry(43740);
    if (sp != NULL)
    {
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_MOONFIRE;
    }

    //Relic - Idol of Terror
    sp = CheckAndReturnSpellEntry(43737);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 10001; //block proc when is already active.. (Primal Instinct duration = 10 sec)
        sp->procFlags = PROC_ON_CAST_SPELL | static_cast<uint32>(PROC_TARGET_SELF);
    }

    //Primal Instinct - Idol of Terror proc
    sp = CheckAndReturnSpellEntry(43738);
    if (sp != NULL)
    {
        sp->custom_self_cast_only = true;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_MANGLE__CAT_;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_MANGLE__BEAR_;
    }

    //Tome of Fiery Redemption
    sp = CheckAndReturnSpellEntry(37197);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    //Thunderfury
    sp = CheckAndReturnSpellEntry(21992);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[2] = EFF_TARGET_ALL_ENEMIES_AROUND_CASTER; // cebernic: for enemies not self
    }

    //Energized
    sp = CheckAndReturnSpellEntry(43750);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    //Spell Haste Trinket
    sp = CheckAndReturnSpellEntry(33297);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL | static_cast<uint32>(PROC_TARGET_SELF);

    //Enchant Weapon - Deathfrost
    sp = CheckAndReturnSpellEntry(46662);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }

    // Sigil of the Unfaltering Knight
    sp = CheckAndReturnSpellEntry(62147);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_ICY_TOUCH;
        sp->custom_proc_interval = 45000;
    }

    // Deadly Aggression - triggered by Deadly Gladiator's Relic/Idol/Libram/Totem
    sp = CheckAndReturnSpellEntry(60549);
    if (sp != NULL)
    {
        // effect 1 and 2 are the same... dunno why
        sp->Effect[1] = SPELL_EFFECT_NULL;
    }

    // Furious Gladiator's Libram of Fortitude - triggered by LK Arena 4 Gladiator's Relic/Idol/Libram/Totem
    sp = CheckAndReturnSpellEntry(60551);
    if (sp != NULL)
    {
        // effect 1 and 2 are the same... dunno why
        sp->Effect[1] = SPELL_EFFECT_NULL;
    }

    // Relentless Aggression - triggered by LK Arena 5 Gladiator's Relic/Idol/Libram/Totem
    sp = CheckAndReturnSpellEntry(60553);
    if (sp != NULL)
    {
        // effect 1 and 2 are the same... dunno why
        sp->Effect[1] = SPELL_EFFECT_NULL;
    }

    // Savage Aggression - triggered by Savage Gladiator's Relic/Idol/Libram/Totem
    sp = CheckAndReturnSpellEntry(60544);
    if (sp != NULL)
    {
        // effect 1 and 2 are the same... dunno why
        sp->Effect[1] = SPELL_EFFECT_NULL;
    }

    // Sigil of Haunted Dreams
    sp = CheckAndReturnSpellEntry(60826);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPECIFIC_SPELL;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_BLOOD_STRIKE;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_HEART_STRIKE;
        sp->custom_proc_interval = 45000;
    }

    // Vestige of Haldor
    sp = CheckAndReturnSpellEntry(60306);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 45000;
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_RANGED_ATTACK;
    }

    // Forge Ember
    sp = CheckAndReturnSpellEntry(60473);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 45000;
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    // Mirror of Truth
    sp = CheckAndReturnSpellEntry(33648);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 45000;
        sp->procFlags = PROC_ON_CRIT_ATTACK;
    }

    // Majestic Dragon Figurine
    sp = CheckAndReturnSpellEntry(60524);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    // Flow of Knowledge
    sp = CheckAndReturnSpellEntry(62114);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 45000;
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    // Embrace of the Spider
    sp = CheckAndReturnSpellEntry(60490);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 45000;
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    // Anvil of Titans
    sp = CheckAndReturnSpellEntry(62115);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 45000;
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_RANGED_ATTACK;
    }

    // Soul of the Dead
    sp = CheckAndReturnSpellEntry(60537);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 45000;
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    // Illustration of the Dragon Soul
    sp = CheckAndReturnSpellEntry(60485);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    // Grim Toll
    sp = CheckAndReturnSpellEntry(60436);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 45000;
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_RANGED_ATTACK;
    }

    // Fury of the Five Flights
    sp = CheckAndReturnSpellEntry(60313);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_RANGED_ATTACK;
    }

    // Bandit's Insignia
    sp = CheckAndReturnSpellEntry(60442);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 45000;
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_RANGED_ATTACK;
    }

    // Meteorite Whetstone
    sp = CheckAndReturnSpellEntry(60301);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 45000;
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_RANGED_ATTACK;
    }

    // Sonic Booster
    sp = CheckAndReturnSpellEntry(54707);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 60000;
        sp->procFlags = PROC_ON_MELEE_ATTACK;
    }

    //Totem of the Third Wind - bad range
    SpellEntry* sp_healing_wave = CheckAndReturnSpellEntry(8004);
    sp = CheckAndReturnSpellEntry(34132);
    if (sp != NULL)
    {
        sp->rangeIndex = sp_healing_wave->rangeIndex;
    }
    sp = CheckAndReturnSpellEntry(42371);
    if (sp != NULL)
    {
        sp->rangeIndex = sp_healing_wave->rangeIndex;
    }
    sp = CheckAndReturnSpellEntry(43729);
    if (sp != NULL)
    {
        sp->rangeIndex = sp_healing_wave->rangeIndex;
    }
    sp = CheckAndReturnSpellEntry(46099);
    if (sp != NULL)
    {
        sp->rangeIndex = sp_healing_wave->rangeIndex;
    }

    //Moonkin Starfire Bonus
    sp = CheckAndReturnSpellEntry(46832);
    if (sp != NULL)
    {
        sp->procFlags = uint32(PROC_ON_CAST_SPELL | PROC_TARGET_SELF);
    }

    // Eye of Acherus, our phase shift mode messes up the control :/
    sp = CheckAndReturnSpellEntry(51852);
    if (sp != NULL)
        sp->Effect[0] = SPELL_EFFECT_NULL;

    // Band of the Eternal Sage
    sp = CheckAndReturnSpellEntry(35083);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    // Band of the Eternal Restorer
    sp = CheckAndReturnSpellEntry(35086);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    // Ashtongue Talisman of Shadows
    sp = CheckAndReturnSpellEntry(40478);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    // Ashtongue Talisman of Swiftness
    sp = CheckAndReturnSpellEntry(40485);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    // Ashtongue Talisman of Valor
    sp = CheckAndReturnSpellEntry(40458);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    // Memento of Tyrande
    sp = CheckAndReturnSpellEntry(37655);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    // Ashtongue Talisman of Insight
    sp = CheckAndReturnSpellEntry(40482);
    if (sp != NULL)
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;

    //Ashtongue Talisman of Equilibrium
    // DankoDJ: To set the same value several times makes no sense!
    sp = CheckAndReturnSpellEntry(40442);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procChance = 40;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectTriggerSpell[0] = 40452;
        sp->maxstack = 1;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procChance = 25;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectTriggerSpell[1] = 40445;
        sp->maxstack = 1;
        sp->Effect[2] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[2] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procChance = 25;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectTriggerSpell[2] = 40446;
        sp->maxstack = 1;
    }

    //Ashtongue Talisman of Acumen
    // DankoDJ: To set the same value several times makes no sense!
    sp = CheckAndReturnSpellEntry(40438);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procChance = 10;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectTriggerSpell[0] = 40441;
        sp->maxstack = 1;
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procChance = 10;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectTriggerSpell[1] = 40440;
        sp->maxstack = 1;
    }
    // Drums of war targets surrounding party members instead of us
    sp = CheckAndReturnSpellEntry(35475);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_ALL_PARTY;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_ALL_PARTY;
        sp->EffectImplicitTargetA[2] = 0;
        sp->EffectImplicitTargetB[0] = 0;
        sp->EffectImplicitTargetB[1] = 0;
        sp->EffectImplicitTargetB[2] = 0;
    }

    // Drums of Battle targets surrounding party members instead of us
    sp = CheckAndReturnSpellEntry(35476);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_ALL_PARTY;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_ALL_PARTY;
        sp->EffectImplicitTargetA[2] = 0;
        sp->EffectImplicitTargetB[0] = 0;
        sp->EffectImplicitTargetB[1] = 0;
        sp->EffectImplicitTargetB[2] = 0;
    }

    // Drums of Panic targets surrounding creatures instead of us
    sp = CheckAndReturnSpellEntry(35474);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_ALL_ENEMIES_AROUND_CASTER;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_ALL_ENEMIES_AROUND_CASTER;
        sp->EffectImplicitTargetA[2] = 0;
        sp->EffectImplicitTargetB[0] = 0;
        sp->EffectImplicitTargetB[1] = 0;
        sp->EffectImplicitTargetB[2] = 0;
    }

    // Drums of Restoration targets surrounding party members instead of us
    sp = CheckAndReturnSpellEntry(35478);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_ALL_PARTY;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_ALL_PARTY;
        sp->EffectImplicitTargetA[2] = 0;
        sp->EffectImplicitTargetB[0] = 0;
        sp->EffectImplicitTargetB[1] = 0;
        sp->EffectImplicitTargetB[2] = 0;
    }
    // Drums of Speed targets surrounding party members instead of us
    sp = CheckAndReturnSpellEntry(35477);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_ALL_PARTY;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_ALL_PARTY;
        sp->EffectImplicitTargetA[2] = 0;
        sp->EffectImplicitTargetB[0] = 0;
        sp->EffectImplicitTargetB[1] = 0;
        sp->EffectImplicitTargetB[2] = 0;
    }

    // Dragonspine Trophy
    sp = CheckAndReturnSpellEntry(34774);
    if (sp != NULL)
    {
        sp->procChance = 6;
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_RANGED_ATTACK;
        sp->custom_proc_interval = 30000;
    }

    //Serpent-Coil Braid
    sp = CheckAndReturnSpellEntry(37447);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procChance = 100;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectTriggerSpell[0] = 37445;
        sp->maxstack = 1;
    }

    // Band of the Eternal Champion
    sp = CheckAndReturnSpellEntry(35080);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_RANGED_ATTACK;
        sp->custom_proc_interval = 60000;
    }
    // Band of the Eternal Sage
    sp = CheckAndReturnSpellEntry(35083);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_proc_interval = 60000;
    }
    // Band of the Eternal Restorer
    sp = CheckAndReturnSpellEntry(35086);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_proc_interval = 60000;
    }
    // Band of the Eternal Defender
    sp = CheckAndReturnSpellEntry(35077);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_ON_SPELL_HIT_VICTIM | PROC_ON_RANGED_ATTACK_VICTIM;
        sp->custom_proc_interval = 60000;
    }

    //Item Set: Malorne Harness
    sp = CheckAndReturnSpellEntry(37306);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;       // DankoDJ: original 20
    }
    sp = CheckAndReturnSpellEntry(37311);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;       // DankoDJ: original 20
    }

    //Item Set: Deathmantle
    sp = CheckAndReturnSpellEntry(37170);
    if (sp != NULL)
    {
        sp->procChance = 4;
        sp->procFlags = PROC_ON_MELEE_ATTACK;       // DankoDJ: original 20
    }

    //Item Set: Netherblade
    sp = CheckAndReturnSpellEntry(37168);
    if (sp != NULL)
    {
        //sp->procFlags = PROC_ON_CAST_SPELL; Need new flag - PROC_ON_FINISH_MOVE;
    }

    //Item Set: Tirisfal Regalia
    sp = CheckAndReturnSpellEntry(37443);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    //Item Set: Avatar Regalia
    sp = CheckAndReturnSpellEntry(37600);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    //Item Set: Incarnate Raiment
    sp = CheckAndReturnSpellEntry(37568);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    //Item Set: Voidheart Raiment
    sp = CheckAndReturnSpellEntry(37377);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_proc_interval = 20;
        sp->EffectTriggerSpell[0] = 37379;
    }
    sp = CheckAndReturnSpellEntry(39437);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_proc_interval = 20;
        sp->EffectTriggerSpell[0] = 37378;
    }

    //Item Set: Cataclysm Raiment
    sp = CheckAndReturnSpellEntry(37227);
    if (sp != NULL)
    {
        sp->custom_proc_interval = 60000;
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    //Item Set: Cataclysm Regalia
    sp = CheckAndReturnSpellEntry(37228);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }
    sp = CheckAndReturnSpellEntry(37237);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    //Item Set: Cataclysm Harness
    sp = CheckAndReturnSpellEntry(37239);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;       // DankoDJ: original 20
    }

    //Item Set: Cyclone Regalia
    sp = CheckAndReturnSpellEntry(37213);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    //Item Set: Lightbringer Battlegear
    sp = CheckAndReturnSpellEntry(38427);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;       // DankoDJ: original 20
    }

    //Item Set: Crystalforge Battlegear
    sp = CheckAndReturnSpellEntry(37195);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    //Item Set: Crystalforge Raiment
    sp = CheckAndReturnSpellEntry(37189);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_SPELL_CRIT_HIT;
        sp->custom_proc_interval = 60000;
    }
    sp = CheckAndReturnSpellEntry(37188);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
    }

    //Item Set: Destroyer Armor
    sp = CheckAndReturnSpellEntry(37525);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM;
    }

    //Item Set: Warbringer Armor
    sp = CheckAndReturnSpellEntry(37516);
    if (sp != NULL)
    {
        sp->procChance = 100;
    }

    //Item Set: Shadowcraft Armor & Darkmantle Armor
    sp = CheckAndReturnSpellEntry(27787);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK_VICTIM;
        sp->procChance = 7;
    }

    // Item Set: Warlock Tier 7 Heroes' Plagueheart Garb
    sp = CheckAndReturnSpellEntry(60172);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 100;
    }

    //all Drums
    sp = CheckAndReturnSpellEntry(35474);
    if (sp != NULL)
        sp->RequiredShapeShift = 0;
    sp = CheckAndReturnSpellEntry(35475);
    if (sp != NULL)
        sp->RequiredShapeShift = 0;
    sp = CheckAndReturnSpellEntry(35476);
    if (sp != NULL)
        sp->RequiredShapeShift = 0;
    sp = CheckAndReturnSpellEntry(35477);
    if (sp != NULL)
        sp->RequiredShapeShift = 0;
    sp = CheckAndReturnSpellEntry(35478);
    if (sp != NULL)
        sp->RequiredShapeShift = 0;

    //this an on equip item spell(2824) :  ice arrow(29501)
    sp = CheckAndReturnSpellEntry(29501);
    if (sp != NULL)
    {
        sp->procChance = 30;//some say it is triggered every now and then
        sp->procFlags = PROC_ON_RANGED_ATTACK;
    }

    //Purify helboar meat
    sp = CheckAndReturnSpellEntry(29200);
    if (sp != NULL)
    {
        sp->Reagent[1] = 0;
        sp->ReagentCount[1] = 0;
    }

    //Thorium Grenade
    sp = CheckAndReturnSpellEntry(19769);
    if (sp != NULL)
    {
        sp->InterruptFlags |= ~(CAST_INTERRUPT_ON_MOVEMENT);
    }

    //M73 Frag Grenade
    sp = CheckAndReturnSpellEntry(13808);
    if (sp != NULL)
    {
        sp->InterruptFlags |= ~(CAST_INTERRUPT_ON_MOVEMENT);
    }

    //Iron Grenade
    sp = CheckAndReturnSpellEntry(4068);
    if (sp != NULL)
    {
        sp->InterruptFlags |= ~(CAST_INTERRUPT_ON_MOVEMENT);
    }

    //Frost Grenade
    sp = CheckAndReturnSpellEntry(39965);
    if (sp != NULL)
    {
        sp->InterruptFlags |= ~(CAST_INTERRUPT_ON_MOVEMENT);
    }

    //Adamantine Grenade
    sp = CheckAndReturnSpellEntry(30217);
    if (sp != NULL)
    {
        sp->InterruptFlags |= ~(CAST_INTERRUPT_ON_MOVEMENT);
    }

    //Swordguard Embroidery
    sp = CheckAndReturnSpellEntry(55776);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;       // DankoDJ: original 20
        sp->custom_proc_interval = 60000;
    }

    //Darkglow Embroidery
    sp = CheckAndReturnSpellEntry(55768);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_proc_interval = 60000;
    }
    ///////////////////////////////////////////////////////////////
    // Trinket Fixes        Please keep nice and clean :)
    ///////////////////////////////////////////////////////////////

    // Citrine Pendant of Golden Healing
    sp = CheckAndReturnSpellEntry(25608);        //    http://www.wowhead.com/?item=20976
    if (sp != NULL)
    {
        //Overrides any spell coefficient calculation - DBCStores.h
        sp->Dspell_coef_override = 0;    //DD&DH
        sp->OTspell_coef_override = 0;    //HOT&DOT
    }
    //Barricade of Eternity
    sp = CheckAndReturnSpellEntry(40475);        //    http://www.wowhead.com/?item=40475
    if (sp != NULL)
        sp->procChance = 50;    // Sets change to proc

    //Figurine - Shadowsong Panther
    sp = CheckAndReturnSpellEntry(46784);        //    http://www.wowhead.com/?item=35702
    if (sp != NULL)
        sp->AttributesEx |= ATTRIBUTESEX_NOT_BREAK_STEALTH;

    // Infernal Protection
    sp = CheckAndReturnSpellEntry(36488);            //    http://www.wowhead.com/?spell=36488
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_FRIEND;

    // Deadly Throw Interrupt
    sp = CheckAndReturnSpellEntry(32748);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_RANGED_ATTACK | PROC_ON_CAST_SPELL;
    }

    //Sundial of the Exiled
    sp = CheckAndReturnSpellEntry(60063);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 60064;      // DankoDJ: sp->EffectTriggerSpell[0] is alread 60064 in spell.dbc !?
        sp->procFlags = PROC_ON_SPELL_HIT;
        sp->custom_proc_interval = 45000;
    }

    //Je'Tze's Bell
    sp = CheckAndReturnSpellEntry(49622);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 49623;      // DankoDJ: sp->EffectTriggerSpell[0] is alread 49623 in spell.dbc !?
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_proc_interval = 45000;
    }

    //Tears of Bitter Anguish
    sp = CheckAndReturnSpellEntry(58901);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 58904;      // DankoDJ: sp->EffectTriggerSpell[0] is alread 58904 in spell.dbc !?
        sp->procFlags = PROC_ON_RANGED_CRIT_ATTACK | PROC_ON_CRIT_ATTACK;
        sp->custom_proc_interval = 60000;
    }

    //Embrace of the Spider
    sp = CheckAndReturnSpellEntry(60490);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 60492;      // DankoDJ: sp->EffectTriggerSpell[0] is alread 60492 in spell.dbc !?
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_proc_interval = 30000;
    }

    //Dying Curse
    sp = CheckAndReturnSpellEntry(60493);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 60494;       // DankoDJ: sp->EffectTriggerSpell[0] is alread 60494 in spell.dbc !?
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_proc_interval = 45000;
    }

    //Fury of the Five Flights
    sp = CheckAndReturnSpellEntry(60313);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 60314;      // DankoDJ: sp->EffectTriggerSpell[0] is alread 60314 in spell.dbc !?
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_RANGED_ATTACK;
        sp->maxstack = 20;
    }

    //Vial of the Sunwell
    sp = CheckAndReturnSpellEntry(45059);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    //Pendant of the Violet Eye
    sp = CheckAndReturnSpellEntry(29601);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL | static_cast<uint32>(PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(35095);
    if (sp != NULL)
    {
        sp->custom_self_cast_only = true;
        sp->procChance = 100;
    }

    sp = CheckAndReturnSpellEntry(38332);        // Ribbon of Sacrifice
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    sp = CheckAndReturnSpellEntry(40475);        // Black temple melee trinket
    if (sp != NULL)
        sp->procChance = 5;

    sp = CheckAndReturnSpellEntry(32642);        // Sporegarr - Petrified Lichen Guard
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_BLOCK_VICTIM;
    }

    //Flow of Knowledge
    sp = CheckAndReturnSpellEntry(62114);
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[1] = 60064;      // DankoDJ: EffectTriggerSpell[0] is 60064 in spell.dbc !?
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_proc_interval = 45000;
    }

    //Majestic Dragon Figurine
    sp = CheckAndReturnSpellEntry(60524);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_proc_interval = 100;
    }

    //Illustration of the Dragon Soul
    sp = CheckAndReturnSpellEntry(60485);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_proc_interval = 100;
    }

    //////////////////////////////////////////
    // BOSSES                                //
    //////////////////////////////////////////

    // Insert boss spell fixes here

    // Major Domo - Magic Reflection
    sp = CheckAndReturnSpellEntry(20619);
    if (sp != NULL)
    {
        for (uint8 i = 0; i < 3; i++)
        {
            if (sp->EffectImplicitTargetA[i] > 0)
                sp->EffectImplicitTargetA[i] = EFF_TARGET_ALL_FRIENDLY_IN_AREA;
            if (sp->EffectImplicitTargetB[i] > 0)
                sp->EffectImplicitTargetB[i] = EFF_TARGET_ALL_FRIENDLY_IN_AREA;
        }
    }

    // Major Domo - Damage Shield
    sp = CheckAndReturnSpellEntry(21075);
    if (sp != NULL)
    {
        for (uint8 i = 0; i < 3; i++)
        {
            if (sp->EffectImplicitTargetA[i] > 0)
                sp->EffectImplicitTargetA[i] = EFF_TARGET_ALL_FRIENDLY_IN_AREA;
            if (sp->EffectImplicitTargetB[i] > 0)
                sp->EffectImplicitTargetB[i] = EFF_TARGET_ALL_FRIENDLY_IN_AREA;
        }
    }

    // Dark Glare
    sp = CheckAndReturnSpellEntry(26029);
    if (sp != NULL)
        sp->cone_width = 15.0f; // 15 degree cone

    // Drain Power (Malacrass) // bugged - the charges fade even when refreshed with new ones. This makes them everlasting.
    sp = CheckAndReturnSpellEntry(44131);
    if (sp != NULL)
        sp->DurationIndex = 21;
    sp = CheckAndReturnSpellEntry(44132);
    if (sp != NULL)
        sp->DurationIndex = 21;

    // Zul'jin spell, proc from Creeping Paralysis
    sp = CheckAndReturnSpellEntry(43437);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = 0;
        sp->EffectImplicitTargetA[1] = 0;
    }

    //Bloodboil
    sp = CheckAndReturnSpellEntry(42005);
    if (sp != NULL)
    {
        sp->MaxTargets = 5;
    }

    //Acidic Wound
    sp = CheckAndReturnSpellEntry(40484);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;   // DankoDJ: original 20
    }

    //Inject Poison
    sp = CheckAndReturnSpellEntry(44599);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;   // DankoDJ: original 20
    }
    sp = CheckAndReturnSpellEntry(46046);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;   // DankoDJ: original 20
    }

    //Doom
    sp = CheckAndReturnSpellEntry(31347);
    if (sp != NULL)
    {
        sp->MaxTargets = 1;
    }
    //Shadow of Death
    sp = CheckAndReturnSpellEntry(40251);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 0;
    }

    sp = CheckAndReturnSpellEntry(9036);
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 20584;
    }

    sp = CheckAndReturnSpellEntry(24379);   //bg Restoration
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[0] = 23493;
    }

    sp = CheckAndReturnSpellEntry(23493);   //bg Restoration
    if (sp != NULL)
    {
        sp->EffectTriggerSpell[0] = 24379;
    }

    sp = CheckAndReturnSpellEntry(5246);    // why self?
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 20511; // cebernic: this just real spell
        sp->EffectImplicitTargetA[0] = EFF_TARGET_NONE;
    }

    //////////////////////////////////////////
    // DEATH KNIGHT                            //
    //////////////////////////////////////////

    // Insert Death Knight spells here ---- Made by Alice

    // Mark of Blood
    // Necessary to proper create entry on m_chargeSpells
    sp = CheckAndReturnSpellEntry(49005);
    if (sp != NULL)
        sp->procFlags = PROC_ON_MELEE_ATTACK | PROC_ON_RANGED_ATTACK | PROC_ON_SPELL_HIT;

    // Unholy Aura - Ranks 1
    sp = CheckAndReturnSpellEntry(50391);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS;
        sp->Effect[0] = SPELL_EFFECT_APPLY_GROUP_AREA_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 50392;
        sp->Effect[1] = SPELL_EFFECT_APPLY_GROUP_AREA_AURA;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_SELF;
    }
    // Unholy Aura - Ranks 2
    sp = CheckAndReturnSpellEntry(50392);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS;
        sp->Effect[0] = SPELL_EFFECT_APPLY_GROUP_AREA_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[1] = 50392;
        sp->Effect[1] = SPELL_EFFECT_APPLY_GROUP_AREA_AURA;
        sp->EffectImplicitTargetA[1] = EFF_TARGET_SELF;
    }

    // MIND FREEZE
    sp = dbcSpell.LookupEntryForced(47528);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_INTERRUPT_CAST;
    }

    //   Blood Presence
    sp = CheckAndReturnSpellEntry(48266);
    if (sp != NULL)
    {
        sp->custom_BGR_one_buff_from_caster_on_self = SPELL_TYPE3_DEATH_KNIGHT_AURA;
    }

    //    Empower Rune Weapon
    sp = CheckAndReturnSpellEntry(47568);
    if (sp != NULL)
    {
        sp->Effect[2] = SPELL_EFFECT_ACTIVATE_RUNES;
        sp->EffectBasePoints[2] = 1;
        sp->EffectMiscValue[2] = RUNE_UNHOLY;
    }

    // Frost Presence
    sp = CheckAndReturnSpellEntry(48263);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_BASE_RESISTANCE_PCT;
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT;
        sp->EffectBasePoints[1] = 9;
        sp->EffectApplyAuraName[2] = SPELL_AURA_MOD_DAMAGE_TAKEN;
        sp->custom_BGR_one_buff_from_caster_on_self = SPELL_TYPE3_DEATH_KNIGHT_AURA;
    }

    //    Unholy Presence
    sp = CheckAndReturnSpellEntry(48265);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_HASTE;
        sp->EffectBasePoints[0] = 14;
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_INCREASE_SPEED;
        sp->EffectBasePoints[1] = 14;
        sp->custom_BGR_one_buff_from_caster_on_self = SPELL_TYPE3_DEATH_KNIGHT_AURA;
    }

    // DEATH AND DECAY
    sp = dbcSpell.LookupEntryForced(49937);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DAMAGE;
        sp->Effect[0] = SPELL_EFFECT_PERSISTENT_AREA_AURA;
    }

    sp = dbcSpell.LookupEntryForced(49936);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DAMAGE;
        sp->Effect[0] = SPELL_EFFECT_PERSISTENT_AREA_AURA;
    }

    sp = dbcSpell.LookupEntryForced(49938);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DAMAGE;
        sp->Effect[0] = SPELL_EFFECT_PERSISTENT_AREA_AURA;
    }

    sp = dbcSpell.LookupEntryForced(43265);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DAMAGE;
        sp->Effect[0] = SPELL_EFFECT_PERSISTENT_AREA_AURA;
    }

    // Death Grip
    sp = CheckAndReturnSpellEntry(49576);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_DUMMY;
    }

    // Runic Empowerment
    /*sp = dbcSpell.LookupEntryForced(81229);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 81229;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_DEATH_STRIKE;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_FROST_STRIKE;
        sp->custom_ProcOnNameHash[2] = SPELL_HASH_DEATH_COIL;
        sp->procChance = 45;
    }*/

    // Vengeance
    sp = dbcSpell.LookupEntryForced(93099);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 76691;
    }

    ///////////////////////////////////////////////////////////
    //    Acherus Deatcharger
    ///////////////////////////////////////////////////////////
    sp = CheckAndReturnSpellEntry(48778);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOUNTED;
        sp->EffectApplyAuraName[1] = SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED;
        sp->EffectBasePoints[1] = 99;
    }

    ///////////////////////////////////////////////////////////
    //    Path of Frost
    ///////////////////////////////////////////////////////////
    sp = CheckAndReturnSpellEntry(3714);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_WATER_WALK;
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
    }

    // Rune Strike
    sp = CheckAndReturnSpellEntry(56815);
    if (sp != NULL)
    {
        sp->Attributes |= ATTRIBUTES_CANT_BE_DPB;
    }

    CreateDummySpell(56817);
    sp = CheckAndReturnSpellEntry(56817);
    if (sp != NULL)
    {
        sp->DurationIndex = 28;
        sp->Effect[0] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
    }

    //Frost Strike
    sp = CheckAndReturnSpellEntry(49143);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_CANT_BE_DPB;
    }
    sp = CheckAndReturnSpellEntry(51416);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_CANT_BE_DPB;
    }
    sp = CheckAndReturnSpellEntry(51417);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_CANT_BE_DPB;
    }
    sp = CheckAndReturnSpellEntry(51418);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_CANT_BE_DPB;
    }
    sp = CheckAndReturnSpellEntry(51419);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_CANT_BE_DPB;
    }
    sp = CheckAndReturnSpellEntry(55268);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_CANT_BE_DPB;
    }

    ///////////////////////////////////////////////////////////
    //      Bloodworms - handled in dummy code
    ///////////////////////////////////////////////////////////

    // Bloodworms Rank 1
    sp = CheckAndReturnSpellEntry(49027);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 50452;
    }

    // Bloodworms Rank 2
    sp = CheckAndReturnSpellEntry(49542);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 50452;
    }

    // Bloodworms Rank 3
    sp = CheckAndReturnSpellEntry(49543);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_MELEE_ATTACK;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 50452;
    }


    // Noggenfogger elixir - reduce size effect
    sp = CheckAndReturnSpellEntry(16595);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_SCALE;
        sp->EffectBasePoints[0] = -50;
        sp->maxstack = 1;
    }

    sp = CheckAndReturnSpellEntry(46584);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_DUMMY;
        sp->Effect[1] = SPELL_EFFECT_NULL;
        sp->Effect[2] = SPELL_EFFECT_NULL;
    }

    //PvP Librams of Justice
    //Gladiator's Libram of Justice
    sp = CheckAndReturnSpellEntry(34139);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    //Merciless Gladiator's Libram of Justice
    sp = CheckAndReturnSpellEntry(42368);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    //Vengeful Gladiator's Libram of Justice
    sp = CheckAndReturnSpellEntry(43726);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    //Brutal Gladiator's Libram of Justice
    sp = CheckAndReturnSpellEntry(46092);
    if (sp != NULL)
        sp->procFlags = PROC_ON_CAST_SPELL;

    //Other Librams
    //Libram of Saints Departed and Libram of Zeal
    sp = CheckAndReturnSpellEntry(34262);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL | static_cast<uint32>(PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(34263);
    if (sp != NULL)
    {
        sp->custom_self_cast_only = true;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_JUDGEMENT;
        sp->procChance = 100;
    }

    //Libram of Avengement
    sp = CheckAndReturnSpellEntry(34258);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL | static_cast<uint32>(PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(34260);
    if (sp != NULL)
    {
        sp->custom_self_cast_only = true;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_JUDGEMENT;
        sp->procChance = 100;
    }

    //Libram of Mending
    sp = CheckAndReturnSpellEntry(43741);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL | static_cast<uint32>(PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(43742);
    if (sp != NULL)
    {
        sp->custom_self_cast_only = true;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_HOLY_LIGHT;
        sp->procChance = 100;
    }

    //Libram of Divine Judgement
    sp = CheckAndReturnSpellEntry(43745);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL | static_cast<uint32>(PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(43747);
    if (sp != NULL)
    {
        sp->custom_self_cast_only = true;
        sp->procChance = 100;
    }

    //Stonebreaker's Totem
    sp = CheckAndReturnSpellEntry(43748);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL | static_cast<uint32>(PROC_TARGET_SELF);
    }
    sp = CheckAndReturnSpellEntry(43749);
    if (sp != NULL)
    {
        sp->custom_self_cast_only = true;
        sp->procChance = 100;
    }

    // Recently Bandaged - is debuff
    sp = CheckAndReturnSpellEntry(11196);
    if (sp != NULL)
    {
        sp->Attributes = ATTRIBUTES_IGNORE_INVULNERABILITY;
    }

    sp = CheckAndReturnSpellEntry(44856);        // Bash'ir Phasing Device
    if (sp != NULL)
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_LEAVE_AREA;

    sp = CheckAndReturnSpellEntry(27997);        //Spellsurge
    if (sp != NULL)
    {
        sp->custom_proc_interval = 30000; // Wowhead Comment
        sp->procChance = 3; //Enchantment Text
    }

    sp = CheckAndReturnSpellEntry(24574);        // Zandalarian Hero Badge 24590 24575
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 24590;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
    }

    //Tempfix for Stone Statues
    sp = CheckAndReturnSpellEntry(32253);
    if (sp != NULL)
        sp->DurationIndex = 64;
    sp = CheckAndReturnSpellEntry(32787);
    if (sp != NULL)
        sp->DurationIndex = 64;
    sp = CheckAndReturnSpellEntry(32788);
    if (sp != NULL)
        sp->DurationIndex = 64;
    sp = CheckAndReturnSpellEntry(32790);
    if (sp != NULL)
        sp->DurationIndex = 64;
    sp = CheckAndReturnSpellEntry(32791);
    if (sp != NULL)
        sp->DurationIndex = 64;

    //////////////////////////////////////////////////////
    // GAME-OBJECT SPELL FIXES                          //
    //////////////////////////////////////////////////////

    // Blessing of Zim'Torga
    sp = CheckAndReturnSpellEntry(51729);
    if (sp)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SCRIPTED_OR_SINGLE_TARGET;
    }

    // Blessing of Zim'Abwa
    sp = CheckAndReturnSpellEntry(51265);
    if (sp)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SCRIPTED_OR_SINGLE_TARGET;
    }

    // Blessing of Zim'Rhuk
    sp = CheckAndReturnSpellEntry(52051);
    if (sp)
    {
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SCRIPTED_OR_SINGLE_TARGET;
    }

    // Ritual of Summoning summons a GameObject that triggers an inexistant spell.
    // This will copy an existant Summon Player spell used for another Ritual Of Summoning
    // to the one taught by Warlock trainers.
    sp = CheckAndReturnSpellEntry(7720);
    if (sp)
    {
        const uint32 ritOfSummId = 62330;
        CreateDummySpell(ritOfSummId);
        SpellEntry * ritOfSumm = dbcSpell.LookupEntryForced(ritOfSummId);
        if (ritOfSumm != NULL)
        {
            memcpy(ritOfSumm, sp, sizeof(SpellEntry));
            ritOfSumm->Id = ritOfSummId;
        }
    }
    //Persistent Shield
    sp = CheckAndReturnSpellEntry(26467);
	if (sp)
	{
		sp->EffectTriggerSpell[0] = 26470;
		sp->Attributes |= ATTRIBUTES_NO_VISUAL_AURA | ATTRIBUTES_PASSIVE;
		sp->DurationIndex = 0;
		sp->procFlags = PROC_ON_CAST_SPELL;
	}
    //Gravity Bomb
    sp = CheckAndReturnSpellEntry(63024);
	if (sp)
	{
		sp->EffectBasePoints[0] = 0;
		sp->Effect[1] = SPELL_EFFECT_NULL;
		sp->Effect[2] = SPELL_EFFECT_NULL;
		sp->TargetAuraState = 0;
		sp->casterAuraSpell = 0;
		sp->CasterAuraState = 0;
		sp->CasterAuraStateNot = 0;
		sp->TargetAuraStateNot = 0;
		sp->targetAuraSpell = 0;
		sp->casterAuraSpellNot = 0;
		sp->targetAuraSpellNot = 0;
		sp->Attributes |= ATTRIBUTES_NEGATIVE;
	}
    // War Stomp
    sp = CheckAndReturnSpellEntry(20549);
    if (sp)
    {
        sp->EffectMechanic[0] = MECHANIC_STUNNED;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_ALL_ENEMY_IN_AREA;
        sp->EffectImplicitTargetB[0] = EFF_TARGET_NONE;
    }

    // Fan of knives
    sp = CheckAndReturnSpellEntry(51723);
    if (sp != NULL)
    {
        //sp->Effect[1] = SPELL_EFFECT_TRIGGER_SPELL;
        //sp->EffectTriggerSpell[1] = 52874;
        sp->EffectMechanic[0] = MECHANIC_SHACKLED;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_ALL_ENEMY_IN_AREA;
        sp->EffectImplicitTargetB[0] = EFF_TARGET_NONE;
    }
}

