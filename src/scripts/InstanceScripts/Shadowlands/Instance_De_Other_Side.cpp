/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_De_Other_Side.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <map>
#include <vector>

enum DeOtherSideEvents
{
    EVENT_MILLHOUSE_FROSTBOLT = 1,
    EVENT_MILLHOUSE_POWER_OVERWHELMING,
    EVENT_MILLHOUSE_SHADOWFURY,

    EVENT_MILLIFICENT_THROW_BUZZ_SAW,
    EVENT_MILLIFICENT_FINGER_LASER,
    EVENT_MILLIFICENT_ERUPTION,
    EVENT_MILLIFICENT_BUZZ_SAW,
    EVENT_MILLIFICENT_SQUIRREL_BOMB,
    EVENT_MILLIFICENT_ROCKET_CHICKEN,

    EVENT_HAKKAR_PIERCING_BARB,
    EVENT_HAKKAR_CORRUPTED_BLOOD,
    EVENT_HAKKAR_SONS,
    EVENT_HAKKAR_BLOOD_BARRIER,
    EVENT_HAKKAR_BLOOD_BARRAGE,
    EVENT_SON_BLOOD_NOVA,

    EVENT_MUEHZALA_SOULCRUSHER,
    EVENT_MUEHZALA_COSMIC_ARTIFICE,
    EVENT_MUEHZALA_STELLAR_CLOUD,
    EVENT_MUEHZALA_CRUSH,
    EVENT_MUEHZALA_MASTER_OF_DEATH,
    EVENT_MUEHZALA_VISAGE_CHECK,
    EVENT_VISAGE_SHATTERED_DOMINION,
    EVENT_VISAGE_COSMIC_ARTIFICE,

    EVENT_XYEXA_ARCANE_LIGHTNING,
    EVENT_XYEXA_LOCALIZED_CONTRIVANCE,
    EVENT_XYEXA_DISPLACEMENT_TRAP,
    EVENT_XYEXA_EXPLOSIVE_CONTRIVANCE
};

namespace
{
    // members of the encounters fought as one
    struct DeOtherSideGroupSlot
    {
        uint32_t dataId;
        uint32_t entry;
    };

    DeOtherSideGroupSlot const DeOtherSideGroupSlots[] =
    {
        { DATA_THE_MANASTORMS, BOSS_MILLHOUSE_MANASTORM },
        { DATA_THE_MANASTORMS, BOSS_MILLIFICENT_MANASTORM }
    };

    float const DeOtherSideTwoPi = 6.2831853f;

    // the floor fighter swaps at this health
    int32_t const ManastormHandOffPct = 50;

    // Hakkar
    uint32_t const SonsPerCall = 2;
    uint32_t const BloodBarrierMs = 15000;

    // Mueh'zala shatters at these health marks
    int32_t const MuehzalaShatterMarks[] = { 70, 40 };
    uint32_t const MuehzalaShatterMarkCount = 2;
    uint32_t const VisagesPerShatter = 3;
    uint32_t const VisageCheckMs = 1000;
    uint32_t const MuehzalaShatterLimitChecks = 45;

    uint32_t const VisageEntries[] = { NPC_SHATTERED_VISAGE, NPC_SHATTERED_VISAGE_ALT };

