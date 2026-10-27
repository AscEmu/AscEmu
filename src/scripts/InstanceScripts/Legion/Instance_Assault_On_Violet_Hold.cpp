/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Assault_On_Violet_Hold.hpp"

#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
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
#include <sstream>
#include <string>
#include <vector>

enum AssaultOnVioletHoldEvents
{
    EVENT_ASSAULT_OPEN_PORTAL = 1,
    EVENT_ASSAULT_WAVE_CHECK,
    EVENT_ASSAULT_RELEASE,

    EVENT_SHIVERMAW_FROST_BREATH,
    EVENT_SHIVERMAW_RELENTLESS_STORM,
    EVENT_SHIVERMAW_TAIL_SWEEP,
    EVENT_SHIVERMAW_WING_BUFFET,
    EVENT_SHIVERMAW_ICE_BOMB,
    EVENT_SHIVERMAW_FRIGID_WINDS,

    EVENT_KAAHRJ_SHADOW_CRASH,
    EVENT_KAAHRJ_DOOM,
    EVENT_KAAHRJ_HYSTERIA,
    EVENT_KAAHRJ_ETERNAL_DARKNESS,

    EVENT_MILLIFICENT_FINGER_LASER,
    EVENT_MILLIFICENT_ROCKET_CHICKEN,

    EVENT_FESTERFACE_CONGEALING_VOMIT,
    EVENT_FESTERFACE_BLACK_BILE,

    EVENT_SAELORN_VENOM_SPRAY,
    EVENT_SAELORN_WEB_GRAB,
    EVENT_SAELORN_FEL_DETONATION,
    EVENT_SAELORN_PHASE_SPIDER,
    EVENT_SAELORN_TOXIC_BLOOD,

    EVENT_ANUBESSET_MANDIBLE_STRIKE,
    EVENT_ANUBESSET_IMPALE,
    EVENT_ANUBESSET_CALL_OF_THE_SWARM,

    EVENT_SCARAB_POISON_SPIT,

    EVENT_THALENA_VAMPYRS_KISS,
    EVENT_THALENA_BLOOD_SWARM,
    EVENT_THALENA_BLOOD_CALL,

    EVENT_BETRUG_CHAOTIC_ENERGY,
    EVENT_BETRUG_FEL_SLASH,
    EVENT_BETRUG_MIGHTY_SMASH,
    EVENT_BETRUG_SEED_OF_DESTRUCTION
};

namespace
{
    struct AssaultJailer
    {
        uint32_t encounterId;
        uint32_t entry;
        uint32_t cellEntry;
    };

    // the seven prisoners a run can draw from
    AssaultJailer const AssaultJailers[] =
    {
        { DATA_SHIVERMAW,               BOSS_SHIVERMAW,                 GO_SHIVERMAW_CELL },
        { DATA_MINDFLAYER_KAAHRJ,       BOSS_MINDFLAYER_KAAHRJ,         GO_KAAHRJ_CELL },
        { DATA_MILLIFICENT_MANASTORM,   BOSS_MILLIFICENT_MANASTORM,     GO_MILLIFICENT_CELL },
        { DATA_FESTERFACE,              BOSS_FESTERFACE,                GO_FESTERFACE_CELL },
        { DATA_SAELORN,                 BOSS_SAELORN,                   GO_SAELORN_CELL },
        { DATA_ANUBESSET,               BOSS_ANUBESSET,                 GO_ANUBESSET_CELL },
        { DATA_BLOOD_PRINCESS_THALENA,  BOSS_BLOOD_PRINCESS_THALENA,    GO_THALENA_CELL }
    };

    AssaultJailer const* findJailerByEncounter(uint32_t encounterId)
    {
        for (AssaultJailer const& jailer : AssaultJailers)
        {
            if (jailer.encounterId == encounterId)
                return &jailer;
        }

        return nullptr;
    }

    AssaultJailer const* findJailerByCell(uint32_t cellEntry)
    {
        for (AssaultJailer const& jailer : AssaultJailers)
        {
            if (jailer.cellEntry == cellEntry)
                return &jailer;
        }

        return nullptr;
    }

    uint32_t const NoJailer = 0xFF;
    uint32_t const AssaultPortalsPerStage = 5;

    LocationVector const AssaultPortalSpots[] =
    {
        { 4682.28f, 4013.68f, 91.80f, 3.14f },
        { 4653.47f, 4041.61f, 78.08f, 3.74f },
        { 4636.86f, 3963.92f, 87.05f, 2.05f },
        { 4623.71f, 4060.67f, 82.72f, 4.76f },
        { 4604.43f, 3980.73f, 78.04f, 1.10f }
    };
    uint32_t const AssaultPortalSpotCount = 5;

    LocationVector const BetrugArrivalSpot(4663.58f, 4014.95f, 82.72f, 3.01f);

