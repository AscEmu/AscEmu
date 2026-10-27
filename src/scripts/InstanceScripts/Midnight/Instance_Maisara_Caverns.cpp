/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Maisara_Caverns.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <map>
#include <set>
#include <utility>
#include <vector>

enum MaisaraCavernsEvents
{
    EVENT_MUROJIN_BARRAGE = 1,
    EVENT_MUROJIN_FREEZING_TRAP,
    EVENT_MUROJIN_FLANKING_SPEAR,
    EVENT_MUROJIN_REVIVE_PET,

    EVENT_NEKRAXX_FETID_QUILLSTORM,
    EVENT_NEKRAXX_INFECTED_PINIONS,
    EVENT_NEKRAXX_CARRION_SWOOP,

    EVENT_VORDAZA_DRAIN_SOUL,
    EVENT_VORDAZA_UNMAKE,
    EVENT_VORDAZA_WREST_PHANTOMS,
    EVENT_VORDAZA_PHANTOMS_ARRIVE,
    EVENT_VORDAZA_NECROTIC_CONVERGENCE,
    EVENT_PHANTOM_SOULROT,

    EVENT_RAKTUL_SPIRITBREAKER,
    EVENT_RAKTUL_CRUSH_SOULS,
    EVENT_RAKTUL_RAISE_TOTEMS,
    EVENT_RAKTUL_DEATHGORGED_VESSEL,
    EVENT_RAKTUL_RESTLESS_MASSES,
    EVENT_TOTEM_SOULBIND,
    EVENT_LOST_SOUL_CRIES,
    EVENT_MALIGNANT_SOUL_SUFFERING
};

namespace
{
    float const MaisaraTwoPi = 6.28318531f;
    float const MaisaraFixateThreat = 1000000.0f;

    // the hunter and his bird share one encounter
    uint32_t const MaisaraPairEntries[2] = { BOSS_MUROJIN, BOSS_NEKRAXX };

    // room doors shut during the fight, passages open after the kill
    struct MaisaraDoor
    {
        uint32_t entry;
        uint32_t dataId;
        bool passage;
    };

    MaisaraDoor const MaisaraDoors[] =
    {
        { GO_MAISARA_BOSS_ONE_GATE, DATA_MUROJIN_AND_NEKRAXX, false }
    };

