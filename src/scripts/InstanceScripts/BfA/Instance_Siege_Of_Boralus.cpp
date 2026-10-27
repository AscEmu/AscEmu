/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Siege_Of_Boralus.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <list>
#include <string>
#include <utility>
#include <vector>

enum SiegeOfBoralusEvents
{
    EVENT_DOCK_CRASH = 1,
    EVENT_DOCK_CHASE,
    EVENT_DOCK_CHASE_END,
    EVENT_DOCK_ARTILLERY,

    EVENT_HADAL_BREAK_WATER,
    EVENT_HADAL_CRASHING_TIDE,

    EVENT_VIQGOTH_PUTRID_WATERS,
    EVENT_VIQGOTH_WIPE_CHECK,
    EVENT_TERROR_SLAM,
    EVENT_TERROR_HULL_CRACKER,

    EVENT_LOCKWOOD_SHOOT,
    EVENT_LOCKWOOD_GUT_SHOT,
    EVENT_LOCKWOOD_CLEAR_THE_DECK,
    EVENT_LOCKWOOD_WITHDRAW,
    EVENT_LOCKWOOD_DREAD_VOLLEY
};

namespace
{
    uint32_t const DockChaseMs = 10000;
    uint32_t const HadalBreakWaterTargets = 5;

    uint32_t const ViqGothCannonShots = 3;
    uint32_t const ViqGothTerrors[] = { NPC_DEMOLISHING_TERROR, NPC_GRIPPING_TERROR };

    // Lockwood falls back to her ship while the crew boards
    LocationVector const LockwoodShipDeck(662.0f, -245.0f, 11.06f, 3.25f);
    LocationVector const LockwoodCrewLanding(641.0f, -244.0f, 12.0f, 3.30f);
    uint32_t const LockwoodDeckhandCount = 2;
    uint32_t const LockwoodGutShotTargets = 3;
    uint32_t const LockwoodVolleyTargets = 3;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class SiegeOfBoralusInstanceScript : public InstanceScript
{
public:
    explicit SiegeOfBoralusInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(SiegeOfBoralusEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new SiegeOfBoralusInstanceScript(pMapMgr); }

    Creature* findLiving(uint32_t entry)
    {
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature->isAlive())
                return pCreature;
        }

