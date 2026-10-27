/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Stonevault.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <map>
#include <utility>
#include <vector>

enum TheStonevaultEvents
{
    EVENT_EDNA_VOLATILE_SPIKE = 1,
    EVENT_EDNA_REFRACTING_BEAM,
    EVENT_EDNA_BEAM_SWEEP,
    EVENT_EDNA_SEISMIC_SMASH,

    EVENT_SKARMORAK_CRYSTALLINE_SMASH,
    EVENT_SKARMORAK_SHARDS_FORM,
    EVENT_SKARMORAK_UNSTABLE_CRASH,
    EVENT_SKARMORAK_FRAGMENTS_FLY,
    EVENT_SKARMORAK_VOID_DISCHARGE,

    EVENT_EIRICH_VOID_CORRUPTION,
    EVENT_EIRICH_CORRUPTION_DRAWN,
    EVENT_EIRICH_ENTROPIC_RECKONING,
    EVENT_EIRICH_UNBRIDLED_VOID,

    EVENT_BROKK_MOLTEN_METAL,
    EVENT_BROKK_SCRAP_SONG,
    EVENT_BROKK_SCRAP_CUBE,
    EVENT_BROKK_EXHAUST_VENTS,

    EVENT_DORLITA_IGNEOUS_HAMMER,
    EVENT_DORLITA_LAVA_CANNON,
    EVENT_DORLITA_CANNON_HIT,
    EVENT_DORLITA_BLAZING_CRESCENDO,
    EVENT_DORLITA_CRESCENDO_BLAST
};

namespace
{
    float const TwoPi = 6.28318531f;
    float const FightReach = 60.0f;

    // room doors shut during the fight, passages open after the kill
    struct StonevaultDoor
    {
        uint32_t entry;
        uint32_t dataId;
        bool passage;
    };

    StonevaultDoor const StonevaultDoors[] =
    {
        { GO_EDNA_ENTRANCE_DOOR,        DATA_EDNA,              false },
        { GO_EDNA_SOUTH_GATE,           DATA_EDNA,              true },
        { GO_EDNA_NORTH_GATE,           DATA_EDNA,              true },
        { GO_MACHINISTS_FIRE_BARRIER,   DATA_MASTER_MACHINISTS, false }
    };

