/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Necrotic_Wake.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <map>

enum TheNecroticWakeEvents
{
    EVENT_BLIGHTBONE_HEAVING_RETCH = 1,
    EVENT_BLIGHTBONE_FETID_GAS,
    EVENT_BLIGHTBONE_CRUNCH,
    EVENT_CARRION_WORM_GORGE,

    EVENT_AMARTH_NECROTIC_BOLT,
    EVENT_AMARTH_LAND_OF_THE_DEAD,
    EVENT_AMARTH_FINAL_HARVEST,
    EVENT_AMARTH_UNHOLY_FRENZY,
    EVENT_REANIMATED_MAGE_FROSTBOLT,

    EVENT_STITCHFLESH_AWAKEN_CREATION,
    EVENT_STITCHFLESH_STITCHNEEDLE,
    EVENT_STITCHFLESH_EMBALMING_ICHOR,
    EVENT_STITCHFLESH_SEVER_FLESH,
    EVENT_STITCHFLESH_ESCAPE,
    EVENT_STITCHFLESH_BACK_ON_TABLE,
    EVENT_CREATION_MEAT_HOOK,
    EVENT_CREATION_MUTILATE,
    EVENT_CREATION_FESTERING_ROT,

    EVENT_NALTHOR_ICY_SHARD,
    EVENT_NALTHOR_FROZEN_BINDS,
    EVENT_NALTHOR_COMET_STORM,
    EVENT_NALTHOR_ICEBOUND_AEGIS,
    EVENT_NALTHOR_DARK_EXILE,
    EVENT_SIPHONER_ENFEEBLE
};

enum TheNecroticWakeActions
{
    ACTION_STITCHFLESH_HOOKED = 1
};

namespace
{
    float const NecroticWakeTwoPi = 6.28318531f;

    uint32_t const CarrionWormsPerRetch = 2;
    uint32_t const CarrionWormGorgesToErupt = 6;

    uint32_t const AmarthWarriorsPerRaise = 2;
    uint32_t const AmarthMagesPerRaise = 2;

    // half width of the hook path that still catches the surgeon
    float const MeatHookReach = 4.0f;
    uint32_t const StitchfleshArenaMs = 30000;

