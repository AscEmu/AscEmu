/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Trial_Of_Valor.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <set>
#include <vector>

enum TrialOfValorEvents
{
    EVENT_ODYN_DRAW_POWER = 1,
    EVENT_ODYN_UNERRING_BLAST,
    EVENT_ODYN_UNERRING_BLAST_HIT,
    EVENT_ODYN_SPEAR_OF_LIGHT,
    EVENT_ODYN_ODYNS_TEST,
    EVENT_ODYN_THUNDER_TRANSITION,
    EVENT_ODYN_STORM_OF_JUSTICE,
    EVENT_ODYN_STORMFORGED_SPEAR,
    EVENT_ODYN_CLEANSING_FLAME,
    EVENT_ODYN_ARCING_STORM,

    EVENT_HYMDALL_HORN_OF_VALOR,
    EVENT_HYMDALL_DANCING_BLADE,
    EVENT_HYRJA_SHIELD_OF_LIGHT,
    EVENT_HYRJA_EXPEL_LIGHT,

    EVENT_GUARM_FLAME_LICK,
    EVENT_GUARM_FROST_LICK,
    EVENT_GUARM_SHADOW_LICK,
    EVENT_GUARM_OFF_THE_LEASH,
    EVENT_GUARM_ROARING_LEAP,
    EVENT_GUARM_ROARING_LEAP_JUMP,
    EVENT_GUARM_HEADLONG_CHARGE,
    EVENT_GUARM_BACK_ON_LEASH,
    EVENT_GUARM_BERSERK,

    EVENT_HELYA_TAINT_OF_THE_SEA,
    EVENT_HELYA_TENTACLE_STRIKE,
    EVENT_HELYA_ORB_OF_CORRUPTION,
    EVENT_HELYA_CORRUPTION_ORBS,
    EVENT_HELYA_ORB_OF_CORROSION,
    EVENT_HELYA_CORROSION_ORBS,
    EVENT_HELYA_CORROSIVE_NOVA,
    EVENT_HELYA_TORRENT,
    EVENT_HELYA_FIRST_FURY,
    EVENT_HELYA_FURY_ADDS,

    EVENT_STRIKING_TENTACLE_STRIKE,
    EVENT_GRIMELORD_ANCHOR_SLAM,
    EVENT_GRIMELORD_SLUDGE_NOVA,
    EVENT_GRIMELORD_FETID_ROT,
    EVENT_MARINER_GIVE_NO_QUARTER,
    EVENT_MARINER_GHOSTLY_RAGE
};

enum TrialOfValorOdynStage : uint8_t
{
    ODYN_STAGE_THRONE,
    ODYN_STAGE_WATCHING,
    ODYN_STAGE_HOLY,
    ODYN_STAGE_THUNDER,
    ODYN_STAGE_DEFEATED
};

namespace
{
    float const ValorTwoPi = 6.28318531f;

    // runebearers enter the arena here
    LocationVector const RunebearerSpots[] =
    {
        { 2465.967f, 484.0451f, 749.0782f, 2.251475f },
        { 2409.139f, 485.3073f, 749.0782f, 1.117011f },
        { 2496.684f, 556.9410f, 749.0782f, 3.525565f },
        { 2466.094f, 573.3021f, 749.0782f, 4.014257f },
        { 2496.595f, 501.7517f, 749.0782f, 2.740167f }
    };

    uint32_t const RunebearerBrands[] =
    {
        SPELL_RUNEBEARER_BRANDED_PURPLE,
        SPELL_RUNEBEARER_BRANDED_ORANGE,
        SPELL_RUNEBEARER_BRANDED_BLUE,
        SPELL_RUNEBEARER_BRANDED_GREEN,
        SPELL_RUNEBEARER_BRANDED_YELLOW
    };

    LocationVector const OdynArenaCentre(2400.65f, 529.31f, 749.0f, 6.18879f);

    uint32_t const OdynCleansingFlames[] = { SPELL_ODYN_CLEANSING_FLAME_1, SPELL_ODYN_CLEANSING_FLAME_2, SPELL_ODYN_CLEANSING_FLAME_3 };

    uint32_t const GuarmBreaths[] = { SPELL_GUARM_GUARDIANS_BREATH_1, SPELL_GUARM_GUARDIANS_BREATH_2, SPELL_GUARM_GUARDIANS_BREATH_3 };

    // tentacle spots on the deck
    LocationVector const HelyaTentacleSpots[] =
    {
        { 525.5886f, 678.2604f, 2.877361f, 4.050999f },
        { 505.8260f, 693.5260f, 2.877363f, 4.667830f },
        { 475.3333f, 606.6406f, 2.891779f, 0.992388f },
        { 466.0955f, 698.0868f, 18.05038f, 4.312522f },
        { 564.8038f, 664.4705f, 18.03976f, 3.007097f }
    };

    uint32_t const HelyaStrikingSpotCount = 3;

    LocationVector const GrimelordSpot(458.674f, 639.483f, 7.53757f, 4.433769f);
    LocationVector const MarinerSpot(516.007f, 614.179f, 5.5823f, 3.943812f);

    uint32_t const HelyaBerserkMs = 660000;
    uint32_t const GuarmBerserkMs = 300000;

    // the Valarjar are friendly templates, the trial turns them hostile
    uint32_t const ValorHostileFaction = 16;
    uint32_t const ValorFriendlyFaction = 35;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TrialOfValorInstanceScript : public InstanceScript
{
public:
    explicit TrialOfValorInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TrialOfValorEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TrialOfValorInstanceScript(pMapMgr); }

