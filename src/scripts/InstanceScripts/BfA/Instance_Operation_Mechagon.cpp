/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Operation_Mechagon.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <list>
#include <string>
#include <vector>

enum OperationMechagonEvents
{
    EVENT_PUMMELER_WHIRLING_EDGE = 1,
    EVENT_PUMMELER_LAY_MINE,
    EVENT_GNOMERCY_MAXIMUM_THRUST,
    EVENT_GNOMERCY_VENT_JETS,

    EVENT_KUJO_BLAZING_CHOMP,
    EVENT_KUJO_AIR_DROP,
    EVENT_KUJO_VENTING_FLAMES,
    EVENT_KUJO_EXPLOSIVE_LEAP,
    EVENT_KUJO_LEAP_JUMP,
    EVENT_KUJO_LEAP_LAND,

    EVENT_SPARKFLUX_DISCOM_BOMBULATOR,

    EVENT_AERIAL_PLASMA_ORBS,
    EVENT_AERIAL_PULSE_BLAST,
    EVENT_AERIAL_RECALIBRATE,
    EVENT_AERIAL_TAKE_OFF,
    EVENT_AERIAL_CUTTING_BEAM,

    EVENT_OMEGA_GIGA_ZAP,
    EVENT_OMEGA_PLASMA_ORBS,
    EVENT_OMEGA_RECALIBRATE,
    EVENT_OMEGA_MAGNETO_ARM,

    EVENT_GOBBAMAK_ELECTRICAL_CHARGE,
    EVENT_GOBBAMAK_RUMBLE,
    EVENT_GOBBAMAK_CALL_GRUNTERS,
    EVENT_GOBBAMAK_GRUNTERS_ARRIVE,

    EVENT_HK8_CANNON_BLAST,
    EVENT_HK8_CANNON_SHELLS,
    EVENT_HK8_CRASH_DOWN,
    EVENT_HK8_LIFT_OFF,
    EVENT_MK1_WRECK,
    EVENT_MK1_FULMINATING_ZAP,
    EVENT_MK1_REINFORCEMENTS,

    EVENT_GUNKER_SLUDGE_BOLT,
    EVENT_GUNKER_SPLATTER,
    EVENT_GUNKER_COALESCE,
    EVENT_GUNKER_TOXIC_FUMES,

    EVENT_TRIXIE_TAZE,
    EVENT_TRIXIE_ELECTRIC_SLIDE,
    EVENT_TRIXIE_SLIDE_END,
    EVENT_TRIXIE_MEGA_TAZE,
    EVENT_TRIXIE_MEGA_TAZE_JOLT,
    EVENT_NAENO_PEDAL_TO_THE_METAL,
    EVENT_NAENO_BURNOUT,
    EVENT_NAENO_DISMOUNT,
    EVENT_NAENO_MOUNT,
    EVENT_NAENO_BOLT_BUSTER,
    EVENT_NAENO_ROADKILL
};

namespace
{
    float const MechagonTwoPi = 6.2831853f;

    // members of the encounters fought by more than one creature
    struct MechagonCrewSlot
    {
        uint32_t dataId;
        uint32_t entry;
        bool joinsOnPull;
        bool neededForKill;
    };

    MechagonCrewSlot const MechagonCrew[] =
    {
        { DATA_TUSSLE_TONKS,  BOSS_PLATINUM_PUMMELER, true,  true },
        { DATA_TUSSLE_TONKS,  BOSS_GNOMERCY,          true,  true },
        { DATA_KING_MECHAGON, BOSS_AERIAL_UNIT,       true,  true },
        // takes over once the Aerial Unit is down
        { DATA_KING_MECHAGON, BOSS_OMEGA_BUSTER,      false, true },
        { DATA_HK8,           BOSS_HK8,               true,  true },
        // shuts down instead of dying
        { DATA_HK8,           NPC_TANK_BUSTER_MK1,    true,  false },
        { DATA_TRIXIE_NAENO,  BOSS_TRIXIE_TAZER,      true,  true },
        { DATA_TRIXIE_NAENO,  BOSS_NAENO_MEGACRASH,   true,  true }
    };

    LocationVector const GobbamakGrunterCave(1188.0f, -75.0f, 21.84f, 3.59f);
    uint32_t const GobbamakGruntersPerCall = 4;

    LocationVector const KingMechagonThrownOut(699.518f, 623.309f, -238.285f, 5.25f);
    uint32_t const PlasmaOrbsPerCall = 5;

    uint32_t const MK1ShutdownPct = 2;
    uint32_t const MK1WalkiesPerRelay = 3;
    uint32_t const WalkieShockieFuseMs = 10000;
    uint32_t const HK8AirborneMs = 30000;
    uint32_t const HK8HaywireMs = 31000;

    uint32_t const NaenoRideMs = 60000;

    // energy bar in tenths, the boss acts on a full bar
    class MechagonEnergyBar
    {
    public:
        void reset(uint32_t energy)
        {
            mTenths = std::min<uint32_t>(energy * 10, 1000);
            mClockMs = 0;
        }

        bool fill(uint32_t diff, uint32_t tenthsPerSecond)
        {
            mClockMs += diff;
            while (mClockMs >= 1000)
            {
                mClockMs -= 1000;
                mTenths = std::min<uint32_t>(mTenths + tenthsPerSecond, 1000);
            }

            return mTenths >= 1000;
        }

        void drain() { mTenths = 0; }

