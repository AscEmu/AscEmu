/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Mists_Of_Tirna_Scithe.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <map>

enum MistsOfTirnaScitheEvents
{
    EVENT_MISTCALLER_DODGE_BALL = 1,
    EVENT_MISTCALLER_PATTY_CAKE,
    EVENT_MISTCALLER_FREEZE_TAG,
    EVENT_VULPIN_PICK_PREY,

    EVENT_TREDOVA_ACID_EXPULSION,
    EVENT_TREDOVA_MIND_LINK,
    EVENT_TREDOVA_MARKED_PREY,
    EVENT_TREDOVA_ACCELERATED_INCUBATION,
    EVENT_LARVA_HUNT,

    EVENT_MALOCH_SOUL_SHACKLE,
    EVENT_MALOCH_EMBRACE_DARKNESS,
    EVENT_MALOCH_SPIRIT_BOLT,
    EVENT_DROMAN_BEWILDERING_POLLEN,
    EVENT_DROMAN_TEARS_OF_THE_FOREST
};

enum MistsOfTirnaScitheActions
{
    ACTION_MISTCALLER_FOUND = 1,
    ACTION_MALOCH_DROMAN_FREED,
    ACTION_DROMAN_SHACKLED,
    ACTION_DROMAN_RELEASED
};

enum MistsOfTirnaScitheCreatureData
{
    CLONE_DATA_REAL_ONE = 1
};

namespace
{
    float const MistsTwoPi = 6.28318531f;

    int32_t const MistcallerGameMarks[3] = { 70, 40, 10 };
    uint32_t const MistcallerClonesPerGame = 4;
    uint32_t const ClonePenaltyMs = 12000;
    uint32_t const VulpinLifeMs = 20000;
    float const VulpinTagReach = 3.0f;

    int32_t const TredovaFeastMarks[2] = { 70, 40 };
    uint32_t const TredovaFeastMs = 20000;
    uint32_t const TredovaShieldSettleMs = 1000;
    uint32_t const TredovaLarvaePerBrood = 3;

    uint32_t const DromanEntries[3] = { NPC_DROMAN_OULFARRAN, NPC_DROMAN_OULFARRAN_ALT, NPC_DROMAN_OULFARRAN_ALT2 };
    // faction the shackle forces on Droman
    uint32_t const DromanShackledFaction = 16;
    uint32_t const DromanFreeFallbackFaction = 35;
    uint32_t const DromanBreakFreePct = 20;
    uint32_t const DromanWrathMs = 15000;