    // Odyn never dies, his trial only ends when he yields
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterDown(id))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        bool open = false;
        if (!doorShouldBeOpen(pGameObject->getEntry(), open))
            return;

        mDoorGuids.insert(pGameObject->GetNewGUID());
        alignDoor(pGameObject, open);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        refreshDoors();

        if (id != DATA_ODYN)
            return;

        switch (state)
        {
            case EncounterStates::InProgress:
                sendAction(BOSS_ODYN, ACTION_ODYN_TRIAL_BEGINS);
                break;
            case EncounterStates::Failed:
                mOdynDefeated = false;
                mYieldedChampions.clear();
                sendAction(BOSS_HYMDALL, ACTION_VALARJAR_RESET);
                sendAction(BOSS_HYRJA, ACTION_VALARJAR_RESET);
                sendAction(BOSS_ODYN, ACTION_ODYN_TRIAL_RESET);
                break;
            case EncounterStates::Performed:
                sendAction(BOSS_HYMDALL, ACTION_VALARJAR_STAND_DOWN);
                sendAction(BOSS_HYRJA, ACTION_VALARJAR_STAND_DOWN);
                break;
            default:
                break;
        }
    }

    bool isEncounterDown(uint32_t id) const
    {
        return id != DATA_ODYN || mOdynDefeated;
    }

    void markOdynDefeated() { mOdynDefeated = true; }

    // Odyn steps in once both champions yielded
    void onChampionYielded(uint32_t entry)
    {
        mYieldedChampions.insert(entry);
        if (mYieldedChampions.count(BOSS_HYMDALL) != 0 && mYieldedChampions.count(BOSS_HYRJA) != 0)
            sendAction(BOSS_ODYN, ACTION_ODYN_TAKES_THE_FIELD);
    }

    std::vector<Creature*> getPullGroup(uint32_t encounterId)
    {
        std::vector<Creature*> group;
        if (encounterId != DATA_ODYN)
            return group;

        for (Creature* pChampion : getCreatureSetForEntries({ BOSS_HYMDALL, BOSS_HYRJA }))
        {
            if (pChampion != nullptr)
                group.push_back(pChampion);
        }

        return group;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature != nullptr && pCreature->isAlive() && pCreature->GetScript() != nullptr)
                pCreature->GetScript()->DoAction(action);
        }
    }

