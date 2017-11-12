/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
 */

#include "StdAfx.h"
#include "Units/Summons/SummonHandler.h"
#include "Management/LootMgr.h"
#include "Units/Creatures/Vehicle.h"
#include "Server/EventableObject.h"
#include "Objects/DynamicObject.h"
#include "Management/Item.h"
#include "Units/Stats.h"
#include "Server/WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgr.h"
#include "Objects/Faction.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/SpellLog.h"
#include "Spell/Definitions/SpellCastTargetFlags.h"
#include "Spell/Definitions/ProcFlags.h"
#include <Spell/Definitions/AuraInterruptFlags.h>
#include "Spell/Definitions/SpellSchoolConversionTable.h"
#include "Spell/Definitions/SpellTypes.h"
#include "Spell/Definitions/SpellIsFlags.h"
#include "Spell/Definitions/SpellMechanics.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/Definitions/SpellDidHitResult.h"
#include "Spell/Definitions/SpellEffectTarget.h"
#include "Spell/SpellHelpers.h"
#include "Creatures/Pet.h"

using ascemu::World::Spell::Helpers::spellModFlatIntValue;
using ascemu::World::Spell::Helpers::spellModPercentageIntValue;
using ascemu::World::Spell::Helpers::spellModFlatFloatValue;
using ascemu::World::Spell::Helpers::spellModPercentageFloatValue;

#ifdef AE_CLASSIC
static float AttackToRageConversionTable[DBC_PLAYER_LEVEL_CAP + 1] =
{
    0.0f,               // 0
    0.499999998893f,
    0.34874214056f,
    0.267397170992f,
    0.216594535676f,
    0.181852997475f,
    0.156596678244f,
    0.137408407814f,
    0.12233646474f,
    0.110185074062f,
    0.100180723915f,    //10
    0.0918008940243f,
    0.084679891259f,
    0.0785541194583f,
    0.0732287738371f,
    0.0685567746212f,
    0.0644249954237f,
    0.0607450001819f,
    0.0574466557344f,
    0.0544736297718f,
    0.0517801553458f,   //20
    0.0493286648502f,
    0.0470880325642f,
    0.0450322506478f,
    0.0431394187932f,
    0.0413909641335f,
    0.0397710324301f,
    0.0382660082118f,
    0.0368641330875f,
    0.035555199573f,
    0.0343303035574f,   //30
    0.0331816427126f,
    0.0321023511953f,
    0.0310863632415f,
    0.0301282999279f,
    0.0292233746364f,
    0.0283673137143f,
    0.0275562895548f,
    0.0267868638875f,
    0.0260559395055f,
    0.0253607190016f,   //40
    0.0246986693537f,
    0.0240674914139f,
    0.0234650935281f,
    0.0228895686471f,
    0.0223391744027f,
    0.0218123157088f,
    0.0213075295236f,
    0.0208234714647f,
    0.02035890402f,
    0.019912686137f,    //50
    0.0194837640053f,
    0.0190711628769f,
    0.0186739797893f,
    0.0182913770778f,
    0.0179225765793f,
    0.0175668544424f,
    0.0172235364711f,
    0.0168919939405f,
    0.0165716398271f,
    0.0162619254091f   //60
};
#endif
#ifdef AE_TBC
static float AttackToRageConversionTable[DBC_PLAYER_LEVEL_CAP + 1] =
{
    0.0f,               // 0
    0.499999998893f,
    0.34874214056f,
    0.267397170992f,
    0.216594535676f,
    0.181852997475f,
    0.156596678244f,
    0.137408407814f,
    0.12233646474f,
    0.110185074062f,
    0.100180723915f,    //10
    0.0918008940243f,
    0.084679891259f,
    0.0785541194583f,
    0.0732287738371f,
    0.0685567746212f,
    0.0644249954237f,
    0.0607450001819f,
    0.0574466557344f,
    0.0544736297718f,
    0.0517801553458f,   //20
    0.0493286648502f,
    0.0470880325642f,
    0.0450322506478f,
    0.0431394187932f,
    0.0413909641335f,
    0.0397710324301f,
    0.0382660082118f,
    0.0368641330875f,
    0.035555199573f,
    0.0343303035574f,   //30
    0.0331816427126f,
    0.0321023511953f,
    0.0310863632415f,
    0.0301282999279f,
    0.0292233746364f,
    0.0283673137143f,
    0.0275562895548f,
    0.0267868638875f,
    0.0260559395055f,
    0.0253607190016f,   //40
    0.0246986693537f,
    0.0240674914139f,
    0.0234650935281f,
    0.0228895686471f,
    0.0223391744027f,
    0.0218123157088f,
    0.0213075295236f,
    0.0208234714647f,
    0.02035890402f,
    0.019912686137f,    //50
    0.0194837640053f,
    0.0190711628769f,
    0.0186739797893f,
    0.0182913770778f,
    0.0179225765793f,
    0.0175668544424f,
    0.0172235364711f,
    0.0168919939405f,
    0.0165716398271f,
    0.0162619254091f,   //60
    0.0159623371939f,
    0.0156723941359f,
    0.0153916451144f,
    0.0151196666436f,
    0.0148560607885f,
    0.0146004532678f,
    0.0143524917226f,
    0.0141118441351f,
    0.0138781973828f,
    0.0136512559131f    //70
};
#endif
#ifdef AE_WOTLK
static float AttackToRageConversionTable[DBC_PLAYER_LEVEL_CAP + 1] =
{
    0.0f,               // 0
    0.499999998893f,
    0.34874214056f,
    0.267397170992f,
    0.216594535676f,
    0.181852997475f,
    0.156596678244f,
    0.137408407814f,
    0.12233646474f,
    0.110185074062f,
    0.100180723915f,    //10
    0.0918008940243f,
    0.084679891259f,
    0.0785541194583f,
    0.0732287738371f,
    0.0685567746212f,
    0.0644249954237f,
    0.0607450001819f,
    0.0574466557344f,
    0.0544736297718f,
    0.0517801553458f,   //20
    0.0493286648502f,
    0.0470880325642f,
    0.0450322506478f,
    0.0431394187932f,
    0.0413909641335f,
    0.0397710324301f,
    0.0382660082118f,
    0.0368641330875f,
    0.035555199573f,
    0.0343303035574f,   //30
    0.0331816427126f,
    0.0321023511953f,
    0.0310863632415f,
    0.0301282999279f,
    0.0292233746364f,
    0.0283673137143f,
    0.0275562895548f,
    0.0267868638875f,
    0.0260559395055f,
    0.0253607190016f,   //40
    0.0246986693537f,
    0.0240674914139f,
    0.0234650935281f,
    0.0228895686471f,
    0.0223391744027f,
    0.0218123157088f,
    0.0213075295236f,
    0.0208234714647f,
    0.02035890402f,
    0.019912686137f,    //50
    0.0194837640053f,
    0.0190711628769f,
    0.0186739797893f,
    0.0182913770778f,
    0.0179225765793f,
    0.0175668544424f,
    0.0172235364711f,
    0.0168919939405f,
    0.0165716398271f,
    0.0162619254091f,   //60
    0.0159623371939f,
    0.0156723941359f,
    0.0153916451144f,
    0.0151196666436f,
    0.0148560607885f,
    0.0146004532678f,
    0.0143524917226f,
    0.0141118441351f,
    0.0138781973828f,
    0.0136512559131f,   //70
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f     //80
};
#endif
#ifdef AE_CATA
static float AttackToRageConversionTable[DBC_PLAYER_LEVEL_CAP + 1] =
{
    0.0f,               // 0
    0.499999998893f,
    0.34874214056f,
    0.267397170992f,
    0.216594535676f,
    0.181852997475f,
    0.156596678244f,
    0.137408407814f,
    0.12233646474f,
    0.110185074062f,
    0.100180723915f,    //10
    0.0918008940243f,
    0.084679891259f,
    0.0785541194583f,
    0.0732287738371f,
    0.0685567746212f,
    0.0644249954237f,
    0.0607450001819f,
    0.0574466557344f,
    0.0544736297718f,
    0.0517801553458f,   //20
    0.0493286648502f,
    0.0470880325642f,
    0.0450322506478f,
    0.0431394187932f,
    0.0413909641335f,
    0.0397710324301f,
    0.0382660082118f,
    0.0368641330875f,
    0.035555199573f,
    0.0343303035574f,   //30
    0.0331816427126f,
    0.0321023511953f,
    0.0310863632415f,
    0.0301282999279f,
    0.0292233746364f,
    0.0283673137143f,
    0.0275562895548f,
    0.0267868638875f,
    0.0260559395055f,
    0.0253607190016f,   //40
    0.0246986693537f,
    0.0240674914139f,
    0.0234650935281f,
    0.0228895686471f,
    0.0223391744027f,
    0.0218123157088f,
    0.0213075295236f,
    0.0208234714647f,
    0.02035890402f,
    0.019912686137f,    //50
    0.0194837640053f,
    0.0190711628769f,
    0.0186739797893f,
    0.0182913770778f,
    0.0179225765793f,
    0.0175668544424f,
    0.0172235364711f,
    0.0168919939405f,
    0.0165716398271f,
    0.0162619254091f,   //60
    0.0159623371939f,
    0.0156723941359f,
    0.0153916451144f,
    0.0151196666436f,
    0.0148560607885f,
    0.0146004532678f,
    0.0143524917226f,
    0.0141118441351f,
    0.0138781973828f,
    0.0136512559131f,   //70
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,    //80
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f,
    0.0136512559131f    // 85
};
#endif

Unit::Unit() : m_currentSpeedWalk(2.5f),
    m_currentSpeedRun(7.0f), m_currentSpeedRunBack(4.5f), m_currentSpeedSwim(4.722222f), m_currentSpeedSwimBack(2.5f),
    m_currentTurnRate(3.141594f), m_currentSpeedFly(7.0f), m_currentSpeedFlyBack(4.5f), m_currentPitchRate(3.14f),
    m_basicSpeedWalk(2.5f),

    m_basicSpeedRun(7.0f), m_basicSpeedRunBack(4.5f), m_basicSpeedSwim(4.722222f), m_basicSpeedSwimBack(2.5f),
    m_basicTurnRate(3.141594f), m_basicSpeedFly(7.0f), m_basicSpeedFlyBack(4.5f), m_basicPitchRate(3.14f),
    m_movementManager()
{
    int i;

    m_attackTimer = 0;
    m_attackTimer_1 = 0;
    m_dualWield = false;

    m_ignoreArmorPctMaceSpec = 0;
    m_ignoreArmorPct = 0;
    m_fearmodifiers = 0;
    m_unitState = UNIT_STATE_NONE;
    m_deathState = ALIVE;
    m_meleespell = 0;
    m_addDmgOnce = 0;

    m_ObjectSlots[0] = 0;
    m_ObjectSlots[1] = 0;
    m_ObjectSlots[2] = 0;
    m_ObjectSlots[3] = 0;
    m_silenced = 0;
    disarmed = false;

    m_objectType |= TYPE_UNIT;
    m_objectTypeId = TYPEID_UNIT;

#if VERSION_STRING != Cata
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_LIVING | UPDATEFLAG_HAS_POSITION);
#else
    m_updateFlag = UPDATEFLAG_LIVING;
#endif

    //DK:modifiers
    PctRegenModifier = 0;
    for (i = 0; i < 4; i++)
    {
        PctPowerRegenModifier[i] = 1;
    }
    m_speedModifier = 0;
    m_slowdown = 0;
    m_mountedspeedModifier = 0;
    m_maxSpeed = 0;
    for (i = 0; i < 32; i++)
    {
        MechanicsDispels[i] = 0;
        MechanicsResistancesPCT[i] = 0;
        ModDamageTakenByMechPCT[i] = 0;
    }

    //SM
    SM_FDamageBonus = 0;
    SM_PDamageBonus = 0;

    SM_FDur = 0;
    SM_PDur = 0;

    SM_FThreat = 0;
    SM_PThreat = 0;

    SM_FEffect1_Bonus = 0;
    SM_PEffect1_Bonus = 0;

    SM_FCharges = 0;
    SM_PCharges = 0;

    SM_FRange = 0;
    SM_PRange = 0;

    SM_FRadius = 0;
    SM_PRadius = 0;

    SM_CriticalChance = 0;

    SM_FMiscEffect = 0;
    SM_PMiscEffect = 0;

    SM_PNonInterrupt = 0;

    SM_FCastTime = 0;
    SM_PCastTime = 0;

    SM_FCooldownTime = 0;
    SM_PCooldownTime = 0;

    SM_FEffect2_Bonus = 0;
    SM_PEffect2_Bonus = 0;

    SM_FCost = 0;
    SM_PCost = 0;

    SM_PCriticalDamage = 0;

    SM_FHitchance = 0;

    SM_FAdditionalTargets = 0;

    SM_FChanceOfSuccess = 0;

    SM_FAmptitude = 0;
    SM_PAmptitude = 0;

    SM_PJumpReduce = 0;

    SM_FGlobalCooldown = 0;
    SM_PGlobalCooldown = 0;

    SM_FDOT = 0;
    SM_PDOT = 0;

    SM_FEffect3_Bonus = 0;
    SM_PEffect3_Bonus = 0;

    SM_FPenalty = 0;
    SM_PPenalty = 0;

    SM_FEffectBonus = 0;
    SM_PEffectBonus = 0;

    SM_FRezist_dispell = 0;
    SM_PRezist_dispell = 0;

    m_pacified = 0;
    m_interruptRegen = 0;
    m_resistChance = 0;
    m_powerRegenPCT = 0;
    RAPvModifier = 0;
    APvModifier = 0;
    stalkedby = 0;

    m_extraattacks = 0;
    m_stunned = 0;
    m_manashieldamt = 0;
    m_rootCounter = 0;
    m_triggerSpell = 0;
    m_triggerDamage = 0;
    m_canMove = 0;
    m_noInterrupt = 0;
    m_modlanguage = -1;
    m_magnetcaster = 0;

    m_CombatResult_Dodge = 0;
    m_CombatResult_Parry = 0;

    m_useAI = false;
    for (i = 0; i < 10; i++)
    {
        dispels[i] = 0;
        CreatureAttackPowerMod[i] = 0;
        CreatureRangedAttackPowerMod[i] = 0;
    }
    //REMIND:Update these if you make any changes
    CreatureAttackPowerMod[UNIT_TYPE_MISC] = 0;
    CreatureRangedAttackPowerMod[UNIT_TYPE_MISC] = 0;
    CreatureAttackPowerMod[11] = 0;
    CreatureRangedAttackPowerMod[11] = 0;

    m_invisibility = 0;
    m_invisible = false;
    m_invisFlag = INVIS_FLAG_NORMAL;

    for (i = 0; i < INVIS_FLAG_TOTAL; i++)
    {
        m_invisDetect[i] = 0;
    }

    m_stealthLevel = 0;
    m_stealthDetectBonus = 0;
    m_stealth = 0;
    m_can_stealth = true;

    for (i = 0; i < 5; i++)
        BaseStats[i] = 0;

    m_H_regenTimer = 2000;
    m_P_regenTimer = 2000;

    //	if (GetTypeId() == TYPEID_PLAYER) //only player for now
    //		CalculateActualArmor();

    m_aiInterface = new AIInterface();
    m_aiInterface->Init(this, AI_SCRIPT_AGRO, Movement::WP_MOVEMENT_SCRIPT_NONE);

    m_emoteState = 0;
    m_oldEmote = 0;

    BaseDamage[0] = 0;
    BaseOffhandDamage[0] = 0;
    BaseRangedDamage[0] = 0;
    BaseDamage[1] = 0;
    BaseOffhandDamage[1] = 0;
    BaseRangedDamage[1] = 0;

    m_CombatUpdateTimer = 0;
    for (i = 0; i < SCHOOL_COUNT; i++)
    {
        SchoolImmunityList[i] = 0;
        BaseResistance[i] = 0;
        HealDoneMod[i] = 0;
        HealDonePctMod[i] = 0;
        HealTakenMod[i] = 0;
        HealTakenPctMod[i] = 0;
        DamageTakenMod[i] = 0;
        SchoolCastPrevent[i] = 0;
        DamageTakenPctMod[i] = 0;
        SpellCritChanceSchool[i] = 0;
        PowerCostMod[i] = 0;
        PowerCostPctMod[i] = 0; // armor penetration & spell penetration
        AttackerCritChanceMod[i] = 0;
        CritMeleeDamageTakenPctMod[i] = 0;
        CritRangedDamageTakenPctMod[i] = 0;
        m_generatedThreatModifyer[i] = 0;
        DoTPctIncrease[i] = 0;
    }
    DamageTakenPctModOnHP35 = 1;
    RangedDamageTaken = 0;
    AOEDmgMod = 1.0f;

    for (i = 0; i < 5; i++)
    {
        m_detectRangeGUID[i] = 0;
        m_detectRangeMOD[i] = 0;
    }

    detectRange = 0;
    trackStealth = false;

    m_threatModifyer = 0;
    memset(m_auras, 0, (MAX_TOTAL_AURAS_END)*sizeof(Aura*));

    // diminishing return stuff
    memset(m_diminishAuraCount, 0, DIMINISHING_GROUP_COUNT);
    memset(m_diminishCount, 0, DIMINISHING_GROUP_COUNT * 2);
    memset(m_diminishTimer, 0, DIMINISHING_GROUP_COUNT * 2);
    memset(m_auraStackCount, 0, MAX_NEGATIVE_VISUAL_AURAS_END);
    memset(m_auravisuals, 0, MAX_NEGATIVE_VISUAL_AURAS_END * sizeof(uint32));

    m_diminishActive = false;
    dynObj = 0;
    pLastSpell = 0;
    m_flyspeedModifier = 0;
    bInvincible = false;
    m_redirectSpellPackets = 0;
    can_parry = false;
    bProcInUse = false;
    spellcritperc = 0;

    polySpell = 0;
    RangedDamageTaken = 0;
    m_procCounter = 0;
    m_damgeShieldsInUse = false;
    //	fearSpell = 0;
    m_extraAttackCounter = false;
    CombatStatus.SetUnit(this);
    m_chargeSpellsInUse = false;
    //	m_spellsbusy=false;
    m_interruptedRegenTime = 0;
    m_hitfrommeleespell = 0;
    m_damageSplitTarget = NULL;
    m_extrastriketarget = false;
    m_extrastriketargetc = 0;
    trigger_on_stun = 0;
    trigger_on_stun_chance = 100;
    trigger_on_stun_victim = 0;
    trigger_on_stun_chance_victim = 100;
    trigger_on_chill = 0;
    trigger_on_chill_chance = 100;
    trigger_on_chill_victim = 0;
    trigger_on_chill_chance_victim = 100;
    m_soulSiphon.amt = 0;
    m_soulSiphon.max = 0;
    m_modelhalfsize = 1.0f; //worst case unit size. (Should be overwritten)

    m_blockfromspell = 0;
    m_dodgefromspell = 0;
    m_parryfromspell = 0;
    m_BlockModPct = 0;

    m_damageShields.clear();
    m_reflectSpellSchool.clear();
    m_procSpells.clear();
    m_chargeSpells.clear();
    m_chargeSpellRemoveQueue.clear();
    tmpAura.clear();
    m_extraStrikeTargets.clear();

    asc_frozen = 0;
    asc_enraged = 0;
    asc_seal = 0;
    asc_bleed = 0;

    Tagged = false;
    TaggerGuid = 0;

    m_singleTargetAura.clear();

    vehicle = NULL;
    currentvehicle = NULL;

    m_noFallDamage = false;
    z_axisposition = 0.0f;
    m_safeFall = 0;
    detectRange = 0.0f;
    m_cTimer = 0;
    m_temp_summon = false;
    m_meleespell_ecn = 0;
    m_manaShieldId = 0;
    m_charmtemp = 0;
    m_auraRaidUpdateMask = 0;
}

Unit::~Unit()
{
    //start to remove badptrs, if you delete from the heap null the ptr's damn!
    RemoveAllAuras();

    if (SM_CriticalChance != NULL)
    {
        delete[] SM_CriticalChance;
        SM_CriticalChance = NULL;
    }

    if (SM_FDur != NULL)
    {
        delete[] SM_FDur;
        SM_FDur = NULL;
    }

    if (SM_PDur != NULL)
    {
        delete[] SM_PDur;
        SM_PDur = NULL;
    }

    if (SM_FRadius != NULL)
    {
        delete[] SM_FRadius;
        SM_FRadius = NULL;
    }

    if (SM_FRange != NULL)
    {
        delete[] SM_FRange;
        SM_FRange = NULL;
    }

    if (SM_PCastTime != NULL)
    {
        delete[] SM_PCastTime;
        SM_PCastTime = NULL;
    }

    if (SM_FCastTime != NULL)
    {
        delete[] SM_FCastTime;
        SM_FCastTime = NULL;
    }

    if (SM_PCriticalDamage != NULL)
    {
        delete[] SM_PCriticalDamage;
        SM_PCriticalDamage = NULL;
    }

    if (SM_FDOT != NULL)
    {
        delete[] SM_FDOT;
        SM_FDOT = NULL;
    }

    if (SM_PDOT != NULL)
    {
        delete[] SM_PDOT;
        SM_PDOT = NULL;
    }

    if (SM_FEffect1_Bonus != NULL)
    {
        delete[] SM_FEffect1_Bonus;
        SM_FEffect1_Bonus = NULL;
    }

    if (SM_PEffect1_Bonus != NULL)
    {
        delete[] SM_PEffect1_Bonus;
        SM_PEffect1_Bonus = NULL;
    }

    if (SM_FEffect2_Bonus != NULL)
    {
        delete[] SM_FEffect2_Bonus;
        SM_FEffect2_Bonus = NULL;
    }

    if (SM_PEffect2_Bonus != NULL)
    {
        delete[] SM_PEffect2_Bonus;
        SM_PEffect2_Bonus = NULL;
    }

    if (SM_FEffect3_Bonus != NULL)
    {
        delete[] SM_FEffect3_Bonus;
        SM_FEffect3_Bonus = NULL;
    }

    if (SM_PEffect3_Bonus != NULL)
    {
        delete[] SM_PEffect3_Bonus;
        SM_PEffect3_Bonus = NULL;
    }

    if (SM_PEffectBonus != NULL)
    {
        delete[] SM_PEffectBonus;
        SM_PEffectBonus = NULL;
    }

    if (SM_FEffectBonus != NULL)
    {
        delete[] SM_FEffectBonus;
        SM_FEffectBonus = NULL;
    }

    if (SM_FDamageBonus != NULL)
    {
        delete[] SM_FDamageBonus;
        SM_FDamageBonus = NULL;
    }

    if (SM_PDamageBonus != NULL)
    {
        delete[] SM_PDamageBonus;
        SM_PDamageBonus = NULL;
    }

    if (SM_PMiscEffect != NULL)
    {
        delete[] SM_PMiscEffect;
        SM_PMiscEffect = NULL;
    }

    if (SM_FMiscEffect != NULL)
    {
        delete[] SM_FMiscEffect;
        SM_FMiscEffect = NULL;
    }

    if (SM_FHitchance != NULL)
    {
        delete[] SM_FHitchance;
        SM_FHitchance = NULL;
    }

    if (SM_PRange != NULL)
    {
        delete[] SM_PRange;
        SM_PRange = NULL;
    }

    if (SM_PRadius != NULL)
    {
        delete[] SM_PRadius;
        SM_PRadius = NULL;
    }

    if (SM_PCost != NULL)
    {
        delete[] SM_PCost;
        SM_PCost = NULL;
    }

    if (SM_FCost != NULL)
    {
        delete[] SM_FCost;
        SM_FCost = NULL;
    }

    if (SM_FAdditionalTargets != NULL)
    {
        delete[] SM_FAdditionalTargets;
        SM_FAdditionalTargets = NULL;
    }

    if (SM_PJumpReduce != NULL)
    {
        delete[] SM_PJumpReduce;
        SM_PJumpReduce = NULL;
    }

    if (SM_FGlobalCooldown != NULL)
    {
        delete[] SM_FGlobalCooldown;
        SM_FGlobalCooldown = NULL;
    }

    if (SM_PGlobalCooldown != NULL)
    {
        delete[] SM_PGlobalCooldown;
        SM_PGlobalCooldown = NULL;
    }

    if (SM_PNonInterrupt != NULL)
    {
        delete[] SM_PNonInterrupt;
        SM_PNonInterrupt = NULL;
    }

    if (SM_FPenalty != NULL)
    {
        delete[] SM_FPenalty;
        SM_FPenalty = NULL;
    }

    if (SM_PPenalty != NULL)
    {
        delete[] SM_PPenalty;
        SM_PPenalty = NULL;
    }

    if (SM_FCooldownTime != NULL)
    {
        delete[] SM_FCooldownTime;
        SM_FCooldownTime = NULL;
    }

    if (SM_PCooldownTime != NULL)
    {
        delete[] SM_PCooldownTime;
        SM_PCooldownTime = NULL;
    }

    if (SM_FChanceOfSuccess != NULL)
    {
        delete[] SM_FChanceOfSuccess;
        SM_FChanceOfSuccess = NULL;
    }

    if (SM_FAmptitude != NULL)
    {
        delete[] SM_FAmptitude;
        SM_FAmptitude = NULL;
    }

    if (SM_PAmptitude != NULL)
    {
        delete[] SM_PAmptitude;
        SM_PAmptitude = NULL;
    }

    if (SM_FRezist_dispell != NULL)
    {
        delete[] SM_FRezist_dispell;
        SM_FRezist_dispell = NULL;
    }

    if (SM_PRezist_dispell != NULL)
    {
        delete[] SM_PRezist_dispell;
        SM_PRezist_dispell = NULL;
    }

    if (SM_FCharges != NULL)
    {
        delete[] SM_FCharges;
        SM_FCharges = NULL;
    }

    if (SM_PCharges != NULL)
    {
        delete[] SM_PCharges;
        SM_PCharges = NULL;
    }

    if (SM_FThreat != NULL)
    {
        delete[] SM_FThreat;
        SM_FThreat = NULL;
    }

    if (SM_PThreat != NULL)
    {
        delete[] SM_PThreat;
        SM_PThreat = NULL;
    }

    delete m_aiInterface;
    m_aiInterface = NULL;

    if (m_currentSpell)
    {
        m_currentSpell->cancel();
        m_currentSpell = NULL;
    }

    if (m_damageSplitTarget)
    {
        delete m_damageSplitTarget;
        m_damageSplitTarget = NULL;
    }

    // reflects not created by auras need to be deleted manually
    for (std::list<struct ReflectSpellSchool*>::iterator i = m_reflectSpellSchool.begin(); i != m_reflectSpellSchool.end(); ++i)
        delete *i;
    m_reflectSpellSchool.clear();

    for (std::list<ExtraStrike*>::iterator itx = m_extraStrikeTargets.begin(); itx != m_extraStrikeTargets.end(); ++itx)
    {
        ExtraStrike* es = *itx;
        LOG_ERROR("ExtraStrike added to Unit %u by Spell ID %u wasn't removed when removing the Aura", GetGUID(), es->spell_info->getId());
        delete es;
    }
    m_extraStrikeTargets.clear();

    // delete auras which did not get added to unit yet
    for (std::map<uint32, Aura*>::iterator i = tmpAura.begin(); i != tmpAura.end(); ++i)
        delete i->second;
    tmpAura.clear();

    for (std::list<SpellProc*>::iterator itr = m_procSpells.begin(); itr != m_procSpells.end(); ++itr)
        delete *itr;
    m_procSpells.clear();

    m_singleTargetAura.clear();

    RemoveGarbage();
}

void Unit::Update(unsigned long time_passed)
{
    _UpdateSpells(time_passed);

    RemoveGarbage();

    if (!IsDead())
    {
        //////////////////////////////////////////////////////////////////////////////////////////
        //POWER & HP REGENERATION
        if (this->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_DISABLE_REGEN))
            return;

        if (time_passed >= m_H_regenTimer)
            RegenerateHealth();
        else
            m_H_regenTimer -= static_cast<uint16>(time_passed);

        if (time_passed >= m_P_regenTimer)
        {
            RegeneratePower(false);
            m_interruptedRegenTime = 0;
        }
        else
        {
            m_P_regenTimer -= static_cast<uint16>(time_passed);
            if (m_interruptedRegenTime)
            {
                if (time_passed >= m_interruptedRegenTime)
                    RegeneratePower(true);
                else
                    m_interruptedRegenTime -= time_passed;
            }
        }

        if (m_aiInterface != NULL)
        {
            if (m_useAI)
                m_aiInterface->Update(time_passed);
            else if (!m_aiInterface->MoveDone())            //pending move
                m_aiInterface->UpdateMovementSpline();
        }

        if (m_diminishActive)
        {
            uint32 count = 0;
            for (uint32 x = 0; x < DIMINISHING_GROUP_COUNT; ++x)
            {
                // diminishing return stuff
                if (m_diminishTimer[x] && !m_diminishAuraCount[x])
                {
                    if (time_passed >= m_diminishTimer[x])
                    {
                        // resetting after 15 sec
                        m_diminishTimer[x] = 0;
                        m_diminishCount[x] = 0;
                    }
                    else
                    {
                        // reducing, still.
                        m_diminishTimer[x] -= static_cast<uint16>(time_passed);
                        ++count;
                    }
                }
            }
            if (!count)
                m_diminishActive = false;
        }

        /*		//if health changed since last time. Would be perfect if it would work for creatures too :)
                if (m_updateMask.GetBit(UNIT_FIELD_HEALTH))
                EventHealthChangeSinceLastUpdate();*/
    }
}

bool Unit::canReachWithAttack(Unit* pVictim)
{
    if (GetMapId() != pVictim->GetMapId())
        return false;

    //	float targetreach = pVictim->GetCombatReach();
    float selfreach;
    if (IsPlayer())
        selfreach = 5.0f; // minimum melee range, UNIT_FIELD_COMBATREACH is too small and used eg. in melee spells
    else
        selfreach = m_floatValues[UNIT_FIELD_COMBATREACH];

    float targetradius;
    //	targetradius = pVictim->m_floatValues[UNIT_FIELD_BOUNDINGRADIUS]; //this is plain wrong. Represents i have no idea what :)
    targetradius = pVictim->GetModelHalfSize();
    float selfradius;
    //	selfradius = m_floatValues[UNIT_FIELD_BOUNDINGRADIUS];
    selfradius = GetModelHalfSize();
    //	float targetscale = pVictim->m_floatValues[OBJECT_FIELD_SCALE_X];
    //	float selfscale = m_floatValues[OBJECT_FIELD_SCALE_X];

    //float distance = sqrt(getDistanceSq(pVictim));
    float delta_x = pVictim->GetPositionX() - GetPositionX();
    float delta_y = pVictim->GetPositionY() - GetPositionY();
    float distance = sqrt(delta_x * delta_x + delta_y * delta_y);


    //	float attackreach = (((targetradius*targetscale) + selfreach) + (((selfradius*selfradius)*selfscale)+1.50f));
    float attackreach = targetradius + selfreach + selfradius;

    //formula adjustment for player side.
    if (IsPlayer())
    {
        // latency compensation!!
        // figure out how much extra distance we need to allow for based on our movespeed and latency.
        if (pVictim->IsPlayer() && static_cast<Player*>(pVictim)->m_isMoving)
        {
            // this only applies to PvP.
            uint32 lat = static_cast<Player*>(pVictim)->GetSession() ? static_cast<Player*>(pVictim)->GetSession()->GetLatency() : 0;

            // if we're over 500 get fucked anyway.. your gonna lag! and this stops cheaters too
            lat = (lat > 500) ? 500 : lat;

            // calculate the added distance
            attackreach += m_currentSpeedRun * 0.001f * lat;
        }

        if (static_cast<Player*>(this)->m_isMoving)
        {
            // this only applies to PvP.
            uint32 lat = static_cast<Player*>(this)->GetSession() ? static_cast<Player*>(this)->GetSession()->GetLatency() : 0;

            // if we're over 500 get fucked anyway.. your gonna lag! and this stops cheaters too
            lat = (lat > 500) ? 500 : lat;

            // calculate the added distance
            attackreach += m_currentSpeedRun * 0.001f * lat;
        }
    }
    return (distance <= attackreach);
}

