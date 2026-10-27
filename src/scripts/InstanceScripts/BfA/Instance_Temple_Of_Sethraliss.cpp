/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Temple_Of_Sethraliss.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

enum TempleOfSethralissEvents
{
    EVENT_ADDERIS_GUST = 1,
    EVENT_ADDERIS_ARCING_BLADE,
    EVENT_ADDERIS_PEAL_OF_THUNDER,
    EVENT_ADDERIS_ARC_DASH,
    EVENT_ASPIX_JOLT,
    EVENT_ASPIX_CONDUCTION,
    EVENT_ASPIX_CYCLONE_STRIKE,

    EVENT_MEREKTHA_NOXIOUS_BREATH,
    EVENT_MEREKTHA_BLINDING_SAND,
    EVENT_MEREKTHA_BURROW,
    EVENT_MEREKTHA_SURFACE,

    EVENT_GALVAZZT_ENERGY_CORES,

    EVENT_AVATAR_HEART_GUARDIAN,
    EVENT_AVATAR_PLAGUE_TOADS,
    EVENT_AVATAR_PLAGUE_DOCTOR,

    EVENT_HEXER_LAVA_BURST,
    EVENT_HEXER_FLAME_SHOCK
};

namespace
{
    struct SethralissBarrier
    {
        uint32_t entry;
        uint32_t encounterId;
    };

    // barriers seal each arena while its fight runs
    SethralissBarrier const SethralissBarriers[] =
    {
        { GO_ADDERIS_ASPIX_BARRIER, DATA_ADDERIS_AND_ASPIX },
        { GO_MEREKTHA_BARRIER,      DATA_MEREKTHA },
        { GO_GALVAZZT_BARRIER,      DATA_GALVAZZT }
    };

    SethralissBarrier const* findSethralissBarrier(uint32_t entry)
    {
        for (SethralissBarrier const& barrier : SethralissBarriers)
        {
            if (barrier.entry == entry)
                return &barrier;
        }

        return nullptr;
    }

    float const SethralissPi = 3.14159265f;

    // twins: charge gained every three seconds while shielded
    uint32_t const TwinChargeTickMs = 3000;
    uint32_t const TwinChargeCap = 100;
    uint32_t const TwinChargeSpecial = 50;
    uint32_t const AdderisArcDashes = 3;

    // Merektha hides below the sand, the strikers come from the dunes
    LocationVector const MerekthaHideout(3557.0f, 3422.0f, 40.0f, 3.3f);
    LocationVector const StrikerDunes(3550.0f, 3447.0f, 51.0f, 4.5f);
    uint32_t const StrikerWaveSize = 6;
    uint32_t const MerekthaBurrowMs = 30000;

    uint32_t const GalvazztChargeCap = 100;
    uint32_t const GalvazztCoresPerCall = 3;
    float const CoreGroundingRange = 5.0f;

    // heart chamber of the Avatar
    LocationVector const HeartChamberGate(4140.0f, 3659.0f, -43.0f, 0.59f);
    LocationVector const HeartChamberMiddle(4161.0f, 3673.0f, -34.0f, 3.69f);
    uint32_t const PlagueToadsPerWave = 6;
    float const AvatarRitualReach = 40.0f;
    float const AvatarWipeReach = 120.0f;