private:
    bool isDone(uint32_t id) const { return getBossState(id) == EncounterStates::Performed; }
    bool isFighting(uint32_t id) const { return getBossState(id) == EncounterStates::InProgress; }

    // false for objects that are no encounter door
    bool doorShouldBeOpen(uint32_t entry, bool& open) const
    {
        switch (entry)
        {
            case GO_ODYN_ARENA_GATE:
                open = !isFighting(DATA_ODYN);
                return true;
            case GO_GUARM_DOOR_ENTRANCE:
                open = !isFighting(DATA_GUARM);
                return true;
            case GO_GUARM_DOOR_EXIT:
                open = isDone(DATA_GUARM) && !isFighting(DATA_HELYA);
                return true;
            default:
                return false;
        }
    }

    void refreshDoors()
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            GameObject* pDoor = getGameObjectByGuid(guid);
            bool open = false;
            if (pDoor != nullptr && doorShouldBeOpen(pDoor->getEntry(), open))
                alignDoor(pDoor, open);
        }
    }

    // changes the state only when it differs
    void alignDoor(GameObject* pDoor, bool open)
    {
        bool const isOpen = pDoor->getState() != GO_STATE_CLOSED;
        if (isOpen != open)
            pDoor->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    bool mOdynDefeated = false;
    std::set<uint32_t> mYieldedChampions;
    std::set<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick while in combat

class TrialOfValorCombatAI : public CreatureAIScript
{
public:
    explicit TrialOfValorCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        onReset();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        onCombatTick(diff);

        if (_isCasting() || !getCreature()->isAlive())
            return;

        if (uint32_t eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onReset() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    TrialOfValorInstanceScript* getTrial()
    {
        if (getCreature()->GetMapId() != MAP_TRIAL_OF_VALOR)
            return nullptr;

        return static_cast<TrialOfValorInstanceScript*>(getInstanceScript());
    }

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank if possible
    Unit* randomNonTank()
    {
        if (Unit* pTarget = getBestPlayerTarget(TargetFilter_NotCurrent))
            return pTarget;

        return randomPlayer();
    }

    // up to count different players
    std::vector<Unit*> randomPlayers(size_t count)
    {
        std::vector<Unit*> picked;
        for (size_t attempt = 0; attempt < count * 3 && picked.size() < count; ++attempt)
        {
            Unit* pTarget = randomPlayer();
            if (pTarget == nullptr)
                break;

            if (std::find(picked.begin(), picked.end(), pTarget) == picked.end())
                picked.push_back(pTarget);
        }

        return picked;
    }

    void castOnRandomPlayer(uint32_t spellId, bool triggered = false)
    {
        if (Unit* pTarget = randomPlayer())
            castSpell(pTarget, spellId, triggered);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // random point on a ring around a unit
    LocationVector aroundUnit(Unit* pUnit, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(ValorTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(pUnit->GetPositionX() + range * std::cos(angle), pUnit->GetPositionY() + range * std::sin(angle), pUnit->GetPositionZ(), angle);
    }

    bool isBelowHealthShare(uint32_t health, uint32_t divisor)
    {
        return uint64_t(health) * divisor <= getCreature()->getMaxHealth();
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class TrialOfValorBossAI : public TrialOfValorCombatAI
{
public:
    TrialOfValorBossAI(Creature* pCreature, uint32_t encounterId) : TrialOfValorCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        // the partner joins unless it already yielded
        if (TrialOfValorInstanceScript* pTrial = getTrial())
        {
            for (Creature* pMember : pTrial->getPullGroup(mEncounterId))
            {
                if (pMember != getCreature() && pMember->isAlive() && !pMember->isInCombat() && !pMember->hasUnitFlags(UNIT_FLAG_NOT_SELECTABLE))
                    setZoneWideCombat(pMember);
            }
        }

        TrialOfValorCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (!getCreature()->isAlive())
            return;

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Failed);
        }

        TrialOfValorCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            TrialOfValorInstanceScript* pTrial = getTrial();
            if (pTrial == nullptr || pTrial->isEncounterDown(mEncounterId))
                pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }
    }

protected:
    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Odyn: watches from his throne, enters once Hymdall and Hyrja yielded, yields at 10%

class TrialOfValorOdynAI : public TrialOfValorCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TrialOfValorOdynAI(c); }
    explicit TrialOfValorOdynAI(Creature* pCreature) : TrialOfValorCombatAI(pCreature) {}

    void OnLoad() override
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance != nullptr && pInstance->getBossState(DATA_ODYN) == EncounterStates::Performed)
            standDown(false);
        else
            takeThrone();
    }

    // stage changes drive the fight, not the pull
    void OnCombatStart(Unit* /*pTarget*/) override {}

    void OnCombatStop(Unit* pTarget) override
    {
        if (!getCreature()->isAlive() || mStage == ODYN_STAGE_DEFEATED)
        {
            scriptEvents.resetEvents();
            return;
        }

        if (mStage >= ODYN_STAGE_HOLY)
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(DATA_ODYN, EncounterStates::Failed);
            }
        }

        TrialOfValorCombatAI::OnCombatStop(pTarget);
        takeThrone();
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mStage < ODYN_STAGE_HOLY || mStage == ODYN_STAGE_DEFEATED)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage >= health)
            *damage = health > 1 ? health - 1 : 0;

        if (isBelowHealthShare(health - *damage, 10))
            mYieldPending = true;
    }

    void DoAction(int32_t action) override
    {
        switch (action)
        {
            case ACTION_ODYN_TRIAL_BEGINS:
                if (mStage == ODYN_STAGE_THRONE)
                    watchTrial();
                break;
            case ACTION_ODYN_TAKES_THE_FIELD:
                if (mStage == ODYN_STAGE_WATCHING)
                    enterArena();
                break;
            case ACTION_ODYN_TRIAL_RESET:
                if (mStage != ODYN_STAGE_DEFEATED)
                    takeThrone();
                break;
            default:
                break;
        }
    }

    // ticks on the throne as well, the champions fight below him
    void AIUpdate(unsigned long time_passed) override
    {
        if (mStage == ODYN_STAGE_THRONE || mStage == ODYN_STAGE_DEFEATED)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mStage >= ODYN_STAGE_HOLY && !_isInCombat())
        {
            // nobody left to fight in the arena
            mIdleInArena += diff;
            if (mIdleInArena >= 5000)
                abandonTrial();
            return;
        }

        mIdleInArena = 0;
        scriptEvents.updateEvents(diff, 0);

        if (mYieldPending)
        {
            standDown(true);
            return;
        }

        if (mStage == ODYN_STAGE_HOLY && _getHealthPercent() <= 55)
            thunderStage();

        if (_isCasting())
            return;

        if (uint32_t eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ODYN_DRAW_POWER:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Odyn draws power from the runes! Bring the Runebearers to their runes!");
                castSpellOnSelf(SPELL_ODYN_DRAW_POWER, true);
                callRunebearers();
                scriptEvents.addEvent(EVENT_ODYN_UNERRING_BLAST, 29000);
                scriptEvents.addEvent(EVENT_ODYN_DRAW_POWER, 70000);
                break;
            case EVENT_ODYN_UNERRING_BLAST:
                castSpellAOE(SPELL_ODYN_UNERRING_BLAST);
                scriptEvents.addEvent(EVENT_ODYN_UNERRING_BLAST_HIT, 100);
                break;
            case EVENT_ODYN_UNERRING_BLAST_HIT:
                castSpellAOE(SPELL_ODYN_UNERRING_BLAST_DAMAGE, true);
                _removeAura(SPELL_ODYN_DRAW_POWER);
                summons.despawnEntry(NPC_VALARJAR_RUNEBEARER);
                break;
            case EVENT_ODYN_SPEAR_OF_LIGHT:
                for (Unit* pTarget : randomPlayers(5))
                    castSpell(pTarget, SPELL_ODYN_SPEAR_OF_LIGHT, true);
                scriptEvents.addEvent(EVENT_ODYN_SPEAR_OF_LIGHT, 10000);
                break;
            case EVENT_ODYN_ODYNS_TEST:
                castSpellOnSelf(SPELL_ODYN_ODYNS_TEST);
                scriptEvents.addEvent(EVENT_ODYN_ODYNS_TEST, 70000);
                break;
            case EVENT_ODYN_THUNDER_TRANSITION:
                setReactState(REACT_AGGRESSIVE);
                castSpellOnSelf(SPELL_ODYN_SPEAR_TRANSITION_THUNDER, true);
                castSpellOnSelf(SPELL_ODYN_ARCING_STORM, true);
                scriptEvents.addEvent(EVENT_ODYN_CLEANSING_FLAME, 1000);
                scriptEvents.addEvent(EVENT_ODYN_STORM_OF_JUSTICE, 4000);
                scriptEvents.addEvent(EVENT_ODYN_ARCING_STORM, 6000);
                scriptEvents.addEvent(EVENT_ODYN_STORMFORGED_SPEAR, 14000);
                break;
            case EVENT_ODYN_STORM_OF_JUSTICE:
                castSpellAOE(SPELL_ODYN_STORM_OF_JUSTICE);
                scriptEvents.addEvent(EVENT_ODYN_STORM_OF_JUSTICE, 12000);
                break;
            case EVENT_ODYN_STORMFORGED_SPEAR:
                castSpellOnVictim(SPELL_ODYN_STORMFORGED_SPEAR);
                scriptEvents.addEvent(EVENT_ODYN_STORMFORGED_SPEAR, 11000);
                break;
            case EVENT_ODYN_CLEANSING_FLAME:
                castSpellAOE(OdynCleansingFlames[mFlameIndex]);
                mFlameIndex = static_cast<uint8_t>((mFlameIndex + 1) % 3);
                scriptEvents.addEvent(EVENT_ODYN_CLEANSING_FLAME, 35000);
                break;
            case EVENT_ODYN_ARCING_STORM:
                castSpellOnSelf(SPELL_ODYN_ARCING_STORM_BUFF);
                scriptEvents.addEvent(EVENT_ODYN_ARCING_STORM, 9000);
                break;
            default:
                break;
        }
    }

