/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Algethar_Academy.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <vector>

enum AlgetharAcademyEvents
{
    EVENT_VEXAMUS_ARCANE_EXPULSION = 1,
    EVENT_VEXAMUS_MANA_BOMBS,

    EVENT_ANCIENT_BARKBREAKER,
    EVENT_ANCIENT_GERMINATE,
    EVENT_ANCIENT_BRANCH_OUT,

    EVENT_BRANCH_HEALING_TOUCH,

    EVENT_CRAWTH_SAVAGE_PECK,
    EVENT_CRAWTH_DEAFENING_SCREECH,
    EVENT_CRAWTH_OVERPOWERING_GUST,
    EVENT_CRAWTH_RUINOUS_WINDS,

    EVENT_DORAGOSA_ASTRAL_BLAST,
    EVENT_DORAGOSA_ARCANE_MISSILES,
    EVENT_DORAGOSA_ENERGY_BOMB,
    EVENT_DORAGOSA_POWER_VACUUM,
    EVENT_DORAGOSA_ARCANE_RIFT
};

namespace
{
    // energy bars, time from empty to full
    uint32_t const VexamusFissureMs = 60000;
    uint32_t const OvergrownBurstMs = 40000;

    // an arcane orb reaching Vexamus fills part of his bar
    uint32_t const VexamusOrbArrivalMs = 10000;
    uint32_t const VexamusOrbBonusMs = 8000;

    // the lesson starts once a player stands this close to the professor
    float const MaxdormuLessonRange = 30.0f;
    float const VexamusLessonDistance = 12.0f;
    uint32_t const HostileMonsterFaction = 16;

    uint8_t const LashersPerGermination = 3;

    // health marks for Play Ball!
    int32_t const CrawthPlayBallPct[] = { 75, 45 };
    uint8_t const CrawthPlayBallCount = 2;

    // applications of Overwhelming Power that tear open a rift
    uint32_t const DoragosaPowerPerRift = 3;
    uint32_t const ArcaneRiftLifetimeMs = 60000;