    uint32_t const AssaultEliteInvaders[] = { NPC_FELGUARD_DESTROYER_1, NPC_FELGUARD_DESTROYER_2, NPC_EREDAR_SHADOW_MENDER, NPC_SHADOW_COUNCIL_WARLOCK };
    uint32_t const AssaultPortalWardens[] = { NPC_PORTAL_GUARDIAN_1, NPC_PORTAL_GUARDIAN_2, NPC_PORTAL_KEEPER_1, NPC_PORTAL_KEEPER_2 };
    uint32_t const AssaultRaiders[] = { NPC_EREDAR_INVADER_1, NPC_EREDAR_INVADER_2, NPC_WRATHLORD_BULWARK, NPC_FELSTALKER_RAVENER_1, NPC_FELSTALKER_RAVENER_2, NPC_INFILTRATOR_ASSASSIN };

    template <std::size_t N>
    uint32_t pickFrom(uint32_t const (&entries)[N]) { return entries[Util::getRandomUInt(static_cast<uint32_t>(N - 1))]; }

    float const AssaultTwoPi = 6.28318531f;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance: portal waves, two freed prisoners, then Fel Lord Betrug

class AssaultOnVioletHoldInstanceScript : public InstanceScript
{
    enum class AssaultStage : uint8_t
    {
        WavesBeforeFirst,
        FirstJailer,
        WavesBeforeSecond,
        SecondJailer,
        WavesBeforeBetrug,
        Betrug
    };

public:
    explicit AssaultOnVioletHoldInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(AssaultOnVioletHoldEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new AssaultOnVioletHoldInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        uint32_t const entry = pCreature->getEntry();
        if (entry == BOSS_FEL_LORD_BETRUG)
        {
            mTrackedGuids[entry] = pCreature->GetNewGUID();
            return;
        }

        for (AssaultJailer const& jailer : AssaultJailers)
        {
            if (jailer.entry == entry)
            {
                mTrackedGuids[entry] = pCreature->GetNewGUID();
                return;
            }
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() != GO_PRISON_SEAL && findJailerByCell(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        alignDoor(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        alignDoors();

        if (mAssaultState != EncounterStates::InProgress || state != EncounterStates::Performed)
            return;

        if (id == DATA_FEL_LORD_BETRUG)
        {
            finishAssault();
            return;
        }

        if ((mStage == AssaultStage::FirstJailer && id == mJailers[0]) || (mStage == AssaultStage::SecondJailer && id == mJailers[1]))
            jailerDefeated(30000);
    }

    void UpdateEvent() override
    {
        if (mAssaultState != EncounterStates::InProgress)
            return;

        scriptEvents.updateEvents(getUpdateFrequency(), 0);

        while (uint32_t const eventId = scriptEvents.getFinishedEvent())
        {
            switch (eventId)
            {
                case EVENT_ASSAULT_OPEN_PORTAL:
                    openPortal();
                    scriptEvents.addEvent(EVENT_ASSAULT_WAVE_CHECK, 3000);
                    break;
                case EVENT_ASSAULT_WAVE_CHECK:
                    if (isWaveStanding())
                        scriptEvents.addEvent(EVENT_ASSAULT_WAVE_CHECK, 2000);
                    else if (mPortalsOpened < AssaultPortalsPerStage)
                        scriptEvents.addEvent(EVENT_ASSAULT_OPEN_PORTAL, 5000);
                    else
                        scriptEvents.addEvent(EVENT_ASSAULT_RELEASE, 5000);
                    break;
                case EVENT_ASSAULT_RELEASE:
                    releaseNext();
                    break;
                default:
                    break;
            }
        }
    }

    // the chosen prisoners survive a restart of the instance
    void writeSaveDataExtended(std::ostringstream& data) override { data << mJailers[0] << ' ' << mJailers[1] << ' '; }

    void readSaveDataExtended(std::istringstream& data) override
    {
        uint32_t first = NoJailer;
        uint32_t second = NoJailer;
        if (data >> first >> second)
        {
            mJailers[0] = first;
            mJailers[1] = second;
        }
    }

    bool canBeginAssault() const
    {
        return mAssaultState == EncounterStates::NotStarted && getBossState(DATA_FEL_LORD_BETRUG) != EncounterStates::Performed;
    }

    void beginAssault()
    {
        if (!canBeginAssault())
            return;

        mAssaultState = EncounterStates::InProgress;
        mStage = AssaultStage::WavesBeforeFirst;
        mPortalsOpened = 0;
        mWaveGuids.clear();

        chooseJailers();
        saveToDB();
        alignDoors();

        scriptEvents.resetEvents();
        scriptEvents.addEvent(EVENT_ASSAULT_OPEN_PORTAL, 10000);
    }

    bool isReleased(uint32_t encounterId) const { return mReleased.find(encounterId) != mReleased.end(); }

private:
    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    // kept from an earlier attempt, otherwise drawn from the spawned prisoners
    void chooseJailers()
    {
        if (findJailerByEncounter(mJailers[0]) != nullptr && findJailerByEncounter(mJailers[1]) != nullptr)
            return;

        std::vector<uint32_t> candidates;
        for (AssaultJailer const& jailer : AssaultJailers)
        {
            Creature* pJailer = getTracked(jailer.entry);
            if (pJailer != nullptr && pJailer->isAlive() && getBossState(jailer.encounterId) != EncounterStates::Performed)
                candidates.push_back(jailer.encounterId);
        }

        for (uint32_t& slot : mJailers)
        {
            slot = NoJailer;
            if (candidates.empty())
                continue;

            uint32_t const pick = Util::getRandomUInt(static_cast<uint32_t>(candidates.size() - 1));
            slot = candidates[pick];
            candidates.erase(candidates.begin() + pick);
        }
    }

    void openPortal()
    {
        ++mPortalsOpened;
        mWaveGuids.clear();

        LocationVector const& gate = AssaultPortalSpots[Util::getRandomUInt(AssaultPortalSpotCount - 1)];
        bool const lateStage = mStage != AssaultStage::WavesBeforeFirst;

        // either a pack of elites or a portal warden with its raiders
        if (Util::getRandomUInt(1) == 0)
        {
            uint32_t const count = lateStage ? 3 : 2;
            for (uint32_t i = 0; i < count; ++i)
                sendInvader(pickFrom(AssaultEliteInvaders), gate);
        }
        else
        {
            sendInvader(pickFrom(AssaultPortalWardens), gate);

            uint32_t const count = lateStage ? 4 : 3;
            for (uint32_t i = 0; i < count; ++i)
                sendInvader(pickFrom(AssaultRaiders), gate);
        }
    }

    void sendInvader(uint32_t entry, LocationVector const& gate)
    {
        float const angle = Util::getRandomFloat(AssaultTwoPi);
        Creature* pInvader = spawnCreature(entry, gate.x + 2.0f * std::cos(angle), gate.y + 2.0f * std::sin(angle), gate.z, gate.o);
        if (pInvader == nullptr)
            return;

        mWaveGuids.push_back(pInvader->GetNewGUID());
        if (pInvader->GetScript() != nullptr)
            pInvader->GetScript()->DoAction(ACTION_INVADER_CHARGE);
    }

    bool isWaveStanding()
    {
        for (WoWGuid const& guid : mWaveGuids)
        {
            Creature* pInvader = getCreatureByGuid(guid);
            if (pInvader != nullptr && pInvader->isAlive())
                return true;
        }

        return false;
    }

    void releaseNext()
    {
        switch (mStage)
        {
            case AssaultStage::WavesBeforeFirst:
                mStage = AssaultStage::FirstJailer;
                releaseJailer(mJailers[0]);
                break;
            case AssaultStage::WavesBeforeSecond:
                mStage = AssaultStage::SecondJailer;
                releaseJailer(mJailers[1]);
                break;
            case AssaultStage::WavesBeforeBetrug:
                mStage = AssaultStage::Betrug;
                callBetrug();
                break;
            default:
                break;
        }
    }

    void releaseJailer(uint32_t encounterId)
    {
        AssaultJailer const* jailer = findJailerByEncounter(encounterId);
        Creature* pJailer = jailer != nullptr ? getTracked(jailer->entry) : nullptr;

        // nothing left to fight in that cell, carry on with the waves
        if (pJailer == nullptr || !pJailer->isAlive() || getBossState(encounterId) == EncounterStates::Performed)
        {
            jailerDefeated(5000);
            return;
        }

        mReleased.insert(encounterId);
        alignDoors();

        if (pJailer->GetScript() != nullptr)
            pJailer->GetScript()->DoAction(ACTION_JAILER_RELEASE);
    }

    void jailerDefeated(uint32_t nextPortalDelay)
    {
        if (mStage == AssaultStage::FirstJailer)
            mStage = AssaultStage::WavesBeforeSecond;
        else if (mStage == AssaultStage::SecondJailer)
            mStage = AssaultStage::WavesBeforeBetrug;
        else
            return;

        mPortalsOpened = 0;
        scriptEvents.addEvent(EVENT_ASSAULT_OPEN_PORTAL, nextPortalDelay);
    }

    // the fel lord comes in person, brought in when the world holds no spawn of him
    void callBetrug()
    {
        if (getBossState(DATA_FEL_LORD_BETRUG) == EncounterStates::Performed)
        {
            finishAssault();
            return;
        }

        mReleased.insert(DATA_FEL_LORD_BETRUG);

        Creature* pBetrug = getTracked(BOSS_FEL_LORD_BETRUG);
        if (pBetrug == nullptr || !pBetrug->isAlive())
            pBetrug = spawnCreature(BOSS_FEL_LORD_BETRUG, BetrugArrivalSpot.x, BetrugArrivalSpot.y, BetrugArrivalSpot.z, BetrugArrivalSpot.o);

        if (pBetrug != nullptr && pBetrug->GetScript() != nullptr)
            pBetrug->GetScript()->DoAction(ACTION_JAILER_RELEASE);
    }

    void finishAssault()
    {
        mAssaultState = EncounterStates::Performed;
        scriptEvents.resetEvents();
        alignDoors();
    }

    void alignDoors()
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor);
        }
    }