    LocationVector necroticWakeRing(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(NecroticWakeTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    LocationVector necroticWakeBetween(LocationVector const& from, LocationVector const& to, float fraction)
    {
        return LocationVector(from.x + (to.x - from.x) * fraction, from.y + (to.y - from.y) * fraction, from.z + (to.z - from.z) * fraction, from.o);
    }

    // probe lies on the flat path from origin to target
    bool necroticWakeOnHookPath(LocationVector const& origin, LocationVector const& target, LocationVector const& probe, float reach)
    {
        float const dx = target.x - origin.x;
        float const dy = target.y - origin.y;
        float const lengthSq = dx * dx + dy * dy;
        if (lengthSq < 1.0f)
            return false;

        float const along = ((probe.x - origin.x) * dx + (probe.y - origin.y) * dy) / lengthSq;
        if (along <= 0.0f || along > 1.0f)
            return false;

        float const offX = origin.x + along * dx - probe.x;
        float const offY = origin.y + along * dy - probe.y;
        return offX * offX + offY * offY <= reach * reach;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TheNecroticWakeInstanceScript : public InstanceScript
{
public:
    explicit TheNecroticWakeInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheNecroticWakeEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheNecroticWakeInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        if (pCreature->getEntry() == BOSS_SURGEON_STITCHFLESH)
            mTrackedGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
    }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    void setHookOrigin(LocationVector const& origin) { mHookOrigin = origin; }
    LocationVector const& getHookOrigin() const { return mHookOrigin; }

private:
    std::map<uint32_t, WoWGuid> mTrackedGuids;
    LocationVector mHookOrigin;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and target helpers

class TheNecroticWakeCombatAI : public CreatureAIScript
{
public:
    explicit TheNecroticWakeCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    TheNecroticWakeInstanceScript* getNecroticWake()
    {
        if (getCreature()->GetMapId() != MAP_THE_NECROTIC_WAKE)
            return nullptr;

        return static_cast<TheNecroticWakeInstanceScript*>(getInstanceScript());
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

    // ground spell under a random player
    void castUnderRandom(uint32_t spellId)
    {
        if (Unit* pTarget = randomPlayer())
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId);
    }

    // summoned add on the side of its summoner that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd == nullptr)
            return nullptr;

        pAdd->setFaction(getCreature()->getFactionTemplate());
        setZoneWideCombat(pAdd);
        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return necroticWakeRing(getCreature()->GetPosition(), minRange, maxRange); }

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

class TheNecroticWakeBossAI : public TheNecroticWakeCombatAI
{
public:
    TheNecroticWakeBossAI(Creature* pCreature, uint32_t dataId) : TheNecroticWakeCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (TheNecroticWakeInstanceScript* pWake = getNecroticWake())
        {
            pWake->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pWake->setBossState(mDataId, EncounterStates::InProgress);
        }

        TheNecroticWakeCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (TheNecroticWakeInstanceScript* pWake = getNecroticWake())
            {
                pWake->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pWake->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        TheNecroticWakeCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (TheNecroticWakeInstanceScript* pWake = getNecroticWake())
        {
            pWake->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pWake->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Blightbone

class TheNecroticWakeBlightboneAI : public TheNecroticWakeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheNecroticWakeBlightboneAI(c); }
    explicit TheNecroticWakeBlightboneAI(Creature* pCreature) : TheNecroticWakeBossAI(pCreature, DATA_BLIGHTBONE) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_BLIGHTBONE_CRUNCH, 6000);
        scriptEvents.addEvent(EVENT_BLIGHTBONE_HEAVING_RETCH, 10000);
        scriptEvents.addEvent(EVENT_BLIGHTBONE_FETID_GAS, 18000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BLIGHTBONE_CRUNCH:
                castSpellOnVictim(SPELL_BLIGHTBONE_CRUNCH);
                scriptEvents.addEvent(EVENT_BLIGHTBONE_CRUNCH, 12000);
                break;
            case EVENT_BLIGHTBONE_HEAVING_RETCH:
                if (Unit* pTarget = castAtRandom(SPELL_BLIGHTBONE_HEAVING_RETCH, true))
                {
                    warn("Blightbone begins a Heaving Retch!");
                    spewWorms(pTarget);
                }
                scriptEvents.addEvent(EVENT_BLIGHTBONE_HEAVING_RETCH, 20000);
                break;
            case EVENT_BLIGHTBONE_FETID_GAS:
                castSpellAOE(SPELL_BLIGHTBONE_FETID_GAS);
                scriptEvents.addEvent(EVENT_BLIGHTBONE_FETID_GAS, 24000);
                break;
            default:
                break;
        }
    }

private:
    // worms land along the retch towards its target
    void spewWorms(Unit* pTarget)
    {
        LocationVector const landing = necroticWakeBetween(getCreature()->GetPosition(), pTarget->GetPosition(), 0.6f);
        for (uint32_t i = 0; i < CarrionWormsPerRetch; ++i)
            callAdd(NPC_BLIGHTBONE_CARRION_WORM, necroticWakeRing(landing, 1.0f, 4.0f));
    }
};

// gorges on a random player instead of meleeing, then erupts
class TheNecroticWakeCarrionWormAI : public TheNecroticWakeCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheNecroticWakeCarrionWormAI(c); }
    explicit TheNecroticWakeCarrionWormAI(Creature* pCreature) : TheNecroticWakeCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mGorges = 0;
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_CARRION_WORM_GORGE, 2000);
    }

    void onDisengage() override { _setMeleeDisabled(false); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_CARRION_WORM_GORGE)
            return;

        // the first bite picks the victim
        if (mGorges == 0)
            focusOn(randomPlayer());

        castSpellOnVictim(SPELL_CARRION_WORM_BLOOD_GORGE);
        if (++mGorges < CarrionWormGorgesToErupt)
        {
            scriptEvents.addEvent(EVENT_CARRION_WORM_GORGE, 3000);
            return;
        }

        castSpellAOE(SPELL_CARRION_WORM_CARRION_ERUPTION, true);
        despawn(500, 0);
    }