    LocationVector algetharRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(6.2831853f);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    bool algetharPlayerNear(Creature* pCreature, float range)
    {
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pCreature->getDistance(pPlayer) <= range)
                return true;
        }

        return false;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class AlgetharAcademyInstanceScript : public InstanceScript
{
public:
    explicit AlgetharAcademyInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(AlgetharAcademyEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new AlgetharAcademyInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy stand-in and target helpers

class AlgetharAcademyCombatAI : public CreatureAIScript
{
public:
    explicit AlgetharAcademyCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnergyMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mEnergyMs = 0;

        if (isAlive())
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

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onReset() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    AlgetharAcademyInstanceScript* getAcademy()
    {
        if (getCreature()->GetMapId() != MAP_ALGETHAR_ACADEMY)
            return nullptr;

        return static_cast<AlgetharAcademyInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    Unit* castAtPlayer(uint32_t spellId, bool triggered = false)
    {
        Unit* pTarget = anyPlayer();
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    // normal difficulty has its own spell versions
    uint32_t byDifficulty(uint32_t heroicSpellId, uint32_t normalSpellId) { return isHeroic() ? heroicSpellId : normalSpellId; }

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return algetharRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // energy stand-in: true once the bar is full and nothing is being cast
    bool energyFull(uint32_t diff, uint32_t fullAfterMs)
    {
        if (mEnergyMs < fullAfterMs)
            mEnergyMs += diff;

        if (mEnergyMs < fullAfterMs || _isCasting())
            return false;

        mEnergyMs = 0;
        return true;
    }

    uint32_t mEnergyMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class AlgetharAcademyBossAI : public AlgetharAcademyCombatAI
{
public:
    AlgetharAcademyBossAI(Creature* pCreature, uint32_t dataId) : AlgetharAcademyCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (AlgetharAcademyInstanceScript* pAcademy = getAcademy())
        {
            pAcademy->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pAcademy->setBossState(mDataId, EncounterStates::InProgress);
        }

        AlgetharAcademyCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (AlgetharAcademyInstanceScript* pAcademy = getAcademy())
            {
                pAcademy->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pAcademy->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        AlgetharAcademyCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (AlgetharAcademyInstanceScript* pAcademy = getAcademy())
        {
            pAcademy->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pAcademy->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vexamus

class AlgetharAcademyVexamusAI : public AlgetharAcademyBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AlgetharAcademyVexamusAI(c); }
    explicit AlgetharAcademyVexamusAI(Creature* pCreature) : AlgetharAcademyBossAI(pCreature, DATA_VEXAMUS) {}

protected:
    void onEngage() override
    {
        mOrbClockMs = 0;
        scriptEvents.addEvent(EVENT_VEXAMUS_ARCANE_EXPULSION, 10000);
        scriptEvents.addEvent(EVENT_VEXAMUS_MANA_BOMBS, 15000);
    }

    void onCombatTick(uint32_t diff) override
    {
        // the professor's orbs feed the bar on top of its own growth
        mOrbClockMs += diff;
        if (mOrbClockMs >= VexamusOrbArrivalMs)
        {
            mOrbClockMs = 0;
            mEnergyMs += VexamusOrbBonusMs;
        }

        if (!energyFull(diff, VexamusFissureMs))
            return;

        warn("Vexamus tears open an Arcane Fissure!");
        castSpellAOE(byDifficulty(SPELL_VEXAMUS_ARCANE_FISSURE, SPELL_VEXAMUS_ARCANE_FISSURE_NORMAL));
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VEXAMUS_ARCANE_EXPULSION:
                castSpellOnVictim(SPELL_VEXAMUS_ARCANE_EXPULSION);
                scriptEvents.addEvent(EVENT_VEXAMUS_ARCANE_EXPULSION, 20000);
                break;
            case EVENT_VEXAMUS_MANA_BOMBS:
                castAtPlayer(SPELL_VEXAMUS_MANA_BOMBS);
                scriptEvents.addEvent(EVENT_VEXAMUS_MANA_BOMBS, 25000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mOrbClockMs = 0;
};

// starts the lesson: Vexamus takes shape in front of the professor
class AlgetharAcademyMaxdormuAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AlgetharAcademyMaxdormuAI(c); }
    explicit AlgetharAcademyMaxdormuAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (mLessonGiven)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mCheckClockMs > diff)
        {
            mCheckClockMs -= diff;
            return;
        }

        mCheckClockMs = 2000;
        if (getCreature()->GetMapId() != MAP_ALGETHAR_ACADEMY)
        {
            mLessonGiven = true;
            return;
        }

        InstanceScript* pAcademy = getInstanceScript();
        if (pAcademy == nullptr)
            return;

        // only once and never next to a placed Vexamus
        if (pAcademy->getBossState(DATA_VEXAMUS) == EncounterStates::Performed || !pAcademy->getCreatureSetForEntry(BOSS_VEXAMUS).empty())
        {
            mLessonGiven = true;
            return;
        }

        if (!algetharPlayerNear(getCreature(), MaxdormuLessonRange))
            return;

        mLessonGiven = true;

        Creature* pSelf = getCreature();
        float const facing = pSelf->GetOrientation();
        LocationVector const spot(pSelf->GetPositionX() + VexamusLessonDistance * std::cos(facing), pSelf->GetPositionY() + VexamusLessonDistance * std::sin(facing), pSelf->GetPositionZ(), facing + 3.14159265f);
        spawnCreature(BOSS_VEXAMUS, spot, HostileMonsterFaction, pSelf->GetPhase());
    }

private:
    bool mLessonGiven = false;
    uint32_t mCheckClockMs = 2000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Overgrown Ancient

class AlgetharAcademyOvergrownAncientAI : public AlgetharAcademyBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AlgetharAcademyOvergrownAncientAI(c); }
    explicit AlgetharAcademyOvergrownAncientAI(Creature* pCreature) : AlgetharAcademyBossAI(pCreature, DATA_OVERGROWN_ANCIENT) {}

protected:
    void onEngage() override
    {
        mDormantGuids.clear();

        scriptEvents.addEvent(EVENT_ANCIENT_BARKBREAKER, 8000);
        scriptEvents.addEvent(EVENT_ANCIENT_GERMINATE, 12000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_ANCIENT_BRANCH_OUT, 20000);
    }

    void onReset() override { mDormantGuids.clear(); }

    void onCombatTick(uint32_t diff) override
    {
        if (!energyFull(diff, OvergrownBurstMs))
            return;

        warn("The Overgrown Ancient bursts forth, awakening the lashers!");
        castSpellAOE(SPELL_ANCIENT_BURST_FORTH);
        awakenLashers();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ANCIENT_BARKBREAKER:
                castSpellOnVictim(SPELL_ANCIENT_BARKBREAKER);
                scriptEvents.addEvent(EVENT_ANCIENT_BARKBREAKER, 16000);
                break;
            case EVENT_ANCIENT_GERMINATE:
                germinate();
                scriptEvents.addEvent(EVENT_ANCIENT_GERMINATE, 30000);
                break;
            case EVENT_ANCIENT_BRANCH_OUT:
                castSpellOnSelf(SPELL_ANCIENT_BRANCH_OUT);
                callAdd(NPC_ANCIENT_BRANCH, aroundSelf(5.0f, 10.0f));
                scriptEvents.addEvent(EVENT_ANCIENT_BRANCH_OUT, 35000);
                break;
            default:
                break;
        }
    }

private:
    // dormant lashers sprout around the arena
    void germinate()
    {
        castSpellOnSelf(SPELL_ANCIENT_GERMINATE);

        for (uint8_t i = 0; i < LashersPerGermination; ++i)
        {
            Creature* pLasher = summonCreature(NPC_HUNGRY_LASHER, aroundSelf(8.0f, 20.0f), CORPSE_DESPAWN);
            if (pLasher == nullptr || pLasher->GetScript() == nullptr)
                continue;

            pLasher->GetScript()->DoAction(ACTION_LASHER_DORMANT);
            mDormantGuids.push_back(pLasher->GetNewGUID());
        }
    }

    void awakenLashers()
    {
        AlgetharAcademyInstanceScript* pAcademy = getAcademy();
        if (pAcademy != nullptr)
        {
            for (WoWGuid const& guid : mDormantGuids)
            {
                Creature* pLasher = pAcademy->getCreatureByGuid(guid);
                if (pLasher != nullptr && pLasher->isAlive() && pLasher->GetScript() != nullptr)
                    pLasher->GetScript()->DoAction(ACTION_LASHER_AWAKEN);
            }
        }

        mDormantGuids.clear();
    }

    std::vector<WoWGuid> mDormantGuids;
};

// placed lashers fight as usual, the sprouted ones wait for Burst Forth
class AlgetharAcademyHungryLasherAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AlgetharAcademyHungryLasherAI(c); }
    explicit AlgetharAcademyHungryLasherAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_LASHER_DORMANT)
        {
            setReactState(REACT_PASSIVE);
            setRooted(true);
            _setMeleeDisabled(true);
        }
        else if (action == ACTION_LASHER_AWAKEN)
        {
            setReactState(REACT_AGGRESSIVE);
            setRooted(false);
            _setMeleeDisabled(false);
            setZoneWideCombat();
        }
    }
};

