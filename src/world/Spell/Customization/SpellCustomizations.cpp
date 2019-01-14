/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Spell/SpellMgr.h"
#include "Spell/Definitions/AuraEffects.h"
#include "Spell/Definitions/DiminishingGroup.h"
#include "Spell/Definitions/SpellDamageType.h"
#include "Spell/Definitions/SpellEffects.h"
#include "Spell/Definitions/SpellIsFlags.h"

//\brief: This file includes all old setted custom values or spell.dbc overwrite values
// If possible, these should be get rid of or moved under appropriate class (like that diminishing group)

bool SpellMgr::isAlwaysApply(SpellInfo const* spellInfo) const
{
    switch (spellInfo->getId())
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
        case 65545:
        {
            return true;
        }
        default:
            return false;
    }
}

// Calculate the Diminishing Group. This is based on id.
// this off course is very hacky, but as its made done in a proper way
// I leave it here.
uint32_t SpellMgr::getDiminishingGroup(uint32_t id) const
{
    int32_t grp = -1;
    bool pve = false;

    switch (id)
    {
        // SPELL_HASH_WYVERN_STING:
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
        // SPELL_HASH_SLEEP:
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
        // SPELL_HASH_RECKLESS_CHARGE: //Gobling Rocket Helmet
        case 13327:
        case 22641:
        case 22646:
        {
            grp = DIMINISHING_GROUP_SLEEP;
        } break;

        // SPELL_HASH_CYCLONE:
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
        // SPELL_HASH_BLIND:
        case 2094:
        case 21060:
        case 34654:
        case 34694:
        case 42972:
        case 43433:
        case 65960:
        {
            grp = DIMINISHING_GROUP_BLIND_CYCLONE;
            pve = true;
        } break;

        // SPELL_HASH_GOUGE:
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
        // SPELL_HASH_REPENTANCE: // Repentance
        case 20066:
        case 29511:
        case 32779:
        case 66008:
        // SPELL_HASH_SAP:
        case 2070:
        case 6770:
        case 11297:
        case 30980:
        case 51724:
        // SPELL_HASH_POLYMORPH: // Polymorph
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
        // SPELL_HASH_POLYMORPH__CHICKEN: // Chicken
        case 228:
        // SPELL_HASH_POLYMORPH__CRAFTY_WOBBLESPROCKET: // Errr right?
        case 45683:
        case 45684:
        // SPELL_HASH_POLYMORPH__SHEEP: // Good ol' sheep
        case 851:
        case 61816:
        case 61839:
        // SPELL_HASH_POLYMORPH___PENGUIN: // Penguiiiin!!! :D
        case 59634:
        // SPELL_HASH_MAIM: // Maybe here?
        case 22570:
        case 49802:
        // SPELL_HASH_HEX: // Should share diminish group with polymorph, but not interruptflags.
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
            grp = DIMINISHING_GROUP_GOUGE_POLY_SAP;
        } break;

        // SPELL_HASH_FEAR:
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
        // SPELL_HASH_PSYCHIC_SCREAM:
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
        // SPELL_HASH_SEDUCTION:
        case 6358:
        case 6359:
        case 20407:
        case 29490:
        case 30850:
        case 31865:
        // SPELL_HASH_HOWL_OF_TERROR:
        case 5484:
        case 17928:
        case 39048:
        case 50577:
        // SPELL_HASH_SCARE_BEAST:
        case 1513:
        case 14326:
        case 14327:
        {
            grp = DIMINISHING_GROUP_FEAR;
        } break;

        // SPELL_HASH_DEATH_COIL:
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
            grp = DIMINISHING_GROUP_HORROR;
        } break;

        // SPELL_HASH_ENSLAVE_DEMON: // Enslave Demon
        case 1098:
        case 11725:
        case 11726:
        case 20882:
        case 61191:
        // SPELL_HASH_MIND_CONTROL:
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
        // SPELL_HASH_TURN_EVIL:
        case 10326:
        {
            grp = DIMINISHING_GROUP_CHARM; //Charm???
        } break;

        // SPELL_HASH_KIDNEY_SHOT:
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
            grp = DIMINISHING_GROUP_KIDNEY_SHOT;
            pve = true;
        }
        break;

        // SPELL_HASH_BANISH: // Banish
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
            grp = DIMINISHING_GROUP_BANISH;
        } break;


        // group where only 10s limit in pvp is applied, not DR

        // SPELL_HASH_FREEZING_TRAP_EFFECT: // Freezing Trap Effect
        case 3355:
        case 14308:
        case 14309:
        case 31932:
        case 55041:
        // SPELL_HASH_HAMSTRING: // Hamstring
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
        // SPELL_HASH_CURSE_OF_TONGUES:
        case 1714:
        case 11719:
        case 12889:
        case 13338:
        case 15470:
        case 25195:
        {
            grp = DIMINISHING_GROUP_NOT_DIMINISHED;
        } break;

        // SPELL_HASH_RIPOSTE: // Riposte
        case 14251:
        case 34097:
        case 34099:
        case 41392:
        case 41393:
        // SPELL_HASH_DISARM: // Disarm
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
            grp = DIMINISHING_GROUP_DISARM;
        } break;

        // SPELL_HASH_BASH
        case 5211:
        case 6798:
        case 8983:
        case 25515:
        case 43612:
        case 57094:
        case 58861:
        //SPELL_HASH_IMPACT
        case 11103:
        case 12355:
        case 12357:
        case 12358:
        case 64343:
        //SPELL_HASH_CHEAP_SHOT
        case 1833:
        case 6409:
        case 14902:
        case 30986:
        case 31819:
        case 31843:
        case 34243:
        //SPELL_HASH_SHADOWFURY
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
        //SPELL_HASH_CHARGE_STUN
        case 7922:
        case 65929:
        //SPELL_HASH_INTERCEPT
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
        //SPELL_HASH_CONCUSSION_BLOW
        case 12809:
        case 22427:
        case 32588:
        case 52719:
        case 54132:
        //SPELL_HASH_INTIMIDATION
        case 7093:
        case 19577:
        case 24394:
        case 70495:
        //SPELL_HASH_WAR_STOMP
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
        //SPELL_HASH_POUNCE
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
        //SPELL_HASH_HAMMER_OF_JUSTICE
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
            grp = DIMINISHING_GROUP_STUN;
            pve = true;
        } break;

        // SPELL_HASH_STARFIRE_STUN
        case 16922:
        // SPELL_HASH_STONECLAW_STUN
        case 39796:
        // SPELL_HASH_STUN              // Generic ones
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
            grp = DIMINISHING_GROUP_STUN_PROC;
            pve = true;
        } break;

        //SPELL_HASH_ENTANGLING_ROOTS
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
        //SPELL_HASH_FROST_NOVA
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
            grp = DIMINISHING_GROUP_ROOT;
        } break;

        //SPELL_HASH_IMPROVED_WING_CLIP
        case 35963:
        case 47168:
        //SPELL_HASH_FROSTBITE
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
        //SPELL_HASH_IMPROVED_HAMSTRING
        case 12289:
        case 12668:
        case 23694:
        case 23695:
        case 24428:
        //SPELL_HASH_ENTRAPMENT
        case 19184:
        case 19185:
        case 19387:
        case 19388:
        case 64803:
        case 64804:
        {
            grp = DIMINISHING_GROUP_ROOT_PROC;
        } break;

        //SPELL_HASH_HIBERNATE
        case 2637:
        case 18657:
        case 18658:
        {
            grp = DIMINISHING_GROUP_SLEEP;
        } break;

        //SPELL_HASH_SILENCE:
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
        //SPELL_HASH_GARROTE___SILENCE:
        case 1330:
        //SPELL_HASH_SILENCED___IMPROVED_COUNTERSPELL:
        case 18469:
        case 55021:
        //SPELL_HASH_SILENCED___IMPROVED_KICK:
        case 18425:
        //SPELL_HASH_SILENCED___GAG_ORDER:
        case 18498:
        case 74347:
        {
            grp = DIMINISHING_GROUP_SILENCE;
        } break;
    }

    uint32_t ret;
    if (pve)
        ret = grp | (1 << 16);
    else
        ret = grp;

    return ret;
}

