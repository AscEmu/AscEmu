/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Spires_Of_Ascension.hpp"

#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <map>
#include <vector>

enum SpiresOfAscensionEvents
{
    EVENT_VENTUNAX_DARK_STRIDE = 1,
    EVENT_VENTUNAX_BLINDING_FLASH,
    EVENT_VENTUNAX_RECHARGE_DONE,

    EVENT_KINTARA_DARK_LANCE,
    EVENT_KINTARA_TAKE_FLIGHT,
    EVENT_KINTARA_CHARGED_SPEAR,
    EVENT_KINTARA_LAND,

    EVENT_AZULES_BARRAGE,
    EVENT_AZULES_INSIDIOUS_VENOM,
    EVENT_AZULES_MAW_TOUCHED_VENOM,

    EVENT_ORYPHRION_CHARGED_STOMP,
    EVENT_ORYPHRION_PURIFYING_BLAST,
    EVENT_ORYPHRION_EMPYREAL_ORDNANCE,
    EVENT_ORYPHRION_RECHARGE,
    EVENT_ORYPHRION_RECHARGE_DONE,

    EVENT_DEVOS_RUN_THROUGH,
    EVENT_DEVOS_LOST_CONFIDENCE,
    EVENT_DEVOS_ABYSSAL_DETONATION,
    EVENT_DEVOS_LINGERING_DOUBT,
    EVENT_DEVOS_DESCEND
};

namespace
{
    // members of the encounters fought as one
    struct SpiresGroupSlot
    {
        uint32_t dataId;
        uint32_t entry;
    };

    SpiresGroupSlot const SpiresGroupSlots[] =
    {
        { DATA_KIN_TARA, BOSS_KIN_TARA },
        { DATA_KIN_TARA, BOSS_AZULES },
        { DATA_KIN_TARA, BOSS_AZORAS }
    };

    uint32_t const ShadowhirlEntries[] = { NPC_SHADOWHIRL, NPC_SHADOWHIRL_TRAIL };

    float const SpiresTwoPi = 6.2831853f;

    // Ventunax spends anima on every stride
    uint32_t const VentunaxAnimaFull = 100;
    uint32_t const VentunaxAnimaPerStride = 25;
    uint32_t const VentunaxRechargeMs = 8000;

    // Kin-Tara
    uint32_t const KinTaraEnergyFull = 100;
    uint32_t const KinTaraEnergyPerSecond = 5;
    uint32_t const KinTaraSpearsPerFlight = 4;

    // Oryphrion pays anima for each attack
    uint32_t const OryphrionAnimaFull = 100;
    uint32_t const OryphrionAnimaCost = 20;
    uint32_t const OryphrionDrainedMs = 10000;
    uint32_t const OryphrionRechargeMs = 6000;
    uint32_t const OrdnanceOrbsPerCast = 3;
    float const AnimaReachRange = 3.0f;
    uint32_t const AnimaMoveCheckMs = 500;

    // Devos leaves the platform at these health marks
    int32_t const DevosFlightMarks[] = { 70, 30 };
    uint32_t const DevosFlightMarkCount = 2;
    uint32_t const DevosFlightMs = 25000;

