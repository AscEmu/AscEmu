/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Priory_Of_The_Sacred_Flame.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <map>
#include <vector>

enum PrioryOfTheSacredFlameEvents
{
    EVENT_BRAUNPYKE_CASTIGATORS_SHIELD = 1,
    EVENT_BRAUNPYKE_DETONATION,
    EVENT_BRAUNPYKE_BURNING_LIGHT,
    EVENT_BRAUNPYKE_HAMMER_OF_PURITY,

    EVENT_DAILCRY_PIERCE_ARMOR,
    EVENT_DAILCRY_SAVAGE_MAULING,
    EVENT_DAILCRY_EARTHSHATTERING_SPEAR,
    EVENT_DAILCRY_BATTLE_CRY,
    EVENT_ELAENA_DIVINE_JUDGMENT,
    EVENT_ELAENA_HOLY_RADIANCE,
    EVENT_SHAYNEMAIL_LUNGING_STRIKE,
    EVENT_SHAYNEMAIL_BRUTAL_SMASH,
    EVENT_TAENER_FIREBALL,
    EVENT_TAENER_EMBER_STORM,

    EVENT_MURRPRAY_HOLY_SMITE,
    EVENT_MURRPRAY_HOLY_FLAME,
    EVENT_MURRPRAY_SACRED_FLAME,
    EVENT_MURRPRAY_INNER_FIRE,
    EVENT_MURRPRAY_RAISE_BARRIER,
    EVENT_MURRPRAY_EMBRACE_THE_LIGHT,
    EVENT_MURRPRAY_BARRIER_CHECK
};

namespace
{
    // members of the encounter fought as one
    struct PrioryGroupSlot
    {
        uint32_t dataId;
        uint32_t entry;
    };

    PrioryGroupSlot const PrioryGroupSlots[] =
    {
        { DATA_CAPTAIN_DAILCRY, BOSS_CAPTAIN_DAILCRY },
        { DATA_CAPTAIN_DAILCRY, BOSS_ELAENA_EMBERLANZ },
        { DATA_CAPTAIN_DAILCRY, BOSS_SERGEANT_SHAYNEMAIL },
        { DATA_CAPTAIN_DAILCRY, BOSS_TAENER_DUELMAL },
        { DATA_CAPTAIN_DAILCRY, BOSS_ELAENA_EMBERLANZ_ALT },
        { DATA_CAPTAIN_DAILCRY, BOSS_SERGEANT_SHAYNEMAIL_ALT },
        { DATA_CAPTAIN_DAILCRY, BOSS_TAENER_DUELMAL_ALT }
    };

    float const PrioryTwoPi = 6.28318531f;

    uint32_t const BraunpykeEnergyPerSecond = 2;

    uint32_t const MurrprayBarrierMinMs = 8000;
    uint32_t const MurrprayBarrierMaxMs = 40000;
    float const MurrprayFlameSearch = 80.0f;
    float const MurrprayFlameStandOff = 6.0f;