void Unit::GiveGroupXP(Unit* pVictim, Player* PlayerInGroup)
{
    if (!PlayerInGroup)
        return;

    if (!pVictim)
        return;

    if (!PlayerInGroup->InGroup())
        return;

    Group* pGroup = PlayerInGroup->GetGroup();
    uint32 xp;
    if (!pGroup)
        return;

    //Get Highest Level Player, Calc Xp and give it to each group member
    Player* pHighLvlPlayer = NULL;
    Player* pGroupGuy = NULL;
    int active_player_count = 0;
    Player* active_player_list[MAX_GROUP_SIZE_RAID];            //since group is small we can afford to do this ratehr then recheck again the whole active player set
    int total_level = 0;
    float xp_mod = 1.0f;

    GroupMembersSet::iterator itr;
    pGroup->Lock();
    for (uint32 i = 0; i < pGroup->GetSubGroupCount(); i++)
    {
        for (itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); ++itr)
        {
            pGroupGuy = (*itr)->m_loggedInPlayer;
            if (pGroupGuy && pGroupGuy->isAlive() && /* PlayerInGroup->GetInstanceID()==pGroupGuy->GetInstanceID() &&*/
                pVictim->GetMapMgr() == pGroupGuy->GetMapMgr() && pGroupGuy->getDistanceSq(pVictim) < 100 * 100)
            {
                active_player_list[active_player_count] = pGroupGuy;
                active_player_count++;
                total_level += pGroupGuy->getLevel();

                if (pHighLvlPlayer)
                {
                    if (pGroupGuy->getLevel() > pHighLvlPlayer->getLevel())
                        pHighLvlPlayer = pGroupGuy;
                }
                else
                    pHighLvlPlayer = pGroupGuy;
            }
        }
    }
    pGroup->Unlock();
    if (active_player_count < 1) //killer is always close to the victim. This should never execute
    {
        /*if (PlayerInGroup == 0) This cannot be true CID 52876
        {
            PlayerInfo* pleaderinfo = pGroup->GetLeader();
            if (!pleaderinfo->m_loggedInPlayer)
                return;

            PlayerInGroup = pleaderinfo->m_loggedInPlayer;
        }*/

        xp = CalculateXpToGive(pVictim, PlayerInGroup);
        PlayerInGroup->GiveXP(xp, pVictim->GetGUID(), true);
    }
    else
    {
        if (pGroup->GetGroupType() == GROUP_TYPE_PARTY)
        {
            if (active_player_count == 3)
                xp_mod = 1.1666f;
            else if (active_player_count == 4)
                xp_mod = 1.3f;
            else if (active_player_count == 5)
                xp_mod = 1.4f;
            else xp_mod = 1; //in case we have only 2 members ;)
        }
        else if (pGroup->GetGroupType() == GROUP_TYPE_RAID)
            xp_mod = 0.5f;

        /*if (pHighLvlPlayer == 0) This cannot be true CID 52744
        {
            PlayerInfo* pleaderinfo = pGroup->GetLeader();
            if (!pleaderinfo->m_loggedInPlayer)
                return;

            pHighLvlPlayer = pleaderinfo->m_loggedInPlayer;
        }*/

        xp = CalculateXpToGive(pVictim, pHighLvlPlayer);
        //I'm not sure about this formula is correct or not. Maybe some brackets are wrong placed ?
        for (int i = 0; i < active_player_count; i++)
        {
            Player* plr = active_player_list[i];
            plr->GiveXP(float2int32(((xp * plr->getLevel()) / total_level) * xp_mod), pVictim->GetGUID(), true);

            active_player_list[i]->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_LASTKILLWITHHONOR);
            if (!sEventMgr.HasEvent(active_player_list[i], EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
            {
                sEventMgr.AddEvent(static_cast<Unit*>(active_player_list[i]), &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_LASTKILLWITHHONOR, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }
            else
            {
                sEventMgr.ModifyEventTimeLeft(active_player_list[i], EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000);
            }

            if (plr->GetSummon() && plr->GetSummon()->CanGainXP())
            {
                uint32 pet_xp = (uint32)(CalculateXpToGive(pVictim, plr->GetSummon()) * xp_mod);   // vojta: this isn't blizzlike probably but i have no idea, feel free to correct it
                if (pet_xp> 0)
                    plr->GetSummon()->GiveXP(pet_xp);
            }
        }
    }
}

uint32 Unit::HandleProc(uint32 flag, Unit* victim, SpellInfo* CastingSpell, bool is_triggered, uint32 dmg, uint32 abs, uint32 weapon_damage_type)
{
    uint32 resisted_dmg = 0;
    ++m_procCounter;
    bool can_delete = !bProcInUse; //if this is a nested proc then we should have this set to TRUE by the father proc
    bProcInUse = true; //locking the proc list

    /* hmm what's a reasonable value here */
    if (m_procCounter > 40)
    {
        /* something has proceed over 10 times in a loop :/ dump the spellids to the crashlog, as the crashdump will most likely be useless. */
        // BURLEX FIX ME!
        //OutputCrashLogLine("HandleProc %u SpellId %u (%s) %u", flag, spellId, sSpellStore.LookupString(sSpellStore.LookupEntry(spellId)->Name), m_procCounter);
        return 0;
    }

    std::list<SpellProc*>::iterator itr, itr2;
    for (itr = m_procSpells.begin(); itr != m_procSpells.end();)    // Proc Trigger Spells for Victim
    {
        itr2 = itr;
        ++itr;

        SpellProc* spell_proc = *itr2;

        // Check if list item was deleted elsewhere, so here it's removed and freed
        if (spell_proc->mDeleted)
        {
            if (can_delete)
            {
                m_procSpells.erase(itr2);
                delete spell_proc;
            }
            continue;
        }

        if (CastingSpell != NULL)
        {
            // A spell cannot proc itself
            if (CastingSpell->getId() == spell_proc->mSpell->getId())
                continue;

            // If this is called by a triggered spell, check if it's allowed
            if (is_triggered && !spell_proc->CanProcOnTriggered(victim, CastingSpell))
                continue;
        }

        // Check if this can proc
        if (!spell_proc->CanProc(victim, CastingSpell))
            continue;

        // Check for flags
        if (!spell_proc->CheckProcFlags(flag))
            continue;

        // Check proc class mask
        if (flag & PROC_ON_CAST_SPELL && CastingSpell && !spell_proc->CheckClassMask(victim, CastingSpell))
            continue;

        uint32 spellId = spell_proc->mSpell->getId();

        SpellInfo* spe = spell_proc->mSpell;

        uint32 origId;
        if (spell_proc->mOrigSpell != NULL)
            origId = spell_proc->mOrigSpell->getId();
        else
            origId = 0;
        SpellInfo* ospinfo = sSpellCustomizations.GetSpellInfo(origId);  //no need to check if exists or not since we were not able to register this trigger if it would not exist :P

        //this requires some specific spell check,not yet implemented
        //this sucks and should be rewrote
        if (spell_proc->mProcFlags & PROC_ON_CAST_SPECIFIC_SPELL)
        {
            if (CastingSpell == NULL)
                continue;

            //this is wrong, dummy is too common to be based on this, we should use spellgroup or something
            if (spe->getSpellIconID() != CastingSpell->getSpellIconID())
            {
                if (ospinfo && !ospinfo->getSchool())
                    continue;
                if (ospinfo && ospinfo->getSchool() != CastingSpell->getSchool())
                    continue;
                if (CastingSpell->getEffectImplicitTargetA(0) == EFF_TARGET_SELF || CastingSpell->getEffectImplicitTargetA(1) == EFF_TARGET_SELF || CastingSpell->getEffectImplicitTargetA(2) == EFF_TARGET_SELF)  //Prevents school based procs affecting caster when self buffing
                    continue;
            }
            else if (spe->getSpellIconID() == 1)
                continue;
        }

        uint32 proc_Chance = spell_proc->CalcProcChance(victim, CastingSpell);

        //Custom procchance modifications based on equipped weapon speed.
        if (this->IsPlayer())
        {
            float ppm = 1.0f;

            switch (spellId)
            {
                //SPELL_HASH_BLACK_TEMPLE_MELEE_TRINKET
                case 40475:
                    ppm = 1.0f;
                    break;

                // SPELL_HASH_MAGTHERIDON_MELEE_TRINKET:
                case 34774:
                    ppm = 1.5f;
                    break;                          // dragonspine trophy

                // SPELL_HASH_ROMULO_S_POISON:
                case 34586:
                case 34587:
                    ppm = 1.5f;
                    break;                          // romulo's

                // SPELL_HASH_FROSTBRAND_ATTACK:
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
                    ppm = 9.0f;
                    break;                          // Frostbrand Weapon

                case 16870:
                    ppm = 2.0f;
                    break; //druid: clearcasting
                default:
                    break;
            }

            Item* mh = static_cast<Player*>(this)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
            Item* of = static_cast<Player*>(this)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);

            if (mh != nullptr && of != nullptr)
            {
                uint32 mhs = mh->GetItemProperties()->Delay;
                uint32 ohs = of->GetItemProperties()->Delay;
                proc_Chance = float2int32((mhs + ohs) * 0.001f * ppm / 0.6f);
            }
            else if (mh != nullptr)
            {
                uint32 mhs = mh->GetItemProperties()->Delay;
                proc_Chance = float2int32(mhs * 0.001f * ppm / 0.6f);
            }
            else
                proc_Chance = 0;

            if (static_cast<Player*>(this)->IsInFeralForm())
            {
                if (static_cast<Player*>(this)->GetShapeShift() == FORM_CAT)
                {
                    proc_Chance = float2int32(ppm / 0.6f);
                }
                else if (static_cast<Player*>(this)->GetShapeShift() == FORM_BEAR || static_cast<Player*>(this)->GetShapeShift() == FORM_DIREBEAR)
                {
                    proc_Chance = float2int32(ppm / 0.24f);
                }
            }
        }

        spellModFlatIntValue(SM_FChanceOfSuccess, (int32*)&proc_Chance, ospinfo->getSpellGroupType());
        if (!Rand(proc_Chance))
            continue;

        //check if we can trigger due to time limitation
        if (ospinfo && ospinfo->custom_proc_interval)
        {
            uint32 now_in_ms = Util::getMSTime();
            if (spell_proc->mLastTrigger + ospinfo->custom_proc_interval > now_in_ms)
                continue; //we can't trigger it yet.
            spell_proc->mLastTrigger = now_in_ms; // consider it triggered
        }

        //since we did not allow to remove auras like these with interrupt flag we have to remove them manually.
        if (spell_proc->mProcFlags & PROC_REMOVEONUSE)
            RemoveAura(origId);

        int dmg_overwrite[3] = { 0, 0, 0 };

        // SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE
        for (uint8 i = 0; i < 3; ++i)
        {
            if (ospinfo && ospinfo->getEffectApplyAuraName(i) == SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE)
            {
                dmg_overwrite[i] = ospinfo->getEffectBasePoints(i) + 1;
                spell_proc->DoEffect(victim, CastingSpell, flag, dmg, abs, dmg_overwrite, weapon_damage_type);
            }
        }

        // give spell_proc a chance to handle the effect
        if (spell_proc->DoEffect(victim, CastingSpell, flag, dmg, abs, dmg_overwrite, weapon_damage_type))
            continue;

        //these are player talents. Fuckem they pull the emu speed down
        if (IsPlayer())
        {
            uint32 talentlevel = 0;
            switch (origId)
            {
                //mace specialization
                case 12284:
                {talentlevel = 1; }
                break;
                case 12701:
                {talentlevel = 2; }
                break;
                case 12702:
                {talentlevel = 3; }
                break;
                case 12703:
                {talentlevel = 4; }
                break;
                case 12704:
                {talentlevel = 5; }
                break;

                //Unbridled Wrath
                case 12332:
                {talentlevel = 1; }
                break;
                case 12999:
                {talentlevel = 2; }
                break;
                case 13000:
                {talentlevel = 3; }
                break;
                case 13001:
                {talentlevel = 4; }
                break;
                case 13002:
                {talentlevel = 5; }
                break;
            }

            switch (spellId)
            {
                case 32747:     //Deadly Throw Interrupt (rogue arena gloves set)
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        case 26679:
                        case 37074:
                        case 48673:
                        case 48674:
                        case 52885:
                        case 59180:
                        case 64499:
                            break;
                        default:
                            continue;
                    }
                }
                break;
                case 16959:     //Druid - Primal Fury Proc
                {
                    if (!IsPlayer())
                        continue;
                    Player* p = static_cast<Player*>(this);
                    if (p->GetShapeShift() != FORM_BEAR && p->GetShapeShift() != FORM_DIREBEAR)
                        continue;
                }
                break;
                case 16953:     //Druid - Blood Frenzy Proc
                {
                    if (!IsPlayer() || !CastingSpell)
                        continue;

                    Player* p = static_cast<Player*>(this);
                    if (p->GetShapeShift() != FORM_CAT)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_SHRED
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
                        //SPELL_HASH_RAVAGE
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
                        //SPELL_HASH_CLAW
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
                        //SPELL_HASH_RAKE
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                case 14189: //Seal Fate
                {
                    if (!this->IsPlayer() || !CastingSpell || CastingSpell->getId() == 14189 || CastingSpell->getId() == 16953 || CastingSpell->getId() == 16959)
                        continue;
                    if (CastingSpell->getEffect(0) != SPELL_EFFECT_ADD_COMBO_POINTS && CastingSpell->getEffect(1) != SPELL_EFFECT_ADD_COMBO_POINTS &&
                        CastingSpell->getEffect(2) != SPELL_EFFECT_ADD_COMBO_POINTS)
                    {
                        switch (CastingSpell->getId())
                        {
                            case 33876:
                            case 33982:
                            case 33983:
                            case 48565:
                            case 48566:
                                break;
                            default:
                                continue;
                        }
                    }
                }
                break;
                case 17106: //druid intensity
                {
                    if (CastingSpell == NULL)
                        continue;
                    if (CastingSpell->getId() != 5229)  //enrage
                        continue;
                }
                break;
                case 31616: //Nature's Guardian
                {
                    //yep, another special case: Nature's grace
                    if (GetHealthPct() > 30)
                        continue;
                }
                break;
                case 37309: //Bloodlust
                {
                    if (!this->IsPlayer())
                        continue;
                    if (static_cast<Player*>(this)->GetShapeShift() != FORM_BEAR ||
                        static_cast<Player*>(this)->GetShapeShift() != FORM_DIREBEAR)
                        continue;
                }
                break;
                case 37310://Bloodlust
                {
                    if (!this->IsPlayer() || static_cast<Player*>(this)->GetShapeShift() != FORM_CAT)
                        continue;
                }
                break;
                case 16459:
                {
                    //sword specialization
                    Item* item_mainhand = static_cast<Player*>(this)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    Item* item_offhand = static_cast<Player*>(this)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    uint32 reqskillMH = 0;
                    uint32 reqskillOH = 0;

                    if (item_mainhand != nullptr)
                        reqskillMH = GetSkillByProto(item_mainhand->GetItemProperties()->Class, item_mainhand->GetItemProperties()->SubClass);

                    if (item_offhand != nullptr)
                        reqskillOH = GetSkillByProto(item_offhand->GetItemProperties()->Class, item_offhand->GetItemProperties()->SubClass);

                    if (reqskillMH != SKILL_SWORDS && reqskillMH != SKILL_2H_SWORDS && reqskillOH != SKILL_SWORDS && reqskillOH != SKILL_2H_SWORDS)
                        continue;
                }
                break;
                case 12721:
                {
                    //deep wound requires a melee weapon
                    auto item = static_cast<Player*>(this)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    if (item)
                    {
                        //class 2 means weapons ;)
                        if (item->GetItemProperties()->Class != 2)
                            continue;
                    }
                    else continue; //no weapon no joy
                }
                break;
                //Warrior - Sword and Board
                case 50227:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_REVENGE
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
                        //SPELL_HASH_DEVASTATE
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
                            break;
                        default:
                            continue;
                    }
                } break;

                //Warrior - Safeguard
                case 46946:
                case 46947:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_INTERVENE
                        case 3411:
                        case 34784:
                        case 41198:
                        case 53476:
                        case 59667:
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //Warrior - Taste for Blood
                case 60503:
                {
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_REND
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //Warrior - Unbridled Wrath
                case 12964:
                {
                    //let's recalc chance to cast since we have a full 100 all time on this one
                    Item* it = static_cast<Player*>(this)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    if (it == nullptr)
                        continue; //no weapon no joy
                    //float chance=float(it->GetProto()->Delay)*float(talentlevel)/600.0f;
                    uint32 chance = it->GetItemProperties()->Delay * talentlevel / 300; //zack this had a very low proc rate. Kinda like a wasted talent
                    uint32 myroll = RandomUInt(100);
                    if (myroll > chance)
                        continue;
                }
                break;
                //Warrior - Gag Order
                case 18498:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_SHIELD_BASH
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
                        //SPELL_HASH_HEROIC_THROW
                        case 57755:
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //Warrior - Bloodsurge
                case 46916:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        case 23881:
                        //SPELL_HASH_WHIRLWIND
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
                        //SPELL_HASH_HEROIC_STRIKE
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
                            break;
                        default:
                            continue;
                    }
                } break;

                ////////////////////////////////////////////////////////////////////////////
                // Mage ignite talent only for fire dmg
                case 12654:
                {
                    if (CastingSpell == NULL)
                        continue;
                    if (CastingSpell->getSchool() != SCHOOL_FIRE)
                        continue;
                    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);   //we already modified this spell on server loading so it must exist
                    auto spell_duration = sSpellDurationStore.LookupEntry(spellInfo->getDurationIndex());
                    uint32 tickcount = GetDuration(spell_duration) / spellInfo->getEffectAmplitude(0);

                    if (ospinfo)
                        dmg_overwrite[0] = ospinfo->getEffectBasePoints(0) * dmg / (100 * tickcount);
                }
                break;
                //druid - Primal Fury
                case 37116:
                case 37117:
                {
                    if (!this->IsPlayer())
                        continue;
                    Player* mPlayer = static_cast<Player*>(this);
                    if (!mPlayer->IsInFeralForm() ||
                        (mPlayer->GetShapeShift() != FORM_CAT &&
                        mPlayer->GetShapeShift() != FORM_BEAR &&
                        mPlayer->GetShapeShift() != FORM_DIREBEAR))
                        continue;
                }
                break;
                //rogue - blade twisting
                case 31125:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere

                    //only trigger effect for specified spells
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_SINISTER_STRIKE
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
                        //SPELL_HASH_SHIV
                        case 5938:
                        case 5940:
                        //SPELL_HASH_BACKSTAB
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
                        //SPELL_HASH_GOUGE
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //priest - Grace
                case 47930:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_PENANCE
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
                        //SPELL_HASH_FLASH_HEAL
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
                        //SPELL_HASH_GREATER_HEAL
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //warlock - Improved Shadow Bolt
                case 17794:
                case 17798:
                case 17797:
                case 17799:
                case 17800:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere

                    //only trigger effect for specified spells
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_SHADOW_BOLT
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
                            break;
                        default:
                            continue;
                    }
                } break;

                // warlock - Seed of Corruption
                case 27285:
                {
                    bool can_proc_now = false;
                    //if we proced on spell tick
                    if (flag & PROC_ON_SPELL_HIT_VICTIM)
                    {
                        if (!CastingSpell)
                            continue;

                        switch (CastingSpell->getId())
                        {
                            //SPELL_HASH_SEED_OF_CORRUPTION
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
                                break;
                            default:
                                continue;
                        }

                        //this spell builds up n time
                        spell_proc->mProcCharges += dmg;
                        if (ospinfo && (int32)spell_proc->mProcCharges >= ospinfo->getEffectBasePoints(1) &&  //if charge built up
                            dmg < this->getUInt32Value(UNIT_FIELD_HEALTH))    //if this is not a killer blow
                            can_proc_now = true;
                    }
                    else can_proc_now = true; //target died
                    if (can_proc_now == false)
                        continue;
                    Unit* new_caster = victim;
                    if (new_caster && new_caster->isAlive())
                    {
                        SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);   //we already modified this spell on server loading so it must exist
                        Spell* spell = sSpellFactoryMgr.NewSpell(new_caster, spellInfo, true, NULL);
                        SpellCastTargets targets;
                        targets.setDestination(GetPosition());
                        spell->prepare(&targets);
                    }
                    spell_proc->mDeleted = true;
                    continue;
                }
                break;
                // warlock - Improved Drain Soul
                case 18371:
                {
                    if (!CastingSpell)
                        continue;

                    //only trigger effect for specified spells
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_DRAIN_SOUL
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
                            break;
                        default:
                            continue;
                    }

                    //null check was made before like 2 times already :P
                    if (ospinfo)
                        dmg_overwrite[0] = (ospinfo->getEffectBasePoints(2) + 1) * GetMaxPower(POWER_TYPE_MANA) / 100;
                }
                break;
                // warlock - Unstable Affliction
                case 31117:
                {
                    //null check was made before like 2 times already :P
                    if (ospinfo)
                        dmg_overwrite[0] = (ospinfo->getEffectBasePoints(0) + 1) * 9;
                }
                break;

                //warlock - Nighfall
                case 17941:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere

                    //only trigger effect for specified spells
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_CORRUPTION
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
                        //SPELL_HASH_DRAIN_LIFE
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //warlock - Shadow Embrace
                case 32386:
                case 32388:
                case 32389:
                case 32390:
                case 32391:
                {
                    if (CastingSpell == NULL)
                        continue;
                    else
                    {
                        switch (CastingSpell->getId())
                        {
                            case 184:       //Corruption
                            //SPELL_HASH_CURSE_OF_AGONY
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
                            //SPELL_HASH_SIPHON_LIFE
                            case 35195:
                            case 41597:
                            case 63106:
                            case 63108:
                            //SPELL_HASH_SEED_OF_CORRUPTION
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
                                break;
                            default:
                                continue;
                        }
                    }
                }
                break;
                //warlock - Aftermath
                case 18118:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere

                    //only trigger effect for specified spells
                    auto skill_line_ability = objmgr.GetSpellSkill(CastingSpell->getId());
                    if (!skill_line_ability)
                        continue;

                    if (skill_line_ability->skilline != SKILL_DESTRUCTION)
                        continue;
                }
                break;
                //warlock - Nether Protection
                case 30300:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere

                    //only trigger effect for specified spells
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                        continue;
                    if (CastingSpell->getSchool() != SCHOOL_FIRE &&
                        CastingSpell->getSchool() != SCHOOL_SHADOW)
                        continue;
                }
                break;
                //warlock - Soul Leech - this whole spell should get rewritten. Uses bad formulas, bad trigger method, spell is rewritten ...
                case 30294:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere
                    //only trigger effect for specified spells
                    uint32 amount;

                    switch (CastingSpell->getId())
                    {
                        // SPELL_HASH_SHADOW_BOLT: //Shadow Bolt
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
                        // SPELL_HASH_SOUL_FIRE: //Soul Fire
                        case 6353:
                        case 17924:
                        case 27211:
                        case 30545:
                        case 47824:
                        case 47825:
                        // SPELL_HASH_INCINERATE: //Incinerate
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
                        // SPELL_HASH_SEARING_PAIN: //Searing Pain
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
                        // SPELL_HASH_CONFLAGRATE: //Conflagrate
                        case 17962:
                        // SPELL_HASH_CHAOS_BOLT: //Chaos Bolt
                        case 50796:
                        case 51287:
                        case 59170:
                        case 59171:
                        case 59172:
                        case 69576:
                        case 71108:
                        {
                            amount = CastingSpell->getEffectBasePoints(0) + 1;
                        } break;

                        //SPELL_HASH_SHADOWBURN
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
                            amount = CastingSpell->getEffectBasePoints(1) + 1;
                        } break;
                        default:
                            amount = 0;

                    }

                    if (!amount)
                        continue;

                    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
                    Spell* spell = sSpellFactoryMgr.NewSpell(this, spellInfo, true, NULL);
                    spell->SetUnitTarget(this);
                    if (ospinfo)
                        spell->Heal(amount * (ospinfo->getEffectBasePoints(0) + 1) / 100);
                    delete spell;
                    spell = NULL;
                    continue;
                }
                break;
                //warlock - pyroclasm
                case 18093:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_RAIN_OF_FIRE
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
                        //SPELL_HASH_HELLFIRE_EFFECT
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
                        //SPELL_HASH_SOUL_FIRE
                        case 6353:
                        case 17924:
                        case 27211:
                        case 30545:
                        case 47824:
                        case 47825:
                            break;
                        default:
                            continue;
                    }
                }
                break;
                case 54274:
                case 54276:
                case 54277:
                {
                    if (CastingSpell == NULL)
                        continue;

                    if (CastingSpell->getId() != 17962)
                        continue;
                }
                break;
                //Mage - Missile Barrage
                case 44401:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_ARCANE_BLAST
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
                        //SPELL_HASH_ARCANE_BARRAGE
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
                        //SPELL_HASH_FIREBALL
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
                        //SPELL_HASH_FROSTBOLT
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
                        //SPELL_HASH_FROSTFIRE_BOLT
                        case 44614:
                        case 47610:
                        case 51779:
                        case 69869:
                        case 69984:
                        case 70616:
                        case 71130:
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //mage - Improved Scorch
                case 22959:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere

                    //only trigger effect for specified spells
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_SCORCH
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //mage - Combustion
                case 28682:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere
                    //only trigger effect for specified spells
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING) || CastingSpell->getSchool() != SCHOOL_FIRE)
                        continue;
                    if (flag & PROC_ON_SPELL_CRIT_HIT)
                    {
                        spell_proc->mProcCharges++;
                        if (spell_proc->mProcCharges >= 3)   //whatch that number cause it depends on original stack count !
                        {
                            uint32 combastion[] =
                            {
                                //SPELL_HASH_COMBUSTION
                                11129,
                                28682,
                                29977,
                                74630,
                                75882,
                                75883,
                                75884,
                                0
                            };

                            removeAllAurasById(combastion);
                            continue;
                        }
                    }
                }
                break;
                //mage - Winter's Chill
                case 12579:
                    // Winter's Chill shouldn't proc on self
                    if (victim == this || CastingSpell->getSchool() != SCHOOL_FROST)
                        continue;
                    break;
                    //item - Thunderfury
                case 21992:
                    if (victim == this)
                        continue;
                    break;
                    //warrior - Intimidating Shout
                case 5246:
                    if (victim == this)
                        continue;
                    break;

                    //priest - Borrowed time
                case 59887:
                case 59888:
                case 59889:
                case 59890:
                case 59891:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_POWER_WORD__SHIELD
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
                            break;
                        default:
                            continue;
                    }
                }
                break;

                //priest - Inspiration
                case 15363:
                case 14893:
                case 15357:
                case 15359:
                {
                    if (!CastingSpell || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))
                        continue;
                }
                break;
                //priest - Blessed Recovery
                case 27813:
                case 27817:
                case 27818:
                {
                    if (!IsPlayer() || !dmg)
                        continue;
                    SpellInfo* parentproc = sSpellCustomizations.GetSpellInfo(origId);
                    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);
                    if (!parentproc || !spellInfo)
                        continue;
                    int32 val = parentproc->getEffectBasePoints(0) + 1;
                    Spell* spell = sSpellFactoryMgr.NewSpell(this, spellInfo, true, NULL);
                    spell->forced_basepoints[0] = (val * dmg) / 300; //per tick
                    SpellCastTargets targets;
                    targets.m_unitTarget = GetGUID();
                    spell->prepare(&targets);
                    continue;
                }
                break;


                //Shaman - Healing Way
                case 29203:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere

                    //only trigger effect for specified spells
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_HEALING_WAVE
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //Shaman - Elemental Devastation
                case 29177:
                case 29178:
                case 30165:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere
                    //only trigger effect for specified spells
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))  //healing wave
                        continue;
                }
                break;
                //Shaman - Ancestral Fortitude
                case 16177:
                case 16236:
                case 16237:
                {
                    if (CastingSpell == NULL)
                        continue;

                    //Do not proc on Earth Shield crits
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_EARTH_SHIELD
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
                            continue;
                        default:
                            break;
                    }
                }
                //Shaman - Earthliving Weapon
                case 51940:
                case 51989:
                case 52004:
                case 52005:
                case 52007:
                case 52008:
                {
                    if (CastingSpell == NULL)
                        continue;

                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))   //healing spell
                        continue;
                }
                break;
                //Shaman - Tidal Waves
                case 51562:
                case 51563:
                case 51564:
                case 51565:
                case 51566:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_CHAIN_HEAL
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
                        //SPELL_HASH_RIPTIDE
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                // Totem of the Third Wind
                case 42371:
                case 34132:
                case 46099:
                case 43729:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_LESSER_HEALING_WAVE
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //Stonebreaker's Totem
                case 43749:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_EARTH_SHOCK
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
                        //SPELL_HASH_FLAME_SHOCK
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
                        //SPELL_HASH_FROST_SHOCK
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                // Librams of Justice
                case 34135:
                case 42369:
                case 43727:
                case 46093:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_FLASH_OF_LIGHT
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //Libram of Divine Judgement
                case 43747:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_JUDGEMENT_OF_COMMAND
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
                        //SPELL_HASH_JUDGEMENT
                        case 10321:
                        case 23590:
                        case 23591:
                        case 35170:
                        case 41467:
                        case 43838:
                        case 54158:
                            break;
                        default:
                            continue;
                    }
                }
                break;
                case 16246:
                {
                    switch (CastingSpell->getId())
                    {
                        // Lightning Overload Proc is already free
                        case 39805:
                        //SPELL_HASH_LIGHTNING_BOLT
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
                        //SPELL_HASH_CHAIN_LIGHTNING
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
                        //SPELL_HASH_EARTH_SHOCK
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
                        //SPELL_HASH_FLAME_SHOCK
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
                        //SPELL_HASH_FROST_SHOCK
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //shaman - windfury weapon
                case 8232:
                case 8235:
                case 10486:
                case 16362:
                case 25505:
                {
                    if (!IsPlayer())
                        continue;
                    //!! The weird thing is that we need the spell that triggered this enchant spell in order to output logs ..we are using oldspell info too
                    //we have to recalc the value of this spell
                    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(origId);
                    uint32 AP_owerride = spellInfo->getEffectBasePoints(0) + 1;
                    uint32 dmg2 = static_cast<Player*>(this)->GetMainMeleeDamage(AP_owerride);
                    SpellInfo* sp_for_the_logs = sSpellCustomizations.GetSpellInfo(spellId);
                    Strike(victim, MELEE, sp_for_the_logs, dmg2, 0, 0, true, false);
                    Strike(victim, MELEE, sp_for_the_logs, dmg2, 0, 0, true, false);
                    spellId = 33010; // WF animation
                }
                break;
                //rogue - Ruthlessness
                case 14157:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere
                    //we need a finishing move for this
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_FINISHING_MOVE) || victim == this)
                        continue;
                    //should fix issue with combo points
                    if (IsPlayer())
                    {
                        static_cast<Player*>(this)->m_spellcomboPoints++;
                        static_cast<Player*>(this)->UpdateComboPoints();
                    }
                }
                break;
                // rogue - T10 4P bonus
                case 70802:
                {
                    // The rogue bonus set of T10 requires a finishing move
                    if (!(CastingSpell && CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_FINISHING_MOVE))
                        continue;
                }
                break;
                //warrior - improved berserker rage
                case 23690:
                case 23691:
                {
                    if (!CastingSpell || CastingSpell->getId() != 18499)
                        continue;
                }
                break;
                //mage - Arcane Concentration
                case 12536:
                {
                    //requires damageing spell
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                        continue;
                }
                break;
                //mage - Improved Blizzard
                case 12484:
                case 12485:
                case 12486:
                {
                    if (CastingSpell == NULL)
                        continue;

                    if (victim == this)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_BLIZZARD
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //mage - Master of Elements
                case 29077:
                {
                    if (CastingSpell == NULL)
                        continue;
                    if (CastingSpell->getSchool() != SCHOOL_FIRE && CastingSpell->getSchool() != SCHOOL_FROST) //fire and frost critical's
                        continue;

                    if (ospinfo)
                        dmg_overwrite[0] = CastingSpell->getManaCost() * (ospinfo->getEffectBasePoints(0) + 1) / 100;
                }
                break;
                //Hunter - The Beast Within
                case 34471:
                {
                    if (CastingSpell == NULL)
                        continue;

                    if (CastingSpell->getId() != 19574)
                        continue;
                }
                //Hunter - Thrill of the Hunt
                case 34720:
                {
                    if (CastingSpell == NULL)
                        continue;
                    dmg_overwrite[0] = CastingSpell->getManaCost() * 40 / 100;
                }
                break;
                //priest - Reflective Shield
                case 33619:
                {
                    if (!abs)
                        continue;

                    //requires Power Word: Shield active
                    uint32 powerWordShield[] =
                    {
                        //SPELL_HASH_POWER_WORD__SHIELD
                        17,
                        592,
                        600,
                        3747,
                        6065,
                        6066,
                        10898,
                        10899,
                        10900,
                        10901,
                        11647,
                        11835,
                        11974,
                        17139,
                        20697,
                        22187,
                        25217,
                        25218,
                        27607,
                        29408,
                        32595,
                        35944,
                        36052,
                        41373,
                        44175,
                        44291,
                        46193,
                        48065,
                        48066,
                        66099,
                        68032,
                        68033,
                        68034,
                        71548,
                        71780,
                        71781,
                        0
                    };

                    int power_word_id = hasAurasWithId(powerWordShield);
                    if (!power_word_id)
                        power_word_id = 17;
                    //make a direct strike then exit rest of handler
                    if (ospinfo)
                    {
                        int tdmg = abs * (ospinfo->getEffectBasePoints(0) + 1) / 100;
                        //somehow we should make this not caused any threat (to be done)
                        SpellNonMeleeDamageLog(victim, power_word_id, tdmg, false, true);
                    }
                    continue;
                }
                break;
                //rogue - combat potency
                case 35542:
                case 35545:
                case 35546:
                case 35547:
                case 35548:
                {
                    if (!IsPlayer() || !dmg)
                        continue;
                    //this needs offhand weapon
                    Item* it = static_cast<Player*>(this)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    if (it == nullptr || it->GetItemProperties()->InventoryType != INVTYPE_WEAPON)
                        continue;
                }
                break;
                //paladin - Improved Lay on Hands
                case 20233:
                case 20236:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_LAY_ON_HANDS
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //paladin - Infusion of Light
                case 53672:
                case 54149:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_HOLY_SHOCK
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //paladin - Sacred Cleansing
                case 53659:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_CLEANSE
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //paladin - Judgements of the Pure
                case 53655:
                case 53656:
                case 53657:
                case 54152:
                case 54153:
                {
                    if (CastingSpell == NULL)
                        continue;
                    if (CastingSpell->getId() != 53408 && CastingSpell->getId() != 53407 && CastingSpell->getId() != 20271)
                        continue;
                }
                break;
                case 21183: //Paladin - Heart of the Crusader
                case 54498:
                case 54499:
                {
                    if (CastingSpell == NULL)
                        continue;
                    if (CastingSpell->getId() != 53408 && CastingSpell->getId() != 53407 && CastingSpell->getId() != 20271)
                        continue;
                }
                break;
                case 54203: //Paladin - Sheath of Light
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_FLASH_OF_LIGHT
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
                        //SPELL_HASH_HOLY_LIGHT
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
                            break;
                        default:
                            continue;
                    }

                    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(54203);
                    auto spell_duration = sSpellDurationStore.LookupEntry(spellInfo->getDurationIndex());
                    uint32 tickcount = GetDuration(spell_duration) / spellInfo->getEffectAmplitude(0);
                    if (ospinfo)
                        dmg_overwrite[0] = ospinfo->getEffectBasePoints(0) * dmg / (100 * tickcount);
                }
                break;
                case 59578: //Paladin - Art of War
                case 53489:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        case 53408:
                        case 53407:
                        case 20271:
                        //SPELL_HASH_CRUSADER_STRIKE
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
                        //SPELL_HASH_DIVINE_STORM
                        case 53385:
                        case 54171:
                        case 54172:
                        case 58127:
                        case 66006:
                            break;
                        default:
                            continue;
                    }
                }
                break;

                //////////////////////////////////////////
                // WARRIOR								//
                //////////////////////////////////////////

                // Warrior - Improved Revenge
                case 12798:
                {
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_REVENGE
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                // Warrior - Unrelenting Assault
                case 64849:
                case 64850:
                {
                    if (CastingSpell == NULL)
                        continue;
                    //trigger only on heal spell cast by NOT us
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING) || this == victim)
                        continue;
                    //this is not counting the bonus effects on heal
                    auto idx = CastingSpell->firstBeneficialEffect();
                    if (idx != 1)
                    {
                        if (ospinfo)
                            dmg_overwrite[0] = ((CastingSpell->getEffectBasePoints(idx) + 1) * (ospinfo->getEffectBasePoints(0) + 1) / 100);
                    }
                }
                break;
                //paladin - Light's Grace
                case 31834:
                {
                    if (CastingSpell == NULL)
                        continue;//this should not occur unless we made a fuckup somewhere

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_HOLY_LIGHT
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //paladin - Blessed Life
                case 31828:
                {
                    //we should test is damage is from environment or not :S
                    resisted_dmg = (dmg / 2);
                    continue; //there is no visual for this ?
                }
                break;
                //paladin - Judgements of the Wise
                case 54180:
                {
                    if (CastingSpell == NULL)
                        continue;
                    if (CastingSpell->getId() != 53408 && CastingSpell->getId() != 53407 && CastingSpell->getId() != 20271)
                        continue;
                    if (!IsPlayer())
                        continue;
                }
                break;
                case 54172: //Paladin - Divine Storm heal effect
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_DIVINE_STORM
                        case 53385:
                        case 54171:
                        case 54172:
                        case 58127:
                        case 66006:
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //Energized
                case 43751:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_LIGHTNING_BOLT
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
                            break;
                        default:
                            continue;
                    }

                }
                break;
                //Spell Haste Trinket
                //http://www.wowhead.com/?item=28190 scarab of the infinite circle
                case 33370:
                {
                    if (CastingSpell == NULL)
                        continue;
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                        continue;
                }
                break;
                case 60487: // Extract of Necromantic Power
                {
                    if (CastingSpell == NULL)
                        continue;
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                        continue;
                }
                break;
                case 33953: // The Egg of Mortal essence
                {
                    if (!CastingSpell)
                        continue;
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))
                        continue;
                }
                break;
                case 60529: // Forethough Talisman
                {
                    if (!CastingSpell)
                        continue;
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))
                        continue;
                }
                break;
                case 53390: //Tidal Waves
                {
                    if (!CastingSpell)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_CHAIN_HEAL
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
                        //SPELL_HASH_RIPTIDE
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //Earthliving
                case 51945:
                case 51990:
                case 51997:
                case 51998:
                case 51999:
                case 52000:
                {
                    if (!CastingSpell)
                        continue;
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))
                        continue;
                }
                break;
                //shaman - Lightning Overload
                case 39805:
                {
                    if (CastingSpell == NULL)
                        continue;                   //this should not occur unless we made a fuckup somewhere

                    //trigger on lightning and chain lightning. Spell should be identical , well maybe next time :P
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_LIGHTNING_BOLT
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
                        //SPELL_HASH_CHAIN_LIGHTNING
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
                            CastSpell(this, 39805, true);
                            spellId = CastingSpell->getId();
                            origId = 39805;
                        } break;
                        default:
                            continue;
                    }
                }
                break;
                //item - Band of the Eternal Sage
                case 35084:
                {
                    if (CastingSpell == NULL)
                        continue;
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))     //requires offensive spell. ! might not cover all spells
                        continue;
                }
                break;
                //druid - Earth and Moon
                case 60431:
                case 60432:
                case 60433:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_STARFIRE
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
                        //SPELL_HASH_WRATH
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                // druid - Celestial Focus
                case 16922:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_STARFIRE
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                case 37565: //setbonus
                {
                    if (!CastingSpell)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_FLASH_HEAL
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //SETBONUSES
                case 37379:
                {
                    if (!CastingSpell || CastingSpell->getSchool() != SCHOOL_SHADOW || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                        continue;
                }
                break;
                case 37378:
                {
                    if (!CastingSpell || CastingSpell->getSchool() != SCHOOL_FIRE || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                        continue;
                }
                break;
                case 45062: // Vial of the Sunwell
                case 39950:	// Wave Trance
                {
                    if (!CastingSpell || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))
                        continue;
                }
                break;
                case 37234:
                case 37214:
                case 37601:
                {
                    if (!CastingSpell || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                        continue;
                }
                break;
                case 37237:
                {
                    if (!CastingSpell)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_LIGHTNING_BOLT
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
                            break;
                        default:
                            continue;
                    }
                } break;
                //Tier 7 Warlock setbonus
                case 61082:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_LIFE_TAP
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                //Tier 5 Paladin setbonus - Crystalforge Battlegear or Crystalforge Raiment
                case 37196:
                case 43838:
                {
                    if (!CastingSpell)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        case 31804:
                        //SPELL_HASH_JUDGEMENT_OF_JUSTICE
                        case 20184:
                        case 53407:
                        //SPELL_HASH_JUDGEMENT_OF_LIGHT
                        case 20185:
                        case 20267:
                        case 20271:
                        case 28775:
                        case 57774:
                        //SPELL_HASH_JUDGEMENT_OF_WISDOM
                        case 20186:
                        case 20268:
                        case 53408:
                        //SPELL_HASH_JUDGEMENT_OF_RIGHTEOUSNESS
                        case 20187:
                        //SPELL_HASH_JUDGEMENT_OF_BLOOD
                        case 31898:
                        case 32220:
                        case 41461:
                        //SPELL_HASH_JUDGEMENT_OF_COMMAND
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                case 43837:
                {
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_FLASH_OF_LIGHT
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
                        //SPELL_HASH_HOLY_LIGHT
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                case 37529:
                {
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_OVERPOWER
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                case 37517:
                {
                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_REVENGE
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
                        case 40392:
                        case 57823:
                            break;
                        case 37517:
                        default:
                            continue;
                    }
                }
                break;
                case 38333: // Ribbon of Sacrifice
                {
                    if (!CastingSpell || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING))
                        continue;
                }
                //SETBONUSES END
                //http://www.wowhead.com/?item=32493 Ashtongue Talisman of Shadows
                case 40480:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_CORRUPTION
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
                            break;
                        default:
                            continue;
                    }
                }
                break;

                //http://www.wowhead.com/?item=32496  Memento of Tyrande
                case 37656: //don't say damaging spell but EACH time spell is cast there is a chance (so can be healing spell)
                {
                    if (CastingSpell == NULL)
                        continue;
                }
                break;
                //http://www.wowhead.com/?item=32488 Ashtongue Talisman of Insight
                case 40483:
                {
                    if (CastingSpell == NULL)
                        continue;
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                        continue;
                }
                break;

                //http://www.wowhead.com/?item=32487 Ashtongue Talisman of Swiftness
                case 40487:
                {
                    switch (CastingSpell->getId())
                    {
                        // SPELL_HASH_STEADY_SHOT:
                        case 34120:
                        case 49051:
                        case 49052:
                        case 56641:
                        case 65867:
                            break;
                        default:
                            continue;
                    }
                }
                break;

                //http://www.wowhead.com/?item=32485 Ashtongue Talisman of Valor
                case 40459:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->GetAreaAuraEffectId())
                    {
                        //SPELL_HASH_SHIELD_SLAM
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
                        //SPELL_HASH_BLOODTHIRST
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
                        //SPELL_HASH_MORTAL_STRIKE
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                case 28804://Epiphany :Each spell you cast can trigger an Epiphany, increasing your mana regeneration by 24 for 30 sec.
                {
                    if (!CastingSpell)
                        continue;
                }
                break;
                //SETBONUSES END
                //item - Band of the Eternal Restorer
                case 35087:
                {
                    if (CastingSpell == NULL)
                        continue;
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING)) //requires healing spell.
                        continue;
                }
                break;

                //http://www.wowhead.com/?item=32486 Ashtongue Talisman of Equilibrium
                case 40452: //Mangle has a 40% chance to grant 140 Strength for 8 sec
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        // SPELL_HASH_MANGLE__BEAR_
                        case 33878:
                        case 33986:
                        case 33987:
                        case 48563:
                        case 48564:
                        // SPELL_HASH_MANGLE__CAT_
                        case 33876:
                        case 33982:
                        case 33983:
                        case 48565:
                        case 48566:
                            break;
                        default:
                            continue;
                    }
                }
                break;
                case 40445: //Starfire has a 25% chance to grant up to 150 spell damage for 8 sec
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_STARFIRE
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                case 40446: //Rejuvenation has a 25% chance to grant up to 210 healing for 8 sec
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_REJUVENATION
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
                            break;
                        default:
                            continue;
                    }
                }
                break;

                //http://www.wowhead.com/?item=32490 Ashtongue Talisman of Acumen
                case 40441: //Each time your Shadow Word: Pain deals damage, it has a 10% chance to grant you 220 spell damage for 10 sec
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_SHADOW_WORD__PAIN
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
                            break;
                        default:
                            continue;
                    }
                }
                break;

                //http://www.wowhead.com/?item=32490 Ashtongue Talisman of Acumen
                case 40440: //Each time your Renew heals, it has a 10% chance to grant you 220 healing for 5 sec
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_RENEW
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
                            break;
                        default:
                            continue;
                    }
                }
                break;

                //http://www.wowhead.com/?item=32492 Ashtongue Talisman of Lethality
                case 37445: //using a mana gem grants you 225 spell damage for 15 sec
                {
                    if (!CastingSpell)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_REPLENISH_MANA
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
                case 16886: // druid - Nature's Grace
                {
                    // Remove aura if it exists so it gets reapplied
                    RemoveAura(16886);
                }
                break;
                case 38395:
                {
                    if (CastingSpell == NULL)
                        continue;

                    switch (CastingSpell->getId())
                    {
                        //SPELL_HASH_IMMOLATE
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
                        //SPELL_HASH_CORRUPTION
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
                            break;
                        default:
                            continue;
                    }
                }
                break;
            }
        }

        if (spellId == 17364 || spellId == 32175 || spellId == 32176)   //Stormstrike
            continue;
        if (spellId == 22858 && isInBack(victim))       //retatliation needs target to be not in front. Can be cast by creatures too
            continue;

        spell_proc->CastSpell(victim, CastingSpell, dmg_overwrite);

        if (origId == 39805)
        {
            RemoveAura(39805);          // Remove lightning overload aura after procing
        }
    }

    m_chargeSpellsInUse = true;
    std::map<uint32, struct SpellCharge>::iterator iter, iter2;
    iter = m_chargeSpells.begin();

    while (iter != m_chargeSpells.end())
    {
        iter2 = iter++;

        if (iter2->second.count)
        {
            if ((iter2->second.ProcFlag & flag))
            {
                //Fixes for spells that don't lose charges when dmg is absorbed
                if (iter2->second.ProcFlag == 680 && dmg == 0)
                    continue;

                if (CastingSpell)
                {
                    auto spell_cast_time = sSpellCastTimesStore.LookupEntry(CastingSpell->getCastingTimeIndex());
                    if (!spell_cast_time)
                        continue;

                    //if we did not proc these then we should not remove them
                    if (CastingSpell->getId() == iter2->second.spellId)
                        continue;

                    switch (iter2->second.spellId)
                    {
                        case 43339: // Shaman - Shamanist Focus
                        {
                            switch (CastingSpell->getId())
                            {
                                //SPELL_HASH_EARTH_SHOCK
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
                                //SPELL_HASH_FLAME_SHOCK
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
                                //SPELL_HASH_FROST_SHOCK
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
                                    break;
                                default:
                                    continue;
                            }
                        }
                        break;
                        case 12043: // Mage - Presence of Mind
                        {
                            //if (!sd->CastTime||sd->CastTime>10000) continue;
                            if (spell_cast_time->CastTime == 0)
                                continue;
                        }
                        break;
                        case 17116: // Shaman - Nature's Swiftness
                        case 16188:	// Druid - Nature's Swiftness
                        {
                            //if (CastingSpell->School!=SCHOOL_NATURE||(!sd->CastTime||sd->CastTime>10000)) continue;
                            if (CastingSpell->getSchool() != SCHOOL_NATURE || spell_cast_time->CastTime == 0)
                                continue;
                        }
                        break;
                        case 16166:
                        {
                            if (!(CastingSpell->getSchool() == SCHOOL_FIRE || CastingSpell->getSchool() == SCHOOL_FROST || CastingSpell->getSchool() == SCHOOL_NATURE))
                                continue;
                        }
                        break;
                        case 14177: // Cold blood will get removed on offensive spell
                        {
                            if (!(CastingSpell->getSpellGroupType(0) & 0x6820206 || CastingSpell->getSpellGroupType(1) & 0x240009))
                                continue;
                        }
                        break;
                        case 46916: // Bloodsurge - Slam! effect should dissapear after casting Slam only
                        {
                            switch (CastingSpell->getId())
                            {
                                //SPELL_HASH_SLAM
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
                                    break;
                                default:
                                    continue;
                            }

                        }
                        break;
                        case 60503: // Taste for Blood should dissapear after casting Overpower
                        {
                            switch (CastingSpell->getId())
                            {
                                //SPELL_HASH_OVERPOWER
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
                                    break;
                                default:
                                    continue;
                            }
                        }
                        break;
                        case 23694: // Imp. Hamstring
                        {
                            switch (CastingSpell->getId())
                            {
                                //SPELL_HASH_IMPROVED_HAMSTRING
                                case 12289:
                                case 12668:
                                case 23694:
                                case 23695:
                                case 24428:
                                    break;
                                default:
                                    continue;
                            }
                        }
                        break;
                        case 65156: // Juggernaut
                        {
                            switch (CastingSpell->getId())
                            {
                                //SPELL_HASH_MORTAL_STRIKE
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
                                //SPELL_HASH_SLAM
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
                                    break;
                                default:
                                    continue;
                            }
                        }
                        break;
                    }
                }
                if (iter2->second.lastproc != 0)
                {
                    if (iter2->second.procdiff > 3000)
                    {
                        //--(iter2->second.count);
                        RemoveAura(iter2->second.spellId);
                    }
                }
                else
                {
                    //--(iter2->second.count);		// done in Aura::Remove
                    this->RemoveAura(iter2->second.spellId);
                }
            }
        }
        if (!iter2->second.count)
        {
            m_chargeSpells.erase(iter2);
        }
    }

    for (; !m_chargeSpellRemoveQueue.empty();)
    {
        iter = m_chargeSpells.find(m_chargeSpellRemoveQueue.front());
        if (iter != m_chargeSpells.end())
        {
            if (iter->second.count > 1)
                --iter->second.count;
            else
                m_chargeSpells.erase(iter);
        }
        m_chargeSpellRemoveQueue.pop_front();
    }

    m_chargeSpellsInUse = false;
    if (can_delete)   //are we the upper level of nested procs ? If yes then we can remove the lock
        bProcInUse = false;

    return resisted_dmg;
}