    LocationVector spiresPointAround(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(SpiresTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class SpiresOfAscensionInstanceScript : public InstanceScript
{
public:
    explicit SpiresOfAscensionInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(SpiresOfAscensionEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new SpiresOfAscensionInstanceScript(pMapMgr); }

    // a fallen partner must not end the encounter while the other stands
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (SpiresGroupSlot const& slot : SpiresGroupSlots)
        {
            if (slot.entry == pCreature->getEntry())
            {
                mMemberGuids[slot.entry] = pCreature->GetNewGUID();
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
        for (SpiresGroupSlot const& slot : SpiresGroupSlots)
        {
            if (slot.dataId != dataId)
                continue;

            if (Creature* pMember = getMember(slot.entry))
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

    // pulling one member pulls the rest
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
// Shared combat base: event clock while in combat

class SpiresOfAscensionCombatAI : public CreatureAIScript
{
public:
    explicit SpiresOfAscensionCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    SpiresOfAscensionInstanceScript* getSpires()
    {
        if (getCreature()->GetMapId() != MAP_SPIRES_OF_ASCENSION)
            return nullptr;

        return static_cast<SpiresOfAscensionInstanceScript*>(getInstanceScript());
    }

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    // anyone but the tank, the tank when alone
    Unit* randomNonTankPlayer()
    {
        FilterArgs args(TargetFilter_Player);
        args.addFilter(TargetFilter_NotCurrent);
        if (Unit* pTarget = selectUnitTarget(args))
            return pTarget;

        return randomPlayer();
    }

    void castAt(Unit* pTarget, uint32_t spellId, bool triggered = false)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void notify(uint32_t entry, int32_t action)
    {
        if (SpiresOfAscensionInstanceScript* pSpires = getSpires())
            pSpires->sendAction(entry, action);
    }

    // stop moving and swinging, used for channels and flight
    void setHolding(bool hold)
    {
        setRooted(hold);
        _setMeleeDisabled(hold);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class SpiresOfAscensionBossAI : public SpiresOfAscensionCombatAI
{
public:
    SpiresOfAscensionBossAI(Creature* pCreature, uint32_t dataId) : SpiresOfAscensionCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (SpiresOfAscensionInstanceScript* pSpires = getSpires())
        {
            pSpires->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pSpires->setBossState(mDataId, EncounterStates::InProgress);
            pSpires->rallyGroup(mDataId, getCreature());
        }

        SpiresOfAscensionCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (SpiresOfAscensionInstanceScript* pSpires = getSpires())
            {
                pSpires->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

                // the wipe counts once the last partner has left the fight
                if (!pSpires->isGroupFighting(mDataId, getCreature()))
                    pSpires->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        SpiresOfAscensionCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        // the instance holds the credit back while a partner stands
        if (SpiresOfAscensionInstanceScript* pSpires = getSpires())
        {
            pSpires->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pSpires->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ventunax

class SpiresOfAscensionVentunaxAI : public SpiresOfAscensionBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SpiresOfAscensionVentunaxAI(c); }
    explicit SpiresOfAscensionVentunaxAI(Creature* pCreature) : SpiresOfAscensionBossAI(pCreature, DATA_VENTUNAX) {}

protected:
    void onEngage() override
    {
        mAnima = VentunaxAnimaFull;

        scriptEvents.addEvent(EVENT_VENTUNAX_DARK_STRIDE, 8000);
        scriptEvents.addEvent(EVENT_VENTUNAX_BLINDING_FLASH, 14000);
    }

    void onReset() override
    {
        setHolding(false);
        _removeAura(SPELL_VENTUNAX_RECHARGE);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_VENTUNAX_DARK_STRIDE:
                castAt(randomNonTankPlayer(), SPELL_VENTUNAX_DARK_STRIDE);
                releaseDarkBolts();

                mAnima = mAnima > VentunaxAnimaPerStride ? mAnima - VentunaxAnimaPerStride : 0;
                if (mAnima == 0)
                    beginRecharge();
                else
                    scriptEvents.addEvent(EVENT_VENTUNAX_DARK_STRIDE, 12000);
                break;
            case EVENT_VENTUNAX_BLINDING_FLASH:
                castSpellOnVictim(SPELL_VENTUNAX_BLINDING_FLASH);
                scriptEvents.addEvent(EVENT_VENTUNAX_BLINDING_FLASH, 20000);
                break;
            case EVENT_VENTUNAX_RECHARGE_DONE:
                mAnima = VentunaxAnimaFull;
                setHolding(false);
                _removeAura(SPELL_VENTUNAX_RECHARGE);
                scriptEvents.addEvent(EVENT_VENTUNAX_DARK_STRIDE, 3000);
                scriptEvents.addEvent(EVENT_VENTUNAX_BLINDING_FLASH, 9000);
                break;
            default:
                break;
        }
    }

private:
    // out of anima she stops to draw it back in
    void beginRecharge()
    {
        scriptEvents.removeEvent(EVENT_VENTUNAX_BLINDING_FLASH);

        warn("Ventunax is depleted of anima and begins to Recharge!");
        setHolding(true);
        castSpellOnSelf(SPELL_VENTUNAX_RECHARGE);
        scriptEvents.addEvent(EVENT_VENTUNAX_RECHARGE_DONE, VentunaxRechargeMs);
    }

    // every Shadowhirl answers her move with a Dark Bolt
    void releaseDarkBolts()
    {
        for (uint32_t entry : ShadowhirlEntries)
        {
            std::list<Creature*> whirls;
            GetCreatureListWithEntryInGrid(whirls, entry, 100.0f);

            for (Creature* pWhirl : whirls)
            {
                if (!pWhirl->isAlive())
                    continue;

                if (Unit* pTarget = randomPlayer())
                    pWhirl->castSpell(pTarget, SPELL_SHADOWHIRL_DARK_BOLT, true);
            }
        }
    }

    uint32_t mAnima = VentunaxAnimaFull;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kin-Tara and Azules

class SpiresOfAscensionKinTaraAI : public SpiresOfAscensionBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SpiresOfAscensionKinTaraAI(c); }
    explicit SpiresOfAscensionKinTaraAI(Creature* pCreature) : SpiresOfAscensionBossAI(pCreature, DATA_KIN_TARA) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_SPIRES_COMPANION_FALLEN || mEnraged || !isAlive() || !_isInCombat())
            return;

        mEnraged = true;
        warn("Kin-Tara is enraged by the loss of her companion!");
        castSpellOnSelf(SPELL_KINTARA_ENRAGE, true);
    }

protected:
    void onEngage() override
    {
        mEnergy = 0;
        mEnergyClock = 0;
        mFlying = false;
        mSpearsLeft = 0;
        mEnraged = false;

        scriptEvents.addEvent(EVENT_KINTARA_DARK_LANCE, 9000);
        scriptEvents.addEvent(EVENT_KINTARA_TAKE_FLIGHT, 30000);
    }

    void onReset() override
    {
        mFlying = false;
        mEnraged = false;
        setHolding(false);
        _removeAura(SPELL_KINTARA_ENRAGE);
    }

    void onDefeat() override
    {
        notify(BOSS_AZULES, ACTION_SPIRES_COMPANION_FALLEN);
        notify(BOSS_AZORAS, ACTION_SPIRES_COMPANION_FALLEN);
    }

    void onCombatTick(uint32_t diff) override
    {
        // no energy while airborne
        if (mFlying)
            return;

        mEnergyClock += diff;
        while (mEnergyClock >= 1000)
        {
            mEnergyClock -= 1000;
            mEnergy += KinTaraEnergyPerSecond;
        }

        if (mEnergy < KinTaraEnergyFull || _isCasting())
            return;

        mEnergy = 0;
        castSpellOnVictim(SPELL_KINTARA_OVERHEAD_SLASH);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KINTARA_DARK_LANCE:
                castAt(randomPlayer(), SPELL_KINTARA_DARK_LANCE);
                scriptEvents.addEvent(EVENT_KINTARA_DARK_LANCE, 12000);
                break;
            case EVENT_KINTARA_TAKE_FLIGHT:
                warn("Kin-Tara takes to the air!");
                mFlying = true;
                mSpearsLeft = KinTaraSpearsPerFlight;
                setHolding(true);
                scriptEvents.addEvent(EVENT_KINTARA_CHARGED_SPEAR, 2000);
                break;
            case EVENT_KINTARA_CHARGED_SPEAR:
                castAt(randomPlayer(), SPELL_KINTARA_CHARGED_SPEAR);
                if (mSpearsLeft > 0 && --mSpearsLeft > 0)
                    scriptEvents.addEvent(EVENT_KINTARA_CHARGED_SPEAR, 3500);
                else
                    scriptEvents.addEvent(EVENT_KINTARA_LAND, 2500);
                break;
            case EVENT_KINTARA_LAND:
                mFlying = false;
                setHolding(false);
                scriptEvents.addEvent(EVENT_KINTARA_TAKE_FLIGHT, 45000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mEnergy = 0;
    uint32_t mEnergyClock = 0;
    bool mFlying = false;
    uint32_t mSpearsLeft = 0;
    bool mEnraged = false;
};

// Azules and the later Azoras template, venoms only once enraged
class SpiresOfAscensionAzulesAI : public SpiresOfAscensionBossAI
{
public:
    static CreatureAIScript* CreateAzules(Creature* c) { return new SpiresOfAscensionAzulesAI(c, SPELL_AZULES_ATTENUATED_BARRAGE); }
    static CreatureAIScript* CreateAzoras(Creature* c) { return new SpiresOfAscensionAzulesAI(c, SPELL_AZORAS_ATTENUATED_BARRAGE); }

    SpiresOfAscensionAzulesAI(Creature* pCreature, uint32_t barrageSpell) : SpiresOfAscensionBossAI(pCreature, DATA_KIN_TARA), mBarrageSpell(barrageSpell) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_SPIRES_COMPANION_FALLEN || mEnraged || !isAlive() || !_isInCombat())
            return;

        mEnraged = true;
        warn("Azules is enraged by the loss of Kin-Tara!");
        castSpellOnSelf(SPELL_KINTARA_ENRAGE, true);
        scriptEvents.addEvent(EVENT_AZULES_INSIDIOUS_VENOM, 4000);
        scriptEvents.addEvent(EVENT_AZULES_MAW_TOUCHED_VENOM, 9000);
    }

protected:
    void onEngage() override
    {
        mEnraged = false;
        scriptEvents.addEvent(EVENT_AZULES_BARRAGE, 6000);
    }

    void onReset() override
    {
        mEnraged = false;
        _removeAura(SPELL_KINTARA_ENRAGE);
    }

    void onDefeat() override { notify(BOSS_KIN_TARA, ACTION_SPIRES_COMPANION_FALLEN); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AZULES_BARRAGE:
                castAt(randomPlayer(), mBarrageSpell);
                scriptEvents.addEvent(EVENT_AZULES_BARRAGE, 10000);
                break;
            case EVENT_AZULES_INSIDIOUS_VENOM:
                castAt(randomPlayer(), SPELL_AZULES_INSIDIOUS_VENOM);
                scriptEvents.addEvent(EVENT_AZULES_INSIDIOUS_VENOM, 14000);
                break;
            case EVENT_AZULES_MAW_TOUCHED_VENOM:
                castAt(randomPlayer(), SPELL_AZULES_MAW_TOUCHED_VENOM);
                scriptEvents.addEvent(EVENT_AZULES_MAW_TOUCHED_VENOM, 20000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mBarrageSpell;
    bool mEnraged = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Oryphrion

class SpiresOfAscensionOryphrionAI : public SpiresOfAscensionBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SpiresOfAscensionOryphrionAI(c); }
    explicit SpiresOfAscensionOryphrionAI(Creature* pCreature) : SpiresOfAscensionBossAI(pCreature, DATA_ORYPHRION) {}

protected:
    void onEngage() override
    {
        mAnima = OryphrionAnimaFull;
        scheduleAttacks(0);
    }

    void onReset() override
    {
        setHolding(false);
        _removeAura(SPELL_ORYPHRION_DRAINED);
        _removeAura(SPELL_ORYPHRION_RECHARGE_ANIMA);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ORYPHRION_CHARGED_STOMP:
                castSpellOnVictim(SPELL_ORYPHRION_CHARGED_STOMP);
                spendAnima(EVENT_ORYPHRION_CHARGED_STOMP, 16000);
                break;
            case EVENT_ORYPHRION_PURIFYING_BLAST:
                castAt(randomPlayer(), SPELL_ORYPHRION_PURIFYING_BLAST);
                spendAnima(EVENT_ORYPHRION_PURIFYING_BLAST, 14000);
                break;
            case EVENT_ORYPHRION_EMPYREAL_ORDNANCE:
                castSpellAOE(SPELL_ORYPHRION_EMPYREAL_ORDNANCE);
                dropOrdnance();
                spendAnima(EVENT_ORYPHRION_EMPYREAL_ORDNANCE, 24000);
                break;
            case EVENT_ORYPHRION_RECHARGE:
                warn("Oryphrion begins to Recharge Anima!");
                castSpellOnSelf(SPELL_ORYPHRION_RECHARGE_ANIMA);
                callAnimaBack();
                scriptEvents.addEvent(EVENT_ORYPHRION_RECHARGE_DONE, OryphrionRechargeMs);
                break;
            case EVENT_ORYPHRION_RECHARGE_DONE:
                mAnima = OryphrionAnimaFull;
                setHolding(false);
                _removeAura(SPELL_ORYPHRION_DRAINED);
                _removeAura(SPELL_ORYPHRION_RECHARGE_ANIMA);
                scheduleAttacks(0);
                break;
            default:
                break;
        }
    }

private:
    void scheduleAttacks(uint32_t delayMs)
    {
        scriptEvents.addEvent(EVENT_ORYPHRION_CHARGED_STOMP, delayMs + 8000);
        scriptEvents.addEvent(EVENT_ORYPHRION_PURIFYING_BLAST, delayMs + 13000);
        scriptEvents.addEvent(EVENT_ORYPHRION_EMPYREAL_ORDNANCE, delayMs + 18000);
    }

    // each attack costs anima, at zero he is Drained
    void spendAnima(uint32_t eventId, uint32_t repeatMs)
    {
        mAnima = mAnima > OryphrionAnimaCost ? mAnima - OryphrionAnimaCost : 0;
        if (mAnima > 0)
        {
            scriptEvents.addEvent(eventId, repeatMs);
            return;
        }

        scriptEvents.removeEvent(EVENT_ORYPHRION_CHARGED_STOMP);
        scriptEvents.removeEvent(EVENT_ORYPHRION_PURIFYING_BLAST);
        scriptEvents.removeEvent(EVENT_ORYPHRION_EMPYREAL_ORDNANCE);

        warn("Oryphrion is Drained!");
        setHolding(true);
        castSpellOnSelf(SPELL_ORYPHRION_DRAINED, true);
        scriptEvents.addEvent(EVENT_ORYPHRION_RECHARGE, OryphrionDrainedMs);
    }

    // anima orbs land among the players
    void dropOrdnance()
    {
        for (uint32_t i = 0; i < OrdnanceOrbsPerCast; ++i)
        {
            Unit* pTarget = randomPlayer();
            LocationVector const spot = pTarget != nullptr ? spiresPointAround(pTarget->GetPosition(), 0.0f, 3.0f) : spiresPointAround(getCreature()->GetPosition(), 8.0f, 15.0f);
            summonCreature(NPC_COALESCED_ANIMA, spot, CORPSE_DESPAWN);
        }
    }

    void callAnimaBack()
    {
        std::list<Creature*> orbs;
        GetCreatureListWithEntryInGrid(orbs, NPC_COALESCED_ANIMA, 150.0f);

        for (Creature* pOrb : orbs)
        {
            if (pOrb->isAlive() && pOrb->GetScript() != nullptr)
                pOrb->GetScript()->DoAction(ACTION_SPIRES_ANIMA_RETURN);
        }
    }

    uint32_t mAnima = OryphrionAnimaFull;
};

// rests where it lands, drifts back to Oryphrion while he recharges
class SpiresOfAscensionCoalescedAnimaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SpiresOfAscensionCoalescedAnimaAI(c); }
    explicit SpiresOfAscensionCoalescedAnimaAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        mReturning = false;
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_SPIRES_ANIMA_RETURN || mReturning || !isAlive())
            return;

        mReturning = true;
        mMoveClock = 0;
        setRooted(false);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mReturning || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mMoveClock > diff)
        {
            mMoveClock -= diff;
            return;
        }

        mMoveClock = AnimaMoveCheckMs;

        Creature* pOryphrion = findNearestCreature(BOSS_ORYPHRION, 150.0f);
        if (pOryphrion == nullptr || !pOryphrion->isAlive())
            return;

        if (getCreature()->getDistance(pOryphrion) > AnimaReachRange)
        {
            moveTo(pOryphrion->GetPositionX(), pOryphrion->GetPositionY(), pOryphrion->GetPositionZ(), false);
            return;
        }

        // reached him: the anima overcharges
        mReturning = false;
        castSpellAOE(SPELL_ANIMA_OVERCHARGE_ANIMA, true);
        despawn(500, 0);
    }

private:
    bool mReturning = false;
    uint32_t mMoveClock = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Devos, Paragon of Doubt

class SpiresOfAscensionDevosAI : public SpiresOfAscensionBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SpiresOfAscensionDevosAI(c); }
    explicit SpiresOfAscensionDevosAI(Creature* pCreature) : SpiresOfAscensionBossAI(pCreature, DATA_DEVOS) {}

protected:
    void onEngage() override
    {
        mFlightIndex = 0;
        mAirborne = false;
        scheduleGroundAttacks();
    }