    LocationVector prioryRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(PrioryTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // a point next to target on the side of origin
    LocationVector prioryPointBeside(LocationVector const& target, LocationVector const& origin, float distance)
    {
        float const dx = origin.x - target.x;
        float const dy = origin.y - target.y;
        float const length = std::sqrt(dx * dx + dy * dy);
        if (length < 0.1f)
            return target;

        return LocationVector(target.x + dx / length * distance, target.y + dy / length * distance, target.z, std::atan2(-dy, -dx));
    }

    uint32_t prioryPercentOf(uint32_t value, uint32_t pct)
    {
        return static_cast<uint32_t>(static_cast<uint64_t>(value) * pct / 100);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class PrioryOfTheSacredFlameInstanceScript : public InstanceScript
{
public:
    explicit PrioryOfTheSacredFlameInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(PrioryOfTheSacredFlameEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new PrioryOfTheSacredFlameInstanceScript(pMapMgr); }

    // a fallen guard must not end the encounter while others stand
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (PrioryGroupSlot const& slot : PrioryGroupSlots)
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

    std::vector<Creature*> getGroup(uint32_t dataId)
    {
        std::vector<Creature*> group;
        for (PrioryGroupSlot const& slot : PrioryGroupSlots)
        {
            if (slot.dataId != dataId)
                continue;

            auto const itr = mMemberGuids.find(slot.entry);
            if (itr == mMemberGuids.end())
                continue;

            if (Creature* pMember = getCreatureByGuid(itr->second))
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

private:
    std::map<uint32_t, WoWGuid> mMemberGuids;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy and target helpers

class PrioryCombatAI : public CreatureAIScript
{
public:
    explicit PrioryCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    PrioryOfTheSacredFlameInstanceScript* getPriory()
    {
        if (getCreature()->GetMapId() != MAP_PRIORY_OF_THE_SACRED_FLAME)
            return nullptr;

        return static_cast<PrioryOfTheSacredFlameInstanceScript*>(getInstanceScript());
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
// Encounter base: frame, boss state and group pull

class PrioryBossAI : public PrioryCombatAI
{
public:
    PrioryBossAI(Creature* pCreature, uint32_t dataId) : PrioryCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (PrioryOfTheSacredFlameInstanceScript* pPriory = getPriory())
        {
            pPriory->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pPriory->setBossState(mDataId, EncounterStates::InProgress);
            pPriory->rallyGroup(mDataId, getCreature());
        }

        PrioryCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (PrioryOfTheSacredFlameInstanceScript* pPriory = getPriory())
            {
                pPriory->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                if (!pPriory->isGroupFighting(mDataId, getCreature()))
                    pPriory->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        PrioryCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (PrioryOfTheSacredFlameInstanceScript* pPriory = getPriory())
        {
            pPriory->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            if (pPriory->isGroupDown(mDataId, getCreature()))
                pPriory->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Baron Braunpyke

class PrioryBaronBraunpykeAI : public PrioryBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PrioryBaronBraunpykeAI(c); }
    explicit PrioryBaronBraunpykeAI(Creature* pCreature) : PrioryBossAI(pCreature, DATA_BARON_BRAUNPYKE) {}

protected:
    void onEngage() override
    {
        mDetonationSpots.clear();

        scriptEvents.addEvent(EVENT_BRAUNPYKE_CASTIGATORS_SHIELD, 8000);
        scriptEvents.addEvent(EVENT_BRAUNPYKE_BURNING_LIGHT, 14000);
        scriptEvents.addEvent(EVENT_BRAUNPYKE_HAMMER_OF_PURITY, 20000);
    }

    void onReset() override
    {
        mDetonationSpots.clear();
        _removeAura(SPELL_BRAUNPYKE_VINDICTIVE_WRATH);
    }

    // full Holy energy empowers him
    void onCombatTick(uint32_t diff) override
    {
        if (!fillEnergy(diff, BraunpykeEnergyPerSecond) || _isCasting())
            return;

        emptyEnergy();
        warn("Baron Braunpyke is filled with Vindictive Wrath!");
        castSpellOnSelf(SPELL_BRAUNPYKE_VINDICTIVE_WRATH, true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BRAUNPYKE_CASTIGATORS_SHIELD:
                if (Unit* pTarget = anyPlayer())
                {
                    castSpell(pTarget, SPELL_BRAUNPYKE_CASTIGATORS_SHIELD);
                    mDetonationSpots.push_back(pTarget->GetPosition());
                    scriptEvents.addEvent(EVENT_BRAUNPYKE_DETONATION, 3000);
                }
                scriptEvents.addEvent(EVENT_BRAUNPYKE_CASTIGATORS_SHIELD, 16000);
                break;
            case EVENT_BRAUNPYKE_DETONATION:
                // the shield leaves a detonation where it struck
                for (LocationVector const& spot : mDetonationSpots)
                    getCreature()->castSpellLoc(spot, SPELL_BRAUNPYKE_CASTIGATORS_DETONATION, true);
                mDetonationSpots.clear();
                break;
            case EVENT_BRAUNPYKE_BURNING_LIGHT:
                warn("Baron Braunpyke begins to cast Burning Light!");
                castSpellAOE(SPELL_BRAUNPYKE_BURNING_LIGHT);
                scriptEvents.addEvent(EVENT_BRAUNPYKE_BURNING_LIGHT, 25000);
                break;
            case EVENT_BRAUNPYKE_HAMMER_OF_PURITY:
                castAt(anyPlayer(), SPELL_BRAUNPYKE_HAMMER_OF_PURITY);
                scriptEvents.addEvent(EVENT_BRAUNPYKE_HAMMER_OF_PURITY, 22000);
                break;
            default:
                break;
        }
    }

private:
    std::vector<LocationVector> mDetonationSpots;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Captain Dailcry and the Trusted Guard

class PrioryCaptainDailcryAI : public PrioryBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PrioryCaptainDailcryAI(c); }
    explicit PrioryCaptainDailcryAI(Creature* pCreature) : PrioryBossAI(pCreature, DATA_CAPTAIN_DAILCRY) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_DAILCRY_PIERCE_ARMOR, 6000);
        scriptEvents.addEvent(EVENT_DAILCRY_SAVAGE_MAULING, 11000);
        scriptEvents.addEvent(EVENT_DAILCRY_EARTHSHATTERING_SPEAR, 16000);
        scriptEvents.addEvent(EVENT_DAILCRY_BATTLE_CRY, 22000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DAILCRY_PIERCE_ARMOR:
                castSpellOnVictim(SPELL_DAILCRY_PIERCE_ARMOR);
                scriptEvents.addEvent(EVENT_DAILCRY_PIERCE_ARMOR, 12000);
                break;
            case EVENT_DAILCRY_SAVAGE_MAULING:
                castAt(offTankPlayer(), SPELL_DAILCRY_SAVAGE_MAULING);
                scriptEvents.addEvent(EVENT_DAILCRY_SAVAGE_MAULING, 25000);
                break;
            case EVENT_DAILCRY_EARTHSHATTERING_SPEAR:
                castSpellAOE(SPELL_DAILCRY_EARTHSHATTERING_SPEAR);
                scriptEvents.addEvent(EVENT_DAILCRY_EARTHSHATTERING_SPEAR, 18000);
                break;
            case EVENT_DAILCRY_BATTLE_CRY:
                warn("Captain Dailcry rallies his allies with a Battle Cry!");
                castSpellAOE(SPELL_DAILCRY_BATTLE_CRY);
                scriptEvents.addEvent(EVENT_DAILCRY_BATTLE_CRY, 30000);
                break;
            default:
                break;
        }
    }
};

// shared by the three guards: stronger while close together
class PrioryTrustedGuardAI : public PrioryBossAI
{
public:
    explicit PrioryTrustedGuardAI(Creature* pCreature) : PrioryBossAI(pCreature, DATA_CAPTAIN_DAILCRY) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_GUARD_STRENGTH_IN_NUMBERS, true);
        onGuardEngage();
    }

    void onReset() override { _removeAura(SPELL_GUARD_STRENGTH_IN_NUMBERS); }

    virtual void onGuardEngage() {}
};

class PrioryElaenaEmberlanzAI : public PrioryTrustedGuardAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PrioryElaenaEmberlanzAI(c); }
    explicit PrioryElaenaEmberlanzAI(Creature* pCreature) : PrioryTrustedGuardAI(pCreature) {}

protected:
    void onGuardEngage() override
    {
        scriptEvents.addEvent(EVENT_ELAENA_DIVINE_JUDGMENT, 7000);
        scriptEvents.addEvent(EVENT_ELAENA_HOLY_RADIANCE, 15000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ELAENA_DIVINE_JUDGMENT:
                castSpellOnVictim(SPELL_ELAENA_DIVINE_JUDGMENT);
                scriptEvents.addEvent(EVENT_ELAENA_DIVINE_JUDGMENT, 12000);
                break;
            case EVENT_ELAENA_HOLY_RADIANCE:
                castSpellOnSelf(SPELL_ELAENA_HOLY_RADIANCE);
                scriptEvents.addEvent(EVENT_ELAENA_HOLY_RADIANCE, 25000);
                break;
            default:
                break;
        }
    }
};

class PriorySergeantShaynemailAI : public PrioryTrustedGuardAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PriorySergeantShaynemailAI(c); }
    explicit PriorySergeantShaynemailAI(Creature* pCreature) : PrioryTrustedGuardAI(pCreature) {}

protected:
    void onGuardEngage() override
    {
        scriptEvents.addEvent(EVENT_SHAYNEMAIL_LUNGING_STRIKE, 9000);
        scriptEvents.addEvent(EVENT_SHAYNEMAIL_BRUTAL_SMASH, 14000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SHAYNEMAIL_LUNGING_STRIKE:
                castAt(offTankPlayer(), SPELL_SHAYNEMAIL_LUNGING_STRIKE);
                scriptEvents.addEvent(EVENT_SHAYNEMAIL_LUNGING_STRIKE, 14000);
                break;
            case EVENT_SHAYNEMAIL_BRUTAL_SMASH:
                castSpellAOE(SPELL_SHAYNEMAIL_BRUTAL_SMASH);
                scriptEvents.addEvent(EVENT_SHAYNEMAIL_BRUTAL_SMASH, 20000);
                break;
            default:
                break;
        }
    }
};

