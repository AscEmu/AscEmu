/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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
#include "Spell/Definitions/SpellDamageType.h"
#include "Creatures/Pet.h"
#include "Data/WoWUnit.hpp"
#include "Server/Packets/SmsgUpdateAuraDuration.h"
#include "Server/Packets/SmsgSetExtraAuraInfo.h"
#include "Server/Packets/SmsgEmote.h"
#include "Server/Packets/SmsgAttackStart.h"
#include "Server/Packets/SmsgAttackStop.h"
#include "Server/Packets/SmsgPowerUpdate.h"
#include "Server/Packets/SmsgSpellDamageShield.h"
#include "Server/Packets/SmsgAuraUpdateAll.h"
#include "Server/Packets/SmsgAuraUpdate.h"
#include "Server/Packets/SmsgPeriodicAuraLog.h"
#include "Server/Packets/SmsgAttackSwingBadFacing.h"
#include "Movement/Spline/New/MoveSpline.h"
#include "Movement/Spline/New/MoveSplineInit.h"

using namespace AscEmu::Packets;

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
#ifdef AE_MOP
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

Unit::Unit() :
    m_movementManager(),
    m_movementAI(this),
    movespline(new MovementNew::MoveSpline())
{
    mControledUnit = this;
    mPlayerControler = nullptr;

    uint8_t i;

    m_canDualWield = false;
    for (i = 0; i < TOTAL_WEAPON_DAMAGE_TYPES; ++i)
    {
        m_attackTimer[i] = 0;
        m_attackSpeed[i] = 1.0f;
    }

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

#if VERSION_STRING < Cata
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_LIVING | UPDATEFLAG_HAS_POSITION);
#else
    m_updateFlag = UPDATEFLAG_LIVING;
#endif

    //DK:modifiers
    PctRegenModifier = 0;
    for (i = 0; i < TOTAL_PLAYER_POWER_TYPES; i++)
    {
        PctPowerRegenModifier[i] = 1;
        m_powerFractions[i] = 0;
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

    m_can_stealth = true;

    for (i = 0; i < 5; i++)
        BaseStats[i] = 0;

    //  if (getObjectTypeId() == TYPEID_PLAYER) // only player for now
    //      CalculateActualArmor();

    m_aiInterface = new AIInterface();
    m_aiInterface->Init(this, AI_SCRIPT_AGRO, Movement::WP_MOVEMENT_SCRIPT_NONE);

    m_oldEmote = 0;

    BaseDamage[0] = 0;
    BaseOffhandDamage[0] = 0;
    BaseRangedDamage[0] = 0;
    BaseDamage[1] = 0;
    BaseOffhandDamage[1] = 0;
    BaseRangedDamage[1] = 0;

    m_CombatUpdateTimer = 0;
    for (i = 0; i < TOTAL_SPELL_SCHOOLS; i++)
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

    trackStealth = false;

    m_threatModifyer = 0;
    memset(m_auras, 0, (MAX_TOTAL_AURAS_END)*sizeof(Aura*));

    // diminishing return stuff
    memset(m_diminishAuraCount, 0, DIMINISHING_GROUP_COUNT);
    memset(m_diminishCount, 0, DIMINISHING_GROUP_COUNT * 2);
    memset(m_diminishTimer, 0, DIMINISHING_GROUP_COUNT * 2);
    memset(m_auravisuals, 0, MAX_NEGATIVE_VISUAL_AURAS_END * sizeof(uint32));

    m_diminishActive = false;
    dynObj = 0;
    m_flyspeedModifier = 0;
    bInvincible = false;
    m_redirectSpellPackets = 0;
    can_parry = false;
    bProcInUse = false;
    spellcritperc = 0;

    RangedDamageTaken = 0;
    m_damgeShieldsInUse = false;
    // fearSpell = 0;
    m_extraAttackCounter = false;
    CombatStatus.SetUnit(this);
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
    tmpAura.clear();
    m_extraStrikeTargets.clear();

    asc_frozen = 0;
    asc_enraged = 0;
    asc_seal = 0;
    asc_bleed = 0;

    Tagged = false;
    TaggerGuid = 0;

    m_singleTargetAura.clear();

    m_vehicle = NULL;
    m_currentVehicle = NULL;

    m_noFallDamage = false;
    z_axisposition = 0.0f;
    m_safeFall = 0;
    m_cTimer = 0;
    m_temp_summon = false;
    m_meleespell_ecn = 0;
    m_manaShieldId = 0;
    m_charmtemp = 0;
    m_auraRaidUpdateMask = 0;

    // APGL End
    // MIT Start

    m_summonInterface = new SummonHandler;

    m_healthBatch.clear();

    for (i = 0; i < INVIS_FLAG_TOTAL; i++)
    {
        m_invisibilityLevel[i] = 0;
        m_invisibilityDetection[i] = 0;
    }

    for (i = 0; i < STEALTH_FLAG_TOTAL; ++i)
    {
        m_stealthLevel[i] = 0;
        m_stealthDetection[i] = 0;
    }

    for (i = 0; i < MAX_SPELLMOD_TYPE; ++i)
    {
        m_spellModifiers[i].clear();
    }

    // MIT End
    // APGL Start
}