    MaisaraDoor const* findMaisaraDoor(uint32_t entry)
    {
        for (MaisaraDoor const& door : MaisaraDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    uint32_t const NekraxxReviveHealthPct = 50;
    uint32_t const ReviveRetryMs = 15000;

    uint32_t const PhantomsPerWave = 3;
    uint32_t const ConvergenceLimitMs = 20000;

    int32_t const RaktulRoarMarks[2] = { 70, 40 };
    uint32_t const RaktulRoarCount = 2;
    uint32_t const LostSoulsPerRoar = 3;
    uint32_t const MalignantSoulsPerRoar = 2;
    uint32_t const RestlessMassesLimitMs = 40000;
    size_t const CrushSoulsTargets = 2;

    LocationVector maisaraRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(MaisaraTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class MaisaraCavernsInstanceScript : public InstanceScript
{
public:
    explicit MaisaraCavernsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(MaisaraCavernsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new MaisaraCavernsInstanceScript(pMapMgr); }

    // the first of the pair to fall must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (uint32_t entry : MaisaraPairEntries)
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
        if (findMaisaraDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.insert(pGameObject->GetNewGUID());
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
        if (dataId != DATA_MUROJIN_AND_NEKRAXX)
            return group;

        for (uint32_t entry : MaisaraPairEntries)
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

    // pulling one of the pair pulls the other
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
    // the encounter decides, the spawn state of a door is not reliable
    void applyDoorState(GameObject* pDoor)
    {
        MaisaraDoor const* pRule = findMaisaraDoor(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        EncounterStates const state = getBossState(pRule->dataId);
        bool const open = pRule->passage ? state == EncounterStates::Performed : state != EncounterStates::InProgress;
        uint8_t const wanted = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::map<uint32_t, WoWGuid> mMemberGuids;
    std::set<WoWGuid> mDoorGuids;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class MaisaraCavernsCombatAI : public CreatureAIScript
{
public:
    explicit MaisaraCavernsCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    MaisaraCavernsInstanceScript* getCaverns()
    {
        if (getCreature()->GetMapId() != MAP_MAISARA_CAVERNS)
            return nullptr;

        return static_cast<MaisaraCavernsInstanceScript*>(getInstanceScript());
    }

    // distinct random players, the tank only when nobody else is left
    std::vector<Unit*> pickPlayers(size_t count, bool skipTank = false)
    {
        TargetFilter const filter = skipTank ? TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent) : TargetFilter_Player;
        FilterArgs const args(filter);

        std::vector<Unit*> pool;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject != nullptr && isValidUnitTarget(pObject, filter, args))
                pool.push_back(static_cast<Unit*>(pObject));
        }

        if (pool.empty() && skipTank)
            return pickPlayers(count, false);

        for (size_t i = 0; i < pool.size() && i < count; ++i)
        {
            size_t const pick = i + Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1 - i));
            std::swap(pool[i], pool[pick]);
        }

        if (pool.size() > count)
            pool.resize(count);

        return pool;
    }

    Unit* anyPlayer(bool skipTank = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    // ground targeted spell at the feet of a unit
    void castAtGround(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId, triggered);
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, durationMs);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange)
    {
        return maisaraRingPoint(getCreature()->GetPosition(), minRange, maxRange);
    }

    uint32_t countLivingSummons(uint32_t entry)
    {
        uint32_t count = 0;
        for (uint64_t const guid : summons)
        {
            Creature* pSummon = getCreature()->getWorldMapCreature(guid);
            if (pSummon != nullptr && pSummon->getEntry() == entry && pSummon->isAlive())
                ++count;
        }

        return count;
    }

    // the add chases one player until it dies
    void fixateRandomPlayer()
    {
        if (Unit* pTarget = anyPlayer())
            addThreat(pTarget, MaisaraFixateThreat);
    }

    void notify(uint32_t entry, int32_t action)
    {
        if (MaisaraCavernsInstanceScript* pCaverns = getCaverns())
            pCaverns->sendAction(entry, action);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class MaisaraCavernsBossAI : public MaisaraCavernsCombatAI
{
public:
    MaisaraCavernsBossAI(Creature* pCreature, uint32_t dataId) : MaisaraCavernsCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (MaisaraCavernsInstanceScript* pCaverns = getCaverns())
            pCaverns->rallyGroup(mDataId, getCreature());

        MaisaraCavernsCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                MaisaraCavernsInstanceScript* pCaverns = getCaverns();
                if (pCaverns == nullptr || !pCaverns->isGroupFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        MaisaraCavernsCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            MaisaraCavernsInstanceScript* pCaverns = getCaverns();
            if (pCaverns == nullptr || pCaverns->isGroupDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        onDefeat();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Muro'jin and Nekraxx

class MaisaraCavernsPairAI : public MaisaraCavernsBossAI
{
public:
    explicit MaisaraCavernsPairAI(Creature* pCreature) : MaisaraCavernsBossAI(pCreature, DATA_MUROJIN_AND_NEKRAXX) {}

    void OnCombatStart(Unit* pTarget) override
    {
        MaisaraCavernsBossAI::OnCombatStart(pTarget);
        callMissingPartner();
    }

protected:
    Creature* getPartner()
    {
        MaisaraCavernsInstanceScript* pCaverns = getCaverns();
        if (pCaverns == nullptr)
            return nullptr;

        return pCaverns->getMember(getCreature()->getEntry() == BOSS_MUROJIN ? BOSS_NEKRAXX : BOSS_MUROJIN);
    }

private:
    // spawn data with one of the pair alone: the other joins at its side
    void callMissingPartner()
    {
        if (getCaverns() == nullptr || getPartner() != nullptr)
            return;

        Creature* pPartner = spawnCreature(getCreature()->getEntry() == BOSS_MUROJIN ? BOSS_NEKRAXX : BOSS_MUROJIN, aroundSelf(3.0f, 5.0f));
        if (pPartner != nullptr)
            setZoneWideCombat(pPartner);
    }
};

class MaisaraCavernsMurojinAI : public MaisaraCavernsPairAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MaisaraCavernsMurojinAI(c); }
    explicit MaisaraCavernsMurojinAI(Creature* pCreature) : MaisaraCavernsPairAI(pCreature) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_MAISARA_PET_DOWN || !isAlive() || !_isInCombat())
            return;

        mPetDown = true;
        scriptEvents.addEvent(EVENT_MUROJIN_REVIVE_PET, 2000);
    }

    // a completed cast brings the bird back, an interrupted one is tried again
    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId != SPELL_MUROJIN_REVIVE_PET || !mPetDown)
            return;

        mPetDown = false;
        scriptEvents.removeEvent(EVENT_MUROJIN_REVIVE_PET);
        notify(BOSS_NEKRAXX, ACTION_MAISARA_PET_REVIVED);
    }

protected:
    void onEngage() override
    {
        mPetDown = false;
        scriptEvents.addEvent(EVENT_MUROJIN_FLANKING_SPEAR, 5000);
        scriptEvents.addEvent(EVENT_MUROJIN_BARRAGE, 9000);
        scriptEvents.addEvent(EVENT_MUROJIN_FREEZING_TRAP, 14000);
    }

    void onReset() override { mPetDown = false; }

    void onDefeat() override { notify(BOSS_NEKRAXX, ACTION_MAISARA_HUNTER_FELL); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MUROJIN_FLANKING_SPEAR:
                castSpellOnVictim(SPELL_MUROJIN_FLANKING_SPEAR);
                scriptEvents.addEvent(EVENT_MUROJIN_FLANKING_SPEAR, 16000);
                break;
            case EVENT_MUROJIN_BARRAGE:
                castAt(anyPlayer(true), SPELL_MUROJIN_BARRAGE);
                scriptEvents.addEvent(EVENT_MUROJIN_BARRAGE, 20000);
                break;
            case EVENT_MUROJIN_FREEZING_TRAP:
                castAtGround(anyPlayer(), SPELL_MUROJIN_FREEZING_TRAP);
                scriptEvents.addEvent(EVENT_MUROJIN_FREEZING_TRAP, 24000);
                break;
            case EVENT_MUROJIN_REVIVE_PET:
                if (!mPetDown)
                    break;
                warn("Muro'jin attempts to Revive Pet! Interrupt him!");
                castSpellOnSelf(SPELL_MUROJIN_REVIVE_PET);
                scriptEvents.addEvent(EVENT_MUROJIN_REVIVE_PET, ReviveRetryMs);
                break;
            default:
                break;
        }
    }

private:
    bool mPetDown = false;
};

