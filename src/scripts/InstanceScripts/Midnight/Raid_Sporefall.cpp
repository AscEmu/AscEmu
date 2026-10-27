/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Sporefall.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <list>
#include <utility>
#include <vector>

enum SporefallEvents
{
    // Rotmire
    EVENT_SPOREFALL_AWAKEN_FUNGI = 1,
    EVENT_SPOREFALL_PUTRID_FIST,
    EVENT_SPOREFALL_FESTERING_VINES,
    EVENT_SPOREFALL_BURSTING_PUSTULES,

    // Sporecap
    EVENT_SPOREFALL_SPORECAP_BLIGHTSHOT,
    EVENT_SPOREFALL_SPORECAP_POISON_BURST
};

namespace
{
    float const SporefallTwoPi = 6.28318531f;
    float const SporefallPlayerReach = 100.0f;

    uint32_t const SporefallBerserkMs = 480000;

    // Rotmire blooms every 50 seconds
    uint32_t const RotmireEnergyFull = 100;
    uint32_t const RotmireEnergyPerSecond = 2;

    // fallen minions that sprout a shroom at the next bloom
    size_t const RotmireShroomSpotLimit = 6;

    uint32_t const RotmireMinions[3] = { NPC_ROTMIRE_SHROOMLING, NPC_ROTMIRE_FUNGLING, NPC_ROTMIRE_SPORECAP };

    LocationVector sporefallRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(SporefallTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class SporefallInstanceScript : public InstanceScript
{
public:
    explicit SporefallInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(SporefallEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new SporefallInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, berserk and target helpers

class SporefallCombatAI : public CreatureAIScript
{
public:
    explicit SporefallCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mBerserkLeftMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mBerserkLeftMs = 0;

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
        tickBerserk(diff);
        onCombatTick(diff);

        // due events run one after another unless a cast holds them
        while (!_isCasting())
        {
            uint32_t const eventId = scriptEvents.getFinishedEvent();
            if (eventId == 0)
                break;

            onEvent(eventId);
        }
    }

protected:
    virtual void onEngage() {}
    virtual void onReset() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    // distinct random players, the current target only as last resort
    std::vector<Unit*> rollPlayers(size_t count, bool spareTank)
    {
        TargetFilter const filter = spareTank ? TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent) : TargetFilter_Player;
        FilterArgs const args(filter);

        std::vector<Unit*> pool;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || getCreature()->getDistance(pObject) > SporefallPlayerReach)
                continue;

            if (isValidUnitTarget(pObject, filter, args))
                pool.push_back(static_cast<Unit*>(pObject));
        }

        if (pool.empty() && spareTank)
            return rollPlayers(count, false);

        for (size_t i = 0; i < pool.size() && i < count; ++i)
            std::swap(pool[i], pool[i + Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1 - i))]);

        if (pool.size() > count)
            pool.resize(count);

        return pool;
    }

    Unit* rollPlayer(bool spareTank)
    {
        std::vector<Unit*> const picked = rollPlayers(1, spareTank);
        return picked.empty() ? nullptr : picked.front();
    }

    void castOnPlayers(uint32_t spellId, size_t count, bool spareTank)
    {
        for (Unit* pTarget : rollPlayers(count, spareTank))
            castSpell(pTarget, spellId, true);
    }

    Unit* castOnPlayer(uint32_t spellId, bool spareTank)
    {
        Unit* pTarget = rollPlayer(spareTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId);

        return pTarget;
    }

    // summoned helper that joins the fight at once
    Creature* bringAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector ringAroundSelf(float minRange, float maxRange)
    {
        return sporefallRingPoint(getCreature()->GetPosition(), minRange, maxRange);
    }

    void announce(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void startBerserkClock(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

private:
    void tickBerserk(uint32_t diff)
    {
        if (mBerserkLeftMs == 0)
            return;

        if (mBerserkLeftMs > diff)
        {
            mBerserkLeftMs -= diff;
            return;
        }

        mBerserkLeftMs = 0;
        castSpellOnSelf(SPELL_SPOREFALL_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rotmire

class SporefallRotmireAI : public SporefallCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SporefallRotmireAI(c); }
    explicit SporefallRotmireAI(Creature* pCreature) : SporefallCombatAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(DATA_ROTMIRE, EncounterStates::InProgress);
        }

        SporefallCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(DATA_ROTMIRE, EncounterStates::Failed);
            }
        }

        SporefallCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(DATA_ROTMIRE, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    // a fallen minion leaves a spot for the next bloom
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon != nullptr && mShroomSpots.size() < RotmireShroomSpotLimit)
            mShroomSpots.push_back(pSummon->GetPosition());
    }

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClockMs = 0;
        mFungiWaves = 0;
        mShroomSpots.clear();

        startBerserkClock(SporefallBerserkMs);
        scriptEvents.addEvent(EVENT_SPOREFALL_PUTRID_FIST, 6000);
        scriptEvents.addEvent(EVENT_SPOREFALL_AWAKEN_FUNGI, 8000);
        scriptEvents.addEvent(EVENT_SPOREFALL_FESTERING_VINES, 16000);
        scriptEvents.addEvent(EVENT_SPOREFALL_BURSTING_PUSTULES, 22000);
    }

    void onReset() override { mShroomSpots.clear(); }

    void onCombatTick(uint32_t diff) override
    {
        mEnergyClockMs += diff;
        while (mEnergyClockMs >= 1000)
        {
            mEnergyClockMs -= 1000;
            mEnergy += RotmireEnergyPerSecond;
        }

        if (mEnergy < RotmireEnergyFull || _isCasting())
            return;

        mEnergy = 0;
        fungalBloom();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SPOREFALL_PUTRID_FIST:
                castSpellOnVictim(SPELL_ROTMIRE_PUTRID_FIST);
                scriptEvents.addEvent(EVENT_SPOREFALL_PUTRID_FIST, 14000);
                break;
            case EVENT_SPOREFALL_AWAKEN_FUNGI:
                awakenFungi();
                scriptEvents.addEvent(EVENT_SPOREFALL_AWAKEN_FUNGI, 35000);
                break;
            case EVENT_SPOREFALL_FESTERING_VINES:
                castOnPlayers(SPELL_ROTMIRE_FESTERING_VINES, 3, true);
                scriptEvents.addEvent(EVENT_SPOREFALL_FESTERING_VINES, 30000);
                break;
            case EVENT_SPOREFALL_BURSTING_PUSTULES:
                castSpellAOE(SPELL_ROTMIRE_BURSTING_PUSTULES);
                scriptEvents.addEvent(EVENT_SPOREFALL_BURSTING_PUSTULES, 32000);
                break;
            default:
                break;
        }
    }