private:
    void abandonTrial()
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(DATA_ODYN, EncounterStates::Failed);
        }

        takeThrone();
        moveToSpawn();
    }

    void takeThrone()
    {
        mStage = ODYN_STAGE_THRONE;
        mYieldPending = false;
        mFlameIndex = 0;
        mIdleInArena = 0;

        scriptEvents.resetEvents();
        summons.despawnEntry(NPC_VALARJAR_RUNEBEARER);
        _removeAura(SPELL_ODYN_DRAW_POWER);

        // hostile for his spells, out of reach until he leaps down
        getCreature()->setFaction(ValorHostileFaction);
        setCanEnterCombat(false);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_PASSIVE);
    }

    // the runes run while the champions are fought
    void watchTrial()
    {
        mStage = ODYN_STAGE_WATCHING;
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());

        scriptEvents.addEvent(EVENT_ODYN_DRAW_POWER, 40000);
    }

    void enterArena()
    {
        mStage = ODYN_STAGE_HOLY;
        scriptEvents.resetEvents();
        summons.despawnEntry(NPC_VALARJAR_RUNEBEARER);
        _removeAura(SPELL_ODYN_DRAW_POWER);

        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Odyn leaps into battle!");
        setCanEnterCombat(true);
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        castSpellOnSelf(SPELL_ODYN_LEAP_INTO_BATTLE, true);
        moveJump(OdynArenaCentre, 20.0f, 10.0f);
        setZoneWideCombat(getCreature());
        castSpellOnSelf(SPELL_ODYN_SPEAR_TRANSITION_HOLY, true);

        scriptEvents.addEvent(EVENT_ODYN_SPEAR_OF_LIGHT, 5000);
        scriptEvents.addEvent(EVENT_ODYN_ODYNS_TEST, 30000);
        scriptEvents.addEvent(EVENT_ODYN_DRAW_POWER, 40000);
    }

    // short pause while the spear changes to thunder
    void thunderStage()
    {
        mStage = ODYN_STAGE_THUNDER;
        scriptEvents.resetEvents();
        summons.despawnEntry(NPC_VALARJAR_RUNEBEARER);
        _removeAura(SPELL_ODYN_DRAW_POWER);
        _removeAura(SPELL_ODYN_SPEAR_TRANSITION_HOLY);
        _removeAura(SPELL_ODYN_ODYNS_TEST);

        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Odyn calls upon the storm!");
        setReactState(REACT_PASSIVE);
        attackStop();
        scriptEvents.addEvent(EVENT_ODYN_THUNDER_TRANSITION, 4000);
    }

    // trial passed, Odyn turns friendly
    void standDown(bool trialWon)
    {
        mStage = ODYN_STAGE_DEFEATED;
        mYieldPending = false;
        scriptEvents.resetEvents();
        summons.despawnEntry(NPC_VALARJAR_RUNEBEARER);

        if (trialWon)
        {
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Odyn yields! The trial is won!");
            if (InstanceScript* pInstance = getInstanceScript())
            {
                if (TrialOfValorInstanceScript* pTrial = getTrial())
                    pTrial->markOdynDefeated();

                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(DATA_ODYN, EncounterStates::Performed);
            }
        }

        setReactState(REACT_PASSIVE);
        attackStop();
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->setFaction(ValorFriendlyFaction);
        setCanEnterCombat(false);

        if (_isInCombat())
            getCreature()->getAIInterface()->enterEvadeMode();
    }

    void callRunebearers()
    {
        std::vector<Unit*> const targets = randomPlayers(5);
        for (size_t i = 0; i < 5; ++i)
        {
            Creature* pRunebearer = summonCreature(NPC_VALARJAR_RUNEBEARER, RunebearerSpots[i], CORPSE_DESPAWN);
            if (pRunebearer == nullptr || targets.empty())
                continue;

            // each runebearer brands one player and goes after him
            Unit* pTarget = targets[i % targets.size()];
            pRunebearer->castSpell(pTarget, RunebearerBrands[i], true);
            if (CreatureAIScript* pRunebearerAI = pRunebearer->GetScript())
            {
                pRunebearerAI->addThreat(pTarget, 100000.0f);
                pRunebearerAI->attackStart(pTarget);
            }
        }
    }

    uint8_t mStage = ODYN_STAGE_THRONE;
    bool mYieldPending = false;
    uint8_t mFlameIndex = 0;
    uint32_t mIdleInArena = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hymdall and Hyrja: cannot die, yield at a quarter health

class TrialOfValorChampionAI : public TrialOfValorBossAI
{
public:
    TrialOfValorChampionAI(Creature* pCreature, char const* yieldText) : TrialOfValorBossAI(pCreature, DATA_ODYN), mYieldText(yieldText) {}

    void OnLoad() override
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance != nullptr && pInstance->getBossState(DATA_ODYN) == EncounterStates::Performed)
            yield(false);
        else
            getCreature()->setFaction(ValorHostileFaction);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        // leaving after the yield is no wipe
        if (mYielded)
        {
            scriptEvents.resetEvents();
            return;
        }

        TrialOfValorBossAI::OnCombatStop(pTarget);
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mYielded)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage >= health)
            *damage = health > 1 ? health - 1 : 0;

        if (isBelowHealthShare(health - *damage, 4))
            mYieldPending = true;
    }

    void DoAction(int32_t action) override
    {
        switch (action)
        {
            case ACTION_VALARJAR_RESET:
                if (mYielded)
                    recover();
                break;
            case ACTION_VALARJAR_STAND_DOWN:
                if (!mYielded)
                    yield(false);
                break;
            default:
                break;
        }
    }