    LocationVector deOtherSidePointAround(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(DeOtherSideTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class DeOtherSideInstanceScript : public InstanceScript
{
public:
    explicit DeOtherSideInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(DeOtherSideEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new DeOtherSideInstanceScript(pMapMgr); }

    // one fallen Manastorm must not end the encounter while the other stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (DeOtherSideGroupSlot const& slot : DeOtherSideGroupSlots)
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

        // a wiped pair fights again in full
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
        for (DeOtherSideGroupSlot const& slot : DeOtherSideGroupSlots)
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

    bool isMemberAlive(uint32_t entry)
    {
        Creature* pMember = getMember(entry);
        return pMember != nullptr && pMember->isAlive();
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
// Shared combat base: event clock while in combat

class DeOtherSideCombatAI : public CreatureAIScript
{
public:
    explicit DeOtherSideCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    DeOtherSideInstanceScript* getDeOtherSide()
    {
        if (getCreature()->GetMapId() != MAP_DE_OTHER_SIDE)
            return nullptr;

        return static_cast<DeOtherSideInstanceScript*>(getInstanceScript());
    }

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

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

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // out of reach: no target, no movement, no swings
    void setOutOfReach(bool apart)
    {
        if (apart)
            getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        setRooted(apart);
        _setMeleeDisabled(apart);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class DeOtherSideBossAI : public DeOtherSideCombatAI
{
public:
    DeOtherSideBossAI(Creature* pCreature, uint32_t dataId) : DeOtherSideCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (DeOtherSideInstanceScript* pInstance = getDeOtherSide())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
            pInstance->rallyGroup(mDataId, getCreature());
        }

        DeOtherSideCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (DeOtherSideInstanceScript* pInstance = getDeOtherSide())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                if (!pInstance->isGroupFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        DeOtherSideCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        // the instance holds the credit back while a partner stands
        if (DeOtherSideInstanceScript* pInstance = getDeOtherSide())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Manastorms, one fights on the workshop floor while the other assists from the railing

class DeOtherSideManastormAI : public DeOtherSideBossAI
{
public:
    DeOtherSideManastormAI(Creature* pCreature, bool startsOnFloor, uint32_t partnerEntry) : DeOtherSideBossAI(pCreature, DATA_THE_MANASTORMS), mStartsOnFloor(startsOnFloor), mPartnerEntry(partnerEntry) {}

    void OnLoad() override { setOnFloor(mStartsOnFloor); }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_MANASTORM_TAKE_FLOOR || mOnFloor || !isAlive() || !_isInCombat())
            return;

        setOnFloor(true);
        scriptEvents.resetEvents();
        scheduleKit();
    }

protected:
    virtual void scheduleFloorKit() = 0;
    virtual void scheduleRailingKit() = 0;
    virtual void handOff() = 0;

    void onEngage() override
    {
        mHandedOff = false;
        setOnFloor(mStartsOnFloor);
        scheduleKit();
    }

    void onReset() override
    {
        mHandedOff = false;
        setOnFloor(mStartsOnFloor);
    }

    // the survivor always comes down
    void onDefeat() override { callPartnerDown(); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mOnFloor || mHandedOff || _isCasting() || _getHealthPercent() > ManastormHandOffPct)
            return;

        mHandedOff = true;
        handOff();
    }

    void scheduleKit()
    {
        if (mOnFloor)
            scheduleFloorKit();
        else
            scheduleRailingKit();
    }

    void setOnFloor(bool onFloor)
    {
        mOnFloor = onFloor;
        setOutOfReach(!onFloor);
    }

    void callPartnerDown()
    {
        if (DeOtherSideInstanceScript* pInstance = getDeOtherSide())
            pInstance->sendAction(mPartnerEntry, ACTION_MANASTORM_TAKE_FLOOR);
    }

    bool isPartnerAlive()
    {
        DeOtherSideInstanceScript* pInstance = getDeOtherSide();
        return pInstance != nullptr && pInstance->isMemberAlive(mPartnerEntry);
    }

    bool mStartsOnFloor;
    uint32_t mPartnerEntry;
    bool mOnFloor = false;
    bool mHandedOff = false;
};

class DeOtherSideMillhouseAI : public DeOtherSideManastormAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DeOtherSideMillhouseAI(c); }
    explicit DeOtherSideMillhouseAI(Creature* pCreature) : DeOtherSideManastormAI(pCreature, true, BOSS_MILLIFICENT_MANASTORM) {}

protected:
    void scheduleFloorKit() override
    {
        scriptEvents.addEvent(EVENT_MILLHOUSE_FROSTBOLT, 2000);
        scriptEvents.addEvent(EVENT_MILLHOUSE_POWER_OVERWHELMING, 15000);
    }

    void scheduleRailingKit() override
    {
        scriptEvents.addEvent(EVENT_MILLHOUSE_FROSTBOLT, 4000);
        scriptEvents.addEvent(EVENT_MILLHOUSE_SHADOWFURY, 8000);
    }

    // he blasts the room and leaves the floor to his wife
    void handOff() override
    {
        warn("Millhouse Manastorm unleashes Diabolical Dooooooom!");
        castSpellAOE(SPELL_MILLHOUSE_DIABOLICAL_DOOM);

        if (!isPartnerAlive())
            return;

        callPartnerDown();
        setOnFloor(false);
        scriptEvents.resetEvents();
        scheduleKit();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MILLHOUSE_FROSTBOLT:
                if (mOnFloor)
                    castSpellOnVictim(SPELL_MILLHOUSE_FROSTBOLT);
                else
                    castAt(randomPlayer(), SPELL_MILLHOUSE_FROSTBOLT);
                scriptEvents.addEvent(EVENT_MILLHOUSE_FROSTBOLT, mOnFloor ? 3500 : 6000);
                break;
            case EVENT_MILLHOUSE_POWER_OVERWHELMING:
                if (!mOnFloor)
                    break;
                warn("Millhouse Manastorm channels Power Overwhelming!");
                castSpellOnSelf(SPELL_MILLHOUSE_POWER_OVERWHELMING);
                scriptEvents.addEvent(EVENT_MILLHOUSE_POWER_OVERWHELMING, 30000);
                break;
            case EVENT_MILLHOUSE_SHADOWFURY:
                if (mOnFloor)
                    break;
                castAt(randomPlayer(), SPELL_MILLHOUSE_SHADOWFURY);
                scriptEvents.addEvent(EVENT_MILLHOUSE_SHADOWFURY, 18000);
                break;
            default:
                break;
        }
    }
};

class DeOtherSideMillificentAI : public DeOtherSideManastormAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DeOtherSideMillificentAI(c); }
    explicit DeOtherSideMillificentAI(Creature* pCreature) : DeOtherSideManastormAI(pCreature, false, BOSS_MILLHOUSE_MANASTORM) {}

protected:
    void scheduleFloorKit() override
    {
        scriptEvents.addEvent(EVENT_MILLIFICENT_BUZZ_SAW, 4000);
        scriptEvents.addEvent(EVENT_MILLIFICENT_SQUIRREL_BOMB, 12000);
        scriptEvents.addEvent(EVENT_MILLIFICENT_ROCKET_CHICKEN, 20000);
    }