    // seal shut during the assault, a cell opens once its prisoner is let out
    void alignDoor(GameObject* pDoor)
    {
        bool wantOpen = false;
        if (pDoor->getEntry() == GO_PRISON_SEAL)
        {
            wantOpen = mAssaultState != EncounterStates::InProgress;
        }
        else if (AssaultJailer const* jailer = findJailerByCell(pDoor->getEntry()))
        {
            wantOpen = isReleased(jailer->encounterId) || getBossState(jailer->encounterId) == EncounterStates::Performed;
        }
        else
        {
            return;
        }

        bool const isOpen = pDoor->getState() != GO_STATE_CLOSED;
        if (wantOpen != isOpen)
            pDoor->setState(wantOpen ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    std::map<uint32_t, WoWGuid> mTrackedGuids;
    std::vector<WoWGuid> mDoorGuids;
    std::vector<WoWGuid> mWaveGuids;
    std::set<uint32_t> mReleased;
    uint32_t mJailers[2] = { NoJailer, NoJailer };
    uint32_t mPortalsOpened = 0;
    AssaultStage mStage = AssaultStage::WavesBeforeFirst;
    EncounterStates mAssaultState = EncounterStates::NotStarted;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lieutenant Sinclari starts the assault

class AssaultOnVioletHoldSinclariGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* pPlayer) override
    {
        AssaultOnVioletHoldInstanceScript* pHold = getVioletHold(pObject);
        if (pHold == nullptr || !pHold->canBeginAssault())
        {
            GossipMenu::sendSimpleMenu(pObject->getGuid(), SinclariAssaultGossipText, pPlayer);
            return;
        }

        GossipMenu::sendQuickMenu(pObject->getGuid(), SinclariAssaultGossipText, pPlayer, 1, GOSSIP_ICON_CHAT, "We are ready. Let the defense of the prison begin.");
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t id, const char* /*enteredCode*/, uint32_t /*gossipId*/) override
    {
        GossipMenu::senGossipComplete(pPlayer);
        if (id != 1)
            return;

        if (AssaultOnVioletHoldInstanceScript* pHold = getVioletHold(pObject))
            pHold->beginAssault();
    }

private:
    static AssaultOnVioletHoldInstanceScript* getVioletHold(Object* pObject)
    {
        if (pObject == nullptr || pObject->GetMapId() != MAP_ASSAULT_ON_VIOLET_HOLD || pObject->getWorldMap() == nullptr)
            return nullptr;

        return static_cast<AssaultOnVioletHoldInstanceScript*>(pObject->getWorldMap()->getScript());
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class AssaultOnVioletHoldCombatAI : public CreatureAIScript
{
public:
    explicit AssaultOnVioletHoldCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    AssaultOnVioletHoldInstanceScript* getVioletHold()
    {
        if (getCreature()->GetMapId() != MAP_ASSAULT_ON_VIOLET_HOLD)
            return nullptr;

        return static_cast<AssaultOnVioletHoldInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return getBestPlayerTarget(TargetFilter_None); }

    Unit* offTankPlayer()
    {
        if (Unit* pTarget = getBestPlayerTarget(TargetFilter_NotCurrent))
            return pTarget;

        return anyPlayer();
    }

    void castAt(Unit* pTarget, uint32_t spellId)
    {
        if (pTarget != nullptr)
            castSpell(pTarget, spellId);
    }

    Creature* raiseAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    static LocationVector nearUnit(Unit* pUnit, float radius)
    {
        float const angle = Util::getRandomFloat(AssaultTwoPi);
        return LocationVector(pUnit->GetPositionX() + radius * std::cos(angle), pUnit->GetPositionY() + radius * std::sin(angle), pUnit->GetPositionZ(), angle);
    }

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class AssaultOnVioletHoldBossAI : public AssaultOnVioletHoldCombatAI
{
public:
    AssaultOnVioletHoldBossAI(Creature* pCreature, uint32_t encounterId) : AssaultOnVioletHoldCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        AssaultOnVioletHoldCombatAI::OnCombatStart(pTarget);
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

        AssaultOnVioletHoldCombatAI::OnCombatStop(pTarget);
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

// kept behind bars until the assault lets them out
class AssaultOnVioletHoldJailerAI : public AssaultOnVioletHoldBossAI
{
public:
    AssaultOnVioletHoldJailerAI(Creature* pCreature, uint32_t encounterId) : AssaultOnVioletHoldBossAI(pCreature, encounterId) {}

    void OnLoad() override
    {
        AssaultOnVioletHoldInstanceScript* pHold = getVioletHold();
        if (pHold == nullptr || !pHold->isReleased(mEncounterId))
            lockAway();

        onPrepare();
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_JAILER_RELEASE || !isAlive())
            return;

        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        setCanEnterCombat(true);
        setReactState(REACT_AGGRESSIVE);
        onBreakFree();
        setZoneWideCombat();
    }

protected:
    virtual void onPrepare() {}
    virtual void onBreakFree() {}

private:
    void lockAway()
    {
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        setCanEnterCombat(false);
        setReactState(REACT_PASSIVE);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Portal invaders rush the defenders once through

class AssaultOnVioletHoldInvaderAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldInvaderAI(c); }
    explicit AssaultOnVioletHoldInvaderAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_INVADER_CHARGE && isAlive())
            setZoneWideCombat();
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shivermaw

class AssaultOnVioletHoldShivermawAI : public AssaultOnVioletHoldJailerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldShivermawAI(c); }
    explicit AssaultOnVioletHoldShivermawAI(Creature* pCreature) : AssaultOnVioletHoldJailerAI(pCreature, DATA_SHIVERMAW) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_SHIVERMAW_FROST_BREATH, 5000);
        scriptEvents.addEvent(EVENT_SHIVERMAW_RELENTLESS_STORM, 9000);
        scriptEvents.addEvent(EVENT_SHIVERMAW_TAIL_SWEEP, 17000);
        scriptEvents.addEvent(EVENT_SHIVERMAW_ICE_BOMB, 40000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_SHIVERMAW_FRIGID_WINDS, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SHIVERMAW_FROST_BREATH:
                castSpellAOE(SPELL_SHIVERMAW_FROST_BREATH);
                scriptEvents.addEvent(EVENT_SHIVERMAW_FROST_BREATH, 26000);
                break;
            case EVENT_SHIVERMAW_RELENTLESS_STORM:
                castSpellAOE(SPELL_SHIVERMAW_RELENTLESS_STORM);
                scriptEvents.addEvent(EVENT_SHIVERMAW_RELENTLESS_STORM, 14000);
                break;
            case EVENT_SHIVERMAW_TAIL_SWEEP:
                castSpellAOE(SPELL_SHIVERMAW_TAIL_SWEEP);
                scriptEvents.addEvent(EVENT_SHIVERMAW_WING_BUFFET, 3000);
                scriptEvents.addEvent(EVENT_SHIVERMAW_TAIL_SWEEP, 18000);
                break;
            case EVENT_SHIVERMAW_WING_BUFFET:
                castSpellAOE(SPELL_SHIVERMAW_WING_BUFFET);
                break;
            case EVENT_SHIVERMAW_ICE_BOMB:
                warn("Shivermaw releases an Ice Bomb!");
                castSpellAOE(SPELL_SHIVERMAW_ICE_BOMB);
                scriptEvents.addEvent(EVENT_SHIVERMAW_ICE_BOMB, 60000);
                break;
            case EVENT_SHIVERMAW_FRIGID_WINDS:
                castSpellAOE(SPELL_SHIVERMAW_FRIGID_WINDS);
                scriptEvents.addEvent(EVENT_SHIVERMAW_FRIGID_WINDS, 60000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mindflayer Kaahrj

class AssaultOnVioletHoldKaahrjAI : public AssaultOnVioletHoldJailerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldKaahrjAI(c); }
    explicit AssaultOnVioletHoldKaahrjAI(Creature* pCreature) : AssaultOnVioletHoldJailerAI(pCreature, DATA_MINDFLAYER_KAAHRJ) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KAAHRJ_SHADOW_CRASH, 4000);
        scriptEvents.addEvent(EVENT_KAAHRJ_DOOM, 8000);
        scriptEvents.addEvent(EVENT_KAAHRJ_HYSTERIA, 13000);
        scriptEvents.addEvent(EVENT_KAAHRJ_ETERNAL_DARKNESS, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KAAHRJ_SHADOW_CRASH:
                castSpellAOE(SPELL_KAAHRJ_SHADOW_CRASH);
                scriptEvents.addEvent(EVENT_KAAHRJ_SHADOW_CRASH, 12000);
                break;
            case EVENT_KAAHRJ_DOOM:
                castSpellOnVictim(SPELL_KAAHRJ_DOOM);
                scriptEvents.addEvent(EVENT_KAAHRJ_DOOM, 12000);
                break;
            case EVENT_KAAHRJ_HYSTERIA:
                castAt(anyPlayer(), SPELL_KAAHRJ_HYSTERIA);
                scriptEvents.addEvent(EVENT_KAAHRJ_HYSTERIA, 12000);
                break;
            case EVENT_KAAHRJ_ETERNAL_DARKNESS:
                warn("Mindflayer Kaahrj calls Faceless Tendrils out of the void!");
                castSpellAOE(SPELL_KAAHRJ_ETERNAL_DARKNESS);
                for (uint32_t i = 0; i < 2; ++i)
                {
                    if (Unit* pTarget = anyPlayer())
                        raiseAdd(NPC_FACELESS_TENDRIL, nearUnit(pTarget, 4.0f));
                }
                scriptEvents.addEvent(EVENT_KAAHRJ_ETERNAL_DARKNESS, 36000);
                break;
            default:
                break;
        }
    }
};

// rooted, flays the minds around it every few seconds
class AssaultOnVioletHoldFacelessTendrilAI : public AssaultOnVioletHoldCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldFacelessTendrilAI(c); }
    explicit AssaultOnVioletHoldFacelessTendrilAI(Creature* pCreature) : AssaultOnVioletHoldCombatAI(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        setReactState(REACT_PASSIVE);
    }