        return nullptr;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        Creature* pTarget = findLiving(entry);
        if (pTarget != nullptr && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and target helpers

class SiegeOfBoralusCombatAI : public CreatureAIScript
{
public:
    explicit SiegeOfBoralusCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();

        if (isAlive())
            onReset();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onReset() {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    SiegeOfBoralusInstanceScript* getSiege()
    {
        if (getCreature()->GetMapId() != MAP_SIEGE_OF_BORALUS)
            return nullptr;

        return static_cast<SiegeOfBoralusInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    std::vector<Unit*> livingPlayers(float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && getCreature()->getDistance(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        return players;
    }

    // up to count different players picked at random
    std::vector<Unit*> somePlayers(uint32_t count, float range)
    {
        std::vector<Unit*> players = livingPlayers(range);
        for (size_t i = players.size(); i > 1; --i)
        {
            size_t const pick = Util::getRandomUInt(static_cast<uint32_t>(i - 1));
            std::swap(players[i - 1], players[pick]);
        }

        if (players.size() > count)
            players.resize(count);

        return players;
    }

    Unit* farthestPlayer(float range)
    {
        Unit* pFarthest = nullptr;
        float farthestRange = -1.0f;
        for (Unit* pPlayer : livingPlayers(range))
        {
            float const distance = getCreature()->getDistance(pPlayer);
            if (distance > farthestRange)
            {
                farthestRange = distance;
                pFarthest = pPlayer;
            }
        }

        return pFarthest;
    }

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // Viq'Goth stirs as soon as one of his terrors is attacked
    void stirViqGoth(int32_t action)
    {
        if (SiegeOfBoralusInstanceScript* pSiege = getSiege())
            pSiege->sendAction(BOSS_VIQGOTH, action);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class SiegeOfBoralusBossAI : public SiegeOfBoralusCombatAI
{
public:
    SiegeOfBoralusBossAI(Creature* pCreature, uint32_t encounterId) : SiegeOfBoralusCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        SiegeOfBoralusCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mEncounterId, EncounterStates::Failed);
            }
        }

        SiegeOfBoralusCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
        onDefeat();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sergeant Bainbridge and Chopper Redhook, the mirrored first fights of both factions

class SiegeOfBoralusDockBossAI : public SiegeOfBoralusBossAI
{
public:
    SiegeOfBoralusDockBossAI(Creature* pCreature, uint32_t encounterId, uint32_t crashSpell, uint32_t chaseSpell, std::string const& chaseText)
        : SiegeOfBoralusBossAI(pCreature, encounterId), mCrashSpell(crashSpell), mChaseSpell(chaseSpell), mChaseText(chaseText) {}

protected:
    void onEngage() override
    {
        mChasing = false;
        scriptEvents.addEvent(EVENT_DOCK_CRASH, 1000);
        scriptEvents.addEvent(EVENT_DOCK_CHASE, 3000);
        scriptEvents.addEvent(EVENT_DOCK_ARTILLERY, 6000);
    }

    void onReset() override { endChase(); }

    void onDefeat() override { endChase(); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DOCK_CRASH:
                castSpellAOE(mCrashSpell);
                scriptEvents.addEvent(EVENT_DOCK_CRASH, 15000);
                break;
            case EVENT_DOCK_CHASE:
                startChase();
                scriptEvents.addEvent(EVENT_DOCK_CHASE, 15000);
                break;
            case EVENT_DOCK_CHASE_END:
                endChase();
                break;
            case EVENT_DOCK_ARTILLERY:
                for (Unit* pPlayer : livingPlayers(100.0f))
                    castSpell(pPlayer, SPELL_DOCK_SIGHTED_ARTILLERY, true);
                scriptEvents.addEvent(EVENT_DOCK_ARTILLERY, 20000);
                break;
            default:
                break;
        }
    }

    virtual void onChaseStart() {}
    virtual void onChaseEnd() {}

private:
    // runs down the farthest player for a while
    void startChase()
    {
        Unit* pTarget = farthestPlayer(100.0f);
        if (pTarget == nullptr)
            return;

        endChase();
        warn(mChaseText);
        castSpell(pTarget, mChaseSpell);
        getCreature()->getThreatManager().fixateTarget(pTarget);
        mChasing = true;
        onChaseStart();

        scriptEvents.removeEvent(EVENT_DOCK_CHASE_END);
        scriptEvents.addEvent(EVENT_DOCK_CHASE_END, DockChaseMs);
    }

    void endChase()
    {
        if (!mChasing)
            return;

        mChasing = false;
        getCreature()->getThreatManager().clearFixate();
        onChaseEnd();
    }

    uint32_t mCrashSpell;
    uint32_t mChaseSpell;
    std::string mChaseText;
    bool mChasing = false;
};

class SiegeOfBoralusBainbridgeAI : public SiegeOfBoralusDockBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SiegeOfBoralusBainbridgeAI(c); }
    explicit SiegeOfBoralusBainbridgeAI(Creature* pCreature)
        : SiegeOfBoralusDockBossAI(pCreature, DATA_SERGEANT_BAINBRIDGE, SPELL_BAINBRIDGE_STEEL_TEMPEST, SPELL_BAINBRIDGE_IRON_GAZE, "Sergeant Bainbridge fixes his Iron Gaze on a target!") {}
};

class SiegeOfBoralusRedhookAI : public SiegeOfBoralusDockBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SiegeOfBoralusRedhookAI(c); }
    explicit SiegeOfBoralusRedhookAI(Creature* pCreature)
        : SiegeOfBoralusDockBossAI(pCreature, DATA_CHOPPER_REDHOOK, SPELL_REDHOOK_GORE_CRASH, SPELL_REDHOOK_ON_THE_HOOK, "Chopper Redhook puts a target On the Hook!") {}

protected:
    // the heavy swings make way for a boiling rage while he hunts
    void onChaseStart() override
    {
        mHadHeavyHitter = hasAura(SPELL_REDHOOK_HEAVY_HITTER);
        _removeAura(SPELL_REDHOOK_HEAVY_HITTER);
        castSpellOnSelf(SPELL_REDHOOK_BOILING_RAGE, true);
    }