// falls to the ground instead of dying while Muro'jin can still revive it
class MaisaraCavernsNekraxxAI : public MaisaraCavernsPairAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MaisaraCavernsNekraxxAI(c); }
    explicit MaisaraCavernsNekraxxAI(Creature* pCreature) : MaisaraCavernsPairAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mFinishing)
            return;

        if (mDowned)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health || !isHunterFighting())
            return;

        *damage = health > 1 ? health - 1 : 0;
        mCollapsePending = true;
    }

    void DoAction(int32_t action) override
    {
        if (!isAlive() || !_isInCombat())
            return;

        if (action == ACTION_MAISARA_PET_REVIVED)
        {
            if (mDowned)
                rise();
            return;
        }

        if (action != ACTION_MAISARA_HUNTER_FELL)
            return;

        // nobody is left to revive it
        if (mDowned || mCollapsePending)
        {
            mFinishing = true;
            getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
            return;
        }

        warn("Nekraxx flies into a Bestial Wrath!");
        castSpellOnSelf(SPELL_NEKRAXX_BESTIAL_WRATH, true);
    }

protected:
    void onEngage() override
    {
        mDowned = false;
        mCollapsePending = false;
        mFinishing = false;
        startRotation();
    }

    void onReset() override
    {
        if (mDowned)
            standUp();

        mDowned = false;
        mCollapsePending = false;
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mCollapsePending || mDowned)
            return;

        mCollapsePending = false;
        collapse();
    }

    void onEvent(uint32_t eventId) override
    {
        if (mDowned)
            return;

        switch (eventId)
        {
            case EVENT_NEKRAXX_FETID_QUILLSTORM:
                castSpellAOE(SPELL_NEKRAXX_FETID_QUILLSTORM);
                scriptEvents.addEvent(EVENT_NEKRAXX_FETID_QUILLSTORM, 22000);
                break;
            case EVENT_NEKRAXX_INFECTED_PINIONS:
                castSpellAOE(SPELL_NEKRAXX_INFECTED_PINIONS);
                scriptEvents.addEvent(EVENT_NEKRAXX_INFECTED_PINIONS, 26000);
                break;
            case EVENT_NEKRAXX_CARRION_SWOOP:
                warn("Nekraxx prepares a Carrion Swoop!");
                castAt(anyPlayer(true), SPELL_NEKRAXX_CARRION_SWOOP);
                scriptEvents.addEvent(EVENT_NEKRAXX_CARRION_SWOOP, 32000);
                break;
            default:
                break;
        }
    }

