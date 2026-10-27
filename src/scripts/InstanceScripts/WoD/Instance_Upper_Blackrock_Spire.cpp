/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Upper_Blackrock_Spire.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <vector>

enum UpperBlackrockSpireEvents
{
    EVENT_UBRS_RUNE_CHECK = 1,

    EVENT_GORASHAN_BLADES_OF_STEEL,
    EVENT_GORASHAN_SHRAPNEL_NOVA,
    EVENT_GORASHAN_LODESTONE_SPIKE,
    EVENT_GORASHAN_SEAL_CONDUIT,
    EVENT_GORASHAN_CACOPHONY,

    EVENT_KYRAK_FIXATION,
    EVENT_KYRAK_REJUVENATING_SERUM,
    EVENT_KYRAK_VILEBLOOD_SERUM,
    EVENT_KYRAK_TOXIC_FUMES,

    EVENT_UBRS_DRAKONID_ERUPTION,

    EVENT_THARBEK_IRON_AXE,
    EVENT_THARBEK_IRON_REAVER,
    EVENT_THARBEK_IMBUED_IRON_AXE,

    EVENT_UBRS_SKYREAVER_SPIT,
    EVENT_UBRS_SKYREAVER_BREATH,

    EVENT_RAGEWING_MAGMA_SPIT,
    EVENT_RAGEWING_RIPPING_CLAW,
    EVENT_RAGEWING_ENGULFING_FIRE,
    EVENT_RAGEWING_FIRE_STORM,
    EVENT_RAGEWING_BURNING_RAGE,

    EVENT_ZAELA_DESTRUCTIVE_SMITE,
    EVENT_ZAELA_REBOUNDING_BLADE,
    EVENT_ZAELA_BLACK_IRON_CYCLONE,

    EVENT_UBRS_RIDER_CHAIN_PULL,
    EVENT_UBRS_RIDER_DRAGON_ROAR
};

enum GorashanPhases
{
    PHASE_GORASHAN_NORMAL  = 1,
    PHASE_GORASHAN_CONDUIT = 2
};

enum RagewingPhases
{
    PHASE_RAGEWING_GROUND = 1,
    PHASE_RAGEWING_AIR    = 2
};

static std::vector<uint32_t> const runeGlowGuardEntries =
{
    NPC_UBRS_BLACK_IRON_BERSERKER,
    NPC_UBRS_BLACK_IRON_LEADBELCHER,
    NPC_UBRS_BLACK_IRON_WARCASTER,
    NPC_UBRS_BLACK_IRON_GRUNT,
    NPC_UBRS_RAGEMAW_WORG
};

static LocationVector const gorashanFieldSpots[2] =
{
    { 162.24f, -276.85f, 95.0f },
    { 126.11f, -240.43f, 95.0f }
};

static LocationVector const kyrakDrakonidSpots[2] =
{
    { 86.39f, -328.90f, 91.52f, 1.9f },
    { 69.96f, -328.22f, 91.54f, 0.5f }
};

static LocationVector const tharbekGateSpots[5] =
{
    { 205.8f, -420.40f, 110.91f, 3.1f },
    { 205.6f, -416.25f, 110.91f, 3.1f },
    { 205.6f, -424.98f, 110.91f, 3.1f },
    { 205.7f, -411.77f, 110.91f, 3.1f },
    { 205.5f, -428.52f, 110.91f, 3.1f }
};

static LocationVector const ragewingWhelpSpots[10] =
{
    { -50.77f, -417.34f, 137.21f, 0.72f },
    { -40.52f, -418.34f, 137.51f, 0.55f },
    { -47.92f, -417.35f, 137.07f, 5.32f },
    { -57.96f, -423.62f, 137.64f, 0.21f },
    { -57.86f, -420.05f, 137.63f, 0.13f },
    { -55.52f, -421.26f, 137.51f, 0.68f },
    { -49.33f, -419.15f, 137.62f, 0.07f },
    { -52.19f, -420.65f, 137.30f, 1.61f },
    { -45.50f, -413.33f, 142.38f, 0.05f },
    { -36.54f, -411.57f, 137.41f, 0.05f }
};

