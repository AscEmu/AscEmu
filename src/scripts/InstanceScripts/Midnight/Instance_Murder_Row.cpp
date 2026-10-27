/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Murder_Row.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <list>
#include <set>
#include <vector>

enum MurderRowEvents
{
    EVENT_KYSTIA_CHAOS_BARRAGE = 1,
    EVENT_KYSTIA_MIRROR_IMAGES,
    EVENT_KYSTIA_FEL_NOVA,
    EVENT_KYSTIA_REINFUSE,
    EVENT_KYSTIA_REINFUSE_DONE,

    EVENT_NIBBLES_FEL_SPRAY,
    EVENT_NIBBLES_CORRODING_SPITTLE,

    EVENT_ZAEN_ENVENOM,
    EVENT_ZAEN_SAME_DAY_DELIVERY,
    EVENT_ZAEN_FIRE_BOMB,
    EVENT_ZAEN_KILLING_SPREE,
    EVENT_ZAEN_FREIGHT_BLAST,

    EVENT_XATHUUX_LEGION_STRIKE,
    EVENT_XATHUUX_INFERNAL_CRUSH,
    EVENT_XATHUUX_AXE_TOSS,
    EVENT_XATHUUX_RAGE_FADES,

    EVENT_LITHIEL_CHAOS_BOLT,
    EVENT_LITHIEL_SEARING_FEL_FLAME,
    EVENT_LITHIEL_SUMMON_VILEFIEND,
    EVENT_LITHIEL_FINGERS_OF_GULDAN,
    EVENT_LITHIEL_IMPS_ARRIVE,
    EVENT_LITHIEL_DEMONIC_GATEWAY,
    EVENT_LITHIEL_MALEFIC_WAVE,
    EVENT_LITHIEL_WAVE_PASSED,

    EVENT_DEMON_ABILITY
};

namespace
{
    float const MurderRowTwoPi = 6.2831853f;

    // Felshield layers before the pull, after each reclaim one fewer down to one
    uint32_t const KystiaOpeningLayers = 4;
    uint32_t const KystiaFirstReclaimLayers = 3;

    // Illicit Infusion lets go of Nibbles below this share of health
    int32_t const NibblesBreakPct = 20;
    float const NibblesEscapeRise = 20.0f;

    uint32_t const XathuuxBurningStepsMs = 15000;

    size_t const LithielMaxFingers = 5;
    float const LithielFingersReach = 60.0f;