private:
    void startRotation()
    {
        scriptEvents.addEvent(EVENT_NEKRAXX_FETID_QUILLSTORM, 10000);
        scriptEvents.addEvent(EVENT_NEKRAXX_INFECTED_PINIONS, 16000);
        scriptEvents.addEvent(EVENT_NEKRAXX_CARRION_SWOOP, 22000);
    }

    bool isHunterFighting()
    {
        Creature* pHunter = getPartner();
        return pHunter != nullptr && pHunter->isAlive() && pHunter->isInCombat();
    }

    void collapse()
    {
        mDowned = true;
        getCreature()->interruptSpell();
        scriptEvents.resetEvents();
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->setStandState(STANDSTATE_DEAD);
        setRooted(true);
        _setMeleeDisabled(true);
        notify(BOSS_MUROJIN, ACTION_MAISARA_PET_DOWN);
    }

    void rise()
    {
        mDowned = false;
        standUp();
        getCreature()->setHealthPct(NekraxxReviveHealthPct);
        startRotation();
    }

    void standUp()
    {
        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->setStandState(STANDSTATE_STAND);
        setRooted(false);
        _setMeleeDisabled(false);
    }

    bool mDowned = false;
    bool mCollapsePending = false;
    bool mFinishing = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vordaza

class MaisaraCavernsVordazaAI : public MaisaraCavernsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MaisaraCavernsVordazaAI(c); }
    explicit MaisaraCavernsVordazaAI(Creature* pCreature) : MaisaraCavernsBossAI(pCreature, DATA_VORDAZA) {}

protected:
    void onEngage() override
    {
        mConvergenceLeftMs = 0;
        castSpellOnSelf(SPELL_VORDAZA_WITHERING_MIASMA, true);

        scriptEvents.addEvent(EVENT_VORDAZA_DRAIN_SOUL, 6000);
        scriptEvents.addEvent(EVENT_VORDAZA_UNMAKE, 12000);
        scriptEvents.addEvent(EVENT_VORDAZA_WREST_PHANTOMS, 18000);
        scriptEvents.addEvent(EVENT_VORDAZA_NECROTIC_CONVERGENCE, 45000);
    }

    void onReset() override { mConvergenceLeftMs = 0; }

    void onCombatTick(uint32_t diff) override
    {
        if (mConvergenceLeftMs == 0)
            return;

        // interrupted in time
        if (!_isCasting())
        {
            mConvergenceLeftMs = 0;
            _removeAura(SPELL_VORDAZA_DEATHSHROUD);
            return;
        }

        if (mConvergenceLeftMs > diff)
        {
            mConvergenceLeftMs -= diff;
            return;
        }

        // nobody stopped her, the gathered souls burst
        mConvergenceLeftMs = 0;
        getCreature()->interruptSpell();
        _removeAura(SPELL_VORDAZA_DEATHSHROUD);
        castSpellOnVictim(SPELL_VORDAZA_COALESCED_DEATH, true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VORDAZA_DRAIN_SOUL:
                castSpellOnVictim(SPELL_VORDAZA_DRAIN_SOUL);
                scriptEvents.addEvent(EVENT_VORDAZA_DRAIN_SOUL, 15000);
                break;
            case EVENT_VORDAZA_UNMAKE:
                castSpellAOE(SPELL_VORDAZA_UNMAKE);
                scriptEvents.addEvent(EVENT_VORDAZA_UNMAKE, 22000);
                break;
            case EVENT_VORDAZA_WREST_PHANTOMS:
                castSpellOnSelf(SPELL_VORDAZA_WREST_PHANTOMS);
                scriptEvents.addEvent(EVENT_VORDAZA_PHANTOMS_ARRIVE, 500);
                scriptEvents.addEvent(EVENT_VORDAZA_WREST_PHANTOMS, 40000);
                break;
            case EVENT_VORDAZA_PHANTOMS_ARRIVE:
                for (uint32_t i = 0; i < PhantomsPerWave; ++i)
                    callAdd(NPC_UNSTABLE_PHANTOM, aroundSelf(8.0f, 14.0f));
                break;
            case EVENT_VORDAZA_NECROTIC_CONVERGENCE:
                warn("Vordaza invokes a Necrotic Convergence! Break her Deathshroud and interrupt her!");
                castSpellOnSelf(SPELL_VORDAZA_DEATHSHROUD, true);
                castSpellOnSelf(SPELL_VORDAZA_NECROTIC_CONVERGENCE);
                mConvergenceLeftMs = ConvergenceLimitMs;
                scriptEvents.addEvent(EVENT_VORDAZA_NECROTIC_CONVERGENCE, 60000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mConvergenceLeftMs = 0;
};

class MaisaraCavernsUnstablePhantomAI : public MaisaraCavernsCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MaisaraCavernsUnstablePhantomAI(c); }
    explicit MaisaraCavernsUnstablePhantomAI(Creature* pCreature) : MaisaraCavernsCombatAI(pCreature) {}

    // the last breath spreads dread over the room
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mDreadReleased || *damage < getCreature()->getHealth())
            return;

        mDreadReleased = true;
        castSpellAOE(SPELL_PHANTOM_LINGERING_DREAD, true);
    }