Unit::~Unit()
{
    //start to remove badptrs, if you delete from the heap null the ptr's damn!
    RemoveAllAuras();
    delete movespline;

    delete m_aiInterface;
    m_aiInterface = NULL;

    for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
    {
        if (getCurrentSpell(CurrentSpellType(i)) != nullptr)
            interruptSpellWithSpellType(CurrentSpellType(i));
    }

    for (uint8_t i = 0; i < MAX_SPELLMOD_TYPE; ++i)
    {
        m_spellModifiers[i].clear();
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
        LOG_ERROR("ExtraStrike added to Unit %u by Spell ID %u wasn't removed when removing the Aura", getGuid(), es->spell_info->getId());
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

    delete m_summonInterface;

    clearHealthBatch();

    RemoveGarbage();
}

void Unit::Update(unsigned long time_passed)
{
    m_movementAI.updateMovement(time_passed);

    const auto msTime = Util::getMSTime();
    
    auto diff = msTime - m_lastSpellUpdateTime;
    if (diff >= 100)
    {
        // Spells and auras are updated every 100ms
        _UpdateSpells(diff);
        _updateAuras(diff);

        // Update spell school lockout timer
        // TODO: Moved here from Spell::CanCast, figure out a better way to handle this... -Appled
        for (uint8_t i = 0; i < TOTAL_SPELL_SCHOOLS; ++i)
        {
            if (SchoolCastPrevent[i] == 0)
                continue;

            if (msTime >= SchoolCastPrevent[i])
                SchoolCastPrevent[i] = 0;
        }

        RemoveGarbage();

        m_lastSpellUpdateTime = msTime;
    }

    // Update summon interface every 100ms to check summon durations
    diff = msTime - m_lastSummonUpdateTime;
    if (diff >= 100)
    {
        getSummonInterface()->update(static_cast<uint16_t>(diff));
        m_lastSummonUpdateTime = msTime;
    }

    if (isAlive())
    {
        // Update health batch
        if (time_passed >= m_healthBatchTime)
        {
            _updateHealth();
            m_healthBatchTime = HEALTH_BATCH_INTERVAL;
        }
        else
        {
            m_healthBatchTime -= static_cast<uint16_t>(time_passed);
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        //POWER & HP REGENERATION
        regenerateHealthAndPowers(static_cast<uint16_t>(time_passed));

#if VERSION_STRING >= WotLK
        // Send power amount to nearby players
        if (time_passed >= m_powerUpdatePacketTime)
        {
            m_powerUpdatePacketTime = REGENERATION_PACKET_UPDATE_INTERVAL;

            switch (getPowerType())
            {
                case POWER_TYPE_MANA:
                    setPower(POWER_TYPE_MANA, m_manaAmount);
                    break;
                case POWER_TYPE_RAGE:
                    setPower(POWER_TYPE_RAGE, m_rageAmount);
                    break;
                case POWER_TYPE_FOCUS:
                    setPower(POWER_TYPE_FOCUS, m_focusAmount);
                    break;
                case POWER_TYPE_ENERGY:
                    setPower(POWER_TYPE_ENERGY, m_energyAmount);
                    break;
                case POWER_TYPE_RUNIC_POWER:
                    setPower(POWER_TYPE_RUNIC_POWER, m_runicPowerAmount);
                    break;
                default:
                    break;
            }
        }
        else
        {
            m_powerUpdatePacketTime -= static_cast<uint16_t>(time_passed);
        }
#endif

        if (m_healthRegenerationInterruptTime > 0)
        {
            if (time_passed >= m_healthRegenerationInterruptTime)
                m_healthRegenerationInterruptTime = 0;
            else
                m_healthRegenerationInterruptTime -= time_passed;
        }

#if VERSION_STRING < Cata
        if (m_powerRegenerationInterruptTime > 0)
        {
            if (time_passed >= m_powerRegenerationInterruptTime)
            {
                m_powerRegenerationInterruptTime = 0;

#if VERSION_STRING != Classic
                if (isPlayer())
                    setUnitFlags2(UNIT_FLAG2_ENABLE_POWER_REGEN);
#endif
            }
            else
            {
                m_powerRegenerationInterruptTime -= time_passed;
            }
        }
#endif

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
    }
}

bool Unit::canReachWithAttack(Unit* pVictim)
{
    if (GetMapId() != pVictim->GetMapId())
        return false;

    // float targetreach = pVictim->getCombatReach();
    float selfreach;
    if (isPlayer())
        selfreach = 5.0f; // minimum melee range, getCombatReach() is too small and used eg. in melee spells
    else
        selfreach = getCombatReach();

    float targetradius;
    // targetradius = pVictim->getBoundingRadius(); //this is plain wrong. Represents i have no idea what :)
    targetradius = pVictim->GetModelHalfSize();
    float selfradius;
    // selfradius = getBoundingRadius();
    selfradius = GetModelHalfSize();
    // float targetscale = pVictim->getScale();
    // float selfscale = getScale();

    //float distance = std::sqrt(getDistanceSq(pVictim));
    float delta_x = pVictim->GetPositionX() - GetPositionX();
    float delta_y = pVictim->GetPositionY() - GetPositionY();
    float distance = std::sqrt(delta_x * delta_x + delta_y * delta_y);


    // float attackreach = (((targetradius*targetscale) + selfreach) + (((selfradius*selfradius)*selfscale)+1.50f));
    float attackreach = targetradius + selfreach + selfradius;

    //formula adjustment for player side.
    if (isPlayer())
    {
        // latency compensation!!
        // figure out how much extra distance we need to allow for based on our movespeed and latency.
        if (pVictim->isPlayer() && static_cast<Player*>(pVictim)->isMoving())
        {
            // this only applies to PvP.
            uint32 lat = static_cast<Player*>(pVictim)->GetSession() ? static_cast<Player*>(pVictim)->GetSession()->GetLatency() : 0;

            // if we're over 500 get fucked anyway.. your gonna lag! and this stops cheaters too
            lat = (lat > 500) ? 500 : lat;

            // calculate the added distance
            attackreach += getSpeedRate(TYPE_RUN, true) * 0.001f * lat;
        }

        if (static_cast<Player*>(this)->isMoving())
        {
            // this only applies to PvP.
            uint32 lat = static_cast<Player*>(this)->GetSession() ? static_cast<Player*>(this)->GetSession()->GetLatency() : 0;

            // if we're over 500 get fucked anyway.. your gonna lag! and this stops cheaters too
            lat = (lat > 500) ? 500 : lat;

            // calculate the added distance
            attackreach += getSpeedRate(TYPE_RUN, true) * 0.001f * lat;
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

    if (!PlayerInGroup->isInGroup())
        return;

    Group* pGroup = PlayerInGroup->getGroup();
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

    pGroup->Lock();
    for (uint32 i = 0; i < pGroup->GetSubGroupCount(); i++)
    {
        for (GroupMembersSet::iterator itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); ++itr)
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
        PlayerInGroup->GiveXP(xp, pVictim->getGuid(), true);
    }
    else
    {
        if (pGroup->getGroupType() == GROUP_TYPE_PARTY)
        {
            if (active_player_count == 3)
                xp_mod = 1.1666f;
            else if (active_player_count == 4)
                xp_mod = 1.3f;
            else if (active_player_count == 5)
                xp_mod = 1.4f;
            else xp_mod = 1; //in case we have only 2 members ;)
        }
        else if (pGroup->getGroupType() == GROUP_TYPE_RAID)
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
            plr->GiveXP(float2int32(((xp * plr->getLevel()) / total_level) * xp_mod), pVictim->getGuid(), true);

            active_player_list[i]->addAuraStateAndAuras(AURASTATE_FLAG_LASTKILLWITHHONOR);
            if (!sEventMgr.HasEvent(active_player_list[i], EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE))
            {
                sEventMgr.AddEvent(static_cast<Unit*>(active_player_list[i]), &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_LASTKILLWITHHONOR, EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }
            else
            {
                sEventMgr.ModifyEventTimeLeft(active_player_list[i], EVENT_LASTKILLWITHHONOR_FLAG_EXPIRE, 20000);
            }

            if (plr->GetSummon() && plr->GetSummon()->CanGainXP())
            {
                uint32 pet_xp = (uint32)(CalculateXpToGive(pVictim, plr->GetSummon()) * xp_mod);   // vojta: this isn't blizzlike probably but i have no idea, feel free to correct it
                if (pet_xp> 0)
                    plr->GetSummon()->giveXp(pet_xp);
            }
        }
    }
}

uint32 Unit::HandleProc(uint32 flag, Unit* victim, SpellInfo const* CastingSpell, DamageInfo damageInfo, bool isSpellTriggered, ProcEvents procEvent/* = PROC_EVENT_DO_ALL*/, Aura* triggeredFromAura/* = nullptr*/)
{
    uint32 resisted_dmg = 0;
    bool can_delete = !bProcInUse; //if this is a nested proc then we should have this set to TRUE by the father proc
    bProcInUse = true; //locking the proc list

    std::list<SpellProc*> happenedProcs;

    for (std::list<SpellProc*>::iterator itr = m_procSpells.begin(); itr != m_procSpells.end();)    // Proc Trigger Spells for Victim
    {
        std::list<SpellProc*>::iterator itr2 = itr;
        ++itr;

        SpellProc* spell_proc = *itr2;

        // Check if list item was deleted elsewhere, so here it's removed and freed
        if (spell_proc->isDeleted())
        {
            if (can_delete)
            {
                m_procSpells.erase(itr2);
                delete spell_proc;
            }
            continue;
        }

        // APGL End
        // MIT Start

        // Check if spell proc is marked to skip this call
        if (spell_proc->isSkippingHandleProc())
        {
            spell_proc->skipOnNextHandleProc(false);
            continue;
        }

        if (CastingSpell != nullptr)
        {
            // A spell cannot proc itself
            if (CastingSpell->getId() == spell_proc->getSpell()->getId())
                continue;

            // Check proc class mask
            if (!spell_proc->checkClassMask(CastingSpell))
                continue;
        }

        if (procEvent == PROC_EVENT_DO_CASTER_PROCS_ONLY && !spell_proc->isCastedOnProcOwner())
            continue;

        if (procEvent == PROC_EVENT_DO_TARGET_PROCS_ONLY && spell_proc->isCastedOnProcOwner())
            continue;

        // Check proc flags
        if (!sScriptMgr.callScriptedSpellCheckProcFlags(spell_proc, static_cast<SpellProcFlags>(flag)))
            continue;

        // Check extra proc flags
        if (!spell_proc->checkExtraProcFlags(this, damageInfo))
            continue;

        // Check if this proc can happen
        if (!sScriptMgr.callScriptedSpellCanProc(spell_proc, victim, CastingSpell, damageInfo))
            continue;
        if (!spell_proc->canProc(victim, CastingSpell))
            continue;

        if (CastingSpell != nullptr)
        {
            // Check if this proc can trigger on already triggered spell
            // by default procs can't
            if (isSpellTriggered && !sScriptMgr.callScriptedSpellCanProcOnTriggered(spell_proc, victim, CastingSpell, triggeredFromAura))
                continue;
        }

        const auto spe = spell_proc->getSpell();
        // Spell id which is going to proc
        auto spellId = spe->getId();

        // Get spellinfo of the spell that created this proc
        uint32_t origId = 0;
        if (spell_proc->getOriginalSpell() != nullptr)
            origId = spell_proc->getOriginalSpell()->getId();

        // No need to check if exists or not since we were not able to register this trigger if it would not exist :P
        const auto ospinfo = spell_proc->getOriginalSpell();

        auto proc_Chance = sScriptMgr.callScriptedSpellCalcProcChance(spell_proc, victim, CastingSpell);

        // Check if spell proc uses procs-per-minute system
        if (isPlayer())
        {
            // Procs-per-minute, or PPM, amount describes how many procs (on average) can occur in one minute
            // To calculate Proc-chance-Per-Hit, or PPH, formula is:
            // unmodified weapon speed * PPM / 60
            auto ppmAmount = spell_proc->getProcsPerMinute();
            const auto plr = static_cast<Player*>(this);

            // Old hackfixes
            switch (spellId)
            {
                //SPELL_HASH_BLACK_TEMPLE_MELEE_TRINKET
                case 40475:
                    ppmAmount = 1.0f;
                    break;
                // SPELL_HASH_MAGTHERIDON_MELEE_TRINKET:
                case 34774:
                    ppmAmount = 1.5f;
                    break;                          // dragonspine trophy
                // SPELL_HASH_ROMULO_S_POISON:
                case 34586:
                case 34587:
                    ppmAmount = 1.5f;
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
                    ppmAmount = 9.0f;
                    break;                          // Frostbrand Weapon
                case 16870:
                    ppmAmount = 2.0f;
                    break; //druid: clearcasting
                default:
                    break;
            }

            // Default value is 0.0
            if (ppmAmount != 0.0f)
            {
                // Unarmed speed is 2 sec
                uint32_t weaponSpeed = 2000;
                if (plr->IsInFeralForm())
                {
                    // Get shapeshift form's attack speed
                    const auto form = sSpellShapeshiftFormStore.LookupEntry(plr->getShapeShiftForm());
                    if (form != nullptr && form->AttackSpeed != 0)
                        weaponSpeed = form->AttackSpeed;
                }
                else
                {
                    switch (damageInfo.weaponType)
                    {
                        case MELEE:
                        {
                            const auto mainHand = plr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                            if (mainHand != nullptr && mainHand->isWeapon())
                                weaponSpeed = mainHand->getItemProperties()->Delay;
                        } break;
                        case OFFHAND:
                        {
                            const auto offHand = plr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                            if (offHand != nullptr && offHand->isWeapon())
                                weaponSpeed = offHand->getItemProperties()->Delay;
                        } break;
                        case RANGED:
                        {
                            const auto ranged = plr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                            if (ranged != nullptr && ranged->isWeapon())
                                weaponSpeed = ranged->getItemProperties()->Delay;
                        } break;
                        default:
                            break;
                    }
                }

                proc_Chance = float2int32((weaponSpeed * 0.001f * ppmAmount / 60.0f) * 100.0f);
            }
        }

        // Apply modifiers to proc chance
        // todo: this will not use spell charges
        applySpellModifiers(SPELLMOD_TRIGGER, &proc_Chance, ospinfo);

        if (!Util::checkChance(proc_Chance))
            continue;

        // Check if proc has interval
        if (spell_proc->getProcInterval() > 0)
        {
            // Check for cooldown cheat
            if (!(spell_proc->getProcOwner()->isPlayer() && static_cast<Player*>(spell_proc->getProcOwner())->m_cheats.hasCooldownCheat))
            {
                const auto timeNow = Util::getMSTime();
                if (spell_proc->getLastTriggerTime() + spell_proc->getProcInterval() > timeNow)
                    continue;

                spell_proc->setLastTriggerTime(timeNow);
            }
        }

        // MIT End
        // APGL Start

        // SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE
        for (uint8 i = 0; i < 3; ++i)
        {
            if (ospinfo && ospinfo->getEffectApplyAuraName(i) == SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE)
            {
                spell_proc->setOverrideEffectDamage(i, ospinfo->getEffectBasePoints(i) + 1);
                sScriptMgr.callScriptedSpellProcDoEffect(spell_proc, victim, CastingSpell, damageInfo);
                spell_proc->doEffect(victim, CastingSpell, flag, damageInfo.realDamage, damageInfo.absorbedDamage, spell_proc->getOverrideEffectDamages(), damageInfo.weaponType);
            }
        }

        // give spell_proc a chance to handle the effect
        const auto scriptResult = sScriptMgr.callScriptedSpellProcDoEffect(spell_proc, victim, CastingSpell, damageInfo);
        if (scriptResult == SpellScriptExecuteState::EXECUTE_PREVENT)
            continue;
        if (spell_proc->doEffect(victim, CastingSpell, flag, damageInfo.realDamage, damageInfo.absorbedDamage, spell_proc->getOverrideEffectDamages(), damageInfo.weaponType))
            continue;

        //these are player talents. Fuckem they pull the emu speed down
        if (isPlayer())
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
                    if (!isPlayer())
                        continue;
                    Player* p = static_cast<Player*>(this);
                    if (p->getShapeShiftForm() != FORM_BEAR && p->getShapeShiftForm() != FORM_DIREBEAR)
                        continue;
                }
                break;
                case 16953:     //Druid - Blood Frenzy Proc
                {
                    if (!isPlayer() || !CastingSpell)
                        continue;

                    Player* p = static_cast<Player*>(this);
                    if (p->getShapeShiftForm() != FORM_CAT)
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
                    if (!this->isPlayer() || !CastingSpell || CastingSpell->getId() == 14189 || CastingSpell->getId() == 16953 || CastingSpell->getId() == 16959)
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
                    if (getHealthPct() > 30)
                        continue;
                }
                break;
                case 37309: //Bloodlust
                {
                    if (!this->isPlayer())
                        continue;
                    if (dynamic_cast<Player*>(this)->getShapeShiftForm() != FORM_BEAR &&
                        dynamic_cast<Player*>(this)->getShapeShiftForm() != FORM_DIREBEAR)
                        continue;
                }
                break;
                case 37310://Bloodlust
                {
                    if (!this->isPlayer() || static_cast<Player*>(this)->getShapeShiftForm() != FORM_CAT)
                        continue;
                }
                break;
                case 16459:
                {
                    //sword specialization
                    Item* item_mainhand = static_cast<Player*>(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    Item* item_offhand = static_cast<Player*>(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    uint32 reqskillMH = 0;
                    uint32 reqskillOH = 0;

                    if (item_mainhand != nullptr)
                        reqskillMH = GetSkillByProto(item_mainhand->getItemProperties()->Class, item_mainhand->getItemProperties()->SubClass);

                    if (item_offhand != nullptr)
                        reqskillOH = GetSkillByProto(item_offhand->getItemProperties()->Class, item_offhand->getItemProperties()->SubClass);

                    if (reqskillMH != SKILL_SWORDS && reqskillMH != SKILL_2H_SWORDS && reqskillOH != SKILL_SWORDS && reqskillOH != SKILL_2H_SWORDS)
                        continue;
                }
                break;
                case 12721:
                {
                    //deep wound requires a melee weapon
                    auto item = static_cast<Player*>(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    if (item)
                    {
                        //class 2 means weapons ;)
                        if (item->getItemProperties()->Class != 2)
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
                    Item* it = static_cast<Player*>(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    if (it == nullptr)
                        continue; //no weapon no joy
                    //float chance=float(it->GetProto()->Delay)*float(talentlevel)/600.0f;
                    uint32 chance = it->getItemProperties()->Delay * talentlevel / 300; //zack this had a very low proc rate. Kinda like a wasted talent
                    uint32 myroll = Util::getRandomUInt(100);
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
                    if (!(CastingSpell->getSchoolMask() & SCHOOL_MASK_FIRE))
                        continue;
                    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);   //we already modified this spell on server loading so it must exist
                    auto spell_duration = sSpellDurationStore.LookupEntry(spellInfo->getDurationIndex());
                    uint32 tickcount = GetDuration(spell_duration) / spellInfo->getEffectAmplitude(0);

                    if (ospinfo)
                        spell_proc->setOverrideEffectDamage(0, ospinfo->getEffectBasePoints(0) * damageInfo.realDamage / (100 * tickcount));
                }
                break;
                //druid - Primal Fury
                case 37116:
                case 37117:
                {
                    if (!this->isPlayer())
                        continue;
                    Player* mPlayer = static_cast<Player*>(this);
                    if (!mPlayer->IsInFeralForm() ||
                        (mPlayer->getShapeShiftForm() != FORM_CAT &&
                        mPlayer->getShapeShiftForm() != FORM_BEAR &&
                        mPlayer->getShapeShiftForm() != FORM_DIREBEAR))
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
                    if (flag & PROC_ON_DONE_PERIODIC)
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
                        if (ospinfo && damageInfo.realDamage < this->getHealth())    //if this is not a killer blow
                            can_proc_now = true;
                    }
                    else can_proc_now = true; //target died
                    if (can_proc_now == false)
                        continue;
                    Unit* new_caster = victim;
                    if (new_caster && new_caster->isAlive())
                    {
                        const auto spellInfo = sSpellMgr.getSpellInfo(spellId);   //we already modified this spell on server loading so it must exist
                        Spell* spell = sSpellMgr.newSpell(new_caster, spellInfo, true, NULL);
                        SpellCastTargets targets;
                        targets.setDestination(GetPosition());
                        spell->prepare(&targets);
                    }
                    spell_proc->deleteProc();
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
                        spell_proc->setOverrideEffectDamage(0, (ospinfo->calculateEffectValue(2)) * getMaxPower(POWER_TYPE_MANA) / 100);
                }
                break;
                // warlock - Unstable Affliction
                case 31117:
                {
                    //null check was made before like 2 times already :P
                    if (ospinfo)
                        spell_proc->setOverrideEffectDamage(0, (ospinfo->calculateEffectValue(0)) * 9);
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
                    auto skill_line_ability = sObjectMgr.GetSpellSkill(CastingSpell->getId());
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
                    if (!(CastingSpell->getSchoolMask() & SCHOOL_MASK_FIRE) &&
                        !(CastingSpell->getSchoolMask() & SCHOOL_MASK_SHADOW))
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
                            amount = CastingSpell->calculateEffectValue(0);
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
                            amount = CastingSpell->calculateEffectValue(1);
                        } break;
                        default:
                            amount = 0;

                    }

                    if (!amount)
                        continue;

                    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
                    Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, NULL);
                    spell->SetUnitTarget(this);
                    if (ospinfo)
                        doSpellHealing(this, spellId, amount * (ospinfo->calculateEffectValue(0)) / 100.0f, true);
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
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING) || !(CastingSpell->getSchoolMask() & SCHOOL_MASK_FIRE))
                        continue;
                    if (damageInfo.isCritical && spell_proc->getCreatedByAura() != nullptr)
                    {
                        auto procAura = spell_proc->getCreatedByAura();
                        procAura->setCharges(procAura->getCharges() + 1);
                        if (procAura->getCharges() >= 3)   //whatch that number cause it depends on original stack count !
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
                    if (victim == this || !(CastingSpell->getSchoolMask() & SCHOOL_MASK_FROST))
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
                    if (!isPlayer() || !damageInfo.realDamage)
                        continue;
                    SpellInfo const* parentproc = sSpellMgr.getSpellInfo(origId);
                    const auto spellInfo = sSpellMgr.getSpellInfo(spellId);
                    if (!parentproc || !spellInfo)
                        continue;
                    int32 val = parentproc->calculateEffectValue(0);
                    Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, NULL);
                    spell->forced_basepoints[0] = (val * damageInfo.realDamage) / 300; //per tick
                    SpellCastTargets targets(getGuid());
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
                    if (!isPlayer())
                        continue;
                    //!! The weird thing is that we need the spell that triggered this enchant spell in order to output logs ..we are using oldspell info too
                    //we have to recalc the value of this spell
                    const auto spellInfo = sSpellMgr.getSpellInfo(origId);
                    uint32 AP_owerride = spellInfo->calculateEffectValue(0);
                    uint32 dmg2 = static_cast<Player*>(this)->GetMainMeleeDamage(AP_owerride);
                    SpellInfo const* sp_for_the_logs = sSpellMgr.getSpellInfo(spellId);
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
                    if (isPlayer())
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
                    spell_proc->setOverrideEffectDamage(0, CastingSpell->getManaCost() * 40 / 100);
                }
                break;
                //priest - Reflective Shield
                case 33619:
                {
                    if (!damageInfo.absorbedDamage)
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
                        auto tdmg = damageInfo.absorbedDamage * (ospinfo->calculateEffectValue(0)) / 100.0f;
                        //somehow we should make this not caused any threat (to be done)
                        doSpellDamage(victim, power_word_id, tdmg, 0, true);
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
                    if (!isPlayer() || !damageInfo.realDamage)
                        continue;
                    //this needs offhand weapon
                    Item* it = static_cast<Player*>(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    if (it == nullptr || it->getItemProperties()->InventoryType != INVTYPE_WEAPON)
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

                    const auto spellInfo = sSpellMgr.getSpellInfo(54203);
                    auto spell_duration = sSpellDurationStore.LookupEntry(spellInfo->getDurationIndex());
                    uint32 tickcount = GetDuration(spell_duration) / spellInfo->getEffectAmplitude(0);
                    if (ospinfo)
                        spell_proc->setOverrideEffectDamage(0, ospinfo->getEffectBasePoints(0) * damageInfo.realDamage / (100 * tickcount));
                }
                break;

                //////////////////////////////////////////////////////////////////////////////////////////
                // WARRIOR

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
                    if (CastingSpell == nullptr)
                        continue;
                    //trigger only on heal spell cast by NOT us
                    if (!(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_HEALING) || this == victim)
                        continue;
                    //this is not counting the bonus effects on heal
                    auto idx = CastingSpell->firstBeneficialEffect();
                    if (idx != 1)
                    {
                        if (ospinfo)
                            spell_proc->setOverrideEffectDamage(0, ((CastingSpell->getEffectBasePoints(static_cast<uint8_t>(idx)) + 1) * (ospinfo->calculateEffectValue(0)) / 100));
                    }
                }
                break;
                //paladin - Light's Grace
                case 31834:
                {
                    if (CastingSpell == nullptr)
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
                    resisted_dmg = (damageInfo.realDamage / 2);
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
                    if (!isPlayer())
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
                            castSpell(this, 39805, true);
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
                    if (!CastingSpell || !(CastingSpell->getSchoolMask() & SCHOOL_MASK_SHADOW) || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                        continue;
                }
                break;
                case 37378:
                {
                    if (!CastingSpell || !(CastingSpell->getSchoolMask() & SCHOOL_MASK_FIRE) || !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
                        continue;
                }
                break;
                case 45062: // Vial of the Sunwell
                case 39950: // Wave Trance
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

                    switch (CastingSpell->getAreaAuraEffect())
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

        spell_proc->castSpell(victim, CastingSpell);

        if (origId == 39805)
        {
            RemoveAura(39805);          // Remove lightning overload aura after procing
        }

        if (spell_proc->getCreatedByAura() != nullptr)
            happenedProcs.push_back(spell_proc);
    }

    if (!happenedProcs.empty())
    {
        for (auto procItr = happenedProcs.begin(); procItr != happenedProcs.end();)
        {
            auto proc = *procItr;
            if (proc->getCreatedByAura() != nullptr)
                proc->getCreatedByAura()->removeCharge();

            procItr = happenedProcs.erase(procItr);
        }
    }

    // Leaving old hackfixes commented here -Appled
    /*switch (iter2->second.spellId)
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
        case 16188: // Druid - Nature's Swiftness
        {
            //if (CastingSpell->School!=SCHOOL_NATURE||(!sd->CastTime||sd->CastTime>10000)) continue;
            if (!(CastingSpell->getSchoolMask() & SCHOOL_MASK_NATURE) || spell_cast_time->CastTime == 0)
                continue;
        }
        break;
        case 16166:
        {
            if (!(CastingSpell->getSchoolMask() & SCHOOL_MASK_FIRE || CastingSpell->getSchoolMask() & SCHOOL_MASK_FROST || CastingSpell->getSchoolMask() & SCHOOL_MASK_NATURE))
                continue;
        }
        break;
        case 14177: // Cold blood will get removed on offensive spell
        {
            if (!(CastingSpell->getSpellFamilyFlags(0) & 0x6820206 || CastingSpell->getSpellFamilyFlags(1) & 0x240009))
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
    }*/

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
    for (std::list<DamageProc>::iterator i = m_damageShields.begin(); i != m_damageShields.end();)    // Deal Damage to Attacker
    {
        std::list<DamageProc>::iterator i2 = i++; //we should not proc on proc.. not get here again.. not needed.Better safe then sorry.
        if ((flag & (*i2).m_flags))
        {
            {
                if (const auto spellInfo = sSpellMgr.getSpellInfo((*i2).m_spellId))
                {
                    SendMessageToSet(SmsgSpellDamageShield(this->getGuid(), attacker->getGuid(), spellInfo->getId(), (*i2).m_damage, spellInfo->getSchoolMask()).serialise().get(), true);
                    addSimpleDamageBatchEvent((*i2).m_damage, this);
                }
            }
        }
    }
    m_damgeShieldsInUse = false;
}

bool Unit::IsInInstance()
{
    MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(this->GetMapId());
    if (pMapinfo)
        return (pMapinfo->type != INSTANCE_NULL);

    return false;
}

void Unit::CalculateResistanceReduction(Unit* pVictim, DamageInfo* dmg, SpellInfo const* ability, float ArmorPctReduce)
{
    float AverageResistance = 0.0f;
    float ArmorReduce;

    if ((*dmg).schoolMask == SCHOOL_MASK_NORMAL) // physical
    {
        if (this->isPlayer())
            ArmorReduce = PowerCostPctMod[0] + ((float)pVictim->getResistance(0) * (ArmorPctReduce + static_cast<Player*>(this)->CalcRating(PCR_ARMOR_PENETRATION_RATING)) / 100.0f);
        else
            ArmorReduce = 0.0f;

        if (ArmorReduce >= pVictim->getResistance(0)) // fully penetrated :O
            return;

        // double Reduction = double(pVictim->getResistance(0)) / double(pVictim->getResistance(0)+400+(85*getLevel()));
        // dmg reduction formula from xinef
        double Reduction = 0;
        if (getLevel() < 60)
            Reduction = double(pVictim->getResistance(0) - ArmorReduce) / double(pVictim->getResistance(0) + 400 + (85 * getLevel()));
        else if (getLevel() > 59 && getLevel() < DBC_PLAYER_LEVEL_CAP)
            Reduction = double(pVictim->getResistance(0) - ArmorReduce) / double(pVictim->getResistance(0) - 22167.5 + (467.5 * getLevel()));
        //
        else
            Reduction = double(pVictim->getResistance(0) - ArmorReduce) / double(pVictim->getResistance(0) + 10557.5);

        if (Reduction > 0.75f)
            Reduction = 0.75f;
        else if (Reduction < 0)
            Reduction = 0;

        if (Reduction)
            (*dmg).fullDamage = (uint32)((*dmg).fullDamage * (1 - Reduction)); // no multiply by 0
    }
    else
    {
        // applying resistance to other type of damage
        int32 RResist = float2int32((pVictim->getResistance((*dmg).getSchoolTypeFromMask()) + ((pVictim->getLevel() > getLevel()) ? (pVictim->getLevel() - this->getLevel()) * 5 : 0)) - PowerCostPctMod[(*dmg).getSchoolTypeFromMask()]);
        if (RResist < 0)
            RResist = 0;
        AverageResistance = ((float)(RResist) / (float)(getLevel() * 5) * 0.75f);
        if (AverageResistance > 0.75f)
            AverageResistance = 0.75f;

        // NOT WOWWIKILIKE but i think it's actually to add some fullresist chance from resistances
        if (!ability || !(ability->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY))
        {
            float Resistchance = (float)pVictim->getResistance((*dmg).getSchoolTypeFromMask()) / (float)pVictim->getLevel();
            Resistchance *= Resistchance;
            if (Util::checkChance(Resistchance))
                AverageResistance = 1.0f;
        }

        if (AverageResistance > 0)
            (*dmg).resistedDamage = (uint32)(((*dmg).fullDamage) * AverageResistance);
        else
            (*dmg).resistedDamage = 0;
    }
}

uint32 Unit::GetSpellDidHitResult(Unit* pVictim, uint32 weapon_damage_type, Spell* castingSpell)
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
    const auto ability = castingSpell->getSpellInfo();

    bool backAttack = !pVictim->isInFront(this);   // isInBack is bugged!
    uint32 vskill = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    //Victim Skill Base Calculation
    if (pVictim->isPlayer())
    {
        vskill = static_cast<Player*>(pVictim)->_GetSkillLineCurrent(SKILL_DEFENSE);
        if (weapon_damage_type != RANGED && !backAttack)                // block chance
        {
            block = static_cast<Player*>(pVictim)->getBlockPercentage(); //shield check already done in Update chances

            if (pVictim->m_stunned <= 0)                                // dodge chance
            {
                dodge = static_cast<Player*>(pVictim)->getDodgePercentage();
            }

            if (pVictim->can_parry && !pVictim->disarmed)               // parry chance
            {
                if (static_cast<Player*>(pVictim)->HasSpell(3127) || static_cast<Player*>(pVictim)->HasSpell(18848))
                {
                    parry = static_cast<Player*>(pVictim)->getParryPercentage();
                }
            }
        }
        victim_skill = float2int32(vskill + static_cast<Player*>(pVictim)->CalcRating(PCR_DEFENCE));
    }
    else                                                                // mob defensive chances
    {
        if (weapon_damage_type != RANGED && !backAttack)
            dodge = pVictim->getStat(STAT_AGILITY) / 14.5f;             // what is this value?
        victim_skill = pVictim->getLevel() * 5;

        if (pVictim->isCreature())
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
    if (this->isPlayer())
    {
        self_skill = 0;
        Player* pr = static_cast<Player*>(this);
        hitmodifier = pr->GetHitFromMeleeSpell();

        switch (weapon_damage_type)
        {
            case MELEE:   // melee main hand weapon
                it = disarmed ? NULL : pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                hitmodifier += pr->CalcRating(PCR_MELEE_HIT);
                self_skill = float2int32(pr->CalcRating(PCR_MELEE_MAIN_HAND_SKILL));
                break;
            case OFFHAND: // melee offhand weapon (dualwield)
                it = disarmed ? NULL : pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                hitmodifier += pr->CalcRating(PCR_MELEE_HIT);
                self_skill = float2int32(pr->CalcRating(PCR_MELEE_OFF_HAND_SKILL));
                break;
            case RANGED:  // ranged weapon
                it = disarmed ? NULL : pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
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
                    it = pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    hitmodifier += pr->CalcRating(PCR_MELEE_HIT);
                    self_skill = float2int32(pr->CalcRating(PCR_MELEE_MAIN_HAND_SKILL));
                } break;
                default:
                    break;
            }
        }

        if (it)
            SubClassSkill = GetSkillByProto(it->getItemProperties()->Class, it->getItemProperties()->SubClass);
        else
            SubClassSkill = SKILL_UNARMED;

        if (SubClassSkill == SKILL_FIST_WEAPONS)
            SubClassSkill = SKILL_UNARMED;

        //chances in feral form don't depend on weapon skill
        if (static_cast<Player*>(this)->IsInFeralForm())
        {
            uint8 form = static_cast<Player*>(this)->getShapeShiftForm();
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
        if (isCreature())
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
    if (pVictim->isPlayer() && pVictim->getStandState()) //every not standing state is>0
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
        if (pVictim->isPlayer())
            hitchance = std::max(hitchance, 95.0f + vsk * 0.1f + hitmodifier);      //wowwiki multiplier - 0.04 but i think 0.1 more balanced
        else
            hitchance = std::max(hitchance, 100.0f + vsk * 0.6f + hitmodifier);     //not wowwiki but more balanced
    }

    if (ability != nullptr && castingSpell != nullptr)
    {
        applySpellModifiers(SPELLMOD_HITCHANCE, &hitchance, ability, castingSpell);
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
    float Roll = Util::getRandomFloat(100.0f);
    uint32 r = 0;

    while (r < 4 && Roll > chances[r])
    {
        r++;
    }

    uint32 roll_results[5] = { SPELL_DID_HIT_MISS, SPELL_DID_HIT_DODGE, SPELL_DID_HIT_PARRY, SPELL_DID_HIT_BLOCK, SPELL_DID_HIT_SUCCESS };
    return roll_results[r];
}

DamageInfo Unit::Strike(Unit* pVictim, WeaponDamageType weaponType, SpellInfo const* ability, int32 add_damage, int32 pct_dmg_mod, uint32 exclusive_damage, bool isSpellTriggered, bool skip_hit_check, bool force_crit, Spell* castingSpell)
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //Unacceptable Cases Processing
    if (!pVictim || !pVictim->isAlive() || !isAlive() || IsStunned() || IsPacified() || IsFeared())
        return DamageInfo();

    if (!(ability && ability->getAttributesEx() & ATTRIBUTESEX_IGNORE_IN_FRONT) && !isInFront(pVictim))
    {
        if (isPlayer())
        {
#if VERSION_STRING < Mop
            static_cast<Player*>(this)->SendPacket(SmsgAttackSwingBadFacing().serialise().get());
#endif
            return DamageInfo();
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Variables Initialization
    DamageInfo dmg = DamageInfo();
    dmg.weaponType = weaponType;

    Item* it = NULL;

    float hitchance = 0.0f;
    float dodge = 0.0f;
    float parry = 0.0f;
    float glanc = 0.0f;
    float block = 0.0f;
    float crit = 0.0f;
    float crush = 0.0f;

    uint32 targetEvent = 0;
    uint32_t hit_status = HITSTATUS_NORMALSWING;

    VisualState vstate = VisualState::ATTACK;
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
        dmg.schoolMask = SchoolMask(ability->getSchoolMask());
    else
    {
        if (isCreature())
            dmg.schoolMask = SchoolMask(g_spellSchoolConversionTable[static_cast<Creature*>(this)->BaseAttackType]);
        else
            dmg.schoolMask = SCHOOL_MASK_NORMAL;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Victim Skill Base Calculation
    if (pVictim->isPlayer())
    {
        Player* plr = static_cast<Player*>(pVictim);
        vskill = plr->_GetSkillLineCurrent(SKILL_DEFENSE);

        if (!backAttack)
        {
            // not an attack from behind so we may dodge/parry/block

            //uint32 pClass = plr->getClass();
            //uint32 pLevel = (getLevel()> DBC_PLAYER_LEVEL_CAP) ? DBC_PLAYER_LEVEL_CAP : getLevel();

            if (dmg.weaponType != RANGED)
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
            Item* it2 = plr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
            if (it2 != nullptr && it2->getItemProperties()->InventoryType == INVTYPE_SHIELD)
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
        if (dmg.weaponType != RANGED && pVictim->m_stunned <= 0)
        {
            dodge = pVictim->getStat(STAT_AGILITY) / 14.5f;
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
        if (pVictim->isCreature())
        {
            if (c->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
            {
                victim_skill = std::max(victim_skill, ((int32)getLevel() + 3) * 5);     //used max to avoid situation when lowlvl hits boss.
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Attacker Skill Base Calculation
    if (this->isPlayer())
    {
        self_skill = 0;
        Player* pr = static_cast<Player*>(this);
        hitmodifier = pr->GetHitFromMeleeSpell();

        switch (dmg.weaponType)
        {
            case MELEE:   // melee main hand weapon
                it = disarmed ? NULL : pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                self_skill = float2int32(pr->CalcRating(PCR_MELEE_MAIN_HAND_SKILL));
                if (it)
                {
                    dmg.schoolMask = SchoolMask(g_spellSchoolConversionTable[it->getItemProperties()->Damage[0].Type]);
                    if (it->getItemProperties()->SubClass == ITEM_SUBCLASS_WEAPON_MACE)
                        ArmorPctReduce += m_ignoreArmorPctMaceSpec;
                }
                break;
            case OFFHAND: // melee offhand weapon (dualwield)
                it = disarmed ? NULL : pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                self_skill = float2int32(pr->CalcRating(PCR_MELEE_OFF_HAND_SKILL));
                hit_status |= HITSTATUS_DUALWIELD;//animation
                if (it)
                {
                    dmg.schoolMask = SchoolMask(g_spellSchoolConversionTable[it->getItemProperties()->Damage[0].Type]);
                    if (it->getItemProperties()->SubClass == ITEM_SUBCLASS_WEAPON_MACE)
                        ArmorPctReduce += m_ignoreArmorPctMaceSpec;
                }
                break;
            case RANGED:  // ranged weapon
                it = disarmed ? NULL : pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                self_skill = float2int32(pr->CalcRating(PCR_RANGED_SKILL));
                if (it)
                    dmg.schoolMask = SchoolMask(g_spellSchoolConversionTable[it->getItemProperties()->Damage[0].Type]);
                break;
        }

        if (it)
        {
            SubClassSkill = GetSkillByProto(it->getItemProperties()->Class, it->getItemProperties()->SubClass);
            if (SubClassSkill == SKILL_FIST_WEAPONS)
                SubClassSkill = SKILL_UNARMED;
        }
        else
            SubClassSkill = SKILL_UNARMED;


        //chances in feral form don't depend on weapon skill
        if (pr->IsInFeralForm())
        {
            uint8 form = pr->getShapeShiftForm();
            if (form == FORM_CAT || form == FORM_BEAR || form == FORM_DIREBEAR)
            {
                SubClassSkill = SKILL_FERAL_COMBAT;
                self_skill += pr->getLevel() * 5;
            }
        }

        self_skill += pr->_GetSkillLineCurrent(SubClassSkill);
        crit = static_cast<Player*>(this)->getMeleeCritPercentage();
    }
    else
    {
        self_skill = this->getLevel() * 5;
        if (isCreature())
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
    if (pVictim->isPlayer() && !this->isPlayer() && !ability && dmg.schoolMask == SCHOOL_MASK_NORMAL)
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

    if (this->isPlayer() && !pVictim->isPlayer() && !ability)
    {
        glanc = 5.0f + diffAcapped;

        if (glanc < 0)
            glanc = 0.0f;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Advanced Chances Modifications
    // by talents
    if (pVictim->isPlayer())
    {
        if (dmg.weaponType != RANGED)
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

    crit += pVictim->isPlayer() ? vsk * 0.04f : std::min(vsk * 0.2f, 0.0f);

    // http://www.wowwiki.com/Miss
    float misschance;
    float ask = -vsk;

    if (pVictim->isPlayer())
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

    if (ability != nullptr && castingSpell != nullptr)
    {
        applySpellModifiers(SPELLMOD_CRITICAL, &crit, ability, castingSpell);
        if (!skip_hit_check)
            applySpellModifiers(SPELLMOD_HITCHANCE, &hitchance, ability, castingSpell);
    }

    // by ratings
    crit -= pVictim->isPlayer() ? static_cast<Player*>(pVictim)->CalcRating(PCR_MELEE_CRIT_RESILIENCE) : 0.0f;

    if (crit < 0)
        crit = 0.0f;

    if (this->isPlayer())
    {
        Player* plr = static_cast<Player*>(this);
        hitmodifier += (dmg.weaponType == RANGED) ? plr->CalcRating(PCR_RANGED_HIT) : plr->CalcRating(PCR_MELEE_HIT);

        float expertise_bonus = plr->CalcRating(PCR_EXPERTISE);
#if VERSION_STRING != Classic
        if (dmg.weaponType == MELEE)
            expertise_bonus += plr->getExpertise();
        else if (dmg.weaponType == OFFHAND)
            expertise_bonus += plr->getOffHandExpertise();
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
    if (dmg.weaponType == RANGED)
    {
        dodge = 0.0f;
        parry = 0.0f;
        glanc = 0.0f;
    }

    if (this->isPlayer())
    {
        it = static_cast<Player*>(this)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);

        if (!ability && it != nullptr && (it->getItemProperties()->InventoryType == INVTYPE_WEAPON || it->getItemProperties()->InventoryType == INVTYPE_WEAPONOFFHAND))
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
    if (this->isPlayer() && ability && static_cast<Player*>(this)->m_finishingmovesdodge && ability->custom_c_is_flags & SPELL_FLAG_IS_FINISHING_MOVE)
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
    if (pVictim->isPlayer() && pVictim->getStandState())    //every not standing state is>0
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
        if (pVictim->isPlayer())
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

    //printf("%s:-\n", isPlayer() ? "Player" : "Mob");
    //printf(" miss: %.2f\n", chances[0]);
    //printf("dodge: %.2f\n", dodge);
    //printf("parry: %.2f\n", parry);
    //printf("glanc: %.2f\n", glanc);
    //printf("block: %.2f\n", block);
    //printf(" crit: %.2f\n", crit);
    //printf("crush: %.2f\n", crush);

    // roll
    float Roll = Util::getRandomFloat(100.0f);
    uint32 r = 0;
    while (r < 7 && Roll> chances[r])
    {
        r++;
    }
    if (force_crit)
        r = 5;
    // postroll processing

    //trigger hostile action in ai
    pVictim->GetAIInterface()->HandleEvent(EVENT_HOSTILEACTION, this, 0);

    switch (r)
    {
        case 0:     // miss
            hit_status |= HITSTATUS_MISS;
            if (pVictim->isCreature() && pVictim->GetAIInterface()->getNextTarget() == NULL)    // dirty ai agro fix
                pVictim->GetAIInterface()->AttackReaction(this, 1, 0);
            break;
        case 1:     //dodge
            if (pVictim->isCreature() && pVictim->GetAIInterface()->getNextTarget() == NULL)    // dirty ai agro fix
                pVictim->GetAIInterface()->AttackReaction(this, 1, 0);

            CALL_SCRIPT_EVENT(pVictim, OnTargetDodged)(this);
            CALL_SCRIPT_EVENT(this, OnDodged)(this);
            targetEvent = 1;
            vstate = VisualState::DODGE;
            pVictim->emote(EMOTE_ONESHOT_PARRYUNARMED); // Animation

            if (this->isPlayer() && this->getClass() == WARRIOR)
            {
                static_cast<Player*>(this)->AddComboPoints(pVictim->getGuid(), 1);
                static_cast<Player*>(this)->UpdateComboPoints();

                if (!sEventMgr.HasEvent(static_cast<Player*>(this), EVENT_COMBO_POINT_CLEAR_FOR_TARGET))
                    sEventMgr.AddEvent(static_cast<Player*>(this), &Player::NullComboPoints, (uint32)EVENT_COMBO_POINT_CLEAR_FOR_TARGET, (uint32)5000, (uint32)1, (uint32)0);
                else
                    sEventMgr.ModifyEventTimeLeft(static_cast<Player*>(this), EVENT_COMBO_POINT_CLEAR_FOR_TARGET, 5000, 0);
            }

            // Rune strike
#if VERSION_STRING > TBC
            if (pVictim->isPlayer() && pVictim->getClass() == DEATHKNIGHT)   // omg! dirty hack!
                pVictim->castSpell(pVictim, 56817, true);
#endif

            pVictim->addAuraStateAndAuras(AURASTATE_FLAG_DODGE_BLOCK_PARRY);
            if (!sEventMgr.HasEvent(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE))
                sEventMgr.AddEvent(pVictim, &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_DODGE_BLOCK_PARRY, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 1, 0);
            else sEventMgr.ModifyEventTimeLeft(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 0);
            break;
        case 2:     //parry
            if (pVictim->isCreature() && pVictim->GetAIInterface()->getNextTarget() == NULL) // dirty ai agro fix
                pVictim->GetAIInterface()->AttackReaction(this, 1, 0);

            CALL_SCRIPT_EVENT(pVictim, OnTargetParried)(this);
            CALL_SCRIPT_EVENT(this, OnParried)(this);
            targetEvent = 3;
            vstate = VisualState::PARRY;
            pVictim->emote(EMOTE_ONESHOT_PARRYUNARMED); // Animation

            if (pVictim->isPlayer())
            {
#if VERSION_STRING > TBC
                // Rune strike
                if (pVictim->getClass() == DEATHKNIGHT) // omg! dirty hack!
                    pVictim->castSpell(pVictim, 56817, true);
#endif

                pVictim->addAuraStateAndAuras(AURASTATE_FLAG_PARRY); // SB@L: Enables spells requiring parry
                if (!sEventMgr.HasEvent(pVictim, EVENT_PARRY_FLAG_EXPIRE))
                    sEventMgr.AddEvent(pVictim, &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_PARRY, EVENT_PARRY_FLAG_EXPIRE, 5000, 1, 0);
                else
                    sEventMgr.ModifyEventTimeLeft(pVictim, EVENT_PARRY_FLAG_EXPIRE, 5000);
                if (static_cast<Player*>(pVictim)->getClass() == 1 || static_cast<Player*>(pVictim)->getClass() == 4) // warriors for 'revenge' and rogues for 'riposte'
                {
                    pVictim->addAuraStateAndAuras(AURASTATE_FLAG_DODGE_BLOCK_PARRY); // SB@L: Enables spells requiring dodge
                    if (!sEventMgr.HasEvent(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE))
                        sEventMgr.AddEvent(pVictim, &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_DODGE_BLOCK_PARRY, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 1, 0);
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
                vstate = VisualState::IMMUNE;
            else
            {
                //////////////////////////////////////////////////////////////////////////////////////////
                //state proc initialization
                vproc |= PROC_ON_TAKEN_ANY_DAMAGE;
                if (dmg.weaponType == RANGED)
                {
                    if (ability != nullptr)
                    {
                        aproc |= PROC_ON_DONE_RANGED_SPELL_HIT;
                        vproc |= PROC_ON_TAKEN_RANGED_SPELL_HIT;
                    }
                    else
                    {
                        aproc |= PROC_ON_DONE_RANGED_HIT;
                        vproc |= PROC_ON_TAKEN_RANGED_HIT;
                    }
                }
                else
                {
                    if (ability != nullptr)
                    {
                        aproc |= PROC_ON_DONE_MELEE_SPELL_HIT;
                        vproc |= PROC_ON_TAKEN_MELEE_SPELL_HIT;
                    }
                    else
                    {
                        aproc |= PROC_ON_DONE_MELEE_HIT;
                        vproc |= PROC_ON_TAKEN_MELEE_HIT;
                    }

                    if (dmg.weaponType == OFFHAND)
                    {
                        aproc |= PROC_ON_DONE_OFFHAND_ATTACK;
                        vproc |= PROC_ON_TAKEN_OFFHAND_ATTACK;
                    }
                }
                //////////////////////////////////////////////////////////////////////////////////////////
                //base damage calculation
                if (exclusive_damage)
                    dmg.fullDamage = exclusive_damage;
                else
                {
                    if (dmg.weaponType == MELEE && ability)
                        dmg.fullDamage = CalculateDamage(this, pVictim, MELEE, ability->getSpellFamilyFlags(), ability);
                    else
                        dmg.fullDamage = CalculateDamage(this, pVictim, dmg.weaponType, 0, ability);
                }

                if (pct_dmg_mod > 0)
                    dmg.fullDamage = dmg.fullDamage * pct_dmg_mod / 100;

                dmg.fullDamage += add_damage;

                // \todo Don't really know why it was here. It should be calculated on Spel::CalculateEffect. Maybe it was bugged there...
                //  if (ability && ability->SpellGroupType)
                //  {
                //      spellModFlatIntValue(TO_UNIT(this)->SM_FDamageBonus, &dmg.full_damage, ability->SpellGroupType);
                //      spellModPercentageIntValue(TO_UNIT(this)->SM_PDamageBonus, &dmg.full_damage, ability->SpellGroupType);
                //  }
                //  else
                //  {
                //      spellModFlatIntValue(((Unit*)this)->SM_FMiscEffect,&dmg.full_damage,(uint64)1<<63);
                //      spellModPercentageIntValue(((Unit*)this)->SM_PMiscEffect,&dmg.full_damage,(uint64)1<<63);
                //  }

                dmg.fullDamage += pVictim->DamageTakenMod[dmg.getSchoolTypeFromMask()];
                if (dmg.weaponType == RANGED)
                {
                    dmg.fullDamage += pVictim->RangedDamageTaken;
                }

                if (ability && ability->getMechanicsType() == MECHANIC_BLEEDING)
                    disable_dR = true;


                dmg.fullDamage += float2int32(dmg.fullDamage * pVictim->DamageTakenPctMod[dmg.getSchoolTypeFromMask()]);

                if (dmg.schoolMask != SCHOOL_MASK_NORMAL)
                    dmg.fullDamage += float2int32(dmg.fullDamage * (GetDamageDonePctMod(dmg.getSchoolTypeFromMask()) - 1));

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
                            dmg.fullDamage += float2int32(dmg.fullDamage * pVictim->ModDamageTakenByMechPCT[MECHANIC_BLEEDING]);
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
                            dmg.fullDamage += float2int32(dmg.fullDamage * pVictim->ModDamageTakenByMechPCT[MECHANIC_BLEEDING]);
                            break;
                    }
                }

                //pet happiness state dmg modifier
                if (isPet() && !static_cast<Pet*>(this)->IsSummonedPet())
                    dmg.fullDamage = (dmg.fullDamage <= 0) ? 0 : float2int32(dmg.fullDamage * static_cast<Pet*>(this)->GetHappinessDmgMod());

                if (dmg.fullDamage < 0)
                    dmg.fullDamage = 0;
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
                            dmg.fullDamage = float2int32(damage_reduction * dmg.fullDamage);
                        }
                        hit_status |= HITSTATUS_GLANCING;
                    }
                    break;
                    //////////////////////////////////////////////////////////////////////////////////////////
                    //block
                    case 4:
                    {
                        Item* shield = static_cast<Player*>(pVictim)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                        if (shield != nullptr)
                        {
                            targetEvent = 2;
                            pVictim->emote(EMOTE_ONESHOT_PARRYSHIELD);// Animation

                            if (shield->getItemProperties()->InventoryType == INVTYPE_SHIELD)
                            {
                                float block_multiplier = (100.0f + static_cast<Player*>(pVictim)->m_modblockabsorbvalue) / 100.0f;
                                if (block_multiplier < 1.0f)block_multiplier = 1.0f;

                                dmg.blockedDamage = float2int32((shield->getItemProperties()->Block + ((static_cast<Player*>(pVictim)->m_modblockvaluefromspells + static_cast<Player*>(pVictim)->getCombatRating(PCR_BLOCK))) + ((pVictim->getStat(STAT_STRENGTH) / 2.0f) - 1.0f)) * block_multiplier);

                                if (Util::checkChance(m_BlockModPct))
                                    dmg.blockedDamage *= 2;
                            }
                            else
                            {
                                dmg.blockedDamage = 0;
                            }

                            if (dmg.fullDamage <= (int32)dmg.blockedDamage)
                                vstate = VisualState::BLOCK;
                            if (dmg.blockedDamage)
                            {
                                CALL_SCRIPT_EVENT(pVictim, OnTargetBlocked)(this, dmg.blockedDamage);
                                CALL_SCRIPT_EVENT(this, OnBlocked)(pVictim, dmg.blockedDamage);
                            }
                            if (pVictim->isPlayer())  //not necessary now but we'll have blocking mobs in future
                            {
                                pVictim->addAuraStateAndAuras(AURASTATE_FLAG_DODGE_BLOCK_PARRY); //SB@L: Enables spells requiring dodge
                                if (!sEventMgr.HasEvent(pVictim, EVENT_DODGE_BLOCK_FLAG_EXPIRE))
                                    sEventMgr.AddEvent(pVictim, &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_DODGE_BLOCK_PARRY, EVENT_DODGE_BLOCK_FLAG_EXPIRE, 5000, 1, 0);
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
                        //LogDebug("DEBUG: Critical Strike! Full_damage: %u" , dmg.full_damage);
                        if (ability != nullptr && castingSpell != nullptr)
                        {
                            applySpellModifiers(SPELLMOD_CRITICAL_DAMAGE, &dmg.fullDamage, ability, castingSpell);
                        }

                        //LogDebug("DEBUG: After CritMeleeDamageTakenPctMod: %u" , dmg.full_damage);
                        if (isPlayer())
                        {
                            if (dmg.weaponType != RANGED)
                            {
                                dmg.fullDamage += dmg.fullDamage * static_cast<Player*>(this)->m_modphyscritdmgPCT / 100;
                            }
                            if (!pVictim->isPlayer())
                                dmg.fullDamage += float2int32(dmg.fullDamage * static_cast<Player*>(this)->IncreaseCricticalByTypePCT[static_cast<Creature*>(pVictim)->GetCreatureProperties()->Type]);
                            //LogDebug("DEBUG: After IncreaseCricticalByTypePCT: %u" , dmg.full_damage);
                        }

                        if (dmg.weaponType == RANGED)
                            dmg.fullDamage = dmg.fullDamage - float2int32(dmg.fullDamage * CritRangedDamageTakenPctMod[dmg.getSchoolTypeFromMask()]);
                        else
                            dmg.fullDamage = dmg.fullDamage - float2int32(dmg.fullDamage * CritMeleeDamageTakenPctMod[dmg.getSchoolTypeFromMask()]);

                        if (pVictim->isPlayer())
                        {
                            //Resilience is a special new rating which was created to reduce the effects of critical hits against your character.
                            float dmg_reduction_pct = 2.0f * static_cast<Player*>(pVictim)->CalcRating(PCR_MELEE_CRIT_RESILIENCE) / 100.0f;
                            if (dmg_reduction_pct > 1.0f)
                                dmg_reduction_pct = 1.0f; //we cannot resist more then he is criticalling us, there is no point of the critical then :P
                            dmg.fullDamage = float2int32(dmg.fullDamage - dmg.fullDamage * dmg_reduction_pct);
                            //LogDebug("DEBUG: After Resilience check: %u" , dmg.full_damage);
                        }

                        if (pVictim->isCreature() && static_cast<Creature*>(pVictim)->GetCreatureProperties()->Rank != ELITE_WORLDBOSS)
                            pVictim->emote(EMOTE_ONESHOT_WOUNDCRITICAL);

                        CALL_SCRIPT_EVENT(pVictim, OnTargetCritHit)(this, dmg.fullDamage);
                        CALL_SCRIPT_EVENT(this, OnCritHit)(pVictim, dmg.fullDamage);
                    }
                    break;
                    //////////////////////////////////////////////////////////////////////////////////////////
                    //crushing blow
                    case 6:
                        hit_status |= HITSTATUS_CRUSHINGBLOW;
                        dmg.fullDamage = (dmg.fullDamage * 3) >> 1;
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
                uint32 dm = dmg.fullDamage;
                dmg.absorbedDamage = pVictim->absorbDamage(dmg.schoolMask, (uint32_t*)&dm);

                if (dmg.fullDamage > (int32)dmg.blockedDamage)
                {
                    uint32 sh = pVictim->ManaShieldAbsorb(dmg.fullDamage);
                    //////////////////////////////////////////////////////////////////////////////////////////
                    //armor reducing
                    if (sh)
                    {
                        dmg.fullDamage -= sh;
                        if (dmg.fullDamage && !disable_dR)
                            CalculateResistanceReduction(pVictim, &dmg, ability, ArmorPctReduce);
                        dmg.fullDamage += sh;
                        dmg.absorbedDamage += sh;
                    }
                    else if (!disable_dR)
                        CalculateResistanceReduction(pVictim, &dmg, ability, ArmorPctReduce);
                }

                if (dmg.schoolMask == SCHOOL_MASK_NORMAL)
                {
                    dmg.absorbedDamage += dmg.resistedDamage;
                    dmg.resistedDamage = 0;
                }

                int32_t realdamage = dmg.fullDamage - dmg.absorbedDamage - dmg.resistedDamage - dmg.blockedDamage;
                if (realdamage < 0)
                {
                    realdamage = 0;
                    vstate = VisualState::IMMUNE;
                    if (!(hit_status & HITSTATUS_BLOCK))
                        hit_status |= HITSTATUS_ABSORBED;
                    else
                        hit_status |= HITSTATUS_BLOCK;
                }
                dmg.realDamage = realdamage;
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
        dmg.fullDamage = pVictim->DoDamageSplitTarget(dmg.fullDamage, dmg.schoolMask, true);
        dmg.realDamage = dmg.fullDamage;
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //special states processing
    if (pVictim->isCreature())
    {
        if (pVictim->GetAIInterface() && (pVictim->GetAIInterface()->isAiState(AI_STATE_EVADE) ||
            (pVictim->GetAIInterface()->GetIsSoulLinked() && pVictim->GetAIInterface()->getSoullinkedWith() != this)))
        {
            vstate = VisualState::EVADE;
            dmg.realDamage = 0;
            dmg.fullDamage = 0;
            dmg.resistedDamage = 0;
        }
    }
    if (pVictim->isPlayer() && static_cast<Player*>(pVictim)->m_cheats.hasGodModeCheat == true)
    {
        dmg.resistedDamage = dmg.fullDamage; //godmode
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //dirty fixes
    //vstate=1-wound,2-dodge,3-parry,4-interrupt,5-block,6-evade,7-immune,8-deflect
    // the above code was remade it for reasons : damage shield needs moslty same flags as handleproc + dual wield should proc too ?

    //damage shield must come before handleproc to not loose 1 charge : spell gets removed before last charge
    if (dmg.realDamage > 0 && dmg.weaponType != OFFHAND)
    {
        pVictim->HandleProcDmgShield(vproc, this);
        HandleProcDmgShield(aproc, pVictim);
    }

    dmg.isCritical = hit_status & HITSTATUS_CRICTICAL;

    /*if (resisted_dmg)
    {
        dmg.resistedDamage += resisted_dmg;
        dmg.fullDamage -= resisted_dmg;
        dmg.realDamage = dmg.realDamage - resisted_dmg < 0 ? 0 : dmg.realDamage - resisted_dmg;
    }*/
    //////////////////////////////////////////////////////////////////////////////////////////
    //spells triggering
    if (dmg.realDamage > 0 && ability == 0)
    {
        if (isPlayer() && static_cast<Player*>(this)->m_onStrikeSpells.size())
        {
            SpellCastTargets targets(pVictim->getGuid());
            Spell* cspell;

            // Loop on hit spells, and strike with those.
            for (std::map<SpellInfo const*, std::pair<uint32, uint32>>::iterator itr = static_cast<Player*>(this)->m_onStrikeSpells.begin();
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
                    cspell = sSpellMgr.newSpell(this, itr->first, true, NULL);
                    cspell->prepare(&targets);
                }
                else
                {
                    cspell = sSpellMgr.newSpell(this, itr->first, true, NULL);
                    cspell->prepare(&targets);
                }
            }
        }

        if (isPlayer() && static_cast<Player*>(this)->m_onStrikeSpellDmg.size())
        {
            for (std::map<uint32, OnHitSpell>::iterator it2 = static_cast<Player*>(this)->m_onStrikeSpellDmg.begin(); it2 != static_cast<Player*>(this)->m_onStrikeSpellDmg.end();)
            {
                std::map<uint32, OnHitSpell>::iterator itr = it2;
                ++it2;

                uint32 dmg2 = itr->second.mindmg;
                uint32 range = itr->second.maxdmg - itr->second.mindmg;
                if (range != 0)
                    dmg2 += Util::getRandomUInt(range);

                doSpellDamage(pVictim, itr->second.spellid, static_cast<float_t>(dmg2), 0);
            }
        }

        //ugly hack for shadowfiend restoring mana
        if (getSummonedByGuid() != 0 && getEntry() == 19668)
        {
            Player* owner = GetMapMgr()->GetPlayer((uint32)getSummonedByGuid());
            if (owner)
            {
                uint32 amount = static_cast<uint32>(owner->getMaxPower(POWER_TYPE_MANA) * 0.05f);
                this->energize(owner, 34650, amount, POWER_TYPE_MANA);
            }
        }
        //ugly hack for Bloodsworm restoring hp
        if (getSummonedByGuid() != 0 && getEntry() == 28017)
        {
            Player* owner = GetMapMgr()->GetPlayer((uint32)getSummonedByGuid());
            if (owner != NULL)
                owner->addSimpleHealingBatchEvent(float2int32(1.5f * dmg.realDamage), owner, sSpellMgr.getSpellInfo(50452));
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //Data Sending
    if (!ability)
    {
        if (dmg.fullDamage > 0)
        {
            if (dmg.fullDamage == (int32)dmg.absorbedDamage)
                hit_status |= HITSTATUS_ABSORBED;
            else if (dmg.fullDamage <= (int32)dmg.resistedDamage)
            {
                hit_status |= HITSTATUS_RESIST;
                dmg.resistedDamage = dmg.fullDamage;
            }
        }

        if (dmg.fullDamage < 0)
            dmg.fullDamage = 0;
    }
    else
    {
        //FIX ME: add log for miss,block etc for ability and ranged
        //example how it works
        //SendSpellLog(this,pVictim,ability->getId(),SPELL_LOG_MISS);
    }

    if (ability && dmg.realDamage == 0)
    {
        SendSpellLog(this, pVictim, ability->getId(), SPELL_LOG_RESIST);
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //Damage Dealing

    if (this->isPlayer() && ability)
        static_cast<Player*>(this)->m_casted_amount[dmg.getSchoolTypeFromMask()] = dmg.realDamage + dmg.absorbedDamage;

    // Generate rage on damage done
    ///\ todo: this is inaccurate and almost directly copied here from few lines below
    uint32_t rageGenerated = 0;
    if (dmg.fullDamage > 0 && isPlayer() && getPowerType() == POWER_TYPE_RAGE && ability == nullptr)
    {
        float_t val = 0.0f;
        uint32_t level = getLevel();
        float_t conv = 0.0f;
        if (level <= DBC_PLAYER_LEVEL_CAP)
            conv = AttackToRageConversionTable[level];
        else
            conv = 3.75f / (0.0091107836f * level * level + 3.225598133f * level + 4.2652911f);

        // Hit Factor
        float_t f = (dmg.weaponType == OFFHAND) ? 1.75f : 3.5f;

        if (hit_status & HITSTATUS_CRICTICAL)
            f *= 2.0f;

        float_t s = 1.0f;

        // Weapon speed (normal)
        const auto weapon = (static_cast<Player*>(this)->getItemInterface())->GetInventoryItem(INVENTORY_SLOT_NOT_SET, (dmg.weaponType == OFFHAND ? EQUIPMENT_SLOT_OFFHAND : EQUIPMENT_SLOT_MAINHAND));
        if (weapon == nullptr)
        {
            if (dmg.weaponType == OFFHAND)
                s = getBaseAttackTime(OFFHAND) / 1000.0f;
            else
                s = getBaseAttackTime(MELEE) / 1000.0f;
        }
        else
        {
            const auto entry = weapon->getEntry();
            const auto pProto = sMySQLStore.getItemProperties(entry);
            if (pProto != nullptr)
            {
                s = pProto->Delay / 1000.0f;
            }
        }

        val = conv * dmg.fullDamage + f * s / 2.0f;
        val *= (1 + (static_cast<Player*>(this)->rageFromDamageDealt / 100.0f));
        const auto ragerate = worldConfig.getFloatRate(RATE_POWER2);
        val *= 10 * ragerate;

        if (val > 0)
        {
            rageGenerated = static_cast<uint32_t>(std::ceil(val));
            hit_status |= HITSTATUS_RAGE_GAIN;
        }
    }

    // Calculate estimated overkill based on current health and current health events in health batch
    const auto overKill = pVictim->calculateEstimatedOverKillForCombatLog(dmg.realDamage);
    if (ability == nullptr)
        sendAttackerStateUpdate(GetNewGUID(), pVictim->GetNewGUID(), HitStatus(hit_status), dmg.realDamage, overKill, dmg, dmg.absorbedDamage, vstate, dmg.blockedDamage, rageGenerated);
    else if (dmg.realDamage > 0)
        pVictim->sendSpellNonMeleeDamageLog(this, pVictim, ability, dmg.realDamage, dmg.absorbedDamage, dmg.resistedDamage, dmg.blockedDamage, overKill, false, hit_status & HITSTATUS_CRICTICAL);

    // invincible people don't take damage
    if (pVictim->bInvincible == false)
    {
        if (dmg.realDamage)
        {
            auto batch = new HealthBatchEvent;
            batch->caster = this;
            batch->damageInfo = dmg;
            batch->spellInfo = ability;

            pVictim->addHealthBatchEvent(batch);
            //pVictim->HandleProcDmgShield(PROC_ON_MELEE_ATTACK_VICTIM,this);
            // HandleProcDmgShield(PROC_ON_MELEE_ATTACK_VICTIM,pVictim);

            if (pVictim->isCastingSpell())
            {
                if (pVictim->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr && pVictim->getCurrentSpell(CURRENT_CHANNELED_SPELL)->getCastTimeLeft() > 0)
                    pVictim->getCurrentSpell(CURRENT_CHANNELED_SPELL)->AddTime(0);
                else if (pVictim->getCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr && pVictim->getCurrentSpell(CURRENT_GENERIC_SPELL)->getCastTimeLeft() > 0)
                    pVictim->getCurrentSpell(CURRENT_GENERIC_SPELL)->AddTime(0);
            }
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
    // proc handling
    HandleProc(aproc, pVictim, ability, dmg, isSpellTriggered);
    pVictim->HandleProc(vproc, this, ability, dmg, isSpellTriggered);

    //durability processing
    if (pVictim->isPlayer())
    {
        static_cast<Player*>(pVictim)->getItemInterface()->ReduceItemDurability();
        if (!this->isPlayer())
        {
            Player* pr = static_cast<Player*>(pVictim);
            if (Util::checkChance(pr->GetSkillUpChance(SKILL_DEFENSE) * worldConfig.getFloatRate(RATE_SKILLCHANCE)))
            {
                pr->_AdvanceSkillLine(SKILL_DEFENSE, float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE)));
                pr->UpdateChances();
            }
        }
        else
        {
            static_cast<Player*>(this)->getItemInterface()->ReduceItemDurability();
        }
    }
    else
    {
        if (this->isPlayer())//not pvp
        {
            static_cast<Player*>(this)->getItemInterface()->ReduceItemDurability();
            Player* pr = static_cast<Player*>(this);
            if (Util::checkChance(pr->GetSkillUpChance(SubClassSkill) * worldConfig.getFloatRate(RATE_SKILLCHANCE)))
            {
                pr->_AdvanceSkillLine(SubClassSkill, float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE)));
                //pr->UpdateChances();
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    //rage processing
    //http://www.wowwiki.com/Formulas:Rage_generation

    if (rageGenerated > 0)
    {
        modPower(POWER_TYPE_RAGE, static_cast<int32_t>(rageGenerated));
        if (getPower(POWER_TYPE_RAGE) > 1000)
            modPower(POWER_TYPE_RAGE, 1000 - getPower(POWER_TYPE_RAGE));
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
            Strike(pVictim, dmg.weaponType, NULL, 0, 0, 0, false, false);
        }

        m_extraAttackCounter = false;
    }

    if (m_extrastriketargetc > 0 && !m_extrastriketarget)
    {
        m_extrastriketarget = true;

        for (std::list<ExtraStrike*>::iterator itx = m_extraStrikeTargets.begin(); itx != m_extraStrikeTargets.end();)
        {
            std::list<ExtraStrike*>::iterator itx2 = itx++;
            ExtraStrike* ex = *itx2;

            for (const auto& itr : getInRangeObjectsSet())
            {
                if (!itr || itr == pVictim || !itr->isCreatureOrPlayer())
                    continue;

                if (CalcDistance(itr) < 5.0f && isAttackable(this, itr) && itr->isInFront(this) && !static_cast<Unit*>(itr)->IsPacified())
                {
                    // Sweeping Strikes hits cannot be dodged, missed or parried (from wowhead)
                    bool skip_hit_check2 = ex->spell_info->getId() == 12328 ? true : false;
                    //zack : should we use the spell id the registered this extra strike when striking ? It would solve a few proc on proc problems if so ;)
                    // Strike(TO<Unit*>(*itr), weapon_damage_type, ability, add_damage, pct_dmg_mod, exclusive_damage, false, skip_hit_check);
                    Strike(static_cast<Unit*>(itr), dmg.weaponType, ex->spell_info, add_damage, pct_dmg_mod, exclusive_damage, false, skip_hit_check2);
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

    return dmg;
}

void Unit::smsg_AttackStop(Unit* pVictim)
{
    if (pVictim)
        SendMessageToSet(SmsgAttackStop(GetNewGUID(), pVictim->GetNewGUID()).serialise().get(), true);
    else
        SendMessageToSet(SmsgAttackStop(GetNewGUID(), WoWGuid()).serialise().get(), true);

    if (pVictim)
    {
        if (pVictim->isPlayer())
        {
            pVictim->CombatStatusHandler_ResetPvPTimeout();
            CombatStatusHandler_ResetPvPTimeout();
        }
        else
        {
            if (!isPlayer() || getClass() == ROGUE)
            {
                m_cTimer = Util::getMSTime() + 8000;
                sEventMgr.RemoveEvents(this, EVENT_COMBAT_TIMER);
                sEventMgr.AddEvent(this, &Unit::EventUpdateFlag, EVENT_COMBAT_TIMER, 8000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                if (pVictim->isCreatureOrPlayer())   // there could be damage coming from objects/enviromental
                    sEventMgr.AddEvent(pVictim, &Unit::EventUpdateFlag, EVENT_COMBAT_TIMER, 8000, 1, 0);
            }
        }
    }
}

void Unit::smsg_AttackStart(Unit* pVictim)
{
    SendMessageToSet(SmsgAttackStart(getGuid(), pVictim->getGuid()).serialise().get(), false);

    LOG_DEBUG("WORLD: Sent SMSG_ATTACKSTART");

    // FLAGS changed so other players see attack animation
    //    addUnitFlag(UNIT_FLAG_COMBAT);
    //    setUpdateMaskBit(UNIT_FIELD_FLAGS);
    if (isPlayer())
    {
        Player* pThis = static_cast<Player*>(this);
        if (pThis->cannibalize)
        {
            sEventMgr.RemoveEvents(pThis, EVENT_CANNIBALIZE);
            pThis->setEmoteState(EMOTE_ONESHOT_NONE);
            pThis->cannibalize = false;
        }
    }
}

bool Unit::RemoveAura(Aura* aur)
{
    if (aur == NULL)
        return false;

    aur->removeAura();
    return true;
}

bool Unit::RemoveAura(uint32 spellId)
{
    //this can be speed up, if we know passive \pos neg
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x] && m_auras[x]->getSpellId() == spellId)
        {
            m_auras[x]->removeAura();
            return true;  // sky: yes, only one, see bug charges/auras queues
        }
    }
    return false;
}

bool Unit::RemoveAuras(uint32* SpellIds)
{
    if (!SpellIds || *SpellIds == 0)
        return false;

    bool res = false;
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x])
        {
            for (uint32 y = 0; SpellIds[y] != 0; y++)
            {
                if (m_auras[x] && m_auras[x]->getSpellId() == SpellIds[y])
                {
                    m_auras[x]->removeAura();
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
            switch (m_auras[x]->getSpellId())
            {
                // remove after heal
                case 35321:
                case 38363:
                case 39215:
                {
                    m_auras[x]->removeAura();
                    res = true;
                }
                break;
                // remove when healed to 100%
                case 31956:
                case 38801:
                case 43093:
                {
                    if (getHealth() == getMaxHealth())
                    {
                        m_auras[x]->removeAura();
                        res = true;
                    }
                }
                break;
                // remove at p% health
                case 38772:
                {
                    uint32 p = m_auras[x]->getSpellInfo()->getEffectBasePoints(1);
                    if (getMaxHealth() * p <= getHealth() * 100)
                    {
                        m_auras[x]->removeAura();
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

        a->removeAura();
    }
}

bool Unit::RemoveAura(uint32 spellId, uint64 guid)
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x])
        {
            if (m_auras[x]->getSpellId() == spellId && m_auras[x]->getCasterGuid() == guid)
            {
                m_auras[x]->removeAura();
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
            if (m_auras[x]->getSpellId() == spellId && m_auras[x]->itemCasterGUID == guid)
            {
                m_auras[x]->removeAura();
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
            if (m_auras[x]->getSpellInfo()->isDeathPersistent())
                continue;
            else
                m_auras[x]->removeAura();
        }
    }
}

void Unit::RemoveAllAuras()
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x])
            m_auras[x]->removeAura();
    }
}

void Unit::RemoveAllNonPersistentAuras()
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x])
        {
            if (m_auras[x]->getSpellInfo()->isDeathPersistent())
                continue;

            m_auras[x]->removeAura();
        }
    }
}

//ex:to remove morph spells
void Unit::RemoveAllAuraType(uint32 auratype)
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x] && m_auras[x]->hasAuraEffect(static_cast<AuraEffect>(auratype)))
            m_auras[x]->removeAura();//remove all morph auras containing to this spell (like wolf morph also gives speed)
}

bool Unit::SetAurDuration(uint32 spellId, Unit* caster, uint32 duration)
{
    LOG_DEBUG("setAurDuration2");
    Aura* aur = getAuraWithIdForGuid(spellId, caster->getGuid());
    if (!aur)
        return false;
    aur->setTimeLeft(duration);
    sEventMgr.ModifyEventTimeLeft(aur, EVENT_AURA_REMOVE, duration);

    return true;
}

bool Unit::SetAurDuration(uint32 spellId, uint32 duration)
{
    Aura* aur = getAuraWithId(spellId);
    if (!aur)
        return false;

    LOG_DEBUG("setAurDuration2");
    aur->setTimeLeft(duration);
    sEventMgr.ModifyEventTimeLeft(aur, EVENT_AURA_REMOVE, duration);

    return true;
}

void Unit::DeMorph()
{
    // hope it solves it :)
    uint32 displayid = this->getNativeDisplayId();
    this->setDisplayId(displayid);
    EventModelChange();
}

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
    data << getGuid();
    data << uint32(0); // new in 2.1.0
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

void Unit::addToInRangeObjects(Object* pObj)
{
    if (pObj->isCreatureOrPlayer())
    {
        if (isHostile(this, pObj))
            addInRangeOppositeFaction(pObj);

        if (isFriendly(this, pObj))
            addInRangeSameFaction(pObj);
    }

    Object::addToInRangeObjects(pObj);
}//427

void Unit::onRemoveInRangeObject(Object* pObj)
{
    removeObjectFromInRangeOppositeFactionSet(pObj);
    removeObjectFromInRangeSameFactionSet(pObj);

    if (pObj->isCreatureOrPlayer())
    {
        Unit* pUnit = static_cast<Unit*>(pObj);
        GetAIInterface()->CheckTarget(pUnit);

        if (getCharmGuid() == pObj->getGuid())
            interruptSpell();
    }
}

void Unit::clearInRangeSets()
{
    Object::clearInRangeSets();
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
        }
    }
}

void Unit::CalcDamage()
{
    if (isPlayer())
        static_cast<Player*>(this)->CalcDamage();
    else
    {
        if (isPet())
            static_cast<Pet*>(this)->UpdateAP();

        float ap_bonus = GetAP() / 14000.0f;

        float bonus = ap_bonus * (getBaseAttackTime(MELEE) + static_cast<Creature*>(this)->m_speedFromHaste);

        float delta = float(static_cast<Creature*>(this)->ModDamageDone[0]);
        float mult = float(static_cast<Creature*>(this)->ModDamageDonePct[0]);
        float r = (BaseDamage[0] + bonus) * mult + delta;
        // give some diversity to pet damage instead of having a 77-78 damage range (as an example)
        setMinDamage(r > 0 ? (isPet() ? r * 0.9f : r) : 0);
        r = (BaseDamage[1] + bonus) * mult + delta;
        setMaxDamage(r > 0 ? (isPet() ? r * 1.1f : r) : 0);

        // setMinRangedDamage(BaseRangedDamage[0]*mult+delta);
        // setMaxRangedDamage(BaseRangedDamage[1]*mult+delta);
    }
}

// returns absorbed dmg
uint32 Unit::ManaShieldAbsorb(uint32 dmg)
{
    if (!m_manashieldamt)
        return 0;
    //mana shield group->16. the only

    uint32 mana = getPower(POWER_TYPE_MANA);

    int32 potential = (mana * 50) / 100;
    if (potential > m_manashieldamt)
        potential = m_manashieldamt;

    if ((int32)dmg < potential)
        potential = dmg;

    uint32 cost = (potential * 100) / 50;

    setPower(POWER_TYPE_MANA, mana - cost);
    m_manashieldamt -= potential;
    if (!m_manashieldamt)
        RemoveAura(m_manaShieldId);
    return potential;
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

void Unit::RemoveAurasByInterruptFlag(uint32 flag)
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        Aura* a = m_auras[x];
        if (a == NULL)
            continue;

        //some spells do not get removed all the time only at specific intervals
        if (a->getSpellInfo()->getAuraInterruptFlags() & flag)
        {
            a->removeAura();
            m_auras[x] = NULL;
        }
    }
}

bool Unit::HasAuraVisual(uint32 visualid)
{
    //passive auras do not have visual (at least when code was written)
    for (uint32 x = MAX_REMOVABLE_AURAS_START; x < MAX_REMOVABLE_AURAS_END; x++)
        if (m_auras[x] && m_auras[x]->getSpellInfo()->getSpellVisual(0) == visualid)
            return true;

    return false;
}

bool Unit::HasAura(uint32 spellid)
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x] && m_auras[x]->getSpellId() == spellid)
            return true;

    return false;
}

Aura* Unit::GetAuraWithSlot(uint32 slot)
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x] && m_auras[x]->m_visualSlot == (uint16)slot)
            return m_auras[x];

    return NULL;
}

uint16 Unit::GetAuraStackCount(uint32 spellid)
{
    uint16 count = 0;
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
        if (m_auras[x] && m_auras[x]->getSpellId() == spellid)
            count++;

    return count;
}

void Unit::DropAurasOnDeath()
{
    for (uint32 x = MAX_REMOVABLE_AURAS_START; x < MAX_REMOVABLE_AURAS_END; x++)
    {
        if (m_auras[x])
        {
            if (m_auras[x] && m_auras[x]->getSpellInfo()->isDeathPersistent())
                continue;

            m_auras[x]->removeAura();
        }
    }
}

void Unit::UpdateSpeed()
{
    if (getMountDisplayId() == 0)
    {
        setSpeedRate(TYPE_RUN, getSpeedRate(TYPE_RUN, false) * (1.0f + static_cast<float>(m_speedModifier) / 100.0f), true);
    }
    else
    {
        setSpeedRate(TYPE_RUN, getSpeedRate(TYPE_RUN, false) * (1.0f + static_cast<float>(m_mountedspeedModifier) / 100.0f), true);
        setSpeedRate(TYPE_RUN, getSpeedRate(TYPE_RUN, true) + (m_speedModifier < 0 ? (getSpeedRate(TYPE_RUN, false) * static_cast<float>(m_speedModifier) / 100.0f) : 0), true);
    }

    setSpeedRate(TYPE_FLY, getSpeedRate(TYPE_FLY, false) * (1.0f + ((float)m_flyspeedModifier) / 100.0f), true);

    // Limit speed due to effects such as http://www.wowhead.com/?spell=31896 [Judgement of Justice]
    if (m_maxSpeed && getSpeedRate(TYPE_RUN, true) > m_maxSpeed)
    {
        setSpeedRate(TYPE_RUN, m_maxSpeed, true);
    }

    if (isPlayer() && static_cast<Player*>(this)->m_changingMaps)
    {
        static_cast<Player*>(this)->resend_speed = true;
    }
    else
    {
        setSpeedRate(TYPE_RUN, getSpeedRate(TYPE_RUN, true), true);
        setSpeedRate(TYPE_FLY, getSpeedRate(TYPE_FLY, true), true);
    }
}

bool Unit::HasBuff(uint32 spellid) // cebernic:it does not check passive auras & must be visible auras
{
    for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_POSITIVE_AURAS_EXTEDED_END; x++)
        if (m_auras[x] && m_auras[x]->getSpellId() == spellid)
            return true;

    return false;
}

bool Unit::HasBuff(uint32 spellid, uint64 guid)
{
    for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_POSITIVE_AURAS_EXTEDED_END; x++)
        if (m_auras[x] && m_auras[x]->getSpellId() == spellid && m_auras[x]->getCasterGuid() == guid)
            return true;

    return false;
}

