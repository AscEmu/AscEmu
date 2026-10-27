/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Kings_Rest.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <map>
#include <string>
#include <vector>

enum KingsRestEvents
{
    EVENT_SERPENT_SPIT_GOLD = 1,
    EVENT_SERPENT_ANIMATE_GOLD,
    EVENT_SERPENT_SERPENTINE_GUST,
    EVENT_SERPENT_TAIL_THRASH,
    EVENT_SERPENT_LUCRES_CALL,

    EVENT_KULA_SEVERING_AXE,
    EVENT_KULA_WHIRLING_AXES,
    EVENT_AKAALI_BACKHAND,
    EVENT_AKAALI_BARREL_THROUGH,
    EVENT_AKAALI_BARREL_CHARGE,
    EVENT_AKAALI_BARREL_IMPACT,
    EVENT_ZANAZAL_LIGHTNING_BOLT,
    EVENT_ZANAZAL_POISON_NOVA,
    EVENT_ZANAZAL_CALL_OF_ELEMENTS,

    EVENT_MCHIMBA_BURN_CORRUPTION,
    EVENT_MCHIMBA_DRAIN_FLUIDS,
    EVENT_MCHIMBA_DESSICATION,

    EVENT_DAZAR_BLADE_COMBO,
    EVENT_DAZAR_GALE_SLASH,
    EVENT_DAZAR_QUAKING_LEAP,
    EVENT_REBAN_HUNTING_LEAP,
    EVENT_TZALA_DEATHLY_ROAR
};

namespace
{
    enum class KingsRestDoorKind : uint8_t
    {
        Room,
        Passage
    };

    struct KingsRestDoor
    {
        uint32_t entry;
        uint32_t encounterId;
        KingsRestDoorKind kind;
    };

    // room doors shut during the fight, passages open after the kill
    KingsRestDoor const KingsRestDoors[] =
    {
        { GO_SERPENT_ROOM_DOOR, DATA_GOLDEN_SERPENT,    KingsRestDoorKind::Room },
        { GO_SERPENT_EXIT_DOOR, DATA_GOLDEN_SERPENT,    KingsRestDoorKind::Passage },
        { GO_COUNCIL_GATE,      DATA_COUNCIL_OF_TRIBES, KingsRestDoorKind::Room },
        { GO_MCHIMBA_GATE,      DATA_MCHIMBA,           KingsRestDoorKind::Room },
        { GO_DAZAR_GATE,        DATA_KING_DAZAR,        KingsRestDoorKind::Room }
    };

