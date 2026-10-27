/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Neltharions_Lair.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <vector>

enum NeltharionsLairEvents
{
    EVENT_ROKMORA_SHATTER = 1,
    EVENT_ROKMORA_RAZOR_SHARDS,
    EVENT_ROKMORA_CRYSTALLINE_GROUND,

    EVENT_ULAROGG_SUNDER,
    EVENT_ULAROGG_STRIKE_MOUNTAIN,
    EVENT_ULAROGG_BELLOW,
    EVENT_ULAROGG_BELLOW_ECHO,
    EVENT_ULAROGG_STANCE,
    EVENT_ULAROGG_STANCE_DEBRIS,

    EVENT_NARAXAS_RANCID_MAW,
    EVENT_NARAXAS_TOXIC_RETCH,
    EVENT_NARAXAS_DEVOUTS,
    EVENT_NARAXAS_SPIKED_TONGUE,
    EVENT_NARAXAS_REACH_CHECK,
    EVENT_NARAXAS_ANGRY_CROWD,

    EVENT_DARGRUL_MAGMA_SCULPTOR,
    EVENT_DARGRUL_LANDSLIDE,
    EVENT_DARGRUL_MOLTEN_CRASH,
    EVENT_DARGRUL_CRYSTAL_SPIKES,
    EVENT_DARGRUL_MAGMA_WAVE,
    EVENT_DARGRUL_LAVA_GEYSER,

    EVENT_CHARSKIN_HATRED
};

enum NeltharionsLairActions
{
    ACTION_NARAXAS_DEVOUR = 1
};

enum NeltharionsLairCreatureData
{
    DEVOUT_DATA_LANE = 1
};

namespace
{
    float const NeltharionsLairTwoPi = 6.28318531f;

    // energy fills at five points per second
    uint32_t const RokmoraShatterEnergyMs = 20000;

    uint32_t const UlaroggStanceFirstMs = 50000;
    uint32_t const UlaroggStanceRepeatMs = 120000;
    // stance idols stay out of reach this long
    uint32_t const UlaroggIdolDormantMs = 12000;

    // energy fills at two points per second
    uint32_t const NaraxasTongueEnergyMs = 50000;
    float const NaraxasDevourRange = 25.0f;

    // devouts drop from the ledge and walk to the maw
    LocationVector const DevoutLedges[2] =
    {
        { 3045.07f, 1807.39f, -44.13f, 3.54f },
        { 3048.89f, 1799.60f, -45.43f, 3.35f }
    };
    LocationVector const DevoutLandings[2] =
    {
        { 3033.45f, 1791.01f, -61.25f, 2.44f },
        { 3024.77f, 1802.29f, -60.10f, 2.44f }
    };
    LocationVector const DevoutAltars[2] =
    {
        { 2995.22f, 1808.65f, -61.29f, 1.82f },
        { 3009.69f, 1826.31f, -60.35f, 3.08f }
    };

    // energy fills to 64 at one point per second
    uint32_t const DargrulMagmaWaveEnergyMs = 64000;

