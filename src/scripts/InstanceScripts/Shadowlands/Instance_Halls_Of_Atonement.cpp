/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Halls_Of_Atonement.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>

enum HallsOfAtonementEvents
{
    EVENT_ECHELON_BLOOD_TORRENT = 1,
    EVENT_ECHELON_FLESH_TO_STONE,
    EVENT_ECHELON_STONE_CALL,
    EVENT_ECHELON_LEAP,
    EVENT_ECHELON_LEAP_IMPACT,

    EVENT_CHAMBERLAIN_TELEKINETIC_TOSS,
    EVENT_CHAMBERLAIN_UNLEASHED_SUFFERING,
    EVENT_CHAMBERLAIN_ERUPTING_TORMENT,
    EVENT_CHAMBERLAIN_CHORUS_REPULSION,
    EVENT_CHAMBERLAIN_CHORUS_ONSLAUGHT,
    EVENT_CHAMBERLAIN_CHORUS_RITUAL,

    EVENT_HALKIAS_CRUMBLING_SLAM,
    EVENT_HALKIAS_HEAVE_DEBRIS,
    EVENT_HALKIAS_SINLIGHT_VISIONS,
    EVENT_HALKIAS_REFRACTED_SINLIGHT,

    EVENT_ALEEZ_ANIMA_BOLT,
    EVENT_ALEEZ_ANIMA_FOUNTAIN,
    EVENT_ALEEZ_SPECTRAL_PROCESSION,
    EVENT_PARISHIONER_HAUNT
};

enum HallsOfAtonementActions
{
    ACTION_STONEFIEND_SHATTER = 1
};

namespace
{
    float const HallsTwoPi = 6.28318531f;

    uint32_t const EchelonStonefiendsPerCall = 3;
    uint32_t const EchelonStonefiendsPerCallHeroic = 4;
    float const EchelonShatterReach = 10.0f;
    uint32_t const StonefiendStoneMs = 20000;

    int32_t const ChamberlainChorusMarks[2] = { 70, 40 };

    uint32_t const ParishionerLifeMs = 40000;

