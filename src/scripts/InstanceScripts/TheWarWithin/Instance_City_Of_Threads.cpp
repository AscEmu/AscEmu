/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_City_Of_Threads.hpp"

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

enum CityOfThreadsEvents
{
    EVENT_COAGLAMATION_OOZING_SMASH = 1,
    EVENT_COAGLAMATION_VISCOUS_DARKNESS,
    EVENT_COAGLAMATION_BLOOD_SURGE,
    EVENT_COAGLAMATION_DARK_PULSE,

    EVENT_KRIXVIZK_SUBJUGATE,
    EVENT_KRIXVIZK_TERRORIZE,

    EVENT_FANGS_KNIFE_THROW,
    EVENT_NX_SHADE_SLASH,
    EVENT_NX_DUSKBRINGER,
    EVENT_VX_RIME_DAGGER,
    EVENT_VX_RIME_DAGGER_BURST,
    EVENT_VX_FREEZING_CHECK,
    EVENT_VX_ICE_SICKLES,
    EVENT_VX_SICKLES_HIT,

    EVENT_IZO_SHIFTING_ANOMALIES,
    EVENT_IZO_SPLICE,
    EVENT_IZO_TREMOR_SLAM,
    EVENT_IZO_SCARABS_EMERGE,
    EVENT_IZO_UMBRAL_WEAVE,
    EVENT_IZO_WEAVE_BIND,
    EVENT_IZO_ELIMINATION_BLAST,

    EVENT_IZO_SCARAB_GORGE
};

namespace
{
    float const TwoPi = 6.28318531f;
    float const FightReach = 60.0f;

    uint32_t const FangEntries[] = { BOSS_FANG_NX, BOSS_FANG_VX };

    // Orator Krix'vizk
    uint32_t const KrixVizkEnergyPerSecond = 4;
    uint32_t const IndoctrinationHoldMs = 6000;
    uint8_t const IndoctrinationPulses = 5;

    // Fangs of the Queen
    uint32_t const FangsEnergyPerSecond = 3;
    uint32_t const FangsLandingMs = 1500;
    uint32_t const ShadeEchoDelayMs = 3000;
    float const ShadeEchoRange = 6.0f;
    float const FreezingBloodWarmRange = 5.0f;

    // Izo, the Grand Splicer
    uint32_t const IzoEnergyPerSecond = 2;
    float const WebHoldRange = 4.0f;

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

    // energy kept by the script, the bar is full at 100
    struct EnergyBar
    {
        uint32_t value = 0;
        uint32_t clockMs = 0;

        void reset()
        {
            value = 0;
            clockMs = 0;
        }