protected:
    void onCombatTick(uint32_t diff) override
    {
        if (mFlayClock > diff)
        {
            mFlayClock -= diff;
            return;
        }

        mFlayClock = 3000;
        castSpellOnSelf(SPELL_TENDRIL_ETERNAL_DARKNESS, true);
    }

private:
    uint32_t mFlayClock = 500;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Millificent Manastorm

class AssaultOnVioletHoldMillificentAI : public AssaultOnVioletHoldJailerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldMillificentAI(c); }
    explicit AssaultOnVioletHoldMillificentAI(Creature* pCreature) : AssaultOnVioletHoldJailerAI(pCreature, DATA_MILLIFICENT_MANASTORM) {}

protected:
    void onEngage() override
    {
        mEnraged = false;
        scriptEvents.addEvent(EVENT_MILLIFICENT_FINGER_LASER, 6000);
        scriptEvents.addEvent(EVENT_MILLIFICENT_ROCKET_CHICKEN, 24000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mEnraged || _getHealthPercent() > 50)
            return;

        mEnraged = true;
        warn("Millificent Manastorm flies into a rage!");
        castSpellOnSelf(SPELL_MILLIFICENT_RAGE, true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MILLIFICENT_FINGER_LASER:
                castAt(anyPlayer(), SPELL_MILLIFICENT_FINGER_LASER);
                scriptEvents.addEvent(EVENT_MILLIFICENT_FINGER_LASER, 7000);
                break;
            case EVENT_MILLIFICENT_ROCKET_CHICKEN:
                if (Unit* pTarget = anyPlayer())
                {
                    warn("Millificent Manastorm unpacks a Thorium Rocket Chicken!");
                    raiseAdd(mEnraged ? NPC_REINFORCED_ROCKET_CHICKEN : NPC_THORIUM_ROCKET_CHICKEN, nearUnit(pTarget, 6.0f));
                }
                scriptEvents.addEvent(EVENT_MILLIFICENT_ROCKET_CHICKEN, 18000);
                break;
            default:
                break;
        }
    }