private:
    uint32_t mGorges = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Amarth, The Harvester

class TheNecroticWakeAmarthAI : public TheNecroticWakeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheNecroticWakeAmarthAI(c); }
    explicit TheNecroticWakeAmarthAI(Creature* pCreature) : TheNecroticWakeBossAI(pCreature, DATA_AMARTH) {}

    // the harvest consumes what is left of the raised dead
    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == SPELL_AMARTH_FINAL_HARVEST)
            dismissMinions();
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_AMARTH_NECROTIC_BOLT, 5000);
        scriptEvents.addEvent(EVENT_AMARTH_LAND_OF_THE_DEAD, 10000);
        scriptEvents.addEvent(EVENT_AMARTH_UNHOLY_FRENZY, 18000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AMARTH_NECROTIC_BOLT:
                castAtRandom(SPELL_AMARTH_NECROTIC_BOLT);
                scriptEvents.addEvent(EVENT_AMARTH_NECROTIC_BOLT, 8000);
                break;
            case EVENT_AMARTH_LAND_OF_THE_DEAD:
                warn("Amarth raises the dead to fight for him!");
                castSpellOnSelf(SPELL_AMARTH_LAND_OF_THE_DEAD);
                raiseMinions();
                scriptEvents.addEvent(EVENT_AMARTH_FINAL_HARVEST, 22000);
                scriptEvents.addEvent(EVENT_AMARTH_LAND_OF_THE_DEAD, 45000);
                break;
            case EVENT_AMARTH_FINAL_HARVEST:
                if (minionsStanding())
                {
                    warn("Amarth begins the Final Harvest!");
                    castSpellAOE(SPELL_AMARTH_FINAL_HARVEST);
                }
                else
                {
                    dismissMinions();
                }
                break;
            case EVENT_AMARTH_UNHOLY_FRENZY:
                castSpellOnSelf(SPELL_AMARTH_UNHOLY_FRENZY);
                scriptEvents.addEvent(EVENT_AMARTH_UNHOLY_FRENZY, 30000);
                break;
            default:
                break;
        }
    }

private:
    void raiseMinions()
    {
        for (uint32_t i = 0; i < AmarthWarriorsPerRaise; ++i)
            callAdd(NPC_AMARTH_REANIMATED_WARRIOR, aroundSelf(6.0f, 14.0f));

        for (uint32_t i = 0; i < AmarthMagesPerRaise; ++i)
            callAdd(NPC_AMARTH_REANIMATED_MAGE, aroundSelf(10.0f, 18.0f));
    }

    bool minionsStanding()
    {
        for (uint64_t const guid : summons)
        {
            Creature* pMinion = getCreature()->getWorldMapCreature(guid);
            if (pMinion != nullptr && pMinion->isAlive())
                return true;
        }

        return false;
    }

    void dismissMinions()
    {
        summons.despawnEntry(NPC_AMARTH_REANIMATED_WARRIOR);
        summons.despawnEntry(NPC_AMARTH_REANIMATED_MAGE);
    }
};

class TheNecroticWakeReanimatedMageAI : public TheNecroticWakeCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheNecroticWakeReanimatedMageAI(c); }
    explicit TheNecroticWakeReanimatedMageAI(Creature* pCreature) : TheNecroticWakeCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_REANIMATED_MAGE_FROSTBOLT, 1500); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_REANIMATED_MAGE_FROSTBOLT)
            return;

        castSpellOnVictim(SPELL_REANIMATED_MAGE_FROSTBOLT);
        scriptEvents.addEvent(EVENT_REANIMATED_MAGE_FROSTBOLT, 3500);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Surgeon Stitchflesh, out of reach on his worktable until a Meat Hook drags him down

