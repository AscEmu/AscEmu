/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Everbloom.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <vector>

enum TheEverbloomEvents
{
    EVENT_WITHERBARK_DRAIN = 1,
    EVENT_WITHERBARK_PARCHED_GASP,
    EVENT_WITHERBARK_AGITATED_WATER,
    EVENT_WITHERBARK_UNCHECKED_GROWTH,
    EVENT_WITHERBARK_GLOBULES,

    EVENT_EB_GLOBULE_FLOW,

    EVENT_GOLA_WATER_BOLT,
    EVENT_GOLA_REVITALIZING_WATERS,
    EVENT_GOLA_RAPID_TIDES,
    EVENT_TELU_NATURES_WRATH,
    EVENT_TELU_BRAMBLE_PATCH,
    EVENT_TELU_BRIARSKIN,
    EVENT_DULHU_RENDING_CHARGE,
    EVENT_DULHU_NOXIOUS_ERUPTION,
    EVENT_DULHU_GRASPING_VINE,
    EVENT_DULHU_SLASH,

    EVENT_XERITAC_TOXIC_BOLT,
    EVENT_XERITAC_SPIDERLING,
    EVENT_XERITAC_SPRAYER,
    EVENT_XERITAC_PALE_ONE,
    EVENT_XERITAC_BURSTER,
    EVENT_XERITAC_DESCEND,
    EVENT_XERITAC_VENOMOUS_STING,
    EVENT_XERITAC_GAS_VOLLEY,

    EVENT_EB_SPRAYER_SPRAY,
    EVENT_EB_PALE_ONE_SWIPE,

    EVENT_SOL_PARASITIC_GROWTH,
    EVENT_SOL_FIREBALL,
    EVENT_SOL_FIREBLOOM,
    EVENT_SOL_FROSTBOLT,
    EVENT_SOL_FROZEN_RAIN,
    EVENT_SOL_ARCANE_BURST,

    EVENT_YALNU_COLOSSAL_BLOW,
    EVENT_YALNU_ENTANGLEMENT,
    EVENT_YALNU_ENTANGLEMENT_PLAYER,
    EVENT_YALNU_FONT_OF_LIFE,
    EVENT_YALNU_GENESIS,
    EVENT_YALNU_GENESIS_BLOOM,

    EVENT_EB_MAGE_PRIMARY,
    EVENT_EB_MAGE_SECONDARY
};

enum TheEverbloomActions
{
    ACTION_EB_GLOBULE_ARRIVED = 1
};

enum WitherbarkPhases
{
    PHASE_WITHERBARK_BARK    = 1,
    PHASE_WITHERBARK_BRITTLE = 2
};

enum XeritacPhases
{
    PHASE_XERITAC_WEB    = 1,
    PHASE_XERITAC_GROUND = 2
};

static LocationVector const witherbarkGlobuleSpots[12] =
{
    { 458.76f, 1648.69f, 86.56f },
    { 434.33f, 1661.59f, 86.56f },
    { 474.81f, 1638.34f, 86.56f },
    { 422.24f, 1652.27f, 86.56f },
    { 443.61f, 1649.08f, 86.56f },
    { 404.96f, 1654.05f, 86.56f },
    { 366.53f, 1614.20f, 86.56f },
    { 376.22f, 1636.73f, 86.56f },
    { 369.49f, 1628.34f, 86.56f },
    { 387.49f, 1650.12f, 86.56f },
    { 362.70f, 1599.98f, 86.56f },
    { 349.98f, 1594.23f, 86.56f }
};

static LocationVector const xeritacAddSpots[5] =
{
    { 931.20f, 1441.587f, 85.600f, 3.289805f },
    { 908.84f, 1449.853f, 85.600f, 0.615524f },
    { 921.09f, 1456.374f, 85.600f, 5.465359f },
    { 938.89f, 1443.991f, 85.600f, 3.359706f },
    { 930.02f, 1434.803f, 85.600f, 2.946586f }
};

static LocationVector const xeritacPaleOneSpots[2] =
{
    { 926.44f, 1413.111f, 66.364f, 2.336323f },
    { 956.79f, 1448.274f, 64.274f, 3.184339f }
};

static LocationVector const yalnuAllySpots[7] =
{
    { 947.38f, -1235.11f, 181.25f, 0.68f },
    { 966.46f, -1214.60f, 181.25f, 4.11f },
    { 960.97f, -1208.64f, 181.24f, 4.22f },
    { 970.09f, -1236.66f, 181.25f, 2.47f },
    { 959.63f, -1238.97f, 181.25f, 1.62f },
    { 942.67f, -1212.62f, 181.35f, 5.44f },
    { 976.43f, -1223.22f, 181.33f, 3.08f }
};

