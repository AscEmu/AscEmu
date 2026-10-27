/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Darkflame_Cleft.hpp"

#include "Objects/GameObject.h"
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

enum DarkflameCleftEvents
{
    EVENT_CANDLE_KING_EERIE_MOLDS = 1,
    EVENT_CANDLE_KING_PARANOID_MIND,
    EVENT_CANDLE_KING_PICKAXE,
    EVENT_CANDLE_KING_THROW_DARKFLAME,

    EVENT_DARKNESS_UMBRAL_SLASH,
    EVENT_DARKNESS_SHADOWBLAST,
    EVENT_DARKNESS_CALL_DARKSPAWN,
    EVENT_DARKNESS_RISING_GLOOM,
    EVENT_DARKSPAWN_DRAIN_LIGHT,

    EVENT_BLAZIKON_WICKLIGHTER_BARRAGE,
    EVENT_BLAZIKON_INCITE_FLAMES,
    EVENT_BLAZIKON_DOUSING_BREATH,
    EVENT_BLAZIKON_REACH_CHECK,

    EVENT_WAXBEARD_KOL_TO_ARMS,
    EVENT_WAXBEARD_ROCK_BUSTER,
    EVENT_WAXBEARD_LURING_CANDLEFLAME,
    EVENT_WAXBEARD_RECKLESS_CHARGE,
    EVENT_WAXBEARD_CAVE_IN
};

namespace
{
    // room doors shut during the fight, passages open after the kill
    struct DarkflameDoor
    {
        uint32_t entry;
        uint32_t dataId;
        bool passage;
    };

    DarkflameDoor const DarkflameDoors[] =
    {
        { GO_CANDLE_KING_GATE,      DATA_THE_CANDLE_KING,   true },
        { GO_BLAZIKON_NORTH_DOOR,   DATA_BLAZIKON,          false },
        { GO_BLAZIKON_SOUTH_DOOR,   DATA_BLAZIKON,          false }
    };

    DarkflameDoor const* findDarkflameDoor(uint32_t entry)
    {
        for (DarkflameDoor const& door : DarkflameDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    float const DarkflameTwoPi = 6.28318531f;

    uint32_t const CandleKingStatueCount = 2;

    uint32_t const BlazikonEnergyPerSecond = 3;
    float const BlazikonMeleeReach = 8.0f;

    uint32_t const WaxbeardLaborersPerWave = 3;
    float const CaveInReach = 10.0f;
    float const LaborerFixateThreat = 1000000.0f;

    float const DarkspawnCandleSearch = 45.0f;

    // living players within range of a creature
    std::vector<Unit*> darkflamePlayersNear(Creature* pCreature, float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pCreature->getDistance(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        return players;
    }

    LocationVector darkflameRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(DarkflameTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class DarkflameCleftInstanceScript : public InstanceScript
{
public:
    explicit DarkflameCleftInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(DarkflameCleftEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new DarkflameCleftInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findDarkflameDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        applyDoorState(pGameObject);
    }

    void OnEncounterStateChange(uint32_t /*id*/, uint32_t /*state*/) override
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
        DarkflameDoor const* pRule = findDarkflameDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        EncounterStates const state = getBossState(pRule->dataId);
        bool const open = pRule->passage ? state == EncounterStates::Performed : state != EncounterStates::InProgress;
        uint8_t const wanted = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::vector<WoWGuid> mDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy and target helpers

class DarkflameCleftCombatAI : public CreatureAIScript
{
public:
    explicit DarkflameCleftCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        onEngage();
    }

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

    InstanceScript* cleftScript()
    {
        if (getCreature()->GetMapId() != MAP_DARKFLAME_CLEFT)
            return nullptr;

        return getInstanceScript();
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank, the tank when alone
    Unit* offTankPlayer()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_NotCurrent);
        if (Unit* pTarget = selectUnitTarget(args))
            return pTarget;

        return anyPlayer();
    }

    Unit* farthestPlayer()
    {
        Unit* pFarthest = nullptr;
        float farthestRange = -1.0f;
        for (Unit* pPlayer : darkflamePlayersNear(getCreature(), 100.0f))
        {
            float const range = getCreature()->getDistance(pPlayer);
            if (range > farthestRange)
            {
                farthestRange = range;
                pFarthest = pPlayer;
            }
        }

        return pFarthest;
    }

    // distinct random players
    std::vector<Unit*> somePlayers(size_t count)
    {
        std::vector<Unit*> pool = darkflamePlayersNear(getCreature(), 100.0f);
        for (size_t i = 0; i < pool.size() && i < count; ++i)
        {
            size_t const pick = i + Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1 - i));
            std::swap(pool[i], pool[pick]);
        }

        if (pool.size() > count)
            pool.resize(count);

        return pool;
    }

    bool anyPlayerInReach(float range) { return !darkflamePlayersNear(getCreature(), range).empty(); }

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return darkflameRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // true once the bar is full
    bool fillEnergy(uint32_t diff, uint32_t perSecond)
    {
        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy += perSecond;
        }

        if (mEnergy > 100)
            mEnergy = 100;

        return mEnergy >= 100;
    }

    void emptyEnergy()
    {
        mEnergy = 0;
        mEnergyClock = 0;
    }

private:
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class DarkflameCleftBossAI : public DarkflameCleftCombatAI
{
public:
    DarkflameCleftBossAI(Creature* pCreature, uint32_t dataId) : DarkflameCleftCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = cleftScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        DarkflameCleftCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = cleftScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        DarkflameCleftCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = cleftScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Candle King

class DarkflameCleftCandleKingAI : public DarkflameCleftBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkflameCleftCandleKingAI(c); }
    explicit DarkflameCleftCandleKingAI(Creature* pCreature) : DarkflameCleftBossAI(pCreature, DATA_THE_CANDLE_KING) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_CANDLE_KING_EERIE_MOLDS, 6000);
        scriptEvents.addEvent(EVENT_CANDLE_KING_PARANOID_MIND, 10500);
        scriptEvents.addEvent(EVENT_CANDLE_KING_PICKAXE, 13200);
        scriptEvents.addEvent(EVENT_CANDLE_KING_THROW_DARKFLAME, 19300);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CANDLE_KING_EERIE_MOLDS:
                warn("The Candle King shapes Eerie Molds of wax!");
                castSpellAOE(SPELL_CANDLE_KING_EERIE_MOLDS);
                moldStatues();
                scriptEvents.addEvent(EVENT_CANDLE_KING_EERIE_MOLDS, 31500);
                break;
            case EVENT_CANDLE_KING_PARANOID_MIND:
                castAt(offTankPlayer(), SPELL_CANDLE_KING_PARANOID_MIND);
                scriptEvents.addEvent(EVENT_CANDLE_KING_PARANOID_MIND, 20600);
                break;
            case EVENT_CANDLE_KING_PICKAXE:
                castAt(anyPlayer(), SPELL_CANDLE_KING_DARKFLAME_PICKAXE);
                scriptEvents.addEvent(EVENT_CANDLE_KING_PICKAXE, 17000);
                break;
            case EVENT_CANDLE_KING_THROW_DARKFLAME:
                castSpellAOE(SPELL_CANDLE_KING_THROW_DARKFLAME);
                scriptEvents.addEvent(EVENT_CANDLE_KING_THROW_DARKFLAME, 17000);
                break;
            default:
                break;
        }
    }

