/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Bloodmaul_Slag_Mines.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <set>

enum BloodmaulSlagMinesEvents
{
    EVENT_CRUSHTO_EARTH_CRUSH = 1,
    EVENT_CRUSHTO_FEROCIOUS_YELL,
    EVENT_CRUSHTO_RAISE_THE_MINERS,
    EVENT_CRUSHTO_CRUSHING_LEAP,
    EVENT_CRUSHTO_WILD_SLAM,

    EVENT_GOGDUH_MAGMA_BARRAGE,
    EVENT_GOGDUH_CALL_RUINATION,
    EVENT_GOGDUH_CALL_CALAMITY,

    EVENT_MAGMOLATUS_WITHERING_FLAMES,
    EVENT_MAGMOLATUS_MOLTEN_IMPACT,
    EVENT_MAGMOLATUS_SLAG_SMASH,

    EVENT_RUINATION_ROUGH_SMASH,
    EVENT_RUINATION_SHATTER_EARTH,
    EVENT_CALAMITY_SCORCH,
    EVENT_CALAMITY_DANCING_FLAMES,
    EVENT_CALAMITY_FIRESTORM,

    EVENT_ROLTALL_FIERY_BOULDER,
    EVENT_ROLTALL_NEXT_BOULDER,
    EVENT_ROLTALL_HEAT_WAVE,
    EVENT_ROLTALL_BURNING_SLAG,

    EVENT_GUGROKK_MAGMA_ERUPTION,
    EVENT_GUGROKK_MOLTEN_BLAST,
    EVENT_GUGROKK_UNSTABLE_SLAG,
    EVENT_GUGROKK_FLAME_BUFFET,

    EVENT_SLAG_EMPOWER
};

enum BloodmaulSlagMinesBoulderMoves
{
    BOULDER_MOVE_ACROSS = 1,
    BOULDER_MOVE_BACK   = 2
};

// bridge lanes in front of Roltall: start and far end
struct BloodmaulBoulderLane
{
    uint32_t entry;
    LocationVector start;
    LocationVector end;
};

static BloodmaulBoulderLane const BloodmaulBoulderLanes[3] =
{
    { NPC_FIERY_BOULDER_RIGHT,  { 2296.247f, -204.237f, 213.545f, 3.068156f }, { 2249.747f, -205.987f, 213.545f } },
    { NPC_FIERY_BOULDER_LEFT,   { 2294.631f, -218.836f, 213.221f, 3.068156f }, { 2251.131f, -220.086f, 213.471f } },
    { NPC_FIERY_BOULDER_MIDDLE, { 2295.813f, -212.111f, 213.075f, 3.068156f }, { 2212.783f, -212.335f, 213.937f } }
};

class BloodmaulSlagMinesInstanceScript : public InstanceScript
{
public:
    explicit BloodmaulSlagMinesInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(BloodmaulSlagMinesEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new BloodmaulSlagMinesInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        if (pCreature->getEntry() == BOSS_GOGDUH)
            mGogduhGuid = pCreature->GetNewGUID();
        else if (pCreature->getEntry() == BOSS_MAGMOLATUS)
            mMagmolatusGuid = pCreature->GetNewGUID();
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_ROLTALL_ENTRANCE_WALL:
            case GO_ROLTALL_DRAW_BRIDGE:
                trackExitDoor(mRoltallEntranceGuids, pGameObject, DATA_MAGMOLATUS);
                break;
            case GO_ROLTALL_EXIT_WALL:
                trackExitDoor(mRoltallExitGuids, pGameObject, DATA_ROLTALL);
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id == DATA_MAGMOLATUS)
        {
            if (state == EncounterStates::Performed)
            {
                clearForgeGuardians();
                openDoors(mRoltallEntranceGuids);
            }
            else if (state == EncounterStates::Failed)
            {
                clearForgeGuardians();

                // the forgemaster comes back for the next attempt
                Creature* pGogduh = getCreatureByGuid(mGogduhGuid);
                if (pGogduh != nullptr && !pGogduh->isAlive())
                    pGogduh->despawn(1000, 5000);
            }
        }
        else if (id == DATA_ROLTALL && state == EncounterStates::Performed)
        {
            openDoors(mRoltallExitGuids);
        }
    }

    void releaseMagmolatus()
    {
        Creature* pMagmolatus = getCreatureByGuid(mMagmolatusGuid);
        if (pMagmolatus != nullptr && pMagmolatus->isAlive() && pMagmolatus->GetScript() != nullptr)
            pMagmolatus->GetScript()->DoAction(ACTION_MAGMOLATUS_RELEASE);
    }