    LocationVector ringPoint(LocationVector const& centre, float maxRange)
    {
        float const angle = Util::getRandomFloat(0.0f, 2.0f * SethralissPi);
        float const range = Util::getRandomFloat(0.0f, maxRange);
        return LocationVector(centre.x + range * std::cos(angle), centre.y + range * std::sin(angle), centre.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class SethralissInstanceScript : public InstanceScript
{
public:
    explicit SethralissInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(TempleOfSethralissEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new SethralissInstanceScript(pMapMgr); }

    // the first twin to fall must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (id == DATA_ADDERIS_AND_ASPIX && state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !areTwinsDown(nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_ADDERIS:
            case BOSS_ASPIX:
            case BOSS_GALVAZZT:
            case NPC_AVATAR_OF_SETHRALISS:
                mTrackedGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
                break;
            case NPC_HOODOO_HEXER:
                mHexerGuids.insert(pCreature->GetNewGUID());
                break;
            default:
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findSethralissBarrier(pGameObject->getEntry()) == nullptr)
            return;

        mBarrierGuids.push_back(pGameObject->GetNewGUID());
        alignBarrier(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mBarrierGuids)
        {
            if (GameObject* pBarrier = getGameObjectByGuid(guid))
                alignBarrier(pBarrier);
        }

        // both twins return after a wipe
        if (id == DATA_ADDERIS_AND_ASPIX && state == EncounterStates::Failed)
        {
            for (Creature* pTwin : getTwins())
            {
                if (!pTwin->isAlive())
                    pTwin->despawn(1000, 5000);
            }
        }
    }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getTwins()
    {
        std::vector<Creature*> twins;
        for (uint32_t entry : { BOSS_ADDERIS, BOSS_ASPIX })
        {
            if (Creature* pTwin = getTracked(entry))
                twins.push_back(pTwin);
        }

        return twins;
    }

    bool areTwinsDown(Creature* pExcept)
    {
        for (Creature* pTwin : getTwins())
        {
            if (pTwin != pExcept && pTwin->isAlive())
                return false;
        }

        return true;
    }

    std::vector<Creature*> getHexers()
    {
        std::vector<Creature*> hexers;
        for (WoWGuid const& guid : mHexerGuids)
        {
            if (Creature* pHexer = getCreatureByGuid(guid))
                hexers.push_back(pHexer);
        }

        return hexers;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        Creature* pTarget = getTracked(entry);
        if (pTarget != nullptr && pTarget->isAlive() && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }

private:
    void alignBarrier(GameObject* pBarrier)
    {
        SethralissBarrier const* barrier = findSethralissBarrier(pBarrier->getEntry());
        if (barrier == nullptr)
            return;

        uint8_t const wantState = getBossState(barrier->encounterId) == EncounterStates::InProgress ? GO_STATE_CLOSED : GO_STATE_OPEN;
        if (pBarrier->getState() != wantState)
            pBarrier->setState(wantState);
    }

    std::map<uint32_t, WoWGuid> mTrackedGuids;
    std::set<WoWGuid> mHexerGuids;
    std::vector<WoWGuid> mBarrierGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class SethralissCombatAI : public CreatureAIScript
{
public:
    explicit SethralissCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    SethralissInstanceScript* getTemple()
    {
        if (getCreature()->GetMapId() != MAP_TEMPLE_OF_SETHRALISS)
            return nullptr;

        return static_cast<SethralissInstanceScript*>(getInstanceScript());
    }

    // random player, the tank only when nobody else is around
    Unit* pickPlayer(bool skipTank = false)
    {
        if (skipTank)
        {
            FilterArgs args(TargetFilter_Player);
            args.addFilter(TargetFilter_NotCurrent);
            if (Unit* pTarget = selectUnitTarget(args))
                return pTarget;
        }

        return selectUnitTarget(FilterArgs(TargetFilter_Player));
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        Unit* pTarget = pickPlayer(skipTank);
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

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class SethralissBossAI : public SethralissCombatAI
{
public:
    SethralissBossAI(Creature* pCreature, uint32_t encounterId) : SethralissCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        SethralissCombatAI::OnCombatStart(pTarget);
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

        SethralissCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            if (isEncounterOver())
                pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }

        onDefeat();
    }

protected:
    virtual bool isEncounterOver() { return true; }
    virtual void onDefeat() {}

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adderis and Aspix, the lightning shield passes between them

class SethralissTwinAI : public SethralissBossAI
{
public:
    SethralissTwinAI(Creature* pCreature, uint32_t partnerEntry, bool shieldedOnPull) : SethralissBossAI(pCreature, DATA_ADDERIS_AND_ASPIX), mPartnerEntry(partnerEntry), mShieldedOnPull(shieldedOnPull) {}

    void OnCombatStart(Unit* pTarget) override
    {
        SethralissBossAI::OnCombatStart(pTarget);

        if (SethralissInstanceScript* pTemple = getTemple())
        {
            Creature* pPartner = pTemple->getTracked(mPartnerEntry);
            if (pPartner != nullptr && pPartner->isAlive() && !pPartner->isInCombat())
                setZoneWideCombat(pPartner);
        }
    }

    // nothing gets through the lightning shield
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage != nullptr && mShielded)
            *damage = 0;
    }

    void DoAction(int32_t action) override
    {
        if (!_isInCombat())
            return;

        if (action == ACTION_TWIN_TAKE_SHIELD)
            raiseShield();
        else if (action == ACTION_TWIN_PARTNER_FELL)
            lowerShield();
    }

protected:
    void onEngage() override
    {
        mShielded = false;
        mCharge = 0;
        mChargeClock = 0;
        mSpecialArmed = false;

        if (mShieldedOnPull)
            raiseShield();

        onTwinEngage();
    }

    void onReset() override { lowerShield(); }

    void onCombatTick(uint32_t diff) override
    {
        if (!mShielded)
            return;

        mChargeClock += diff;
        while (mChargeClock >= TwinChargeTickMs)
        {
            mChargeClock -= TwinChargeTickMs;
            mCharge = std::min(TwinChargeCap, mCharge + Util::getRandomUInt(1, 5));
        }

        if (!mSpecialArmed && mCharge >= TwinChargeSpecial)
        {
            mSpecialArmed = true;
            onChargeHalfway();
        }

        if (mCharge >= TwinChargeCap && !_isCasting())
            handOverShield();
    }

    bool isEncounterOver() override
    {
        SethralissInstanceScript* pTemple = getTemple();
        return pTemple == nullptr || pTemple->areTwinsDown(getCreature());
    }

    // the survivor can be hurt again
    void onDefeat() override
    {
        if (SethralissInstanceScript* pTemple = getTemple())
            pTemple->sendAction(mPartnerEntry, ACTION_TWIN_PARTNER_FELL);
    }

    virtual void onTwinEngage() {}
    virtual void onChargeHalfway() {}
    virtual void onShieldHandedOver() {}

    bool mShielded = false;

private:
    void raiseShield()
    {
        mShielded = true;
        mCharge = 0;
        mChargeClock = 0;
        mSpecialArmed = false;
        castSpellOnSelf(SPELL_TWIN_LIGHTNING_SHIELD, true);
        setRooted(true);
    }

    void lowerShield()
    {
        mShielded = false;
        mSpecialArmed = false;
        _removeAura(SPELL_TWIN_LIGHTNING_SHIELD);
        setRooted(false);
    }

    void handOverShield()
    {
        SethralissInstanceScript* pTemple = getTemple();
        Creature* pPartner = pTemple != nullptr ? pTemple->getTracked(mPartnerEntry) : nullptr;
        if (pPartner == nullptr || !pPartner->isAlive())
        {
            lowerShield();
            return;
        }

        lowerShield();
        mCharge = 0;
        pTemple->sendAction(mPartnerEntry, ACTION_TWIN_TAKE_SHIELD);
        onShieldHandedOver();
    }

    uint32_t mPartnerEntry;
    bool mShieldedOnPull;
    uint32_t mCharge = 0;
    uint32_t mChargeClock = 0;
    bool mSpecialArmed = false;
};

class SethralissAdderisAI : public SethralissTwinAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethralissAdderisAI(c); }
    explicit SethralissAdderisAI(Creature* pCreature) : SethralissTwinAI(pCreature, BOSS_ASPIX, false) {}

protected:
    void onTwinEngage() override
    {
        mDashesLeft = 0;
        scriptEvents.addEvent(EVENT_ADDERIS_GUST, 10000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_ADDERIS_ARCING_BLADE, 12000);
    }