static LocationVector const zaelaPlatformCenter = { 20.648f, -122.817f, 97.819f };

class UpperBlackrockSpireInstanceScript : public InstanceScript
{
public:
    explicit UpperBlackrockSpireInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(UpperBlackrockSpireEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new UpperBlackrockSpireInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        if (pCreature->getEntry() == NPC_UBRS_RUNE_GLOW && pCreature->isAlive())
        {
            ++mRuneGlowCount;
            refreshDoors();
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_UBRS_ENTRANCE_AREA_EXIT:
            case GO_UBRS_GORASHAN_DOOR_IN:
            case GO_UBRS_GORASHAN_DOOR_OUT:
            case GO_UBRS_KYRAK_EXIT_1:
            case GO_UBRS_KYRAK_EXIT_2:
            case GO_UBRS_THARBEK_SPAWN_GATE:
            case GO_UBRS_THARBEK_ROOM_GATE:
            case GO_UBRS_THARBEK_EXIT_GATE:
            case GO_UBRS_RAGEWING_ENTRANCE:
            case GO_UBRS_RAGEWING_EXIT:
                mDoorGuids.push_back(pGameObject->GetNewGUID());
                applyDoorState(pGameObject);
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t /*bossId*/, uint32_t /*state*/) override
    {
        refreshDoors();
    }

    void setLocalData(uint32_t type, uint32_t /*data*/) override
    {
        if (type != DATA_UBRS_RUNE_DISABLED)
            return;

        ++mRunesDisabled;
        refreshDoors();
    }

    uint32_t getLocalData(uint32_t type) const override
    {
        return type == DATA_UBRS_RUNE_DISABLED ? mRunesDisabled : 0;
    }

private:
    // the entrance hall opens once every rune glow is down, or the warcaster was already engaged
    bool areRunesDisabled() const
    {
        if (getBossState(DATA_GORASHAN) != NotStarted && getBossState(DATA_GORASHAN) != InvalidState)
            return true;

        return mRunesDisabled >= mRuneGlowCount;
    }

    bool isDoorOpen(uint32_t entry) const
    {
        switch (entry)
        {
            case GO_UBRS_ENTRANCE_AREA_EXIT:
                return areRunesDisabled();
            case GO_UBRS_GORASHAN_DOOR_IN:
                return areRunesDisabled() && getBossState(DATA_GORASHAN) != InProgress;
            case GO_UBRS_GORASHAN_DOOR_OUT:
                return getBossState(DATA_GORASHAN) == Performed && getBossState(DATA_KYRAK) != InProgress;
            case GO_UBRS_KYRAK_EXIT_1:
            case GO_UBRS_KYRAK_EXIT_2:
                return getBossState(DATA_KYRAK) == Performed;
            case GO_UBRS_THARBEK_SPAWN_GATE:
                return getBossState(DATA_THARBEK) == InProgress || getBossState(DATA_THARBEK) == Performed;
            case GO_UBRS_THARBEK_ROOM_GATE:
                return getBossState(DATA_THARBEK) != InProgress;
            case GO_UBRS_THARBEK_EXIT_GATE:
                return getBossState(DATA_THARBEK) == Performed;
            case GO_UBRS_RAGEWING_ENTRANCE:
                return getBossState(DATA_RAGEWING) != InProgress;
            case GO_UBRS_RAGEWING_EXIT:
                return getBossState(DATA_RAGEWING) == Performed;
            default:
                return true;
        }
    }

    void applyDoorState(GameObject* pDoor)
    {
        pDoor->setState(isDoorOpen(pDoor->getEntry()) ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    void refreshDoors()
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoorState(pDoor);
        }
    }

    std::vector<WoWGuid> mDoorGuids;
    uint32_t mRuneGlowCount = 0;
    uint32_t mRunesDisabled = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rune Glow, goes out once its guards in the entrance hall are dead

class UpperBlackrockSpireRuneGlowAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UpperBlackrockSpireRuneGlowAI(c); }
    explicit UpperBlackrockSpireRuneGlowAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        scriptEvents.addEvent(EVENT_UBRS_RUNE_CHECK, 2000);
    }

    // leaving combat clears all events, keep watching the guards
    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_UBRS_RUNE_CHECK, 2000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (scriptEvents.getFinishedEvent() != EVENT_UBRS_RUNE_CHECK)
            return;

        if (hasLivingGuard())
        {
            mGuarded = true;
        }
        else if (mGuarded || isPlayerClose())
        {
            if (getInstanceScript() != nullptr)
                getInstanceScript()->setLocalData(DATA_UBRS_RUNE_DISABLED, 1);

            despawn(500, 0);
            return;
        }

        scriptEvents.addEvent(EVENT_UBRS_RUNE_CHECK, 2000);
    }