    void scheduleRailingKit() override
    {
        scriptEvents.addEvent(EVENT_MILLIFICENT_THROW_BUZZ_SAW, 5000);
        scriptEvents.addEvent(EVENT_MILLIFICENT_FINGER_LASER, 10000);
    }

    // she calls him back down and both finish the fight on the floor
    void handOff() override
    {
        warn("Millificent Manastorm calls Millhouse back into the workshop!");
        castSpellAOE(SPELL_MILLIFICENT_ROCKET_CHICKEN_BARRAGE);
        callPartnerDown();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MILLIFICENT_THROW_BUZZ_SAW:
                if (mOnFloor)
                    break;
                castAt(randomPlayer(), SPELL_MILLIFICENT_THROW_BUZZ_SAW);
                scriptEvents.addEvent(EVENT_MILLIFICENT_THROW_BUZZ_SAW, 12000);
                break;
            case EVENT_MILLIFICENT_FINGER_LASER:
                if (mOnFloor)
                    break;
                castAt(randomPlayer(), SPELL_MILLIFICENT_ECHO_FINGER_LASER);
                scriptEvents.addEvent(EVENT_MILLIFICENT_ERUPTION, 3000);
                scriptEvents.addEvent(EVENT_MILLIFICENT_FINGER_LASER, 20000);
                break;
            case EVENT_MILLIFICENT_ERUPTION:
                castAt(randomPlayer(), SPELL_MILLIFICENT_ERUPTION, true);
                break;
            case EVENT_MILLIFICENT_BUZZ_SAW:
                if (!mOnFloor)
                    break;
                castSpellOnVictim(SPELL_MILLIFICENT_BUZZ_SAW);
                scriptEvents.addEvent(EVENT_MILLIFICENT_BUZZ_SAW, 9000);
                break;
            case EVENT_MILLIFICENT_SQUIRREL_BOMB:
                if (!mOnFloor)
                    break;
                warn("Millificent Manastorm deploys an Experimental Squirrel Bomb!");
                castSpellAOE(SPELL_MILLIFICENT_SQUIRREL_BOMB);
                scriptEvents.addEvent(EVENT_MILLIFICENT_SQUIRREL_BOMB, 25000);
                break;
            case EVENT_MILLIFICENT_ROCKET_CHICKEN:
                if (!mOnFloor)
                    break;
                castSpellAOE(SPELL_MILLIFICENT_ROCKET_CHICKEN_BARRAGE);
                scriptEvents.addEvent(EVENT_MILLIFICENT_ROCKET_CHICKEN, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hakkar the Soulflayer

class DeOtherSideHakkarAI : public DeOtherSideBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DeOtherSideHakkarAI(c); }
    explicit DeOtherSideHakkarAI(Creature* pCreature) : DeOtherSideBossAI(pCreature, DATA_HAKKAR_SOULFLAYER) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_HAKKAR_PIERCING_BARB, 7000);
        scriptEvents.addEvent(EVENT_HAKKAR_CORRUPTED_BLOOD, 12000);
        scriptEvents.addEvent(EVENT_HAKKAR_SONS, 20000);
        scriptEvents.addEvent(EVENT_HAKKAR_BLOOD_BARRIER, 30000);
    }

    void onReset() override { _removeAura(SPELL_HAKKAR_BLOOD_BARRIER); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HAKKAR_PIERCING_BARB:
                castSpellOnVictim(SPELL_HAKKAR_PIERCING_BARB);
                scriptEvents.addEvent(EVENT_HAKKAR_PIERCING_BARB, 12000);
                break;
            case EVENT_HAKKAR_CORRUPTED_BLOOD:
                castAt(randomPlayer(), SPELL_HAKKAR_CORRUPTED_BLOOD);
                scriptEvents.addEvent(EVENT_HAKKAR_CORRUPTED_BLOOD, 20000);
                break;
            case EVENT_HAKKAR_SONS:
                warn("Atal'ai priests sacrifice themselves and rise as Sons of Hakkar!");
                for (uint32_t i = 0; i < SonsPerCall; ++i)
                    callAdd(NPC_SON_OF_HAKKAR, deOtherSidePointAround(getCreature()->GetPosition(), 12.0f, 18.0f));
                scriptEvents.addEvent(EVENT_HAKKAR_SONS, 35000);
                break;
            case EVENT_HAKKAR_BLOOD_BARRIER:
                warn("Hakkar the Soulflayer shields himself with a Blood Barrier!");
                castSpellOnSelf(SPELL_HAKKAR_BLOOD_BARRIER);
                scriptEvents.addEvent(EVENT_HAKKAR_BLOOD_BARRAGE, BloodBarrierMs);
                scriptEvents.addEvent(EVENT_HAKKAR_BLOOD_BARRIER, 45000);
                break;
            case EVENT_HAKKAR_BLOOD_BARRAGE:
                // the barrier breaks into a barrage
                _removeAura(SPELL_HAKKAR_BLOOD_BARRIER);
                warn("Hakkar the Soulflayer lashes out with a Blood Barrage!");
                castSpellAOE(SPELL_HAKKAR_BLOOD_BARRAGE);
                break;
            default:
                break;
        }
    }
};