private:
    void trackExitDoor(std::set<WoWGuid>& doorGuids, GameObject* pDoor, uint32_t encounterId)
    {
        if (doorGuids.insert(pDoor->GetNewGUID()).second)
            setDoorOpen(pDoor, getBossState(encounterId) == EncounterStates::Performed);
    }

    void openDoors(std::set<WoWGuid> const& doorGuids)
    {
        for (WoWGuid const& guid : doorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                setDoorOpen(pDoor, true);
        }
    }

    // the encounter decides, the spawn state of a door is not reliable
    void setDoorOpen(GameObject* pDoor, bool open)
    {
        uint8_t const state = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != state)
            pDoor->setState(state);
    }

    // Ruination and Calamity outlive the forgemaster, they leave with the encounter
    void clearForgeGuardians()
    {
        for (Creature* pGuardian : getCreatureSetForEntries({ NPC_RUINATION, NPC_CALAMITY }))
        {
            if (pGuardian != nullptr && pGuardian->isAlive())
                pGuardian->despawn(1000, 0);
        }
    }

    WoWGuid mGogduhGuid;
    WoWGuid mMagmolatusGuid;
    std::set<WoWGuid> mRoltallEntranceGuids;
    std::set<WoWGuid> mRoltallExitGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick while in combat

