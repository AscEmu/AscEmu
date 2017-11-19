/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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
#include "crc32.h"
#include "Units/Players/PlayerClasses.hpp"
#include "Server/MainServerDefines.h"
#include "Spell/SpellAuras.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Spell/Definitions/SpellModifierType.h"
#include "Spell/Definitions/SpellInFrontStatus.h"
#include "Spell/Definitions/SpellDamageType.h"
#include "Spell/Definitions/ProcFlags.h"
#include "Spell/Definitions/CastInterruptFlags.h"
#include <Spell/Definitions/AuraInterruptFlags.h>
#include "Spell/Definitions/SpellRanged.h"
#include "Spell/Definitions/SpellCoefficientsFlags.h"
#include "Spell/Definitions/DispelType.h"
#include "Spell/Definitions/SpellMechanics.h"
#include "Spell/Definitions/SpellEffectTarget.h"
#include "Spell/SpellHelpers.h"
#include "SpellCustomizations.hpp"

using ascemu::World::Spell::Helpers::decimalToMask;

void CreateDummySpell(uint32 id)
{
    const char* name = "Dummy Trigger";
    SpellInfo* sp = new SpellInfo;
    memset(sp, 0, sizeof(SpellInfo));
    sp->setId(id);
    sp->setAttributes(ATTRIBUTES_NO_CAST | ATTRIBUTES_NO_VISUAL_AURA); //384
    sp->setAttributesEx(ATTRIBUTESEX_UNK30);    //268435456
    sp->setAttributesExB(ATTRIBUTESEXB_UNK4);   //4
    sp->setCastingTimeIndex(1);
    sp->setProcChance(75);
    sp->setRangeIndex(13);
    sp->setEquippedItemClass(-1);
    sp->setEffect(SPELL_EFFECT_DUMMY, 0);
    sp->setEffectImplicitTargetA(EFF_TARGET_DUEL, 0);
    sp->custom_NameHash = crc32((const unsigned char*)name, (unsigned int)strlen(name));
    sp->setDmg_multiplier(1.0f, 0);
#if VERSION_STRING != Cata
    sp->setStanceBarOrder(-1);
#endif
    sWorld.dummySpellList.push_back(sp);
}

void Modify_EffectBasePoints(SpellInfo* sp)
{
    if (sp == nullptr)
    {
        LOG_ERROR("Something tried to call with an invalid spell pointer!");
        return;
    }

    //Rogue: Poison time fix for 2.3
    if (sp->getId() == 3408)                 // Crippling Poison && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->setEffectBasePoints(3599, 0);
    if (sp->getId() == 5761)                 // Mind-numbing Poison && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->setEffectBasePoints(3599, 0);
    if (sp->getId() == 8679)                 // Instant Poison && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->setEffectBasePoints(3599, 0);
    if (sp->getId() == 2823)                 // Deadly Poison && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->setEffectBasePoints(3599, 0);
    if (sp->getId() == 13219)                // Wound Poison && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->setEffectBasePoints(3599, 0);
    if (sp->getId() == 26785)                // Anesthetic Poison && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->setEffectBasePoints(3599, 0);

    // Zyres: According to the description the weapon damage gets increased from 2 to 12 (depends on the different spell ids)
    if (sp->getId() == 2828 || sp->getId() == 29452 || sp->getId() == 29453 || sp->getId() == 56308) //Sharpen Blade && Effect[0] == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
        sp->setEffectBasePoints(3599, 0);

    // Set the diff. EffectBasePoint from description.
    if (sp->getId() == 11119)     // Ignite Rank 1
        sp->setEffectBasePoints(8, 0);
    if (sp->getId() == 11120)     // Ignite Rank 2
        sp->setEffectBasePoints(16, 0);
    if (sp->getId() == 12846)     // Ignite Rank 3
        sp->setEffectBasePoints(24, 0);
    if (sp->getId() == 12847)     // Ignite Rank 4
        sp->setEffectBasePoints(32, 0);
    if (sp->getId() == 12848)     // Ignite Rank 5
        sp->setEffectBasePoints(40, 0);
}

void Set_missing_spellLevel(SpellInfo* sp)
{
    if (sp == nullptr)
    {
        LOG_ERROR("Something tried to call with an invalid spell pointer!");
        return;
    }

    //stupid spell ranking problem
    if (sp->getSpellLevel() == 0)
    {
        uint32 new_level = 0;

        // 16/03/08 Zyres: just replaced name assignes with spell ids. \todo remove not teachable spells.
        switch (sp->getId())
        {
            // name "Aprentice "
            case 2020:
            case 2155:
            case 2275:
            case 2372:
            case 2551:
            case 2581:
            case 3279:
            case 3911:
            case 4039:
            case 7414:
            case 7733:
            case 8615:
            case 25245:
            case 29535:
            case 33388:
            case 33389:
            case 45375:
            case 51216:
            {
                new_level = 1;
            } break;
            // name "Journeyman "
            case 2021:
            case 2154:
            case 2280:
            case 2373:
            case 2582:
            case 3280:
            case 3412:
            case 3912:
            case 4040:
            case 7415:
            case 7734:
            case 8619:
            case 25246:
            case 33391:
            case 33392:
            case 45376:
            case 64485:
            {
                new_level = 2;
            } break;
            // name "Expert "
            case 2552:
            case 3465:
            case 3539:
            case 3568:
            case 3571:
            case 3812:
            case 3913:
            case 4041:
            case 7416:
            case 7736:
            case 7925:
            case 8620:
            case 19886:
            case 19889:
            case 19903:
            case 28896:
            case 34090:
            case 34092:
            case 45377:
            case 54083:
            case 54254:
            case 54257:
            {
                new_level = 3;
            } break;
            // name "Artisan "
            case 9786:
            case 10249:
            case 10663:
            case 10769:
            case 10847:
            case 11612:
            case 11994:
            case 12181:
            case 12657:
            case 13921:
            case 18249:
            case 18261:
            case 19887:
            case 19890:
            case 19902:
            case 28899:
            case 34091:
            case 34093:
            case 45378:
            {
                new_level = 4;
            } break;
            // name "Master "
            case 12613:
            case 13958:
            case 13970:
            case 13971:
            case 14904:
            case 15024:
            case 15025:
            case 15026:
            case 15027:
            case 17039:
            case 17040:
            case 17041:
            case 18709:
            case 18710:
            case 18767:
            case 18768:
            case 19825:
            case 21935:
            case 21940:
            case 23759:
            case 23760:
            case 23761:
            case 23762:
            case 23784:
            case 23785:
            case 23822:
            case 23823:
            case 23824:
            case 23825:
            case 23826:
            case 23827:
            case 23828:
            case 23829:
            case 23830:
            case 23831:
            case 23832:
            case 23833:
            case 23834:
            case 23835:
            case 23836:
            case 23837:
            case 23838:
            case 23839:
            case 23840:
            case 23841:
            case 23842:
            case 23843:
            case 23844:
            case 24347:
            case 24626:
            case 24627:
            case 26791:
            case 27029:
            case 27235:
            case 27236:
            case 27237:
            case 28030:
            case 28597:
            case 28696:
            case 28901:
            case 29074:
            case 29075:
            case 29076:
            case 29077:
            case 29355:
            case 29460:
            case 29461:
            case 29845:
            case 30351:
            case 31221:
            case 31222:
            case 31223:
            case 31226:
            case 31227:
            case 31665:
            case 31666:
            case 32550:
            case 32679:
            case 33098:
            case 33100:
            case 33360:
            case 33361:
            case 33894:
            case 34130:
            case 34149:
            case 34150:
            case 34485:
            case 34486:
            case 34487:
            case 34488:
            case 34489:
            case 34506:
            case 34507:
            case 34508:
            case 34833:
            case 34834:
            case 34835:
            case 34836:
            case 34837:
            case 34838:
            case 34839:
            case 35702:
            case 35703:
            case 35704:
            case 35705:
            case 35706:
            case 35708:
            case 35874:
            case 35912:
            case 36001:
            case 38734:
            case 39097:
            case 39098:
            case 39099:
            case 40369:
            case 40385:
            case 40388:
            case 43784:
            case 44868:
            case 45379:
            case 45713:
            case 46417:
            case 46418:
            case 46444:
            case 47872:
            case 47873:
            case 47874:
            case 47875:
            case 47876:
            case 47877:
            case 48028:
            case 48411:
            case 48412:
            case 48418:
            case 48420:
            case 48421:
            case 48422:
            case 48729:
            case 48730:
            case 49776:
            case 52143:
            case 52559:
            case 53125:
            case 53662:
            case 53663:
            case 53664:
            case 53665:
            case 53666:
            case 54084:
            case 54255:
            case 54256:
            case 54721:
            case 55188:
            case 55434:
            case 55435:
            case 57853:
            case 58410:
            case 62698:
            case 65022:
            case 66409:
            case 66410:
            case 66417:
            case 66420:
            case 66423:
            case 66457:
            case 66460:
            case 66461:
            case 66677:
            case 66810:
            case 66812:
            case 67551:
            case 69716:
            case 70528:
            {
                new_level = 5;
            } break;
            // name "Grand Master "
            case 45380:
            case 50299:
            case 50301:
            case 50307:
            case 50309:
            case 51293:
            case 51295:
            case 51298:
            case 51301:
            case 51303:
            case 51305:
            case 51308:
            case 51310:
            case 51312:
            case 61464:
            case 64484:
            case 65281:
            case 65282:
            case 65283:
            case 65284:
            case 65285:
            case 65286:
            case 65287:
            case 65288:
            case 65289:
            case 65290:
            case 65291:
            case 65292:
            case 65293:
            {
                new_level = 6;
            } break;
            default:
                break;
        }

        if (new_level != 0)
        {
            uint32 teachspell = 0;
            if (sp->getEffect(0) == SPELL_EFFECT_LEARN_SPELL)
                teachspell = sp->getEffectTriggerSpell(0);
            else if (sp->getEffect(1) == SPELL_EFFECT_LEARN_SPELL)
                teachspell = sp->getEffectTriggerSpell(1);
            else if (sp->getEffect(2) == SPELL_EFFECT_LEARN_SPELL)
                teachspell = sp->getEffectTriggerSpell(2);

            if (teachspell)
            {
                SpellInfo* spellInfo = Spell::checkAndReturnSpellEntry(teachspell);
                if (spellInfo != nullptr)
                {
                    spellInfo->setSpellLevel(new_level);
                    sp->setSpellLevel(new_level);
                }
            }
        }
    }
}

void Modify_AuraInterruptFlags(SpellInfo* sp)
{
    if (sp == nullptr)
    {
        LOG_ERROR("Something tried to call with an invalid spell pointer!");
        return;
    }

    // HACK FIX: Break roots/fear on damage.. this needs to be fixed properly!
    if (!(sp->getAuraInterruptFlags() & AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN))
    {
        for (uint8 z = 0; z < 3; ++z)
        {
            if (sp->getEffectApplyAuraName(z) == SPELL_AURA_MOD_FEAR || sp->getEffectApplyAuraName(z) == SPELL_AURA_MOD_ROOT)
            {
                sp->addAuraInterruptFlags(AURA_INTERRUPT_ON_UNUSED2);
                break;
            }
        }
    }
}

void Modify_RecoveryTime(SpellInfo* sp)
{
    if (sp == nullptr)
    {
        LOG_ERROR("Something tried to call with an invalid spell pointer!");
        return;
    }

    // Description includes
    switch (sp->getId())
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
            sp->setRecoveryTime(1000);
            sp->setCategoryRecoveryTime(1000);
        } break;
        default:
            break;
    }
}