    StonevaultDoor const* findStonevaultDoor(uint32_t entry)
    {
        for (StonevaultDoor const& door : StonevaultDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    uint32_t const MachinistEntries[] = { BOSS_SPEAKER_BROKK, BOSS_SPEAKER_DORLITA };

    // E.D.N.A.
    uint32_t const SpikeLifetimeMs = 60000;
    uint32_t const SpikeArmMs = 2000;
    uint32_t const SpikeCheckMs = 250;
    float const SpikeTouchRange = 2.5f;
    float const BeamHalfWidth = 3.0f;

    // Skarmorak
    uint32_t const SkarmorakDrainPerSecond = 2;
    uint32_t const ShellBasePct = 5;
    uint32_t const ShellPerShardPct = 2;
    uint32_t const ShellMaxMs = 25000;
    uint8_t const ShardsPerSmash = 3;

    // Void Speaker Eirich
    uint32_t const CorruptionDrawnMs = 15000;

    LocationVector pointAround(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(TwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    std::vector<Unit*> playersNear(Creature* pCreature, LocationVector const& spot, float range)
    {
        std::vector<Unit*> players;
        for (Object* pObject : pCreature->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pPlayer->getDistance(spot) <= range)
                players.push_back(pPlayer);
        }

        return players;
    }

    // 2d distance of a point to the segment from to
    float distanceToSegment(LocationVector const& from, LocationVector const& to, LocationVector const& point)
    {
        float const dx = to.x - from.x;
        float const dy = to.y - from.y;
        float const lengthSq = dx * dx + dy * dy;

        float t = 0.0f;
        if (lengthSq > 0.0f)
        {
            t = ((point.x - from.x) * dx + (point.y - from.y) * dy) / lengthSq;
            t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        }

        float const offX = from.x + t * dx - point.x;
        float const offY = from.y + t * dy - point.y;
        return std::sqrt(offX * offX + offY * offY);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TheStonevaultInstanceScript : public InstanceScript
{
public:
    explicit TheStonevaultInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheStonevaultEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheStonevaultInstanceScript(pMapMgr); }

    // one fallen speaker must not end the encounter while the other stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (uint32_t entry : MachinistEntries)
        {
            if (entry == pCreature->getEntry())
            {
                mMemberGuids[entry] = pCreature->GetNewGUID();
                return;
            }
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findStonevaultDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        applyDoorState(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoorState(pDoor);
        }

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
        if (dataId != DATA_MASTER_MACHINISTS)
            return group;

        for (uint32_t entry : MachinistEntries)
        {
            if (Creature* pMember = getMember(entry))
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

    // pulling one speaker pulls the other
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
    void applyDoorState(GameObject* pDoor)
    {
        StonevaultDoor const* pRule = findStonevaultDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        EncounterStates const state = getBossState(pRule->dataId);
        bool const open = pRule->passage ? state == EncounterStates::Performed : state != EncounterStates::InProgress;
        uint8_t const wanted = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::map<uint32_t, WoWGuid> mMemberGuids;
    std::vector<WoWGuid> mDoorGuids;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and target helpers

class StonevaultCombatAI : public CreatureAIScript
{
public:
    explicit StonevaultCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

        while (!_isCasting())
        {
            uint32_t const eventId = scriptEvents.getFinishedEvent();
            if (eventId == 0)
                break;

            onEvent(eventId);
        }
    }

protected:
    virtual void onEngage() {}
    virtual void onReset() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    TheStonevaultInstanceScript* getStonevault()
    {
        if (getCreature()->GetMapId() != MAP_THE_STONEVAULT)
            return nullptr;

        return static_cast<TheStonevaultInstanceScript*>(getInstanceScript());
    }

    Unit* victim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

    // distinct random players in reach, the tank only when nobody else is left
    std::vector<Unit*> pickPlayers(size_t count, bool skipTank = false)
    {
        Unit* pTank = skipTank ? victim() : nullptr;
        std::vector<Unit*> pool;
        for (Unit* pPlayer : playersNear(getCreature(), getCreature()->GetPosition(), FightReach))
        {
            if (pPlayer != pTank)
                pool.push_back(pPlayer);
        }

        if (pool.empty() && pTank != nullptr && pTank->isAlive())
            pool.push_back(pTank);

        for (size_t i = 0; i < pool.size() && i < count; ++i)
        {
            size_t const pick = i + Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1 - i));
            std::swap(pool[i], pool[pick]);
        }

        if (pool.size() > count)
            pool.resize(count);

        return pool;
    }

    Unit* randomPlayer(bool skipTank = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* castAtRandomPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        Unit* pTarget = randomPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    std::vector<Creature*> livingSummons(uint32_t entry)
    {
        std::vector<Creature*> found;
        for (uint64_t const guid : summons)
        {
            Creature* pSummon = getCreature()->getWorldMapCreature(guid);
            if (pSummon != nullptr && pSummon->getEntry() == entry && pSummon->isAlive())
                found.push_back(pSummon);
        }

        return found;
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class StonevaultBossAI : public StonevaultCombatAI
{
public:
    StonevaultBossAI(Creature* pCreature, uint32_t dataId) : StonevaultCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (TheStonevaultInstanceScript* pStonevault = getStonevault())
            pStonevault->rallyGroup(mDataId, getCreature());

        StonevaultCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                TheStonevaultInstanceScript* pStonevault = getStonevault();
                if (pStonevault == nullptr || !pStonevault->isGroupFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        StonevaultCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            TheStonevaultInstanceScript* pStonevault = getStonevault();
            if (pStonevault == nullptr || pStonevault->isGroupDown(mDataId, getCreature()))
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
// E.D.N.A.

class StonevaultEdnaAI : public StonevaultBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonevaultEdnaAI(c); }
    explicit StonevaultEdnaAI(Creature* pCreature) : StonevaultBossAI(pCreature, DATA_EDNA) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_EDNA_VOLATILE_SPIKE, 8100);
        scriptEvents.addEvent(EVENT_EDNA_REFRACTING_BEAM, 11800);
        scriptEvents.addEvent(EVENT_EDNA_SEISMIC_SMASH, 15400);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_EDNA_VOLATILE_SPIKE:
                plantSpikes();
                scriptEvents.addEvent(EVENT_EDNA_VOLATILE_SPIKE, 14600);
                break;
            case EVENT_EDNA_REFRACTING_BEAM:
                if (Unit* pTarget = randomPlayer())
                {
                    mBeamFrom = getCreature()->GetPosition();
                    mBeamTo = pTarget->GetPosition();
                    castSpell(pTarget, SPELL_EDNA_REFRACTING_BEAM);
                    scriptEvents.addEvent(EVENT_EDNA_BEAM_SWEEP, 300);
                }
                scriptEvents.addEvent(EVENT_EDNA_REFRACTING_BEAM, 10900);
                break;
            case EVENT_EDNA_BEAM_SWEEP:
                shatterSpikesInBeam();
                break;
            case EVENT_EDNA_SEISMIC_SMASH:
                warn("E.D.N.A. readies a Seismic Smash!");
                castSpellOnVictim(SPELL_EDNA_SEISMIC_SMASH);
                scriptEvents.addEvent(EVENT_EDNA_SEISMIC_SMASH, 23100);
                break;
            default:
                break;
        }
    }

private:
    // a spike lands where each chosen player stands
    void plantSpikes()
    {
        castSpellAOE(SPELL_EDNA_VOLATILE_SPIKE, true);

        for (Unit* pTarget : pickPlayers(isHeroic() ? 3 : 2))
            summonCreature(NPC_EDNA_VOLATILE_SPIKE, pTarget->GetPosition(), TIMED_DESPAWN, SpikeLifetimeMs);
    }

    // spikes in the beam's path are destroyed without exploding
    void shatterSpikesInBeam()
    {
        for (Creature* pSpike : livingSummons(NPC_EDNA_VOLATILE_SPIKE))
        {
            if (distanceToSegment(mBeamFrom, mBeamTo, pSpike->GetPosition()) <= BeamHalfWidth)
                pSpike->despawn(200, 0);
        }
    }

    LocationVector mBeamFrom;
    LocationVector mBeamTo;
};

// explodes on the first player stepping onto it
class StonevaultVolatileSpikeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonevaultVolatileSpikeAI(c); }
    explicit StonevaultVolatileSpikeAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive() || mSpent)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mArmMs > diff)
        {
            mArmMs -= diff;
            return;
        }
        mArmMs = 0;

        mCheckMs += diff;
        if (mCheckMs < SpikeCheckMs)
            return;
        mCheckMs = 0;

        if (playersNear(getCreature(), getCreature()->GetPosition(), SpikeTouchRange).empty())
            return;

        mSpent = true;
        castSpellAOE(SPELL_EDNA_SPIKE_VOLATILE_EXPLOSION, true);
        despawn(300, 0);
    }

private:
    uint32_t mArmMs = SpikeArmMs;
    uint32_t mCheckMs = 0;
    bool mSpent = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Skarmorak, shelled whenever its energy runs dry

class StonevaultSkarmorakAI : public StonevaultBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonevaultSkarmorakAI(c); }
    explicit StonevaultSkarmorakAI(Creature* pCreature) : StonevaultBossAI(pCreature, DATA_SKARMORAK) {}

    // the shell soaks damage until its pool is gone
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || !mShelled || mShellBroken)
            return;

        if (*damage < mShellLeft)
        {
            mShellLeft -= *damage;
            *damage = 0;
            return;
        }

        *damage -= mShellLeft;
        mShellLeft = 0;
        mShellBroken = true;
    }