        bool fill(uint32_t diff, uint32_t perSecond)
        {
            clockMs += diff;
            while (clockMs >= 1000)
            {
                clockMs -= 1000;
                value = value + perSecond >= 100 ? 100 : value + perSecond;
            }

            return value >= 100;
        }
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class CityOfThreadsInstanceScript : public InstanceScript
{
public:
    explicit CityOfThreadsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(CityOfThreadsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new CityOfThreadsInstanceScript(pMapMgr); }

    // one fallen fang must not end the encounter while the other stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (uint32_t entry : FangEntries)
        {
            if (entry == pCreature->getEntry())
            {
                mMemberGuids[entry] = pCreature->GetNewGUID();
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
        if (dataId != DATA_FANGS_OF_THE_QUEEN)
            return group;

        for (uint32_t entry : FangEntries)
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

    // pulling one fang pulls the other
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
// Shared combat base: event clock and target helpers

class CityOfThreadsCombatAI : public CreatureAIScript
{
public:
    explicit CityOfThreadsCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    CityOfThreadsInstanceScript* getCity()
    {
        if (getCreature()->GetMapId() != MAP_CITY_OF_THREADS)
            return nullptr;

        return static_cast<CityOfThreadsInstanceScript*>(getInstanceScript());
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

    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    Unit* unitByGuid(uint64_t guid)
    {
        Unit* pUnit = guid != 0 ? getCreature()->getWorldMapUnit(guid) : nullptr;
        return pUnit != nullptr && pUnit->isAlive() ? pUnit : nullptr;
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class CityOfThreadsBossAI : public CityOfThreadsCombatAI
{
public:
    CityOfThreadsBossAI(Creature* pCreature, uint32_t dataId) : CityOfThreadsCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (CityOfThreadsInstanceScript* pCity = getCity())
            pCity->rallyGroup(mDataId, getCreature());

        CityOfThreadsCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                CityOfThreadsInstanceScript* pCity = getCity();
                if (pCity == nullptr || !pCity->isGroupFighting(mDataId, getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        CityOfThreadsCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            CityOfThreadsInstanceScript* pCity = getCity();
            if (pCity == nullptr || pCity->isGroupDown(mDataId, getCreature()))
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
// The Coaglamation

class CityOfThreadsCoaglamationAI : public CityOfThreadsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CityOfThreadsCoaglamationAI(c); }
    explicit CityOfThreadsCoaglamationAI(Creature* pCreature) : CityOfThreadsBossAI(pCreature, DATA_THE_COAGLAMATION) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_COAGLAMATION_OOZING_SMASH, 6000);
        scriptEvents.addEvent(EVENT_COAGLAMATION_VISCOUS_DARKNESS, 12000);
        scriptEvents.addEvent(EVENT_COAGLAMATION_BLOOD_SURGE, 16000);
        scriptEvents.addEvent(EVENT_COAGLAMATION_DARK_PULSE, 22000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_COAGLAMATION_OOZING_SMASH:
                castSpellOnVictim(SPELL_COAGLAMATION_OOZING_SMASH);
                scriptEvents.addEvent(EVENT_COAGLAMATION_OOZING_SMASH, 15000);
                break;
            case EVENT_COAGLAMATION_VISCOUS_DARKNESS:
                warn("The Coaglamation releases Viscous Darkness!");
                castSpellAOE(SPELL_COAGLAMATION_VISCOUS_DARKNESS);
                scriptEvents.addEvent(EVENT_COAGLAMATION_VISCOUS_DARKNESS, 25000);
                break;
            case EVENT_COAGLAMATION_BLOOD_SURGE:
                castSpellAOE(SPELL_COAGLAMATION_BLOOD_SURGE);
                scriptEvents.addEvent(EVENT_COAGLAMATION_BLOOD_SURGE, 22000);
                break;
            case EVENT_COAGLAMATION_DARK_PULSE:
                warn("The Coaglamation gathers a Dark Pulse!");
                castSpellAOE(SPELL_COAGLAMATION_DARK_PULSE);
                scriptEvents.addEvent(EVENT_COAGLAMATION_DARK_PULSE, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Orator Krix'vizk, Vociferous Indoctrination at full energy

class CityOfThreadsKrixVizkAI : public CityOfThreadsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CityOfThreadsKrixVizkAI(c); }
    explicit CityOfThreadsKrixVizkAI(Creature* pCreature) : CityOfThreadsBossAI(pCreature, DATA_ORATOR_KRIX_VIZK) {}

protected:
    void onEngage() override
    {
        mEnergy.reset();
        mHoldMs = 0;
        mPulsesLeft = 0;
        mPulseMs = 0;
        mSubjugateLongGap = true;
        mTerrorizeShortGap = true;

        castSpellOnSelf(SPELL_KRIXVIZK_CHAINS_OF_OPPRESSION, true);
        scriptEvents.addEvent(EVENT_KRIXVIZK_SUBJUGATE, 4500);
        scriptEvents.addEvent(EVENT_KRIXVIZK_TERRORIZE, 9400);
    }

    void onCombatTick(uint32_t diff) override
    {
        // the speech keeps hitting the room while it lasts
        if (mPulsesLeft > 0)
        {
            mPulseMs += diff;
            if (mPulseMs >= 1000)
            {
                mPulseMs -= 1000;
                --mPulsesLeft;
                castSpellAOE(SPELL_KRIXVIZK_INDOCTRINATION_PULSE, true);
            }
        }

        if (mHoldMs > 0)
        {
            mHoldMs = mHoldMs > diff ? mHoldMs - diff : 0;
            return;
        }

        if (!mEnergy.fill(diff, KrixVizkEnergyPerSecond) || _isCasting())
            return;

        mEnergy.reset();
        mHoldMs = IndoctrinationHoldMs;
        mPulsesLeft = IndoctrinationPulses;
        mPulseMs = 0;

        warn("Orator Krix'vizk begins a Vociferous Indoctrination!");
        castSpellOnSelf(SPELL_KRIXVIZK_VOCIFEROUS_INDOCTRINATION);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KRIXVIZK_SUBJUGATE:
                castSpellOnVictim(SPELL_KRIXVIZK_SUBJUGATE);
                scriptEvents.addEvent(EVENT_KRIXVIZK_SUBJUGATE, mSubjugateLongGap ? 17100 : 12000);
                mSubjugateLongGap = !mSubjugateLongGap;
                break;
            case EVENT_KRIXVIZK_TERRORIZE:
                castAtRandomPlayer(SPELL_KRIXVIZK_TERRORIZE);
                scriptEvents.addEvent(EVENT_KRIXVIZK_TERRORIZE, mTerrorizeShortGap ? 8100 : 21100);
                mTerrorizeShortGap = !mTerrorizeShortGap;
                break;
            default:
                break;
        }
    }

private:
    EnergyBar mEnergy;
    uint32_t mHoldMs = 0;
    uint8_t mPulsesLeft = 0;
    uint32_t mPulseMs = 0;
    bool mSubjugateLongGap = true;
    bool mTerrorizeShortGap = true;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fangs of the Queen, one fights up front while the other works from the side

class CityOfThreadsFangAI : public CityOfThreadsBossAI
{
public:
    CityOfThreadsFangAI(Creature* pCreature, uint32_t partnerEntry, bool startsInFront)
        : CityOfThreadsBossAI(pCreature, DATA_FANGS_OF_THE_QUEEN), mPartnerEntry(partnerEntry), mStartsInFront(startsInFront) {}

    void DoAction(int32_t action) override
    {
        if (!isAlive() || !_isInCombat())
            return;

        if (action == ACTION_FANGS_SYNERGIC_STEP)
        {
            // the partner has not moved yet, its spot is the destination
            if (Creature* pPartner = partner())
                stepTo(pPartner->GetPosition());
        }
        else if (action == ACTION_FANGS_PARTNER_FELL)
        {
            mPartnerFell = true;
            mLandingMs = 0;
            takeRole(true);
        }
    }

protected:
    void onEngage() override
    {
        mPartnerFell = false;
        mLandingMs = 0;
        castSpellOnSelf(SPELL_FANGS_TWIN_FANGS, true);
        takeRole(mStartsInFront);
    }

    void onReset() override
    {
        mLandingMs = 0;
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onDefeat() override
    {
        if (CityOfThreadsInstanceScript* pCity = getCity())
            pCity->sendAction(mPartnerEntry, ACTION_FANGS_PARTNER_FELL);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mLandingMs == 0)
            return;

        if (mLandingMs > diff)
        {
            mLandingMs -= diff;
            return;
        }

        mLandingMs = 0;
        takeRole(mPendingFront);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_FANGS_KNIFE_THROW)
            return;

        castAtRandomPlayer(SPELL_FANGS_KNIFE_THROW, true);
        scriptEvents.addEvent(EVENT_FANGS_KNIFE_THROW, 9000);
    }

    virtual void scheduleFront() {}
    virtual void scheduleSideline() {}

    Creature* partner()
    {
        CityOfThreadsInstanceScript* pCity = getCity();
        Creature* pPartner = pCity != nullptr ? pCity->getMember(mPartnerEntry) : nullptr;
        return pPartner != nullptr && pPartner->isAlive() ? pPartner : nullptr;
    }

    bool isLanding() const { return mLandingMs != 0; }

    // leap onto the partner's spot, the roles swap on landing
    void stepTo(LocationVector const& spot)
    {
        scriptEvents.resetEvents();
        setRooted(false);
        castSpellAOE(SPELL_FANGS_SYNERGIC_STEP, true);
        moveJump(spot, 20.0f, 8.0f);

        mPendingFront = !mFront;
        mLandingMs = FangsLandingMs;
    }

    void takeRole(bool front)
    {
        mFront = front;
        setRooted(!front);
        _setMeleeDisabled(!front);

        scriptEvents.resetEvents();
        if (front)
            scheduleFront();
        else
            scheduleSideline();
    }

    uint32_t mPartnerEntry;
    bool mStartsInFront;
    bool mFront = false;
    bool mPendingFront = false;
    bool mPartnerFell = false;
    uint32_t mLandingMs = 0;
};

// Nx keeps the shared energy and calls the Synergic Step
class CityOfThreadsNxAI : public CityOfThreadsFangAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CityOfThreadsNxAI(c); }
    explicit CityOfThreadsNxAI(Creature* pCreature) : CityOfThreadsFangAI(pCreature, BOSS_FANG_VX, true) {}

protected:
    void onEngage() override
    {
        mEnergy.reset();
        CityOfThreadsFangAI::onEngage();
    }

    void onCombatTick(uint32_t diff) override
    {
        CityOfThreadsFangAI::onCombatTick(diff);

        if (mPartnerFell || isLanding())
            return;

        if (!mEnergy.fill(diff, FangsEnergyPerSecond) || _isCasting())
            return;

        Creature* pVx = partner();
        if (pVx == nullptr)
            return;

        mEnergy.reset();
        LocationVector const vxSpot = pVx->GetPosition();

        warn("Nx and Vx exchange their power with a Synergic Step!");
        if (CityOfThreadsInstanceScript* pCity = getCity())
            pCity->sendAction(BOSS_FANG_VX, ACTION_FANGS_SYNERGIC_STEP);

        stepTo(vxSpot);
    }

    void scheduleFront() override
    {
        scriptEvents.addEvent(EVENT_NX_SHADE_SLASH, 5000);
        scriptEvents.addEvent(EVENT_NX_DUSKBRINGER, 15000);
    }

    void scheduleSideline() override
    {
        scriptEvents.addEvent(EVENT_FANGS_KNIFE_THROW, 4000);
        scriptEvents.addEvent(EVENT_NX_DUSKBRINGER, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NX_SHADE_SLASH:
                castSpellOnVictim(SPELL_NX_SHADE_SLASH);
                summonCreature(NPC_NX_ECHOING_SHADE, getCreature()->GetPosition(), TIMED_DESPAWN, ShadeEchoDelayMs + 2000);
                scriptEvents.addEvent(EVENT_NX_SHADE_SLASH, 10000);
                break;
            case EVENT_NX_DUSKBRINGER:
                warn("Nx calls the Duskbringer!");
                castSpellOnSelf(SPELL_NX_DUSKBRINGER);
                scriptEvents.addEvent(EVENT_NX_DUSKBRINGER, 30000);
                break;
            default:
                CityOfThreadsFangAI::onEvent(eventId);
                break;
        }
    }

private:
    EnergyBar mEnergy;
};

class CityOfThreadsVxAI : public CityOfThreadsFangAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CityOfThreadsVxAI(c); }
    explicit CityOfThreadsVxAI(Creature* pCreature) : CityOfThreadsFangAI(pCreature, BOSS_FANG_NX, false) {}

protected:
    void scheduleFront() override
    {
        scriptEvents.addEvent(EVENT_VX_RIME_DAGGER, 6000);
    }

    void scheduleSideline() override
    {
        scriptEvents.addEvent(EVENT_FANGS_KNIFE_THROW, 4000);
        scriptEvents.addEvent(EVENT_VX_ICE_SICKLES, 8000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VX_RIME_DAGGER:
                castSpellOnVictim(SPELL_VX_RIME_DAGGER);
                scriptEvents.addEvent(EVENT_VX_RIME_DAGGER_BURST, 300);
                scriptEvents.addEvent(EVENT_VX_RIME_DAGGER, 14000);
                break;
            case EVENT_VX_RIME_DAGGER_BURST:
                castSpellAOE(SPELL_VX_RIME_DAGGER_BURST, true);
                if (Unit* pTarget = castAtRandomPlayer(SPELL_VX_FREEZING_BLOOD, true, true))
                {
                    mFrozenTarget = pTarget->getGuid();
                    scriptEvents.addEvent(EVENT_VX_FREEZING_CHECK, 6000);
                }
                break;
            case EVENT_VX_FREEZING_CHECK:
                // no ally close enough to warm the target
                if (Unit* pTarget = unitByGuid(mFrozenTarget))
                {
                    if (playersNear(getCreature(), pTarget->GetPosition(), FreezingBloodWarmRange).size() <= 1)
                        castSpell(pTarget, SPELL_VX_FROZEN_SOLID, true);
                }
                break;
            case EVENT_VX_ICE_SICKLES:
                if (Unit* pTarget = castAtRandomPlayer(SPELL_VX_ICE_SICKLES))
                {
                    mSickleTarget = pTarget->getGuid();
                    scriptEvents.addEvent(EVENT_VX_SICKLES_HIT, 300);
                }
                scriptEvents.addEvent(EVENT_VX_ICE_SICKLES, 16000);
                break;
            case EVENT_VX_SICKLES_HIT:
                if (Unit* pTarget = unitByGuid(mSickleTarget))
                    castSpell(pTarget, SPELL_VX_ICE_SICKLES_HIT, true);
                break;
            default:
                CityOfThreadsFangAI::onEvent(eventId);
                break;
        }
    }

private:
    uint64_t mFrozenTarget = 0;
    uint64_t mSickleTarget = 0;
};

// repeats Nx's slash where it was left behind
class CityOfThreadsEchoingShadeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CityOfThreadsEchoingShadeAI(c); }
    explicit CityOfThreadsEchoingShadeAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive() || mDone)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mDelayMs > diff)
        {
            mDelayMs -= diff;
            return;
        }