    private:
        uint32_t mTenths = 0;
        uint32_t mClockMs = 0;
    };

    // Periodic Energy Gain: two energy every 800 ms
    uint32_t const MechagonEnergyTenthsPerSecond = 25;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class OperationMechagonInstanceScript : public InstanceScript
{
public:
    explicit OperationMechagonInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(OperationMechagonEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new OperationMechagonInstanceScript(pMapMgr); }

    // a credited member falling before the rest must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isCrewDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (state != EncounterStates::Failed)
            return;

        // a wiped encounter is fought again in full
        for (Creature* pMember : getCrew(id, true))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }

        if (id == DATA_HK8)
        {
            sendAction(NPC_TANK_BUSTER_MK1, ACTION_MK1_REBOOT);
        }
        else if (id == DATA_KING_MECHAGON)
        {
            sendAction(BOSS_OMEGA_BUSTER, ACTION_OMEGA_BUSTER_DORMANT);
            sendAction(NPC_KING_MECHAGON, ACTION_KING_RESET);
        }
    }

    std::vector<Creature*> getCrew(uint32_t dataId, bool neededOnly)
    {
        std::vector<Creature*> crew;
        for (MechagonCrewSlot const& slot : MechagonCrew)
        {
            if (slot.dataId != dataId || (neededOnly && !slot.neededForKill))
                continue;

            for (Creature* pMember : getCreatureSetForEntry(slot.entry))
                crew.push_back(pMember);
        }

        return crew;
    }

    bool isCrewDown(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : getCrew(dataId, true))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    // pulling one member pulls the ones that fight from the start
    void rallyCrew(uint32_t dataId, Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (MechagonCrewSlot const& slot : MechagonCrew)
        {
            if (slot.dataId != dataId || !slot.joinsOnPull)
                continue;

            for (Creature* pMember : getCreatureSetForEntry(slot.entry))
            {
                if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat() || pMember->GetScript() == nullptr)
                    continue;

                pMember->GetScript()->setZoneWideCombat();
            }
        }
        mRallying = false;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        for (Creature* pTarget : getCreatureSetForEntry(entry))
        {
            if (pTarget->isAlive() && pTarget->GetScript() != nullptr)
                pTarget->GetScript()->DoAction(action);
        }
    }

private:
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and target helpers

class OperationMechagonCombatAI : public CreatureAIScript
{
public:
    explicit OperationMechagonCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    OperationMechagonInstanceScript* getMechagon()
    {
        if (getCreature()->GetMapId() != MAP_OPERATION_MECHAGON)
            return nullptr;

        return static_cast<OperationMechagonInstanceScript*>(getInstanceScript());
    }

    void notify(uint32_t entry, int32_t action)
    {
        if (OperationMechagonInstanceScript* pMechagon = getMechagon())
            pMechagon->sendAction(entry, action);
    }

    Unit* currentVictim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

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

    LocationVector ringSpot(float minRange, float maxRange)
    {
        Creature* pSelf = getCreature();
        float const angle = Util::getRandomFloat(MechagonTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(pSelf->GetPositionX() + range * std::cos(angle), pSelf->GetPositionY() + range * std::sin(angle), pSelf->GetPositionZ(), angle);
    }

    // charges a unit and hits it on the way
    void rushAt(Unit* pTarget, uint32_t spellId)
    {
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, spellId, true);
        moveCharge(pTarget->GetPosition(), 40.0f);
    }

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void orbsDo(int32_t action)
    {
        std::list<Creature*> orbs;
        GetCreatureListWithEntryInGrid(orbs, NPC_PLASMA_ORB, 100.0f);
        for (Creature* pOrb : orbs)
        {
            if (pOrb->isAlive() && pOrb->GetScript() != nullptr)
                pOrb->GetScript()->DoAction(action);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class OperationMechagonBossAI : public OperationMechagonCombatAI
{
public:
    OperationMechagonBossAI(Creature* pCreature, uint32_t encounterId) : OperationMechagonCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        if (OperationMechagonInstanceScript* pMechagon = getMechagon())
            pMechagon->rallyCrew(mEncounterId, getCreature());

        OperationMechagonCombatAI::OnCombatStart(pTarget);
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

        OperationMechagonCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            OperationMechagonInstanceScript* pMechagon = getMechagon();
            if (pMechagon == nullptr || pMechagon->isCrewDown(mEncounterId, getCreature()))
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
// Tussle Tonks: The Platinum Pummeler and Gnomercy 4.U.

class OperationMechagonPummelerAI : public OperationMechagonBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonPummelerAI(c); }
    explicit OperationMechagonPummelerAI(Creature* pCreature) : OperationMechagonBossAI(pCreature, DATA_TUSSLE_TONKS) {}

    void OnLoad() override { plate(); }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_PUMMELER_WHIRLING_EDGE, 5000);
        scriptEvents.addEvent(EVENT_PUMMELER_LAY_MINE, 10000);
    }

    void onReset() override { plate(); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PUMMELER_WHIRLING_EDGE:
                castSpellAOE(SPELL_PUMMELER_WHIRLING_EDGE);
                scriptEvents.addEvent(EVENT_PUMMELER_WHIRLING_EDGE, 20000);
                break;
            case EVENT_PUMMELER_LAY_MINE:
                castAt(anyPlayer(), SPELL_PUMMELER_LAY_MINE, true);
                scriptEvents.addEvent(EVENT_PUMMELER_LAY_MINE, 30000);
                break;
            default:
                break;
        }
    }