private:
    bool mEnraged = false;
};

// stands still and fires rockets at random players
class AssaultOnVioletHoldRocketChickenAI : public AssaultOnVioletHoldCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldRocketChickenAI(c); }
    explicit AssaultOnVioletHoldRocketChickenAI(Creature* pCreature) : AssaultOnVioletHoldCombatAI(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        setReactState(REACT_PASSIVE);
        castSpellOnSelf(SPELL_CHICKEN_SWARM_ROCKETS, true);
        if (getCreature()->getEntry() == NPC_REINFORCED_ROCKET_CHICKEN)
            castSpellOnSelf(SPELL_CHICKEN_THORIUM_PLATING, true);
    }

protected:
    void onCombatTick(uint32_t diff) override
    {
        if (mRocketClock > diff)
        {
            mRocketClock -= diff;
            return;
        }

        mRocketClock = 4000;
        if (!_isCasting())
            castAt(anyPlayer(), SPELL_CHICKEN_ROCKET);
    }

private:
    uint32_t mRocketClock = 2000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Festerface

class AssaultOnVioletHoldFesterfaceAI : public AssaultOnVioletHoldJailerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldFesterfaceAI(c); }
    explicit AssaultOnVioletHoldFesterfaceAI(Creature* pCreature) : AssaultOnVioletHoldJailerAI(pCreature, DATA_FESTERFACE) {}