class PrioryTaenerDuelmalAI : public PrioryTrustedGuardAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PrioryTaenerDuelmalAI(c); }
    explicit PrioryTaenerDuelmalAI(Creature* pCreature) : PrioryTrustedGuardAI(pCreature) {}

protected:
    void onGuardEngage() override
    {
        scriptEvents.addEvent(EVENT_TAENER_FIREBALL, 2000);
        scriptEvents.addEvent(EVENT_TAENER_EMBER_STORM, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TAENER_FIREBALL:
                castAt(anyPlayer(), SPELL_TAENER_FIREBALL);
                scriptEvents.addEvent(EVENT_TAENER_FIREBALL, 4500);
                break;
            case EVENT_TAENER_EMBER_STORM:
                castSpellOnSelf(SPELL_TAENER_EMBER_STORM);
                scriptEvents.addEvent(EVENT_TAENER_EMBER_STORM, 25000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Prioress Murrpray, retreats behind a Barrier of Light at half health

class PrioryPrioressMurrprayAI : public PrioryBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PrioryPrioressMurrprayAI(c); }
    explicit PrioryPrioressMurrprayAI(Creature* pCreature) : PrioryBossAI(pCreature, DATA_PRIORESS_MURRPRAY) {}

    // the second spawn at the Baron only looks on, the fight is in the sanctum
    void OnLoad() override
    {
        LocationVector const baronRoom(2965.5f, 1502.2f, 574.7f);
        if (getCreature()->GetPosition().distanceSquare(baronRoom) > 30.0f * 30.0f)
            return;

        setCanEnterCombat(false);
        setReactState(REACT_PASSIVE);
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mBarrierUsed || !_isInCombat())
            return;

        uint32_t const health = getCreature()->getHealth();
        uint32_t const left = health > *damage ? health - *damage : 0;
        if (left <= prioryPercentOf(getCreature()->getMaxHealth(), 50))
        {
            mBarrierUsed = true;
            mBarrierPending = true;
        }
    }

protected:
    void onEngage() override
    {
        mBarrierUsed = false;
        mBarrierPending = false;
        mBehindBarrier = false;
        mBarrierMs = 0;
        scheduleFirstStage();
    }

    void onReset() override
    {
        mBarrierPending = false;
        mBehindBarrier = false;
        setRooted(false);
        _removeAura(SPELL_MURRPRAY_BARRIER_OF_LIGHT);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mBehindBarrier)
            mBarrierMs += diff;

        if (!mBarrierPending || _isCasting())
            return;

        mBarrierPending = false;
        retreatToFlame();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MURRPRAY_HOLY_SMITE:
                castAt(anyPlayer(), SPELL_MURRPRAY_HOLY_SMITE);
                scriptEvents.addEvent(EVENT_MURRPRAY_HOLY_SMITE, 7000);
                break;
            case EVENT_MURRPRAY_HOLY_FLAME:
                castAt(anyPlayer(), SPELL_MURRPRAY_HOLY_FLAME);
                scriptEvents.addEvent(EVENT_MURRPRAY_HOLY_FLAME, 16000);
                break;
            case EVENT_MURRPRAY_SACRED_FLAME:
                warn("Prioress Murrpray focuses The Sacred Flame!");
                castAt(offTankPlayer(), SPELL_MURRPRAY_THE_SACRED_FLAME);
                scriptEvents.addEvent(EVENT_MURRPRAY_SACRED_FLAME, 26000);
                break;
            case EVENT_MURRPRAY_INNER_FIRE:
                castSpellOnSelf(SPELL_MURRPRAY_INNER_FIRE);
                scriptEvents.addEvent(EVENT_MURRPRAY_INNER_FIRE, 30000);
                break;
            case EVENT_MURRPRAY_RAISE_BARRIER:
                raiseBarrier();
                break;
            case EVENT_MURRPRAY_EMBRACE_THE_LIGHT:
                if (!mBehindBarrier)
                    break;
                castSpellAOE(SPELL_MURRPRAY_EMBRACE_THE_LIGHT, true);
                scriptEvents.addEvent(EVENT_MURRPRAY_EMBRACE_THE_LIGHT, 3000);
                break;
            case EVENT_MURRPRAY_BARRIER_CHECK:
                if (!mBehindBarrier)
                    break;
                if ((mBarrierMs >= MurrprayBarrierMinMs && !hasAura(SPELL_MURRPRAY_BARRIER_OF_LIGHT)) || mBarrierMs >= MurrprayBarrierMaxMs)
                    dropBarrier();
                else
                    scriptEvents.addEvent(EVENT_MURRPRAY_BARRIER_CHECK, 1000);
                break;
            default:
                break;
        }
    }

