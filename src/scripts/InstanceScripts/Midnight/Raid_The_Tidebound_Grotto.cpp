/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_The_Tidebound_Grotto.hpp"

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

enum TideboundGrottoEvents
{
    // Nymrissa Wavecaller
    EVENT_TIDEBOUND_WATER_JET = 1,
    EVENT_TIDEBOUND_ICEBLADE_FLURRY,
    EVENT_TIDEBOUND_CHILLING_FROST,
    EVENT_TIDEBOUND_ABYSSAL_RAIN,
    EVENT_TIDEBOUND_ALLURING_BUBBLE,
    EVENT_TIDEBOUND_SWIRLING_WHIRLPOOLS,

    // Bubblefin Berserker
    EVENT_TIDEBOUND_PULSING_TIDES
};

namespace
{
    float const TideboundTwoPi = 6.28318531f;
    float const TideboundPlayerReach = 100.0f;

    uint32_t const TideboundBerserkMs = 420000;

    // the bubble calls a pack of murlocs, the whirlpools burst it later
    uint32_t const BubbleMurlocCount = 6;
    uint32_t const BubbleLifetimeMs = 16000;
    float const BubbleCatchRange = 8.0f;
    uint32_t const BubbleMoveId = 1;

    LocationVector tideboundRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(TideboundTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TideboundGrottoInstanceScript : public InstanceScript
{
public:
    explicit TideboundGrottoInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TideboundGrottoEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TideboundGrottoInstanceScript(pMapMgr); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, berserk and target helpers

class TideboundCombatAI : public CreatureAIScript
{
public:
    explicit TideboundCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        tickBerserk(diff);

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
    virtual void onEvent(uint32_t /*eventId*/) {}

    // distinct random players, the current target only as last resort
    std::vector<Unit*> rollPlayers(size_t count, bool spareTank)
    {
        TargetFilter const filter = spareTank ? TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent) : TargetFilter_Player;
        FilterArgs const args(filter);

        std::vector<Unit*> pool;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || getCreature()->getDistance(pObject) > TideboundPlayerReach)
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

    void castOnPlayers(uint32_t spellId, size_t count, bool spareTank)
    {
        for (Unit* pTarget : rollPlayers(count, spareTank))
            castSpell(pTarget, spellId, true);
    }

    Creature* bringAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
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
        castSpellOnSelf(SPELL_TIDEBOUND_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nymrissa Wavecaller

class TideboundNymrissaAI : public TideboundCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TideboundNymrissaAI(c); }
    explicit TideboundNymrissaAI(Creature* pCreature) : TideboundCombatAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(DATA_NYMRISSA_WAVECALLER, EncounterStates::InProgress);
        }

        TideboundCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(DATA_NYMRISSA_WAVECALLER, EncounterStates::Failed);
            }
        }

        TideboundCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(DATA_NYMRISSA_WAVECALLER, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    void onEngage() override
    {
        startBerserkClock(TideboundBerserkMs);
        scriptEvents.addEvent(EVENT_TIDEBOUND_WATER_JET, 8000);
        scriptEvents.addEvent(EVENT_TIDEBOUND_CHILLING_FROST, 14000);
        scriptEvents.addEvent(EVENT_TIDEBOUND_ICEBLADE_FLURRY, 19000);
        scriptEvents.addEvent(EVENT_TIDEBOUND_ABYSSAL_RAIN, 26000);
        scriptEvents.addEvent(EVENT_TIDEBOUND_ALLURING_BUBBLE, 32000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TIDEBOUND_WATER_JET:
                castSpellOnVictim(SPELL_NYMRISSA_WATER_JET);
                scriptEvents.addEvent(EVENT_TIDEBOUND_WATER_JET, 22000);
                break;
            case EVENT_TIDEBOUND_ICEBLADE_FLURRY:
                castSpellOnVictim(SPELL_NYMRISSA_ICEBLADE_FLURRY);
                scriptEvents.addEvent(EVENT_TIDEBOUND_ICEBLADE_FLURRY, 22000);
                break;
            case EVENT_TIDEBOUND_CHILLING_FROST:
                castOnPlayers(SPELL_NYMRISSA_CHILLING_FROST, 2, true);
                scriptEvents.addEvent(EVENT_TIDEBOUND_CHILLING_FROST, 30000);
                break;
            case EVENT_TIDEBOUND_ABYSSAL_RAIN:
                castSpellAOE(SPELL_NYMRISSA_ABYSSAL_RAIN);
                if (isHeroic())
                    castSpellOnSelf(SPELL_NYMRISSA_WAVECALLERS_MIGHT, true);
                scriptEvents.addEvent(EVENT_TIDEBOUND_ABYSSAL_RAIN, 42000);
                break;
            case EVENT_TIDEBOUND_ALLURING_BUBBLE:
                raiseBubble();
                scriptEvents.addEvent(EVENT_TIDEBOUND_SWIRLING_WHIRLPOOLS, BubbleLifetimeMs);
                scriptEvents.addEvent(EVENT_TIDEBOUND_ALLURING_BUBBLE, 70000);
                break;
            case EVENT_TIDEBOUND_SWIRLING_WHIRLPOOLS:
                burstBubble();
                break;
            default:
                break;
        }
    }

private:
    // murlocs from the edges rush toward the bubble without fighting
    void raiseBubble()
    {
        mBubbleSpot = tideboundRingPoint(getCreature()->GetPosition(), 10.0f, 16.0f);
        announce("Nymrissa Wavecaller conjures an Alluring Bubble! The murlocs swarm toward it!");
        castSpellAOE(SPELL_NYMRISSA_ALLURING_BUBBLE);

        for (uint32_t i = 0; i < BubbleMurlocCount; ++i)
        {
            Creature* pMurloc = summonCreature(NPC_BUBBLEFIN_SHORERUNNER, tideboundRingPoint(mBubbleSpot, 28.0f, 36.0f), CORPSE_DESPAWN);
            if (pMurloc == nullptr || pMurloc->GetScript() == nullptr)
                continue;

            pMurloc->GetScript()->setReactState(REACT_PASSIVE);
            pMurloc->GetScript()->movePoint(BubbleMoveId, mBubbleSpot);
        }
    }

    // murlocs inside the bubble turn into Berserkers, the rest attack
    void burstBubble()
    {
        castSpellAOE(SPELL_NYMRISSA_SWIRLING_WHIRLPOOLS, true);
        announce("Swirling Whirlpools burst the Alluring Bubble!");
        castSpellAOE(SPELL_NYMRISSA_POP, true);

        std::list<Creature*> murlocs;
        GetCreatureListWithEntryInGrid(murlocs, NPC_BUBBLEFIN_SHORERUNNER, TideboundPlayerReach);
        for (Creature* pMurloc : murlocs)
        {
            if (!pMurloc->isAlive())
                continue;

            if (pMurloc->getExactDist2d(mBubbleSpot) <= BubbleCatchRange)
            {
                bringAdd(NPC_BUBBLEFIN_BERSERKER, pMurloc->GetPosition());
                pMurloc->despawn(100, 0);
                continue;
            }

            if (pMurloc->GetScript() != nullptr)
                pMurloc->GetScript()->setReactState(REACT_AGGRESSIVE);
            setZoneWideCombat(pMurloc);
        }
    }

    LocationVector mBubbleSpot;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Bubblefin Shorerunner: only runs and fights, steered by Nymrissa

class TideboundShorerunnerAI : public TideboundCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TideboundShorerunnerAI(c); }
    explicit TideboundShorerunnerAI(Creature* pCreature) : TideboundCombatAI(pCreature) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Bubblefin Berserker

class TideboundBerserkerAI : public TideboundCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TideboundBerserkerAI(c); }
    explicit TideboundBerserkerAI(Creature* pCreature) : TideboundCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_TIDEBOUND_PULSING_TIDES, 3000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_TIDEBOUND_PULSING_TIDES)
            return;

        castSpellAOE(SPELL_BERSERKER_PULSING_TIDES);
        scriptEvents.addEvent(EVENT_TIDEBOUND_PULSING_TIDES, 9000);
    }
};

void SetupTheTideboundGrotto(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_TIDEBOUND_GROTTO, &TideboundGrottoInstanceScript::Create);

    mgr->register_creature_script(BOSS_NYMRISSA_WAVECALLER, &TideboundNymrissaAI::Create);
    mgr->register_creature_script(NPC_BUBBLEFIN_SHORERUNNER, &TideboundShorerunnerAI::Create);
    mgr->register_creature_script(NPC_BUBBLEFIN_BERSERKER, &TideboundBerserkerAI::Create);
}