//damage shield is a triggered spell by owner to atacker
void Unit::HandleProcDmgShield(uint32 flag, Unit* attacker)
{
    //make sure we do not loop dmg procs
    if (this == attacker || !attacker)
        return;
    if (m_damgeShieldsInUse)
        return;
    m_damgeShieldsInUse = true;
    //charges are already removed in handleproc
    WorldPacket data(24);
    std::list<DamageProc>::iterator i;
    std::list<DamageProc>::iterator i2;
    for (i = m_damageShields.begin(); i != m_damageShields.end();)    // Deal Damage to Attacker
    {
        i2 = i++; //we should not proc on proc.. not get here again.. not needed.Better safe then sorry.
        if ((flag & (*i2).m_flags))
        {
            if (PROC_MISC & (*i2).m_flags)
            {
                data.Initialize(SMSG_SPELLDAMAGESHIELD);
                data << this->GetGUID();
                data << attacker->GetGUID();
                data << (*i2).m_spellId;
                data << (*i2).m_damage;
                data << (1 << (*i2).m_school);
                SendMessageToSet(&data, true);
                this->DealDamage(attacker, (*i2).m_damage, 0, 0, (*i2).m_spellId);
            }
            else
            {
                SpellInfo*	ability = sSpellCustomizations.GetSpellInfo((*i2).m_spellId);
                this->Strike(attacker, RANGED, ability, 0, 0, (*i2).m_damage, true, true);  //can dmg shields miss at all ?
            }
        }
    }
    m_damgeShieldsInUse = false;
}

bool Unit::IsCasting()
{
    return (m_currentSpell != NULL);
}

bool Unit::IsInInstance()
{
    MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(this->GetMapId());
    if (pMapinfo)
        return (pMapinfo->type != INSTANCE_NULL);

    return false;
}

void Unit::RegenerateHealth()
{
    m_H_regenTimer = 2000;      //set next regen time

    if (!isAlive())
        return;

    // player regen
    if (this->IsPlayer())
    {
        // These only NOT in combat
        if (!CombatStatus.IsInCombat())
            static_cast<Player*>(this)->RegenerateHealth(false);
        else
            static_cast<Player*>(this)->RegenerateHealth(true);
    }
    else
    {
        // Only regen health out of combat
        if (!CombatStatus.IsInCombat())
            static_cast<Creature*>(this)->RegenerateHealth();
    }
}

void Unit::RegeneratePower(bool isinterrupted)
{
    // This is only 2000 IF the power is not rage
    if (isinterrupted)
        m_interruptedRegenTime = 2000;
    else
        m_P_regenTimer = 2000;      //set next regen time

    if (!isAlive())
        return;

    if (!IsPlayer() && IsVehicle())
    {
        uint32 powertype = GetPowerType();
        float wrate = worldConfig.getFloatRate(RATE_VEHICLES_POWER_REGEN);
        float amount = wrate * 20.0f;
        SetPower(powertype, static_cast<int32>(GetPower(powertype) + amount));
    }

    //druids regen every tick, which is every 100ms, at one energy, as of 3.0.2
    //I don't know how mana has changed exactly, but it has, will research it - optical
    if (IsPlayer() && GetPowerType() == POWER_TYPE_ENERGY)
    {
        static_cast<Player*>(this)->RegenerateEnergy();
        // druids regen mana when shapeshifted
        if (getClass() == DRUID)
        {
            static_cast<Player*>(this)->RegenerateMana(isinterrupted);
        }
        return;
    }

    // player regen
    if (this->IsPlayer())
    {
        uint32 powertype = GetPowerType();
        switch (powertype)
        {
            case POWER_TYPE_MANA:
                static_cast<Player*>(this)->RegenerateMana(isinterrupted);
                break;

            case POWER_TYPE_RAGE:
            {
                // These only NOT in combat
                if (!CombatStatus.IsInCombat())
                {
                    m_P_regenTimer = 3000;
                    if (HasAura(12296))
                    {
                        static_cast<Player*>(this)->LooseRage(20);
                    }
                    else
                        static_cast<Player*>(this)->LooseRage(30);
                }
                else
                {
                    if (HasAura(12296))
                    {
                        m_P_regenTimer = 3000;
                        static_cast<Player*>(this)->LooseRage(-10);
                    }
                }

            }
            break;

            case POWER_TYPE_FOCUS:
            {
                m_P_regenTimer = 350; // This seems to be the exact Blizzlike timer
                uint32 cur = GetPower(POWER_TYPE_FOCUS);
                uint32 mm = GetMaxPower(POWER_TYPE_FOCUS);
                if (cur >= mm)
                    return;
                cur += 2;
                SetPower(POWER_TYPE_FOCUS, (cur >= mm) ? mm : cur);
            }
            break;

            case POWER_TYPE_RUNIC_POWER:
            {
                if (!CombatStatus.IsInCombat())
                {
#if VERSION_STRING == WotLK
                    uint32 cur = getUInt32Value(UNIT_FIELD_POWER7);
                    SetPower(POWER_TYPE_RUNIC_POWER, cur - 20);
#endif
                }
            }
            break;
        }

        /*

        There is a problem here for druids.
        Druids when shapeshifted into bear have 2 power with different regen timers
        a) Mana (which regenerates while shapeshifted
        b) Rage

        Mana has a regen timer of 2 seconds
        Rage has a regen timer of 3 seconds

        I think the only viable way of fixing this is to have 2 different timers
        to check each individual power.

        Atm, mana is being regen at 3 seconds while shapeshifted...

        */


        // druids regen mana when shapeshifted
        if (getClass() == DRUID && powertype != POWER_TYPE_MANA)
            static_cast<Player*>(this)->RegenerateMana(isinterrupted);
    }
    else
    {
        uint32 powertype = GetPowerType();
        switch (powertype)
        {
            case POWER_TYPE_MANA:
                static_cast<Creature*>(this)->RegenerateMana();
                break;
            case POWER_TYPE_FOCUS:
                static_cast<Creature*>(this)->RegenerateFocus();
                m_P_regenTimer = 4000;
                break;
        }
    }
}

void Unit::CalculateResistanceReduction(Unit* pVictim, dealdamage* dmg, SpellInfo* ability, float ArmorPctReduce)
{
    float AverageResistance = 0.0f;
    float ArmorReduce;

    if ((*dmg).school_type == 0)        //physical
    {
        if (this->IsPlayer())
            ArmorReduce = PowerCostPctMod[0] + ((float)pVictim->GetResistance(0) * (ArmorPctReduce + static_cast<Player*>(this)->CalcRating(PCR_ARMOR_PENETRATION_RATING)) / 100.0f);
        else
            ArmorReduce = 0.0f;

        if (ArmorReduce >= pVictim->GetResistance(0))		// fully penetrated :O
            return;

        //		double Reduction = double(pVictim->GetResistance(0)) / double(pVictim->GetResistance(0)+400+(85*getLevel()));
        //dmg reduction formula from xinef
        double Reduction = 0;
        if (getLevel() < 60)
            Reduction = double(pVictim->GetResistance(0) - ArmorReduce) / double(pVictim->GetResistance(0) + 400 + (85 * getLevel()));
        else if (getLevel() > 59 && getLevel() < DBC_PLAYER_LEVEL_CAP)
            Reduction = double(pVictim->GetResistance(0) - ArmorReduce) / double(pVictim->GetResistance(0) - 22167.5 + (467.5 * getLevel()));
        //
        else
            Reduction = double(pVictim->GetResistance(0) - ArmorReduce) / double(pVictim->GetResistance(0) + 10557.5);

        if (Reduction > 0.75f)
            Reduction = 0.75f;
        else if (Reduction < 0)
            Reduction = 0;

        if (Reduction)
            dmg[0].full_damage = (uint32)(dmg[0].full_damage * (1 - Reduction));	 // no multiply by 0
    }
    else
    {
        // applying resistance to other type of damage
        int32 RResist = float2int32((pVictim->GetResistance((*dmg).school_type) + ((pVictim->getLevel() > getLevel()) ? (pVictim->getLevel() - this->getLevel()) * 5 : 0)) - PowerCostPctMod[(*dmg).school_type]);
        if (RResist < 0)
            RResist = 0;
        AverageResistance = ((float)(RResist) / (float)(getLevel() * 5) * 0.75f);
        if (AverageResistance > 0.75f)
            AverageResistance = 0.75f;

        // NOT WOWWIKILIKE but i think it's actually to add some fullresist chance from resistances
        if (!ability || !(ability->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY))
        {
            float Resistchance = (float)pVictim->GetResistance((*dmg).school_type) / (float)pVictim->getLevel();
            Resistchance *= Resistchance;
            if (Rand(Resistchance))
                AverageResistance = 1.0f;
        }

        if (AverageResistance > 0)
            (*dmg).resisted_damage = (uint32)(((*dmg).full_damage) * AverageResistance);
        else
            (*dmg).resisted_damage = 0;
    }
}