    void onChaseEnd() override
    {
        _removeAura(SPELL_REDHOOK_BOILING_RAGE);
        if (mHadHeavyHitter && isAlive())
            castSpellOnSelf(SPELL_REDHOOK_HEAVY_HITTER, true);
    }

private:
    bool mHadHeavyHitter = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hadal Darkfathom

class SiegeOfBoralusHadalAI : public SiegeOfBoralusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SiegeOfBoralusHadalAI(c); }
    explicit SiegeOfBoralusHadalAI(Creature* pCreature) : SiegeOfBoralusBossAI(pCreature, DATA_HADAL_DARKFATHOM) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_HADAL_BREAK_WATER, 5000);
        scriptEvents.addEvent(EVENT_HADAL_CRASHING_TIDE, 10000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HADAL_BREAK_WATER:
                castSpellAOE(SPELL_HADAL_BREAK_WATER);
                for (Unit* pPlayer : somePlayers(HadalBreakWaterTargets, 100.0f))
                    castSpell(pPlayer, SPELL_HADAL_BREAK_WATER_MISSILE, true);
                scriptEvents.addEvent(EVENT_HADAL_BREAK_WATER, 15000);
                break;
            case EVENT_HADAL_CRASHING_TIDE:
                castSpellOnVictim(SPELL_HADAL_CRASHING_TIDE_MISSILE);
                scriptEvents.addEvent(EVENT_HADAL_CRASHING_TIDE, 15000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Viq'Goth, out of reach in the bay, each slain Demolishing Terror frees a cannon shot

class SiegeOfBoralusViqGothAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SiegeOfBoralusViqGothAI(c); }
    explicit SiegeOfBoralusViqGothAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
    }

    void DoAction(int32_t action) override
    {
        if (!isAlive())
            return;

        if (action == ACTION_VIQGOTH_RISE)
            rise();
        else if (action == ACTION_VIQGOTH_CANNON_SHOT)
            takeCannonShot();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        mRisen = false;
        scriptEvents.resetEvents();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(DATA_VIQGOTH, EncounterStates::Performed);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mRisen)
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_VIQGOTH_PUTRID_WATERS:
                for (Object* pObject : getCreature()->getInRangePlayersSet())
                {
                    Unit* pPlayer = static_cast<Unit*>(pObject);
                    if (pPlayer != nullptr && pPlayer->isAlive() && getRangeToObject(pPlayer) <= 150.0f)
                        castSpell(pPlayer, SPELL_VIQGOTH_PUTRID_WATERS, true);
                }
                scriptEvents.addEvent(EVENT_VIQGOTH_PUTRID_WATERS, 20000);
                break;
            case EVENT_VIQGOTH_WIPE_CHECK:
                if (anyPlayerStanding())
                    scriptEvents.addEvent(EVENT_VIQGOTH_WIPE_CHECK, 3000);
                else
                    sink();
                break;
            default:
                break;
        }
    }

