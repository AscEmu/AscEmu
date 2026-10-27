/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Windrunner_Spire.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <map>
#include <vector>

enum WindrunnerSpireEvents
{
    EVENT_EMBERDAWN_SEARING_BEAK = 1,
    EVENT_EMBERDAWN_FLAMING_UPDRAFT,
    EVENT_EMBERDAWN_EMBERS_DROP,
    EVENT_EMBERDAWN_BURNING_GALE,

    EVENT_KALIS_SHADOW_BOLT,

    EVENT_LATCH_BONE_HACK,
    EVENT_LATCH_SPLATTERING_SPEW,
    EVENT_LATCH_HEAVING_CHOP,

    EVENT_KROLUK_RAMPAGE,
    EVENT_KROLUK_RECKLESS_LEAP,
    EVENT_KROLUK_BLADESTORM,

    EVENT_WARPARTY_ABILITY,

    EVENT_HEART_TEMPEST_SLASH,
    EVENT_HEART_ARROW_RAIN,
    EVENT_HEART_BOLT_GALE
};

namespace
{
    float const SpireTwoPi = 6.2831853f;

    // Kroluk calls his warparty at these health marks
    int32_t const KrolukBellowMarks[2] = { 66, 33 };

    uint32_t const DuoMembers[2] = { BOSS_KALIS, BOSS_LATCH };

    size_t const EmberdawnMaxEmbers = 8;