private:
    void scheduleFirstStage()
    {
        scriptEvents.addEvent(EVENT_MURRPRAY_HOLY_SMITE, 3000);
        scriptEvents.addEvent(EVENT_MURRPRAY_HOLY_FLAME, 9000);
        scriptEvents.addEvent(EVENT_MURRPRAY_SACRED_FLAME, 15000);
        scriptEvents.addEvent(EVENT_MURRPRAY_INNER_FIRE, 20000);
    }

    void retreatToFlame()
    {
        scriptEvents.resetEvents();
        warn("Prioress Murrpray retreats to the Sacred Flame and shrouds herself in a Barrier of Light!");

        Creature* pFlame = findNearestCreature(NPC_ETERNAL_FLAME, MurrprayFlameSearch);
        if (pFlame != nullptr)
            moveJump(prioryPointBeside(pFlame->GetPosition(), getCreature()->GetPosition(), MurrprayFlameStandOff), 20.0f, 8.0f);

        scriptEvents.addEvent(EVENT_MURRPRAY_RAISE_BARRIER, 2000);
    }

    void raiseBarrier()
    {
        mBehindBarrier = true;
        mBarrierMs = 0;
        setRooted(true);
        castSpellOnSelf(SPELL_MURRPRAY_BARRIER_OF_LIGHT, true);

        // her devoted come to the flame
        uint32_t const count = isHeroic() ? 4 : 3;
        for (uint32_t i = 0; i < count; ++i)
        {
            Creature* pNeophyte = callAdd(NPC_ARATHI_NEOPHYTE, prioryRingPoint(getCreature()->GetPosition(), 8.0f, 14.0f));
            if (pNeophyte != nullptr)
                pNeophyte->castSpell(pNeophyte, SPELL_NEOPHYTE_OVERWHELMING_POWER, true);
        }

        scriptEvents.addEvent(EVENT_MURRPRAY_EMBRACE_THE_LIGHT, 3000);
        scriptEvents.addEvent(EVENT_MURRPRAY_BARRIER_CHECK, 1000);
    }

    void dropBarrier()
    {
        mBehindBarrier = false;
        _removeAura(SPELL_MURRPRAY_BARRIER_OF_LIGHT);
        setRooted(false);
        warn("The Barrier of Light around Prioress Murrpray fades!");

        scriptEvents.removeEvent(EVENT_MURRPRAY_EMBRACE_THE_LIGHT);
        scheduleFirstStage();
    }

    bool mBarrierUsed = false;
    bool mBarrierPending = false;
    bool mBehindBarrier = false;
    uint32_t mBarrierMs = 0;
};