private:
    bool hasLivingGuard()
    {
        for (uint32_t entry : runeGlowGuardEntries)
        {
            std::list<Creature*> guards;
            GetCreatureListWithEntryInGrid(guards, entry, 15.0f);
            for (Creature* pGuard : guards)
            {
                if (pGuard->isAlive())
                    return true;
            }
        }
        return false;
    }

    bool isPlayerClose()
    {
        Player* pPlayer = getNearestPlayer();
        return pPlayer != nullptr && pPlayer->isAlive() && getRangeToObject(pPlayer) <= 15.0f;
    }

    bool mGuarded = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Orebender Gor'ashan

class UpperBlackrockSpireGorashanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UpperBlackrockSpireGorashanAI(c); }
    explicit UpperBlackrockSpireGorashanAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_GORASHAN, EncounterStates::InProgress);

        // he never leaves his platform
        setRooted(true);
        mPhase = PHASE_GORASHAN_NORMAL;
        mCacophonies = 0;

        summonCreature(NPC_UBRS_LIGHTNING_FIELD, gorashanFieldSpots[0]);
        if (isHeroic())
            summonCreature(NPC_UBRS_LIGHTNING_FIELD, gorashanFieldSpots[1]);

        scriptEvents.addEvent(EVENT_GORASHAN_BLADES_OF_STEEL, 1000, PHASE_GORASHAN_NORMAL);
        scriptEvents.addEvent(EVENT_GORASHAN_SHRAPNEL_NOVA, 10000, PHASE_GORASHAN_NORMAL);
        scriptEvents.addEvent(EVENT_GORASHAN_LODESTONE_SPIKE, 14000, PHASE_GORASHAN_NORMAL);
        scriptEvents.addEvent(EVENT_GORASHAN_SEAL_CONDUIT, 30000, PHASE_GORASHAN_NORMAL);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        setRooted(false);
        mPhase = PHASE_GORASHAN_NORMAL;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GORASHAN, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_GORASHAN, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mPhase);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GORASHAN_BLADES_OF_STEEL:
                castSpellOnVictim(SPELL_GORASHAN_BLADES_OF_STEEL);
                scriptEvents.addEvent(EVENT_GORASHAN_BLADES_OF_STEEL, 2000, PHASE_GORASHAN_NORMAL);
                break;
            case EVENT_GORASHAN_SHRAPNEL_NOVA:
                castSpellAOE(SPELL_GORASHAN_SHRAPNEL_NOVA);
                scriptEvents.addEvent(EVENT_GORASHAN_SHRAPNEL_NOVA, 15000, PHASE_GORASHAN_NORMAL);
                break;
            case EVENT_GORASHAN_LODESTONE_SPIKE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GORASHAN_LODESTONE_SPIKE);
                scriptEvents.addEvent(EVENT_GORASHAN_LODESTONE_SPIKE, 30000, PHASE_GORASHAN_NORMAL);
                break;
            case EVENT_GORASHAN_SEAL_CONDUIT:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Orebender Gor'ashan seals a Rune Conduit!");
                castSpellOnSelf(SPELL_GORASHAN_SEAL_CONDUIT, true);
                mPhase = PHASE_GORASHAN_CONDUIT;
                mCacophonies = 0;
                scriptEvents.addEvent(EVENT_GORASHAN_CACOPHONY, 1000, PHASE_GORASHAN_CONDUIT);
                break;
            case EVENT_GORASHAN_CACOPHONY:
                castSpellAOE(SPELL_GORASHAN_THUNDERING_CACOPHONY);

                // the sealed conduit powers four cacophonies, then the normal kit resumes
                if (++mCacophonies >= 4)
                {
                    mPhase = PHASE_GORASHAN_NORMAL;
                    scriptEvents.addEvent(EVENT_GORASHAN_SEAL_CONDUIT, 30000, PHASE_GORASHAN_NORMAL);
                }
                else
                {
                    scriptEvents.addEvent(EVENT_GORASHAN_CACOPHONY, 3000, PHASE_GORASHAN_CONDUIT);
                }
                break;
            default:
                break;
        }
    }

