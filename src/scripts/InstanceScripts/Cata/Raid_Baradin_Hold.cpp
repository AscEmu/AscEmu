/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Baradin Hold - Argaloth (4.0), Occu'thar (4.1) and Alizabal (4.3) ported from their
// live encounter design (verified against wowhead) into AscEmu's own CreatureAIScript /
// InstanceScript framework - this is a hand port, not a literal code translation, since
// we don't expose the same per-spell-effect scripting hooks used there.
//
// Known simplifications versus the original fight design:
//  - Occu'thar's "Focused Fire" / "Eyes of Occu'thar" mechanics revolve around players
//    riding Occu'thar's vehicle and manually aiming his eye beam; that vehicle-seat
//    interaction isn't available here. Both abilities instead land as straightforward
//    target debuffs/AoE, so the encounter is playable but not mechanically identical.
//  - Argaloth's and Alizabal's rotations (timers, health-percent phase triggers, Blade
//    Dance charges) are created at full fidelity.

#include "Setup.h"
#include "Raid_Baradin_Hold.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

enum BaradinHoldEvents
{
    // Argaloth
    EVENT_ARGALOTH_METEOR_SLASH = 1,
    EVENT_ARGALOTH_CONSUMING_DARKNESS,
    EVENT_ARGALOTH_FEL_FIRESTORM,
    EVENT_ARGALOTH_END_FEL_FLAME_PHASE,
    EVENT_ARGALOTH_BERSERK,

    // Occu'thar
    EVENT_OCCUTHAR_SEARING_SHADOWS,
    EVENT_OCCUTHAR_FOCUSED_FIRE,
    EVENT_OCCUTHAR_EYES_OF_OCCUTHAR,
    EVENT_OCCUTHAR_BERSERK,

    // Alizabal
    EVENT_ALIZABAL_SEETHING_HATE,
    EVENT_ALIZABAL_SKEWER,
    EVENT_ALIZABAL_BLADE_DANCE,
    EVENT_ALIZABAL_BLADE_DANCE_CHARGE,
    EVENT_ALIZABAL_BERSERK,

    // Disciple of Hate
    EVENT_DISCIPLE_OF_HATE_RUN_THROUGH,
    EVENT_DISCIPLE_OF_HATE_WHIRL_OF_BLADES
};

class BaradinHoldInstanceScript : public InstanceScript
{
public:
    explicit BaradinHoldInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(BaradinHoldEncounterCount);

        mArgalothDoorGuid = 0;
        mOccutharDoorGuid = 0;
        mAlizabalDoorGuid = 0;
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new BaradinHoldInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_ARGALOTH_DOOR:
                mArgalothDoorGuid = pGameObject->getGuidLow();
                break;
            case GO_OCCUTHAR_DOOR:
                mOccutharDoorGuid = pGameObject->getGuidLow();
                break;
            case GO_ALIZABAL_DOOR:
                mAlizabalDoorGuid = pGameObject->getGuidLow();
                break;
            default:
                break;
        }
    }

    void OpenDoorForBoss(uint32_t dataId)
    {
        uint32_t doorGuid = 0;
        switch (dataId)
        {
            case DATA_ARGALOTH: doorGuid = mArgalothDoorGuid; break;
            case DATA_OCCUTHAR: doorGuid = mOccutharDoorGuid; break;
            case DATA_ALIZABAL: doorGuid = mAlizabalDoorGuid; break;
            default: break;
        }

        if (doorGuid == 0)
            return;

        if (GameObject* pDoor = GetGameObjectByGuid(doorGuid))
            useDoorOrButton(pDoor);
    }

private:
    uint32_t mArgalothDoorGuid;
    uint32_t mOccutharDoorGuid;
    uint32_t mAlizabalDoorGuid;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Argaloth