void Unit::RemoveAurasByBuffType(uint32 buff_type, const uint64 & guid, uint32 skip)
{
    uint64 sguid = buff_type >= SPELL_TYPE_BLESSING ? guid : 0;

    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x]  //have aura
            && (m_auras[x]->getSpellInfo()->custom_BGR_one_buff_on_target & buff_type) // aura is in same group
            && m_auras[x]->getSpellId() != skip // make sure to not do self removes in case aura will stack
            && (!sguid || (sguid && m_auras[x]->getCasterGuid() == sguid)) // we either remove everything or just buffs from us
            )
        {
            m_auras[x]->removeAura();
        }
    }
}

bool Unit::HasAurasOfBuffType(uint32 buff_type, const uint64 & guid, uint32 skip)
{
    uint64 sguid = (buff_type == SPELL_TYPE_BLESSING || buff_type == SPELL_TYPE_WARRIOR_SHOUT) ? guid : 0;

    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        if (m_auras[x] && m_auras[x]->getSpellInfo()->custom_BGR_one_buff_on_target & buff_type && m_auras[x]->getSpellId() != skip)
            if (!sguid || (m_auras[x]->getCasterGuid() == sguid))
                return true;
    }

    return false;
}

AuraCheckResponse Unit::AuraCheck(SpellInfo const* proto, Object* /*caster*/)
{
    AuraCheckResponse resp;

    // no error for now
    resp.Error = AURA_CHECK_RESULT_NONE;
    resp.Misc = 0;

    uint32 name_hash = proto->custom_NameHash;
    uint32 rank = proto->custom_RankNumber;

    // look for spells with same namehash
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        Aura* aura = m_auras[x];
        if (aura != NULL && aura->getSpellInfo()->custom_NameHash == name_hash)
        {
            // we've got an aura with the same name as the one we're trying to apply
            // but first we check if it has the same effects
            SpellInfo const* aura_sp = aura->getSpellInfo();

            if ((aura_sp->getEffect(0) == proto->getEffect(0) && (aura_sp->getEffect(0) != SPELL_EFFECT_APPLY_AURA ||
                aura_sp->getEffectApplyAuraName(0) == proto->getEffectApplyAuraName(0))) &&
                (aura_sp->getEffect(1) == proto->getEffect(1) && (aura_sp->getEffect(1) != SPELL_EFFECT_APPLY_AURA ||
                aura_sp->getEffectApplyAuraName(1) == proto->getEffectApplyAuraName(1))) &&
                (aura_sp->getEffect(2) == proto->getEffect(2) && (aura_sp->getEffect(2) != SPELL_EFFECT_APPLY_AURA ||
                aura_sp->getEffectApplyAuraName(2) == proto->getEffectApplyAuraName(2))))
            {
                resp.Misc = aura->getSpellInfo()->getId();

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

AuraCheckResponse Unit::AuraCheck(SpellInfo const* proto, Aura* aur, Object* /*caster*/)
{
    AuraCheckResponse resp;
    SpellInfo const* aura_sp = aur->getSpellInfo();

    // no error for now
    resp.Error = AURA_CHECK_RESULT_NONE;
    resp.Misc = 0;

    // look for spells with same namehash
    if (aur->getSpellInfo()->custom_NameHash == proto->custom_NameHash)
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
            resp.Misc = aur->getSpellInfo()->getId();

            // compare the rank to our applying spell
            if (aur->getSpellInfo()->custom_RankNumber > proto->custom_RankNumber)
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

#if VERSION_STRING >= WotLK
    if (getVehicleComponent() != NULL)
        getVehicleComponent()->InstallAccessories();

    z_axisposition = 0.0f;
#endif
}

//! Remove Unit from world
void Unit::RemoveFromWorld(bool free_guid)
{
    if (getCurrentVehicle() != NULL)
        getCurrentVehicle()->EjectPassenger(this);

    if (getVehicleComponent() != NULL)
    {
        getVehicleComponent()->RemoveAccessories();
        getVehicleComponent()->EjectAllPassengers();
    }

    removeVehicleComponent();

    CombatStatus.OnRemoveFromWorld();
#if VERSION_STRING > TBC
    if (getCritterGuid() != 0)
    {
        setCritterGuid(0);

        if (Unit* u = m_mapMgr->GetUnit(getCritterGuid()))
            u->Delete();
    }
#endif

    if (dynObj != 0)
        dynObj->Remove();

    for (uint8 i = 0; i < 4; ++i)
    {
        if (m_ObjectSlots[i] != 0)
        {
            if (GameObject* obj = m_mapMgr->GetGameObject(m_ObjectSlots[i]))
                obj->ExpireAndDelete();

            m_ObjectSlots[i] = 0;
        }
    }

    ClearAllAreaAuraTargets();
    RemoveAllAreaAuraByOther();

    // Attempt to prevent memory corruption
    for (auto& obj : getInRangeObjectsSet())
    {
        if (!obj->isCreatureOrPlayer())
            continue;

        static_cast<Unit*>(obj)->clearCasterFromHealthBatch(this);
    }

    Object::RemoveFromWorld(free_guid);

    //zack: should relocate new events to new eventmanager and not to -1
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x] != 0)
        {
            if (m_auras[x]->m_deleted)
            {
                m_auras[x] = NULL;
                continue;
            }
            m_auras[x]->RelocateEvents();
        }
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
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; x++)
    {
        Aura* a = m_auras[x];
        if (a == nullptr)
            continue;

        //some spells do not get removed all the time only at specific intervals
        if ((a->getSpellInfo()->getAuraInterruptFlags() & flag) && (a->getSpellInfo()->getId() != skip))
        {
            a->removeAura();
        }
    }
}