    // grounded point on a ring around the anchor
    LocationVector neltharionsRingPoint(Creature* pAnchor, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(NeltharionsLairTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        float const x = pAnchor->GetPositionX() + range * std::cos(angle);
        float const y = pAnchor->GetPositionY() + range * std::sin(angle);
        float z = pAnchor->GetPositionZ();
        pAnchor->updateGroundPositionZ(x, y, z);
        return LocationVector(x, y, z, angle);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class NeltharionsLairInstanceScript : public InstanceScript
{
public:
    explicit NeltharionsLairInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(NeltharionsLairEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new NeltharionsLairInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        bool open = false;
        if (!barrierShouldBeOpen(pGameObject->getEntry(), open))
            return;

        mBarrierGuids.push_back(pGameObject->GetNewGUID());
        alignBarrier(pGameObject, open);
    }

    void OnEncounterStateChange(uint32_t /*id*/, uint32_t /*state*/) override
    {
        for (WoWGuid const& guid : mBarrierGuids)
        {
            GameObject* pBarrier = getGameObjectByGuid(guid);
            bool open = false;
            if (pBarrier != nullptr && barrierShouldBeOpen(pBarrier->getEntry(), open))
                alignBarrier(pBarrier, open);
        }
    }

private:
    bool isDone(uint32_t id) const { return getBossState(id) == EncounterStates::Performed; }
    bool isFighting(uint32_t id) const { return getBossState(id) == EncounterStates::InProgress; }

    // false for objects that are no encounter barrier
    bool barrierShouldBeOpen(uint32_t entry, bool& open) const
    {
        switch (entry)
        {
            case GO_ROKMORA_EXIT_BARRIER:
                open = isDone(DATA_ROKMORA);
                return true;
            case GO_ULAROGG_EXIT_BARRIER:
                open = isDone(DATA_ULAROGG_CRAGSHAPER);
                return true;
            case GO_ULAROGG_ROOM_BARRIER:
                open = !isFighting(DATA_ULAROGG_CRAGSHAPER);
                return true;
            case GO_NARAXAS_ROOM_BARRIER:
                open = !isFighting(DATA_NARAXAS);
                return true;
            default:
                return false;
        }
    }

    // changes the state only when it differs
    void alignBarrier(GameObject* pBarrier, bool open)
    {
        bool const isOpen = pBarrier->getState() != GO_STATE_CLOSED;
        if (isOpen != open)
            pBarrier->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    std::vector<WoWGuid> mBarrierGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick in combat

class NeltharionsLairCombatAI : public CreatureAIScript
{
public:
    explicit NeltharionsLairCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        onCombatTick();

        if (_isCasting())
            return;

        if (uint32_t eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onCombatTick() {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    bool victimInMelee()
    {
        Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
        return pVictim == nullptr || getCreature()->isWithinMeleeRange(pVictim);
    }

    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class NeltharionsLairBossAI : public NeltharionsLairCombatAI
{
public:
    NeltharionsLairBossAI(Creature* pCreature, uint32_t encounterId) : NeltharionsLairCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        NeltharionsLairCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        // a dying boss leaves combat as well
        if (isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mEncounterId, EncounterStates::Failed);
            }
        }

        NeltharionsLairCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }
    }

protected:
    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rokmora

class NeltharionsLairRokmoraAI : public NeltharionsLairBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeltharionsLairRokmoraAI(c); }
    explicit NeltharionsLairRokmoraAI(Creature* pCreature) : NeltharionsLairBossAI(pCreature, DATA_ROKMORA) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ROKMORA_SHATTER, RokmoraShatterEnergyMs);
        scriptEvents.addEvent(EVENT_ROKMORA_RAZOR_SHARDS, 30000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_ROKMORA_CRYSTALLINE_GROUND, 4000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ROKMORA_SHATTER:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Rokmora begins to cast Shatter!");
                castSpellAOE(SPELL_ROKMORA_SHATTER);
                scriptEvents.addEvent(EVENT_ROKMORA_SHATTER, RokmoraShatterEnergyMs);
                break;
            case EVENT_ROKMORA_RAZOR_SHARDS:
                castSpellOnVictim(SPELL_ROKMORA_RAZOR_SHARDS);
                scriptEvents.addEvent(EVENT_ROKMORA_RAZOR_SHARDS, 26000);
                break;
            case EVENT_ROKMORA_CRYSTALLINE_GROUND:
                castSpellAOE(SPELL_ROKMORA_CRYSTALLINE_GROUND);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ularogg Cragshaper, hides among his idols

class NeltharionsLairUlaroggAI : public NeltharionsLairBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeltharionsLairUlaroggAI(c); }
    explicit NeltharionsLairUlaroggAI(Creature* pCreature) : NeltharionsLairBossAI(pCreature, DATA_ULAROGG_CRAGSHAPER) {}

    // the idol he hides in fell
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (mInStance && pSummon->GetNewGUID() == mStanceIdolGuid)
            leaveStance(pSummon->GetPosition());
    }

protected:
    void onEngage() override
    {
        mInStance = false;
        scheduleRotation(UlaroggStanceFirstMs);
    }

    void onDisengage() override
    {
        if (!mInStance)
            return;

        mInStance = false;
        unhide();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ULAROGG_SUNDER:
                castSpellOnVictim(SPELL_ULAROGG_SUNDER);
                scriptEvents.addEvent(EVENT_ULAROGG_SUNDER, 10000);
                break;
            case EVENT_ULAROGG_STRIKE_MOUNTAIN:
                castSpellAOE(SPELL_ULAROGG_STRIKE_MOUNTAIN);
                scriptEvents.addEvent(EVENT_ULAROGG_STRIKE_MOUNTAIN, 16000);
                break;
            case EVENT_ULAROGG_BELLOW:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Ularogg Cragshaper raises Bellowing Idols!");
                callAdd(NPC_BELLOWING_IDOL_DEEPS, neltharionsRingPoint(getCreature(), 10.0f, 20.0f));
                scriptEvents.addEvent(EVENT_ULAROGG_BELLOW_ECHO, 3000);
                scriptEvents.addEvent(EVENT_ULAROGG_BELLOW, 32000);
                break;
            case EVENT_ULAROGG_BELLOW_ECHO:
                callAdd(NPC_BELLOWING_IDOL_DEEPS, neltharionsRingPoint(getCreature(), 10.0f, 20.0f));
                break;
            case EVENT_ULAROGG_STANCE:
                takeStance();
                break;
            case EVENT_ULAROGG_STANCE_DEBRIS:
                castSpellOnSelf(SPELL_ULAROGG_STANCE_DEBRIS, true);
                break;
            default:
                break;
        }
    }

private:
    void scheduleRotation(uint32_t stanceDelay)
    {
        scriptEvents.addEvent(EVENT_ULAROGG_SUNDER, 8000);
        scriptEvents.addEvent(EVENT_ULAROGG_STRIKE_MOUNTAIN, 16000);
        scriptEvents.addEvent(EVENT_ULAROGG_BELLOW, 20000);
        scriptEvents.addEvent(EVENT_ULAROGG_STANCE, stanceDelay);
    }