    LocationVector hallsRing(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(HallsTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // true once when the countdown runs out
    bool hallsCountdown(uint32_t& leftMs, uint32_t diff)
    {
        if (leftMs == 0)
            return false;

        if (leftMs > diff)
        {
            leftMs -= diff;
            return false;
        }

        leftMs = 0;
        return true;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class HallsOfAtonementInstanceScript : public InstanceScript
{
public:
    explicit HallsOfAtonementInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(HallsOfAtonementEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new HallsOfAtonementInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and target helpers

class HallsOfAtonementCombatAI : public CreatureAIScript
{
public:
    explicit HallsOfAtonementCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        onCombatTick(diff);

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    HallsOfAtonementInstanceScript* getHalls()
    {
        if (getCreature()->GetMapId() != MAP_HALLS_OF_ATONEMENT)
            return nullptr;

        return static_cast<HallsOfAtonementInstanceScript*>(getInstanceScript());
    }

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank, the tank when alone
    Unit* randomNonTank()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_NotCurrent);
        if (Unit* pTarget = selectUnitTarget(args))
            return pTarget;

        return randomPlayer();
    }

    Unit* castAtRandom(uint32_t spellId, bool skipTank = false)
    {
        Unit* pTarget = skipTank ? randomNonTank() : randomPlayer();
        if (pTarget != nullptr)
            castSpell(pTarget, spellId);

        return pTarget;
    }

    // summoned add on the side of its summoner that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, durationMs);
        if (pAdd == nullptr)
            return nullptr;

        pAdd->setFaction(getCreature()->getFactionTemplate());
        setZoneWideCombat(pAdd);
        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return hallsRing(getCreature()->GetPosition(), minRange, maxRange); }

    void focusOn(Unit* pTarget)
    {
        if (pTarget == nullptr)
            return;

        _clearHateList();
        addThreat(pTarget, 100000.0f);
        attackStart(pTarget);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class HallsOfAtonementBossAI : public HallsOfAtonementCombatAI
{
public:
    HallsOfAtonementBossAI(Creature* pCreature, uint32_t dataId) : HallsOfAtonementCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (HallsOfAtonementInstanceScript* pHalls = getHalls())
        {
            pHalls->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pHalls->setBossState(mDataId, EncounterStates::InProgress);
        }

        HallsOfAtonementCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (HallsOfAtonementInstanceScript* pHalls = getHalls())
            {
                pHalls->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pHalls->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        HallsOfAtonementCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (HallsOfAtonementInstanceScript* pHalls = getHalls())
        {
            pHalls->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pHalls->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Echelon

class HallsOfAtonementEchelonAI : public HallsOfAtonementBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfAtonementEchelonAI(c); }
    explicit HallsOfAtonementEchelonAI(Creature* pCreature) : HallsOfAtonementBossAI(pCreature, DATA_ECHELON) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ECHELON_BLOOD_TORRENT, 6000);
        scriptEvents.addEvent(EVENT_ECHELON_FLESH_TO_STONE, 12000);
        scriptEvents.addEvent(EVENT_ECHELON_STONE_CALL, 15000);
        scriptEvents.addEvent(EVENT_ECHELON_LEAP, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ECHELON_BLOOD_TORRENT:
                castSpellAOE(SPELL_ECHELON_BLOOD_TORRENT);
                scriptEvents.addEvent(EVENT_ECHELON_BLOOD_TORRENT, 17000);
                break;
            case EVENT_ECHELON_FLESH_TO_STONE:
                castAtRandom(SPELL_ECHELON_FLESH_TO_STONE);
                scriptEvents.addEvent(EVENT_ECHELON_FLESH_TO_STONE, 22000);
                break;
            case EVENT_ECHELON_STONE_CALL:
            {
                warn("Echelon calls forth Undying Stonefiends!");
                castSpellOnSelf(SPELL_ECHELON_STONE_CALL);
                uint32_t const count = isHeroic() ? EchelonStonefiendsPerCallHeroic : EchelonStonefiendsPerCall;
                for (uint32_t i = 0; i < count; ++i)
                    callAdd(NPC_ECHELON_UNDYING_STONEFIEND, aroundSelf(8.0f, 16.0f));
                scriptEvents.addEvent(EVENT_ECHELON_STONE_CALL, 35000);
                break;
            }
            case EVENT_ECHELON_LEAP:
                if (Unit* pTarget = randomPlayer())
                {
                    warn("Echelon prepares a Stone Shattering Leap!");
                    mLeapSpot = pTarget->GetPosition();
                    moveJump(mLeapSpot, 25.0f, 12.0f);
                    scriptEvents.addEvent(EVENT_ECHELON_LEAP_IMPACT, 1500);
                }
                scriptEvents.addEvent(EVENT_ECHELON_LEAP, 30000);
                break;
            case EVENT_ECHELON_LEAP_IMPACT:
                getCreature()->castSpellLoc(mLeapSpot, SPELL_ECHELON_STONE_SHATTERING_LEAP, true);
                shatterStonefiends();
                break;
            default:
                break;
        }
    }

private:
    // stonefiends turned to stone break apart under the impact
    void shatterStonefiends()
    {
        for (uint64_t const guid : summons)
        {
            Creature* pFiend = getCreature()->getWorldMapCreature(guid);
            if (pFiend == nullptr || !pFiend->isAlive() || pFiend->GetScript() == nullptr)
                continue;

            if (pFiend->getDistance(mLeapSpot) <= EchelonShatterReach)
                pFiend->GetScript()->DoAction(ACTION_STONEFIEND_SHATTER);
        }
    }

    LocationVector mLeapSpot;
};

// cannot die by normal means, turns to stone at its last breath
class HallsOfAtonementUndyingStonefiendAI : public HallsOfAtonementCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfAtonementUndyingStonefiendAI(c); }
    explicit HallsOfAtonementUndyingStonefiendAI(Creature* pCreature) : HallsOfAtonementCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mShattering)
            return;

        if (mStone)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        mPetrifyPending = true;
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_STONEFIEND_SHATTER || !mStone || mShattering)
            return;

        mShattering = true;
        getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
    }

protected:
    void onEngage() override
    {
        mStone = false;
        mPetrifyPending = false;
        mShattering = false;
    }

    void onDisengage() override
    {
        if (mStone)
            wakeFromStone();
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mPetrifyPending)
        {
            mPetrifyPending = false;
            turnToStone();
        }

        if (mStone && hallsCountdown(mStoneLeftMs, diff))
            wakeFromStone();
    }

private:
    void turnToStone()
    {
        mStone = true;
        mStoneLeftMs = StonefiendStoneMs;
        getCreature()->interruptSpell();
        castSpellAOE(SPELL_STONEFIEND_VOLATILE_TRANSFORMATION, true);
        castSpellOnSelf(SPELL_STONEFIEND_STONE_FORM, true);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    // left whole for too long it rises again
    void wakeFromStone()
    {
        mStone = false;
        mStoneLeftMs = 0;
        _removeAura(SPELL_STONEFIEND_STONE_FORM);
        setRooted(false);
        _setMeleeDisabled(false);
        getCreature()->setHealth(getCreature()->getMaxHealth());
    }

    bool mStone = false;
    bool mPetrifyPending = false;
    bool mShattering = false;
    uint32_t mStoneLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lord Chamberlain, sings his Chorus at 70% and 40%

class HallsOfAtonementLordChamberlainAI : public HallsOfAtonementBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfAtonementLordChamberlainAI(c); }
    explicit HallsOfAtonementLordChamberlainAI(Creature* pCreature) : HallsOfAtonementBossAI(pCreature, DATA_LORD_CHAMBERLAIN) {}