static std::vector<uint32_t> const everbloomProtectorEntries = { BOSS_LIFE_WARDEN_GOLA, BOSS_EARTHSHAPER_TELU, BOSS_DULHU };

class EverbloomInstanceScript : public InstanceScript
{
public:
    explicit EverbloomInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheEverbloomEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new EverbloomInstanceScript(pMapMgr); }

    // a credited member falling before its partners must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && id == DATA_ANCIENT_PROTECTORS && getBossState(id) == EncounterStates::InProgress)
        {
            for (Creature* pMember : getCreatureSetForEntries(everbloomProtectorEntries))
            {
                if (pMember->isAlive())
                    return false;
            }
        }

        return InstanceScript::setBossState(id, state);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_EB_WITHERBARK_VINE_WALL_1:
            case GO_EB_WITHERBARK_VINE_WALL_2:
                mDoorGuids.push_back(pGameObject->GetNewGUID());
                applyDoorState(pGameObject);
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t /*bossId*/, uint32_t /*state*/) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoorState(pDoor);
        }
    }

private:
    void applyDoorState(GameObject* pDoor)
    {
        // both vine walls seal the Witherbark glade while he is engaged
        pDoor->setState(getBossState(DATA_WITHERBARK) != InProgress ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    std::vector<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Witherbark

class EverbloomWitherbarkAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomWitherbarkAI(c); }
    explicit EverbloomWitherbarkAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_WITHERBARK, EncounterStates::InProgress);

        mPhase = PHASE_WITHERBARK_BARK;
        mWater = 100;

        scriptEvents.addEvent(EVENT_WITHERBARK_DRAIN, 3000, PHASE_WITHERBARK_BARK);
        scriptEvents.addEvent(EVENT_WITHERBARK_AGITATED_WATER, 5000, PHASE_WITHERBARK_BARK);
        scriptEvents.addEvent(EVENT_WITHERBARK_UNCHECKED_GROWTH, 5000, PHASE_WITHERBARK_BARK);
        scriptEvents.addEvent(EVENT_WITHERBARK_PARCHED_GASP, 8000, PHASE_WITHERBARK_BARK);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        setRooted(false);
        _setMeleeDisabled(false);
        mPhase = PHASE_WITHERBARK_BARK;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_WITHERBARK, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_WITHERBARK, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
    }

    // a globule reaching the tree restores a quarter of its water
    void DoAction(int32_t action) override
    {
        if (action != ACTION_EB_GLOBULE_ARRIVED || mPhase != PHASE_WITHERBARK_BRITTLE)
            return;

        mWater += 25;
        if (mWater >= 100)
            leaveBrittleBark();
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
            case EVENT_WITHERBARK_DRAIN:
                mWater = mWater > 10 ? mWater - 10 : 0;
                if (mWater == 0)
                    enterBrittleBark();
                else
                    scriptEvents.addEvent(EVENT_WITHERBARK_DRAIN, 3000, PHASE_WITHERBARK_BARK);
                break;
            case EVENT_WITHERBARK_AGITATED_WATER:
                castSpellOnSelf(SPELL_WITHERBARK_AGITATED_WATER, true);
                scriptEvents.addEvent(EVENT_WITHERBARK_AGITATED_WATER, 6000, PHASE_WITHERBARK_BARK);
                break;
            case EVENT_WITHERBARK_UNCHECKED_GROWTH:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    summonCreature(NPC_EB_UNCHECKED_GROWTH, pTarget->GetPositionX() + 3.0f, pTarget->GetPositionY(), pTarget->GetPositionZ(), pTarget->GetOrientation(), TIMED_DESPAWN, 30000);
                scriptEvents.addEvent(EVENT_WITHERBARK_UNCHECKED_GROWTH, 11000, PHASE_WITHERBARK_BARK);
                break;
            case EVENT_WITHERBARK_PARCHED_GASP:
                castSpellOnVictim(SPELL_WITHERBARK_PARCHED_GASP);
                scriptEvents.addEvent(EVENT_WITHERBARK_PARCHED_GASP, 12000, PHASE_WITHERBARK_BARK);
                break;
            case EVENT_WITHERBARK_GLOBULES:
                // one globule out of every quarter of the pond
                summonCreature(NPC_EB_AQUEOUS_GLOBULE, witherbarkGlobuleSpots[Util::getRandomUInt(0, 3)]);
                summonCreature(NPC_EB_AQUEOUS_GLOBULE, witherbarkGlobuleSpots[Util::getRandomUInt(4, 7)]);
                summonCreature(NPC_EB_AQUEOUS_GLOBULE, witherbarkGlobuleSpots[Util::getRandomUInt(8, 11)]);
                scriptEvents.addEvent(EVENT_WITHERBARK_GLOBULES, 15000, PHASE_WITHERBARK_BRITTLE);
                break;
            default:
                break;
        }
    }