class TheNecroticWakeStitchfleshAI : public TheNecroticWakeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheNecroticWakeStitchfleshAI(c); }
    explicit TheNecroticWakeStitchfleshAI(Creature* pCreature) : TheNecroticWakeBossAI(pCreature, DATA_SURGEON_STITCHFLESH) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_STITCHFLESH_HOOKED || !mOnWorktable || !_isInCombat())
            return;

        if (TheNecroticWakeInstanceScript* pWake = getNecroticWake())
            enterArena(pWake->getHookOrigin());
    }

protected:
    void onEngage() override
    {
        mWorktable = getCreature()->GetPosition();
        climbWorktable();

        // creations already on their slabs join at once
        bool creationReady = false;
        std::list<Creature*> creations;
        GetCreatureListWithEntryInGrid(creations, NPC_STITCHFLESHS_CREATION, 60.0f);
        for (Creature* pCreation : creations)
        {
            if (!pCreation->isAlive())
                continue;

            creationReady = true;
            if (!pCreation->isInCombat())
                setZoneWideCombat(pCreation);
        }

        scriptEvents.addEvent(EVENT_STITCHFLESH_AWAKEN_CREATION, creationReady ? 25000 : 1000);
        scriptEvents.addEvent(EVENT_STITCHFLESH_STITCHNEEDLE, 6000);
        scriptEvents.addEvent(EVENT_STITCHFLESH_EMBALMING_ICHOR, 12000);
    }

    void onDisengage() override
    {
        mOnWorktable = false;
        setRooted(false);
        _setMeleeDisabled(false);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_STITCHFLESH_AWAKEN_CREATION:
                warn("Surgeon Stitchflesh awakens a creation!");
                castSpellOnSelf(SPELL_STITCHFLESH_AWAKEN_CREATION);
                awakenCreation();
                scriptEvents.addEvent(EVENT_STITCHFLESH_AWAKEN_CREATION, 30000);
                break;
            case EVENT_STITCHFLESH_STITCHNEEDLE:
                castAtRandom(SPELL_STITCHFLESH_STITCHNEEDLE);
                scriptEvents.addEvent(EVENT_STITCHFLESH_STITCHNEEDLE, 9000);
                break;
            case EVENT_STITCHFLESH_EMBALMING_ICHOR:
                castSpellAOE(SPELL_STITCHFLESH_EMBALMING_ICHOR);
                scriptEvents.addEvent(EVENT_STITCHFLESH_EMBALMING_ICHOR, 18000);
                break;
            case EVENT_STITCHFLESH_SEVER_FLESH:
                castSpellOnVictim(SPELL_STITCHFLESH_SEVER_FLESH);
                scriptEvents.addEvent(EVENT_STITCHFLESH_SEVER_FLESH, 10000);
                break;
            case EVENT_STITCHFLESH_ESCAPE:
                warn("Surgeon Stitchflesh tries to Escape back to his worktable!");
                scriptEvents.removeEvent(EVENT_STITCHFLESH_SEVER_FLESH);
                castSpellOnSelf(SPELL_STITCHFLESH_ESCAPE, true);
                moveJump(mWorktable, 20.0f, 15.0f);
                scriptEvents.addEvent(EVENT_STITCHFLESH_BACK_ON_TABLE, 2000);
                break;
            case EVENT_STITCHFLESH_BACK_ON_TABLE:
                climbWorktable();
                scriptEvents.addEvent(EVENT_STITCHFLESH_STITCHNEEDLE, 4000);
                scriptEvents.addEvent(EVENT_STITCHFLESH_AWAKEN_CREATION, 8000);
                break;
            default:
                break;
        }
    }

