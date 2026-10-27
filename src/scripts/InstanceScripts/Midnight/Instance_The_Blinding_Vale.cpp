/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Blinding_Vale.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <list>
#include <map>
#include <vector>

enum TheBlindingValeEvents
{
    EVENT_MEITTIK_BEDROCK_SLAM = 1,

    EVENT_LEKSHI_THORNBLADE,
    EVENT_LEKSHI_LIGHTSOWER_DASH,

    EVENT_KEZKITT_LIGHT_BOLT,
    EVENT_KEZKITT_LIGHTBLOSSOM_BEAM,

    EVENT_IKUZZ_BLOODTHORN_ROOTS,
    EVENT_IKUZZ_VERDANT_STOMP,
    EVENT_IKUZZ_THORNCALLER_ROAR,

    EVENT_RUIA_SHAPESHIFT,
    EVENT_RUIA_WARDENS_WRATH,
    EVENT_RUIA_LIGHTFIRE,
    EVENT_RUIA_LIGHTFALL,
    EVENT_RUIA_MANGLING_CLAWS,
    EVENT_RUIA_PULVERIZING_STRIKES,
    EVENT_RUIA_GRIEVOUS_THRASH,

    EVENT_ZIEKKET_THORNSPIKE,
    EVENT_ZIEKKET_AWAKEN_THE_LIGHTBLOOM,
    EVENT_ZIEKKET_CONCENTRATED_LIGHTBEAM,

    EVENT_LASHER_LIGHTSPORE_SHOT
};

namespace
{
    float const ValeTwoPi = 6.2831853f;

    uint32_t const TrioMembers[3] = { BOSS_TRIO_MEITTIK, BOSS_TRIO_KEZKITT, BOSS_TRIO_LEKSHI };

    size_t const MaxLoamPatches = 6;
    uint32_t const BlossomBurstMs = 20000;
    uint32_t const BlossomBeamGrowthMs = 5000;

    uint32_t const GazeChaseMs = 8000;
    float const InciseReach = 5.0f;
    int32_t const IkuzzFrenzyPct = 30;
    float const RootsGripReach = 4.0f;

    uint32_t const RuiaFormMs = 30000;

    uint8_t const LashersPerAwakening = 3;
    float const LightbeamReach = 8.0f;

