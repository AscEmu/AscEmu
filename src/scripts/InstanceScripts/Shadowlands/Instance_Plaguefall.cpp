/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Plaguefall.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <map>

enum PlaguefallEvents
{
    EVENT_GLOBGROG_SLIME_WAVE = 1,
    EVENT_GLOBGROG_PLAGUESTOMP,
    EVENT_GLOBGROG_BECKON_SLIME,

    EVENT_ICKUS_BURNING_STRAIN,
    EVENT_ICKUS_SLIME_INJECTION,
    EVENT_ICKUS_INJECTION_OOZE,
    EVENT_ICKUS_PESTILENCE_SURGE,
    EVENT_ICKUS_LUNGE_LANDING,
    EVENT_PESTILENCE_SLIME_VIRAL_GLOBS,
    EVENT_PESTILENCE_SLIME_INFECT,

    EVENT_DOMINA_CYTOTOXIC_SLASH,
    EVENT_DOMINA_VENOMBLADES,
    EVENT_DOMINA_SHADOW_AMBUSH,
    EVENT_DOMINA_BROOD_ASSASSINS,
    EVENT_BROOD_ASSASSIN_ASSASSINATE,

    EVENT_STRADAMA_PLAGUE_ROT,
    EVENT_STRADAMA_INFECTIOUS_RAIN,
    EVENT_STRADAMA_PLAGUE_CRASH,
    EVENT_STRADAMA_MALIGNANT_GROWTH,
    EVENT_STRADAMA_SUBMERGED_CRASH,
    EVENT_STRADAMA_EMERGE,
    EVENT_MALIGNANT_SPAWN_TOUCH
};

enum PlaguefallActions
{
    ACTION_MORSEL_BECKON = 1
};

namespace
{
    float const PlaguefallTwoPi = 6.28318531f;

    uint32_t const SlimyMorselEntries[3] = { NPC_GLOBGROG_SLIMY_MORSEL, NPC_GLOBGROG_SLIMY_MORSEL_ALT, NPC_GLOBGROG_SLIMY_MORSEL_ALT2 };
    uint32_t const MorselsPerBeckon = 3;
    float const MorselBeckonRange = 40.0f;
    float const MorselFeedingReach = 3.0f;

    int32_t const IckusLungeMarks[2] = { 66, 33 };
    uint32_t const IckusSlimesPerSurge = 2;
    uint32_t const PlagueBombFuseMs = 20000;
    uint32_t const EruptingOozeFuseMs = 10000;
    float const EruptingOozeStepReach = 2.0f;
    float const PestilenceSlimeFeedReach = 10.0f;

    uint32_t const BroodAssassinsPerCall = 2;

    int32_t const StradamaSinkMarks[2] = { 66, 33 };
    uint32_t const StradamaSubmergedMs = 20000;
    uint32_t const PlagueTentacleLifeMs = 6000;