private:
    // three layers of plating before the fight
    void plate()
    {
        for (uint8_t i = 0; i < 3; ++i)
            castSpellOnSelf(SPELL_PUMMELER_PLATINUM_PLATING, true);
    }
};

class OperationMechagonGnomercyAI : public OperationMechagonBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonGnomercyAI(c); }
    explicit OperationMechagonGnomercyAI(Creature* pCreature) : OperationMechagonBossAI(pCreature, DATA_TUSSLE_TONKS) {}

protected:
    // the robot only rams, it never swings
    void onEngage() override
    {
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_GNOMERCY_MAXIMUM_THRUST, 1000);
        scriptEvents.addEvent(EVENT_GNOMERCY_VENT_JETS, 6000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GNOMERCY_MAXIMUM_THRUST:
                if (Unit* pTarget = farthestPlayer(100.0f))
                {
                    warn("Gnomercy 4.U. engages Maximum Thrust!");
                    rushAt(pTarget, SPELL_GNOMERCY_MAXIMUM_THRUST);
                }
                scriptEvents.addEvent(EVENT_GNOMERCY_MAXIMUM_THRUST, 20000);
                break;
            case EVENT_GNOMERCY_VENT_JETS:
                castSpellOnSelf(SPELL_GNOMERCY_VENT_JETS);
                scriptEvents.addEvent(EVENT_GNOMERCY_VENT_JETS, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// K.U.-J.0.

class OperationMechagonKujoAI : public OperationMechagonBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonKujoAI(c); }
    explicit OperationMechagonKujoAI(Creature* pCreature) : OperationMechagonBossAI(pCreature, DATA_KUJO) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KUJO_BLAZING_CHOMP, 3000);
        scriptEvents.addEvent(EVENT_KUJO_AIR_DROP, 5000);
        scriptEvents.addEvent(EVENT_KUJO_VENTING_FLAMES, 15000);
        scriptEvents.addEvent(EVENT_KUJO_EXPLOSIVE_LEAP, 38000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KUJO_BLAZING_CHOMP:
                castSpellOnVictim(SPELL_KUJO_BLAZING_CHOMP, true);
                scriptEvents.addEvent(EVENT_KUJO_BLAZING_CHOMP, 60000);
                break;
            case EVENT_KUJO_AIR_DROP:
                summonCreature(NPC_FLYING_CLAW, ringSpot(5.0f, 25.0f), TIMED_DESPAWN, 8000);
                scriptEvents.addEvent(EVENT_KUJO_AIR_DROP, 30000);
                break;
            case EVENT_KUJO_VENTING_FLAMES:
                warn("K.U.-J.0. begins Venting Flames! Hide behind the junk!");
                castSpellAOE(SPELL_KUJO_VENTING_FLAMES);
                scriptEvents.addEvent(EVENT_KUJO_VENTING_FLAMES, 35000);
                break;
            case EVENT_KUJO_EXPLOSIVE_LEAP:
                if (Unit* pTarget = farthestPlayer(100.0f))
                {
                    mLeapSpot = pTarget->GetPosition();
                    scriptEvents.addEvent(EVENT_KUJO_LEAP_JUMP, 3000);
                }
                scriptEvents.addEvent(EVENT_KUJO_EXPLOSIVE_LEAP, 20000);
                break;
            case EVENT_KUJO_LEAP_JUMP:
                moveJump(mLeapSpot, 25.0f, 12.0f);
                scriptEvents.addEvent(EVENT_KUJO_LEAP_LAND, 1500);
                break;
            case EVENT_KUJO_LEAP_LAND:
                castSpellAOE(SPELL_KUJO_EXPLOSIVE_LEAP, true);
                break;
            default:
                break;
        }
    }

private:
    LocationVector mLeapSpot;
};

// drops its load a moment after it arrives
class OperationMechagonFlyingClawAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonFlyingClawAI(c); }
    explicit OperationMechagonFlyingClawAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mDropLeftMs == 0 || !getCreature()->isSummon())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mDropLeftMs > diff)
        {
            mDropLeftMs -= diff;
            return;
        }

        mDropLeftMs = 0;
        castSpellAOE(SPELL_FLYING_CLAW_AIR_DROP, true);
    }

private:
    uint32_t mDropLeftMs = 3000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Machinist's Garden: Head Machinist Sparkflux

class OperationMechagonSparkfluxAI : public OperationMechagonBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonSparkfluxAI(c); }
    explicit OperationMechagonSparkfluxAI(Creature* pCreature) : OperationMechagonBossAI(pCreature, DATA_MACHINISTS_GARDEN) {}

protected:
    void onEngage() override
    {
        mEnergy.reset(0);
        scriptEvents.addEvent(EVENT_SPARKFLUX_DISCOM_BOMBULATOR, 8000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mEnergy.fill(diff, MechagonEnergyTenthsPerSecond) || _isCasting())
            return;

        mEnergy.drain();
        warn("A \"Hidden\" Flame Cannon opens fire!");
        castAt(anyPlayer(), SPELL_SPARKFLUX_HIDDEN_FLAME_CANNON);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SPARKFLUX_DISCOM_BOMBULATOR)
            return;

        castAt(anyPlayer(), SPELL_SPARKFLUX_DISCOM_BOMBULATOR, true);
        scriptEvents.addEvent(EVENT_SPARKFLUX_DISCOM_BOMBULATOR, 20000);
    }