protected:
    void onEngage() override
    {
        mEnergy = 100;
        mDrainMs = 0;
        mShelled = false;
        mShellBroken = false;
        scheduleAssault(4600, 10500);
    }

    void onReset() override
    {
        mShelled = false;
        _removeAura(SPELL_SKARMORAK_FORTIFIED_SHELL);
        _removeAura(SPELL_SKARMORAK_VOID_DISCHARGE);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mShelled)
        {
            mShellMs += diff;
            if (mShellBroken)
                dropShell(true);
            else if (mShellMs >= ShellMaxMs)
                dropShell(false);
            return;
        }

        mDrainMs += diff;
        while (mDrainMs >= 1000)
        {
            mDrainMs -= 1000;
            mEnergy = mEnergy > SkarmorakDrainPerSecond ? mEnergy - SkarmorakDrainPerSecond : 0;
        }

        if (mEnergy == 0 && !_isCasting())
            raiseShell();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SKARMORAK_CRYSTALLINE_SMASH:
                if (Unit* pTarget = victim())
                {
                    mSmashSpot = pTarget->GetPosition();
                    castSpell(pTarget, SPELL_SKARMORAK_CRYSTALLINE_SMASH);
                    scriptEvents.addEvent(EVENT_SKARMORAK_SHARDS_FORM, 300);
                }
                scriptEvents.addEvent(EVENT_SKARMORAK_CRYSTALLINE_SMASH, 17000);
                break;
            case EVENT_SKARMORAK_SHARDS_FORM:
                for (uint8_t i = 0; i < ShardsPerSmash; ++i)
                    summonCreature(NPC_SKARMORAK_CRYSTAL_SHARD, pointAround(mSmashSpot, 3.0f, 8.0f), CORPSE_DESPAWN);
                break;
            case EVENT_SKARMORAK_UNSTABLE_CRASH:
                castSpellAOE(SPELL_SKARMORAK_UNSTABLE_CRASH);
                scriptEvents.addEvent(EVENT_SKARMORAK_FRAGMENTS_FLY, 300);
                scriptEvents.addEvent(EVENT_SKARMORAK_UNSTABLE_CRASH, 19400);
                break;
            case EVENT_SKARMORAK_FRAGMENTS_FLY:
                for (Unit* pTarget : pickPlayers(2))
                    getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_SKARMORAK_UNSTABLE_FRAGMENTS, true);
                break;
            case EVENT_SKARMORAK_VOID_DISCHARGE:
                warn("Skarmorak unleashes a Void Discharge! Break its shell!");
                castSpellOnSelf(SPELL_SKARMORAK_VOID_DISCHARGE);
                break;
            default:
                break;
        }
    }