    KingsRestDoor const* findKingsRestDoor(uint32_t entry)
    {
        for (KingsRestDoor const& door : KingsRestDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    // order in which the tribal champions take the floor
    uint32_t const TribeChampions[] = { BOSS_KULA_THE_BUTCHER, BOSS_AKAALI, BOSS_ZANAZAL_THE_WISE };

    uint32_t const ElementTotems[] = { NPC_EXPLOSIVE_TOTEM, NPC_THUNDERING_TOTEM, NPC_TORRENT_TOTEM, NPC_EARTHWALL_TOTEM };

    // centre of the tribal ring
    LocationVector const TribalRingCentre(-1101.0f, 2911.0f, 775.56f);

    float const KingsRestPi = 3.14159265f;
    uint32_t const SpitGoldFesterMs = 9000;
    uint32_t const DrainFluidsMs = 8000;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class KingsRestInstanceScript : public InstanceScript
{
public:
    explicit KingsRestInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(KingsRestEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new KingsRestInstanceScript(pMapMgr); }

    // a fallen champion hands over to the next, only the last one ends the council
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (id == DATA_COUNCIL_OF_TRIBES && state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isTribeDown(nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_GOLDEN_SERPENT:
            case BOSS_KULA_THE_BUTCHER:
            case BOSS_AKAALI:
            case BOSS_ZANAZAL_THE_WISE:
                mTrackedGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
                break;
            default:
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findKingsRestDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        alignDoor(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor);
        }

        // a wiped council gathers again in full
        if (id == DATA_COUNCIL_OF_TRIBES && state == EncounterStates::Failed)
        {
            for (Creature* pChampion : getTribe())
            {
                if (!pChampion->isAlive())
                    pChampion->despawn(1000, 5000);
            }
        }
    }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getTribe()
    {
        std::vector<Creature*> tribe;
        for (uint32_t entry : TribeChampions)
        {
            if (Creature* pChampion = getTracked(entry))
                tribe.push_back(pChampion);
        }

        return tribe;
    }

    bool isTribeDown(Creature* pExcept)
    {
        for (Creature* pChampion : getTribe())
        {
            if (pChampion != pExcept && pChampion->isAlive())
                return false;
        }

        return true;
    }

    // first champion of the order still standing
    Creature* getCurrentChampion()
    {
        for (Creature* pChampion : getTribe())
        {
            if (pChampion->isAlive())
                return pChampion;
        }

        return nullptr;
    }

private:
    void alignDoor(GameObject* pDoor)
    {
        KingsRestDoor const* door = findKingsRestDoor(pDoor->getEntry());
        if (door == nullptr)
            return;

        EncounterStates const state = getBossState(door->encounterId);
        bool const wantOpen = door->kind == KingsRestDoorKind::Room ? state != EncounterStates::InProgress : state == EncounterStates::Performed;
        uint8_t const wantState = wantOpen ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wantState)
            pDoor->setState(wantState);
    }

    std::map<uint32_t, WoWGuid> mTrackedGuids;
    std::vector<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class KingsRestCombatAI : public CreatureAIScript
{
public:
    explicit KingsRestCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    KingsRestInstanceScript* getKingsRest()
    {
        if (getCreature()->GetMapId() != MAP_KINGS_REST)
            return nullptr;

        return static_cast<KingsRestInstanceScript*>(getInstanceScript());
    }

    // random player, the tank only when nobody else is around
    Unit* pickPlayer(bool skipTank = false)
    {
        if (skipTank)
        {
            FilterArgs args(TargetFilter_Player);
            args.addFilter(TargetFilter_NotCurrent);
            if (Unit* pTarget = selectUnitTarget(args))
                return pTarget;
        }

        return selectUnitTarget(FilterArgs(TargetFilter_Player));
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false)
    {
        Unit* pTarget = pickPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId);

        return pTarget;
    }

    Unit* farthestPlayer()
    {
        Unit* pFarthest = nullptr;
        float farthestRange = -1.0f;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer == nullptr || !pPlayer->isAlive())
                continue;

            float const range = getCreature()->getDistance(pPlayer);
            if (range > farthestRange)
            {
                farthestRange = range;
                pFarthest = pPlayer;
            }
        }

        return pFarthest;
    }

    Unit* findUnit(WoWGuid const& guid)
    {
        if (guid.isEmpty() || getCreature()->getWorldMap() == nullptr)
            return nullptr;

        return getCreature()->getWorldMap()->getUnit(guid);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class KingsRestBossAI : public KingsRestCombatAI
{
public:
    KingsRestBossAI(Creature* pCreature, uint32_t encounterId) : KingsRestCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        KingsRestCombatAI::OnCombatStart(pTarget);
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

        KingsRestCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            if (isEncounterOver())
                pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }

        onDefeat();
    }

protected:
    virtual bool isEncounterOver() { return true; }
    virtual void onDefeat() {}

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Golden Serpent

class KingsRestGoldenSerpentAI : public KingsRestBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingsRestGoldenSerpentAI(c); }
    explicit KingsRestGoldenSerpentAI(Creature* pCreature) : KingsRestBossAI(pCreature, DATA_GOLDEN_SERPENT) {}

protected:
    void onEngage() override
    {
        mSpitTargetGuid = WoWGuid();
        scriptEvents.addEvent(EVENT_SERPENT_SPIT_GOLD, 9000);
        scriptEvents.addEvent(EVENT_SERPENT_SERPENTINE_GUST, 12500);
        scriptEvents.addEvent(EVENT_SERPENT_TAIL_THRASH, 15500);
        scriptEvents.addEvent(EVENT_SERPENT_LUCRES_CALL, 40000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SERPENT_SPIT_GOLD:
                if (Unit* pTarget = castAtPlayer(SPELL_SERPENT_SPIT_GOLD))
                {
                    mSpitTargetGuid = pTarget->GetNewGUID();
                    scriptEvents.addEvent(EVENT_SERPENT_ANIMATE_GOLD, SpitGoldFesterMs);
                }
                scriptEvents.addEvent(EVENT_SERPENT_SPIT_GOLD, 11000);
                break;
            case EVENT_SERPENT_ANIMATE_GOLD:
                animateGold();
                break;
            case EVENT_SERPENT_SERPENTINE_GUST:
                castSpellOnSelf(SPELL_SERPENT_SERPENTINE_GUST);
                scriptEvents.addEvent(EVENT_SERPENT_SERPENTINE_GUST, 21500);
                break;
            case EVENT_SERPENT_TAIL_THRASH:
                castSpellOnVictim(SPELL_SERPENT_TAIL_THRASH);
                scriptEvents.addEvent(EVENT_SERPENT_TAIL_THRASH, 17000);
                break;
            case EVENT_SERPENT_LUCRES_CALL:
                callTheGold();
                scriptEvents.addEvent(EVENT_SERPENT_LUCRES_CALL, 41500);
                break;
            default:
                break;
        }
    }