private:
    uint32_t mPhase = PHASE_GORASHAN_NORMAL;
    uint32_t mCacophonies = 0;
};

// Lightning Field, pulses electricity over the platform
class UpperBlackrockSpireLightningFieldAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UpperBlackrockSpireLightningFieldAI(c); }
    explicit UpperBlackrockSpireLightningFieldAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_LIGHTNING_FIELD_ELECTRIC_PULSE, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kyrak

class UpperBlackrockSpireKyrakAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UpperBlackrockSpireKyrakAI(c); }
    explicit UpperBlackrockSpireKyrakAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_KYRAK, EncounterStates::InProgress);

        for (LocationVector const& spot : kyrakDrakonidSpots)
            summonCreature(NPC_UBRS_DRAKONID_MONSTROSITY, spot);

        scriptEvents.addEvent(EVENT_KYRAK_FIXATION, 8000);
        scriptEvents.addEvent(EVENT_KYRAK_REJUVENATING_SERUM, 26000);

        // heroic uses the serum from the pull, normal from half health
        mSerumActive = isHeroic();
        if (isHeroic())
        {
            scriptEvents.addEvent(EVENT_KYRAK_VILEBLOOD_SERUM, 10000);
            scriptEvents.addEvent(EVENT_KYRAK_TOXIC_FUMES, 12000);
        }
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KYRAK, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_KYRAK, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mSerumActive && _getHealthPercent() <= 50)
        {
            mSerumActive = true;
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Kyrak drinks his Vileblood Serum!");
            scriptEvents.addEvent(EVENT_KYRAK_VILEBLOOD_SERUM, 5000);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KYRAK_FIXATION:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent))))
                    castSpell(pTarget, SPELL_KYRAK_DEBILITATING_FIXATION);
                scriptEvents.addEvent(EVENT_KYRAK_FIXATION, 20000);
                break;
            case EVENT_KYRAK_REJUVENATING_SERUM:
                castSpellOnSelf(SPELL_KYRAK_REJUVENATING_SERUM);
                scriptEvents.addEvent(EVENT_KYRAK_REJUVENATING_SERUM, 33000);
                break;
            case EVENT_KYRAK_VILEBLOOD_SERUM:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_KYRAK_VILEBLOOD_SERUM);
                scriptEvents.addEvent(EVENT_KYRAK_VILEBLOOD_SERUM, 15000);
                break;
            case EVENT_KYRAK_TOXIC_FUMES:
                castSpellAOE(SPELL_KYRAK_SALVE_OF_TOXIC_FUMES);
                scriptEvents.addEvent(EVENT_KYRAK_TOXIC_FUMES, 12000);
                break;
            default:
                break;
        }
    }

private:
    bool mSerumActive = false;
};