protected:
    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mYieldPending)
            yield(true);
    }

private:
    void yield(bool announce)
    {
        mYieldPending = false;
        mYielded = true;
        scriptEvents.resetEvents();

        if (announce)
        {
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, mYieldText);
            if (InstanceScript* pInstance = getInstanceScript())
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        }

        getCreature()->setFaction(ValorFriendlyFaction);
        setCanEnterCombat(false);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_PASSIVE);
        attackStop();

        if (announce)
        {
            if (TrialOfValorInstanceScript* pTrial = getTrial())
                pTrial->onChampionYielded(getCreature()->getEntry());
        }
    }

    void recover()
    {
        mYielded = false;
        mYieldPending = false;
        getCreature()->setFaction(ValorHostileFaction);
        setCanEnterCombat(true);
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        getCreature()->setHealth(getCreature()->getMaxHealth());
    }

    char const* mYieldText;
    bool mYielded = false;
    bool mYieldPending = false;
};

class TrialOfValorHymdallAI : public TrialOfValorChampionAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TrialOfValorHymdallAI(c); }
    explicit TrialOfValorHymdallAI(Creature* pCreature) : TrialOfValorChampionAI(pCreature, "Hymdall yields and leaves the arena!") {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_HYMDALL_HORN_OF_VALOR, 10000);
        scriptEvents.addEvent(EVENT_HYMDALL_DANCING_BLADE, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HYMDALL_HORN_OF_VALOR:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Hymdall sounds the Horn of Valor!");
                castSpellAOE(SPELL_HYMDALL_HORN_OF_VALOR);
                scriptEvents.addEvent(EVENT_HYMDALL_HORN_OF_VALOR, 69000);
                break;
            case EVENT_HYMDALL_DANCING_BLADE:
                if (Unit* pTarget = randomNonTank())
                    castSpell(pTarget, SPELL_HYMDALL_DANCING_BLADE);
                scriptEvents.addEvent(EVENT_HYMDALL_DANCING_BLADE, 30000);
                break;
            default:
                break;
        }
    }
};

class TrialOfValorHyrjaAI : public TrialOfValorChampionAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TrialOfValorHyrjaAI(c); }
    explicit TrialOfValorHyrjaAI(Creature* pCreature) : TrialOfValorChampionAI(pCreature, "Hyrja yields and leaves the arena!") {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_HYRJA_SHIELD_OF_LIGHT, 30000);
        scriptEvents.addEvent(EVENT_HYRJA_EXPEL_LIGHT, 40000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HYRJA_SHIELD_OF_LIGHT:
                if (Unit* pTarget = randomNonTank())
                {
                    sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Hyrja casts Shield of Light! Stand between her and her target!");
                    castSpell(pTarget, SPELL_HYRJA_SHIELD_OF_LIGHT);
                }
                scriptEvents.addEvent(EVENT_HYRJA_SHIELD_OF_LIGHT, 32000);
                break;
            case EVENT_HYRJA_EXPEL_LIGHT:
                castOnRandomPlayer(SPELL_HYRJA_EXPEL_LIGHT);
                scriptEvents.addEvent(EVENT_HYRJA_EXPEL_LIGHT, 19500);
                break;
            default:
                break;
        }
    }
};

class TrialOfValorRunebearerAI : public TrialOfValorCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TrialOfValorRunebearerAI(c); }
    explicit TrialOfValorRunebearerAI(Creature* pCreature) : TrialOfValorCombatAI(pCreature) {}

    void OnLoad() override { castSpellOnSelf(SPELL_RUNEBEARER_SPAWN, true); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Guarm: 33 energy on pull, 5 per second, Guardian's Breath at full; off the leash every 30s

class TrialOfValorGuarmAI : public TrialOfValorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TrialOfValorGuarmAI(c); }
    explicit TrialOfValorGuarmAI(Creature* pCreature) : TrialOfValorBossAI(pCreature, DATA_GUARM) {}

    void OnLoad() override
    {
        // vehicle template: the core keeps it passive unless the AI is switched on
        getCreature()->setAItoUse(true);
        setCanEnterCombat(true);
    }