class BloodmaulSlagMinesCombatAI : public CreatureAIScript
{
public:
    explicit BloodmaulSlagMinesCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (uint32_t eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    void castOnRandomPlayer(uint32_t spellId)
    {
        if (Unit* pTarget = randomPlayer())
            castSpell(pTarget, spellId);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector nearSelf(float offsetX, float offsetY)
    {
        Creature* pSelf = getCreature();
        return LocationVector(pSelf->GetPositionX() + offsetX, pSelf->GetPositionY() + offsetY, pSelf->GetPositionZ(), pSelf->GetOrientation());
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class BloodmaulSlagMinesBossAI : public BloodmaulSlagMinesCombatAI
{
public:
    BloodmaulSlagMinesBossAI(Creature* pCreature, uint32_t encounterId) : BloodmaulSlagMinesCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        BloodmaulSlagMinesCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Failed);
        }

        BloodmaulSlagMinesCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }
    }

protected:
    BloodmaulSlagMinesInstanceScript* getSlagMines()
    {
        if (getCreature()->GetMapId() != MAP_BLOODMAUL_SLAG_MINES)
            return nullptr;

        return static_cast<BloodmaulSlagMinesInstanceScript*>(getInstanceScript());
    }

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Slave Watcher Crushto

class BloodmaulSlagMinesCrushtoAI : public BloodmaulSlagMinesBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BloodmaulSlagMinesCrushtoAI(c); }
    explicit BloodmaulSlagMinesCrushtoAI(Creature* pCreature) : BloodmaulSlagMinesBossAI(pCreature, DATA_CRUSHTO) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_CRUSHTO_EARTH_CRUSH, 5000);
        scriptEvents.addEvent(EVENT_CRUSHTO_FEROCIOUS_YELL, 10000);
        scriptEvents.addEvent(EVENT_CRUSHTO_RAISE_THE_MINERS, 16000);
        scriptEvents.addEvent(EVENT_CRUSHTO_CRUSHING_LEAP, 23000);
        scriptEvents.addEvent(EVENT_CRUSHTO_WILD_SLAM, 27000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CRUSHTO_EARTH_CRUSH:
                castOnRandomPlayer(SPELL_CRUSHTO_EARTH_CRUSH);
                scriptEvents.addEvent(EVENT_CRUSHTO_EARTH_CRUSH, Util::getRandomUInt(20000, 25000));
                break;
            case EVENT_CRUSHTO_FEROCIOUS_YELL:
                castSpellOnSelf(SPELL_CRUSHTO_FEROCIOUS_YELL);
                scriptEvents.addEvent(EVENT_CRUSHTO_FEROCIOUS_YELL, Util::getRandomUInt(10000, 12000));
                break;
            case EVENT_CRUSHTO_RAISE_THE_MINERS:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Slave Watcher Crushto calls the captured miners to arms!");
                castSpellOnSelf(SPELL_CRUSHTO_RAISE_THE_MINERS);
                callAdd(NPC_CAPTURED_MINER, nearSelf(5.0f, 0.0f));
                callAdd(NPC_CAPTURED_MINER, nearSelf(-5.0f, -5.0f));
                callAdd(NPC_CAPTURED_MINER, nearSelf(0.0f, 5.0f));
                scriptEvents.addEvent(EVENT_CRUSHTO_RAISE_THE_MINERS, Util::getRandomUInt(25000, 29000));
                break;
            case EVENT_CRUSHTO_CRUSHING_LEAP:
                castOnRandomPlayer(SPELL_CRUSHTO_CRUSHING_LEAP);
                scriptEvents.addEvent(EVENT_CRUSHTO_CRUSHING_LEAP, Util::getRandomUInt(20000, 23000));
                break;
            case EVENT_CRUSHTO_WILD_SLAM:
                castSpellOnVictim(SPELL_CRUSHTO_WILD_SLAM);
                scriptEvents.addEvent(EVENT_CRUSHTO_WILD_SLAM, Util::getRandomUInt(20000, 27000));
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Forgemaster Gog'duh, first half of the Magmolatus encounter

class BloodmaulSlagMinesGogduhAI : public BloodmaulSlagMinesBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BloodmaulSlagMinesGogduhAI(c); }
    explicit BloodmaulSlagMinesGogduhAI(Creature* pCreature) : BloodmaulSlagMinesBossAI(pCreature, DATA_MAGMOLATUS) {}

    // guardians are not bound to the forgemaster
    void onSummonedCreature(Creature* /*pSummon*/) override {}

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        if (BloodmaulSlagMinesInstanceScript* pSlagMines = getSlagMines())
            pSlagMines->releaseMagmolatus();
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GOGDUH_MAGMA_BARRAGE, 4000);
        scriptEvents.addEvent(EVENT_GOGDUH_CALL_RUINATION, 6000);
        scriptEvents.addEvent(EVENT_GOGDUH_CALL_CALAMITY, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GOGDUH_MAGMA_BARRAGE:
                castSpellOnSelf(SPELL_GOGDUH_MAGMA_BARRAGE);
                scriptEvents.addEvent(EVENT_GOGDUH_MAGMA_BARRAGE, 10000);
                break;
            case EVENT_GOGDUH_CALL_RUINATION:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Earth, crush them!");
                callAdd(NPC_RUINATION, nearSelf(0.0f, -4.0f));
                break;
            case EVENT_GOGDUH_CALL_CALAMITY:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Fire, burn them!");
                callAdd(NPC_CALAMITY, nearSelf(4.0f, 0.0f));
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Magmolatus, dormant until Gog'duh falls

class BloodmaulSlagMinesMagmolatusAI : public BloodmaulSlagMinesBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BloodmaulSlagMinesMagmolatusAI(c); }
    explicit BloodmaulSlagMinesMagmolatusAI(Creature* pCreature) : BloodmaulSlagMinesBossAI(pCreature, DATA_MAGMOLATUS) {}

    void OnLoad() override { becomeDormant(); }

    void justReachedSpawn() override { becomeDormant(); }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_MAGMOLATUS_RELEASE)
            return;

        setRooted(false);
        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "I am free!");
        setZoneWideCombat();
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_MAGMOLATUS_WITHERING_FLAMES, 5000);
        scriptEvents.addEvent(EVENT_MAGMOLATUS_SLAG_SMASH, 7000);
        scriptEvents.addEvent(EVENT_MAGMOLATUS_MOLTEN_IMPACT, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MAGMOLATUS_WITHERING_FLAMES:
                castOnRandomPlayer(SPELL_MAGMOLATUS_WITHERING_FLAMES);
                scriptEvents.addEvent(EVENT_MAGMOLATUS_WITHERING_FLAMES, 24000);
                break;
            case EVENT_MAGMOLATUS_SLAG_SMASH:
                castSpellOnSelf(SPELL_MAGMOLATUS_SLAG_SMASH);
                scriptEvents.addEvent(EVENT_MAGMOLATUS_SLAG_SMASH, 16000);
                break;
            case EVENT_MAGMOLATUS_MOLTEN_IMPACT:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Magmolatus creates a Molten Impact!");
                castOnRandomPlayer(SPELL_MAGMOLATUS_MOLTEN_IMPACT);
                scriptEvents.addEvent(EVENT_MAGMOLATUS_MOLTEN_IMPACT, 20000);
                break;
            default:
                break;
        }
    }