uint32 Unit::GetSpellDidHitResult(Unit* pVictim, uint32 weapon_damage_type, SpellInfo* ability)
{
    Item* it = NULL;
    float hitchance = 0.0f;
    float dodge = 0.0f;
    float parry = 0.0f;
    float block = 0.0f;

    float hitmodifier = 0;
    int32 self_skill;
    int32 victim_skill;
    uint32 SubClassSkill = SKILL_UNARMED;

    bool backAttack = !pVictim->isInFront(this);   // isInBack is bugged!
    uint32 vskill = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    //Victim Skill Base Calculation
    if (pVictim->IsPlayer())
    {
        vskill = static_cast<Player*>(pVictim)->_GetSkillLineCurrent(SKILL_DEFENSE);
        if (weapon_damage_type != RANGED && !backAttack)                // block chance
        {
            block = pVictim->getFloatValue(PLAYER_BLOCK_PERCENTAGE);    //shield check already done in Update chances

            if (pVictim->m_stunned <= 0)                                // dodge chance
            {
                dodge = pVictim->getFloatValue(PLAYER_DODGE_PERCENTAGE);
            }

            if (pVictim->can_parry && !pVictim->disarmed)               // parry chance
            {
                if (static_cast<Player*>(pVictim)->HasSpell(3127) || static_cast<Player*>(pVictim)->HasSpell(18848))
                {
                    parry = pVictim->getFloatValue(PLAYER_PARRY_PERCENTAGE);
                }
            }
        }
        victim_skill = float2int32(vskill + static_cast<Player*>(pVictim)->CalcRating(PCR_DEFENCE));
    }
    else                                                                // mob defensive chances
    {
        if (weapon_damage_type != RANGED && !backAttack)
            dodge = pVictim->GetStat(STAT_AGILITY) / 14.5f;             // what is this value?
        victim_skill = pVictim->getLevel() * 5;

        if (pVictim->IsCreature())
        {
            Creature* c = static_cast<Creature*>(pVictim);
            if (c->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
            {
                victim_skill = std::max(victim_skill, ((int32)this->getLevel() + 3) * 5);       //used max to avoid situation when lowlvl hits boss.
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Attacker Skill Base Calculation
    if (this->IsPlayer())
    {
        self_skill = 0;
        Player* pr = static_cast<Player*>(this);
        hitmodifier = pr->GetHitFromMeleeSpell();

        switch (weapon_damage_type)
        {
            case MELEE:   // melee main hand weapon
                it = disarmed ? NULL : pr->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                hitmodifier += pr->CalcRating(PCR_MELEE_HIT);
                self_skill = float2int32(pr->CalcRating(PCR_MELEE_MAIN_HAND_SKILL));
                break;
            case OFFHAND: // melee offhand weapon (dualwield)
                it = disarmed ? NULL : pr->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                hitmodifier += pr->CalcRating(PCR_MELEE_HIT);
                self_skill = float2int32(pr->CalcRating(PCR_MELEE_OFF_HAND_SKILL));
                break;
            case RANGED:  // ranged weapon
                it = disarmed ? NULL : pr->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                hitmodifier += pr->CalcRating(PCR_RANGED_HIT);
                self_skill = float2int32(pr->CalcRating(PCR_RANGED_SKILL));
                break;
        }

        // erm. some spells don't use ranged weapon skill but are still a ranged spell and use melee stats instead
        // i.e. hammer of wrath
        if (ability)
        {
            switch (ability->getId())
            {
                //SPELL_HASH_HAMMER_OF_WRATH
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
                    it = pr->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    hitmodifier += pr->CalcRating(PCR_MELEE_HIT);
                    self_skill = float2int32(pr->CalcRating(PCR_MELEE_MAIN_HAND_SKILL));
                } break;
                default:
                    break;
            }
        }

        if (it)
            SubClassSkill = GetSkillByProto(it->GetItemProperties()->Class, it->GetItemProperties()->SubClass);
        else
            SubClassSkill = SKILL_UNARMED;

        if (SubClassSkill == SKILL_FIST_WEAPONS)
            SubClassSkill = SKILL_UNARMED;

        //chances in feral form don't depend on weapon skill
        if (static_cast<Player*>(this)->IsInFeralForm())
        {
            uint8 form = static_cast<Player*>(this)->GetShapeShift();
            if (form == FORM_CAT || form == FORM_BEAR || form == FORM_DIREBEAR)
            {
                SubClassSkill = SKILL_FERAL_COMBAT;
                self_skill += pr->getLevel() * 5;           // Adjust skill for Level * 5 for Feral Combat
            }
        }


        self_skill += pr->_GetSkillLineCurrent(SubClassSkill);
    }
    else
    {
        self_skill = this->getLevel() * 5;
        if (IsCreature())
        {
            Creature* c = static_cast<Creature*>(this);
            if (c->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
                self_skill = std::max(self_skill, ((int32)pVictim->getLevel() + 3) * 5);        //used max to avoid situation when lowlvl hits boss.
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Special Chances Base Calculation
    //<THE SHIT> to avoid Linux bug.
    float diffVcapped = (float)self_skill;
    if (int32(pVictim->getLevel() * 5) > victim_skill)
        diffVcapped -= (float)victim_skill;
    else
        diffVcapped -= (float)(pVictim->getLevel() * 5);

    float diffAcapped = (float)victim_skill;
    if (int32(this->getLevel() * 5) > self_skill)
        diffAcapped -= (float)self_skill;
    else
        diffAcapped -= (float)(getLevel() * 5);
    //<SHIT END>

    // by victim state
    if (pVictim->IsPlayer() && pVictim->GetStandState()) //every not standing state is>0
    {
        hitchance = 100.0f;
    }

    // by damage type and by weapon type
    if (weapon_damage_type == RANGED)
    {
        dodge = 0.0f;
        parry = 0.0f;
    }

    // by skill difference
    float vsk = (float)self_skill - (float)victim_skill;
    dodge = std::max(0.0f, dodge - vsk * 0.04f);

    if (parry)
        parry = std::max(0.0f, parry - vsk * 0.04f);

    if (block)
        block = std::max(0.0f, block - vsk * 0.04f);

    if (vsk > 0)
        hitchance = std::max(hitchance, 95.0f + vsk * 0.02f + hitmodifier);
    else
    {
        if (pVictim->IsPlayer())
            hitchance = std::max(hitchance, 95.0f + vsk * 0.1f + hitmodifier);      //wowwiki multiplier - 0.04 but i think 0.1 more balanced
        else
            hitchance = std::max(hitchance, 100.0f + vsk * 0.6f + hitmodifier);     //not wowwiki but more balanced
    }

    if (ability != nullptr)
    {
        spellModFlatFloatValue(SM_FHitchance, &hitchance, ability->getSpellGroupType());
    }

    if (ability && ability->getAttributes() & ATTRIBUTES_CANT_BE_DPB)
    {
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //One Roll Processing
    // cumulative chances generation
    float chances[4];
    chances[0] = std::max(0.0f, 100.0f - hitchance);
    chances[1] = chances[0] + dodge;
    chances[2] = chances[1] + parry;
    chances[3] = chances[2] + block;


    // roll
    float Roll = RandomFloat(100.0f);
    uint32 r = 0;

    while (r < 4 && Roll > chances[r])
    {
        r++;
    }

    uint32 roll_results[5] = { SPELL_DID_HIT_MISS, SPELL_DID_HIT_DODGE, SPELL_DID_HIT_PARRY, SPELL_DID_HIT_BLOCK, SPELL_DID_HIT_SUCCESS };
    return roll_results[r];
}

void Unit::Strike(Unit* pVictim, uint32 weapon_damage_type, SpellInfo* ability, int32 add_damage, int32 pct_dmg_mod, uint32 exclusive_damage, bool disable_proc, bool skip_hit_check, bool force_crit)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //Unacceptable Cases Processing
    if (!pVictim || !pVictim->isAlive() || !isAlive() || IsStunned() || IsPacified() || IsFeared())
        return;

    if (!(ability && ability->getAttributesEx() & ATTRIBUTESEX_IGNORE_IN_FRONT) && !isInFront(pVictim))
    {
        if (IsPlayer())
        {
            static_cast<Player*>(this)->GetSession()->OutPacket(SMSG_ATTACKSWING_BADFACING);
            return;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Variables Initialization
    dealdamage dmg = { 0, 0, 0 };

    Item* it = NULL;

    float hitchance = 0.0f;
    float dodge = 0.0f;
    float parry = 0.0f;
    float glanc = 0.0f;
    float block = 0.0f;
    float crit = 0.0f;
    float crush = 0.0f;

    uint32 targetEvent = 0;
    uint32 hit_status = 0;

    uint32 blocked_damage = 0;
    int32  realdamage = 0;

    uint32 vstate = 1;
    uint32 aproc = 0;
    uint32 vproc = 0;

    float hitmodifier = 0;
    float ArmorPctReduce = m_ignoreArmorPct;
    int32 self_skill;
    int32 victim_skill;
    uint32 SubClassSkill = SKILL_UNARMED;

    bool backAttack = !pVictim->isInFront(this);
    uint32 vskill = 0;
    bool disable_dR = false;

    if (ability)
        dmg.school_type = ability->getSchool();
    else
    {
        if (IsCreature())
            dmg.school_type = static_cast<Creature*>(this)->BaseAttackType;
        else
            dmg.school_type = SCHOOL_NORMAL;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Victim Skill Base Calculation
    if (pVictim->IsPlayer())
    {
        Player* plr = static_cast<Player*>(pVictim);
        vskill = plr->_GetSkillLineCurrent(SKILL_DEFENSE);

        if (!backAttack)
        {
            // not an attack from behind so we may dodge/parry/block

            //uint32 pClass = plr->getClass();
            //uint32 pLevel = (getLevel()> DBC_PLAYER_LEVEL_CAP) ? DBC_PLAYER_LEVEL_CAP : getLevel();

            if (weapon_damage_type != RANGED)
            {
                // cannot dodge/parry ranged attacks

                if (pVictim->m_stunned <= 0)
                {
                    // can dodge as long as we're not stunned
                    dodge = plr->GetDodgeChance();
                }

                if (pVictim->can_parry && !disarmed)
                {
                    // can parry as long as we're not disarmed
                    parry = plr->GetParryChance();
                }
            }
            // can block ranged attacks

            // Is an offhand equipped and is it a shield?
            Item* it2 = plr->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
            if (it2 != nullptr && it2->GetItemProperties()->InventoryType == INVTYPE_SHIELD)
            {
                block = plr->GetBlockChance();
            }
        }
        victim_skill = float2int32(vskill + floorf(plr->CalcRating(PCR_DEFENCE)));
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //mob defensive chances
    else
    {
        // not a player, must be a creature
        Creature* c = static_cast<Creature*>(pVictim);

        // mobs can dodge attacks from behind
        if (weapon_damage_type != RANGED && pVictim->m_stunned <= 0)
        {
            dodge = pVictim->getUInt32Value(UNIT_FIELD_STAT1) / 14.5f;
            dodge += pVictim->GetDodgeFromSpell();
        }

        if (!backAttack)
        {
            // can parry attacks from the front
            ///\todo different bosses have different parry rates (db patch?)
            if (!disarmed)    ///\todo this is wrong
            {
                parry = c->GetBaseParry();
                parry += pVictim->GetParryFromSpell();
            }

            ///\todo add shield check/block chance here how do we check what the creature has equipped?
        }

        victim_skill = pVictim->getLevel() * 5;
        if (pVictim->IsCreature())
        {
            if (c->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
            {
                victim_skill = std::max(victim_skill, ((int32)getLevel() + 3) * 5);     //used max to avoid situation when lowlvl hits boss.
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Attacker Skill Base Calculation
    if (this->IsPlayer())
    {
        self_skill = 0;
        Player* pr = static_cast<Player*>(this);
        hitmodifier = pr->GetHitFromMeleeSpell();

        switch (weapon_damage_type)
        {
            case MELEE:   // melee main hand weapon
                it = disarmed ? NULL : pr->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                self_skill = float2int32(pr->CalcRating(PCR_MELEE_MAIN_HAND_SKILL));
                if (it)
                {
                    dmg.school_type = it->GetItemProperties()->Damage[0].Type;
                    if (it->GetItemProperties()->SubClass == ITEM_SUBCLASS_WEAPON_MACE)
                        ArmorPctReduce += m_ignoreArmorPctMaceSpec;
                }
                break;
            case OFFHAND: // melee offhand weapon (dualwield)
                it = disarmed ? NULL : pr->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                self_skill = float2int32(pr->CalcRating(PCR_MELEE_OFF_HAND_SKILL));
                hit_status |= HITSTATUS_DUALWIELD;//animation
                if (it)
                {
                    dmg.school_type = it->GetItemProperties()->Damage[0].Type;
                    if (it->GetItemProperties()->SubClass == ITEM_SUBCLASS_WEAPON_MACE)
                        ArmorPctReduce += m_ignoreArmorPctMaceSpec;
                }
                break;
            case RANGED:  // ranged weapon
                it = disarmed ? NULL : pr->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                self_skill = float2int32(pr->CalcRating(PCR_RANGED_SKILL));
                if (it)
                    dmg.school_type = it->GetItemProperties()->Damage[0].Type;
                break;
        }

        if (it)
        {
            SubClassSkill = GetSkillByProto(it->GetItemProperties()->Class, it->GetItemProperties()->SubClass);
            if (SubClassSkill == SKILL_FIST_WEAPONS)
                SubClassSkill = SKILL_UNARMED;
        }
        else
            SubClassSkill = SKILL_UNARMED;


        //chances in feral form don't depend on weapon skill
        if (pr->IsInFeralForm())
        {
            uint8 form = pr->GetShapeShift();
            if (form == FORM_CAT || form == FORM_BEAR || form == FORM_DIREBEAR)
            {
                SubClassSkill = SKILL_FERAL_COMBAT;
                self_skill += pr->getLevel() * 5;
            }
        }

        self_skill += pr->_GetSkillLineCurrent(SubClassSkill);
        crit = getFloatValue(PLAYER_CRIT_PERCENTAGE);
    }
    else
    {
        self_skill = this->getLevel() * 5;
        if (IsCreature())
        {
            Creature* c = static_cast<Creature*>(this);
            if (c->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
                self_skill = std::max(self_skill, ((int32)pVictim->getLevel() + 3) * 5);    //used max to avoid situation when lowlvl hits boss.
        }
        crit = 5.0f;        //will be modified later
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Special Chances Base Calculation

    // crushing blow chance
    //http://www.wowwiki.com/Crushing_blow
    if (pVictim->IsPlayer() && !this->IsPlayer() && !ability && !dmg.school_type)
    {
        int32 baseDefense = static_cast<Player*>(pVictim)->_GetSkillLineCurrent(SKILL_DEFENSE, false);
        int32 skillDiff = self_skill - baseDefense;
        if (skillDiff >= 15)
            crush = -15.0f + 2.0f * skillDiff;
        else
            crush = 0.0f;
    }

    // glancing blow chance
    //http://www.wowwiki.com/Glancing_blow
    // did my own quick research here, seems base glancing against equal level mob is about 5%
    // and goes up 5% each level. Need to check this further.
    float diffAcapped = victim_skill - std::min((float)self_skill, getLevel() * 5.0f);

    if (this->IsPlayer() && !pVictim->IsPlayer() && !ability)
    {
        glanc = 5.0f + diffAcapped;

        if (glanc < 0)
            glanc = 0.0f;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Advanced Chances Modifications
    // by talents
    if (pVictim->IsPlayer())
    {
        if (weapon_damage_type != RANGED)
        {
            crit += static_cast<Player*>(pVictim)->res_M_crit_get();
            hitmodifier += static_cast<Player*>(pVictim)->m_resist_hit[MOD_MELEE];
        }
        else
        {
            crit += static_cast<Player*>(pVictim)->res_R_crit_get();                 //this could be ability but in that case we overwrite the value
            hitmodifier += static_cast<Player*>(pVictim)->m_resist_hit[MOD_RANGED];
        }
    }
    crit += (float)(pVictim->AttackerCritChanceMod[0]);

    // by skill difference
    float vsk = (float)(self_skill - victim_skill);
    dodge = std::max(0.0f, dodge - vsk * 0.04f);

    if (parry)
        parry = std::max(0.0f, parry - vsk * 0.04f);

    if (block)
        block = std::max(0.0f, block - vsk * 0.04f);

    crit += pVictim->IsPlayer() ? vsk * 0.04f : std::min(vsk * 0.2f, 0.0f);

    // http://www.wowwiki.com/Miss
    float misschance;
    float ask = -vsk;

    if (pVictim->IsPlayer())
    {
        if (ask > 0)
            misschance = ask * 0.04f;
        else
            misschance = ask * 0.02f;
    }
    else
    {
        if (ask <= 10)
            misschance = (ask * 0.1f);
        else
            misschance = (2 + (ask - 10) * 0.4f);
    }
    hitchance = 100.0f - misschance;            // base miss chances are worked out further down

    if (ability != nullptr)
    {
        spellModFlatFloatValue(SM_CriticalChance, &crit, ability->getSpellGroupType());
        spellModFlatFloatValue(SM_FHitchance, &hitchance, ability->getSpellGroupType());
    }

    // by ratings
    crit -= pVictim->IsPlayer() ? static_cast<Player*>(pVictim)->CalcRating(PCR_MELEE_CRIT_RESILIENCE) : 0.0f;

    if (crit < 0)
        crit = 0.0f;

    if (this->IsPlayer())
    {
        Player* plr = static_cast<Player*>(this);
        hitmodifier += (weapon_damage_type == RANGED) ? plr->CalcRating(PCR_RANGED_HIT) : plr->CalcRating(PCR_MELEE_HIT);

        float expertise_bonus = plr->CalcRating(PCR_EXPERTISE);
#if VERSION_STRING != Classic
        if (weapon_damage_type == MELEE)
            expertise_bonus += plr->getUInt32Value(PLAYER_EXPERTISE);
        else if (weapon_damage_type == OFFHAND)
            expertise_bonus += plr->getUInt32Value(PLAYER_OFFHAND_EXPERTISE);
#endif

        dodge -= expertise_bonus;
        if (dodge < 0)
            dodge = 0.0f;

        parry -= expertise_bonus;
        if (parry < 0)
            parry = 0.0f;
    }

    //by aura mods
    //Aura 248 SPELL_AURA_MOD_COMBAT_RESULT_CHANCE
    dodge += m_CombatResult_Dodge;
    if (dodge < 0)
        dodge = 0.0f;

    //parry += m_CombatResult_Parry;
    //if (parry <0)
    //parry = 0.0f;

    //by damage type and by weapon type
    if (weapon_damage_type == RANGED)
    {
        dodge = 0.0f;
        parry = 0.0f;
        glanc = 0.0f;
    }

    if (this->IsPlayer())
    {
        it = static_cast<Player*>(this)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);

        if (!ability && it != nullptr && (it->GetItemProperties()->InventoryType == INVTYPE_WEAPON || it->GetItemProperties()->InventoryType == INVTYPE_WEAPONOFFHAND))
        {
            // offhand weapon can either be a 1 hander weapon or an offhander weapon
            hitmodifier -= 24.0f;   //dualwield miss chance
        }
        else
        {
            hitmodifier -= 5.0f;    // base miss chance
        }
    }
    else
    {
        hitmodifier -= 5.0f;        // mobs base hit chance
    }

    hitchance += hitmodifier;

    //Hackfix for Surprise Attacks
    if (this->IsPlayer() && ability && static_cast<Player*>(this)->m_finishingmovesdodge && ability->custom_c_is_flags & SPELL_FLAG_IS_FINISHING_MOVE)
        dodge = 0.0f;

    if (skip_hit_check)
    {
        hitchance = 100.0f;
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
    }

    if (ability != NULL && ability->getAttributes() & ATTRIBUTES_CANT_BE_DPB)
    {
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
    }

    if (ability && ability->getAttributesExB() & ATTRIBUTESEXB_CANT_CRIT)
        crit = 0.0f;

    // by victim state
    if (pVictim->IsPlayer() && pVictim->GetStandState())    //every not standing state is>0
    {
        hitchance = 100.0f;
        dodge = 0.0f;
        parry = 0.0f;
        block = 0.0f;
        crush = 0.0f;
        crit = 100.0f;
    }
    if (backAttack)
    {
        if (pVictim->IsPlayer())
        {
            dodge = 0.0f;               //However mobs can dodge attacks from behind
        }
        parry = 0.0f;
        block = 0.0f;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //One Roll Processing
    // cumulative chances generation
    float chances[7];
    chances[0] = std::max(0.0f, 100.0f - hitchance);
    chances[1] = chances[0] + dodge;
    chances[2] = chances[1] + parry;
    chances[3] = chances[2] + glanc;
    chances[4] = chances[3] + block;
    chances[5] = chances[4] + crit;
    chances[6] = chances[5] + crush;

    //printf("%s:-\n", IsPlayer() ? "Player" : "Mob");
    //printf(" miss: %.2f\n", chances[0]);
    //printf("dodge: %.2f\n", dodge);
    //printf("parry: %.2f\n", parry);
    //printf("glanc: %.2f\n", glanc);
    //printf("block: %.2f\n", block);
    //printf(" crit: %.2f\n", crit);
    //printf("crush: %.2f\n", crush);

    // roll
    float Roll = RandomFloat(100.0f);
    uint32 r = 0;
    while (r < 7 && Roll> chances[r])
    {
        r++;
    }
    if (force_crit)
        r = 5;
    // postroll processing
    uint32 abs = 0;


    //trigger hostile action in ai
    pVictim->GetAIInterface()->HandleEvent(EVENT_HOSTILEACTION, this, 0);

    switch (r)
    {
        case 0:     // miss
            hit_status |= HITSTATUS_MISS;
            if (pVictim->IsCreature() && pVictim->GetAIInterface()->getNextTarget() == NULL)    // dirty ai agro fix
                pVictim->GetAIInterface()->AttackReaction(this, 1, 0);
            break;
        case 1:     //dodge
            if (pVictim->IsCreature() && pVictim->GetAIInterface()->getNextTarget() == NULL)    // dirty ai agro fix
                pVictim->GetAIInterface()->AttackReaction(this, 1, 0);

            CALL_SCRIPT_EVENT(pVictim, OnTargetDodged)(this);
            CALL_SCRIPT_EVENT(this, OnDodged)(this);
            targetEvent = 1;
            vstate = DODGE;
            vproc |= PROC_ON_DODGE_VICTIM;
            pVictim->Emote(EMOTE_ONESHOT_PARRYUNARMED);			// Animation

            if (this->IsPlayer() && this->getClass() == WARRIOR)
            {
                static_cast<Player*>(this)->AddComboPoints(pVictim->GetGUID(), 1);
                static_cast<Player*>(this)->UpdateComboPoints();

                if (!sEventMgr.HasEvent(static_cast<Player*>(this), EVENT_COMBO_POINT_CLEAR_FOR_TARGET))
                    sEventMgr.AddEvent(static_cast<Player*>(this), &Player::NullComboPoints, (uint32)EVENT_COMBO_POINT_CLEAR_FOR_TARGET, (uint32)5000, (uint32)1, (uint32)0);
                else
                    sEventMgr.ModifyEventTimeLeft(static_cast<Player*>(this), EVENT_COMBO_POINT_CLEAR_FOR_TARGET, 5000, 0);
            }

            // Rune strike
            if (pVictim->IsPlayer() && pVictim->getClass() == DEATHKNIGHT)   // omg! dirty hack!
                pVictim->CastSpell(pVictim, 56817, true);

            pVictim->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_DODGE_BLOCK);
            if (!sEventMgr.HasEvent(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE))
                sEventMgr.AddEvent(pVictim, &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_DODGE_BLOCK, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 1, 0);
            else sEventMgr.ModifyEventTimeLeft(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 0);
            break;
        case 2:     //parry
            if (pVictim->IsCreature() && pVictim->GetAIInterface()->getNextTarget() == NULL)    // dirty ai agro fix
                pVictim->GetAIInterface()->AttackReaction(this, 1, 0);

            CALL_SCRIPT_EVENT(pVictim, OnTargetParried)(this);
            CALL_SCRIPT_EVENT(this, OnParried)(this);
            targetEvent = 3;
            vstate = PARRY;
            pVictim->Emote(EMOTE_ONESHOT_PARRYUNARMED);			// Animation

            if (pVictim->IsPlayer())
            {
                // Rune strike
                if (pVictim->getClass() == DEATHKNIGHT)         // omg! dirty hack!
                    pVictim->CastSpell(pVictim, 56817, true);

                pVictim->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_PARRY);	                        //SB@L: Enables spells requiring parry
                if (!sEventMgr.HasEvent(pVictim, EVENT_PARRY_FLAG_EXPIRE))
                    sEventMgr.AddEvent(pVictim, &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_PARRY, EVENT_PARRY_FLAG_EXPIRE, 5000, 1, 0);
                else
                    sEventMgr.ModifyEventTimeLeft(pVictim, EVENT_PARRY_FLAG_EXPIRE, 5000);
                if (static_cast<Player*>(pVictim)->getClass() == 1 || static_cast<Player*>(pVictim)->getClass() == 4)     //warriors for 'revenge' and rogues for 'riposte'
                {
                    pVictim->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_DODGE_BLOCK);	                //SB@L: Enables spells requiring dodge
                    if (!sEventMgr.HasEvent(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE))
                        sEventMgr.AddEvent(pVictim, &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_DODGE_BLOCK, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 1, 0);
                    else
                        sEventMgr.ModifyEventTimeLeft(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000);
                }
            }
            break;
            //////////////////////////////////////////////////////////////////////////////////////////
            //not miss,dodge or parry
        default:
            hit_status |= HITSTATUS_HITANIMATION;//hit animation on victim
            if (pVictim->SchoolImmunityList[0])
                vstate = IMMUNE;
            else
            {
                //////////////////////////////////////////////////////////////////////////////////////////
                //state proc initialization
                vproc |= PROC_ON_ANY_DAMAGE_VICTIM;
                if (weapon_damage_type != RANGED)
                {
                    aproc |= PROC_ON_MELEE_ATTACK;
                    vproc |= PROC_ON_MELEE_ATTACK_VICTIM;
                }
                else
                {
                    aproc |= PROC_ON_RANGED_ATTACK;
                    vproc |= PROC_ON_RANGED_ATTACK_VICTIM;
                    if (ability && ability->getId() == 3018 && IsPlayer() && getClass() == HUNTER)
                        aproc |= PROC_ON_AUTO_SHOT_HIT;
                }
                //////////////////////////////////////////////////////////////////////////////////////////
                //base damage calculation
                if (exclusive_damage)
                    dmg.full_damage = exclusive_damage;
                else
                {
                    if (weapon_damage_type == MELEE && ability)
                        dmg.full_damage = CalculateDamage(this, pVictim, MELEE, ability->getSpellGroupType(), ability);
                    else
                        dmg.full_damage = CalculateDamage(this, pVictim, weapon_damage_type, 0, ability);
                }

                if (pct_dmg_mod > 0)
                    dmg.full_damage = dmg.full_damage * pct_dmg_mod / 100;

                dmg.full_damage += add_damage;

                // \todo Don't really know why it was here. It should be calculated on Spel::CalculateEffect. Maybe it was bugged there...
                //			if (ability && ability->SpellGroupType)
                //			{
                //				spellModFlatIntValue(TO_UNIT(this)->SM_FDamageBonus, &dmg.full_damage, ability->SpellGroupType);
                //				spellModPercentageIntValue(TO_UNIT(this)->SM_PDamageBonus, &dmg.full_damage, ability->SpellGroupType);
                //			}
                //			else
                //			{
                //				spellModFlatIntValue(((Unit*)this)->SM_FMiscEffect,&dmg.full_damage,(uint64)1<<63);
                //				spellModPercentageIntValue(((Unit*)this)->SM_PMiscEffect,&dmg.full_damage,(uint64)1<<63);
                //			}

                dmg.full_damage += pVictim->DamageTakenMod[dmg.school_type];
                if (weapon_damage_type == RANGED)
                {
                    dmg.full_damage += pVictim->RangedDamageTaken;
                }

                if (ability && ability->getMechanicsType() == MECHANIC_BLEEDING)
                    disable_dR = true;


                dmg.full_damage += float2int32(dmg.full_damage * pVictim->DamageTakenPctMod[dmg.school_type]);

                if (dmg.school_type != SCHOOL_NORMAL)
                    dmg.full_damage += float2int32(dmg.full_damage * (GetDamageDonePctMod(dmg.school_type) - 1));

                if (ability != NULL)
                {
                    switch (ability->getId())
                    {
                        //SPELL_HASH_SHRED
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
                            dmg.full_damage += float2int32(dmg.full_damage * pVictim->ModDamageTakenByMechPCT[MECHANIC_BLEEDING]);
                            break;
                    }
                }

                if (ability != NULL)
                {
                    switch (ability->getId())
                    {
                        //SPELL_HASH_MAUL
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
                            dmg.full_damage += float2int32(dmg.full_damage * pVictim->ModDamageTakenByMechPCT[MECHANIC_BLEEDING]);
                            break;
                    }
                }

                //pet happiness state dmg modifier
                if (IsPet() && !static_cast<Pet*>(this)->IsSummonedPet())
                    dmg.full_damage = (dmg.full_damage <= 0) ? 0 : float2int32(dmg.full_damage * static_cast<Pet*>(this)->GetHappinessDmgMod());

                if (dmg.full_damage <0)
                    dmg.full_damage = 0;
                //////////////////////////////////////////////////////////////////////////////////////////
                //check for special hits
                switch (r)
                {
                    //////////////////////////////////////////////////////////////////////////////////////////
                    //glancing blow
                    case 3:
                    {
                        float low_dmg_mod = 1.5f - (0.05f * diffAcapped);
                        if (low_dmg_mod < 0.01)
                            low_dmg_mod = 0.01f;
                        if (low_dmg_mod > 0.91)
                            low_dmg_mod = 0.91f;
                        float high_dmg_mod = 1.2f - (0.03f * diffAcapped);
                        if (high_dmg_mod> 0.99)
                            high_dmg_mod = 0.99f;
                        if (high_dmg_mod < 0.2)
                            high_dmg_mod = 0.2f;

                        float damage_reduction = (high_dmg_mod + low_dmg_mod) / 2.0f;
                        if (damage_reduction > 0)
                        {
                            dmg.full_damage = float2int32(damage_reduction * dmg.full_damage);
                        }
                        hit_status |= HITSTATUS_GLANCING;
                    }
                    break;
                    //////////////////////////////////////////////////////////////////////////////////////////
                    //block
                    case 4:
                    {
                        Item* shield = static_cast<Player*>(pVictim)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                        if (shield != nullptr)
                        {
                            targetEvent = 2;
                            pVictim->Emote(EMOTE_ONESHOT_PARRYSHIELD);// Animation

                            if (shield->GetItemProperties()->InventoryType == INVTYPE_SHIELD)
                            {
                                float block_multiplier = (100.0f + static_cast<Player*>(pVictim)->m_modblockabsorbvalue) / 100.0f;
                                if (block_multiplier < 1.0f)block_multiplier = 1.0f;

                                blocked_damage = float2int32((shield->GetItemProperties()->Block + ((static_cast<Player*>(pVictim)->m_modblockvaluefromspells + pVictim->getUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_BLOCK))) + ((pVictim->GetStat(STAT_STRENGTH) / 2.0f) - 1.0f)) * block_multiplier);

                                if (Rand(m_BlockModPct))
                                    blocked_damage *= 2;
                            }
                            else
                            {
                                blocked_damage = 0;
                            }

                            if (dmg.full_damage <= (int32)blocked_damage)
                                vstate = BLOCK;
                            if (blocked_damage)
                            {
                                CALL_SCRIPT_EVENT(pVictim, OnTargetBlocked)(this, blocked_damage);
                                CALL_SCRIPT_EVENT(this, OnBlocked)(pVictim, blocked_damage);
                                vproc |= PROC_ON_BLOCK_VICTIM;
                            }
                            if (pVictim->IsPlayer())  //not necessary now but we'll have blocking mobs in future
                            {
                                pVictim->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_DODGE_BLOCK);	//SB@L: Enables spells requiring dodge
                                if (!sEventMgr.HasEvent(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE))
                                    sEventMgr.AddEvent(pVictim, &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_DODGE_BLOCK, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 1, 0);
                                else
                                    sEventMgr.ModifyEventTimeLeft(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000);
                            }
                        }
                    }
                    break;
                    //////////////////////////////////////////////////////////////////////////////////////////
                    //critical hit
                    case 5:
                    {
                        hit_status |= HITSTATUS_CRICTICAL;
                        int32 dmgbonus = dmg.full_damage;
                        //LogDebug("DEBUG: Critical Strike! Full_damage: %u" , dmg.full_damage);
                        if (ability != nullptr)
                        {
                            int32 dmg_bonus_pct = 100;
                            spellModFlatIntValue(SM_PCriticalDamage, &dmg_bonus_pct, ability->getSpellGroupType());
                            dmgbonus = dmgbonus * dmg_bonus_pct / 100;
                        }

                        //LogDebug("DEBUG: After CritMeleeDamageTakenPctMod: %u" , dmg.full_damage);
                        if (IsPlayer())
                        {
                            if (weapon_damage_type != RANGED)
                            {
                                dmg.full_damage += dmg.full_damage * static_cast<Player*>(this)->m_modphyscritdmgPCT / 100;
                            }
                            if (!pVictim->IsPlayer())
                                dmg.full_damage += float2int32(dmg.full_damage * static_cast<Player*>(this)->IncreaseCricticalByTypePCT[static_cast<Creature*>(pVictim)->GetCreatureProperties()->Type]);
                            //LogDebug("DEBUG: After IncreaseCricticalByTypePCT: %u" , dmg.full_damage);
                        }

                        dmg.full_damage += dmgbonus;

                        if (weapon_damage_type == RANGED)
                            dmg.full_damage = dmg.full_damage - float2int32(dmg.full_damage * CritRangedDamageTakenPctMod[dmg.school_type]);
                        else
                            dmg.full_damage = dmg.full_damage - float2int32(dmg.full_damage * CritMeleeDamageTakenPctMod[dmg.school_type]);

                        if (pVictim->IsPlayer())
                        {
                            //Resilience is a special new rating which was created to reduce the effects of critical hits against your character.
                            float dmg_reduction_pct = 2.0f * static_cast<Player*>(pVictim)->CalcRating(PCR_MELEE_CRIT_RESILIENCE) / 100.0f;
                            if (dmg_reduction_pct > 1.0f)
                                dmg_reduction_pct = 1.0f; //we cannot resist more then he is criticalling us, there is no point of the critical then :P
                            dmg.full_damage = float2int32(dmg.full_damage - dmg.full_damage * dmg_reduction_pct);
                            //LogDebug("DEBUG: After Resilience check: %u" , dmg.full_damage);
                        }

                        if (pVictim->IsCreature() && static_cast<Creature*>(pVictim)->GetCreatureProperties()->Rank != ELITE_WORLDBOSS)
                            pVictim->Emote(EMOTE_ONESHOT_WOUNDCRITICAL);

                        vproc |= PROC_ON_CRIT_HIT_VICTIM;
                        aproc |= PROC_ON_CRIT_ATTACK;

                        if (weapon_damage_type == RANGED)
                        {
                            vproc |= PROC_ON_RANGED_CRIT_ATTACK_VICTIM;
                            aproc |= PROC_ON_RANGED_CRIT_ATTACK;
                        }

                        if (IsPlayer())
                        {
                            this->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_CRITICAL);
                            if (!sEventMgr.HasEvent(this, EVENT_CRIT_FLAG_EXPIRE))
                                sEventMgr.AddEvent(this, &Unit::EventAurastateExpire, uint32(AURASTATE_FLAG_CRITICAL), EVENT_CRIT_FLAG_EXPIRE, 5000, 1, 0);
                            else sEventMgr.ModifyEventTimeLeft(this, EVENT_CRIT_FLAG_EXPIRE, 5000);
                        }

                        CALL_SCRIPT_EVENT(pVictim, OnTargetCritHit)(this, dmg.full_damage);
                        CALL_SCRIPT_EVENT(this, OnCritHit)(pVictim, dmg.full_damage);
                    }
                    break;
                    //////////////////////////////////////////////////////////////////////////////////////////
                    //crushing blow
                    case 6:
                        hit_status |= HITSTATUS_CRUSHINGBLOW;
                        dmg.full_damage = (dmg.full_damage * 3) >> 1;
                        break;
                        //////////////////////////////////////////////////////////////////////////////////////////
                        //regular hit
                    default:
                        break;
                }
                //////////////////////////////////////////////////////////////////////////////////////////
                //Post Roll Damage Processing
                //////////////////////////////////////////////////////////////////////////////////////////
                //absorption
                uint32 dm = dmg.full_damage;
                abs = pVictim->AbsorbDamage(dmg.school_type, (uint32*)&dm);

                if (dmg.full_damage > (int32)blocked_damage)
                {
                    uint32 sh = pVictim->ManaShieldAbsorb(dmg.full_damage);
                    //////////////////////////////////////////////////////////////////////////////////////////
                    //armor reducing
                    if (sh)
                    {
                        dmg.full_damage -= sh;
                        if (dmg.full_damage && !disable_dR)
                            CalculateResistanceReduction(pVictim, &dmg, ability, ArmorPctReduce);
                        dmg.full_damage += sh;
                        abs += sh;
                    }
                    else if (!disable_dR)
                        CalculateResistanceReduction(pVictim, &dmg, ability, ArmorPctReduce);
                }

                if (abs)
                    vproc |= PROC_ON_ABSORB;

                if (dmg.school_type == SCHOOL_NORMAL)
                {
                    abs += dmg.resisted_damage;
                    dmg.resisted_damage = 0;
                }

                realdamage = dmg.full_damage - abs - dmg.resisted_damage - blocked_damage;
                if (realdamage < 0)
                {
                    realdamage = 0;
                    vstate = IMMUNE;
                    if (!(hit_status & HITSTATUS_BLOCK))
                        hit_status |= HITSTATUS_ABSORBED;
                    else
                        hit_status |= HITSTATUS_BLOCK;
                }
                CALL_SCRIPT_EVENT(this, OnHit)(pVictim, float(realdamage));
            }
            break;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Post Roll Special Cases Processing
    //////////////////////////////////////////////////////////////////////////////////////////
    // Special Effects Processing
    // Paladin: Blessing of Sacrifice, and Warlock: Soul Link
    if (pVictim->m_damageSplitTarget)
    {
        dmg.full_damage = pVictim->DoDamageSplitTarget(dmg.full_damage, dmg.school_type, true);
        realdamage = dmg.full_damage;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //special states processing
    if (pVictim->IsCreature())
    {
        if (pVictim->GetAIInterface() && (pVictim->GetAIInterface()->isAiState(AI_STATE_EVADE) ||
            (pVictim->GetAIInterface()->GetIsSoulLinked() && pVictim->GetAIInterface()->getSoullinkedWith() != this)))
        {
            vstate = EVADE;
            realdamage = 0;
            dmg.full_damage = 0;
            dmg.resisted_damage = 0;
        }
    }
    if (pVictim->IsPlayer() && static_cast<Player*>(pVictim)->GodModeCheat == true)
    {
        dmg.resisted_damage = dmg.full_damage; //godmode
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //dirty fixes
    //vstate=1-wound,2-dodge,3-parry,4-interrupt,5-block,6-evade,7-immune,8-deflect
    // the above code was remade it for reasons : damage shield needs moslty same flags as handleproc + dual wield should proc too ?

    uint32 resisted_dmg;

    //damage shield must come before handleproc to not loose 1 charge : spell gets removed before last charge
    if ((realdamage > 0 || vproc & PROC_ON_BLOCK_VICTIM) && weapon_damage_type != OFFHAND)
    {
        pVictim->HandleProcDmgShield(vproc, this);
        HandleProcDmgShield(aproc, pVictim);
    }

    HandleProc(aproc, pVictim, ability, disable_proc, realdamage, abs, weapon_damage_type);   //maybe using dmg.resisted_damage is better sometimes but then if using godmode dmg is resisted instead of absorbed....bad
    m_procCounter = 0;

    resisted_dmg = pVictim->HandleProc(vproc, this, ability, disable_proc, realdamage, abs, weapon_damage_type);
    pVictim->m_procCounter = 0;

    if (resisted_dmg)
    {
        dmg.resisted_damage += resisted_dmg;
        dmg.full_damage -= resisted_dmg;
        realdamage -= resisted_dmg;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //spells triggering
    if (realdamage > 0 && ability == 0)
    {
        if (IsPlayer() && static_cast<Player*>(this)->m_onStrikeSpells.size())
        {
            SpellCastTargets targets;
            targets.m_unitTarget = pVictim->GetGUID();
            targets.m_targetMask = TARGET_FLAG_UNIT;
            Spell* cspell;

            // Loop on hit spells, and strike with those.
            for (std::map<SpellInfo*, std::pair<uint32, uint32>>::iterator itr = static_cast<Player*>(this)->m_onStrikeSpells.begin();
                 itr != static_cast<Player*>(this)->m_onStrikeSpells.end(); ++itr)
            {
                if (itr->second.first)
                {
                    // We have a *periodic* delayed spell.
                    uint32 t = Util::getMSTime();
                    if (t > itr->second.second)    // Time expired
                    {
                        // Set new time
                        itr->second.second = t + itr->second.first;
                    }

                    // Cast.
                    cspell = sSpellFactoryMgr.NewSpell(this, itr->first, true, NULL);
                    cspell->prepare(&targets);
                }
                else
                {
                    cspell = sSpellFactoryMgr.NewSpell(this, itr->first, true, NULL);
                    cspell->prepare(&targets);
                }
            }
        }

        if (IsPlayer() && static_cast<Player*>(this)->m_onStrikeSpellDmg.size())
        {
            std::map<uint32, OnHitSpell>::iterator it2 = static_cast<Player*>(this)->m_onStrikeSpellDmg.begin();
            std::map<uint32, OnHitSpell>::iterator itr;
            uint32 range, dmg2;
            for (; it2 != static_cast<Player*>(this)->m_onStrikeSpellDmg.end();)
            {
                itr = it2;
                ++it2;

                dmg2 = itr->second.mindmg;
                range = itr->second.maxdmg - itr->second.mindmg;
                if (range != 0)
                    dmg2 += RandomUInt(range);

                SpellNonMeleeDamageLog(pVictim, itr->second.spellid, dmg2, true);
            }
        }

        //ugly hack for shadowfiend restoring mana
        if (GetSummonedByGUID() != 0 && GetEntry() == 19668)
        {
            Player* owner = GetMapMgr()->GetPlayer((uint32)GetSummonedByGUID());
            uint32 amount = static_cast<uint32>(owner->GetMaxPower(POWER_TYPE_MANA) * 0.05f);
            if (owner != NULL)
                this->Energize(owner, 34650, amount, POWER_TYPE_MANA);
        }
        //ugly hack for Bloodsworm restoring hp
        if (getUInt64Value(UNIT_FIELD_SUMMONEDBY) != 0 && getUInt32Value(OBJECT_FIELD_ENTRY) == 28017)
        {
            Player* owner = GetMapMgr()->GetPlayer((uint32)getUInt64Value(UNIT_FIELD_SUMMONEDBY));
            if (owner != NULL)
                Heal(owner, 50452, float2int32(1.5f * realdamage));
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Data Sending
    if (!ability)
    {
        if (dmg.full_damage > 0)
        {
            if (dmg.full_damage == (int32)abs)
                hit_status |= HITSTATUS_ABSORBED;
            else if (dmg.full_damage <= (int32)dmg.resisted_damage)
            {
                hit_status |= HITSTATUS_RESIST;
                vproc |= PROC_ON_ABSORB;
                dmg.resisted_damage = dmg.full_damage;
            }
        }

        if (dmg.full_damage < 0)
            dmg.full_damage = 0;

        if (realdamage < 0)
            realdamage = 0;

        SendAttackerStateUpdate(this, pVictim, &dmg, realdamage, abs, blocked_damage, hit_status, vstate);
    }
    else
    {
        if (realdamage > 0)  //\todo FIX ME: add log for miss,block etc for ability and ranged
        {
            // here we send "dmg.resisted_damage" for "AbsorbedDamage", "0" for "ResistedDamage", and "false" for "PhysicalDamage" even though "School" is "SCHOOL_NORMAL"   o_O
            SendSpellNonMeleeDamageLog(this, pVictim, ability->getId(), realdamage, static_cast<uint8>(dmg.school_type), dmg.resisted_damage, 0, false, blocked_damage, ((hit_status & HITSTATUS_CRICTICAL) != 0), true);
        }
        //FIX ME: add log for miss,block etc for ability and ranged
        //example how it works
        //SendSpellLog(this,pVictim,ability->getId(),SPELL_LOG_MISS);
    }

    if (ability && realdamage == 0)
    {
        SendSpellLog(this, pVictim, ability->getId(), SPELL_LOG_RESIST);
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Damage Dealing

    if (this->IsPlayer() && ability)
        static_cast<Player*>(this)->m_casted_amount[dmg.school_type] = (uint32)(realdamage + abs);

    // invincible people don't take damage
    if (pVictim->bInvincible == false)
    {
        if (realdamage)
        {
            DealDamage(pVictim, realdamage, 0, targetEvent, 0);
            //pVictim->HandleProcDmgShield(PROC_ON_MELEE_ATTACK_VICTIM,this);
            //		HandleProcDmgShield(PROC_ON_MELEE_ATTACK_VICTIM,pVictim);

            if (pVictim->GetCurrentSpell())
                pVictim->GetCurrentSpell()->AddTime(0);
        }
        else
        {
            // have to set attack target here otherwise it wont be set
            // because dealdamage is not called.
            //setAttackTarget(pVictim);
            this->CombatStatus.OnDamageDealt(pVictim);
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Post Damage Dealing Processing
    //////////////////////////////////////////////////////////////////////////////////////////
    //durability processing
    if (pVictim->IsPlayer())
    {
        static_cast<Player*>(pVictim)->GetItemInterface()->ReduceItemDurability();
        if (!this->IsPlayer())
        {
            Player* pr = static_cast<Player*>(pVictim);
            if (Rand(pr->GetSkillUpChance(SKILL_DEFENSE) * worldConfig.getFloatRate(RATE_SKILLCHANCE)))
            {
                pr->_AdvanceSkillLine(SKILL_DEFENSE, float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE)));
                pr->UpdateChances();
            }
        }
        else
        {
            static_cast<Player*>(this)->GetItemInterface()->ReduceItemDurability();
        }
    }
    else
    {
        if (this->IsPlayer())//not pvp
        {
            static_cast<Player*>(this)->GetItemInterface()->ReduceItemDurability();
            Player* pr = static_cast<Player*>(this);
            if (Rand(pr->GetSkillUpChance(SubClassSkill) * worldConfig.getFloatRate(RATE_SKILLCHANCE)))
            {
                pr->_AdvanceSkillLine(SubClassSkill, float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE)));
                //pr->UpdateChances();
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //rage processing
    //http://www.wowwiki.com/Formulas:Rage_generation

    if (dmg.full_damage && IsPlayer() && GetPowerType() == POWER_TYPE_RAGE && !ability)
    {
        float val;
        uint32 level = getLevel();
        float conv;
        if (level <= DBC_PLAYER_LEVEL_CAP)
            conv = AttackToRageConversionTable[level];
        else
            conv = 3.75f / (0.0091107836f * level * level + 3.225598133f * level + 4.2652911f);

        // Hit Factor
        float f = (weapon_damage_type == OFFHAND) ? 1.75f : 3.5f;

        if (hit_status & HITSTATUS_CRICTICAL)
            f *= 2.0f;

        float s = 1.0f;

        // Weapon speed (normal)
        Item* weapon = (static_cast<Player*>(this)->GetItemInterface())->GetInventoryItem(INVENTORY_SLOT_NOT_SET, (weapon_damage_type == OFFHAND ? EQUIPMENT_SLOT_OFFHAND : EQUIPMENT_SLOT_MAINHAND));
        if (weapon == nullptr)
        {
            if (weapon_damage_type == OFFHAND)
                s = getUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1) / 1000.0f;
            else
                s = GetBaseAttackTime(MELEE) / 1000.0f;
        }
        else
        {
            uint32 entry = weapon->GetEntry();
            ItemProperties const* pProto = sMySQLStore.getItemProperties(entry);
            if (pProto != nullptr)
            {
                s = pProto->Delay / 1000.0f;
            }
        }

        val = conv * dmg.full_damage + f * s / 2.0f;
        val *= (1 + (static_cast<Player*>(this)->rageFromDamageDealt / 100.0f));
        float ragerate = worldConfig.getFloatRate(RATE_POWER2);
        val *= 10 * ragerate;

        //float r = (7.5f * dmg.full_damage / c + f * s) / 2.0f;
        //float p = (1 + (TO<Player*>(this)->rageFromDamageDealt / 100.0f));
        //LOG_DEBUG("Rd(%i) d(%i) c(%f) f(%f) s(%f) p(%f) r(%f) rage = %f", realdamage, dmg.full_damage, c, f, s, p, r, val);

        ModPower(POWER_TYPE_RAGE, (int32)val);
        if (GetPower(POWER_TYPE_RAGE) > 1000)
            ModPower(POWER_TYPE_RAGE, 1000 - GetPower(POWER_TYPE_RAGE));

    }

    RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_START_ATTACK);
    //////////////////////////////////////////////////////////////////////////////////////////
    //extra strikes processing
    if (!m_extraAttackCounter)
    {
        int32 extra_attacks = m_extraattacks;
        m_extraAttackCounter = true;
        m_extraattacks = 0;

        while (extra_attacks > 0)
        {
            extra_attacks--;
            Strike(pVictim, weapon_damage_type, NULL, 0, 0, 0, false, false);
        }

        m_extraAttackCounter = false;
    }

    if (m_extrastriketargetc > 0 && !m_extrastriketarget)
    {
        m_extrastriketarget = true;

        std::list<ExtraStrike*>::iterator itx, itx2;
        for (itx = m_extraStrikeTargets.begin(); itx != m_extraStrikeTargets.end();)
        {
            itx2 = itx++;
            ExtraStrike* ex = *itx2;

            for (std::set<Object*>::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
            {
                if ((*itr) == pVictim || !(*itr)->IsUnit())
                    continue;

                if (CalcDistance(*itr) < 5.0f && isAttackable(this, (*itr)) && (*itr)->isInFront(this) && !static_cast<Unit*>(*itr)->IsPacified())
                {
                    // Sweeping Strikes hits cannot be dodged, missed or parried (from wowhead)
                    bool skip_hit_check2 = ex->spell_info->getId() == 12328 ? true : false;
                    //zack : should we use the spell id the registered this extra strike when striking ? It would solve a few proc on proc problems if so ;)
                    //					Strike(TO<Unit*>(*itr), weapon_damage_type, ability, add_damage, pct_dmg_mod, exclusive_damage, false, skip_hit_check);
                    Strike(static_cast<Unit*>(*itr), weapon_damage_type, ex->spell_info, add_damage, pct_dmg_mod, exclusive_damage, false, skip_hit_check2);
                    break;
                }
            }

            // Sweeping Strikes charges are used up regardless whether there is a secondary target in range or not. (from wowhead)
            if (ex->charges > 0)
            {
                ex->charges--;
                if (ex->charges <= 0)
                {
                    m_extrastriketargetc--;
                    m_extraStrikeTargets.erase(itx2);
                    delete ex;
                }
            }
        }

        m_extrastriketarget = false;
    }
}

void Unit::smsg_AttackStop(Unit* pVictim)
{
    if (!pVictim)
        return;

    WorldPacket data(SMSG_ATTACKSTOP, 24);
    if (IsPlayer())
    {
        data << pVictim->GetNewGUID();
        data << uint8(0);
        data << uint32(0);
        static_cast<Player*>(this)->GetSession()->SendPacket(&data);
        data.clear();
    }

    data << GetNewGUID();
    data << pVictim->GetNewGUID();
    data << uint32(0);
    SendMessageToSet(&data, true);
    // stop swinging, reset pvp timeout

    if (pVictim->IsPlayer())
    {
        pVictim->CombatStatusHandler_ResetPvPTimeout();
        CombatStatusHandler_ResetPvPTimeout();
    }
    else
    {
        if (!IsPlayer() || getClass() == ROGUE)
        {
            m_cTimer = Util::getMSTime() + 8000;
            sEventMgr.RemoveEvents(this, EVENT_COMBAT_TIMER);
            sEventMgr.AddEvent(this, &Unit::EventUpdateFlag, EVENT_COMBAT_TIMER, 8000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            if (pVictim->IsUnit())   // there could be damage coming from objects/enviromental
                sEventMgr.AddEvent(pVictim, &Unit::EventUpdateFlag, EVENT_COMBAT_TIMER, 8000, 1, 0);
        }
        else
        {
        }
    }
}

void Unit::smsg_AttackStop(uint64 victimGuid)
{
    WorldPacket data(20);
    data.Initialize(SMSG_ATTACKSTOP);
    data << GetNewGUID();
    FastGUIDPack(data, victimGuid);
    data << uint32(0);
    SendMessageToSet(&data, IsPlayer());
}

void Unit::smsg_AttackStart(Unit* pVictim)
{
    // Send out ATTACKSTART
    WorldPacket data(SMSG_ATTACKSTART, 16);
    data << GetGUID();
    data << pVictim->GetGUID();
    SendMessageToSet(&data, false);
    LOG_DEBUG("WORLD: Sent SMSG_ATTACKSTART");

    // FLAGS changed so other players see attack animation
    //    addUnitFlag(UNIT_FLAG_COMBAT);
    //    setUpdateMaskBit(UNIT_FIELD_FLAGS);
    if (IsPlayer())
    {
        Player* pThis = static_cast<Player*>(this);
        if (pThis->cannibalize)
        {
            sEventMgr.RemoveEvents(pThis, EVENT_CANNIBALIZE);
            pThis->SetEmoteState(EMOTE_ONESHOT_NONE);
            pThis->cannibalize = false;
        }
    }
}

uint8 Unit::FindVisualSlot(uint32 SpellId, bool IsPos)
{
    uint32 from, to;
    uint8 visualslot = 0xFF;
    if (IsPos)
    {
        from = 0;
        to = MAX_POSITIVE_VISUAL_AURAS_END;
    }
    else
    {
        from = MAX_NEGATIVE_VISUAL_AURAS_START;
        to = MAX_NEGATIVE_VISUAL_AURAS_END;
    }
    //check for already visual same aura
    for (uint32 i = from; i < to; i++)
        if (m_auravisuals[i] == SpellId)
        {
            visualslot = static_cast<uint8>(i);
            break;
        }
    if (visualslot == 0xFF)
        for (uint32 i = from; i < to; i++)
            if (m_auravisuals[i] == 0)
            {
                visualslot = static_cast<uint8>(i);
                break;
            }
    return visualslot;
}

void Unit::AddAura(Aura* aur)
{
    if (aur == NULL)
        return;

    if (!(isAlive() || (aur->GetSpellInfo()->isDeathPersistent())))
    {
        delete aur;
        return;
    }

    if (m_mapId != 530 && (m_mapId != 571 || (IsPlayer() && !static_cast<Player*>(this)->HasSpell(54197) && static_cast<Player*>(this)->getDeathState() == ALIVE)))
        // can't use flying auras in non-outlands or non-northrend (northrend requires cold weather flying)
    {
        for (uint8 i = 0; i < 3; ++i)
        {
            if (aur->GetSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_ENABLE_FLIGHT_WITH_UNMOUNTED_SPEED || aur->GetSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_ENABLE_FLIGHT2)
            {
                delete aur;
                return;
            }
        }
    }

    if (aur->GetSpellInfo()->getSchool() && SchoolImmunityList[aur->GetSpellInfo()->getSchool()])
    {
        delete aur;
        return;
    }

    // If this aura can only affect one target at a time
    if (aur->GetSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SINGLE_TARGET_AURA)
    {
        // remove aura from the previous applied target
        Unit* caster = aur->GetUnitCaster();
        uint64 prev_target_guid = 0;

        if (caster != NULL)
        {
            prev_target_guid = caster->getSingleTargetGuidForAura(aur->GetSpellInfo()->getId());

            if (prev_target_guid && prev_target_guid != aur->GetTarget()->GetGUID())
            {
                Unit* prev_target = this->GetMapMgr()->GetUnit(prev_target_guid);
                if (prev_target != NULL)
                    prev_target->removeAllAurasById(aur->GetSpellInfo()->getId());
            }
        }

        // remove aura from this unit. other player/unit may have casted on this target
        // this is necessary for the following case:
        //  1) attacker A cast on target A
        //  2) attacker B cast on target B
        //  3) attacker A cast on target B, and aura is removed from target A
        //  4) attacker B cast on target A, and aura is not removed from target B, because caster A is now the one that casted on target B
        if (prev_target_guid && prev_target_guid != aur->GetTarget()->GetGUID())
            removeAllAurasById(aur->GetSpellInfo()->getId());
    }

    uint16 AuraSlot = 0xFFFF;
    //all this code block is to try to find a valid slot for our new aura.
    if (!aur->IsPassive())
    {
        uint32 AlreadyApplied = 0, CheckLimit, StartCheck;
        if (aur->IsPositive())
        {
            StartCheck = MAX_POSITIVE_AURAS_EXTEDED_START; //also check talents to make sure they will not stack. Maybe not required ?
            CheckLimit = MAX_POSITIVE_AURAS_EXTEDED_END;
        }
        else
        {
            StartCheck = MAX_NEGATIVE_AURAS_EXTEDED_START;
            CheckLimit = MAX_NEGATIVE_AURAS_EXTEDED_END;
        }
        // Nasty check for Blood Fury debuff (spell system based on namehashes is bs anyways)
        if (!sSpellCustomizations.isAlwaysApply(aur->GetSpellInfo()) == false)
        {
            //uint32 aurName = aur->GetSpellProto()->Name;
            //uint32 aurRank = aur->GetSpellProto()->Rank;
            uint32 maxStack = aur->GetSpellInfo()->getMaxstack();
            if (aur->GetSpellInfo()->getProcCharges() > 0)
            {
                int charges = aur->GetSpellInfo()->getProcCharges();
                Unit* ucaster = aur->GetUnitCaster();
                if (ucaster != nullptr)
                {
                    spellModFlatIntValue(ucaster->SM_FCharges, &charges, aur->GetSpellInfo()->getSpellGroupType());
                    spellModPercentageIntValue(ucaster->SM_PCharges, &charges, aur->GetSpellInfo()->getSpellGroupType());
                }
                maxStack = charges;
            }
            if (IsPlayer() && static_cast<Player*>(this)->AuraStackCheat)
                maxStack = 999;

            SpellInfo* info = aur->GetSpellInfo();
            //uint32 flag3 = aur->GetSpellProto()->Flags3;

            AuraCheckResponse acr;
            WorldPacket data(21);
            bool deleteAur = false;

            //check if we already have this aura by this caster -> update duration
            for (uint32 x = StartCheck; x < CheckLimit; x++)
            {
                if (m_auras[x])
                {
                    if (m_auras[x]->GetSpellId() == aur->GetSpellId())
                    {
                        if (!aur->IsPositive()
                            && m_auras[x]->m_casterGuid != aur->m_casterGuid
                            && (m_auras[x]->GetSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_MAXSTACK_FOR_DEBUFF) == 0
                            )
                        {
                            continue;
                        }
                        AlreadyApplied++;
                        //update duration,the same aura (update the whole stack whenever we cast a new one)
                        m_auras[x]->ResetDuration();

                        if (maxStack <= AlreadyApplied)
                        {
                            ModVisualAuraStackCount(m_auras[x], 0);
                            if (AlreadyApplied == 1)
                                m_auras[x]->UpdateModifiers();
                            deleteAur = true;
                            break;
                        }
                    }
                    else if ((aur->pSpellId != m_auras[x]->GetSpellInfo()->getId()))     // if this is a proc spell then it should not remove it's mother : test with combustion later
                    {
                        // Check for auras by specific type.
                        if (info->custom_BGR_one_buff_on_target > 0 && m_auras[x]->GetSpellInfo()->custom_BGR_one_buff_on_target & info->custom_BGR_one_buff_on_target && maxStack == 0)
                        {
                            deleteAur = HasAurasOfBuffType(info->custom_BGR_one_buff_on_target, aur->m_casterGuid, 0);
                        }
                        // Check for auras with the same name and a different rank.
                        else
                        {
                            acr = AuraCheck(info, m_auras[x], aur->GetCaster());
                            if (acr.Error == AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT)
                            {
                                deleteAur = true;
                            }
                            else if (acr.Error == AURA_CHECK_RESULT_LOWER_BUFF_PRESENT)
                            {
                                // remove the lower aura
                                m_auras[x]->Remove();

                                // no more checks on bad ptr
                                continue;
                            }
                        }
                    }
                }
                else if (AuraSlot == 0xFFFF)
                {
                    AuraSlot = static_cast<uint16>(x);
                }
            }

            if (deleteAur)
            {
                sEventMgr.RemoveEvents(aur);

                // Once stacked 5 times, each application of Deadly poison also causes the poison on the Rogue's other weapon to apply
                // http://www.wowhead.com/?item=43233#comments
                if (AlreadyApplied >= maxStack && info->custom_c_is_flags & SPELL_FLAG_IS_POISON)
                {
                    Player* caster = aur->GetPlayerCaster();
                    if (caster != NULL)
                    {
                        switch (info->getId())
                        {
                            // SPELL_HASH_DEADLY_POISON_IX:
                            case 57970:
                            case 57973:
                                // SPELL_HASH_DEADLY_POISON_VIII:
                            case 57969:
                            case 57972:
                                // SPELL_HASH_DEADLY_POISON_VII:
                            case 27186:
                            case 27187:
                                // SPELL_HASH_DEADLY_POISON_VI:
                            case 26967:
                            case 26968:
                                // SPELL_HASH_DEADLY_POISON_V:
                            case 25349:
                            case 25351:
                                // SPELL_HASH_DEADLY_POISON_IV:
                            case 11354:
                            case 11356:
                                // SPELL_HASH_DEADLY_POISON_III:
                            case 11353:
                            case 11355:
                                // SPELL_HASH_DEADLY_POISON_II:
                            case 2819:
                            case 2824:
                                // SPELL_HASH_DEADLY_POISON:
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
                                Item* mh = caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                                Item* oh = caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);

                                if (mh != NULL && oh != NULL)
                                {
                                    uint32 mh_spell = 0;
                                    uint32 oh_spell = 0;
                                    bool is_mh_deadly_poison = false;
                                    bool is_oh_deadly_poison = false;

                                    // Find mainhand enchantment
                                    EnchantmentInstance* ench = mh->GetEnchantment(TEMP_ENCHANTMENT_SLOT);
                                    if (ench)
                                    {
                                        DBC::Structures::SpellItemEnchantmentEntry const* Entry = ench->Enchantment;
                                        for (uint8 c = 0; c < 3; c++)
                                        {
                                            if (Entry->type[c] && Entry->spell[c])
                                            {
                                                SpellInfo* sp = sSpellCustomizations.GetSpellInfo(Entry->spell[c]);
                                                if (sp && sp->custom_c_is_flags & SPELL_FLAG_IS_POISON)
                                                {
                                                    switch (sp->getId())
                                                    {
                                                        // SPELL_HASH_DEADLY_POISON_IX:
                                                        case 57970:
                                                        case 57973:
                                                            // SPELL_HASH_DEADLY_POISON_VIII:
                                                        case 57969:
                                                        case 57972:
                                                            // SPELL_HASH_DEADLY_POISON_VII:
                                                        case 27186:
                                                        case 27187:
                                                            // SPELL_HASH_DEADLY_POISON_VI:
                                                        case 26967:
                                                        case 26968:
                                                            // SPELL_HASH_DEADLY_POISON_V:
                                                        case 25349:
                                                        case 25351:
                                                            // SPELL_HASH_DEADLY_POISON_IV:
                                                        case 11354:
                                                        case 11356:
                                                            // SPELL_HASH_DEADLY_POISON_III:
                                                        case 11353:
                                                        case 11355:
                                                            // SPELL_HASH_DEADLY_POISON_II:
                                                        case 2819:
                                                        case 2824:
                                                            // SPELL_HASH_DEADLY_POISON:
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
                                                            is_mh_deadly_poison = true;
                                                            break;
                                                    }

                                                    mh_spell = Entry->spell[c];
                                                    break;
                                                }
                                            }
                                        }

                                        // Find offhand enchantment
                                        ench = oh->GetEnchantment(TEMP_ENCHANTMENT_SLOT);
                                        if (ench)
                                        {
                                            DBC::Structures::SpellItemEnchantmentEntry const* Entry = ench->Enchantment;
                                            for (uint8 c = 0; c < 3; c++)
                                            {
                                                if (Entry->type[c] && Entry->spell[c])
                                                {
                                                    SpellInfo* sp = sSpellCustomizations.GetSpellInfo(Entry->spell[c]);
                                                    if (sp && sp->custom_c_is_flags & SPELL_FLAG_IS_POISON)
                                                    {
                                                        switch (sp->getId())
                                                        {
                                                            // SPELL_HASH_DEADLY_POISON_IX:
                                                            case 57970:
                                                            case 57973:
                                                                // SPELL_HASH_DEADLY_POISON_VIII:
                                                            case 57969:
                                                            case 57972:
                                                                // SPELL_HASH_DEADLY_POISON_VII:
                                                            case 27186:
                                                            case 27187:
                                                                // SPELL_HASH_DEADLY_POISON_VI:
                                                            case 26967:
                                                            case 26968:
                                                                // SPELL_HASH_DEADLY_POISON_V:
                                                            case 25349:
                                                            case 25351:
                                                                // SPELL_HASH_DEADLY_POISON_IV:
                                                            case 11354:
                                                            case 11356:
                                                                // SPELL_HASH_DEADLY_POISON_III:
                                                            case 11353:
                                                            case 11355:
                                                                // SPELL_HASH_DEADLY_POISON_II:
                                                            case 2819:
                                                            case 2824:
                                                                // SPELL_HASH_DEADLY_POISON:
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
                                                                is_oh_deadly_poison = true;
                                                                break;
                                                        }

                                                        oh_spell = Entry->spell[c];
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    // Only apply if both weapons are enchanted and enchantment is poison and enchantment type is different
                                    if (mh_spell && oh_spell && mh_spell != oh_spell && is_mh_deadly_poison != is_oh_deadly_poison)
                                    {
                                        if (mh_spell != info->getId())
                                            caster->CastSpell(aur->GetTarget(), mh_spell, true);
                                        else
                                            caster->CastSpell(aur->GetTarget(), oh_spell, true);
                                    }
                                }

                                break;
                            }
                        }
                    }
                }

                delete aur;
                return;
            }
        }
        else
        {
            //these auras stack to infinite and with anything. Don't ask me why there is no better solution for them :P
            for (uint32 x = StartCheck; x < CheckLimit; x++)
                if (!m_auras[x])
                {
                    AuraSlot = static_cast<uint16>(x);
                    break;
                }
        }
    }
    else
    {
        //talents just get applied always. Maybe we should check stack for these as well?
        for (uint32 x = MAX_PASSIVE_AURAS_START; x < MAX_PASSIVE_AURAS_END; x++)
            if (!m_auras[x])
            {
                AuraSlot = static_cast<uint16>(x);
                break;
            }
        //			else if (m_auras[x]->GetID()==aur->GetID()) printf("OMG stacking talents ?\n");
    }


    //check if we can store this aura in some empty slot
    if (AuraSlot == 0xFFFF)
    {
        LOG_ERROR("Aura error in active aura. ");
        sEventMgr.RemoveEvents(aur);
        delete aur;
        /*
                if (aur != NULL)
                {
                delete [] aur;
                aur = NULL;
                }
                */
        return;
    }

    //Zack : if all mods were resisted it means we did not apply anything and we do not need to delete this spell either
    if (aur->TargetWasImuneToMods())
    {
        ///\todo notify client that we are immune to this spell
        sEventMgr.RemoveEvents(aur);
        delete aur;
        return;
    }

    uint8 visualslot = 0xFF;
    //search for a visual slot
    if (!aur->IsPassive() || (aur->m_spellInfo->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO))
        visualslot = FindVisualSlot(aur->GetSpellId(), aur->IsPositive());
    aur->m_visualSlot = visualslot;

    // Zack : No idea how a new aura can already have a slot. Leaving it for compatibility
    if (aur->m_auraSlot != 0xffff)
        m_auras[aur->m_auraSlot] = NULL;

    aur->m_auraSlot = AuraSlot;

    m_auras[AuraSlot] = aur;
    UpdateAuraForGroup(visualslot);
    ModVisualAuraStackCount(aur, 1);

    aur->ApplyModifiers(true);

    // We add 500ms here to allow for the last tick in DoT spells. This is a dirty hack, but at least it doesn't crash like my other method.
    // - Burlex
    if (aur->GetDuration() > 0)
    {
        sEventMgr.AddEvent(aur, &Aura::Remove, EVENT_AURA_REMOVE, aur->GetDuration() + 500, 1,
                           EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT | EVENT_FLAG_DELETES_OBJECT);
    }

    //have to hate these relocate events. They run in a separate thread :P
    aur->RelocateEvents();

    // Reaction from enemy AI
    if (!aur->IsPositive() && aur->IsCombatStateAffecting())	  // Creature
    {
        Unit* pCaster = aur->GetUnitCaster();
        if (pCaster && pCaster->isAlive() && this->isAlive())
        {
            pCaster->CombatStatus.OnDamageDealt(this);

            if (IsCreature())
                m_aiInterface->AttackReaction(pCaster, 1, aur->GetSpellId());
        }
    }

    if (aur->GetSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_INVINCIBLE)
    {
        Unit* pCaster = aur->GetUnitCaster();
        if (pCaster)
        {
            pCaster->RemoveStealth();
            pCaster->RemoveInvisibility();

            uint32 iceBlock[] =
            {
                //SPELL_HASH_ICE_BLOCK
                27619,
                36911,
                41590,
                45438,
                45776,
                46604,
                46882,
                56124,
                56644,
                62766,
                65802,
                69924,
                0
            };
            pCaster->removeAllAurasById(iceBlock);

            uint32 divineShield[] =
            {
                //SPELL_HASH_DIVINE_SHIELD
                642,
                13874,
                29382,
                33581,
                40733,
                41367,
                54322,
                63148,
                66010,
                67251,
                71550,
                0
            };
            pCaster->removeAllAurasById(divineShield);
            //SPELL_HASH_BLESSING_OF_PROTECTION
            pCaster->removeAllAurasById(41450);
        }
    }

    // If this aura can only affect one target at a time, store this target GUID for future reference
    if (aur->GetSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SINGLE_TARGET_AURA)
    {
        Unit* caster = aur->GetUnitCaster();
        if (caster != NULL)
            caster->setSingleTargetGuidForAura(aur->GetSpellInfo()->getId(), this->GetGUID());
    }

    /* Set aurastates */
    uint32 flag = 0;
    if (aur->GetSpellInfo()->getMechanicsType() == MECHANIC_ENRAGED && !asc_enraged++)
        flag |= AURASTATE_FLAG_ENRAGED;
    else if (aur->GetSpellInfo()->getMechanicsType() == MECHANIC_BLEEDING && !asc_bleed++)
        flag |= AURASTATE_FLAG_BLEED;
    if (aur->GetSpellInfo()->custom_BGR_one_buff_on_target & SPELL_TYPE_SEAL && !asc_seal++)
        flag |= AURASTATE_FLAG_JUDGEMENT;

    SetFlag(UNIT_FIELD_AURASTATE, flag);
}

bool Unit::RemoveAura(Aura* aur)
{
    if (aur == NULL)
        return false;

    aur->Remove();
    return true;
}

bool Unit::RemoveAura(uint32 spellId)
{
    //this can be speed up, if we know passive \pos neg
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x] && m_auras[x]->GetSpellId() == spellId)
        {
            m_auras[x]->Remove();
            return true;  // sky: yes, only one, see bug charges/auras queues
        }
    return false;
}

bool Unit::RemoveAuras(uint32* SpellIds)
{
    if (!SpellIds || *SpellIds == 0)
        return false;

    uint32 x, y;
    bool res = false;
    for (x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x])
        {
            for (y = 0; SpellIds[y] != 0; y++)
            {
                if (m_auras[x] && m_auras[x]->GetSpellId() == SpellIds[y])
                {
                    m_auras[x]->Remove();
                    res = true;
                }
            }
        }
    }
    return res;
}

bool Unit::RemoveAurasByHeal()
{
    bool res = false;
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x])
        {
            switch (m_auras[x]->GetSpellId())
            {
                // remove after heal
                case 35321:
                case 38363:
                case 39215:
                {
                    m_auras[x]->Remove();
                    res = true;
                }
                break;
                // remove when healed to 100%
                case 31956:
                case 38801:
                case 43093:
                {
                    if (getUInt32Value(UNIT_FIELD_HEALTH) == getUInt32Value(UNIT_FIELD_MAXHEALTH))
                    {
                        m_auras[x]->Remove();
                        res = true;
                    }
                }
                break;
                // remove at p% health
                case 38772:
                {
                    uint32 p = m_auras[x]->GetSpellInfo()->getEffectBasePoints(1);
                    if (getUInt32Value(UNIT_FIELD_MAXHEALTH) * p <= getUInt32Value(UNIT_FIELD_HEALTH) * 100)
                    {
                        m_auras[x]->Remove();
                        res = true;
                    }
                }
                break;
            }
        }
    }

    return res;
}

bool Unit::AuraActionIf(AuraAction* action, AuraCondition* condition)
{
    bool done = false;

    for (uint32 i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; i++)
    {
        Aura* aura = m_auras[i];

        if (aura == NULL)
            continue;

        if ((*condition)(aura))
        {
            (*action)(aura);
            done = true;
        }
    }

    return done;
}

void Unit::ClearAllAreaAuraTargets()
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        Aura* a = m_auras[x];

        if (a == NULL)
            continue;

        if (a->m_areaAura)   // This was not casted by us, so no removal
            continue;

        if (a->IsAreaAura())
            a->ClearAATargets();
    }
}

void Unit::RemoveAllAreaAuraByOther()
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        Aura* a = m_auras[x];

        if (a == NULL)   // empty slot
            continue;

        if (!a->m_areaAura)   // not area aura, or we casted it
            continue;

        a->Remove();
    }
}

bool Unit::RemoveAura(uint32 spellId, uint64 guid)
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x])
        {
            if (m_auras[x]->GetSpellId() == spellId && m_auras[x]->m_casterGuid == guid)
            {
                m_auras[x]->Remove();
                return true;
            }
        }
    }
    return false;
}

bool Unit::RemoveAuraByItemGUID(uint32 spellId, uint64 guid)
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x])
        {
            if (m_auras[x]->GetSpellId() == spellId && m_auras[x]->itemCasterGUID == guid)
            {
                m_auras[x]->Remove();
                return true;
            }
        }
    }
    return false;
}

void Unit::RemoveNegativeAuras()
{
    for (uint32 x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_REMOVABLE_AURAS_END; x++)
    {
        if (m_auras[x])
        {
            if (m_auras[x]->GetSpellInfo()->isDeathPersistent())
                continue;
            else
                m_auras[x]->Remove();
        }
    }
}

void Unit::RemoveAllAuras()
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x])
            m_auras[x]->Remove();
}