protected:
    void onEngage() override
    {
        mChorusCount = 0;
        mSinging = false;
        scriptEvents.addEvent(EVENT_CHAMBERLAIN_TELEKINETIC_TOSS, 6000);
        scriptEvents.addEvent(EVENT_CHAMBERLAIN_UNLEASHED_SUFFERING, 10000);
        scriptEvents.addEvent(EVENT_CHAMBERLAIN_ERUPTING_TORMENT, 20000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mSinging && mChorusCount < 2 && _getHealthPercent() <= ChamberlainChorusMarks[mChorusCount])
            startChorus();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CHAMBERLAIN_TELEKINETIC_TOSS:
                castAtRandom(SPELL_CHAMBERLAIN_TELEKINETIC_TOSS, true);
                scriptEvents.addEvent(EVENT_CHAMBERLAIN_TELEKINETIC_TOSS, 12000);
                break;
            case EVENT_CHAMBERLAIN_UNLEASHED_SUFFERING:
                castSpellOnVictim(SPELL_CHAMBERLAIN_UNLEASHED_SUFFERING);
                scriptEvents.addEvent(EVENT_CHAMBERLAIN_UNLEASHED_SUFFERING, 18000);
                break;
            case EVENT_CHAMBERLAIN_ERUPTING_TORMENT:
                warn("Lord Chamberlain unleashes Erupting Torment!");
                castSpellAOE(SPELL_CHAMBERLAIN_ERUPTING_TORMENT);
                scriptEvents.addEvent(EVENT_CHAMBERLAIN_ERUPTING_TORMENT, 25000);
                break;
            case EVENT_CHAMBERLAIN_CHORUS_REPULSION:
                castSpellAOE(SPELL_CHAMBERLAIN_TELEKINETIC_REPULSION);
                break;
            case EVENT_CHAMBERLAIN_CHORUS_ONSLAUGHT:
                castSpellAOE(SPELL_CHAMBERLAIN_TELEKINETIC_ONSLAUGHT);
                break;
            case EVENT_CHAMBERLAIN_CHORUS_RITUAL:
            {
                mSinging = false;
                warn("Lord Chamberlain begins the Ritual of Woe!");
                Creature* pStatue = findNearestCreature(NPC_CHAMBERLAIN_SINSTONE_STATUE, 60.0f);
                if (pStatue != nullptr)
                    castSpell(pStatue, SPELL_CHAMBERLAIN_RITUAL_OF_WOE);
                else
                    castSpellOnSelf(SPELL_CHAMBERLAIN_RITUAL_OF_WOE);
                break;
            }
            default:
                break;
        }
    }

private:
    // repulsion, onslaught and the ritual follow in order
    void startChorus()
    {
        ++mChorusCount;
        mSinging = true;
        warn("Lord Chamberlain begins Chamberlain's Chorus!");
        getCreature()->interruptSpell();
        castSpellOnSelf(SPELL_CHAMBERLAIN_CHORUS, true);

        scriptEvents.addEvent(EVENT_CHAMBERLAIN_CHORUS_REPULSION, 1500);
        scriptEvents.addEvent(EVENT_CHAMBERLAIN_CHORUS_ONSLAUGHT, 4000);
        scriptEvents.addEvent(EVENT_CHAMBERLAIN_CHORUS_RITUAL, 8000);
    }

    uint32_t mChorusCount = 0;
    bool mSinging = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Halkias, the Sin-Stained Goliath