    LocationVector murderRowRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(MurderRowTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class MurderRowInstanceScript : public InstanceScript
{
public:
    explicit MurderRowInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(MurderRowEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new MurderRowInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        if (pCreature->getEntry() == BOSS_KYSTIA)
            mKystiaGuid = pCreature->GetNewGUID();
        else if (pCreature->getEntry() == NPC_KYSTIA_NIBBLES)
            mNibblesGuid = pCreature->GetNewGUID();
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_KYSTIA_FEL_DOOR && mFelDoorGuids.insert(pGameObject->GetNewGUID()).second)
            applyFelDoor(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t /*state*/) override
    {
        if (id != DATA_KYSTIA)
            return;

        for (WoWGuid const& guid : mFelDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyFelDoor(pDoor);
        }
    }

    Creature* getKystia() { return getCreatureByGuid(mKystiaGuid); }
    Creature* getNibbles() { return getCreatureByGuid(mNibblesGuid); }

    void tell(Creature* pTarget, int32_t action)
    {
        if (pTarget != nullptr && pTarget->isAlive() && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }

private:
    // the hideout is shut only while Kystia fights
    void applyFelDoor(GameObject* pDoor)
    {
        uint8_t const wanted = getBossState(DATA_KYSTIA) == EncounterStates::InProgress ? GO_STATE_CLOSED : GO_STATE_OPEN;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    WoWGuid mKystiaGuid;
    WoWGuid mNibblesGuid;
    std::set<WoWGuid> mFelDoorGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy bar and target helpers

class MurderRowCombatAI : public CreatureAIScript
{
public:
    explicit MurderRowCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnergyFillMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mEnergyFillMs = 0;

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
        tickEnergy(diff);
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
    virtual void onEnergyFull() {}

    MurderRowInstanceScript* getRow()
    {
        if (getCreature()->GetMapId() != MAP_MURDER_ROW)
            return nullptr;

        return static_cast<MurderRowInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank, the tank when alone
    Unit* anyPlayerButTank()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_NotCurrent);
        if (Unit* pTarget = selectUnitTarget(args))
            return pTarget;

        return anyPlayer();
    }

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    void castUnder(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId, triggered);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return murderRowRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // the bar fills from empty in fillMs
    void armEnergy(uint32_t fillMs)
    {
        mEnergyFillMs = fillMs;
        mEnergyMs = 0;
        mEnergyFull = false;
    }

private:
    void tickEnergy(uint32_t diff)
    {
        if (mEnergyFillMs == 0)
            return;

        if (!mEnergyFull)
        {
            mEnergyMs += diff;
            mEnergyFull = mEnergyMs >= mEnergyFillMs;
        }

        // a full bar waits for the running cast
        if (!mEnergyFull || _isCasting())
            return;

        mEnergyFull = false;
        mEnergyMs = 0;
        onEnergyFull();
    }

    uint32_t mEnergyFillMs = 0;
    uint32_t mEnergyMs = 0;
    bool mEnergyFull = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class MurderRowBossAI : public MurderRowCombatAI
{
public:
    MurderRowBossAI(Creature* pCreature, uint32_t dataId) : MurderRowCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        MurderRowCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        MurderRowCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        onDefeat();
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kystia Manaheart, shielded by Felshield while her mana wyrm serves her

class MurderRowKystiaAI : public MurderRowBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MurderRowKystiaAI(c); }
    explicit MurderRowKystiaAI(Creature* pCreature) : MurderRowBossAI(pCreature, DATA_KYSTIA) {}

    void OnLoad() override { layFelshield(KystiaOpeningLayers); }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_NIBBLES_TURNED || !isAlive() || !_isInCombat())
            return;

        // the wyrm's light strips every layer and staggers her
        warn("Nibbles breaks free and Destabilizes Kystia Manaheart!");
        _removeAura(SPELL_KYSTIA_FELSHIELD);
        castSpellOnSelf(SPELL_KYSTIA_DESTABILIZED, true);
        scriptEvents.addEvent(EVENT_KYSTIA_REINFUSE, 10000);
    }

protected:
    void onEngage() override
    {
        mReclaims = 0;
        if (!hasAura(SPELL_KYSTIA_FELSHIELD))
            layFelshield(KystiaOpeningLayers);

        if (MurderRowInstanceScript* pRow = getRow())
        {
            Creature* pNibbles = pRow->getNibbles();
            if (pNibbles != nullptr && pNibbles->isAlive())
            {
                pRow->sendUnitEncounter(EncounterFrameEngage, pNibbles);
                if (!pNibbles->isInCombat())
                    setZoneWideCombat(pNibbles);
            }
        }

        scriptEvents.addEvent(EVENT_KYSTIA_CHAOS_BARRAGE, 1000);
        scriptEvents.addEvent(EVENT_KYSTIA_MIRROR_IMAGES, 14000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_KYSTIA_FEL_NOVA, 12000);
    }

    // a fresh wyrm and a fresh shield for the next attempt
    void onReset() override
    {
        _removeAura(SPELL_KYSTIA_DESTABILIZED);
        layFelshield(KystiaOpeningLayers);

        if (MurderRowInstanceScript* pRow = getRow())
        {
            if (Creature* pNibbles = pRow->getNibbles())
            {
                pRow->sendUnitEncounter(EncounterFrameDisengaged, pNibbles);
                pNibbles->despawn(1000, 5000);
            }
        }
    }

    void onDefeat() override
    {
        MurderRowInstanceScript* pRow = getRow();
        Creature* pNibbles = pRow != nullptr ? pRow->getNibbles() : nullptr;
        if (pNibbles == nullptr)
            return;

        pRow->sendUnitEncounter(EncounterFrameDisengaged, pNibbles);
        pRow->tell(pNibbles, ACTION_NIBBLES_FLEES);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KYSTIA_CHAOS_BARRAGE:
                castSpellOnVictim(SPELL_KYSTIA_CHAOS_BARRAGE);
                scriptEvents.addEvent(EVENT_KYSTIA_CHAOS_BARRAGE, 3500);
                break;
            case EVENT_KYSTIA_MIRROR_IMAGES:
                warn("Kystia Manaheart conjures Mirror Images!");
                castSpellOnSelf(SPELL_KYSTIA_MIRROR_IMAGES);
                scriptEvents.addEvent(EVENT_KYSTIA_MIRROR_IMAGES, 30500);
                break;
            case EVENT_KYSTIA_FEL_NOVA:
                blinkAndNova();
                scriptEvents.addEvent(EVENT_KYSTIA_FEL_NOVA, 15000);
                break;
            case EVENT_KYSTIA_REINFUSE:
                reinfuseNibbles();
                break;
            case EVENT_KYSTIA_REINFUSE_DONE:
                reclaimNibbles();
                break;
            default:
                break;
        }
    }

private:
    // stacks of the shield are separate applications
    void layFelshield(uint32_t layers)
    {
        _removeAura(SPELL_KYSTIA_FELSHIELD);
        for (uint32_t i = 0; i < layers; ++i)
            castSpellOnSelf(SPELL_KYSTIA_FELSHIELD, true);
    }

    void blinkAndNova()
    {
        Unit* pTarget = anyPlayerButTank();
        if (pTarget == nullptr)
            return;

        MoveTeleport(pTarget->GetPosition());
        castSpellAOE(SPELL_KYSTIA_FEL_NOVA);
    }

    void reinfuseNibbles()
    {
        MurderRowInstanceScript* pRow = getRow();
        Creature* pNibbles = pRow != nullptr ? pRow->getNibbles() : nullptr;
        if (pNibbles == nullptr || !pNibbles->isAlive())
            return;

        warn("Kystia Manaheart forces an Illicit Infusion back into Nibbles!");
        castSpell(pNibbles, SPELL_KYSTIA_INFUSE_NIBBLES);
        scriptEvents.addEvent(EVENT_KYSTIA_REINFUSE_DONE, 500);
    }

    // each reclaim leaves her with a thinner shield
    void reclaimNibbles()
    {
        uint32_t const layers = mReclaims < KystiaFirstReclaimLayers - 1 ? KystiaFirstReclaimLayers - mReclaims : 1;
        ++mReclaims;
        layFelshield(layers);

        if (MurderRowInstanceScript* pRow = getRow())
            pRow->tell(pRow->getNibbles(), ACTION_NIBBLES_RECLAIMED);
    }

    uint32_t mReclaims = 0;
};

// Kystia's mana wyrm, cannot fall and turns on her below a fifth of its health
class MurderRowNibblesAI : public MurderRowCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MurderRowNibblesAI(c); }
    explicit MurderRowNibblesAI(Creature* pCreature) : MurderRowCombatAI(pCreature) {}

    void OnLoad() override
    {
        mHostileFaction = getCreature()->getFactionTemplate();
        mFree = false;

        // Kystia already fell, the wyrm stays a harmless pet
        if (InstanceScript* pInstance = getInstanceScript())
        {
            if (getCreature()->GetMapId() == MAP_MURDER_ROW && pInstance->getBossState(DATA_KYSTIA) == EncounterStates::Performed)
                turnFriendly();
        }
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        uint32_t const health = getCreature()->getHealth();
        if (*damage >= health)
            *damage = health > 1 ? health - 1 : 0;
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_NIBBLES_RECLAIMED)
            reclaim();
        else if (action == ACTION_NIBBLES_FLEES)
            flee();
    }