private:
    void enterBrittleBark()
    {
        mPhase = PHASE_WITHERBARK_BRITTLE;
        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Witherbark dries out and his bark turns brittle!");
        castSpellOnSelf(SPELL_WITHERBARK_BRITTLE_BARK, true);
        setRooted(true);
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_WITHERBARK_GLOBULES, 1000, PHASE_WITHERBARK_BRITTLE);
    }

    void leaveBrittleBark()
    {
        mPhase = PHASE_WITHERBARK_BARK;
        mWater = 100;
        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Witherbark is revitalized by the water!");
        _removeAura(SPELL_WITHERBARK_BRITTLE_BARK);
        setRooted(false);
        _setMeleeDisabled(false);
        scriptEvents.removeEvent(EVENT_WITHERBARK_GLOBULES);
        summons.despawnEntry(NPC_EB_AQUEOUS_GLOBULE);
        scriptEvents.addEvent(EVENT_WITHERBARK_DRAIN, 3000, PHASE_WITHERBARK_BARK);
    }

    uint32_t mPhase = PHASE_WITHERBARK_BARK;
    uint32_t mWater = 100;
};

// Aqueous Globule, flows to Witherbark and feeds him on arrival
class EverbloomAqueousGlobuleAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomAqueousGlobuleAI(c); }
    explicit EverbloomAqueousGlobuleAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* summoner) override
    {
        setReactState(REACT_PASSIVE);
        if (summoner != nullptr)
            moveTo(summoner->GetPositionX(), summoner->GetPositionY(), summoner->GetPositionZ(), false);

        scriptEvents.addEvent(EVENT_EB_GLOBULE_FLOW, 1000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (scriptEvents.getFinishedEvent() != EVENT_EB_GLOBULE_FLOW)
            return;

        Creature* pWitherbark = findNearestCreature(BOSS_WITHERBARK, 150.0f);
        if (pWitherbark == nullptr || !pWitherbark->isAlive())
            return;

        if (getRangeToObject(pWitherbark) <= 5.0f)
        {
            castSpellAOE(SPELL_AQUEOUS_GLOBULE_BURST, true);
            if (pWitherbark->GetScript())
                pWitherbark->GetScript()->DoAction(ACTION_EB_GLOBULE_ARRIVED);
            despawn(300, 0);
            return;
        }

        moveTo(pWitherbark->GetPositionX(), pWitherbark->GetPositionY(), pWitherbark->GetPositionZ(), false);
        scriptEvents.addEvent(EVENT_EB_GLOBULE_FLOW, 1000);
    }
};

// Unchecked Growth, roots its vines into a random player
class EverbloomUncheckedGrowthAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomUncheckedGrowthAI(c); }
    explicit EverbloomUncheckedGrowthAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        castSpellOnSelf(SPELL_UNCHECKED_GROWTH_VISUAL, true);

        if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
        {
            castSpell(pTarget, SPELL_UNCHECKED_GROWTH_VINES, true);
            addThreat(pTarget, 10000.0f);
            attackStart(pTarget);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ancient Protectors

// shared encounter handling of Gola, Telu and Dulhu
class EverbloomProtectorAI : public CreatureAIScript
{
public:
    explicit EverbloomProtectorAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());

        if (getInstanceScript()->getBossState(DATA_ANCIENT_PROTECTORS) != InProgress)
        {
            getInstanceScript()->setBossState(DATA_ANCIENT_PROTECTORS, EncounterStates::InProgress);

            // pulling one protector pulls the whole circle
            for (Creature* pProtector : livingProtectors())
            {
                if (pProtector != getCreature())
                    setZoneWideCombat(pProtector);
            }
        }

        scheduleAbilities();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        scriptEvents.resetEvents();

        if (getInstanceScript()->getBossState(DATA_ANCIENT_PROTECTORS) == Performed)
            return;

        getInstanceScript()->setBossState(DATA_ANCIENT_PROTECTORS, EncounterStates::Failed);

        // fallen protectors return for the next attempt
        for (Creature* pProtector : getInstanceScript()->getCreatureSetForEntries(everbloomProtectorEntries))
        {
            if (!pProtector->isAlive())
                pProtector->Despawn(1000, 5000);
        }
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        if (livingProtectors().empty())
            getInstanceScript()->setBossState(DATA_ANCIENT_PROTECTORS, EncounterStates::Performed);
    }