private:
    void scheduleAssault(uint32_t smashMs, uint32_t crashMs)
    {
        scriptEvents.addEvent(EVENT_SKARMORAK_CRYSTALLINE_SMASH, smashMs);
        scriptEvents.addEvent(EVENT_SKARMORAK_UNSTABLE_CRASH, crashMs);
    }

    // every living shard thickens the shell
    void raiseShell()
    {
        std::vector<Creature*> const shards = livingSummons(NPC_SKARMORAK_CRYSTAL_SHARD);
        uint32_t const shellPct = ShellBasePct + ShellPerShardPct * static_cast<uint32_t>(shards.size());

        mShelled = true;
        mShellBroken = false;
        mShellMs = 0;
        mShellLeft = static_cast<uint32_t>(static_cast<uint64_t>(getCreature()->getMaxHealth()) * shellPct / 100);

        scriptEvents.resetEvents();
        warn("Skarmorak hardens into a Fortified Shell!");
        castSpellOnSelf(SPELL_SKARMORAK_FORTIFIED_SHELL, true);

        for (Creature* pShard : shards)
        {
            if (pShard->GetScript() != nullptr)
                pShard->GetScript()->DoAction(ACTION_SKARMORAK_SHARD_FEED);
        }

        scriptEvents.addEvent(EVENT_SKARMORAK_VOID_DISCHARGE, 3000);
    }

    void dropShell(bool broken)
    {
        mShelled = false;
        mShellBroken = false;
        mEnergy = 100;
        mDrainMs = 0;

        getCreature()->interruptSpell();
        _removeAura(SPELL_SKARMORAK_FORTIFIED_SHELL);
        _removeAura(SPELL_SKARMORAK_VOID_DISCHARGE);
        scriptEvents.resetEvents();

        if (broken)
        {
            warn("Skarmorak's Fortified Shell shatters!");
            castSpellOnSelf(SPELL_SKARMORAK_SHATTERED_SHELL, true);
        }

        // the shards spend themselves on the shell
        for (Creature* pShard : livingSummons(NPC_SKARMORAK_CRYSTAL_SHARD))
        {
            if (pShard->GetScript() != nullptr)
                pShard->GetScript()->DoAction(ACTION_SKARMORAK_SHARD_ERUPT);
        }

        scheduleAssault(10000, 16100);
    }

    uint32_t mEnergy = 100;
    uint32_t mDrainMs = 0;
    bool mShelled = false;
    bool mShellBroken = false;
    uint32_t mShellMs = 0;
    uint32_t mShellLeft = 0;
    LocationVector mSmashSpot;
};

class StonevaultCrystalShardAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonevaultCrystalShardAI(c); }
    explicit StonevaultCrystalShardAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void DoAction(int32_t action) override
    {
        if (!isAlive() || mSpent)
            return;

        if (action == ACTION_SKARMORAK_SHARD_FEED)
        {
            if (Creature* pSkarmorak = findNearestCreature(BOSS_SKARMORAK, 100.0f))
                castSpell(pSkarmorak, SPELL_SKARMORAK_SHARD_SHELL, true);
        }
        else if (action == ACTION_SKARMORAK_SHARD_ERUPT)
        {
            mSpent = true;
            castSpellAOE(SPELL_SKARMORAK_SHARD_ERUPTION, true);
            despawn(500, 0);
        }
    }