    // half charged: thunder follows the party
    void onChargeHalfway() override { scriptEvents.addEvent(EVENT_ADDERIS_PEAL_OF_THUNDER, 10000); }

    void onShieldHandedOver() override
    {
        scriptEvents.removeEvent(EVENT_ADDERIS_PEAL_OF_THUNDER);
        warn("Adderis begins his Arc Dash!");
        mDashesLeft = AdderisArcDashes;
        scriptEvents.addEvent(EVENT_ADDERIS_ARC_DASH, 500);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ADDERIS_GUST:
                if (!mShielded)
                    castSpellOnVictim(SPELL_ADDERIS_GUST);
                scriptEvents.addEvent(EVENT_ADDERIS_GUST, 10000);
                break;
            case EVENT_ADDERIS_ARCING_BLADE:
                castSpellOnVictim(SPELL_ADDERIS_ARCING_BLADE);
                scriptEvents.addEvent(EVENT_ADDERIS_ARCING_BLADE, 12000);
                break;
            case EVENT_ADDERIS_PEAL_OF_THUNDER:
                if (!mShielded)
                    break;
                if (Unit* pTarget = pickPlayer())
                {
                    MoveTeleport(pTarget->GetPosition());
                    castSpell(pTarget, SPELL_ADDERIS_PEAL_OF_THUNDER);
                }
                scriptEvents.addEvent(EVENT_ADDERIS_PEAL_OF_THUNDER, 10000);
                break;
            case EVENT_ADDERIS_ARC_DASH:
                if (Unit* pTarget = pickPlayer())
                {
                    moveCharge(pTarget->GetPosition(), 40.0f);
                    castSpell(pTarget, SPELL_ADDERIS_ARC_DASH, true);
                }
                if (mDashesLeft > 0 && --mDashesLeft > 0)
                    scriptEvents.addEvent(EVENT_ADDERIS_ARC_DASH, 1500);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mDashesLeft = 0;
};

class SethralissAspixAI : public SethralissTwinAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethralissAspixAI(c); }
    explicit SethralissAspixAI(Creature* pCreature) : SethralissTwinAI(pCreature, BOSS_ADDERIS, true) {}