protected:
    void onEngage() override
    {
        if (mFree)
            return;

        // the wyrm pulls its mistress along
        if (MurderRowInstanceScript* pRow = getRow())
        {
            Creature* pKystia = pRow->getKystia();
            if (pKystia != nullptr && pKystia->isAlive() && !pKystia->isInCombat())
                setZoneWideCombat(pKystia);
        }

        scriptEvents.addEvent(EVENT_NIBBLES_CORRODING_SPITTLE, 4500);
        scriptEvents.addEvent(EVENT_NIBBLES_FEL_SPRAY, 8000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mFree && _getHealthPercent() < NibblesBreakPct)
            breakFree();
    }

    void onEvent(uint32_t eventId) override
    {
        if (mFree)
            return;

        switch (eventId)
        {
            case EVENT_NIBBLES_CORRODING_SPITTLE:
                castAt(anyPlayer(), SPELL_NIBBLES_CORRODING_SPITTLE);
                scriptEvents.addEvent(EVENT_NIBBLES_CORRODING_SPITTLE, 14500);
                break;
            case EVENT_NIBBLES_FEL_SPRAY:
                warn("Nibbles spews a stream of Fel Spray!");
                castAt(anyPlayer(), SPELL_NIBBLES_FEL_SPRAY);
                scriptEvents.addEvent(EVENT_NIBBLES_FEL_SPRAY, 51000);
                break;
            default:
                break;
        }
    }

