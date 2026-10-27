/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_The_Unbinding_Of_Kithix.hpp"

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

enum UnbindingOfKithixEvents
{
    // Kith'ix
    EVENT_KITHIX_DARK_DEVASTATION = 1,
    EVENT_KITHIX_VOIDSWARM,
    EVENT_KITHIX_TWISTED_APPENDAGE,
    EVENT_KITHIX_UNSPEAKABLE_HORRORS,
    EVENT_KITHIX_EXTINGUISH,
    EVENT_KITHIX_STYGIAN_HOWL,
    EVENT_KITHIX_NULL_GATE,
    EVENT_KITHIX_NEGATION,

    // Voidswarm
    EVENT_KITHIX_HULK_VENOM_ROAR,
    EVENT_KITHIX_VOIDWEAVER_VOIDWEAVE,
    EVENT_KITHIX_VOIDWEAVER_NIGHTFALL
};

namespace
{
    float const KithixTwoPi = 6.28318531f;
    float const KithixPlayerReach = 100.0f;

    uint32_t const KithixBerserkMs = 540000;

    // Suffocating Darkness every 50 seconds in stage one
    uint32_t const KithixEnergyFull = 100;
    uint32_t const KithixEnergyPerSecond = 2;

    int32_t const KithixLastLightPct = 35;

    float const BeetleBurstRange = 4.0f;

    struct KithixSwarmPart
    {
        uint32_t entry;
        uint32_t count;
    };

    KithixSwarmPart const KithixSwarm[3] =
    {
        { NPC_KITHIX_VENOMOUS_HULK_PLACEHOLDER, 1 },
        { NPC_KITHIX_VOIDWEAVER_PLACEHOLDER, 2 },
        { NPC_KITHIX_BOMBER_BEETLE_PLACEHOLDER, 2 }
    };