class AlgetharAcademyAncientBranchAI : public AlgetharAcademyCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AlgetharAcademyAncientBranchAI(c); }
    explicit AlgetharAcademyAncientBranchAI(Creature* pCreature) : AlgetharAcademyCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_BRANCH_HEALING_TOUCH, 5000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_BRANCH_HEALING_TOUCH)
            return;

        Creature* pAncient = findNearestCreature(BOSS_OVERGROWN_ANCIENT, 80.0f);
        if (pAncient != nullptr && pAncient->isAlive())
            castSpell(pAncient, SPELL_BRANCH_HEALING_TOUCH);

        scriptEvents.addEvent(EVENT_BRANCH_HEALING_TOUCH, 12000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Crawth

class AlgetharAcademyCrawthAI : public AlgetharAcademyBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AlgetharAcademyCrawthAI(c); }
    explicit AlgetharAcademyCrawthAI(Creature* pCreature) : AlgetharAcademyBossAI(pCreature, DATA_CRAWTH) {}

protected:
    void onEngage() override
    {
        mPlayBalls = 0;

        scriptEvents.addEvent(EVENT_CRAWTH_SAVAGE_PECK, 5000);
        scriptEvents.addEvent(EVENT_CRAWTH_DEAFENING_SCREECH, 10000);
        scriptEvents.addEvent(EVENT_CRAWTH_OVERPOWERING_GUST, 14000);
        scriptEvents.addEvent(EVENT_CRAWTH_RUINOUS_WINDS, 25000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mPlayBalls >= CrawthPlayBallCount || _isCasting() || _getHealthPercent() > CrawthPlayBallPct[mPlayBalls])
            return;

        ++mPlayBalls;
        warn("Crawth calls for Play Ball! The goals of the arena awaken!");
        castSpellOnSelf(SPELL_CRAWTH_PLAY_BALL);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CRAWTH_SAVAGE_PECK:
                castSpellOnVictim(SPELL_CRAWTH_SAVAGE_PECK);
                scriptEvents.addEvent(EVENT_CRAWTH_SAVAGE_PECK, 12000);
                break;
            case EVENT_CRAWTH_DEAFENING_SCREECH:
                castSpellAOE(byDifficulty(SPELL_CRAWTH_DEAFENING_SCREECH, SPELL_CRAWTH_DEAFENING_SCREECH_NORMAL));
                scriptEvents.addEvent(EVENT_CRAWTH_DEAFENING_SCREECH, 22000);
                break;
            case EVENT_CRAWTH_OVERPOWERING_GUST:
                castSpellOnVictim(SPELL_CRAWTH_OVERPOWERING_GUST);
                scriptEvents.addEvent(EVENT_CRAWTH_OVERPOWERING_GUST, 28000);
                break;
            case EVENT_CRAWTH_RUINOUS_WINDS:
                castSpellOnSelf(SPELL_CRAWTH_RUINOUS_WINDS);
                scriptEvents.addEvent(EVENT_CRAWTH_RUINOUS_WINDS, 40000);
                break;
            default:
                break;
        }
    }