protected:
    void onEngage() override
    {
        mBile = 0;
        mBileClock = 0;
        castSpellOnSelf(SPELL_FESTERFACE_AGGRO, true);
        scriptEvents.addEvent(EVENT_FESTERFACE_CONGEALING_VOMIT, 4000);
    }

    // heroic: two energy per second, a full stomach spews Black Bile
    void onCombatTick(uint32_t diff) override
    {
        if (!isHeroic())
            return;

        mBileClock += diff;
        while (mBileClock >= 1000)
        {
            mBileClock -= 1000;
            mBile += 2;
            if (mBile >= 100)
            {
                mBile = 0;
                scriptEvents.addEvent(EVENT_FESTERFACE_BLACK_BILE, 100);
            }
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_FESTERFACE_CONGEALING_VOMIT:
                if (Unit* pTarget = anyPlayer())
                {
                    warn("Festerface unleashes a Congealing Vomit!");
                    castSpell(pTarget, SPELL_FESTERFACE_CONGEALING_VOMIT);
                    spillGoo(pTarget);
                }
                scriptEvents.addEvent(EVENT_FESTERFACE_CONGEALING_VOMIT, 51000);
                break;
            case EVENT_FESTERFACE_BLACK_BILE:
                warn("Festerface loses control of his stomach!");
                raiseAdd(NPC_BLACK_BILE, nearUnit(getCreature(), 4.0f));
                break;
            default:
                break;
        }
    }

private:
    // the bile pools congeal along the line of the vomit
    void spillGoo(Unit* pTarget)
    {
        uint32_t const count = isHeroic() ? 5 : 3;
        LocationVector const origin = getCreature()->GetPosition();
        float const angle = getCreature()->getAbsoluteAngle(pTarget);

        for (uint32_t i = 0; i < count; ++i)
        {
            float const range = 8.0f + 5.0f * static_cast<float>(i);
            LocationVector const spot(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
            summonCreature(NPC_CONGEALED_GOO, spot, CORPSE_DESPAWN);
        }
    }

    uint32_t mBile = 0;
    uint32_t mBileClock = 0;
};

class AssaultOnVioletHoldCongealedGooAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldCongealedGooAI(c); }
    explicit AssaultOnVioletHoldCongealedGooAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        castSpellOnSelf(SPELL_GOO_SLIME_COSMETICS, true);
        castSpellOnSelf(SPELL_GOO_FETID_STENCH, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sael'orn

class AssaultOnVioletHoldSaelornAI : public AssaultOnVioletHoldJailerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldSaelornAI(c); }
    explicit AssaultOnVioletHoldSaelornAI(Creature* pCreature) : AssaultOnVioletHoldJailerAI(pCreature, DATA_SAELORN) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_SAELORN_VENOM_SPRAY, 3000);
        scriptEvents.addEvent(EVENT_SAELORN_WEB_GRAB, 14000);
        scriptEvents.addEvent(EVENT_SAELORN_PHASE_SPIDER, 15000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_SAELORN_TOXIC_BLOOD, 10000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SAELORN_VENOM_SPRAY:
            {
                Unit* pTarget = getBestPlayerTarget(TargetFilter_InRangeOnly, 0.0f, 35.0f);
                castAt(pTarget != nullptr ? pTarget : anyPlayer(), SPELL_SAELORN_VENOM_SPRAY);
                scriptEvents.addEvent(EVENT_SAELORN_VENOM_SPRAY, 27000);
                break;
            }
            case EVENT_SAELORN_WEB_GRAB:
                warn("Sael'orn pulls her prey in with a Web Grab!");
                castSpellAOE(SPELL_SAELORN_WEB_GRAB);
                scriptEvents.addEvent(EVENT_SAELORN_FEL_DETONATION, 500);
                scriptEvents.addEvent(EVENT_SAELORN_WEB_GRAB, 27000);
                break;
            case EVENT_SAELORN_FEL_DETONATION:
                castSpellAOE(SPELL_SAELORN_FEL_DETONATION);
                break;
            case EVENT_SAELORN_PHASE_SPIDER:
                if (Unit* pTarget = anyPlayer())
                    raiseAdd(NPC_PHASE_SPIDER, nearUnit(pTarget, 5.0f));
                break;
            case EVENT_SAELORN_TOXIC_BLOOD:
                castAt(anyPlayer(), SPELL_SAELORN_TOXIC_BLOOD);
                scriptEvents.addEvent(EVENT_SAELORN_TOXIC_BLOOD, 10000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Anub'esset

class AssaultOnVioletHoldAnubessetAI : public AssaultOnVioletHoldJailerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldAnubessetAI(c); }
    explicit AssaultOnVioletHoldAnubessetAI(Creature* pCreature) : AssaultOnVioletHoldJailerAI(pCreature, DATA_ANUBESSET) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ANUBESSET_MANDIBLE_STRIKE, 8000);
        scriptEvents.addEvent(EVENT_ANUBESSET_IMPALE, 18000);
        scriptEvents.addEvent(EVENT_ANUBESSET_CALL_OF_THE_SWARM, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ANUBESSET_MANDIBLE_STRIKE:
                castSpellOnVictim(SPELL_ANUBESSET_MANDIBLE_STRIKE);
                scriptEvents.addEvent(EVENT_ANUBESSET_MANDIBLE_STRIKE, 23000);
                break;
            case EVENT_ANUBESSET_IMPALE:
                castAt(anyPlayer(), SPELL_ANUBESSET_IMPALE);
                scriptEvents.addEvent(EVENT_ANUBESSET_IMPALE, 23000);
                break;
            case EVENT_ANUBESSET_CALL_OF_THE_SWARM:
                warn("Anub'esset calls Spitting Scarabs to his side!");
                for (uint32_t i = 0; i < 2; ++i)
                    raiseAdd(NPC_SPITTING_SCARAB, nearUnit(getCreature(), 6.0f));
                scriptEvents.addEvent(EVENT_ANUBESSET_CALL_OF_THE_SWARM, 60000);
                break;
            default:
                break;
        }
    }
};

