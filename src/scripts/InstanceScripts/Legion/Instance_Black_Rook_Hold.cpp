/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Black_Rook_Hold.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <list>
#include <map>
#include <string>
#include <vector>

enum BlackRookHoldEvents
{
    EVENT_AMALGAM_SWIRLING_SCYTHE = 1,
    EVENT_AMALGAM_SOUL_ECHOES,
    EVENT_AMALGAM_REAP_SOUL,
    EVENT_AMALGAM_CALL_SOULS,
    EVENT_AMALGAM_SOUL_COUNT,

    EVENT_ILLYSANNA_BRUTAL_GLAIVE,
    EVENT_ILLYSANNA_VENGEFUL_SHEAR,
    EVENT_ILLYSANNA_DARK_RUSH,
    EVENT_ILLYSANNA_DARK_RUSH_DASH,
    EVENT_ILLYSANNA_RAMPART_ADDS,
    EVENT_ILLYSANNA_EYE_BEAMS,
    EVENT_ILLYSANNA_DESCEND,

    EVENT_SMASHSPITE_STOMP,
    EVENT_SMASHSPITE_FEL_BAT,
    EVENT_SMASHSPITE_HATEFUL_GAZE,
    EVENT_SMASHSPITE_HATEFUL_CHARGE,
    EVENT_SMASHSPITE_BRUTALITY,

    EVENT_KURTALOS_UNERRING_SHEAR,
    EVENT_KURTALOS_WHIRLING_BLADE,

    EVENT_LATOSIUS_BLINK,
    EVENT_LATOSIUS_SHADOW_BOLT,
    EVENT_LATOSIUS_DARK_BLAST,
    EVENT_LATOSIUS_SAP_SOUL,
    EVENT_LATOSIUS_UNMASK,

    EVENT_DANTALIONAX_VOLLEY,
    EVENT_DANTALIONAX_HYPNOSIS,
    EVENT_DANTALIONAX_GUILE,
    EVENT_DANTALIONAX_IMAGE,
    EVENT_DANTALIONAX_GUILE_END,
    EVENT_DANTALIONAX_STINGING_SWARM
};

namespace
{
    enum class BlackRookDoorKind : uint8_t
    {
        Room,
        Passage
    };

    struct BlackRookDoorRule
    {
        uint32_t entry;
        uint32_t encounterId;
        BlackRookDoorKind kind;
    };

    // room doors shut during the fight, passages open after the kill
    BlackRookDoorRule const BlackRookDoorRules[] =
    {
        { GO_AMALGAM_DOOR_1,    DATA_AMALGAM_OF_SOULS,  BlackRookDoorKind::Room },
        { GO_AMALGAM_DOOR_2,    DATA_AMALGAM_OF_SOULS,  BlackRookDoorKind::Room },
        { GO_AMALGAM_DOOR_3,    DATA_AMALGAM_OF_SOULS,  BlackRookDoorKind::Room },
        { GO_AMALGAM_DOOR_4,    DATA_AMALGAM_OF_SOULS,  BlackRookDoorKind::Room },
        { GO_AMALGAM_EXIT,      DATA_AMALGAM_OF_SOULS,  BlackRookDoorKind::Passage },
        { GO_ILLYSANNA_DOOR_1,  DATA_ILLYSANNA,         BlackRookDoorKind::Passage },
        { GO_ILLYSANNA_DOOR_2,  DATA_ILLYSANNA,         BlackRookDoorKind::Passage },
        { GO_ILLYSANNA_DOOR_3,  DATA_ILLYSANNA,         BlackRookDoorKind::Room },
        { GO_SMASHSPITE_DOOR,   DATA_SMASHSPITE,        BlackRookDoorKind::Room },
        { GO_SMASHSPITE_EXIT_1, DATA_SMASHSPITE,        BlackRookDoorKind::Passage },
        { GO_SMASHSPITE_EXIT_2, DATA_SMASHSPITE,        BlackRookDoorKind::Passage },
        { GO_KURTALOS_DOOR,     DATA_KURTALOS,          BlackRookDoorKind::Room }
    };

    BlackRookDoorRule const* findBlackRookDoor(uint32_t entry)
    {
        for (BlackRookDoorRule const& rule : BlackRookDoorRules)
        {
            if (rule.entry == entry)
                return &rule;
        }

        return nullptr;
    }

    // crypt floor the restless souls drift towards
    LocationVector const AmalgamCryptCentre(3251.35f, 7582.79f, 12.75f);
    LocationVector const RestlessSoulRises[] =
    {
        { 3288.37f, 7593.30f, 14.10f, 3.45f },
        { 3259.17f, 7620.32f, 14.10f, 4.53f },
        { 3239.09f, 7618.44f, 14.10f, 5.05f },
        { 3213.97f, 7587.99f, 14.10f, 6.12f },
        { 3215.85f, 7567.98f, 14.10f, 0.36f },
        { 3245.75f, 7541.42f, 14.10f, 1.41f },
        { 3265.50f, 7543.86f, 14.10f, 1.90f }
    };
    uint32_t const AmalgamSoulPhaseMs = 35000;