    LocationVector plaguefallRing(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(PlaguefallTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // true once when the countdown runs out
    bool plaguefallCountdown(uint32_t& leftMs, uint32_t diff)
    {
        if (leftMs == 0)
            return false;

        if (leftMs > diff)
        {
            leftMs -= diff;
            return false;
        }

        leftMs = 0;
        return true;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class PlaguefallInstanceScript : public InstanceScript
{
public:
    explicit PlaguefallInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(PlaguefallEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new PlaguefallInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        if (pCreature->getEntry() == BOSS_DOCTOR_ICKUS)
            mTrackedGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
    }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

private:
    std::map<uint32_t, WoWGuid> mTrackedGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and target helpers

class PlaguefallCombatAI : public CreatureAIScript
{
public:
    explicit PlaguefallCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        onDisengage();
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
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    PlaguefallInstanceScript* getPlaguefall()
    {
        if (getCreature()->GetMapId() != MAP_PLAGUEFALL)
            return nullptr;

        return static_cast<PlaguefallInstanceScript*>(getInstanceScript());
    }

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank, the tank when alone
    Unit* randomNonTank()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_NotCurrent);
        if (Unit* pTarget = selectUnitTarget(args))
            return pTarget;

        return randomPlayer();
    }

    Unit* castAtRandom(uint32_t spellId, bool skipTank = false)
    {
        Unit* pTarget = skipTank ? randomNonTank() : randomPlayer();
        if (pTarget != nullptr)
            castSpell(pTarget, spellId);

        return pTarget;
    }

    // helper on the side of its summoner, it does not join the fight by itself
    Creature* placeHelper(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pHelper = summonCreature(entry, spot, despawnType, durationMs);
        if (pHelper != nullptr)
            pHelper->setFaction(getCreature()->getFactionTemplate());

        return pHelper;
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = placeHelper(entry, spot);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return plaguefallRing(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class PlaguefallBossAI : public PlaguefallCombatAI
{
public:
    PlaguefallBossAI(Creature* pCreature, uint32_t dataId) : PlaguefallCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (PlaguefallInstanceScript* pPlaguefall = getPlaguefall())
        {
            pPlaguefall->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pPlaguefall->setBossState(mDataId, EncounterStates::InProgress);
        }

        PlaguefallCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (PlaguefallInstanceScript* pPlaguefall = getPlaguefall())
            {
                pPlaguefall->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pPlaguefall->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        PlaguefallCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (PlaguefallInstanceScript* pPlaguefall = getPlaguefall())
        {
            pPlaguefall->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pPlaguefall->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Globgrog

class PlaguefallGlobgrogAI : public PlaguefallBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PlaguefallGlobgrogAI(c); }
    explicit PlaguefallGlobgrogAI(Creature* pCreature) : PlaguefallBossAI(pCreature, DATA_GLOBGROG) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GLOBGROG_SLIME_WAVE, 8000);
        scriptEvents.addEvent(EVENT_GLOBGROG_PLAGUESTOMP, 14000);
        // a full energy bar calls the slimes
        scriptEvents.addEvent(EVENT_GLOBGROG_BECKON_SLIME, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GLOBGROG_SLIME_WAVE:
                castSpellOnVictim(SPELL_GLOBGROG_SLIME_WAVE);
                scriptEvents.addEvent(EVENT_GLOBGROG_SLIME_WAVE, 18000);
                break;
            case EVENT_GLOBGROG_PLAGUESTOMP:
                castSpellAOE(SPELL_GLOBGROG_PLAGUESTOMP);
                scriptEvents.addEvent(EVENT_GLOBGROG_PLAGUESTOMP, 22000);
                break;
            case EVENT_GLOBGROG_BECKON_SLIME:
                warn("Globgrog beckons the Slimy Morsels!");
                castSpellOnSelf(SPELL_GLOBGROG_BECKON_SLIME);
                beckonMorsels();
                scriptEvents.addEvent(EVENT_GLOBGROG_BECKON_SLIME, 40000);
                break;
            default:
                break;
        }
    }

private:
    // morsels around answer first, the rest crawl out of the ooze
    void beckonMorsels()
    {
        uint32_t heeding = 0;
        for (uint32_t const entry : SlimyMorselEntries)
        {
            std::list<Creature*> morsels;
            GetCreatureListWithEntryInGrid(morsels, entry, MorselBeckonRange);
            for (Creature* pMorsel : morsels)
            {
                if (heeding >= MorselsPerBeckon)
                    break;

                if (!pMorsel->isAlive() || pMorsel->GetScript() == nullptr)
                    continue;

                pMorsel->GetScript()->DoAction(ACTION_MORSEL_BECKON);
                ++heeding;
            }
        }

        for (; heeding < MorselsPerBeckon; ++heeding)
            placeHelper(NPC_GLOBGROG_SLIMY_MORSEL, aroundSelf(18.0f, 25.0f));
    }
};

// crawls to Globgrog and is swallowed on arrival
class PlaguefallSlimyMorselAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PlaguefallSlimyMorselAI(c); }
    explicit PlaguefallSlimyMorselAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* pSummoner) override
    {
        if (pSummoner != nullptr && pSummoner->getEntry() == BOSS_GLOBGROG)
            heedBeckon(pSummoner->getGuid());
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_MORSEL_BECKON || mGlobgrogGuid != 0)
            return;

        if (Creature* pGlobgrog = findNearestCreature(BOSS_GLOBGROG, 80.0f))
            heedBeckon(pGlobgrog->getGuid());
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mGlobgrogGuid == 0 || !isAlive())
            return;

        if (!plaguefallCountdown(mStepLeftMs, static_cast<uint32_t>(time_passed)))
            return;

        mStepLeftMs = 1000;

        Creature* pGlobgrog = getCreature()->getWorldMapCreature(mGlobgrogGuid);
        if (pGlobgrog == nullptr || !pGlobgrog->isAlive() || !pGlobgrog->isInCombat())
        {
            mGlobgrogGuid = 0;
            stopMovement();
            return;
        }

        if (getCreature()->getDistance(pGlobgrog) > MorselFeedingReach)
        {
            moveTo(pGlobgrog->GetPositionX(), pGlobgrog->GetPositionY(), pGlobgrog->GetPositionZ(), false);
            return;
        }

        pGlobgrog->castSpell(getCreature(), SPELL_GLOBGROG_CONSUME_SLIME, true);
        mGlobgrogGuid = 0;
        despawn(500, 0);
    }

private:
    void heedBeckon(uint64_t globgrogGuid)
    {
        mGlobgrogGuid = globgrogGuid;
        mStepLeftMs = 1;
        setReactState(REACT_PASSIVE);
    }