private:
    uint8_t mPlayBalls = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Echo of Doragosa

class AlgetharAcademyEchoOfDoragosaAI : public AlgetharAcademyBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AlgetharAcademyEchoOfDoragosaAI(c); }
    explicit AlgetharAcademyEchoOfDoragosaAI(Creature* pCreature) : AlgetharAcademyBossAI(pCreature, DATA_ECHO_OF_DORAGOSA) {}

protected:
    void onEngage() override
    {
        mPowerApplied = 0;

        scriptEvents.addEvent(EVENT_DORAGOSA_ARCANE_MISSILES, 4000);
        scriptEvents.addEvent(EVENT_DORAGOSA_ASTRAL_BLAST, 6000);
        scriptEvents.addEvent(EVENT_DORAGOSA_ENERGY_BOMB, 14000);
        scriptEvents.addEvent(EVENT_DORAGOSA_POWER_VACUUM, 20000);

        // normal: rifts open on their own
        if (!isHeroic())
            scriptEvents.addEvent(EVENT_DORAGOSA_ARCANE_RIFT, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DORAGOSA_ASTRAL_BLAST:
            {
                Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
                castSpellOnVictim(SPELL_DORAGOSA_ASTRAL_BLAST);
                empower(pVictim);
                scriptEvents.addEvent(EVENT_DORAGOSA_ASTRAL_BLAST, 12000);
                break;
            }
            case EVENT_DORAGOSA_ARCANE_MISSILES:
                empower(castAtPlayer(byDifficulty(SPELL_DORAGOSA_ARCANE_MISSILES, SPELL_DORAGOSA_ARCANE_MISSILES_NORMAL)));
                scriptEvents.addEvent(EVENT_DORAGOSA_ARCANE_MISSILES, 10000);
                break;
            case EVENT_DORAGOSA_ENERGY_BOMB:
                castAtPlayer(byDifficulty(SPELL_DORAGOSA_ENERGY_BOMB, SPELL_DORAGOSA_ENERGY_BOMB_NORMAL));
                scriptEvents.addEvent(EVENT_DORAGOSA_ENERGY_BOMB, 18000);
                break;
            case EVENT_DORAGOSA_POWER_VACUUM:
                warn("The Echo of Doragosa draws everyone in with a Power Vacuum!");
                castSpellAOE(byDifficulty(SPELL_DORAGOSA_POWER_VACUUM, SPELL_DORAGOSA_POWER_VACUUM_NORMAL));
                scriptEvents.addEvent(EVENT_DORAGOSA_POWER_VACUUM, 24000);
                break;
            case EVENT_DORAGOSA_ARCANE_RIFT:
                openRift(anyPlayer());
                scriptEvents.addEvent(EVENT_DORAGOSA_ARCANE_RIFT, 30000);
                break;
            default:
                break;
        }
    }