protected:
    void onEngage() override
    {
        mDreadReleased = false;
        castSpellOnSelf(SPELL_PHANTOM_VEILED_PRESENCE, true);
        fixateRandomPlayer();
        scriptEvents.addEvent(EVENT_PHANTOM_SOULROT, 3000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_PHANTOM_SOULROT)
            return;

        castSpellOnVictim(SPELL_PHANTOM_SOULROT);
        scriptEvents.addEvent(EVENT_PHANTOM_SOULROT, 8000);
    }

private:
    bool mDreadReleased = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rak'tul, Vessel of Souls

class MaisaraCavernsRaktulAI : public MaisaraCavernsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MaisaraCavernsRaktulAI(c); }
    explicit MaisaraCavernsRaktulAI(Creature* pCreature) : MaisaraCavernsBossAI(pCreature, DATA_RAKTUL) {}

protected:
    void onEngage() override
    {
        mRoarsDone = 0;
        mMassesLeftMs = 0;
        mTotemSpots.clear();

        scriptEvents.addEvent(EVENT_RAKTUL_SPIRITBREAKER, 6000);
        scriptEvents.addEvent(EVENT_RAKTUL_CRUSH_SOULS, 12000);
        scriptEvents.addEvent(EVENT_RAKTUL_DEATHGORGED_VESSEL, 24000);
    }

    void onReset() override { releaseSouls(); }

    void onDefeat() override { releaseSouls(); }

    void onCombatTick(uint32_t diff) override
    {
        if (mRoarsDone < RaktulRoarCount && _getHealthPercent() <= RaktulRoarMarks[mRoarsDone] && !_isCasting())
        {
            ++mRoarsDone;
            warn("Rak'tul unleashes a Soulrending Roar!");
            castSpellAOE(SPELL_RAKTUL_SOULRENDING_ROAR);
            scriptEvents.addEvent(EVENT_RAKTUL_RESTLESS_MASSES, 500);
        }

        if (mMassesLeftMs == 0)
            return;

        // the masses fade once their malignant souls are gone
        if (mMassesLeftMs > diff && countLivingSummons(NPC_MALIGNANT_SOUL) > 0)
        {
            mMassesLeftMs -= diff;
            return;
        }

        releaseSouls();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RAKTUL_SPIRITBREAKER:
                castSpellOnVictim(SPELL_RAKTUL_SPIRITBREAKER);
                scriptEvents.addEvent(EVENT_RAKTUL_SPIRITBREAKER, 18000);
                break;
            case EVENT_RAKTUL_CRUSH_SOULS:
            {
                // totems rise where the crushed souls stood
                std::vector<Unit*> const targets = pickPlayers(CrushSoulsTargets);
                mTotemSpots.clear();
                for (Unit* pTarget : targets)
                    mTotemSpots.push_back(pTarget->GetPosition());

                if (!targets.empty())
                    castSpell(targets.front(), SPELL_RAKTUL_CRUSH_SOULS);

                scriptEvents.addEvent(EVENT_RAKTUL_RAISE_TOTEMS, 500);
                scriptEvents.addEvent(EVENT_RAKTUL_CRUSH_SOULS, 30000);
                break;
            }
            case EVENT_RAKTUL_RAISE_TOTEMS:
                for (LocationVector const& spot : mTotemSpots)
                    callAdd(NPC_SOULBIND_TOTEM, spot);
                mTotemSpots.clear();
                break;
            case EVENT_RAKTUL_DEATHGORGED_VESSEL:
                castSpellAOE(SPELL_RAKTUL_DEATHGORGED_VESSEL);
                scriptEvents.addEvent(EVENT_RAKTUL_DEATHGORGED_VESSEL, 35000);
                break;
            case EVENT_RAKTUL_RESTLESS_MASSES:
                callRestlessMasses();
                break;
            default:
                break;
        }
    }