private:
    void becomeDormant()
    {
        setReactState(REACT_PASSIVE);
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ruination and Calamity

class BloodmaulSlagMinesRuinationAI : public BloodmaulSlagMinesCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BloodmaulSlagMinesRuinationAI(c); }
    explicit BloodmaulSlagMinesRuinationAI(Creature* pCreature) : BloodmaulSlagMinesCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_RUINATION_ROUGH_SMASH, 3000);
        scriptEvents.addEvent(EVENT_RUINATION_SHATTER_EARTH, 6000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RUINATION_ROUGH_SMASH:
                castSpellOnVictim(SPELL_RUINATION_ROUGH_SMASH);
                scriptEvents.addEvent(EVENT_RUINATION_ROUGH_SMASH, 5000);
                break;
            case EVENT_RUINATION_SHATTER_EARTH:
                castSpellAOE(SPELL_RUINATION_SHATTER_EARTH);
                scriptEvents.addEvent(EVENT_RUINATION_SHATTER_EARTH, 10000);
                break;
            default:
                break;
        }
    }
};

class BloodmaulSlagMinesCalamityAI : public BloodmaulSlagMinesCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BloodmaulSlagMinesCalamityAI(c); }
    explicit BloodmaulSlagMinesCalamityAI(Creature* pCreature) : BloodmaulSlagMinesCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_CALAMITY_SCORCH, 3000);
        scriptEvents.addEvent(EVENT_CALAMITY_DANCING_FLAMES, 7000);
        scriptEvents.addEvent(EVENT_CALAMITY_FIRESTORM, 15000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CALAMITY_SCORCH:
                castSpellOnVictim(SPELL_CALAMITY_SCORCH);
                scriptEvents.addEvent(EVENT_CALAMITY_SCORCH, 5000);
                break;
            case EVENT_CALAMITY_DANCING_FLAMES:
                castOnRandomPlayer(SPELL_CALAMITY_DANCING_FLAMES);
                scriptEvents.addEvent(EVENT_CALAMITY_DANCING_FLAMES, 10000);
                break;
            case EVENT_CALAMITY_FIRESTORM:
                castSpellAOE(SPELL_CALAMITY_FIRESTORM);
                scriptEvents.addEvent(EVENT_CALAMITY_FIRESTORM, 20000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Roltall, stays on his platform above the bridge

class BloodmaulSlagMinesRoltallAI : public BloodmaulSlagMinesBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BloodmaulSlagMinesRoltallAI(c); }
    explicit BloodmaulSlagMinesRoltallAI(Creature* pCreature) : BloodmaulSlagMinesBossAI(pCreature, DATA_ROLTALL) {}

    void OnLoad() override { setRooted(true); }

protected:
    void onEngage() override
    {
        mNextLane = 0;
        mSlagLeft = 0;

        if (isHeroic())
            castSpellOnSelf(SPELL_ROLTALL_SCORCHING_AURA, true);

        scriptEvents.addEvent(EVENT_ROLTALL_FIERY_BOULDER, 7000);
        scriptEvents.addEvent(EVENT_ROLTALL_HEAT_WAVE, 20000);
        scriptEvents.addEvent(EVENT_ROLTALL_BURNING_SLAG, 28000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ROLTALL_FIERY_BOULDER:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Roltall hurls Fiery Boulders down the bridge!");
                castSpellOnSelf(SPELL_ROLTALL_FIERY_BOULDER);
                mNextLane = 0;
                scriptEvents.addEvent(EVENT_ROLTALL_NEXT_BOULDER, 500);
                scriptEvents.addEvent(EVENT_ROLTALL_FIERY_BOULDER, 30000);
                break;
            case EVENT_ROLTALL_NEXT_BOULDER:
                // the three lanes roll in random order, half a second apart
                if (mNextLane == 0)
                    shuffleLanes();
                summonCreature(BloodmaulBoulderLanes[mLaneOrder[mNextLane]].entry, BloodmaulBoulderLanes[mLaneOrder[mNextLane]].start, TIMED_DESPAWN, 20000);
                if (++mNextLane < 3)
                    scriptEvents.addEvent(EVENT_ROLTALL_NEXT_BOULDER, 500);
                break;
            case EVENT_ROLTALL_HEAT_WAVE:
                castSpellOnSelf(SPELL_ROLTALL_HEAT_WAVE);
                scriptEvents.addEvent(EVENT_ROLTALL_HEAT_WAVE, 40000);
                break;
            case EVENT_ROLTALL_BURNING_SLAG:
                if (mSlagLeft == 0)
                    mSlagLeft = 6;
                castOnRandomPlayer(SPELL_ROLTALL_BURNING_SLAG);
                scriptEvents.addEvent(EVENT_ROLTALL_BURNING_SLAG, --mSlagLeft > 0 ? 550 : 28000);
                break;
            default:
                break;
        }
    }

private:
    void shuffleLanes()
    {
        for (uint8_t i = 0; i < 3; ++i)
            mLaneOrder[i] = i;

        for (uint8_t i = 2; i > 0; --i)
        {
            uint8_t const j = static_cast<uint8_t>(Util::getRandomUInt(i));
            uint8_t const swap = mLaneOrder[i];
            mLaneOrder[i] = mLaneOrder[j];
            mLaneOrder[j] = swap;
        }
    }

    uint8_t mNextLane = 0;
    uint8_t mLaneOrder[3] = { 0, 1, 2 };
    uint32_t mSlagLeft = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fiery Boulder, rolls along its bridge lane

class BloodmaulSlagMinesFieryBoulderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BloodmaulSlagMinesFieryBoulderAI(c); }
    explicit BloodmaulSlagMinesFieryBoulderAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        for (BloodmaulBoulderLane const& lane : BloodmaulBoulderLanes)
        {
            if (lane.entry == pCreature->getEntry())
                mLane = &lane;
        }
    }

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mLane == nullptr)
            return;

        if (!mRolling)
        {
            mRolling = true;
            castSpellOnSelf(SPELL_BOULDER_IMPACT, true);
            movePoint(BOULDER_MOVE_ACROSS, mLane->end);
        }

        mTrampleTimer += static_cast<uint32_t>(time_passed);
        if (mTrampleTimer >= 500)
        {
            mTrampleTimer = 0;
            // squared range, 5 yards
            _castOnInrangePlayersWithinDist(0.0f, 25.0f, SPELL_BOULDER_TRAMPLE, true);
        }
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type != POINT_MOTION_TYPE || mLane == nullptr)
            return;

        // on heroic the boulder rolls back once
        if (id == BOULDER_MOVE_ACROSS && isHeroic())
            movePoint(BOULDER_MOVE_BACK, mLane->start);
        else
            despawn(200, 0);
    }