private:
    void turnFriendly()
    {
        mFree = true;
        scriptEvents.resetEvents();
        getCreature()->interruptSpell();
        _removeAura(SPELL_NIBBLES_ILLICIT_INFUSION);
        setReactState(REACT_PASSIVE);
        getCreature()->setFaction(35);
    }

    // the infusion lets go, the wyrm lends its light to the players
    void breakFree()
    {
        turnFriendly();

        MurderRowInstanceScript* pRow = getRow();
        Creature* pKystia = pRow != nullptr ? pRow->getKystia() : nullptr;
        if (pKystia == nullptr || !pKystia->isAlive())
            return;

        castSpell(pKystia, SPELL_NIBBLES_LIGHT_INFUSION, true);
        pRow->tell(pKystia, ACTION_NIBBLES_TURNED);
    }

    void reclaim()
    {
        if (!mFree || !isAlive())
            return;

        mFree = false;
        getCreature()->setFaction(mHostileFaction);
        getCreature()->setFullHealth();
        castSpellOnSelf(SPELL_NIBBLES_ILLICIT_INFUSION, true);
        setReactState(REACT_AGGRESSIVE);
        setZoneWideCombat();
    }

    // Kystia is gone, the wyrm flies off
    void flee()
    {
        turnFriendly();
        castSpellOnSelf(SPELL_NIBBLES_ESCAPE, true);
        setFlyMode(true);

        Creature* pSelf = getCreature();
        moveTo(pSelf->GetPositionX(), pSelf->GetPositionY(), pSelf->GetPositionZ() + NibblesEscapeRise, false);
        despawn(5000, 0);
    }

    uint32_t mHostileFaction = 0;
    bool mFree = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Zaen Bladesorrow

class MurderRowZaenAI : public MurderRowBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MurderRowZaenAI(c); }
    explicit MurderRowZaenAI(Creature* pCreature) : MurderRowBossAI(pCreature, DATA_ZAEN) {}