void SpellMgr::setSpellEffectAmplitude(SpellInfo* sp)
{
    // Avoid division by zero if spell is a periodic triggered with amplitude of zero
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (sp->getEffect(i) != SPELL_EFFECT_APPLY_AURA)
            continue;

        if (sp->getEffectApplyAuraName(i) == 0 &&
            (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE))
        {
            sp->setEffectAmplitude(1000, i);

            LogDebugFlag(LF_DB_TABLES, "SpellMgr::setSpellEffectAmplitude : Modified amplitude for spell %s (%u)", sp->getName().c_str(), sp->getId());
        }
    }
}

void SpellMgr::setSpellMeleeSpellBool(SpellInfo* sp)
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (sp->getEffect(i) == SPELL_EFFECT_SCHOOL_DAMAGE && sp->getDmgClass() == SPELL_DMG_TYPE_MELEE)
        {
            sp->custom_is_melee_spell = true;
            continue;
        }

        switch (sp->getEffect(i))
        {
        case SPELL_EFFECT_WEAPON_DAMAGE:
        case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
        case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
        case SPELL_EFFECT_DUMMYMELEE:
            sp->custom_is_melee_spell = true;
            break;
        default:
            continue;
        }
    }

    if (sp->custom_is_melee_spell)
        LogDebugFlag(LF_DB_TABLES, "SpellMgr::setSpellMeleeSpellBool : custom_is_melee_spell is true for spell %s (%u)", sp->getName().c_str(), sp->getId());
}

void SpellMgr::setSpellRangedSpellBool(SpellInfo* sp)
{
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (sp->getEffect(i) == SPELL_EFFECT_SCHOOL_DAMAGE && sp->getDmgClass() == SPELL_DMG_TYPE_RANGED)
            sp->custom_is_ranged_spell = true;
    }

    if (sp->custom_is_ranged_spell)
        LogDebugFlag(LF_DB_TABLES, "SpellMgr::setSpellRangedSpellBool : custom_is_ranged_spell is true for spell %s (%u)", sp->getName().c_str(), sp->getId());
}

void SpellMgr::setSpellMissingCIsFlags(SpellInfo* sp)
{
    // Zyres: Special cases, not handled in spell_custom_assign!
    if (sp->isDamagingSpell())
        sp->custom_c_is_flags |= SPELL_FLAG_IS_DAMAGING;
    if (sp->isHealingSpell())
        sp->custom_c_is_flags |= SPELL_FLAG_IS_HEALING;
    if (sp->isTargetingStealthed())
        sp->custom_c_is_flags |= SPELL_FLAG_IS_TARGETINGSTEALTHED;
    if (sp->isRequireCooldownSpell())
        sp->custom_c_is_flags |= SPELL_FLAG_IS_REQUIRECOOLDOWNUPDATE;
}

void SpellMgr::setSpellOnShapeshiftChange(SpellInfo* sp)
{
    // Currently only for spell Track Humanoids
    if (sp->getId() == 5225 || sp->getId() == 19883)
        sp->custom_apply_on_shapeshift_change = true;
}
