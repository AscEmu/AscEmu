/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Nokhud_Offensive.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <map>
#include <vector>

enum TheNokhudOffensiveEvents
{
    EVENT_BALAKAR_IRON_SPEAR = 1,
    EVENT_BALAKAR_UPHEAVAL,
    EVENT_BALAKAR_RENDING_STRIKE,
    EVENT_BALAKAR_SAVAGE_STRIKE,
    EVENT_BALAKAR_LIGHTNING,
    EVENT_BALAKAR_STATIC_SPEAR,
    EVENT_BALAKAR_CRACKLING_UPHEAVAL,
    EVENT_BALAKAR_CONDUCTIVE_STRIKE,
    EVENT_BALAKAR_THUNDER_STRIKE,

    EVENT_STORMCASTER_STORM_BOLT,

    EVENT_TEERA_QUICK_SHOT,
    EVENT_TEERA_SPIRIT_LEAP,
    EVENT_TEERA_REPEL,

    EVENT_MARUUK_BRUTALIZE,
    EVENT_MARUUK_FRIGHTFUL_ROAR,

    EVENT_TEMPEST_LIGHTNING_STRIKE,
    EVENT_TEMPEST_ENERGY_SURGE,
    EVENT_TEMPEST_WIND_BURST,

    EVENT_GRANYTH_SHARDS_OF_STONE,
    EVENT_GRANYTH_TECTONIC_STOMP,
    EVENT_GRANYTH_SABOTEUR
};

namespace
{
    // members of the encounters fought as one
    struct NokhudGroupSlot
    {
        uint32_t dataId;
        uint32_t entry;
    };

    NokhudGroupSlot const NokhudGroupSlots[] =
    {
        { DATA_TEERA_AND_MARUUK, BOSS_TEERA },
        { DATA_TEERA_AND_MARUUK, BOSS_MARUUK }
    };

    // Balakar's stages
    uint8_t const BalakarStageMight = 1;
    uint8_t const BalakarStageIntermission = 2;
    uint8_t const BalakarStageStorm = 3;

    int32_t const BalakarIntermissionPct = 60;
    uint8_t const BalakarStormcasterCount = 3;

    // energy bars, time from empty to full
    uint32_t const TeeraGaleArrowMs = 40000;
    uint32_t const MaruukEarthsplitterMs = 50000;
    uint32_t const TempestStormMs = 45000;
    uint32_t const GranythEruptionMs = 45000;

    // without a lance to stop it the eruption runs out on its own
    uint32_t const GranythEruptionLimitMs = 12000;

    float const SaboteurLanceSearch = 150.0f;
    float const SaboteurWorkRange = 5.0f;

    LocationVector nokhudRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(6.2831853f);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class NokhudOffensiveInstanceScript : public InstanceScript
{
public:
    explicit NokhudOffensiveInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheNokhudOffensiveEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new NokhudOffensiveInstanceScript(pMapMgr); }