private:
    bool mSpent = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Void Speaker Eirich

class StonevaultEirichAI : public StonevaultBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonevaultEirichAI(c); }
    explicit StonevaultEirichAI(Creature* pCreature) : StonevaultBossAI(pCreature, DATA_VOID_SPEAKER_EIRICH) {}

protected:
    void onEngage() override
    {
        // two rifts torn open at the start
        for (uint8_t i = 0; i < 2; ++i)
            getCreature()->castSpellLoc(pointAround(getCreature()->GetPosition(), 10.0f, 16.0f), SPELL_EIRICH_VOID_RIFT, true);

        scriptEvents.addEvent(EVENT_EIRICH_VOID_CORRUPTION, 8000);
        scriptEvents.addEvent(EVENT_EIRICH_ENTROPIC_RECKONING, 14000);
        scriptEvents.addEvent(EVENT_EIRICH_UNBRIDLED_VOID, 22000);
    }

    void onReset() override { _removeAuraOnPlayers(SPELL_EIRICH_VOID_CORRUPTION); }

    void onDefeat() override { _removeAuraOnPlayers(SPELL_EIRICH_VOID_CORRUPTION); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_EIRICH_VOID_CORRUPTION:
                castAtRandomPlayer(SPELL_EIRICH_VOID_CORRUPTION);
                scriptEvents.addEvent(EVENT_EIRICH_CORRUPTION_DRAWN, CorruptionDrawnMs);
                scriptEvents.addEvent(EVENT_EIRICH_VOID_CORRUPTION, 20000);
                break;
            case EVENT_EIRICH_CORRUPTION_DRAWN:
                // the rifts pull the corruption back in
                _removeAuraOnPlayers(SPELL_EIRICH_VOID_CORRUPTION);
                break;
            case EVENT_EIRICH_ENTROPIC_RECKONING:
                castAtRandomPlayer(SPELL_EIRICH_ENTROPIC_RECKONING);
                scriptEvents.addEvent(EVENT_EIRICH_ENTROPIC_RECKONING, 22000);
                break;
            case EVENT_EIRICH_UNBRIDLED_VOID:
                if (castAtRandomPlayer(SPELL_EIRICH_UNBRIDLED_VOID) != nullptr)
                    warn("Void Speaker Eirich unleashes Unbridled Void!");
                scriptEvents.addEvent(EVENT_EIRICH_UNBRIDLED_VOID, 26000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Master Machinists, the survivor becomes a Silenced Speaker

class StonevaultMachinistAI : public StonevaultBossAI
{
public:
    StonevaultMachinistAI(Creature* pCreature, uint32_t partnerEntry) : StonevaultBossAI(pCreature, DATA_MASTER_MACHINISTS), mPartnerEntry(partnerEntry) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_MACHINISTS_PARTNER_FELL && isAlive() && _isInCombat())
        {
            warn("The surviving speaker is empowered by grief!");
            castSpellOnSelf(SPELL_MACHINISTS_SILENCED_SPEAKER, true);
        }
    }

protected:
    void onDefeat() override
    {
        if (TheStonevaultInstanceScript* pStonevault = getStonevault())
            pStonevault->sendAction(mPartnerEntry, ACTION_MACHINISTS_PARTNER_FELL);
    }

    uint32_t mPartnerEntry;
};