private:
    MechagonEnergyBar mEnergy;
};

//////////////////////////////////////////////////////////////////////////////////////////
// King Mechagon: the Aerial Unit first, then the Omega Buster

class OperationMechagonAerialUnitAI : public OperationMechagonBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonAerialUnitAI(c); }
    explicit OperationMechagonAerialUnitAI(Creature* pCreature) : OperationMechagonBossAI(pCreature, DATA_KING_MECHAGON) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_AERIAL_PLASMA_ORBS, 1000);
        scriptEvents.addEvent(EVENT_AERIAL_PULSE_BLAST, 2000);
        scriptEvents.addEvent(EVENT_AERIAL_RECALIBRATE, 7000);
        scriptEvents.addEvent(EVENT_AERIAL_TAKE_OFF, 12000);
    }

    // the king climbs into the Omega Buster
    void onDefeat() override
    {
        notify(BOSS_OMEGA_BUSTER, ACTION_OMEGA_BUSTER_ACTIVATE);
        notify(NPC_KING_MECHAGON, ACTION_KING_STAGE_TWO);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AERIAL_PLASMA_ORBS:
                for (uint32_t i = 0; i < PlasmaOrbsPerCall; ++i)
                    summonCreature(NPC_PLASMA_ORB, ringSpot(4.0f, 15.0f), MANUAL_DESPAWN);
                break;
            case EVENT_AERIAL_PULSE_BLAST:
                castSpellOnVictim(SPELL_AERIAL_PULSE_BLAST);
                scriptEvents.addEvent(EVENT_AERIAL_PULSE_BLAST, 2000);
                break;
            case EVENT_AERIAL_RECALIBRATE:
                orbsDo(ACTION_PLASMA_ORB_RECALIBRATE);
                scriptEvents.addEvent(EVENT_AERIAL_RECALIBRATE, 20000);
                break;
            case EVENT_AERIAL_TAKE_OFF:
                warn("The Aerial Unit takes off and charges its Cutting Beam!");
                castSpellOnSelf(SPELL_AERIAL_TAKE_OFF);
                scriptEvents.addEvent(EVENT_AERIAL_CUTTING_BEAM, 11000);
                scriptEvents.addEvent(EVENT_AERIAL_TAKE_OFF, 30000);
                break;
            case EVENT_AERIAL_CUTTING_BEAM:
                castSpellOnSelf(SPELL_AERIAL_CUTTING_BEAM);
                break;
            default:
                break;
        }
    }
};

// dormant until the Aerial Unit falls
class OperationMechagonOmegaBusterAI : public OperationMechagonBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonOmegaBusterAI(c); }
    explicit OperationMechagonOmegaBusterAI(Creature* pCreature) : OperationMechagonBossAI(pCreature, DATA_KING_MECHAGON) {}

    void OnLoad() override { goDormant(); }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_OMEGA_BUSTER_DORMANT)
        {
            goDormant();
            return;
        }

        if (action != ACTION_OMEGA_BUSTER_ACTIVATE || mActive || !isAlive())
            return;

        mActive = true;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        setZoneWideCombat();
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_OMEGA_GIGA_ZAP, 5000);
        scriptEvents.addEvent(EVENT_OMEGA_PLASMA_ORBS, 10000);
        scriptEvents.addEvent(EVENT_OMEGA_RECALIBRATE, 15000);
        scriptEvents.addEvent(EVENT_OMEGA_MAGNETO_ARM, 20000);
    }

    void onReset() override { goDormant(); }

    void onDefeat() override { notify(NPC_KING_MECHAGON, ACTION_KING_DEFEATED); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_OMEGA_GIGA_ZAP:
                castAt(anyPlayer(), SPELL_OMEGA_GIGA_ZAP);
                scriptEvents.addEvent(EVENT_OMEGA_GIGA_ZAP, 20000);
                break;
            case EVENT_OMEGA_PLASMA_ORBS:
                for (uint32_t i = 0; i < PlasmaOrbsPerCall; ++i)
                    summonCreature(NPC_PLASMA_ORB, ringSpot(4.0f, 15.0f), MANUAL_DESPAWN);
                break;
            case EVENT_OMEGA_RECALIBRATE:
                orbsDo(ACTION_PLASMA_ORB_RECALIBRATE);
                scriptEvents.addEvent(EVENT_OMEGA_RECALIBRATE, 25000);
                break;
            case EVENT_OMEGA_MAGNETO_ARM:
                castSpellOnSelf(SPELL_OMEGA_MAGNETO_ARM);
                orbsDo(ACTION_PLASMA_ORB_MAGNETIZE);
                scriptEvents.addEvent(EVENT_OMEGA_MAGNETO_ARM, 30000);
                break;
            default:
                break;
        }
    }

private:
    void goDormant()
    {
        mActive = false;
        setRooted(true);
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    bool mActive = false;
};

// rides along, never fights himself
class OperationMechagonKingAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonKingAI(c); }
    explicit OperationMechagonKingAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override { stayAside(); }

    void DoAction(int32_t action) override
    {
        switch (action)
        {
            case ACTION_KING_STAGE_TWO:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "King Mechagon takes control of the Omega Buster!");
                castSpellAOE(SPELL_KING_OBNOXIOUS_MONOLOGUE);
                break;
            case ACTION_KING_DEFEATED:
                MoveTeleport(KingMechagonThrownOut);
                removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                break;
            case ACTION_KING_RESET:
                MoveTeleport(getCreature()->GetSpawnPosition());
                stayAside();
                break;
            default:
                break;
        }
    }