    uint64_t mGlobgrogGuid = 0;
    uint32_t mStepLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Doctor Ickus

class PlaguefallDoctorIckusAI : public PlaguefallBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PlaguefallDoctorIckusAI(c); }
    explicit PlaguefallDoctorIckusAI(Creature* pCreature) : PlaguefallBossAI(pCreature, DATA_DOCTOR_ICKUS) {}

protected:
    void onEngage() override
    {
        mLunges = 0;
        mLunging = false;
        mInjectedGuid = 0;
        scriptEvents.addEvent(EVENT_ICKUS_BURNING_STRAIN, 6000);
        scriptEvents.addEvent(EVENT_ICKUS_SLIME_INJECTION, 10000);
        scriptEvents.addEvent(EVENT_ICKUS_PESTILENCE_SURGE, 20000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mLunging && mLunges < 2 && _getHealthPercent() <= IckusLungeMarks[mLunges])
            lunge();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ICKUS_BURNING_STRAIN:
                castAtRandom(SPELL_ICKUS_BURNING_STRAIN);
                scriptEvents.addEvent(EVENT_ICKUS_BURNING_STRAIN, 18000);
                break;
            case EVENT_ICKUS_SLIME_INJECTION:
                if (Unit* pTarget = castAtRandom(SPELL_ICKUS_SLIME_INJECTION, true))
                {
                    mInjectedGuid = pTarget->getGuid();
                    scriptEvents.addEvent(EVENT_ICKUS_INJECTION_OOZE, 6000);
                }
                scriptEvents.addEvent(EVENT_ICKUS_SLIME_INJECTION, 20000);
                break;
            case EVENT_ICKUS_INJECTION_OOZE:
                releaseOoze();
                break;
            case EVENT_ICKUS_PESTILENCE_SURGE:
                warn("Doctor Ickus releases a Pestilence Surge!");
                castSpellAOE(isHeroic() ? SPELL_ICKUS_PESTILENCE_SURGE_HEROIC : SPELL_ICKUS_PESTILENCE_SURGE);
                for (uint32_t i = 0; i < IckusSlimesPerSurge; ++i)
                    callAdd(NPC_ICKUS_PESTILENCE_SLIME, aroundSelf(10.0f, 18.0f));
                scriptEvents.addEvent(EVENT_ICKUS_PESTILENCE_SURGE, 35000);
                break;
            case EVENT_ICKUS_LUNGE_LANDING:
                mLunging = false;
                getCreature()->castSpellLoc(mLungeSpot, SPELL_ICKUS_SLIME_LUNGE, true);
                placeHelper(NPC_ICKUS_PLAGUE_BOMB, plaguefallRing(mLungeSpot, 2.0f, 4.0f));
                break;
            default:
                break;
        }
    }

