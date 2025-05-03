/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Faction_Champion.hpp"
#include "Raid_TrialOfTheCrusader.hpp"
#include "Map/AreaBoundary.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Utilities/Random.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
///  Champion Controller
ChampionControllerAI::ChampionControllerAI(Creature* pCreature) : CreatureAIScript(pCreature) { }
CreatureAIScript* ChampionControllerAI::Create(Creature* pCreature) { return new ChampionControllerAI(pCreature); }

void ChampionControllerAI::InitOrReset()
{
    mChampionsNotStarted = 0;
    mChampionsFailed     = 0;
    mChampionsKilled     = 0;
    mInProgress          = false;
}

void ChampionControllerAI::DoAction(int32_t action)
{
    switch (action)
    {
        case champions::ACTION_SUMMON:
        {
            summonChampions();
        } break;
        case champions::ACTION_START:
        {
            for (SummonList::iterator i = summons.begin(); i != summons.end(); ++i)
            {
                if (Creature* summon = getCreature()->getWorldMapCreature(*i))
                {
                    summon->getAIInterface()->setReactState(REACT_AGGRESSIVE);
                    summon->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
                    summon->getAIInterface()->setImmuneToPC(false);
                }
            }
        } break;
        case champions::ACTION_INPROGRESS:
        {
            if (!mInProgress)
            {
                mChampionsNotStarted = 0;
                mChampionsFailed = 0;
                mChampionsKilled = 0;
                mInProgress = true;

                getInstanceScript()->setBossState(DATA_FACTION_CRUSADERS, EncounterStates::InProgress);
            }
        } break;
        case champions::ACTION_FAILED:
        {
            mChampionsFailed++;
            const auto championsKilled = static_cast<uint32_t>(mChampionsFailed + mChampionsKilled);
            if (championsKilled >= summons.size())
            {
                getInstanceScript()->setBossState(DATA_FACTION_CRUSADERS, EncounterStates::Failed);
                summons.despawnAll();
                despawn();
            }
        } break;
        case champions::ACTION_PERFORMED:
        {
            mChampionsKilled++;
            if (mChampionsKilled == 1)
            {
                getInstanceScript()->setLocalData(DATA_FACTION_CRUSADERS, 0); // Used in Resilience will Fix Achievement
            }
            else if (mChampionsKilled >= summons.size())
            {
                getInstanceScript()->setBossState(DATA_FACTION_CRUSADERS, EncounterStates::Performed);
                summons.despawnAll();
                despawn();
            }
        } break;
    }
}

void ChampionControllerAI::summonChampions()
{
    PlayerTeam teamInInstance = getInstanceScript()->getInstance()->getTeamIdInInstance();

    std::vector<LocationVector> vChampionJumpOrigin;
    if (teamInInstance == TEAM_ALLIANCE)
        for (uint8_t i = 0; i < 5; i++)
            vChampionJumpOrigin.push_back(FactionChampionLoc[i]);
    else
        for (uint8_t i = 5; i < 10; i++)
            vChampionJumpOrigin.push_back(FactionChampionLoc[i]);

    std::vector<LocationVector> vChampionJumpTarget;
    for (uint8_t i = 10; i < 20; i++)
        vChampionJumpTarget.push_back(FactionChampionLoc[i]);

    std::vector<uint32_t> vChampionEntries = selectChampions(teamInInstance);

    for (uint8_t i = 0; i < vChampionEntries.size(); ++i)
    {
        uint8_t pos = Util::getRandomUInt(0, static_cast<uint32_t>(vChampionJumpTarget.size() - 1));
        const auto maxChampionOrigin = static_cast<uint32_t>(vChampionJumpOrigin.size() - 1);
        if (Creature* champion = summonCreature(vChampionEntries[i], vChampionJumpOrigin[Util::getRandomUInt(0, maxChampionOrigin)], CreatureSummonDespawnType::MANUAL_DESPAWN))
        {
            summons.summon(champion);
            champion->getAIInterface()->setReactState(REACT_PASSIVE);
            champion->setUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
            champion->getAIInterface()->setImmuneToPC(false);

            if (teamInInstance == TEAM_ALLIANCE)
            {
                champion->SetSpawnLocation(vChampionJumpTarget[pos].getPositionX(), vChampionJumpTarget[pos].getPositionY(), vChampionJumpTarget[pos].getPositionZ(), 0);
                champion->getMovementManager()->moveJump(vChampionJumpTarget[pos], 20.0f, 20.0f);
                champion->SetOrientation(0);
            }
            else
            {
                champion->SetSpawnLocation((ToCCommonLoc[1].getPositionX() * 2) - vChampionJumpTarget[pos].getPositionX(), vChampionJumpTarget[pos].getPositionY(), vChampionJumpTarget[pos].getPositionZ(), 3);
                champion->getMovementManager()->moveJump((ToCCommonLoc[1].getPositionX() * 2) - vChampionJumpTarget[pos].getPositionX(), vChampionJumpTarget[pos].getPositionY(), vChampionJumpTarget[pos].getPositionZ(), vChampionJumpTarget[pos].getOrientation(), 20.0f, 20.0f);
                champion->SetOrientation(3);
            }
        }
        vChampionJumpTarget.erase(vChampionJumpTarget.begin() + pos);
    }
}