private:
    // a waxen copy rises where a player stands
    void moldStatues()
    {
        for (Unit* pPlayer : somePlayers(CandleKingStatueCount))
            callAdd(NPC_WAX_STATUE, pPlayer->GetPosition());
    }
};

// statue stays where it was molded
class DarkflameCleftWaxStatueAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkflameCleftWaxStatueAI(c); }
    explicit DarkflameCleftWaxStatueAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_CANDLE_KING_CURSED_WAX, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Darkness

class DarkflameCleftDarknessAI : public DarkflameCleftBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkflameCleftDarknessAI(c); }
    explicit DarkflameCleftDarknessAI(Creature* pCreature) : DarkflameCleftBossAI(pCreature, DATA_THE_DARKNESS) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_DARKNESS_UMBRAL_SLASH, 6000);
        scriptEvents.addEvent(EVENT_DARKNESS_SHADOWBLAST, 11000);
        scriptEvents.addEvent(EVENT_DARKNESS_CALL_DARKSPAWN, 16000);
        scriptEvents.addEvent(EVENT_DARKNESS_RISING_GLOOM, 24000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DARKNESS_UMBRAL_SLASH:
                castSpellOnVictim(SPELL_DARKNESS_UMBRAL_SLASH);
                scriptEvents.addEvent(EVENT_DARKNESS_UMBRAL_SLASH, 14000);
                break;
            case EVENT_DARKNESS_SHADOWBLAST:
                castAt(anyPlayer(), SPELL_DARKNESS_SHADOWBLAST);
                scriptEvents.addEvent(EVENT_DARKNESS_SHADOWBLAST, 18000);
                break;
            case EVENT_DARKNESS_CALL_DARKSPAWN:
            {
                warn("The Darkness calls forth Darkspawn to devour the light!");
                castSpellOnSelf(SPELL_DARKNESS_CALL_DARKSPAWN, true);
                uint32_t const count = isHeroic() ? 3 : 2;
                for (uint32_t i = 0; i < count; ++i)
                    callAdd(NPC_WRIGGLING_DARKSPAWN, aroundSelf(10.0f, 16.0f));
                scriptEvents.addEvent(EVENT_DARKNESS_CALL_DARKSPAWN, 32000);
                break;
            }
            case EVENT_DARKNESS_RISING_GLOOM:
                castSpellAOE(SPELL_DARKNESS_RISING_GLOOM);
                scriptEvents.addEvent(EVENT_DARKNESS_RISING_GLOOM, 28000);
                break;
            default:
                break;
        }
    }
};