private:
    // the roar shatters the totems and drags the Restless Masses in
    void callRestlessMasses()
    {
        summons.despawnEntry(NPC_SOULBIND_TOTEM);
        _castOnInrangePlayers(SPELL_RAKTUL_WITHERING_SOUL, true);

        for (uint32_t i = 0; i < LostSoulsPerRoar; ++i)
            callAdd(NPC_LOST_SOUL, aroundSelf(8.0f, 16.0f));

        for (uint32_t i = 0; i < MalignantSoulsPerRoar; ++i)
            callAdd(NPC_MALIGNANT_SOUL, aroundSelf(10.0f, 16.0f));

        mMassesLeftMs = RestlessMassesLimitMs;
    }

    void releaseSouls()
    {
        mMassesLeftMs = 0;
        summons.despawnEntry(NPC_LOST_SOUL);
        summons.despawnEntry(NPC_MALIGNANT_SOUL);
        _removeAuraOnPlayers(SPELL_RAKTUL_WITHERING_SOUL);
    }

    uint32_t mRoarsDone = 0;
    uint32_t mMassesLeftMs = 0;
    std::vector<LocationVector> mTotemSpots;
};

class MaisaraCavernsSoulbindTotemAI : public MaisaraCavernsCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MaisaraCavernsSoulbindTotemAI(c); }
    explicit MaisaraCavernsSoulbindTotemAI(Creature* pCreature) : MaisaraCavernsCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        castAt(anyPlayer(), SPELL_TOTEM_CHILL_OF_DEATH, true);
        scriptEvents.addEvent(EVENT_TOTEM_SOULBIND, 1000);
    }

    // a broken bind is woven again
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_TOTEM_SOULBIND)
            return;

        castAt(anyPlayer(), SPELL_TOTEM_SOULBIND);
        scriptEvents.addEvent(EVENT_TOTEM_SOULBIND, 6000);
    }
};

class MaisaraCavernsLostSoulAI : public MaisaraCavernsCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MaisaraCavernsLostSoulAI(c); }
    explicit MaisaraCavernsLostSoulAI(Creature* pCreature) : MaisaraCavernsCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_LOST_SOUL_CRIES, 2000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_LOST_SOUL_CRIES)
            return;

        castAt(anyPlayer(), SPELL_LOST_SOUL_CRIES_OF_THE_FALLEN);
        scriptEvents.addEvent(EVENT_LOST_SOUL_CRIES, 9000);
    }
};

class MaisaraCavernsMalignantSoulAI : public MaisaraCavernsCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MaisaraCavernsMalignantSoulAI(c); }
    explicit MaisaraCavernsMalignantSoulAI(Creature* pCreature) : MaisaraCavernsCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_MALIGNANT_SOUL_SUFFERING, 1500); }

    // channels until interrupted, then starts over
    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_MALIGNANT_SOUL_SUFFERING)
            return;

        castSpellOnSelf(SPELL_MALIGNANT_SOUL_ETERNAL_SUFFERING);
        scriptEvents.addEvent(EVENT_MALIGNANT_SOUL_SUFFERING, 6000);
    }
};

void SetupMaisaraCaverns(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MAISARA_CAVERNS, &MaisaraCavernsInstanceScript::Create);

    mgr->register_creature_script(BOSS_MUROJIN, &MaisaraCavernsMurojinAI::Create);
    mgr->register_creature_script(BOSS_NEKRAXX, &MaisaraCavernsNekraxxAI::Create);

    mgr->register_creature_script(BOSS_VORDAZA, &MaisaraCavernsVordazaAI::Create);
    mgr->register_creature_script(NPC_UNSTABLE_PHANTOM, &MaisaraCavernsUnstablePhantomAI::Create);

    mgr->register_creature_script(BOSS_RAKTUL, &MaisaraCavernsRaktulAI::Create);
    mgr->register_creature_script(NPC_SOULBIND_TOTEM, &MaisaraCavernsSoulbindTotemAI::Create);
    mgr->register_creature_script(NPC_LOST_SOUL, &MaisaraCavernsLostSoulAI::Create);
    mgr->register_creature_script(NPC_MALIGNANT_SOUL, &MaisaraCavernsMalignantSoulAI::Create);
}