private:
    // jetpack leap into a slime pool, a bomb is set on landing
    void lunge()
    {
        ++mLunges;
        mLunging = true;
        warn("Doctor Ickus leaps away and plants a Plague Bomb!");
        getCreature()->interruptSpell();
        mLungeSpot = aroundSelf(15.0f, 25.0f);
        moveJump(mLungeSpot, 20.0f, 10.0f);
        scriptEvents.addEvent(EVENT_ICKUS_LUNGE_LANDING, 1500);
    }

    // the injection bursts out of its victim as an ooze
    void releaseOoze()
    {
        Unit* pVictim = mInjectedGuid != 0 ? getCreature()->getWorldMapUnit(mInjectedGuid) : nullptr;
        mInjectedGuid = 0;
        if (pVictim == nullptr || !pVictim->isAlive())
            return;

        placeHelper(NPC_ICKUS_ERUPTING_OOZE, plaguefallRing(pVictim->GetPosition(), 1.0f, 3.0f), TIMED_DESPAWN, EruptingOozeFuseMs + 5000);
    }

    uint32_t mLunges = 0;
    bool mLunging = false;
    uint64_t mInjectedGuid = 0;
    LocationVector mLungeSpot;
};

// detonates unless destroyed in time
class PlaguefallPlagueBombAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PlaguefallPlagueBombAI(c); }
    explicit PlaguefallPlagueBombAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        mFuseLeftMs = PlagueBombFuseMs;
    }

    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == SPELL_PLAGUE_BOMB_VIRULENT_EXPLOSION)
            despawn(500, 0);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (plaguefallCountdown(mFuseLeftMs, diff))
        {
            castSpellAOE(SPELL_PLAGUE_BOMB_VIRULENT_EXPLOSION);
            mVanishLeftMs = 8000;
        }

        if (plaguefallCountdown(mVanishLeftMs, diff))
            despawn(100, 0);
    }

private:
    uint32_t mFuseLeftMs = 0;
    uint32_t mVanishLeftMs = 0;
};

// bursts harmlessly when stepped on, erupts on the party otherwise
class PlaguefallEruptingOozeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PlaguefallEruptingOozeAI(c); }
    explicit PlaguefallEruptingOozeAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        mFuseLeftMs = EruptingOozeFuseMs;
        mCheckLeftMs = 500;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mSpent || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (plaguefallCountdown(mFuseLeftMs, diff))
        {
            mSpent = true;
            castSpellAOE(SPELL_ERUPTING_OOZE_ERUPT, true);
            despawn(500, 0);
            return;
        }

        if (!plaguefallCountdown(mCheckLeftMs, diff))
            return;

        mCheckLeftMs = 300;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer == nullptr || !pPlayer->isAlive() || getCreature()->getDistance(pPlayer) > EruptingOozeStepReach)
                continue;

            mSpent = true;
            castSpell(pPlayer, SPELL_ERUPTING_OOZE_BURSTING_OOZE, true);
            despawn(300, 0);
            return;
        }
    }

private:
    uint32_t mFuseLeftMs = 0;
    uint32_t mCheckLeftMs = 0;
    bool mSpent = false;
};