std::vector<uint32_t> ChampionControllerAI::selectChampions(PlayerTeam playerTeam)
{
    std::vector<uint32_t> vHealersEntries;
    vHealersEntries.clear();
    vHealersEntries.push_back(playerTeam == TEAM_ALLIANCE ? NPC_HORDE_DRUID_RESTORATION : NPC_ALLIANCE_DRUID_RESTORATION);
    vHealersEntries.push_back(playerTeam == TEAM_ALLIANCE ? NPC_HORDE_PALADIN_HOLY : NPC_ALLIANCE_PALADIN_HOLY);
    vHealersEntries.push_back(playerTeam == TEAM_ALLIANCE ? NPC_HORDE_PRIEST_DISCIPLINE : NPC_ALLIANCE_PRIEST_DISCIPLINE);
    vHealersEntries.push_back(playerTeam == TEAM_ALLIANCE ? NPC_HORDE_SHAMAN_RESTORATION : NPC_ALLIANCE_SHAMAN_RESTORATION);

    std::vector<uint32_t> vOtherEntries;
    vOtherEntries.clear();
    vOtherEntries.push_back(playerTeam == TEAM_ALLIANCE ? NPC_HORDE_DEATH_KNIGHT : NPC_ALLIANCE_DEATH_KNIGHT);
    vOtherEntries.push_back(playerTeam == TEAM_ALLIANCE ? NPC_HORDE_HUNTER : NPC_ALLIANCE_HUNTER);
    vOtherEntries.push_back(playerTeam == TEAM_ALLIANCE ? NPC_HORDE_MAGE : NPC_ALLIANCE_MAGE);
    vOtherEntries.push_back(playerTeam == TEAM_ALLIANCE ? NPC_HORDE_ROGUE : NPC_ALLIANCE_ROGUE);
    vOtherEntries.push_back(playerTeam == TEAM_ALLIANCE ? NPC_HORDE_WARLOCK : NPC_ALLIANCE_WARLOCK);
    vOtherEntries.push_back(playerTeam == TEAM_ALLIANCE ? NPC_HORDE_WARRIOR : NPC_ALLIANCE_WARRIOR);

    uint8_t healersSubtracted = 2;

    // Max Allowed Healers
    if (getInstanceScript()->getInstance()->getDifficulty() == InstanceDifficulty::RAID_25MAN_NORMAL || getInstanceScript()->getInstance()->getDifficulty() == InstanceDifficulty::RAID_25MAN_HEROIC)
        healersSubtracted = 1;

    // Fill Healers
    for (uint8_t i = 0; i < healersSubtracted; ++i)
    {
        uint8_t pos = Util::getRandomUInt(0, static_cast<uint32_t>(vHealersEntries.size() - 1));
        switch (vHealersEntries[pos])
        {
            case NPC_ALLIANCE_DRUID_RESTORATION:
                vOtherEntries.push_back(NPC_ALLIANCE_DRUID_BALANCE);
                break;
            case NPC_HORDE_DRUID_RESTORATION:
                vOtherEntries.push_back(NPC_HORDE_DRUID_BALANCE);
                break;
            case NPC_ALLIANCE_PALADIN_HOLY:
                vOtherEntries.push_back(NPC_ALLIANCE_PALADIN_RETRIBUTION);
                break;
            case NPC_HORDE_PALADIN_HOLY:
                vOtherEntries.push_back(NPC_HORDE_PALADIN_RETRIBUTION);
                break;
            case NPC_ALLIANCE_PRIEST_DISCIPLINE:
                vOtherEntries.push_back(NPC_ALLIANCE_PRIEST_SHADOW);
                break;
            case NPC_HORDE_PRIEST_DISCIPLINE:
                vOtherEntries.push_back(NPC_HORDE_PRIEST_SHADOW);
                break;
            case NPC_ALLIANCE_SHAMAN_RESTORATION:
                vOtherEntries.push_back(NPC_ALLIANCE_SHAMAN_ENHANCEMENT);
                break;
            case NPC_HORDE_SHAMAN_RESTORATION:
                vOtherEntries.push_back(NPC_HORDE_SHAMAN_ENHANCEMENT);
                break;
            default:
                break;
        }
        vHealersEntries.erase(vHealersEntries.begin() + pos);
    }

    // Damage Dealers
    if (getInstanceScript()->getInstance()->getDifficulty() == InstanceDifficulty::RAID_10MAN_NORMAL || getInstanceScript()->getInstance()->getDifficulty() == InstanceDifficulty::RAID_10MAN_HEROIC)
        for (uint8_t i = 0; i < 4; ++i)
            vOtherEntries.erase(vOtherEntries.begin() + Util::getRandomUInt(0, static_cast<uint32_t>(vOtherEntries.size() - 1)));

    std::vector<uint32_t> vChampionEntries;
    vChampionEntries.clear();

    // Fill Our Entries
    for (uint8_t i = 0; i < vHealersEntries.size(); ++i)
        vChampionEntries.push_back(vHealersEntries[i]);
    for (uint8_t i = 0; i < vOtherEntries.size(); ++i)
        vChampionEntries.push_back(vOtherEntries[i]);

    return vChampionEntries;
}

//////////////////////////////////////////////////////////////////////////////////////////
///  Main Structure for Champions AI
FactionChampionsAI::FactionChampionsAI(Creature* pCreature, uint8_t aitype) : CreatureAIScript(pCreature)
{
    // Add Boundary
    pCreature->getAIInterface()->addBoundary(std::make_unique<CircleBoundary>(LocationVector(563.26f, 139.6f), 75.0));
    mAIType = aitype;
    mTeamInInstance = 0;
}

CreatureAIScript* FactionChampionsAI::Create(Creature* pCreature) { return new FactionChampionsAI(pCreature, champions::AI_MELEE); }

void FactionChampionsAI::InitOrReset()
{
    mTeamInInstance = getInstanceScript()->getInstance()->getTeamIdInInstance();

    addAIFunction(&FactionChampionsAI::update, DoLoopScheduler(4s, false));

    if (isHeroic() && (mAIType != champions::AI_PET))
        addAIFunction(&FactionChampionsAI::removeCC, DoOnceScheduler(5s));
}

bool FactionChampionsAI::onAttackStart(Unit* target)
{
    if (!target)
        return true;

    if (target && getCreature()->getAIInterface()->doInitialAttack(target, true))
    {
        getCreature()->getThreatManager().addThreat(target, 10.0f, nullptr, true, true);

        if (mAIType == champions::AI_MELEE || mAIType == champions::AI_PET)
            moveChase(target);
        else
            moveChase(target, 20.0f);

        // Clear distracted state on attacking
        if (getCreature()->hasUnitStateFlag(UNIT_STATE_DISTRACTED))
        {
            getCreature()->removeUnitStateFlag(UNIT_STATE_DISTRACTED);
            getMovementManager()->clear();
        }
        
    }

    return true;
}

void FactionChampionsAI::OnCombatStart(Unit* /*_target*/)
{
    castSpellOnSelf(champions::SPELL_ANTI_AOE, true);
    setZoneWideCombat();

    if (Creature* pChampionController = getInstanceScript()->getCreatureFromData(DATA_FACTION_CRUSADERS))
        pChampionController->GetScript()->DoAction(champions::ACTION_INPROGRESS);
}

void FactionChampionsAI::OnTargetDied(Unit* _target)
{
    if (_target->isPlayer())
    {
        if (mTeamInInstance == TEAM_ALLIANCE)
        {
            if (Creature* varian = getInstanceScript()->getCreatureFromData(DATA_VARIAN))
                varian->GetScript()->DoAction(ACTION_SAY_KILLED_PLAYER);
        }
        else if (Creature* garrosh = getInstanceScript()->getCreatureFromData(DATA_GARROSH))
        {
            garrosh->GetScript()->DoAction(ACTION_SAY_KILLED_PLAYER);
        }
    }
}

void FactionChampionsAI::OnDied(Unit* /*_killer*/)
{
    if (mAIType != champions::AI_PET)
        if (Creature* pChampionController = getInstanceScript()->getCreatureFromData(DATA_FACTION_CRUSADERS))
            pChampionController->GetScript()->DoAction(champions::ACTION_PERFORMED);
}

void FactionChampionsAI::justReachedSpawn()
{
    if (Creature* pChampionController = getInstanceScript()->getCreatureFromData(DATA_FACTION_CRUSADERS))
        pChampionController->GetScript()->DoAction(champions::ACTION_FAILED);

    despawn();
}