    // one fallen centaur must not end the encounter while the other stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (NokhudGroupSlot const& slot : NokhudGroupSlots)
        {
            if (slot.entry == pCreature->getEntry())
            {
                mMemberGuids[slot.entry] = pCreature->GetNewGUID();
                return;
            }
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (state != EncounterStates::Failed)
            return;

        // a wiped group fights again in full
        for (Creature* pMember : getGroup(id))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    Creature* getMember(uint32_t entry)
    {
        auto const itr = mMemberGuids.find(entry);
        return itr != mMemberGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getGroup(uint32_t dataId)
    {
        std::vector<Creature*> group;
        for (NokhudGroupSlot const& slot : NokhudGroupSlots)
        {
            if (slot.dataId != dataId)
                continue;

            if (Creature* pMember = getMember(slot.entry))
                group.push_back(pMember);
        }

        return group;
    }

    bool isGroupDown(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    bool isGroupFighting(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

    // pulling one member pulls the rest
    void rallyGroup(uint32_t dataId, Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getGroup(dataId))
        {
            if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat() || pMember->GetScript() == nullptr)
                continue;

            pMember->GetScript()->setZoneWideCombat();
        }
        mRallying = false;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        Creature* pMember = getMember(entry);
        if (pMember != nullptr && pMember->isAlive() && pMember->GetScript() != nullptr)
            pMember->GetScript()->DoAction(action);
    }

private:
    std::map<uint32_t, WoWGuid> mMemberGuids;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy stand-in and target helpers

class NokhudOffensiveCombatAI : public CreatureAIScript
{
public:
    explicit NokhudOffensiveCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnergyMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mEnergyMs = 0;

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

    NokhudOffensiveInstanceScript* getNokhud()
    {
        if (getCreature()->GetMapId() != MAP_THE_NOKHUD_OFFENSIVE)
            return nullptr;

        return static_cast<NokhudOffensiveInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    Unit* castAtPlayer(uint32_t spellId, bool triggered = false)
    {
        Unit* pTarget = anyPlayer();
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    // normal difficulty has its own spell versions
    uint32_t byDifficulty(uint32_t heroicSpellId, uint32_t normalSpellId) { return isHeroic() ? heroicSpellId : normalSpellId; }

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return nokhudRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void notify(uint32_t entry, int32_t action)
    {
        if (NokhudOffensiveInstanceScript* pNokhud = getNokhud())
            pNokhud->sendAction(entry, action);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // energy stand-in: true once the bar is full and nothing is being cast
    bool energyFull(uint32_t diff, uint32_t fullAfterMs)
    {
        if (mEnergyMs < fullAfterMs)
            mEnergyMs += diff;

        if (mEnergyMs < fullAfterMs || _isCasting())
            return false;

        mEnergyMs = 0;
        return true;
    }

    uint32_t mEnergyMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class NokhudOffensiveBossAI : public NokhudOffensiveCombatAI
{
public:
    NokhudOffensiveBossAI(Creature* pCreature, uint32_t dataId) : NokhudOffensiveCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (NokhudOffensiveInstanceScript* pNokhud = getNokhud())
        {
            pNokhud->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pNokhud->setBossState(mDataId, EncounterStates::InProgress);
            pNokhud->rallyGroup(mDataId, getCreature());
        }

        NokhudOffensiveCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (NokhudOffensiveInstanceScript* pNokhud = getNokhud())
            {
                pNokhud->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                if (!pNokhud->isGroupFighting(mDataId, getCreature()))
                    pNokhud->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        NokhudOffensiveCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (NokhudOffensiveInstanceScript* pNokhud = getNokhud())
        {
            pNokhud->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            if (pNokhud->isGroupDown(mDataId, getCreature()))
                pNokhud->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Balakar Khan

class NokhudOffensiveBalakarKhanAI : public NokhudOffensiveBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NokhudOffensiveBalakarKhanAI(c); }
    explicit NokhudOffensiveBalakarKhanAI(Creature* pCreature) : NokhudOffensiveBossAI(pCreature, DATA_BALAKAR_KHAN) {}

    // untouchable while the ritual runs
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mStage == BalakarStageIntermission)
            *damage = 0;
    }

protected:
    void onEngage() override
    {
        mStage = BalakarStageMight;
        mStormcasterGuids.clear();

        scriptEvents.addEvent(EVENT_BALAKAR_RENDING_STRIKE, 6000);
        scriptEvents.addEvent(EVENT_BALAKAR_IRON_SPEAR, 10000);
        scriptEvents.addEvent(EVENT_BALAKAR_UPHEAVAL, 16000);
    }

    void onReset() override
    {
        if (mStage == BalakarStageIntermission)
            leaveRitual();

        mStage = 0;
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == BalakarStageMight && _getHealthPercent() <= BalakarIntermissionPct && !_isCasting())
        {
            enterRitual();
            return;
        }

        if (mStage == BalakarStageIntermission && stormcastersDown())
            enterStorm();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BALAKAR_IRON_SPEAR:
                warn("Balakar Khan hurls his Iron Spear!");
                castAtPlayer(SPELL_BALAKAR_IRON_SPEAR);
                scriptEvents.addEvent(EVENT_BALAKAR_IRON_SPEAR, 22000);
                break;
            case EVENT_BALAKAR_UPHEAVAL:
                castSpellAOE(SPELL_BALAKAR_UPHEAVAL);
                scriptEvents.addEvent(EVENT_BALAKAR_UPHEAVAL, 26000);
                break;
            case EVENT_BALAKAR_RENDING_STRIKE:
                castSpellOnVictim(SPELL_BALAKAR_RENDING_STRIKE);
                scriptEvents.addEvent(EVENT_BALAKAR_SAVAGE_STRIKE, 2000);
                scriptEvents.addEvent(EVENT_BALAKAR_RENDING_STRIKE, 16000);
                break;
            case EVENT_BALAKAR_SAVAGE_STRIKE:
                castSpellOnVictim(SPELL_BALAKAR_SAVAGE_STRIKE);
                break;
            case EVENT_BALAKAR_LIGHTNING:
                castAtPlayer(SPELL_BALAKAR_LIGHTNING);
                scriptEvents.addEvent(EVENT_BALAKAR_LIGHTNING, mStage == BalakarStageIntermission ? 4000 : 12000);
                break;
            case EVENT_BALAKAR_STATIC_SPEAR:
                warn("Balakar Khan hurls a Static Spear!");
                castAtPlayer(SPELL_BALAKAR_STATIC_SPEAR);
                scriptEvents.addEvent(EVENT_BALAKAR_STATIC_SPEAR, 22000);
                break;
            case EVENT_BALAKAR_CRACKLING_UPHEAVAL:
                castSpellAOE(SPELL_BALAKAR_CRACKLING_UPHEAVAL);
                scriptEvents.addEvent(EVENT_BALAKAR_CRACKLING_UPHEAVAL, 26000);
                break;
            case EVENT_BALAKAR_CONDUCTIVE_STRIKE:
                castSpellOnVictim(SPELL_BALAKAR_CONDUCTIVE_STRIKE);
                scriptEvents.addEvent(EVENT_BALAKAR_THUNDER_STRIKE, 2000);
                scriptEvents.addEvent(EVENT_BALAKAR_CONDUCTIVE_STRIKE, 16000);
                break;
            case EVENT_BALAKAR_THUNDER_STRIKE:
                castSpellOnVictim(SPELL_BALAKAR_THUNDER_STRIKE);
                break;
            default:
                break;
        }
    }

private:
    // the stormcasters hold the line while he channels the storm
    void enterRitual()
    {
        mStage = BalakarStageIntermission;
        scriptEvents.resetEvents();
        warn("Balakar Khan calls on his Stormcasters to finish the ritual!");

        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_BALAKAR_STORMWINDS, true);

        mStormcasterGuids.clear();
        for (uint8_t i = 0; i < BalakarStormcasterCount; ++i)
        {
            if (Creature* pCaster = callAdd(NPC_NOKHUD_STORMCASTER, aroundSelf(12.0f, 18.0f)))
                mStormcasterGuids.push_back(pCaster->GetNewGUID());
        }

        scriptEvents.addEvent(EVENT_BALAKAR_LIGHTNING, 3000);
    }

    void leaveRitual()
    {
        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(false);
        _setMeleeDisabled(false);
        _removeAura(SPELL_BALAKAR_STORMWINDS);
    }

    void enterStorm()
    {
        mStage = BalakarStageStorm;
        scriptEvents.resetEvents();
        leaveRitual();
        warn("Balakar Khan rejoins the fight, infused with the storm!");

        scriptEvents.addEvent(EVENT_BALAKAR_CONDUCTIVE_STRIKE, 5000);
        scriptEvents.addEvent(EVENT_BALAKAR_LIGHTNING, 8000);
        scriptEvents.addEvent(EVENT_BALAKAR_STATIC_SPEAR, 10000);
        scriptEvents.addEvent(EVENT_BALAKAR_CRACKLING_UPHEAVAL, 16000);
    }

    bool stormcastersDown()
    {
        NokhudOffensiveInstanceScript* pNokhud = getNokhud();
        if (pNokhud == nullptr)
            return true;

        for (WoWGuid const& guid : mStormcasterGuids)
        {
            Creature* pCaster = pNokhud->getCreatureByGuid(guid);
            if (pCaster != nullptr && pCaster->isAlive())
                return false;
        }

        return true;
    }

    uint8_t mStage = 0;
    std::vector<WoWGuid> mStormcasterGuids;
};

class NokhudOffensiveStormcasterAI : public NokhudOffensiveCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NokhudOffensiveStormcasterAI(c); }
    explicit NokhudOffensiveStormcasterAI(Creature* pCreature) : NokhudOffensiveCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_STORMCASTER_STORM_BOLT, 2000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_STORMCASTER_STORM_BOLT)
            return;