    LocationVector valeRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(ValeTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class TheBlindingValeInstanceScript : public InstanceScript
{
public:
    explicit TheBlindingValeInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TheBlindingValeEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheBlindingValeInstanceScript(pMapMgr); }

    // one rutaani falling must not end the Lightblossom Trio
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (uint32_t entry : TrioMembers)
        {
            if (pCreature->getEntry() == entry)
                mTrioGuids[entry] = pCreature->GetNewGUID();
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id != DATA_LIGHTBLOSSOM_TRIO)
            return;

        if (state != EncounterStates::InProgress)
            mLoamPatches.clear();

        if (state != EncounterStates::Failed)
            return;

        // a wiped trio fights again in full
        for (Creature* pMember : getTrio())
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getTrio()
    {
        std::vector<Creature*> trio;
        for (auto const& member : mTrioGuids)
        {
            if (Creature* pMember = getCreatureByGuid(member.second))
                trio.push_back(pMember);
        }

        return trio;
    }

    // single boss encounters are down with their boss
    bool isEncounterDown(uint32_t dataId, Creature* pExcept)
    {
        if (dataId != DATA_LIGHTBLOSSOM_TRIO)
            return true;

        for (Creature* pMember : getTrio())
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    bool isTrioFighting(Creature* pExcept)
    {
        for (Creature* pMember : getTrio())
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

    // pulling one rutaani pulls the other two
    void rallyTrio(CreatureAIScript* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getTrio())
        {
            if (pMember == pPuller->getCreature() || !pMember->isAlive() || pMember->isInCombat())
                continue;

            pPuller->setZoneWideCombat(pMember);
        }
        mRallying = false;
    }

    // Meittik's loam waits for Lekshi to plant it
    void addLoamPatch(LocationVector const& spot)
    {
        if (mLoamPatches.size() < MaxLoamPatches)
            mLoamPatches.push_back(spot);
    }

    std::vector<LocationVector> takeLoamPatches()
    {
        std::vector<LocationVector> patches;
        patches.swap(mLoamPatches);
        return patches;
    }

private:
    std::map<uint32_t, WoWGuid> mTrioGuids;
    std::vector<LocationVector> mLoamPatches;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy bar and target helpers

class TheBlindingValeCombatAI : public CreatureAIScript
{
public:
    explicit TheBlindingValeCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mEnergyFillMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mEnergyFillMs = 0;

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
        tickEnergy(diff);
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
    virtual void onEnergyFull() {}

    TheBlindingValeInstanceScript* getVale()
    {
        if (getCreature()->GetMapId() != MAP_THE_BLINDING_VALE)
            return nullptr;

        return static_cast<TheBlindingValeInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank, the tank when alone
    Unit* anyPlayerButTank()
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

    void castUnder(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId, triggered);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return valeRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // the bar fills from empty in fillMs
    void armEnergy(uint32_t fillMs)
    {
        mEnergyFillMs = fillMs;
        mEnergyMs = 0;
        mEnergyFull = false;
    }

private:
    void tickEnergy(uint32_t diff)
    {
        if (mEnergyFillMs == 0)
            return;

        if (!mEnergyFull)
        {
            mEnergyMs += diff;
            mEnergyFull = mEnergyMs >= mEnergyFillMs;
        }

        // a full bar waits for the running cast
        if (!mEnergyFull || _isCasting())
            return;

        mEnergyFull = false;
        mEnergyMs = 0;
        onEnergyFull();
    }

    uint32_t mEnergyFillMs = 0;
    uint32_t mEnergyMs = 0;
    bool mEnergyFull = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and trio pull

class TheBlindingValeBossAI : public TheBlindingValeCombatAI
{
public:
    TheBlindingValeBossAI(Creature* pCreature, uint32_t dataId) : TheBlindingValeCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (mDataId == DATA_LIGHTBLOSSOM_TRIO)
        {
            if (TheBlindingValeInstanceScript* pVale = getVale())
                pVale->rallyTrio(this);
        }

        TheBlindingValeCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last rutaani has left the fight
                TheBlindingValeInstanceScript* pVale = getVale();
                if (mDataId != DATA_LIGHTBLOSSOM_TRIO || pVale == nullptr || !pVale->isTrioFighting(getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        TheBlindingValeCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            TheBlindingValeInstanceScript* pVale = getVale();
            if (pVale == nullptr || pVale->isEncounterDown(mDataId, getCreature()))
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
// Lightblossom Trio

// The shared thicket aura splits every hit between the living rutaani
class TheBlindingValeTrioMemberAI : public TheBlindingValeBossAI
{
public:
    explicit TheBlindingValeTrioMemberAI(Creature* pCreature) : TheBlindingValeBossAI(pCreature, DATA_LIGHTBLOSSOM_TRIO) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || *damage == 0 || !_isInCombat())
            return;

        TheBlindingValeInstanceScript* pVale = getVale();
        if (pVale == nullptr)
            return;

        std::vector<Creature*> partners;
        for (Creature* pMember : pVale->getTrio())
        {
            if (pMember != getCreature() && pMember->isAlive())
                partners.push_back(pMember);
        }

        if (partners.empty())
            return;

        uint32_t const share = *damage / static_cast<uint32_t>(partners.size() + 1);
        *damage -= share * static_cast<uint32_t>(partners.size());

        // shares never finish a partner, its own wounds do
        for (Creature* pPartner : partners)
        {
            uint32_t const health = pPartner->getHealth();
            pPartner->setHealth(health > share ? health - share : 1);
        }
    }

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_TRIO_SHARED_THICKET, true);
        onTrioEngage();
    }

    void onReset() override { _removeAura(SPELL_TRIO_SHARED_THICKET); }

    virtual void onTrioEngage() {}
};

// Meittik, sows Fertile Loam around the tank
class TheBlindingValeMeittikAI : public TheBlindingValeTrioMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheBlindingValeMeittikAI(c); }
    explicit TheBlindingValeMeittikAI(Creature* pCreature) : TheBlindingValeTrioMemberAI(pCreature) {}

protected:
    void onTrioEngage() override { scriptEvents.addEvent(EVENT_MEITTIK_BEDROCK_SLAM, 6000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_MEITTIK_BEDROCK_SLAM)
            return;

        if (Unit* pTank = getCreature()->getAIInterface()->getCurrentTarget())
        {
            castSpell(pTank, SPELL_MEITTIK_BEDROCK_SLAM);

            TheBlindingValeInstanceScript* pVale = getVale();
            for (uint8_t i = 0; i < 2; ++i)
            {
                LocationVector const patch = valeRingPoint(pTank->GetPosition(), 3.0f, 7.0f);
                getCreature()->castSpellLoc(patch, SPELL_MEITTIK_FERTILE_LOAM, true);
                if (pVale != nullptr)
                    pVale->addLoamPatch(patch);
            }
        }

        scriptEvents.addEvent(EVENT_MEITTIK_BEDROCK_SLAM, 16000);
    }
};

// Lekshi, dashes through the loam and plants Lightblossoms
class TheBlindingValeLekshiAI : public TheBlindingValeTrioMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheBlindingValeLekshiAI(c); }
    explicit TheBlindingValeLekshiAI(Creature* pCreature) : TheBlindingValeTrioMemberAI(pCreature) {}

protected:
    void onTrioEngage() override
    {
        scriptEvents.addEvent(EVENT_LEKSHI_THORNBLADE, 9000);
        scriptEvents.addEvent(EVENT_LEKSHI_LIGHTSOWER_DASH, 18000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LEKSHI_THORNBLADE:
                castAt(anyPlayerButTank(), SPELL_LEKSHI_THORNBLADE);
                scriptEvents.addEvent(EVENT_LEKSHI_THORNBLADE, 15000);
                break;
            case EVENT_LEKSHI_LIGHTSOWER_DASH:
                sowBlossoms();
                scriptEvents.addEvent(EVENT_LEKSHI_LIGHTSOWER_DASH, 28000);
                break;
            default:
                break;
        }
    }

private:
    // without loam a single blossom is planted at a player
    void sowBlossoms()
    {
        castSpellOnSelf(SPELL_LEKSHI_LIGHTSOWER_DASH);

        TheBlindingValeInstanceScript* pVale = getVale();
        std::vector<LocationVector> patches = pVale != nullptr ? pVale->takeLoamPatches() : std::vector<LocationVector>();
        if (patches.empty())
        {
            if (Unit* pTarget = anyPlayer())
                patches.push_back(valeRingPoint(pTarget->GetPosition(), 2.0f, 5.0f));
        }

        if (!patches.empty())
            warn("Lekshi dashes through the loam, Lightblossoms take root!");

        for (LocationVector const& patch : patches)
            summonCreature(NPC_TRIO_LIGHTBLOSSOM, patch, CORPSE_DESPAWN);
    }
};