// Reference dialogue data for entry 47120 only has the Fel Firestorm cast-announcement emote
// already used below - no aggro, kill, or death lines exist for this boss in the reference at
// all, so none are added here.
class ArgalothAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ArgalothAI(c); }
    explicit ArgalothAI(Creature* pCreature) : CreatureAIScript(pCreature), mFelFirestormCount(0)
    {
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ARGALOTH, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ARGALOTH_METEOR_SLASH, 10800);
        scriptEvents.addEvent(EVENT_ARGALOTH_CONSUMING_DARKNESS, 6000);
        scriptEvents.addEvent(EVENT_ARGALOTH_BERSERK, 300000);

        mFelFirestormCount = 0;
        mFelFlameGuids.clear();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ARGALOTH, EncounterStates::Failed);
        despawnFelFlames();
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ARGALOTH, EncounterStates::Performed);
        static_cast<BaradinHoldInstanceScript*>(getInstanceScript())->OpenDoorForBoss(DATA_ARGALOTH);
        despawnFelFlames();
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (mFelFirestormCount > 1 || damage == nullptr)
            return;

        const uint32_t curHealth = getCreature()->getHealth();
        if (curHealth <= *damage)
            return;

        const uint32_t maxHealth = getCreature()->getMaxHealth();
        if (maxHealth == 0)
            return;

        const uint32_t postHitPct = (curHealth - *damage) * 100 / maxHealth;
        const uint32_t triggerPct = mFelFirestormCount == 0 ? 66 : 33;

        if (postHitPct <= triggerPct)
        {
            scriptEvents.resetEvents();
            scriptEvents.addEvent(EVENT_ARGALOTH_FEL_FIRESTORM, 1);
            ++mFelFirestormCount;
        }
    }

    void onSummonedCreature(Creature* summon) override
    {
        mFelFlameGuids.push_back(summon->getGuidLow());
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ARGALOTH_METEOR_SLASH:
                castSpellOnSelf(SPELL_ARGALOTH_METEOR_SLASH_VISUAL);
                castSpellAOE(getRaidModeValue(SPELL_ARGALOTH_METEOR_SLASH_10, SPELL_ARGALOTH_METEOR_SLASH_25, SPELL_ARGALOTH_METEOR_SLASH_10, SPELL_ARGALOTH_METEOR_SLASH_25));
                scriptEvents.addEvent(EVENT_ARGALOTH_METEOR_SLASH, 17000);
                break;
            case EVENT_ARGALOTH_CONSUMING_DARKNESS:
                castSpellAOE(getRaidModeValue(SPELL_ARGALOTH_CONSUMING_DARKNESS_10, SPELL_ARGALOTH_CONSUMING_DARKNESS_25, SPELL_ARGALOTH_CONSUMING_DARKNESS_10, SPELL_ARGALOTH_CONSUMING_DARKNESS_25));
                scriptEvents.addEvent(EVENT_ARGALOTH_CONSUMING_DARKNESS, 23000);
                break;
            case EVENT_ARGALOTH_BERSERK:
                castSpellOnSelf(SPELL_ARGALOTH_BERSERK, true);
                break;
            case EVENT_ARGALOTH_FEL_FIRESTORM:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Argaloth begins to cast Fel Firestorm!");
                attackStop();
                setReactState(REACT_PASSIVE);
                castSpellAOE(SPELL_ARGALOTH_FEL_FIRESTORM);
                scriptEvents.addEvent(EVENT_ARGALOTH_END_FEL_FLAME_PHASE, 18000);
                break;
            case EVENT_ARGALOTH_END_FEL_FLAME_PHASE:
                setReactState(REACT_AGGRESSIVE);
                despawnFelFlames();
                scriptEvents.addEvent(EVENT_ARGALOTH_CONSUMING_DARKNESS, 6000);
                scriptEvents.addEvent(EVENT_ARGALOTH_METEOR_SLASH, 9000);
                break;
            default:
                break;
        }
    }

private:
    void despawnFelFlames()
    {
        for (const uint32_t guid : mFelFlameGuids)
        {
            if (Creature* pFelFlame = getInstanceScript()->GetCreatureByGuid(guid))
                pFelFlame->Despawn(1000, 0);
        }

        mFelFlameGuids.clear();
    }

    uint8_t mFelFirestormCount;
    std::vector<uint32_t> mFelFlameGuids;
};

class ArgalothFelFlamesAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ArgalothFelFlamesAI(c); }
    explicit ArgalothFelFlamesAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        setReactState(REACT_PASSIVE);
        mCastTimerId = _addTimer(1100);
    }

    void AIUpdate() override
    {
        if (mCastTimerId != 0 && _isTimerFinished(mCastTimerId))
        {
            castSpellOnSelf(SPELL_FEL_FLAMES, true);
            _removeTimer(mCastTimerId);
        }
    }

private:
    uint32_t mCastTimerId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Occu'thar (simplified - see file header comment)

class OccutharAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new OccutharAI(c); }
    explicit OccutharAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_OCCUTHAR, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_OCCUTHAR_SEARING_SHADOWS, 8000);
        scriptEvents.addEvent(EVENT_OCCUTHAR_FOCUSED_FIRE, 15000);
        scriptEvents.addEvent(EVENT_OCCUTHAR_EYES_OF_OCCUTHAR, 30000);
        scriptEvents.addEvent(EVENT_OCCUTHAR_BERSERK, 300000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_OCCUTHAR, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_OCCUTHAR, EncounterStates::Performed);
        static_cast<BaradinHoldInstanceScript*>(getInstanceScript())->OpenDoorForBoss(DATA_OCCUTHAR);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_OCCUTHAR_SEARING_SHADOWS:
                castSpellAOE(SPELL_OCCUTHAR_SEARING_SHADOWS);
                scriptEvents.addEvent(EVENT_OCCUTHAR_SEARING_SHADOWS, 25000);
                break;
            case EVENT_OCCUTHAR_FOCUSED_FIRE:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_OCCUTHAR_FOCUSED_FIRE_TRIGGER, true);
                scriptEvents.addEvent(EVENT_OCCUTHAR_FOCUSED_FIRE, 15000);
                break;
            case EVENT_OCCUTHAR_EYES_OF_OCCUTHAR:
                castSpellAOE(SPELL_OCCUTHAR_EYES_OF_OCCUTHAR);
                scriptEvents.addEvent(EVENT_OCCUTHAR_EYES_OF_OCCUTHAR, 60000);
                break;
            case EVENT_OCCUTHAR_BERSERK:
                castSpellOnSelf(SPELL_OCCUTHAR_BERSERK, true);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Alizabal

class AlizabalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AlizabalAI(c); }
    explicit AlizabalAI(Creature* pCreature) : CreatureAIScript(pCreature), mBladeDanceChargeCount(0)
    {
    }

    // Reference dialogue data for entry 55869 also has a longer SAY_INTRO line ("How I HATE
    // this place...", sound 25779, played once when the room's captor-statues are triggered
    // rather than on combat start) and several unlabeled "I hate..." one-liners (sounds
    // 25787-25790) with no documented trigger, so neither has a cast point to attach to here.
    void OnCombatStart(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 25777, "I hate adventurers.");
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_ALIZABAL, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_ALIZABAL_SEETHING_HATE, 9500);
        scriptEvents.addEvent(EVENT_ALIZABAL_SKEWER, 18000);
        scriptEvents.addEvent(EVENT_ALIZABAL_BLADE_DANCE, 27500);
        scriptEvents.addEvent(EVENT_ALIZABAL_BERSERK, 600000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ALIZABAL, EncounterStates::Failed);
        scriptEvents.resetEvents();
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 25783, "I hate mercy.");
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 25778, "I hate... every one of you...");
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_ALIZABAL, EncounterStates::Performed);
        static_cast<BaradinHoldInstanceScript*>(getInstanceScript())->OpenDoorForBoss(DATA_ALIZABAL);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_ALIZABAL_SEETHING_HATE:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 25786, "I hate martyrs.");
                castSpellAOE(getRaidModeValue(SPELL_ALIZABAL_SEETHING_HATE_10, SPELL_ALIZABAL_SEETHING_HATE_25, SPELL_ALIZABAL_SEETHING_HATE_10, SPELL_ALIZABAL_SEETHING_HATE_25));
                break;
            case EVENT_ALIZABAL_SKEWER:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 25785, "I hate armor.");
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Alizabal skewers her target to the ground.");
                castSpellOnVictim(SPELL_ALIZABAL_SKEWER);
                break;
            case EVENT_ALIZABAL_BLADE_DANCE:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 25791, "I hate standing still.");
                castSpellOnSelf(SPELL_ALIZABAL_BLADE_DANCE);
                mBladeDanceChargeCount = 0;
                scriptEvents.addEvent(EVENT_ALIZABAL_BLADE_DANCE_CHARGE, 100);
                scriptEvents.addEvent(EVENT_ALIZABAL_BLADE_DANCE, 60000);
                scheduleFollowUpEvents();
                break;
            case EVENT_ALIZABAL_BLADE_DANCE_CHARGE:
                if (mBladeDanceChargeCount < 3)
                {
                    if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                        castSpell(pTarget, SPELL_ALIZABAL_BLADE_DANCE_CHARGE);
                    else
                        castSpellOnVictim(SPELL_ALIZABAL_BLADE_DANCE_CHARGE);

                    scriptEvents.addEvent(EVENT_ALIZABAL_BLADE_DANCE_CHARGE, 4100);
                    ++mBladeDanceChargeCount;
                }
                break;
            case EVENT_ALIZABAL_BERSERK:
                castSpellOnSelf(SPELL_ALIZABAL_BERSERK, true);
                break;
            default:
                break;
        }
    }

private:
    void scheduleFollowUpEvents()
    {
        scriptEvents.addEvent(EVENT_ALIZABAL_SKEWER, 23000);
        scriptEvents.addEvent(EVENT_ALIZABAL_SEETHING_HATE, 31400);
        scriptEvents.addEvent(EVENT_ALIZABAL_SKEWER, 43400);
        scriptEvents.addEvent(EVENT_ALIZABAL_SEETHING_HATE, 51800);
    }

    uint8_t mBladeDanceChargeCount;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Disciple of Hate - trash guard on Alizabal's platform.

class DiscipleOfHateAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DiscipleOfHateAI(c); }
    explicit DiscipleOfHateAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_DISCIPLE_OF_HATE_RUN_THROUGH, 15000);
        scriptEvents.addEvent(EVENT_DISCIPLE_OF_HATE_WHIRL_OF_BLADES, 25000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override { scriptEvents.resetEvents(); }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);
        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DISCIPLE_OF_HATE_RUN_THROUGH:
                castSpellOnVictim(SPELL_DISCIPLE_OF_HATE_RUN_THROUGH);
                scriptEvents.addEvent(EVENT_DISCIPLE_OF_HATE_RUN_THROUGH, 15000);
                break;
            case EVENT_DISCIPLE_OF_HATE_WHIRL_OF_BLADES:
                castSpellAOE(SPELL_DISCIPLE_OF_HATE_WHIRL_OF_BLADES);
                scriptEvents.addEvent(EVENT_DISCIPLE_OF_HATE_WHIRL_OF_BLADES, 25000);
                break;
            default:
                break;
        }
    }
};

void SetupBaradinHold(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BARADIN_HOLD, &BaradinHoldInstanceScript::Create);

    mgr->register_creature_script(BOSS_ARGALOTH, &ArgalothAI::Create);
    mgr->register_creature_script(NPC_FEL_FLAMES, &ArgalothFelFlamesAI::Create);

    mgr->register_creature_script(BOSS_OCCUTHAR, &OccutharAI::Create);

    mgr->register_creature_script(BOSS_ALIZABAL, &AlizabalAI::Create);

    mgr->register_creature_script(NPC_DISCIPLE_OF_HATE, &DiscipleOfHateAI::Create);
}