private:
    // the spat gold comes alive where it burned its victim
    void animateGold()
    {
        Unit* pVictim = findUnit(mSpitTargetGuid);
        mSpitTargetGuid = WoWGuid();
        if (pVictim != nullptr)
            summonCreature(NPC_ANIMATED_GOLD, pVictim->GetPosition(), CORPSE_DESPAWN);
    }

    void callTheGold()
    {
        warn("The Golden Serpent calls its gold back with Lucre's Call!");
        castSpellOnSelf(SPELL_SERPENT_LUCRES_CALL);

        std::list<Creature*> golds;
        GetCreatureListWithEntryInGrid(golds, NPC_ANIMATED_GOLD, 150.0f);
        for (Creature* pGold : golds)
        {
            if (pGold->isAlive() && pGold->GetScript() != nullptr)
                pGold->GetScript()->DoAction(ACTION_GOLD_HEED_CALL);
        }
    }

    WoWGuid mSpitTargetGuid;
};

// melts into a pool when beaten, otherwise feeds the serpent on its call
class KingsRestAnimatedGoldAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingsRestAnimatedGoldAI(c); }
    explicit KingsRestAnimatedGoldAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        if (!mPooled)
            meltIntoPool();
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_GOLD_HEED_CALL && !mPooled)
            mHeading = true;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mHeading || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mStepClock > diff)
        {
            mStepClock -= diff;
            return;
        }

        mStepClock = 1000;

        Creature* pSerpent = getSerpent();
        if (pSerpent == nullptr || !pSerpent->isAlive())
        {
            mHeading = false;
            return;
        }

        if (getCreature()->getDistance(pSerpent) > 4.0f)
        {
            moveTo(pSerpent->GetPositionX(), pSerpent->GetPositionY(), pSerpent->GetPositionZ(), false);
            return;
        }

        mHeading = false;
        castSpellOnSelf(SPELL_GOLD_LUSTER, true);
        despawn(1000, 0);
    }

private:
    Creature* getSerpent()
    {
        if (getCreature()->GetMapId() != MAP_KINGS_REST)
            return nullptr;

        KingsRestInstanceScript* pKingsRest = static_cast<KingsRestInstanceScript*>(getInstanceScript());
        return pKingsRest != nullptr ? pKingsRest->getTracked(BOSS_GOLDEN_SERPENT) : nullptr;
    }

    void meltIntoPool()
    {
        mPooled = true;
        mHeading = false;
        stopMovement();
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_GOLD_MOLTEN_GOLD, true);
    }

    uint32_t mStepClock = 1000;
    bool mPooled = false;
    bool mHeading = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Council of Tribes, one champion at a time

class KingsRestChampionAI : public KingsRestBossAI
{
public:
    explicit KingsRestChampionAI(Creature* pCreature) : KingsRestBossAI(pCreature, DATA_COUNCIL_OF_TRIBES) {}

    void OnCombatStart(Unit* pTarget) override
    {
        KingsRestBossAI::OnCombatStart(pTarget);

        // the whole council is drawn in, the rest waits for its turn
        if (KingsRestInstanceScript* pKingsRest = getKingsRest())
        {
            for (Creature* pChampion : pKingsRest->getTribe())
            {
                if (pChampion != getCreature() && pChampion->isAlive() && !pChampion->isInCombat())
                    setZoneWideCombat(pChampion);
            }
        }
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_CHAMPION_STEP_IN && mWaiting && _isInCombat())
            stepIn();
    }