class DeOtherSideSonOfHakkarAI : public DeOtherSideCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DeOtherSideSonOfHakkarAI(c); }
    explicit DeOtherSideSonOfHakkarAI(Creature* pCreature) : DeOtherSideCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_SON_BLOOD_NOVA, 5000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SON_BLOOD_NOVA)
            return;

        castSpellAOE(SPELL_SON_OF_HAKKAR_BLOOD_NOVA);
        scriptEvents.addEvent(EVENT_SON_BLOOD_NOVA, 10000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mueh'zala

class DeOtherSideMuehzalaAI : public DeOtherSideBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DeOtherSideMuehzalaAI(c); }
    explicit DeOtherSideMuehzalaAI(Creature* pCreature) : DeOtherSideBossAI(pCreature, DATA_MUEHZALA) {}

protected:
    void onEngage() override
    {
        mShatterIndex = 0;
        mShattered = false;
        mShatterChecks = 0;
        scheduleAttacks();
    }

    void onReset() override
    {
        mShattered = false;
        setOutOfReach(false);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mShattered || mShatterIndex >= MuehzalaShatterMarkCount || _isCasting())
            return;

        if (_getHealthPercent() <= MuehzalaShatterMarks[mShatterIndex])
            shatter();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MUEHZALA_SOULCRUSHER:
                castSpellOnVictim(SPELL_MUEHZALA_SOULCRUSHER);
                scriptEvents.addEvent(EVENT_MUEHZALA_SOULCRUSHER, 14000);
                break;
            case EVENT_MUEHZALA_COSMIC_ARTIFICE:
                castSpellAOE(SPELL_MUEHZALA_COSMIC_ARTIFICE);
                scriptEvents.addEvent(EVENT_MUEHZALA_COSMIC_ARTIFICE, 22000);
                break;
            case EVENT_MUEHZALA_STELLAR_CLOUD:
                castAt(randomPlayer(), SPELL_MUEHZALA_STELLAR_CLOUD);
                scriptEvents.addEvent(EVENT_MUEHZALA_STELLAR_CLOUD, 20000);
                break;
            case EVENT_MUEHZALA_CRUSH:
                castSpellAOE(SPELL_MUEHZALA_CRUSH);
                scriptEvents.addEvent(EVENT_MUEHZALA_CRUSH, 18000);
                break;
            case EVENT_MUEHZALA_MASTER_OF_DEATH:
                warn("Mueh'zala channels the Master of Death!");
                castSpellAOE(SPELL_MUEHZALA_MASTER_OF_DEATH);
                scriptEvents.addEvent(EVENT_MUEHZALA_MASTER_OF_DEATH, 32000);
                break;
            case EVENT_MUEHZALA_VISAGE_CHECK:
                // he coalesces once the visages are gone or after a while
                if (++mShatterChecks >= MuehzalaShatterLimitChecks || !anyVisageAlive())
                    coalesce();
                else
                    scriptEvents.addEvent(EVENT_MUEHZALA_VISAGE_CHECK, VisageCheckMs);
                break;
            default:
                break;
        }
    }