// Kezkitt, germinates the blossoms with Lightblossom Beam
class TheBlindingValeKezkittAI : public TheBlindingValeTrioMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheBlindingValeKezkittAI(c); }
    explicit TheBlindingValeKezkittAI(Creature* pCreature) : TheBlindingValeTrioMemberAI(pCreature) {}

protected:
    void onTrioEngage() override
    {
        scriptEvents.addEvent(EVENT_KEZKITT_LIGHT_BOLT, 2000);
        scriptEvents.addEvent(EVENT_KEZKITT_LIGHTBLOSSOM_BEAM, 22000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KEZKITT_LIGHT_BOLT:
                castAt(anyPlayer(), SPELL_KEZKITT_LIGHT_BOLT);
                scriptEvents.addEvent(EVENT_KEZKITT_LIGHT_BOLT, 3000);
                break;
            case EVENT_KEZKITT_LIGHTBLOSSOM_BEAM:
                feedBlossoms();
                scriptEvents.addEvent(EVENT_KEZKITT_LIGHTBLOSSOM_BEAM, 26000);
                break;
            default:
                break;
        }
    }

private:
    // every blossom in reach gorges on light and grows faster
    void feedBlossoms()
    {
        std::list<Creature*> blossoms;
        GetCreatureListWithEntryInGrid(blossoms, NPC_TRIO_LIGHTBLOSSOM, 80.0f);

        bool fed = false;
        for (Creature* pBlossom : blossoms)
        {
            if (!pBlossom->isAlive())
                continue;

            if (!fed)
            {
                warn("Kezkitt bathes the Lightblossoms in a Lightblossom Beam! Block the rays!");
                castSpellAOE(SPELL_KEZKITT_LIGHTBLOSSOM_BEAM);
                fed = true;
            }

            castSpell(pBlossom, SPELL_BLOSSOM_LIGHT_GORGED, true);
            if (pBlossom->GetScript() != nullptr)
                pBlossom->GetScript()->DoAction(ACTION_BLOSSOM_FED);
        }
    }
};