    LocationVector mistsRing(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(MistsTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // true once when the countdown runs out
    bool mistsCountdown(uint32_t& leftMs, uint32_t diff)
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

class MistsOfTirnaScitheInstanceScript : public InstanceScript
{
public:
    explicit MistsOfTirnaScitheInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(MistsOfTirnaScitheEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new MistsOfTirnaScitheInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        uint32_t const entry = pCreature->getEntry();
        if (entry == BOSS_INGRA_MALOCH || std::find(std::begin(DromanEntries), std::end(DromanEntries), entry) != std::end(DromanEntries))
            mTrackedGuids[entry] = pCreature->GetNewGUID();
    }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    // whichever Droman template stands in this instance
    Creature* getDroman()
    {
        for (uint32_t const entry : DromanEntries)
        {
            Creature* pDroman = getTracked(entry);
            if (pDroman != nullptr && pDroman->isAlive())
                return pDroman;
        }

        return nullptr;
    }

private:
    std::map<uint32_t, WoWGuid> mTrackedGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock and target helpers

class MistsOfTirnaScitheCombatAI : public CreatureAIScript
{
public:
    explicit MistsOfTirnaScitheCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    MistsOfTirnaScitheInstanceScript* getMists()
    {
        if (getCreature()->GetMapId() != MAP_MISTS_OF_TIRNA_SCITHE)
            return nullptr;

        return static_cast<MistsOfTirnaScitheInstanceScript*>(getInstanceScript());
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

    // ground spell under a random player
    void castUnderRandom(uint32_t spellId, bool triggered = false)
    {
        if (Unit* pTarget = randomPlayer())
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId, triggered);
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
    Creature* callAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = placeHelper(entry, spot, despawnType, durationMs);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return mistsRing(getCreature()->GetPosition(), minRange, maxRange); }

    void focusOn(Unit* pTarget)
    {
        if (pTarget == nullptr)
            return;

        _clearHateList();
        addThreat(pTarget, 100000.0f);
        attackStart(pTarget);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class MistsOfTirnaScitheBossAI : public MistsOfTirnaScitheCombatAI
{
public:
    MistsOfTirnaScitheBossAI(Creature* pCreature, uint32_t dataId) : MistsOfTirnaScitheCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (MistsOfTirnaScitheInstanceScript* pMists = getMists())
        {
            pMists->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pMists->setBossState(mDataId, EncounterStates::InProgress);
        }

        MistsOfTirnaScitheCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (MistsOfTirnaScitheInstanceScript* pMists = getMists())
            {
                pMists->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pMists->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        MistsOfTirnaScitheCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (MistsOfTirnaScitheInstanceScript* pMists = getMists())
        {
            pMists->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pMists->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
        onDefeat();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mistcaller, hides among her clones at 70%, 40% and 10%

class MistsOfTirnaScitheMistcallerAI : public MistsOfTirnaScitheBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MistsOfTirnaScitheMistcallerAI(c); }
    explicit MistsOfTirnaScitheMistcallerAI(Creature* pCreature) : MistsOfTirnaScitheBossAI(pCreature, DATA_MISTCALLER) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_MISTCALLER_FOUND || !mHiding || !_isInCombat())
            return;

        summons.despawnEntry(NPC_MISTCALLER_ILLUSIONARY_CLONE);
        leaveHiding();
        warn("Mistcaller is flushed out of hiding!");
        startEvents();
    }

protected:
    void onEngage() override
    {
        mGames = 0;
        mHiding = false;
        startEvents();
    }

    void onDisengage() override
    {
        if (mHiding)
            leaveHiding();
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mHiding && mGames < 3 && _getHealthPercent() <= MistcallerGameMarks[mGames])
            startGuessingGame();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MISTCALLER_DODGE_BALL:
                castSpellOnVictim(SPELL_MISTCALLER_DODGE_BALL);
                scriptEvents.addEvent(EVENT_MISTCALLER_DODGE_BALL, 12000);
                break;
            case EVENT_MISTCALLER_PATTY_CAKE:
                castAtRandom(SPELL_MISTCALLER_PATTY_CAKE, true);
                scriptEvents.addEvent(EVENT_MISTCALLER_PATTY_CAKE, 20000);
                break;
            case EVENT_MISTCALLER_FREEZE_TAG:
                warn("Mistcaller starts a game of Freeze Tag!");
                callAdd(NPC_MISTCALLER_ILLUSIONARY_VULPIN, aroundSelf(5.0f, 10.0f), TIMED_DESPAWN, VulpinLifeMs);
                scriptEvents.addEvent(EVENT_MISTCALLER_FREEZE_TAG, 35000);
                break;
            default:
                break;
        }
    }

private:
    void startEvents()
    {
        scriptEvents.addEvent(EVENT_MISTCALLER_DODGE_BALL, 6000);
        scriptEvents.addEvent(EVENT_MISTCALLER_PATTY_CAKE, 12000);
        scriptEvents.addEvent(EVENT_MISTCALLER_FREEZE_TAG, 20000);
    }

    // one of the clones is the real Mistcaller
    void startGuessingGame()
    {
        ++mGames;
        mHiding = true;
        warn("Mistcaller hides among her illusions. Find the real one!");

        getCreature()->interruptSpell();
        scriptEvents.resetEvents();
        summons.despawnEntry(NPC_MISTCALLER_ILLUSIONARY_VULPIN);

        castSpellOnSelf(SPELL_MISTCALLER_GUESSING_GAME, true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(true);
        _setMeleeDisabled(true);

        uint32_t const realOne = Util::getRandomUInt(MistcallerClonesPerGame - 1);
        for (uint32_t i = 0; i < MistcallerClonesPerGame; ++i)
        {
            Creature* pClone = placeHelper(NPC_MISTCALLER_ILLUSIONARY_CLONE, aroundSelf(8.0f, 14.0f));
            if (pClone != nullptr && pClone->GetScript() != nullptr)
                pClone->GetScript()->SetCreatureData(CLONE_DATA_REAL_ONE, i == realOne ? 1 : 0);
        }
    }

    void leaveHiding()
    {
        mHiding = false;
        _removeAura(SPELL_MISTCALLER_GUESSING_GAME);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(false);
        _setMeleeDisabled(false);
    }

    uint32_t mGames = 0;
    bool mHiding = false;
};

// clones punish a slow guess, a wrong kill backfires
class MistsOfTirnaScitheIllusionaryCloneAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MistsOfTirnaScitheIllusionaryCloneAI(c); }
    explicit MistsOfTirnaScitheIllusionaryCloneAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        mPenaltyLeftMs = ClonePenaltyMs;
    }

    void OnSummon(Unit* pSummoner) override
    {
        if (pSummoner != nullptr)
            mMistcallerGuid = pSummoner->getGuid();
    }

    void SetCreatureData(uint32_t type, uint32_t data) override
    {
        if (type == CLONE_DATA_REAL_ONE)
            mRealOne = data != 0;
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mFallen || *damage < getCreature()->getHealth())
            return;

        mFallen = true;
        if (!mRealOne)
            castSpellAOE(SPELL_CLONE_OOPSIE, true);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (!mRealOne || mMistcallerGuid == 0)
            return;

        Creature* pMistcaller = getCreature()->getWorldMapCreature(mMistcallerGuid);
        if (pMistcaller != nullptr && pMistcaller->isAlive() && pMistcaller->GetScript() != nullptr)
            pMistcaller->GetScript()->DoAction(ACTION_MISTCALLER_FOUND);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive() || !mistsCountdown(mPenaltyLeftMs, static_cast<uint32_t>(time_passed)))
            return;

