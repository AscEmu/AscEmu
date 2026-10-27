/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Altar_Of_Fangs.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

enum AltarOfFangsEvents
{
    EVENT_RAVI_HYDRASTRIKE = 1,
    EVENT_RAVI_TRIPLE_SHOT,
    EVENT_RAVI_REGURGITATE,
    EVENT_RAVI_RAVENOUS_STOMP,
    EVENT_RAVI_OUT_OF_ENERGY,

    EVENT_COIL_TAIL_SCYTHE,
    EVENT_COIL_SYNCHRONIZED_VENOM,
    EVENT_COIL_VINDICTIVE_ONSLAUGHT,
    EVENT_COIL_VENOM_JET,
    EVENT_COIL_TOXIC_BARRAGE,
    EVENT_COIL_DEATH_RATTLE,
    EVENT_WRITHE_TOXIC_ATROPHY,
    EVENT_WRITHE_UNDERMINING,

    EVENT_ZULJAN_CHOP_DOWN,
    EVENT_ZULJAN_AXEGRINDER,
    EVENT_ZULJAN_BONESLICER,
    EVENT_ZULJAN_RITUAL_OF_THE_FANG,
    EVENT_ZULJAN_RITUAL_DONE,
    EVENT_ZULJAN_BLOODLETTING
};

namespace
{
    float const AltarTwoPi = 6.28318531f;

    size_t const AltarRoomPlayers = 40;

    size_t const TripleShotTargets = 3;
    uint32_t const RaviEnergyMs = 40000;
    uint32_t const ScavengeMs = 20000;
    uint32_t const FeedingFrenzyMs = 25000;
    uint32_t const MealShieldGraceMs = 1500;
    uint32_t const CarrionBurstPulseMs = 3000;

    // Death Rattle is broken after this long, the coil uncoils
    uint32_t const RattleWindowMs = 15000;
    uint32_t const UncoiledLimitMs = 20000;
    uint32_t const WrithesPerUncoil = 2;