    LocationVector kithixRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(KithixTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // placeholder entries stay unregistered
    void registerKithixCreature(ScriptMgr* mgr, uint32_t entry, exp_create_creature_ai creator)
    {
        if (entry != 0)
            mgr->register_creature_script(entry, creator);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class UnbindingOfKithixInstanceScript : public InstanceScript
{
public:
    explicit UnbindingOfKithixInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(UnbindingOfKithixEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new UnbindingOfKithixInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, berserk and target helpers

class KithixCombatAI : public CreatureAIScript
{
public:
    explicit KithixCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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
            if (pObject == nullptr || getCreature()->getDistance(pObject) > KithixPlayerReach)
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

    // a placeholder entry summons nothing
    Creature* bringAdd(uint32_t entry, LocationVector const& spot)
    {
        if (entry == 0)
            return nullptr;

        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector ringAroundSelf(float minRange, float maxRange)
    {
        return kithixRingPoint(getCreature()->GetPosition(), minRange, maxRange);
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
        castSpellOnSelf(SPELL_KITHIX_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kith'ix

class UnbindingKithixAI : public KithixCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UnbindingKithixAI(c); }
    explicit UnbindingKithixAI(Creature* pCreature) : KithixCombatAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(DATA_KITHIX, EncounterStates::InProgress);
        }

        KithixCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(DATA_KITHIX, EncounterStates::Failed);
            }
        }

        KithixCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(DATA_KITHIX, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    void onEngage() override
    {
        mLastLight = false;
        mEnergy = 0;
        mEnergyClockMs = 0;

        startBerserkClock(KithixBerserkMs);
        castSpellOnSelf(SPELL_KITHIX_COMMANDING_PRESENCE, true);

        scriptEvents.addEvent(EVENT_KITHIX_VOIDSWARM, 5000);
        scriptEvents.addEvent(EVENT_KITHIX_DARK_DEVASTATION, 10000);
        scriptEvents.addEvent(EVENT_KITHIX_TWISTED_APPENDAGE, 14000);
        scriptEvents.addEvent(EVENT_KITHIX_UNSPEAKABLE_HORRORS, 21000);
        scriptEvents.addEvent(EVENT_KITHIX_EXTINGUISH, 38000);
    }

    void onReset() override { _removeAura(SPELL_KITHIX_COMMANDING_PRESENCE); }

    void onCombatTick(uint32_t diff) override
    {
        if (!mLastLight && _getHealthPercent() <= KithixLastLightPct)
        {
            enterLastLight();
            return;
        }

        // the darkness only gathers in stage one
        if (mLastLight)
            return;

        mEnergyClockMs += diff;
        while (mEnergyClockMs >= 1000)
        {
            mEnergyClockMs -= 1000;
            mEnergy += KithixEnergyPerSecond;
        }

        if (mEnergy < KithixEnergyFull || _isCasting())
            return;

        mEnergy = 0;
        suffocatingDarkness();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KITHIX_DARK_DEVASTATION:
                castSpellOnVictim(SPELL_KITHIX_DARK_DEVASTATION);
                scriptEvents.addEvent(EVENT_KITHIX_DARK_DEVASTATION, 26000);
                break;
            case EVENT_KITHIX_VOIDSWARM:
                callVoidswarm();
                scriptEvents.addEvent(EVENT_KITHIX_VOIDSWARM, 45000);
                break;
            case EVENT_KITHIX_TWISTED_APPENDAGE:
                castOnPlayer(SPELL_KITHIX_TWISTED_APPENDAGE, true);
                scriptEvents.addEvent(EVENT_KITHIX_TWISTED_APPENDAGE, 20000);
                break;
            case EVENT_KITHIX_UNSPEAKABLE_HORRORS:
                castOnPlayers(SPELL_KITHIX_UNSPEAKABLE_HORRORS, 2, true);
                scriptEvents.addEvent(EVENT_KITHIX_UNSPEAKABLE_HORRORS, 34000);
                break;
            case EVENT_KITHIX_EXTINGUISH:
                announce("Kith'ix begins to Extinguish all light!");
                castSpellAOE(SPELL_KITHIX_EXTINGUISH);
                scriptEvents.addEvent(EVENT_KITHIX_EXTINGUISH, 60000);
                break;
            case EVENT_KITHIX_STYGIAN_HOWL:
                castSpellAOE(SPELL_KITHIX_STYGIAN_HOWL);
                scriptEvents.addEvent(EVENT_KITHIX_STYGIAN_HOWL, 30000);
                break;
            case EVENT_KITHIX_NULL_GATE:
                castOnPlayer(SPELL_KITHIX_NULL_GATE, true);
                scriptEvents.addEvent(EVENT_KITHIX_NULL_GATE, 40000);
                break;
            case EVENT_KITHIX_NEGATION:
                castOnPlayer(SPELL_KITHIX_NEGATION, false);
                scriptEvents.addEvent(EVENT_KITHIX_NEGATION, 25000);
                break;
            default:
                break;
        }
    }

private:
    void callVoidswarm()
    {
        castSpellAOE(SPELL_KITHIX_VOIDSWARM);
        for (KithixSwarmPart const& part : KithixSwarm)
        {
            for (uint32_t i = 0; i < part.count; ++i)
                bringAdd(part.entry, ringAroundSelf(10.0f, 22.0f));
        }
    }

    // aqir still standing are empowered by the darkness
    void suffocatingDarkness()
    {
        announce("Kith'ix smothers the chamber in Suffocating Darkness!");
        castSpellAOE(SPELL_KITHIX_SUFFOCATING_DARKNESS);
        castOnPlayers(SPELL_KITHIX_ABYSSAL_GRASP, 2, true);

        for (KithixSwarmPart const& part : KithixSwarm)
        {
            if (part.entry == 0)
                continue;

            std::list<Creature*> aqir;
            GetCreatureListWithEntryInGrid(aqir, part.entry, KithixPlayerReach);
            for (Creature* pAqir : aqir)
            {
                if (pAqir->isAlive() && pAqir->isInCombat())
                    castSpell(pAqir, SPELL_KITHIX_DARK_FRENZY, true);
            }
        }
    }

    // the swarm and the darkness give way to the void gates
    void enterLastLight()
    {
        mLastLight = true;
        announce("Kith'ix lets out a Stygian Howl as the last light flickers!");

        scriptEvents.removeEvent(EVENT_KITHIX_VOIDSWARM);
        scriptEvents.removeEvent(EVENT_KITHIX_TWISTED_APPENDAGE);
        scriptEvents.removeEvent(EVENT_KITHIX_UNSPEAKABLE_HORRORS);

        scriptEvents.addEvent(EVENT_KITHIX_STYGIAN_HOWL, 4000);
        scriptEvents.addEvent(EVENT_KITHIX_NEGATION, 10000);
        scriptEvents.addEvent(EVENT_KITHIX_NULL_GATE, 16000);
    }

    bool mLastLight = false;
    uint32_t mEnergy = 0;
    uint32_t mEnergyClockMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Venomous Hulk

class KithixVenomousHulkAI : public KithixCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KithixVenomousHulkAI(c); }
    explicit KithixVenomousHulkAI(Creature* pCreature) : KithixCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_KITHIX_HULK_VENOM_ROAR, 6000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_KITHIX_HULK_VENOM_ROAR)
            return;

        castSpellAOE(SPELL_HULK_VENOM_ROAR);
        scriptEvents.addEvent(EVENT_KITHIX_HULK_VENOM_ROAR, 15000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Voidweaver

class KithixVoidweaverAI : public KithixCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KithixVoidweaverAI(c); }
    explicit KithixVoidweaverAI(Creature* pCreature) : KithixCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KITHIX_VOIDWEAVER_VOIDWEAVE, 3000);
        scriptEvents.addEvent(EVENT_KITHIX_VOIDWEAVER_NIGHTFALL, 10000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KITHIX_VOIDWEAVER_VOIDWEAVE:
                castOnPlayer(SPELL_VOIDWEAVER_VOIDWEAVE, false);
                scriptEvents.addEvent(EVENT_KITHIX_VOIDWEAVER_VOIDWEAVE, 8000);
                break;
            case EVENT_KITHIX_VOIDWEAVER_NIGHTFALL:
                castSpellAOE(SPELL_VOIDWEAVER_NIGHTFALL);
                scriptEvents.addEvent(EVENT_KITHIX_VOIDWEAVER_NIGHTFALL, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Bomber Beetle: fixates a player and bursts on reaching it

class KithixBomberBeetleAI : public KithixCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KithixBomberBeetleAI(c); }
    explicit KithixBomberBeetleAI(Creature* pCreature) : KithixCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mBurst = false;
        if (Unit* pPrey = rollPlayer(true))
        {
            castSpell(pPrey, SPELL_BEETLE_FIXATE, true);
            addThreat(pPrey, 1000000.0f);
        }
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mBurst)
            return;

        Unit* pPrey = getCreature()->getAIInterface()->getCurrentTarget();
        if (pPrey == nullptr || getCreature()->getDistance(pPrey) > BeetleBurstRange)
            return;

        mBurst = true;
        castSpellAOE(SPELL_BEETLE_GUT_REACTION, true);
        despawn(500, 0);
    }

private:
    bool mBurst = false;
};

void SetupTheUnbindingOfKithix(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_UNBINDING_OF_KITHIX, &UnbindingOfKithixInstanceScript::Create);

    mgr->register_creature_script(BOSS_KITHIX, &UnbindingKithixAI::Create);

    registerKithixCreature(mgr, NPC_KITHIX_VENOMOUS_HULK_PLACEHOLDER, &KithixVenomousHulkAI::Create);
    registerKithixCreature(mgr, NPC_KITHIX_VOIDWEAVER_PLACEHOLDER, &KithixVoidweaverAI::Create);
    registerKithixCreature(mgr, NPC_KITHIX_BOMBER_BEETLE_PLACEHOLDER, &KithixBomberBeetleAI::Create);
}