private:
    void scheduleAttacks()
    {
        scriptEvents.addEvent(EVENT_MUEHZALA_SOULCRUSHER, 6000);
        scriptEvents.addEvent(EVENT_MUEHZALA_COSMIC_ARTIFICE, 12000);
        scriptEvents.addEvent(EVENT_MUEHZALA_STELLAR_CLOUD, 16000);
        scriptEvents.addEvent(EVENT_MUEHZALA_CRUSH, 20000);
        scriptEvents.addEvent(EVENT_MUEHZALA_MASTER_OF_DEATH, 28000);
    }

    // splits into Shattered Visages and stays out of reach meanwhile
    void shatter()
    {
        ++mShatterIndex;
        mShattered = true;
        mShatterChecks = 0;

        scriptEvents.resetEvents();
        warn("Mueh'zala shatters reality into Shattered Visages!");
        setOutOfReach(true);

        for (uint32_t i = 0; i < VisagesPerShatter; ++i)
            callAdd(NPC_SHATTERED_VISAGE, deOtherSidePointAround(getCreature()->GetPosition(), 15.0f, 20.0f));

        scriptEvents.addEvent(EVENT_MUEHZALA_VISAGE_CHECK, VisageCheckMs);
    }

    void coalesce()
    {
        mShattered = false;
        summons.despawnEntry(NPC_SHATTERED_VISAGE);

        warn("Mueh'zala coalesces back into his physical form!");
        setOutOfReach(false);
        scheduleAttacks();
    }

    bool anyVisageAlive()
    {
        for (uint32_t entry : VisageEntries)
        {
            std::list<Creature*> visages;
            GetCreatureListWithEntryInGrid(visages, entry, 150.0f);

            for (Creature* pVisage : visages)
            {
                if (pVisage->isAlive())
                    return true;
            }
        }

        return false;
    }

    uint32_t mShatterIndex = 0;
    bool mShattered = false;
    uint32_t mShatterChecks = 0;
};