protected:
    virtual void scheduleAbilities() = 0;

    std::vector<Creature*> livingProtectors()
    {
        std::vector<Creature*> living;
        for (Creature* pProtector : getInstanceScript()->getCreatureSetForEntries(everbloomProtectorEntries))
        {
            if (pProtector->isAlive())
                living.push_back(pProtector);
        }
        return living;
    }

    Creature* weakestProtector()
    {
        Creature* pWeakest = nullptr;
        for (Creature* pProtector : livingProtectors())
        {
            if (pWeakest == nullptr || pProtector->getHealthPct() < pWeakest->getHealthPct())
                pWeakest = pProtector;
        }
        return pWeakest;
    }
};

class EverbloomGolaAI : public EverbloomProtectorAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomGolaAI(c); }
    explicit EverbloomGolaAI(Creature* pCreature) : EverbloomProtectorAI(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GOLA_WATER_BOLT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_GOLA_WATER_BOLT);
                scriptEvents.addEvent(EVENT_GOLA_WATER_BOLT, 3100);
                break;
            case EVENT_GOLA_REVITALIZING_WATERS:
                if (Creature* pAlly = weakestProtector())
                    castSpell(pAlly, SPELL_GOLA_REVITALIZING_WATERS);
                scriptEvents.addEvent(EVENT_GOLA_REVITALIZING_WATERS, 22000);
                break;
            case EVENT_GOLA_RAPID_TIDES:
                if (Creature* pAlly = weakestProtector())
                    castSpell(pAlly, SPELL_GOLA_RAPID_TIDES);
                scriptEvents.addEvent(EVENT_GOLA_RAPID_TIDES, 22000);
                break;
            default:
                break;
        }
    }

protected:
    void scheduleAbilities() override
    {
        scriptEvents.addEvent(EVENT_GOLA_WATER_BOLT, 1000);
        scriptEvents.addEvent(EVENT_GOLA_REVITALIZING_WATERS, 18000);
        scriptEvents.addEvent(EVENT_GOLA_RAPID_TIDES, 22000);
    }
};

class EverbloomTeluAI : public EverbloomProtectorAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomTeluAI(c); }
    explicit EverbloomTeluAI(Creature* pCreature) : EverbloomProtectorAI(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_TELU_NATURES_WRATH:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_TELU_NATURES_WRATH);
                scriptEvents.addEvent(EVENT_TELU_NATURES_WRATH, 3100);
                break;
            case EVENT_TELU_BRAMBLE_PATCH:
                castSpellAOE(SPELL_TELU_BRAMBLE_PATCH);
                scriptEvents.addEvent(EVENT_TELU_BRAMBLE_PATCH, 20000);
                break;
            case EVENT_TELU_BRIARSKIN:
                if (Creature* pAlly = weakestProtector())
                    castSpell(pAlly, SPELL_TELU_BRIARSKIN);
                scriptEvents.addEvent(EVENT_TELU_BRIARSKIN, 22000);
                break;
            default:
                break;
        }
    }

protected:
    void scheduleAbilities() override
    {
        scriptEvents.addEvent(EVENT_TELU_NATURES_WRATH, 1000);
        scriptEvents.addEvent(EVENT_TELU_BRAMBLE_PATCH, 12000);
        scriptEvents.addEvent(EVENT_TELU_BRIARSKIN, 26000);
    }
};

class EverbloomDulhuAI : public EverbloomProtectorAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomDulhuAI(c); }
    explicit EverbloomDulhuAI(Creature* pCreature) : EverbloomProtectorAI(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DULHU_RENDING_CHARGE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_DULHU_RENDING_CHARGE);
                scriptEvents.addEvent(EVENT_DULHU_RENDING_CHARGE, 20000);
                break;
            case EVENT_DULHU_NOXIOUS_ERUPTION:
                castSpellAOE(SPELL_DULHU_NOXIOUS_ERUPTION);
                scriptEvents.addEvent(EVENT_DULHU_NOXIOUS_ERUPTION, 18000);
                break;
            case EVENT_DULHU_GRASPING_VINE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_DULHU_GRASPING_VINE);
                scriptEvents.addEvent(EVENT_DULHU_SLASH, 2000);
                scriptEvents.addEvent(EVENT_DULHU_GRASPING_VINE, 22000);
                break;
            case EVENT_DULHU_SLASH:
                castSpellOnVictim(SPELL_DULHU_SLASH);
                break;
            default:
                break;
        }
    }