protected:
    void onEngage() override
    {
        KingsRestInstanceScript* pKingsRest = getKingsRest();
        if (pKingsRest == nullptr || pKingsRest->getCurrentChampion() == getCreature())
        {
            mWaiting = false;
            beginRound();
            return;
        }

        mWaiting = true;
        setSidelined(true);
    }

    void onReset() override
    {
        mWaiting = false;
        setSidelined(false);
    }

    bool isEncounterOver() override
    {
        KingsRestInstanceScript* pKingsRest = getKingsRest();
        return pKingsRest == nullptr || pKingsRest->isTribeDown(getCreature());
    }

    void onDefeat() override
    {
        KingsRestInstanceScript* pKingsRest = getKingsRest();
        if (pKingsRest == nullptr)
            return;

        Creature* pNext = pKingsRest->getCurrentChampion();
        if (pNext != nullptr && pNext->GetScript() != nullptr)
            pNext->GetScript()->DoAction(ACTION_CHAMPION_STEP_IN);
    }

    virtual void beginRound() {}
    virtual char const* getStepInText() const = 0;

private:
    void stepIn()
    {
        mWaiting = false;
        setSidelined(false);
        warn(getStepInText());
        beginRound();
    }

    void setSidelined(bool sidelined)
    {
        setRooted(sidelined);
        _setMeleeDisabled(sidelined);

        if (sidelined)
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    bool mWaiting = false;
};

class KingsRestKulaAI : public KingsRestChampionAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingsRestKulaAI(c); }
    explicit KingsRestKulaAI(Creature* pCreature) : KingsRestChampionAI(pCreature) {}

protected:
    void beginRound() override
    {
        scriptEvents.addEvent(EVENT_KULA_SEVERING_AXE, 5000);
        scriptEvents.addEvent(EVENT_KULA_WHIRLING_AXES, 10000);
    }

    char const* getStepInText() const override { return "Kula the Butcher steps into the ring!"; }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KULA_SEVERING_AXE:
                castAtPlayer(SPELL_KULA_SEVERING_AXE);
                scriptEvents.addEvent(EVENT_KULA_SEVERING_AXE, 15000);
                break;
            case EVENT_KULA_WHIRLING_AXES:
                castSpellAOE(SPELL_KULA_WHIRLING_AXES);
                scriptEvents.addEvent(EVENT_KULA_WHIRLING_AXES, 20000);
                break;
            default:
                break;
        }
    }
};

class KingsRestAkaaliAI : public KingsRestChampionAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingsRestAkaaliAI(c); }
    explicit KingsRestAkaaliAI(Creature* pCreature) : KingsRestChampionAI(pCreature) {}

protected:
    void beginRound() override
    {
        mRamTargetGuid = WoWGuid();
        scriptEvents.addEvent(EVENT_AKAALI_BACKHAND, 5000);
        scriptEvents.addEvent(EVENT_AKAALI_BARREL_THROUGH, 10000);
    }

    char const* getStepInText() const override { return "Aka'ali the Conqueror steps into the ring!"; }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AKAALI_BACKHAND:
                castSpellOnVictim(SPELL_AKAALI_DEBILITATING_BACKHAND);
                scriptEvents.addEvent(EVENT_AKAALI_BACKHAND, 15000);
                break;
            case EVENT_AKAALI_BARREL_THROUGH:
                if (Unit* pTarget = castAtPlayer(SPELL_AKAALI_BARREL_THROUGH))
                {
                    warn("Aka'ali the Conqueror prepares to Barrel Through!");
                    mRamTargetGuid = pTarget->GetNewGUID();
                    scriptEvents.addEvent(EVENT_AKAALI_BARREL_CHARGE, 8000);
                    scriptEvents.addEvent(EVENT_AKAALI_BARREL_IMPACT, 10000);
                }
                scriptEvents.addEvent(EVENT_AKAALI_BARREL_THROUGH, 20000);
                break;
            case EVENT_AKAALI_BARREL_CHARGE:
                if (Unit* pTarget = findUnit(mRamTargetGuid))
                    castSpell(pTarget, SPELL_AKAALI_BARREL_CHARGE, true);
                break;
            case EVENT_AKAALI_BARREL_IMPACT:
                mRamTargetGuid = WoWGuid();
                castSpellAOE(SPELL_AKAALI_BARREL_IMPACT, true);
                break;
            default:
                break;
        }
    }

private:
    WoWGuid mRamTargetGuid;
};

class KingsRestZanazalAI : public KingsRestChampionAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingsRestZanazalAI(c); }
    explicit KingsRestZanazalAI(Creature* pCreature) : KingsRestChampionAI(pCreature) {}