void Unit::RemoveAllNonPersistentAuras()
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x])
        {
            if (m_auras[x]->GetSpellInfo()->isDeathPersistent())
                continue;
            else
                m_auras[x]->Remove();
        }
}

//ex:to remove morph spells
void Unit::RemoveAllAuraType(uint32 auratype)
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x] && m_auras[x]->HasModType(auratype))
            m_auras[x]->Remove();//remove all morph auras containing to this spell (like wolf morph also gives speed)
}

void Unit::RemoveAllAurasByRequiredShapeShift(uint32 mask)
{
    Aura* aura;

    for (uint32 i = MAX_REMOVABLE_AURAS_START; i < MAX_REMOVABLE_AURAS_END; ++i)
    {
        aura = m_auras[i];
        if (aura == NULL || !aura->IsPositive())
            continue;

        if (aura->GetSpellInfo()->getRequiredShapeShift() & mask)
            aura->Remove();
    }
}

bool Unit::SetAurDuration(uint32 spellId, Unit* caster, uint32 duration)
{
    LOG_DEBUG("setAurDuration2");
    Aura* aur = getAuraWithIdForGuid(spellId, caster->GetGUID());
    if (!aur)
        return false;
    aur->SetDuration(duration);
    sEventMgr.ModifyEventTimeLeft(aur, EVENT_AURA_REMOVE, duration);

    return true;
}

bool Unit::SetAurDuration(uint32 spellId, uint32 duration)
{
    Aura* aur = getAuraWithId(spellId);

    if (!aur)
        return false;

    LOG_DEBUG("setAurDuration2");
    aur->SetDuration(duration);
    sEventMgr.ModifyEventTimeLeft(aur, EVENT_AURA_REMOVE, duration);

    return true;
}

void Unit::_UpdateSpells(uint32 time)
{
    // to avoid deleting the current spell
    if (m_currentSpell != NULL)
    {
        //		m_spellsbusy=true;
        m_currentSpell->Update(time);
        //		m_spellsbusy=false;
    }
}

void Unit::castSpell(Spell* pSpell)
{
    // check if we have a spell already casting etc
    if (m_currentSpell && pSpell != m_currentSpell)
    {
        m_currentSpell->cancel();
    }

    m_currentSpell = pSpell;
    pLastSpell = pSpell->GetSpellInfo();
}

int32 Unit::GetSpellDmgBonus(Unit* pVictim, SpellInfo* spellInfo, int32 base_dmg, bool isdot)
{
    float plus_damage = 0.0f;
    Unit* caster = this;
    uint32 school = spellInfo->getSchool();

    if (spellInfo->custom_c_is_flags & SPELL_FLAG_IS_NOT_USING_DMG_BONUS)
        return 0;

    if (caster->IsPlayer())
    {
        switch (static_cast<Player*>(this)->getClass())
        {
            case ROGUE:
            case WARRIOR:
            case DEATHKNIGHT:
            case HUNTER:
                return 0;
            default:
                break;
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //by school
    plus_damage += static_cast<float>(caster->GetDamageDoneMod(school));
    plus_damage += static_cast<float>(base_dmg * (caster->GetDamageDonePctMod(school) - 1)); //value is initialized with 1
    //////////////////////////////////////////////////////////////////////////////////////////
    //by victim type
    if (!pVictim->IsPlayer() && caster->IsPlayer())
        plus_damage += static_cast<float>(static_cast<Player*>(caster)->IncreaseDamageByType[static_cast<Creature*>(pVictim)->GetCreatureProperties()->Type]);

    //////////////////////////////////////////////////////////////////////////////////////////
    //Spell Damage Bonus Modifications
    //////////////////////////////////////////////////////////////////////////////////////////
    //by cast duration

    // do not execute this if plus dmg is 0 or lower
    if (plus_damage > 0.0f)
    {
        if (spellInfo->Dspell_coef_override >= 0.0f && !isdot)
            plus_damage = plus_damage * spellInfo->Dspell_coef_override;
        else if (spellInfo->OTspell_coef_override >= 0.0f && isdot)
            plus_damage = plus_damage * spellInfo->OTspell_coef_override;
        else
        {
            //Bonus to DD part
            if (spellInfo->fixed_dddhcoef >= 0.0f && !isdot)
                plus_damage = plus_damage * spellInfo->fixed_dddhcoef;
            //Bonus to DoT part
            else if (spellInfo->fixed_hotdotcoef >= 0.0f && isdot)
            {
                plus_damage = plus_damage * spellInfo->fixed_hotdotcoef;
                if (caster->IsPlayer())
                {
                    int32 durmod = 0;
                    spellModFlatIntValue(caster->SM_FDur, &durmod, spellInfo->getSpellGroupType());
                    plus_damage += static_cast<float>(plus_damage * durmod / 15000);
                }
            }
            //In case we dont fit in previous cases do old thing
            else
            {
                plus_damage = plus_damage * spellInfo->casttime_coef;
                float td = static_cast<float>(GetDuration(sSpellDurationStore.LookupEntry(spellInfo->getDurationIndex())));

                switch (spellInfo->getId())
                {
                    //SPELL_HASH_MOONFIRE
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
                    //SPELL_HASH_PYROBLAST
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
                    //SPELL_HASH_ICE_LANCE
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
                    //SPELL_HASH_IMMOLATE
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
                        plus_damage = plus_damage * (1.0f - ((td / 15000.0f) / ((td / 15000.0f))));
                    default:
                        break;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Bonus Adding To Main Damage
    if ((pVictim->HasAuraWithMechanics(MECHANIC_ENSNARED) || pVictim->HasAuraWithMechanics(MECHANIC_DAZED)) && caster->IsPlayer())
        plus_damage += static_cast<float>(static_cast<Player*>(caster)->m_IncreaseDmgSnaredSlowed);


        int32 bonus_damage = 0;
        spellModFlatIntValue(caster->SM_FPenalty, &bonus_damage, spellInfo->getSpellGroupType());
        spellModFlatIntValue(caster->SM_FDamageBonus, &bonus_damage, spellInfo->getSpellGroupType());

        int32 dmg_bonus_pct = 0;
        spellModFlatIntValue(caster->SM_PPenalty, &dmg_bonus_pct, spellInfo->getSpellGroupType());
        spellModFlatIntValue(caster->SM_PDamageBonus, &dmg_bonus_pct, spellInfo->getSpellGroupType());

        plus_damage += static_cast<float>((base_dmg + bonus_damage) * dmg_bonus_pct / 100);


    return static_cast<int32>(plus_damage);
}

float Unit::CalcSpellDamageReduction(Unit* victim, SpellInfo* spell, float res)
{
    float reduced_damage = 0;
    reduced_damage += static_cast<float>(victim->DamageTakenMod[spell->getSchool()]);
    reduced_damage += res * victim->DamageTakenPctMod[spell->getSchool()];
    reduced_damage += res * victim->ModDamageTakenByMechPCT[spell->getMechanicsType()];
    return reduced_damage;
}

void Unit::InterruptSpell()
{
    if (m_currentSpell)
    {
        m_currentSpell->cancel();
    }
}

void Unit::DeMorph()
{
    // hope it solves it :)
    uint32 displayid = this->GetNativeDisplayId();
    this->SetDisplayId(displayid);
    EventModelChange();
}

#if VERSION_STRING < Cata
void Unit::Emote(EmoteType emote)
{
#if VERSION_STRING < Cata
    if (emote == 0)
        return;
#endif

    WorldPacket data(SMSG_EMOTE, 12);
    data << uint32(emote);
    data << this->GetGUID();
    SendMessageToSet(&data, true);
}
#else
void Unit::Emote(EmoteType emote)
{
    WorldPacket data(SMSG_EMOTE, 12);
    data << uint32_t(emote);
    data << uint64_t(GetGUID());
    SendMessageToSet(&data, true);
}
#endif

void Unit::SendChatMessageAlternateEntry(uint32 entry, uint8 type, uint32 lang, const char* msg)
{
    size_t UnitNameLength = 0, MessageLength = 0;
    CreatureProperties const* ci = sMySQLStore.getCreatureProperties(entry);
    if (ci == nullptr)
        return;

    UnitNameLength = ci->Name.size();
    MessageLength = strlen((char*)msg) + 1;

    WorldPacket data(SMSG_MESSAGECHAT, 35 + UnitNameLength + MessageLength);
    data << type;
    data << lang;
    data << GetGUID();
    data << uint32(0);			// new in 2.1.0
    data << uint32(UnitNameLength);
    data << ci->Name;
    data << uint64(0);
    data << uint32(MessageLength);
    data << msg;
    data << uint8(0x00);
    SendMessageToSet(&data, true);
}

void Unit::WipeHateList()
{
    GetAIInterface()->WipeHateList();
}

void Unit::ClearHateList()
{
    GetAIInterface()->ClearHateList();
}

void Unit::WipeTargetList()
{
    GetAIInterface()->WipeTargetList();
}

void Unit::AddInRangeObject(Object* pObj)
{
    if (pObj->IsUnit())
    {
        if (isHostile(this, pObj))
            m_oppFactsInRange.insert(pObj);

        if (isFriendly(this, pObj))
            m_sameFactsInRange.insert(pObj);
    }

    Object::AddInRangeObject(pObj);
}//427

void Unit::OnRemoveInRangeObject(Object* pObj)
{
    m_oppFactsInRange.erase(pObj);
    m_sameFactsInRange.erase(pObj);

    if (pObj->IsUnit())
    {
        Unit* pUnit = static_cast<Unit*>(pObj);
        GetAIInterface()->CheckTarget(pUnit);

        if (GetCharmedUnitGUID() == pObj->GetGUID())
            if (m_currentSpell)
                m_currentSpell->cancel();
    }
}

void Unit::ClearInRangeSet()
{
    Object::ClearInRangeSet();
}

//Events
void Unit::EventAddEmote(EmoteType emote, uint32 time)
{
    m_oldEmote = GetEmoteState();
    SetEmoteState(emote);
    sEventMgr.AddEvent(this, &Creature::EmoteExpire, EVENT_UNIT_EMOTE, time, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Unit::EmoteExpire()
{
    SetEmoteState(m_oldEmote);
    sEventMgr.RemoveEvents(this, EVENT_UNIT_EMOTE);
}

void Unit::MoveToWaypoint(uint32 wp_id)
{
    if (this->m_useAI && this->GetAIInterface() != nullptr)
    {
        AIInterface* aiInterface = this->GetAIInterface();
        Movement::WayPoint* wayPoint = aiInterface->getWayPoint(wp_id);
        if (wayPoint != nullptr)
        {
            aiInterface->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
            aiInterface->setWayPointToMove(wp_id);
        }
        else
        {
            LOG_ERROR("Invalid waypoint specified.");
            return;
        }
    }
}

void Unit::CalcDamage()
{
    if (IsPlayer())
        static_cast<Player*>(this)->CalcDamage();
    else
    {
        if (IsPet())
            static_cast<Pet*>(this)->UpdateAP();
        float r;
        float delta;
        float mult;

        float ap_bonus = GetAP() / 14000.0f;

        float bonus = ap_bonus * (GetBaseAttackTime(MELEE) + static_cast<Creature*>(this)->m_speedFromHaste);

        delta = float(static_cast<Creature*>(this)->ModDamageDone[0]);
        mult = float(static_cast<Creature*>(this)->ModDamageDonePct[0]);
        r = (BaseDamage[0] + bonus) * mult + delta;
        // give some diversity to pet damage instead of having a 77-78 damage range (as an example)
        SetMinDamage(r > 0 ? (IsPet() ? r * 0.9f : r) : 0);
        r = (BaseDamage[1] + bonus) * mult + delta;
        SetMaxDamage(r > 0 ? (IsPet() ? r * 1.1f : r) : 0);

        //	SetMinRangedDamage(BaseRangedDamage[0]*mult+delta);
        //	SetMaxRangedDamage(BaseRangedDamage[1]*mult+delta);
    }
}

/// returns absorbed dmg
uint32 Unit::ManaShieldAbsorb(uint32 dmg)
{
    if (!m_manashieldamt)
        return 0;
    //mana shield group->16. the only

    uint32 mana = GetPower(POWER_TYPE_MANA);
    int32 effectbonus = SM_PEffectBonus ? SM_PEffectBonus[16] : 0;

    int32 potential = (mana * 50) / ((100 + effectbonus));
    if (potential > m_manashieldamt)
        potential = m_manashieldamt;

    if ((int32)dmg < potential)
        potential = dmg;

    uint32 cost = (potential * (100 + effectbonus)) / 50;

    setUInt32Value(UNIT_FIELD_POWER1, mana - cost);
    m_manashieldamt -= potential;
    if (!m_manashieldamt)
        RemoveAura(m_manaShieldId);
    return potential;
}

uint32 Unit::AbsorbDamage(uint32 School, uint32* dmg)
{
    if (dmg == NULL)
        return 0;

    if (School > 6)
        return 0;

    uint32 dmg_absorbed = 0;
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x] == NULL || !m_auras[x]->IsAbsorb())
            continue;

        AbsorbAura* aur = static_cast<AbsorbAura*>(m_auras[x]);

        dmg_absorbed += aur->AbsorbDamage(School, dmg);
    }

    if (IsPlayer() && static_cast<Player*>(this)->GodModeCheat)
    {
        dmg_absorbed += *dmg;
        *dmg = 0;
    }

    return dmg_absorbed;
}

bool Unit::setDetectRangeMod(uint64 guid, int32 amount)
{
    int next_free_slot = -1;
    for (uint8 i = 0; i < 5; i++)
    {
        if (m_detectRangeGUID[i] == 0 && next_free_slot == -1)
        {
            next_free_slot = i;
        }
        if (m_detectRangeGUID[i] == guid)
        {
            m_detectRangeMOD[i] = amount;
            return true;
        }
    }
    if (next_free_slot != -1)
    {
        m_detectRangeGUID[next_free_slot] = guid;
        m_detectRangeMOD[next_free_slot] = amount;
        return true;
    }
    return false;
}

void Unit::unsetDetectRangeMod(uint64 guid)
{
    for (uint8 i = 0; i < 5; i++)
    {
        if (m_detectRangeGUID[i] == guid)
        {
            m_detectRangeGUID[i] = 0;
            m_detectRangeMOD[i] = 0;
        }
    }
}

int32 Unit::getDetectRangeMod(uint64 guid)
{
    for (uint8 i = 0; i < 5; i++)
    {
        if (m_detectRangeGUID[i] == guid)
        {
            return m_detectRangeMOD[i];
        }
    }
    return 0;
}

bool Unit::IsSitting()
{
    uint8 s = GetStandState();
    return
        s == STANDSTATE_SIT_CHAIR        || s == STANDSTATE_SIT_LOW_CHAIR  ||
        s == STANDSTATE_SIT_MEDIUM_CHAIR || s == STANDSTATE_SIT_HIGH_CHAIR ||
        s == STANDSTATE_SIT;
}

void Unit::SetStandState(uint8 standstate)
{
    //only take actions if standstate did change.
    StandState bef = GetStandState();
    if (bef == standstate)
        return;

    setByteValue(UNIT_FIELD_BYTES_1, 0, standstate);
    if (standstate == STANDSTATE_STAND)  //standup
        RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_STAND_UP);

    if (IsPlayer())
        static_cast<Player*>(this)->GetSession()->OutPacket(SMSG_STANDSTATE_UPDATE, 1, &standstate);
}

void Unit::RemoveAurasByInterruptFlag(uint32 flag)
{
    Aura* a;
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        a = m_auras[x];
        if (a == NULL)
            continue;

        //some spells do not get removed all the time only at specific intervals
        if ((a->m_spellInfo->getAuraInterruptFlags() & flag) && !(a->m_spellInfo->getProcFlags() & PROC_REMOVEONUSE))
        {
            a->Remove();
            m_auras[x] = NULL;
        }
    }
}

bool Unit::HasAuraVisual(uint32 visualid)
{
    //passive auras do not have visual (at least when code was written)
    for (uint32 x = MAX_REMOVABLE_AURAS_START; x < MAX_REMOVABLE_AURAS_END; x++)
        if (m_auras[x] && m_auras[x]->GetSpellInfo()->getSpellVisual() == visualid)
            return true;
    return false;
}

bool Unit::HasAura(uint32 spellid)
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x] && m_auras[x]->GetSpellId() == spellid)
        {
            return true;
        }

    return false;
}