protected:
    void scheduleAbilities() override
    {
        scriptEvents.addEvent(EVENT_DULHU_RENDING_CHARGE, 6000);
        scriptEvents.addEvent(EVENT_DULHU_NOXIOUS_ERUPTION, 7000);
        scriptEvents.addEvent(EVENT_DULHU_GRASPING_VINE, 22000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Xeri'tac

class EverbloomXeritacAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomXeritacAI(c); }
    explicit EverbloomXeritacAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_XERITAC, EncounterStates::InProgress);

        // web phase, she stays on her web and sends the brood
        mPhase = PHASE_XERITAC_WEB;
        mSpiderlingKills = 0;
        setRooted(true);
        _setMeleeDisabled(true);

        scriptEvents.addEvent(EVENT_XERITAC_TOXIC_BOLT, 6000);
        scriptEvents.addEvent(EVENT_XERITAC_PALE_ONE, 20000);
        scriptEvents.addEvent(EVENT_XERITAC_SPIDERLING, 10000, PHASE_XERITAC_WEB);
        scriptEvents.addEvent(EVENT_XERITAC_SPRAYER, 10000, PHASE_XERITAC_WEB);
        scriptEvents.addEvent(EVENT_XERITAC_DESCEND, 90000, PHASE_XERITAC_WEB);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_XERITAC_BURSTER, 10000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        setRooted(false);
        _setMeleeDisabled(false);
        mPhase = PHASE_XERITAC_WEB;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_XERITAC, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_XERITAC, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    // four slain spiderlings bring her down from the web
    void OnSummonDies(Creature* summon, Unit* /*killer*/) override
    {
        if (summon->getEntry() != NPC_EB_TOXIC_SPIDERLING || mPhase != PHASE_XERITAC_WEB)
            return;

        if (++mSpiderlingKills >= 4)
            descend();
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
            case EVENT_XERITAC_TOXIC_BOLT:
                if (mPhase == PHASE_XERITAC_WEB)
                {
                    if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                        castSpell(pTarget, SPELL_XERITAC_TOXIC_BOLT);
                }
                else
                {
                    castSpellOnVictim(SPELL_XERITAC_TOXIC_BOLT);
                }
                scriptEvents.addEvent(EVENT_XERITAC_TOXIC_BOLT, Util::getRandomUInt(6000, 8000));
                break;
            case EVENT_XERITAC_SPIDERLING:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Xeri'tac unleashes Toxic Spiderlings!");
                summonCreature(NPC_EB_TOXIC_SPIDERLING, randomAddSpot());
                scriptEvents.addEvent(EVENT_XERITAC_SPIDERLING, 30000, PHASE_XERITAC_WEB);
                break;
            case EVENT_XERITAC_SPRAYER:
                summonCreature(NPC_EB_VENOM_SPRAYER, randomAddSpot());
                scriptEvents.addEvent(EVENT_XERITAC_SPRAYER, 30000, PHASE_XERITAC_WEB);
                break;
            case EVENT_XERITAC_PALE_ONE:
                summonCreature(NPC_EB_VENOM_CRAZED_PALE_ONE, xeritacPaleOneSpots[Util::getRandomUInt(0, 1)]);
                scriptEvents.addEvent(EVENT_XERITAC_PALE_ONE, 35000);
                break;
            case EVENT_XERITAC_BURSTER:
                summonCreature(NPC_EB_GORGED_BURSTER, randomAddSpot());
                scriptEvents.addEvent(EVENT_XERITAC_BURSTER, 30000);
                break;
            case EVENT_XERITAC_DESCEND:
                descend();
                break;
            case EVENT_XERITAC_VENOMOUS_STING:
                castSpellOnVictim(SPELL_XERITAC_VENOMOUS_STING);
                scriptEvents.addEvent(EVENT_XERITAC_VENOMOUS_STING, 15000, PHASE_XERITAC_GROUND);
                break;
            case EVENT_XERITAC_GAS_VOLLEY:
                castSpellAOE(SPELL_XERITAC_GAS_VOLLEY);
                scriptEvents.addEvent(EVENT_XERITAC_GAS_VOLLEY, 20000, PHASE_XERITAC_GROUND);
                break;
            default:
                break;
        }
    }

private:
    static LocationVector const& randomAddSpot()
    {
        return xeritacAddSpots[Util::getRandomUInt(0, 4)];
    }

    void descend()
    {
        if (mPhase != PHASE_XERITAC_WEB)
            return;

        mPhase = PHASE_XERITAC_GROUND;
        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Xeri'tac descends from her web!");
        setRooted(false);
        _setMeleeDisabled(false);

        scriptEvents.removeEvent(EVENT_XERITAC_DESCEND);
        scriptEvents.addEvent(EVENT_XERITAC_VENOMOUS_STING, 16000, PHASE_XERITAC_GROUND);
        scriptEvents.addEvent(EVENT_XERITAC_GAS_VOLLEY, 30000, PHASE_XERITAC_GROUND);
    }

    uint32_t mPhase = PHASE_XERITAC_WEB;
    uint32_t mSpiderlingKills = 0;
};