    LocationVector spireRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(SpireTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    bool isWarpartyMember(uint32_t entry)
    {
        return entry == NPC_HAUNTING_GRUNT || entry == NPC_HAUNTING_GRUNT_ALT || entry == NPC_SPECTRAL_AXETHROWER || entry == NPC_SPECTRAL_AXETHROWER_ALT;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class WindrunnerSpireInstanceScript : public InstanceScript
{
public:
    explicit WindrunnerSpireInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(WindrunnerSpireEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new WindrunnerSpireInstanceScript(pMapMgr); }

    // Kalis or Latch alone must not end the Derelict Duo
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (uint32_t entry : DuoMembers)
        {
            if (pCreature->getEntry() == entry)
                mDuoGuids[entry] = pCreature->GetNewGUID();
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id != DATA_DERELICT_DUO || state != EncounterStates::Failed)
            return;

        // a wiped duo fights again in full
        for (Creature* pMember : getDuo())
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getDuo()
    {
        std::vector<Creature*> duo;
        for (auto const& member : mDuoGuids)
        {
            if (Creature* pMember = getCreatureByGuid(member.second))
                duo.push_back(pMember);
        }

        return duo;
    }

    // single boss encounters are down with their boss
    bool isEncounterDown(uint32_t dataId, Creature* pExcept)
    {
        if (dataId != DATA_DERELICT_DUO)
            return true;

        for (Creature* pMember : getDuo())
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    bool isDuoFighting(Creature* pExcept)
    {
        for (Creature* pMember : getDuo())
        {
            if (pMember != pExcept && pMember->isAlive() && pMember->isInCombat())
                return true;
        }

        return false;
    }

    // pulling Kalis pulls Latch and the other way round
    void rallyDuo(CreatureAIScript* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getDuo())
        {
            if (pMember == pPuller->getCreature() || !pMember->isAlive() || pMember->isInCombat())
                continue;

            pPuller->setZoneWideCombat(pMember);
        }
        mRallying = false;
    }

    void notifyDuo(Creature* pSender, int32_t action)
    {
        for (Creature* pMember : getDuo())
        {
            if (pMember != pSender && pMember->isAlive() && pMember->GetScript() != nullptr)
                pMember->GetScript()->DoAction(action);
        }
    }

private:
    std::map<uint32_t, WoWGuid> mDuoGuids;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock, energy bar and target helpers

class WindrunnerSpireCombatAI : public CreatureAIScript
{
public:
    explicit WindrunnerSpireCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    WindrunnerSpireInstanceScript* getSpire()
    {
        if (getCreature()->GetMapId() != MAP_WINDRUNNER_SPIRE)
            return nullptr;

        return static_cast<WindrunnerSpireInstanceScript*>(getInstanceScript());
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

    LocationVector aroundSelf(float minRange, float maxRange) { return spireRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

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
// Encounter base: frame, boss state and duo pull

class WindrunnerSpireBossAI : public WindrunnerSpireCombatAI
{
public:
    WindrunnerSpireBossAI(Creature* pCreature, uint32_t dataId) : WindrunnerSpireCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (mDataId == DATA_DERELICT_DUO)
        {
            if (WindrunnerSpireInstanceScript* pSpire = getSpire())
                pSpire->rallyDuo(this);
        }

        WindrunnerSpireCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the partner has left the fight as well
                WindrunnerSpireInstanceScript* pSpire = getSpire();
                if (mDataId != DATA_DERELICT_DUO || pSpire == nullptr || !pSpire->isDuoFighting(getCreature()))
                    pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        WindrunnerSpireCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            WindrunnerSpireInstanceScript* pSpire = getSpire();
            if (pSpire == nullptr || pSpire->isEncounterDown(mDataId, getCreature()))
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
// Emberdawn

class WindrunnerSpireEmberdawnAI : public WindrunnerSpireBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WindrunnerSpireEmberdawnAI(c); }
    explicit WindrunnerSpireEmberdawnAI(Creature* pCreature) : WindrunnerSpireBossAI(pCreature, DATA_EMBERDAWN) {}

protected:
    void onEngage() override
    {
        mUpdraftTargets.clear();
        mEmberSpots.clear();

        scriptEvents.addEvent(EVENT_EMBERDAWN_SEARING_BEAK, 6000);
        scriptEvents.addEvent(EVENT_EMBERDAWN_FLAMING_UPDRAFT, 11000);
        scriptEvents.addEvent(EVENT_EMBERDAWN_BURNING_GALE, 32000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_EMBERDAWN_SEARING_BEAK:
                castSpellOnVictim(SPELL_EMBERDAWN_SEARING_BEAK);
                scriptEvents.addEvent(EVENT_EMBERDAWN_SEARING_BEAK, 15000);
                break;
            case EVENT_EMBERDAWN_FLAMING_UPDRAFT:
                liftPlayers();
                scriptEvents.addEvent(EVENT_EMBERDAWN_FLAMING_UPDRAFT, 22000);
                break;
            case EVENT_EMBERDAWN_EMBERS_DROP:
                dropEmbers();
                break;
            case EVENT_EMBERDAWN_BURNING_GALE:
                warn("Emberdawn whips up a Burning Gale!");
                castSpellAOE(SPELL_EMBERDAWN_BURNING_GALE);
                igniteTwisters();
                scriptEvents.addEvent(EVENT_EMBERDAWN_BURNING_GALE, 45000);
                break;
            default:
                break;
        }
    }

private:
    // two players are caught in the updraft
    void liftPlayers()
    {
        for (uint8_t i = 0; i < 2; ++i)
        {
            Unit* pTarget = anyPlayer();
            if (pTarget == nullptr)
                break;

            castSpell(pTarget, SPELL_EMBERDAWN_FLAMING_UPDRAFT, true);
            mUpdraftTargets.push_back(pTarget->getGuid());
        }

        scriptEvents.addEvent(EVENT_EMBERDAWN_EMBERS_DROP, 6000);
    }

    // the updraft ends in a patch of embers below each victim
    void dropEmbers()
    {
        for (uint64_t guid : mUpdraftTargets)
        {
            Unit* pTarget = getCreature()->getWorldMapUnit(guid);
            if (pTarget == nullptr || !pTarget->isAlive())
                continue;

            getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_EMBERDAWN_IGNITED_EMBERS, true);
            if (mEmberSpots.size() < EmberdawnMaxEmbers)
                mEmberSpots.push_back(pTarget->GetPosition());
        }

        mUpdraftTargets.clear();
    }

    // the gale turns every ember patch into a twister
    void igniteTwisters()
    {
        for (LocationVector const& spot : mEmberSpots)
            getCreature()->castSpellLoc(spot, SPELL_EMBERDAWN_FLAMING_TWISTERS, true);

        mEmberSpots.clear();
    }

    std::vector<uint64_t> mUpdraftTargets;
    std::vector<LocationVector> mEmberSpots;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Derelict Duo

class WindrunnerSpireDuoMemberAI : public WindrunnerSpireBossAI
{
public:
    explicit WindrunnerSpireDuoMemberAI(Creature* pCreature) : WindrunnerSpireBossAI(pCreature, DATA_DERELICT_DUO) {}

    void DoAction(int32_t action) override
    {
        // the survivor fights on empowered
        if (action == ACTION_DUO_PARTNER_FALLEN && isAlive() && _isInCombat())
            castSpellOnSelf(SPELL_DUO_BROKEN_BOND, true);
    }

protected:
    void onReset() override { _removeAura(SPELL_DUO_BROKEN_BOND); }

    void onDefeat() override
    {
        if (WindrunnerSpireInstanceScript* pSpire = getSpire())
            pSpire->notifyDuo(getCreature(), ACTION_DUO_PARTNER_FALLEN);
    }
};

class WindrunnerSpireKalisAI : public WindrunnerSpireDuoMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WindrunnerSpireKalisAI(c); }
    explicit WindrunnerSpireKalisAI(Creature* pCreature) : WindrunnerSpireDuoMemberAI(pCreature) {}

protected:
    void onEngage() override
    {
        armEnergy(32000);
        scriptEvents.addEvent(EVENT_KALIS_SHADOW_BOLT, 2000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_KALIS_SHADOW_BOLT)
            return;

        castAt(anyPlayer(), SPELL_KALIS_SHADOW_BOLT);
        scriptEvents.addEvent(EVENT_KALIS_SHADOW_BOLT, 3500);
    }

    // channelled until interrupted
    void onEnergyFull() override
    {
        warn("Kalis wails a Debilitating Shriek!");
        castSpellOnSelf(SPELL_KALIS_DEBILITATING_SHRIEK);
    }
};

class WindrunnerSpireLatchAI : public WindrunnerSpireDuoMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WindrunnerSpireLatchAI(c); }
    explicit WindrunnerSpireLatchAI(Creature* pCreature) : WindrunnerSpireDuoMemberAI(pCreature) {}

protected:
    void onEngage() override
    {
        mYankGuid = 0;
        armEnergy(28000);
        scriptEvents.addEvent(EVENT_LATCH_BONE_HACK, 6000);
        scriptEvents.addEvent(EVENT_LATCH_SPLATTERING_SPEW, 10000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LATCH_BONE_HACK:
                castSpellOnVictim(SPELL_LATCH_BONE_HACK);
                scriptEvents.addEvent(EVENT_LATCH_BONE_HACK, 14000);
                break;
            case EVENT_LATCH_SPLATTERING_SPEW:
                spewGunk();
                scriptEvents.addEvent(EVENT_LATCH_SPLATTERING_SPEW, 18000);
                break;
            case EVENT_LATCH_HEAVING_CHOP:
                if (Unit* pTarget = getCreature()->getWorldMapUnit(mYankGuid))
                {
                    if (pTarget->isAlive())
                        castSpell(pTarget, SPELL_LATCH_HEAVING_CHOP);
                }
                mYankGuid = 0;
                break;
            default:
                break;
        }
    }

    // yank a player in and chop down on them
    void onEnergyFull() override
    {
        Unit* pTarget = anyPlayerButTank();
        if (pTarget == nullptr)
            return;

        warn("Latch tries to drag a victim in with Heaving Yank!");
        castSpell(pTarget, SPELL_LATCH_HEAVING_YANK);
        mYankGuid = pTarget->getGuid();
        scriptEvents.addEvent(EVENT_LATCH_HEAVING_CHOP, 2500);
    }

private:
    void spewGunk()
    {
        castSpellAOE(SPELL_LATCH_SPLATTERING_SPEW);
        for (uint8_t i = 0; i < 2; ++i)
            castAt(anyPlayer(), SPELL_LATCH_GUNK_SPLATTER, true);
    }

    uint64_t mYankGuid = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Commander Kroluk, shielded while his warparty stands

class WindrunnerSpireKrolukAI : public WindrunnerSpireBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WindrunnerSpireKrolukAI(c); }
    explicit WindrunnerSpireKrolukAI(Creature* pCreature) : WindrunnerSpireBossAI(pCreature, DATA_COMMANDER_KROLUK) {}

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || !isWarpartyMember(pSummon->getEntry()) || mWarpartyLeft == 0)
            return;