protected:
    void onEngage() override
    {
        armEnergy(40000);
        scriptEvents.addEvent(EVENT_ZAEN_ENVENOM, 6000);
        scriptEvents.addEvent(EVENT_ZAEN_SAME_DAY_DELIVERY, 9000);
        scriptEvents.addEvent(EVENT_ZAEN_FIRE_BOMB, 14000);
        scriptEvents.addEvent(EVENT_ZAEN_KILLING_SPREE, 22000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ZAEN_ENVENOM:
                castSpellOnVictim(SPELL_ZAEN_ENVENOM);
                scriptEvents.addEvent(EVENT_ZAEN_ENVENOM, 13000);
                break;
            case EVENT_ZAEN_SAME_DAY_DELIVERY:
                deliverFreight();
                scriptEvents.addEvent(EVENT_ZAEN_SAME_DAY_DELIVERY, 26000);
                break;
            case EVENT_ZAEN_FIRE_BOMB:
                castSpellAOE(SPELL_ZAEN_FIRE_BOMB);
                scriptEvents.addEvent(EVENT_ZAEN_FIRE_BOMB, 19000);
                break;
            case EVENT_ZAEN_KILLING_SPREE:
                castSpellOnSelf(SPELL_ZAEN_KILLING_SPREE);
                scriptEvents.addEvent(EVENT_ZAEN_KILLING_SPREE, 32000);
                break;
            case EVENT_ZAEN_FREIGHT_BLAST:
                detonateFreight();
                break;
            default:
                break;
        }
    }

    // only the crates give cover
    void onEnergyFull() override
    {
        warn("Zaen Bladesorrow takes aim: Murder in a Row! Get behind the freight!");
        castSpellOnSelf(SPELL_ZAEN_MURDER_IN_A_ROW);

        // heroic: the shot up crates go off once the volley is over
        if (isHeroic())
            scriptEvents.addEvent(EVENT_ZAEN_FREIGHT_BLAST, 500);
    }

private:
    // crates drop where two players stand
    void deliverFreight()
    {
        castSpellAOE(SPELL_ZAEN_SAME_DAY_DELIVERY);
        for (uint8_t i = 0; i < 2; ++i)
        {
            if (Unit* pTarget = anyPlayer())
                summonCreature(NPC_ZAEN_FORBIDDEN_FREIGHT, murderRowRingPoint(pTarget->GetPosition(), 1.0f, 3.0f), MANUAL_DESPAWN);
        }
    }

    void detonateFreight()
    {
        std::list<Creature*> crates;
        GetCreatureListWithEntryInGrid(crates, NPC_ZAEN_FORBIDDEN_FREIGHT, 100.0f);

        if (MurderRowInstanceScript* pRow = getRow())
        {
            for (Creature* pCrate : crates)
                pRow->tell(pCrate, ACTION_FREIGHT_DETONATE);
        }
    }
};

// cover for Murder in a Row
class MurderRowForbiddenFreightAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MurderRowForbiddenFreightAI(c); }
    explicit MurderRowForbiddenFreightAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_FREIGHT_DETONATE || mDetonated)
            return;

        mDetonated = true;
        castSpellOnSelf(SPELL_FREIGHT_EXPLOSION, true);
        despawn(1500, 0);
    }

private:
    bool mDetonated = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Xathuux the Annihilator

class MurderRowXathuuxAI : public MurderRowBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MurderRowXathuuxAI(c); }
    explicit MurderRowXathuuxAI(Creature* pCreature) : MurderRowBossAI(pCreature, DATA_XATHUUX) {}

protected:
    void onEngage() override
    {
        armEnergy(45000);
        scriptEvents.addEvent(EVENT_XATHUUX_LEGION_STRIKE, 5000);
        scriptEvents.addEvent(EVENT_XATHUUX_INFERNAL_CRUSH, 11000);
        scriptEvents.addEvent(EVENT_XATHUUX_AXE_TOSS, 17000);
    }

    void onReset() override
    {
        _removeAura(SPELL_XATHUUX_DEMONIC_RAGE);
        _removeAura(SPELL_XATHUUX_BURNING_STEPS);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_XATHUUX_LEGION_STRIKE:
                castSpellOnVictim(SPELL_XATHUUX_LEGION_STRIKE);
                scriptEvents.addEvent(EVENT_XATHUUX_LEGION_STRIKE, 13000);
                break;
            case EVENT_XATHUUX_INFERNAL_CRUSH:
                castUnder(anyPlayer(), SPELL_XATHUUX_INFERNAL_CRUSH);
                scriptEvents.addEvent(EVENT_XATHUUX_INFERNAL_CRUSH, 21000);
                break;
            case EVENT_XATHUUX_AXE_TOSS:
                warn("Xathuux the Annihilator hurls his axe!");
                castUnder(anyPlayerButTank(), SPELL_XATHUUX_AXE_TOSS);
                scriptEvents.addEvent(EVENT_XATHUUX_AXE_TOSS, 30000);
                break;
            case EVENT_XATHUUX_RAGE_FADES:
                _removeAura(SPELL_XATHUUX_BURNING_STEPS);
                break;
            default:
                break;
        }
    }

    // his rage boils over, every step leaves fel fire behind
    void onEnergyFull() override
    {
        warn("Xathuux the Annihilator erupts with Demonic Rage!");
        castSpellOnSelf(SPELL_XATHUUX_DEMONIC_RAGE);
        castSpellOnSelf(SPELL_XATHUUX_BURNING_STEPS, true);
        scriptEvents.addEvent(EVENT_XATHUUX_RAGE_FADES, XathuuxBurningStepsMs);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lithiel Cinderfury

class MurderRowLithielAI : public MurderRowBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MurderRowLithielAI(c); }
    explicit MurderRowLithielAI(Creature* pCreature) : MurderRowBossAI(pCreature, DATA_LITHIEL) {}