bool Unit::HasAuraWithName(uint32 name)
{

    for (uint32 i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
        if (m_auras[i] != NULL && m_auras[i]->getSpellInfo()->appliesAreaAura(name))
            return true;

    return false;
}

uint32 Unit::GetAuraCountWithName(uint32 name)
{
    uint32 count = 0;

    for (uint32 i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
        if (m_auras[i] != NULL && m_auras[i]->getSpellInfo()->appliesAreaAura(name))
            ++count;

    return count;
}

bool Unit::HasAuraWithMechanics(uint32 mechanic)
{
    for (uint32 x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_NEGATIVE_AURAS_EXTEDED_END; ++x)
        if (m_auras[x] && m_auras[x]->getSpellInfo())
            if (Spell::GetMechanic(m_auras[x]->getSpellInfo()) == mechanic)
                return true;

    return false;
}

bool Unit::IsPoisoned()
{
    for (uint32 x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_NEGATIVE_AURAS_EXTEDED_END; ++x)
        if (m_auras[x] && m_auras[x]->getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_POISON)
            return true;

    return false;
}

void Unit::RemoveAurasOfSchool(uint32 School, bool Positive, bool Immune)
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x]
            && m_auras[x]->getSpellInfo()->getFirstSchoolFromSchoolMask() == School
            && (m_auras[x]->isNegative() || Positive)
            && (!Immune && m_auras[x]->getSpellInfo()->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY)
            )
        {
            m_auras[x]->removeAura();
        }
    }
}