        if (--mWarpartyLeft == 0)
            dropShieldWall();
    }

protected:
    void onEngage() override
    {
        mBellows = 0;
        mWarpartyLeft = 0;
        scriptEvents.addEvent(EVENT_KROLUK_RAMPAGE, 8000);
        scriptEvents.addEvent(EVENT_KROLUK_RECKLESS_LEAP, 14000);
    }

    void onReset() override
    {
        mWarpartyLeft = 0;
        _removeAura(SPELL_KROLUK_SHIELD_WALL);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mBellows >= 2 || _getHealthPercent() > KrolukBellowMarks[mBellows])
            return;

        ++mBellows;
        callWarparty();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KROLUK_RAMPAGE:
                castSpellOnVictim(SPELL_KROLUK_RAMPAGE);
                scriptEvents.addEvent(EVENT_KROLUK_RAMPAGE, 20000);
                break;
            case EVENT_KROLUK_RECKLESS_LEAP:
                leapAtPlayer();
                scriptEvents.addEvent(EVENT_KROLUK_RECKLESS_LEAP, 24000);
                break;
            case EVENT_KROLUK_BLADESTORM:
                if (mWarpartyLeft == 0)
                    break;
                castSpellOnSelf(SPELL_KROLUK_BLADESTORM);
                scriptEvents.addEvent(EVENT_KROLUK_BLADESTORM, 18000);
                break;
            default:
                break;
        }
    }