// Venom Sprayer, sprays venom at its target
class EverbloomVenomSprayerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomVenomSprayerAI(c); }
    explicit EverbloomVenomSprayerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_EB_SPRAYER_SPRAY, Util::getRandomUInt(7000, 9000));
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

        if (scriptEvents.getFinishedEvent() == EVENT_EB_SPRAYER_SPRAY)
        {
            castSpellOnVictim(SPELL_VENOM_SPRAYER_SPRAY);
            scriptEvents.addEvent(EVENT_EB_SPRAYER_SPRAY, Util::getRandomUInt(7000, 9000));
        }
    }
};

// Venom-Crazed Pale One, swipes everything in front of it
class EverbloomPaleOneAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomPaleOneAI(c); }
    explicit EverbloomPaleOneAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_EB_PALE_ONE_SWIPE, 3000);
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

        if (scriptEvents.getFinishedEvent() == EVENT_EB_PALE_ONE_SWIPE)
        {
            castSpellOnVictim(SPELL_PALE_ONE_SWIPE);
            scriptEvents.addEvent(EVENT_EB_PALE_ONE_SWIPE, 8000);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Archmage Sol

class EverbloomArchmageSolAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomArchmageSolAI(c); }
    explicit EverbloomArchmageSolAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ARCHMAGE_SOL, EncounterStates::InProgress);

        mSchool = 0;
        castSpellOnSelf(SPELL_SOL_AFFINITY_FIRE, true);

        scriptEvents.addEvent(EVENT_SOL_PARASITIC_GROWTH, 29000);
        scheduleSchool();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ARCHMAGE_SOL, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ARCHMAGE_SOL, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        if (summon->getEntry() == NPC_EB_SPORE_IMAGE)
            setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        // event phase 1 fire, 2 frost, 3 arcane
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mSchool + 1);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SOL_PARASITIC_GROWTH:
                castSpellOnSelf(SPELL_SOL_PARASITIC_GROWTH);
                shiftSchool();
                if (mSchool < 2)
                    scriptEvents.addEvent(EVENT_SOL_PARASITIC_GROWTH, 29000);
                break;
            case EVENT_SOL_FIREBALL:
                castSpellOnVictim(SPELL_SOL_FIREBALL);
                scriptEvents.addEvent(EVENT_SOL_FIREBALL, 4000, 1);
                break;
            case EVENT_SOL_FIREBLOOM:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_SOL_FIREBLOOM);
                scriptEvents.addEvent(EVENT_SOL_FIREBLOOM, 12000, 1);
                break;
            case EVENT_SOL_FROSTBOLT:
                castSpellOnVictim(SPELL_SOL_FROSTBOLT);
                scriptEvents.addEvent(EVENT_SOL_FROSTBOLT, 4000, 2);
                break;
            case EVENT_SOL_FROZEN_RAIN:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    summonCreature(NPC_EB_FROZEN_RAIN, pTarget->GetPosition(), TIMED_DESPAWN, 20000);
                scriptEvents.addEvent(EVENT_SOL_FROZEN_RAIN, 8000, 2);
                break;
            case EVENT_SOL_ARCANE_BURST:
                castSpellAOE(SPELL_SOL_ARCANE_BURST);
                scriptEvents.addEvent(EVENT_SOL_ARCANE_BURST, 5000, 3);
                break;
            default:
                break;
        }
    }

private:
    void scheduleSchool()
    {
        switch (mSchool)
        {
            case 0:
                scriptEvents.addEvent(EVENT_SOL_FIREBALL, 6000, 1);
                scriptEvents.addEvent(EVENT_SOL_FIREBLOOM, 22000, 1);
                break;
            case 1:
                scriptEvents.addEvent(EVENT_SOL_FROZEN_RAIN, 5000, 2);
                scriptEvents.addEvent(EVENT_SOL_FROSTBOLT, 6000, 2);
                break;
            default:
                scriptEvents.addEvent(EVENT_SOL_ARCANE_BURST, 6000, 3);
                break;
        }
    }

    // fire, then frost, then arcane until the end
    void shiftSchool()
    {
        if (mSchool >= 2)
            return;

        _removeAura(mSchool == 0 ? SPELL_SOL_AFFINITY_FIRE : SPELL_SOL_AFFINITY_FROST);
        ++mSchool;

        castSpellOnSelf(mSchool == 1 ? SPELL_SOL_AFFINITY_FROST : SPELL_SOL_AFFINITY_ARCANE, true);
        castSpellOnSelf(SPELL_SOL_PARASITIC_INJECTION, true);
        sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, mSchool == 1 ? "Archmage Sol begins casting Frost spells!" : "Archmage Sol begins casting Arcane spells!");
        scheduleSchool();

        if (isHeroic())
        {
            float const angle = Util::getRandomFloat(0.0f, 6.28f);
            summonCreature(NPC_EB_SPORE_IMAGE, getCreature()->GetPositionX() + 15.0f * std::cos(angle), getCreature()->GetPositionY() + 15.0f * std::sin(angle), getCreature()->GetPositionZ(), angle);
        }
    }

    uint32_t mSchool = 0;
};

