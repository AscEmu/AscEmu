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
    SpellInfo* sp = new SpellInfo;
    memset(sp, 0, sizeof(SpellInfo));
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
    sWorld.dummyspells.push_back(sp);
}

void Modify_EffectBasePoints(SpellInfo* sp)
{
    if (sp == nullptr)
    {
        LOG_ERROR("Something tried to call with an invalid spell pointer!");
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

void Set_missing_spellLevel(SpellInfo* sp)
{
    if (sp == nullptr)
    {
        LOG_ERROR("Something tried to call with an invalid spell pointer!");
        return;
    }

    //stupid spell ranking problem
    if (sp->spellLevel == 0)
    {
        uint32 new_level = 0;

        // 16/03/08 Zyres: just replaced name assignes with spell ids. \todo remove not teachable spells.
        switch (sp->Id)
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
            if (sp->Effect[0] == SPELL_EFFECT_LEARN_SPELL)
                teachspell = sp->EffectTriggerSpell[0];
            else if (sp->Effect[1] == SPELL_EFFECT_LEARN_SPELL)
                teachspell = sp->EffectTriggerSpell[1];
            else if (sp->Effect[2] == SPELL_EFFECT_LEARN_SPELL)
                teachspell = sp->EffectTriggerSpell[2];

            if (teachspell)
            {
                SpellInfo* spellInfo;
                spellInfo = CheckAndReturnSpellEntry(teachspell);
                spellInfo->spellLevel = new_level;
                sp->spellLevel = new_level;
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

void Modify_RecoveryTime(SpellInfo* sp)
{
    if (sp == nullptr)
    {
        LOG_ERROR("Something tried to call with an invalid spell pointer!");
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

void ApplyObsoleteNameHash(SpellInfo* sp)
{
    switch (sp->Id)
    {
        case 5215:
        case 6783:
        case 8152:
        case 9913:
        case 24450:
        case 24451:
        case 24452:
        case 24453:
        case 24454:
        case 24455:
        case 42932:
        {
            sp->custom_NameHash = SPELL_HASH_PROWL;
            break;
        }
        case 58984:
        case 62196:
        case 62199:
        {
            sp->custom_NameHash = SPELL_HASH_SHADOWMELD;
            break;
        }
        case 3043:
        case 18545:
        case 52604:
        {
            sp->custom_NameHash = SPELL_HASH_SCORPID_STING;
            break;
        }
        case 19386:
        case 24131:
        case 24132:
        case 24133:
        case 24134:
        case 24135:
        case 24335:
        case 24336:
        case 26180:
        case 26233:
        case 26748:
        case 27068:
        case 27069:
        case 41186:
        case 49009:
        case 49010:
        case 49011:
        case 49012:
        case 65877:
        case 65878:
        {
            sp->custom_NameHash = SPELL_HASH_WYVERN_STING;
            break;
        }
        case 1978:
        case 13549:
        case 13550:
        case 13551:
        case 13552:
        case 13553:
        case 13554:
        case 13555:
        case 25295:
        case 27016:
        case 31975:
        case 35511:
        case 36984:
        case 38859:
        case 38914:
        case 39182:
        case 49000:
        case 49001:
        {
            sp->custom_NameHash = SPELL_HASH_SERPENT_STING;
            break;
        }
        case 3034:
        case 31407:
        case 37551:
        case 39413:
        case 65881:
        case 67991:
        case 67992:
        case 67993:
        {
            sp->custom_NameHash = SPELL_HASH_VIPER_STING;
            break;
        }
        case 54197:
        {
            sp->custom_NameHash = SPELL_HASH_COLD_WEATHER_FLYING;
            break;
        }
        case 6143:
        case 6144:
        case 8461:
        case 8462:
        case 8463:
        case 8464:
        case 10177:
        case 10178:
        case 15044:
        case 25641:
        case 27396:
        case 28609:
        case 32796:
        case 32797:
        case 43012:
        {
            sp->custom_NameHash = SPELL_HASH_FROST_WARD;
            break;
        }
        case 543:
        case 1035:
        case 8457:
        case 8458:
        case 8459:
        case 8460:
        case 10223:
        case 10224:
        case 10225:
        case 10226:
        case 15041:
        case 27128:
        case 27395:
        case 37844:
        case 43010:
        {
            sp->custom_NameHash = SPELL_HASH_FIRE_WARD;
            break;
        }
        case 20185:
        case 20267:
        case 20271:
        case 28775:
        case 57774:
        {
            sp->custom_NameHash = SPELL_HASH_JUDGEMENT_OF_LIGHT;
            break;
        }
        case 20186:
        case 20268:
        case 53408:
        {
            sp->custom_NameHash = SPELL_HASH_JUDGEMENT_OF_WISDOM;
            break;
        }
        case 20184:
        case 53407:
        {
            sp->custom_NameHash = SPELL_HASH_JUDGEMENT_OF_JUSTICE;
            break;
        }
        case 31804:
        {
            sp->custom_NameHash = SPELL_HASH_JUDGEMENT_OF_VENGEANCE;
            break;
        }
        case 53733:
        {
            sp->custom_NameHash = SPELL_HASH_JUDGEMENT_OF_CORRUPTION;
            break;
        }
        case 20187:
        {
            sp->custom_NameHash = SPELL_HASH_JUDGEMENT_OF_RIGHTEOUSNESS;
            break;
        }
        case 589:
        case 594:
        case 970:
        case 992:
        case 2767:
        case 10892:
        case 10893:
        case 10894:
        case 11639:
        case 14032:
        case 15654:
        case 17146:
        case 19776:
        case 23268:
        case 23952:
        case 24212:
        case 25367:
        case 25368:
        case 27605:
        case 30854:
        case 30898:
        case 34441:
        case 34941:
        case 34942:
        case 37275:
        case 41355:
        case 46560:
        case 48124:
        case 48125:
        case 57778:
        case 59864:
        case 60005:
        case 60446:
        case 65541:
        case 68088:
        case 68089:
        case 68090:
        case 72318:
        case 72319:
        {
            sp->custom_NameHash = SPELL_HASH_SHADOW_WORD__PAIN;
            break;
        }
        case 44394:
        case 44395:
        case 44396:
        case 44413:
        {
            sp->custom_NameHash = SPELL_HASH_INCANTER_S_ABSORPTION;
            break;
        }
        case 4:
        {
            sp->custom_NameHash = SPELL_HASH_WORD_OF_RECALL_OTHER;
            break;
        }
        case 32546:
        case 48119:
        case 48120:
        {
            sp->custom_NameHash = SPELL_HASH_BINDING_HEAL;
            break;
        }
        case 2061:
        case 9472:
        case 9473:
        case 9474:
        case 10915:
        case 10916:
        case 10917:
        case 17137:
        case 17138:
        case 17843:
        case 25233:
        case 25235:
        case 27608:
        case 38588:
        case 42420:
        case 43431:
        case 43516:
        case 43575:
        case 48070:
        case 48071:
        case 56331:
        case 56919:
        case 66104:
        case 68023:
        case 68024:
        case 68025:
        case 71595:
        case 71782:
        case 71783:
        {
            sp->custom_NameHash = SPELL_HASH_FLASH_HEAL;
            break;
        }
        case 10833:
        case 16067:
        case 18091:
        case 20883:
        case 22893:
        case 22920:
        case 22940:
        case 24857:
        case 30451:
        case 30661:
        case 31457:
        case 32935:
        case 34793:
        case 35314:
        case 35927:
        case 36032:
        case 37126:
        case 38342:
        case 38344:
        case 38538:
        case 38881:
        case 40837:
        case 40881:
        case 42894:
        case 42896:
        case 42897:
        case 49198:
        case 50545:
        case 51797:
        case 51830:
        case 56969:
        case 58462:
        case 59257:
        case 59909:
        case 65791:
        case 67997:
        case 67998:
        case 67999:
        {
            sp->custom_NameHash = SPELL_HASH_ARCANE_BLAST;
            break;
        }
        case 2139:
        case 15122:
        case 19715:
        case 20537:
        case 20788:
        case 29443:
        case 29961:
        case 31596:
        case 31999:
        case 37470:
        case 51610:
        case 65790:
        {
            sp->custom_NameHash = SPELL_HASH_COUNTERSPELL;
            break;
        }
        case 3252:
        case 5221:
        case 6800:
        case 8992:
        case 9829:
        case 9830:
        case 27001:
        case 27002:
        case 27555:
        case 48571:
        case 48572:
        case 49121:
        case 49165:
        case 61548:
        case 61549:
        {
            sp->custom_NameHash = SPELL_HASH_SHRED;
            break;
        }
        case 6807:
        case 6808:
        case 6809:
        case 7092:
        case 8972:
        case 9745:
        case 9880:
        case 9881:
        case 12161:
        case 15793:
        case 17156:
        case 20751:
        case 26996:
        case 27553:
        case 34298:
        case 48479:
        case 48480:
        case 51875:
        case 52506:
        case 54459:
        {
            sp->custom_NameHash = SPELL_HASH_MAUL;
            break;
        }
        case 19820:
        case 22689:
        case 31041:
        case 33917:
        case 39164:
        case 41439:
        case 42389:
        case 57657:
        case 59988:
        case 71925:
        {
            sp->custom_NameHash = SPELL_HASH_MANGLE;
            break;
        }
        case 563:
        case 8921:
        case 8924:
        case 8925:
        case 8926:
        case 8927:
        case 8928:
        case 8929:
        case 9833:
        case 9834:
        case 9835:
        case 15798:
        case 20690:
        case 21669:
        case 22206:
        case 23380:
        case 24957:
        case 26987:
        case 26988:
        case 27737:
        case 31270:
        case 31401:
        case 32373:
        case 32415:
        case 37328:
        case 43545:
        case 45821:
        case 45900:
        case 47072:
        case 48462:
        case 48463:
        case 52502:
        case 57647:
        case 59987:
        case 65856:
        case 67944:
        case 67945:
        case 67946:
        case 75329:
        {
            sp->custom_NameHash = SPELL_HASH_MOONFIRE;
            break;
        }
        case 33876:
        case 33982:
        case 33983:
        case 48565:
        case 48566:
        {
            sp->custom_NameHash = SPELL_HASH_MANGLE__CAT_;
            break;
        }
        case 33878:
        case 33986:
        case 33987:
        case 48563:
        case 48564:
        {
            sp->custom_NameHash = SPELL_HASH_MANGLE__BEAR_;
            break;
        }
        case 45477:
        case 49723:
        case 49896:
        case 49903:
        case 49904:
        case 49909:
        case 50349:
        case 52372:
        case 52378:
        case 53549:
        case 55313:
        case 55331:
        case 59011:
        case 59131:
        case 60952:
        case 66021:
        case 67718:
        case 67881:
        case 67938:
        case 67939:
        case 67940:
        case 69916:
        case 70589:
        case 70591:
        {
            sp->custom_NameHash = SPELL_HASH_ICY_TOUCH;
            break;
        }
        case 45902:
        case 49926:
        case 49927:
        case 49928:
        case 49929:
        case 49930:
        case 52374:
        case 52377:
        case 59130:
        case 60945:
        case 61696:
        case 66215:
        case 66975:
        case 66976:
        case 66977:
        case 66978:
        case 66979:
        {
            sp->custom_NameHash = SPELL_HASH_BLOOD_STRIKE;
            break;
        }
        case 55050:
        case 55258:
        case 55259:
        case 55260:
        case 55261:
        case 55262:
        case 55978:
        {
            sp->custom_NameHash = SPELL_HASH_HEART_STRIKE;
            break;
        }
        case 45463:
        case 45469:
        case 45470:
        case 49923:
        case 49924:
        case 49998:
        case 49999:
        case 53639:
        case 66188:
        case 66950:
        case 66951:
        case 66952:
        case 66953:
        case 71489:
        {
            sp->custom_NameHash = SPELL_HASH_DEATH_STRIKE;
            break;
        }
        case 43568:
        case 49143:
        case 51416:
        case 51417:
        case 51418:
        case 51419:
        case 55268:
        case 60951:
        case 66047:
        case 66196:
        case 66958:
        case 66959:
        case 66960:
        case 66961:
        case 66962:
        case 67935:
        case 67936:
        case 67937:
        {
            sp->custom_NameHash = SPELL_HASH_FROST_STRIKE;
            break;
        }
        case 6789:
        case 17925:
        case 17926:
        case 27223:
        case 28412:
        case 30500:
        case 30741:
        case 32709:
        case 33130:
        case 34437:
        case 35954:
        case 38065:
        case 39661:
        case 41070:
        case 44142:
        case 46283:
        case 47541:
        case 47632:
        case 47633:
        case 47859:
        case 47860:
        case 49892:
        case 49893:
        case 49894:
        case 49895:
        case 50668:
        case 52375:
        case 52376:
        case 53769:
        case 55209:
        case 55210:
        case 55320:
        case 56362:
        case 59134:
        case 60949:
        case 62900:
        case 62901:
        case 62902:
        case 62903:
        case 62904:
        case 65820:
        case 66019:
        case 67929:
        case 67930:
        case 67931:
        case 68139:
        case 68140:
        case 68141:
        case 71490:
        {
            sp->custom_NameHash = SPELL_HASH_DEATH_COIL;
            break;
        }
        case 10321:
        case 23590:
        case 23591:
        case 35170:
        case 41467:
        case 43838:
        case 54158:
        {
            sp->custom_NameHash = SPELL_HASH_JUDGEMENT;
            break;
        }
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
        {
            sp->custom_NameHash = SPELL_HASH_HOLY_LIGHT;
            break;
        }
        case 29964:
        case 29965:
        case 31661:
        case 33041:
        case 33042:
        case 33043:
        case 35250:
        case 37289:
        case 42949:
        case 42950:
        {
            sp->custom_NameHash = SPELL_HASH_DRAGON_S_BREATH;
            break;
        }
        case 1831:
        case 11113:
        case 13018:
        case 13019:
        case 13020:
        case 13021:
        case 15091:
        case 15744:
        case 16046:
        case 17145:
        case 17277:
        case 20229:
        case 22424:
        case 23039:
        case 23113:
        case 23331:
        case 25049:
        case 27133:
        case 30092:
        case 30600:
        case 33061:
        case 33933:
        case 36278:
        case 38064:
        case 38536:
        case 38712:
        case 39001:
        case 39038:
        case 42944:
        case 42945:
        case 58970:
        case 60290:
        case 61362:
        case 66044:
        case 68312:
        case 70407:
        case 71151:
        {
            sp->custom_NameHash = SPELL_HASH_BLAST_WAVE;
            break;
        }
        case 49018:
        case 49529:
        case 49530:
        {
            sp->custom_NameHash = SPELL_HASH_SUDDEN_DOOM;
            break;
        }
        case 29166:
        {
            sp->custom_NameHash = SPELL_HASH_INNERVATE;
            break;
        }
        case 100:
        case 6178:
        case 7370:
        case 11578:
        case 20508:
        case 22120:
        case 22911:
        case 24023:
        case 24193:
        case 24315:
        case 24408:
        case 25821:
        case 25999:
        case 26184:
        case 26185:
        case 26186:
        case 26202:
        case 28343:
        case 29320:
        case 29847:
        case 31426:
        case 31733:
        case 32323:
        case 33709:
        case 34846:
        case 35412:
        case 35570:
        case 35754:
        case 36058:
        case 36140:
        case 36509:
        case 37511:
        case 38461:
        case 39574:
        case 40602:
        case 41581:
        case 42003:
        case 43519:
        case 43651:
        case 43807:
        case 44357:
        case 44884:
        case 49758:
        case 50582:
        case 51492:
        case 51756:
        case 51842:
        case 52538:
        case 52577:
        case 52856:
        case 53148:
        case 54460:
        case 55317:
        case 55530:
        case 57627:
        case 58619:
        case 58991:
        case 59040:
        case 59611:
        case 60067:
        case 61685:
        case 62563:
        case 62613:
        case 62614:
        case 62874:
        case 62960:
        case 62961:
        case 62977:
        case 63003:
        case 63010:
        case 63661:
        case 63665:
        case 64591:
        case 64719:
        case 65927:
        case 66481:
        case 68282:
        case 68284:
        case 68301:
        case 68307:
        case 68321:
        case 68498:
        case 68501:
        case 68762:
        case 68763:
        case 68764:
        case 71553:
        case 74399:
        {
            sp->custom_NameHash = SPELL_HASH_CHARGE;
            break;
        }
        case 10400:
        case 15567:
        case 15568:
        case 15569:
        case 16311:
        case 16312:
        case 16313:
        case 58784:
        case 58791:
        case 58792:
        {
            sp->custom_NameHash = SPELL_HASH_FLAMETONGUE_WEAPON__PASSIVE_;
            break;
        }
        case 5171:
        case 6434:
        case 6774:
        case 30470:
        case 43547:
        case 60847:
        {
            sp->custom_NameHash = SPELL_HASH_SLICE_AND_DICE;
            break;
        }
        case 15407:
        case 16568:
        case 17165:
        case 17311:
        case 17312:
        case 17313:
        case 17314:
        case 18807:
        case 22919:
        case 23953:
        case 25387:
        case 26044:
        case 26143:
        case 28310:
        case 29407:
        case 29570:
        case 32417:
        case 35507:
        case 37276:
        case 37330:
        case 37621:
        case 38243:
        case 40842:
        case 42396:
        case 43512:
        case 46562:
        case 48155:
        case 48156:
        case 52586:
        case 54339:
        case 54805:
        case 57779:
        case 57941:
        case 58381:
        case 59367:
        case 59974:
        case 60006:
        case 60472:
        case 65488:
        case 68042:
        case 68043:
        case 68044:
        {
            sp->custom_NameHash = SPELL_HASH_MIND_FLAY;
            break;
        }
        case 8092:
        case 8102:
        case 8103:
        case 8104:
        case 8105:
        case 8106:
        case 10945:
        case 10946:
        case 10947:
        case 13860:
        case 15587:
        case 17194:
        case 17287:
        case 20830:
        case 25372:
        case 25375:
        case 26048:
        case 31516:
        case 37531:
        case 38259:
        case 41374:
        case 48126:
        case 48127:
        case 52722:
        case 58850:
        case 60447:
        case 60453:
        case 60500:
        case 65492:
        case 68038:
        case 68039:
        case 68040:
        {
            sp->custom_NameHash = SPELL_HASH_MIND_BLAST;
            break;
        }
        case 32379:
        case 32409:
        case 32996:
        case 41375:
        case 47697:
        case 48157:
        case 48158:
        case 51818:
        case 56920:
        {
            sp->custom_NameHash = SPELL_HASH_SHADOW_WORD__DEATH;
            break;
        }
        case 34914:
        case 34916:
        case 34917:
        case 34919:
        case 48159:
        case 48160:
        case 52723:
        case 52724:
        case 60501:
        case 64085:
        case 65490:
        case 68091:
        case 68092:
        case 68093:
        {
            sp->custom_NameHash = SPELL_HASH_VAMPIRIC_TOUCH;
            break;
        }
        case 53742:
        {
            sp->custom_NameHash = SPELL_HASH_BLOOD_CORRUPTION;
            break;
        }
        case 31803:
        {
            sp->custom_NameHash = SPELL_HASH_HOLY_VENGEANCE;
            break;
        }
        case 8034:
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
            sp->custom_NameHash = SPELL_HASH_FROSTBRAND_ATTACK;
            break;
        }
        case 57975:
        case 57978:
        {
            sp->custom_NameHash = SPELL_HASH_WOUND_POISON_VII;
            break;
        }
        case 57974:
        case 57977:
        {
            sp->custom_NameHash = SPELL_HASH_WOUND_POISON_VI;
            break;
        }
        case 27188:
        case 27189:
        {
            sp->custom_NameHash = SPELL_HASH_WOUND_POISON_V;
            break;
        }
        case 13224:
        case 13227:
        {
            sp->custom_NameHash = SPELL_HASH_WOUND_POISON_IV;
            break;
        }
        case 13223:
        case 13226:
        {
            sp->custom_NameHash = SPELL_HASH_WOUND_POISON_III;
            break;
        }
        case 13222:
        case 13225:
        {
            sp->custom_NameHash = SPELL_HASH_WOUND_POISON_II;
            break;
        }
        case 13218:
        case 13219:
        case 30984:
        case 36974:
        case 39665:
        case 43461:
        case 54074:
        case 65962:
        {
            sp->custom_NameHash = SPELL_HASH_WOUND_POISON;
            break;
        }
        case 8679:
        case 8680:
        case 28428:
        case 41189:
        case 59242:
        {
            sp->custom_NameHash = SPELL_HASH_INSTANT_POISON; 
            break;
        }
        case 8685:
        case 8686:
        {
            sp->custom_NameHash = SPELL_HASH_INSTANT_POISON_II; 
            break;
        }
        case 8688:
        case 8689:
        {
            sp->custom_NameHash = SPELL_HASH_INSTANT_POISON_III; 
            break;
        }
        case 11335:
        case 11338:
        {
            sp->custom_NameHash = SPELL_HASH_INSTANT_POISON_IV; 
            break;
        }
        case 57965:
        case 57968:
        {
            sp->custom_NameHash = SPELL_HASH_INSTANT_POISON_IX; 
            break;
        }
        case 11336:
        case 11339:
        {
            sp->custom_NameHash = SPELL_HASH_INSTANT_POISON_V; 
            break;
        }
        case 11337:
        case 11340:
        {
            sp->custom_NameHash = SPELL_HASH_INSTANT_POISON_VI; 
            break;
        }
        case 26890:
        case 26891:
        {
            sp->custom_NameHash = SPELL_HASH_INSTANT_POISON_VII; 
            break;
        }
        case 57964:
        case 57967:
        {
            sp->custom_NameHash = SPELL_HASH_INSTANT_POISON_VIII; 
            break;
        }
        case 2818:
        case 2823:
        case 3583:
        case 10022:
        case 13582:
        case 21787:
        case 21788:
        case 32970:
        case 32971:
        case 34616:
        case 34655:
        case 34657:
        case 36872:
        case 38519:
        case 38520:
        case 41191:
        case 41192:
        case 41485:
        case 43580:
        case 43581:
        case 56145:
        case 56149:
        case 59479:
        case 59482:
        case 63755:
        case 63756:
        case 67710:
        case 67711:
        case 68315:
        case 72329:
        case 72330:
        {
            sp->custom_NameHash = SPELL_HASH_DEADLY_POISON; 
            break;
        }
        case 2819:
        case 2824:
        {
            sp->custom_NameHash = SPELL_HASH_DEADLY_POISON_II; 
            break;
        }
        case 11353:
        case 11355:
        {
            sp->custom_NameHash = SPELL_HASH_DEADLY_POISON_III; 
            break;
        }
        case 11354:
        case 11356:
        {
            sp->custom_NameHash = SPELL_HASH_DEADLY_POISON_IV; 
            break;
        }
        case 57970:
        case 57973:
        {
            sp->custom_NameHash = SPELL_HASH_DEADLY_POISON_IX; 
            break;
        }
        case 25349:
        case 25351:
        {
            sp->custom_NameHash = SPELL_HASH_DEADLY_POISON_V; 
            break;
        }
        case 26967:
        case 26968:
        {
            sp->custom_NameHash = SPELL_HASH_DEADLY_POISON_VI; 
            break;
        }
        case 27186:
        case 27187:
        {
            sp->custom_NameHash = SPELL_HASH_DEADLY_POISON_VII; 
            break;
        }
        case 57969:
        case 57972:
        {
            sp->custom_NameHash = SPELL_HASH_DEADLY_POISON_VIII; 
            break;
        }

        case 3408:
        case 3409:
        case 25809:
        case 30981:
        case 44289:
        {
            sp->custom_NameHash = SPELL_HASH_CRIPPLING_POISON; 
            break;
        }

        case 5760:
        case 5761:
        case 25810:
        case 34615:
        case 41190:
        {
            sp->custom_NameHash = SPELL_HASH_MIND_NUMBING_POISON; 
            break;
        }

        case 51664:
        case 51665:
        case 51667:
        case 51668:
        case 51669:
        {
            sp->custom_NameHash = SPELL_HASH_CUT_TO_THE_CHASE; 
            break;
        }

        case 51625:
        case 51626:
        {
            sp->custom_NameHash = SPELL_HASH_DEADLY_BREW; 
            break;
        }

        case 51692:
        case 51693:
        case 51696:
        {
            sp->custom_NameHash = SPELL_HASH_WAYLAY; 
            break;
        }

        case 15337:
        case 15338:
        case 49694:
        case 59000:
        {
            sp->custom_NameHash = SPELL_HASH_IMPROVED_SPIRIT_TAP; 
            break;
        }

        case 34753:
        case 34754:
        case 34859:
        case 34860:
        case 63724:
        case 63725:
        {
            sp->custom_NameHash = SPELL_HASH_HOLY_CONCENTRATION; 
            break;
        }

        case 47509:
        case 47511:
        case 47515:
        case 47753:
        case 54704:
        {
            sp->custom_NameHash = SPELL_HASH_DIVINE_AEGIS; 
            break;
        }

        case 63625:
        case 63626:
        case 63627:
        case 63675:
        case 75999:
        {
            sp->custom_NameHash = SPELL_HASH_IMPROVED_DEVOURING_PLAGUE; 
            break;
        }

        case 15286:
        case 15290:
        case 71269:
        {
            sp->custom_NameHash = SPELL_HASH_VAMPIRIC_EMBRACE; 
            break;
        }

        case 63534:
        case 63542:
        case 63543:
        case 63544:
        {
            sp->custom_NameHash = SPELL_HASH_EMPOWERED_RENEW; 
            break;
        }

        case 33191:
        case 33192:
        case 33193:
        case 33196:
        case 33197:
        case 33198:
        {
            sp->custom_NameHash = SPELL_HASH_MISERY; 
            break;
        }

        case 33076:
        case 33110:
        case 41635:
        case 41637:
        case 44583:
        case 44586:
        case 46045:
        case 48110:
        case 48111:
        case 48112:
        case 48113:
        {
            sp->custom_NameHash = SPELL_HASH_PRAYER_OF_MENDING; 
            break;
        }

        case 15270:
        case 15271:
        case 15335:
        case 15336:
        {
            sp->custom_NameHash = SPELL_HASH_SPIRIT_TAP; 
            break;
        }

        case 20375:
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
            sp->custom_NameHash = SPELL_HASH_SEAL_OF_COMMAND; 
            break;
        }

        case 9799:
        case 25988:
        case 25997:
        {
            sp->custom_NameHash = SPELL_HASH_EYE_FOR_AN_EYE; 
            break;
        }

        case 43742:
        {
            sp->custom_NameHash = SPELL_HASH_GRACE_OF_THE_NAARU; 
            break;
        }

        case 31785:
        case 31786:
        case 33776:
        {
            sp->custom_NameHash = SPELL_HASH_SPIRITUAL_ATTUNEMENT; 
            break;
        }

        case 49182:
        case 49500:
        case 49501:
        case 51789:
        case 55225:
        case 55226:
        case 64855:
        case 64856:
        case 64858:
        case 64859:
        {
            sp->custom_NameHash = SPELL_HASH_BLADE_BARRIER; 
            break;
        }

        case 41434:
        case 41435:
        {
            sp->custom_NameHash = SPELL_HASH_THE_TWIN_BLADES_OF_AZZINOTH; 
            break;
        }

        case 6307:
        case 7804:
        case 7805:
        case 11766:
        case 11767:
        case 27268:
        case 47982:
        {
            sp->custom_NameHash = SPELL_HASH_BLOOD_PACT; 
            break;
        }

        case 54424:
        case 57564:
        case 57565:
        case 57566:
        case 57567:
        {
            sp->custom_NameHash = SPELL_HASH_FEL_INTELLIGENCE; 
            break;
        }

        case 32233:
        case 32234:
        case 32600:
        case 62137:
        case 63623:
        case 65220:
        {
            sp->custom_NameHash = SPELL_HASH_AVOIDANCE; 
            break;
        }

        case 19481:
        case 20435:
        case 41002:
        {
            sp->custom_NameHash = SPELL_HASH_PARANOIA; 
            break;
        }

        case 134:
        case 2947:
        case 2949:
        case 8316:
        case 8317:
        case 8318:
        case 8319:
        case 11350:
        case 11351:
        case 11770:
        case 11771:
        case 11772:
        case 11773:
        case 11968:
        case 13376:
        case 18968:
        case 19627:
        case 20322:
        case 20323:
        case 20324:
        case 20326:
        case 20327:
        case 27269:
        case 27486:
        case 27489:
        case 30513:
        case 30514:
        case 32749:
        case 32751:
        case 35265:
        case 35266:
        case 36907:
        case 37282:
        case 37283:
        case 37318:
        case 37434:
        case 38732:
        case 38733:
        case 38855:
        case 38893:
        case 38901:
        case 38902:
        case 38933:
        case 38934:
        case 47983:
        case 47998:
        case 61144:
        case 63778:
        case 63779:
        case 71514:
        case 71515:
        {
            sp->custom_NameHash = SPELL_HASH_FIRE_SHIELD; 
            break;
        }

        case 4511:
        case 4630:
        case 8611:
        case 8612:
        case 20329:
        case 29309:
        case 29315:
        {
            sp->custom_NameHash = SPELL_HASH_PHASE_SHIFT; 
            break;
        }

        case 17767:
        case 17776:
        case 17850:
        case 17851:
        case 17852:
        case 17853:
        case 17854:
        case 17855:
        case 17856:
        case 17857:
        case 17859:
        case 17860:
        case 20387:
        case 20388:
        case 20389:
        case 20390:
        case 20391:
        case 20392:
        case 27272:
        case 27491:
        case 36472:
        case 47987:
        case 47988:
        case 48003:
        case 48004:
        case 49739:
        case 54501:
        {
            sp->custom_NameHash = SPELL_HASH_CONSUME_SHADOWS;
            break;
        }

        case 3680:
        case 7870:
        case 7880:
        case 12845:
        case 20408:
        {
            sp->custom_NameHash = SPELL_HASH_LESSER_INVISIBILITY;
            break;
        }

        case 45:
        case 11876:
        case 15593:
        case 16727:
        case 16740:
        case 19482:
        case 20549:
        case 24375:
        case 25188:
        case 27758:
        case 28125:
        case 28725:
        case 31408:
        case 31480:
        case 31755:
        case 33707:
        case 35238:
        case 36835:
        case 38682:
        case 38750:
        case 38911:
        case 39313:
        case 40936:
        case 41534:
        case 46026:
        case 56427:
        case 59705:
        case 60960:
        case 61065:
        {
            sp->custom_NameHash = SPELL_HASH_WAR_STOMP;
            break;
        }

        case 1050:
        case 7812:
        case 7885:
        case 19438:
        case 19439:
        case 19440:
        case 19441:
        case 19442:
        case 19443:
        case 19444:
        case 19445:
        case 19446:
        case 19447:
        case 20381:
        case 20382:
        case 20383:
        case 20384:
        case 20385:
        case 20386:
        case 22651:
        case 27273:
        case 27492:
        case 30115:
        case 33587:
        case 34661:
        case 47985:
        case 47986:
        case 48001:
        case 48002:
        {
            sp->custom_NameHash = SPELL_HASH_SACRIFICE;
            break;
        }

        case 26094:
        case 26189:
        case 26190:
        case 27366:
        case 34388:
        case 61580:
        case 63900:
        {
            sp->custom_NameHash = SPELL_HASH_THUNDERSTOMP;
            break;
        }

        case 3149:
        case 24604:
        case 30636:
        case 35942:
        case 50728:
        case 59274:
        case 64491:
        case 64492:
        case 64493:
        case 64494:
        case 64495:
        {
            sp->custom_NameHash = SPELL_HASH_FURIOUS_HOWL;
            break;
        }

        case 1850:
        case 9821:
        case 33357:
        case 36589:
        case 43317:
        case 44029:
        case 44531:
        case 61684:
        {
            sp->custom_NameHash = SPELL_HASH_DASH; 
            break;
        }

        case 23145:
        case 23146:
        case 23149:
        case 23150:
        case 29903:
        case 37156:
        case 37588:
        case 40279:
        case 43187:
        {
            sp->custom_NameHash = SPELL_HASH_DIVE; 
            break;
        }

        case 26064:
        case 26065:
        case 40087:
        case 46327:
        {
            sp->custom_NameHash = SPELL_HASH_SHELL_SHIELD; 
            break;
        }

        case 31707:
        case 72898:
        {
            sp->custom_NameHash = SPELL_HASH_WATERBOLT; 
            break;
        }

        case 58877:
        case 58879:
        {
            sp->custom_NameHash = SPELL_HASH_SPIRIT_HUNT; 
            break;
        }

        case 58857:
        {
            sp->custom_NameHash = SPELL_HASH_TWIN_HOWL; 
            break;
        }

        case 5211:
        case 6798:
        case 8983:
        case 25515:
        case 43612:
        case 57094:
        case 58861:
        {
            sp->custom_NameHash = SPELL_HASH_BASH; 
            break;
        }

        case 8024:
        case 8027:
        case 8030:
        case 16339:
        case 16341:
        case 16342:
        case 25489:
        case 58785:
        case 58789:
        case 58790:
        case 65979:
        {
            sp->custom_NameHash = SPELL_HASH_FLAMETONGUE_WEAPON; 
            break;
        }

        case 1464:
        case 8820:
        case 11430:
        case 11604:
        case 11605:
        case 25241:
        case 25242:
        case 34620:
        case 47474:
        case 47475:
        case 50782:
        case 50783:
        case 52026:
        case 67028:
        {
            sp->custom_NameHash = SPELL_HASH_SLAM; 
            break;
        }

        case 10:
        case 1196:
        case 6141:
        case 6142:
        case 8364:
        case 8427:
        case 8428:
        case 10185:
        case 10186:
        case 10187:
        case 10188:
        case 10189:
        case 10190:
        case 15783:
        case 19099:
        case 20680:
        case 21096:
        case 21367:
        case 25019:
        case 26607:
        case 27085:
        case 27384:
        case 27618:
        case 29458:
        case 29951:
        case 30093:
        case 31266:
        case 31581:
        case 33418:
        case 33624:
        case 33634:
        case 34167:
        case 34183:
        case 34356:
        case 37263:
        case 37671:
        case 38646:
        case 39416:
        case 41382:
        case 41482:
        case 42198:
        case 42208:
        case 42209:
        case 42210:
        case 42211:
        case 42212:
        case 42213:
        case 42937:
        case 42938:
        case 42939:
        case 42940:
        case 44178:
        case 46195:
        case 47727:
        case 49034:
        case 50715:
        case 56936:
        case 58693:
        case 59278:
        case 59369:
        case 59854:
        case 61085:
        case 62576:
        case 62577:
        case 62602:
        case 62603:
        case 62706:
        case 64642:
        case 64653:
        case 70362:
        case 70421:
        case 71118:
        {
            sp->custom_NameHash = SPELL_HASH_BLIZZARD; 
            break;
        }

        case 17:
        case 592:
        case 600:
        case 3747:
        case 6065:
        case 6066:
        case 10898:
        case 10899:
        case 10900:
        case 10901:
        case 11647:
        case 11835:
        case 11974:
        case 17139:
        case 20697:
        case 22187:
        case 25217:
        case 25218:
        case 27607:
        case 29408:
        case 32595:
        case 35944:
        case 36052:
        case 41373:
        case 44175:
        case 44291:
        case 46193:
        case 48065:
        case 48066:
        case 66099:
        case 68032:
        case 68033:
        case 68034:
        case 71548:
        case 71780:
        case 71781:
        {
            sp->custom_NameHash = SPELL_HASH_POWER_WORD__SHIELD; 
            break;
        }

        case 25:
        case 56:
        case 2880:
        case 9179:
        case 17308:
        case 20170:
        case 20310:
        case 23454:
        case 24647:
        case 27880:
        case 34510:
        case 35856:
        case 39568:
        case 46441:
        case 52093:
        case 52847:
        {
            sp->custom_NameHash = SPELL_HASH_STUN; 
            break;
        }

        case 53:
        case 2589:
        case 2590:
        case 2591:
        case 7159:
        case 8721:
        case 11279:
        case 11280:
        case 11281:
        case 15582:
        case 15657:
        case 22416:
        case 25300:
        case 26863:
        case 30992:
        case 34614:
        case 37685:
        case 48656:
        case 48657:
        case 52540:
        case 58471:
        case 63754:
        case 71410:
        case 72427:
        {
            sp->custom_NameHash = SPELL_HASH_BACKSTAB; 
            break;
        }

        case 67:
        case 9452:
        case 26016:
        case 26017:
        case 36002:
        {
            sp->custom_NameHash = SPELL_HASH_VINDICATION; 
            break;
        }

        case 72:
        case 1671:
        case 1672:
        case 11972:
        case 29704:
        case 33871:
        case 35178:
        case 36988:
        case 38233:
        case 41180:
        case 41197:
        case 70964:
        case 72194:
        case 72196:
        {
            sp->custom_NameHash = SPELL_HASH_SHIELD_BASH; 
            break;
        }

        case 78:
        case 284:
        case 285:
        case 1608:
        case 11564:
        case 11565:
        case 11566:
        case 11567:
        case 25286:
        case 25710:
        case 25712:
        case 29426:
        case 29567:
        case 29707:
        case 30324:
        case 31827:
        case 41975:
        case 45026:
        case 47449:
        case 47450:
        case 52221:
        case 53395:
        case 57846:
        case 59035:
        case 59607:
        case 62444:
        case 69566:
        {
            sp->custom_NameHash = SPELL_HASH_HEROIC_STRIKE; 
            break;
        }

        case 116:
        case 205:
        case 837:
        case 7322:
        case 8406:
        case 8407:
        case 8408:
        case 9672:
        case 10179:
        case 10180:
        case 10181:
        case 11538:
        case 12675:
        case 12737:
        case 13322:
        case 13439:
        case 15043:
        case 15497:
        case 15530:
        case 16249:
        case 16799:
        case 17503:
        case 20297:
        case 20792:
        case 20806:
        case 20819:
        case 20822:
        case 21369:
        case 23102:
        case 23412:
        case 24942:
        case 25304:
        case 27071:
        case 27072:
        case 28478:
        case 28479:
        case 29457:
        case 29926:
        case 29954:
        case 30942:
        case 31296:
        case 31622:
        case 32364:
        case 32370:
        case 32984:
        case 34347:
        case 35316:
        case 36279:
        case 36710:
        case 36990:
        case 37930:
        case 38238:
        case 38534:
        case 38645:
        case 38697:
        case 38826:
        case 39064:
        case 40429:
        case 40430:
        case 41384:
        case 41486:
        case 42719:
        case 42803:
        case 42841:
        case 42842:
        case 43083:
        case 43428:
        case 44606:
        case 44843:
        case 46035:
        case 46987:
        case 49037:
        case 50378:
        case 50721:
        case 54791:
        case 55802:
        case 55807:
        case 56775:
        case 56837:
        case 57665:
        case 57825:
        case 58457:
        case 58535:
        case 59017:
        case 59251:
        case 59280:
        case 59638:
        case 59855:
        case 61087:
        case 61461:
        case 61590:
        case 61730:
        case 61747:
        case 62583:
        case 62601:
        case 63913:
        case 65807:
        case 68003:
        case 68004:
        case 68005:
        case 69274:
        case 69573:
        case 70277:
        case 70327:
        case 71318:
        case 71420:
        case 72007:
        case 72166:
        case 72167:
        case 72501:
        case 72502:
        {
            sp->custom_NameHash = SPELL_HASH_FROSTBOLT; 
            break;
        }

        case 118:
        case 12824:
        case 12825:
        case 12826:
        case 13323:
        case 14621:
        case 15534:
        case 27760:
        case 28271:
        case 28272:
        case 28285:
        case 29124:
        case 29848:
        case 30838:
        case 34639:
        case 36840:
        case 38245:
        case 38896:
        case 41334:
        case 43309:
        case 46280:
        case 58537:
        case 61025:
        case 61305:
        case 61721:
        case 61780:
        case 65801:
        case 66043:
        case 68311:
        case 71319:
        {
            sp->custom_NameHash = SPELL_HASH_POLYMORPH; 
            break;
        }

        case 122:
        case 865:
        case 1194:
        case 1225:
        case 6131:
        case 6132:
        case 9915:
        case 10230:
        case 10231:
        case 11831:
        case 12674:
        case 12748:
        case 14907:
        case 15063:
        case 15531:
        case 15532:
        case 22645:
        case 27088:
        case 27387:
        case 29849:
        case 30094:
        case 31250:
        case 32192:
        case 32365:
        case 34326:
        case 36989:
        case 38033:
        case 39035:
        case 39063:
        case 42917:
        case 43426:
        case 44177:
        case 45905:
        case 46555:
        case 57629:
        case 57668:
        case 58458:
        case 59253:
        case 59995:
        case 61376:
        case 61462:
        case 62597:
        case 62605:
        case 63912:
        case 65792:
        case 68198:
        case 69060:
        case 69571:
        case 70209:
        case 71320:
        case 71929:
        {
            sp->custom_NameHash = SPELL_HASH_FROST_NOVA; 
            break;
        }

        case 133:
        case 143:
        case 145:
        case 3140:
        case 8400:
        case 8401:
        case 8402:
        case 9053:
        case 9487:
        case 9488:
        case 10148:
        case 10149:
        case 10150:
        case 10151:
        case 10578:
        case 11839:
        case 11921:
        case 11985:
        case 12466:
        case 13140:
        case 13375:
        case 13438:
        case 14034:
        case 15228:
        case 15242:
        case 15536:
        case 15662:
        case 15665:
        case 16101:
        case 16412:
        case 16413:
        case 16415:
        case 16788:
        case 17290:
        case 18082:
        case 18105:
        case 18108:
        case 18199:
        case 18392:
        case 18796:
        case 19391:
        case 19816:
        case 20420:
        case 20678:
        case 20692:
        case 20714:
        case 20793:
        case 20797:
        case 20808:
        case 20811:
        case 20815:
        case 20823:
        case 21072:
        case 21159:
        case 21162:
        case 21402:
        case 21549:
        case 22088:
        case 23024:
        case 23411:
        case 24374:
        case 24611:
        case 25306:
        case 27070:
        case 29456:
        case 29925:
        case 29953:
        case 30218:
        case 30534:
        case 30691:
        case 30943:
        case 30967:
        case 31262:
        case 31620:
        case 32363:
        case 32369:
        case 32414:
        case 32491:
        case 33417:
        case 33793:
        case 33794:
        case 34083:
        case 34348:
        case 34653:
        case 36711:
        case 36805:
        case 36920:
        case 36971:
        case 37111:
        case 37329:
        case 37463:
        case 38641:
        case 38692:
        case 38824:
        case 39267:
        case 40554:
        case 40598:
        case 40877:
        case 41383:
        case 41484:
        case 42802:
        case 42832:
        case 42833:
        case 42834:
        case 42853:
        case 44189:
        case 44202:
        case 44237:
        case 45580:
        case 45595:
        case 45748:
        case 46164:
        case 46988:
        case 47074:
        case 49512:
        case 52282:
        case 54094:
        case 54095:
        case 54096:
        case 57628:
        case 59994:
        case 61567:
        case 61909:
        case 62796:
        case 63789:
        case 63815:
        case 66042:
        case 68310:
        case 68926:
        case 69570:
        case 69583:
        case 69668:
        case 70282:
        case 70409:
        case 70754:
        case 71153:
        case 71500:
        case 71501:
        case 71504:
        case 71748:
        case 71928:
        case 72023:
        case 72024:
        case 72163:
        case 72164:
        {
            sp->custom_NameHash = SPELL_HASH_FIREBALL; 
            break;
        }

        case 139:
        case 6074:
        case 6075:
        case 6076:
        case 6077:
        case 6078:
        case 8362:
        case 10927:
        case 10928:
        case 10929:
        case 11640:
        case 22168:
        case 23895:
        case 25058:
        case 25221:
        case 25222:
        case 25315:
        case 27606:
        case 28807:
        case 31325:
        case 34423:
        case 36679:
        case 36969:
        case 37260:
        case 37978:
        case 38210:
        case 41456:
        case 44174:
        case 45859:
        case 46192:
        case 46563:
        case 47079:
        case 48067:
        case 48068:
        case 49263:
        case 56332:
        case 57777:
        case 60004:
        case 61967:
        case 62333:
        case 62441:
        case 66177:
        case 66537:
        case 67675:
        case 68035:
        case 68036:
        case 68037:
        case 71932:
        {
            sp->custom_NameHash = SPELL_HASH_RENEW; break;
        }

        case 172:
        case 6222:
        case 6223:
        case 7648:
        case 11671:
        case 11672:
        case 13530:
        case 16402:
        case 16985:
        case 17510:
        case 18088:
        case 18376:
        case 18656:
        case 21068:
        case 23642:
        case 25311:
        case 27216:
        case 28829:
        case 30938:
        case 31405:
        case 32063:
        case 32197:
        case 37113:
        case 37961:
        case 39212:
        case 39621:
        case 41988:
        case 47782:
        case 47812:
        case 47813:
        case 56898:
        case 57645:
        case 58811:
        case 60016:
        case 61563:
        case 65810:
        case 68133:
        case 68134:
        case 68135:
        case 70602:
        case 70904:
        case 71937:
        {
            sp->custom_NameHash = SPELL_HASH_CORRUPTION; break;
        }

        case 184:
        {
            sp->custom_NameHash = SPELL_HASH_FIRE_SHIELD_II; break;
        }

        case 228:
        {
            sp->custom_NameHash = SPELL_HASH_POLYMORPH__CHICKEN; break;
        }

        case 331:
        case 332:
        case 547:
        case 913:
        case 939:
        case 959:
        case 8005:
        case 10395:
        case 10396:
        case 11986:
        case 12491:
        case 12492:
        case 15982:
        case 25357:
        case 25391:
        case 25396:
        case 26097:
        case 38330:
        case 43548:
        case 48700:
        case 49272:
        case 49273:
        case 51586:
        case 52868:
        case 55597:
        case 57785:
        case 58980:
        case 59083:
        case 60012:
        case 61569:
        case 67528:
        case 68318:
        case 69958:
        case 71133:
        case 75382:
        {
            sp->custom_NameHash = SPELL_HASH_HEALING_WAVE; break;
        }

        case 339:
        case 1062:
        case 5195:
        case 5196:
        case 9852:
        case 9853:
        case 11922:
        case 12747:
        case 19970:
        case 19971:
        case 19972:
        case 19973:
        case 19974:
        case 19975:
        case 20654:
        case 20699:
        case 21331:
        case 22127:
        case 22415:
        case 22800:
        case 24648:
        case 26071:
        case 26989:
        case 27010:
        case 28858:
        case 31287:
        case 32173:
        case 33844:
        case 37823:
        case 40363:
        case 53308:
        case 53313:
        case 57095:
        case 65857:
        case 66070:
        {
            sp->custom_NameHash = SPELL_HASH_ENTANGLING_ROOTS; break;
        }

        case 348:
        case 707:
        case 1094:
        case 2941:
        case 8981:
        case 9034:
        case 9275:
        case 9276:
        case 11665:
        case 11667:
        case 11668:
        case 11962:
        case 11984:
        case 12742:
        case 15505:
        case 15506:
        case 15570:
        case 15661:
        case 15732:
        case 15733:
        case 17883:
        case 18542:
        case 20294:
        case 20787:
        case 20800:
        case 20826:
        case 25309:
        case 27215:
        case 29928:
        case 36637:
        case 36638:
        case 37668:
        case 38805:
        case 38806:
        case 41958:
        case 44267:
        case 44518:
        case 46042:
        case 46191:
        case 47810:
        case 47811:
        case 75383:
        {
            sp->custom_NameHash = SPELL_HASH_IMMOLATE; break;
        }

        case 379:
        case 974:
        case 32593:
        case 32594:
        case 32734:
        case 38590:
        case 49283:
        case 49284:
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
            sp->custom_NameHash = SPELL_HASH_EARTH_SHIELD; break;
        }

        case 403:
        case 529:
        case 548:
        case 915:
        case 943:
        case 6041:
        case 8246:
        case 9532:
        case 10391:
        case 10392:
        case 12167:
        case 13482:
        case 13527:
        case 14109:
        case 14119:
        case 15207:
        case 15208:
        case 15234:
        case 15801:
        case 16782:
        case 18081:
        case 18089:
        case 19874:
        case 20295:
        case 20802:
        case 20805:
        case 20824:
        case 22414:
        case 23592:
        case 25448:
        case 25449:
        case 26098:
        case 31764:
        case 34345:
        case 35010:
        case 36152:
        case 37273:
        case 37661:
        case 37664:
        case 38465:
        case 39065:
        case 41184:
        case 42024:
        case 43526:
        case 43903:
        case 45075:
        case 45284:
        case 45286:
        case 45287:
        case 45288:
        case 45289:
        case 45290:
        case 45291:
        case 45292:
        case 45293:
        case 45294:
        case 45295:
        case 45296:
        case 48698:
        case 48895:
        case 49237:
        case 49238:
        case 49239:
        case 49240:
        case 49418:
        case 49454:
        case 51587:
        case 51618:
        case 53044:
        case 53314:
        case 54843:
        case 55044:
        case 56326:
        case 56891:
        case 57780:
        case 57781:
        case 59006:
        case 59024:
        case 59081:
        case 59169:
        case 59199:
        case 59683:
        case 59863:
        case 60009:
        case 60032:
        case 61374:
        case 61893:
        case 63809:
        case 64098:
        case 64696:
        case 65987:
        case 68112:
        case 68113:
        case 68114:
        case 69567:
        case 69970:
        case 71136:
        case 71934:
        {
            sp->custom_NameHash = SPELL_HASH_LIGHTNING_BOLT; break;
        }

        case 408:
        case 8643:
        case 27615:
        case 30621:
        case 30832:
        case 32864:
        case 41389:
        case 49616:
        case 72335:
        {
            sp->custom_NameHash = SPELL_HASH_KIDNEY_SHOT; break;
        }

        case 421:
        case 930:
        case 2860:
        case 10605:
        case 12058:
        case 15117:
        case 15305:
        case 15659:
        case 16006:
        case 16033:
        case 16921:
        case 20831:
        case 21179:
        case 22355:
        case 23106:
        case 23206:
        case 24680:
        case 25021:
        case 25439:
        case 25442:
        case 27567:
        case 28167:
        case 28293:
        case 28900:
        case 31330:
        case 31717:
        case 32337:
        case 33643:
        case 37448:
        case 39066:
        case 39945:
        case 40536:
        case 41183:
        case 42441:
        case 42804:
        case 43435:
        case 44318:
        case 45297:
        case 45298:
        case 45299:
        case 45300:
        case 45301:
        case 45302:
        case 45868:
        case 46380:
        case 48140:
        case 48699:
        case 49268:
        case 49269:
        case 49270:
        case 49271:
        case 50830:
        case 52383:
        case 54334:
        case 54531:
        case 59082:
        case 59220:
        case 59223:
        case 59273:
        case 59517:
        case 59716:
        case 59844:
        case 61528:
        case 61879:
        case 62131:
        case 63479:
        case 64213:
        case 64215:
        case 64390:
        case 64758:
        case 64759:
        case 67529:
        case 68319:
        case 69696:
        case 75362:
        {
            sp->custom_NameHash = SPELL_HASH_CHAIN_LIGHTNING; break;
        }

        case 465:
        case 643:
        case 1032:
        case 8258:
        case 10290:
        case 10291:
        case 10292:
        case 10293:
        case 17232:
        case 27149:
        case 41452:
        case 48941:
        case 48942:
        case 52442:
        case 57740:
        case 58944:
        {
            sp->custom_NameHash = SPELL_HASH_DEVOTION_AURA; break;
        }

        case 498:
        case 13007:
        case 27778:
        case 27779:
        {
            sp->custom_NameHash = SPELL_HASH_DIVINE_PROTECTION; break;
        }

        case 585:
        case 591:
        case 598:
        case 984:
        case 1004:
        case 6060:
        case 10933:
        case 10934:
        case 25363:
        case 25364:
        case 35155:
        case 48122:
        case 48123:
        case 61923:
        case 69967:
        case 71146:
        case 71546:
        case 71547:
        case 71778:
        case 71779:
        case 71841:
        case 71842:
        {
            sp->custom_NameHash = SPELL_HASH_SMITE; break;
        }

        case 605:
        case 11446:
        case 15690:
        case 36797:
        case 36798:
        case 43550:
        case 43871:
        case 43875:
        case 45112:
        case 67229:
        {
            sp->custom_NameHash = SPELL_HASH_MIND_CONTROL; break;
        }

        case 633:
        case 2800:
        case 9257:
        case 10310:
        case 17233:
        case 20233:
        case 20236:
        case 27154:
        case 48788:
        case 53778:
        {
            sp->custom_NameHash = SPELL_HASH_LAY_ON_HANDS; break;
        }

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
            sp->custom_NameHash = SPELL_HASH_DIVINE_SHIELD; break;
        }

        case 676:
        case 1843:
        case 6713:
        case 8379:
        case 11879:
        case 13534:
        case 15752:
        case 22691:
        case 27581:
        case 30013:
        case 31955:
        case 36139:
        case 41062:
        case 48883:
        case 65935:
        {
            sp->custom_NameHash = SPELL_HASH_DISARM; break;
        }

        case 686:
        case 695:
        case 705:
        case 1088:
        case 1106:
        case 7617:
        case 7619:
        case 7641:
        case 9613:
        case 11659:
        case 11660:
        case 11661:
        case 12471:
        case 12739:
        case 13440:
        case 13480:
        case 14106:
        case 14122:
        case 15232:
        case 15472:
        case 15537:
        case 16408:
        case 16409:
        case 16410:
        case 16783:
        case 16784:
        case 17393:
        case 17434:
        case 17435:
        case 17483:
        case 17509:
        case 18111:
        case 18138:
        case 18164:
        case 18205:
        case 18211:
        case 18214:
        case 18217:
        case 19728:
        case 19729:
        case 20298:
        case 20791:
        case 20807:
        case 20816:
        case 20825:
        case 21077:
        case 21141:
        case 22336:
        case 22677:
        case 24668:
        case 25307:
        case 26006:
        case 27209:
        case 29317:
        case 29487:
        case 29626:
        case 29640:
        case 29927:
        case 30055:
        case 30505:
        case 30686:
        case 31618:
        case 31627:
        case 32666:
        case 32860:
        case 33335:
        case 34344:
        case 36714:
        case 36868:
        case 36972:
        case 36986:
        case 36987:
        case 38378:
        case 38386:
        case 38628:
        case 38825:
        case 38892:
        case 39025:
        case 39026:
        case 39297:
        case 39309:
        case 40185:
        case 41069:
        case 41280:
        case 41957:
        case 43330:
        case 43649:
        case 43667:
        case 45055:
        case 45679:
        case 45680:
        case 47076:
        case 47248:
        case 47808:
        case 47809:
        case 49084:
        case 50455:
        case 51363:
        case 51432:
        case 51608:
        case 52257:
        case 52534:
        case 53086:
        case 53333:
        case 54113:
        case 55984:
        case 56405:
        case 57374:
        case 57464:
        case 57644:
        case 57725:
        case 58827:
        case 59016:
        case 59246:
        case 59254:
        case 59351:
        case 59357:
        case 59389:
        case 59575:
        case 60015:
        case 61558:
        case 61562:
        case 65821:
        case 68151:
        case 68152:
        case 68153:
        case 69028:
        case 69068:
        case 69211:
        case 69212:
        case 69387:
        case 69577:
        case 69972:
        case 70043:
        case 70080:
        case 70182:
        case 70208:
        case 70270:
        case 70386:
        case 70387:
        case 71143:
        case 71254:
        case 71296:
        case 71297:
        case 71936:
        case 72008:
        case 72503:
        case 72504:
        case 72901:
        case 72960:
        case 72961:
        case 75330:
        case 75331:
        case 75384:
        {
            sp->custom_NameHash = SPELL_HASH_SHADOW_BOLT; break;
        }

        case 689:
        case 699:
        case 709:
        case 7651:
        case 11699:
        case 11700:
        case 12693:
        case 16375:
        case 16414:
        case 16608:
        case 17173:
        case 17238:
        case 17620:
        case 18084:
        case 18557:
        case 18815:
        case 18817:
        case 20743:
        case 21170:
        case 24300:
        case 24435:
        case 24585:
        case 24618:
        case 26693:
        case 27219:
        case 27220:
        case 27994:
        case 29155:
        case 30412:
        case 34107:
        case 34696:
        case 35748:
        case 36224:
        case 36655:
        case 36825:
        case 37992:
        case 38817:
        case 39676:
        case 43417:
        case 44294:
        case 46155:
        case 46291:
        case 46466:
        case 47857:
        case 55646:
        case 64159:
        case 64160:
        case 69066:
        case 70213:
        case 71838:
        case 71839:
        {
            sp->custom_NameHash = SPELL_HASH_DRAIN_LIFE; break;
        }

        case 700:
        case 1090:
        case 8399:
        case 9159:
        case 9160:
        case 12098:
        case 15970:
        case 20663:
        case 20669:
        case 20989:
        case 24004:
        case 24664:
        case 24778:
        case 31292:
        case 31298:
        case 31541:
        case 31548:
        case 34801:
        case 36402:
        case 41396:
        case 52721:
        case 52742:
        case 53045:
        case 58849:
        case 59165:
        case 66290:
        {
            sp->custom_NameHash = SPELL_HASH_SLEEP; break;
        }

        case 703:
        case 8631:
        case 8632:
        case 8633:
        case 8818:
        case 11289:
        case 11290:
        case 26839:
        case 26884:
        case 37066:
        case 48675:
        case 48676:
        {
            sp->custom_NameHash = SPELL_HASH_GARROTE; break;
        }

        case 710:
        case 8994:
        case 18647:
        case 24466:
        case 27565:
        case 30231:
        case 35182:
        case 37527:
        case 37546:
        case 37833:
        case 38009:
        case 38376:
        case 38791:
        case 39622:
        case 39674:
        case 40370:
        case 44765:
        case 44836:
        case 71298:
        {
            sp->custom_NameHash = SPELL_HASH_BANISH; break;
        }

        case 755:
        case 3698:
        case 3699:
        case 3700:
        case 11693:
        case 11694:
        case 11695:
        case 16569:
        case 27259:
        case 40671:
        case 46467:
        case 47856:
        case 60829:
        {
            sp->custom_NameHash = SPELL_HASH_HEALTH_FUNNEL; break;
        }

        case 770:
        case 6950:
        case 13424:
        case 13752:
        case 16498:
        case 20656:
        case 21670:
        case 25602:
        case 32129:
        case 65863:
        {
            sp->custom_NameHash = SPELL_HASH_FAERIE_FIRE; break;
        }

        case 772:
        case 6546:
        case 6547:
        case 6548:
        case 11572:
        case 11573:
        case 11574:
        case 11977:
        case 12054:
        case 13318:
        case 13443:
        case 13445:
        case 13738:
        case 14087:
        case 14118:
        case 16393:
        case 16403:
        case 16406:
        case 16509:
        case 17153:
        case 17504:
        case 18075:
        case 18078:
        case 18106:
        case 18200:
        case 18202:
        case 21949:
        case 25208:
        case 29574:
        case 29578:
        case 36965:
        case 36991:
        case 37662:
        case 43246:
        case 43931:
        case 46845:
        case 47465:
        case 48880:
        case 53317:
        case 54703:
        case 54708:
        case 59239:
        case 59343:
        case 59691:
        {
            sp->custom_NameHash = SPELL_HASH_REND; break;
        }

        case 774:
        case 1058:
        case 1430:
        case 2090:
        case 2091:
        case 3627:
        case 8070:
        case 8910:
        case 9839:
        case 9840:
        case 9841:
        case 12160:
        case 15981:
        case 20664:
        case 20701:
        case 25299:
        case 26981:
        case 26982:
        case 27532:
        case 28716:
        case 28722:
        case 28723:
        case 28724:
        case 31782:
        case 32131:
        case 38657:
        case 42544:
        case 48440:
        case 48441:
        case 53607:
        case 64801:
        case 66065:
        case 67971:
        case 67972:
        case 67973:
        case 69898:
        case 70691:
        case 71142:
        {
            sp->custom_NameHash = SPELL_HASH_REJUVENATION; break;
        }

        case 851:
        case 61816:
        case 61839:
        {
            sp->custom_NameHash = SPELL_HASH_POLYMORPH__SHEEP; break;
        }

        case 853:
        case 5588:
        case 5589:
        case 10308:
        case 13005:
        case 32416:
        case 37369:
        case 39077:
        case 41468:
        case 66007:
        case 66613:
        case 66863:
        case 66940:
        case 66941:
        {
            sp->custom_NameHash = SPELL_HASH_HAMMER_OF_JUSTICE; break;
        }

        case 879:
        case 5614:
        case 5615:
        case 10312:
        case 10313:
        case 10314:
        case 17147:
        case 17149:
        case 27138:
        case 33632:
        case 48800:
        case 48801:
        case 52445:
        case 58822:
        {
            sp->custom_NameHash = SPELL_HASH_EXORCISM; break;
        }

        case 980:
        case 1014:
        case 6217:
        case 11711:
        case 11712:
        case 11713:
        case 14868:
        case 14875:
        case 17771:
        case 18266:
        case 18671:
        case 27218:
        case 29930:
        case 32418:
        case 37334:
        case 39672:
        case 46190:
        case 47863:
        case 47864:
        case 65814:
        case 68136:
        case 68137:
        case 68138:
        case 69404:
        case 70391:
        case 71112:
        {
            sp->custom_NameHash = SPELL_HASH_CURSE_OF_AGONY; break;
        }

        case 1064:
        case 10622:
        case 10623:
        case 14900:
        case 15799:
        case 16367:
        case 25422:
        case 25423:
        case 33642:
        case 41114:
        case 42027:
        case 42477:
        case 43527:
        case 48894:
        case 54481:
        case 55458:
        case 55459:
        case 59473:
        case 69923:
        case 70425:
        case 71120:
        case 75370:
        {
            sp->custom_NameHash = SPELL_HASH_CHAIN_HEAL; break;
        }

        case 1079:
        case 9492:
        case 9493:
        case 9752:
        case 9894:
        case 9896:
        case 27008:
        case 33912:
        case 36590:
        case 49799:
        case 49800:
        case 57661:
        case 59989:
        case 71926:
        {
            sp->custom_NameHash = SPELL_HASH_RIP; break;
        }

        case 1082:
        case 2975:
        case 2976:
        case 2977:
        case 2980:
        case 2981:
        case 2982:
        case 3009:
        case 3010:
        case 3029:
        case 3666:
        case 3667:
        case 5201:
        case 9849:
        case 9850:
        case 16827:
        case 16828:
        case 16829:
        case 16830:
        case 16831:
        case 16832:
        case 24187:
        case 27000:
        case 27049:
        case 27347:
        case 31289:
        case 47468:
        case 48569:
        case 48570:
        case 51772:
        case 52471:
        case 52472:
        case 62225:
        case 67774:
        case 67793:
        case 67879:
        case 67980:
        case 67981:
        case 67982:
        case 75159:
        {
            sp->custom_NameHash = SPELL_HASH_CLAW; break;
        }

        case 1098:
        case 11725:
        case 11726:
        case 20882:
        case 61191:
        {
            sp->custom_NameHash = SPELL_HASH_ENSLAVE_DEMON; break;
        }

        case 34453:
        case 34454:
        case 68361:
        {
            sp->custom_NameHash = SPELL_HASH_ANIMAL_HANDLER; break;
        }
        case 44425:
        case 44780:
        case 44781:
        case 50273:
        case 50804:
        case 56397:
        case 58456:
        case 59248:
        case 59381:
        case 63934:
        case 64599:
        case 64607:
        case 65799:
        case 67994:
        case 67995:
        case 67996:
        {
            sp->custom_NameHash = SPELL_HASH_ARCANE_BARRAGE; break;
        }
        case 24544:
        case 31571:
        case 31572:
        case 33421:
        case 33713:
        case 57529:
        case 57531:
        {
            sp->custom_NameHash = SPELL_HASH_ARCANE_POTENCY; break;
        }
        case 3044:
        case 14281:
        case 14282:
        case 14283:
        case 14284:
        case 14285:
        case 14286:
        case 14287:
        case 27019:
        case 34829:
        case 35401:
        case 36293:
        case 36609:
        case 36623:
        case 38807:
        case 49044:
        case 49045:
        case 51742:
        case 55624:
        case 58973:
        case 69989:
        case 71116:
        {
            sp->custom_NameHash = SPELL_HASH_ARCANE_SHOT; break;
        }
        case 31850:
        case 31851:
        case 31852:
        case 66233:
        case 66235:
        {
            sp->custom_NameHash = SPELL_HASH_ARDENT_DEFENDER; break;
        }
        case 20655:
        case 22812:
        case 63408:
        case 63409:
        case 65860:
        {
            sp->custom_NameHash = SPELL_HASH_BARKSKIN; break;
        }
        case 18499:
        {
            sp->custom_NameHash = SPELL_HASH_BERSERKER_RAGE; break;
        }
        case 19590:
        case 19592:
        {
            sp->custom_NameHash = SPELL_HASH_BESTIAL_DISCIPLINE; break;
        }
        case 40475:
        {
            sp->custom_NameHash = SPELL_HASH_BLACK_TEMPLE_MELEE_TRINKET; break;
        }
        case 13877:
        case 22482:
        case 33735:
        case 44181:
        case 51211:
        case 65956:
        {
            sp->custom_NameHash = SPELL_HASH_BLADE_FLURRY; break;
        }
        case 9632:
        case 35131:
        case 46924:
        case 63784:
        case 63785:
        case 65946:
        case 65947:
        case 67541:
        case 69652:
        case 69653:
        {
            sp->custom_NameHash = SPELL_HASH_BLADESTORM; break;
        }
        case 41450:
        {
            sp->custom_NameHash = SPELL_HASH_BLESSING_OF_PROTECTION; break;
        }
        case 2094:
        case 21060:
        case 34654:
        case 34694:
        case 42972:
        case 43433:
        case 65960:
        {
            sp->custom_NameHash = SPELL_HASH_BLIND; break;
        }
        case 16952:
        case 16954:
        case 29836:
        case 29859:
        case 30069:
        case 30070:
        {
            sp->custom_NameHash = SPELL_HASH_BLOOD_FRENZY; break;
        }
        case 2825:
        case 6742:
        case 16170:
        case 21049:
        case 23951:
        case 24185:
        case 27689:
        case 28902:
        case 33555:
        case 37067:
        case 37309:
        case 37310:
        case 37472:
        case 37599:
        case 41185:
        case 43578:
        case 45584:
        case 50730:
        case 54516:
        case 65980:
        {
            sp->custom_NameHash = SPELL_HASH_BLOODLUST; break;
        }
        case 23880:
        case 23881:
        case 23885:
        case 23892:
        case 23893:
        case 23894:
        case 25251:
        case 30335:
        case 30474:
        case 30475:
        case 30476:
        case 31996:
        case 31997:
        case 31998:
        case 33964:
        case 35123:
        case 35125:
        case 35947:
        case 35948:
        case 35949:
        case 39070:
        case 39071:
        case 39072:
        case 40423:
        case 55968:
        case 55969:
        case 55970:
        case 57790:
        case 57791:
        case 57792:
        case 60017:
        case 71938:
        {
            sp->custom_NameHash = SPELL_HASH_BLOODTHIRST; break;
        }
        case 34462:
        case 34464:
        case 34465:
        {
            sp->custom_NameHash = SPELL_HASH_CATLIKE_REFLEXES; break;
        }
        case 50796:
        case 51287:
        case 59170:
        case 59171:
        case 59172:
        case 69576:
        case 71108:
        {
            sp->custom_NameHash = SPELL_HASH_CHAOS_BOLT; break;
        }
        case 7922:
        case 65929:
        {
            sp->custom_NameHash = SPELL_HASH_CHARGE_STUN; break;
        }
        case 1833:
        case 6409:
        case 14902:
        case 30986:
        case 31819:
        case 31843:
        case 34243:
        {
            sp->custom_NameHash = SPELL_HASH_CHEAP_SHOT; break;
        }
        case 4987:
        case 28787:
        case 28788:
        case 29380:
        case 32400:
        case 39078:
        case 57767:
        case 66116:
        case 68621:
        case 68622:
        case 68623:
        {
            sp->custom_NameHash = SPELL_HASH_CLEANSE; break;
        }
        case 12536:
        case 16246:
        case 16870:
        case 67210:
        {
            sp->custom_NameHash = SPELL_HASH_CLEARCASTING; break;
        }
        case 11129:
        case 28682:
        case 29977:
        case 74630:
        case 75882:
        case 75883:
        case 75884:
        {
            sp->custom_NameHash = SPELL_HASH_COMBUSTION; break;
        }
        case 19746:
        {
            sp->custom_NameHash = SPELL_HASH_CONCENTRATION_AURA; break;
        }
        case 12809:
        case 22427:
        case 32588:
        case 52719:
        case 54132:
        {
            sp->custom_NameHash = SPELL_HASH_CONCUSSION_BLOW; break;
        }
        case 17962:
        {
            sp->custom_NameHash = SPELL_HASH_CONFLAGRATE; break;
        }
        case 32223:
        {
            sp->custom_NameHash = SPELL_HASH_CRUSADER_AURA; break;
        }
        case 14517:
        case 14518:
        case 17281:
        case 35395:
        case 35509:
        case 36647:
        case 50773:
        case 65166:
        case 66003:
        case 71549:
        {
            sp->custom_NameHash = SPELL_HASH_CRUSADER_STRIKE; break;
        }
        case 1714:
        case 11719:
        case 12889:
        case 13338:
        case 15470:
        case 25195:
        {
            sp->custom_NameHash = SPELL_HASH_CURSE_OF_TONGUES; break;
        }
        case 29538:
        case 32334:
        case 33786:
        case 38516:
        case 38517:
        case 39594:
        case 40578:
        case 43120:
        case 43121:
        case 43528:
        case 60236:
        case 61662:
        case 62632:
        case 62633:
        case 65859:
        case 69699:
        {
            sp->custom_NameHash = SPELL_HASH_CYCLONE; break;
        }
        case 1604:
        case 5101:
        case 13496:
        case 15571:
        case 29703:
        case 35955:
        case 50259:
        case 50411:
        case 51372:
        {
            sp->custom_NameHash = SPELL_HASH_DAZED; break;
        }
        case 26679:
        case 37074:
        case 48673:
        case 48674:
        case 52885:
        case 59180:
        case 64499:
        {
            sp->custom_NameHash = SPELL_HASH_DEADLY_THROW; break;
        }
        case 17471:
        case 17698:
        case 48743:
        case 51956:
        {
            sp->custom_NameHash = SPELL_HASH_DEATH_PACT; break;
        }
        case 12292:
        {
            sp->custom_NameHash = SPELL_HASH_DEATH_WISH; break;
        }
        case 18788:
        {
            sp->custom_NameHash = SPELL_HASH_DEMONIC_SACRIFICE; break;
        }
        case 20243:
        case 30016:
        case 30017:
        case 30022:
        case 36891:
        case 36894:
        case 38849:
        case 38967:
        case 44452:
        case 47497:
        case 47498:
        case 57795:
        case 60018:
        case 62317:
        case 69902:
        {
            sp->custom_NameHash = SPELL_HASH_DEVASTATE; break;
        }
        case 47218:
        case 47585:
        case 49766:
        case 49768:
        case 60069:
        case 63230:
        case 65544:
        {
            sp->custom_NameHash = SPELL_HASH_DISPERSION; break;
        }
        case 53385:
        case 54171:
        case 54172:
        case 58127:
        case 66006:
        {
            sp->custom_NameHash = SPELL_HASH_DIVINE_STORM; break;
        }
        case 1120:
        case 8288:
        case 8289:
        case 11675:
        case 18371:
        case 27217:
        case 32862:
        case 35839:
        case 47855:
        case 60452:
        {
            sp->custom_NameHash = SPELL_HASH_DRAIN_SOUL; break;
        }
        case 8042:
        case 8044:
        case 8045:
        case 8046:
        case 10412:
        case 10413:
        case 10414:
        case 13281:
        case 13728:
        case 15501:
        case 22885:
        case 23114:
        case 24685:
        case 25025:
        case 25454:
        case 26194:
        case 43305:
        case 47071:
        case 49230:
        case 49231:
        case 54511:
        case 56506:
        case 57783:
        case 60011:
        case 61668:
        case 65973:
        case 68100:
        case 68101:
        case 68102:
        {
            sp->custom_NameHash = SPELL_HASH_EARTH_SHOCK; break;
        }
        case 19583:
        case 19584:
        case 19585:
        case 19586:
        case 19587:
        {
            sp->custom_NameHash = SPELL_HASH_ENDURANCE_TRAINING; break;
        }
        case 19184:
        case 19185:
        case 19387:
        case 19388:
        case 64803:
        case 64804:
        {
            sp->custom_NameHash = SPELL_HASH_ENTRAPMENT; break;
        }
        case 32645:
        case 32684:
        case 39967:
        case 41487:
        case 41509:
        case 41510:
        case 57992:
        case 57993:
        {
            sp->custom_NameHash = SPELL_HASH_ENVENOM; break;
        }
        case 59752:
        {
            sp->custom_NameHash = SPELL_HASH_EVERY_MAN_FOR_HIMSELF; break;
        }
        case 2098:
        case 6760:
        case 6761:
        case 6762:
        case 8623:
        case 8624:
        case 11299:
        case 11300:
        case 15691:
        case 15692:
        case 26865:
        case 27611:
        case 31016:
        case 41177:
        case 46189:
        case 48667:
        case 48668:
        case 57641:
        case 60008:
        case 65957:
        case 67709:
        case 68094:
        case 68095:
        case 68096:
        case 68317:
        case 71933:
        {
            sp->custom_NameHash = SPELL_HASH_EVISCERATE; break;
        }
        case 16857:
        case 60089:
        {
            sp->custom_NameHash = SPELL_HASH_FAERIE_FIRE__FERAL_; break;
        }
        case 51723:
        case 52874:
        case 61739:
        case 61740:
        case 61741:
        case 61742:
        case 61743:
        case 61744:
        case 61745:
        case 61746:
        case 63753:
        case 65955:
        case 67706:
        case 68097:
        case 68098:
        case 68099:
        case 69921:
        case 71128:
        {
            sp->custom_NameHash = SPELL_HASH_FAN_OF_KNIVES; break;
        }
        case 5782:
        case 6213:
        case 6215:
        case 12096:
        case 12542:
        case 22678:
        case 26070:
        case 26580:
        case 26661:
        case 27641:
        case 27990:
        case 29168:
        case 29321:
        case 30002:
        case 30530:
        case 30584:
        case 30615:
        case 31358:
        case 31970:
        case 32241:
        case 33547:
        case 33924:
        case 34259:
        case 38154:
        case 38595:
        case 38660:
        case 39119:
        case 39176:
        case 39210:
        case 39415:
        case 41150:
        case 46561:
        case 51240:
        case 59669:
        case 65809:
        case 68950:
        {
            sp->custom_NameHash = SPELL_HASH_FEAR; break;
        }
        case 6346:
        {
            sp->custom_NameHash = SPELL_HASH_FEAR_WARD; break;
        }
        case 17002:
        case 24866:
        {
            sp->custom_NameHash = SPELL_HASH_FERAL_SWIFTNESS; break;
        }
        case 22568:
        case 22827:
        case 22828:
        case 22829:
        case 24248:
        case 27557:
        case 31018:
        case 48576:
        case 48577:
        {
            sp->custom_NameHash = SPELL_HASH_FEROCIOUS_BITE; break;
        }
        case 4154:
        case 16934:
        case 16935:
        case 16936:
        case 16937:
        case 16938:
        case 19598:
        case 19599:
        case 19600:
        case 19601:
        case 19602:
        case 33667:
        {
            sp->custom_NameHash = SPELL_HASH_FEROCITY; break;
        }
        case 19891:
        case 19899:
        case 19900:
        case 27153:
        case 48947:
        {
            sp->custom_NameHash = SPELL_HASH_FIRE_RESISTANCE_AURA; break;
        }
        case 7712:
        case 7714:
        case 7715:
        case 7716:
        case 7717:
        case 7718:
        case 7719:
        {
            sp->custom_NameHash = SPELL_HASH_FIRE_STRIKE; break;
        }
        case 8050:
        case 8052:
        case 8053:
        case 10447:
        case 10448:
        case 13729:
        case 15039:
        case 15096:
        case 15616:
        case 16804:
        case 22423:
        case 23038:
        case 25457:
        case 29228:
        case 32967:
        case 34354:
        case 39529:
        case 39590:
        case 41115:
        case 43303:
        case 49232:
        case 49233:
        case 51588:
        case 55613:
        case 58940:
        case 58971:
        case 59684:
        {
            sp->custom_NameHash = SPELL_HASH_FLAME_SHOCK; break;
        }
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
            sp->custom_NameHash = SPELL_HASH_FLASH_OF_LIGHT; break;
        }
        case 3355:
        case 14308:
        case 14309:
        case 31932:
        case 55041:
        {
            sp->custom_NameHash = SPELL_HASH_FREEZING_TRAP_EFFECT; break;
        }
        case 19888:
        case 19897:
        case 19898:
        case 27152:
        case 48945:
        {
            sp->custom_NameHash = SPELL_HASH_FROST_RESISTANCE_AURA; break;
        }
        case 8056:
        case 8058:
        case 10472:
        case 10473:
        case 12548:
        case 15089:
        case 15499:
        case 19133:
        case 21030:
        case 21401:
        case 22582:
        case 23115:
        case 25464:
        case 29666:
        case 34353:
        case 37332:
        case 37865:
        case 38234:
        case 39062:
        case 41116:
        case 43524:
        case 46180:
        case 49235:
        case 49236:
        {
            sp->custom_NameHash = SPELL_HASH_FROST_SHOCK; break;
        }
        case 11071:
        case 12494:
        case 12496:
        case 12497:
        case 57455:
        case 57456:
        case 61572:
        case 72004:
        case 72098:
        case 72120:
        case 72121:
        {
            sp->custom_NameHash = SPELL_HASH_FROSTBITE; break;
        }
        case 44614:
        case 47610:
        case 51779:
        case 69869:
        case 69984:
        case 70616:
        case 71130:
        {
            sp->custom_NameHash = SPELL_HASH_FROSTFIRE_BOLT; break;
        }
        case 1330:
        {
            sp->custom_NameHash = SPELL_HASH_GARROTE___SILENCE; break;
        }
        case 4102:
        case 32019:
        case 35290:
        case 35291:
        case 35292:
        case 35293:
        case 35294:
        case 35295:
        case 35299:
        case 35300:
        case 35302:
        case 35303:
        case 35304:
        case 35305:
        case 35306:
        case 35307:
        case 35308:
        case 48130:
        case 51751:
        case 59264:
        {
            sp->custom_NameHash = SPELL_HASH_GORE; break;
        }
        case 1776:
        case 1777:
        case 8629:
        case 11285:
        case 11286:
        case 12540:
        case 13579:
        case 24698:
        case 28456:
        case 29425:
        case 34940:
        case 36862:
        case 38764:
        case 38863:
        {
            sp->custom_NameHash = SPELL_HASH_GOUGE; break;
        }
        case 2060:
        case 10963:
        case 10964:
        case 10965:
        case 22009:
        case 25210:
        case 25213:
        case 25314:
        case 28809:
        case 29564:
        case 34119:
        case 35096:
        case 38580:
        case 41378:
        case 48062:
        case 48063:
        case 49348:
        case 57775:
        case 60003:
        case 61965:
        case 62334:
        case 62442:
        case 63760:
        case 69963:
        case 71131:
        case 71931:
        {
            sp->custom_NameHash = SPELL_HASH_GREATER_HEAL; break;
        }
        case 53595:
        case 54423:
        case 66867:
        case 66903:
        case 66904:
        case 66905:
        case 67680:
        {
            sp->custom_NameHash = SPELL_HASH_HAMMER_OF_THE_RIGHTEOUS; break;
        }
        case 24239:
        case 24274:
        case 24275:
        case 27180:
        case 32772:
        case 37251:
        case 37255:
        case 37259:
        case 48805:
        case 48806:
        case 51384:
        {
            sp->custom_NameHash = SPELL_HASH_HAMMER_OF_WRATH; break;
        }
        case 1715:
        case 7372:
        case 7373:
        case 9080:
        case 25212:
        case 26141:
        case 26211:
        case 27584:
        case 29667:
        case 30989:
        case 31553:
        case 38262:
        case 38995:
        case 48639:
        case 62845:
        {
            sp->custom_NameHash = SPELL_HASH_HAMSTRING; break;
        }
        case 48181:
        case 48184:
        case 48210:
        case 50091:
        case 59161:
        case 59163:
        case 59164:
        {
            sp->custom_NameHash = SPELL_HASH_HAUNT; break;
        }
        case 17003:
        case 17004:
        case 17005:
        case 17006:
        case 24894:
        {
            sp->custom_NameHash = SPELL_HASH_HEART_OF_THE_WILD; break;
        }
        case 5857:
        case 11681:
        case 11682:
        case 27214:
        case 30860:
        case 47822:
        case 65817:
        case 68142:
        case 68143:
        case 68144:
        case 69585:
        case 70284:
        {
            sp->custom_NameHash = SPELL_HASH_HELLFIRE_EFFECT; break;
        }
        case 16511:
        case 17347:
        case 17348:
        case 26864:
        case 30478:
        case 37331:
        case 45897:
        case 48660:
        case 65954:
        {
            sp->custom_NameHash = SPELL_HASH_HEMORRHAGE; break;
        }
        case 57755:
        {
            sp->custom_NameHash = SPELL_HASH_HEROIC_THROW; break;
        }
        case 23682:
        case 23689:
        case 32182:
        case 32927:
        case 32955:
        case 37471:
        case 39200:
        case 65983:
        {
            sp->custom_NameHash = SPELL_HASH_HEROISM; break;
        }
        case 11641:
        case 16097:
        case 16707:
        case 16708:
        case 16709:
        case 17172:
        case 18503:
        case 22566:
        case 24053:
        case 29044:
        case 36700:
        case 40400:
        case 46295:
        case 51514:
        case 53439:
        case 66054:
        {
            sp->custom_NameHash = SPELL_HASH_HEX; break;
        }
        case 2637:
        case 18657:
        case 18658:
        {
            sp->custom_NameHash = SPELL_HASH_HIBERNATE; break;
        }
        case 20473:
        case 20929:
        case 20930:
        case 25902:
        case 25903:
        case 25911:
        case 25912:
        case 25913:
        case 25914:
        case 27174:
        case 27175:
        case 27176:
        case 32771:
        case 33072:
        case 33073:
        case 33074:
        case 35160:
        case 36340:
        case 38921:
        case 48820:
        case 48821:
        case 48822:
        case 48823:
        case 48824:
        case 48825:
        case 66114:
        case 68014:
        case 68015:
        case 68016:
        {
            sp->custom_NameHash = SPELL_HASH_HOLY_SHOCK; break;
        }
        case 5484:
        case 17928:
        case 39048:
        case 50577:
        {
            sp->custom_NameHash = SPELL_HASH_HOWL_OF_TERROR; break;
        }
        case 27619:
        case 36911:
        case 41590:
        case 45438:
        case 45776:
        case 46604:
        case 46882:
        case 56124:
        case 56644:
        case 62766:
        case 65802:
        case 69924:
        {
            sp->custom_NameHash = SPELL_HASH_ICE_BLOCK; break;
        }
        case 30455:
        case 31766:
        case 42913:
        case 42914:
        case 43427:
        case 43571:
        case 44176:
        case 45906:
        case 46194:
        case 49906:
        case 54261:
        {
            sp->custom_NameHash = SPELL_HASH_ICE_LANCE; break;
        }
        case 3261:
        case 11119:
        case 11120:
        case 12654:
        case 12846:
        case 12847:
        case 12848:
        case 52210:
        case 58438:
        {
            sp->custom_NameHash = SPELL_HASH_IGNITE; break;
        }
        case 11103:
        case 12355:
        case 12357:
        case 12358:
        case 64343:
        {
            sp->custom_NameHash = SPELL_HASH_IMPACT; break;
        }
        case 62905:
        case 62908:
        {
            sp->custom_NameHash = SPELL_HASH_IMPROVED_DEATH_STRIKE; break;
        }
        case 12289:
        case 12668:
        case 23694:
        case 23695:
        case 24428:
        {
            sp->custom_NameHash = SPELL_HASH_IMPROVED_HAMSTRING; break;
        }
        case 59088:
        case 59089:
        {
            sp->custom_NameHash = SPELL_HASH_IMPROVED_SPELL_REFLECTION; break;
        }
        case 35963:
        case 47168:
        {
            sp->custom_NameHash = SPELL_HASH_IMPROVED_WING_CLIP; break;
        }
        case 19397:
        case 23308:
        case 23309:
        case 29722:
        case 32231:
        case 32707:
        case 36832:
        case 38401:
        case 38918:
        case 39083:
        case 40239:
        case 41960:
        case 43971:
        case 44519:
        case 46043:
        case 47837:
        case 47838:
        case 53493:
        case 69973:
        case 71135:
        {
            sp->custom_NameHash = SPELL_HASH_INCINERATE; break;
        }
        case 20252:
        case 20253:
        case 20614:
        case 20615:
        case 20616:
        case 20617:
        case 25272:
        case 25273:
        case 25274:
        case 25275:
        case 27577:
        case 27826:
        case 30151:
        case 30153:
        case 30154:
        case 30194:
        case 30195:
        case 30197:
        case 30198:
        case 30199:
        case 30200:
        case 47995:
        case 47996:
        case 50823:
        case 58743:
        case 58747:
        case 58769:
        case 61490:
        case 61491:
        case 67540:
        case 67573:
        {
            sp->custom_NameHash = SPELL_HASH_INTERCEPT; break;
        }
        case 3411:
        case 34784:
        case 41198:
        case 53476:
        case 59667:
        {
            sp->custom_NameHash = SPELL_HASH_INTERVENE; break;
        }
        case 7093:
        case 19577:
        case 24394:
        case 70495:
        {
            sp->custom_NameHash = SPELL_HASH_INTIMIDATION; break;
        }
        case 31898:
        case 32220:
        case 41461:
        {
            sp->custom_NameHash = SPELL_HASH_JUDGEMENT_OF_BLOOD; break;
        }
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
            sp->custom_NameHash = SPELL_HASH_JUDGEMENT_OF_COMMAND; break;
        }
        case 48492:
        case 48494:
        case 48495:
        case 51178:
        case 51185:
        {
            sp->custom_NameHash = SPELL_HASH_KING_OF_THE_JUNGLE; break;
        }
        case 21158:
        case 51505:
        case 53788:
        case 55659:
        case 55704:
        case 56491:
        case 58972:
        case 59182:
        case 59519:
        case 60043:
        case 61924:
        case 64870:
        case 64991:
        case 66813:
        case 67330:
        case 71824:
        {
            sp->custom_NameHash = SPELL_HASH_LAVA_BURST; break;
        }
        case 8004:
        case 8008:
        case 8010:
        case 10466:
        case 10467:
        case 10468:
        case 25420:
        case 27624:
        case 28849:
        case 28850:
        case 44256:
        case 46181:
        case 49275:
        case 49276:
        case 49309:
        case 66055:
        case 68115:
        case 68116:
        case 68117:
        case 75366:
        {
            sp->custom_NameHash = SPELL_HASH_LESSER_HEALING_WAVE; break;
        }
        case 1454:
        case 1455:
        case 1456:
        case 4090:
        case 11687:
        case 11688:
        case 11689:
        case 27222:
        case 28830:
        case 31818:
        case 32553:
        case 57946:
        case 63321:
        {
            sp->custom_NameHash = SPELL_HASH_LIFE_TAP; break;
        }
        case 16614:
        case 23686:
        case 23687:
        case 27983:
        case 37841:
        case 52944:
        case 53062:
        {
            sp->custom_NameHash = SPELL_HASH_LIGHTNING_STRIKE; break;
        }
        case 7001:
        case 27873:
        case 27874:
        case 28276:
        case 48084:
        case 48085:
        case 60123:
        {
            sp->custom_NameHash = SPELL_HASH_LIGHTWELL_RENEW; break;
        }
        case 5530:
        case 12284:
        case 12701:
        case 12702:
        case 12703:
        case 12704:
        case 13709:
        case 13800:
        case 13801:
        case 13802:
        case 13803:
        case 20864:
        case 59224:
        {
            sp->custom_NameHash = SPELL_HASH_MACE_SPECIALIZATION; break;
        }
        case 34774:
        {
            sp->custom_NameHash = SPELL_HASH_MAGTHERIDON_MELEE_TRINKET; break;
        }
        case 22570:
        case 49802:
        {
            sp->custom_NameHash = SPELL_HASH_MAIM; break;
        }
        case 31221:
        case 31222:
        case 31223:
        case 31665:
        case 31666:
        {
            sp->custom_NameHash = SPELL_HASH_MASTER_OF_SUBTLETY; break;
        }
        case 45150:
        {
            sp->custom_NameHash = SPELL_HASH_METEOR_SLASH; break;
        }
        case 30482:
        case 34913:
        case 35915:
        case 35916:
        case 43043:
        case 43044:
        case 43045:
        case 43046:
        {
            sp->custom_NameHash = SPELL_HASH_MOLTEN_ARMOR; break;
        }
        case 1495:
        case 14269:
        case 14270:
        case 14271:
        case 36916:
        case 53339:
        {
            sp->custom_NameHash = SPELL_HASH_MONGOOSE_BITE; break;
        }
        case 9347:
        case 12294:
        case 13737:
        case 15708:
        case 16856:
        case 17547:
        case 19643:
        case 21551:
        case 21552:
        case 21553:
        case 24573:
        case 25248:
        case 27580:
        case 29572:
        case 30330:
        case 31911:
        case 32736:
        case 35054:
        case 37335:
        case 39171:
        case 40220:
        case 43441:
        case 43529:
        case 44268:
        case 47485:
        case 47486:
        case 57789:
        case 65926:
        case 67542:
        case 68782:
        case 68783:
        case 68784:
        case 71552:
        {
            sp->custom_NameHash = SPELL_HASH_MORTAL_STRIKE; break;
        }
        case 1329:
        case 5374:
        case 27576:
        case 32319:
        case 32320:
        case 32321:
        case 34411:
        case 34412:
        case 34413:
        case 34414:
        case 34415:
        case 34416:
        case 34417:
        case 34418:
        case 34419:
        case 41103:
        case 48661:
        case 48662:
        case 48663:
        case 48664:
        case 48665:
        case 48666:
        case 60850:
        {
            sp->custom_NameHash = SPELL_HASH_MUTILATE; break;
        }
        case 58426:
        case 58427:
        {
            sp->custom_NameHash = SPELL_HASH_OVERKILL; break;
        }
        case 7384:
        case 7887:
        case 11584:
        case 11585:
        case 14895:
        case 17198:
        case 24407:
        case 32154:
        case 37321:
        case 37529:
        case 43456:
        case 58516:
        case 65924:
        {
            sp->custom_NameHash = SPELL_HASH_OVERPOWER; break;
        }
        case 47540:
        case 47666:
        case 47750:
        case 47757:
        case 47758:
        case 52983:
        case 52984:
        case 52985:
        case 52986:
        case 52987:
        case 52988:
        case 52998:
        case 52999:
        case 53000:
        case 53001:
        case 53002:
        case 53003:
        case 53005:
        case 53006:
        case 53007:
        case 54518:
        case 54520:
        case 66097:
        case 66098:
        case 68029:
        case 68030:
        case 68031:
        case 69905:
        case 69906:
        case 71137:
        case 71138:
        case 71139:
        {
            sp->custom_NameHash = SPELL_HASH_PENANCE; break;
        }
        case 59634:
        {
            sp->custom_NameHash = SPELL_HASH_POLYMORPH___PENGUIN; break;
        }
        case 45683:
        case 45684:
        {
            sp->custom_NameHash = SPELL_HASH_POLYMORPH__CRAFTY_WOBBLESPROCKET; break;
        }
        case 9005:
        case 9823:
        case 9827:
        case 27006:
        case 39449:
        case 43356:
        case 49803:
        case 54272:
        case 55077:
        case 61184:
        case 64399:
        {
            sp->custom_NameHash = SPELL_HASH_POUNCE; break;
        }
        case 12043:
        case 29976:
        {
            sp->custom_NameHash = SPELL_HASH_PRESENCE_OF_MIND; break;
        }
        case 8122:
        case 8124:
        case 10888:
        case 10890:
        case 13704:
        case 15398:
        case 22884:
        case 26042:
        case 27610:
        case 34322:
        case 43432:
        case 65543:
        {
            sp->custom_NameHash = SPELL_HASH_PSYCHIC_SCREAM; break;
        }
        case 52942:
        case 52961:
        case 59836:
        case 59837:
        {
            sp->custom_NameHash = SPELL_HASH_PULSING_SHOCKWAVE; break;
        }
        case 42292:
        case 65547:
        {
            sp->custom_NameHash = SPELL_HASH_PVP_TRINKET; break;
        }
        case 11366:
        case 12505:
        case 12522:
        case 12523:
        case 12524:
        case 12525:
        case 12526:
        case 17273:
        case 17274:
        case 18809:
        case 20228:
        case 24995:
        case 27132:
        case 29459:
        case 29978:
        case 31263:
        case 33938:
        case 33975:
        case 36277:
        case 36819:
        case 38535:
        case 41578:
        case 42890:
        case 42891:
        case 64698:
        case 70516:
        {
            sp->custom_NameHash = SPELL_HASH_PYROBLAST; break;
        }
        case 4629:
        case 5740:
        case 6219:
        case 11677:
        case 11678:
        case 11990:
        case 16005:
        case 19474:
        case 19717:
        case 20754:
        case 24669:
        case 27212:
        case 28794:
        case 31340:
        case 31598:
        case 33508:
        case 33617:
        case 33627:
        case 33972:
        case 34169:
        case 34185:
        case 34360:
        case 34435:
        case 36808:
        case 37279:
        case 37465:
        case 38635:
        case 38741:
        case 39024:
        case 39273:
        case 39363:
        case 39376:
        case 42023:
        case 42218:
        case 42223:
        case 42224:
        case 42225:
        case 42226:
        case 42227:
        case 43440:
        case 47817:
        case 47818:
        case 47819:
        case 47820:
        case 49518:
        case 54099:
        case 54210:
        case 57757:
        case 58936:
        case 59971:
        case 69670:
        {
            sp->custom_NameHash = SPELL_HASH_RAIN_OF_FIRE; break;
        }
        case 1822:
        case 1823:
        case 1824:
        case 9904:
        case 24331:
        case 24332:
        case 27003:
        case 27556:
        case 27638:
        case 36332:
        case 48573:
        case 48574:
        case 53499:
        case 54668:
        case 59881:
        case 59882:
        case 59883:
        case 59884:
        case 59885:
        case 59886:
        {
            sp->custom_NameHash = SPELL_HASH_RAKE; break;
        }
        case 34948:
        case 34949:
        case 35098:
        case 35099:
        {
            sp->custom_NameHash = SPELL_HASH_RAPID_KILLING; break;
        }
        case 53228:
        case 53232:
        case 56654:
        case 58882:
        case 58883:
        case 64180:
        case 64181:
        {
            sp->custom_NameHash = SPELL_HASH_RAPID_RECUPERATION; break;
        }
        case 3242:
        case 3446:
        case 6785:
        case 6787:
        case 8391:
        case 9866:
        case 9867:
        case 24213:
        case 24333:
        case 27005:
        case 29906:
        case 33781:
        case 48578:
        case 48579:
        case 50518:
        case 53558:
        case 53559:
        case 53560:
        case 53561:
        case 53562:
        {
            sp->custom_NameHash = SPELL_HASH_RAVAGE; break;
        }
        case 13327:
        case 22641:
        case 22646:
        {
            sp->custom_NameHash = SPELL_HASH_RECKLESS_CHARGE; break;
        }
        case 8936:
        case 8938:
        case 8939:
        case 8940:
        case 8941:
        case 9750:
        case 9856:
        case 9857:
        case 9858:
        case 16561:
        case 20665:
        case 22373:
        case 22695:
        case 26980:
        case 27637:
        case 28744:
        case 34361:
        case 39000:
        case 39125:
        case 48442:
        case 48443:
        case 66067:
        case 67968:
        case 67969:
        case 67970:
        case 69882:
        case 71141:
        {
            sp->custom_NameHash = SPELL_HASH_REGROWTH; break;
        }
        case 20066:
        case 29511:
        case 32779:
        case 66008:
        {
            sp->custom_NameHash = SPELL_HASH_REPENTANCE; break;
        }
        case 5405:
        case 10052:
        case 10057:
        case 10058:
        case 18385:
        case 27103:
        case 33394:
        case 42987:
        case 42988:
        case 71565:
        case 71574:
        {
            sp->custom_NameHash = SPELL_HASH_REPLENISH_MANA; break;
        }
        case 46699:
        {
            sp->custom_NameHash = SPELL_HASH_REQUIRES_NO_AMMO; break;
        }
        case 7294:
        case 8990:
        case 10298:
        case 10299:
        case 10300:
        case 10301:
        case 13008:
        case 27150:
        case 54043:
        {
            sp->custom_NameHash = SPELL_HASH_RETRIBUTION_AURA; break;
        }
        case 6572:
        case 6574:
        case 7379:
        case 11600:
        case 11601:
        case 12170:
        case 19130:
        case 25269:
        case 25288:
        case 28844:
        case 30357:
        case 37517:
        case 40392:
        case 57823:
        {
            sp->custom_NameHash = SPELL_HASH_REVENGE; break;
        }
        case 14251:
        case 34097:
        case 34099:
        case 41392:
        case 41393:
        {
            sp->custom_NameHash = SPELL_HASH_RIPOSTE; break;
        }
        case 22419:
        case 61295:
        case 61299:
        case 61300:
        case 61301:
        case 66053:
        case 68118:
        case 68119:
        case 68120:
        case 75367:
        {
            sp->custom_NameHash = SPELL_HASH_RIPTIDE; break;
        }
        case 34586:
        case 34587:
        {
            sp->custom_NameHash = SPELL_HASH_ROMULO_S_POISON; break;
        }
        case 1943:
        case 8639:
        case 8640:
        case 11273:
        case 11274:
        case 11275:
        case 14874:
        case 14903:
        case 15583:
        case 26867:
        case 48671:
        case 48672:
        {
            sp->custom_NameHash = SPELL_HASH_RUPTURE; break;
        }
        case 53601:
        case 58597:
        {
            sp->custom_NameHash = SPELL_HASH_SACRED_SHIELD; break;
        }
        case 2070:
        case 6770:
        case 11297:
        case 30980:
        case 51724:
        {
            sp->custom_NameHash = SPELL_HASH_SAP; break;
        }
        case 1513:
        case 14326:
        case 14327:
        {
            sp->custom_NameHash = SPELL_HASH_SCARE_BEAST; break;
        }
        case 1811:
        case 2948:
        case 8444:
        case 8445:
        case 8446:
        case 8447:
        case 8448:
        case 8449:
        case 10205:
        case 10206:
        case 10207:
        case 10208:
        case 10209:
        case 10210:
        case 13878:
        case 15241:
        case 17195:
        case 27073:
        case 27074:
        case 27375:
        case 27376:
        case 35377:
        case 36807:
        case 38391:
        case 38636:
        case 42858:
        case 42859:
        case 47723:
        case 50183:
        case 56938:
        case 62546:
        case 62548:
        case 62549:
        case 62551:
        case 62553:
        case 63473:
        case 63474:
        case 63475:
        case 63476:
        case 75412:
        case 75419:
        {
            sp->custom_NameHash = SPELL_HASH_SCORCH; break;
        }
        case 20154:
        case 21084:
        case 25742:
        {
            sp->custom_NameHash = SPELL_HASH_SEAL_OF_RIGHTEOUSNESS; break;
        }
        case 5676:
        case 17919:
        case 17920:
        case 17921:
        case 17922:
        case 17923:
        case 27210:
        case 29492:
        case 30358:
        case 30459:
        case 47814:
        case 47815:
        case 65819:
        case 68148:
        case 68149:
        case 68150:
        {
            sp->custom_NameHash = SPELL_HASH_SEARING_PAIN; break;
        }
        case 6358:
        case 6359:
        case 20407:
        case 29490:
        case 30850:
        case 31865:
        {
            sp->custom_NameHash = SPELL_HASH_SEDUCTION; break;
        }
        case 27243:
        case 27285:
        case 32863:
        case 32865:
        case 36123:
        case 37826:
        case 38252:
        case 39367:
        case 43991:
        case 44141:
        case 47831:
        case 47832:
        case 47833:
        case 47834:
        case 47835:
        case 47836:
        case 70388:
        {
            sp->custom_NameHash = SPELL_HASH_SEED_OF_CORRUPTION; break;
        }
        case 34466:
        case 34467:
        case 34468:
        case 34469:
        case 34470:
        {
            sp->custom_NameHash = SPELL_HASH_SERPENT_S_SWIFTNESS; break;
        }
        case 14171:
        case 14172:
        case 14173:
        {
            sp->custom_NameHash = SPELL_HASH_SERRATED_BLADES; break;
        }
        case 19876:
        case 19895:
        case 19896:
        case 27151:
        case 48943:
        {
            sp->custom_NameHash = SPELL_HASH_SHADOW_RESISTANCE_AURA; break;
        }
        case 17877:
        case 18867:
        case 18868:
        case 18869:
        case 18870:
        case 18871:
        case 27263:
        case 29341:
        case 30546:
        case 47826:
        case 47827:
        {
            sp->custom_NameHash = SPELL_HASH_SHADOWBURN; break;
        }
        case 30283:
        case 30413:
        case 30414:
        case 35373:
        case 39082:
        case 45270:
        case 47846:
        case 47847:
        case 56733:
        case 61463:
        {
            sp->custom_NameHash = SPELL_HASH_SHADOWFURY; break;
        }
        case 53600:
        case 61411:
        {
            sp->custom_NameHash = SPELL_HASH_SHIELD_OF_RIGHTEOUSNESS; break;
        }
        case 8242:
        case 15655:
        case 23922:
        case 23923:
        case 23924:
        case 23925:
        case 25258:
        case 29684:
        case 30356:
        case 30688:
        case 46762:
        case 47487:
        case 47488:
        case 49863:
        case 59142:
        case 69903:
        case 72645:
        {
            sp->custom_NameHash = SPELL_HASH_SHIELD_SLAM; break;
        }
        case 5938:
        case 5940:
        {
            sp->custom_NameHash = SPELL_HASH_SHIV; break;
        }
        case 25425:
        case 33686:
        case 46968:
        case 55636:
        case 55918:
        case 57728:
        case 57741:
        case 58947:
        case 58977:
        case 63783:
        case 63982:
        case 72149:
        case 73499:
        case 73794:
        case 73795:
        case 73796:
        case 75343:
        case 75417:
        case 75418:
        {
            sp->custom_NameHash = SPELL_HASH_SHOCKWAVE; break;
        }
        case 6726:
        case 8988:
        case 12528:
        case 15487:
        case 18278:
        case 18327:
        case 22666:
        case 23207:
        case 26069:
        case 27559:
        case 29943:
        case 30225:
        case 37160:
        case 38491:
        case 38913:
        case 54093:
        case 56777:
        case 65542:
        {
            sp->custom_NameHash = SPELL_HASH_SILENCE; break;
        }
        case 18498:
        case 74347:
        {
            sp->custom_NameHash = SPELL_HASH_SILENCED___GAG_ORDER; break;
        }
        case 18469:
        case 55021:
        {
            sp->custom_NameHash = SPELL_HASH_SILENCED___IMPROVED_COUNTERSPELL; break;
        }
        case 18425:
        {
            sp->custom_NameHash = SPELL_HASH_SILENCED___IMPROVED_KICK; break;
        }
        case 1752:
        case 1757:
        case 1758:
        case 1759:
        case 1760:
        case 8621:
        case 11293:
        case 11294:
        case 14873:
        case 15581:
        case 15667:
        case 19472:
        case 26861:
        case 26862:
        case 46558:
        case 48637:
        case 48638:
        case 57640:
        case 59409:
        case 60195:
        case 69920:
        case 71145:
        {
            sp->custom_NameHash = SPELL_HASH_SINISTER_STRIKE; break;
        }
        case 35195:
        case 41597:
        case 63106:
        case 63108:
        {
            sp->custom_NameHash = SPELL_HASH_SIPHON_LIFE; break;
        }
        case 6353:
        case 17924:
        case 27211:
        case 30545:
        case 47824:
        case 47825:
        {
            sp->custom_NameHash = SPELL_HASH_SOUL_FIRE; break;
        }
        case 2912:
        case 8949:
        case 8950:
        case 8951:
        case 9875:
        case 9876:
        case 21668:
        case 25298:
        case 26986:
        case 35243:
        case 38935:
        case 40344:
        case 48464:
        case 48465:
        case 65854:
        case 67947:
        case 67948:
        case 67949:
        case 75332:
        {
            sp->custom_NameHash = SPELL_HASH_STARFIRE; break;
        }
        case 16922:
        {
            sp->custom_NameHash = SPELL_HASH_STARFIRE_STUN; break;
        }
        case 34120:
        case 49051:
        case 49052:
        case 56641:
        case 65867:
        {
            sp->custom_NameHash = SPELL_HASH_STEADY_SHOT; break;
        }
        case 1784:
        case 1785:
        case 1786:
        case 1787:
        case 8822:
        case 30831:
        case 30991:
        case 31526:
        case 31621:
        case 32199:
        case 32615:
        case 34189:
        case 42347:
        case 42866:
        case 42943:
        case 52188:
        case 58506:
        {
            sp->custom_NameHash = SPELL_HASH_STEALTH; break;
        }
        case 39796:
        {
            sp->custom_NameHash = SPELL_HASH_STONECLAW_STUN; break;
        }
        case 7386:
        case 7405:
        case 8380:
        case 11596:
        case 11597:
        case 11971:
        case 13444:
        case 15502:
        case 15572:
        case 16145:
        case 21081:
        case 24317:
        case 25051:
        case 25225:
        case 27991:
        case 30901:
        case 47467:
        case 48893:
        case 50370:
        case 53618:
        case 54188:
        case 57807:
        case 58461:
        case 58567:
        case 59350:
        case 59608:
        case 64978:
        case 65936:
        case 71554:
        {
            sp->custom_NameHash = SPELL_HASH_SUNDER_ARMOR; break;
        }
        case 27554:
        case 31279:
        case 50256:
        case 53498:
        case 53526:
        case 53528:
        case 53529:
        case 53532:
        case 53533:
        {
            sp->custom_NameHash = SPELL_HASH_SWIPE; break;
        }
        case 34471:
        case 34692:
        case 38373:
        case 50098:
        case 70029:
        {
            sp->custom_NameHash = SPELL_HASH_THE_BEAST_WITHIN; break;
        }
        case 16929:
        case 16930:
        case 16931:
        case 19609:
        case 19610:
        case 19612:
        case 50502:
        {
            sp->custom_NameHash = SPELL_HASH_THICK_HIDE; break;
        }
        case 6343:
        case 8198:
        case 8204:
        case 8205:
        case 11580:
        case 11581:
        case 13532:
        case 25264:
        case 47501:
        case 47502:
        case 57832:
        case 60019:
        {
            sp->custom_NameHash = SPELL_HASH_THUNDER_CLAP; break;
        }
        case 10326:
        {
            sp->custom_NameHash = SPELL_HASH_TURN_EVIL; break;
        }
        case 19616:
        case 19617:
        case 19618:
        case 19619:
        case 19620:
        {
            sp->custom_NameHash = SPELL_HASH_UNLEASHED_FURY; break;
        }
        case 30108:
        case 30404:
        case 30405:
        case 31117:
        case 34438:
        case 34439:
        case 35183:
        case 43522:
        case 43523:
        case 47841:
        case 47843:
        case 65812:
        case 65813:
        case 68154:
        case 68155:
        case 68156:
        case 68157:
        case 68158:
        case 68159:
        {
            sp->custom_NameHash = SPELL_HASH_UNSTABLE_AFFLICTION; break;
        }
        case 1856:
        case 1857:
        case 11327:
        case 11329:
        case 24223:
        case 24228:
        case 24229:
        case 24230:
        case 24231:
        case 24232:
        case 24233:
        case 24699:
        case 26888:
        case 26889:
        case 27617:
        case 29448:
        case 31619:
        case 35205:
        case 39667:
        case 41476:
        case 41479:
        case 44290:
        case 55964:
        case 71400:
        {
            sp->custom_NameHash = SPELL_HASH_VANISH;
            break;
        }
        case 34428:
        {
            sp->custom_NameHash = SPELL_HASH_VICTORY_RUSH;
            break;
        }
        case 1680:
        case 8989:
        case 9633:
        case 13736:
        case 15576:
        case 15577:
        case 15578:
        case 15589:
        case 17207:
        case 24236:
        case 26038:
        case 26083:
        case 26084:
        case 26686:
        case 28334:
        case 28335:
        case 29573:
        case 29851:
        case 29852:
        case 31737:
        case 31738:
        case 31909:
        case 31910:
        case 33238:
        case 33239:
        case 33500:
        case 36132:
        case 36142:
        case 36175:
        case 36981:
        case 36982:
        case 37582:
        case 37583:
        case 37640:
        case 37641:
        case 37704:
        case 38618:
        case 38619:
        case 39232:
        case 40236:
        case 40653:
        case 40654:
        case 41056:
        case 41057:
        case 41058:
        case 41059:
        case 41061:
        case 41097:
        case 41098:
        case 41194:
        case 41195:
        case 41399:
        case 41400:
        case 43442:
        case 44949:
        case 45895:
        case 45896:
        case 46270:
        case 46271:
        case 48280:
        case 48281:
        case 49807:
        case 50228:
        case 50229:
        case 50622:
        case 52027:
        case 52028:
        case 52977:
        case 54797:
        case 55266:
        case 55267:
        case 55463:
        case 55977:
        case 56408:
        case 59322:
        case 59323:
        case 59549:
        case 59550:
        case 61076:
        case 61078:
        case 61136:
        case 61137:
        case 61139:
        case 63805:
        case 63806:
        case 63807:
        case 63808:
        case 65510:
        case 67037:
        case 67716: 
        {
            sp->custom_NameHash = SPELL_HASH_WHIRLWIND; 
            break; 
        }
        case 7744: 
        {
            sp->custom_NameHash = SPELL_HASH_WILL_OF_THE_FORSAKEN; 
            break; 
        }
        case 8232:
        case 8235:
        case 10486:
        case 16362:
        case 25505:
        case 32911:
        case 35886:
        case 58801:
        case 58803:
        case 58804:
        {
            sp->custom_NameHash = SPELL_HASH_WINDFURY_WEAPON; 
            break; 
        }
        case 5176:
        case 5177:
        case 5178:
        case 5179:
        case 5180:
        case 6780:
        case 8905:
        case 9739:
        case 9912:
        case 17144:
        case 18104:
        case 20698:
        case 21667:
        case 21807:
        case 26984:
        case 26985:
        case 31784:
        case 43619:
        case 48459:
        case 48461:
        case 52501:
        case 57648:
        case 59986:
        case 62793:
        case 63259:
        case 63569:
        case 65862:
        case 67951:
        case 67952:
        case 67953:
        case 69968:
        case 71148:
        case 75327:
        {
            sp->custom_NameHash = SPELL_HASH_WRATH; 
            break;
        }
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

        //apply custom name hash to spell id
        ApplyObsoleteNameHash(sp);

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
        switch (sp->Id)
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
                temp = sp->Effect[1];
                sp->Effect[1] = sp->Effect[2];
                sp->Effect[2] = temp;
                temp = sp->EffectDieSides[1];
                sp->EffectDieSides[1] = sp->EffectDieSides[2];
                sp->EffectDieSides[2] = temp;
                //temp = sp->EffectBaseDice[1];    sp->EffectBaseDice[1] = sp->EffectBaseDice[2] ;        sp->EffectBaseDice[2] = temp;
                //ftemp = sp->EffectDicePerLevel[1];            sp->EffectDicePerLevel[1] = sp->EffectDicePerLevel[2] ;                sp->EffectDicePerLevel[2] = ftemp;
                ftemp = sp->EffectRealPointsPerLevel[1];
                sp->EffectRealPointsPerLevel[1] = sp->EffectRealPointsPerLevel[2];
                sp->EffectRealPointsPerLevel[2] = ftemp;
                temp = sp->EffectBasePoints[1];
                sp->EffectBasePoints[1] = sp->EffectBasePoints[2];
                sp->EffectBasePoints[2] = temp;
                temp = sp->EffectMechanic[1];
                sp->EffectMechanic[1] = sp->EffectMechanic[2];
                sp->EffectMechanic[2] = temp;
                temp = sp->EffectImplicitTargetA[1];
                sp->EffectImplicitTargetA[1] = sp->EffectImplicitTargetA[2];
                sp->EffectImplicitTargetA[2] = temp;
                temp = sp->EffectImplicitTargetB[1];
                sp->EffectImplicitTargetB[1] = sp->EffectImplicitTargetB[2];
                sp->EffectImplicitTargetB[2] = temp;
                temp = sp->EffectRadiusIndex[1];
                sp->EffectRadiusIndex[1] = sp->EffectRadiusIndex[2];
                sp->EffectRadiusIndex[2] = temp;
                temp = sp->EffectApplyAuraName[1];
                sp->EffectApplyAuraName[1] = sp->EffectApplyAuraName[2];
                sp->EffectApplyAuraName[2] = temp;
                temp = sp->EffectAmplitude[1];
                sp->EffectAmplitude[1] = sp->EffectAmplitude[2];
                sp->EffectAmplitude[2] = temp;
                ftemp = sp->EffectMultipleValue[1];
                sp->EffectMultipleValue[1] = sp->EffectMultipleValue[2];
                sp->EffectMultipleValue[2] = ftemp;
                temp = sp->EffectChainTarget[1];
                sp->EffectChainTarget[1] = sp->EffectChainTarget[2];
                sp->EffectChainTarget[2] = temp;
                temp = sp->EffectMiscValue[1];
                sp->EffectMiscValue[1] = sp->EffectMiscValue[2];
                sp->EffectMiscValue[2] = temp;
                temp = sp->EffectTriggerSpell[1];
                sp->EffectTriggerSpell[1] = sp->EffectTriggerSpell[2];
                sp->EffectTriggerSpell[2] = temp;
                ftemp = sp->EffectPointsPerComboPoint[1];
                sp->EffectPointsPerComboPoint[1] = sp->EffectPointsPerComboPoint[2];
                sp->EffectPointsPerComboPoint[2] = ftemp;
            } break;
            default:
                break;
        }

        for (uint32 b = 0; b < 3; ++b)
        {
            if (sp->EffectTriggerSpell[b] != 0 && sSpellCustomizations.GetSpellInfo(sp->EffectTriggerSpell[b]) == NULL)
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
				sp->AttributesExC |= ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD;
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

        // find diminishing status
        //\todo 16/03/08 Zyres: sql
        sp->custom_DiminishStatus = GetDiminishingGroup(sp->custom_NameHash);

        // various flight spells
        // these make vehicles and other charmed stuff fliable
        if (sp->activeIconID == 2158)
            sp->Attributes |= ATTRIBUTES_PASSIVE;


        //Name includes "" overwrites
        switch (sp->Id)
        {
        case 70908:
            sp->EffectImplicitTargetA[0] = EFF_TARGET_DYNAMIC_OBJECT;
            break;
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

        if (sp->custom_proc_interval > 0)      // if (sp->custom_proc_interval != 0)
            sp->procFlags |= PROC_REMOVEONUSE;

        //shaman - shock, has no spellgroup.very dangerous move !

        //mage - fireball. Only some of the spell has the flags

        switch (sp->Id)
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

    for (auto it = sSpellCustomizations.GetSpellInfoStore()->begin(); it != sSpellCustomizations.GetSpellInfoStore()->end(); ++it)
    {
        // get spellentry
        sp = sSpellCustomizations.GetSpellInfo(it->first);
        if (sp == nullptr)
            continue;

        //Setting Cast Time Coefficient
        auto spell_cast_time = sSpellCastTimesStore.LookupEntry(sp->CastingTimeIndex);
        float castaff = float(GetCastTime(spell_cast_time));
        if (castaff < 1500)
            castaff = 1500;
        else if (castaff > 7000)
            castaff = 7000;

        sp->casttime_coef = castaff / 3500;

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
            // SPELL_HASH_GRACE (without ranks)
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
                sp->AttributesExC |= ATTRIBUTESEXC_NO_DONE_BONUS;
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
                sp->AttributesExC |= ATTRIBUTESEXC_NO_DONE_BONUS;
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
            sp = sSpellCustomizations.GetSpellInfo(f[0].GetUInt32());
            if (sp != NULL)
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

    ////////////////////////////////////////////////////////////
    // Arms

    // Juggernaut
    sp = CheckAndReturnSpellEntry(65156);
    if (sp != NULL)
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;

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

    // Warrior - Heroic Fury
    sp = CheckAndReturnSpellEntry(60970);
    if (sp != NULL)
    {
        sp->Effect[0] = SPELL_EFFECT_DUMMY;
    }

    ////////////////////////////////////////////////////////////
    // Protection

    // Intervene  Ranger: stop attack
    sp = CheckAndReturnSpellEntry(3411);
    if (sp != NULL)
    {
        sp->Attributes |= ATTRIBUTES_STOP_ATTACK;
    }

    //////////////////////////////////////////
    // PALADIN                                //
    //////////////////////////////////////////

    // Insert paladin spell fixes here

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

    //Paladin - Seal of Martyr
    sp = CheckAndReturnSpellEntry(53720);
    if (sp != NULL)
    {
        sp->School = SCHOOL_HOLY;
    }
    //Paladin - seal of blood
    sp = CheckAndReturnSpellEntry(31892);
    if (sp != NULL)
    {
        sp->School = SCHOOL_HOLY;
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
    }

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

    //Paladin - Art of War
    sp = CheckAndReturnSpellEntry(53486);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_DAMAGE_DONE;
    }
    sp = CheckAndReturnSpellEntry(53489);
    if (sp != NULL)
        sp->AuraInterruptFlags = AURA_INTERRUPT_ON_CAST_SPELL;

    sp = CheckAndReturnSpellEntry(53488);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_MOD_DAMAGE_DONE;
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

    //////////////////////////////////////////
    // HUNTER                                //
    //////////////////////////////////////////

    // Insert hunter spell fixes here

    //Hunter - Bestial Wrath
    sp = CheckAndReturnSpellEntry(19574);
    if (sp != NULL)
        sp->EffectApplyAuraName[2] = SPELL_AURA_DUMMY;

    //Hunter - The Beast Within
    sp = CheckAndReturnSpellEntry(34471);
    if (sp != NULL)
        sp->EffectApplyAuraName[2] = SPELL_AURA_DUMMY;

    //Hunter - Go for the Throat
    sp = CheckAndReturnSpellEntry(34952);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;
    sp = CheckAndReturnSpellEntry(34953);
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_PET;

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
    
    /* Zyres: Same procFlags are already in the dbcs!
    //Hunter : Entrapment
    sp = CheckAndReturnSpellEntry(19184);
    if (sp != NULL)
        sp->procFlags = PROC_ON_TRAP_TRIGGER;
    sp = CheckAndReturnSpellEntry(19387);
    if (sp != NULL)
        sp->procFlags = PROC_ON_TRAP_TRIGGER;
    sp = CheckAndReturnSpellEntry(19388);
    if (sp != NULL)
        sp->procFlags = PROC_ON_TRAP_TRIGGER;*/

    // Feed pet
    sp = CheckAndReturnSpellEntry(6991);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[0] = 0;
    }

    //\todo 16/03/08 Zyres: sql
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
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_FRIEND;
    }
    sp = CheckAndReturnSpellEntry(48110);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_FRIEND;
    }
    sp = CheckAndReturnSpellEntry(48111);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_DUMMY;
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
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    }
    sp = CheckAndReturnSpellEntry(15312);   //rank 2
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    }
    sp = CheckAndReturnSpellEntry(15313);   //rank 3
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    }
    sp = CheckAndReturnSpellEntry(15314);   //rank 4
    if (sp != NULL)
    {
        sp->Effect[1] = SPELL_EFFECT_APPLY_AURA;
        sp->EffectApplyAuraName[1] = SPELL_AURA_DUMMY;
    }
    sp = CheckAndReturnSpellEntry(15316);   //rank 5
    if (sp != NULL)
    {
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
        sp->AttributesExC |= ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD;
    sp = CheckAndReturnSpellEntry(27792);   // This is casted by Apply Aura: Spirit of Redemption
    if (sp != NULL)
        sp->AttributesExC |= ATTRIBUTESEXC_CAN_PERSIST_AND_CASTED_WHILE_DEAD;

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
    SpellInfo*  parentsp = CheckAndReturnSpellEntry(30823);
    SpellInfo* triggersp = CheckAndReturnSpellEntry(30824);
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

    ////////////////////////////////////////////////////////////
    // Backlash
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

    //////////////////////////////////////////
    // DRUID                                //
    //////////////////////////////////////////

    // Insert druid spell fixes here

    ////////////////////////////////////////////////////////////
    // Balance
    ////////////////////////////////////////////////////////////

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

    // Druid - Primal Fury (talent)
    sp = CheckAndReturnSpellEntry(37116);
    if (sp != NULL)
        sp->RequiredShapeShift = 0;

    sp = CheckAndReturnSpellEntry(37117);
    if (sp != NULL)
        sp->RequiredShapeShift = 0;

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

    //Primal Instinct - Idol of Terror proc
    sp = CheckAndReturnSpellEntry(43738);
    if (sp != NULL)
    {
        sp->custom_self_cast_only = true;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_MANGLE__CAT_;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_MANGLE__BEAR_;
    }

    //Thunderfury
    sp = CheckAndReturnSpellEntry(21992);
    if (sp != NULL)
    {
        sp->EffectImplicitTargetA[2] = EFF_TARGET_ALL_ENEMIES_AROUND_CASTER; // cebernic: for enemies not self
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

    //Totem of the Third Wind - bad range
    sp = CheckAndReturnSpellEntry(34132);
    if (sp != NULL)
    {
        sp->rangeIndex = 5;
    }
    sp = CheckAndReturnSpellEntry(42371);
    if (sp != NULL)
    {
        sp->rangeIndex = 5;
    }
    sp = CheckAndReturnSpellEntry(43729);
    if (sp != NULL)
    {
        sp->rangeIndex = 5;
    }
    sp = CheckAndReturnSpellEntry(46099);
    if (sp != NULL)
    {
        sp->rangeIndex = 5;
    }

    // Eye of Acherus, our phase shift mode messes up the control :/
    sp = CheckAndReturnSpellEntry(51852);
    if (sp != NULL)
        sp->Effect[0] = SPELL_EFFECT_NULL;


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

    //Figurine - Shadowsong Panther
    sp = CheckAndReturnSpellEntry(46784);        //    http://www.wowhead.com/?item=35702
    if (sp != NULL)
        sp->AttributesEx |= ATTRIBUTESEX_NOT_BREAK_STEALTH;

    // Infernal Protection
    sp = CheckAndReturnSpellEntry(36488);            //    http://www.wowhead.com/?spell=36488
    if (sp != NULL)
        sp->EffectImplicitTargetA[0] = EFF_TARGET_SINGLE_FRIEND;


    //Fury of the Five Flights
    sp = CheckAndReturnSpellEntry(60313);
    if (sp != NULL)
    {
        sp->maxstack = 20;
    }

    //Pendant of the Violet Eye
    sp = CheckAndReturnSpellEntry(35095);
    if (sp != NULL)
    {
        sp->custom_self_cast_only = true;
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

    // Insert Death Knight spells here

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
    sp = sSpellCustomizations.GetSpellInfo(49937);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DAMAGE;
        sp->Effect[0] = SPELL_EFFECT_PERSISTENT_AREA_AURA;
    }

    sp = sSpellCustomizations.GetSpellInfo(49936);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DAMAGE;
        sp->Effect[0] = SPELL_EFFECT_PERSISTENT_AREA_AURA;
    }

    sp = sSpellCustomizations.GetSpellInfo(49938);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DAMAGE;
        sp->Effect[0] = SPELL_EFFECT_PERSISTENT_AREA_AURA;
    }

    sp = sSpellCustomizations.GetSpellInfo(43265);
    if (sp != NULL)
    {
        sp->EffectApplyAuraName[0] = SPELL_AURA_PERIODIC_DAMAGE;
        sp->Effect[0] = SPELL_EFFECT_PERSISTENT_AREA_AURA;
    }

    // Runic Empowerment
    /*sp = sSpellCustomizations.GetSpellInfo(81229);
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
    sp = sSpellCustomizations.GetSpellInfo(93099);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_ANY_DAMAGE_VICTIM;
        sp->EffectApplyAuraName[0] = SPELL_AURA_PROC_TRIGGER_SPELL;
        sp->EffectTriggerSpell[0] = 76691;
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

    //Other Librams
    //Libram of Saints Departed and Libram of Zeal
    sp = CheckAndReturnSpellEntry(34263);
    if (sp != NULL)
    {
        sp->custom_self_cast_only = true;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_JUDGEMENT;
        sp->procChance = 100;
    }

    //Libram of Avengement
    sp = CheckAndReturnSpellEntry(34260);
    if (sp != NULL)
    {
        sp->custom_self_cast_only = true;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_JUDGEMENT;
        sp->procChance = 100;
    }

    //Libram of Mending
    sp = CheckAndReturnSpellEntry(43742);
    if (sp != NULL)
    {
        sp->custom_self_cast_only = true;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_HOLY_LIGHT;
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
        SpellInfo * ritOfSumm = sSpellCustomizations.GetSpellInfo(ritOfSummId);
        if (ritOfSumm != NULL)
        {
            memcpy(ritOfSumm, sp, sizeof(SpellInfo));
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

    //Mage - firestarter talent ranks 1 & 2
    // overwrite procs, should only proc on these 2 spellgroups.
    sp = CheckAndReturnSpellEntry(44442);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->procChance = 50;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_DRAGON_S_BREATH;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_BLAST_WAVE;
    }
    sp = CheckAndReturnSpellEntry(44443);
    if (sp != NULL)
    {
        sp->procFlags = PROC_ON_CAST_SPELL;
        sp->custom_ProcOnNameHash[0] = SPELL_HASH_DRAGON_S_BREATH;
        sp->custom_ProcOnNameHash[1] = SPELL_HASH_BLAST_WAVE;
    }
}