private:
    // two pursuers each wave, a Sporecap every second wave
    void awakenFungi()
    {
        castSpellAOE(SPELL_ROTMIRE_AWAKEN_FUNGI);
        ++mFungiWaves;

        bringAdd(NPC_ROTMIRE_SHROOMLING, ringAroundSelf(8.0f, 18.0f));
        bringAdd(NPC_ROTMIRE_FUNGLING, ringAroundSelf(8.0f, 18.0f));
        if (mFungiWaves % 2 == 0)
            bringAdd(NPC_ROTMIRE_SPORECAP, ringAroundSelf(10.0f, 20.0f));
    }

    void fungalBloom()
    {
        announce("Rotmire unleashes a Fungal Bloom!");
        castSpellAOE(SPELL_ROTMIRE_FUNGAL_BLOOM);

        // shrooms sprout from the corpses
        for (LocationVector const& spot : mShroomSpots)
            getCreature()->castSpellLoc(spot, SPELL_ROTMIRE_BURSTING_SHROOM, true);
        mShroomSpots.clear();

        // living minions are driven into a frenzy
        for (uint32_t entry : RotmireMinions)
        {
            std::list<Creature*> minions;
            GetCreatureListWithEntryInGrid(minions, entry, SporefallPlayerReach);
            for (Creature* pMinion : minions)
            {
                if (pMinion->isAlive() && pMinion->isInCombat())
                    castSpell(pMinion, SPELL_ROTMIRE_FUNGAL_FRENZY, true);
            }
        }
    }

    uint32_t mEnergy = 0;
    uint32_t mEnergyClockMs = 0;
    uint32_t mFungiWaves = 0;
    std::vector<LocationVector> mShroomSpots;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shroomling and Fungling: chase one player relentlessly

class SporefallPursuerAI : public SporefallCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SporefallPursuerAI(c); }
    explicit SporefallPursuerAI(Creature* pCreature) : SporefallCombatAI(pCreature) {}

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        if (_isInCombat())
            fixate();
    }

protected:
    void onEngage() override { fixate(); }

private:
    void fixate()
    {
        if (Unit* pPrey = rollPlayer(false))
            addThreat(pPrey, 1000000.0f);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sporecap

class SporefallSporecapAI : public SporefallCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SporefallSporecapAI(c); }
    explicit SporefallSporecapAI(Creature* pCreature) : SporefallCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_SPOREFALL_SPORECAP_BLIGHTSHOT, 3000);
        scriptEvents.addEvent(EVENT_SPOREFALL_SPORECAP_POISON_BURST, 7000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SPOREFALL_SPORECAP_BLIGHTSHOT:
                castOnPlayer(SPELL_SPORECAP_BLIGHTSHOT, false);
                scriptEvents.addEvent(EVENT_SPOREFALL_SPORECAP_BLIGHTSHOT, 8000);
                break;
            case EVENT_SPOREFALL_SPORECAP_POISON_BURST:
                castSpellAOE(SPELL_SPORECAP_POISON_BURST);
                scriptEvents.addEvent(EVENT_SPOREFALL_SPORECAP_POISON_BURST, 15000);
                break;
            default:
                break;
        }
    }
};

void SetupSporefall(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SPOREFALL, &SporefallInstanceScript::Create);

    mgr->register_creature_script(BOSS_ROTMIRE, &SporefallRotmireAI::Create);
    mgr->register_creature_script(NPC_ROTMIRE_SHROOMLING, &SporefallPursuerAI::Create);
    mgr->register_creature_script(NPC_ROTMIRE_FUNGLING, &SporefallPursuerAI::Create);
    mgr->register_creature_script(NPC_ROTMIRE_SPORECAP, &SporefallSporecapAI::Create);
}