// Frozen Rain, stationary ice storm of the frost school
class EverbloomFrozenRainAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomFrozenRainAI(c); }
    explicit EverbloomFrozenRainAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_FROZEN_RAIN_SPAWN, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Yalnu

class EverbloomYalnuAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomYalnuAI(c); }
    explicit EverbloomYalnuAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_YALNU, EncounterStates::InProgress);

        // the Kirin Tor join the fight, Lady Baihu takes the last spot
        for (uint32_t i = 0; i < 6; ++i)
            summonCreature(NPC_EB_KIRIN_TOR_BATTLE_MAGE, yalnuAllySpots[i]);
        summonCreature(NPC_EB_LADY_BAIHU, yalnuAllySpots[6]);

        scriptEvents.addEvent(EVENT_YALNU_COLOSSAL_BLOW, 5000);
        scriptEvents.addEvent(EVENT_YALNU_FONT_OF_LIFE, 12000);
        scriptEvents.addEvent(EVENT_YALNU_ENTANGLEMENT, 15000);
        scriptEvents.addEvent(EVENT_YALNU_GENESIS, 30000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_YALNU_ENTANGLEMENT_PLAYER, 50000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_YALNU, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_YALNU, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        if (summon->getEntry() != NPC_EB_KIRIN_TOR_BATTLE_MAGE && summon->getEntry() != NPC_EB_LADY_BAIHU)
            setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_YALNU_COLOSSAL_BLOW:
                castSpellOnVictim(SPELL_YALNU_COLOSSAL_BLOW);
                scriptEvents.addEvent(EVENT_YALNU_COLOSSAL_BLOW, 20000);
                break;
            case EVENT_YALNU_FONT_OF_LIFE:
                spawnAroundSelf(lifeFontAddEntry(), 10.0f);
                scriptEvents.addEvent(EVENT_YALNU_FONT_OF_LIFE, 20000);
                break;
            case EVENT_YALNU_ENTANGLEMENT:
                if (Creature* pMage = pickBattleMage())
                    castSpell(pMage, SPELL_YALNU_ENTANGLEMENT);
                scriptEvents.addEvent(EVENT_YALNU_ENTANGLEMENT, 30000);
                break;
            case EVENT_YALNU_ENTANGLEMENT_PLAYER:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_YALNU_ENTANGLEMENT_PLAYER);
                scriptEvents.addEvent(EVENT_YALNU_ENTANGLEMENT_PLAYER, 60000);
                break;
            case EVENT_YALNU_GENESIS:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Yalnu begins to channel Genesis!");
                castSpellOnSelf(SPELL_YALNU_GENESIS);
                scriptEvents.addEvent(EVENT_YALNU_GENESIS_BLOOM, 8000);
                scriptEvents.addEvent(EVENT_YALNU_GENESIS, 45000);
                break;
            case EVENT_YALNU_GENESIS_BLOOM:
            {
                // sprouts that were left alone grow into feral lashers
                uint32_t const lashers = isHeroic() ? 5 : 3;
                for (uint32_t i = 0; i < lashers; ++i)
                    summonCreature(NPC_EB_FERAL_LASHER, Util::getRandomFloat(930.0f, 983.0f), Util::getRandomFloat(-1267.0f, -1201.0f), getCreature()->GetPositionZ(), 0.0f);
            } break;
            default:
                break;
        }
    }