float FactionChampionsAI::calculateThreat(float distance, uint32_t armor, uint32_t health) const
{
    float dist_mod = (mAIType == champions::AI_MELEE || mAIType == champions::AI_PET) ? 15.0f / (15.0f + distance) : 1.0f;
    float armor_mod = (mAIType == champions::AI_MELEE || mAIType == champions::AI_PET) ? armor / 16635.0f : 0.0f;
    float eh = (health + 1) * (1.0f + armor_mod);
    return dist_mod * 30000.0f / eh;
}

uint32_t FactionChampionsAI::enemiesInRange(float range)
{
    uint32_t count = 0;
    for (ThreatReference const* ref : getCreature()->getThreatManager().getModifiableThreatList())
        if (getCreature()->getDistance2d(ref->getVictim()) < range)
            ++count;
    return count;
}

void FactionChampionsAI::update(CreatureAIFunc pThis)
{
    // Power Management
    if (getCreature()->getPowerType() == POWER_TYPE_MANA)
        getCreature()->modPower(POWER_TYPE_MANA, getCreature()->getMaxPower(POWER_TYPE_MANA) / 3);

    // Threath Modifiers
    for (ThreatReference* ref : getCreature()->getThreatManager().getModifiableThreatList())
        if (Player* victim = ref->getVictim()->ToPlayer())
        {
            ref->scaleThreat(0.0f);
            ref->addThreat(1000000.0f * calculateThreat(getCreature()->getDistance2d(victim), victim->getResistance(0), victim->getHealth()));
        }
}

void FactionChampionsAI::removeCC(CreatureAIFunc pThis)
{
    if (hasBreakableByDamageCrowdControlAura())
    {
        getCreature()->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STUN);
        getCreature()->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STUN);
        getCreature()->removeAllAurasByAuraEffect(SPELL_AURA_MOD_FEAR);
        getCreature()->removeAllAurasByAuraEffect(SPELL_AURA_MOD_ROOT);
        getCreature()->removeAllAurasByAuraEffect(SPELL_AURA_MOD_PACIFY);
        getCreature()->removeAllAurasByAuraEffect(SPELL_AURA_MOD_CONFUSE);

        addAIFunction(&FactionChampionsAI::removeCC, DoOnceScheduler(2min));
    }
    else
    {
        addAIFunction(&FactionChampionsAI::removeCC, DoOnceScheduler(3s));
    }
}