void ApplyNormalFixes()
{
    //Updating spell.dbc

    LogNotice("World : Processing %u spells...", sSpellCustomizations.GetSpellInfoStore()->size());

    SpellInfo* sp = nullptr;

    for (auto it = sSpellCustomizations.GetSpellInfoStore()->begin(); it != sSpellCustomizations.GetSpellInfoStore()->end(); ++it)
    {
        // Read every SpellEntry row
        sp = sSpellCustomizations.GetSpellInfo(it->first);
        if (sp == nullptr)
            continue;

        sp->custom_NameHash = 0;

        float radius = std::max(::GetRadius(sSpellRadiusStore.LookupEntry(sp->getEffectRadiusIndex(0))), ::GetRadius(sSpellRadiusStore.LookupEntry(sp->getEffectRadiusIndex(1))));
        radius = std::max(::GetRadius(sSpellRadiusStore.LookupEntry(sp->getEffectRadiusIndex(2))), radius);
        radius = std::max(GetMaxRange(sSpellRangeStore.LookupEntry(sp->getRangeIndex())), radius);
        sp->custom_base_range_or_radius_sqr = radius * radius;

        sp->ai_target_type = sp->aiTargetType();
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
        sp->custom_SchoolMask = sp->getSchool();
        for (uint8 i = 0; i < SCHOOL_COUNT; ++i)
        {
            if (sp->getSchool() & (1 << i))
            {
                sp->setSchool(i);
                break;
            }
        }

        ARCEMU_ASSERT(sp->getSchool() < SCHOOL_COUNT);

        // correct caster/target aura states
        if (sp->getCasterAuraState() > 1)
            sp->setCasterAuraState(1 << (sp->getCasterAuraState() - 1));

        if (sp->getTargetAuraState() > 1)
            sp->setTargetAuraState(1 << (sp->getTargetAuraState() - 1));


        //there are some spells that change the "damage" value of 1 effect to another : devastate = bonus first then damage
        //this is a total bullshit so remove it when spell system supports effect overwriting
        switch (sp->getId())
        {
            case 20243:     // Devastate Rank 1
            case 30016:     // Devastate Rank 2
            case 30022:     // Devastate Rank 3
            case 47497:     // Devastate Rank 4
            case 47498:     // Devastate Rank 5
            case 57795:     // Devastate
            case 60018:     // Devastate
            case 62317:     // Devastate
            case 69902:     // Devastate
            {
                uint32 temp;
                float ftemp;
                temp = sp->getEffect(1);
                sp->setEffect(sp->getEffect(2), 1);
                sp->setEffect(temp, 2);

                temp = sp->getEffectDieSides(1);
                sp->setEffectDieSides(sp->getEffectDieSides(2), 1);
                sp->setEffectDieSides(temp, 2);

                ftemp = sp->getEffectRealPointsPerLevel(1);
                sp->setEffectRealPointsPerLevel(sp->getEffectRealPointsPerLevel(2), 1);
                sp->setEffectRealPointsPerLevel(ftemp, 2);

                temp = sp->getEffectBasePoints(1);
                sp->setEffectBasePoints(sp->getEffectBasePoints(2), 1);
                sp->setEffectBasePoints(temp, 2);

                temp = sp->getEffectMechanic(1);
                sp->setEffectMechanic(sp->getEffectMechanic(2), 1);
                sp->setEffectMechanic(temp, 2);

                temp = sp->getEffectImplicitTargetA(1);
                sp->setEffectImplicitTargetA(sp->getEffectImplicitTargetA(2), 1);
                sp->setEffectImplicitTargetA(temp, 2);

                temp = sp->getEffectImplicitTargetB(1);
                sp->setEffectImplicitTargetB(sp->getEffectImplicitTargetB(2), 1);
                sp->setEffectImplicitTargetB(temp, 2);

                temp = sp->getEffectRadiusIndex(1);
                sp->setEffectRadiusIndex(sp->getEffectRadiusIndex(2), 1);
                sp->setEffectRadiusIndex(temp, 2);

                temp = sp->getEffectApplyAuraName(1);
                sp->setEffectApplyAuraName(sp->getEffectApplyAuraName(2), 1);
                sp->setEffectApplyAuraName(temp, 2);

                temp = sp->getEffectAmplitude(1);
                sp->setEffectAmplitude(sp->getEffectAmplitude(2), 1);
                sp->setEffectAmplitude(temp, 2);

                ftemp = sp->getEffectMultipleValue(1);
                sp->setEffectMultipleValue(sp->getEffectMultipleValue(2), 1);
                sp->setEffectMultipleValue(ftemp, 2);

                temp = sp->getEffectChainTarget(1);
                sp->setEffectChainTarget(sp->getEffectChainTarget(2), 1);
                sp->setEffectChainTarget(temp, 2);

                temp = sp->getEffectMiscValue(1);
                sp->setEffectMiscValue(sp->getEffectMiscValue(2), 1);
                sp->setEffectMiscValue(temp, 2);

                temp = sp->getEffectTriggerSpell(1);
                sp->setEffectTriggerSpell(sp->getEffectTriggerSpell(2), 1);
                sp->setEffectTriggerSpell(temp, 1);

                ftemp = sp->getEffectPointsPerComboPoint(1);
                sp->setEffectPointsPerComboPoint(sp->getEffectPointsPerComboPoint(2), 1);
                sp->setEffectPointsPerComboPoint(ftemp, 2);
            } break;
            default:
                break;
        }

        for (uint8 b = 0; b < 3; ++b)
        {
            if (sp->getEffectTriggerSpell(b) != 0 && sSpellCustomizations.GetSpellInfo(sp->getEffectTriggerSpell(b)) == NULL)
            {
                // proc spell referencing non-existent spell. create a dummy spell for use w/ it.
                CreateDummySpell(sp->getEffectTriggerSpell(b));
            }

            if (sp->getAttributes() & ATTRIBUTES_ONLY_OUTDOORS && sp->getEffectApplyAuraName(b) == SPELL_AURA_MOUNTED)
            {
                sp->removeAttributes(ATTRIBUTES_ONLY_OUTDOORS);
            }

            if (sp->getEffectApplyAuraName(b) == SPELL_AURA_PREVENT_RESURRECTION)
			{
				sp->addAttributes(ATTRIBUTES_NEGATIVE);
				sp->addAttributesExC(ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD);
			}
        }

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

        // various flight spells
        // these make vehicles and other charmed stuff fliable
        if (sp->getActiveIconID() == 2158)
            sp->addAttributes(ATTRIBUTES_PASSIVE);


        //Name includes "" overwrites
        switch (sp->getId())
        {
        case 70908:
            sp->setEffectImplicitTargetA(EFF_TARGET_DYNAMIC_OBJECT, 0);
            break;
            // Name includes "Winter's Chill"
            // Winter's Chill handled by frost school
            case 11180:     // Winter's Chill Rank 1
            case 12579:
            case 28592:     // Winter's Chill Rank 2
            case 28593:     // Winter's Chill Rank 3
            case 63094:
            {
                sp->setSchool(SCHOOL_FROST);
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
                sp->setEffectDieSides(49, 0);
            } break;

            default:
                break;
        }

        // Set default mechanics if we don't already have one
        if (!sp->getMechanicsType())
        {
            //Set Silencing spells mechanic.
            if (sp->getEffectApplyAuraName(0) == SPELL_AURA_MOD_SILENCE ||
                sp->getEffectApplyAuraName(1) == SPELL_AURA_MOD_SILENCE ||
                sp->getEffectApplyAuraName(2) == SPELL_AURA_MOD_SILENCE)
                sp->setMechanicsType(MECHANIC_SILENCED);

            //Set Stunning spells mechanic.
            if (sp->getEffectApplyAuraName(0) == SPELL_AURA_MOD_STUN ||
                sp->getEffectApplyAuraName(1) == SPELL_AURA_MOD_STUN ||
                sp->getEffectApplyAuraName(2) == SPELL_AURA_MOD_STUN)
                sp->setMechanicsType(MECHANIC_STUNNED);

            //Set Fearing spells mechanic
            if (sp->getEffectApplyAuraName(0) == SPELL_AURA_MOD_FEAR ||
                sp->getEffectApplyAuraName(1) == SPELL_AURA_MOD_FEAR ||
                sp->getEffectApplyAuraName(2) == SPELL_AURA_MOD_FEAR)
                sp->setMechanicsType(MECHANIC_FLEEING);

            //Set Interrupted spells mech
            if (sp->getEffect(0) == SPELL_EFFECT_INTERRUPT_CAST ||
                sp->getEffect(1) == SPELL_EFFECT_INTERRUPT_CAST ||
                sp->getEffect(2) == SPELL_EFFECT_INTERRUPT_CAST)
                sp->setMechanicsType(MECHANIC_INTERRUPTED);
        }

        if (sp->custom_proc_interval > 0)      // if (sp->custom_proc_interval != 0)
            sp->addProcFlags(PROC_REMOVEONUSE);

        //shaman - shock, has no spellgroup.very dangerous move !

        //mage - fireball. Only some of the spell has the flags

        switch (sp->getId())
        {
            // SPELL_HASH_SEAL_OF_COMMAND
            case 20375:     // Seal of Command - School/dmg_type
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
                sp->setSchool(SCHOOL_HOLY); //the procspells of the original seal of command have physical school instead of holy
                sp->setSpell_Dmg_Type(SPELL_DMG_TYPE_MAGIC); //heh, crazy spell uses melee/ranged/magic dmg type for 1 spell. Now which one is correct ?
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
                sp->setSpell_Dmg_Type(SPELL_DMG_TYPE_MAGIC);
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
                sp->setMechanicsType(MECHANIC_INVULNARABLE);
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
                sp->setFacingCasterFlags(SPELL_INFRONT_STATUS_REQUIRE_INBACK);
            } break;
        }
    }

    /////////////////////////////////////////////////////////////////
    //SPELL COEFFICIENT SETTINGS START
    //////////////////////////////////////////////////////////////////

    for (auto it = sSpellCustomizations.GetSpellInfoStore()->begin(); it != sSpellCustomizations.GetSpellInfoStore()->end(); ++it)
    {
        // get spellentry
        sp = sSpellCustomizations.GetSpellInfo(it->first);
        if (sp == nullptr)
            continue;

        //Setting Cast Time Coefficient
        auto spell_cast_time = sSpellCastTimesStore.LookupEntry(sp->getCastingTimeIndex());
        float castaff = float(GetCastTime(spell_cast_time));
        if (castaff < 1500)
            castaff = 1500;
        else if (castaff > 7000)
            castaff = 7000;

        sp->casttime_coef = castaff / 3500;

        //Calculating fixed coeficients
        //Channeled spells
        if (sp->getChannelInterruptFlags() != 0)
        {
            float Duration = float(GetDuration(sSpellDurationStore.LookupEntry(sp->getDurationIndex())));
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
            float Duration = float(GetDuration(sSpellDurationStore.LookupEntry(sp->getDurationIndex())));
            sp->fixed_hotdotcoef = (Duration / 15000.0f);

            if (sp->custom_spell_coef_flags & SPELL_FLAG_ADITIONAL_EFFECT)
                sp->fixed_hotdotcoef *= 0.95f;
            if (sp->custom_spell_coef_flags & SPELL_FLAG_AOE_SPELL)
                sp->fixed_hotdotcoef *= 0.5f;

        }

        //Combined standard and over-time spells
        else if (sp->custom_spell_coef_flags & SPELL_FLAG_IS_DD_DH_DOT_SPELL)
        {
            float Duration = float(GetDuration(sSpellDurationStore.LookupEntry(sp->getDurationIndex())));
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
        switch (sp->getId())
        {
            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_SHIELD_OF_RIGHTEOUSNESS
            case 53600:     // Shield of Righteousness Rank 1
            case 61411:     // Shield of Righteousness Rank 2
            {
                sp->setSchool(SCHOOL_HOLY);
                sp->setEffect(SPELL_EFFECT_DUMMY, 0);
                sp->setEffect(SPELL_EFFECT_NULL, 1);          //hacks, handling it in Spell::SpellEffectSchoolDMG(uint32 i)
                sp->setEffect(SPELL_EFFECT_SCHOOL_DAMAGE, 2); //hack
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
                sp->setSchool(SCHOOL_HOLY); //Consecration is a holy redirected spell.
                sp->setSpell_Dmg_Type(SPELL_DMG_TYPE_MAGIC); //Speaks for itself.
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_SEALS_OF_THE_PURE
#if VERSION_STRING != Cata
            case 20224:     // Seals of the Pure Rank 1
            case 20225:     // Seals of the Pure Rank 2
            case 20330:     // Seals of the Pure Rank 3
            case 20331:     // Seals of the Pure Rank 4
            case 20332:     // Seals of the Pure Rank 5
            {
                sp->setEffectSpellClassMask(0x08000400, 0, 0);
                sp->setEffectSpellClassMask(0x20000000, 0, 1);
                sp->setEffectSpellClassMask(0x800, 1, 1);
            } break;
#endif

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
                sp->setChannelInterruptFlags(0);
            } break;

            //////////////////////////////////////////////////////////////////////////////////////////
            // SPELL_HASH_GRACE (without ranks)
            case 47930:     // Grace
            {
                sp->setRangeIndex(4);
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
                sp->addAttributesExC(ATTRIBUTESEXC_NO_DONE_BONUS);
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
                sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 0);
                sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 0);
                sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
                sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 1);
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
                sp->addAttributes(ATTRIBUTESEXC_NO_DONE_BONUS);
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
                sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
                sp->setEffectTriggerSpell(8349, 1);
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 8498:      //Fire Nova Rank 2
            {
                sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
                sp->setEffectTriggerSpell(8502, 1);
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 8499:      //Fire Nova Rank 3
            {
                sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
                sp->setEffectTriggerSpell(8503, 1);
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 11314:     //Fire Nova Rank 4
            {
                sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
                sp->setEffectTriggerSpell(11306, 1);
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 11315:     //Fire Nova Rank 5
            {
                sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
                sp->setEffectTriggerSpell(11307, 1);
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 25546:     //Fire Nova Rank 6
            {
                sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
                sp->setEffectTriggerSpell(25535, 1);
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 25547:     //Fire Nova Rank 7
            {
                sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
                sp->setEffectTriggerSpell(25537, 1);
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 61649:     //Fire Nova Rank 8
            {
                sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
                sp->setEffectTriggerSpell(61650, 1);
                // Fire Nova - 0% spd coefficient
                sp->fixed_dddhcoef = 0.0f;
            } break;
            case 61657:     //Fire Nova Rank 9
            {
                sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
                sp->setEffectTriggerSpell(61654, 1);
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
                sp->addAuraInterruptFlags(AURA_INTERRUPT_ON_UNUSED2);
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
                sp->setRequiredShapeShift(1);
            } break;
            default:
                break;
        }
    }
    // END OF LOOP

    //                                                  0    1            2                       3
    QueryResult* resultx = WorldDatabase.Query("SELECT id, name, Dspell_coef_override, OTspell_coef_override FROM spell_coef_override");
    if (resultx != nullptr)
    {
        do
        {
            Field* f = resultx->Fetch();
            sp = sSpellCustomizations.GetSpellInfo(f[0].GetUInt32());
            if (sp != nullptr)
            {
                sp->Dspell_coef_override = f[2].GetFloat();
                sp->OTspell_coef_override = f[3].GetFloat();
            }
            else
                LOG_ERROR("Has nonexistent spell %u.", f[0].GetUInt32());
        }
        while (resultx->NextRow());
        delete resultx;
    }

    //Fully loaded coefficients, we must share channeled coefficient to its triggered spells
    for (auto it = sSpellCustomizations.GetSpellInfoStore()->begin(); it != sSpellCustomizations.GetSpellInfoStore()->end(); ++it)
    {
        // get spellentry
        sp = sSpellCustomizations.GetSpellInfo(it->first);
        if (sp == nullptr)
            continue;

        SpellInfo* spz;

        //Case SPELL_AURA_PERIODIC_TRIGGER_SPELL
        for (uint8 i = 0; i < 3; ++i)
        {
            if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL)
            {
                spz = Spell::checkAndReturnSpellEntry(sp->getEffectTriggerSpell(i));
                if (spz != NULL)
                {
                    if (sp->Dspell_coef_override >= 0)
                        spz->Dspell_coef_override = sp->Dspell_coef_override;
                    else
                    {
                        //we must set bonus per tick on triggered spells now (i.e. Arcane Missiles)
                        if (sp->getChannelInterruptFlags() != 0)
                        {
                            float Duration = float(GetDuration(sSpellDurationStore.LookupEntry(sp->getDurationIndex())));
                            float amp = float(sp->getEffectAmplitude(i));
                            sp->fixed_dddhcoef = sp->fixed_hotdotcoef * amp / Duration;
                        }
                        spz->fixed_dddhcoef = sp->fixed_dddhcoef;
                    }

                    if (sp->OTspell_coef_override >= 0)
                        spz->OTspell_coef_override = sp->OTspell_coef_override;
                    else
                    {
                        //we must set bonus per tick on triggered spells now (i.e. Arcane Missiles)
                        if (sp->getChannelInterruptFlags() != 0)
                        {
                            float Duration = float(GetDuration(sSpellDurationStore.LookupEntry(sp->getDurationIndex())));
                            float amp = float(sp->getEffectAmplitude(i));
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
        sp = Spell::checkAndReturnSpellEntry(thrown_spells[i]);
        if (sp != nullptr && sp->getRecoveryTime() == 0 && sp->getStartRecoveryTime() == 0)
        {
            if (sp->getId() == SPELL_RANGED_GENERAL)
                sp->setRecoveryTime(500);    // cebernic: hunter general with 0.5s
            else
                sp->setRecoveryTime(1500); // 1.5cd
        }
    }

    ////////////////////////////////////////////////////////////
    // Wands
    sp = Spell::checkAndReturnSpellEntry(SPELL_RANGED_WAND);
    if (sp != nullptr)
        sp->setSpell_Dmg_Type(SPELL_DMG_TYPE_RANGED);


    //////////////////////////////////////////////////////
    // CLASS-SPECIFIC SPELL FIXES                        //
    //////////////////////////////////////////////////////

    // Note: when applying spell hackfixes, please follow a template

    //////////////////////////////////////////
    // WARRIOR                                //
    //////////////////////////////////////////

    // Insert warrior spell fixes here

    ////////////////////////////////////////////////////////////
    // Arms

    // Juggernaut
    sp = Spell::checkAndReturnSpellEntry(65156);
    if (sp != nullptr)
        sp->setAuraInterruptFlags(AURA_INTERRUPT_ON_CAST_SPELL);

    // Warrior - Overpower Rank 1
    sp = Spell::checkAndReturnSpellEntry(7384);
    if (sp != nullptr)
        sp->addAttributes(ATTRIBUTES_CANT_BE_DPB);
    // Warrior - Overpower Rank 2
    sp = Spell::checkAndReturnSpellEntry(7887);
    if (sp != nullptr)
        sp->addAttributes(ATTRIBUTES_CANT_BE_DPB);
    // Warrior - Overpower Rank 3
    sp = Spell::checkAndReturnSpellEntry(11584);
    if (sp != nullptr)
        sp->addAttributes(ATTRIBUTES_CANT_BE_DPB);
    // Warrior - Overpower Rank 4
    sp = Spell::checkAndReturnSpellEntry(11585);
    if (sp != nullptr)
        sp->addAttributes(ATTRIBUTES_CANT_BE_DPB);

    // Warrior - Tactical Mastery Rank 1
    sp = Spell::checkAndReturnSpellEntry(12295);
    if (sp != nullptr)
        sp->setRequiredShapeShift(0x00070000);
    // Warrior - Tactical Mastery Rank 2
    sp = Spell::checkAndReturnSpellEntry(12676);
    if (sp != nullptr)
        sp->setRequiredShapeShift(0x00070000);
    // Warrior - Tactical Mastery Rank 3
    sp = Spell::checkAndReturnSpellEntry(12677);
    if (sp != nullptr)
        sp->setRequiredShapeShift(0x00070000);

    // Warrior - Heroic Throw
    sp = Spell::checkAndReturnSpellEntry(57755);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_SCHOOL_DAMAGE, 0);
    }

    // Warrior - Rend
    sp = Spell::checkAndReturnSpellEntry(772);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(6546);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(6547);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(6548);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(11572);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(11573);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(11574);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(25208);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);

    ////////////////////////////////////////////////////////////
    // Fury

    // Warrior - Slam
    sp = Spell::checkAndReturnSpellEntry(1464);
    if (sp != nullptr)
        sp->setEffect(SPELL_EFFECT_SCHOOL_DAMAGE, 0);

    sp = Spell::checkAndReturnSpellEntry(8820);
    if (sp != nullptr)
        sp->setEffect(SPELL_EFFECT_SCHOOL_DAMAGE, 0);

    sp = Spell::checkAndReturnSpellEntry(11604);
    if (sp != nullptr)
        sp->setEffect(SPELL_EFFECT_SCHOOL_DAMAGE, 0);

    sp = Spell::checkAndReturnSpellEntry(11605);
    if (sp != nullptr)
        sp->setEffect(SPELL_EFFECT_SCHOOL_DAMAGE, 0);

    sp = Spell::checkAndReturnSpellEntry(25241);
    if (sp != nullptr)
        sp->setEffect(SPELL_EFFECT_SCHOOL_DAMAGE, 0);

    sp = Spell::checkAndReturnSpellEntry(25242);
    if (sp != nullptr)
        sp->setEffect(SPELL_EFFECT_SCHOOL_DAMAGE, 0);

    sp = Spell::checkAndReturnSpellEntry(47474);
    if (sp != nullptr)
        sp->setEffect(SPELL_EFFECT_SCHOOL_DAMAGE, 0);

    sp = Spell::checkAndReturnSpellEntry(47475);
    if (sp != nullptr)
        sp->setEffect(SPELL_EFFECT_SCHOOL_DAMAGE, 0);

    // Warrior - Bloodthirst new version is ok but old version is wrong from now on :(
    sp = Spell::checkAndReturnSpellEntry(23881);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1); //cast on us, it is good
        sp->setEffectTriggerSpell(23885, 1); //evil , but this is good for us :D
    }
    sp = Spell::checkAndReturnSpellEntry(23892);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(23886, 1); //evil , but this is good for us :D  // DankoDJ: Is there a reason to trigger an non existing spell?
    }
    sp = Spell::checkAndReturnSpellEntry(23893);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1); //
        sp->setEffectTriggerSpell(23887, 1); //evil , but this is good for us :D // DankoDJ: Is there a reason to trigger an non existing spell?
    }
    sp = Spell::checkAndReturnSpellEntry(23894);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1); //
        sp->setEffectTriggerSpell(23888, 1); //evil , but this is good for us :D // DankoDJ: Is there a reason to trigger an non existing spell?
    }
    sp = Spell::checkAndReturnSpellEntry(25251);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1); //aura
        sp->setEffectTriggerSpell(25252, 1); //evil , but this is good for us :D // DankoDJ: Is there a reason to trigger an non existing spell?
    }
    sp = Spell::checkAndReturnSpellEntry(30335);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1); //aura
        sp->setEffectTriggerSpell(30339, 1); //evil , but this is good for us :D // DankoDJ: Is there a reason to trigger an non existing spell?
    }

    // Warrior - Berserker Rage
    sp = Spell::checkAndReturnSpellEntry(18499);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 0);//Forcing a dummy aura, so we can add the missing 4th effect.
        sp->setEffect(SPELL_EFFECT_NULL, 1);
        sp->setEffect(SPELL_EFFECT_NULL, 2);
    }

    // Warrior - Heroic Fury
    sp = Spell::checkAndReturnSpellEntry(60970);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_DUMMY, 0);
    }

    ////////////////////////////////////////////////////////////
    // Protection

    // Intervene  Ranger: stop attack
    sp = Spell::checkAndReturnSpellEntry(3411);
    if (sp != nullptr)
    {
        sp->addAttributes(ATTRIBUTES_STOP_ATTACK);
    }

    //////////////////////////////////////////
    // PALADIN                                //
    //////////////////////////////////////////

    // Insert paladin spell fixes here

    //Paladin - Seal of Command - Holy damage, but melee mechanics (crit damage, chance, etc)
    sp = Spell::checkAndReturnSpellEntry(20424);
    if (sp != nullptr)
        sp->custom_is_melee_spell = true;

    //Paladin - Hammer of the Righteous
    sp = Spell::checkAndReturnSpellEntry(53595);
    if (sp != nullptr)
    {
        sp->setSpeed(0);    //without, no damage is done
    }

    //Paladin - Seal of Martyr
    sp = Spell::checkAndReturnSpellEntry(53720);
    if (sp != nullptr)
    {
        sp->setSchool(SCHOOL_HOLY);
    }
    //Paladin - seal of blood
    sp = Spell::checkAndReturnSpellEntry(31892);
    if (sp != nullptr)
    {
        sp->setSchool(SCHOOL_HOLY);
    }
    sp = Spell::checkAndReturnSpellEntry(53719);
    if (sp != nullptr)
    {
        sp->setSchool(SCHOOL_HOLY);
        sp->setSpell_Dmg_Type(SPELL_DMG_TYPE_MAGIC);
    }
    sp = Spell::checkAndReturnSpellEntry(31893);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_PHYSICAL_ATTACK);
        sp->setSchool(SCHOOL_HOLY);
        sp->setSpell_Dmg_Type(SPELL_DMG_TYPE_MAGIC);
    }

    //Paladin - Divine Storm
    sp = Spell::checkAndReturnSpellEntry(53385);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectRadiusIndex(43, 0); //16 yards
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(54172, 1);
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setProcChance(100);
        sp->setMaxTargets(4);
    }

    //Paladin - Judgements of the Wise
    sp = Spell::checkAndReturnSpellEntry(31930);
    if (sp != nullptr)
    {
        sp->setSpellFamilyName(0);
        sp->setSpellGroupType(0, 0);
        sp->setSpellGroupType(0, 1);
        sp->setSpellGroupType(0, 2);
    }

    sp = Spell::checkAndReturnSpellEntry(54180);
    if (sp != nullptr)
    {
        sp->setSpellFamilyName(0);
        sp->setSpellGroupType(0, 0);
        sp->setSpellGroupType(0, 1);
        sp->setSpellGroupType(0, 2);
    }

    //Paladin - Avenging Wrath marker - Is forced debuff
    sp = Spell::checkAndReturnSpellEntry(61987);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_IGNORE_INVULNERABILITY);
    }

    //Paladin - Forbearance - Is forced debuff
    sp = Spell::checkAndReturnSpellEntry(25771);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_IGNORE_INVULNERABILITY);
    }

    //Divine Protection
    sp = Spell::checkAndReturnSpellEntry(498);
    if (sp != nullptr)
        sp->setTargetAuraSpellNot(25771);

    //Divine Shield
    sp = Spell::checkAndReturnSpellEntry(642);
    if (sp != nullptr)
        sp->setTargetAuraSpellNot(25771);

    //Hand of Protection Rank 1
    sp = Spell::checkAndReturnSpellEntry(1022);
    if (sp != nullptr)
        sp->setTargetAuraSpellNot(25771);

    //Hand of Protection Rank 2
    sp = Spell::checkAndReturnSpellEntry(5599);
    if (sp != nullptr)
        sp->setTargetAuraSpellNot(25771);

    //Hand of Protection Rank 3
    sp = Spell::checkAndReturnSpellEntry(10278);
    if (sp != nullptr)
        sp->setTargetAuraSpellNot(25771);

    //Paladin - Art of War
    sp = Spell::checkAndReturnSpellEntry(53486);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_DONE, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(53489);
    if (sp != nullptr)
        sp->setAuraInterruptFlags(AURA_INTERRUPT_ON_CAST_SPELL);

    sp = Spell::checkAndReturnSpellEntry(53488);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_DONE, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(59578);
    if (sp != nullptr)
        sp->setAuraInterruptFlags(AURA_INTERRUPT_ON_CAST_SPELL);

    //Paladin - Hammer of Justice - Interrupt effect
    sp = Spell::checkAndReturnSpellEntry(853);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(32747, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(5588);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(32747, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(5589);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(32747, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(10308);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(32747, 1);
    }

    //////////////////////////////////////////
    // HUNTER                                //
    //////////////////////////////////////////

    // Insert hunter spell fixes here

    //Hunter - Bestial Wrath
    sp = Spell::checkAndReturnSpellEntry(19574);
    if (sp != nullptr)
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 2);

    //Hunter - The Beast Within
    sp = Spell::checkAndReturnSpellEntry(34471);
    if (sp != nullptr)
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 2);

    //Hunter - Go for the Throat
    sp = Spell::checkAndReturnSpellEntry(34952);
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    sp = Spell::checkAndReturnSpellEntry(34953);
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);

    // Hunter - Spirit Bond
    sp = Spell::checkAndReturnSpellEntry(19578);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(19579, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(20895);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(24529, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(19579);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1); //we should do the same for player too as we did for pet
        sp->setEffectApplyAuraName(sp->getEffectApplyAuraName(0), 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
        sp->setEffectBasePoints(sp->getEffectBasePoints(0), 1);
        sp->setEffectAmplitude(sp->getEffectAmplitude(0), 1);
        sp->setEffectDieSides(sp->getEffectDieSides(0), 1);
    }
    sp = Spell::checkAndReturnSpellEntry(24529);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1); //we should do the same for player too as we did for pet
        sp->setEffectApplyAuraName(sp->getEffectApplyAuraName(0), 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
        sp->setEffectBasePoints(sp->getEffectBasePoints(0), 1);
        sp->setEffectAmplitude(sp->getEffectAmplitude(0), 1);
        sp->setEffectDieSides(sp->getEffectDieSides(0), 1);
    }

    //Hunter Silencing Shot
    sp = Spell::checkAndReturnSpellEntry(34490);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_SILENCE, 1);
    }

    // Hunter - Ferocious Inspiration
    sp = Spell::checkAndReturnSpellEntry(34455);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        sp->setEffectTriggerSpell(34456, 0);
        sp->setProcFlags(PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF)); //maybe target master ?
        sp->setEffect(SPELL_EFFECT_NULL, 1); //remove this
    }
    sp = Spell::checkAndReturnSpellEntry(34459);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        sp->setEffectTriggerSpell(34456, 0);
        sp->setProcFlags(PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF));
        sp->setEffect(SPELL_EFFECT_NULL, 1); //remove this
    }
    sp = Spell::checkAndReturnSpellEntry(34460);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        sp->setEffectTriggerSpell(34456, 0);
        sp->setProcFlags(PROC_ON_CRIT_ATTACK | PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF));
        sp->setEffect(SPELL_EFFECT_NULL, 1); //remove this
    }

    // Hunter - Focused Fire
    sp = Spell::checkAndReturnSpellEntry(35029);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(35060, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(35030);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(35061, 0);
    }

    // Hunter - Thrill of the Hunt
    sp = Spell::checkAndReturnSpellEntry(34497);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF));
        sp->setProcChance(sp->getEffectBasePoints(0) + 1);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(34720, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(34498);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF));
        sp->setProcChance(sp->getEffectBasePoints(0) + 1);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(34720, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(34499);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF));
        sp->setProcChance(sp->getEffectBasePoints(0) + 1);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(34720, 0);
    }

    //Hunter - Frenzy
    sp = Spell::checkAndReturnSpellEntry(19621);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(19615, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        sp->setProcChance(sp->getEffectBasePoints(0));
        sp->setProcFlags(PROC_ON_CRIT_ATTACK | static_cast<uint32>(PROC_TARGET_SELF));        //Zyres: moved from custom_c_is_flag
    }
    sp = Spell::checkAndReturnSpellEntry(19622);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(19615, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        sp->setProcChance(sp->getEffectBasePoints(0));
        sp->setProcFlags(PROC_ON_CRIT_ATTACK | static_cast<uint32>(PROC_TARGET_SELF));        //Zyres: moved from custom_c_is_flag
    }
    sp = Spell::checkAndReturnSpellEntry(19623);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(19615, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        sp->setProcChance(sp->getEffectBasePoints(0));
        sp->setProcFlags(PROC_ON_CRIT_ATTACK | static_cast<uint32>(PROC_TARGET_SELF));        //Zyres: moved from custom_c_is_flag
    }
    sp = Spell::checkAndReturnSpellEntry(19624);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(19615, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        sp->setProcChance(sp->getEffectBasePoints(0));
        sp->setProcFlags(PROC_ON_CRIT_ATTACK | static_cast<uint32>(PROC_TARGET_SELF));        //Zyres: moved from custom_c_is_flag
    }
    sp = Spell::checkAndReturnSpellEntry(19625);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(19615, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        sp->setProcChance(sp->getEffectBasePoints(0));
        sp->setProcFlags(PROC_ON_CRIT_ATTACK | static_cast<uint32>(PROC_TARGET_SELF));        //Zyres: moved from custom_c_is_flag
    }

    //Hunter : Pathfinding
    sp = Spell::checkAndReturnSpellEntry(19559);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(19560);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 0);
    }

    //Hunter : Rapid Killing - might need to add honor trigger too here. I'm guessing you receive Xp too so I'm avoiding double proc
    sp = Spell::checkAndReturnSpellEntry(34948);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_GAIN_EXPIERIENCE | static_cast<uint32>(PROC_TARGET_SELF));
    }
    sp = Spell::checkAndReturnSpellEntry(34949);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_GAIN_EXPIERIENCE | static_cast<uint32>(PROC_TARGET_SELF));
    }

    // Feed pet
    sp = Spell::checkAndReturnSpellEntry(6991);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 0);
    }

    //\todo 16/03/08 Zyres: sql
    // MesoX: Serendipity http://www.wowhead.com/?spell=63730
    sp = Spell::checkAndReturnSpellEntry(63730);   // Rank 1
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
    }
    sp = Spell::checkAndReturnSpellEntry(63733);   // Rank 2
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
    }
    sp = Spell::checkAndReturnSpellEntry(63737);   // Rank 3
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
    }


    //////////////////////////////////////////
    // ROGUE                                //
    //////////////////////////////////////////

    // Insert rogue spell fixes here

    // Garrote - this is used?
    sp = Spell::checkAndReturnSpellEntry(37066);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_SINGLE_ENEMY, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_SINGLE_ENEMY, 1);
    }

    //rogue - Camouflage.
    sp = Spell::checkAndReturnSpellEntry(13975);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(14062);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(14063);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 0);
    }

    //rogue - Vanish : Second Trigger Spell
    sp = Spell::checkAndReturnSpellEntry(18461);
    if (sp != nullptr)
        sp->addAttributesEx(ATTRIBUTESEX_NOT_BREAK_STEALTH);

    // rogue - Blind (Make it able to miss!)
    sp = Spell::checkAndReturnSpellEntry(2094);
    if (sp != nullptr)
    {
        sp->setSpell_Dmg_Type(SPELL_DMG_TYPE_RANGED);
        sp->custom_is_ranged_spell = true;
    }

    //rogue - Shadowstep
    sp = Spell::checkAndReturnSpellEntry(36563);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 2);
    }
    // Still related to shadowstep - prevent the trigger spells from breaking stealth.
    sp = Spell::checkAndReturnSpellEntry(44373);
    if (sp != nullptr)
        sp->addAttributesEx(ATTRIBUTESEX_NOT_BREAK_STEALTH);
    sp = Spell::checkAndReturnSpellEntry(36563);
    if (sp != nullptr)
        sp->addAttributesEx(ATTRIBUTESEX_NOT_BREAK_STEALTH);
    sp = Spell::checkAndReturnSpellEntry(36554);
    if (sp != nullptr)
        sp->addAttributesEx(ATTRIBUTESEX_NOT_BREAK_STEALTH);

    //garrot
    sp = Spell::checkAndReturnSpellEntry(703);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(8631);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(8632);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(8633);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(11289);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(11290);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(26839);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(26884);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);

    //rupture
    sp = Spell::checkAndReturnSpellEntry(1943);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(8639);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(8640);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(11273);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(11274);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(11275);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);
    sp = Spell::checkAndReturnSpellEntry(26867);
    if (sp != nullptr)
        sp->setMechanicsType(MECHANIC_BLEEDING);

    //Rogue - Killing Spree Stealth fix
    sp = Spell::checkAndReturnSpellEntry(51690);
    if (sp != nullptr)
        sp->addAttributesEx(ATTRIBUTESEX_NOT_BREAK_STEALTH);


    //////////////////////////////////////////
    // PRIEST                                //
    //////////////////////////////////////////

    // Insert priest spell fixes here

    // Prayer of mending
    sp = Spell::checkAndReturnSpellEntry(41635);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_SINGLE_FRIEND, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(48110);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_SINGLE_FRIEND, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(48111);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_SINGLE_FRIEND, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(33110);
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_SINGLE_FRIEND, 0);

    // Vampiric Embrace heal spell
    sp = Spell::checkAndReturnSpellEntry(15290);
    if (sp != nullptr)
    {
        sp->setEffectBasePoints(2, 0);
        sp->setEffectBasePoints(14, 1);
    }

    // Improved Mind Blast
    sp = Spell::checkAndReturnSpellEntry(15273);   //rank 1
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(15312);   //rank 2
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(15313);   //rank 3
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(15314);   //rank 4
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(15316);   //rank 5
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 1);
    }

    // Body and soul - fix duration of cleanse poison
    sp = Spell::checkAndReturnSpellEntry(64134);
    if (sp != nullptr)
        sp->setDurationIndex(29);

    // Spirit of Redemption - required spells can be casted while dead
    sp = Spell::checkAndReturnSpellEntry(27795);   // This is casted by shape shift
    if (sp != nullptr)
        sp->addAttributesExC(ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD);
    sp = Spell::checkAndReturnSpellEntry(27792);   // This is casted by Apply Aura: Spirit of Redemption
    if (sp != nullptr)
        sp->addAttributesExC(ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD);

    //Priest - Wand Specialization
    sp = Spell::checkAndReturnSpellEntry(14524);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(14525);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(14526);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(14527);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(14528);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 0);
    }

    //Priest - Inspiration proc spell
    sp = Spell::checkAndReturnSpellEntry(14893);
    if (sp != nullptr)
        sp->setRangeIndex(4);
    sp = Spell::checkAndReturnSpellEntry(15357);
    if (sp != nullptr)
        sp->setRangeIndex(4);
    sp = Spell::checkAndReturnSpellEntry(15359);
    if (sp != nullptr)
        sp->setRangeIndex(4);

    //priest - surge of light
    sp = Spell::checkAndReturnSpellEntry(33151);
    if (sp != nullptr)
    {
        sp->setAuraInterruptFlags(AURA_INTERRUPT_ON_CAST_SPELL);
    }
    // priest - Reflective Shield
    sp = Spell::checkAndReturnSpellEntry(33201);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_ABSORB);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(33619, 0); //!! WRONG spell, we will make direct dmg here
    }
    sp = Spell::checkAndReturnSpellEntry(33202);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_ABSORB);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(33619, 0); //!! WRONG spell, we will make direct dmg here
    }
    // Weakened Soul - Is debuff
    sp = Spell::checkAndReturnSpellEntry(6788);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_IGNORE_INVULNERABILITY);
    }

    // Penance
    sp = Spell::checkAndReturnSpellEntry(47540);
    if (sp != nullptr)
    {
        sp->setDurationIndex(566); // Change to instant cast as script will cast the real channeled spell.
        sp->setChannelInterruptFlags(0); // Remove channeling behavior.
    }

    sp = Spell::checkAndReturnSpellEntry(53005);
    if (sp != nullptr)
    {
        sp->setDurationIndex(566);
        sp->setChannelInterruptFlags(0);
    }

    sp = Spell::checkAndReturnSpellEntry(53006);
    if (sp != nullptr)
    {
        sp->setDurationIndex(566);
        sp->setChannelInterruptFlags(0);
    }

    sp = Spell::checkAndReturnSpellEntry(53007);
    if (sp != nullptr)
    {
        sp->setDurationIndex(566);
        sp->setChannelInterruptFlags(0);
    }

    // Penance triggered healing spells have wrong targets.
    sp = Spell::checkAndReturnSpellEntry(47750);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_SINGLE_FRIEND, 0);
    }

    sp = Spell::checkAndReturnSpellEntry(52983);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_SINGLE_FRIEND, 0);
    }

    sp = Spell::checkAndReturnSpellEntry(52984);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_SINGLE_FRIEND, 0);
    }

    sp = Spell::checkAndReturnSpellEntry(52985);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_SINGLE_FRIEND, 0);
    }


    //////////////////////////////////////////
    // SHAMAN                                //
    //////////////////////////////////////////

    // Insert shaman spell fixes here
    //shaman - Healing Way
    sp = Spell::checkAndReturnSpellEntry(29202);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);     // DankoDJ: No triggered Spell! We override SPELL_AURA_ADD_PCT_MODIFIER with this crap?
    }
    sp = Spell::checkAndReturnSpellEntry(29205);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);     // DankoDJ: No triggered Spell! We override SPELL_AURA_ADD_PCT_MODIFIER with this crap?
    }
    sp = Spell::checkAndReturnSpellEntry(29206);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);     // DankoDJ: No triggered Spell! We override SPELL_AURA_ADD_PCT_MODIFIER with this crap?
    }

    // Elemental Mastery
    sp = Spell::checkAndReturnSpellEntry(16166);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_CRITICAL, 0);
        sp->setEffectMiscValue(SMT_COST, 0);
        // sp->AuraInterruptFlags = AURA_INTERRUPT_ON_AFTER_CAST_SPELL;
    }

    ////////////////////////////////////////////////////////////
    // Shamanistic Rage
    SpellInfo*  parentsp = Spell::checkAndReturnSpellEntry(30823);
    SpellInfo* triggersp = Spell::checkAndReturnSpellEntry(30824);
    if (parentsp != nullptr && triggersp != nullptr)
        triggersp->setEffectBasePoints(parentsp->getEffectBasePoints(0), 0);

    //summon only 1 elemental totem
    sp = Spell::checkAndReturnSpellEntry(2894);
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_TOTEM_FIRE, 0); //remove this targeting. it is enough to get 1 target

    //summon only 1 elemental totem
    sp = Spell::checkAndReturnSpellEntry(2062);
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_TOTEM_EARTH, 0); //remove this targeting. it is enough to get 1 target


    ////////////////////////////////////////////////////////////
    // Bloodlust
    //Bloodlust
    sp = Spell::checkAndReturnSpellEntry(2825);
    if (sp != nullptr)
        sp->setCasterAuraSpellNot(57724); //sated debuff

    // Sated - is debuff
    sp = Spell::checkAndReturnSpellEntry(57724);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_IGNORE_INVULNERABILITY);
    }

    ////////////////////////////////////////////////////////////
    // Heroism
    //Heroism
    sp = Spell::checkAndReturnSpellEntry(32182);
    if (sp != nullptr)
        sp->setCasterAuraSpellNot(57723); //sated debuff

    // Sated - is debuff
    sp = Spell::checkAndReturnSpellEntry(57723);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_IGNORE_INVULNERABILITY);
    }

    ////////////////////////////////////////////////////////////
    // Purge
    sp = Spell::checkAndReturnSpellEntry(370);
    if (sp != nullptr)
        sp->setDispelType(DISPEL_MAGIC);
    sp = Spell::checkAndReturnSpellEntry(8012);
    if (sp != nullptr)
        sp->setDispelType(DISPEL_MAGIC);
    sp = Spell::checkAndReturnSpellEntry(27626);
    if (sp != nullptr)
        sp->setDispelType(DISPEL_MAGIC);
    sp = Spell::checkAndReturnSpellEntry(33625);
    if (sp != nullptr)
        sp->setDispelType(DISPEL_MAGIC);

    //Shaman - Shamanistic Focus
    // needs to be fixed (doesn't need to proc, it now just reduces mana cost always by %)
    sp = Spell::checkAndReturnSpellEntry(43338);
    if (sp != nullptr)
    {
        sp->setEffectTriggerSpell(43339, 0);
    }

    sp = Spell::checkAndReturnSpellEntry(43339);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_COST, 0);
    }

    //shaman - Improved Chain Heal
    sp = Spell::checkAndReturnSpellEntry(30873);
    if (sp != nullptr)
    {
        sp->setEffectDieSides(0, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(30872);
    if (sp != nullptr)
    {
        sp->setEffectDieSides(0, 0);
    }

    //shaman - Improved Weapon Totems
    sp = Spell::checkAndReturnSpellEntry(29193);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 1);
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 0);
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(29192);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 1);
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 0);
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 1);
    }

    // Shaman - Improved Fire Totems
    sp = Spell::checkAndReturnSpellEntry(16544);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_DURATION, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(16086);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_DURATION, 0);
    }

    //shaman - Elemental Weapons
    sp = Spell::checkAndReturnSpellEntry(29080);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_DAMAGE_DONE, 1);
        sp->setEffectMiscValue(SMT_DAMAGE_DONE, 2);
    }
    sp = Spell::checkAndReturnSpellEntry(29079);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_DAMAGE_DONE, 1);
        sp->setEffectMiscValue(SMT_DAMAGE_DONE, 2);
    }
    sp = Spell::checkAndReturnSpellEntry(16266);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_DAMAGE_DONE, 1);
        sp->setEffectMiscValue(SMT_DAMAGE_DONE, 2);
    }

    // Magma Totem - 0% spd coefficient
    sp = Spell::checkAndReturnSpellEntry(25550);
    if (sp != nullptr)
        sp->fixed_dddhcoef = 0.0f;
    sp = Spell::checkAndReturnSpellEntry(10581);
    if (sp != nullptr)
        sp->fixed_dddhcoef = 0.0f;
    sp = Spell::checkAndReturnSpellEntry(10580);
    if (sp != nullptr)
        sp->fixed_dddhcoef = 0.0f;
    sp = Spell::checkAndReturnSpellEntry(10579);
    if (sp != nullptr)
        sp->fixed_dddhcoef = 0.0f;
    sp = Spell::checkAndReturnSpellEntry(8187);
    if (sp != nullptr)
        sp->fixed_dddhcoef = 0.0f;

    ////////////////////////////////////////////////////////////
    //  Unleashed Rage - LordLeeCH
    sp = Spell::checkAndReturnSpellEntry(30802);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CRIT_ATTACK);
        sp->setEffect(SPELL_EFFECT_APPLY_GROUP_AREA_AURA, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(30808);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CRIT_ATTACK);
        sp->setEffect(SPELL_EFFECT_APPLY_GROUP_AREA_AURA, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(30809);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CRIT_ATTACK);
        sp->setEffect(SPELL_EFFECT_APPLY_GROUP_AREA_AURA, 0);
    }

    ////////////////////////////////////////////////////////////
    // Ancestral healing proc spell
    sp = Spell::checkAndReturnSpellEntry(16177);
    if (sp != nullptr)
        sp->setRangeIndex(4);
    sp = Spell::checkAndReturnSpellEntry(16236);
    if (sp != nullptr)
        sp->setRangeIndex(4);
    sp = Spell::checkAndReturnSpellEntry(16237);
    if (sp != nullptr)
        sp->setRangeIndex(4);

    sp = Spell::checkAndReturnSpellEntry(20608);   //Reincarnation
    if (sp != nullptr)
    {
        for (uint8 i = 0; i < 8; ++i)
        {
            if (sp->getReagent(i))
            {
                sp->setReagent(0, i);
                sp->setReagentCount(0, i);
            }
        }
    }

    //////////////////////////////////////////
    // SHAMAN WRATH OF AIR TOTEM            //
    //////////////////////////////////////////
    sp = Spell::checkAndReturnSpellEntry(2895);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 2);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 0);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 1);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 2);
    }

    // Rogue - Master of Subtlety
    sp = Spell::checkAndReturnSpellEntry(31665);
    if (sp != nullptr)
        sp->addAttributesEx(ATTRIBUTESEX_NOT_BREAK_STEALTH);

    //////////////////////////////////////////
    // MAGE                                    //
    //////////////////////////////////////////

    // Insert mage spell fixes here

    // Brain Freeze rank 1
    sp = Spell::checkAndReturnSpellEntry(44546);
    if (sp != nullptr)
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 0);

    // Brain Freeze rank 2
    sp = Spell::checkAndReturnSpellEntry(44548);
    if (sp != nullptr)
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 0);

    // Brain Freeze rank 3
    sp = Spell::checkAndReturnSpellEntry(44549);
    if (sp != nullptr)
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 0);

    // Fingers of Frost rank 1
    sp = Spell::checkAndReturnSpellEntry(44543);
    if (sp != nullptr)
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 0);


    // Fingers of Frost rank 2
    sp = Spell::checkAndReturnSpellEntry(44545);
    if (sp != nullptr)
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 0);


    //Mage - Spell Power
    sp = Spell::checkAndReturnSpellEntry(35578);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_CRITICAL_DAMAGE, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(35581);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_CRITICAL_DAMAGE, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
    }

    //Mage - Elemental Precision
    sp = Spell::checkAndReturnSpellEntry(29438);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectMiscValue(SMT_COST, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(29439);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectMiscValue(SMT_COST, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(29440);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectMiscValue(SMT_COST, 0);
    }

    //Mage - Arcane Blast
    sp = Spell::checkAndReturnSpellEntry(30451);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
        sp->setProcFlags(PROC_ON_CAST_SPECIFIC_SPELL);
    }

    // Arcane Blast
    sp = Spell::checkAndReturnSpellEntry(42894);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
        sp->setProcFlags(PROC_ON_CAST_SPECIFIC_SPELL);
    }

    sp = Spell::checkAndReturnSpellEntry(42896);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
        sp->setProcFlags(PROC_ON_CAST_SPECIFIC_SPELL);
    }

    sp = Spell::checkAndReturnSpellEntry(42897);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
        sp->setProcFlags(PROC_ON_CAST_SPECIFIC_SPELL);
    }

    //mage : Empowered Arcane Missiles
    //heh B thinks he is smart by adding this to description ? If it doesn't work std then it still needs to made by hand
    sp = Spell::checkAndReturnSpellEntry(31579);
    if (sp != nullptr)
    {
        sp->setEffectBasePoints(5 * (sp->getEffectBasePoints(0) + 1), 0);
    }
    sp = Spell::checkAndReturnSpellEntry(31582);
    if (sp != nullptr)
    {
        sp->setEffectBasePoints(5 * (sp->getEffectBasePoints(0) + 1), 0);
    }
    sp = Spell::checkAndReturnSpellEntry(31583);
    if (sp != nullptr)
    {
        sp->setEffectBasePoints(5 * (sp->getEffectBasePoints(0) + 1), 0);
    }

    // cebernic: not for self?
    // impact
    sp = Spell::checkAndReturnSpellEntry(12355);
    if (sp != nullptr)
    {
        // passive rank: 11103, 12357, 12358 ,12359,12360 :D
        sp->setProcFlags(PROC_ON_ANY_DAMAGE_VICTIM | PROC_ON_SPELL_CRIT_HIT | PROC_ON_SPELL_HIT);
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER, 0);
        sp->setEffectImplicitTargetB(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER, 1);
        sp->setEffectImplicitTargetB(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER, 2);
        sp->setEffectImplicitTargetB(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER, 2);
    }

    //Mage - Invisibility
    sp = Spell::checkAndReturnSpellEntry(66);
    if (sp != nullptr)
    {
        sp->addAuraInterruptFlags(AURA_INTERRUPT_ON_CAST_SPELL);
        sp->setEffect(SPELL_EFFECT_NULL, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_PERIODIC_TRIGGER_SPELL, 2);
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 2);
        sp->setEffectAmplitude(3000, 2);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 2);
        sp->setEffectDieSides(1, 2);
        sp->setEffectTriggerSpell(32612, 2);
        sp->setEffectBasePoints(-1, 2);
    }

    //Invisibility triggered spell, should be removed on cast
    sp = Spell::checkAndReturnSpellEntry(32612);
    if (sp != nullptr)
    {
        sp->addAuraInterruptFlags(AURA_INTERRUPT_ON_CAST_SPELL);
    }

    //Arcane Potency procs
    sp = Spell::checkAndReturnSpellEntry(57529);
    if (sp != nullptr)
    {
        sp->setProcFlags(0);
        sp->setAuraInterruptFlags(0);
    }

    sp = Spell::checkAndReturnSpellEntry(57531);
    if (sp != nullptr)
    {
        sp->setProcFlags(0);
        sp->setAuraInterruptFlags(0);
    }

    //Hot Streak proc
    sp = Spell::checkAndReturnSpellEntry(48108);
    if (sp != nullptr)
    {
        sp->addAuraInterruptFlags(AURA_INTERRUPT_ON_CAST_SPELL);
    }

    //Ice Lances
    sp = Spell::checkAndReturnSpellEntry(42914);
    if (sp != nullptr)
        sp->Dspell_coef_override = 0.1429f;

    sp = Spell::checkAndReturnSpellEntry(42913);
    if (sp != nullptr)
        sp->Dspell_coef_override = 0.1429f;

    sp = Spell::checkAndReturnSpellEntry(30455);
    if (sp != nullptr)
        sp->Dspell_coef_override = 0.1429f;

    // Frostfire Bolts
    sp = Spell::checkAndReturnSpellEntry(47610);
    if (sp != nullptr)
        sp->fixed_dddhcoef = 0.8571f;

    sp = Spell::checkAndReturnSpellEntry(44614);
    if (sp != nullptr)
        sp->fixed_dddhcoef = 0.8571f;


    //mage - Combustion
    sp = Spell::checkAndReturnSpellEntry(11129);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_NULL, 0);
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(28682, 1);
        sp->setProcFlags(PROC_ON_SPELL_HIT | PROC_ON_SPELL_CRIT_HIT | static_cast<uint32>(PROC_TARGET_SELF));
        sp->setProcChance(0);
    }

    // mage - Conjure Refreshment Table
    sp = Spell::checkAndReturnSpellEntry(43985);
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_DYNAMIC_OBJECT, 0);

    // Hypothermia - forced debuff
    sp = Spell::checkAndReturnSpellEntry(41425);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_IGNORE_INVULNERABILITY);
    }

    // Mage - Permafrost Rank 1
    sp = Spell::checkAndReturnSpellEntry(11175);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 1);
    }

    // Mage - Permafrost Rank 2
    sp = Spell::checkAndReturnSpellEntry(12569);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 1);
    }

    // Mage - Permafrost Rank 3
    sp = Spell::checkAndReturnSpellEntry(12571);
    if (sp != nullptr)
    {
        sp->setEffectMiscValue(SMT_MISC_EFFECT, 1);
    }

    //Improved Counterspell rank 1
    sp = Spell::checkAndReturnSpellEntry(11255);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPECIFIC_SPELL);
    }

    //Improved Counterspell rank 2
    sp = Spell::checkAndReturnSpellEntry(12598);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPECIFIC_SPELL);
    }
    //////////////////////////////////////////
    // WARLOCK                                //
    //////////////////////////////////////////

    // Insert warlock spell fixes here

    //Dummy for Demonic Circle
    sp = Spell::checkAndReturnSpellEntry(48018);
    if (sp != nullptr)
    {

        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
        CreateDummySpell(62388);
        sp = Spell::checkAndReturnSpellEntry(62388);
        if (sp != nullptr)
        {
            sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
            sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 0);
        }
    }

    //megai2: Immolation Aura
    sp = Spell::checkAndReturnSpellEntry(50589);
    if (sp != nullptr)
    {
        sp->setChannelInterruptFlags(0); // Remove channeling behaviour.
    }