private:
    BloodmaulBoulderLane const* mLane = nullptr;
    bool mRolling = false;
    uint32_t mTrampleTimer = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gug'rokk

class BloodmaulSlagMinesGugrokkAI : public BloodmaulSlagMinesBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BloodmaulSlagMinesGugrokkAI(c); }
    explicit BloodmaulSlagMinesGugrokkAI(Creature* pCreature) : BloodmaulSlagMinesBossAI(pCreature, DATA_GUGROKK) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GUGROKK_MOLTEN_BLAST, Util::getRandomUInt(5000, 7000));
        scriptEvents.addEvent(EVENT_GUGROKK_MAGMA_ERUPTION, 10000);
        scriptEvents.addEvent(EVENT_GUGROKK_UNSTABLE_SLAG, 20000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_GUGROKK_FLAME_BUFFET, 6000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GUGROKK_MOLTEN_BLAST:
                castSpellOnVictim(SPELL_GUGROKK_MOLTEN_BLAST);
                scriptEvents.addEvent(EVENT_GUGROKK_MOLTEN_BLAST, Util::getRandomUInt(8000, 9000));
                break;
            case EVENT_GUGROKK_MAGMA_ERUPTION:
                castOnRandomPlayer(SPELL_GUGROKK_MAGMA_ERUPTION);
                scriptEvents.addEvent(EVENT_GUGROKK_MAGMA_ERUPTION, Util::getRandomUInt(13000, 18000));
                break;
            case EVENT_GUGROKK_UNSTABLE_SLAG:
            {
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Gug'rokk summons an Unstable Slag!");
                float const angle = static_cast<float>(Util::getRandomUInt(359)) * 0.0174533f;
                callAdd(NPC_UNSTABLE_SLAG, nearSelf(7.0f * std::cos(angle), 7.0f * std::sin(angle)));
                scriptEvents.addEvent(EVENT_GUGROKK_UNSTABLE_SLAG, 20000);
                break;
            }
            case EVENT_GUGROKK_FLAME_BUFFET:
                castSpellOnVictim(SPELL_GUGROKK_FLAME_BUFFET);
                scriptEvents.addEvent(EVENT_GUGROKK_FLAME_BUFFET, Util::getRandomUInt(10000, 19000));
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Unstable Slag, empowers itself when left alive

class BloodmaulSlagMinesUnstableSlagAI : public BloodmaulSlagMinesCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BloodmaulSlagMinesUnstableSlagAI(c); }
    explicit BloodmaulSlagMinesUnstableSlagAI(Creature* pCreature) : BloodmaulSlagMinesCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_SLAG_EMPOWER, 12000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId == EVENT_SLAG_EMPOWER)
            castSpellOnSelf(SPELL_SLAG_EMPOWERED_FLAMES);
    }
};