class HallsOfAtonementHalkiasAI : public HallsOfAtonementBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfAtonementHalkiasAI(c); }
    explicit HallsOfAtonementHalkiasAI(Creature* pCreature) : HallsOfAtonementBossAI(pCreature, DATA_HALKIAS) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_HALKIAS_CRUMBLING_SLAM, 5000);
        scriptEvents.addEvent(EVENT_HALKIAS_HEAVE_DEBRIS, 10000);
        scriptEvents.addEvent(EVENT_HALKIAS_SINLIGHT_VISIONS, 16000);
        scriptEvents.addEvent(EVENT_HALKIAS_REFRACTED_SINLIGHT, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HALKIAS_CRUMBLING_SLAM:
                castSpellOnVictim(SPELL_HALKIAS_CRUMBLING_SLAM);
                scriptEvents.addEvent(EVENT_HALKIAS_CRUMBLING_SLAM, 12000);
                break;
            case EVENT_HALKIAS_HEAVE_DEBRIS:
                castAtRandom(SPELL_HALKIAS_HEAVE_DEBRIS, true);
                scriptEvents.addEvent(EVENT_HALKIAS_HEAVE_DEBRIS, 15000);
                break;
            case EVENT_HALKIAS_SINLIGHT_VISIONS:
                castAtRandom(SPELL_HALKIAS_SINLIGHT_VISIONS, true);
                scriptEvents.addEvent(EVENT_HALKIAS_SINLIGHT_VISIONS, 20000);
                break;
            case EVENT_HALKIAS_REFRACTED_SINLIGHT:
                warn("Halkias focuses the light into beams of Refracted Sinlight!");
                castSpellAOE(SPELL_HALKIAS_REFRACTED_SINLIGHT);
                scriptEvents.addEvent(EVENT_HALKIAS_REFRACTED_SINLIGHT, 45000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// High Adjudicator Aleez

class HallsOfAtonementHighAdjudicatorAleezAI : public HallsOfAtonementBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfAtonementHighAdjudicatorAleezAI(c); }
    explicit HallsOfAtonementHighAdjudicatorAleezAI(Creature* pCreature) : HallsOfAtonementBossAI(pCreature, DATA_HIGH_ADJUDICATOR_ALEEZ) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ALEEZ_ANIMA_BOLT, 3000);
        scriptEvents.addEvent(EVENT_ALEEZ_ANIMA_FOUNTAIN, 12000);
        scriptEvents.addEvent(EVENT_ALEEZ_SPECTRAL_PROCESSION, 18000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ALEEZ_ANIMA_BOLT:
                castSpellOnVictim(SPELL_ALEEZ_ANIMA_BOLT);
                scriptEvents.addEvent(EVENT_ALEEZ_ANIMA_BOLT, 5000);
                break;
            case EVENT_ALEEZ_ANIMA_FOUNTAIN:
                castSpellAOE(SPELL_ALEEZ_ANIMA_FOUNTAIN);
                scriptEvents.addEvent(EVENT_ALEEZ_ANIMA_FOUNTAIN, 20000);
                break;
            case EVENT_ALEEZ_SPECTRAL_PROCESSION:
                warn("High Adjudicator Aleez calls forth a Ghastly Parishioner!");
                callAdd(NPC_ALEEZ_GHASTLY_PARISHIONER, aroundSelf(6.0f, 10.0f), TIMED_DESPAWN, ParishionerLifeMs);
                scriptEvents.addEvent(EVENT_ALEEZ_SPECTRAL_PROCESSION, 30000);
                break;
            default:
                break;
        }
    }
};

// immune to harm, haunts a player and pulses until it fades
class HallsOfAtonementGhastlyParishionerAI : public HallsOfAtonementCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HallsOfAtonementGhastlyParishionerAI(c); }
    explicit HallsOfAtonementGhastlyParishionerAI(Creature* pCreature) : HallsOfAtonementCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr)
            *damage = 0;
    }

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_PARISHIONER_EPHEMERAL_VISAGE, true);
        castSpellOnSelf(SPELL_PARISHIONER_PULSE_FROM_BEYOND, true);
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_PARISHIONER_HAUNT, 500);
    }

    void onDisengage() override { _setMeleeDisabled(false); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_PARISHIONER_HAUNT)
            return;

        if (Unit* pTarget = randomPlayer())
        {
            castSpell(pTarget, SPELL_PARISHIONER_HAUNTING_FIXATION, true);
            focusOn(pTarget);
        }
        scriptEvents.addEvent(EVENT_PARISHIONER_HAUNT, 12000);
    }
};

void SetupHallsOfAtonement(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HALLS_OF_ATONEMENT, &HallsOfAtonementInstanceScript::Create);

    // Echelon
    mgr->register_creature_script(BOSS_ECHELON, &HallsOfAtonementEchelonAI::Create);
    mgr->register_creature_script(NPC_ECHELON_UNDYING_STONEFIEND, &HallsOfAtonementUndyingStonefiendAI::Create);

    // Lord Chamberlain
    mgr->register_creature_script(BOSS_LORD_CHAMBERLAIN, &HallsOfAtonementLordChamberlainAI::Create);

    // Halkias, the Sin-Stained Goliath
    mgr->register_creature_script(BOSS_HALKIAS, &HallsOfAtonementHalkiasAI::Create);

    // High Adjudicator Aleez
    mgr->register_creature_script(BOSS_HIGH_ADJUDICATOR_ALEEZ, &HallsOfAtonementHighAdjudicatorAleezAI::Create);
    mgr->register_creature_script(NPC_ALEEZ_GHASTLY_PARISHIONER, &HallsOfAtonementGhastlyParishionerAI::Create);
}