// speeds up the doctor while close to him
class PlaguefallPestilenceSlimeAI : public PlaguefallCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PlaguefallPestilenceSlimeAI(c); }
    explicit PlaguefallPestilenceSlimeAI(Creature* pCreature) : PlaguefallCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_PESTILENCE_SLIME_INFECT, 3000);
        scriptEvents.addEvent(EVENT_PESTILENCE_SLIME_VIRAL_GLOBS, 4000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PESTILENCE_SLIME_VIRAL_GLOBS:
                castSpellAOE(SPELL_PESTILENCE_SLIME_VIRAL_GLOBS);
                scriptEvents.addEvent(EVENT_PESTILENCE_SLIME_VIRAL_GLOBS, 10000);
                break;
            case EVENT_PESTILENCE_SLIME_INFECT:
                infectDoctor();
                scriptEvents.addEvent(EVENT_PESTILENCE_SLIME_INFECT, 5000);
                break;
            default:
                break;
        }
    }

private:
    void infectDoctor()
    {
        PlaguefallInstanceScript* pPlaguefall = getPlaguefall();
        Creature* pIckus = pPlaguefall != nullptr ? pPlaguefall->getTracked(BOSS_DOCTOR_ICKUS) : nullptr;
        if (pIckus == nullptr || !pIckus->isAlive() || !pIckus->isInCombat())
            return;

        if (getCreature()->getDistance(pIckus) <= PestilenceSlimeFeedReach)
            pIckus->castSpell(pIckus, SPELL_PESTILENCE_SLIME_RAPID_INFECTION, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Domina Venomblade

class PlaguefallDominaVenombladeAI : public PlaguefallBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PlaguefallDominaVenombladeAI(c); }
    explicit PlaguefallDominaVenombladeAI(Creature* pCreature) : PlaguefallBossAI(pCreature, DATA_DOMINA_VENOMBLADE) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_DOMINA_VENOMBLADES, true);
        scriptEvents.addEvent(EVENT_DOMINA_CYTOTOXIC_SLASH, 5000);
        scriptEvents.addEvent(EVENT_DOMINA_SHADOW_AMBUSH, 14000);
        // a full energy bar calls the brood
        scriptEvents.addEvent(EVENT_DOMINA_BROOD_ASSASSINS, 25000);
        scriptEvents.addEvent(EVENT_DOMINA_VENOMBLADES, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DOMINA_CYTOTOXIC_SLASH:
                castSpellOnVictim(SPELL_DOMINA_CYTOTOXIC_SLASH);
                scriptEvents.addEvent(EVENT_DOMINA_CYTOTOXIC_SLASH, 12000);
                break;
            case EVENT_DOMINA_SHADOW_AMBUSH:
                if (Unit* pTarget = randomNonTank())
                {
                    warn("Domina Venomblade vanishes for a Shadow Ambush!");
                    castSpell(pTarget, SPELL_DOMINA_SHADOW_AMBUSH);
                }
                scriptEvents.addEvent(EVENT_DOMINA_SHADOW_AMBUSH, 24000);
                break;
            case EVENT_DOMINA_BROOD_ASSASSINS:
                warn("Domina Venomblade calls her Brood Assassins out of the webs!");
                for (uint32_t i = 0; i < BroodAssassinsPerCall; ++i)
                    callAdd(NPC_DOMINA_BROOD_ASSASSIN, aroundSelf(10.0f, 18.0f));
                scriptEvents.addEvent(EVENT_DOMINA_BROOD_ASSASSINS, 45000);
                break;
            case EVENT_DOMINA_VENOMBLADES:
                castSpellOnSelf(SPELL_DOMINA_VENOMBLADES, true);
                scriptEvents.addEvent(EVENT_DOMINA_VENOMBLADES, 30000);
                break;
            default:
                break;
        }
    }
};

// assassinates from the webs until the first hit reveals it
class PlaguefallBroodAssassinAI : public PlaguefallCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PlaguefallBroodAssassinAI(c); }
    explicit PlaguefallBroodAssassinAI(Creature* pCreature) : PlaguefallCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (mConcealed && damage != nullptr && *damage > 0)
            reveal();
    }