    LocationVector altarRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(AltarTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class AltarOfFangsInstanceScript : public InstanceScript
{
public:
    explicit AltarOfFangsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(AltarOfFangsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new AltarOfFangsInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        if (pCreature->getEntry() == BOSS_WRITHING_COIL)
            mCoilGuid = pCreature->GetNewGUID();
    }

    Creature* getCoil() { return getCreatureByGuid(mCoilGuid); }

private:
    WoWGuid mCoilGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class AltarOfFangsCombatAI : public CreatureAIScript
{
public:
    explicit AltarOfFangsCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    AltarOfFangsInstanceScript* getAltar()
    {
        if (getCreature()->GetMapId() != MAP_ALTAR_OF_FANGS)
            return nullptr;

        return static_cast<AltarOfFangsInstanceScript*>(getInstanceScript());
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
        return altarRingPoint(getCreature()->GetPosition(), minRange, maxRange);
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

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class AltarOfFangsBossAI : public AltarOfFangsCombatAI
{
public:
    AltarOfFangsBossAI(Creature* pCreature, uint32_t dataId) : AltarOfFangsCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        AltarOfFangsCombatAI::OnCombatStart(pTarget);
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

        AltarOfFangsCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        onDefeat();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rav'i

class AltarOfFangsRaviAI : public AltarOfFangsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AltarOfFangsRaviAI(c); }
    explicit AltarOfFangsRaviAI(Creature* pCreature) : AltarOfFangsBossAI(pCreature, DATA_RAVI) {}

protected:
    void onEngage() override
    {
        mFreshMeat = false;
        mMealSpell = 0;
        startRotation();
    }

    void onReset() override
    {
        if (mMealSpell != 0)
        {
            setRooted(false);
            _setMeleeDisabled(false);
        }

        mFreshMeat = false;
        mMealSpell = 0;
    }

    // eating until the shield breaks, every pulse feeds Carrion Burst
    void onCombatTick(uint32_t diff) override
    {
        if (mMealSpell == 0)
            return;

        mBurstClock += diff;
        if (mBurstClock >= CarrionBurstPulseMs)
        {
            mBurstClock = 0;
            for (Unit* pTarget : pickPlayers(AltarRoomPlayers))
                castAtGround(pTarget, SPELL_RAVI_CARRION_BURST, true);
        }

        mMealGraceMs = mMealGraceMs > diff ? mMealGraceMs - diff : 0;
        bool const shieldBroken = mMealGraceMs == 0 && !hasAura(mMealSpell);
        if (!shieldBroken && mMealLeftMs > diff)
        {
            mMealLeftMs -= diff;
            return;
        }

        finishMeal();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RAVI_HYDRASTRIKE:
                castSpellOnVictim(SPELL_RAVI_HYDRASTRIKE);
                scriptEvents.addEvent(EVENT_RAVI_HYDRASTRIKE, 9000);
                break;
            case EVENT_RAVI_TRIPLE_SHOT:
                for (Unit* pTarget : pickPlayers(TripleShotTargets))
                    castSpell(pTarget, SPELL_RAVI_TRIPLE_SHOT, true);
                scriptEvents.addEvent(EVENT_RAVI_TRIPLE_SHOT, 18000);
                break;
            case EVENT_RAVI_REGURGITATE:
                castAt(anyPlayer(true), SPELL_RAVI_REGURGITATE);
                scriptEvents.addEvent(EVENT_RAVI_REGURGITATE, 22000);
                break;
            case EVENT_RAVI_RAVENOUS_STOMP:
                // the stomp drops Fresh Meat, her next meal turns into a frenzy
                castAtGround(getCreature(), SPELL_RAVI_RAVENOUS_STOMP);
                castSpellOnSelf(SPELL_RAVI_SCENT_OF_BLOOD, true);
                mFreshMeat = true;
                scriptEvents.addEvent(EVENT_RAVI_RAVENOUS_STOMP, 35000);
                break;
            case EVENT_RAVI_OUT_OF_ENERGY:
                startMeal();
                break;
            default:
                break;
        }
    }

private:
    void startRotation()
    {
        scriptEvents.addEvent(EVENT_RAVI_HYDRASTRIKE, 5000);
        scriptEvents.addEvent(EVENT_RAVI_TRIPLE_SHOT, 10000);
        scriptEvents.addEvent(EVENT_RAVI_REGURGITATE, 14000);
        scriptEvents.addEvent(EVENT_RAVI_RAVENOUS_STOMP, 22000);
        scriptEvents.addEvent(EVENT_RAVI_OUT_OF_ENERGY, RaviEnergyMs);
    }

    void startMeal()
    {
        scriptEvents.resetEvents();

        bool const frenzy = mFreshMeat;
        mFreshMeat = false;
        _removeAura(SPELL_RAVI_SCENT_OF_BLOOD);

        mMealSpell = frenzy ? SPELL_RAVI_FEEDING_FRENZY : SPELL_RAVI_SSSCAVENGING;
        mMealLeftMs = frenzy ? FeedingFrenzyMs : ScavengeMs;
        mMealGraceMs = MealShieldGraceMs;
        mBurstClock = 0;

        warn(frenzy ? "Rav'i catches the scent of Fresh Meat and falls into a Feeding Frenzy!" : "Rav'i runs out of energy and begins Ssscavenging!");
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(mMealSpell, true);
    }

    void finishMeal()
    {
        _removeAura(mMealSpell);
        mMealSpell = 0;
        setRooted(false);
        _setMeleeDisabled(false);
        startRotation();
    }

    bool mFreshMeat = false;
    uint32_t mMealSpell = 0;
    uint32_t mMealLeftMs = 0;
    uint32_t mMealGraceMs = 0;
    uint32_t mBurstClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Writhing Coil

class AltarOfFangsWrithingCoilAI : public AltarOfFangsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AltarOfFangsWrithingCoilAI(c); }
    explicit AltarOfFangsWrithingCoilAI(Creature* pCreature) : AltarOfFangsBossAI(pCreature, DATA_WRITHING_COIL) {}

protected:
    void onEngage() override
    {
        mRattleLeftMs = 0;
        mUncoiledLeftMs = 0;
        mJetPending = false;
        startRotation();
    }

    void onReset() override
    {
        if (mUncoiledLeftMs != 0)
            gatherBody();

        mRattleLeftMs = 0;
        mUncoiledLeftMs = 0;
        mJetPending = false;
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mRattleLeftMs != 0)
        {
            // a broken rattle uncoils the beast at once
            if (!_isCasting())
            {
                uncoil();
                return;
            }

            if (mRattleLeftMs > diff)
            {
                mRattleLeftMs -= diff;
                return;
            }

            getCreature()->interruptSpell();
            uncoil();
            return;
        }

        if (mUncoiledLeftMs == 0)
            return;

        if (mUncoiledLeftMs > diff && countLivingSummons(NPC_UNCOILED_WRITHE) > 0)
        {
            mUncoiledLeftMs -= diff;
            return;
        }

        assimilate();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_COIL_TAIL_SCYTHE:
                castSpellOnVictim(SPELL_COIL_TAIL_SCYTHE);
                scriptEvents.addEvent(EVENT_COIL_TAIL_SCYTHE, 12000);
                break;
            case EVENT_COIL_SYNCHRONIZED_VENOM:
                castAt(anyPlayer(), SPELL_COIL_SYNCHRONIZED_VENOM);
                scriptEvents.addEvent(EVENT_COIL_SYNCHRONIZED_VENOM, 20000);
                break;
            case EVENT_COIL_VINDICTIVE_ONSLAUGHT:
                castSpellAOE(SPELL_COIL_VINDICTIVE_ONSLAUGHT, true);
                if (Unit* pTarget = anyPlayer(true))
                {
                    castSpell(pTarget, SPELL_COIL_BURROWING_CHARGE, true);
                    mJetSpot = pTarget->GetPosition();
                    mJetPending = true;
                    scriptEvents.addEvent(EVENT_COIL_VENOM_JET, 1500);
                }
                scriptEvents.addEvent(EVENT_COIL_VINDICTIVE_ONSLAUGHT, 32000);
                break;
            case EVENT_COIL_VENOM_JET:
                if (mJetPending)
                    getCreature()->castSpellLoc(mJetSpot, SPELL_COIL_VENOM_JET);
                mJetPending = false;
                break;
            case EVENT_COIL_TOXIC_BARRAGE:
                castSpellOnSelf(SPELL_COIL_TOXIC_BARRAGE, true);
                _castOnInrangePlayers(SPELL_COIL_TOXIC_ATROPHY, true);
                scriptEvents.addEvent(EVENT_COIL_TOXIC_BARRAGE, 50000);
                break;
            case EVENT_COIL_DEATH_RATTLE:
                warn("The Writhing Coil begins a Death Rattle!");
                castSpellOnSelf(SPELL_COIL_DEATH_RATTLE);
                mRattleLeftMs = RattleWindowMs;
                scriptEvents.addEvent(EVENT_COIL_DEATH_RATTLE, 50000);
                break;
            default:
                break;
        }
    }

private:
    void startRotation()
    {
        scriptEvents.addEvent(EVENT_COIL_TAIL_SCYTHE, 6000);
        scriptEvents.addEvent(EVENT_COIL_SYNCHRONIZED_VENOM, 14000);
        scriptEvents.addEvent(EVENT_COIL_VINDICTIVE_ONSLAUGHT, 20000);
        scriptEvents.addEvent(EVENT_COIL_TOXIC_BARRAGE, 26000);
        scriptEvents.addEvent(EVENT_COIL_DEATH_RATTLE, 35000);
    }