/********************************************************************
                            HEALERS
********************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////
/// Druid AI
DruidAI::DruidAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* DruidAI::Create(Creature* pCreature) { return new DruidAI(pCreature, champions::AI_HEALER); }

void DruidAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(51799, 0);

    SpellDesc mLifeBloomInfo(champions::SPELL_LIFEBLOOM, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    mLifeBloomInfo.setUseSpellCD(true);
    addAISpell(mLifeBloomInfo, DoLoopScheduler(5s, 25s, 15.0f));

    SpellDesc mNourishInfo(champions::SPELL_NOURISH, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    mNourishInfo.setUseSpellCD(true);
    addAISpell(mNourishInfo, DoLoopScheduler(5s, 25s, 15.0f));

    SpellDesc mRegrowthInfo(champions::SPELL_REGROWTH, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    mRegrowthInfo.setUseSpellCD(true);
    addAISpell(mRegrowthInfo, DoLoopScheduler(5s, 25s, 15.0f));
    
    SpellDesc mRejuvenationInfo(champions::SPELL_REJUVENATION, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    mRejuvenationInfo.setUseSpellCD(true);
    addAISpell(mRejuvenationInfo, DoLoopScheduler(5s, 25s, 15.0f));

    SpellDesc mTranquilityInfo(champions::SPELL_TRANQUILITY, FilterArgs(TargetFilter_AOE), false);
    addAISpell(mTranquilityInfo, DoLoopScheduler(5s, 40s, 15.0f));

    DoLoopScheduler barkskinArgs(5s, 15s, 15.0f);
    barkskinArgs.setMinMaxPercentHp(0.0f, 30.0f);
    SpellDesc mBarkSkinInfo(champions::SPELL_BARKSKIN, FilterArgs(TargetFilter_Self), false);
    mBarkSkinInfo.setUseSpellCD(true);
    addAISpell(mBarkSkinInfo, barkskinArgs);

    SpellDesc mThornsInfo(champions::SPELL_THORNS, FilterArgs(TargetFilter_Friendly, 0.0f, 0.0f, -champions::SPELL_THORNS), false);
    mThornsInfo.setUseSpellCD(true);
    addAISpell(mThornsInfo, DoLoopScheduler(2s, 15s, 15.0f));
    
    SpellDesc mNatureGaspInfo(champions::SPELL_NATURE_GRASP, FilterArgs(TargetFilter_Self), false);
    mNatureGaspInfo.setUseSpellCD(true);
    addAISpell(mNatureGaspInfo, DoLoopScheduler(5s, 15.0f));
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Shaman AI
ShamanAI::ShamanAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* ShamanAI::Create(Creature* pCreature) { return new ShamanAI(pCreature, champions::AI_HEALER); }

void ShamanAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(49992, 0);

    SpellDesc mHealingWaveInfo(champions::SPELL_HEALING_WAVE, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    addAISpell(mHealingWaveInfo, DoLoopScheduler(5s, 15s, 33.0f));

    SpellDesc mRiptideInfo(champions::SPELL_RIPTIDE, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    mRiptideInfo.setUseSpellCD(true);
    addAISpell(mRiptideInfo, DoLoopScheduler(5s, 15s, 33.0f));

    SpellDesc mSpiritCleanseInfo(champions::SPELL_SPIRIT_CLEANSE, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    addAISpell(mSpiritCleanseInfo, DoLoopScheduler(5s, 15s, 33.0f));

    SpellDesc mHexInfo(champions::SPELL_HEX, FilterArgs(TargetFilter_None), false);
    mHexInfo.setUseSpellCD(true);
    addAISpell(mHexInfo, DoLoopScheduler(5s, 15s, 33.0f));

    SpellDesc mEarthShieldInfo(champions::SPELL_EARTH_SHIELD, FilterArgs(TargetFilter_Friendly, 0.0f, 0.0f, -champions::SPELL_EARTH_SHIELD), false);
    mEarthShieldInfo.setUseSpellCD(true);
    addAISpell(mEarthShieldInfo, DoLoopScheduler(3s, 8s, 33.0f));

    SpellDesc mEarthShockInfo(champions::SPELL_EARTH_SHOCK, FilterArgs(TargetFilter_Caster), false);
    mEarthShockInfo.setUseSpellCD(true);
    addAISpell(mEarthShockInfo, DoLoopScheduler(5s, 15s, 33.0f));

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (getCreature()->getFactionTemplate()) //Am i alliance?
            {
                if (!getCreature()->hasAurasWithId(champions::AURA_EXHAUSTION))
                    castSpellAOE(champions::SPELL_HEROISM);
            }
            else
            {
                if (!getCreature()->hasAurasWithId(champions::AURA_SATED))
                    castSpellAOE(champions::SPELL_BLOODLUST);
            }
            repeatFunctionFromScheduler(pThis, 5min);
        }, DoOnceScheduler(10s, 20s, 33.0f));
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Paladin AI
PaladinAI::PaladinAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* PaladinAI::Create(Creature* pCreature) { return new PaladinAI(pCreature, champions::AI_HEALER); }

void PaladinAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(50771, 47079);

    SpellDesc mHandOfFreeInfo(champions::SPELL_HAND_OF_FREEDOM, FilterArgs(TargetFilter_Friendly, 0.0f, 0.0f, -champions::SPELL_HAND_OF_FREEDOM), false);
    mHandOfFreeInfo.setUseSpellCD(true);
    addAISpell(mHandOfFreeInfo, DoLoopScheduler(5s, 15s, 15.0f));

    DoLoopScheduler divineArgs(5s, 15s, 15.0f);
    divineArgs.setMinMaxPercentHp(0.0f, 30.0f);
    SpellDesc mDivineShieldInfo(champions::SPELL_DIVINE_SHIELD, FilterArgs(TargetFilter_Self, 0.0f, 0.0f, -champions::SPELL_FORBEARANCE), false);
    mDivineShieldInfo.setUseSpellCD(true);
    addAISpell(mDivineShieldInfo, divineArgs);

    SpellDesc mCleanseInfo(champions::SPELL_CLEANSE, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    addAISpell(mCleanseInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mFlashOfLightInfo(champions::SPELL_FLASH_OF_LIGHT, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    addAISpell(mFlashOfLightInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mHolyLightInfo(champions::SPELL_HOLY_LIGHT, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    addAISpell(mHolyLightInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mHolyShockInfo(champions::SPELL_HOLY_SHOCK, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    mHolyShockInfo.setUseSpellCD(true);
    addAISpell(mHolyShockInfo, DoLoopScheduler(5s, 15s, 15.0f));

    DoLoopScheduler handOfProtArgs(15s, 30s, 15.0f);
    handOfProtArgs.setMinMaxPercentHp(0.0f, 30.0f);
    SpellDesc mHandOfProtInfo(champions::SPELL_HAND_OF_PROTECTION, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 30.0f, -champions::SPELL_FORBEARANCE), false);
    mHandOfProtInfo.setUseSpellCD(true);
    addAISpell(mHandOfProtInfo, handOfProtArgs);

    SpellDesc mHammerOfJusticeInfo(champions::SPELL_HAMMER_OF_JUSTICE, FilterArgs(TargetFilter(TargetFilter_Player | TargetFilter_InRangeOnly), 0.0f, 15.0f, 0), false);
    mHammerOfJusticeInfo.setUseSpellCD(true);
    addAISpell(mHammerOfJusticeInfo, DoLoopScheduler(5s, 15s, 33.0f));
}

PriestAI::PriestAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* PriestAI::Create(Creature* pCreature) { return new PriestAI(pCreature, champions::AI_HEALER); }

//////////////////////////////////////////////////////////////////////////////////////////
/// Priest AI
void PriestAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(49992, 0);

    SpellDesc mRenewInfo(champions::SPELL_RENEW, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    addAISpell(mRenewInfo, DoLoopScheduler(5s, 15s, 20.0f));

    SpellDesc mShieldInfo(champions::SPELL_SHIELD, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    mShieldInfo.setUseSpellCD(true);
    addAISpell(mShieldInfo, DoLoopScheduler(5s, 15s, 20.0f));

    SpellDesc mFlashHealInfo(champions::SPELL_FLASH_HEAL, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    addAISpell(mFlashHealInfo, DoLoopScheduler(5s, 15s, 20.0f));

    SpellDesc mDispelInfo(champions::SPELL_DISPEL, (Util::getRandomUInt(1) ? FilterArgs(TargetFilter_InRangeOnly, 0.0f, 30.0f) : FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f)), false);
    addAISpell(mDispelInfo, DoLoopScheduler(5s, 15s, 20.0f));

    SpellDesc mManaBurnInfo(champions::SPELL_MANA_BURN, FilterArgs(TargetFilter_Caster), false);
    mManaBurnInfo.setUseSpellCD(true);
    addAISpell(mManaBurnInfo, DoLoopScheduler(5s, 15s, 20.0f));

    SpellDesc mPenanceInfo(champions::SPELL_PENANCE, FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f), false);
    mPenanceInfo.setUseSpellCD(true);
    addAISpell(mPenanceInfo, DoLoopScheduler(5s, 15s, 20.0f));

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (enemiesInRange(10.0f) >= 2)
                castSpellAOE(champions::SPELL_PSYCHIC_SCREAM);

            repeatFunctionFromScheduler(pThis);
        }, DoOnceScheduler(10s, 30s, 20.0f));
}

/********************************************************************
                            RANGED
********************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////
/// Shadow Priest AI
ShadowPriestAI::ShadowPriestAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* ShadowPriestAI::Create(Creature* pCreature) { return new ShadowPriestAI(pCreature, champions::AI_RANGED); }

void ShadowPriestAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(50040, 0);

    // Form
    castSpellOnSelf(champions::SPELL_SHADOWFORM);

    SpellDesc mSilenceInfo(champions::SPELL_SILENCE, FilterArgs(TargetFilter_CasterWhileCasting), false);
    mSilenceInfo.setUseSpellCD(true);
    addAISpell(mSilenceInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mVampireTouchInfo(champions::SPELL_VAMPIRIC_TOUCH, FilterArgs(TargetFilter_InRangeOnly, 0.0f, 30.0f, 0), false);
    addAISpell(mVampireTouchInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mSwPainInfo(champions::SPELL_SW_PAIN, FilterArgs(TargetFilter_InRangeOnly, 0.0f, 40.0f, 0), false);
    mSwPainInfo.setUseSpellCD(true);
    addAISpell(mSwPainInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mMindBlastInfo(champions::SPELL_MIND_BLAST, FilterArgs(TargetFilter_Current), false);
    mMindBlastInfo.setUseSpellCD(true);
    addAISpell(mMindBlastInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mHorrorInfo(champions::SPELL_HORROR, FilterArgs(TargetFilter_Current), false);
    mHorrorInfo.setUseSpellCD(true);
    addAISpell(mHorrorInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mDispelInfo(champions::SPELL_DISPEL, (Util::getRandomUInt(1) ? FilterArgs(TargetFilter_InRangeOnly, 0.0f, 30.0f) : FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 40.0f)), false);
    addAISpell(mDispelInfo, DoLoopScheduler(5s, 15s, 20.0f));

    DoLoopScheduler dispersionArgs(10s, 30s, 15.0f);
    dispersionArgs.setMinMaxPercentHp(0.0f, 40.0f);
    SpellDesc mDispersionInfo(champions::SPELL_DISPERSION, FilterArgs(TargetFilter_Self), false);
    mDispersionInfo.setUseSpellCD(true);
    addAISpell(mDispersionInfo, dispersionArgs);
    
    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (enemiesInRange(10.0f) >= 2)
                castSpellAOE(champions::SPELL_PSYCHIC_SCREAM);

            repeatFunctionFromScheduler(pThis);
        }, DoOnceScheduler(10s, 30s, 33.0f));

    getCreature()->setOnMeleeSpell(champions::SPELL_MIND_FLAY);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Warlock AI
WarlockAI::WarlockAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* WarlockAI::Create(Creature* pCreature) { return new WarlockAI(pCreature, champions::AI_RANGED); }

void WarlockAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(49992, 0);

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (enemiesInRange(10.0f) >= 2)
                castSpellAOE(champions::SPELL_HELLFIRE);

            repeatFunctionFromScheduler(pThis);
        }, DoOnceScheduler(10s, 30s, 33.0f));

    SpellDesc mCorruptionInfo(champions::SPELL_CORRUPTION, FilterArgs(TargetFilter_InRangeOnly, 0.0f, 30.0f, 0), false);
    addAISpell(mCorruptionInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mCurseOfAgonyInfo(champions::SPELL_CURSE_OF_AGONY, FilterArgs(TargetFilter_InRangeOnly, 0.0f, 30.0f, 0), false);
    addAISpell(mCurseOfAgonyInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mExhaustionInfo(champions::SPELL_CURSE_OF_EXHAUSTION, FilterArgs(TargetFilter_InRangeOnly, 0.0f, 30.0f, 0), false);
    addAISpell(mExhaustionInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mFearInfo(champions::SPELL_FEAR, FilterArgs(TargetFilter(TargetFilter_Player | TargetFilter_InRangeOnly), 0.0f, 20.0f, 0), false);
    mFearInfo.setUseSpellCD(true);
    addAISpell(mFearInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mSearingPainInfo(champions::SPELL_SEARING_PAIN, FilterArgs(TargetFilter_Current), false);
    addAISpell(mSearingPainInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mAfflictionInfo(champions::SPELL_UNSTABLE_AFFLICTION, FilterArgs(TargetFilter_InRangeOnly, 0.0f, 30.0f, 0), false);
    mAfflictionInfo.setUseSpellCD(true);
    addAISpell(mAfflictionInfo, DoLoopScheduler(5s, 15s, 15.0f));

    getCreature()->setOnMeleeSpell(champions::SPELL_SHADOW_BOLT);
}

void WarlockAI::OnCombatStart(Unit* _target)
{
    FactionChampionsAI::OnCombatStart(_target);

    castSpellOnSelf(champions::SPELL_SUMMON_FELHUNTER);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Mage AI
MageAI::MageAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* MageAI::Create(Creature* pCreature) { return new MageAI(pCreature, champions::AI_RANGED); }

void MageAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(47524, 0);

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (enemiesInRange(10.0f) >= 2)
                castSpellAOE(champions::SPELL_ARCANE_EXPLOSION);

            repeatFunctionFromScheduler(pThis);
        }, DoOnceScheduler(10s, 30s, 33.0f));

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (enemiesInRange(10.0f) >= 2)
                castSpellOnSelf(champions::SPELL_BLINK);

            repeatFunctionFromScheduler(pThis);
        }, DoOnceScheduler(10s, 30s, 33.0f));

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (enemiesInRange(10.0f) >= 2)
                castSpellAOE(champions::SPELL_FROST_NOVA);

            repeatFunctionFromScheduler(pThis);
        }, DoOnceScheduler(10s, 30s, 33.0f));

    DoOnceScheduler mIceBlockScheduler(10s, 20s, 33.0f);
    mIceBlockScheduler.setMinMaxPercentHp(0.0f, 30.0f);
    SpellDesc mIceBlockInfo(champions::SPELL_ICE_BLOCK, FilterArgs(TargetFilter_Self), false);
    mIceBlockInfo.setUseSpellCD(true);
    addAISpell(mIceBlockInfo, mIceBlockScheduler);

    SpellDesc mArcaneBarrageInfo(champions::SPELL_ARCANE_BARRAGE, FilterArgs(TargetFilter_Current), false);
    mArcaneBarrageInfo.setUseSpellCD(true);
    addAISpell(mArcaneBarrageInfo, DoLoopScheduler(5s, 15s, 25.0f));

    SpellDesc mArcaneBlastInfo(champions::SPELL_ARCANE_BLAST, FilterArgs(TargetFilter_Current), false);
    addAISpell(mArcaneBlastInfo, DoLoopScheduler(5s, 15s, 25.0f));

    SpellDesc mCounterSpellInfo(champions::SPELL_COUNTERSPELL, FilterArgs(TargetFilter_CasterWhileCasting), false);
    mCounterSpellInfo.setUseSpellCD(true);
    addAISpell(mCounterSpellInfo, DoLoopScheduler(5s, 15s, 25.0f));

    SpellDesc mPolymorphInfo(champions::SPELL_POLYMORPH, FilterArgs(TargetFilter_None), false);
    mPolymorphInfo.setUseSpellCD(true);
    addAISpell(mPolymorphInfo, DoLoopScheduler(5s, 15s, 25.0f));

    getCreature()->setOnMeleeSpell(champions::SPELL_FROSTBOLT);
}


//////////////////////////////////////////////////////////////////////////////////////////
/// Hunter AI
HunterAI::HunterAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* HunterAI::Create(Creature* pCreature) { return new HunterAI(pCreature, champions::AI_RANGED); }

void HunterAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(47156, 0, 48711);

    DoLoopScheduler mDeterrenceScheduler(10s, 20s, 15.0f);
    mDeterrenceScheduler.setMinMaxPercentHp(0.0f, 30.0f);
    SpellDesc mDeterrenceInfo(champions::SPELL_DETERRENCE, FilterArgs(TargetFilter_Self), false);
    mDeterrenceInfo.setUseSpellCD(true);
    addAISpell(mDeterrenceInfo, mDeterrenceScheduler);

    SpellDesc mAimedShotInfo(champions::SPELL_AIMED_SHOT, FilterArgs(TargetFilter_Current), false);
    mAimedShotInfo.setUseSpellCD(true);
    addAISpell(mAimedShotInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mSteadyShotInfo(champions::SPELL_STEADY_SHOT, FilterArgs(TargetFilter_Current), false);
    mSteadyShotInfo.setUseSpellCD(true);
    addAISpell(mSteadyShotInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mWingClipInfo(champions::SPELL_WING_CLIP, FilterArgs(TargetFilter_CurrentInRangeOnly, 0.0f, 6.0f, 0), false);
    mWingClipInfo.setUseSpellCD(true);
    addAISpell(mWingClipInfo, DoLoopScheduler(5s, 15s, 15.0f));

    SpellDesc mWyvernStingInfo(champions::SPELL_WYVERN_STING, FilterArgs(TargetFilter_None), false);
    mWyvernStingInfo.setUseSpellCD(true);
    addAISpell(mWyvernStingInfo, DoLoopScheduler(10s, 30s, 15.0f));

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (enemiesInRange(10.0f) >= 2)
                castSpellAOE(champions::SPELL_FROST_TRAP);

            repeatFunctionFromScheduler(pThis);
        }, DoOnceScheduler(10s, 30s, 33.0f));

    getCreature()->setOnMeleeSpell(champions::SPELL_SHOOT);
}

void HunterAI::OnCombatStart(Unit* _target)
{
    FactionChampionsAI::OnCombatStart(_target);
    castSpellOnSelf(champions::SPELL_CALL_PET);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Boomkin AI
BoomkinAI::BoomkinAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* BoomkinAI::Create(Creature* pCreature) { return new BoomkinAI(pCreature, champions::AI_RANGED); }

void BoomkinAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(50966);

    DoLoopScheduler barkskinArgs(5s, 15s, 15.0f);
    barkskinArgs.setMinMaxPercentHp(0.0f, 30.0f);
    SpellDesc mBarkSkinInfo(champions::SPELL_BARKSKIN, FilterArgs(TargetFilter_Self), false);
    mBarkSkinInfo.setUseSpellCD(true);
    addAISpell(mBarkSkinInfo, barkskinArgs);

    SpellDesc mCycloneInfo(champions::SPELL_CYCLONE, FilterArgs(TargetFilter_None), false);
    mCycloneInfo.setUseSpellCD(true);
    addAISpell(mCycloneInfo, DoLoopScheduler(5s, 25s, 15.0f));

    SpellDesc mEntanglingRootsInfo(champions::SPELL_ENTANGLING_ROOTS, FilterArgs(TargetFilter(TargetFilter_InRangeOnly | TargetFilter_Player), 0.0f, 30.0f, 0), false);
    mEntanglingRootsInfo.setUseSpellCD(true);
    addAISpell(mEntanglingRootsInfo, DoLoopScheduler(5s, 25s, 15.0f));

    SpellDesc mFearieFireInfo(champions::SPELL_FAERIE_FIRE, FilterArgs(TargetFilter_Current), false);
    addAISpell(mFearieFireInfo, DoLoopScheduler(5s, 25s, 15.0f));

    SpellDesc mForceOfNatureInfo(champions::SPELL_FORCE_OF_NATURE, FilterArgs(TargetFilter_Current), false);
    mForceOfNatureInfo.setUseSpellCD(true);
    addAISpell(mForceOfNatureInfo, DoLoopScheduler(5s, 25s, 15.0f));

    SpellDesc mInsectSwarmInfo(champions::SPELL_INSECT_SWARM, FilterArgs(TargetFilter_Current), false);
    addAISpell(mInsectSwarmInfo, DoLoopScheduler(5s, 25s, 15.0f));

    SpellDesc mMoonfireInfo(champions::SPELL_MOONFIRE, FilterArgs(TargetFilter_Current), false);
    mMoonfireInfo.setUseSpellCD(true);
    addAISpell(mMoonfireInfo, DoLoopScheduler(5s, 25s, 15.0f));

    SpellDesc mStarfireInfo(champions::SPELL_STARFIRE, FilterArgs(TargetFilter_Current), false);
    addAISpell(mStarfireInfo, DoLoopScheduler(5s, 25s, 15.0f));

    getCreature()->setOnMeleeSpell(champions::SPELL_WRATH);
}

/********************************************************************
                            MELEE
********************************************************************/