protected:
    void onTwinEngage() override
    {
        scriptEvents.addEvent(EVENT_ASPIX_JOLT, 5000);
        scriptEvents.addEvent(EVENT_ASPIX_CYCLONE_STRIKE, 25000);
    }

    // half charged: conduction jumps between players
    void onChargeHalfway() override { scriptEvents.addEvent(EVENT_ASPIX_CONDUCTION, 15000); }

    void onShieldHandedOver() override
    {
        scriptEvents.removeEvent(EVENT_ASPIX_CONDUCTION);
        warn("Aspix releases a Static Shock!");
        castSpellAOE(SPELL_ASPIX_STATIC_SHOCK);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ASPIX_JOLT:
                if (mShielded)
                    castAtPlayer(SPELL_ASPIX_JOLT);
                scriptEvents.addEvent(EVENT_ASPIX_JOLT, 5000);
                break;
            case EVENT_ASPIX_CONDUCTION:
                if (!mShielded)
                    break;
                castAtPlayer(SPELL_ASPIX_CONDUCTION);
                scriptEvents.addEvent(EVENT_ASPIX_CONDUCTION, 15000);
                break;
            case EVENT_ASPIX_CYCLONE_STRIKE:
                if (!mShielded)
                    castSpellOnSelf(SPELL_ASPIX_CYCLONE_STRIKE);
                scriptEvents.addEvent(EVENT_ASPIX_CYCLONE_STRIKE, 25000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Merektha, dives into the sand and sends strikers

class SethralissMerekthaAI : public SethralissBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethralissMerekthaAI(c); }
    explicit SethralissMerekthaAI(Creature* pCreature) : SethralissBossAI(pCreature, DATA_MEREKTHA) {}

protected:
    void onEngage() override
    {
        mBurrowCycle = false;
        mBurrowed = false;
        mWaveCount = 0;
        resumeAboveGround();
    }

    void onReset() override
    {
        if (!mBurrowed)
            return;

        mBurrowed = false;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        MoveTeleport(getCreature()->GetSpawnPosition());
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        // the dives begin once she is hurt badly enough
        if (!mBurrowCycle && _getHealthPercent() <= 60)
        {
            mBurrowCycle = true;
            scriptEvents.addEvent(EVENT_MEREKTHA_BURROW, 1000);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MEREKTHA_NOXIOUS_BREATH:
                castSpellOnVictim(SPELL_MEREKTHA_NOXIOUS_BREATH);
                scriptEvents.addEvent(EVENT_MEREKTHA_NOXIOUS_BREATH, 15000);
                break;
            case EVENT_MEREKTHA_BLINDING_SAND:
                castSpellAOE(SPELL_MEREKTHA_BLINDING_SAND);
                scriptEvents.addEvent(EVENT_MEREKTHA_BLINDING_SAND, 20000);
                break;
            case EVENT_MEREKTHA_BURROW:
                burrow();
                break;
            case EVENT_MEREKTHA_SURFACE:
                surface();
                break;
            default:
                break;
        }
    }

private:
    void resumeAboveGround()
    {
        scriptEvents.addEvent(EVENT_MEREKTHA_NOXIOUS_BREATH, 3000);
        scriptEvents.addEvent(EVENT_MEREKTHA_BLINDING_SAND, 8000);
    }

    void burrow()
    {
        warn("Merektha burrows into the sand!");
        castSpellAOE(SPELL_MEREKTHA_BURROW, true);

        mBurrowed = true;
        scriptEvents.removeEvent(EVENT_MEREKTHA_NOXIOUS_BREATH);
        scriptEvents.removeEvent(EVENT_MEREKTHA_BLINDING_SAND);
        attackStop();
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        MoveTeleport(MerekthaHideout);

        // waves alternate between the two striker kinds
        uint32_t const strikerEntry = mWaveCount++ % 2 == 0 ? NPC_SAND_CRUSTED_STRIKER : NPC_SAND_CRUSTED_STRIKER_ALT;
        for (uint32_t i = 0; i < StrikerWaveSize; ++i)
            callAdd(strikerEntry, ringPoint(StrikerDunes, 10.0f));

        scriptEvents.addEvent(EVENT_MEREKTHA_SURFACE, MerekthaBurrowMs);
    }

    void surface()
    {
        mBurrowed = false;
        MoveTeleport(getCreature()->GetSpawnPosition());
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);

        resumeAboveGround();
        scriptEvents.addEvent(EVENT_MEREKTHA_BURROW, MerekthaBurrowMs);
    }

    bool mBurrowCycle = false;
    bool mBurrowed = false;
    uint32_t mWaveCount = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Galvazzt, fed by energy cores nobody stands in front of