        mDone = true;
        for (Unit* pPlayer : playersNear(getCreature(), getCreature()->GetPosition(), ShadeEchoRange))
            castSpell(pPlayer, SPELL_NX_SHADE_SLASH_ECHO, true);

        despawn(500, 0);
    }

private:
    uint32_t mDelayMs = ShadeEchoDelayMs;
    bool mDone = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Izo, the Grand Splicer, Process of Elimination at full energy

class CityOfThreadsIzoAI : public CityOfThreadsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CityOfThreadsIzoAI(c); }
    explicit CityOfThreadsIzoAI(Creature* pCreature) : CityOfThreadsBossAI(pCreature, DATA_IZO_THE_GRAND_SPLICER) {}

protected:
    void onEngage() override
    {
        mEnergy.reset();
        scriptEvents.addEvent(EVENT_IZO_SHIFTING_ANOMALIES, 3000);
        scriptEvents.addEvent(EVENT_IZO_SPLICE, 10000);
        scriptEvents.addEvent(EVENT_IZO_TREMOR_SLAM, 14000);
        scriptEvents.addEvent(EVENT_IZO_UMBRAL_WEAVE, 19000);
    }

    void onReset() override { _removeAuraOnPlayers(SPELL_IZO_UMBRAL_WEAVE_BIND); }

    void onDefeat() override { _removeAuraOnPlayers(SPELL_IZO_UMBRAL_WEAVE_BIND); }

    void onCombatTick(uint32_t diff) override
    {
        if (!mEnergy.fill(diff, IzoEnergyPerSecond) || _isCasting())
            return;

        mEnergy.reset();
        warn("Izo begins a Process of Elimination! Stay away from her target!");
        castSpellOnVictim(SPELL_IZO_PROCESS_OF_ELIMINATION);
        scriptEvents.addEvent(EVENT_IZO_ELIMINATION_BLAST, 300);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_IZO_SHIFTING_ANOMALIES:
                castSpellOnSelf(SPELL_IZO_SHIFTING_ANOMALIES);
                scriptEvents.addEvent(EVENT_IZO_SHIFTING_ANOMALIES, 45000);
                break;
            case EVENT_IZO_SPLICE:
                castSpellAOE(SPELL_IZO_SPLICE);
                scriptEvents.addEvent(EVENT_IZO_SPLICE, 20000);
                break;
            case EVENT_IZO_TREMOR_SLAM:
                castSpellAOE(SPELL_IZO_TREMOR_SLAM);
                scriptEvents.addEvent(EVENT_IZO_SCARABS_EMERGE, 300);
                scriptEvents.addEvent(EVENT_IZO_TREMOR_SLAM, 25000);
                break;
            case EVENT_IZO_SCARABS_EMERGE:
                // the slam wakes scarabs under the party
                for (Unit* pTarget : pickPlayers(2))
                    callAdd(NPC_IZO_RAVENOUS_SCARAB, pointAround(pTarget->GetPosition(), 3.0f, 6.0f));
                break;
            case EVENT_IZO_UMBRAL_WEAVE:
                castSpellAOE(SPELL_IZO_UMBRAL_WEAVE);
                scriptEvents.addEvent(EVENT_IZO_WEAVE_BIND, 300);
                scriptEvents.addEvent(EVENT_IZO_UMBRAL_WEAVE, 26000);
                break;
            case EVENT_IZO_WEAVE_BIND:
                for (Unit* pTarget : pickPlayers(2, true))
                {
                    castSpell(pTarget, SPELL_IZO_UMBRAL_WEAVE_BIND, true);
                    summonCreature(NPC_IZO_UMBRAL_WEB, pTarget->GetPosition(), CORPSE_DESPAWN);
                }
                break;
            case EVENT_IZO_ELIMINATION_BLAST:
                if (Unit* pTarget = victim())
                    getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_IZO_PROCESS_OF_ELIMINATION_BLAST, true);
                break;
            default:
                break;
        }
    }