#if VERSION_STRING != Cata
    //megai2: Everlasting Affliction
    sp = Spell::checkAndReturnSpellEntry(47205);
    if (sp != nullptr)
    {
        sp->setEffectSpellClassMask(0x111, 1, 0);
        sp->setEffectSpellClassMask(0, 1, 1);
        sp->setProcFlags(PROC_ON_ANY_HOSTILE_ACTION);
    }

    sp = Spell::checkAndReturnSpellEntry(47204);
    if (sp != nullptr)
    {
        sp->setEffectSpellClassMask(0x111, 1, 0);
        sp->setEffectSpellClassMask(0, 1, 1);
        sp->setProcFlags(PROC_ON_ANY_HOSTILE_ACTION);
    }

    sp = Spell::checkAndReturnSpellEntry(47203);
    if (sp != nullptr)
    {
        sp->setEffectSpellClassMask(0x111, 1, 0);
        sp->setEffectSpellClassMask(0, 1, 1);
        sp->setProcFlags(PROC_ON_ANY_HOSTILE_ACTION);
    }

    sp = Spell::checkAndReturnSpellEntry(47202);
    if (sp != nullptr)
    {
        sp->setEffectSpellClassMask(0x111, 1, 0);
        sp->setEffectSpellClassMask(0, 1, 1);
        sp->setProcFlags(PROC_ON_ANY_HOSTILE_ACTION);
    }

    sp = Spell::checkAndReturnSpellEntry(47201);
    if (sp != nullptr)
    {
        sp->setEffectSpellClassMask(0x111, 1, 0);
        sp->setEffectSpellClassMask(0, 1, 1);
    }