protected:
    void onEngage() override
    {
        mEnergy = 33;
        mEnergyClock = 0;
        mFangsUsed = false;
        mOffTheLeash = false;

        leashedRotation();
        scriptEvents.addEvent(EVENT_GUARM_BERSERK, GuarmBerserkMs);
    }

    void onCombatTick(uint32_t diff) override
    {
        // no energy while running loose
        if (mOffTheLeash)
            return;

        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy += 5;
        }

        if (_isCasting())
            return;

        if (!mFangsUsed && mEnergy >= 50)
        {
            mFangsUsed = true;
            castSpellOnVictim(SPELL_GUARM_FLASHING_FANGS);
            return;
        }

        if (mEnergy >= 100)
        {
            mEnergy = 0;
            mFangsUsed = false;
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Guarm unleashes Guardian's Breath!");
            castSpellAOE(GuarmBreaths[Util::getRandomUInt(2)]);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GUARM_FLAME_LICK:
                castOnRandomPlayer(SPELL_GUARM_FLAME_LICK);
                scriptEvents.addEvent(EVENT_GUARM_FLAME_LICK, 13000);
                break;
            case EVENT_GUARM_FROST_LICK:
                castOnRandomPlayer(SPELL_GUARM_FROST_LICK);
                scriptEvents.addEvent(EVENT_GUARM_FROST_LICK, 24000);
                break;
            case EVENT_GUARM_SHADOW_LICK:
                castOnRandomPlayer(SPELL_GUARM_SHADOW_LICK);
                scriptEvents.addEvent(EVENT_GUARM_SHADOW_LICK, 27000);
                break;
            case EVENT_GUARM_OFF_THE_LEASH:
                mOffTheLeash = true;
                scriptEvents.removeEvent(EVENT_GUARM_FLAME_LICK);
                scriptEvents.removeEvent(EVENT_GUARM_FROST_LICK);
                scriptEvents.removeEvent(EVENT_GUARM_SHADOW_LICK);
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Guarm breaks free! Off the Leash!");
                castSpellOnSelf(SPELL_GUARM_OFF_THE_LEASH, true);
                scriptEvents.addEvent(EVENT_GUARM_ROARING_LEAP, 2000);
                scriptEvents.addEvent(EVENT_GUARM_HEADLONG_CHARGE, 7000);
                scriptEvents.addEvent(EVENT_GUARM_BACK_ON_LEASH, 18000);
                break;
            case EVENT_GUARM_ROARING_LEAP:
                castSpellAOE(SPELL_GUARM_ROARING_LEAP);
                scriptEvents.addEvent(EVENT_GUARM_ROARING_LEAP_JUMP, 2000);
                break;
            case EVENT_GUARM_ROARING_LEAP_JUMP:
                castOnRandomPlayer(SPELL_GUARM_ROARING_LEAP_JUMP);
                break;
            case EVENT_GUARM_HEADLONG_CHARGE:
                castSpellAOE(SPELL_GUARM_HEADLONG_CHARGE);
                break;
            case EVENT_GUARM_BACK_ON_LEASH:
                mOffTheLeash = false;
                _removeAura(SPELL_GUARM_OFF_THE_LEASH);
                leashedRotation();
                break;
            case EVENT_GUARM_BERSERK:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Guarm goes berserk!");
                castSpellOnSelf(SPELL_GUARM_BERSERK, true);
                break;
            default:
                break;
        }
    }

private:
    void leashedRotation()
    {
        scriptEvents.addEvent(EVENT_GUARM_FLAME_LICK, 12000);
        scriptEvents.addEvent(EVENT_GUARM_FROST_LICK, 24000);
        scriptEvents.addEvent(EVENT_GUARM_SHADOW_LICK, 27000);
        scriptEvents.addEvent(EVENT_GUARM_OFF_THE_LEASH, 30000);
    }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    bool mFangsUsed = false;
    bool mOffTheLeash = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Helya: stage 1 on the deck, submerged at 65% until the Gripping Tentacles fall, stage 3 on the deck again

class TrialOfValorHelyaAI : public TrialOfValorBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TrialOfValorHelyaAI(c); }
    explicit TrialOfValorHelyaAI(Creature* pCreature) : TrialOfValorBossAI(pCreature, DATA_HELYA) {}

    void OnLoad() override { setRooted(true); }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mStage == 2)
            *damage = 0;
    }

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || pSummon->getEntry() != NPC_GRIPPING_TENTACLE || mStage != 2)
            return;

        if (mTentaclesLeft > 0 && --mTentaclesLeft == 0)
            resurface();
    }

protected:
    void onEngage() override
    {
        mStage = 1;
        mEnergy = 79;
        mEnergyTicks = 0;
        mFury = 0;
        mResourceClock = 0;
        mTentaclesLeft = 0;
        mBerserkLeft = HelyaBerserkMs;

        scriptEvents.addEvent(EVENT_HELYA_TAINT_OF_THE_SEA, 12000);
        scriptEvents.addEvent(EVENT_HELYA_ORB_OF_CORRUPTION, 18000);
        scriptEvents.addEvent(EVENT_HELYA_TENTACLE_STRIKE, 53000);
        scriptEvents.addEvent(EVENT_HELYA_CORROSIVE_NOVA, 5000);
    }

    void onReset() override
    {
        mStage = 0;
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mBerserkLeft > 0)
        {
            if (mBerserkLeft > diff)
            {
                mBerserkLeft -= diff;
            }
            else
            {
                mBerserkLeft = 0;
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Helya goes berserk!");
                castSpellOnSelf(SPELL_HELYA_BERSERK, true);
            }
        }

        if (mStage == 1 && _getHealthPercent() <= 65)
        {
            submerge();
            return;
        }

        // energy in stage 1 and 3, fury of the maw in stage 2 and 3
        mResourceClock += diff;
        while (mResourceClock >= 1000)
        {
            mResourceClock -= 1000;
            if (mStage == 1)
                mEnergy += (mEnergyTicks++ % 5 == 4) ? 1 : 2;
            else if (mStage == 3)
                mEnergy += 2;

            if (mStage >= 2)
                mFury += 2;
        }

        if (_isCasting())
            return;

        if (mStage != 2 && mEnergy >= 100)
        {
            mEnergy = 0;
            if (mStage == 1)
            {
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Helya begins to cast Bilewater Breath!");
                castSpellAOE(SPELL_HELYA_BILEWATER_BREATH);
            }
            else
            {
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Helya begins to cast Corrupted Breath!");
                castSpellAOE(SPELL_HELYA_CORRUPTED_BREATH);
            }
            return;
        }

        if (mStage >= 2 && mFury >= 100)
        {
            mFury = 0;
            furyOfTheMaw();
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HELYA_TAINT_OF_THE_SEA:
                for (Unit* pTarget : randomPlayers(3))
                    castSpell(pTarget, SPELL_HELYA_TAINT_OF_THE_SEA, true);
                scriptEvents.addEvent(EVENT_HELYA_TAINT_OF_THE_SEA, 12000);
                break;
            case EVENT_HELYA_TENTACLE_STRIKE:
                strikeWithTentacle();
                scriptEvents.addEvent(EVENT_HELYA_TENTACLE_STRIKE, 30000);
                break;
            case EVENT_HELYA_ORB_OF_CORRUPTION:
                castSpellAOE(SPELL_HELYA_ORB_OF_CORRUPTION);
                scriptEvents.addEvent(EVENT_HELYA_CORRUPTION_ORBS, 2000);
                scriptEvents.addEvent(EVENT_HELYA_ORB_OF_CORRUPTION, 26000);
                break;
            case EVENT_HELYA_CORRUPTION_ORBS:
                releaseOrbs(NPC_ORB_OF_CORRUPTION);
                break;
            case EVENT_HELYA_ORB_OF_CORROSION:
                castSpellAOE(SPELL_HELYA_ORB_OF_CORROSION);
                scriptEvents.addEvent(EVENT_HELYA_CORROSION_ORBS, 2000);
                scriptEvents.addEvent(EVENT_HELYA_ORB_OF_CORROSION, 17000);
                break;
            case EVENT_HELYA_CORROSION_ORBS:
                releaseOrbs(NPC_ORB_OF_CORROSION);
                break;
            case EVENT_HELYA_CORROSIVE_NOVA:
            {
                // nobody in melee reach
                Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
                if (pVictim == nullptr || getRangeToObject(pVictim) > 10.0f)
                    castSpellAOE(SPELL_HELYA_CORROSIVE_NOVA);
                scriptEvents.addEvent(EVENT_HELYA_CORROSIVE_NOVA, 5000);
                break;
            }
            case EVENT_HELYA_TORRENT:
                castOnRandomPlayer(SPELL_HELYA_TORRENT);
                scriptEvents.addEvent(EVENT_HELYA_TORRENT, 10000);
                break;
            case EVENT_HELYA_FIRST_FURY:
                furyOfTheMaw();
                break;
            case EVENT_HELYA_FURY_ADDS:
                if (mStage == 2)
                    callAdd(NPC_GRIMELORD, GrimelordSpot);
                callAdd(NPC_NIGHT_WATCH_MARINER, MarinerSpot);
                break;
            default:
                break;
        }
    }