        castAtPlayer(SPELL_STORMCASTER_STORM_BOLT);
        scriptEvents.addEvent(EVENT_STORMCASTER_STORM_BOLT, 4000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Teera and Maruuk, each one covers the other's big ability

class NokhudOffensiveTeeraAI : public NokhudOffensiveBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NokhudOffensiveTeeraAI(c); }
    explicit NokhudOffensiveTeeraAI(Creature* pCreature) : NokhudOffensiveBossAI(pCreature, DATA_TEERA_AND_MARUUK) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_TEERA_REPEL && _isInCombat())
            scriptEvents.addEvent(EVENT_TEERA_REPEL, 500);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_TEERA_QUICK_SHOT, 4000);
        scriptEvents.addEvent(EVENT_TEERA_SPIRIT_LEAP, 12000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!energyFull(diff, TeeraGaleArrowMs))
            return;

        warn("Teera looses a Gale Arrow!");
        castAtPlayer(SPELL_TEERA_GALE_ARROW);
        notify(BOSS_MARUUK, ACTION_MARUUK_FRIGHTFUL_ROAR);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TEERA_QUICK_SHOT:
                castAtPlayer(SPELL_TEERA_QUICK_SHOT);
                scriptEvents.addEvent(EVENT_TEERA_QUICK_SHOT, 10000);
                break;
            case EVENT_TEERA_SPIRIT_LEAP:
                castAtPlayer(SPELL_TEERA_SPIRIT_LEAP);
                scriptEvents.addEvent(EVENT_TEERA_SPIRIT_LEAP, 25000);
                break;
            case EVENT_TEERA_REPEL:
                guardMaruuk();
                break;
            default:
                break;
        }
    }