    LocationVector const IllysannaRampartFloor(3089.76f, 7299.66f, 103.53f);
    LocationVector const IllysannaArcanistSpot(3105.11f, 7296.94f, 103.28f, 3.14f);
    LocationVector const IllysannaVanguardSpot(3083.07f, 7313.35f, 103.28f, 4.92f);
    uint32_t const IllysannaFuryCap = 100;
    uint32_t const IllysannaFuryOnPull = 50;
    uint32_t const IllysannaEyeBeamCost = 34;

    uint32_t const SmashspiteBrutalityCap = 100;

    // ring of spots on the Ravencrest roof
    LocationVector const LatosiusRoofSpots[] =
    {
        { 3169.09f, 7432.38f, 271.60f, 5.42f },
        { 3165.53f, 7421.75f, 271.60f, 5.85f },
        { 3158.29f, 7414.00f, 271.60f, 0.00f },
        { 3164.16f, 7395.63f, 271.60f, 0.62f },
        { 3173.76f, 7387.44f, 271.60f, 1.08f },
        { 3184.47f, 7383.66f, 271.60f, 1.49f },
        { 3200.61f, 7390.41f, 271.60f, 2.14f },
        { 3205.70f, 7400.89f, 271.60f, 2.61f },
        { 3210.50f, 7409.22f, 271.60f, 3.02f },
        { 3204.84f, 7427.28f, 271.60f, 3.84f },
        { 3195.91f, 7430.39f, 271.60f, 4.24f },
        { 3186.87f, 7436.26f, 271.60f, 4.70f }
    };
    uint32_t const LatosiusRoofSpotCount = 12;
    uint32_t const DreadlordImageCount = 11;
    uint32_t const HiddenDisplayId = 11686;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class BlackRookHoldInstanceScript : public InstanceScript
{
public:
    explicit BlackRookHoldInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(BlackRookHoldEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new BlackRookHoldInstanceScript(pMapMgr); }

    // Kur'talos falls halfway, only the unmasked dreadlord ends the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (id == DATA_KURTALOS && state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !mDreadlordSlain)
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_KURTALOS:
            case BOSS_LATOSIUS:
            case BOSS_DANTALIONAX:
                mTrackedGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
                break;
            default:
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findBlackRookDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        alignDoor(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor);
        }

        if (id != DATA_KURTALOS)
            return;

        if (state == EncounterStates::Failed)
        {
            mDreadlordSlain = false;

            Creature* pKurtalos = getTracked(BOSS_KURTALOS);
            if (pKurtalos != nullptr && !pKurtalos->isAlive())
                pKurtalos->despawn(1000, 5000);

            if (Creature* pDreadlord = getTracked(BOSS_DANTALIONAX))
                pDreadlord->despawn(1000, 0);
        }
        else if (state == EncounterStates::Performed)
        {
            if (Creature* pLatosius = getTracked(BOSS_LATOSIUS))
                pLatosius->despawn(1000, 0);
        }
    }

    void markDreadlordSlain() { mDreadlordSlain = true; }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        Creature* pTarget = getTracked(entry);
        if (pTarget != nullptr && pTarget->isAlive() && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }

private:
    void alignDoor(GameObject* pDoor)
    {
        BlackRookDoorRule const* rule = findBlackRookDoor(pDoor->getEntry());
        if (rule == nullptr)
            return;

        EncounterStates const state = getBossState(rule->encounterId);
        bool const wantOpen = rule->kind == BlackRookDoorKind::Room ? state != EncounterStates::InProgress : state == EncounterStates::Performed;
        bool const isOpen = pDoor->getState() != GO_STATE_CLOSED;
        if (wantOpen != isOpen)
            pDoor->setState(wantOpen ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    std::map<uint32_t, WoWGuid> mTrackedGuids;
    std::vector<WoWGuid> mDoorGuids;
    bool mDreadlordSlain = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class BlackRookHoldCombatAI : public CreatureAIScript
{
public:
    explicit BlackRookHoldCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    BlackRookHoldInstanceScript* getBlackRookHold()
    {
        if (getCreature()->GetMapId() != MAP_BLACK_ROOK_HOLD)
            return nullptr;

        return static_cast<BlackRookHoldInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return getBestPlayerTarget(TargetFilter_None); }

    // anyone but the tank, the tank when alone
    Unit* offTankPlayer()
    {
        if (Unit* pTarget = getBestPlayerTarget(TargetFilter_NotCurrent))
            return pTarget;

        return anyPlayer();
    }

    Unit* farthestPlayer()
    {
        Unit* pFarthest = nullptr;
        float farthestRange = -1.0f;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer == nullptr || !pPlayer->isAlive())
                continue;

            float const range = getCreature()->getDistance(pPlayer);
            if (range > farthestRange)
            {
                farthestRange = range;
                pFarthest = pPlayer;
            }
        }

        return pFarthest;
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

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void notify(uint32_t entry, int32_t action)
    {
        if (BlackRookHoldInstanceScript* pHold = getBlackRookHold())
            pHold->sendAction(entry, action);
    }

    // brings a resting partner into the fight
    void pullPartner(uint32_t entry)
    {
        BlackRookHoldInstanceScript* pHold = getBlackRookHold();
        if (pHold == nullptr)
            return;

        Creature* pPartner = pHold->getTracked(entry);
        if (pPartner != nullptr && pPartner->isAlive() && !pPartner->isInCombat())
            setZoneWideCombat(pPartner);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class BlackRookHoldBossAI : public BlackRookHoldCombatAI
{
public:
    BlackRookHoldBossAI(Creature* pCreature, uint32_t encounterId, bool endsEncounter = true) : BlackRookHoldCombatAI(pCreature), mEncounterId(encounterId), mEndsEncounter(endsEncounter) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        BlackRookHoldCombatAI::OnCombatStart(pTarget);
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

        BlackRookHoldCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        onDefeat();

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            if (mEndsEncounter)
                pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }
    }

protected:
    virtual void onDefeat() {}

    uint32_t mEncounterId;
    bool mEndsEncounter;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Amalgam of Souls

class BlackRookHoldAmalgamAI : public BlackRookHoldBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BlackRookHoldAmalgamAI(c); }
    explicit BlackRookHoldAmalgamAI(Creature* pCreature) : BlackRookHoldBossAI(pCreature, DATA_AMALGAM_OF_SOULS) {}

protected:
    void onEngage() override
    {
        mSoulsCalled = false;
        mSoulPhase = false;
        mSoulPhaseLeft = 0;
        startCryptRotation();
    }

    void onReset() override { endSoulPhase(false); }

    void onCombatTick(uint32_t diff) override
    {
        // heroic: the crypt souls are lured in once
        if (!mSoulsCalled && isHeroic() && _getHealthPercent() <= 50)
            beginSoulPhase();

        if (!mSoulPhase)
            return;

        if (_getHealthPercent() < 16)
        {
            endSoulPhase(true);
            return;
        }

        if (mSoulPhaseLeft == 0)
            return;

        if (mSoulPhaseLeft <= diff)
            endSoulPhase(true);
        else
            mSoulPhaseLeft -= diff;
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AMALGAM_SWIRLING_SCYTHE:
                castAt(offTankPlayer(), SPELL_AMALGAM_SWIRLING_SCYTHE);
                scriptEvents.addEvent(EVENT_AMALGAM_SWIRLING_SCYTHE, 20000);
                break;
            case EVENT_AMALGAM_SOUL_ECHOES:
                castAt(anyPlayer(), SPELL_AMALGAM_SOUL_ECHOES);
                scriptEvents.addEvent(EVENT_AMALGAM_SOUL_ECHOES, 28000);
                break;
            case EVENT_AMALGAM_REAP_SOUL:
                warn("The Amalgam of Souls prepares to Reap Soul!");
                castSpellAOE(SPELL_AMALGAM_REAP_SOUL);
                scriptEvents.addEvent(EVENT_AMALGAM_REAP_SOUL, 13000);
                break;
            case EVENT_AMALGAM_CALL_SOULS:
                callRestlessSouls();
                break;
            case EVENT_AMALGAM_SOUL_COUNT:
                if (anySoulLeft())
                    scriptEvents.addEvent(EVENT_AMALGAM_SOUL_COUNT, 2000);
                else
                    endSoulPhase(true);
                break;
            default:
                break;
        }
    }