protected:
    void onEngage() override
    {
        mFingerSpots.clear();
        scriptEvents.addEvent(EVENT_LITHIEL_CHAOS_BOLT, 4000);
        scriptEvents.addEvent(EVENT_LITHIEL_SEARING_FEL_FLAME, 8000);
        scriptEvents.addEvent(EVENT_LITHIEL_SUMMON_VILEFIEND, 12000);
        scriptEvents.addEvent(EVENT_LITHIEL_FINGERS_OF_GULDAN, 20000);
    }

    void onReset() override { _removeAura(SPELL_LITHIEL_FELSHIELD); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LITHIEL_CHAOS_BOLT:
                castAt(anyPlayer(), SPELL_LITHIEL_CHAOS_BOLT);
                scriptEvents.addEvent(EVENT_LITHIEL_CHAOS_BOLT, 9000);
                break;
            case EVENT_LITHIEL_SEARING_FEL_FLAME:
                castSpellAOE(SPELL_LITHIEL_SEARING_FEL_FLAME);
                scriptEvents.addEvent(EVENT_LITHIEL_SEARING_FEL_FLAME, 15000);
                break;
            case EVENT_LITHIEL_SUMMON_VILEFIEND:
                callAdd(NPC_LITHIEL_FURIOUS_VILEFIEND, aroundSelf(4.0f, 8.0f));
                scriptEvents.addEvent(EVENT_LITHIEL_SUMMON_VILEFIEND, 40000);
                break;
            case EVENT_LITHIEL_FINGERS_OF_GULDAN:
                pointFingers();
                scriptEvents.addEvent(EVENT_LITHIEL_FINGERS_OF_GULDAN, 50000);
                break;
            case EVENT_LITHIEL_IMPS_ARRIVE:
                ensureImps();
                break;
            case EVENT_LITHIEL_DEMONIC_GATEWAY:
                castSpellOnSelf(SPELL_LITHIEL_DEMONIC_GATEWAY);
                scriptEvents.addEvent(EVENT_LITHIEL_MALEFIC_WAVE, 2000);
                break;
            case EVENT_LITHIEL_MALEFIC_WAVE:
                warn("Lithiel Cinderfury sends out a Malefic Wave!");
                castSpellOnSelf(SPELL_LITHIEL_FELSHIELD, true);
                castAt(anyPlayer(), SPELL_LITHIEL_MALEFIC_WAVE);
                scriptEvents.addEvent(EVENT_LITHIEL_WAVE_PASSED, 8000);
                break;
            case EVENT_LITHIEL_WAVE_PASSED:
                _removeAura(SPELL_LITHIEL_FELSHIELD);
                empowerDemons();
                break;
            default:
                break;
        }
    }