private:
    // pushes players away and wraps Maruuk in wind
    void guardMaruuk()
    {
        castSpellAOE(SPELL_TEERA_REPEL);

        NokhudOffensiveInstanceScript* pNokhud = getNokhud();
        Creature* pMaruuk = pNokhud != nullptr ? pNokhud->getMember(BOSS_MARUUK) : nullptr;
        if (pMaruuk != nullptr && pMaruuk->isAlive())
            castSpell(pMaruuk, SPELL_TEERA_GUARDIAN_WIND, true);
    }
};

class NokhudOffensiveMaruukAI : public NokhudOffensiveBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NokhudOffensiveMaruukAI(c); }
    explicit NokhudOffensiveMaruukAI(Creature* pCreature) : NokhudOffensiveBossAI(pCreature, DATA_TEERA_AND_MARUUK) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_MARUUK_FRIGHTFUL_ROAR && _isInCombat())
            scriptEvents.addEvent(EVENT_MARUUK_FRIGHTFUL_ROAR, 1000);
    }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_MARUUK_BRUTALIZE, 5000); }

    void onCombatTick(uint32_t diff) override
    {
        if (!energyFull(diff, MaruukEarthsplitterMs))
            return;

        warn("Maruuk brings down an Earthsplitter!");
        castSpellAOE(SPELL_MARUUK_EARTHSPLITTER);
        notify(BOSS_TEERA, ACTION_TEERA_REPEL);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MARUUK_BRUTALIZE:
                castSpellOnVictim(SPELL_MARUUK_BRUTALIZE);
                scriptEvents.addEvent(EVENT_MARUUK_BRUTALIZE, 12000);
                break;
            case EVENT_MARUUK_FRIGHTFUL_ROAR:
                castSpellAOE(SPELL_MARUUK_FRIGHTFUL_ROAR);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Raging Tempest

class NokhudOffensiveRagingTempestAI : public NokhudOffensiveBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NokhudOffensiveRagingTempestAI(c); }
    explicit NokhudOffensiveRagingTempestAI(Creature* pCreature) : NokhudOffensiveBossAI(pCreature, DATA_THE_RAGING_TEMPEST) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_TEMPEST_UNCONTROLLABLE_ENERGY, true);

        scriptEvents.addEvent(EVENT_TEMPEST_LIGHTNING_STRIKE, 8000);
        scriptEvents.addEvent(EVENT_TEMPEST_ENERGY_SURGE, 15000);
        scriptEvents.addEvent(EVENT_TEMPEST_WIND_BURST, 22000);
    }

    void onReset() override { _removeAura(SPELL_TEMPEST_UNCONTROLLABLE_ENERGY); }

    void onCombatTick(uint32_t diff) override
    {
        if (!energyFull(diff, TempestStormMs))
            return;

        warn("The Raging Tempest unleashes an Electrical Storm!");
        castSpellOnSelf(SPELL_TEMPEST_ELECTRICAL_STORM);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TEMPEST_LIGHTNING_STRIKE:
                castAtPlayer(SPELL_TEMPEST_LIGHTNING_STRIKE);
                scriptEvents.addEvent(EVENT_TEMPEST_LIGHTNING_STRIKE, 20000);
                break;
            case EVENT_TEMPEST_ENERGY_SURGE:
                castSpellOnSelf(SPELL_TEMPEST_ENERGY_SURGE);
                scriptEvents.addEvent(EVENT_TEMPEST_ENERGY_SURGE, 30000);
                break;
            case EVENT_TEMPEST_WIND_BURST:
                castSpellAOE(SPELL_TEMPEST_WIND_BURST);
                scriptEvents.addEvent(EVENT_TEMPEST_WIND_BURST, 32000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Granyth

class NokhudOffensiveGranythAI : public NokhudOffensiveBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NokhudOffensiveGranythAI(c); }
    explicit NokhudOffensiveGranythAI(Creature* pCreature) : NokhudOffensiveBossAI(pCreature, DATA_GRANYTH) {}

protected:
    void onEngage() override
    {
        mErupting = false;
        mEruptionMs = 0;

        scriptEvents.addEvent(EVENT_GRANYTH_SHARDS_OF_STONE, 10000);
        scriptEvents.addEvent(EVENT_GRANYTH_TECTONIC_STOMP, 15000);
        scriptEvents.addEvent(EVENT_GRANYTH_SABOTEUR, 25000);
    }

    void onReset() override
    {
        if (mErupting)
            endEruption();
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mErupting)
        {
            mEruptionMs += diff;
            if (mEruptionMs >= GranythEruptionLimitMs)
                endEruption();
            return;
        }

        if (!energyFull(diff, GranythEruptionMs))
            return;

        warn("Granyth begins a massive Eruption!");
        mErupting = true;
        mEruptionMs = 0;
        castSpellOnSelf(byDifficulty(SPELL_GRANYTH_ERUPTION, SPELL_GRANYTH_ERUPTION_NORMAL));
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GRANYTH_SHARDS_OF_STONE:
                castSpellAOE(SPELL_GRANYTH_SHARDS_OF_STONE);
                scriptEvents.addEvent(EVENT_GRANYTH_SHARDS_OF_STONE, 20000);
                break;
            case EVENT_GRANYTH_TECTONIC_STOMP:
                castSpellAOE(SPELL_GRANYTH_TECTONIC_STOMP);
                scriptEvents.addEvent(EVENT_GRANYTH_TECTONIC_STOMP, 24000);
                break;
            case EVENT_GRANYTH_SABOTEUR:
                sendSaboteur();
                scriptEvents.addEvent(EVENT_GRANYTH_SABOTEUR, 30000);
                break;
            default:
                break;
        }
    }