private:
    void startCryptRotation()
    {
        scriptEvents.addEvent(EVENT_AMALGAM_SWIRLING_SCYTHE, 8000);
        scriptEvents.addEvent(EVENT_AMALGAM_SOUL_ECHOES, 14000);
        scriptEvents.addEvent(EVENT_AMALGAM_REAP_SOUL, 20000);
    }

    void beginSoulPhase()
    {
        mSoulsCalled = true;
        mSoulPhase = true;
        mSoulPhaseLeft = 0;

        scriptEvents.resetEvents();
        setReactState(REACT_PASSIVE);
        attackStop();
        moveTo(AmalgamCryptCentre.x, AmalgamCryptCentre.y, AmalgamCryptCentre.z);
        scriptEvents.addEvent(EVENT_AMALGAM_CALL_SOULS, 3000);
    }

    void callRestlessSouls()
    {
        setRooted(true);
        warn("The Amalgam of Souls calls the restless souls of the crypt!");
        castSpellOnSelf(SPELL_AMALGAM_CALL_SOULS);

        for (LocationVector const& rise : RestlessSoulRises)
            summonCreature(NPC_RESTLESS_SOUL, rise, CORPSE_DESPAWN);

        mSoulPhaseLeft = AmalgamSoulPhaseMs;
        scriptEvents.addEvent(EVENT_AMALGAM_SOUL_COUNT, 2000);
    }

    bool anySoulLeft()
    {
        std::list<Creature*> souls;
        GetCreatureListWithEntryInGrid(souls, NPC_RESTLESS_SOUL, 100.0f);
        for (Creature* pSoul : souls)
        {
            if (pSoul->isAlive())
                return true;
        }

        return false;
    }

    void endSoulPhase(bool burst)
    {
        if (!mSoulPhase)
            return;

        mSoulPhase = false;
        mSoulPhaseLeft = 0;
        summons.despawnEntry(NPC_RESTLESS_SOUL);
        scriptEvents.removeEvent(EVENT_AMALGAM_CALL_SOULS);
        scriptEvents.removeEvent(EVENT_AMALGAM_SOUL_COUNT);
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);

        if (!burst)
            return;

        castSpellAOE(SPELL_AMALGAM_SOUL_BURST);
        startCryptRotation();
    }

    bool mSoulsCalled = false;
    bool mSoulPhase = false;
    uint32_t mSoulPhaseLeft = 0;
};

// drifts to the Amalgam and feeds it unless slain on the way
class BlackRookHoldRestlessSoulAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BlackRookHoldRestlessSoulAI(c); }
    explicit BlackRookHoldRestlessSoulAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        driftToCentre();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mConsumed || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mCheckClock > diff)
        {
            mCheckClock -= diff;
            return;
        }

        mCheckClock = 1000;
        if (getCreature()->getDistance(AmalgamCryptCentre) > 3.0f)
        {
            driftToCentre();
            return;
        }

        mConsumed = true;
        if (Creature* pAmalgam = findNearestCreature(BOSS_AMALGAM_OF_SOULS, 40.0f))
            castSpell(pAmalgam, SPELL_AMALGAM_SOULGORGE, true);

        despawn(500, 0);
    }

private:
    void driftToCentre() { moveTo(AmalgamCryptCentre.x, AmalgamCryptCentre.y, AmalgamCryptCentre.z, false); }

    uint32_t mCheckClock = 1000;
    bool mConsumed = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Illysanna Ravencrest, Fury fills on the ground and drains in the air

class BlackRookHoldIllysannaAI : public BlackRookHoldBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BlackRookHoldIllysannaAI(c); }
    explicit BlackRookHoldIllysannaAI(Creature* pCreature) : BlackRookHoldBossAI(pCreature, DATA_ILLYSANNA) {}

protected:
    void onEngage() override
    {
        mFury = IllysannaFuryOnPull;
        mFuryClock = 0;
        mAloft = false;
        startGroundRotation();
    }

    void onReset() override { descend(false); }

    void onCombatTick(uint32_t diff) override
    {
        if (mAloft)
            return;

        mFuryClock += diff;
        while (mFuryClock >= 1000)
        {
            mFuryClock -= 1000;
            if (mFury < IllysannaFuryCap)
                ++mFury;
        }

        if (mFury >= IllysannaFuryCap && !_isCasting())
            takeToRamparts();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ILLYSANNA_BRUTAL_GLAIVE:
                castAt(offTankPlayer(), SPELL_ILLYSANNA_BRUTAL_GLAIVE);
                scriptEvents.addEvent(EVENT_ILLYSANNA_BRUTAL_GLAIVE, 16000);
                break;
            case EVENT_ILLYSANNA_VENGEFUL_SHEAR:
                castSpellOnVictim(SPELL_ILLYSANNA_VENGEFUL_SHEAR);
                scriptEvents.addEvent(EVENT_ILLYSANNA_VENGEFUL_SHEAR, Util::getRandomUInt(12000, 15000));
                break;
            case EVENT_ILLYSANNA_DARK_RUSH:
                warn("Illysanna Ravencrest fixes her gaze for a Dark Rush!");
                castSpellAOE(SPELL_ILLYSANNA_DARK_RUSH_MARK);
                scriptEvents.addEvent(EVENT_ILLYSANNA_BRUTAL_GLAIVE, 9000);
                scriptEvents.addEvent(EVENT_ILLYSANNA_VENGEFUL_SHEAR, 9000);
                scriptEvents.addEvent(EVENT_ILLYSANNA_DARK_RUSH_DASH, 6000);
                scriptEvents.addEvent(EVENT_ILLYSANNA_DARK_RUSH, 30000);
                break;
            case EVENT_ILLYSANNA_DARK_RUSH_DASH:
                castAt(offTankPlayer(), SPELL_ILLYSANNA_DARK_RUSH);
                break;
            case EVENT_ILLYSANNA_RAMPART_ADDS:
                if (isHeroic())
                    raiseAdd(NPC_RISEN_ARCANIST, IllysannaArcanistSpot);
                raiseAdd(NPC_SOUL_TORN_VANGUARD, IllysannaVanguardSpot);
                break;
            case EVENT_ILLYSANNA_EYE_BEAMS:
                castAt(anyPlayer(), SPELL_ILLYSANNA_EYE_BEAMS);
                mFury = mFury > IllysannaEyeBeamCost ? mFury - IllysannaEyeBeamCost : 0;
                if (mFury == 0)
                    scriptEvents.addEvent(EVENT_ILLYSANNA_DESCEND, 14000);
                else
                    scriptEvents.addEvent(EVENT_ILLYSANNA_EYE_BEAMS, 13000);
                break;
            case EVENT_ILLYSANNA_DESCEND:
                descend(true);
                break;
            default:
                break;
        }
    }