class AssaultOnVioletHoldSpittingScarabAI : public AssaultOnVioletHoldCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldSpittingScarabAI(c); }
    explicit AssaultOnVioletHoldSpittingScarabAI(Creature* pCreature) : AssaultOnVioletHoldCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_SCARAB_POISON_SPIT, 2000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SCARAB_POISON_SPIT)
            return;

        castAt(anyPlayer(), SPELL_SCARAB_POISON_SPIT);
        scriptEvents.addEvent(EVENT_SCARAB_POISON_SPIT, 2000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Blood-Princess Thal'ena

class AssaultOnVioletHoldThalenaAI : public AssaultOnVioletHoldJailerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldThalenaAI(c); }
    explicit AssaultOnVioletHoldThalenaAI(Creature* pCreature) : AssaultOnVioletHoldJailerAI(pCreature, DATA_BLOOD_PRINCESS_THALENA) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_THALENA_SHROUD_OF_SORROW, true);
        scriptEvents.addEvent(EVENT_THALENA_VAMPYRS_KISS, 6000);
        scriptEvents.addEvent(EVENT_THALENA_BLOOD_SWARM, 20000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_THALENA_BLOOD_CALL, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_THALENA_VAMPYRS_KISS:
                // a single bite per pull
                warn("Blood-Princess Thal'ena shares her Vampyr's Kiss!");
                castAt(offTankPlayer(), SPELL_THALENA_VAMPYRS_KISS);
                break;
            case EVENT_THALENA_BLOOD_SWARM:
                castSpellAOE(SPELL_THALENA_BLOOD_SWARM);
                scriptEvents.addEvent(EVENT_THALENA_BLOOD_SWARM, 21000);
                break;
            case EVENT_THALENA_BLOOD_CALL:
                castSpellAOE(SPELL_THALENA_BLOOD_CALL);
                scriptEvents.addEvent(EVENT_THALENA_BLOOD_CALL, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fel Lord Betrug, arrives once both prisoners have fallen

class AssaultOnVioletHoldBetrugAI : public AssaultOnVioletHoldJailerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AssaultOnVioletHoldBetrugAI(c); }
    explicit AssaultOnVioletHoldBetrugAI(Creature* pCreature) : AssaultOnVioletHoldJailerAI(pCreature, DATA_FEL_LORD_BETRUG) {}

protected:
    void onPrepare() override { castSpellOnSelf(SPELL_BETRUG_ARMED, true); }

    void onBreakFree() override
    {
        MoveTeleport(BetrugArrivalSpot);
        warn("Fel Lord Betrug storms into the prison!");
    }

    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_BETRUG_CHAOTIC_ENERGY, 6000);
        scriptEvents.addEvent(EVENT_BETRUG_FEL_SLASH, 12000);
        scriptEvents.addEvent(EVENT_BETRUG_MIGHTY_SMASH, 32000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_BETRUG_SEED_OF_DESTRUCTION, 16000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BETRUG_CHAOTIC_ENERGY:
                castSpellOnSelf(SPELL_BETRUG_CHAOTIC_ENERGY, true);
                scriptEvents.addEvent(EVENT_BETRUG_CHAOTIC_ENERGY, 12000);
                break;
            case EVENT_BETRUG_FEL_SLASH:
                castSpellAOE(SPELL_BETRUG_FEL_SLASH);
                scriptEvents.addEvent(EVENT_BETRUG_FEL_SLASH, 16000);
                break;
            case EVENT_BETRUG_MIGHTY_SMASH:
                warn("Fel Lord Betrug prepares a Mighty Smash!");
                castSpellAOE(SPELL_BETRUG_MIGHTY_SMASH);
                scriptEvents.addEvent(EVENT_BETRUG_MIGHTY_SMASH, 54000);
                break;
            case EVENT_BETRUG_SEED_OF_DESTRUCTION:
                castSpellAOE(SPELL_BETRUG_SEED_OF_DESTRUCTION);
                scriptEvents.addEvent(EVENT_BETRUG_SEED_OF_DESTRUCTION, 22000);
                break;
            default:
                break;
        }
    }
};

void SetupAssaultOnVioletHold(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ASSAULT_ON_VIOLET_HOLD, &AssaultOnVioletHoldInstanceScript::Create);

    mgr->register_creature_gossip(NPC_LIEUTENANT_SINCLARI, new AssaultOnVioletHoldSinclariGossip());

    mgr->register_creature_script(BOSS_SHIVERMAW, &AssaultOnVioletHoldShivermawAI::Create);

    mgr->register_creature_script(BOSS_MINDFLAYER_KAAHRJ, &AssaultOnVioletHoldKaahrjAI::Create);
    mgr->register_creature_script(NPC_FACELESS_TENDRIL, &AssaultOnVioletHoldFacelessTendrilAI::Create);

    mgr->register_creature_script(BOSS_MILLIFICENT_MANASTORM, &AssaultOnVioletHoldMillificentAI::Create);
    mgr->register_creature_script(NPC_THORIUM_ROCKET_CHICKEN, &AssaultOnVioletHoldRocketChickenAI::Create);
    mgr->register_creature_script(NPC_REINFORCED_ROCKET_CHICKEN, &AssaultOnVioletHoldRocketChickenAI::Create);

    mgr->register_creature_script(BOSS_FESTERFACE, &AssaultOnVioletHoldFesterfaceAI::Create);
    mgr->register_creature_script(NPC_CONGEALED_GOO, &AssaultOnVioletHoldCongealedGooAI::Create);

    mgr->register_creature_script(BOSS_SAELORN, &AssaultOnVioletHoldSaelornAI::Create);

    mgr->register_creature_script(BOSS_ANUBESSET, &AssaultOnVioletHoldAnubessetAI::Create);
    mgr->register_creature_script(NPC_SPITTING_SCARAB, &AssaultOnVioletHoldSpittingScarabAI::Create);

    mgr->register_creature_script(BOSS_BLOOD_PRINCESS_THALENA, &AssaultOnVioletHoldThalenaAI::Create);

    mgr->register_creature_script(BOSS_FEL_LORD_BETRUG, &AssaultOnVioletHoldBetrugAI::Create);

    uint32_t const invaders[] =
    {
        NPC_PORTAL_GUARDIAN_1, NPC_PORTAL_GUARDIAN_2, NPC_PORTAL_KEEPER_1, NPC_PORTAL_KEEPER_2,
        NPC_FELGUARD_DESTROYER_1, NPC_FELGUARD_DESTROYER_2, NPC_SHADOW_COUNCIL_WARLOCK, NPC_EREDAR_SHADOW_MENDER,
        NPC_INFILTRATOR_ASSASSIN, NPC_WRATHLORD_BULWARK, NPC_FELSTALKER_RAVENER_1, NPC_FELSTALKER_RAVENER_2,
        NPC_EREDAR_INVADER_1, NPC_EREDAR_INVADER_2
    };

    for (uint32_t entry : invaders)
        mgr->register_creature_script(entry, &AssaultOnVioletHoldInvaderAI::Create);
}