    // one idol holds him, two or four decoys stand around
    void takeStance()
    {
        Creature* pRealIdol = summonCreature(NPC_BELLOWING_IDOL_STANCE, getCreature()->GetPosition(), CORPSE_DESPAWN);
        if (pRealIdol == nullptr)
        {
            scheduleRotation(UlaroggStanceRepeatMs);
            return;
        }

        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Ularogg Cragshaper takes the Stance of the Mountain!");

        mInStance = true;
        mStanceIdolGuid = pRealIdol->GetNewGUID();
        scriptEvents.resetEvents();

        castSpellOnSelf(SPELL_ULAROGG_STANCE_FORM, true);
        setRooted(true);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        setZoneWideCombat(pRealIdol);
        uint32_t const decoys = isHeroic() ? 4 : 2;
        for (uint32_t i = 0; i < decoys; ++i)
            callAdd(NPC_BELLOWING_IDOL_STANCE, neltharionsRingPoint(getCreature(), 12.0f, 22.0f));

        scriptEvents.addEvent(EVENT_ULAROGG_STANCE_DEBRIS, UlaroggIdolDormantMs);
    }

    void leaveStance(LocationVector const& idolSpot)
    {
        mInStance = false;
        scriptEvents.resetEvents();
        summons.despawnEntry(NPC_BELLOWING_IDOL_STANCE);

        MoveTeleport(idolSpot);
        unhide();
        scheduleRotation(UlaroggStanceRepeatMs);
    }

    void unhide()
    {
        _removeAura(SPELL_ULAROGG_STANCE_FORM);
        _removeAura(SPELL_ULAROGG_STANCE_DEBRIS);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        _setMeleeDisabled(false);
        setRooted(false);
    }