private:
    void startGroundRotation()
    {
        scriptEvents.addEvent(EVENT_ILLYSANNA_BRUTAL_GLAIVE, 6000);
        scriptEvents.addEvent(EVENT_ILLYSANNA_VENGEFUL_SHEAR, 9000);
        scriptEvents.addEvent(EVENT_ILLYSANNA_DARK_RUSH, 12000);
    }

    void takeToRamparts()
    {
        mAloft = true;
        scriptEvents.resetEvents();

        warn("Illysanna Ravencrest leaps to the ramparts!");
        attackStop();
        setReactState(REACT_PASSIVE);
        castSpellOnSelf(SPELL_ILLYSANNA_LEAP, true);
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        scriptEvents.addEvent(EVENT_ILLYSANNA_RAMPART_ADDS, 1000);
        scriptEvents.addEvent(EVENT_ILLYSANNA_EYE_BEAMS, 2000);
    }

    void descend(bool resume)
    {
        if (!mAloft)
            return;

        mAloft = false;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);

        if (!resume)
            return;

        moveJump(IllysannaRampartFloor, 25.0f, 15.0f);
        mFury = 0;
        mFuryClock = 0;
        startGroundRotation();
    }

    uint32_t mFury = 0;
    uint32_t mFuryClock = 0;
    bool mAloft = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Smashspite the Hateful, Brutality builds with every swing

class BlackRookHoldSmashspiteAI : public BlackRookHoldBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BlackRookHoldSmashspiteAI(c); }
    explicit BlackRookHoldSmashspiteAI(Creature* pCreature) : BlackRookHoldBossAI(pCreature, DATA_SMASHSPITE) {}

protected:
    void onEngage() override
    {
        mBrutality = 0;
        scriptEvents.addEvent(EVENT_SMASHSPITE_BRUTALITY, 2000);
        scriptEvents.addEvent(EVENT_SMASHSPITE_STOMP, 12000);
        scriptEvents.addEvent(EVENT_SMASHSPITE_FEL_BAT, 25000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_SMASHSPITE_HATEFUL_GAZE, 6000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SMASHSPITE_BRUTALITY:
                gainBrutality();
                scriptEvents.addEvent(EVENT_SMASHSPITE_BRUTALITY, 2000);
                break;
            case EVENT_SMASHSPITE_STOMP:
                warn("Smashspite the Hateful raises his foot for an Earthshaking Stomp!");
                castSpellAOE(SPELL_SMASHSPITE_EARTHSHAKING_STOMP);
                scriptEvents.addEvent(EVENT_SMASHSPITE_STOMP, 17000);
                break;
            case EVENT_SMASHSPITE_FEL_BAT:
                rouseFelBat();
                scriptEvents.addEvent(EVENT_SMASHSPITE_FEL_BAT, 25000);
                break;
            case EVENT_SMASHSPITE_HATEFUL_GAZE:
                if (Unit* pTarget = farthestPlayer())
                {
                    warn("Smashspite the Hateful fixes his Hateful Gaze on a distant foe!");
                    castSpell(pTarget, SPELL_SMASHSPITE_HATEFUL_GAZE);
                    scriptEvents.addEvent(EVENT_SMASHSPITE_HATEFUL_CHARGE, 5000);
                    scriptEvents.addEvent(EVENT_SMASHSPITE_STOMP, 10000);
                }
                scriptEvents.addEvent(EVENT_SMASHSPITE_HATEFUL_GAZE, 25000);
                break;
            case EVENT_SMASHSPITE_HATEFUL_CHARGE:
                castAt(farthestPlayer(), SPELL_SMASHSPITE_HATEFUL_CHARGE);
                break;
            default:
                break;
        }
    }