private:
    void leapAtPlayer()
    {
        Unit* pTarget = anyPlayerButTank();
        if (pTarget == nullptr)
            return;

        castSpellOnSelf(SPELL_KROLUK_RECKLESS_LEAP, true);
        moveJump(pTarget->GetPosition(), 25.0f, 12.0f);
    }

    // two grunts and two axethrowers rally to the commander
    void callWarparty()
    {
        warn("Commander Kroluk lets out a Rallying Bellow!");
        castSpellOnSelf(SPELL_KROLUK_RALLYING_BELLOW, true);

        uint32_t const members[4] = { NPC_HAUNTING_GRUNT, NPC_HAUNTING_GRUNT, NPC_SPECTRAL_AXETHROWER, NPC_SPECTRAL_AXETHROWER };
        for (uint32_t entry : members)
        {
            if (callAdd(entry, aroundSelf(6.0f, 12.0f)) != nullptr)
                ++mWarpartyLeft;
        }

        if (mWarpartyLeft == 0)
            return;

        castSpellOnSelf(SPELL_KROLUK_SHIELD_WALL, true);
        scriptEvents.addEvent(EVENT_KROLUK_BLADESTORM, 4000);
    }

    void dropShieldWall()
    {
        warn("Commander Kroluk's warparty has fallen, his Shield Wall breaks!");
        _removeAura(SPELL_KROLUK_SHIELD_WALL);
        scriptEvents.removeEvent(EVENT_KROLUK_BLADESTORM);
    }

    uint8_t mBellows = 0;
    uint32_t mWarpartyLeft = 0;
};

// warparty member with one timed ability
class WindrunnerSpireWarpartyAI : public WindrunnerSpireCombatAI
{
public:
    static CreatureAIScript* CreateGrunt(Creature* c) { return new WindrunnerSpireWarpartyAI(c, SPELL_GRUNT_INTERCEPTING_CHARGE, 3000, 15000, true); }
    static CreatureAIScript* CreateAxethrower(Creature* c) { return new WindrunnerSpireWarpartyAI(c, SPELL_AXETHROWER_THROW_AXE, 2000, 6000, false); }