    bool mInStance = false;
    WoWGuid mStanceIdolGuid;
};

// idols of both kinds, stance idols wake after a while
class NeltharionsLairBellowingIdolAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeltharionsLairBellowingIdolAI(c); }
    explicit NeltharionsLairBellowingIdolAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);

        mDormantMs = getCreature()->getEntry() == NPC_BELLOWING_IDOL_STANCE ? UlaroggIdolDormantMs : 0;
        if (mDormantMs > 0)
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            castSpellOnSelf(SPELL_BELLOWING_IDOL_DEBRIS, true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mDormantMs == 0)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mDormantMs > diff)
        {
            mDormantMs -= diff;
            return;
        }

        mDormantMs = 0;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_BELLOWING_IDOL_DEBRIS, true);
    }

private:
    uint32_t mDormantMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Naraxas, rooted in its pit

class NeltharionsLairNaraxasAI : public NeltharionsLairBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeltharionsLairNaraxasAI(c); }
    explicit NeltharionsLairNaraxasAI(Creature* pCreature) : NeltharionsLairBossAI(pCreature, DATA_NARAXAS) {}

    void OnLoad() override { setRooted(true); }

    // a devout reached the maw
    void DoAction(int32_t action) override
    {
        if (action != ACTION_NARAXAS_DEVOUR || !_isInCombat())
            return;

        std::list<Creature*> devouts;
        GetCreatureListWithEntryInGrid(devouts, NPC_WORMSPEAKER_DEVOUT, NaraxasDevourRange);
        for (Creature* pDevout : devouts)
        {
            if (!pDevout->isAlive())
                continue;

            castSpell(pDevout, SPELL_NARAXAS_DEVOUR_FANATIC, true);
            castSpellOnSelf(SPELL_NARAXAS_RAVENOUS, true);
            getCreature()->dealDamage(pDevout, pDevout->getHealth(), SPELL_NARAXAS_DEVOUR_FANATIC);
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Naraxas devours a Wormspeaker Devout!");
            break;
        }
    }

protected:
    void onEngage() override
    {
        mFrenzied = false;
        scriptEvents.addEvent(EVENT_NARAXAS_DEVOUTS, 7000);
        scriptEvents.addEvent(EVENT_NARAXAS_RANCID_MAW, 8000);
        scriptEvents.addEvent(EVENT_NARAXAS_TOXIC_RETCH, 12000);
        scriptEvents.addEvent(EVENT_NARAXAS_SPIKED_TONGUE, NaraxasTongueEnergyMs);
        scriptEvents.addEvent(EVENT_NARAXAS_REACH_CHECK, 2000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_NARAXAS_ANGRY_CROWD, 4000);
    }

    void onCombatTick() override
    {
        if (!mFrenzied && _getHealthPercent() <= 20)
        {
            mFrenzied = true;
            castSpellOnSelf(SPELL_NARAXAS_FRENZY, true);
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Naraxas goes into a frenzy!");
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NARAXAS_RANCID_MAW:
                castSpellAOE(SPELL_NARAXAS_RANCID_MAW);
                scriptEvents.addEvent(EVENT_NARAXAS_RANCID_MAW, 18000);
                break;
            case EVENT_NARAXAS_TOXIC_RETCH:
                castSpellAOE(SPELL_NARAXAS_TOXIC_RETCH);
                scriptEvents.addEvent(EVENT_NARAXAS_TOXIC_RETCH, 14000);
                break;
            case EVENT_NARAXAS_DEVOUTS:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Wormspeaker Devouts rush in to feed Naraxas!");
                for (uint32_t lane = 0; lane < 2; ++lane)
                {
                    Creature* pDevout = summonCreature(NPC_WORMSPEAKER_DEVOUT, DevoutLedges[lane], CORPSE_DESPAWN);
                    if (pDevout != nullptr && pDevout->GetScript() != nullptr)
                        pDevout->GetScript()->SetCreatureData(DEVOUT_DATA_LANE, lane);
                }
                scriptEvents.addEvent(EVENT_NARAXAS_DEVOUTS, 64000);
                break;
            case EVENT_NARAXAS_SPIKED_TONGUE:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Naraxas lashes out with its Spiked Tongue!");
                castSpellOnSelf(SPELL_NARAXAS_SPIKED_TONGUE, true);
                castSpellOnVictim(SPELL_NARAXAS_SPIKED_TONGUE_PULL);
                scriptEvents.addEvent(EVENT_NARAXAS_SPIKED_TONGUE, NaraxasTongueEnergyMs);
                break;
            case EVENT_NARAXAS_REACH_CHECK:
                if (!victimInMelee())
                    castSpellAOE(SPELL_NARAXAS_PUTRID_SKIES);
                scriptEvents.addEvent(EVENT_NARAXAS_REACH_CHECK, 2000);
                break;
            case EVENT_NARAXAS_ANGRY_CROWD:
                castSpellAOE(SPELL_NARAXAS_ANGRY_CROWD);
                break;
            default:
                break;
        }
    }