private:
    // melee swings feed the meter, a full meter ends in a Brutal Haymaker
    void gainBrutality()
    {
        Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
        if (pVictim == nullptr || getRangeToObject(pVictim) > 8.0f)
            return;

        mBrutality += Util::getRandomUInt(6, 9);
        if (mBrutality < SmashspiteBrutalityCap)
            return;

        mBrutality = 0;
        warn("Smashspite the Hateful winds up a Brutal Haymaker!");
        castSpellOnVictim(SPELL_SMASHSPITE_BRUTAL_HAYMAKER);
    }

    // one of the bats perched around the courtyard spits at a player
    void rouseFelBat()
    {
        Unit* pTarget = anyPlayer();
        if (pTarget == nullptr)
            return;

        std::list<Creature*> bats;
        GetCreatureListWithEntryInGrid(bats, NPC_FEL_BAT, 80.0f);

        std::vector<Creature*> awake;
        for (Creature* pBat : bats)
        {
            if (pBat->isAlive())
                awake.push_back(pBat);
        }

        if (awake.empty())
            return;

        Creature* pBat = awake[Util::getRandomUInt(static_cast<uint32_t>(awake.size() - 1))];
        pBat->castSpell(pTarget, SPELL_FEL_BAT_FEL_VOMIT, false);
    }

    uint32_t mBrutality = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lord Kur'talos Ravencrest, betrayed by his advisor at a fifth of his health

class BlackRookHoldKurtalosAI : public BlackRookHoldBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BlackRookHoldKurtalosAI(c); }
    explicit BlackRookHoldKurtalosAI(Creature* pCreature) : BlackRookHoldBossAI(pCreature, DATA_KURTALOS, false) {}

    void OnCombatStart(Unit* pTarget) override
    {
        BlackRookHoldBossAI::OnCombatStart(pTarget);
        pullPartner(BOSS_LATOSIUS);
    }

    // he cannot fall before the betrayal
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (mSapped)
            return;

        uint32_t const health = getCreature()->getHealth();
        if (*damage >= health)
            *damage = health > 1 ? health - 1 : 0;
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_KURTALOS_SOUL_SAPPED && mSapped && isAlive())
            getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
    }

protected:
    void onEngage() override
    {
        mSapped = false;
        scriptEvents.addEvent(EVENT_KURTALOS_UNERRING_SHEAR, 6000);
        scriptEvents.addEvent(EVENT_KURTALOS_WHIRLING_BLADE, 10000);
    }

    void onReset() override
    {
        mSapped = false;
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mSapped || _getHealthPercent() > 20)
            return;

        mSapped = true;
        scriptEvents.resetEvents();
        attackStop();
        setReactState(REACT_PASSIVE);
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

        warn("Latosius turns on Lord Kur'talos Ravencrest!");
        notify(BOSS_LATOSIUS, ACTION_LATOSIUS_BETRAYAL);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KURTALOS_UNERRING_SHEAR:
                castSpellOnVictim(SPELL_KURTALOS_UNERRING_SHEAR);
                scriptEvents.addEvent(EVENT_KURTALOS_UNERRING_SHEAR, 13000);
                break;
            case EVENT_KURTALOS_WHIRLING_BLADE:
                castAt(anyPlayer(), SPELL_KURTALOS_WHIRLING_BLADE);
                scriptEvents.addEvent(EVENT_KURTALOS_WHIRLING_BLADE, 10000);
                break;
            default:
                break;
        }
    }

private:
    bool mSapped = false;
};

// out of reach on the roof, unmasks himself once Kur'talos is drained
class BlackRookHoldLatosiusAI : public BlackRookHoldBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BlackRookHoldLatosiusAI(c); }
    explicit BlackRookHoldLatosiusAI(Creature* pCreature) : BlackRookHoldBossAI(pCreature, DATA_KURTALOS, false) {}

    void OnLoad() override
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance != nullptr && pInstance->getBossState(DATA_KURTALOS) == EncounterStates::Performed)
        {
            despawn(1000, 0);
            return;
        }

        keepAloof();
    }

    void OnCombatStart(Unit* pTarget) override
    {
        BlackRookHoldBossAI::OnCombatStart(pTarget);
        pullPartner(BOSS_KURTALOS);
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_LATOSIUS_BETRAYAL || mBetrayed || !_isInCombat())
            return;

        mBetrayed = true;
        scriptEvents.resetEvents();
        scriptEvents.addEvent(EVENT_LATOSIUS_SAP_SOUL, 2000);
        scriptEvents.addEvent(EVENT_LATOSIUS_UNMASK, 10000);
    }