private:
    static uint32_t lifeFontAddEntry()
    {
        switch (Util::getRandomUInt(0, 2))
        {
            case 0:
                return NPC_EB_VICIOUS_MANDRAGORA;
            case 1:
                return NPC_EB_GNARLED_ANCIENT;
            default:
                return NPC_EB_SWIFT_SPROUTLING;
        }
    }

    void spawnAroundSelf(uint32_t entry, float radius)
    {
        float const angle = Util::getRandomFloat(0.0f, 6.28f);
        summonCreature(entry, getCreature()->GetPositionX() + radius * std::cos(angle), getCreature()->GetPositionY() + radius * std::sin(angle), getCreature()->GetPositionZ(), angle);
    }

    Creature* pickBattleMage()
    {
        std::list<Creature*> mages;
        GetCreatureListWithEntryInGrid(mages, NPC_EB_KIRIN_TOR_BATTLE_MAGE, 80.0f);

        std::vector<Creature*> living;
        for (Creature* pMage : mages)
        {
            if (pMage->isAlive())
                living.push_back(pMage);
        }

        if (living.empty())
            return nullptr;

        return Util::selectRandomContainerElement(living);
    }
};

// Kirin Tor Battle-Mage and Lady Baihu, stationary allies bombarding Yalnu
class EverbloomKirinTorMageAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EverbloomKirinTorMageAI(c); }
    explicit EverbloomKirinTorMageAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        setRooted(true);
        castSpellOnSelf(SPELL_KIRIN_TOR_BLINK, true);

        // 0 fire, 1 frost, 2 arcane
        mSchool = Util::getRandomUInt(0, 2);
        scriptEvents.addEvent(EVENT_EB_MAGE_PRIMARY, 2000);
        scriptEvents.addEvent(EVENT_EB_MAGE_SECONDARY, Util::getRandomUInt(18000, 30000));
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        uint32_t const eventId = scriptEvents.getFinishedEvent();
        if (eventId == 0)
            return;

        Creature* pYalnu = findNearestCreature(BOSS_YALNU, 100.0f);
        bool const canStrike = pYalnu != nullptr && pYalnu->isAlive();

        switch (eventId)
        {
            case EVENT_EB_MAGE_PRIMARY:
                if (canStrike)
                    castSpell(pYalnu, primarySpell());
                scriptEvents.addEvent(EVENT_EB_MAGE_PRIMARY, 3000);
                break;
            case EVENT_EB_MAGE_SECONDARY:
                if (canStrike)
                    castSpell(pYalnu, secondarySpell());
                scriptEvents.addEvent(EVENT_EB_MAGE_SECONDARY, Util::getRandomUInt(18000, 30000));
                break;
            default:
                break;
        }
    }

private:
    uint32_t primarySpell() const
    {
        if (mSchool == 0)
            return SPELL_KIRIN_TOR_FIREBALL;
        if (mSchool == 1)
            return SPELL_KIRIN_TOR_FROSTBOLT;
        return SPELL_KIRIN_TOR_ARCANE_BLAST;
    }

    uint32_t secondarySpell() const
    {
        if (mSchool == 0)
            return SPELL_KIRIN_TOR_FLAMESTRIKE;
        if (mSchool == 1)
            return SPELL_KIRIN_TOR_ICE_COMET;
        return SPELL_KIRIN_TOR_ARCANE_ORB;
    }

    uint32_t mSchool = 0;
};

void SetupTheEverbloom(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_EVERBLOOM, &EverbloomInstanceScript::Create);

    mgr->register_creature_script(BOSS_WITHERBARK, &EverbloomWitherbarkAI::Create);
    mgr->register_creature_script(BOSS_LIFE_WARDEN_GOLA, &EverbloomGolaAI::Create);
    mgr->register_creature_script(BOSS_EARTHSHAPER_TELU, &EverbloomTeluAI::Create);
    mgr->register_creature_script(BOSS_DULHU, &EverbloomDulhuAI::Create);
    mgr->register_creature_script(BOSS_XERITAC, &EverbloomXeritacAI::Create);
    mgr->register_creature_script(BOSS_ARCHMAGE_SOL, &EverbloomArchmageSolAI::Create);
    mgr->register_creature_script(BOSS_YALNU, &EverbloomYalnuAI::Create);

    mgr->register_creature_script(NPC_EB_AQUEOUS_GLOBULE, &EverbloomAqueousGlobuleAI::Create);
    mgr->register_creature_script(NPC_EB_UNCHECKED_GROWTH, &EverbloomUncheckedGrowthAI::Create);
    mgr->register_creature_script(NPC_EB_VENOM_SPRAYER, &EverbloomVenomSprayerAI::Create);
    mgr->register_creature_script(NPC_EB_VENOM_CRAZED_PALE_ONE, &EverbloomPaleOneAI::Create);
    mgr->register_creature_script(NPC_EB_FROZEN_RAIN, &EverbloomFrozenRainAI::Create);
    mgr->register_creature_script(NPC_EB_KIRIN_TOR_BATTLE_MAGE, &EverbloomKirinTorMageAI::Create);
    mgr->register_creature_script(NPC_EB_LADY_BAIHU, &EverbloomKirinTorMageAI::Create);
}