protected:
    void beginRound() override
    {
        scriptEvents.addEvent(EVENT_ZANAZAL_LIGHTNING_BOLT, 5000);
        scriptEvents.addEvent(EVENT_ZANAZAL_POISON_NOVA, 10000);
        scriptEvents.addEvent(EVENT_ZANAZAL_CALL_OF_ELEMENTS, 15000);
    }

    char const* getStepInText() const override { return "Zanazal the Wise steps into the ring!"; }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ZANAZAL_LIGHTNING_BOLT:
                castSpellOnVictim(SPELL_ZANAZAL_LIGHTNING_BOLT);
                scriptEvents.addEvent(EVENT_ZANAZAL_LIGHTNING_BOLT, 3000);
                break;
            case EVENT_ZANAZAL_POISON_NOVA:
                castSpellAOE(SPELL_ZANAZAL_POISON_NOVA);
                scriptEvents.addEvent(EVENT_ZANAZAL_POISON_NOVA, 20000);
                break;
            case EVENT_ZANAZAL_CALL_OF_ELEMENTS:
                callElements();
                scriptEvents.addEvent(EVENT_ZANAZAL_CALL_OF_ELEMENTS, 30000);
                break;
            default:
                break;
        }
    }

private:
    // one totem of each kind somewhere in the ring
    void callElements()
    {
        warn("Zanazal the Wise calls upon the elements!");
        castSpellOnSelf(SPELL_ZANAZAL_CALL_OF_ELEMENTS);

        for (uint32_t entry : ElementTotems)
        {
            float const angle = Util::getRandomFloat(0.0f, 2.0f * KingsRestPi);
            float const range = Util::getRandomFloat(0.0f, 30.0f);
            LocationVector const spot(TribalRingCentre.x + range * std::cos(angle), TribalRingCentre.y + range * std::sin(angle), TribalRingCentre.z, angle);
            summonCreature(entry, spot, CORPSE_DESPAWN);
        }
    }
};

// rooted totem, the explosive and thundering kinds release their power once
class KingsRestElementTotemAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingsRestElementTotemAI(c); }
    explicit KingsRestElementTotemAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mReleaseClock == 0 || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mReleaseClock > diff)
        {
            mReleaseClock -= diff;
            return;
        }

        mReleaseClock = 0;
        if (getCreature()->getEntry() == NPC_EXPLOSIVE_TOTEM)
            castSpellAOE(SPELL_TOTEM_EXPLODE);
        else if (getCreature()->getEntry() == NPC_THUNDERING_TOTEM)
            castSpellAOE(SPELL_TOTEM_THUNDERING_CRASH);
    }

private:
    uint32_t mReleaseClock = 1000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mchimba the Embalmer

class KingsRestMchimbaAI : public KingsRestBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingsRestMchimbaAI(c); }
    explicit KingsRestMchimbaAI(Creature* pCreature) : KingsRestBossAI(pCreature, DATA_MCHIMBA) {}

protected:
    void onEngage() override
    {
        mDrainedGuid = WoWGuid();
        scriptEvents.addEvent(EVENT_MCHIMBA_BURN_CORRUPTION, 10000);
        scriptEvents.addEvent(EVENT_MCHIMBA_DRAIN_FLUIDS, 15000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MCHIMBA_BURN_CORRUPTION:
                castAtPlayer(SPELL_MCHIMBA_BURN_CORRUPTION);
                scriptEvents.addEvent(EVENT_MCHIMBA_BURN_CORRUPTION, 15000);
                break;
            case EVENT_MCHIMBA_DRAIN_FLUIDS:
                if (Unit* pTarget = castAtPlayer(SPELL_MCHIMBA_DRAIN_FLUIDS))
                {
                    mDrainedGuid = pTarget->GetNewGUID();
                    scriptEvents.addEvent(EVENT_MCHIMBA_DESSICATION, DrainFluidsMs);
                }
                scriptEvents.addEvent(EVENT_MCHIMBA_DRAIN_FLUIDS, 20000);
                break;
            case EVENT_MCHIMBA_DESSICATION:
                // a drained body dries out afterwards
                if (Unit* pTarget = findUnit(mDrainedGuid))
                {
                    if (pTarget->isAlive())
                        castSpell(pTarget, SPELL_MCHIMBA_DESSICATION, true);
                }
                mDrainedGuid = WoWGuid();
                break;
            default:
                break;
        }
    }