void SetupPrioryOfTheSacredFlame(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_PRIORY_OF_THE_SACRED_FLAME, &PrioryOfTheSacredFlameInstanceScript::Create);

    mgr->register_creature_script(BOSS_BARON_BRAUNPYKE, &PrioryBaronBraunpykeAI::Create);

    mgr->register_creature_script(BOSS_CAPTAIN_DAILCRY, &PrioryCaptainDailcryAI::Create);
    mgr->register_creature_script(BOSS_ELAENA_EMBERLANZ, &PrioryElaenaEmberlanzAI::Create);
    mgr->register_creature_script(BOSS_ELAENA_EMBERLANZ_ALT, &PrioryElaenaEmberlanzAI::Create);
    mgr->register_creature_script(BOSS_SERGEANT_SHAYNEMAIL, &PriorySergeantShaynemailAI::Create);
    mgr->register_creature_script(BOSS_SERGEANT_SHAYNEMAIL_ALT, &PriorySergeantShaynemailAI::Create);
    mgr->register_creature_script(BOSS_TAENER_DUELMAL, &PrioryTaenerDuelmalAI::Create);
    mgr->register_creature_script(BOSS_TAENER_DUELMAL_ALT, &PrioryTaenerDuelmalAI::Create);

    mgr->register_creature_script(BOSS_PRIORESS_MURRPRAY, &PrioryPrioressMurrprayAI::Create);
}