private:
    bool mFrenzied = false;
};

// jumps down from the ledge and offers itself at the maw
class NeltharionsLairWormspeakerDevoutAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeltharionsLairWormspeakerDevoutAI(c); }
    explicit NeltharionsLairWormspeakerDevoutAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        mStep = DevoutStep::Ledge;
        mStepMs = 1000;
    }

    void SetCreatureData(uint32_t type, uint32_t data) override
    {
        if (type == DEVOUT_DATA_LANE)
            mLane = data % 2;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mStep == DevoutStep::Offered || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mStepMs > diff)
        {
            mStepMs -= diff;
            return;
        }

        mStepMs = 1000;

        switch (mStep)
        {
            case DevoutStep::Ledge:
                moveJump(DevoutLandings[mLane], 15.0f, 15.0f);
                mStep = DevoutStep::Landing;
                mStepMs = 2500;
                break;
            case DevoutStep::Landing:
                if (getCreature()->getDistance(DevoutAltars[mLane]) > 3.0f)
                {
                    movePoint(1, DevoutAltars[mLane]);
                    break;
                }

                mStep = DevoutStep::Offered;
                castSpellOnSelf(SPELL_DEVOUT_FANATICS_SACRIFICE, true);
                if (Creature* pNaraxas = findNearestCreature(BOSS_NARAXAS, 60.0f))
                {
                    if (pNaraxas->isAlive() && pNaraxas->GetScript() != nullptr)
                        pNaraxas->GetScript()->DoAction(ACTION_NARAXAS_DEVOUR);
                }
                break;
            default:
                break;
        }
    }

private:
    enum class DevoutStep : uint8_t
    {
        Ledge,
        Landing,
        Offered
    };

    DevoutStep mStep = DevoutStep::Ledge;
    uint32_t mStepMs = 1000;
    uint32_t mLane = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Dargrul the Underking

class NeltharionsLairDargrulAI : public NeltharionsLairBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeltharionsLairDargrulAI(c); }
    explicit NeltharionsLairDargrulAI(Creature* pCreature) : NeltharionsLairBossAI(pCreature, DATA_DARGRUL_THE_UNDERKING) {}