class SethralissGalvazztAI : public SethralissBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethralissGalvazztAI(c); }
    explicit SethralissGalvazztAI(Creature* pCreature) : SethralissBossAI(pCreature, DATA_GALVAZZT) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_GALVAZZT_CORE_CHARGE && _isInCombat() && mCharge < GalvazztChargeCap)
            ++mCharge;
    }

protected:
    void onEngage() override
    {
        mCharge = 0;
        scriptEvents.addEvent(EVENT_GALVAZZT_ENERGY_CORES, 15000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mCharge < GalvazztChargeCap || _isCasting())
            return;

        mCharge = 0;
        warn("Galvazzt is fully charged and casts Consume Charge!");
        castSpellAOE(SPELL_GALVAZZT_CONSUME_CHARGE);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_GALVAZZT_ENERGY_CORES)
            return;

        for (uint32_t i = 0; i < GalvazztCoresPerCall; ++i)
            summonCreature(NPC_ENERGY_CORE, ringPoint(getCreature()->GetPosition(), 20.0f), MANUAL_DESPAWN);

        scriptEvents.addEvent(EVENT_GALVAZZT_ENERGY_CORES, Util::getRandomUInt(15000, 30000));
    }

private:
    uint32_t mCharge = 0;
};

// grounds itself on a nearby player, otherwise charges Galvazzt
class SethralissEnergyCoreAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethralissEnergyCoreAI(c); }
    explicit SethralissEnergyCoreAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_CORE_ENERGY_CORE, true);
        castSpellOnSelf(SPELL_CORE_ARC, true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mPulseClock > diff)
        {
            mPulseClock -= diff;
            return;
        }

        mPulseClock = 1000;
        if (getCreature()->GetMapId() != MAP_TEMPLE_OF_SETHRALISS)
            return;

        SethralissInstanceScript* pTemple = static_cast<SethralissInstanceScript*>(getInstanceScript());
        Creature* pGalvazzt = pTemple != nullptr ? pTemple->getTracked(BOSS_GALVAZZT) : nullptr;
        if (pGalvazzt == nullptr || !pGalvazzt->isAlive() || !pGalvazzt->isInCombat())
            return;

        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && getCreature()->getDistance(pPlayer) <= CoreGroundingRange)
            {
                castSpell(pPlayer, SPELL_CORE_GALVANIZE, true);
                return;
            }
        }

        pTemple->sendAction(BOSS_GALVAZZT, ACTION_GALVAZZT_CORE_CHARGE);
    }