private:
    void stayAside()
    {
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }
};

// bursts on contact, the machines throw it around
class OperationMechagonPlasmaOrbAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonPlasmaOrbAI(c); }
    explicit OperationMechagonPlasmaOrbAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_PLASMA_ORB_VISUAL, true);
    }

    void DoAction(int32_t action) override
    {
        if (mBurst)
            return;

        if (action == ACTION_PLASMA_ORB_RECALIBRATE)
        {
            std::vector<Unit*> players = playersWithin(60.0f);
            if (players.empty())
                return;

            Unit* pTarget = players[Util::getRandomUInt(static_cast<uint32_t>(players.size() - 1))];
            moveCharge(pTarget->GetPosition(), 20.0f);
            mKnockLeftMs = 2000;
        }
        else if (action == ACTION_PLASMA_ORB_MAGNETIZE)
        {
            Creature* pBuster = findNearestCreature(BOSS_OMEGA_BUSTER, 100.0f);
            if (pBuster != nullptr && pBuster->isAlive())
                moveTo(pBuster->GetPositionX(), pBuster->GetPositionY(), pBuster->GetPositionZ(), false);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mBurst || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mKnockLeftMs > 0)
        {
            if (mKnockLeftMs > diff)
            {
                mKnockLeftMs -= diff;
            }
            else
            {
                mKnockLeftMs = 0;
                castSpellAOE(SPELL_PLASMA_ORB_RECALIBRATE, true);
            }
        }

        if (mContactClockMs > diff)
        {
            mContactClockMs -= diff;
            return;
        }

        mContactClockMs = 500;
        if (playersWithin(2.5f).empty())
            return;

        mBurst = true;
        castSpellAOE(SPELL_PLASMA_ORB_BURST, true);
        despawn(500, 0);
    }

private:
    std::vector<Unit*> playersWithin(float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && getRangeToObject(pPlayer) <= range)
                players.push_back(pPlayer);
        }

        return players;
    }

    uint32_t mKnockLeftMs = 0;
    uint32_t mContactClockMs = 1500;
    bool mBurst = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// King Gobbamak, a Charged Smash on every full bar

class OperationMechagonGobbamakAI : public OperationMechagonBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonGobbamakAI(c); }
    explicit OperationMechagonGobbamakAI(Creature* pCreature) : OperationMechagonBossAI(pCreature, DATA_KING_GOBBAMAK) {}

protected:
    void onEngage() override
    {
        mEnergy.reset(33);
        scriptEvents.addEvent(EVENT_GOBBAMAK_ELECTRICAL_CHARGE, 3000);
        scriptEvents.addEvent(EVENT_GOBBAMAK_RUMBLE, 8000);
        scriptEvents.addEvent(EVENT_GOBBAMAK_CALL_GRUNTERS, 17000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mEnergy.fill(diff, MechagonEnergyTenthsPerSecond) || _isCasting())
            return;

        mEnergy.drain();
        warn("King Gobbamak winds up a Charged Smash!");
        castSpellOnVictim(SPELL_GOBBAMAK_CHARGED_SMASH);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GOBBAMAK_ELECTRICAL_CHARGE:
                castAt(anyPlayer(), SPELL_GOBBAMAK_ELECTRICAL_CHARGE, true);
                scriptEvents.addEvent(EVENT_GOBBAMAK_ELECTRICAL_CHARGE, 15000);
                break;
            case EVENT_GOBBAMAK_RUMBLE:
                castSpellOnSelf(SPELL_GOBBAMAK_RUMBLE);
                scriptEvents.addEvent(EVENT_GOBBAMAK_RUMBLE, 45000);
                break;
            case EVENT_GOBBAMAK_CALL_GRUNTERS:
                scriptEvents.addEvent(EVENT_GOBBAMAK_GRUNTERS_ARRIVE, 3000);
                scriptEvents.addEvent(EVENT_GOBBAMAK_CALL_GRUNTERS, 25000);
                break;
            case EVENT_GOBBAMAK_GRUNTERS_ARRIVE:
                for (uint32_t i = 0; i < GobbamakGruntersPerCall; ++i)
                    callAdd(NPC_SCRAPBONE_GRUNTER, GobbamakGrunterCave);
                break;
            default:
                break;
        }
    }

private:
    MechagonEnergyBar mEnergy;
};

//////////////////////////////////////////////////////////////////////////////////////////
// HK-8 Aerial Oppression Unit, shielded above the Tank Buster until it shuts down

class OperationMechagonHK8AI : public OperationMechagonBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonHK8AI(c); }
    explicit OperationMechagonHK8AI(Creature* pCreature) : OperationMechagonBossAI(pCreature, DATA_HK8) {}

    void OnLoad() override { hover(); }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_HK8_STAGE_TWO || mStage == 2 || !isAlive())
            return;

        if (!_isInCombat())
            setZoneWideCombat();

        mStage = 2;
        scriptEvents.resetEvents();
        takeToTheAir();
    }