bool Unit::IsDazed()
{
    for (uint32 x = MAX_TOTAL_AURAS_START; x < MAX_TOTAL_AURAS_END; ++x)
    {
        if (m_auras[x])
        {
            if (m_auras[x]->getSpellInfo()->getMechanicsType() == MECHANIC_ENSNARED)
                return true;

            for (uint8_t y = 0; y < 3; y++)
                if (m_auras[x]->getSpellInfo()->getEffectMechanic(y) == MECHANIC_ENSNARED)
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

    if (isPlayer())
    {
        Player* plr = static_cast<Player*>(this);
        for (const auto& itr2 : getInRangeObjectsSet())
        {
            if (itr2)
            {
                Object* pObj = itr2;

                can_see = plr->canSee(pObj);
                is_visible = plr->IsVisible(pObj->getGuid());
                if (can_see)
                {
                    if (!is_visible)
                    {
                        buf.clear();
                        count = pObj->buildCreateUpdateBlockForPlayer(&buf, plr);
                        plr->getUpdateMgr().pushCreationData(&buf, count);
                        plr->AddVisibleObject(pObj->getGuid());
                    }
                }
                else
                {
                    if (is_visible)
                    {
                        plr->sendDestroyObjectPacket(pObj->getGuid());
                        plr->RemoveVisibleObject(pObj->getGuid());
                    }
                }

                if (pObj->isPlayer())
                {
                    Player* pl = static_cast<Player*>(pObj);
                    can_see = pl->canSee(plr);
                    is_visible = pl->IsVisible(plr->getGuid());
                    if (can_see)
                    {
                        if (!is_visible)
                        {
                            buf.clear();
                            count = plr->buildCreateUpdateBlockForPlayer(&buf, pl);
                            pl->getUpdateMgr().pushCreationData(&buf, count);
                            pl->AddVisibleObject(plr->getGuid());
                        }
                    }
                    else
                    {
                        if (is_visible)
                        {
                            pl->sendDestroyObjectPacket(plr->getGuid());
                            pl->RemoveVisibleObject(plr->getGuid());
                        }
                    }
                }
            }
        }
    }
    else // For units we can save a lot of work
    {
        for (const auto& it2 : getInRangePlayersSet())
        {
            Player* p = static_cast<Player*>(it2);
            if (p)
            {
                can_see = p->canSee(this);
                is_visible = p->IsVisible(this->getGuid());
                if (!can_see)
                {
                    if (is_visible)
                    {
                        p->sendDestroyObjectPacket(getGuid());
                        p->RemoveVisibleObject(getGuid());
                    }
                }
                else
                {
                    if (!is_visible)
                    {
                        buf.clear();
                        count = buildCreateUpdateBlockForPlayer(&buf, p);
                        p->getUpdateMgr().pushCreationData(&buf, count);
                        p->AddVisibleObject(this->getGuid());
                    }
                }
            }
        }
    }
}

int32 Unit::GetAP()
{
    int32 baseap = getAttackPower() + getAttackPowerMods();
    float totalap = baseap * (getAttackPowerMultiplier() + 1);
    if (totalap >= 0)
        return float2int32(totalap);
    return 0;
}

int32 Unit::GetRAP()
{
    int32 baseap = getRangedAttackPower() + getRangedAttackPowerMods();
    float totalap = baseap * (getRangedAttackPowerMultiplier() + 1);
    if (totalap >= 0)
        return float2int32(totalap);
    return 0;
}

bool Unit::GetSpeedDecrease()
{
    int32 before = m_speedModifier;
    m_speedModifier -= m_slowdown;
    m_slowdown = 0;
    
    for (auto& itr : speedReductionMap)
        m_slowdown = static_cast<int32>(std::min(m_slowdown, itr.second));

    if (m_slowdown < -100)
        m_slowdown = 100; //do not walk backwards !

    m_speedModifier += m_slowdown;
    //save bandwidth :P
    if (m_speedModifier != before)
        return true;

    return false;
}

#if VERSION_STRING == TBC
void Unit::SetFacing(float newo)
{
    SetOrientation(newo);

    //generate smsg_monster_move
    WorldPacket data(SMSG_MONSTER_MOVE, 60);

    data << GetNewGUID();

    data << GetPositionX();
    data << GetPositionY();
    data << GetPositionZ();
    data << Util::getMSTime();
    if (newo != 0.0f)
    {
        data << uint8(4);
        data << newo;
    }
    else
    {
        data << uint8(0);
    }

    data << uint32(0x1000); //move flags: run
    data << uint32(0); //movetime
    data << uint32(1); //1 point
    data << GetPositionX();
    data << GetPositionY();
    data << GetPositionZ();

    SendMessageToSet(&data, true);
}
#else
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
    data << Util::getMSTime();
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
#endif

float Unit::get_chance_to_daze(Unit* target)
{
    if (target->getLevel() < CREATURE_DAZE_MIN_LEVEL) // since 3.3.0
        return 0.0f;
    float attack_skill = getLevel() * 5.0f;
    float defense_skill;
    if (target->isPlayer())
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
    for (HealedSet::iterator i = m_healers.begin(); i != m_healers.end(); ++i)
    {
        Player* pt = m_Unit->GetMapMgr()->GetPlayer(*i);
        if (pt != NULL)
            pt->CombatStatus.RemoveHealed(m_Unit);
    }

    m_healers.clear();
}

void CombatStatusHandler::RemoveHealed(Unit* pHealTarget)
{
    m_healed.erase(pHealTarget->getGuidLow());
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
            //printf(I64FMT" is now in combat.\n", m_Unit->getGuid());
            m_Unit->addUnitFlags(UNIT_FLAG_COMBAT);
            if (!m_Unit->hasUnitStateFlag(UNIT_STATE_ATTACKING)) m_Unit->addUnitStateFlag(UNIT_STATE_ATTACKING);
        }
        else
        {
            //printf(I64FMT" is no longer in combat.\n", m_Unit->getGuid());
            m_Unit->removeUnitFlags(UNIT_FLAG_COMBAT);
            if (m_Unit->hasUnitStateFlag(UNIT_STATE_ATTACKING)) m_Unit->removeUnitStateFlag(UNIT_STATE_ATTACKING);

            // remove any of our healers from combat too, if they are able to be.
            ClearMyHealers();

            if (m_Unit->isPlayer())
                static_cast<Player*>(m_Unit)->UpdatePotionCooldown();
        }
    }
}