// ripens and bursts in a Lightbloom Overgrowth
class TheBlindingValeLightblossomAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheBlindingValeLightblossomAI(c); }
    explicit TheBlindingValeLightblossomAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_BLOSSOM_FED)
            mGrownMs += BlossomBeamGrowthMs;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mBurst || !isAlive())
            return;

        mGrownMs += static_cast<uint32_t>(time_passed);
        if (mGrownMs < BlossomBurstMs)
            return;

        mBurst = true;
        castSpellOnSelf(SPELL_BLOSSOM_LIGHTBLOOM_OVERGROWTH, true);
        getCreature()->castSpellLoc(getCreature()->GetPosition(), SPELL_BLOSSOM_LIGHT_SCORCHED_EARTH, true);
        despawn(4000, 0);
    }

private:
    uint32_t mGrownMs = 0;
    bool mBurst = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ikuzz the Light Hunter

class TheBlindingValeIkuzzAI : public TheBlindingValeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheBlindingValeIkuzzAI(c); }
    explicit TheBlindingValeIkuzzAI(Creature* pCreature) : TheBlindingValeBossAI(pCreature, DATA_IKUZZ) {}

protected:
    void onEngage() override
    {
        mPreyGuid = 0;
        mChaseLeftMs = 0;
        mFrenzied = false;

        armEnergy(40000);
        scriptEvents.addEvent(EVENT_IKUZZ_BLOODTHORN_ROOTS, 9000);
        scriptEvents.addEvent(EVENT_IKUZZ_VERDANT_STOMP, 14000);
        scriptEvents.addEvent(EVENT_IKUZZ_THORNCALLER_ROAR, 25000);
    }

    void onReset() override
    {
        endGaze();
        _removeAura(SPELL_IKUZZ_LIGHTCRAZED_FRENZY);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mFrenzied && _getHealthPercent() <= IkuzzFrenzyPct)
        {
            mFrenzied = true;
            warn("Ikuzz the Light Hunter falls into a Lightcrazed Frenzy!");
            castSpellOnSelf(SPELL_IKUZZ_LIGHTCRAZED_FRENZY, true);
        }

        if (mPreyGuid != 0)
            chasePrey(diff);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_IKUZZ_BLOODTHORN_ROOTS:
                entanglePlayer();
                scriptEvents.addEvent(EVENT_IKUZZ_BLOODTHORN_ROOTS, 22000);
                break;
            case EVENT_IKUZZ_VERDANT_STOMP:
                castSpellAOE(SPELL_IKUZZ_VERDANT_STOMP);
                scriptEvents.addEvent(EVENT_IKUZZ_VERDANT_STOMP, 20000);
                break;
            case EVENT_IKUZZ_THORNCALLER_ROAR:
                warn("Ikuzz the Light Hunter lets out a Thorncaller Roar!");
                castSpellOnSelf(SPELL_IKUZZ_THORNCALLER_ROAR);
                scriptEvents.addEvent(EVENT_IKUZZ_THORNCALLER_ROAR, 38000);
                break;
            default:
                break;
        }
    }

    // fixates one player and tries to reach them
    void onEnergyFull() override
    {
        Unit* pPrey = anyPlayerButTank();
        if (pPrey == nullptr)
            return;

        warn("Ikuzz the Light Hunter fixes his Bloodthirsty Gaze on a player!");
        castSpell(pPrey, SPELL_IKUZZ_BLOODTHIRSTY_GAZE, true);
        castSpellOnSelf(SPELL_IKUZZ_CRUSHING_FOOTFALLS, true);
        getCreature()->getThreatManager().fixateTarget(pPrey);

        mPreyGuid = pPrey->getGuid();
        mChaseLeftMs = GazeChaseMs;
    }