Aura* Unit::GetAuraWithSlot(uint32 slot)
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x] && m_auras[x]->m_visualSlot == (uint16)slot)
        {
            return m_auras[x];
        }

    return NULL;
}

uint16 Unit::GetAuraStackCount(uint32 spellid)
{
    uint16 count = 0;
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x] && m_auras[x]->GetSpellId() == spellid)
            count++;
    return count;
}

void Unit::DropAurasOnDeath()
{
    for (uint32 x = MAX_REMOVABLE_AURAS_START; x < MAX_REMOVABLE_AURAS_END; x++)
        if (m_auras[x])
        {
            if (m_auras[x] && m_auras[x]->GetSpellInfo()->isDeathPersistent())
                continue;
            else
                m_auras[x]->Remove();
        }
}

bool Unit::IsControlledByPlayer()
{
    if (IS_PLAYER_GUID(GetCharmedByGUID()) || IsPlayer())
        return true;
    return false;
}

void Unit::UpdateSpeed()
{
    if (GetMount() == 0)
    {
        m_currentSpeedRun = m_basicSpeedRun * (1.0f + ((float)m_speedModifier) / 100.0f);
    }
    else
    {
        m_currentSpeedRun = m_basicSpeedRun * (1.0f + ((float)m_mountedspeedModifier) / 100.0f);
        m_currentSpeedRun += (m_speedModifier < 0) ? (m_basicSpeedRun * ((float)m_speedModifier) / 100.0f) : 0;
    }

    m_currentSpeedFly = m_basicSpeedFly * (1.0f + ((float)m_flyspeedModifier) / 100.0f);

    // Limit speed due to effects such as http://www.wowhead.com/?spell=31896 [Judgement of Justice]
    if (m_maxSpeed && m_currentSpeedRun > m_maxSpeed)
    {
        m_currentSpeedRun = m_maxSpeed;
    }

    if (IsPlayer() && static_cast<Player*>(this)->m_changingMaps)
    {
        static_cast<Player*>(this)->resend_speed = true;
    }
    else
    {
        setSpeedForType(TYPE_RUN, m_currentSpeedRun);
        setSpeedForType(TYPE_FLY, m_currentSpeedFly);
    }
}

bool Unit::HasBuff(uint32 spellid) // cebernic:it does not check passive auras & must be visible auras
{
    for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_POSITIVE_AURAS_EXTEDED_END; x++)
        if (m_auras[x] && m_auras[x]->GetSpellId() == spellid)
            return true;

    return false;
}

bool Unit::HasBuff(uint32 spellid, uint64 guid)
{
    for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_POSITIVE_AURAS_EXTEDED_END; x++)
        if (m_auras[x] && m_auras[x]->GetSpellId() == spellid && m_auras[x]->m_casterGuid == guid)
            return true;

    return false;
}

uint8 Unit::CastSpell(Unit* Target, SpellInfo* Sp, bool triggered)
{
    if (Sp == NULL)
        return SPELL_FAILED_UNKNOWN;

    Spell* newSpell = sSpellFactoryMgr.NewSpell(this, Sp, triggered, 0);
    SpellCastTargets targets(0);
    if (Target)
    {
        targets.m_targetMask |= TARGET_FLAG_UNIT;
        targets.m_unitTarget = Target->GetGUID();
    }
    else
    {
        newSpell->GenerateTargets(&targets);
    }

    return newSpell->prepare(&targets);
}

uint8 Unit::CastSpell(Unit* Target, uint32 SpellID, bool triggered)
{
    SpellInfo* ent = sSpellCustomizations.GetSpellInfo(SpellID);
    if (ent == NULL) return SPELL_FAILED_UNKNOWN;

    return CastSpell(Target, ent, triggered);
}

uint8 Unit::CastSpell(uint64 targetGuid, SpellInfo* Sp, bool triggered)
{
    if (Sp == NULL)
        return SPELL_FAILED_UNKNOWN;

    SpellCastTargets targets(targetGuid);
    Spell* newSpell = sSpellFactoryMgr.NewSpell(this, Sp, triggered, 0);
    return newSpell->prepare(&targets);
}

uint8 Unit::CastSpell(uint64 targetGuid, uint32 SpellID, bool triggered)
{
    SpellInfo* ent = sSpellCustomizations.GetSpellInfo(SpellID);
    if (ent == NULL) return SPELL_FAILED_UNKNOWN;

    return CastSpell(targetGuid, ent, triggered);
}

uint8 Unit::CastSpell(Unit* Target, uint32 SpellID, uint32 forced_basepoints, bool triggered)
{
    return CastSpell(Target, sSpellCustomizations.GetSpellInfo(SpellID), forced_basepoints, triggered);
}

uint8 Unit::CastSpell(Unit* Target, SpellInfo* Sp, uint32 forced_basepoints, bool triggered)
{
    if (Sp == NULL)
        return SPELL_FAILED_UNKNOWN;

    Spell* newSpell = sSpellFactoryMgr.NewSpell(this, Sp, triggered, 0);
    newSpell->forced_basepoints[0] = forced_basepoints;
    SpellCastTargets targets(0);
    if (Target != NULL)
    {
        targets.m_targetMask |= TARGET_FLAG_UNIT;
        targets.m_unitTarget = Target->GetGUID();
    }
    else
    {
        newSpell->GenerateTargets(&targets);
    }

    return newSpell->prepare(&targets);
}

uint8 Unit::CastSpell(Unit* Target, uint32 SpellID, uint32 forced_basepoints, int32 charges, bool triggered)
{
    return CastSpell(Target, sSpellCustomizations.GetSpellInfo(SpellID), forced_basepoints, charges, triggered);
}

uint8 Unit::CastSpell(Unit* Target, SpellInfo* Sp, uint32 forced_basepoints, int32 charges, bool triggered)
{
    if (Sp == NULL)
        return SPELL_FAILED_UNKNOWN;

    Spell* newSpell = sSpellFactoryMgr.NewSpell(this, Sp, triggered, 0);
    newSpell->forced_basepoints[0] = forced_basepoints;
    newSpell->m_charges = charges;
    SpellCastTargets targets(0);
    if (Target != NULL)
    {
        targets.m_targetMask |= TARGET_FLAG_UNIT;
        targets.m_unitTarget = Target->GetGUID();
    }
    else
    {
        newSpell->GenerateTargets(&targets);
    }

    return newSpell->prepare(&targets);
}

void Unit::CastSpellAoF(LocationVector lv, SpellInfo* Sp, bool triggered)
{
    if (Sp == nullptr)
        return;

    SpellCastTargets targets;
    targets.setDestination(lv);
    targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;
    Spell* newSpell = sSpellFactoryMgr.NewSpell(this, Sp, triggered, 0);
    newSpell->prepare(&targets);
}

void Unit::RemoveAurasByBuffType(uint32 buff_type, const uint64 & guid, uint32 skip)
{
    uint64 sguid = buff_type >= SPELL_TYPE_BLESSING ? guid : 0;

    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x]  //have aura
            && (m_auras[x]->GetSpellInfo()->custom_BGR_one_buff_on_target & buff_type) // aura is in same group
            && m_auras[x]->GetSpellId() != skip // make sure to not do self removes in case aura will stack
            && (!sguid || (sguid && m_auras[x]->m_casterGuid == sguid)) // we either remove everything or just buffs from us
            )
            m_auras[x]->Remove();
    }
}

bool Unit::HasAurasOfBuffType(uint32 buff_type, const uint64 & guid, uint32 skip)
{
    uint64 sguid = (buff_type == SPELL_TYPE_BLESSING || buff_type == SPELL_TYPE_WARRIOR_SHOUT) ? guid : 0;

    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x] && m_auras[x]->GetSpellInfo()->custom_BGR_one_buff_on_target & buff_type && m_auras[x]->GetSpellId() != skip)
            if (!sguid || (m_auras[x]->m_casterGuid == sguid))
                return true;
    }

    return false;
}

AuraCheckResponse Unit::AuraCheck(SpellInfo* proto, Object* caster)
{
    AuraCheckResponse resp;

    // no error for now
    resp.Error = AURA_CHECK_RESULT_NONE;
    resp.Misc = 0;

    uint32 name_hash = proto->custom_NameHash;
    uint32 rank = proto->custom_RankNumber;
    Aura* aura;
    SpellInfo* aura_sp;

    // look for spells with same namehash
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        aura = m_auras[x];
        if (aura != NULL && aura->GetSpellInfo()->custom_NameHash == name_hash)
        {
            // we've got an aura with the same name as the one we're trying to apply
            // but first we check if it has the same effects
            aura_sp = aura->GetSpellInfo();

            if ((aura_sp->getEffect(0) == proto->getEffect(0) && (aura_sp->getEffect(0) != SPELL_EFFECT_APPLY_AURA ||
                aura_sp->getEffectApplyAuraName(0) == proto->getEffectApplyAuraName(0))) &&
                (aura_sp->getEffect(1) == proto->getEffect(1) && (aura_sp->getEffect(1) != SPELL_EFFECT_APPLY_AURA ||
                aura_sp->getEffectApplyAuraName(1) == proto->getEffectApplyAuraName(1))) &&
                (aura_sp->getEffect(2) == proto->getEffect(2) && (aura_sp->getEffect(2) != SPELL_EFFECT_APPLY_AURA ||
                aura_sp->getEffectApplyAuraName(2) == proto->getEffectApplyAuraName(2))))
            {
                resp.Misc = aura->GetSpellInfo()->getId();

                // compare the rank to our applying spell
                if (aura_sp->custom_RankNumber > rank)
                {
                    if (proto->getEffect(0) == SPELL_EFFECT_TRIGGER_SPELL ||
                        proto->getEffect(1) == SPELL_EFFECT_TRIGGER_SPELL ||
                        proto->getEffect(2) == SPELL_EFFECT_TRIGGER_SPELL)
                    {
                        resp.Error = AURA_CHECK_RESULT_LOWER_BUFF_PRESENT;
                    }
                    else
                        resp.Error = AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT;
                }
                else
                    resp.Error = AURA_CHECK_RESULT_LOWER_BUFF_PRESENT;

                // we found something, save some loops and exit
                break;
            }
        }
    }
    //LOG_DEBUG("resp = %i", resp.Error);
    // return it back to our caller
    return resp;
}

AuraCheckResponse Unit::AuraCheck(SpellInfo* proto, Aura* aur, Object* caster)
{
    AuraCheckResponse resp;
    SpellInfo* aura_sp = aur->GetSpellInfo();

    // no error for now
    resp.Error = AURA_CHECK_RESULT_NONE;
    resp.Misc = 0;

    // look for spells with same namehash
    if (aur->GetSpellInfo()->custom_NameHash == proto->custom_NameHash)
    {
        // we've got an aura with the same name as the one we're trying to apply
        // but first we check if it has the same effects
        if ((aura_sp->getEffect(0) == proto->getEffect(0) &&
            (aura_sp->getEffect(0) != SPELL_EFFECT_APPLY_AURA || aura_sp->getEffectApplyAuraName(0) == proto->getEffectApplyAuraName(0))) &&
            (aura_sp->getEffect(1) == proto->getEffect(1) &&
            (aura_sp->getEffect(1) != SPELL_EFFECT_APPLY_AURA || aura_sp->getEffectApplyAuraName(1) == proto->getEffectApplyAuraName(1))) &&
            (aura_sp->getEffect(2) == proto->getEffect(2) &&
            (aura_sp->getEffect(2) != SPELL_EFFECT_APPLY_AURA || aura_sp->getEffectApplyAuraName(2) == proto->getEffectApplyAuraName(2))))
        {
            resp.Misc = aur->GetSpellInfo()->getId();

            // compare the rank to our applying spell
            if (aur->GetSpellInfo()->custom_RankNumber > proto->custom_RankNumber)
                resp.Error = AURA_CHECK_RESULT_HIGHER_BUFF_PRESENT;
            else
                resp.Error = AURA_CHECK_RESULT_LOWER_BUFF_PRESENT;
        }
    }

    // return it back to our caller
    return resp;
}

void Unit::OnPushToWorld()
{
    //Zack : we already relocated events on aura add ?
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x] != 0)
            m_auras[x]->RelocateEvents();
    }

    if (GetVehicleComponent() != NULL)
        GetVehicleComponent()->InstallAccessories();

    z_axisposition = 0.0f;
}

//! Remove Unit from world
void Unit::RemoveFromWorld(bool free_guid)
{
    if (GetCurrentVehicle() != NULL)
        GetCurrentVehicle()->EjectPassenger(this);

    if (GetVehicleComponent() != NULL)
    {
        GetVehicleComponent()->RemoveAccessories();
        GetVehicleComponent()->EjectAllPassengers();
    }

    RemoveVehicleComponent();

    CombatStatus.OnRemoveFromWorld();
    if (GetSummonedCritterGUID() != 0)
    {
        SetSummonedCritterGUID(0);

        Unit* u = m_mapMgr->GetUnit(GetSummonedCritterGUID());
        if (u != NULL)
            u->Delete();
    }

    if (dynObj != 0)
        dynObj->Remove();

    for (uint8 i = 0; i < 4; ++i)
    {
        if (m_ObjectSlots[i] != 0)
        {
            GameObject* obj = m_mapMgr->GetGameObject(m_ObjectSlots[i]);
            if (obj)
                obj->ExpireAndDelete();

            m_ObjectSlots[i] = 0;
        }
    }

    ClearAllAreaAuraTargets();
    RemoveAllAreaAuraByOther();

    Object::RemoveFromWorld(free_guid);

    //zack: should relocate new events to new eventmanager and not to -1
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
        if (m_auras[x] != 0)
        {
            if (m_auras[x]->m_deleted)
            {
                m_auras[x] = NULL;
                continue;
            }
            m_auras[x]->RelocateEvents();
        }

    m_aiInterface->WipeReferences();
}

void Unit::Deactivate(MapMgr* mgr)
{
    CombatStatus.Vanished();
    Object::Deactivate(mgr);
}

void Unit::RemoveAurasByInterruptFlagButSkip(uint32 flag, uint32 skip)
{
    Aura* a;
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        a = m_auras[x];
        if (a == 0)
            continue;

        //some spells do not get removed all the time only at specific intervals
        if ((a->m_spellInfo->getAuraInterruptFlags() & flag) && (a->m_spellInfo->getId() != skip) && a->m_spellInfo->custom_proc_interval == 0)
        {
            //the black sheep's of society
            if (a->m_spellInfo->getAuraInterruptFlags() & AURA_INTERRUPT_ON_CAST_SPELL)
            {
                switch (a->GetSpellInfo()->getId())
                {
                    //priest - surge of light
                    case 33151:
                    {
                        switch (m_currentSpell->GetSpellInfo()->getId())
                        {
                            //SPELL_HASH_SMITE
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
                                //our luck. it got triggered on smite..we do not remove it just yet
                                if (m_currentSpell)
                                    continue;
                            }
                        }

                        //this spell gets removed only when casting smite
                        SpellInfo* spi = sSpellCustomizations.GetSpellInfo(skip);

                        if (spi)
                        {
                            switch (spi->getId())
                            {
                                //SPELL_HASH_SMITE
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
                                    continue;
                            }
                        }
                    }
                    break;
                    case 34936:	// Backlash
                    {
                        if (m_currentSpell)
                        {
                            switch (m_currentSpell->GetSpellInfo()->getId())
                            {
                                //SPELL_HASH_SHADOW_BOLT
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
                                //SPELL_HASH_INCINERATE
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
                                    continue;
                                default:
                                    break;
                            }
                        }

                        SpellInfo* spi = sSpellCustomizations.GetSpellInfo(skip);
                        if (spi)
                        {
                            switch (spi->getId())
                            {
                                //SPELL_HASH_SHADOW_BOLT
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
                                //SPELL_HASH_INCINERATE
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
                                    break;
                                default:
                                    continue;
                            }
                        }
                    }
                    break;
                    case 59578: // Art of War
                    case 53489:
                    {
                        if (m_currentSpell)
                        {
                            switch (m_currentSpell->m_spellInfo->getId())
                            {
                                //SPELL_HASH_FLASH_OF_LIGHT
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
                                    continue;
                                default:
                                    break;
                            }
                        }

                        SpellInfo* spi = sSpellCustomizations.GetSpellInfo(skip);
                        if (spi)
                        {
                            switch (spi->getId())
                            {
                                //SPELL_HASH_FLASH_OF_LIGHT
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
                                    break;
                                default:
                                    continue;
                            }
                        }
                    }
                    break;
                    case 17941: //Shadow Trance
                    {
                        if (m_currentSpell)
                        {
                            switch (m_currentSpell->GetSpellInfo()->getId())
                            {
                                //SPELL_HASH_SHADOW_BOLT
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
                                    continue;
                                default:
                                    break;
                            }
                        }

                        SpellInfo* spi = sSpellCustomizations.GetSpellInfo(skip);
                        if (spi)
                        {
                            switch (spi->getId())
                            {
                                //SPELL_HASH_SHADOW_BOLT
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
                                    break;
                                default:
                                    continue;
                            }
                        }
                    }
                    break;
                    case 16166: // [Shaman] Elemental Mastery
                    {
                        SpellInfo* spi = sSpellCustomizations.GetSpellInfo(skip);
                        if (spi && !(spi->getSchool() == SCHOOL_FIRE || spi->getSchool() == SCHOOL_FROST || spi->getSchool() == SCHOOL_NATURE))
                            continue;
                    }
                    break;
                    case 48108: // Hot Streak
                    {
                        if (m_currentSpell)
                            continue;

                        switch (m_currentSpell->GetSpellInfo()->getId())
                        {
                            //SPELL_HASH_PYROBLAST
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
                                break;
                            default:
                                continue;
                        }
                    }
                    break;
                }
            }
            a->Remove();
        }
    }
}

bool Unit::HasAuraWithName(uint32 name)
{

    for (uint32 i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        if (m_auras[i] != NULL && m_auras[i]->GetSpellInfo()->appliesAreaAura(name))
            return true;
    }

    return false;
}

uint32 Unit::GetAuraCountWithName(uint32 name)
{
    uint32 count = 0;

    for (uint32 i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        if (m_auras[i] != NULL && m_auras[i]->GetSpellInfo()->appliesAreaAura(name))
            ++count;
    }

    return count;
}

bool Unit::HasAuraWithMechanics(uint32 mechanic)
{
    for (uint32 x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_NEGATIVE_AURAS_EXTEDED_END; ++x)
    {
        if (m_auras[x] && m_auras[x]->m_spellInfo)
            if (Spell::GetMechanic(m_auras[x]->m_spellInfo) == mechanic)
                return true;
    }

    return false;
}

bool Unit::IsPoisoned()
{
    for (uint32 x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_NEGATIVE_AURAS_EXTEDED_END; ++x)
        if (m_auras[x] && m_auras[x]->GetSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_POISON)
            return true;

    return false;
}

#if VERSION_STRING != Cata
void Unit::SendFullAuraUpdate()
{
#if VERSION_STRING > TBC
    WorldPacket data(SMSG_AURA_UPDATE_ALL, 200);

    data << WoWGuid(GetNewGUID());

    uint32 Updates = 0;

    for (uint32 i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aur = m_auras[i];
        if (aur != NULL)
        {
            uint8 Flags = uint8(aur->GetAuraFlags());

            Flags = (AFLAG_EFFECT_1 | AFLAG_EFFECT_2 | AFLAG_EFFECT_3);

            if (aur->IsPositive())
                Flags |= AFLAG_CANCELLABLE;
            else
                Flags |= AFLAG_NEGATIVE;

            if (aur->GetDuration() != 0)
                Flags |= AFLAG_DURATION;

            data << uint8(aur->m_visualSlot);
            data << uint32(aur->GetSpellId());

            data << uint8(Flags);

            data << uint8(getLevel());
            data << uint8(m_auraStackCount[aur->m_visualSlot]);

            if ((Flags & AFLAG_NOT_CASTER) == 0)
                data << WoWGuid(aur->GetCasterGUID());

            if (Flags & AFLAG_DURATION)
            {
                data << uint32(aur->GetDuration());
                data << uint32(aur->GetTimeLeft());
            }

            ++Updates;
        }
    }
    SendMessageToSet(&data, true);

    LOG_DEBUG("Full Aura Update: GUID: " I64FMT " - Updates: %u", GetGUID(), Updates);
#endif
}
#else
void Unit::SendFullAuraUpdate()
{
    WorldPacket data(SMSG_AURA_UPDATE_ALL, 200);

    data << WoWGuid(GetNewGUID());

    uint32 Updates = 0;

    for (uint32 i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        Aura* aur = m_auras[i];
        if (aur != NULL)
        {
            uint8 Flags = uint8(aur->GetAuraFlags());

            Flags = (AFLAG_EFFECT_1 | AFLAG_EFFECT_2 | AFLAG_EFFECT_3);

            if (aur->IsPositive())
                Flags |= AFLAG_CANCELLABLE;
            else
                Flags |= AFLAG_NEGATIVE;

            if (aur->GetDuration() != 0)
                Flags |= AFLAG_DURATION;

            data << uint8(aur->m_visualSlot);
            data << uint32(aur->GetSpellId());

            data << uint16(Flags);

            data << uint8(getLevel());
            data << uint8(m_auraStackCount[aur->m_visualSlot]);

            if ((Flags & AFLAG_NOT_CASTER) == 0)
                data << WoWGuid(aur->GetCasterGUID());

            if (Flags & AFLAG_DURATION)
            {
                data << uint32(aur->GetDuration());
                data << uint32(aur->GetTimeLeft());
            }

            ++Updates;
        }
    }
    SendMessageToSet(&data, true);

    LOG_DEBUG("Full Aura Update: GUID: " I64FMT " - Updates: %u", GetGUID(), Updates);
}
#endif

#if VERSION_STRING != Cata
void Unit::SendAuraUpdate(uint32 AuraSlot, bool remove)
{
#if VERSION_STRING > TBC
    Aura* aur = m_auras[AuraSlot];
    ARCEMU_ASSERT(aur != NULL);

    WorldPacket data(SMSG_AURA_UPDATE, 30);

    if (remove)
    {
        data << WoWGuid(GetGUID());
        data << uint8(aur->m_visualSlot);
        data << uint32(0);
    }
    else
    {
        uint8 flags = (AFLAG_EFFECT_1 | AFLAG_EFFECT_2 | AFLAG_EFFECT_3);

        if (aur->IsPositive())
            flags |= AFLAG_CANCELLABLE;
        else
            flags |= AFLAG_NEGATIVE;

        if (aur->GetDuration() != 0 && !(aur->GetSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_HIDE_DURATION))
            flags |= AFLAG_DURATION;

        data << WoWGuid(GetGUID());
        data << uint8(aur->m_visualSlot);

        data << uint32(aur->GetSpellId());
        data << uint8(flags);

        Unit* caster = aur->GetUnitCaster();
        if (caster != NULL)
            data << uint8(caster->getLevel());
        else
            data << uint8(worldConfig.player.playerLevelCap);

        data << uint8(m_auraStackCount[aur->m_visualSlot]);

        if ((flags & AFLAG_NOT_CASTER) == 0)
            data << WoWGuid(aur->GetCasterGUID());

        if (flags & AFLAG_DURATION)
        {
            data << uint32(aur->GetDuration());
            data << uint32(aur->GetTimeLeft());
        }
    }

    SendMessageToSet(&data, true);
#endif
}
#else
void Unit::SendAuraUpdate(uint32 AuraSlot, bool remove)
{
    Aura* aur = m_auras[AuraSlot];
    ARCEMU_ASSERT(aur != NULL);

    WorldPacket data(SMSG_AURA_UPDATE, 200);
    data << WoWGuid(GetGUID());
    data << uint8(aur->m_visualSlot);

    if (remove)
    {
        data << uint32(0);
    }
    else
    {
        data << uint32(aur->GetSpellId());

        uint32 flags = (AFLAG_EFFECT_1 | AFLAG_EFFECT_2 | AFLAG_EFFECT_3);

        if (aur->IsPositive())
            flags |= AFLAG_CANCELLABLE;
        else
            flags |= AFLAG_NEGATIVE;

        if (aur->GetDuration() != 0 && !(aur->GetSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_HIDE_DURATION))
            flags |= AFLAG_DURATION;

        data << uint16(flags);

        Unit* caster = aur->GetUnitCaster();
        if (caster != nullptr)
            data << uint8(caster->getLevel());
        else
            data << uint8(worldConfig.player.playerLevelCap);

        data << uint8(m_auraStackCount[aur->m_visualSlot]);

        if ((flags & AFLAG_NOT_CASTER) == 0)
            data << WoWGuid(aur->GetCasterGUID());

        if (flags & AFLAG_DURATION)
        {
            data << uint32(aur->GetDuration());
            data << uint32(aur->GetTimeLeft());
        }
    }

    SendMessageToSet(&data, true);
}
#endif

uint32 Unit::ModVisualAuraStackCount(Aura* aur, int32 count)
{
    if (!aur)
        return 0;

    uint8 slot = aur->m_visualSlot;
    if (slot >= MAX_NEGATIVE_VISUAL_AURAS_END)
        return 0;

    if (count < 0 && m_auraStackCount[slot] <= -count)
    {
        m_auraStackCount[slot] = 0;
        m_auravisuals[slot] = 0;

        SendAuraUpdate(aur->m_auraSlot, true);
    }
    else
    {
        m_auraStackCount[slot] += static_cast<uint8>(count);
        m_auravisuals[slot] = aur->GetSpellId();

        SendAuraUpdate(aur->m_auraSlot, false);
    }

    return m_auraStackCount[slot];
}

void Unit::RemoveAurasOfSchool(uint32 School, bool Positive, bool Immune)
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
        if (m_auras[x]
            && m_auras[x]->GetSpellInfo()->getSchool() == School
            && (!m_auras[x]->IsPositive() || Positive)
            && (!Immune && m_auras[x]->GetSpellInfo()->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY)
            )
            m_auras[x]->Remove();
}

bool Unit::IsDazed()
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x])
        {
            if (m_auras[x]->GetSpellInfo()->getMechanicsType() == MECHANIC_ENSNARED)
                return true;
            for (uint32 y = 0; y < 3; y++)
                if (m_auras[x]->GetSpellInfo()->getEffectMechanic(y) == MECHANIC_ENSNARED)
                    return true;
        }
    }

    return false;
}

void Unit::UpdateVisibility()
{
    ByteBuffer buf(3000);
    uint32 count;
    bool can_see;
    bool is_visible;
    Player* pl;
    Object* pObj;
    Player* plr;

    if (IsPlayer())
    {
        plr = static_cast<Player*>(this);
        for (Object::InRangeSet::iterator itr2 = m_objectsInRange.begin(); itr2 != m_objectsInRange.end();)
        {
            pObj = (*itr2);
            ++itr2;

            can_see = plr->CanSee(pObj);
            is_visible = plr->IsVisible(pObj->GetGUID());
            if (can_see)
            {
                if (!is_visible)
                {
                    buf.clear();
                    count = pObj->BuildCreateUpdateBlockForPlayer(&buf, plr);
                    plr->PushCreationData(&buf, count);
                    plr->AddVisibleObject(pObj->GetGUID());
                }
            }
            else
            {
                if (is_visible)
                {
                    plr->SendDestroyObject(pObj->GetGUID());
                    plr->RemoveVisibleObject(pObj->GetGUID());
                }
            }

            if (pObj->IsPlayer())
            {
                pl = static_cast<Player*>(pObj);
                can_see = pl->CanSee(plr);
                is_visible = pl->IsVisible(plr->GetGUID());
                if (can_see)
                {
                    if (!is_visible)
                    {
                        buf.clear();
                        count = plr->BuildCreateUpdateBlockForPlayer(&buf, pl);
                        pl->PushCreationData(&buf, count);
                        pl->AddVisibleObject(plr->GetGUID());
                    }
                }
                else
                {
                    if (is_visible)
                    {
                        pl->SendDestroyObject(plr->GetGUID());
                        pl->RemoveVisibleObject(plr->GetGUID());
                    }
                }
            }
        }
    }
    else			// For units we can save a lot of work
    {
        for (std::set<Object*>::iterator it2 = GetInRangePlayerSetBegin(); it2 != GetInRangePlayerSetEnd(); ++it2)
        {

            Player* p = static_cast<Player*>(*it2);

            can_see = p->CanSee(this);
            is_visible = p->IsVisible(this->GetGUID());
            if (!can_see)
            {
                if (is_visible)
                {
                    p->SendDestroyObject(GetGUID());
                    p->RemoveVisibleObject(GetGUID());
                }
            }
            else
            {
                if (!is_visible)
                {
                    buf.clear();
                    count = BuildCreateUpdateBlockForPlayer(&buf, p);
                    p->PushCreationData(&buf, count);
                    p->AddVisibleObject(this->GetGUID());
                }
            }
        }
    }
}

void Unit::EventHealthChangeSinceLastUpdate()
{
    int pct = GetHealthPct();
    if (pct < 35)
    {
        uint32 toset = AURASTATE_FLAG_HEALTH35;
        if (pct < 20)
            toset |= AURASTATE_FLAG_HEALTH20;
        else
            RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH20);
        SetFlag(UNIT_FIELD_AURASTATE, toset);
    }
    else
        RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH35 | AURASTATE_FLAG_HEALTH20);

    if (pct < 75)
        RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH75);
    else
        SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_HEALTH75);
}

int32 Unit::GetAP()
{
    int32 baseap = GetAttackPower() + GetAttackPowerMods();
    float totalap = baseap * (getFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER) + 1);
    if (totalap >= 0)
        return float2int32(totalap);
    return	0;
}

int32 Unit::GetRAP()
{
    int32 baseap = GetRangedAttackPower() + GetRangedAttackPowerMods();
    float totalap = baseap * (GetRangedAttackPowerMultiplier() + 1);
    if (totalap >= 0)
        return float2int32(totalap);
    return	0;
}

bool Unit::GetSpeedDecrease()
{
    int32 before = m_speedModifier;
    m_speedModifier -= m_slowdown;
    m_slowdown = 0;
    std::map<uint32, int32>::iterator itr = speedReductionMap.begin();
    for (; itr != speedReductionMap.end(); ++itr)
        m_slowdown = (int32)std::min(m_slowdown, itr->second);

    if (m_slowdown < -100)
        m_slowdown = 100; //do not walk backwards !

    m_speedModifier += m_slowdown;
    //save bandwidth :P
    if (m_speedModifier != before)
        return true;
    return false;
}

void Unit::EventCastSpell(Unit* Target, SpellInfo* Sp)
{
    ARCEMU_ASSERT(Sp != NULL);
    Spell* pSpell = sSpellFactoryMgr.NewSpell(Target, Sp, true, NULL);
    SpellCastTargets targets(Target->GetGUID());
    pSpell->prepare(&targets);
}

void Unit::SetFacing(float newo)
{
    SetOrientation(newo);

    //generate smsg_monster_move
    WorldPacket data(SMSG_MONSTER_MOVE, 100);

    data << GetNewGUID();
    data << uint8(0); //vehicle seat index
    data << GetPositionX();
    data << GetPositionY();
    data << GetPositionZ();
    data <<Util::getMSTime();
    data << uint8(4); //set orientation
    data << newo;
    data << uint32(0x1000); //move flags: run
    data << uint32(0); //movetime
    data << uint32(1); //1 point
    data << GetPositionX();
    data << GetPositionY();
    data << GetPositionZ();

    SendMessageToSet(&data, true);
}

float Unit::get_chance_to_daze(Unit* target)
{
    if (target->getLevel() < CREATURE_DAZE_MIN_LEVEL) // since 3.3.0
        return 0.0f;
    float attack_skill = getLevel() * 5.0f;
    float defense_skill;
    if (target->IsPlayer())
        defense_skill = float(static_cast<Player*>(target)->_GetSkillLineCurrent(SKILL_DEFENSE, false));
    else
        defense_skill = target->getLevel() * 5.0f;

    if (!defense_skill)
        defense_skill = 1;
    float chance_to_daze = attack_skill * 20 / defense_skill;//if level is equal then we get a 20% chance to daze
    chance_to_daze = chance_to_daze * std::min(target->getLevel() / 30.0f, 1.0f); //for targets below level 30 the chance decreases
    if (chance_to_daze > 40)
        return 40.0f;
    else
        return chance_to_daze;
}

void CombatStatusHandler::ClearMyHealers()
{
    // this is where we check all our healers
    HealedSet::iterator i;
    Player* pt;
    for (i = m_healers.begin(); i != m_healers.end(); ++i)
    {
        pt = m_Unit->GetMapMgr()->GetPlayer(*i);
        if (pt != NULL)
            pt->CombatStatus.RemoveHealed(m_Unit);
    }

    m_healers.clear();
}