// feeds on the nearest candle
class DarkflameCleftDarkspawnAI : public DarkflameCleftCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkflameCleftDarkspawnAI(c); }
    explicit DarkflameCleftDarkspawnAI(Creature* pCreature) : DarkflameCleftCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_DARKSPAWN_DRAIN_LIGHT, 4000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_DARKSPAWN_DRAIN_LIGHT)
            return;

        Creature* pCandle = findNearestCreature(NPC_DARKNESS_CANDLE, DarkspawnCandleSearch);
        if (pCandle != nullptr && pCandle->isAlive())
            castSpell(pCandle, SPELL_DARKNESS_DRAIN_LIGHT);

        scriptEvents.addEvent(EVENT_DARKSPAWN_DRAIN_LIGHT, 9000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Blazikon

class DarkflameCleftBlazikonAI : public DarkflameCleftBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkflameCleftBlazikonAI(c); }
    explicit DarkflameCleftBlazikonAI(Creature* pCreature) : DarkflameCleftBossAI(pCreature, DATA_BLAZIKON) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_BLAZIKON_WICKLIGHTER_BARRAGE, 5000);
        scriptEvents.addEvent(EVENT_BLAZIKON_INCITE_FLAMES, 10000);
        scriptEvents.addEvent(EVENT_BLAZIKON_DOUSING_BREATH, 16000);
        scriptEvents.addEvent(EVENT_BLAZIKON_REACH_CHECK, 3000);
    }

    // full energy sets every unlit candle ablaze
    void onCombatTick(uint32_t diff) override
    {
        if (!fillEnergy(diff, BlazikonEnergyPerSecond) || _isCasting())
            return;

        emptyEnergy();
        warn("Blazikon unleashes an Enkindling Inferno!");
        castSpellAOE(SPELL_BLAZIKON_ENKINDLING_INFERNO);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BLAZIKON_WICKLIGHTER_BARRAGE:
                castAt(anyPlayer(), SPELL_BLAZIKON_WICKLIGHTER_BARRAGE);
                scriptEvents.addEvent(EVENT_BLAZIKON_WICKLIGHTER_BARRAGE, 16000);
                break;
            case EVENT_BLAZIKON_INCITE_FLAMES:
                castAt(offTankPlayer(), SPELL_BLAZIKON_INCITE_FLAMES);
                scriptEvents.addEvent(EVENT_BLAZIKON_INCITE_FLAMES, 22000);
                break;
            case EVENT_BLAZIKON_DOUSING_BREATH:
                warn("Blazikon draws in a Dousing Breath!");
                castSpellAOE(SPELL_BLAZIKON_DOUSING_BREATH);
                scriptEvents.addEvent(EVENT_BLAZIKON_DOUSING_BREATH, 26000);
                break;
            case EVENT_BLAZIKON_REACH_CHECK:
                // nobody in melee: storms for everyone
                if (!anyPlayerInReach(BlazikonMeleeReach))
                    castSpellAOE(SPELL_BLAZIKON_BLAZING_STORMS);
                scriptEvents.addEvent(EVENT_BLAZIKON_REACH_CHECK, 3000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ol' Waxbeard

class DarkflameCleftOlWaxbeardAI : public DarkflameCleftBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkflameCleftOlWaxbeardAI(c); }
    explicit DarkflameCleftOlWaxbeardAI(Creature* pCreature) : DarkflameCleftBossAI(pCreature, DATA_OL_WAXBEARD) {}

protected:
    void onEngage() override
    {
        mChargeSpot = getCreature()->GetPosition();

        scriptEvents.addEvent(EVENT_WAXBEARD_KOL_TO_ARMS, 4000);
        scriptEvents.addEvent(EVENT_WAXBEARD_ROCK_BUSTER, 8000);
        scriptEvents.addEvent(EVENT_WAXBEARD_LURING_CANDLEFLAME, 14000);
        scriptEvents.addEvent(EVENT_WAXBEARD_RECKLESS_CHARGE, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_WAXBEARD_KOL_TO_ARMS:
            {
                warn("Ol' Waxbeard calls his kobolds to arms!");
                castSpellOnSelf(SPELL_WAXBEARD_KOL_TO_ARMS);
                uint32_t const count = isHeroic() ? WaxbeardLaborersPerWave + 1 : WaxbeardLaborersPerWave;
                for (uint32_t i = 0; i < count; ++i)
                    callAdd(NPC_MENIAL_LABORER, aroundSelf(8.0f, 14.0f));
                scriptEvents.addEvent(EVENT_WAXBEARD_KOL_TO_ARMS, 30000);
                break;
            }
            case EVENT_WAXBEARD_ROCK_BUSTER:
                castSpellOnVictim(SPELL_WAXBEARD_ROCK_BUSTER);
                scriptEvents.addEvent(EVENT_WAXBEARD_ROCK_BUSTER, 13000);
                break;
            case EVENT_WAXBEARD_LURING_CANDLEFLAME:
                if (Unit* pMarked = offTankPlayer())
                {
                    castSpell(pMarked, SPELL_WAXBEARD_LURING_CANDLEFLAME);
                    turnLaborersOn(pMarked);
                }
                scriptEvents.addEvent(EVENT_WAXBEARD_LURING_CANDLEFLAME, 24000);
                break;
            case EVENT_WAXBEARD_RECKLESS_CHARGE:
                if (Unit* pFar = farthestPlayer())
                {
                    warn("Ol' Waxbeard spurs Wick into a Reckless Charge!");
                    mChargeSpot = pFar->GetPosition();
                    castSpell(pFar, SPELL_WAXBEARD_RECKLESS_CHARGE);
                    scriptEvents.addEvent(EVENT_WAXBEARD_CAVE_IN, 2500);
                }
                scriptEvents.addEvent(EVENT_WAXBEARD_RECKLESS_CHARGE, 28000);
                break;
            case EVENT_WAXBEARD_CAVE_IN:
                castSpellAOE(SPELL_WAXBEARD_CAVE_IN);
                crushLaborers();
                break;
            default:
                break;
        }
    }

private:
    std::list<Creature*> nearbyLaborers()
    {
        std::list<Creature*> laborers;
        GetCreatureListWithEntryInGrid(laborers, NPC_MENIAL_LABORER, 100.0f);
        return laborers;
    }

    // every laborer goes after the marked player
    void turnLaborersOn(Unit* pMarked)
    {
        for (Creature* pLaborer : nearbyLaborers())
        {
            if (!pLaborer->isAlive() || !pLaborer->isInCombat() || pLaborer->GetScript() == nullptr)
                continue;

            pLaborer->GetScript()->_clearHateList();
            pLaborer->GetScript()->addThreat(pMarked, LaborerFixateThreat);
        }
    }

    // the charge and the falling rocks flatten kobolds at the impact
    void crushLaborers()
    {
        for (Creature* pLaborer : nearbyLaborers())
        {
            if (pLaborer->isAlive() && pLaborer->getDistance(mChargeSpot) <= CaveInReach)
                getCreature()->dealDamage(pLaborer, pLaborer->getHealth(), 0);
        }
    }

    LocationVector mChargeSpot;
};