private:
    void submerge()
    {
        mStage = 2;
        mFury = 0;
        scriptEvents.removeEvent(EVENT_HELYA_TAINT_OF_THE_SEA);
        scriptEvents.removeEvent(EVENT_HELYA_TENTACLE_STRIKE);
        scriptEvents.removeEvent(EVENT_HELYA_ORB_OF_CORRUPTION);
        scriptEvents.removeEvent(EVENT_HELYA_CORRUPTION_ORBS);
        scriptEvents.removeEvent(EVENT_HELYA_CORROSIVE_NOVA);

        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Helya submerges! Destroy the Gripping Tentacles!");
        castSpellOnSelf(SPELL_HELYA_SUBMERGED, true);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_PASSIVE);
        attackStop();

        mTentaclesLeft = 0;
        for (LocationVector const& spot : HelyaTentacleSpots)
        {
            if (callAdd(NPC_GRIPPING_TENTACLE, spot) != nullptr)
                ++mTentaclesLeft;
        }

        if (mTentaclesLeft == 0)
        {
            resurface();
            return;
        }

        scriptEvents.addEvent(EVENT_HELYA_FIRST_FURY, 2000);
        scriptEvents.addEvent(EVENT_HELYA_TORRENT, 9000);
    }

    void resurface()
    {
        mStage = 3;
        mEnergy = 61;
        mFury = 42;
        scriptEvents.removeEvent(EVENT_HELYA_TORRENT);
        scriptEvents.removeEvent(EVENT_HELYA_FIRST_FURY);

        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Helya rises from the depths!");
        _removeAura(SPELL_HELYA_SUBMERGED);
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);

        scriptEvents.addEvent(EVENT_HELYA_TAINT_OF_THE_SEA, 12000);
        scriptEvents.addEvent(EVENT_HELYA_ORB_OF_CORROSION, 18000);
        scriptEvents.addEvent(EVENT_HELYA_CORROSIVE_NOVA, 5000);
    }

    // boats bring Kvaldir reinforcements a few seconds later
    void furyOfTheMaw()
    {
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Helya calls the Fury of the Maw!");
        castSpellAOE(SPELL_HELYA_FURY_OF_THE_MAW);
        scriptEvents.addEvent(EVENT_HELYA_FURY_ADDS, 4000);
    }

    void strikeWithTentacle()
    {
        LocationVector const& spot = HelyaTentacleSpots[Util::getRandomUInt(HelyaStrikingSpotCount - 1)];
        if (Creature* pTentacle = summonCreature(NPC_STRIKING_TENTACLE, spot, TIMED_DESPAWN, 9000))
        {
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "A Striking Tentacle rises from the sea!");
            setZoneWideCombat(pTentacle);
        }
    }

    void releaseOrbs(uint32_t entry)
    {
        for (Unit* pTarget : randomPlayers(2))
            summonCreature(entry, aroundUnit(pTarget, 5.0f, 8.0f), TIMED_DESPAWN, 9000);
    }

    uint8_t mStage = 0;
    uint32_t mEnergy = 0;
    uint32_t mEnergyTicks = 0;
    uint32_t mFury = 0;
    uint32_t mResourceClock = 0;
    uint32_t mTentaclesLeft = 0;
    uint32_t mBerserkLeft = 0;
};

class TrialOfValorGrippingTentacleAI : public TrialOfValorCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TrialOfValorGrippingTentacleAI(c); }
    explicit TrialOfValorGrippingTentacleAI(Creature* pCreature) : TrialOfValorCombatAI(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        setReactState(REACT_PASSIVE);
    }
};

class TrialOfValorStrikingTentacleAI : public TrialOfValorCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TrialOfValorStrikingTentacleAI(c); }
    explicit TrialOfValorStrikingTentacleAI(Creature* pCreature) : TrialOfValorCombatAI(pCreature) {}

    void OnLoad() override { setRooted(true); }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_STRIKING_TENTACLE_STRIKE, 1000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId == EVENT_STRIKING_TENTACLE_STRIKE)
            castOnRandomPlayer(SPELL_STRIKING_TENTACLE_STRIKE);
    }
};