void CombatStatusHandler::RemoveHealed(Unit* pHealTarget)
{
    m_healed.erase(pHealTarget->GetLowGUID());
    UpdateFlag();
}

void CombatStatusHandler::UpdateFlag()
{
    bool n_status = InternalIsInCombat();
    if (n_status != m_lastStatus)
    {
        m_lastStatus = n_status;
        if (n_status)
        {
            //printf(I64FMT" is now in combat.\n", m_Unit->GetGUID());
            m_Unit->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);
            if (!m_Unit->hasUnitStateFlag(UNIT_STATE_ATTACKING)) m_Unit->addUnitStateFlag(UNIT_STATE_ATTACKING);
        }
        else
        {
            //printf(I64FMT" is no longer in combat.\n", m_Unit->GetGUID());
            m_Unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);
            if (m_Unit->hasUnitStateFlag(UNIT_STATE_ATTACKING)) m_Unit->removeUnitStateFlag(UNIT_STATE_ATTACKING);

            // remove any of our healers from combat too, if they are able to be.
            ClearMyHealers();

            if (m_Unit->IsPlayer())
                static_cast<Player*>(m_Unit)->UpdatePotionCooldown();
        }
    }
}

bool CombatStatusHandler::InternalIsInCombat()
{
    if (m_Unit->IsPlayer() && m_Unit->GetMapMgr() && m_Unit->GetMapMgr()->IsCombatInProgress())
        return true;

    if (m_healed.size() > 0)
        return true;

    if (m_attackTargets.size() > 0)
        return true;

    if (m_attackers.size() > 0)
        return true;

    return false;
}

void CombatStatusHandler::AddAttackTarget(const uint64 & guid)
{
    if (guid == m_Unit->GetGUID())
       return;

    //we MUST be in world
    ARCEMU_ASSERT(m_Unit->IsInWorld());

    m_attackTargets.insert(guid);
    //printf("Adding attack target " I64FMT " to " I64FMT "\n", guid, m_Unit->GetGUID());
    if (m_Unit->IsPlayer() &&
        m_primaryAttackTarget != guid)			// players can only have one attack target.
    {
        if (m_primaryAttackTarget)
            ClearPrimaryAttackTarget();

        m_primaryAttackTarget = guid;
    }

    UpdateFlag();
}

void CombatStatusHandler::ClearPrimaryAttackTarget()
{
    //printf("ClearPrimaryAttackTarget for " I64FMT "\n", m_Unit->GetGUID());
    if (m_primaryAttackTarget != 0)
    {
        Unit* pt = m_Unit->GetMapMgr()->GetUnit(m_primaryAttackTarget);
        if (pt != NULL)
        {
            // remove from their attacker set. (if we have no longer got any DoT's, etc)
            if (!IsAttacking(pt))
            {
                pt->CombatStatus.RemoveAttacker(m_Unit, m_Unit->GetGUID());
                m_attackTargets.erase(m_primaryAttackTarget);
            }

            m_primaryAttackTarget = 0;
        }
        else
        {
            m_attackTargets.erase(m_primaryAttackTarget);
            m_primaryAttackTarget = 0;
        }
    }

    UpdateFlag();
}

bool CombatStatusHandler::IsAttacking(Unit* pTarget)
{
    // check the target for any of our DoT's.
    for (uint32 i = MAX_NEGATIVE_AURAS_EXTEDED_START; i < MAX_NEGATIVE_AURAS_EXTEDED_END; ++i)
        if (pTarget->m_auras[i] != NULL)
            if (m_Unit->GetGUID() == pTarget->m_auras[i]->m_casterGuid && pTarget->m_auras[i]->IsCombatStateAffecting())
                return true;

    // place any additional checks here
    return false;
}

void CombatStatusHandler::RemoveAttackTarget(Unit* pTarget)
{
    // called on aura remove, etc.
    AttackerMap::iterator itr = m_attackTargets.find(pTarget->GetGUID());
    if (itr == m_attackTargets.end())
        return;

   if (!IsAttacking(pTarget))
    {
        //printf("Removing attack target " I64FMT " on " I64FMT "\n", pTarget->GetGUID(), m_Unit->GetGUID());
        m_attackTargets.erase(itr);
        if (m_primaryAttackTarget == pTarget->GetGUID())
            m_primaryAttackTarget = 0;

        UpdateFlag();
    }
    /*else
        printf("Cannot remove attack target " I64FMT " from " I64FMT "\n", pTarget->GetGUID(), m_Unit->GetGUID());*/
}

void CombatStatusHandler::RemoveAttacker(Unit* pAttacker, const uint64 & guid)
{
    AttackerMap::iterator itr = m_attackers.find(guid);
    if (itr == m_attackers.end())
        return;

    if ((!pAttacker) || (!pAttacker->CombatStatus.IsAttacking(m_Unit)))
    {
        //printf("Removing attacker " I64FMT " from " I64FMT "\n", guid, m_Unit->GetGUID());
        m_attackers.erase(itr);
        UpdateFlag();
    }
    /*else
    {
    printf("Cannot remove attacker " I64FMT " from " I64FMT "\n", guid, m_Unit->GetGUID());
    }*/
}

void CombatStatusHandler::OnDamageDealt(Unit* pTarget)
{
    // we added an aura, or dealt some damage to a target. they need to have us as an attacker, and they need to be our attack target if not.
    //printf("OnDamageDealt to " I64FMT " from " I64FMT "\n", pTarget->GetGUID(), m_Unit->GetGUID());
    if (pTarget == m_Unit)
        return;

    //no need to be in combat if dead
    if (!pTarget->isAlive() || !m_Unit->isAlive())
        return;

    AttackerMap::iterator itr = m_attackTargets.find(pTarget->GetGUID());
    if (itr == m_attackTargets.end())
        AddAttackTarget(pTarget->GetGUID());

    itr = pTarget->CombatStatus.m_attackers.find(m_Unit->GetGUID());
    if (itr == pTarget->CombatStatus.m_attackers.end())
        pTarget->CombatStatus.AddAttacker(m_Unit->GetGUID());

    // update the timeout
    m_Unit->CombatStatusHandler_ResetPvPTimeout();
}

void CombatStatusHandler::AddAttacker(const uint64 & guid)
{
    //we MUST be in world
    ARCEMU_ASSERT(m_Unit->IsInWorld());
    m_attackers.insert(guid);
    UpdateFlag();
}

void CombatStatusHandler::ClearAttackers()
{
    //If we are not in world, CombatStatusHandler::OnRemoveFromWorld() would have been already called so m_attackTargets
    //and m_attackers should be empty. If it's not, something wrong happened.

    // this is a FORCED function, only use when the reference will be destroyed.
    AttackerMap::iterator itr = m_attackTargets.begin();
    Unit* pt;
    for (; itr != m_attackTargets.end(); ++itr)
    {
        pt = m_Unit->GetMapMgr()->GetUnit(*itr);
        if (pt)
        {
            pt->CombatStatus.m_attackers.erase(m_Unit->GetGUID());
            pt->CombatStatus.UpdateFlag();
        }
    }

    for (itr = m_attackers.begin(); itr != m_attackers.end(); ++itr)
    {
        pt = m_Unit->GetMapMgr()->GetUnit(*itr);
        if (pt)
        {
            pt->CombatStatus.m_attackTargets.erase(m_Unit->GetGUID());
            pt->CombatStatus.UpdateFlag();
        }
    }

    m_attackers.clear();
    m_attackTargets.clear();
    m_primaryAttackTarget = 0;
    UpdateFlag();
}

void CombatStatusHandler::ClearHealers()
{
    //If we are not in world, CombatStatusHandler::OnRemoveFromWorld() would have been already called so m_healed should
    //be empty. If it's not, something wrong happened.

    HealedSet::iterator itr = m_healed.begin();
    Player* pt;
    for (; itr != m_healed.end(); ++itr)
    {
        pt = m_Unit->GetMapMgr()->GetPlayer(*itr);
        if (pt)
        {
            pt->CombatStatus.m_healers.erase(m_Unit->GetLowGUID());
            pt->CombatStatus.UpdateFlag();
        }
    }

    for (itr = m_healers.begin(); itr != m_healers.end(); ++itr)
    {
        pt = m_Unit->GetMapMgr()->GetPlayer(*itr);
        if (pt)
        {
            pt->CombatStatus.m_healed.erase(m_Unit->GetLowGUID());
            pt->CombatStatus.UpdateFlag();
        }
    }

    m_healed.clear();
    m_healers.clear();
    UpdateFlag();
}

void Unit::CombatStatusHandler_ResetPvPTimeout()
{
    if (!IsPlayer())
        return;

    m_lock.Acquire();
    EventMap::iterator itr = m_events.find(EVENT_ATTACK_TIMEOUT);
    if (itr != m_events.end())
    {
        for (; itr != m_events.upper_bound(EVENT_ATTACK_TIMEOUT); ++itr)
        {
            if (!itr->second->deleted)
            {
                itr->second->currTime = 5000;
                m_lock.Release();
                return;
            }
        }
    }

    sEventMgr.AddEvent(this, &Unit::CombatStatusHandler_UpdatePvPTimeout, EVENT_ATTACK_TIMEOUT, 5000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    m_lock.Release();
}

void Unit::CombatStatusHandler_UpdatePvPTimeout()
{
    CombatStatus.TryToClearAttackTargets();
}

void CombatStatusHandler::TryToClearAttackTargets()
{
    AttackerMap::iterator i, i2;
    Unit* pt;

    if (m_Unit->IsPlayer())
        static_cast<Player*>(m_Unit)->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_CONT_PVP);

    for (i = m_attackTargets.begin(); i != m_attackTargets.end();)
    {
        i2 = i++;
        pt = m_Unit->GetMapMgr()->GetUnit(*i2);
        if (pt == NULL)
        {
            m_attackTargets.erase(i2);
            continue;
        }

        RemoveAttackTarget(pt);
        pt->CombatStatus.RemoveAttacker(m_Unit, m_Unit->GetGUID());
    }
}

void CombatStatusHandler::AttackersForgetHate()
{
    AttackerMap::iterator i, i2;
    Unit* pt;

    for (i = m_attackTargets.begin(); i != m_attackTargets.end();)
    {
        i2 = i++;
        pt = m_Unit->GetMapMgr()->GetUnit(*i2);
        if (pt == NULL)
        {
            m_attackTargets.erase(i2);
            continue;
        }

        if (pt->GetAIInterface())
            pt->GetAIInterface()->RemoveThreatByPtr(m_Unit);
    }
}

bool CombatStatusHandler::IsInCombat() const
{
    // If the unit doesn't exist - OR - the unit exists but is not in world
    if (m_Unit == NULL || !m_Unit->IsInWorld())
        return false;

    switch (m_Unit->GetTypeId())
    {
        case TYPEID_UNIT:
        {
            if (m_Unit->IsPet() && static_cast<Pet*>(m_Unit)->GetPetAction() == PET_ACTION_ATTACK)
                return true;
            else if (m_Unit->IsPet())
                return m_lastStatus;
            else
                return m_Unit->GetAIInterface()->getAITargetsCount() == 0 ? false : true;
        }
        break;
        case TYPEID_PLAYER:
        {
            std::list<Pet*> summons = static_cast<Player*>(m_Unit)->GetSummons();
            for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
            {
                if ((*itr)->GetPetOwner() == m_Unit && (*itr)->CombatStatus.IsInCombat())
                    return true;
            }

            return m_lastStatus;
        }
        break;
        default:
            return false;
    }
}

void CombatStatusHandler::WeHealed(Unit* pHealTarget)
{
    if (!pHealTarget->IsPlayer() || !m_Unit->IsPlayer() || pHealTarget == m_Unit)
        return;

    if (pHealTarget->CombatStatus.IsInCombat())
    {
        m_healed.insert(pHealTarget->GetLowGUID());
        pHealTarget->CombatStatus.m_healers.insert(m_Unit->GetLowGUID());
    }

    UpdateFlag();
}

void CombatStatusHandler::OnRemoveFromWorld()
{
    ClearAttackers();
    ClearHealers();
}

void Unit::Heal(Unit* target, uint32 SpellId, uint32 amount)
{
    if (!target || !SpellId || !amount)
        return;

    uint32 ch = target->GetHealth();
    uint32 mh = target->GetMaxHealth();
    if (mh != ch)
    {
        ch += amount;
        uint32 overheal = 0;

        if (ch > mh)
        {
            target->SetHealth(mh);
            overheal = amount - mh;
            amount += (mh - ch);
        }
        else
            target->SetHealth(ch);

        Spell::SendHealSpellOnPlayer(this, target, amount, false, overheal, SpellId);

        target->RemoveAurasByHeal();
    }
}

void Unit::Energize(Unit* target, uint32 SpellId, uint32 amount, uint32 type)
{
    //Static energize
    if (!target || !SpellId || !amount)
        return;

    uint32 cur = target->GetPower(POWER_TYPE_MANA + type);
    uint32 max = target->GetMaxPower(POWER_TYPE_MANA + type);

    if (cur + amount > max)
        amount = max - cur;

    target->SetPower(POWER_TYPE_MANA + type, cur + amount);

    Spell::SendHealManaSpellOnPlayer(this, target, amount, type, SpellId);
}