protected:
    void onEngage() override
    {
        // pulled straight into the second stage when the tank buster is already down
        if (mStage != 0)
            return;

        mStage = 1;
        scriptEvents.addEvent(EVENT_HK8_CANNON_BLAST, 19000);
    }

    void onReset() override
    {
        mStage = 0;
        _removeAura(SPELL_HK8_HAYWIRE);
        hover();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HK8_CANNON_BLAST:
                warn("HK-8 Aerial Oppression Unit targets the arena with its Cannon Blast!");
                castSpellAOE(SPELL_HK8_CANNON_BLAST);
                scriptEvents.addEvent(EVENT_HK8_CANNON_SHELLS, 3000);
                scriptEvents.addEvent(EVENT_HK8_CANNON_BLAST, 35000);
                break;
            case EVENT_HK8_CANNON_SHELLS:
                for (Unit* pPlayer : livingPlayers(150.0f))
                    castSpell(pPlayer, SPELL_HK8_CANNON_BLAST_SHELL, true);
                break;
            case EVENT_HK8_CRASH_DOWN:
                crashDown();
                break;
            case EVENT_HK8_LIFT_OFF:
                castSpellAOE(SPELL_HK8_LIFT_OFF);
                _removeAura(SPELL_HK8_HAYWIRE);
                takeToTheAir();
                break;
            default:
                break;
        }
    }

private:
    // out of reach behind the force shield
    void hover()
    {
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_HK8_FORCE_SHIELD, true);
    }

    void takeToTheAir()
    {
        hover();
        castSpellOnSelf(SPELL_HK8_ANNIHILATION_RAY);
        scriptEvents.addEvent(EVENT_HK8_CRASH_DOWN, HK8AirborneMs);
    }

    // shield down and grounded for a while
    void crashDown()
    {
        warn("HK-8 Aerial Oppression Unit goes haywire and crashes down!");
        _removeAura(SPELL_HK8_ANNIHILATION_RAY);
        _removeAura(SPELL_HK8_FORCE_SHIELD);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        _setMeleeDisabled(false);
        castSpellOnSelf(SPELL_HK8_HAYWIRE, true);
        scriptEvents.addEvent(EVENT_HK8_LIFT_OFF, HK8HaywireMs);
    }

    uint32_t mStage = 0;
};

class OperationMechagonTankBusterAI : public OperationMechagonBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonTankBusterAI(c); }
    explicit OperationMechagonTankBusterAI(Creature* pCreature) : OperationMechagonBossAI(pCreature, DATA_HK8) {}

    // a shut down tank buster is no wipe
    void OnCombatStop(Unit* pTarget) override
    {
        if (mShutDown)
        {
            OperationMechagonCombatAI::OnCombatStop(pTarget);
            return;
        }

        OperationMechagonBossAI::OnCombatStop(pTarget);
    }

    // stops short of dying
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mShutDown)
        {
            *damage = 0;
            return;
        }

        uint32_t const floor = getCreature()->getMaxHealth() / 100 * MK1ShutdownPct;
        uint32_t const health = getCreature()->getHealth();
        if (health > *damage + floor)
            return;

        *damage = health > floor ? health - floor : 0;
        shutDown();
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_MK1_REBOOT || !mShutDown)
            return;

        mShutDown = false;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        getCreature()->setHealth(getCreature()->getMaxHealth());
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_MK1_WRECK, 3000);
        scriptEvents.addEvent(EVENT_MK1_FULMINATING_ZAP, 8000);
        scriptEvents.addEvent(EVENT_MK1_REINFORCEMENTS, 13000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MK1_WRECK:
                castSpellOnSelf(SPELL_MK1_WRECK);
                scriptEvents.addEvent(EVENT_MK1_WRECK, 3000);
                break;
            case EVENT_MK1_FULMINATING_ZAP:
                castAt(anyPlayer(), SPELL_MK1_FULMINATING_ZAP);
                scriptEvents.addEvent(EVENT_MK1_FULMINATING_ZAP, 15000);
                break;
            case EVENT_MK1_REINFORCEMENTS:
                for (uint32_t i = 0; i < MK1WalkiesPerRelay; ++i)
                    callAdd(NPC_WALKIE_SHOCKIE_X1, ringSpot(8.0f, 15.0f));
                scriptEvents.addEvent(EVENT_MK1_REINFORCEMENTS, 30000);
                break;
            default:
                break;
        }
    }

private:
    void shutDown()
    {
        mShutDown = true;
        scriptEvents.resetEvents();
        summons.despawnAll();
        attackStop();
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        notify(BOSS_HK8, ACTION_HK8_STAGE_TWO);
    }

    bool mShutDown = false;
};

// rolls in and blows up on a short fuse
class OperationMechagonWalkieShockieAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonWalkieShockieAI(c); }
    explicit OperationMechagonWalkieShockieAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override { castSpellOnSelf(SPELL_WALKIE_BATTERY_LIFE, true); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mFuseLeftMs == 0 || !isAlive() || !getCreature()->isSummon())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mFuseLeftMs > diff)
        {
            mFuseLeftMs -= diff;
            return;
        }

        mFuseLeftMs = 0;
        castSpellAOE(SPELL_WALKIE_SELF_DESTRUCT, true);
        despawn(1000, 0);
    }

private:
    uint32_t mFuseLeftMs = WalkieShockieFuseMs;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gunker, rooted in its pool, a Toxic Wave on every full bar

class OperationMechagonGunkerAI : public OperationMechagonBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonGunkerAI(c); }
    explicit OperationMechagonGunkerAI(Creature* pCreature) : OperationMechagonBossAI(pCreature, DATA_GUNKER) {}

    void OnLoad() override { setRooted(true); }