    void onReset() override
    {
        mAirborne = false;
        land();
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mAirborne || mFlightIndex >= DevosFlightMarkCount || _isCasting())
            return;

        if (_getHealthPercent() <= DevosFlightMarks[mFlightIndex])
            takeToTheSkies();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DEVOS_RUN_THROUGH:
                castAt(randomNonTankPlayer(), SPELL_DEVOS_RUN_THROUGH);
                scriptEvents.addEvent(EVENT_DEVOS_RUN_THROUGH, 16000);
                break;
            case EVENT_DEVOS_LOST_CONFIDENCE:
                castSpellAOE(SPELL_DEVOS_LOST_CONFIDENCE);
                scriptEvents.addEvent(EVENT_DEVOS_LOST_CONFIDENCE, 22000);
                break;
            case EVENT_DEVOS_ABYSSAL_DETONATION:
                warn("Devos begins an Abyssal Detonation!");
                castSpellAOE(SPELL_DEVOS_ABYSSAL_DETONATION);
                scriptEvents.addEvent(EVENT_DEVOS_ABYSSAL_DETONATION, 30000);
                break;
            case EVENT_DEVOS_LINGERING_DOUBT:
                castAt(randomPlayer(), SPELL_DEVOS_LINGERING_DOUBT, true);
                if (mAirborne)
                    scriptEvents.addEvent(EVENT_DEVOS_LINGERING_DOUBT, 5000);
                break;
            case EVENT_DEVOS_DESCEND:
                mAirborne = false;
                land();
                warn("Devos is forced back down to the platform!");
                scheduleGroundAttacks();
                break;
            default:
                break;
        }
    }