void SetupBloodmaulSlagMines(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLOODMAUL_SLAG_MINES, &BloodmaulSlagMinesInstanceScript::Create);

    mgr->register_creature_script(BOSS_CRUSHTO, &BloodmaulSlagMinesCrushtoAI::Create);
    mgr->register_creature_script(BOSS_GOGDUH, &BloodmaulSlagMinesGogduhAI::Create);
    mgr->register_creature_script(BOSS_MAGMOLATUS, &BloodmaulSlagMinesMagmolatusAI::Create);
    mgr->register_creature_script(BOSS_ROLTALL, &BloodmaulSlagMinesRoltallAI::Create);
    mgr->register_creature_script(BOSS_GUGROKK, &BloodmaulSlagMinesGugrokkAI::Create);

    mgr->register_creature_script(NPC_RUINATION, &BloodmaulSlagMinesRuinationAI::Create);
    mgr->register_creature_script(NPC_CALAMITY, &BloodmaulSlagMinesCalamityAI::Create);
    mgr->register_creature_script(NPC_FIERY_BOULDER_LEFT, &BloodmaulSlagMinesFieryBoulderAI::Create);
    mgr->register_creature_script(NPC_FIERY_BOULDER_MIDDLE, &BloodmaulSlagMinesFieryBoulderAI::Create);
    mgr->register_creature_script(NPC_FIERY_BOULDER_RIGHT, &BloodmaulSlagMinesFieryBoulderAI::Create);
    mgr->register_creature_script(NPC_UNSTABLE_SLAG, &BloodmaulSlagMinesUnstableSlagAI::Create);
}