private:
    void rise()
    {
        InstanceScript* pInstance = getInstanceScript();
        if (mRisen || pInstance == nullptr || pInstance->getBossState(DATA_VIQGOTH) == EncounterStates::Performed)
            return;

        mRisen = true;
        mShotsTaken = 0;
        pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
        pInstance->setBossState(DATA_VIQGOTH, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_VIQGOTH_PUTRID_WATERS, 6000);
        scriptEvents.addEvent(EVENT_VIQGOTH_WIPE_CHECK, 3000);
    }

    // a third of Viq'Goth goes with every shot, the last one ends him
    void takeCannonShot()
    {
        if (!mRisen)
            return;

        ++mShotsTaken;
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "A cannon blast tears into Viq'Goth!");

        if (mShotsTaken >= ViqGothCannonShots)
        {
            getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
            return;
        }

        uint32_t const cut = getCreature()->getMaxHealth() / ViqGothCannonShots;
        uint32_t const health = getCreature()->getHealth();
        getCreature()->setHealth(health > cut ? health - cut : 1);
    }

    bool anyPlayerStanding()
    {
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && getRangeToObject(pPlayer) <= 250.0f)
                return true;
        }

        return false;
    }

    // the party fell, the terrors and the beast start over
    void sink()
    {
        mRisen = false;
        mShotsTaken = 0;
        scriptEvents.resetEvents();
        getCreature()->setHealth(getCreature()->getMaxHealth());

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(DATA_VIQGOTH, EncounterStates::Failed);
        }

        for (uint32_t entry : ViqGothTerrors)
        {
            std::list<Creature*> terrors;
            GetCreatureListWithEntryInGrid(terrors, entry, 250.0f);
            for (Creature* pTerror : terrors)
            {
                if (!pTerror->isAlive())
                    pTerror->despawn(1000, 5000);
            }
        }
    }

    bool mRisen = false;
    uint32_t mShotsTaken = 0;
};

class SiegeOfBoralusDemolishingTerrorAI : public SiegeOfBoralusCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SiegeOfBoralusDemolishingTerrorAI(c); }
    explicit SiegeOfBoralusDemolishingTerrorAI(Creature* pCreature) : SiegeOfBoralusCombatAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());

        stirViqGoth(ACTION_VIQGOTH_RISE);
        SiegeOfBoralusCombatAI::OnCombatStart(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        stirViqGoth(ACTION_VIQGOTH_CANNON_SHOT);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_TERROR_SLAM, 1000);
        scriptEvents.addEvent(EVENT_TERROR_HULL_CRACKER, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TERROR_SLAM:
                castSpellAOE(SPELL_DEMOLISHING_TERROR_SLAM);
                scriptEvents.addEvent(EVENT_TERROR_SLAM, 15000);
                break;
            case EVENT_TERROR_HULL_CRACKER:
                castSpellAOE(SPELL_DEMOLISHING_TERROR_HULL_CRACKER);
                scriptEvents.addEvent(EVENT_TERROR_HULL_CRACKER, 17000);
                break;
            default:
                break;
        }
    }
};

class SiegeOfBoralusGrippingTerrorAI : public SiegeOfBoralusCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SiegeOfBoralusGrippingTerrorAI(c); }
    explicit SiegeOfBoralusGrippingTerrorAI(Creature* pCreature) : SiegeOfBoralusCombatAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        stirViqGoth(ACTION_VIQGOTH_RISE);
        SiegeOfBoralusCombatAI::OnCombatStart(pTarget);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Dread Captain Lockwood, withdraws to her ship until the cannoneer falls

class SiegeOfBoralusLockwoodAI : public SiegeOfBoralusBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SiegeOfBoralusLockwoodAI(c); }
    explicit SiegeOfBoralusLockwoodAI(Creature* pCreature) : SiegeOfBoralusBossAI(pCreature, DATA_DREAD_CAPTAIN_LOCKWOOD) {}

    void onSummonedCreature(Creature* pSummon) override
    {
        SiegeOfBoralusBossAI::onSummonedCreature(pSummon);

        InstanceScript* pInstance = getInstanceScript();
        if (pInstance != nullptr && isCrew(pSummon->getEntry()))
            pInstance->sendUnitEncounter(EncounterFrameEngage, pSummon);
    }

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (!isCrew(pSummon->getEntry()))
            return;

        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, pSummon);

        if (mWithdrawn && pSummon->getEntry() == NPC_ASHVANE_CANNONEER)
            returnToDeck(true);
    }