#endif

    ////////////////////////////////////////////////////////////
    // Backlash
    sp = Spell::checkAndReturnSpellEntry(34936);
    if (sp != nullptr)
    {
        sp->setAuraInterruptFlags(AURA_INTERRUPT_ON_CAST_SPELL);
    }

    ////////////////////////////////////////////////////////////
    // Demonic Knowledge
    sp = Spell::checkAndReturnSpellEntry(35691);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_DONE, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 0);
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_DONE, 1);
        sp->setEffectBasePoints(sp->getEffectBasePoints(0), 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 2);
        sp->setEffectTriggerSpell(35696, 2);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 2);
    }
    sp = Spell::checkAndReturnSpellEntry(35692);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_DONE, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 0);
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_DONE, 1);
        sp->setEffectBasePoints(sp->getEffectBasePoints(0), 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 2);
        sp->setEffectTriggerSpell(35696, 2);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 2);
    }
    sp = Spell::checkAndReturnSpellEntry(35693);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_DONE, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 0);
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_DONE, 1);
        sp->setEffectBasePoints(sp->getEffectBasePoints(0), 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 2);
        sp->setEffectTriggerSpell(35696, 2);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 2);
    }
    sp = Spell::checkAndReturnSpellEntry(35696);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0); //making this only for the visible effect
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 0); //no effect here
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    }

    //Shadow Trance should be removed on the first SB
    sp = Spell::checkAndReturnSpellEntry(17941);
    if (sp != nullptr)
    {
        sp->setAuraInterruptFlags(AURA_INTERRUPT_ON_CAST_SPELL);
    }

    //warlock: Empowered Corruption
    sp = Spell::checkAndReturnSpellEntry(32381);
    if (sp != nullptr)
    {
        sp->setEffectBasePoints(sp->getEffectBasePoints(0) * 6, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(32382);
    if (sp != nullptr)
    {
        sp->setEffectBasePoints(sp->getEffectBasePoints(0) * 6, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(32383);
    if (sp != nullptr)
    {
        sp->setEffectBasePoints(sp->getEffectBasePoints(0) * 6, 0);
    }

    //warlock - Demonic Tactics
    sp = Spell::checkAndReturnSpellEntry(30242);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_NULL, 0); //disable this. This is just blizz crap. Pure proof that they suck :P
        sp->setEffectImplicitTargetB(EFF_TARGET_PET, 1);
        //sp->EffectApplyAuraName[2] = SPELL_AURA_MOD_SPELL_CRIT_CHANCE; //lvl 1 has it fucked up :O
                                                                        // Zyres: No you fukced it up. This spell was defined few lines below.
        sp->setEffectImplicitTargetB(EFF_TARGET_PET, 2);
    }
    sp = Spell::checkAndReturnSpellEntry(30245);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_NULL, 0); //disable this. This is just blizz crap. Pure proof that they suck :P
        sp->setEffectImplicitTargetB(EFF_TARGET_PET, 1);
        sp->setEffectImplicitTargetB(EFF_TARGET_PET, 2);
    }
    sp = Spell::checkAndReturnSpellEntry(30246);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_NULL, 0); //disable this. This is just blizz crap. Pure proof that they suck :P
        sp->setEffectImplicitTargetB(EFF_TARGET_PET, 1);
        sp->setEffectImplicitTargetB(EFF_TARGET_PET, 2);
    }
    sp = Spell::checkAndReturnSpellEntry(30247);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_NULL, 0); //disable this. This is just blizz crap. Pure proof that they suck :P
        sp->setEffectImplicitTargetB(EFF_TARGET_PET, 1);
        sp->setEffectImplicitTargetB(EFF_TARGET_PET, 2);
    }
    sp = Spell::checkAndReturnSpellEntry(30248);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_NULL, 0); //disable this. This is just blizz crap. Pure proof that they suck :P
        sp->setEffectImplicitTargetB(EFF_TARGET_PET, 1);
        sp->setEffectImplicitTargetB(EFF_TARGET_PET, 2);
    }

    //warlock - Demonic Resilience
    sp = Spell::checkAndReturnSpellEntry(30319);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(30320);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(30321);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
    }

    //warlock - Improved Imp
    sp = Spell::checkAndReturnSpellEntry(18694);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(18695);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(18696);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    }

    //warlock - Demonic Brutality
    sp = Spell::checkAndReturnSpellEntry(18705);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(18706);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(18707);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    }

    //warlock - Improved Succubus
    sp = Spell::checkAndReturnSpellEntry(18754);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(18755);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(18756);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
    }

    //warlock - Fel Vitality
    sp = Spell::checkAndReturnSpellEntry(18731);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_PERCENT_STAT, 0);
        sp->setEffectMiscValue(3, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(18743);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_PERCENT_STAT, 0);
        sp->setEffectMiscValue(3, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(18744);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_PERCENT_STAT, 0);
        sp->setEffectMiscValue(3, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    }

    //warlock - Unholy Power
    sp = Spell::checkAndReturnSpellEntry(18769);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        //this is required since blizz uses spells for melee attacks while we use fixed functions
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
        sp->setEffectMiscValue(SCHOOL_NORMAL, 1);
        sp->setEffectBasePoints(sp->getEffectBasePoints(0), 1);
    }
    sp = Spell::checkAndReturnSpellEntry(18770);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        //this is required since blizz uses spells for melee attacks while we use fixed functions
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
        sp->setEffectMiscValue(SCHOOL_NORMAL, 1);
        sp->setEffectBasePoints(sp->getEffectBasePoints(0), 1);
    }
    sp = Spell::checkAndReturnSpellEntry(18771);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        //this is required since blizz uses spells for melee attacks while we use fixed functions
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
        sp->setEffectMiscValue(SCHOOL_NORMAL, 1);
        sp->setEffectBasePoints(sp->getEffectBasePoints(0), 1);
    }
    sp = Spell::checkAndReturnSpellEntry(18772);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        //this is required since blizz uses spells for melee attacks while we use fixed functions
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
        sp->setEffectMiscValue(SCHOOL_NORMAL, 1);
        sp->setEffectBasePoints(sp->getEffectBasePoints(0), 1);
    }
    sp = Spell::checkAndReturnSpellEntry(18773);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_ADD_PCT_MODIFIER, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
        //this is required since blizz uses spells for melee attacks while we use fixed functions
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 1);
        sp->setEffectMiscValue(SCHOOL_NORMAL, 1);
        sp->setEffectBasePoints(sp->getEffectBasePoints(0), 1);
    }

    //warlock - Master Demonologist - 25 spells here
    sp = Spell::checkAndReturnSpellEntry(23785);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(23784, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(23822);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(23830, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(23823);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(23831, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(23824);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(23832, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(23825);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(35708, 0);
    }
    //and the rest
    sp = Spell::checkAndReturnSpellEntry(23784);
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    sp = Spell::checkAndReturnSpellEntry(23830);
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    sp = Spell::checkAndReturnSpellEntry(23831);
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    sp = Spell::checkAndReturnSpellEntry(23832);
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    sp = Spell::checkAndReturnSpellEntry(35708);
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_PET, 0);
    sp = Spell::checkAndReturnSpellEntry(23759);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(23760);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(23761);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(23762);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(23826);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(23827);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
    }
    sp = Spell::checkAndReturnSpellEntry(23828);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA,0);
    }
    sp = Spell::checkAndReturnSpellEntry(23829);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
    }
    // Zyres: eeek
    for (uint32 i = 23833; i <= 23844; ++i)
    {
        sp = Spell::checkAndReturnSpellEntry(i);
        if (sp != nullptr)
        {
            sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
        }
    }
    sp = Spell::checkAndReturnSpellEntry(35702);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
        sp->setEffect(SPELL_EFFECT_NULL, 1); //hacks, we are handling this in another way
    }
    sp = Spell::checkAndReturnSpellEntry(35703);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
        sp->setEffect(SPELL_EFFECT_NULL, 1); //hacks, we are handling this in another way
    }
    sp = Spell::checkAndReturnSpellEntry(35704);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
        sp->setEffect(SPELL_EFFECT_NULL, 1); //hacks, we are handling this in another way
    }
    sp = Spell::checkAndReturnSpellEntry(35705);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
        sp->setEffect(SPELL_EFFECT_NULL, 1); //hacks, we are handling this in another way
    }
    sp = Spell::checkAndReturnSpellEntry(35706);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
        sp->setEffect(SPELL_EFFECT_NULL, 1); //hacks, we are handling this in another way
    }

    //warlock - Improved Drain Soul
    sp = Spell::checkAndReturnSpellEntry(18213);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_TARGET_DIE | static_cast<uint32>(PROC_TARGET_SELF));
        sp->setProcChance(100);
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(18371, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 0);
        sp->setEffect(SPELL_EFFECT_NULL, 2); //remove this effect
    }
    sp = Spell::checkAndReturnSpellEntry(18372);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_TARGET_DIE | static_cast<uint32>(PROC_TARGET_SELF));
        sp->setProcChance(100);
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(18371, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 0);
        sp->setEffect(SPELL_EFFECT_NULL, 2); //remove this effect
    }

    //Warlock Chaos bolt
    sp = Spell::checkAndReturnSpellEntry(50796);
    if (sp != nullptr)
    {
        sp->addAttributes(ATTRIBUTES_IGNORE_INVULNERABILITY);
        sp->setSchool(SCHOOL_FIRE);
    }

    sp = Spell::checkAndReturnSpellEntry(59170);
    if (sp != nullptr)
    {
        sp->addAttributes(ATTRIBUTES_IGNORE_INVULNERABILITY);
        sp->setSchool(SCHOOL_FIRE);
    }

    sp = Spell::checkAndReturnSpellEntry(59171);
    if (sp != nullptr)
    {
        sp->addAttributes(ATTRIBUTES_IGNORE_INVULNERABILITY);
        sp->setSchool(SCHOOL_FIRE);
    }

    sp = Spell::checkAndReturnSpellEntry(59172);
    if (sp != nullptr)
    {
        sp->addAttributes(ATTRIBUTES_IGNORE_INVULNERABILITY);
        sp->setSchool(SCHOOL_FIRE);
    }
    // End Warlock chaos bolt

    //Warlock Healthstones
    int HealthStoneID[8] = { 6201, 6202, 5699, 11729, 11730, 27230, 47871, 47878 };
    for (uint8 i = 0; i < 8; i++)
    {
        sp = Spell::checkAndReturnSpellEntry(HealthStoneID[i]);
        if (sp != nullptr)
        {
            sp->setReagent(0, 1);
        }
    }

    //////////////////////////////////////////
    // DRUID                                //
    //////////////////////////////////////////

    // Insert druid spell fixes here

    ////////////////////////////////////////////////////////////
    // Balance
    ////////////////////////////////////////////////////////////

    // Druid - Force of Nature
    sp = Spell::checkAndReturnSpellEntry(33831);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 0); //some land under target is used that gathers multiple targets ...
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 1);
    }

    ////////////////////////////////////////////////////////////
    //    Feral Combat
    ////////////////////////////////////////////////////////////

    // Druid - Infected Wounds
    sp = Spell::checkAndReturnSpellEntry(48483);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPECIFIC_SPELL);
    }

    sp = Spell::checkAndReturnSpellEntry(48484);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPECIFIC_SPELL);
    }

    sp = Spell::checkAndReturnSpellEntry(48485);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPECIFIC_SPELL);
    }

    // Druid - Bash - Interrupt effect
    sp = Spell::checkAndReturnSpellEntry(5211);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(32747, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(6798);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(32747, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(8983);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(32747, 1);
    }

    //Druid - Feral Swiftness
    sp = Spell::checkAndReturnSpellEntry(17002);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(24867, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(24866);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(24864, 1);
    }

    // Druid - Maim
    sp = Spell::checkAndReturnSpellEntry(22570);
    if (sp != nullptr)
    {
        sp->setAuraInterruptFlags(AURA_INTERRUPT_ON_UNUSED2);
        sp->custom_is_melee_spell = true;
    }
    sp = Spell::checkAndReturnSpellEntry(49802);
    if (sp != nullptr)
    {
        sp->setAuraInterruptFlags(AURA_INTERRUPT_ON_UNUSED2);
        sp->custom_is_melee_spell = true;
    }

    sp = Spell::checkAndReturnSpellEntry(20719); //feline grace
    if (sp != nullptr)
        sp->setEffect(SPELL_EFFECT_NULL, 0);

    // Druid - Feral Swiftness
    sp = Spell::checkAndReturnSpellEntry(17002);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(24867, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(24866);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(24864, 1);
    }

    // Druid - Frenzied Regeneration
    sp = Spell::checkAndReturnSpellEntry(22842);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_PERIODIC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(22845, 0);
    }

    // Druid - Primal Fury (talent)
    sp = Spell::checkAndReturnSpellEntry(37116);
    if (sp != nullptr)
        sp->setRequiredShapeShift(0);

    sp = Spell::checkAndReturnSpellEntry(37117);
    if (sp != nullptr)
        sp->setRequiredShapeShift(0);

    // Druid - Predatory Strikes
    uint32 mm = decimalToMask(FORM_BEAR) | decimalToMask(FORM_DIREBEAR) | decimalToMask(FORM_MOONKIN) | decimalToMask(FORM_CAT);

    sp = Spell::checkAndReturnSpellEntry(16972);
    if (sp != nullptr)
        sp->setRequiredShapeShift(mm);
    sp = Spell::checkAndReturnSpellEntry(16974);
    if (sp != nullptr)
        sp->setRequiredShapeShift(mm);
    sp = Spell::checkAndReturnSpellEntry(16975);
    if (sp != nullptr)
        sp->setRequiredShapeShift(mm);

    ////////////////////////////////////////////////////////////
    // Restoration
    ////////////////////////////////////////////////////////////

    // Druid - Natural Shapeshifter
    sp = Spell::checkAndReturnSpellEntry(16833);
    if (sp != nullptr)
        sp->setDurationIndex(0);
    sp = Spell::checkAndReturnSpellEntry(16834);
    if (sp != nullptr)
        sp->setDurationIndex(0);
    sp = Spell::checkAndReturnSpellEntry(16835);
    if (sp != nullptr)
        sp->setDurationIndex(0);

    sp = Spell::checkAndReturnSpellEntry(61177); // Northrend Inscription Research
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_NULL, 1);
        sp->setEffectBasePoints(0, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 1);
        sp->setEffectDieSides(0, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(61288); // Minor Inscription Research
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_NULL, 1);
        sp->setEffectBasePoints(0, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 1);
        sp->setEffectDieSides(0, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(60893); // Northrend Alchemy Research
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_NULL, 1);
        sp->setEffectBasePoints(0, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 1);
        sp->setEffectDieSides(0, 1);
    }