private:
    void endEruption()
    {
        mErupting = false;
        getCreature()->interruptSpell();
        _removeAura(SPELL_GRANYTH_ERUPTION);
        _removeAura(SPELL_GRANYTH_ERUPTION_NORMAL);
    }

    // saboteurs only come while there is a lance to wreck
    void sendSaboteur()
    {
        Creature* pLance = findNearestCreature(NPC_DRAGONKILLER_LANCE, SaboteurLanceSearch);
        if (pLance == nullptr || !pLance->isAlive())
            return;

        summonCreature(NPC_NOKHUD_SABOTEUR, aroundSelf(20.0f, 25.0f), CORPSE_DESPAWN);
    }

    bool mErupting = false;
    uint32_t mEruptionMs = 0;
};

// runs past the players to dismantle a lance, fights normally afterwards
class NokhudOffensiveSaboteurAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NokhudOffensiveSaboteurAI(c); }
    explicit NokhudOffensiveSaboteurAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        mOnTask = true;
        mTaskClockMs = 500;
        setReactState(REACT_PASSIVE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mOnTask || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mTaskClockMs > diff)
        {
            mTaskClockMs -= diff;
            return;
        }

        mTaskClockMs = 1000;

        Creature* pLance = findNearestCreature(NPC_DRAGONKILLER_LANCE, SaboteurLanceSearch);
        if (pLance == nullptr || !pLance->isAlive())
        {
            finishTask();
            return;
        }

        if (getCreature()->getDistance(pLance) > SaboteurWorkRange)
        {
            moveTo(pLance->GetPositionX(), pLance->GetPositionY(), pLance->GetPositionZ());
            return;
        }

        castSpell(pLance, SPELL_SABOTEUR_DISMANTLE);
        finishTask();
    }