private:
    EnergyBar mEnergy;
};

// cutting the web frees the players caught in it
class CityOfThreadsUmbralWebAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CityOfThreadsUmbralWebAI(c); }
    explicit CityOfThreadsUmbralWebAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        for (Unit* pPlayer : playersNear(getCreature(), getCreature()->GetPosition(), WebHoldRange))
            pPlayer->removeAllAurasById(SPELL_IZO_UMBRAL_WEAVE_BIND);
    }
};

class CityOfThreadsRavenousScarabAI : public CityOfThreadsCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CityOfThreadsRavenousScarabAI(c); }
    explicit CityOfThreadsRavenousScarabAI(Creature* pCreature) : CityOfThreadsCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_IZO_SCARAB_GORGE, 3000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_IZO_SCARAB_GORGE)
            return;

        castSpellOnVictim(SPELL_IZO_SCARAB_GORGE);
        scriptEvents.addEvent(EVENT_IZO_SCARAB_GORGE, 6000);
    }
};

void SetupCityOfThreads(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_CITY_OF_THREADS, &CityOfThreadsInstanceScript::Create);

    mgr->register_creature_script(BOSS_THE_COAGLAMATION, &CityOfThreadsCoaglamationAI::Create);

    mgr->register_creature_script(BOSS_ORATOR_KRIX_VIZK, &CityOfThreadsKrixVizkAI::Create);

    mgr->register_creature_script(BOSS_FANG_NX, &CityOfThreadsNxAI::Create);
    mgr->register_creature_script(BOSS_FANG_VX, &CityOfThreadsVxAI::Create);
    mgr->register_creature_script(NPC_NX_ECHOING_SHADE, &CityOfThreadsEchoingShadeAI::Create);

    mgr->register_creature_script(BOSS_IZO_THE_GRAND_SPLICER, &CityOfThreadsIzoAI::Create);
    mgr->register_creature_script(NPC_IZO_UMBRAL_WEB, &CityOfThreadsUmbralWebAI::Create);
    mgr->register_creature_script(NPC_IZO_RAVENOUS_SCARAB, &CityOfThreadsRavenousScarabAI::Create);
}