#if VERSION_STRING != Cata
    sp = Spell::checkAndReturnSpellEntry(46097); // Brutal Totem of Survival
    if (sp != nullptr)
    {
        sp->setEffectSpellClassMask(0x00100000 | 0x10000000 | 0x80000000, 0, 0);
        sp->setEffectSpellClassMask(0x08000000, 0, 1);
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(43860); // Totem of Survival
    if (sp != nullptr)
    {
        sp->setEffectSpellClassMask(0x00100000 | 0x10000000 | 0x80000000, 0, 0);
        sp->setEffectSpellClassMask(0x08000000, 0, 1);;
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(43861); // Merciless Totem of Survival
    if (sp != nullptr)
    {
        sp->setEffectSpellClassMask(0x00100000 | 0x10000000 | 0x80000000, 0, 0);
        sp->setEffectSpellClassMask(0x08000000, 0, 1);
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(43862); // Vengeful Totem of Survival
    if (sp != nullptr)
    {
        sp->setEffectSpellClassMask(0x00100000 | 0x10000000 | 0x80000000, 0, 0);
        sp->setEffectSpellClassMask(0x08000000, 0, 1);
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
    }
    sp = Spell::checkAndReturnSpellEntry(60564); // Savage Gladiator's Totem of Survival
    if (sp != nullptr)
    {
        sp->setEffectSpellClassMask(0x00100000 | 0x10000000 | 0x80000000, 0, 0);
        sp->setEffectSpellClassMask(0x08000000, 0, 1);
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(60565, 1); // Savage Magic
    }
    sp = Spell::checkAndReturnSpellEntry(60571); // Hateful Gladiator's Totem of Survival
    if (sp != nullptr)
    {
        sp->setEffectSpellClassMask(0x00100000 | 0x10000000 | 0x80000000, 0, 0);
        sp->setEffectSpellClassMask(0x08000000, 0, 1);
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(60566, 1); // Hateful Magic
    }
    sp = Spell::checkAndReturnSpellEntry(60572); // Deadly Gladiator's Totem of Survival
    if (sp != nullptr)
    {
        sp->setEffectSpellClassMask(0x00100000 | 0x10000000 | 0x80000000, 0, 0);
        sp->setEffectSpellClassMask(0x08000000, 0, 1);
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(60567, 1); // Deadly Magic
    }
    sp = Spell::checkAndReturnSpellEntry(60567); // Deadly Magic
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
    sp = Spell::checkAndReturnSpellEntry(46098); // Brutal Totem of Third WInd
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectSpellClassMask(0x00000080, 0, 0);
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(46099, 1); // Brutal Gladiator's Totem of the Third Wind
    }
    sp = Spell::checkAndReturnSpellEntry(34138); // Totem of the Third Wind
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectSpellClassMask(0x00000080, 0, 0);
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(34132, 1); // Gladiator's Totem of the Third Wind
    }
    sp = Spell::checkAndReturnSpellEntry(42370); // Merciless Totem of the Third WInd
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectSpellClassMask(0x00000080, 0, 0);
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(42371, 1); // Merciless Gladiator's Totem of the Third Wind
    }
    sp = Spell::checkAndReturnSpellEntry(43728); // Vengeful Totem of Third WInd
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectSpellClassMask(0x00000080, 0, 0);
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(43729, 1); // Vengeful Gladiator's Totem of the Third Wind
    }