bool CombatStatusHandler::InternalIsInCombat()
{
    if (m_Unit->isPlayer() && m_Unit->GetMapMgr() && m_Unit->GetMapMgr()->IsCombatInProgress())
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
    if (guid == m_Unit->getGuid())
       return;

    //we MUST be in world
    ARCEMU_ASSERT(m_Unit->IsInWorld());

    m_attackTargets.insert(guid);
    //printf("Adding attack target " I64FMT " to " I64FMT "\n", guid, m_Unit->getGuid());
    if (m_Unit->isPlayer() &&
        m_primaryAttackTarget != guid) // players can only have one attack target.
    {
        if (m_primaryAttackTarget)
            ClearPrimaryAttackTarget();

        m_primaryAttackTarget = guid;
    }

    UpdateFlag();
}

void CombatStatusHandler::ClearPrimaryAttackTarget()
{
    //printf("ClearPrimaryAttackTarget for " I64FMT "\n", m_Unit->getGuid());
    if (m_primaryAttackTarget != 0)
    {
        if (Unit* pt = m_Unit->GetMapMgr()->GetUnit(m_primaryAttackTarget))
        {
            // remove from their attacker set. (if we have no longer got any DoT's, etc)
            if (!IsAttacking(pt))
            {
                pt->CombatStatus.RemoveAttacker(m_Unit, m_Unit->getGuid());
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
            if (m_Unit->getGuid() == pTarget->m_auras[i]->getCasterGuid() && pTarget->m_auras[i]->IsCombatStateAffecting())
                return true;

    // place any additional checks here
    return false;
}

void CombatStatusHandler::RemoveAttackTarget(Unit* pTarget)
{
    // called on aura remove, etc.
    AttackerMap::iterator itr = m_attackTargets.find(pTarget->getGuid());
    if (itr == m_attackTargets.end())
        return;

   if (!IsAttacking(pTarget))
    {
        m_attackTargets.erase(itr);
        if (m_primaryAttackTarget == pTarget->getGuid())
            m_primaryAttackTarget = 0;

        UpdateFlag();
    }
}

void CombatStatusHandler::RemoveAttacker(Unit* pAttacker, const uint64 & guid)
{
    AttackerMap::iterator itr = m_attackers.find(guid);
    if (itr == m_attackers.end())
        return;

    if ((!pAttacker) || (!pAttacker->CombatStatus.IsAttacking(m_Unit)))
    {
        m_attackers.erase(itr);
        UpdateFlag();
    }
}

void CombatStatusHandler::OnDamageDealt(Unit* pTarget)
{
    // we added an aura, or dealt some damage to a target. they need to have us as an attacker, and they need to be our attack target if not.
    if (pTarget == m_Unit)
        return;

    //no need to be in combat if dead
    if (!pTarget->isAlive() || !m_Unit->isAlive())
        return;

    AttackerMap::iterator itr = m_attackTargets.find(pTarget->getGuid());
    if (itr == m_attackTargets.end())
        AddAttackTarget(pTarget->getGuid());

    itr = pTarget->CombatStatus.m_attackers.find(m_Unit->getGuid());
    if (itr == pTarget->CombatStatus.m_attackers.end())
        pTarget->CombatStatus.AddAttacker(m_Unit->getGuid());

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
    for (AttackerMap::iterator itr = m_attackTargets.begin(); itr != m_attackTargets.end(); ++itr)
    {
        Unit* pt = m_Unit->GetMapMgr()->GetUnit(*itr);
        if (pt)
        {
            pt->CombatStatus.m_attackers.erase(m_Unit->getGuid());
            pt->CombatStatus.UpdateFlag();
        }
    }

    for (AttackerMap::iterator itr = m_attackers.begin(); itr != m_attackers.end(); ++itr)
    {
        Unit* pt = m_Unit->GetMapMgr()->GetUnit(*itr);
        if (pt)
        {
            pt->CombatStatus.m_attackTargets.erase(m_Unit->getGuid());
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
    for (HealedSet::iterator itr = m_healed.begin(); itr != m_healed.end(); ++itr)
    {
        Player* pt = m_Unit->GetMapMgr()->GetPlayer(*itr);
        if (pt)
        {
            pt->CombatStatus.m_healers.erase(m_Unit->getGuidLow());
            pt->CombatStatus.UpdateFlag();
        }
    }

    for (HealedSet::iterator itr = m_healers.begin(); itr != m_healers.end(); ++itr)
    {
        Player* pt = m_Unit->GetMapMgr()->GetPlayer(*itr);
        if (pt)
        {
            pt->CombatStatus.m_healed.erase(m_Unit->getGuidLow());
            pt->CombatStatus.UpdateFlag();
        }
    }

    m_healed.clear();
    m_healers.clear();
    UpdateFlag();
}

void Unit::CombatStatusHandler_ResetPvPTimeout()
{
    if (!isPlayer())
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
    if (m_Unit->isPlayer())
        static_cast<Player*>(m_Unit)->removePlayerFlags(PLAYER_FLAG_PVP_GUARD_ATTACKABLE);

    for (AttackerMap::iterator i = m_attackTargets.begin(); i != m_attackTargets.end();)
    {
        AttackerMap::iterator i2 = i++;
        Unit* pt = m_Unit->GetMapMgr()->GetUnit(*i2);
        if (pt == NULL)
        {
            m_attackTargets.erase(i2);
            continue;
        }

        RemoveAttackTarget(pt);
        pt->CombatStatus.RemoveAttacker(m_Unit, m_Unit->getGuid());
    }
}

void CombatStatusHandler::AttackersForgetHate()
{
    for (AttackerMap::iterator i = m_attackTargets.begin(); i != m_attackTargets.end();)
    {
        AttackerMap::iterator i2 = i++;
        Unit* pt = m_Unit->GetMapMgr()->GetUnit(*i2);
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

    switch (m_Unit->getObjectTypeId())
    {
        case TYPEID_UNIT:
        {
            if (m_Unit->isPet() && static_cast<Pet*>(m_Unit)->GetPetAction() == PET_ACTION_ATTACK)
                return true;
            else if (m_Unit->isPet())
                return m_lastStatus;

            return m_Unit->GetAIInterface()->getAITargetsCount() == 0 ? false : true;
        }
        case TYPEID_PLAYER:
        {
            std::list<Pet*> summons = static_cast<Player*>(m_Unit)->GetSummons();
            for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
                if ((*itr)->getPlayerOwner() == m_Unit && (*itr)->CombatStatus.IsInCombat())
                    return true;

            return m_lastStatus;
        }
        default:
            return false;
    }
}

void CombatStatusHandler::WeHealed(Unit* pHealTarget)
{
    if (!pHealTarget->isPlayer() || !m_Unit->isPlayer() || pHealTarget == m_Unit)
        return;

    if (pHealTarget->CombatStatus.IsInCombat())
    {
        m_healed.insert(pHealTarget->getGuidLow());
        pHealTarget->CombatStatus.m_healers.insert(m_Unit->getGuidLow());
    }

    UpdateFlag();
}

void CombatStatusHandler::OnRemoveFromWorld()
{
    ClearAttackers();
    ClearHealers();
}

void Unit::DispelAll(bool positive)
{
    for (uint32 i = MAX_TOTAL_AURAS_START; i < MAX_TOTAL_AURAS_END; ++i)
    {
        if (m_auras[i] != NULL)
            if (!m_auras[i]->isNegative() && positive || m_auras[i]->isNegative())
                m_auras[i]->removeAura();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// bool Unit::RemoveAllAurasByMechanic (renamed from MechanicImmunityMassDispel)
// Removes all auras on this unit that are of a specific mechanic.
// Useful for things like.. Apply Aura: Immune Mechanic, where existing (de)buffs are *always supposed* to be removed.
// I'm not sure if this goes here under unit.
//
// \param uint32 MechanicType
// \return False if no buffs were dispelled, true if more than 0 were dispelled.
//////////////////////////////////////////////////////////////////////////////////////////

// MaxDispel was set to -1 which will led to a uint32 of 4294967295
bool Unit::RemoveAllAurasByMechanic(uint32 MechanicType, uint32 /*MaxDispel = 0*/, bool HostileOnly = true)
{
    LogDebugFlag(LF_AURA, "Unit::MechanicImmunityMassDispel called, mechanic: %u" , MechanicType);
    uint32 DispelCount = 0;
    for (uint32 x = (HostileOnly ? MAX_NEGATIVE_AURAS_EXTEDED_START : MAX_POSITIVE_AURAS_EXTEDED_START); x < MAX_REMOVABLE_AURAS_END; x++)    // If HostileOnly = 1, then we use aura slots 40-56 (hostile). Otherwise, we use 0-56 (all)
    {
        // This check is will never be true since DispelCount is 0 and MaxDispel was 4294967295!
        /*if (DispelCount >= MaxDispel && MaxDispel > 0)
            return true;*/

        if (m_auras[x])
        {
            if (m_auras[x]->getSpellInfo()->getMechanicsType() == MechanicType)   // Remove all mechanics of type MechanicType (my english goen boom)
            {
                LogDebugFlag(LF_AURA, "Removed aura. [AuraSlot %u, SpellId %u]", x, m_auras[x]->getSpellId());
                ///\todo Stop moving if fear was removed.
                m_auras[x]->removeAura(); // EZ-Remove
                DispelCount++;
            }
            else if (MechanicType == MECHANIC_ENSNARED)   // if got immunity for slow, remove some that are not in the mechanics
            {
                for (uint8 i = 0; i < 3; i++)
                {
                    // SNARE + ROOT
                    if (m_auras[x]->getSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_MOD_DECREASE_SPEED || m_auras[x]->getSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_MOD_ROOT)
                    {
                        m_auras[x]->removeAura();
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
            if (m_auras[x]->getSpellInfo()->getMechanicsType() == MECHANIC_ROOTED
                || m_auras[x]->getSpellInfo()->getMechanicsType() == MECHANIC_ENSNARED
                || m_auras[x]->getSpellInfo()->getMechanicsType() == MECHANIC_DAZED)

            {
                m_auras[x]->removeAura();
            }
            else
            {
                for (uint8 i = 0; i < 3; i++)
                {
                    if (m_auras[x]->getSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_MOD_DECREASE_SPEED
                        || m_auras[x]->getSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_MOD_ROOT)
                    {
                        m_auras[x]->removeAura();
                        break;
                    }
                }
            }
        }
    }
}

void Unit::ReplaceAIInterface(AIInterface* new_interface)
{
    delete m_aiInterface; //be careful when you do this. Might screw unit states !
    m_aiInterface = new_interface;
}

void Unit::EventUpdateFlag()
{
    CombatStatus.UpdateFlag();
}

void Unit::EventModelChange()
{
    MySQLStructure::DisplayBoundingBoxes const* displayBoundingBox = sMySQLStore.getDisplayBounding(getDisplayId());

    //\todo if has mount, grab mount model and add the z value of attachment 0
    if (displayBoundingBox != nullptr)
        m_modelhalfsize = displayBoundingBox->high[2] / 2;
    else
        m_modelhalfsize = 1.0f;
}

void Unit::RemoveFieldSummon()
{
    uint64 guid = getSummonGuid();
    if (guid && GetMapMgr())
    {
        Creature* summon = static_cast<Creature*>(GetMapMgr()->GetUnit(guid));
        if (summon)
            summon->RemoveFromWorld(false, true);
        setSummonGuid(0);
    }
}

void Unit::AggroPvPGuards()
{
    for (const auto& i : getInRangeObjectsSet())
    {
        if (i && i->isCreature())
        {
            Unit* tmpUnit = static_cast<Unit*>(i);
            if (tmpUnit->GetAIInterface() && tmpUnit->GetAIInterface()->m_isNeutralGuard && CalcDistance(tmpUnit) <= (50.0f * 50.0f))
                tmpUnit->GetAIInterface()->AttackReaction(this, 1, 0);
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
        if (t_trigger_on_stun_chance < 100 && !Util::checkChance(t_trigger_on_stun_chance))
            return;

        const auto spellInfo = sSpellMgr.getSpellInfo(t_trigger_on_stun);

        if (!spellInfo)
            return;

        Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, NULL);
        SpellCastTargets targets;

        if (proc_target)
            targets.setUnitTarget(proc_target->getGuid());
        else
            targets.setUnitTarget(getGuid());

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
        if (t_trigger_on_chill_chance < 100 && !Util::checkChance(t_trigger_on_chill_chance))
            return;

        const auto spellInfo = sSpellMgr.getSpellInfo(t_trigger_on_chill);
        if (!spellInfo)
            return;

        Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, NULL);
        SpellCastTargets targets;

        if (proc_target)
            targets.setUnitTarget(proc_target->getGuid());
        else
            targets.setUnitTarget(getGuid());

        spell->prepare(&targets);
    }
}

void Unit::RemoveExtraStrikeTarget(SpellInfo const* spell_info)
{
    for (std::list<ExtraStrike*>::iterator i = m_extraStrikeTargets.begin(); i != m_extraStrikeTargets.end(); ++i)
    {
        ExtraStrike* es = *i;
        if (spell_info == es->spell_info)
        {
            m_extrastriketargetc--;
            m_extraStrikeTargets.erase(i);
            delete es;
            break;
        }
    }
}

void Unit::AddExtraStrikeTarget(SpellInfo const* spell_info, uint32 charges)
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

uint32 Unit::DoDamageSplitTarget(uint32 res, SchoolMask schoolMask, bool melee_dmg)
{
    DamageSplitTarget* ds = m_damageSplitTarget;

    Unit* splittarget = (GetMapMgr() != NULL) ? GetMapMgr()->GetUnit(ds->m_target) : NULL;
    if (splittarget != NULL && res > 0)
    {
        // calculate damage
        uint32 tmpsplit = ds->m_flatDamageSplit;
        if (tmpsplit > res)
            tmpsplit = res; // prevent <0 damage

        uint32 splitdamage = tmpsplit;
        res -= tmpsplit;
        tmpsplit = float2int32(ds->m_pctDamageSplit * res);
        if (tmpsplit > res)
            tmpsplit = res;

        splitdamage += tmpsplit;
        res -= tmpsplit;

        if (splitdamage)
        {
            splittarget->dealDamage(splittarget, splitdamage, 0);

            // Send damage log
            if (melee_dmg)
            {
                DamageInfo sdmg;
                sdmg.fullDamage = splitdamage;
                sdmg.schoolMask = schoolMask;
                sendAttackerStateUpdate(GetNewGUID(), splittarget->GetNewGUID(), HITSTATUS_NORMALSWING, splitdamage, 0, sdmg, 0, VisualState::ATTACK, 0, 0);
            }
            else
            {
                uint32_t overKill = 0;
                if (splitdamage > splittarget->getHealth())
                    overKill = splitdamage - splittarget->getHealth();

                splittarget->sendSpellNonMeleeDamageLog(this, splittarget, sSpellMgr.getSpellInfo(ds->m_spellId), splitdamage, 0, 0, 0, overKill, false, false);
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
    {
        if (spellid == (*i)->spellId)
        {
            delete *i;
            i = m_reflectSpellSchool.erase(i);
            //break; better check all list elements
        }
        else
        {
            ++i;
        }
    }


    if (apply && spellid == 23920 && isPlayer())
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
            Group* pGroup = pPlayer->getGroup();
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
                        pPlayer->castSpell(member, 59725, true);
                        targets -= 1;
                    }
                }
                pGroup->Unlock();
            }
        }
    }

    if (!apply && spellid == 59725 && isPlayer())
    {
        Player* pPlayer = static_cast<Player*>(this);
        Group* pGroup = pPlayer->getGroup();
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

void Unit::AddGarbageAura(Aura* aur)
{
    m_GarbageAuras.push_back(aur);
}

void Unit::AddGarbagePet(Pet* pet)
{
    ARCEMU_ASSERT(pet->getPlayerOwner()->getGuid() == getGuid() && !pet->IsInWorld());
    m_GarbagePets.push_back(pet);
}

void Unit::RemoveGarbage()
{
    for (std::list<Aura*>::iterator itr1 = m_GarbageAuras.begin(); itr1 != m_GarbageAuras.end(); ++itr1)
    {
        Aura* aur = *itr1;
        delete aur;
    }

    for (std::list<Pet*>::iterator itr3 = m_GarbagePets.begin(); itr3 != m_GarbagePets.end(); ++itr3)
    {
        Pet* pet = *itr3;
        delete pet;
    }

    m_GarbageAuras.clear();
    m_GarbagePets.clear();
}

void Unit::Tag(uint64 TaggerGUID)
{
    Tagged = true;
    this->TaggerGuid = TaggerGUID;
    addDynamicFlags(U_DYN_FLAG_TAGGED_BY_OTHER);
}

void Unit::UnTag()
{
    Tagged = false;
    TaggerGuid = 0;
    removeDynamicFlags(U_DYN_FLAG_TAGGED_BY_OTHER);
}

bool Unit::IsTagged()
{
    return Tagged;
}

bool Unit::IsTaggable()
{
    if (!isPet() && !Tagged)
        return true;

    return false;
}

uint64 Unit::GetTaggerGUID()
{
    return TaggerGuid;
}

bool Unit::isLootable()
{
    if (IsTagged() && !isPet() && !(isPlayer() && !IsInBg()) && (getCreatedByGuid() == 0) && !isVehicle())
    {
        auto creature_prop = sMySQLStore.getCreatureProperties(getEntry());
        if (isCreature() && !sLootMgr.HasLootForCreature(getEntry()) && creature_prop != nullptr && (creature_prop->money == 0))  // Since it is inworld we can safely assume there is a proto cached with this Id!
            return false;

        return true;
    }
    
    return false;
}

void Unit::Die(Unit* /*pAttacker*/, uint32 /*damage*/, uint32 /*spellid*/)
{}

void Unit::Phase(uint8 command, uint32 newphase)
{
    Object::Phase(command, newphase);

    for (const auto& itr : getInRangeObjectsSet())
    {
        if (itr && itr->isCreatureOrPlayer())
            static_cast<Unit*>(itr)->UpdateVisibility();
    }

    UpdateVisibility();
}

uint32 Unit::GetAuraCountWithDispelType(uint32 dispel_type, uint64 guid)
{
    uint32 result = 0;

    for (auto& m_aura : m_auras)
    {
        if (m_aura == nullptr)
            continue;

        if (m_aura->getSpellInfo()->getDispelType() == dispel_type && (guid == 0 || m_aura->getCasterGuid() == guid))
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
    const auto spellInfo = sSpellMgr.getSpellInfo(GetOnMeleeSpell());
    Spell* spell = sSpellMgr.newSpell(this, spellInfo, true, NULL);
    spell->extra_cast_number = GetOnMeleeSpellEcn();
    SpellCastTargets targets(getTargetGuid());
    spell->prepare(&targets);
    SetOnMeleeSpell(0);
}

void Unit::BuildMovementPacket(ByteBuffer* data)
{
    *data << uint32(getUnitMovementFlags());            // movement flags
#if VERSION_STRING == TBC
    *data << uint8(getExtraUnitMovementFlags());        // 2.3.0
#elif VERSION_STRING >= WotLK
    *data << uint16(getExtraUnitMovementFlags());       // 3.x.x
#endif
    *data << uint32(Util::getMSTime());                 // time / counter
    *data << GetPositionX();
    *data << GetPositionY();
    *data << GetPositionZ();
    *data << GetOrientation();

#if VERSION_STRING < Cata
    // 0x00000200
    if (hasUnitMovementFlag(MOVEFLAG_TRANSPORT))
    {
        if (isPlayer())
        {
            const auto plr = dynamic_cast<Player*>(this);
            if (plr->obj_movement_info.hasMovementFlag(MOVEFLAG_TRANSPORT))
            {
                obj_movement_info.transport_guid = plr->obj_movement_info.transport_guid;
            }
        }
        if (Unit* u = getVehicleBase())
            obj_movement_info.transport_guid = u->getGuid();
        *data << obj_movement_info.transport_guid;
        *data << obj_movement_info.transport_guid;
        *data << GetTransOffsetX();
        *data << GetTransOffsetY();
        *data << GetTransOffsetZ();
        *data << GetTransOffsetO();
        *data << GetTransTime();
#ifdef FT_VEHICLES
        *data << GetTransSeat();

        // TODO what is this in BC?
        if (getExtraUnitMovementFlags() & MOVEFLAG2_INTERPOLATED_MOVE)
            *data << getMovementInfo()->transport_time2;
#endif
    }

    // 0x02200000
    if ((getUnitMovementFlags() & (MOVEFLAG_SWIMMING | MOVEFLAG_FLYING))
        || (getExtraUnitMovementFlags() & MOVEFLAG2_ALLOW_PITCHING))
        *data << getMovementInfo()->pitch_rate;

    *data << getMovementInfo()->fall_time;
#endif
    // 0x00001000
#if VERSION_STRING < Cata
    if (getUnitMovementFlags() & MOVEFLAG_FALLING)
    {
        *data << getMovementInfo()->jump_info.velocity;
        *data << getMovementInfo()->jump_info.sinAngle;
        *data << getMovementInfo()->jump_info.cosAngle;
        *data << getMovementInfo()->jump_info.xyspeed;
    }

    // 0x04000000
    if (getUnitMovementFlags() & MOVEFLAG_SPLINE_ELEVATION)
        *data << getMovementInfo()->spline_elevation;
#endif
}


void Unit::BuildMovementPacket(ByteBuffer* data, float x, float y, float z, float o)
{
    *data << uint32(getUnitMovementFlags());            // movement flags
#if VERSION_STRING == TBC
    *data << uint8(getExtraUnitMovementFlags());        // 2.3.0
#elif VERSION_STRING >= WotLK
    *data << uint16(getExtraUnitMovementFlags());       // 3.x.x
#endif
    *data << uint32(Util::getMSTime());                 // time / counter
    *data << x;
    *data << y;
    *data << z;
    *data << o;

#if VERSION_STRING < Cata
    // 0x00000200
    if (hasUnitMovementFlag(MOVEFLAG_TRANSPORT))
    {
        // Code left commented for reference
        // TODO: Research whether vehicle transport guid is being updated correctly or not (and if not, update it elsewhere and remove this)
        /*if (isPlayer() && static_cast<Player*>(this)->m_transport)
            obj_movement_info.transporter_info.guid = static_cast<Player*>(this)->m_transport->getGuid();
        if (Unit* u = getVehicleBase())
            obj_movement_info.transporter_info.guid = u->getGuid();*/
        *data << obj_movement_info.transport_guid;
        *data << GetTransOffsetX();
        *data << GetTransOffsetY();
        *data << GetTransOffsetZ();
        *data << GetTransOffsetO();
        *data << GetTransTime();
#ifdef FT_VEHICLES
        *data << GetTransSeat();

        if (getExtraUnitMovementFlags() & MOVEFLAG2_INTERPOLATED_MOVE)
            *data << getMovementInfo()->transport_time2;
#endif
    }

    // 0x02200000
    if ((getUnitMovementFlags() & (MOVEFLAG_SWIMMING | MOVEFLAG_FLYING))
        || (getExtraUnitMovementFlags() & MOVEFLAG2_ALLOW_PITCHING))
        *data << getMovementInfo()->pitch_rate;

    *data << getMovementInfo()->fall_time;
#endif
    // 0x00001000
#if VERSION_STRING < Cata
    if (getUnitMovementFlags() & MOVEFLAG_FALLING)
    {
        *data << getMovementInfo()->jump_info.velocity;
        *data << getMovementInfo()->jump_info.sinAngle;
        *data << getMovementInfo()->jump_info.cosAngle;
        *data << getMovementInfo()->jump_info.xyspeed;
    }

    // 0x04000000
    if (getUnitMovementFlags() & MOVEFLAG_SPLINE_ELEVATION)
        *data << getMovementInfo()->spline_elevation;
#endif
}

void Unit::UpdateAuraForGroup(uint8 slot)
{
    if (slot >= 64)
        return;

    if (isPlayer())
    {
        auto player = dynamic_cast<Player*>(this);
        if (player->getGroup())
        {
            player->AddGroupUpdateFlag(GROUP_UPDATE_FLAG_AURAS);
            player->SetAuraUpdateMaskForRaid(slot);
        }
    }
    else if (Player* owner = getPlayerOwner())
    {
        if (owner->getGroup())
        {
            owner->AddGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_AURAS);
            SetAuraUpdateMaskForRaid(slot);
        }
    }
}

void Unit::Possess(Unit* pTarget, uint32 delay)
{
    Player* pThis = nullptr;
    if (isPlayer())
        pThis = dynamic_cast<Player*>(this);
    else // do not support creatures just yet
        return;

    if (!pThis)
        return;
    if (getCharmGuid())
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

    pThis->m_CurrentCharm = pTarget->getGuid();
    if (pTarget->isCreature())
    {
        // unit-only stuff.
        pTarget->setAItoUse(false);
        pTarget->GetAIInterface()->StopMovement(0);
        pTarget->m_redirectSpellPackets = pThis;
        pTarget->mPlayerControler = pThis;
    }

    m_noInterrupt++;
    setCharmGuid(pTarget->getGuid());
    pTarget->setCharmedByGuid(getGuid());
    pTarget->SetCharmTempVal(pTarget->getFactionTemplate());
    pThis->setFarsightGuid(pTarget->getGuid());
    pThis->mControledUnit = pTarget;
    pTarget->SetFaction(getFactionTemplate());
    pTarget->addUnitFlags(UNIT_FLAG_PLAYER_CONTROLLED_CREATURE | UNIT_FLAG_PVP_ATTACKABLE);

    addUnitFlags(UNIT_FLAG_LOCK_PLAYER);

    // send "switch mover" packet
    pThis->sendClientControlPacket(pTarget, 1);

    // update target faction set
    pTarget->updateInRangeOppositeFactionSet();

    if (!(pTarget->isPet() && dynamic_cast< Pet* >(pTarget) == pThis->GetSummon()))
    {
        WorldPacket data(SMSG_PET_SPELLS, 4 * 4 + 20);
        pTarget->BuildPetSpellList(data);
        pThis->GetSession()->SendPacket(&data);
    }
}

void Unit::UnPossess()
{
    Player* pThis = nullptr;
    if (isPlayer())
        pThis = dynamic_cast<Player*>(this);
    else // creatures no support yet
        return;

    if (!pThis)
        return;

    if (!getCharmGuid())
        return;

    Unit* pTarget = GetMapMgr()->GetUnit(getCharmGuid());
    if (!pTarget)
        return;

    pThis->m_CurrentCharm = 0;

    pThis->SpeedCheatReset();

    if (pTarget->isCreature())
    {
        // unit-only stuff.
        pTarget->setAItoUse(true);
        pTarget->m_redirectSpellPackets = nullptr;
        pTarget->mPlayerControler = nullptr;
    }

    m_noInterrupt--;
    pThis->setFarsightGuid(0);
    pThis->mControledUnit = this;
    setCharmGuid(0);
    pTarget->setCharmedByGuid(0);
    setCharmGuid(0);

    removeUnitFlags(UNIT_FLAG_LOCK_PLAYER);
    pTarget->removeUnitFlags(UNIT_FLAG_PLAYER_CONTROLLED_CREATURE | UNIT_FLAG_PVP_ATTACKABLE);
    pTarget->SetFaction(pTarget->GetCharmTempVal());
    pTarget->updateInRangeOppositeFactionSet();

    // send "switch mover" packet
    pThis->sendClientControlPacket(pTarget, 0);

    if (!(pTarget->isPet() && dynamic_cast< Pet* >(pTarget) == pThis->GetSummon()))
        pThis->SendEmptyPetSpellList();

    setMoveRoot(false);

    if (!pTarget->isPet() && (pTarget->getCreatedByGuid() == getGuid()))
    {
        sEventMgr.AddEvent(static_cast< Object* >(pTarget), &Object::Delete, 0, 1, 1, 0);
        return;
    }
}