void Unit::InheritSMMods(Unit* inherit_from)
{
    if (inherit_from == NULL)
        return;

    if (inherit_from->SM_CriticalChance)
    {
        if (SM_CriticalChance == 0)
            SM_CriticalChance = new int32[SPELL_GROUPS];
        memcpy(SM_CriticalChance, inherit_from->SM_CriticalChance, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FDur)
    {
        if (SM_FDur == 0)
            SM_FDur = new int32[SPELL_GROUPS];
        memcpy(SM_FDur, inherit_from->SM_FDur, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PDur)
    {
        if (SM_PDur == 0)
            SM_PDur = new int32[SPELL_GROUPS];
        memcpy(SM_PDur, inherit_from->SM_PDur, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PRadius)
    {
        if (SM_PRadius == 0)
            SM_PRadius = new int32[SPELL_GROUPS];
        memcpy(SM_PRadius, inherit_from->SM_PRadius, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FRadius)
    {
        if (SM_FRadius == 0)
            SM_FRadius = new int32[SPELL_GROUPS];
        memcpy(SM_FRadius, inherit_from->SM_FRadius, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FRange)
    {
        if (SM_FRange == 0)
            SM_FRange = new int32[SPELL_GROUPS];
        memcpy(SM_FRange, inherit_from->SM_FRange, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PCastTime)
    {
        if (SM_PCastTime == 0)
            SM_PCastTime = new int32[SPELL_GROUPS];
        memcpy(SM_PCastTime, inherit_from->SM_PCastTime, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FCastTime)
    {
        if (SM_FCastTime == 0)
            SM_FCastTime = new int32[SPELL_GROUPS];
        memcpy(SM_FCastTime, inherit_from->SM_FCastTime, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PCriticalDamage)
    {
        if (SM_PCriticalDamage == 0)
            SM_PCriticalDamage = new int32[SPELL_GROUPS];
        memcpy(SM_PCriticalDamage, inherit_from->SM_PCriticalDamage, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FDOT)
    {
        if (SM_FDOT == 0)
            SM_FDOT = new int32[SPELL_GROUPS];
        memcpy(SM_FDOT, inherit_from->SM_FDOT, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PDOT)
    {
        if (SM_PDOT == 0)
            SM_PDOT = new int32[SPELL_GROUPS];
        memcpy(SM_PDOT, inherit_from->SM_PDOT, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FEffect1_Bonus)
    {
        if (SM_FEffect1_Bonus == 0)
            SM_FEffect1_Bonus = new int32[SPELL_GROUPS];
        memcpy(SM_FEffect1_Bonus, inherit_from->SM_FEffect1_Bonus, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PEffect1_Bonus)
    {
        if (SM_PEffect1_Bonus == 0)
            SM_PEffect1_Bonus = new int32[SPELL_GROUPS];
        memcpy(SM_PEffect1_Bonus, inherit_from->SM_PEffect1_Bonus, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FEffect2_Bonus)
    {
        if (SM_FEffect2_Bonus == 0)
            SM_FEffect2_Bonus = new int32[SPELL_GROUPS];
        memcpy(SM_FEffect2_Bonus, inherit_from->SM_FEffect2_Bonus, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PEffect2_Bonus)
    {
        if (SM_PEffect2_Bonus == 0)
            SM_PEffect2_Bonus = new int32[SPELL_GROUPS];
        memcpy(SM_PEffect2_Bonus, inherit_from->SM_PEffect2_Bonus, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FEffect3_Bonus)
    {
        if (SM_FEffect3_Bonus == 0)
            SM_FEffect3_Bonus = new int32[SPELL_GROUPS];
        memcpy(SM_FEffect3_Bonus, inherit_from->SM_FEffect3_Bonus, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PEffect3_Bonus)
    {
        if (SM_PEffect3_Bonus == 0)
            SM_PEffect3_Bonus = new int32[SPELL_GROUPS];
        memcpy(SM_PEffect3_Bonus, inherit_from->SM_PEffect3_Bonus, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FEffectBonus)
    {
        if (SM_FEffectBonus == 0)
            SM_FEffectBonus = new int32[SPELL_GROUPS];
        memcpy(SM_FEffectBonus, inherit_from->SM_FEffectBonus, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PEffectBonus)
    {
        if (SM_PEffectBonus == 0)
            SM_PEffectBonus = new int32[SPELL_GROUPS];
        memcpy(SM_PEffectBonus, inherit_from->SM_PEffectBonus, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FDamageBonus)
    {
        if (SM_FDamageBonus == 0)
            SM_FDamageBonus = new int32[SPELL_GROUPS];
        memcpy(SM_FDamageBonus, inherit_from->SM_FDamageBonus, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PDamageBonus)
    {
        if (SM_PDamageBonus == 0)
            SM_PDamageBonus = new int32[SPELL_GROUPS];
        memcpy(SM_PDamageBonus, inherit_from->SM_PDamageBonus, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PMiscEffect)
    {
        if (SM_PMiscEffect == 0)
            SM_PMiscEffect = new int32[SPELL_GROUPS];
        memcpy(SM_PMiscEffect, inherit_from->SM_PMiscEffect, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FMiscEffect)
    {
        if (SM_FMiscEffect == 0)
            SM_FMiscEffect = new int32[SPELL_GROUPS];
        memcpy(SM_FMiscEffect, inherit_from->SM_FMiscEffect, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FHitchance)
    {
        if (SM_FHitchance == 0)
            SM_FHitchance = new int32[SPELL_GROUPS];
        memcpy(SM_FHitchance, inherit_from->SM_FHitchance, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PRange)
    {
        if (SM_PRange == 0)
            SM_PRange = new int32[SPELL_GROUPS];
        memcpy(SM_PRange, inherit_from->SM_PRange, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PRadius)
    {
        if (SM_PRadius == 0)
            SM_PRadius = new int32[SPELL_GROUPS];
        memcpy(SM_PRadius, inherit_from->SM_PRadius, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PCost)
    {
        if (SM_PCost == 0)
            SM_PCost = new int32[SPELL_GROUPS];
        memcpy(SM_PCost, inherit_from->SM_PCost, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FCost)
    {
        if (SM_FCost == 0)
            SM_FCost = new int32[SPELL_GROUPS];
        memcpy(SM_FCost, inherit_from->SM_FCost, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FAdditionalTargets)
    {
        if (SM_FAdditionalTargets == 0)
            SM_FAdditionalTargets = new int32[SPELL_GROUPS];
        memcpy(SM_FAdditionalTargets, inherit_from->SM_FAdditionalTargets, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PJumpReduce)
    {
        if (SM_PJumpReduce == 0)
            SM_PJumpReduce = new int32[SPELL_GROUPS];
        memcpy(SM_PJumpReduce, inherit_from->SM_PJumpReduce, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FGlobalCooldown)
    {
        if (SM_FGlobalCooldown == 0)
            SM_FGlobalCooldown = new int32[SPELL_GROUPS];
        memcpy(SM_FGlobalCooldown, inherit_from->SM_FGlobalCooldown, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PGlobalCooldown)
    {
        if (SM_PGlobalCooldown == 0)
            SM_PGlobalCooldown = new int32[SPELL_GROUPS];
        memcpy(SM_PGlobalCooldown, inherit_from->SM_PGlobalCooldown, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PNonInterrupt)
    {
        if (SM_PNonInterrupt == 0)
            SM_PNonInterrupt = new int32[SPELL_GROUPS];
        memcpy(SM_PNonInterrupt, inherit_from->SM_PNonInterrupt, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FPenalty)
    {
        if (SM_FPenalty == 0)
            SM_FPenalty = new int32[SPELL_GROUPS];
        memcpy(SM_FPenalty, inherit_from->SM_FPenalty, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PPenalty)
    {
        if (SM_PPenalty == 0)
            SM_PPenalty = new int32[SPELL_GROUPS];
        memcpy(SM_PPenalty, inherit_from->SM_PPenalty, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FCooldownTime)
    {
        if (SM_FCooldownTime == 0)
            SM_FCooldownTime = new int32[SPELL_GROUPS];
        memcpy(SM_FCooldownTime, inherit_from->SM_FCooldownTime, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PCooldownTime)
    {
        if (SM_PCooldownTime == 0)
            SM_PCooldownTime = new int32[SPELL_GROUPS];
        memcpy(SM_PCooldownTime, inherit_from->SM_PCooldownTime, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FChanceOfSuccess)
    {
        if (SM_FChanceOfSuccess == 0)
            SM_FChanceOfSuccess = new int32[SPELL_GROUPS];
        memcpy(SM_FChanceOfSuccess, inherit_from->SM_FChanceOfSuccess, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FAmptitude)
    {
        if (SM_FAmptitude == 0)
            SM_FAmptitude = new int32[SPELL_GROUPS];
        memcpy(SM_FAmptitude, inherit_from->SM_FAmptitude, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PAmptitude)
    {
        if (SM_PAmptitude == 0)
            SM_PAmptitude = new int32[SPELL_GROUPS];
        memcpy(SM_PAmptitude, inherit_from->SM_PAmptitude, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FRezist_dispell)
    {
        if (SM_FRezist_dispell == 0)
            SM_FRezist_dispell = new int32[SPELL_GROUPS];
        memcpy(SM_FRezist_dispell, inherit_from->SM_FRezist_dispell, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PRezist_dispell)
    {
        if (SM_PRezist_dispell == 0)
            SM_PRezist_dispell = new int32[SPELL_GROUPS];
        memcpy(SM_PRezist_dispell, inherit_from->SM_PRezist_dispell, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FCharges)
    {
        if (SM_FCharges == 0)
            SM_FCharges = new int32[SPELL_GROUPS];
        memcpy(SM_FCharges, inherit_from->SM_FCharges, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PCharges)
    {
        if (SM_PCharges == 0)
            SM_PCharges = new int32[SPELL_GROUPS];
        memcpy(SM_PCharges, inherit_from->SM_PCharges, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_FThreat)
    {
        if (SM_FThreat == 0)
            SM_FThreat = new int32[SPELL_GROUPS];
        memcpy(SM_FThreat, inherit_from->SM_FThreat, sizeof(int)*SPELL_GROUPS);
    }
    if (inherit_from->SM_PThreat)
    {
        if (SM_PThreat == 0)
            SM_PThreat = new int32[SPELL_GROUPS];
        memcpy(SM_PThreat, inherit_from->SM_PThreat, sizeof(int)*SPELL_GROUPS);
    }
}

void Unit::CancelSpell(Spell* ptr)
{
    /*
        if (ptr)
        ptr->cancel();
        else */
    if (m_currentSpell)
    {
        // this logically might seem a little bit twisted
        // crash situation : an already deleted spell will be called to get canceled by eventmanager
        // solution : We should not delay spell canceling more then second spell canceling.
        // problem : might remove spells that should not be removed. Not sure about it :(
        sEventMgr.RemoveEvents(this, EVENT_UNIT_DELAYED_SPELL_CANCEL);
        m_currentSpell->cancel();
    }
}

void Unit::EventStopChanneling(bool abort)
{
    auto spell = GetCurrentSpell();

    if (spell == nullptr)
        return;

    spell->SendChannelUpdate(0);
    spell->finish(abort);
}

void Unit::EventStrikeWithAbility(uint64 guid, SpellInfo* sp, uint32 damage)
{
    Unit* victim = m_mapMgr ? m_mapMgr->GetUnit(guid) : NULL;
    if (victim)
        Strike(victim, RANGED, sp, 0, 0, 0, false, true);
}

void Unit::DispelAll(bool positive)
{
    for (uint32 i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        if (m_auras[i] != NULL)
            if ((m_auras[i]->IsPositive() && positive) || !m_auras[i]->IsPositive())
                m_auras[i]->Remove();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// bool Unit::RemoveAllAurasByMechanic (renamed from MechanicImmunityMassDispel)
/// Removes all auras on this unit that are of a specific mechanic.
/// Useful for things like.. Apply Aura: Immune Mechanic, where existing (de)buffs are *always supposed* to be removed.
/// I'm not sure if this goes here under unit.
///
/// \param uint32 MechanicType
/// \return False if no buffs were dispelled, true if more than 0 were dispelled.
//////////////////////////////////////////////////////////////////////////////////////////
bool Unit::RemoveAllAurasByMechanic(uint32 MechanicType, uint32 MaxDispel = -1, bool HostileOnly = true)
{
    LogDebugFlag(LF_AURA, "Unit::MechanicImmunityMassDispel called, mechanic: %u" , MechanicType);
    uint32 DispelCount = 0;
    for (uint32 x = (HostileOnly ? MAX_NEGATIVE_AURAS_EXTEDED_START : MAX_POSITIVE_AURAS_EXTEDED_START); x < MAX_REMOVABLE_AURAS_END; x++)    // If HostileOnly = 1, then we use aura slots 40-56 (hostile). Otherwise, we use 0-56 (all)
    {
        if (DispelCount >= MaxDispel && MaxDispel > 0)
            return true;

        if (m_auras[x])
        {
            if (m_auras[x]->GetSpellInfo()->getMechanicsType() == MechanicType)   // Remove all mechanics of type MechanicType (my english goen boom)
            {
                LogDebugFlag(LF_AURA, "Removed aura. [AuraSlot %u, SpellId %u]", x, m_auras[x]->GetSpellId());
                ///\todo Stop moving if fear was removed.
                m_auras[x]->Remove(); // EZ-Remove
                DispelCount++;
            }
            else if (MechanicType == MECHANIC_ENSNARED)   // if got immunity for slow, remove some that are not in the mechanics
            {
                for (uint8 i = 0; i < 3; i++)
                {
                    // SNARE + ROOT
                    if (m_auras[x]->GetSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_MOD_DECREASE_SPEED || m_auras[x]->GetSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_MOD_ROOT)
                    {
                        m_auras[x]->Remove();
                        break;
                    }
                }
            }
        }
    }
    return (DispelCount == 0);
}

void Unit::RemoveAllMovementImpairing()
{
    for (uint32 x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_REMOVABLE_AURAS_END; x++)
    {
        if (m_auras[x] != NULL)
        {
            if (m_auras[x]->GetSpellInfo()->getMechanicsType() == MECHANIC_ROOTED
                || m_auras[x]->GetSpellInfo()->getMechanicsType() == MECHANIC_ENSNARED
                || m_auras[x]->GetSpellInfo()->getMechanicsType() == MECHANIC_DAZED)

            {
                m_auras[x]->Remove();
            }
            else
            {
                for (uint8 i = 0; i < 3; i++)
                {
                    if (m_auras[x]->GetSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_MOD_DECREASE_SPEED
                        || m_auras[x]->GetSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_MOD_ROOT)
                    {
                        m_auras[x]->Remove();
                        break;
                    }
                }
            }
        }
    }
}

void Unit::setAttackTimer(int32 time, bool offhand)
{
    if (!time)
        time = offhand ? m_uint32Values[UNIT_FIELD_BASEATTACKTIME + 1] : m_uint32Values[UNIT_FIELD_BASEATTACKTIME];

    time = std::max(1000, float2int32(time * GetCastSpeedMod()));
    if (time> 300000)		// just in case.. shouldn't happen though
        time = offhand ? m_uint32Values[UNIT_FIELD_BASEATTACKTIME + 1] : m_uint32Values[UNIT_FIELD_BASEATTACKTIME];

    if (offhand)
        m_attackTimer_1 = Util::getMSTime() + time;
    else
        m_attackTimer = Util::getMSTime() + time;
}

bool Unit::isAttackReady(bool offhand)
{
    if (offhand)
        return (Util::getMSTime() >= m_attackTimer_1) ? true : false;
    else
        return (Util::getMSTime() >= m_attackTimer) ? true : false;
}

void Unit::ReplaceAIInterface(AIInterface* new_interface)
{
    delete m_aiInterface;	//be careful when you do this. Might screw unit states !
    m_aiInterface = new_interface;
}

void Unit::EventUpdateFlag()
{
    CombatStatus.UpdateFlag();
}

void Unit::EventModelChange()
{
    MySQLStructure::DisplayBoundingBoxes const* displayBoundingBox = sMySQLStore.getDisplayBounding(getUInt32Value(UNIT_FIELD_DISPLAYID));

    //\todo if has mount, grab mount model and add the z value of attachment 0
    if (displayBoundingBox != nullptr)
    {
        m_modelhalfsize = displayBoundingBox->high[2] / 2;
    }
    else
    {
        m_modelhalfsize = 1.0f;
    }
}

void Unit::RemoveFieldSummon()
{
    uint64 guid = GetSummonedUnitGUID();
    if (guid && GetMapMgr())
    {
        Creature* summon = static_cast<Creature*>(GetMapMgr()->GetUnit(guid));
        if (summon)
        {
            summon->RemoveFromWorld(false, true);
        }
        SetSummonedUnitGUID(0);
    }
}

void Unit::AggroPvPGuards()
{
    Unit* tmpUnit;
    for (Object::InRangeSet::iterator i = GetInRangeSetBegin(); i != GetInRangeSetEnd(); ++i)
    {
        if ((*i)->IsCreature())
        {
            tmpUnit = static_cast<Unit*>(*i);
            if (tmpUnit->GetAIInterface() && tmpUnit->GetAIInterface()->m_isNeutralGuard && CalcDistance(tmpUnit) <= (50.0f * 50.0f))
            {
                tmpUnit->GetAIInterface()->AttackReaction(this, 1, 0);
            }
        }
    }
}

//what is an Immobilize spell ? Have to add it later to spell effect handler
void Unit::EventStunOrImmobilize(Unit* proc_target, bool is_victim)
{
    if (this == proc_target)
        return; //how and why would we stun ourselves

    int32 t_trigger_on_stun, t_trigger_on_stun_chance;
    if (is_victim == false)
    {
        t_trigger_on_stun = trigger_on_stun;
        t_trigger_on_stun_chance = trigger_on_stun_chance;
    }
    else
    {
        t_trigger_on_stun = trigger_on_stun_victim;
        t_trigger_on_stun_chance = trigger_on_stun_chance_victim;
    }

    if (t_trigger_on_stun)
    {
        if (t_trigger_on_stun_chance < 100 && !Rand(t_trigger_on_stun_chance))
            return;

        SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(t_trigger_on_stun);

        if (!spellInfo)
            return;

        Spell* spell = sSpellFactoryMgr.NewSpell(this, spellInfo, true, NULL);
        SpellCastTargets targets;

        if (spellInfo->getProcFlags() & PROC_TARGET_SELF)
            targets.m_unitTarget = GetGUID();
        else if (proc_target)
            targets.m_unitTarget = proc_target->GetGUID();
        else
            targets.m_unitTarget = GetGUID();
        spell->prepare(&targets);
    }
}

// Proc on chill effects (such as frostbolt slow effect)
void Unit::EventChill(Unit* proc_target, bool is_victim)
{
    if (this == proc_target)
        return; //how and why would we chill ourselves

    int32 t_trigger_on_chill, t_trigger_on_chill_chance;
    if (is_victim == false)
    {
        t_trigger_on_chill = trigger_on_chill;
        t_trigger_on_chill_chance = trigger_on_chill_chance;
    }
    else
    {
        t_trigger_on_chill = trigger_on_chill_victim;
        t_trigger_on_chill_chance = trigger_on_chill_chance_victim;
    }

    if (t_trigger_on_chill)
    {
        if (t_trigger_on_chill_chance < 100 && !Rand(t_trigger_on_chill_chance))
            return;

        SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(t_trigger_on_chill);

        if (!spellInfo)
            return;

        Spell* spell = sSpellFactoryMgr.NewSpell(this, spellInfo, true, NULL);
        SpellCastTargets targets;

        if (spellInfo->getProcFlags() & PROC_TARGET_SELF)
            targets.m_unitTarget = GetGUID();
        else if (proc_target)
            targets.m_unitTarget = proc_target->GetGUID();
        else
            targets.m_unitTarget = GetGUID();
        spell->prepare(&targets);
    }
}

void Unit::RemoveExtraStrikeTarget(SpellInfo* spell_info)
{
    ExtraStrike* es;
    for (std::list<ExtraStrike*>::iterator i = m_extraStrikeTargets.begin(); i != m_extraStrikeTargets.end(); ++i)
    {
        es = *i;
        if (spell_info == es->spell_info)
        {
            m_extrastriketargetc--;
            m_extraStrikeTargets.erase(i);
            delete es;
            break;
        }
    }
}

void Unit::AddExtraStrikeTarget(SpellInfo* spell_info, uint32 charges)
{
    for (std::list<ExtraStrike*>::iterator i = m_extraStrikeTargets.begin(); i != m_extraStrikeTargets.end(); ++i)
    {
        //a pointer check or id check ...should be the same
        if (spell_info == (*i)->spell_info)
        {
            (*i)->charges = charges;
            return;
        }
    }

    ExtraStrike* es = new ExtraStrike;

    es->spell_info = spell_info;
    es->charges = charges;
    m_extraStrikeTargets.push_back(es);
    m_extrastriketargetc++;
}

uint32 Unit::DoDamageSplitTarget(uint32 res, uint32 school_type, bool melee_dmg)
{
    Unit* splittarget;
    uint32 splitdamage, tmpsplit;
    DamageSplitTarget* ds = m_damageSplitTarget;

    splittarget = (GetMapMgr() != NULL) ? GetMapMgr()->GetUnit(ds->m_target) : NULL;
    if (splittarget != NULL && res > 0)
    {
        // calculate damage
        tmpsplit = ds->m_flatDamageSplit;
        if (tmpsplit > res)
            tmpsplit = res; // prevent <0 damage
        splitdamage = tmpsplit;
        res -= tmpsplit;
        tmpsplit = float2int32(ds->m_pctDamageSplit * res);
        if (tmpsplit > res)
            tmpsplit = res;
        splitdamage += tmpsplit;
        res -= tmpsplit;

        if (splitdamage)
        {
            splittarget->DealDamage(splittarget, splitdamage, 0, 0, 0);

            // Send damage log
            if (melee_dmg)
            {
                dealdamage sdmg;

                sdmg.full_damage = splitdamage;
                sdmg.resisted_damage = 0;
                sdmg.school_type = school_type;
                SendAttackerStateUpdate(this, splittarget, &sdmg, splitdamage, 0, 0, 0, ATTACK);
            }
            else
            {
                SendSpellNonMeleeDamageLog(this, splittarget, ds->m_spellId, splitdamage, static_cast<uint8>(school_type), 0, 0, true, 0, 0, true);
            }
        }
    }

    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////
///Removes and deletes reflects from unit by spell id, does not remove aura which created it
///In specific cases reflects can be created by a dummy spelleffect (eg. spell 28332 or 13043), then we need to remove it in ~unit
//////////////////////////////////////////////////////////////////////////////////////////
void Unit::RemoveReflect(uint32 spellid, bool apply)
{
    for (std::list<struct ReflectSpellSchool*>::iterator i = m_reflectSpellSchool.begin(); i != m_reflectSpellSchool.end();)
        if (spellid == (*i)->spellId)
        {
            delete *i;
            i = m_reflectSpellSchool.erase(i);
            //break; better check all list elements
        }
        else
            ++i;


    if (apply && spellid == 23920 && IsPlayer())
    {
        uint32 improvedSpellReflection[] =
        {
            //SPELL_HASH_IMPROVED_SPELL_REFLECTION
            59088,
            59089,
            0
        };

        if (hasAurasWithId(improvedSpellReflection))
        {
            Player* pPlayer = static_cast<Player*>(this);
            Group* pGroup = pPlayer->GetGroup();

            if (pGroup != NULL)
            {
                int32 targets = 0;
                if (pPlayer->HasAura(59088))
                    targets = 2;
                else if (pPlayer->HasAura(59089))
                    targets = 4;

                pGroup->Lock();
                for (uint32 i = 0; i < pGroup->GetSubGroupCount(); ++i)
                {
                    SubGroup* subGroup = pGroup->GetSubGroup(i);
                    for (GroupMembersSet::iterator itr = subGroup->GetGroupMembersBegin(); itr != subGroup->GetGroupMembersEnd() && targets > 0; ++itr)
                    {
                        Player* member = (*itr)->m_loggedInPlayer;
                        if (member == NULL || member == pPlayer || !member->IsInWorld() || !member->isAlive() || member->HasAura(59725))
                            continue;

                        if (!member->isInRange(pPlayer, 20))
                            continue;
                        pPlayer->CastSpell(member, 59725, true);
                        targets -= 1;
                    }
                }
                pGroup->Unlock();
            }
        }
    }

    if (!apply && spellid == 59725 && IsPlayer())
    {
        Player* pPlayer = static_cast<Player*>(this);
        Group* pGroup = pPlayer->GetGroup();

        if (pGroup != NULL)
        {
            pGroup->Lock();
            for (uint32 i = 0; i < pGroup->GetSubGroupCount(); ++i)
            {
                for (GroupMembersSet::iterator itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); ++itr)
                {
                    Player* member = (*itr)->m_loggedInPlayer;
                    if (member == NULL)
                        continue;

                    member->RemoveAura(59725);
                }
            }
            pGroup->Unlock();
        }
    }
}

void Unit::SetPower(uint32 type, int32 value)
{
    uint32 maxpower = getUInt32Value(UNIT_FIELD_MAXPOWER1 + type);

    if (value < 0)
        value = 0;
    else if (value > (int32)maxpower)
        value = maxpower;

    setUInt32Value(UNIT_FIELD_POWER1 + type, value);
}

void Unit::SendPowerUpdate(bool self)
{
#if VERSION_STRING == Cata
    uint32 amount = getUInt32Value(UNIT_FIELD_POWER1 + GetPowerType()); //save the amount, so we send the same to the player and everyone else

    WorldPacket data(SMSG_POWER_UPDATE, 14);
    FastGUIDPack(data, GetGUID());
    data << uint32(1);
    data << uint8(GetPowerType());
    data << int32(amount);
    // \todo This was added in revision 1726.  Is it necessary?  To me, it seems to just be sending the packet twice.
    //	If it is needed for something, put it back in I guess.
    //	CopyAndSendDelayedPacket(&data);
    SendMessageToSet(&data, IsPlayer());

    //VLack: On 3.1.3, create and send a field update packet to everyone else, as this is the only way to update their GUI with the power values.
    WorldPacket* pkt = BuildFieldUpdatePacket(UNIT_FIELD_MAXPOWER1 + GetPowerType(), amount);
    SendMessageToSet(pkt, false);
    delete pkt;
#else
#if VERSION_STRING > TBC
    uint32 amount = getUInt32Value(UNIT_FIELD_POWER1 + GetPowerType()); //save the amount, so we send the same to the player and everyone else

    WorldPacket data(SMSG_POWER_UPDATE, 14);
    FastGUIDPack(data, GetGUID());
    data << (uint8)GetPowerType();
    data << amount;
    // \todo This was added in revision 1726.  Is it necessary?  To me, it seems to just be sending the packet twice.
    //	If it is needed for something, put it back in I guess.
    //	CopyAndSendDelayedPacket(&data);
    SendMessageToSet(&data, self);

    //VLack: On 3.1.3, create and send a field update packet to everyone else, as this is the only way to update their GUI with the power values.
    WorldPacket* pkt = BuildFieldUpdatePacket(UNIT_FIELD_POWER1 + GetPowerType(), amount);
    SendMessageToSet(pkt, false);
    delete pkt;
#endif
#endif
}

void Unit::UpdatePowerAmm()
{
#if VERSION_STRING > TBC
    if (!IsPlayer())
        return;
    WorldPacket data(SMSG_POWER_UPDATE, 14);
    FastGUIDPack(data, GetGUID());
    data << uint8(GetPowerType());
    data << getUInt32Value(UNIT_FIELD_POWER1 + GetPowerType());
    SendMessageToSet(&data, true);
#endif
}

void Unit::SetDualWield(bool enabled)
{
    m_dualWield = enabled;

    // Titan's grip
    if (!enabled && IsPlayer())
        removeAllAurasById(49152);
}

void Unit::AddGarbageAura(Aura* aur)
{
    m_GarbageAuras.push_back(aur);
}

void Unit::AddGarbageSpell(Spell* sp)
{
    m_GarbageSpells.push_back(sp);
}

void Unit::AddGarbagePet(Pet* pet)
{
    ARCEMU_ASSERT(pet->GetPetOwner()->GetGUID() == GetGUID() && !pet->IsInWorld());
    m_GarbagePets.push_back(pet);
}

void Unit::RemoveGarbage()
{
    std::list<Aura*>::iterator itr1;
    for (itr1 = m_GarbageAuras.begin(); itr1 != m_GarbageAuras.end(); ++itr1)
    {
        Aura* aur = *itr1;

        delete aur;
    }

    std::list<Spell*>::iterator itr2;
    for (itr2 = m_GarbageSpells.begin(); itr2 != m_GarbageSpells.end(); ++itr2)
    {
        Spell* sp = *itr2;

        delete sp;
    }

    std::list<Pet*>::iterator itr3;
    for (itr3 = m_GarbagePets.begin(); itr3 != m_GarbagePets.end(); ++itr3)
    {
        Pet* pet = *itr3;

        delete pet;
    }

    m_GarbageAuras.clear();
    m_GarbageSpells.clear();
    m_GarbagePets.clear();
}

void Unit::Tag(uint64 TaggerGUID)
{
    Tagged = true;
    this->TaggerGuid = TaggerGUID;
    m_uint32Values[UNIT_DYNAMIC_FLAGS] |= U_DYN_FLAG_TAGGED_BY_OTHER;
}

void Unit::UnTag()
{
    Tagged = false;
    TaggerGuid = 0;
    m_uint32Values[UNIT_DYNAMIC_FLAGS] &= ~U_DYN_FLAG_TAGGED_BY_OTHER;
}

bool Unit::IsTagged()
{
    return Tagged;
}

bool Unit::IsTaggable()
{
    if (!IsPet() && !Tagged)
        return true;
    else
        return false;
}

uint64 Unit::GetTaggerGUID()
{
    return TaggerGuid;
}

bool Unit::isLootable()
{
    if (IsTagged() && !IsPet() && !(IsPlayer() && !IsInBg()) && (GetCreatedByGUID() == 0) && !IsVehicle())
    {
        auto creature_prop = sMySQLStore.getCreatureProperties(GetEntry());
        if (IsCreature() && !lootmgr.HasLootForCreature(GetEntry()) && creature_prop != nullptr && (creature_prop->money == 0))  // Since it is inworld we can safely assume there is a proto cached with this Id!
            return false;

        return true;
    }
    else
        return false;
}

SpellProc* Unit::AddProcTriggerSpell(SpellInfo* spell, SpellInfo* orig_spell, uint64 caster, uint32 procChance, uint32 procFlags, uint32 procCharges, uint32* groupRelation, uint32* procClassMask, Object* obj)
{
    SpellProc* sp = NULL;
    if (spell != NULL)
        sp = GetProcTriggerSpell(spell->getId(), caster);
    if (sp != NULL && !sp->mDeleted)
        return sp;

    sp = sSpellProcMgr.NewSpellProc(this, spell, orig_spell, caster, procChance, procFlags, procCharges, groupRelation, procClassMask, obj);
    if (sp == NULL)
    {
        if (orig_spell != NULL)
            LOG_ERROR("Spell id %u tried to add a non-existent spell to Unit %p as SpellProc", orig_spell->getId(), this);
        else
            LOG_ERROR("Something tried to add a non-existent spell to Unit %p as SpellProc", this);
        return NULL;
    }
    m_procSpells.push_back(sp);

    return sp;
}

SpellProc* Unit::AddProcTriggerSpell(uint32 spell_id, uint32 orig_spell_id, uint64 caster, uint32 procChance, uint32 procFlags, uint32 procCharges, uint32* groupRelation, uint32* procClassMask, Object* obj)
{
    return AddProcTriggerSpell(sSpellCustomizations.GetSpellInfo(spell_id), sSpellCustomizations.GetSpellInfo(orig_spell_id), caster, procChance, procFlags, procCharges, groupRelation, procClassMask, obj);
}

SpellProc* Unit::AddProcTriggerSpell(SpellInfo* sp, uint64 caster, uint32* groupRelation, uint32* procClassMask, Object* obj)
{
    return AddProcTriggerSpell(sp, sp, caster, sp->getProcChance(), sp->getProcFlags(), sp->getProcCharges(), groupRelation, procClassMask, obj);
}

SpellProc* Unit::GetProcTriggerSpell(uint32 spellId, uint64 casterGuid)
{
    for (std::list<SpellProc*>::iterator itr = m_procSpells.begin(); itr != m_procSpells.end(); ++itr)
    {
        SpellProc* sp = *itr;
        if (sp->mSpell->getId() == spellId && (casterGuid == 0 || sp->mCaster == casterGuid))
            return sp;
    }

    return NULL;
}

void Unit::RemoveProcTriggerSpell(uint32 spellId, uint64 casterGuid, uint64 misc)
{
    for (std::list<SpellProc*>::iterator itr = m_procSpells.begin(); itr != m_procSpells.end(); ++itr)
    {
        SpellProc* sp = *itr;
        if (sp->CanDelete(spellId, casterGuid, misc))
        {
            sp->mDeleted = true;
            return;
        }
    }
}

void Unit::TakeDamage(Unit* pAttacker, uint32 damage, uint32 spellid, bool no_remove_auras)
{}

void Unit::Die(Unit* pAttacker, uint32 damage, uint32 spellid)
{}

void Unit::SendPeriodicAuraLog(const WoWGuid & CasterGUID, const WoWGuid & TargetGUID, uint32 SpellID, uint32 School, uint32 Amount, uint32 abs_dmg, uint32 resisted_damage, uint32 Flags, bool is_critical)
{

    WorldPacket data(SMSG_PERIODICAURALOG, 47);

    data << TargetGUID;             // target guid
    data << CasterGUID;             // caster guid
    data << uint32(SpellID);        // spellid
    data << uint32(1);              // unknown? need research?
    data << uint32(Flags | 0x1);    // aura school
    data << uint32(Amount);         // amount of done to target / heal / damage
    data << uint32(0);              // cebernic: unknown?? needs more research, but it should fix unknown damage type with suffered.
    data << uint32(g_spellSchoolConversionTable[School]);
    data << uint32(abs_dmg);
    data << uint32(resisted_damage);
    data << uint8(is_critical);

    SendMessageToSet(&data, true);
}

void Unit::SendPeriodicHealAuraLog(const WoWGuid & CasterGUID, const WoWGuid & TargetGUID, uint32 SpellID, uint32 healed, uint32 over_healed, bool is_critical)
{
    WorldPacket data(SMSG_PERIODICAURALOG, 41);

    data << TargetGUID;
    data << CasterGUID;
    data << SpellID;
    data << uint32(1);
    data << uint32(FLAG_PERIODIC_HEAL);
    data << uint32(healed);
    data << uint32(over_healed);
    data << uint32(0);              // I don't know what it is. maybe something related to absorbed heal?
    data << uint8(is_critical);

    SendMessageToSet(&data, true);
}


void Unit::Phase(uint8 command, uint32 newphase)
{
    Object::Phase(command, newphase);

    for (std::set<Object*>::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
    {
        if ((*itr)->IsUnit())
            static_cast<Unit*>(*itr)->UpdateVisibility();
    }

    UpdateVisibility();
}

bool Unit::InParty(Unit* u)
{
    Player* p = static_cast<Player*>(GetPlayerOwner());
    Player* uFrom = static_cast<Player*>(u->GetPlayerOwner());
    if (p == NULL || uFrom == NULL)
        return false;

    if (p == uFrom)
        return true;

    if (p->GetGroup() != NULL && uFrom->GetGroup() != NULL && p->GetGroup() == uFrom->GetGroup() && p->GetSubGroup() == uFrom->GetSubGroup())
        return true;

    return false;
}

bool Unit::InRaid(Unit* u)
{
    Player* p = static_cast<Player*>(GetPlayerOwner());
    Player* uFrom = static_cast<Player*>(u->GetPlayerOwner());
    if (p == NULL || uFrom == NULL)
        return false;

    if (p == uFrom)
        return true;

    if (p->GetGroup() != NULL && uFrom->GetGroup() != NULL && p->GetGroup() == uFrom->GetGroup())
        return true;

    return false;
}

bool Unit::IsCriticalDamageForSpell(Object* victim, SpellInfo* spell)
{
    bool result = false;
    float CritChance = 0.0f;
    PlayerCombatRating resilience_type = PCR_RANGED_SKILL;

    if (spell->custom_is_ranged_spell)
    {
        if (IsPlayer())
        {
            CritChance = getFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE);
            if (victim->IsPlayer())
                CritChance += static_cast<Player*>(victim)->res_R_crit_get();

            if (victim->IsUnit())
                CritChance += static_cast<float>(static_cast<Unit*>(victim)->AttackerCritChanceMod[spell->getSchool()]);
        }
        else
            CritChance = 5.0f; // static value for mobs.. not blizzlike, but an unfinished formula is not fatal :)

        if (victim->IsPlayer())
            resilience_type = PCR_RANGED_CRIT_RESILIENCE;
    }
    else if (spell->custom_is_melee_spell)
    {
        // Same shit with the melee spells, such as Judgment/Seal of Command
        if (IsPlayer())
            CritChance = getFloatValue(PLAYER_CRIT_PERCENTAGE);

        if (victim->IsPlayer())
        {
            CritChance += static_cast<Player*>(victim)->res_R_crit_get(); //this could be ability but in that case we overwrite the value
            resilience_type = PCR_MELEE_CRIT_RESILIENCE;
        }

        // Victim's (!) crit chance mod for physical attacks?
        if (victim->IsUnit())
            CritChance += static_cast<float>(static_cast<Unit*>(victim)->AttackerCritChanceMod[0]);
    }
    else
    {
        CritChance = spellcritperc + SpellCritChanceSchool[spell->getSchool()];

        if (victim->IsUnit())
        {
            CritChance += static_cast<float>(static_cast<Unit*>(victim)->AttackerCritChanceMod[spell->getSchool()]);

            //\todo Zyres: is tis relly the way this should work?
            if (IsPlayer() && (static_cast<Unit*>(victim)->m_rootCounter - static_cast<Unit*>(victim)->m_stunned))
                CritChance += static_cast<float>(static_cast<Player*>(this)->m_RootedCritChanceBonus);
        }

        spellModFlatFloatValue(SM_CriticalChance, &CritChance, spell->getSpellGroupType());

        if (victim->IsPlayer())
            resilience_type = PCR_SPELL_CRIT_RESILIENCE;
    }

    if (resilience_type)
        CritChance -= static_cast<Player*>(victim)->CalcRating(resilience_type);

    if (CritChance < 0.0f)
        CritChance = 0.0f;
    else if (CritChance > 95.0f)
        CritChance = 95.0f;

    result = Rand(CritChance);

    // HACK!!!
    Aura* fs = NULL;

    uint32 flameShock[] =
    {
        //SPELL_HASH_FLAME_SHOCK
        8050,
        8052,
        8053,
        10447,
        10448,
        13729,
        15039,
        15096,
        15616,
        16804,
        22423,
        23038,
        25457,
        29228,
        32967,
        34354,
        39529,
        39590,
        41115,
        43303,
        49232,
        49233,
        51588,
        55613,
        58940,
        58971,
        59684,
        0
    };

    if (victim->IsUnit()
        && (fs = static_cast<Unit*>(victim)->getAuraWithId(flameShock)) != NULL)
    {
        switch (spell->getId())
        {
            //SPELL_HASH_LAVA_BURST
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
                result = true;
                if (!HasAura(55447))            // Glyph of Flame Shock
                    fs->Remove();
            } break;
        }
    }

    return result;
}

float Unit::GetCriticalDamageBonusForSpell(Object* victim, SpellInfo* spell, float amount)
{
    int32 critical_bonus = 100;
    spellModFlatIntValue(SM_PCriticalDamage, &critical_bonus, spell->getSpellGroupType());

    if (critical_bonus > 0)
    {
        // the bonuses are halved by 50% (funky blizzard math :S)
        float b;
        if (spell->getSchool() == 0 || spell->custom_is_melee_spell || spell->custom_is_ranged_spell)		// physical || hackfix SoCommand/JoCommand
            b = critical_bonus / 100.0f + 1.0f;
        else
            b = critical_bonus / 200.0f + 1.0f;

        amount *= b;
    }

    if (victim->IsPlayer())
    {
        //res = res*(1.0f-2.0f*TO<Player*>(pVictim)->CalcRating(PLAYER_RATING_MODIFIER_MELEE_CRIT_RESISTANCE));
        //Resilience is a special new rating which was created to reduce the effects of critical hits against your character.
        //It has two components; it reduces the chance you will be critically hit by x%,
        //and it reduces the damage dealt to you by critical hits by 2x%. x is the percentage resilience granted by a given resilience rating.
        //It is believed that resilience also functions against spell crits,
        //though it's worth noting that NPC mobs cannot get critical hits with spells.

        float dmg_reduction_pct = 2 * static_cast<Player*>(victim)->CalcRating(PCR_MELEE_CRIT_RESILIENCE) / 100.0f;

        if (dmg_reduction_pct > 1.0f)
            dmg_reduction_pct = 1.0f; //we cannot resist more then he is criticalling us, there is no point of the critical then :P

        amount -= amount * dmg_reduction_pct;
    }

    if (victim->IsCreature() && static_cast<Creature*>(victim)->GetCreatureProperties()->Rank != ELITE_WORLDBOSS)
        static_cast<Creature*>(victim)->Emote(EMOTE_ONESHOT_WOUNDCRITICAL);

    return amount;
}

bool Unit::IsCriticalHealForSpell(Object* victim, SpellInfo* spell)
{
    int32 crit_chance = 0;

    crit_chance = float2int32(this->spellcritperc + this->SpellCritChanceSchool[spell->getSchool()]);

    //Sacred Shield
    if (victim->IsUnit())
    {
        uint32 sacredShield[] =
        {
            //SPELL_HASH_SACRED_SHIELD
            53601,
            58597,
            0
        };

        if (static_cast<Unit*>(victim)->hasAurasWithId(sacredShield))
        {
            switch (spell->getId())
            {
                //SPELL_HASH_FLASH_OF_LIGHT
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
                    crit_chance += 50;
                    break;
                default:
                    break;
            }
        }
    }

    spellModFlatIntValue(this->SM_CriticalChance, &crit_chance, spell->getSpellGroupType());

    return Rand(crit_chance);
}

float Unit::GetCriticalHealBonusForSpell(Object* victim, SpellInfo* spell, float amount)
{
    int32 critical_bonus = 100;
    spellModFlatIntValue(this->SM_PCriticalDamage, &critical_bonus, spell->getSpellGroupType());

    if (critical_bonus > 0)
    {
        // the bonuses are halved by 50% (funky blizzard math :S)
        float b = critical_bonus / 200.0f;
        amount += float2int32(amount * b);
    }

    return amount;
}

uint32 Unit::GetAuraCountWithDispelType(uint32 dispel_type, uint64 guid)
{
    uint32 result = 0;

    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x] == NULL)
            continue;

        if (m_auras[x]->GetSpellInfo()->getDispelType() == dispel_type && (guid == 0 || m_auras[x]->GetCasterGUID() == guid))
            result++;
    }

    return result;
}

void Unit::HandleKnockback(Object* caster, float horizontal, float vertical)
{
    //This is in unit and not creature because players who are mind controlled must use this.
    if (caster == NULL)
        caster = this;
    float angle = calcRadAngle(caster->GetPositionX(), caster->GetPositionY(), GetPositionX(), GetPositionY());
    if (caster == this)
        angle = float(GetOrientation() + M_PI);

    float destx, desty, destz;
    if (GetPoint(angle, horizontal, destx, desty, destz, true))
        GetAIInterface()->splineMoveKnockback(destx, desty, destz, horizontal, vertical);
}

void Unit::BuildPetSpellList(WorldPacket& data)
{
    data << uint64(0);
}

void Unit::CastOnMeleeSpell()
{
    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(GetOnMeleeSpell());
    Spell* spell = sSpellFactoryMgr.NewSpell(this, spellInfo, true, NULL);
    spell->extra_cast_number = GetOnMeleeSpellEcn();
    SpellCastTargets targets;
    targets.m_unitTarget = GetTargetGUID();
    spell->prepare(&targets);
    SetOnMeleeSpell(0);
}

void Unit::SendHopOnVehicle(Unit* vehicleowner, uint32 seat)
{
    WorldPacket data(SMSG_MONSTER_MOVE_TRANSPORT, 50);
    data << GetNewGUID();
    data << vehicleowner->GetNewGUID();
    data << uint8(seat);

    if (IsPlayer())
        data << uint8(1);
    else
        data << uint8(0);

    data << float(GetPositionX());
    data << float(GetPositionY());
    data << float(GetPositionZ());
    data <<Util::getMSTime();
    data << uint8(4);                // splinetype_facing_angle
    data << float(0.0f);             // facing angle
    data << uint32(0x00800000);      // splineflag transport
    data << uint32(0);               // movetime
    data << uint32(1);               // wp count
    data << float(0.0f);             // x
    data << float(0.0f);             // y
    data << float(0.0f);             // z

    SendMessageToSet(&data, true);
}

void Unit::SendHopOffVehicle(Unit* vehicleowner, LocationVector& landposition)
{
    WorldPacket data(SMSG_MONSTER_MOVE, 1 + 12 + 4 + 1 + 4 + 4 + 4 + 12 + 8);
    data << GetNewGUID();

    if (IsPlayer())
        data << uint8(1);
    else
        data << uint8(0);

    data << float(GetPositionX());
    data << float(GetPositionY());
    data << float(GetPositionZ());
    data << uint32(Util::getMSTime());
    data << uint8(4);                            // SPLINETYPE_FACING_ANGLE
    data << float(GetOrientation());             // guess
    data << uint32(0x01000000);                  // SPLINEFLAG_EXIT_VEHICLE
    data << uint32(0);                           // Time in between points
    data << uint32(1);                           // 1 single waypoint
    data << float(vehicleowner->GetPositionX());
    data << float(vehicleowner->GetPositionY());
    data << float(vehicleowner->GetPositionZ());

    SendMessageToSet(&data, true);
}

void Unit::EnterVehicle(uint64 guid, uint32 delay)
{
    if (delay != 0)
    {
        sEventMgr.AddEvent(this, &Unit::EnterVehicle, guid, uint32(0), 0, delay, 1, 0);
        return;
    }

    Unit* u = m_mapMgr->GetUnit(guid);
    if (u == NULL)
        return;

    if (u->GetVehicleComponent() == NULL)
        return;

    if (currentvehicle != NULL)
        return;

    u->GetVehicleComponent()->AddPassenger(this);
}

Vehicle* Unit::GetCurrentVehicle()
{
    return currentvehicle;
}

Vehicle* Unit::GetVehicleComponent()
{
    return vehicle;
}

Unit* Unit::GetVehicleBase()
{
    if (currentvehicle != NULL)
        return currentvehicle->GetOwner();
    else
        if (vehicle != NULL)
            return this;

    return NULL;
}

void Unit::SendEnvironmentalDamageLog(uint64 guid, uint8 type, uint32 damage)
{
    WorldPacket data(SMSG_ENVIRONMENTALDAMAGELOG, 20);

    data << uint64(guid);
    data << uint8(type);
    data << uint32(damage);
    data << uint64(0);

    SendMessageToSet(&data, true, false);
}

void Unit::BuildMovementPacket(ByteBuffer* data)
{
    *data << uint32(GetUnitMovementFlags());            // movement flags
    *data << uint16(GetExtraUnitMovementFlags());       // 2.3.0
    *data << uint32(Util::getMSTime());                       // time / counter
    *data << GetPositionX();
    *data << GetPositionY();
    *data << GetPositionZ();
    *data << GetOrientation();

#if VERSION_STRING != Cata
    // 0x00000200
    if (HasUnitMovementFlag(MOVEFLAG_TRANSPORT))
    {
        if (IsPlayer())
        {
            auto plr = static_cast<Player*>(this);
            if (plr->obj_movement_info.IsOnTransport())
            {
                obj_movement_info.transporter_info.guid = plr->obj_movement_info.transporter_info.guid;
            }
        }
        if (Unit* u = GetVehicleBase())
            obj_movement_info.transporter_info.guid = u->GetGUID();
        *data << obj_movement_info.transporter_info.guid;
        *data << obj_movement_info.transporter_info.guid;
        *data << GetTransPositionX();
        *data << GetTransPositionY();
        *data << GetTransPositionZ();
        *data << GetTransPositionO();
        *data << GetTransTime();
        *data << GetTransSeat();

        if (GetExtraUnitMovementFlags() & MOVEFLAG2_INTERPOLATED_MOVE)
            *data << uint32(GetMovementInfo()->transporter_info.time2);
    }

    // 0x02200000
    if ((GetUnitMovementFlags() & (MOVEFLAG_SWIMMING | MOVEFLAG_FLYING))
        || (GetExtraUnitMovementFlags() & MOVEFLAG2_ALLOW_PITCHING))
        *data << (float)GetMovementInfo()->pitch;

    *data << (uint32)GetMovementInfo()->fall_time;
#endif
    // 0x00001000
#if VERSION_STRING != Cata
    if (GetUnitMovementFlags() & MOVEFLAG_REDIRECTED)
    {
        *data << (float)GetMovementInfo()->redirectVelocity;
        *data << (float)GetMovementInfo()->redirectSin;
        *data << (float)GetMovementInfo()->redirectCos;
        *data << (float)GetMovementInfo()->redirect2DSpeed;
    }

    // 0x04000000
    if (GetUnitMovementFlags() & MOVEFLAG_SPLINE_MOVER)
        *data << (float)GetMovementInfo()->spline_elevation;
#endif
}


void Unit::BuildMovementPacket(ByteBuffer* data, float x, float y, float z, float o)
{
    *data << uint32(GetUnitMovementFlags());            // movement flags
    *data << uint16(GetExtraUnitMovementFlags());       // 2.3.0
    *data << uint32(Util::getMSTime());                       // time / counter
    *data << x;
    *data << y;
    *data << z;
    *data << o;

#if VERSION_STRING != Cata
    // 0x00000200
    if (HasUnitMovementFlag(MOVEFLAG_TRANSPORT))
    {
        // Code left commented for reference
        // TODO: Research whether vehicle transport guid is being updated correctly or not (and if not, update it elsewhere and remove this)
        /*if (IsPlayer() && static_cast<Player*>(this)->m_transport)
            obj_movement_info.transporter_info.guid = static_cast<Player*>(this)->m_transport->GetGUID();
        if (Unit* u = GetVehicleBase())
            obj_movement_info.transporter_info.guid = u->GetGUID();*/
        *data << obj_movement_info.transporter_info.guid;
        *data << GetTransPositionX();
        *data << GetTransPositionY();
        *data << GetTransPositionZ();
        *data << GetTransPositionO();
        *data << GetTransTime();
        *data << GetTransSeat();

        if (GetExtraUnitMovementFlags() & MOVEFLAG2_INTERPOLATED_MOVE)
            *data << uint32(GetMovementInfo()->transporter_info.time2);
    }

    // 0x02200000
    if ((GetUnitMovementFlags() & (MOVEFLAG_SWIMMING | MOVEFLAG_FLYING))
        || (GetExtraUnitMovementFlags() & MOVEFLAG2_ALLOW_PITCHING))
        *data << (float)GetMovementInfo()->pitch;

    *data << (uint32)GetMovementInfo()->fall_time;
#endif
    // 0x00001000
#if VERSION_STRING != Cata
    if (GetUnitMovementFlags() & MOVEFLAG_REDIRECTED)
    {
        *data << (float)GetMovementInfo()->redirectVelocity;
        *data << (float)GetMovementInfo()->redirectSin;
        *data << (float)GetMovementInfo()->redirectCos;
        *data << (float)GetMovementInfo()->redirect2DSpeed;
    }

    // 0x04000000
    if (GetUnitMovementFlags() & MOVEFLAG_SPLINE_MOVER)
        *data << (float)GetMovementInfo()->spline_elevation;
#endif
}

void Unit::setLevel(uint32 level)
{
    setUInt32Value(UNIT_FIELD_LEVEL, level);
    if (IsPlayer())
        static_cast< Player* >(this)->SetNextLevelXp(sMySQLStore.getPlayerXPForLevel(level));
}

void Unit::UpdateAuraForGroup(uint8 slot)
{
    if (slot >= 64)
        return;

    if (IsPlayer())
    {
        Player* player = static_cast<Player*>(this);
        if (player->GetGroup())
        {
            player->AddGroupUpdateFlag(GROUP_UPDATE_FLAG_AURAS);
            player->SetAuraUpdateMaskForRaid(slot);
        }
    }
    else if (GetPlayerOwner())
    {
        if (GetPlayerOwner())
        {
            Player* owner = static_cast<Player*>(GetPlayerOwner());
            if (owner->GetGroup())
            {
                owner->AddGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_AURAS);
                SetAuraUpdateMaskForRaid(slot);
            }
        }
    }
}

void Unit::HandleUpdateFieldChange(uint32 Index)
{
    Player* player = NULL;
    bool pet = false;

    if (!IsInWorld())
        return;

    if (IsPlayer())
        player = static_cast<Player*>(this);
    else if (GetPlayerOwner())
    {
        player = static_cast<Player*>(GetPlayerOwner());
        pet = true;
    }

    if (player == NULL || !player->IsInWorld() || !player->GetGroup())
        return;

    uint32 Flags = 0;
    switch (Index)
    {
        case UNIT_FIELD_HEALTH:
            Flags = pet ? GROUP_UPDATE_FLAG_PET_CUR_HP : GROUP_UPDATE_FLAG_CUR_HP;
            break;

        case UNIT_FIELD_MAXHEALTH:
            Flags = pet ? GROUP_UPDATE_FLAG_PET_MAX_HP : GROUP_UPDATE_FLAG_MAX_HP;
            break;

        case UNIT_FIELD_POWER1:
        case UNIT_FIELD_POWER2:
        case UNIT_FIELD_POWER3:
        case UNIT_FIELD_POWER4:
        case UNIT_FIELD_POWER5:
#if VERSION_STRING == WotLK
        case UNIT_FIELD_POWER6:
        case UNIT_FIELD_POWER7:
#endif
            Flags = pet ? GROUP_UPDATE_FLAG_PET_CUR_POWER : GROUP_UPDATE_FLAG_CUR_POWER;
            break;

        case UNIT_FIELD_MAXPOWER1:
        case UNIT_FIELD_MAXPOWER2:
        case UNIT_FIELD_MAXPOWER3:
        case UNIT_FIELD_MAXPOWER4:
        case UNIT_FIELD_MAXPOWER5:
#if VERSION_STRING == WotLK
        case UNIT_FIELD_MAXPOWER6:
        case UNIT_FIELD_MAXPOWER7:
#endif
            Flags = pet ? GROUP_UPDATE_FLAG_PET_CUR_POWER : GROUP_UPDATE_FLAG_MAX_POWER;
            break;

        case UNIT_FIELD_DISPLAYID:
            Flags = pet ? GROUP_UPDATE_FLAG_PET_MODEL_ID : 0;
            break;
        case UNIT_FIELD_LEVEL:
            Flags = pet ? 0 : GROUP_UPDATE_FLAG_LEVEL;
            break;
        case UNIT_FIELD_BYTES_0:
            Flags = pet ? GROUP_UPDATE_FLAG_PET_POWER_TYPE : GROUP_UPDATE_FLAG_POWER_TYPE;
            break;
        case UNIT_FIELD_BYTES_2:
        case PLAYER_FLAGS:
            Flags = pet ? 0 : GROUP_UPDATE_FLAG_STATUS;
            break;
        default:
            break;
    }
    player->AddGroupUpdateFlag(Flags);
}

void Unit::Possess(Unit* pTarget, uint32 delay)
{
    Player* pThis = NULL;
    if (IsPlayer())
        pThis = static_cast<Player*>(this);
    else // do not support creatures just yet
        return;
    if (!pThis)
        return;
    if (GetCharmedUnitGUID())
        return;

    setMoveRoot(true);

    if (delay != 0)
    {
        sEventMgr.AddEvent(this, &Unit::Possess, pTarget, uint32(0), 0, delay, 1, 0);
        return;
    }
    if (pTarget == NULL)
    {
        setMoveRoot(false);
        return;
    }

    pThis->m_CurrentCharm = pTarget->GetGUID();
    if (pTarget->IsCreature())
    {
        // unit-only stuff.
        pTarget->setAItoUse(false);
        pTarget->GetAIInterface()->StopMovement(0);
        pTarget->m_redirectSpellPackets = pThis;
    }

    m_noInterrupt++;
    SetCharmedUnitGUID(pTarget->GetGUID());
    pTarget->SetCharmedByGUID(GetGUID());
    pTarget->SetCharmTempVal(pTarget->GetFaction());
    pThis->SetFarsightTarget(pTarget->GetGUID());
    pTarget->SetFaction(GetFaction());
    pTarget->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE | UNIT_FLAG_PVP_ATTACKABLE);

    SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);

    // send "switch mover" packet
    pThis->SetClientControl(pTarget, 1);

    // update target faction set
    pTarget->UpdateOppFactionSet();

    if (!(pTarget->IsPet() && static_cast< Pet* >(pTarget) == pThis->GetSummon()))
    {
        WorldPacket data(SMSG_PET_SPELLS, 4 * 4 + 20);
        pTarget->BuildPetSpellList(data);
        pThis->GetSession()->SendPacket(&data);
    }
}

void Unit::UnPossess()
{
    Player* pThis = NULL;
    if (IsPlayer())
        pThis = static_cast<Player*>(this);
    else // creatures no support yet
        return;
    if (!pThis)
        return;
    if (!GetCharmedUnitGUID())
        return;

    Unit* pTarget = GetMapMgr()->GetUnit(GetCharmedUnitGUID());
    if (!pTarget)
        return;

    pThis->m_CurrentCharm = 0;

    pThis->SpeedCheatReset();

    if (pTarget->IsCreature())
    {
        // unit-only stuff.
        pTarget->setAItoUse(true);
        pTarget->m_redirectSpellPackets = 0;
    }

    m_noInterrupt--;
    pThis->SetFarsightTarget(0);
    SetCharmedUnitGUID(0);
    pTarget->SetCharmedByGUID(0);
    SetCharmedUnitGUID(0);

    RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER);
    pTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE | UNIT_FLAG_PVP_ATTACKABLE);
    pTarget->SetFaction(pTarget->GetCharmTempVal());
    pTarget->UpdateOppFactionSet();

    // send "switch mover" packet
    pThis->SetClientControl(pTarget, 0);

    if (!(pTarget->IsPet() && static_cast< Pet* >(pTarget) == pThis->GetSummon()))
        pThis->SendEmptyPetSpellList();

    setMoveRoot(false);

    if (!pTarget->IsPet() && (pTarget->GetCreatedByGUID() == GetGUID()))
    {
        sEventMgr.AddEvent(static_cast< Object* >(pTarget), &Object::Delete, 0, 1, 1, 0);
        return;
    }
}