private:
    void finishTask()
    {
        mOnTask = false;
        setReactState(REACT_AGGRESSIVE);
        setZoneWideCombat();
    }

    bool mOnTask = false;
    uint32_t mTaskClockMs = 0;
};

void SetupTheNokhudOffensive(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_NOKHUD_OFFENSIVE, &NokhudOffensiveInstanceScript::Create);

    mgr->register_creature_script(BOSS_BALAKAR_KHAN, &NokhudOffensiveBalakarKhanAI::Create);
    mgr->register_creature_script(NPC_NOKHUD_STORMCASTER, &NokhudOffensiveStormcasterAI::Create);

    mgr->register_creature_script(BOSS_TEERA, &NokhudOffensiveTeeraAI::Create);
    mgr->register_creature_script(BOSS_MARUUK, &NokhudOffensiveMaruukAI::Create);

    mgr->register_creature_script(BOSS_THE_RAGING_TEMPEST, &NokhudOffensiveRagingTempestAI::Create);

    mgr->register_creature_script(BOSS_GRANYTH, &NokhudOffensiveGranythAI::Create);
    mgr->register_creature_script(NPC_NOKHUD_SABOTEUR, &NokhudOffensiveSaboteurAI::Create);
    mgr->register_creature_script(NPC_NOKHUD_SABOTEUR_SECOND, &NokhudOffensiveSaboteurAI::Create);
    mgr->register_creature_script(NPC_NOKHUD_SABOTEUR_THIRD, &NokhudOffensiveSaboteurAI::Create);
}