protected:
    void onEngage() override
    {
        mBetrayed = false;
        keepAloof();
        scriptEvents.addEvent(EVENT_LATOSIUS_BLINK, 500);
        scriptEvents.addEvent(EVENT_LATOSIUS_DARK_BLAST, 10000);
    }

    void onReset() override
    {
        mBetrayed = false;
        getCreature()->resetDisplayId();
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        MoveTeleport(getCreature()->GetSpawnPosition());
        keepAloof();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LATOSIUS_BLINK:
                MoveTeleport(LatosiusRoofSpots[Util::getRandomUInt(LatosiusRoofSpotCount - 1)]);
                scriptEvents.addEvent(EVENT_LATOSIUS_SHADOW_BOLT, 500);
                break;
            case EVENT_LATOSIUS_SHADOW_BOLT:
                castAt(anyPlayer(), SPELL_LATOSIUS_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_LATOSIUS_SHADOW_BOLT, 3000);
                break;
            case EVENT_LATOSIUS_DARK_BLAST:
                warn("Latosius begins to cast Dark Blast!");
                castSpellAOE(SPELL_LATOSIUS_DARK_BLAST);
                scriptEvents.removeEvent(EVENT_LATOSIUS_SHADOW_BOLT);
                scriptEvents.addEvent(EVENT_LATOSIUS_BLINK, 5000);
                scriptEvents.addEvent(EVENT_LATOSIUS_DARK_BLAST, 18000);
                break;
            case EVENT_LATOSIUS_SAP_SOUL:
                if (BlackRookHoldInstanceScript* pHold = getBlackRookHold())
                {
                    if (Creature* pKurtalos = pHold->getTracked(BOSS_KURTALOS))
                        castSpell(pKurtalos, SPELL_LATOSIUS_SAP_SOUL, true);
                }
                break;
            case EVENT_LATOSIUS_UNMASK:
                unmask();
                break;
            default:
                break;
        }
    }

private:
    void keepAloof()
    {
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        _setMeleeDisabled(true);
        setRooted(true);
    }

    // the dreadlord steps out, the lord's soul lingers to aid the party
    void unmask()
    {
        BlackRookHoldInstanceScript* pHold = getBlackRookHold();
        Creature* pKurtalos = pHold != nullptr ? pHold->getTracked(BOSS_KURTALOS) : nullptr;
        LocationVector const soulSpot = pKurtalos != nullptr ? pKurtalos->GetPosition() : getCreature()->GetPosition();

        warn("Latosius reveals himself as the dreadlord Dantalionax!");
        if (Creature* pDreadlord = spawnCreature(BOSS_DANTALIONAX, getCreature()->GetPosition(), 0, getCreature()->GetPhase()))
            setZoneWideCombat(pDreadlord);

        summonCreature(NPC_SOUL_OF_RAVENCREST, soulSpot, TIMED_DESPAWN, 30000);
        notify(BOSS_KURTALOS, ACTION_KURTALOS_SOUL_SAPPED);

        attackStop();
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        _setDisplayId(HiddenDisplayId);
    }

    bool mBetrayed = false;
};

class BlackRookHoldDantalionaxAI : public BlackRookHoldBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BlackRookHoldDantalionaxAI(c); }
    explicit BlackRookHoldDantalionaxAI(Creature* pCreature) : BlackRookHoldBossAI(pCreature, DATA_KURTALOS) {}

protected:
    void onEngage() override
    {
        mHidden = false;
        mImagesCalled = 0;
        scriptEvents.addEvent(EVENT_DANTALIONAX_VOLLEY, 14500);
        scriptEvents.addEvent(EVENT_DANTALIONAX_HYPNOSIS, 26000);
        scriptEvents.addEvent(EVENT_DANTALIONAX_GUILE, 35000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_DANTALIONAX_STINGING_SWARM, 19000);
    }

    void onReset() override { reappear(); }

    void onDefeat() override
    {
        if (BlackRookHoldInstanceScript* pHold = getBlackRookHold())
            pHold->markDreadlordSlain();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DANTALIONAX_VOLLEY:
                castSpellAOE(SPELL_DANTALIONAX_SHADOW_BOLT_VOLLEY);
                scriptEvents.addEvent(EVENT_DANTALIONAX_VOLLEY, 8500);
                break;
            case EVENT_DANTALIONAX_HYPNOSIS:
                castAt(anyPlayer(), SPELL_DANTALIONAX_CLOUD_OF_HYPNOSIS);
                scriptEvents.addEvent(EVENT_DANTALIONAX_HYPNOSIS, 32000);
                break;
            case EVENT_DANTALIONAX_GUILE:
                beginGuile();
                break;
            case EVENT_DANTALIONAX_IMAGE:
                summonCreature(NPC_IMAGE_OF_LATOSIUS, LatosiusRoofSpots[mImagesCalled], TIMED_DESPAWN, 4000);
                if (++mImagesCalled >= DreadlordImageCount)
                    scriptEvents.addEvent(EVENT_DANTALIONAX_GUILE_END, 3000);
                else
                    scriptEvents.addEvent(EVENT_DANTALIONAX_IMAGE, 1600);
                break;
            case EVENT_DANTALIONAX_GUILE_END:
                reappear();
                break;
            case EVENT_DANTALIONAX_STINGING_SWARM:
                castAt(anyPlayer(), SPELL_DANTALIONAX_STINGING_SWARM);
                scriptEvents.addEvent(EVENT_DANTALIONAX_STINGING_SWARM, 17000);
                break;
            default:
                break;
        }
    }