//////////////////////////////////////////////////////////////////////////////////////////
/// Warrior AI
WarriorAI::WarriorAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* WarriorAI::Create(Creature* pCreature) { return new WarriorAI(pCreature, champions::AI_MELEE); }

void WarriorAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(47427, 46964);

    SpellDesc mBladeStormInfo(champions::SPELL_BLADESTORM, FilterArgs(TargetFilter_Current), false);
    mBladeStormInfo.setUseSpellCD(true);
    addAISpell(mBladeStormInfo, DoLoopScheduler(10s, 20s, 33.0f));

    SpellDesc mShoutInfo(champions::SPELL_INTIMIDATING_SHOUT, FilterArgs(TargetFilter_AOE), false);
    mShoutInfo.setUseSpellCD(true);
    addAISpell(mShoutInfo, DoLoopScheduler(20s, 25s, 33.0f));
    
    SpellDesc mMortalStrikeInfo(champions::SPELL_MORTAL_STRIKE, FilterArgs(TargetFilter_Current), false);
    mMortalStrikeInfo.setUseSpellCD(true);
    addAISpell(mMortalStrikeInfo, DoLoopScheduler(5s, 20s, 33.0f));

    SpellDesc mChargeInfo(champions::SPELL_CHARGE, FilterArgs(TargetFilter_Current), false);
    mChargeInfo.setUseSpellCD(true);
    addAISpell(mChargeInfo, DoLoopScheduler(1s, 75.0f));
    
    SpellDesc mDisarmInfo(champions::SPELL_DISARM, FilterArgs(TargetFilter_Current), false);
    mDisarmInfo.setUseSpellCD(true);
    addAISpell(mDisarmInfo, DoLoopScheduler(5s, 20s, 33.0f));
    
    SpellDesc mOverpowerInfo(champions::SPELL_OVERPOWER, FilterArgs(TargetFilter_Current), false);
    mOverpowerInfo.setUseSpellCD(true);
    addAISpell(mOverpowerInfo, DoLoopScheduler(10s, 20s, 33.0f));
    
    SpellDesc mSunderArmorInfo(champions::SPELL_SUNDER_ARMOR, FilterArgs(TargetFilter_Current), false);
    mSunderArmorInfo.setUseSpellCD(true);
    addAISpell(mSunderArmorInfo, DoLoopScheduler(2s, 10s, 33.0f));

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (Unit* target = getCreature()->getAIInterface()->getCurrentTarget())
            {
                if (target->hasAuraWithMechanic(MECHANIC_INVULNERABLE))
                {
                    castSpell(target, champions::SPELL_SHATTERING_THROW);
                    repeatFunctionFromScheduler(pThis, 5min);
                }
            }
            repeatFunctionFromScheduler(pThis, 3s);
        }, DoOnceScheduler(20s, 40s, 33.0f));

    DoOnceScheduler mRetaliationScheduler(5s, 10s, 33.0f);
    mRetaliationScheduler.setMinMaxPercentHp(0.0f, 50.0f);
    SpellDesc mRetaliationInfo(champions::SPELL_RETALIATION, FilterArgs(TargetFilter_Self), false);
    mRetaliationInfo.setUseSpellCD(true);
    addAISpell(mRetaliationInfo, mRetaliationScheduler);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// DK AI