class StonevaultBrokkAI : public StonevaultMachinistAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonevaultBrokkAI(c); }
    explicit StonevaultBrokkAI(Creature* pCreature) : StonevaultMachinistAI(pCreature, BOSS_SPEAKER_DORLITA) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_BROKK_MOLTEN_METAL, 6000);
        scriptEvents.addEvent(EVENT_BROKK_SCRAP_SONG, 16000);
        scriptEvents.addEvent(EVENT_BROKK_EXHAUST_VENTS, 26000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BROKK_MOLTEN_METAL:
                castAtRandomPlayer(SPELL_BROKK_MOLTEN_METAL);
                scriptEvents.addEvent(EVENT_BROKK_MOLTEN_METAL, 14000);
                break;
            case EVENT_BROKK_SCRAP_SONG:
                warn("Speaker Brokk sings a Scrap Song!");
                castSpellAOE(SPELL_BROKK_SCRAP_SONG);
                scriptEvents.addEvent(EVENT_BROKK_SCRAP_CUBE, 300);
                scriptEvents.addEvent(EVENT_BROKK_SCRAP_SONG, 36000);
                break;
            case EVENT_BROKK_SCRAP_CUBE:
                castAtRandomPlayer(SPELL_BROKK_SCRAP_CUBE, false, true);
                break;
            case EVENT_BROKK_EXHAUST_VENTS:
                warn("Speaker Brokk opens the Exhaust Vents!");
                for (Unit* pPlayer : playersNear(getCreature(), getCreature()->GetPosition(), FightReach))
                    castSpell(pPlayer, SPELL_BROKK_EXHAUST_VENTS, true);
                scriptEvents.addEvent(EVENT_BROKK_EXHAUST_VENTS, 36000);
                break;
            default:
                break;
        }
    }
};

class StonevaultDorlitaAI : public StonevaultMachinistAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new StonevaultDorlitaAI(c); }
    explicit StonevaultDorlitaAI(Creature* pCreature) : StonevaultMachinistAI(pCreature, BOSS_SPEAKER_BROKK) {}

protected:
    void onEngage() override
    {
        mCannonTarget = 0;
        scriptEvents.addEvent(EVENT_DORLITA_IGNEOUS_HAMMER, 5000);
        scriptEvents.addEvent(EVENT_DORLITA_LAVA_CANNON, 9000);
        scriptEvents.addEvent(EVENT_DORLITA_BLAZING_CRESCENDO, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DORLITA_IGNEOUS_HAMMER:
                castSpellOnVictim(SPELL_DORLITA_IGNEOUS_HAMMER);
                scriptEvents.addEvent(EVENT_DORLITA_IGNEOUS_HAMMER, 12000);
                break;
            case EVENT_DORLITA_LAVA_CANNON:
                if (Unit* pTarget = castAtRandomPlayer(SPELL_DORLITA_LAVA_CANNON))
                {
                    mCannonTarget = pTarget->getGuid();
                    scriptEvents.addEvent(EVENT_DORLITA_CANNON_HIT, 300);
                }
                scriptEvents.addEvent(EVENT_DORLITA_LAVA_CANNON, 16000);
                break;
            case EVENT_DORLITA_CANNON_HIT:
            {
                Unit* pTarget = getCreature()->getWorldMapUnit(mCannonTarget);
                if (pTarget != nullptr && pTarget->isAlive())
                    castSpell(pTarget, SPELL_DORLITA_LAVA_CANNON_HIT, true);
                break;
            }
            case EVENT_DORLITA_BLAZING_CRESCENDO:
                warn("Speaker Dorlita builds to a Blazing Crescendo!");
                castSpellAOE(SPELL_DORLITA_BLAZING_CRESCENDO);
                scriptEvents.addEvent(EVENT_DORLITA_CRESCENDO_BLAST, 300);
                scriptEvents.addEvent(EVENT_DORLITA_BLAZING_CRESCENDO, 40000);
                break;
            case EVENT_DORLITA_CRESCENDO_BLAST:
                castSpellAOE(SPELL_DORLITA_CRESCENDO_BLAST, true);
                break;
            default:
                break;
        }
    }

private:
    uint64_t mCannonTarget = 0;
};

void SetupTheStonevault(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_STONEVAULT, &TheStonevaultInstanceScript::Create);

    mgr->register_creature_script(BOSS_EDNA, &StonevaultEdnaAI::Create);
    mgr->register_creature_script(NPC_EDNA_VOLATILE_SPIKE, &StonevaultVolatileSpikeAI::Create);

    mgr->register_creature_script(BOSS_SKARMORAK, &StonevaultSkarmorakAI::Create);
    mgr->register_creature_script(NPC_SKARMORAK_CRYSTAL_SHARD, &StonevaultCrystalShardAI::Create);

    mgr->register_creature_script(BOSS_VOID_SPEAKER_EIRICH, &StonevaultEirichAI::Create);

    mgr->register_creature_script(BOSS_SPEAKER_BROKK, &StonevaultBrokkAI::Create);
    mgr->register_creature_script(BOSS_SPEAKER_DORLITA, &StonevaultDorlitaAI::Create);
}