protected:
    void onEngage() override
    {
        mWithdrawn = false;
        startDeckRotation();
    }

    void onReset() override { returnToDeck(false); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LOCKWOOD_SHOOT:
                for (Unit* pPlayer : somePlayers(1, 30.0f))
                    castSpell(pPlayer, SPELL_LOCKWOOD_SHOOT);
                scriptEvents.addEvent(EVENT_LOCKWOOD_SHOOT, 2500);
                break;
            case EVENT_LOCKWOOD_GUT_SHOT:
                for (Unit* pPlayer : somePlayers(LockwoodGutShotTargets, 45.0f))
                    castSpell(pPlayer, SPELL_LOCKWOOD_GUT_SHOT, true);
                scriptEvents.addEvent(EVENT_LOCKWOOD_GUT_SHOT, 15000);
                break;
            case EVENT_LOCKWOOD_CLEAR_THE_DECK:
                castSpellAOE(SPELL_LOCKWOOD_CLEAR_THE_DECK);
                scriptEvents.addEvent(EVENT_LOCKWOOD_CLEAR_THE_DECK, 20000);
                break;
            case EVENT_LOCKWOOD_WITHDRAW:
                withdraw();
                break;
            case EVENT_LOCKWOOD_DREAD_VOLLEY:
                for (Unit* pPlayer : somePlayers(LockwoodVolleyTargets, 100.0f))
                    castSpell(pPlayer, SPELL_LOCKWOOD_DREAD_VOLLEY, true);
                scriptEvents.addEvent(EVENT_LOCKWOOD_DREAD_VOLLEY, 15000);
                break;
            default:
                break;
        }
    }

private:
    static bool isCrew(uint32_t entry) { return entry == NPC_ASHVANE_DECKHAND || entry == NPC_ASHVANE_CANNONEER; }

    void startDeckRotation()
    {
        scriptEvents.addEvent(EVENT_LOCKWOOD_SHOOT, 1000);
        scriptEvents.addEvent(EVENT_LOCKWOOD_GUT_SHOT, 3000);
        scriptEvents.addEvent(EVENT_LOCKWOOD_CLEAR_THE_DECK, 6000);
        scriptEvents.addEvent(EVENT_LOCKWOOD_WITHDRAW, 30000);
    }

    void withdraw()
    {
        mWithdrawn = true;
        scriptEvents.resetEvents();

        warn("Dread Captain Lockwood withdraws to her ship and sends her crew!");
        castSpellOnSelf(SPELL_LOCKWOOD_WITHDRAW, true);
        attackStop();
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        MoveTeleport(LockwoodShipDeck);
        setRooted(true);

        callAdd(NPC_ASHVANE_CANNONEER, LockwoodCrewLanding);
        for (uint32_t i = 0; i < LockwoodDeckhandCount; ++i)
            callAdd(NPC_ASHVANE_DECKHAND, LockwoodCrewLanding);

        scriptEvents.addEvent(EVENT_LOCKWOOD_DREAD_VOLLEY, 3000);
    }

    void returnToDeck(bool resume)
    {
        if (!mWithdrawn)
            return;

        mWithdrawn = false;
        scriptEvents.removeEvent(EVENT_LOCKWOOD_DREAD_VOLLEY);
        MoveTeleport(getCreature()->GetSpawnPosition());
        setRooted(false);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        setReactState(REACT_AGGRESSIVE);

        if (resume)
            startDeckRotation();
    }

    bool mWithdrawn = false;
};

void SetupSiegeOfBoralus(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SIEGE_OF_BORALUS, &SiegeOfBoralusInstanceScript::Create);

    mgr->register_creature_script(BOSS_SERGEANT_BAINBRIDGE, &SiegeOfBoralusBainbridgeAI::Create);
    mgr->register_creature_script(BOSS_CHOPPER_REDHOOK, &SiegeOfBoralusRedhookAI::Create);

    mgr->register_creature_script(BOSS_HADAL_DARKFATHOM, &SiegeOfBoralusHadalAI::Create);

    mgr->register_creature_script(BOSS_VIQGOTH, &SiegeOfBoralusViqGothAI::Create);
    mgr->register_creature_script(NPC_DEMOLISHING_TERROR, &SiegeOfBoralusDemolishingTerrorAI::Create);
    mgr->register_creature_script(NPC_GRIPPING_TERROR, &SiegeOfBoralusGrippingTerrorAI::Create);

    mgr->register_creature_script(BOSS_DREAD_CAPTAIN_LOCKWOOD, &SiegeOfBoralusLockwoodAI::Create);
}