protected:
    void onEngage() override
    {
        if (isHeroic())
            castSpellOnSelf(SPELL_DARGRUL_MAGMA_BREAKER, true);

        scriptEvents.addEvent(EVENT_DARGRUL_CRYSTAL_SPIKES, 6000);
        scriptEvents.addEvent(EVENT_DARGRUL_MAGMA_SCULPTOR, 10000);
        scriptEvents.addEvent(EVENT_DARGRUL_LANDSLIDE, 16000);
        scriptEvents.addEvent(EVENT_DARGRUL_MAGMA_WAVE, DargrulMagmaWaveEnergyMs);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DARGRUL_MAGMA_SCULPTOR:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Dargrul shapes a Molten Charskin!");
                callAdd(NPC_MOLTEN_CHARSKIN, neltharionsRingPoint(getCreature(), 6.0f, 10.0f));
                scriptEvents.addEvent(EVENT_DARGRUL_MAGMA_SCULPTOR, 70000);
                break;
            case EVENT_DARGRUL_LANDSLIDE:
                castSpellOnVictim(SPELL_DARGRUL_LANDSLIDE);
                scriptEvents.addEvent(EVENT_DARGRUL_MOLTEN_CRASH, 3000);
                scriptEvents.addEvent(EVENT_DARGRUL_LANDSLIDE, 16000);
                break;
            case EVENT_DARGRUL_MOLTEN_CRASH:
                castSpellOnVictim(SPELL_DARGRUL_MOLTEN_CRASH);
                break;
            case EVENT_DARGRUL_CRYSTAL_SPIKES:
                castSpellAOE(SPELL_DARGRUL_CRYSTAL_SPIKES);
                scriptEvents.addEvent(EVENT_DARGRUL_CRYSTAL_SPIKES, 22000);
                break;
            case EVENT_DARGRUL_MAGMA_WAVE:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Dargrul begins to cast Magma Wave! Hide behind the crystal spikes!");
                castSpellAOE(SPELL_DARGRUL_MAGMA_WAVE);
                if (isHeroic())
                    scriptEvents.addEvent(EVENT_DARGRUL_LAVA_GEYSER, 1000);
                scriptEvents.addEvent(EVENT_DARGRUL_MAGMA_WAVE, DargrulMagmaWaveEnergyMs);
                break;
            case EVENT_DARGRUL_LAVA_GEYSER:
                castSpellAOE(SPELL_DARGRUL_LAVA_GEYSER);
                break;
            default:
                break;
        }
    }
};

// hunts one player at a time
class NeltharionsLairMoltenCharskinAI : public NeltharionsLairCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NeltharionsLairMoltenCharskinAI(c); }
    explicit NeltharionsLairMoltenCharskinAI(Creature* pCreature) : NeltharionsLairCombatAI(pCreature) {}

    void OnLoad() override { castSpellOnSelf(isHeroic() ? SPELL_CHARSKIN_RISING_INFERNO : SPELL_CHARSKIN_MAGMA_BREAKER, true); }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_CHARSKIN_HATRED, 1000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_CHARSKIN_HATRED)
            return;

        Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
        if (pVictim == nullptr || !pVictim->isAlive() || !pVictim->hasAurasWithId(SPELL_CHARSKIN_BURNING_HATRED))
        {
            if (Unit* pTarget = randomPlayer())
            {
                castSpell(pTarget, SPELL_CHARSKIN_BURNING_HATRED, true);
                addThreat(pTarget, 1000000.0f);
            }
        }

        scriptEvents.addEvent(EVENT_CHARSKIN_HATRED, 2000);
    }
};

void SetupNeltharionsLair(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_NELTHARIONS_LAIR, &NeltharionsLairInstanceScript::Create);

    mgr->register_creature_script(BOSS_ROKMORA, &NeltharionsLairRokmoraAI::Create);

    mgr->register_creature_script(BOSS_ULAROGG_CRAGSHAPER, &NeltharionsLairUlaroggAI::Create);
    mgr->register_creature_script(NPC_BELLOWING_IDOL_DEEPS, &NeltharionsLairBellowingIdolAI::Create);
    mgr->register_creature_script(NPC_BELLOWING_IDOL_STANCE, &NeltharionsLairBellowingIdolAI::Create);

    mgr->register_creature_script(BOSS_NARAXAS, &NeltharionsLairNaraxasAI::Create);
    mgr->register_creature_script(NPC_WORMSPEAKER_DEVOUT, &NeltharionsLairWormspeakerDevoutAI::Create);

    mgr->register_creature_script(BOSS_DARGRUL, &NeltharionsLairDargrulAI::Create);
    mgr->register_creature_script(NPC_MOLTEN_CHARSKIN, &NeltharionsLairMoltenCharskinAI::Create);
}