    WindrunnerSpireWarpartyAI(Creature* pCreature, uint32_t spellId, uint32_t firstMs, uint32_t repeatMs, bool skipTank)
        : WindrunnerSpireCombatAI(pCreature), mSpellId(spellId), mFirstMs(firstMs), mRepeatMs(repeatMs), mSkipTank(skipTank) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_WARPARTY_ABILITY, mFirstMs); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_WARPARTY_ABILITY)
            return;

        castAt(mSkipTank ? anyPlayerButTank() : anyPlayer(), mSpellId);
        scriptEvents.addEvent(EVENT_WARPARTY_ABILITY, mRepeatMs);
    }

private:
    uint32_t mSpellId;
    uint32_t mFirstMs;
    uint32_t mRepeatMs;
    bool mSkipTank;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Restless Heart

class WindrunnerSpireRestlessHeartAI : public WindrunnerSpireBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WindrunnerSpireRestlessHeartAI(c); }
    explicit WindrunnerSpireRestlessHeartAI(Creature* pCreature) : WindrunnerSpireBossAI(pCreature, DATA_RESTLESS_HEART) {}

protected:
    void onEngage() override
    {
        armEnergy(35000);
        scriptEvents.addEvent(EVENT_HEART_TEMPEST_SLASH, 7000);
        scriptEvents.addEvent(EVENT_HEART_ARROW_RAIN, 12000);
        scriptEvents.addEvent(EVENT_HEART_BOLT_GALE, 18000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HEART_TEMPEST_SLASH:
                castSpellOnVictim(SPELL_HEART_TEMPEST_SLASH);
                scriptEvents.addEvent(EVENT_HEART_TEMPEST_SLASH, 16000);
                break;
            case EVENT_HEART_ARROW_RAIN:
                castSpellOnSelf(SPELL_HEART_ARROW_RAIN);
                scriptEvents.addEvent(EVENT_HEART_ARROW_RAIN, 24000);
                break;
            case EVENT_HEART_BOLT_GALE:
                castAt(anyPlayer(), SPELL_HEART_BOLT_GALE);
                scriptEvents.addEvent(EVENT_HEART_BOLT_GALE, 26000);
                break;
            default:
                break;
        }
    }

    // wind-infused arrow, the ring of Billowing Wind expands from the impact
    void onEnergyFull() override
    {
        warn("The Restless Heart looses a Bullseye Windblast!");
        castUnder(anyPlayer(), SPELL_HEART_BULLSEYE_WINDBLAST);
    }
};

void SetupWindrunnerSpire(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_WINDRUNNER_SPIRE, &WindrunnerSpireInstanceScript::Create);

    mgr->register_creature_script(BOSS_EMBERDAWN, &WindrunnerSpireEmberdawnAI::Create);

    mgr->register_creature_script(BOSS_KALIS, &WindrunnerSpireKalisAI::Create);
    mgr->register_creature_script(BOSS_LATCH, &WindrunnerSpireLatchAI::Create);

    mgr->register_creature_script(BOSS_COMMANDER_KROLUK, &WindrunnerSpireKrolukAI::Create);
    mgr->register_creature_script(NPC_HAUNTING_GRUNT, &WindrunnerSpireWarpartyAI::CreateGrunt);
    mgr->register_creature_script(NPC_HAUNTING_GRUNT_ALT, &WindrunnerSpireWarpartyAI::CreateGrunt);
    mgr->register_creature_script(NPC_SPECTRAL_AXETHROWER, &WindrunnerSpireWarpartyAI::CreateAxethrower);
    mgr->register_creature_script(NPC_SPECTRAL_AXETHROWER_ALT, &WindrunnerSpireWarpartyAI::CreateAxethrower);

    mgr->register_creature_script(BOSS_RESTLESS_HEART, &WindrunnerSpireRestlessHeartAI::Create);
}