#endif
    //////////////////////////////////////////
    // ITEMS                                //
    //////////////////////////////////////////

    // Insert items spell fixes here

    //Compact Harvest Reaper
    sp = Spell::checkAndReturnSpellEntry(4078);
    if (sp != nullptr)
    {
        sp->setDurationIndex(6);
    }

    //Graccu's Mince Meat Fruitcake
    sp = Spell::checkAndReturnSpellEntry(25990);
    if (sp != nullptr)
    {
        sp->setEffectAmplitude(1000, 1);
    }

    //Extract Gas
    sp = Spell::checkAndReturnSpellEntry(30427);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_DUMMY, 0);
    }

    //Relic - Idol of the Unseen Moon
    sp = Spell::checkAndReturnSpellEntry(43739);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
    }

    //Primal Instinct - Idol of Terror proc
    sp = Spell::checkAndReturnSpellEntry(43738);
    if (sp != nullptr)
    {
        sp->custom_self_cast_only = true;
    }

    //Thunderfury
    sp = Spell::checkAndReturnSpellEntry(21992);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER, 2); // cebernic: for enemies not self
    }

    // Sigil of the Unfaltering Knight
    sp = Spell::checkAndReturnSpellEntry(62147);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPECIFIC_SPELL);
        sp->custom_proc_interval = 45000;
    }

    // Deadly Aggression - triggered by Deadly Gladiator's Relic/Idol/Libram/Totem
    sp = Spell::checkAndReturnSpellEntry(60549);
    if (sp != nullptr)
    {
        // effect 1 and 2 are the same... dunno why
        sp->setEffect(SPELL_EFFECT_NULL, 1);
    }

    // Furious Gladiator's Libram of Fortitude - triggered by LK Arena 4 Gladiator's Relic/Idol/Libram/Totem
    sp = Spell::checkAndReturnSpellEntry(60551);
    if (sp != nullptr)
    {
        // effect 1 and 2 are the same... dunno why
        sp->setEffect(SPELL_EFFECT_NULL, 1);
    }

    // Relentless Aggression - triggered by LK Arena 5 Gladiator's Relic/Idol/Libram/Totem
    sp = Spell::checkAndReturnSpellEntry(60553);
    if (sp != nullptr)
    {
        // effect 1 and 2 are the same... dunno why
        sp->setEffect(SPELL_EFFECT_NULL, 1);
    }

    // Savage Aggression - triggered by Savage Gladiator's Relic/Idol/Libram/Totem
    sp = Spell::checkAndReturnSpellEntry(60544);
    if (sp != nullptr)
    {
        // effect 1 and 2 are the same... dunno why
        sp->setEffect(SPELL_EFFECT_NULL, 1);
    }

    // Sigil of Haunted Dreams
    sp = Spell::checkAndReturnSpellEntry(60826);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPECIFIC_SPELL);
        sp->custom_proc_interval = 45000;
    }

    //Totem of the Third Wind - bad range
    sp = Spell::checkAndReturnSpellEntry(34132);
    if (sp != nullptr)
    {
        sp->setRangeIndex(5);
    }
    sp = Spell::checkAndReturnSpellEntry(42371);
    if (sp != nullptr)
    {
        sp->setRangeIndex(5);
    }
    sp = Spell::checkAndReturnSpellEntry(43729);
    if (sp != nullptr)
    {
        sp->setRangeIndex(5);
    }
    sp = Spell::checkAndReturnSpellEntry(46099);
    if (sp != nullptr)
    {
        sp->setRangeIndex(5);
    }

    // Eye of Acherus, our phase shift mode messes up the control :/
    sp = Spell::checkAndReturnSpellEntry(51852);
    if (sp != nullptr)
        sp->setEffect(SPELL_EFFECT_NULL, 0);


    //Ashtongue Talisman of Equilibrium
    // DankoDJ: To set the same value several times makes no sense!
    sp = Spell::checkAndReturnSpellEntry(40442);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(40452, 0);
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(40445, 1);
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 2);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 2);
        sp->setProcChance(25);
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectTriggerSpell(40446, 2);
        sp->setMaxstack(1);
    }

    //Ashtongue Talisman of Acumen
    // DankoDJ: To set the same value several times makes no sense!
    sp = Spell::checkAndReturnSpellEntry(40438);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setProcChance(10);
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectTriggerSpell(40441, 0);
        sp->setMaxstack(1);
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(40440, 1);
    }
    // Drums of war targets surrounding party members instead of us
    sp = Spell::checkAndReturnSpellEntry(35475);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_PARTY, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_PARTY, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 2);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 0);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 1);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 2);
    }

    // Drums of Battle targets surrounding party members instead of us
    sp = Spell::checkAndReturnSpellEntry(35476);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_PARTY, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_PARTY, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 2);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 0);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 1);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 2);
    }

    // Drums of Panic targets surrounding creatures instead of us
    sp = Spell::checkAndReturnSpellEntry(35474);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_ENEMIES_AROUND_CASTER, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 2);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 0);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 1);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 2);
    }

    // Drums of Restoration targets surrounding party members instead of us
    sp = Spell::checkAndReturnSpellEntry(35478);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_PARTY, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_PARTY, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 2);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 0);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 1);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 2);
    }
    // Drums of Speed targets surrounding party members instead of us
    sp = Spell::checkAndReturnSpellEntry(35477);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_PARTY, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_PARTY, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 2);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 0);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 1);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 2);
    }

    //all Drums
    sp = Spell::checkAndReturnSpellEntry(35474);
    if (sp != nullptr)
        sp->setRequiredShapeShift(0);
    sp = Spell::checkAndReturnSpellEntry(35475);
    if (sp != nullptr)
        sp->setRequiredShapeShift(0);
    sp = Spell::checkAndReturnSpellEntry(35476);
    if (sp != nullptr)
        sp->setRequiredShapeShift(0);
    sp = Spell::checkAndReturnSpellEntry(35477);
    if (sp != nullptr)
        sp->setRequiredShapeShift(0);
    sp = Spell::checkAndReturnSpellEntry(35478);
    if (sp != nullptr)
        sp->setRequiredShapeShift(0);

    //Purify helboar meat
    sp = Spell::checkAndReturnSpellEntry(29200);
    if (sp != nullptr)
    {
        sp->setReagent(0, 1);
        sp->setReagentCount(0, 1);
    }

    //Thorium Grenade
    sp = Spell::checkAndReturnSpellEntry(19769);
    if (sp != nullptr)
    {
        sp->removeInterruptFlags(CAST_INTERRUPT_ON_MOVEMENT);
    }

    //M73 Frag Grenade
    sp = Spell::checkAndReturnSpellEntry(13808);
    if (sp != nullptr)
    {
        sp->removeInterruptFlags(CAST_INTERRUPT_ON_MOVEMENT);
    }

    //Iron Grenade
    sp = Spell::checkAndReturnSpellEntry(4068);
    if (sp != nullptr)
    {
        sp->removeInterruptFlags(CAST_INTERRUPT_ON_MOVEMENT);
    }

    //Frost Grenade
    sp = Spell::checkAndReturnSpellEntry(39965);
    if (sp != nullptr)
    {
        sp->removeInterruptFlags(CAST_INTERRUPT_ON_MOVEMENT);
    }

    //Adamantine Grenade
    sp = Spell::checkAndReturnSpellEntry(30217);
    if (sp != nullptr)
    {
        sp->removeInterruptFlags(CAST_INTERRUPT_ON_MOVEMENT);
    }

    ///////////////////////////////////////////////////////////////
    // Trinket Fixes        Please keep nice and clean :)
    ///////////////////////////////////////////////////////////////

    // Citrine Pendant of Golden Healing
    sp = Spell::checkAndReturnSpellEntry(25608);        //    http://www.wowhead.com/?item=20976
    if (sp != nullptr)
    {
        //Overrides any spell coefficient calculation - DBCStores.h
        sp->Dspell_coef_override = 0;    //DD&DH
        sp->OTspell_coef_override = 0;    //HOT&DOT
    }

    //Figurine - Shadowsong Panther
    sp = Spell::checkAndReturnSpellEntry(46784);        //    http://www.wowhead.com/?item=35702
    if (sp != nullptr)
        sp->addAttributesEx(ATTRIBUTESEX_NOT_BREAK_STEALTH);

    // Infernal Protection
    sp = Spell::checkAndReturnSpellEntry(36488);            //    http://www.wowhead.com/?spell=36488
    if (sp != nullptr)
        sp->setEffectImplicitTargetA(EFF_TARGET_SINGLE_FRIEND, 0);


    //Fury of the Five Flights
    sp = Spell::checkAndReturnSpellEntry(60313);
    if (sp != nullptr)
    {
        sp->setMaxstack(20);
    }

    //Pendant of the Violet Eye
    sp = Spell::checkAndReturnSpellEntry(35095);
    if (sp != nullptr)
    {
        sp->custom_self_cast_only = true;
    }

    //////////////////////////////////////////
    // BOSSES                                //
    //////////////////////////////////////////

    // Insert boss spell fixes here

    // Major Domo - Magic Reflection
    sp = Spell::checkAndReturnSpellEntry(20619);
    if (sp != nullptr)
    {
        for (uint8 i = 0; i < 3; ++i)
        {
            if (sp->getEffectImplicitTargetA(i) > 0)
                sp->setEffectImplicitTargetA(EFF_TARGET_ALL_FRIENDLY_IN_AREA, i);
            if (sp->getEffectImplicitTargetB(i) > 0)
                sp->setEffectImplicitTargetB(EFF_TARGET_ALL_FRIENDLY_IN_AREA, i);
        }
    }

    // Major Domo - Damage Shield
    sp = Spell::checkAndReturnSpellEntry(21075);
    if (sp != nullptr)
    {
        for (uint8 i = 0; i < 3; ++i)
        {
            if (sp->getEffectImplicitTargetA(i) > 0)
                sp->setEffectImplicitTargetA(EFF_TARGET_ALL_FRIENDLY_IN_AREA, i);
            if (sp->getEffectImplicitTargetB(i) > 0)
                sp->setEffectImplicitTargetB(EFF_TARGET_ALL_FRIENDLY_IN_AREA, i);
        }
    }

    // Dark Glare
    sp = Spell::checkAndReturnSpellEntry(26029);
    if (sp != nullptr)
        sp->cone_width = 15.0f; // 15 degree cone

    // Drain Power (Malacrass) // bugged - the charges fade even when refreshed with new ones. This makes them everlasting.
    sp = Spell::checkAndReturnSpellEntry(44131);
    if (sp != nullptr)
        sp->setDurationIndex(21);
    sp = Spell::checkAndReturnSpellEntry(44132);
    if (sp != nullptr)
        sp->setDurationIndex(21);

    // Zul'jin spell, proc from Creeping Paralysis
    sp = Spell::checkAndReturnSpellEntry(43437);
    if (sp != nullptr)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 1);
    }

    //Bloodboil
    sp = Spell::checkAndReturnSpellEntry(42005);
    if (sp != nullptr)
    {
        sp->setMaxTargets(5);
    }

    //Doom
    sp = Spell::checkAndReturnSpellEntry(31347);
    if (sp != nullptr)
    {
        sp->setMaxTargets(1);
    }
    //Shadow of Death
    sp = Spell::checkAndReturnSpellEntry(40251);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PERIODIC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(0, 0);
    }

    sp = Spell::checkAndReturnSpellEntry(9036);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(20584, 1);
    }

    sp = Spell::checkAndReturnSpellEntry(24379);   //bg Restoration
    if (sp != nullptr)
    {
        sp->setEffectTriggerSpell(23493, 0);
    }

    sp = Spell::checkAndReturnSpellEntry(23493);   //bg Restoration
    if (sp != nullptr)
    {
        sp->setEffectTriggerSpell(24379, 0);
    }

    sp = Spell::checkAndReturnSpellEntry(5246);    // why self?
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(20511, 0); // cebernic: this just real spell
        sp->setEffectImplicitTargetA(EFF_TARGET_NONE, 0);
    }

    //////////////////////////////////////////
    // DEATH KNIGHT                            //
    //////////////////////////////////////////

    // Insert Death Knight spells here

    // Unholy Aura - Ranks 1
    sp = Spell::checkAndReturnSpellEntry(50391);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS, 0);
        sp->setEffect(SPELL_EFFECT_APPLY_GROUP_AREA_AURA, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(50392, 1);
        sp->setEffect(SPELL_EFFECT_APPLY_GROUP_AREA_AURA, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
    }
    // Unholy Aura - Ranks 2
    sp = Spell::checkAndReturnSpellEntry(50392);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS, 0);
        sp->setEffect(SPELL_EFFECT_APPLY_GROUP_AREA_AURA, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
        sp->setEffectTriggerSpell(50392, 1);
        sp->setEffect(SPELL_EFFECT_APPLY_GROUP_AREA_AURA, 1);
        sp->setEffectImplicitTargetA(EFF_TARGET_SELF, 1);
    }

    //    Empower Rune Weapon
    sp = Spell::checkAndReturnSpellEntry(47568);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_ACTIVATE_RUNES, 2);
        sp->setEffectBasePoints(1, 2);
        sp->setEffectMiscValue(RUNE_UNHOLY, 2);
    }

    // Frost Presence
    sp = Spell::checkAndReturnSpellEntry(48263);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_BASE_RESISTANCE_PCT, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT, 1);
        sp->setEffectBasePoints(9, 1);
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_DAMAGE_TAKEN, 2);
    }

    //    Unholy Presence
    sp = Spell::checkAndReturnSpellEntry(48265);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_HASTE, 0);
        sp->setEffectBasePoints(14, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_INCREASE_SPEED, 1);
        sp->setEffectBasePoints(14, 1);
    }

    // DEATH AND DECAY
    sp = sSpellCustomizations.GetSpellInfo(49937);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE, 0);
        sp->setEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA, 0);
    }

    sp = sSpellCustomizations.GetSpellInfo(49936);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE, 0);
        sp->setEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA, 0);
    }

    sp = sSpellCustomizations.GetSpellInfo(49938);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE, 0);
        sp->setEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA, 0);
    }

    sp = sSpellCustomizations.GetSpellInfo(43265);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE, 0);
        sp->setEffect(SPELL_EFFECT_PERSISTENT_AREA_AURA, 0);
    }

    // Vengeance
    sp = sSpellCustomizations.GetSpellInfo(93099);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_ANY_DAMAGE_VICTIM);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(76691, 0);
    }

    ///////////////////////////////////////////////////////////
    //    Path of Frost
    ///////////////////////////////////////////////////////////
    sp = Spell::checkAndReturnSpellEntry(3714);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_WATER_WALK, 0);
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
    }

    // Rune Strike
    sp = Spell::checkAndReturnSpellEntry(56815);
    if (sp != nullptr)
    {
        sp->addAttributes(ATTRIBUTES_CANT_BE_DPB);
    }

    CreateDummySpell(56817);
    sp = Spell::checkAndReturnSpellEntry(56817);
    if (sp != nullptr)
    {
        sp->setDurationIndex(28);
        sp->setEffect(SPELL_EFFECT_APPLY_AURA, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_DUMMY, 0);
    }

    //Frost Strike
    sp = Spell::checkAndReturnSpellEntry(49143);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_CANT_BE_DPB);
    }
    sp = Spell::checkAndReturnSpellEntry(51416);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_CANT_BE_DPB);
    }
    sp = Spell::checkAndReturnSpellEntry(51417);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_CANT_BE_DPB);
    }
    sp = Spell::checkAndReturnSpellEntry(51418);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_CANT_BE_DPB);
    }
    sp = Spell::checkAndReturnSpellEntry(51419);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_CANT_BE_DPB);
    }
    sp = Spell::checkAndReturnSpellEntry(55268);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_CANT_BE_DPB);
    }

    // Noggenfogger elixir - reduce size effect
    sp = Spell::checkAndReturnSpellEntry(16595);
    if (sp != nullptr)
    {
        sp->setEffectApplyAuraName(SPELL_AURA_MOD_SCALE, 0);
        sp->setEffectBasePoints(-50, 0);
        sp->setMaxstack(1);
    }

    sp = Spell::checkAndReturnSpellEntry(46584);
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_DUMMY, 0);
        sp->setEffect(SPELL_EFFECT_NULL, 1);
        sp->setEffect(SPELL_EFFECT_NULL, 2);
    }

    //Other Librams
    //Libram of Saints Departed and Libram of Zeal
    sp = Spell::checkAndReturnSpellEntry(34263);
    if (sp != nullptr)
    {
        sp->custom_self_cast_only = true;
        sp->setProcChance(100);
    }

    //Libram of Avengement
    sp = Spell::checkAndReturnSpellEntry(34260);
    if (sp != nullptr)
    {
        sp->custom_self_cast_only = true;
        sp->setProcChance(100);
    }

    //Libram of Mending
    sp = Spell::checkAndReturnSpellEntry(43742);
    if (sp != nullptr)
    {
        sp->custom_self_cast_only = true;
        sp->setProcChance(100);
    }

    // Recently Bandaged - is debuff
    sp = Spell::checkAndReturnSpellEntry(11196);
    if (sp != nullptr)
    {
        sp->setAttributes(ATTRIBUTES_IGNORE_INVULNERABILITY);
    }

    sp = Spell::checkAndReturnSpellEntry(44856);        // Bash'ir Phasing Device
    if (sp != nullptr)
        sp->setAuraInterruptFlags(AURA_INTERRUPT_ON_LEAVE_AREA);


    sp = Spell::checkAndReturnSpellEntry(24574);        // Zandalarian Hero Badge 24590 24575
    if (sp != nullptr)
    {
        sp->setEffect(SPELL_EFFECT_TRIGGER_SPELL, 0);
        sp->setEffectTriggerSpell(24590, 0);
        sp->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
    }

    //Tempfix for Stone Statues
    sp = Spell::checkAndReturnSpellEntry(32253);
    if (sp != nullptr)
        sp->setDurationIndex(64);
    sp = Spell::checkAndReturnSpellEntry(32787);
    if (sp != nullptr)
        sp->setDurationIndex(64);
    sp = Spell::checkAndReturnSpellEntry(32788);
    if (sp != nullptr)
        sp->setDurationIndex(64);
    sp = Spell::checkAndReturnSpellEntry(32790);
    if (sp != nullptr)
        sp->setDurationIndex(64);
    sp = Spell::checkAndReturnSpellEntry(32791);
    if (sp != nullptr)
        sp->setDurationIndex(64);

    //////////////////////////////////////////////////////
    // GAME-OBJECT SPELL FIXES                          //
    //////////////////////////////////////////////////////

    // Blessing of Zim'Torga
    sp = Spell::checkAndReturnSpellEntry(51729);
    if (sp)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_SCRIPTED_OR_SINGLE_TARGET, 0);
    }

    // Blessing of Zim'Abwa
    sp = Spell::checkAndReturnSpellEntry(51265);
    if (sp)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_SCRIPTED_OR_SINGLE_TARGET, 0);
    }

    // Blessing of Zim'Rhuk
    sp = Spell::checkAndReturnSpellEntry(52051);
    if (sp)
    {
        sp->setEffectImplicitTargetA(EFF_TARGET_SCRIPTED_OR_SINGLE_TARGET, 0);
    }

    // Ritual of Summoning summons a GameObject that triggers an inexistant spell.
    // This will copy an existant Summon Player spell used for another Ritual Of Summoning
    // to the one taught by Warlock trainers.
    sp = Spell::checkAndReturnSpellEntry(7720);
    if (sp)
    {
        const uint32 ritOfSummId = 62330;
        CreateDummySpell(ritOfSummId);
        SpellInfo * ritOfSumm = sSpellCustomizations.GetSpellInfo(ritOfSummId);
        if (ritOfSumm != NULL)
        {
            memcpy(ritOfSumm, sp, sizeof(SpellInfo));
            ritOfSumm->setId(ritOfSummId);
        }
    }
    //Persistent Shield
    sp = Spell::checkAndReturnSpellEntry(26467);
	if (sp)
	{
		sp->setEffectTriggerSpell(26470, 0);
		sp->addAttributes(ATTRIBUTES_NO_VISUAL_AURA | ATTRIBUTES_PASSIVE);
		sp->setDurationIndex(0);
		sp->setProcFlags(PROC_ON_CAST_SPELL);
	}
    //Gravity Bomb
    sp = Spell::checkAndReturnSpellEntry(63024);
	if (sp)
	{
		sp->setEffectBasePoints(0, 0);
		sp->setEffect(SPELL_EFFECT_NULL, 1);
		sp->setEffect(SPELL_EFFECT_NULL, 2);
		sp->setTargetAuraState(0);
		sp->setCasterAuraSpell(0);
		sp->setCasterAuraState(0);
		sp->setCasterAuraStateNot(0);
		sp->setTargetAuraStateNot(0);
		sp->setTargetAuraSpell(0);
		sp->setCasterAuraSpellNot(0);
		sp->setTargetAuraSpellNot(0);
		sp->addAttributes(ATTRIBUTES_NEGATIVE);
	}
    // War Stomp
    sp = Spell::checkAndReturnSpellEntry(20549);
    if (sp)
    {
        sp->setEffectMechanic(MECHANIC_STUNNED, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_ENEMY_IN_AREA, 0);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 0);
    }

    // Fan of knives
    sp = Spell::checkAndReturnSpellEntry(51723);
    if (sp != nullptr)
    {

        sp->setEffectMechanic(MECHANIC_SHACKLED, 0);
        sp->setEffectImplicitTargetA(EFF_TARGET_ALL_ENEMY_IN_AREA, 0);
        sp->setEffectImplicitTargetB(EFF_TARGET_NONE, 0);
    }

    //Mage - firestarter talent ranks 1 & 2
    // overwrite procs, should only proc on these 2 spellgroups.
    sp = Spell::checkAndReturnSpellEntry(44442);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
        sp->setProcChance(50);
    }
    sp = Spell::checkAndReturnSpellEntry(44443);
    if (sp != nullptr)
    {
        sp->setProcFlags(PROC_ON_CAST_SPELL);
    }
}