private:
    // heroic: her hits stack Overwhelming Power, enough of it tears open a rift
    void empower(Unit* pTarget)
    {
        if (pTarget == nullptr || !isHeroic())
            return;

        castSpell(pTarget, SPELL_DORAGOSA_OVERWHELMING_POWER, true);
        if (++mPowerApplied < DoragosaPowerPerRift)
            return;

        mPowerApplied = 0;
        openRift(pTarget);
    }

    void openRift(Unit* pTarget)
    {
        if (pTarget != nullptr)
            summonCreature(NPC_ARCANE_RIFT, algetharRingPoint(pTarget->GetPosition(), 0.0f, 2.0f), TIMED_DESPAWN, ArcaneRiftLifetimeMs);
    }

    uint32_t mPowerApplied = 0;
};

// invisible rift, pulses wild energy until it closes
class AlgetharAcademyArcaneRiftAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AlgetharAcademyArcaneRiftAI(c); }
    explicit AlgetharAcademyArcaneRiftAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mPulseClockMs > diff)
        {
            mPulseClockMs -= diff;
            return;
        }

        mPulseClockMs = 4000;
        castSpellAOE(SPELL_ARCANE_RIFT_UNCONTROLLED_ENERGY, true);
    }

private:
    uint32_t mPulseClockMs = 2000;
};

void SetupAlgetharAcademy(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ALGETHAR_ACADEMY, &AlgetharAcademyInstanceScript::Create);

    mgr->register_creature_script(BOSS_VEXAMUS, &AlgetharAcademyVexamusAI::Create);
    mgr->register_creature_script(NPC_PROFESSOR_MAXDORMU, &AlgetharAcademyMaxdormuAI::Create);

    mgr->register_creature_script(BOSS_OVERGROWN_ANCIENT, &AlgetharAcademyOvergrownAncientAI::Create);
    mgr->register_creature_script(NPC_ANCIENT_BRANCH, &AlgetharAcademyAncientBranchAI::Create);
    mgr->register_creature_script(NPC_HUNGRY_LASHER, &AlgetharAcademyHungryLasherAI::Create);

    mgr->register_creature_script(BOSS_CRAWTH, &AlgetharAcademyCrawthAI::Create);

    mgr->register_creature_script(BOSS_ECHO_OF_DORAGOSA, &AlgetharAcademyEchoOfDoragosaAI::Create);
    mgr->register_creature_script(NPC_ARCANE_RIFT, &AlgetharAcademyArcaneRiftAI::Create);
}