protected:
    void onEngage() override
    {
        mConcealed = true;
        castSpellOnSelf(SPELL_BROOD_ASSASSIN_SHROUDWEB, true);
        setRooted(true);
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_BROOD_ASSASSIN_ASSASSINATE, 2000);
    }

    void onDisengage() override
    {
        mConcealed = false;
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_BROOD_ASSASSIN_ASSASSINATE || !mConcealed)
            return;

        castAtRandom(SPELL_BROOD_ASSASSIN_ASSASSINATE);
        scriptEvents.addEvent(EVENT_BROOD_ASSASSIN_ASSASSINATE, 8000);
    }

private:
    void reveal()
    {
        mConcealed = false;
        scriptEvents.removeEvent(EVENT_BROOD_ASSASSIN_ASSASSINATE);
        _removeAura(SPELL_BROOD_ASSASSIN_SHROUDWEB);
        setRooted(false);
        _setMeleeDisabled(false);
    }

    bool mConcealed = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Margrave Stradama

class PlaguefallMargraveStradamaAI : public PlaguefallBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PlaguefallMargraveStradamaAI(c); }
    explicit PlaguefallMargraveStradamaAI(Creature* pCreature) : PlaguefallBossAI(pCreature, DATA_MARGRAVE_STRADAMA) {}

protected:
    void onEngage() override
    {
        mSinks = 0;
        mSubmerged = false;
        startSurfaceEvents();
    }

    void onDisengage() override
    {
        if (mSubmerged)
            surface();
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mSubmerged && mSinks < 2 && _getHealthPercent() <= StradamaSinkMarks[mSinks])
            submerge();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_STRADAMA_PLAGUE_ROT:
                castSpellAOE(SPELL_STRADAMA_PLAGUE_ROT);
                scriptEvents.addEvent(EVENT_STRADAMA_PLAGUE_ROT, 20000);
                break;
            case EVENT_STRADAMA_INFECTIOUS_RAIN:
                castSpellOnSelf(SPELL_STRADAMA_INFECTIOUS_RAIN);
                scriptEvents.addEvent(EVENT_STRADAMA_INFECTIOUS_RAIN, 25000);
                break;
            case EVENT_STRADAMA_PLAGUE_CRASH:
                raiseTentacles(1);
                scriptEvents.addEvent(EVENT_STRADAMA_PLAGUE_CRASH, 15000);
                break;
            case EVENT_STRADAMA_MALIGNANT_GROWTH:
                castSpellOnSelf(SPELL_STRADAMA_MALIGNANT_GROWTH);
                callAdd(NPC_STRADAMA_MALIGNANT_SPAWN, aroundSelf(6.0f, 12.0f));
                scriptEvents.addEvent(EVENT_STRADAMA_MALIGNANT_GROWTH, 30000);
                break;
            case EVENT_STRADAMA_SUBMERGED_CRASH:
                raiseTentacles(2);
                scriptEvents.addEvent(EVENT_STRADAMA_SUBMERGED_CRASH, 4000);
                break;
            case EVENT_STRADAMA_EMERGE:
                surface();
                warn("Margrave Stradama rises from the ooze!");
                startSurfaceEvents();
                break;
            default:
                break;
        }
    }