private:
    // vanishes while his images sweep the roof one after another
    void beginGuile()
    {
        warn("Dantalionax splits into a ring of shadowy images!");
        castSpellAOE(SPELL_DANTALIONAX_DREADLORDS_GUILE);

        mHidden = true;
        attackStop();
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        _setDisplayId(HiddenDisplayId);

        mImagesCalled = 0;
        scriptEvents.addEvent(EVENT_DANTALIONAX_IMAGE, 4000);
        scriptEvents.addEvent(EVENT_DANTALIONAX_VOLLEY, 26000);
        scriptEvents.addEvent(EVENT_DANTALIONAX_HYPNOSIS, 26000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_DANTALIONAX_STINGING_SWARM, 26000);
        scriptEvents.addEvent(EVENT_DANTALIONAX_GUILE, 82000);
    }

    void reappear()
    {
        if (!mHidden)
            return;

        mHidden = false;
        getCreature()->resetDisplayId();
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
    }

    bool mHidden = false;
    uint32_t mImagesCalled = 0;
};

// casts one Dark Obliteration and fades
class BlackRookHoldImageOfLatosiusAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BlackRookHoldImageOfLatosiusAI(c); }
    explicit BlackRookHoldImageOfLatosiusAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mCastDelay == 0)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mCastDelay > diff)
        {
            mCastDelay -= diff;
            return;
        }

        mCastDelay = 0;
        castSpellAOE(SPELL_IMAGE_DARK_OBLITERATION);
    }

private:
    uint32_t mCastDelay = 500;
};

// friendly spirit that empowers the party while the encounter lasts
class BlackRookHoldSoulOfRavencrestAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BlackRookHoldSoulOfRavencrestAI(c); }
    explicit BlackRookHoldSoulOfRavencrestAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        getCreature()->setFaction(35);
        setReactState(REACT_PASSIVE);
        castSpellOnSelf(SPELL_RAVENCREST_GHOST_VISUAL, true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mLegacyClock == 0)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mLegacyClock > diff)
        {
            mLegacyClock -= diff;
            return;
        }

        mLegacyClock = 0;
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance != nullptr && pInstance->getBossState(DATA_KURTALOS) == EncounterStates::InProgress)
            castSpellOnSelf(SPELL_RAVENCREST_LEGACY, true);

        despawn(2000, 0);
    }

private:
    uint32_t mLegacyClock = 24000;
};

void SetupBlackRookHold(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACK_ROOK_HOLD, &BlackRookHoldInstanceScript::Create);

    mgr->register_creature_script(BOSS_AMALGAM_OF_SOULS, &BlackRookHoldAmalgamAI::Create);
    mgr->register_creature_script(NPC_RESTLESS_SOUL, &BlackRookHoldRestlessSoulAI::Create);

    mgr->register_creature_script(BOSS_ILLYSANNA, &BlackRookHoldIllysannaAI::Create);

    mgr->register_creature_script(BOSS_SMASHSPITE, &BlackRookHoldSmashspiteAI::Create);

    mgr->register_creature_script(BOSS_KURTALOS, &BlackRookHoldKurtalosAI::Create);
    mgr->register_creature_script(BOSS_LATOSIUS, &BlackRookHoldLatosiusAI::Create);
    mgr->register_creature_script(BOSS_DANTALIONAX, &BlackRookHoldDantalionaxAI::Create);
    mgr->register_creature_script(NPC_IMAGE_OF_LATOSIUS, &BlackRookHoldImageOfLatosiusAI::Create);
    mgr->register_creature_script(NPC_SOUL_OF_RAVENCREST, &BlackRookHoldSoulOfRavencrestAI::Create);
}