class DeOtherSideShatteredVisageAI : public DeOtherSideCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DeOtherSideShatteredVisageAI(c); }
    explicit DeOtherSideShatteredVisageAI(Creature* pCreature) : DeOtherSideCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_VISAGE_SHATTERED_DOMINION, 4000);
        scriptEvents.addEvent(EVENT_VISAGE_COSMIC_ARTIFICE, 8000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VISAGE_SHATTERED_DOMINION:
                castSpellAOE(SPELL_VISAGE_SHATTERED_DOMINION);
                scriptEvents.addEvent(EVENT_VISAGE_SHATTERED_DOMINION, 9000);
                break;
            case EVENT_VISAGE_COSMIC_ARTIFICE:
                castSpellAOE(SPELL_MUEHZALA_COSMIC_ARTIFICE);
                scriptEvents.addEvent(EVENT_VISAGE_COSMIC_ARTIFICE, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Dealer Xy'exa

class DeOtherSideDealerXyexaAI : public DeOtherSideBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DeOtherSideDealerXyexaAI(c); }
    explicit DeOtherSideDealerXyexaAI(Creature* pCreature) : DeOtherSideBossAI(pCreature, DATA_DEALER_XYEXA) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_XYEXA_ARCANE_LIGHTNING, 6000);
        scriptEvents.addEvent(EVENT_XYEXA_LOCALIZED_CONTRIVANCE, 12000);
        scriptEvents.addEvent(EVENT_XYEXA_DISPLACEMENT_TRAP, 18000);
        scriptEvents.addEvent(EVENT_XYEXA_EXPLOSIVE_CONTRIVANCE, 35000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_XYEXA_ARCANE_LIGHTNING:
                castAt(randomPlayer(), SPELL_XYEXA_ARCANE_LIGHTNING);
                scriptEvents.addEvent(EVENT_XYEXA_ARCANE_LIGHTNING, 15000);
                break;
            case EVENT_XYEXA_LOCALIZED_CONTRIVANCE:
                castAt(randomPlayer(), SPELL_XYEXA_LOCALIZED_EXPLOSIVE_CONTRIVANCE);
                scriptEvents.addEvent(EVENT_XYEXA_LOCALIZED_CONTRIVANCE, 20000);
                break;
            case EVENT_XYEXA_DISPLACEMENT_TRAP:
                castSpellAOE(SPELL_XYEXA_DISPLACEMENT_TRAP);
                scriptEvents.addEvent(EVENT_XYEXA_DISPLACEMENT_TRAP, 28000);
                break;
            case EVENT_XYEXA_EXPLOSIVE_CONTRIVANCE:
                warn("Dealer Xy'exa primes an Explosive Contrivance!");
                castAt(randomPlayer(), SPELL_XYEXA_EXPLOSIVE_CONTRIVANCE);
                scriptEvents.addEvent(EVENT_XYEXA_EXPLOSIVE_CONTRIVANCE, 40000);
                break;
            default:
                break;
        }
    }
};

void SetupDeOtherSide(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_DE_OTHER_SIDE, &DeOtherSideInstanceScript::Create);

    mgr->register_creature_script(BOSS_MILLHOUSE_MANASTORM, &DeOtherSideMillhouseAI::Create);
    mgr->register_creature_script(BOSS_MILLIFICENT_MANASTORM, &DeOtherSideMillificentAI::Create);

    mgr->register_creature_script(BOSS_HAKKAR_THE_SOULFLAYER, &DeOtherSideHakkarAI::Create);
    mgr->register_creature_script(NPC_SON_OF_HAKKAR, &DeOtherSideSonOfHakkarAI::Create);

    mgr->register_creature_script(BOSS_MUEHZALA, &DeOtherSideMuehzalaAI::Create);
    mgr->register_creature_script(NPC_SHATTERED_VISAGE, &DeOtherSideShatteredVisageAI::Create);
    mgr->register_creature_script(NPC_SHATTERED_VISAGE_ALT, &DeOtherSideShatteredVisageAI::Create);

    mgr->register_creature_script(BOSS_DEALER_XYEXA, &DeOtherSideDealerXyexaAI::Create);
}