private:
    void startSurfaceEvents()
    {
        scriptEvents.addEvent(EVENT_STRADAMA_PLAGUE_ROT, 4000);
        scriptEvents.addEvent(EVENT_STRADAMA_INFECTIOUS_RAIN, 8000);
        scriptEvents.addEvent(EVENT_STRADAMA_PLAGUE_CRASH, 12000);
        scriptEvents.addEvent(EVENT_STRADAMA_MALIGNANT_GROWTH, 18000);
    }

    // sinks below the ooze while the tentacles crash down
    void submerge()
    {
        ++mSinks;
        mSubmerged = true;
        warn("Margrave Stradama sinks beneath the ooze!");
        getCreature()->interruptSpell();
        scriptEvents.resetEvents();

        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(true);
        _setMeleeDisabled(true);

        scriptEvents.addEvent(EVENT_STRADAMA_SUBMERGED_CRASH, 2000);
        scriptEvents.addEvent(EVENT_STRADAMA_EMERGE, StradamaSubmergedMs);
    }

    void surface()
    {
        mSubmerged = false;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(false);
        _setMeleeDisabled(false);
    }

    void raiseTentacles(uint32_t count)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            if (Unit* pTarget = randomPlayer())
                placeHelper(NPC_STRADAMA_PLAGUE_TENTACLE, pTarget->GetPosition(), TIMED_DESPAWN, PlagueTentacleLifeMs);
        }
    }

    uint32_t mSinks = 0;
    bool mSubmerged = false;
};

// rises under a player and slams down once
class PlaguefallPlagueTentacleAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PlaguefallPlagueTentacleAI(c); }
    explicit PlaguefallPlagueTentacleAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        mCrashLeftMs = 1500;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (plaguefallCountdown(mCrashLeftMs, static_cast<uint32_t>(time_passed)))
            castSpellAOE(SPELL_STRADAMA_PLAGUE_CRASH);
    }

private:
    uint32_t mCrashLeftMs = 0;
};

class PlaguefallMalignantSpawnAI : public PlaguefallCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PlaguefallMalignantSpawnAI(c); }
    explicit PlaguefallMalignantSpawnAI(Creature* pCreature) : PlaguefallCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_MALIGNANT_SPAWN_TOUCH, 4000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_MALIGNANT_SPAWN_TOUCH)
            return;

        castSpellOnSelf(SPELL_MALIGNANT_SPAWN_TOUCH_OF_SLIME);
        scriptEvents.addEvent(EVENT_MALIGNANT_SPAWN_TOUCH, 6000);
    }
};

void SetupPlaguefall(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_PLAGUEFALL, &PlaguefallInstanceScript::Create);

    // Globgrog
    mgr->register_creature_script(BOSS_GLOBGROG, &PlaguefallGlobgrogAI::Create);
    for (uint32_t const entry : SlimyMorselEntries)
        mgr->register_creature_script(entry, &PlaguefallSlimyMorselAI::Create);

    // Doctor Ickus
    mgr->register_creature_script(BOSS_DOCTOR_ICKUS, &PlaguefallDoctorIckusAI::Create);
    mgr->register_creature_script(NPC_ICKUS_PLAGUE_BOMB, &PlaguefallPlagueBombAI::Create);
    mgr->register_creature_script(NPC_ICKUS_ERUPTING_OOZE, &PlaguefallEruptingOozeAI::Create);
    mgr->register_creature_script(NPC_ICKUS_ERUPTING_OOZE_ALT, &PlaguefallEruptingOozeAI::Create);
    mgr->register_creature_script(NPC_ICKUS_PESTILENCE_SLIME, &PlaguefallPestilenceSlimeAI::Create);

    // Domina Venomblade
    mgr->register_creature_script(BOSS_DOMINA_VENOMBLADE, &PlaguefallDominaVenombladeAI::Create);
    mgr->register_creature_script(NPC_DOMINA_BROOD_ASSASSIN, &PlaguefallBroodAssassinAI::Create);

    // Margrave Stradama
    mgr->register_creature_script(BOSS_MARGRAVE_STRADAMA, &PlaguefallMargraveStradamaAI::Create);
    mgr->register_creature_script(NPC_STRADAMA_MALIGNANT_SPAWN, &PlaguefallMalignantSpawnAI::Create);
    mgr->register_creature_script(NPC_STRADAMA_PLAGUE_TENTACLE, &PlaguefallPlagueTentacleAI::Create);
}