// Drakonid Monstrosity, Kyrak's bodyguard
class UpperBlackrockSpireDrakonidAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UpperBlackrockSpireDrakonidAI(c); }
    explicit UpperBlackrockSpireDrakonidAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        castSpellOnSelf(SPELL_DRAKONID_MONSTROUS_CLAWS, true);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_UBRS_DRAKONID_ERUPTION, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_UBRS_DRAKONID_ERUPTION)
        {
            castSpellAOE(SPELL_DRAKONID_ERUPTION);
            scriptEvents.addEvent(EVENT_UBRS_DRAKONID_ERUPTION, 18000);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Commander Tharbek

class UpperBlackrockSpireTharbekAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UpperBlackrockSpireTharbekAI(c); }
    explicit UpperBlackrockSpireTharbekAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_THARBEK, EncounterStates::InProgress);

        mSecondWaveCalled = false;
        mSkyreaverCalled = false;

        // first wave storms out of the opened spawn gate
        summonCreature(NPC_UBRS_BLACK_IRON_LEADBELCHER, tharbekGateSpots[0]);
        summonCreature(NPC_UBRS_BLACK_IRON_LEADBELCHER, tharbekGateSpots[1]);
        summonCreature(NPC_UBRS_RAGEMAW_WORG, tharbekGateSpots[2]);
        summonCreature(NPC_UBRS_RAGEMAW_WORG, tharbekGateSpots[3]);

        scriptEvents.addEvent(EVENT_THARBEK_IMBUED_IRON_AXE, 10000);
        scriptEvents.addEvent(EVENT_THARBEK_IRON_REAVER, 15000);
        scriptEvents.addEvent(EVENT_THARBEK_IRON_AXE, 20000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_THARBEK, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_THARBEK, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mSecondWaveCalled && _getHealthPercent() <= 70)
        {
            mSecondWaveCalled = true;
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Commander Tharbek calls for reinforcements!");
            summonCreature(NPC_UBRS_BLACK_IRON_BERSERKER, tharbekGateSpots[0]);
            summonCreature(NPC_UBRS_BLACK_IRON_BERSERKER, tharbekGateSpots[1]);
            summonCreature(NPC_UBRS_BLACK_IRON_DREADWEAVER, tharbekGateSpots[2]);
            summonCreature(NPC_UBRS_BLACK_IRON_DREADWEAVER, tharbekGateSpots[3]);
        }

        if (!mSkyreaverCalled && _getHealthPercent() <= 50)
        {
            mSkyreaverCalled = true;
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Commander Tharbek flies into an Iron Rage and calls his Ironbarb Skyreaver!");
            castSpellOnSelf(SPELL_THARBEK_IRON_RAGE, true);
            summonCreature(NPC_UBRS_IRONBARB_SKYREAVER, tharbekGateSpots[4]);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_THARBEK_IMBUED_IRON_AXE:
                castSpellOnSelf(SPELL_THARBEK_IMBUED_IRON_AXE, true);
                scriptEvents.addEvent(EVENT_THARBEK_IMBUED_IRON_AXE, 25000);
                break;
            case EVENT_THARBEK_IRON_REAVER:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_THARBEK_IRON_REAVER);
                scriptEvents.addEvent(EVENT_THARBEK_IRON_REAVER, 30000);
                break;
            case EVENT_THARBEK_IRON_AXE:
                castSpellOnVictim(SPELL_THARBEK_IRON_AXE);
                scriptEvents.addEvent(EVENT_THARBEK_IRON_AXE, 20000);
                break;
            default:
                break;
        }
    }

private:
    bool mSecondWaveCalled = false;
    bool mSkyreaverCalled = false;
};