protected:
    void onEngage() override
    {
        mEnergy.reset(0);
        scriptEvents.addEvent(EVENT_GUNKER_SLUDGE_BOLT, 1000);
        scriptEvents.addEvent(EVENT_GUNKER_SPLATTER, 8000);
        scriptEvents.addEvent(EVENT_GUNKER_COALESCE, 20000);
        scriptEvents.addEvent(EVENT_GUNKER_TOXIC_FUMES, 50000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mEnergy.fill(diff, MechagonEnergyTenthsPerSecond) || _isCasting())
            return;

        mEnergy.drain();
        warn("Gunker unleashes a Toxic Wave!");
        castSpellOnSelf(SPELL_GUNKER_TOXIC_WAVE);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GUNKER_SLUDGE_BOLT:
            {
                // the tank out of reach gets shot instead
                Unit* pVictim = currentVictim();
                if (pVictim != nullptr && getRangeToObject(pVictim) > 5.0f)
                    castSpell(pVictim, SPELL_GUNKER_SLUDGE_BOLT);
                scriptEvents.addEvent(EVENT_GUNKER_SLUDGE_BOLT, 2000);
                break;
            }
            case EVENT_GUNKER_SPLATTER:
                castSpellOnSelf(SPELL_GUNKER_SPLATTER);
                scriptEvents.addEvent(EVENT_GUNKER_SPLATTER, 35000);
                break;
            case EVENT_GUNKER_COALESCE:
                castSpellOnSelf(SPELL_GUNKER_COALESCE);
                scriptEvents.addEvent(EVENT_GUNKER_COALESCE, 30000);
                break;
            case EVENT_GUNKER_TOXIC_FUMES:
                castSpellOnSelf(SPELL_GUNKER_TOXIC_FUMES, true);
                scriptEvents.addEvent(EVENT_GUNKER_TOXIC_FUMES, 25000);
                break;
            default:
                break;
        }
    }

private:
    MechagonEnergyBar mEnergy;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Trixie Tazer and Naeno Megacrash

class OperationMechagonTrixieAI : public OperationMechagonBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonTrixieAI(c); }
    explicit OperationMechagonTrixieAI(Creature* pCreature) : OperationMechagonBossAI(pCreature, DATA_TRIXIE_NAENO) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_GNOME_PARTNER_DOWN || !_isInCombat())
            return;

        castSpellOnSelf(SPELL_TRIXIE_SUPER_BOOST, true);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_TRIXIE_TAZE, 3000);
        scriptEvents.addEvent(EVENT_TRIXIE_ELECTRIC_SLIDE, 10000);
        scriptEvents.addEvent(EVENT_TRIXIE_MEGA_TAZE, 15000);
    }

    void onReset() override { _removeAura(SPELL_TRIXIE_ELECTRIC_SLIDE); }

    void onDefeat() override { notify(BOSS_NAENO_MEGACRASH, ACTION_GNOME_PARTNER_DOWN); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TRIXIE_TAZE:
                castAt(anyPlayer(), SPELL_TRIXIE_TAZE);
                scriptEvents.addEvent(EVENT_TRIXIE_TAZE, 5000);
                break;
            case EVENT_TRIXIE_ELECTRIC_SLIDE:
                if (Creature* pJumpPoint = findNearestCreature(NPC_JUMP_POINT_STALKER, 25.0f))
                {
                    castSpellOnSelf(SPELL_TRIXIE_ELECTRIC_SLIDE, true);
                    moveJump(pJumpPoint->GetPosition(), 30.0f, 30.0f);
                    scriptEvents.addEvent(EVENT_TRIXIE_SLIDE_END, 3000);
                }
                scriptEvents.addEvent(EVENT_TRIXIE_ELECTRIC_SLIDE, 20000);
                break;
            case EVENT_TRIXIE_SLIDE_END:
                _removeAura(SPELL_TRIXIE_ELECTRIC_SLIDE);
                break;
            case EVENT_TRIXIE_MEGA_TAZE:
                if (Unit* pTarget = anyPlayer())
                {
                    warn("Trixie Tazer channels a Mega Taze! Hide in the smoke!");
                    castSpell(pTarget, SPELL_TRIXIE_MEGA_TAZE);
                    mTazeTarget = pTarget->GetNewGUID();
                    scriptEvents.addEvent(EVENT_TRIXIE_MEGA_TAZE_JOLT, 8000);
                }
                scriptEvents.addEvent(EVENT_TRIXIE_MEGA_TAZE, 30000);
                break;
            case EVENT_TRIXIE_MEGA_TAZE_JOLT:
                joltTazeTarget();
                break;
            default:
                break;
        }
    }

private:
    // the smoke cloud of the bike shields the marked player
    void joltTazeTarget()
    {
        WorldMap* pMap = getCreature()->getWorldMap();
        Unit* pTarget = (pMap != nullptr && !mTazeTarget.isEmpty()) ? pMap->getUnit(mTazeTarget) : nullptr;
        if (pTarget == nullptr || !pTarget->isAlive() || pTarget->hasAurasWithId(SPELL_NAENO_SMOKE_CLOUD))
            return;

        castSpell(pTarget, SPELL_TRIXIE_MEGA_TAZE_JOLT, true);
    }

    WoWGuid mTazeTarget;
};