private:
    void chasePrey(uint32_t diff)
    {
        Unit* pPrey = getCreature()->getWorldMapUnit(mPreyGuid);
        if (pPrey == nullptr || !pPrey->isAlive())
        {
            endGaze();
            return;
        }

        if (getCreature()->getDistance(pPrey) <= InciseReach && !_isCasting())
        {
            castSpell(pPrey, SPELL_IKUZZ_INCISE);
            endGaze();
            return;
        }

        if (mChaseLeftMs > diff)
            mChaseLeftMs -= diff;
        else
            endGaze();
    }

    void endGaze()
    {
        if (mPreyGuid == 0)
            return;

        mPreyGuid = 0;
        mChaseLeftMs = 0;
        getCreature()->getThreatManager().clearFixate();
        _removeAura(SPELL_IKUZZ_CRUSHING_FOOTFALLS);
    }

    // the roots hold their victim until cut down
    void entanglePlayer()
    {
        Unit* pTarget = anyPlayerButTank();
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, SPELL_IKUZZ_BLOODTHORN_ROOTS, true);
        summonCreature(NPC_IKUZZ_BLOODTHORN_ROOTS, pTarget->GetPosition(), CORPSE_DESPAWN);
    }

    uint64_t mPreyGuid = 0;
    uint32_t mChaseLeftMs = 0;
    bool mFrenzied = false;
};

// destroying the roots frees the player held inside
class TheBlindingValeBloodthornRootsAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheBlindingValeBloodthornRootsAI(c); }
    explicit TheBlindingValeBloodthornRootsAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && getCreature()->getDistance(pPlayer) <= RootsGripReach)
                pPlayer->removeAllAurasById(SPELL_IKUZZ_BLOODTHORN_ROOTS);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lightwarden Ruia, alternates between Moonkin and Bear form

class TheBlindingValeRuiaAI : public TheBlindingValeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheBlindingValeRuiaAI(c); }
    explicit TheBlindingValeRuiaAI(Creature* pCreature) : TheBlindingValeBossAI(pCreature, DATA_LIGHTWARDEN_RUIA) {}

protected:
    void onEngage() override
    {
        takeMoonkinForm();
        scriptEvents.addEvent(EVENT_RUIA_SHAPESHIFT, RuiaFormMs);
    }

    void onReset() override
    {
        mBearForm = false;
        _setMeleeDisabled(false);
        _removeAura(SPELL_RUIA_MANGLING_CLAWS);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RUIA_SHAPESHIFT:
                if (mBearForm)
                    takeMoonkinForm();
                else
                    takeBearForm();
                scriptEvents.addEvent(EVENT_RUIA_SHAPESHIFT, RuiaFormMs);
                break;
            case EVENT_RUIA_WARDENS_WRATH:
                castSpellOnVictim(SPELL_RUIA_WARDENS_WRATH);
                scriptEvents.addEvent(EVENT_RUIA_WARDENS_WRATH, 2500);
                break;
            case EVENT_RUIA_LIGHTFIRE:
                castAt(anyPlayer(), SPELL_RUIA_LIGHTFIRE);
                scriptEvents.addEvent(EVENT_RUIA_LIGHTFIRE, 14000);
                break;
            case EVENT_RUIA_LIGHTFALL:
                warn("Lightwarden Ruia calls down Lightfall!");
                castSpellAOE(SPELL_RUIA_LIGHTFALL);
                scriptEvents.addEvent(EVENT_RUIA_LIGHTFALL, 20000);
                break;
            case EVENT_RUIA_MANGLING_CLAWS:
                castSpellOnSelf(SPELL_RUIA_MANGLING_CLAWS);
                scriptEvents.addEvent(EVENT_RUIA_MANGLING_CLAWS, 10000);
                break;
            case EVENT_RUIA_PULVERIZING_STRIKES:
                castSpellOnSelf(SPELL_RUIA_PULVERIZING_STRIKES);
                scriptEvents.addEvent(EVENT_RUIA_PULVERIZING_STRIKES, 16000);
                break;
            case EVENT_RUIA_GRIEVOUS_THRASH:
                castSpellAOE(SPELL_RUIA_GRIEVOUS_THRASH);
                scriptEvents.addEvent(EVENT_RUIA_GRIEVOUS_THRASH, 18000);
                break;
            default:
                break;
        }
    }