// Ironbarb Skyreaver, Tharbek's drake joining on the ground
class UpperBlackrockSpireSkyreaverAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UpperBlackrockSpireSkyreaverAI(c); }
    explicit UpperBlackrockSpireSkyreaverAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_UBRS_SKYREAVER_SPIT, 5000);
        scriptEvents.addEvent(EVENT_UBRS_SKYREAVER_BREATH, 15000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_UBRS_SKYREAVER_SPIT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SKYREAVER_NOXIOUS_SPIT);
                scriptEvents.addEvent(EVENT_UBRS_SKYREAVER_SPIT, 20000);
                break;
            case EVENT_UBRS_SKYREAVER_BREATH:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SKYREAVER_INCINERATING_BREATH);
                scriptEvents.addEvent(EVENT_UBRS_SKYREAVER_BREATH, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ragewing the Untamed

class UpperBlackrockSpireRagewingAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UpperBlackrockSpireRagewingAI(c); }
    explicit UpperBlackrockSpireRagewingAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_RAGEWING, EncounterStates::InProgress);

        mPhase = PHASE_RAGEWING_GROUND;
        mNextAirPhase = 70;
        mStormCasts = 0;

        scriptEvents.addEvent(EVENT_RAGEWING_RIPPING_CLAW, 6000, PHASE_RAGEWING_GROUND);
        scriptEvents.addEvent(EVENT_RAGEWING_MAGMA_SPIT, 8000, PHASE_RAGEWING_GROUND);
        scriptEvents.addEvent(EVENT_RAGEWING_ENGULFING_FIRE, 13000, PHASE_RAGEWING_GROUND);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        setRooted(false);
        _setMeleeDisabled(false);
        mPhase = PHASE_RAGEWING_GROUND;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RAGEWING, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_RAGEWING, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mPhase);

        if (mPhase == PHASE_RAGEWING_GROUND && mNextAirPhase > 0 && _getHealthPercent() <= mNextAirPhase)
            takeToTheSkies();

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_RAGEWING_RIPPING_CLAW:
                castSpellOnVictim(SPELL_RAGEWING_RIPPING_CLAW);
                scriptEvents.addEvent(EVENT_RAGEWING_RIPPING_CLAW, 15000, PHASE_RAGEWING_GROUND);
                break;
            case EVENT_RAGEWING_MAGMA_SPIT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_RAGEWING_MAGMA_SPIT);
                scriptEvents.addEvent(EVENT_RAGEWING_MAGMA_SPIT, 17000, PHASE_RAGEWING_GROUND);
                break;
            case EVENT_RAGEWING_ENGULFING_FIRE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Ragewing the Untamed breathes Engulfing Fire!");
                castSpellAOE(SPELL_RAGEWING_ENGULFING_FIRE);
                scriptEvents.addEvent(EVENT_RAGEWING_ENGULFING_FIRE, 22000, PHASE_RAGEWING_GROUND);
                break;
            case EVENT_RAGEWING_FIRE_STORM:
                castSpellAOE(SPELL_RAGEWING_FIRE_STORM);
                if (++mStormCasts >= 6)
                    land();
                else
                    scriptEvents.addEvent(EVENT_RAGEWING_FIRE_STORM, 2500, PHASE_RAGEWING_AIR);
                break;
            case EVENT_RAGEWING_BURNING_RAGE:
                castSpellOnSelf(SPELL_RAGEWING_BURNING_RAGE);
                scriptEvents.addEvent(EVENT_RAGEWING_BURNING_RAGE, 15000, PHASE_RAGEWING_GROUND);
                break;
            default:
                break;
        }
    }

private:
    // air phase at 70% and 40%, he stays out of melee and rains fire while the whelps attack
    void takeToTheSkies()
    {
        mPhase = PHASE_RAGEWING_AIR;
        mNextAirPhase = mNextAirPhase == 70 ? 40 : 0;
        mStormCasts = 0;

        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Ragewing the Untamed takes to the skies and calls his whelps!");
        setRooted(true);
        _setMeleeDisabled(true);

        for (LocationVector const& spot : ragewingWhelpSpots)
            summonCreature(NPC_UBRS_RAGEWING_WHELP, spot);

        scriptEvents.addEvent(EVENT_RAGEWING_FIRE_STORM, 2000, PHASE_RAGEWING_AIR);
    }

    void land()
    {
        mPhase = PHASE_RAGEWING_GROUND;
        setRooted(false);
        _setMeleeDisabled(false);

        // after the last flight he stays enraged on the ground
        if (mNextAirPhase == 0)
            scriptEvents.addEvent(EVENT_RAGEWING_BURNING_RAGE, 5000, PHASE_RAGEWING_GROUND);
    }

    uint32_t mPhase = PHASE_RAGEWING_GROUND;
    int32_t mNextAirPhase = 70;
    uint32_t mStormCasts = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Warlord Zaela

class UpperBlackrockSpireZaelaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UpperBlackrockSpireZaelaAI(c); }
    explicit UpperBlackrockSpireZaelaAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ZAELA, EncounterStates::InProgress);

        mRidersCalled = false;
        scriptEvents.addEvent(EVENT_ZAELA_REBOUNDING_BLADE, 1000);
        scriptEvents.addEvent(EVENT_ZAELA_BLACK_IRON_CYCLONE, 5000);
        scriptEvents.addEvent(EVENT_ZAELA_DESTRUCTIVE_SMITE, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ZAELA, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ZAELA, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mRidersCalled && _getHealthPercent() <= 60)
        {
            mRidersCalled = true;
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Warlord Zaela calls her Black Iron Wyrm Riders!");

            // three riders drop around the platform center
            for (uint32_t i = 0; i < 3; ++i)
            {
                float const angle = Util::getRandomFloat(0.0f, 6.28f);
                summonCreature(NPC_UBRS_BLACK_IRON_WYRM_RIDER, zaelaPlatformCenter.x + 15.0f * std::cos(angle), zaelaPlatformCenter.y + 15.0f * std::sin(angle), zaelaPlatformCenter.z, angle);
            }
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ZAELA_REBOUNDING_BLADE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_ZAELA_REBOUNDING_BLADE);
                scriptEvents.addEvent(EVENT_ZAELA_REBOUNDING_BLADE, 10500);
                break;
            case EVENT_ZAELA_BLACK_IRON_CYCLONE:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Warlord Zaela spins into a Black Iron Cyclone!");
                castSpellOnSelf(SPELL_ZAELA_BLACK_IRON_CYCLONE, true);
                scriptEvents.addEvent(EVENT_ZAELA_BLACK_IRON_CYCLONE, 19500);
                break;
            case EVENT_ZAELA_DESTRUCTIVE_SMITE:
                castSpellOnVictim(isHeroic() ? SPELL_ZAELA_DESTRUCTIVE_SMITE_HEROIC : SPELL_ZAELA_DESTRUCTIVE_SMITE);
                scriptEvents.addEvent(EVENT_ZAELA_DESTRUCTIVE_SMITE, 15500);
                break;
            default:
                break;
        }
    }

private:
    bool mRidersCalled = false;
};

// Black Iron Wyrm Rider, chains players to itself
class UpperBlackrockSpireWyrmRiderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UpperBlackrockSpireWyrmRiderAI(c); }
    explicit UpperBlackrockSpireWyrmRiderAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_UBRS_RIDER_CHAIN_PULL, 5000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_UBRS_RIDER_DRAGON_ROAR, 6000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_UBRS_RIDER_CHAIN_PULL:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent))))
                    castSpell(pTarget, SPELL_WYRM_RIDER_CHAIN_PULL);
                scriptEvents.addEvent(EVENT_UBRS_RIDER_CHAIN_PULL, 15000);
                break;
            case EVENT_UBRS_RIDER_DRAGON_ROAR:
                castSpellAOE(SPELL_WYRM_RIDER_DRAGON_ROAR);
                scriptEvents.addEvent(EVENT_UBRS_RIDER_DRAGON_ROAR, 6000);
                break;
            default:
                break;
        }
    }
};

void SetupUpperBlackrockSpire(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_UPPER_BLACKROCK_SPIRE, &UpperBlackrockSpireInstanceScript::Create);

    mgr->register_creature_script(BOSS_OREBENDER_GORASHAN, &UpperBlackrockSpireGorashanAI::Create);
    mgr->register_creature_script(BOSS_KYRAK, &UpperBlackrockSpireKyrakAI::Create);
    mgr->register_creature_script(BOSS_COMMANDER_THARBEK, &UpperBlackrockSpireTharbekAI::Create);
    mgr->register_creature_script(BOSS_RAGEWING_THE_UNTAMED, &UpperBlackrockSpireRagewingAI::Create);
    mgr->register_creature_script(BOSS_WARLORD_ZAELA, &UpperBlackrockSpireZaelaAI::Create);

    mgr->register_creature_script(NPC_UBRS_RUNE_GLOW, &UpperBlackrockSpireRuneGlowAI::Create);
    mgr->register_creature_script(NPC_UBRS_LIGHTNING_FIELD, &UpperBlackrockSpireLightningFieldAI::Create);
    mgr->register_creature_script(NPC_UBRS_DRAKONID_MONSTROSITY, &UpperBlackrockSpireDrakonidAI::Create);
    mgr->register_creature_script(NPC_UBRS_IRONBARB_SKYREAVER, &UpperBlackrockSpireSkyreaverAI::Create);
    mgr->register_creature_script(NPC_UBRS_BLACK_IRON_WYRM_RIDER, &UpperBlackrockSpireWyrmRiderAI::Create);
}