        castSpellAOE(SPELL_CLONE_PENALIZING_BURST);
        mPenaltyLeftMs = ClonePenaltyMs;
    }

private:
    uint64_t mMistcallerGuid = 0;
    uint32_t mPenaltyLeftMs = 0;
    bool mRealOne = false;
    bool mFallen = false;
};

// chases one player and freezes everyone around on contact
class MistsOfTirnaScitheIllusionaryVulpinAI : public MistsOfTirnaScitheCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MistsOfTirnaScitheIllusionaryVulpinAI(c); }
    explicit MistsOfTirnaScitheIllusionaryVulpinAI(Creature* pCreature) : MistsOfTirnaScitheCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        mPreyGuid = 0;
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_VULPIN_PICK_PREY, 500);
    }

    void onDisengage() override { _setMeleeDisabled(false); }

    void onCombatTick(uint32_t diff) override
    {
        if (mPreyGuid == 0 || !mistsCountdown(mCheckLeftMs, diff))
            return;

        mCheckLeftMs = 400;

        Unit* pPrey = getCreature()->getWorldMapUnit(mPreyGuid);
        if (pPrey == nullptr || !pPrey->isAlive())
        {
            mPreyGuid = 0;
            scriptEvents.addEvent(EVENT_VULPIN_PICK_PREY, 500);
            return;
        }

        if (getCreature()->getDistance(pPrey) > VulpinTagReach)
            return;

        castSpellAOE(SPELL_VULPIN_FREEZING_BURST, true);
        pPrey->removeAllAurasById(SPELL_VULPIN_FREEZE_TAG_FIXATION);
        mPreyGuid = 0;
        scriptEvents.addEvent(EVENT_VULPIN_PICK_PREY, 3000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_VULPIN_PICK_PREY)
            return;

        Unit* pPrey = randomPlayer();
        if (pPrey == nullptr)
            return;

        castSpell(pPrey, SPELL_VULPIN_FREEZE_TAG_FIXATION, true);
        focusOn(pPrey);
        mPreyGuid = pPrey->getGuid();
        mCheckLeftMs = 400;
    }

private:
    uint64_t mPreyGuid = 0;
    uint32_t mCheckLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Tred'ova, feasts on the Cocoon of Lakali at 70% and 40%

class MistsOfTirnaScitheTredovaAI : public MistsOfTirnaScitheBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MistsOfTirnaScitheTredovaAI(c); }
    explicit MistsOfTirnaScitheTredovaAI(Creature* pCreature) : MistsOfTirnaScitheBossAI(pCreature, DATA_TREDOVA) {}