private:
    uint32_t mPulseClock = 1000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Avatar of Sethraliss, healed back to life while the hexers try to stop it

class SethralissAvatarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethralissAvatarAI(c); }
    explicit SethralissAvatarAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        setRooted(true);
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_AVATAR_RITUAL_BEGIN && !mRitual)
            beginRitual();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr || pInstance->getBossState(DATA_AVATAR_OF_SETHRALISS) == EncounterStates::Performed)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (!mRitual)
        {
            if (mWatchClock > diff)
            {
                mWatchClock -= diff;
                return;
            }

            mWatchClock = 1000;
            if (isPlayerNear(AvatarRitualReach))
                beginRitual();
            return;
        }

        scriptEvents.updateEvents(diff, 0);
        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);

        if (mWatchClock > diff)
        {
            mWatchClock -= diff;
            return;
        }

        mWatchClock = 2000;
        if (!isPlayerNear(AvatarWipeReach))
        {
            failRitual();
            return;
        }

        checkRestoration();
    }

private:
    SethralissInstanceScript* getTemple()
    {
        if (getCreature()->GetMapId() != MAP_TEMPLE_OF_SETHRALISS)
            return nullptr;

        return static_cast<SethralissInstanceScript*>(getInstanceScript());
    }

    bool isPlayerNear(float range)
    {
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer != nullptr && pPlayer->isAlive() && getCreature()->getDistance(pPlayer) <= range)
                return true;
        }

        return false;
    }

    // the Avatar starts drained, the party must heal it to full
    void beginRitual()
    {
        SethralissInstanceScript* pTemple = getTemple();
        if (pTemple == nullptr)
            return;

        mRitual = true;
        mWatchClock = 2000;
        mHexersRaisedLow = false;
        mHexersRaisedHigh = false;

        pTemple->setBossState(DATA_AVATAR_OF_SETHRALISS, EncounterStates::InProgress);
        pTemple->sendUnitEncounter(EncounterFrameEngage, getCreature());

        getCreature()->m_canRegenerateHP = false;
        getCreature()->setHealth(std::max<uint32_t>(1, getCreature()->getMaxHealth() / 10));
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "The Avatar of Sethraliss must be restored!");

        for (Creature* pHexer : pTemple->getHexers())
        {
            if (!pHexer->isAlive())
                continue;

            pTemple->sendUnitEncounter(EncounterFrameEngage, pHexer);
            if (!pHexer->isInCombat())
                setZoneWideCombat(pHexer);
        }

        scriptEvents.addEvent(EVENT_AVATAR_HEART_GUARDIAN, 5000);
        scriptEvents.addEvent(EVENT_AVATAR_PLAGUE_TOADS, 25000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_AVATAR_PLAGUE_DOCTOR, 10000);
    }

    void onEvent(uint32_t eventId)
    {
        switch (eventId)
        {
            case EVENT_AVATAR_HEART_GUARDIAN:
                callDefender(NPC_HEART_GUARDIAN, HeartChamberGate);
                scriptEvents.addEvent(EVENT_AVATAR_HEART_GUARDIAN, 15000);
                break;
            case EVENT_AVATAR_PLAGUE_TOADS:
                for (uint32_t i = 0; i < PlagueToadsPerWave; ++i)
                    callDefender(NPC_PLAGUE_TOAD, ringPoint(HeartChamberMiddle, 30.0f));
                scriptEvents.addEvent(EVENT_AVATAR_PLAGUE_TOADS, 25000);
                break;
            case EVENT_AVATAR_PLAGUE_DOCTOR:
                callDefender(NPC_PLAGUE_DOCTOR, HeartChamberGate);
                scriptEvents.addEvent(EVENT_AVATAR_PLAGUE_DOCTOR, 20000);
                break;
            default:
                break;
        }
    }

    // the temple guards turn hostile and join at once
    void callDefender(uint32_t entry, LocationVector const& spot)
    {
        Creature* pDefender = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pDefender == nullptr)
            return;

        pDefender->setFaction(14);
        pDefender->removeUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT | UNIT_FLAG_NOT_SELECTABLE);
        setZoneWideCombat(pDefender);
    }

    void checkRestoration()
    {
        uint32_t const maxHealth = getCreature()->getMaxHealth();
        uint32_t const health = getCreature()->getHealth();
        if (maxHealth == 0)
            return;

        if (health >= maxHealth)
        {
            completeRitual();
            return;
        }

        // more hexers rise as the Avatar recovers
        uint32_t const percent = static_cast<uint32_t>(static_cast<uint64_t>(health) * 100 / maxHealth);
        if (!mHexersRaisedLow && percent >= 40)
        {
            mHexersRaisedLow = true;
            raiseFallenHexers();
        }

        if (!mHexersRaisedHigh && percent >= 70)
        {
            mHexersRaisedHigh = true;
            raiseFallenHexers();
        }
    }

    void raiseFallenHexers()
    {
        if (SethralissInstanceScript* pTemple = getTemple())
        {
            for (Creature* pHexer : pTemple->getHexers())
            {
                if (!pHexer->isAlive())
                    pHexer->despawn(500, 1000);
            }
        }
    }

    void completeRitual()
    {
        mRitual = false;
        scriptEvents.resetEvents();
        summons.despawnAll();
        getCreature()->m_canRegenerateHP = true;

        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "The Avatar of Sethraliss is restored!");

        if (SethralissInstanceScript* pTemple = getTemple())
        {
            pTemple->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            for (Creature* pHexer : pTemple->getHexers())
            {
                if (pHexer->isAlive())
                    pHexer->despawn(1000, 0);
            }

            pTemple->setBossState(DATA_AVATAR_OF_SETHRALISS, EncounterStates::Performed);
        }
    }

    // nobody left standing nearby: everything goes back to the start
    void failRitual()
    {
        mRitual = false;
        mWatchClock = 5000;
        scriptEvents.resetEvents();
        summons.despawnAll();
        getCreature()->m_canRegenerateHP = true;
        getCreature()->setHealth(getCreature()->getMaxHealth());

        if (SethralissInstanceScript* pTemple = getTemple())
        {
            pTemple->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            for (Creature* pHexer : pTemple->getHexers())
            {
                if (!pHexer->isAlive())
                    pHexer->despawn(1000, 5000);
            }

            pTemple->setBossState(DATA_AVATAR_OF_SETHRALISS, EncounterStates::Failed);
        }
    }

    uint32_t mWatchClock = 1000;
    bool mRitual = false;
    bool mHexersRaisedLow = false;
    bool mHexersRaisedHigh = false;
};