private:
    // a fel hand comes down on every player, an imp crawls out of each
    void pointFingers()
    {
        warn("Lithiel Cinderfury calls down the Fingers of Gul'dan!");
        mFingerSpots.clear();

        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer == nullptr || !pPlayer->isAlive() || getCreature()->getDistance(pPlayer) > LithielFingersReach)
                continue;

            getCreature()->castSpellLoc(pPlayer->GetPosition(), SPELL_LITHIEL_FINGERS_OF_GULDAN, true);
            mFingerSpots.push_back(pPlayer->GetPosition());
            if (mFingerSpots.size() >= LithielMaxFingers)
                break;
        }

        scriptEvents.addEvent(EVENT_LITHIEL_IMPS_ARRIVE, 1500);
        scriptEvents.addEvent(EVENT_LITHIEL_DEMONIC_GATEWAY, 3000);
    }

    // the spell may not have brought the imps, they are called in by hand then
    void ensureImps()
    {
        if (!summons.hasEntry(NPC_LITHIEL_WILD_IMP))
        {
            for (LocationVector const& spot : mFingerSpots)
                callAdd(NPC_LITHIEL_WILD_IMP, murderRowRingPoint(spot, 1.0f, 3.0f));
        }

        mFingerSpots.clear();
    }

    // demons still standing when the wave rolls over them grow stronger
    void empowerDemons()
    {
        uint32_t const demons[2] = { NPC_LITHIEL_WILD_IMP, NPC_LITHIEL_FURIOUS_VILEFIEND };
        for (uint32_t entry : demons)
        {
            std::list<Creature*> found;
            GetCreatureListWithEntryInGrid(found, entry, 80.0f);
            for (Creature* pDemon : found)
            {
                if (pDemon->isAlive())
                    castSpell(pDemon, SPELL_LITHIEL_MALEFIC_EMPOWERMENT, true);
            }
        }
    }

    std::vector<LocationVector> mFingerSpots;
};

// summoned demon with one timed ability
class MurderRowDemonAI : public MurderRowCombatAI
{
public:
    static CreatureAIScript* CreateWildImp(Creature* c) { return new MurderRowDemonAI(c, SPELL_WILD_IMP_FELFIRE_BURST, 2000, 6000, false); }
    static CreatureAIScript* CreateVilefiend(Creature* c) { return new MurderRowDemonAI(c, SPELL_VILEFIEND_SHADOW_BITE, 3000, 8000, true); }

    MurderRowDemonAI(Creature* pCreature, uint32_t spellId, uint32_t firstMs, uint32_t repeatMs, bool onVictim)
        : MurderRowCombatAI(pCreature), mSpellId(spellId), mFirstMs(firstMs), mRepeatMs(repeatMs), mOnVictim(onVictim) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_DEMON_ABILITY, mFirstMs); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_DEMON_ABILITY)
            return;

        if (mOnVictim)
            castSpellOnVictim(mSpellId);
        else
            castAt(anyPlayer(), mSpellId);

        scriptEvents.addEvent(EVENT_DEMON_ABILITY, mRepeatMs);
    }

private:
    uint32_t mSpellId;
    uint32_t mFirstMs;
    uint32_t mRepeatMs;
    bool mOnVictim;
};

void SetupMurderRow(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MURDER_ROW, &MurderRowInstanceScript::Create);

    mgr->register_creature_script(BOSS_KYSTIA, &MurderRowKystiaAI::Create);
    mgr->register_creature_script(NPC_KYSTIA_NIBBLES, &MurderRowNibblesAI::Create);

    mgr->register_creature_script(BOSS_ZAEN, &MurderRowZaenAI::Create);
    mgr->register_creature_script(NPC_ZAEN_FORBIDDEN_FREIGHT, &MurderRowForbiddenFreightAI::Create);

    mgr->register_creature_script(BOSS_XATHUUX, &MurderRowXathuuxAI::Create);

    mgr->register_creature_script(BOSS_LITHIEL, &MurderRowLithielAI::Create);
    mgr->register_creature_script(BOSS_LITHIEL_ALT, &MurderRowLithielAI::Create);
    mgr->register_creature_script(NPC_LITHIEL_WILD_IMP, &MurderRowDemonAI::CreateWildImp);
    mgr->register_creature_script(NPC_LITHIEL_FURIOUS_VILEFIEND, &MurderRowDemonAI::CreateVilefiend);
}