private:
    void scheduleGroundAttacks()
    {
        scriptEvents.addEvent(EVENT_DEVOS_RUN_THROUGH, 9000);
        scriptEvents.addEvent(EVENT_DEVOS_LOST_CONFIDENCE, 15000);
        scriptEvents.addEvent(EVENT_DEVOS_ABYSSAL_DETONATION, 25000);
    }

    // she activates the conduit and rules the winds for a while
    void takeToTheSkies()
    {
        ++mFlightIndex;
        mAirborne = true;

        scriptEvents.removeEvent(EVENT_DEVOS_RUN_THROUGH);
        scriptEvents.removeEvent(EVENT_DEVOS_LOST_CONFIDENCE);
        scriptEvents.removeEvent(EVENT_DEVOS_ABYSSAL_DETONATION);

        warn("Devos activates the Anima Conduit and takes to the skies!");
        castSpellAOE(SPELL_DEVOS_UNLEASHED_ANIMA, true);

        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setHolding(true);

        scriptEvents.addEvent(EVENT_DEVOS_LINGERING_DOUBT, 4000);
        scriptEvents.addEvent(EVENT_DEVOS_DESCEND, DevosFlightMs);
    }

    void land()
    {
        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setHolding(false);
    }

    uint32_t mFlightIndex = 0;
    bool mAirborne = false;
};

void SetupSpiresOfAscension(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SPIRES_OF_ASCENSION, &SpiresOfAscensionInstanceScript::Create);

    mgr->register_creature_script(BOSS_VENTUNAX, &SpiresOfAscensionVentunaxAI::Create);

    mgr->register_creature_script(BOSS_KIN_TARA, &SpiresOfAscensionKinTaraAI::Create);
    mgr->register_creature_script(BOSS_AZULES, &SpiresOfAscensionAzulesAI::CreateAzules);
    mgr->register_creature_script(BOSS_AZORAS, &SpiresOfAscensionAzulesAI::CreateAzoras);

    mgr->register_creature_script(BOSS_ORYPHRION, &SpiresOfAscensionOryphrionAI::Create);
    mgr->register_creature_script(NPC_COALESCED_ANIMA, &SpiresOfAscensionCoalescedAnimaAI::Create);

    mgr->register_creature_script(BOSS_DEVOS, &SpiresOfAscensionDevosAI::Create);
}