// out of reach on the Mechacycle half of the time
class OperationMechagonNaenoAI : public OperationMechagonBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OperationMechagonNaenoAI(c); }
    explicit OperationMechagonNaenoAI(Creature* pCreature) : OperationMechagonBossAI(pCreature, DATA_TRIXIE_NAENO) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_GNOME_PARTNER_DOWN || !_isInCombat())
            return;

        mPartnerDown = true;
        castSpellOnSelf(SPELL_NAENO_TURBO_BOOST, true);

        if (mRiding)
            stepOff();
        else
            scriptEvents.removeEvent(EVENT_NAENO_MOUNT);
    }

protected:
    void onEngage() override
    {
        mPartnerDown = false;
        ride();
    }

    void onReset() override
    {
        mPartnerDown = false;
        mRiding = false;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        _setMeleeDisabled(false);
    }

    void onDefeat() override { notify(BOSS_TRIXIE_TAZER, ACTION_GNOME_PARTNER_DOWN); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NAENO_PEDAL_TO_THE_METAL:
                rushAt(farthestPlayer(100.0f), SPELL_NAENO_PEDAL_TO_THE_METAL);
                scriptEvents.addEvent(EVENT_NAENO_PEDAL_TO_THE_METAL, 20000);
                break;
            case EVENT_NAENO_BURNOUT:
                castSpellAOE(SPELL_NAENO_BURNOUT);
                scriptEvents.addEvent(EVENT_NAENO_BURNOUT, 25000);
                break;
            case EVENT_NAENO_DISMOUNT:
                stepOff();
                break;
            case EVENT_NAENO_MOUNT:
                ride();
                break;
            case EVENT_NAENO_BOLT_BUSTER:
                castSpellOnVictim(SPELL_NAENO_BOLT_BUSTER);
                scriptEvents.addEvent(EVENT_NAENO_BOLT_BUSTER, 15000);
                break;
            case EVENT_NAENO_ROADKILL:
                rushAt(farthestPlayer(100.0f), SPELL_NAENO_ROADKILL);
                scriptEvents.addEvent(EVENT_NAENO_ROADKILL, 20000);
                break;
            default:
                break;
        }
    }

private:
    void ride()
    {
        mRiding = true;
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        _setMeleeDisabled(true);

        scriptEvents.removeEvent(EVENT_NAENO_BOLT_BUSTER);
        scriptEvents.removeEvent(EVENT_NAENO_ROADKILL);
        scriptEvents.addEvent(EVENT_NAENO_PEDAL_TO_THE_METAL, 5000);
        scriptEvents.addEvent(EVENT_NAENO_BURNOUT, 10000);
        scriptEvents.addEvent(EVENT_NAENO_DISMOUNT, NaenoRideMs);
    }

    void stepOff()
    {
        mRiding = false;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        _setMeleeDisabled(false);

        scriptEvents.removeEvent(EVENT_NAENO_PEDAL_TO_THE_METAL);
        scriptEvents.removeEvent(EVENT_NAENO_BURNOUT);
        scriptEvents.removeEvent(EVENT_NAENO_DISMOUNT);
        scriptEvents.addEvent(EVENT_NAENO_BOLT_BUSTER, 3000);
        scriptEvents.addEvent(EVENT_NAENO_ROADKILL, 10000);

        if (!mPartnerDown)
            scriptEvents.addEvent(EVENT_NAENO_MOUNT, NaenoRideMs);
    }

    bool mRiding = false;
    bool mPartnerDown = false;
};

void SetupOperationMechagon(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_OPERATION_MECHAGON, &OperationMechagonInstanceScript::Create);

    mgr->register_creature_script(BOSS_PLATINUM_PUMMELER, &OperationMechagonPummelerAI::Create);
    mgr->register_creature_script(BOSS_GNOMERCY, &OperationMechagonGnomercyAI::Create);

    mgr->register_creature_script(BOSS_KUJO, &OperationMechagonKujoAI::Create);
    mgr->register_creature_script(NPC_FLYING_CLAW, &OperationMechagonFlyingClawAI::Create);

    mgr->register_creature_script(BOSS_HEAD_MACHINIST_SPARKFLUX, &OperationMechagonSparkfluxAI::Create);

    mgr->register_creature_script(BOSS_AERIAL_UNIT, &OperationMechagonAerialUnitAI::Create);
    mgr->register_creature_script(BOSS_OMEGA_BUSTER, &OperationMechagonOmegaBusterAI::Create);
    mgr->register_creature_script(NPC_KING_MECHAGON, &OperationMechagonKingAI::Create);
    mgr->register_creature_script(NPC_PLASMA_ORB, &OperationMechagonPlasmaOrbAI::Create);

    mgr->register_creature_script(BOSS_KING_GOBBAMAK, &OperationMechagonGobbamakAI::Create);

    mgr->register_creature_script(BOSS_HK8, &OperationMechagonHK8AI::Create);
    mgr->register_creature_script(NPC_TANK_BUSTER_MK1, &OperationMechagonTankBusterAI::Create);
    mgr->register_creature_script(NPC_WALKIE_SHOCKIE_X1, &OperationMechagonWalkieShockieAI::Create);

    mgr->register_creature_script(BOSS_GUNKER, &OperationMechagonGunkerAI::Create);

    mgr->register_creature_script(BOSS_TRIXIE_TAZER, &OperationMechagonTrixieAI::Create);
    mgr->register_creature_script(BOSS_NAENO_MEGACRASH, &OperationMechagonNaenoAI::Create);
}