protected:
    void onEngage() override
    {
        mFeasts = 0;
        mFeasting = false;
        startEvents(2000);
    }

    void onDisengage() override
    {
        mFeasting = false;
        _removeAura(SPELL_TREDOVA_GORGING_SHIELD);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mFeasting)
        {
            tickFeast(diff);
            return;
        }

        if (mFeasts < 2 && _getHealthPercent() <= TredovaFeastMarks[mFeasts])
            startFeast();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TREDOVA_ACID_EXPULSION:
                castUnderRandom(SPELL_TREDOVA_ACID_EXPULSION);
                scriptEvents.addEvent(EVENT_TREDOVA_ACID_EXPULSION, sharpened(15000));
                break;
            case EVENT_TREDOVA_MIND_LINK:
                castAtRandom(SPELL_TREDOVA_MIND_LINK, true);
                scriptEvents.addEvent(EVENT_TREDOVA_MIND_LINK, sharpened(25000));
                break;
            case EVENT_TREDOVA_MARKED_PREY:
                castAtRandom(SPELL_TREDOVA_MARKED_PREY, true);
                scriptEvents.addEvent(EVENT_TREDOVA_MARKED_PREY, sharpened(20000));
                break;
            case EVENT_TREDOVA_ACCELERATED_INCUBATION:
                castSpellOnSelf(SPELL_TREDOVA_ACCELERATED_INCUBATION);
                for (uint32_t i = 0; i < TredovaLarvaePerBrood; ++i)
                    callAdd(NPC_TREDOVA_GORMLING_LARVA, aroundSelf(5.0f, 10.0f));
                scriptEvents.addEvent(EVENT_TREDOVA_ACCELERATED_INCUBATION, sharpened(32000));
                break;
            default:
                break;
        }
    }

private:
    void startEvents(uint32_t firstMs)
    {
        scriptEvents.addEvent(EVENT_TREDOVA_ACID_EXPULSION, firstMs + 4000);
        scriptEvents.addEvent(EVENT_TREDOVA_MIND_LINK, firstMs + 8000);
        scriptEvents.addEvent(EVENT_TREDOVA_MARKED_PREY, firstMs + 12000);
        scriptEvents.addEvent(EVENT_TREDOVA_ACCELERATED_INCUBATION, firstMs + 18000);
    }

    // every feast makes her abilities come faster
    uint32_t sharpened(uint32_t baseMs) const { return baseMs - baseMs * mFeasts / 5; }

    void startFeast()
    {
        ++mFeasts;
        mFeasting = true;
        mFeastLeftMs = TredovaFeastMs;
        mShieldSettleMs = TredovaShieldSettleMs;
        mShedLeftMs = 3000;

        warn("Tred'ova feasts upon the Cocoon of Lakali! Shatter her Gorging Shield!");
        getCreature()->interruptSpell();
        scriptEvents.resetEvents();

        castSpellOnSelf(SPELL_TREDOVA_GORGING_SHIELD, true);
        Creature* pCocoon = findNearestCreature(NPC_TREDOVA_COCOON_OF_LAKALI, 60.0f);
        if (pCocoon != nullptr)
            castSpell(pCocoon, SPELL_TREDOVA_CONSUMPTION);
        else
            castSpellOnSelf(SPELL_TREDOVA_CONSUMPTION);
    }

    void tickFeast(uint32_t diff)
    {
        if (mistsCountdown(mShedLeftMs, diff))
        {
            castUnderRandom(SPELL_TREDOVA_ANIMA_SHEDDING, true);
            mShedLeftMs = 4000;
        }

        mShieldSettleMs = mShieldSettleMs > diff ? mShieldSettleMs - diff : 0;
        if (mShieldSettleMs == 0 && !hasAura(SPELL_TREDOVA_GORGING_SHIELD))
        {
            endFeast(true);
            return;
        }

        if (mistsCountdown(mFeastLeftMs, diff))
            endFeast(false);
    }

    void endFeast(bool shieldShattered)
    {
        mFeasting = false;
        getCreature()->interruptSpell();

        if (shieldShattered)
        {
            warn("The Gorging Shield shatters and Tred'ova rejects the anima!");
            castSpellAOE(SPELL_TREDOVA_ANIMA_REJECTION, true);
        }
        else
        {
            _removeAura(SPELL_TREDOVA_GORGING_SHIELD);
        }

        startEvents(1000);
    }

    uint32_t mFeasts = 0;
    bool mFeasting = false;
    uint32_t mFeastLeftMs = 0;
    uint32_t mShieldSettleMs = 0;
    uint32_t mShedLeftMs = 0;
};