    // the body splits into serpents, their wounds stay with it
    void uncoil()
    {
        mRattleLeftMs = 0;
        mJetPending = false;
        scriptEvents.resetEvents();

        warn("The Writhing Coil Uncoils!");
        castSpellAOE(SPELL_COIL_UNCOIL, true);

        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(true);
        _setMeleeDisabled(true);

        for (uint32_t i = 0; i < WrithesPerUncoil; ++i)
            callAdd(NPC_UNCOILED_WRITHE, aroundSelf(4.0f, 8.0f));

        mUncoiledLeftMs = UncoiledLimitMs;
    }

    void assimilate()
    {
        mUncoiledLeftMs = 0;
        summons.despawnEntry(NPC_UNCOILED_WRITHE);
        gatherBody();
        startRotation();
    }

    void gatherBody()
    {
        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(false);
        _setMeleeDisabled(false);
    }

    uint32_t mRattleLeftMs = 0;
    uint32_t mUncoiledLeftMs = 0;
    bool mJetPending = false;
    LocationVector mJetSpot;
};

// damage taken by a serpent is taken from the coil as well
class AltarOfFangsUncoiledWritheAI : public AltarOfFangsCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AltarOfFangsUncoiledWritheAI(c); }
    explicit AltarOfFangsUncoiledWritheAI(Creature* pCreature) : AltarOfFangsCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || *damage == 0)
            return;

        AltarOfFangsInstanceScript* pAltar = getAltar();
        Creature* pCoil = pAltar != nullptr ? pAltar->getCoil() : nullptr;
        if (pCoil == nullptr || !pCoil->isAlive())
            return;

        uint32_t const dealt = std::min(*damage, getCreature()->getHealth());
        uint32_t const left = pCoil->getHealth();
        pCoil->setHealth(left > dealt ? left - dealt : 1);
    }

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_WRITHE_UNCOILED, true);
        scriptEvents.addEvent(EVENT_WRITHE_TOXIC_ATROPHY, 3000);
        scriptEvents.addEvent(EVENT_WRITHE_UNDERMINING, 6000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_WRITHE_TOXIC_ATROPHY:
                castAt(anyPlayer(), SPELL_COIL_TOXIC_ATROPHY);
                scriptEvents.addEvent(EVENT_WRITHE_TOXIC_ATROPHY, 12000);
                break;
            case EVENT_WRITHE_UNDERMINING:
                castAt(anyPlayer(), SPELL_WRITHE_UNDERMINING);
                scriptEvents.addEvent(EVENT_WRITHE_UNDERMINING, 9000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Zul'jan

class AltarOfFangsZuljanAI : public AltarOfFangsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AltarOfFangsZuljanAI(c); }
    explicit AltarOfFangsZuljanAI(Creature* pCreature) : AltarOfFangsBossAI(pCreature, DATA_ZULJAN) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ZULJAN_CHOP_DOWN, 5000);
        scriptEvents.addEvent(EVENT_ZULJAN_AXEGRINDER, 9000);
        scriptEvents.addEvent(EVENT_ZULJAN_BONESLICER, 15000);
        scriptEvents.addEvent(EVENT_ZULJAN_RITUAL_OF_THE_FANG, 20000);
    }

    void onReset() override { _removeAuraOnPlayers(SPELL_ZULJAN_RITUAL_VENOM); }

    void onDefeat() override { _removeAuraOnPlayers(SPELL_ZULJAN_RITUAL_VENOM); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ZULJAN_CHOP_DOWN:
                castSpellOnVictim(SPELL_ZULJAN_CHOP_DOWN);
                scriptEvents.addEvent(EVENT_ZULJAN_CHOP_DOWN, 11000);
                break;
            case EVENT_ZULJAN_AXEGRINDER:
                castSpellOnSelf(SPELL_ZULJAN_AXEGRINDER);
                scriptEvents.addEvent(EVENT_ZULJAN_AXEGRINDER, 20000);
                break;
            case EVENT_ZULJAN_BONESLICER:
                castAt(anyPlayer(), SPELL_ZULJAN_BONESLICER);
                scriptEvents.addEvent(EVENT_ZULJAN_BONESLICER, 24000);
                break;
            case EVENT_ZULJAN_RITUAL_OF_THE_FANG:
                warn("Zul'jan begins the Ritual of the Fang!");
                castSpellOnSelf(SPELL_ZULJAN_RITUAL_OF_THE_FANG);
                scriptEvents.addEvent(EVENT_ZULJAN_RITUAL_DONE, 500);
                scriptEvents.addEvent(EVENT_ZULJAN_RITUAL_OF_THE_FANG, 45000);
                break;
            case EVENT_ZULJAN_RITUAL_DONE:
                // the venoms empower the dagger and linger on everyone
                castSpellOnSelf(SPELL_ZULJAN_FANG_EMPOWERED, true);
                _castOnInrangePlayers(SPELL_ZULJAN_RITUAL_VENOM, true);
                scriptEvents.addEvent(EVENT_ZULJAN_BLOODLETTING, 12000);
                break;
            case EVENT_ZULJAN_BLOODLETTING:
                castSpellAOE(SPELL_ZULJAN_BLOODLETTING);
                _removeAuraOnPlayers(SPELL_ZULJAN_RITUAL_VENOM);
                _removeAura(SPELL_ZULJAN_FANG_EMPOWERED);
                break;
            default:
                break;
        }
    }
};

void SetupAltarOfFangs(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ALTAR_OF_FANGS, &AltarOfFangsInstanceScript::Create);

    mgr->register_creature_script(BOSS_RAVI, &AltarOfFangsRaviAI::Create);

    mgr->register_creature_script(BOSS_WRITHING_COIL, &AltarOfFangsWrithingCoilAI::Create);
    mgr->register_creature_script(NPC_UNCOILED_WRITHE, &AltarOfFangsUncoiledWritheAI::Create);
    mgr->register_creature_script(NPC_UNCOILED_WRITHE_ALT, &AltarOfFangsUncoiledWritheAI::Create);

    mgr->register_creature_script(BOSS_ZULJAN, &AltarOfFangsZuljanAI::Create);
}