private:
    // Warden's Wrath replaces her melee
    void takeMoonkinForm()
    {
        mBearForm = false;
        _setMeleeDisabled(true);
        _removeAura(SPELL_RUIA_MANGLING_CLAWS);

        scriptEvents.removeEvent(EVENT_RUIA_MANGLING_CLAWS);
        scriptEvents.removeEvent(EVENT_RUIA_PULVERIZING_STRIKES);
        scriptEvents.removeEvent(EVENT_RUIA_GRIEVOUS_THRASH);

        scriptEvents.addEvent(EVENT_RUIA_WARDENS_WRATH, 1500);
        scriptEvents.addEvent(EVENT_RUIA_LIGHTFIRE, 6000);
        scriptEvents.addEvent(EVENT_RUIA_LIGHTFALL, 12000);
    }

    void takeBearForm()
    {
        mBearForm = true;
        _setMeleeDisabled(false);
        warn("Lightwarden Ruia shifts into Bear Form!");

        scriptEvents.removeEvent(EVENT_RUIA_WARDENS_WRATH);
        scriptEvents.removeEvent(EVENT_RUIA_LIGHTFIRE);
        scriptEvents.removeEvent(EVENT_RUIA_LIGHTFALL);

        scriptEvents.addEvent(EVENT_RUIA_MANGLING_CLAWS, 2000);
        scriptEvents.addEvent(EVENT_RUIA_PULVERIZING_STRIKES, 7000);
        scriptEvents.addEvent(EVENT_RUIA_GRIEVOUS_THRASH, 11000);
    }

    bool mBearForm = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ziekket

class TheBlindingValeZiekketAI : public TheBlindingValeBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheBlindingValeZiekketAI(c); }
    explicit TheBlindingValeZiekketAI(Creature* pCreature) : TheBlindingValeBossAI(pCreature, DATA_ZIEKKET) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_ZIEKKET_OOZING_XYLEM, true);
        scriptEvents.addEvent(EVENT_ZIEKKET_THORNSPIKE, 6000);
        scriptEvents.addEvent(EVENT_ZIEKKET_AWAKEN_THE_LIGHTBLOOM, 10000);
        scriptEvents.addEvent(EVENT_ZIEKKET_CONCENTRATED_LIGHTBEAM, 20000);
    }

    void onReset() override { _removeAura(SPELL_ZIEKKET_OOZING_XYLEM); }

    void onDefeat() override { _removeAura(SPELL_ZIEKKET_OOZING_XYLEM); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ZIEKKET_THORNSPIKE:
                castSpellOnVictim(SPELL_ZIEKKET_THORNSPIKE);
                scriptEvents.addEvent(EVENT_ZIEKKET_THORNSPIKE, 13000);
                break;
            case EVENT_ZIEKKET_AWAKEN_THE_LIGHTBLOOM:
                awakenLightbloom();
                scriptEvents.addEvent(EVENT_ZIEKKET_AWAKEN_THE_LIGHTBLOOM, 38000);
                break;
            case EVENT_ZIEKKET_CONCENTRATED_LIGHTBEAM:
                fireLightbeam();
                scriptEvents.addEvent(EVENT_ZIEKKET_CONCENTRATED_LIGHTBEAM, 28000);
                break;
            default:
                break;
        }
    }

private:
    std::vector<Creature*> lashersAround()
    {
        std::vector<Creature*> lashers;
        uint32_t const entries[2] = { NPC_ZIEKKET_LIGHTSPAWN_LASHER, NPC_ZIEKKET_LIGHTSPAWN_LASHER_ALT };
        for (uint32_t entry : entries)
        {
            std::list<Creature*> found;
            GetCreatureListWithEntryInGrid(found, entry, 100.0f);
            for (Creature* pLasher : found)
            {
                if (pLasher->isAlive())
                    lashers.push_back(pLasher);
            }
        }

        return lashers;
    }

    static bool isDormant(Creature* pLasher)
    {
        return pLasher->GetScript() != nullptr && pLasher->GetScript()->GetCreatureData(CREATURE_DATA_LASHER_DORMANT) != 0;
    }

    // dormant lashers rise again and fresh ones sprout
    void awakenLightbloom()
    {
        warn("Ziekket Awakens the Lightbloom!");
        castSpellOnSelf(SPELL_ZIEKKET_AWAKEN_THE_LIGHTBLOOM);

        for (Creature* pLasher : lashersAround())
        {
            if (isDormant(pLasher))
                pLasher->GetScript()->DoAction(ACTION_LASHER_AWAKEN);
        }

        for (uint8_t i = 0; i < LashersPerAwakening; ++i)
            callAdd(NPC_ZIEKKET_LIGHTSPAWN_LASHER, aroundSelf(8.0f, 18.0f));
    }

    // dormant lashers caught in the beam turn into Lightsap
    void fireLightbeam()
    {
        Unit* pTarget = anyPlayer();
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, SPELL_ZIEKKET_CONCENTRATED_LIGHTBEAM);

        for (Creature* pLasher : lashersAround())
        {
            if (!isDormant(pLasher) || pLasher->getDistance(pTarget) > LightbeamReach)
                continue;

            getCreature()->castSpellLoc(pLasher->GetPosition(), SPELL_ZIEKKET_LIGHTSAP, true);
            pLasher->despawn(500, 0);
        }
    }
};