// hunts whoever carries Marked Prey
class MistsOfTirnaScitheGormlingLarvaAI : public MistsOfTirnaScitheCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MistsOfTirnaScitheGormlingLarvaAI(c); }
    explicit MistsOfTirnaScitheGormlingLarvaAI(Creature* pCreature) : MistsOfTirnaScitheCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mBurst || *damage < getCreature()->getHealth())
            return;

        mBurst = true;
        castSpellAOE(SPELL_LARVA_DECOMPOSITION_POOL, true);
    }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_LARVA_HUNT, 500); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_LARVA_HUNT)
            return;

        Unit* pPrey = markedPrey();
        focusOn(pPrey != nullptr ? pPrey : randomPlayer());
        scriptEvents.addEvent(EVENT_LARVA_HUNT, 6000);
    }

private:
    Unit* markedPrey()
    {
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && pPlayer->hasAurasWithId(SPELL_TREDOVA_MARKED_PREY))
                return pPlayer;
        }

        return nullptr;
    }

    bool mBurst = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ingra Maloch and the shackled Droman Oulfarran

class MistsOfTirnaScitheIngraMalochAI : public MistsOfTirnaScitheBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MistsOfTirnaScitheIngraMalochAI(c); }
    explicit MistsOfTirnaScitheIngraMalochAI(Creature* pCreature) : MistsOfTirnaScitheBossAI(pCreature, DATA_INGRA_MALOCH) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_MALOCH_DROMAN_FREED || mDromanFree || !_isInCombat())
            return;

        mDromanFree = true;
        warn("Droman Oulfarran breaks free and strikes back at Ingra Maloch!");
        _removeAura(SPELL_MALOCH_EMBRACE_DARKNESS);
        if (!hasAura(SPELL_DROMAN_DROMANS_WRATH))
            castSpellOnSelf(SPELL_DROMAN_DROMANS_WRATH, true);

        scriptEvents.removeEvent(EVENT_MALOCH_EMBRACE_DARKNESS);
        scriptEvents.addEvent(EVENT_MALOCH_SOUL_SHACKLE, DromanWrathMs);
    }

protected:
    void onEngage() override
    {
        mDromanFree = false;
        scriptEvents.addEvent(EVENT_MALOCH_SOUL_SHACKLE, 500);
        scriptEvents.addEvent(EVENT_MALOCH_SPIRIT_BOLT, 4000);
    }

    void onDisengage() override { releaseDroman(); }

    void onDefeat() override { releaseDroman(); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MALOCH_SOUL_SHACKLE:
                if (shackleDroman())
                {
                    mDromanFree = false;
                    warn("Ingra Maloch binds Droman Oulfarran with a Soul Shackle!");
                    scriptEvents.addEvent(EVENT_MALOCH_EMBRACE_DARKNESS, 3000);
                }
                break;
            case EVENT_MALOCH_EMBRACE_DARKNESS:
                warn("Ingra Maloch embraces the darkness!");
                castSpellOnSelf(SPELL_MALOCH_EMBRACE_DARKNESS);
                scriptEvents.addEvent(EVENT_MALOCH_EMBRACE_DARKNESS, 30000);
                break;
            case EVENT_MALOCH_SPIRIT_BOLT:
                castSpellOnVictim(SPELL_MALOCH_SPIRIT_BOLT);
                scriptEvents.addEvent(EVENT_MALOCH_SPIRIT_BOLT, 6000);
                break;
            default:
                break;
        }
    }

private:
    Creature* droman()
    {
        MistsOfTirnaScitheInstanceScript* pMists = getMists();
        return pMists != nullptr ? pMists->getDroman() : nullptr;
    }

    bool shackleDroman()
    {
        Creature* pDroman = droman();
        if (pDroman == nullptr || pDroman->GetScript() == nullptr)
            return false;

        castSpell(pDroman, SPELL_MALOCH_SOUL_SHACKLE, true);
        pDroman->GetScript()->DoAction(ACTION_DROMAN_SHACKLED);
        setZoneWideCombat(pDroman);
        return true;
    }

    void releaseDroman()
    {
        Creature* pDroman = droman();
        if (pDroman != nullptr && pDroman->GetScript() != nullptr)
            pDroman->GetScript()->DoAction(ACTION_DROMAN_RELEASED);
    }

    bool mDromanFree = false;
};