DKAI::DKAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* DKAI::Create(Creature* pCreature) { return new DKAI(pCreature, champions::AI_MELEE); }

void DKAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(47518, 51021);
    getCreature()->setDualWield(true);

    SpellDesc mChainsOfIceInfo(champions::SPELL_CHAINS_OF_ICE, FilterArgs(TargetFilter_Current), false);
    mChainsOfIceInfo.setUseSpellCD(true);
    addAISpell(mChainsOfIceInfo, DoLoopScheduler(5s, 15s, 33.0f));
    
    SpellDesc mDeathCoilInfo(champions::SPELL_DEATH_COIL, FilterArgs(TargetFilter_Current), false);
    mDeathCoilInfo.setUseSpellCD(true);
    addAISpell(mDeathCoilInfo, DoLoopScheduler(5s, 15s, 33.0f));
    
    SpellDesc mDeathGripInfo(champions::SPELL_DEATH_GRIP, FilterArgs(TargetFilter(TargetFilter_CurrentInRangeOnly | TargetFilter_Player), 5.0f, 30.0f, 0), false);
    mDeathGripInfo.setUseSpellCD(true);
    addAISpell(mDeathGripInfo, DoLoopScheduler(5s, 20s, 33.0f));
    
    SpellDesc mFrostStrikeInfo(champions::SPELL_FROST_STRIKE, FilterArgs(TargetFilter_Current), false);
    mFrostStrikeInfo.setUseSpellCD(true);
    addAISpell(mFrostStrikeInfo, DoLoopScheduler(5s, 15s, 33.0f));
    
    SpellDesc mIceBoundInfo(champions::SPELL_ICEBOUND_FORTITUDE, FilterArgs(TargetFilter_SelfBelowHealth, 0.0f, 0.0f, 0.0f, 50.0f, 0), false);
    mIceBoundInfo.setUseSpellCD(true);
    addAISpell(mIceBoundInfo, DoLoopScheduler(5s, 20s, 33.0f));
    
    SpellDesc mIcyTouchInfo(champions::SPELL_ICY_TOUCH, FilterArgs(TargetFilter_Current), false);
    mIcyTouchInfo.setUseSpellCD(true);
    addAISpell(mIcyTouchInfo, DoLoopScheduler(5s, 20s, 33.0f));
    
    SpellDesc mStrangulateInfo(champions::SPELL_STRANGULATE, FilterArgs(TargetFilter_Caster), false);
    mStrangulateInfo.setUseSpellCD(true);
    addAISpell(mStrangulateInfo, DoLoopScheduler(5s, 20s, 33.0f));
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Rogue AI
RogueAI::RogueAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* RogueAI::Create(Creature* pCreature) { return new RogueAI(pCreature, champions::AI_MELEE); }

void RogueAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(47422, 49982);
    getCreature()->setDualWield(true);
    getCreature()->setPowerType(POWER_TYPE_ENERGY);
    getCreature()->setPower(POWER_TYPE_ENERGY, getCreature()->getMaxPower(POWER_TYPE_ENERGY));

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (enemiesInRange(10.0f) >= 2)
                castSpellAOE(champions::SPELL_FAN_OF_KNIVES);

            repeatFunctionFromScheduler(pThis);
        }, DoOnceScheduler(10s, 20s, 33.0f));

    SpellDesc mBlindInfo(champions::SPELL_BLIND, FilterArgs(TargetFilter_None), false);
    mBlindInfo.setUseSpellCD(true);
    addAISpell(mBlindInfo, DoLoopScheduler(5s, 20s, 33.0f));

    SpellDesc mCloackInfo(champions::SPELL_CLOAK, FilterArgs(TargetFilter_SelfBelowHealth, 0.0f, 0.0f, 0.0f, 50.0f), false);
    mCloackInfo.setUseSpellCD(true);
    addAISpell(mCloackInfo, DoLoopScheduler(5s, 20s, 33.0f));

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (enemiesInRange(10.0f) >= 2)
                castSpellAOE(champions::SPELL_BLADE_FLURRY);

            repeatFunctionFromScheduler(pThis, 120s);
        }, DoOnceScheduler(10s, 20s, 33.0f));

    SpellDesc mShadowStepInfo(champions::SPELL_SHADOWSTEP, FilterArgs(TargetFilter_InRangeOnly, 10.0f, 40.0f), false);
    mShadowStepInfo.setUseSpellCD(true);
    addAISpell(mShadowStepInfo, DoLoopScheduler(5s, 20s, 33.0f));

    SpellDesc mHemorrInfo(champions::SPELL_HEMORRHAGE, FilterArgs(TargetFilter_Current), false);
    mHemorrInfo.setUseSpellCD(true);
    addAISpell(mHemorrInfo, DoLoopScheduler(5s, 20s, 33.0f));

    SpellDesc mEviscerateInfo(champions::SPELL_EVISCERATE, FilterArgs(TargetFilter_Current), false);
    mEviscerateInfo.setUseSpellCD(true);
    addAISpell(mEviscerateInfo, DoLoopScheduler(5s, 20s, 33.0f));

    SpellDesc mWoundPoisonInfo(champions::SPELL_WOUND_POISON, FilterArgs(TargetFilter_Current), false);
    mWoundPoisonInfo.setUseSpellCD(true);
    addAISpell(mWoundPoisonInfo, DoLoopScheduler(5s, 20s, 33.0f));
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Enhancer AI
EnhancerAI::EnhancerAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype)
{
    initialize();
}

CreatureAIScript* EnhancerAI::Create(Creature* pCreature) { return new EnhancerAI(pCreature, champions::AI_MELEE); }

void EnhancerAI::initialize()
{
    mTotemCount = 0;
    mTotemOldCenterX = getCreature()->GetPositionX();
    mTotemOldCenterY = getCreature()->GetPositionY();
}

void EnhancerAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(51803, 48013);
    getCreature()->setDualWield(true);

    // Spells
    SpellDesc mEarthShockInfo(champions::SPELL_EARTH_SHOCK, FilterArgs(TargetFilter_CasterWhileCasting), false);
    mEarthShockInfo.setUseSpellCD(true);
    addAISpell(mEarthShockInfo, DoLoopScheduler(5s, 10s, 33.0f));

    SpellDesc mLavaLashInfo(champions::SPELL_LAVA_LASH, FilterArgs(TargetFilter_Current), false);
    mLavaLashInfo.setUseSpellCD(true);
    addAISpell(mLavaLashInfo, DoLoopScheduler(3s, 5s, 33.0f));

    SpellDesc mStormstrikeInfo(champions::SPELL_STORMSTRIKE, FilterArgs(TargetFilter_Current), false);
    mStormstrikeInfo.setUseSpellCD(true);
    addAISpell(mStormstrikeInfo, DoLoopScheduler(3s, 5s, 33.0f));

    SpellDesc mWindfuryInfo(champions::SPELL_WINDFURY, FilterArgs(TargetFilter_Current), false);
    addAISpell(mWindfuryInfo, DoLoopScheduler(20s, 50s, 33.0f));

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (getCreature()->getFactionTemplate()) //Am i alliance?
            {
                if (!getCreature()->hasAurasWithId(champions::AURA_EXHAUSTION))
                    castSpellAOE(champions::SPELL_HEROISM);
            }
            else
            {
                if (!getCreature()->hasAurasWithId(champions::AURA_SATED))
                    castSpellAOE(champions::SPELL_BLOODLUST);
            }
            repeatFunctionFromScheduler(pThis, 5min);
        }, DoOnceScheduler(10s, 20s, 33.0f));

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (mTotemCount < 4 || getCreature()->getDistance2d(mTotemOldCenterX, mTotemOldCenterY) > 20.0f)
                deployTotems();

            repeatFunctionFromScheduler(pThis, 1s);
        }, DoOnceScheduler(1s, 75.0f));

    initialize();
    summons.despawnAll();
}

void EnhancerAI::OnDied(Unit* killer)
{
    FactionChampionsAI::OnDied(killer);
    summons.despawnAll();
}

void EnhancerAI::onSummonedCreature(Creature* summon)
{
    summons.summon(summon);
}

void EnhancerAI::OnSummonDespawn(Creature* summon)
{
    --mTotemCount;
}

void EnhancerAI::deployTotems()
{
    mTotemCount = 4;
    mTotemOldCenterX = getCreature()->GetPositionX();
    mTotemOldCenterY = getCreature()->GetPositionY();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Retri AI
RetriAI::RetriAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* RetriAI::Create(Creature* pCreature) { return new RetriAI(pCreature, champions::AI_MELEE); }

void RetriAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    // Equipment
    _setDisplayWeaponIds(47519);
    
    SpellDesc mAvengingWrathInfo(champions::SPELL_AVENGING_WRATH, FilterArgs(TargetFilter_Self), false);
    mAvengingWrathInfo.setUseSpellCD(true);
    addAISpell(mAvengingWrathInfo, DoLoopScheduler(25s, 35s, 33.0f));

    SpellDesc mCrusaderStrikeInfo(champions::SPELL_CRUSADER_STRIKE, FilterArgs(TargetFilter_Self), false);
    mCrusaderStrikeInfo.setUseSpellCD(true);
    addAISpell(mCrusaderStrikeInfo, DoLoopScheduler(5s, 10s, 33.0f));

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (enemiesInRange(10.0f) >= 2)
                castSpellAOE(champions::SPELL_DIVINE_STORM);

            repeatFunctionFromScheduler(pThis);
        }, DoOnceScheduler(10s, 20s, 33.0f));

    SpellDesc mHammerStrikeInfo(champions::SPELL_HAMMER_OF_JUSTICE_RET, FilterArgs(TargetFilter_Current), false);
    mHammerStrikeInfo.setUseSpellCD(true);
    addAISpell(mHammerStrikeInfo, DoLoopScheduler(10s, 30s, 33.0f));

    SpellDesc mJudgementOfCommandInfo(champions::SPELL_JUDGEMENT_OF_COMMAND, FilterArgs(TargetFilter_Current), false);
    addAISpell(mJudgementOfCommandInfo, DoLoopScheduler(8s, 15s, 33.0f));

    SpellDesc mReptanceInfo(champions::SPELL_REPENTANCE, FilterArgs(TargetFilter_None), false);
    mReptanceInfo.setUseSpellCD(true);
    addAISpell(mReptanceInfo, DoLoopScheduler(15s, 30s, 33.0f));

    addAIFunction([this](CreatureAIFunc pThis)
        {
            if (Unit* target = selectUnitTarget(FilterArgs(TargetFilter_WoundedFriendlyLowestHealthInRange, 0.0f, 30.0f)))
            {
                if (!target->hasAurasWithId(champions::SPELL_FORBEARANCE))
                {
                    castSpell(target, champions::SPELL_HAND_OF_PROTECTION);
                    repeatFunctionFromScheduler(pThis, 5min);
                }
                else
                {
                    repeatFunctionFromScheduler(pThis, 5s);
                }
            }
            else
            {
                repeatFunctionFromScheduler(pThis, 5s);
            }
        }, DoOnceScheduler(20s, 30s, 33.0f));

    SpellDesc mDivineShieldInfo(champions::SPELL_DIVINE_SHIELD, FilterArgs(TargetFilter_SelfBelowHealth, 0.0f, 0.0f, 0.0f, 30.0f, -champions::SPELL_FORBEARANCE), false);
    mDivineShieldInfo.setUseSpellCD(true);
    addAISpell(mDivineShieldInfo, DoLoopScheduler(15s, 30s, 33.0f));
}

void RetriAI::OnCombatStart(Unit* _target)
{
    FactionChampionsAI::OnCombatStart(_target);
    castSpellOnSelf(champions::SPELL_SEAL_OF_COMMAND);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// PetWarlock AI
PetWarlockAI::PetWarlockAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* PetWarlockAI::Create(Creature* pCreature) { return new PetWarlockAI(pCreature, champions::AI_PET); }

void PetWarlockAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    SpellDesc mDevourMagicInfo(champions::SPELL_DEVOUR_MAGIC, FilterArgs(TargetFilter_Current), false);
    mDevourMagicInfo.setUseSpellCD(true);
    addAISpell(mDevourMagicInfo, DoLoopScheduler(8s, 15s, 33.0f));

    SpellDesc mLockInfo(champions::SPELL_SPELL_LOCK, FilterArgs(TargetFilter_Current), false);
    mLockInfo.setUseSpellCD(true);
    addAISpell(mLockInfo, DoLoopScheduler(8s, 15s, 33.0f));
}

//////////////////////////////////////////////////////////////////////////////////////////
/// PetHunter AI
PetHunterAI::PetHunterAI(Creature* pCreature, uint8_t aitype) : FactionChampionsAI(pCreature, aitype) { }

CreatureAIScript* PetHunterAI::Create(Creature* pCreature) { return new PetHunterAI(pCreature, champions::AI_PET); }

void PetHunterAI::InitOrReset()
{
    FactionChampionsAI::InitOrReset();

    SpellDesc mClawInfo(champions::SPELL_CLAW, FilterArgs(TargetFilter_Current), false);
    addAISpell(mClawInfo, DoLoopScheduler(5s, 10s, 75.0f));
}