// orbs drift towards the closest player and burst on contact
class TrialOfValorHelyaOrbAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TrialOfValorHelyaOrbAI(c); }
    explicit TrialOfValorHelyaOrbAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        bool const corrosion = getCreature()->getEntry() == NPC_ORB_OF_CORROSION;
        mFixateSpell = corrosion ? SPELL_ORB_OF_CORROSION_FIXATE : SPELL_ORB_OF_CORRUPTION_FIXATE;

        setReactState(REACT_PASSIVE);
        castSpellOnSelf(corrosion ? SPELL_ORB_OF_CORROSION_AURA : SPELL_ORB_OF_CORRUPTION_AURA, true);
        mFollowClock = 500;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mFollowClock > diff)
        {
            mFollowClock -= diff;
            return;
        }

        mFollowClock = 1000;
        Player* pPlayer = getNearestPlayer();
        if (pPlayer == nullptr)
            return;

        if (!mFixated)
        {
            mFixated = true;
            castSpell(pPlayer, mFixateSpell, true);
        }

        movePoint(1, pPlayer->GetPosition(), false);
    }

private:
    uint32_t mFixateSpell = 0;
    uint32_t mFollowClock = 0;
    bool mFixated = false;
};

class TrialOfValorGrimelordAI : public TrialOfValorCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TrialOfValorGrimelordAI(c); }
    explicit TrialOfValorGrimelordAI(Creature* pCreature) : TrialOfValorCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GRIMELORD_FETID_ROT, 9000);
        scriptEvents.addEvent(EVENT_GRIMELORD_ANCHOR_SLAM, 14000);
        scriptEvents.addEvent(EVENT_GRIMELORD_SLUDGE_NOVA, 16000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GRIMELORD_FETID_ROT:
                for (Unit* pTarget : randomPlayers(2))
                    castSpell(pTarget, SPELL_GRIMELORD_FETID_ROT, true);
                scriptEvents.addEvent(EVENT_GRIMELORD_FETID_ROT, 17000);
                break;
            case EVENT_GRIMELORD_ANCHOR_SLAM:
                castSpellOnVictim(SPELL_GRIMELORD_ANCHOR_SLAM);
                scriptEvents.addEvent(EVENT_GRIMELORD_ANCHOR_SLAM, 12000);
                break;
            case EVENT_GRIMELORD_SLUDGE_NOVA:
                castSpellAOE(SPELL_GRIMELORD_SLUDGE_NOVA);
                scriptEvents.addEvent(EVENT_GRIMELORD_SLUDGE_NOVA, 25000);
                break;
            default:
                break;
        }
    }
};

// Night Watch Mariner: lantern at full energy, roughly every 30s
class TrialOfValorNightWatchMarinerAI : public TrialOfValorCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TrialOfValorNightWatchMarinerAI(c); }
    explicit TrialOfValorNightWatchMarinerAI(Creature* pCreature) : TrialOfValorCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyTicks = 0;
        mEnergyClock = 0;
        scriptEvents.addEvent(EVENT_MARINER_GIVE_NO_QUARTER, 8000);
        scriptEvents.addEvent(EVENT_MARINER_GHOSTLY_RAGE, 10000);
    }

    void onCombatTick(uint32_t diff) override
    {
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy += (mEnergyTicks++ % 3 == 2) ? 4 : 3;
        }

        if (mEnergy >= 100 && !_isCasting())
        {
            mEnergy = 0;
            castSpellAOE(SPELL_MARINER_LANTERN_OF_DARKNESS);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MARINER_GIVE_NO_QUARTER:
                castOnRandomPlayer(SPELL_MARINER_GIVE_NO_QUARTER);
                scriptEvents.addEvent(EVENT_MARINER_GIVE_NO_QUARTER, 9000);
                break;
            case EVENT_MARINER_GHOSTLY_RAGE:
                castSpellAOE(SPELL_MARINER_GHOSTLY_RAGE);
                scriptEvents.addEvent(EVENT_MARINER_GHOSTLY_RAGE, 16000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mEnergy = 0;
    uint32_t mEnergyTicks = 0;
    uint32_t mEnergyClock = 0;
};

void SetupTrialOfValor(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TRIAL_OF_VALOR, &TrialOfValorInstanceScript::Create);

    mgr->register_creature_script(BOSS_ODYN, &TrialOfValorOdynAI::Create);
    mgr->register_creature_script(BOSS_HYMDALL, &TrialOfValorHymdallAI::Create);
    mgr->register_creature_script(BOSS_HYRJA, &TrialOfValorHyrjaAI::Create);
    mgr->register_creature_script(NPC_VALARJAR_RUNEBEARER, &TrialOfValorRunebearerAI::Create);

    mgr->register_creature_script(BOSS_GUARM, &TrialOfValorGuarmAI::Create);

    mgr->register_creature_script(BOSS_HELYA, &TrialOfValorHelyaAI::Create);
    mgr->register_creature_script(NPC_GRIPPING_TENTACLE, &TrialOfValorGrippingTentacleAI::Create);
    mgr->register_creature_script(NPC_STRIKING_TENTACLE, &TrialOfValorStrikingTentacleAI::Create);
    mgr->register_creature_script(NPC_ORB_OF_CORRUPTION, &TrialOfValorHelyaOrbAI::Create);
    mgr->register_creature_script(NPC_ORB_OF_CORROSION, &TrialOfValorHelyaOrbAI::Create);
    mgr->register_creature_script(NPC_GRIMELORD, &TrialOfValorGrimelordAI::Create);
    mgr->register_creature_script(NPC_NIGHT_WATCH_MARINER, &TrialOfValorNightWatchMarinerAI::Create);
}