private:
    WoWGuid mDrainedGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// King Dazar, calls his beasts as he weakens

class KingsRestDazarAI : public KingsRestBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingsRestDazarAI(c); }
    explicit KingsRestDazarAI(Creature* pCreature) : KingsRestBossAI(pCreature, DATA_KING_DAZAR) {}

protected:
    void onEngage() override
    {
        mRebanCalled = false;
        mTzalaCalled = false;
        scriptEvents.addEvent(EVENT_DAZAR_BLADE_COMBO, 5000);
        scriptEvents.addEvent(EVENT_DAZAR_GALE_SLASH, 10000);
        scriptEvents.addEvent(EVENT_DAZAR_QUAKING_LEAP, 15000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mRebanCalled && _getHealthPercent() <= 80)
        {
            mRebanCalled = true;
            warn("King Dazar calls Reban to his side!");
            callAdd(NPC_REBAN, getCreature()->GetPosition());
        }

        if (!mTzalaCalled && _getHealthPercent() <= 60)
        {
            mTzalaCalled = true;
            warn("King Dazar calls T'zala to his side!");
            callAdd(NPC_TZALA, getCreature()->GetPosition());
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DAZAR_BLADE_COMBO:
                castSpellOnVictim(SPELL_DAZAR_BLADE_COMBO);
                scriptEvents.addEvent(EVENT_DAZAR_BLADE_COMBO, 15000);
                break;
            case EVENT_DAZAR_GALE_SLASH:
                castAtPlayer(SPELL_DAZAR_GALE_SLASH);
                scriptEvents.addEvent(EVENT_DAZAR_GALE_SLASH, 30000);
                break;
            case EVENT_DAZAR_QUAKING_LEAP:
                if (Unit* pTarget = farthestPlayer())
                    castSpell(pTarget, SPELL_DAZAR_QUAKING_LEAP);
                scriptEvents.addEvent(EVENT_DAZAR_QUAKING_LEAP, 20000);
                break;
            default:
                break;
        }
    }

private:
    bool mRebanCalled = false;
    bool mTzalaCalled = false;
};

class KingsRestRebanAI : public KingsRestCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingsRestRebanAI(c); }
    explicit KingsRestRebanAI(Creature* pCreature) : KingsRestCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_REBAN_HUNTING_LEAP, 3000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_REBAN_HUNTING_LEAP)
            return;

        castAtPlayer(SPELL_REBAN_HUNTING_LEAP);
        scriptEvents.addEvent(EVENT_REBAN_HUNTING_LEAP, 15000);
    }
};

class KingsRestTzalaAI : public KingsRestCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingsRestTzalaAI(c); }
    explicit KingsRestTzalaAI(Creature* pCreature) : KingsRestCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_TZALA_DEATHLY_ROAR, 3000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_TZALA_DEATHLY_ROAR)
            return;

        castSpellAOE(SPELL_TZALA_DEATHLY_ROAR);
        scriptEvents.addEvent(EVENT_TZALA_DEATHLY_ROAR, 30000);
    }
};

void SetupKingsRest(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_KINGS_REST, &KingsRestInstanceScript::Create);

    mgr->register_creature_script(BOSS_GOLDEN_SERPENT, &KingsRestGoldenSerpentAI::Create);
    mgr->register_creature_script(NPC_ANIMATED_GOLD, &KingsRestAnimatedGoldAI::Create);

    mgr->register_creature_script(BOSS_KULA_THE_BUTCHER, &KingsRestKulaAI::Create);
    mgr->register_creature_script(BOSS_AKAALI, &KingsRestAkaaliAI::Create);
    mgr->register_creature_script(BOSS_ZANAZAL_THE_WISE, &KingsRestZanazalAI::Create);
    for (uint32_t entry : ElementTotems)
        mgr->register_creature_script(entry, &KingsRestElementTotemAI::Create);

    mgr->register_creature_script(BOSS_MCHIMBA, &KingsRestMchimbaAI::Create);

    mgr->register_creature_script(BOSS_KING_DAZAR, &KingsRestDazarAI::Create);
    mgr->register_creature_script(NPC_REBAN, &KingsRestRebanAI::Create);
    mgr->register_creature_script(NPC_TZALA, &KingsRestTzalaAI::Create);
}