private:
    void climbWorktable()
    {
        mOnWorktable = true;
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
    }

    void enterArena(LocationVector const& hookOrigin)
    {
        mOnWorktable = false;
        warn("Surgeon Stitchflesh is hooked off his worktable!");
        getCreature()->interruptSpell();

        scriptEvents.removeEvent(EVENT_STITCHFLESH_STITCHNEEDLE);
        scriptEvents.removeEvent(EVENT_STITCHFLESH_AWAKEN_CREATION);

        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        setRooted(false);
        _setMeleeDisabled(false);
        moveJump(necroticWakeRing(hookOrigin, 2.0f, 3.0f), 25.0f, 10.0f);

        scriptEvents.addEvent(EVENT_STITCHFLESH_SEVER_FLESH, 5000);
        scriptEvents.addEvent(EVENT_STITCHFLESH_ESCAPE, StitchfleshArenaMs);
    }

    // creations rise in the arena below, next to a player
    void awakenCreation()
    {
        Unit* pPlayer = randomPlayer();
        LocationVector const spot = pPlayer != nullptr ? necroticWakeRing(pPlayer->GetPosition(), 5.0f, 8.0f) : aroundSelf(8.0f, 14.0f);

        Creature* pCreation = summonCreature(NPC_STITCHFLESHS_CREATION, spot, CORPSE_DESPAWN);
        if (pCreation == nullptr)
            return;

        pCreation->setFaction(getCreature()->getFactionTemplate());
        pCreation->removeUnitFlags(UNIT_FLAG_STUNNED | UNIT_FLAG_NOT_SELECTABLE);
        setZoneWideCombat(pCreation);
    }

    LocationVector mWorktable;
    bool mOnWorktable = false;
};

class TheNecroticWakeCreationAI : public TheNecroticWakeCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheNecroticWakeCreationAI(c); }
    explicit TheNecroticWakeCreationAI(Creature* pCreature) : TheNecroticWakeCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_CREATION_FESTERING_ROT, 4000);
        scriptEvents.addEvent(EVENT_CREATION_MUTILATE, 6000);
        scriptEvents.addEvent(EVENT_CREATION_MEAT_HOOK, 10000);

        // the surgeon watches over every creation
        Creature* pSurgeon = surgeon();
        if (pSurgeon != nullptr && pSurgeon->isAlive() && !pSurgeon->isInCombat())
            setZoneWideCombat(pSurgeon);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CREATION_FESTERING_ROT:
                castSpellOnVictim(SPELL_CREATION_FESTERING_ROT);
                scriptEvents.addEvent(EVENT_CREATION_FESTERING_ROT, 14000);
                break;
            case EVENT_CREATION_MUTILATE:
                castSpellOnVictim(SPELL_CREATION_MUTILATE);
                scriptEvents.addEvent(EVENT_CREATION_MUTILATE, 12000);
                break;
            case EVENT_CREATION_MEAT_HOOK:
                throwMeatHook();
                scriptEvents.addEvent(EVENT_CREATION_MEAT_HOOK, 16000);
                break;
            default:
                break;
        }
    }