// cannot be killed, falls Dormant instead until reawakened or vaporized
class TheBlindingValeLasherAI : public TheBlindingValeCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TheBlindingValeLasherAI(c); }
    explicit TheBlindingValeLasherAI(Creature* pCreature) : TheBlindingValeCombatAI(pCreature) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mDormant)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        fallDormant();
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_LASHER_AWAKEN)
            reawaken();
    }

    uint32_t GetCreatureData(uint32_t type) const override
    {
        return type == CREATURE_DATA_LASHER_DORMANT && mDormant ? 1 : 0;
    }

protected:
    void onEngage() override
    {
        if (!mDormant)
            scriptEvents.addEvent(EVENT_LASHER_LIGHTSPORE_SHOT, 2000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_LASHER_LIGHTSPORE_SHOT || mDormant)
            return;

        castAt(anyPlayer(), SPELL_LASHER_LIGHTSPORE_SHOT);
        scriptEvents.addEvent(EVENT_LASHER_LIGHTSPORE_SHOT, 4000);
    }

private:
    void fallDormant()
    {
        mDormant = true;
        scriptEvents.resetEvents();
        getCreature()->interruptSpell();
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_LASHER_DORMANT, true);
    }

    void reawaken()
    {
        if (!mDormant || !isAlive())
            return;

        mDormant = false;
        _removeAura(SPELL_LASHER_DORMANT);
        getCreature()->setFullHealth();
        castSpellOnSelf(SPELL_LASHER_VICIOUS_REGROWTH, true);

        _setMeleeDisabled(false);
        setReactState(REACT_AGGRESSIVE);
        setZoneWideCombat();
        scriptEvents.addEvent(EVENT_LASHER_LIGHTSPORE_SHOT, 2000);
    }

    bool mDormant = false;
};

void SetupTheBlindingVale(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_BLINDING_VALE, &TheBlindingValeInstanceScript::Create);

    mgr->register_creature_script(BOSS_TRIO_MEITTIK, &TheBlindingValeMeittikAI::Create);
    mgr->register_creature_script(BOSS_TRIO_KEZKITT, &TheBlindingValeKezkittAI::Create);
    mgr->register_creature_script(BOSS_TRIO_LEKSHI, &TheBlindingValeLekshiAI::Create);
    mgr->register_creature_script(NPC_TRIO_LIGHTBLOSSOM, &TheBlindingValeLightblossomAI::Create);

    mgr->register_creature_script(BOSS_IKUZZ, &TheBlindingValeIkuzzAI::Create);
    mgr->register_creature_script(NPC_IKUZZ_BLOODTHORN_ROOTS, &TheBlindingValeBloodthornRootsAI::Create);
    mgr->register_creature_script(NPC_IKUZZ_BLOODTHORN_ROOTS_ALT, &TheBlindingValeBloodthornRootsAI::Create);

    mgr->register_creature_script(BOSS_LIGHTWARDEN_RUIA, &TheBlindingValeRuiaAI::Create);
    mgr->register_creature_script(BOSS_LIGHTWARDEN_RUIA_ALT1, &TheBlindingValeRuiaAI::Create);
    mgr->register_creature_script(BOSS_LIGHTWARDEN_RUIA_ALT2, &TheBlindingValeRuiaAI::Create);
    mgr->register_creature_script(BOSS_LIGHTWARDEN_RUIA_ALT3, &TheBlindingValeRuiaAI::Create);

    mgr->register_creature_script(BOSS_ZIEKKET, &TheBlindingValeZiekketAI::Create);
    mgr->register_creature_script(NPC_ZIEKKET_LIGHTSPAWN_LASHER, &TheBlindingValeLasherAI::Create);
    mgr->register_creature_script(NPC_ZIEKKET_LIGHTSPAWN_LASHER_ALT, &TheBlindingValeLasherAI::Create);
}