// channels taint into the Avatar and starts the ritual when attacked
class SethralissHoodooHexerAI : public SethralissCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethralissHoodooHexerAI(c); }
    explicit SethralissHoodooHexerAI(Creature* pCreature) : SethralissCombatAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (SethralissInstanceScript* pTemple = getTemple())
            pTemple->sendAction(NPC_AVATAR_OF_SETHRALISS, ACTION_AVATAR_RITUAL_BEGIN);

        SethralissCombatAI::OnCombatStart(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (_isInCombat())
        {
            SethralissCombatAI::AIUpdate(time_passed);
            return;
        }

        // out of combat the taint keeps flowing
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mTaintClock > diff)
        {
            mTaintClock -= diff;
            return;
        }

        mTaintClock = 5000;
        if (!isAlive() || _isCasting())
            return;

        SethralissInstanceScript* pTemple = getTemple();
        Creature* pAvatar = pTemple != nullptr ? pTemple->getTracked(NPC_AVATAR_OF_SETHRALISS) : nullptr;
        if (pAvatar != nullptr && pAvatar->isAlive() && pTemple->getBossState(DATA_AVATAR_OF_SETHRALISS) != EncounterStates::Performed)
            castSpell(pAvatar, SPELL_HEXER_TAINT, true);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_HEXER_LAVA_BURST, 3000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_HEXER_FLAME_SHOCK, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HEXER_LAVA_BURST:
                castAtPlayer(SPELL_HEXER_LAVA_BURST);
                scriptEvents.addEvent(EVENT_HEXER_LAVA_BURST, 3000);
                break;
            case EVENT_HEXER_FLAME_SHOCK:
                castAtPlayer(SPELL_HEXER_FLAME_SHOCK);
                scriptEvents.addEvent(EVENT_HEXER_FLAME_SHOCK, 15000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mTaintClock = 2000;
};

// spreads its plague on the first foe it reaches
class SethralissPlagueToadAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SethralissPlagueToadAI(c); }
    explicit SethralissPlagueToadAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (pTarget != nullptr)
            castSpell(pTarget, SPELL_TOAD_PLAGUE);
    }
};

void SetupTempleOfSethraliss(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TEMPLE_OF_SETHRALISS, &SethralissInstanceScript::Create);

    mgr->register_creature_script(BOSS_ADDERIS, &SethralissAdderisAI::Create);
    mgr->register_creature_script(BOSS_ASPIX, &SethralissAspixAI::Create);

    mgr->register_creature_script(BOSS_MEREKTHA, &SethralissMerekthaAI::Create);

    mgr->register_creature_script(BOSS_GALVAZZT, &SethralissGalvazztAI::Create);
    mgr->register_creature_script(NPC_ENERGY_CORE, &SethralissEnergyCoreAI::Create);

    mgr->register_creature_script(NPC_AVATAR_OF_SETHRALISS, &SethralissAvatarAI::Create);
    mgr->register_creature_script(NPC_HOODOO_HEXER, &SethralissHoodooHexerAI::Create);
    mgr->register_creature_script(NPC_PLAGUE_TOAD, &SethralissPlagueToadAI::Create);
}