// fixates a random player as soon as it joins
class DarkflameCleftMenialLaborerAI : public DarkflameCleftCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkflameCleftMenialLaborerAI(c); }
    explicit DarkflameCleftMenialLaborerAI(Creature* pCreature) : DarkflameCleftCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_LABORER_CRUDE_WEAPONS, true);

        if (Unit* pPrey = anyPlayer())
        {
            _clearHateList();
            addThreat(pPrey, LaborerFixateThreat);
        }
    }
};

void SetupDarkflameCleft(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_DARKFLAME_CLEFT, &DarkflameCleftInstanceScript::Create);

    mgr->register_creature_script(BOSS_THE_CANDLE_KING, &DarkflameCleftCandleKingAI::Create);
    mgr->register_creature_script(NPC_WAX_STATUE, &DarkflameCleftWaxStatueAI::Create);

    mgr->register_creature_script(BOSS_THE_DARKNESS, &DarkflameCleftDarknessAI::Create);
    mgr->register_creature_script(BOSS_THE_DARKNESS_ALT, &DarkflameCleftDarknessAI::Create);
    mgr->register_creature_script(NPC_WRIGGLING_DARKSPAWN, &DarkflameCleftDarkspawnAI::Create);

    mgr->register_creature_script(BOSS_BLAZIKON, &DarkflameCleftBlazikonAI::Create);

    mgr->register_creature_script(BOSS_OL_WAXBEARD, &DarkflameCleftOlWaxbeardAI::Create);
    mgr->register_creature_script(BOSS_OL_WAXBEARD_ALT, &DarkflameCleftOlWaxbeardAI::Create);
    mgr->register_creature_script(NPC_MENIAL_LABORER, &DarkflameCleftMenialLaborerAI::Create);
}