private:
    Creature* surgeon()
    {
        TheNecroticWakeInstanceScript* pWake = getNecroticWake();
        return pWake != nullptr ? pWake->getTracked(BOSS_SURGEON_STITCHFLESH) : nullptr;
    }

    // a hook flying past the worktable drags the surgeon down
    void throwMeatHook()
    {
        Unit* pTarget = castAtRandom(SPELL_CREATION_MEAT_HOOK);
        if (pTarget == nullptr)
            return;

        TheNecroticWakeInstanceScript* pWake = getNecroticWake();
        Creature* pSurgeon = surgeon();
        if (pWake == nullptr || pSurgeon == nullptr || !pSurgeon->isAlive() || pSurgeon->GetScript() == nullptr)
            return;

        LocationVector const origin = getCreature()->GetPosition();
        if (!necroticWakeOnHookPath(origin, pTarget->GetPosition(), pSurgeon->GetPosition(), MeatHookReach))
            return;

        pWake->setHookOrigin(origin);
        pSurgeon->GetScript()->DoAction(ACTION_STITCHFLESH_HOOKED);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nalthor the Rimebinder

class TheNecroticWakeNalthorAI : public TheNecroticWakeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheNecroticWakeNalthorAI(c); }
    explicit TheNecroticWakeNalthorAI(Creature* pCreature) : TheNecroticWakeBossAI(pCreature, DATA_NALTHOR) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_NALTHOR_ICY_SHARD, 4000);
        scriptEvents.addEvent(EVENT_NALTHOR_FROZEN_BINDS, 9000);
        scriptEvents.addEvent(EVENT_NALTHOR_COMET_STORM, 14000);
        scriptEvents.addEvent(EVENT_NALTHOR_ICEBOUND_AEGIS, 22000);
        // a full energy bar banishes a player
        scriptEvents.addEvent(EVENT_NALTHOR_DARK_EXILE, 40000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NALTHOR_ICY_SHARD:
                castSpellOnVictim(SPELL_NALTHOR_ICY_SHARD);
                scriptEvents.addEvent(EVENT_NALTHOR_ICY_SHARD, 8000);
                break;
            case EVENT_NALTHOR_FROZEN_BINDS:
                castAtRandom(SPELL_NALTHOR_FROZEN_BINDS, true);
                scriptEvents.addEvent(EVENT_NALTHOR_FROZEN_BINDS, 20000);
                break;
            case EVENT_NALTHOR_COMET_STORM:
                warn("Nalthor the Rimebinder calls down a Comet Storm!");
                castUnderRandom(SPELL_NALTHOR_COMET_STORM);
                scriptEvents.addEvent(EVENT_NALTHOR_COMET_STORM, 26000);
                break;
            case EVENT_NALTHOR_ICEBOUND_AEGIS:
                castSpellOnSelf(SPELL_NALTHOR_ICEBOUND_AEGIS);
                scriptEvents.addEvent(EVENT_NALTHOR_ICEBOUND_AEGIS, 30000);
                break;
            case EVENT_NALTHOR_DARK_EXILE:
                if (castAtRandom(SPELL_NALTHOR_DARK_EXILE, true) != nullptr)
                    warn("Nalthor the Rimebinder casts Dark Exile!");
                scriptEvents.addEvent(EVENT_NALTHOR_DARK_EXILE, 40000);
                break;
            default:
                break;
        }
    }
};

class TheNecroticWakeZolramusSiphonerAI : public TheNecroticWakeCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheNecroticWakeZolramusSiphonerAI(c); }
    explicit TheNecroticWakeZolramusSiphonerAI(Creature* pCreature) : TheNecroticWakeCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_SIPHONER_ENFEEBLE, 3000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SIPHONER_ENFEEBLE)
            return;

        castSpellOnVictim(SPELL_SIPHONER_ENFEEBLE);
        scriptEvents.addEvent(EVENT_SIPHONER_ENFEEBLE, 8000);
    }
};

void SetupTheNecroticWake(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_NECROTIC_WAKE, &TheNecroticWakeInstanceScript::Create);

    // Blightbone
    mgr->register_creature_script(BOSS_BLIGHTBONE, &TheNecroticWakeBlightboneAI::Create);
    mgr->register_creature_script(NPC_BLIGHTBONE_CARRION_WORM, &TheNecroticWakeCarrionWormAI::Create);
    mgr->register_creature_script(NPC_BLIGHTBONE_CARRION_WORM_ALT, &TheNecroticWakeCarrionWormAI::Create);

    // Amarth, The Harvester
    mgr->register_creature_script(BOSS_AMARTH, &TheNecroticWakeAmarthAI::Create);
    mgr->register_creature_script(NPC_AMARTH_REANIMATED_MAGE, &TheNecroticWakeReanimatedMageAI::Create);

    // Surgeon Stitchflesh
    mgr->register_creature_script(BOSS_SURGEON_STITCHFLESH, &TheNecroticWakeStitchfleshAI::Create);
    mgr->register_creature_script(NPC_STITCHFLESHS_CREATION, &TheNecroticWakeCreationAI::Create);

    // Nalthor the Rimebinder
    mgr->register_creature_script(BOSS_NALTHOR_THE_RIMEBINDER, &TheNecroticWakeNalthorAI::Create);
    mgr->register_creature_script(NPC_NALTHOR_ZOLRAMUS_SIPHONER, &TheNecroticWakeZolramusSiphonerAI::Create);
}