// hostile while shackled, breaks free at 20% and turns on Maloch
class MistsOfTirnaScitheDromanOulfarranAI : public MistsOfTirnaScitheCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MistsOfTirnaScitheDromanOulfarranAI(c); }
    explicit MistsOfTirnaScitheDromanOulfarranAI(Creature* pCreature) : MistsOfTirnaScitheCombatAI(pCreature) {}

    void OnLoad() override { mFreeFaction = getCreature()->getFactionTemplate(); }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_DROMAN_SHACKLED)
            bind();
        else if (action == ACTION_DROMAN_RELEASED)
            unbind(true);
    }

    // Droman never falls, she breaks free instead
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (!mShackled)
        {
            *damage = 0;
            return;
        }

        uint32_t const floor = std::max<uint32_t>(getCreature()->getMaxHealth() * DromanBreakFreePct / 100, 1);
        uint32_t const health = getCreature()->getHealth();
        if (health > *damage + floor)
            return;

        *damage = health > floor ? health - floor : 0;
        mBreakPending = true;
    }

protected:
    void onEngage() override
    {
        if (mShackled)
            startEvents();
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (!mBreakPending)
            return;

        mBreakPending = false;
        breakFree();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DROMAN_BEWILDERING_POLLEN:
                castSpellOnVictim(SPELL_DROMAN_BEWILDERING_POLLEN);
                scriptEvents.addEvent(EVENT_DROMAN_BEWILDERING_POLLEN, 12000);
                break;
            case EVENT_DROMAN_TEARS_OF_THE_FOREST:
                castSpellOnSelf(SPELL_DROMAN_TEARS_OF_THE_FOREST);
                scriptEvents.addEvent(EVENT_DROMAN_TEARS_OF_THE_FOREST, 20000);
                break;
            default:
                break;
        }
    }

private:
    void startEvents()
    {
        scriptEvents.addEvent(EVENT_DROMAN_BEWILDERING_POLLEN, 6000);
        scriptEvents.addEvent(EVENT_DROMAN_TEARS_OF_THE_FOREST, 12000);
    }

    void bind()
    {
        mShackled = true;
        mBreakPending = false;
        getCreature()->setHealth(getCreature()->getMaxHealth());
        getCreature()->setFaction(DromanShackledFaction);
        setReactState(REACT_AGGRESSIVE);

        if (_isInCombat())
        {
            scriptEvents.resetEvents();
            startEvents();
        }
    }

    void unbind(bool restoreHealth)
    {
        mShackled = false;
        mBreakPending = false;
        scriptEvents.resetEvents();
        getCreature()->interruptSpell();
        getCreature()->setFaction(mFreeFaction);
        setReactState(REACT_PASSIVE);
        attackStop();
        _wipeHateList();

        if (restoreHealth)
            getCreature()->setHealth(getCreature()->getMaxHealth());
    }

    void breakFree()
    {
        unbind(false);

        MistsOfTirnaScitheInstanceScript* pMists = getMists();
        Creature* pMaloch = pMists != nullptr ? pMists->getTracked(BOSS_INGRA_MALOCH) : nullptr;
        if (pMaloch == nullptr || !pMaloch->isAlive() || pMaloch->GetScript() == nullptr)
            return;

        getCreature()->castSpell(pMaloch, SPELL_DROMAN_DROMANS_WRATH, true);
        pMaloch->GetScript()->DoAction(ACTION_MALOCH_DROMAN_FREED);
    }

    uint32_t mFreeFaction = DromanFreeFallbackFaction;
    bool mShackled = false;
    bool mBreakPending = false;
};

void SetupMistsOfTirnaScithe(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MISTS_OF_TIRNA_SCITHE, &MistsOfTirnaScitheInstanceScript::Create);

    // Mistcaller
    mgr->register_creature_script(BOSS_MISTCALLER, &MistsOfTirnaScitheMistcallerAI::Create);
    mgr->register_creature_script(NPC_MISTCALLER_ILLUSIONARY_CLONE, &MistsOfTirnaScitheIllusionaryCloneAI::Create);
    mgr->register_creature_script(NPC_MISTCALLER_ILLUSIONARY_VULPIN, &MistsOfTirnaScitheIllusionaryVulpinAI::Create);

    // Tred'ova
    mgr->register_creature_script(BOSS_TREDOVA, &MistsOfTirnaScitheTredovaAI::Create);
    mgr->register_creature_script(NPC_TREDOVA_GORMLING_LARVA, &MistsOfTirnaScitheGormlingLarvaAI::Create);

    // Ingra Maloch
    mgr->register_creature_script(BOSS_INGRA_MALOCH, &MistsOfTirnaScitheIngraMalochAI::Create);
    for (uint32_t const entry : DromanEntries)
        mgr->register_creature_script(entry, &MistsOfTirnaScitheDromanOulfarranAI::Create);
}
