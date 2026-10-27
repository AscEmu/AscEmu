/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Auchindoun.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <set>

enum AuchindounEvents
{
    EVENT_KAATHAR_SANCTIFIED_STRIKE = 1,
    EVENT_KAATHAR_HALLOWED_GROUND,
    EVENT_KAATHAR_HOLY_SHIELD,
    EVENT_KAATHAR_CONSECRATED_LIGHT,
    EVENT_KAATHAR_FATE,

    EVENT_NYAMI_MIND_SPIKE,
    EVENT_NYAMI_SHADOW_WORD_PAIN,
    EVENT_NYAMI_SOUL_VESSEL,
    EVENT_NYAMI_TORN_SPIRITS,

    EVENT_DEFENDER_CRUSADER_STRIKE,
    EVENT_MAGUS_ARCANE_BOLT,
    EVENT_MAGUS_ARCANE_BOMB,
    EVENT_ARBITER_HAMMER,
    EVENT_ARBITER_RADIANT_FURY,

    EVENT_AZZAKEL_FEL_LASH,
    EVENT_AZZAKEL_CURTAIN_OF_FLAME,
    EVENT_AZZAKEL_MALEVOLENT_CRUSH,
    EVENT_AZZAKEL_FEL_POOL,
    EVENT_AZZAKEL_CLAWS_OF_ARGUS,
    EVENT_AZZAKEL_CALL_DEMON,
    EVENT_AZZAKEL_CLAWS_END,

    EVENT_TERONGOR_SHADOW_BOLT,
    EVENT_TERONGOR_CORRUPTION,
    EVENT_TERONGOR_RAIN_OF_FIRE,
    EVENT_TERONGOR_DRAIN_LIFE,
    EVENT_TERONGOR_CURSE_OF_EXHAUSTION,
    EVENT_TERONGOR_CHAOS_BOLT,
    EVENT_TERONGOR_AGONY,
    EVENT_TERONGOR_UNSTABLE_AFFLICTION,
    EVENT_TERONGOR_SEED_OF_MALEVOLENCE,
    EVENT_TERONGOR_IMMOLATE,
    EVENT_TERONGOR_INCINERATE,
    EVENT_TERONGOR_CONFLAGRATE,
    EVENT_TERONGOR_DOOM,
    EVENT_TERONGOR_DEMONIC_LEAP,
    EVENT_TERONGOR_TOUCH_OF_CHAOS
};

// demon gates around Azzakel's platform
static LocationVector const AzzakelDemonGates[4] =
{
    { 1911.90f, 2680.62f, 31.418f, 1.450705f },
    { 1911.79f, 2764.35f, 31.418f, 4.721891f },
    { 1953.55f, 2722.47f, 31.418f, 3.139304f },
    { 1869.70f, 2722.45f, 31.418f, 0.001632f }
};

class AuchindounInstanceScript : public InstanceScript
{
public:
    explicit AuchindounInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(AuchindounEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new AuchindounInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_KAATHAR_HOLY_BARRIER:
                trackExitDoor(mKaatharBarrierGuids, pGameObject, DATA_KAATHAR);
                break;
            case GO_AZZAKEL_FEL_BARRIER:
                trackExitDoor(mAzzakelBarrierGuids, pGameObject, DATA_AZZAKEL);
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (state != EncounterStates::Performed)
            return;

        if (id == DATA_KAATHAR)
            openDoors(mKaatharBarrierGuids);
        else if (id == DATA_AZZAKEL)
            openDoors(mAzzakelBarrierGuids);
    }

private:
    // a door pushed after its boss died opens right away
    void trackExitDoor(std::set<WoWGuid>& doorGuids, GameObject* pDoor, uint32_t encounterId)
    {
        if (doorGuids.insert(pDoor->GetNewGUID()).second)
            setDoorOpen(pDoor, getBossState(encounterId) == EncounterStates::Performed);
    }

    void openDoors(std::set<WoWGuid> const& doorGuids)
    {
        for (WoWGuid const& guid : doorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                setDoorOpen(pDoor, true);
        }
    }

    // the encounter decides, the spawn state of a door is not reliable
    void setDoorOpen(GameObject* pDoor, bool open)
    {
        uint8_t const state = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != state)
            pDoor->setState(state);
    }

    std::set<WoWGuid> mKaatharBarrierGuids;
    std::set<WoWGuid> mAzzakelBarrierGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick while in combat

class AuchindounCombatAI : public CreatureAIScript
{
public:
    explicit AuchindounCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
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
    virtual void onCombatTick() {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    void castOnRandomPlayer(uint32_t spellId)
    {
        if (Unit* pTarget = randomPlayer())
            castSpell(pTarget, spellId);
    }

    // summoned add joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector nearSelf(float offsetX, float offsetY)
    {
        Creature* pSelf = getCreature();
        return LocationVector(pSelf->GetPositionX() + offsetX, pSelf->GetPositionY() + offsetY, pSelf->GetPositionZ(), pSelf->GetOrientation());
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class AuchindounBossAI : public AuchindounCombatAI
{
public:
    AuchindounBossAI(Creature* pCreature, uint32_t encounterId) : AuchindounCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        AuchindounCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Failed);
        }

        AuchindounCombatAI::OnCombatStop(pTarget);
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
// Vigilant Kaathar

class AuchindounKaatharAI : public AuchindounBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AuchindounKaatharAI(c); }
    explicit AuchindounKaatharAI(Creature* pCreature) : AuchindounBossAI(pCreature, DATA_KAATHAR) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KAATHAR_SANCTIFIED_STRIKE, 8000);
        scriptEvents.addEvent(EVENT_KAATHAR_HALLOWED_GROUND, Util::getRandomUInt(12000, 17000));
        scriptEvents.addEvent(EVENT_KAATHAR_HOLY_SHIELD, 30000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_KAATHAR_FATE, 45000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KAATHAR_SANCTIFIED_STRIKE:
                castSpellOnVictim(SPELL_KAATHAR_SANCTIFIED_STRIKE);
                scriptEvents.addEvent(EVENT_KAATHAR_SANCTIFIED_STRIKE, 8000);
                break;
            case EVENT_KAATHAR_HALLOWED_GROUND:
                castOnRandomPlayer(SPELL_KAATHAR_HALLOWED_GROUND);
                scriptEvents.addEvent(EVENT_KAATHAR_HALLOWED_GROUND, 8000);
                break;
            case EVENT_KAATHAR_HOLY_SHIELD:
                if (Unit* pTarget = randomPlayer())
                {
                    sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Vigilant Kaathar hurls his Holy Shield!");
                    castSpell(pTarget, SPELL_KAATHAR_HOLY_SHIELD);
                    // the light burst follows the throw, hide behind the shield
                    scriptEvents.addEvent(EVENT_KAATHAR_CONSECRATED_LIGHT, 4000);
                }
                scriptEvents.addEvent(EVENT_KAATHAR_HOLY_SHIELD, 30000);
                break;
            case EVENT_KAATHAR_CONSECRATED_LIGHT:
                castSpellAOE(SPELL_KAATHAR_CONSECRATED_LIGHT);
                break;
            case EVENT_KAATHAR_FATE:
                castSpellAOE(SPELL_KAATHAR_FATE);
                scriptEvents.addEvent(EVENT_KAATHAR_FATE, 30000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Soulbinder Nyami

class AuchindounNyamiAI : public AuchindounBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AuchindounNyamiAI(c); }
    explicit AuchindounNyamiAI(Creature* pCreature) : AuchindounBossAI(pCreature, DATA_NYAMI) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_NYAMI_MIND_SPIKE, Util::getRandomUInt(8000, 15000));
        scriptEvents.addEvent(EVENT_NYAMI_SHADOW_WORD_PAIN, Util::getRandomUInt(12000, 18000));
        scriptEvents.addEvent(EVENT_NYAMI_SOUL_VESSEL, 20000);
        scriptEvents.addEvent(EVENT_NYAMI_TORN_SPIRITS, 35000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NYAMI_MIND_SPIKE:
                castOnRandomPlayer(SPELL_NYAMI_MIND_SPIKE);
                scriptEvents.addEvent(EVENT_NYAMI_MIND_SPIKE, Util::getRandomUInt(6000, 8000));
                break;
            case EVENT_NYAMI_SHADOW_WORD_PAIN:
                castOnRandomPlayer(SPELL_NYAMI_SHADOW_WORD_PAIN);
                scriptEvents.addEvent(EVENT_NYAMI_SHADOW_WORD_PAIN, Util::getRandomUInt(12000, 18000));
                break;
            case EVENT_NYAMI_SOUL_VESSEL:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Soulbinder Nyami begins to cast Soul Vessel!");
                castSpellAOE(SPELL_NYAMI_SOUL_VESSEL);
                scriptEvents.addEvent(EVENT_NYAMI_SOUL_VESSEL, 25000);
                break;
            case EVENT_NYAMI_TORN_SPIRITS:
                castSpellAOE(SPELL_NYAMI_TORN_SPIRITS);
                // one of each fallen Auchenai rises
                callAdd(NPC_MALEFIC_DEFENDER, nearSelf(4.0f, 0.0f));
                callAdd(NPC_TWISTED_MAGUS, nearSelf(-4.0f, 3.0f));
                callAdd(NPC_SPITEFUL_ARBITER, nearSelf(-4.0f, -3.0f));
                scriptEvents.addEvent(EVENT_NYAMI_TORN_SPIRITS, 35000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Torn Spirits adds

class AuchindounMaleficDefenderAI : public AuchindounCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AuchindounMaleficDefenderAI(c); }
    explicit AuchindounMaleficDefenderAI(Creature* pCreature) : AuchindounCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_DEFENDER_CRUSADER_STRIKE, 5000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_DEFENDER_CRUSADER_STRIKE)
            return;

        castSpellOnVictim(SPELL_DEFENDER_CRUSADER_STRIKE);
        scriptEvents.addEvent(EVENT_DEFENDER_CRUSADER_STRIKE, Util::getRandomUInt(7000, 12000));
    }
};

class AuchindounTwistedMagusAI : public AuchindounCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AuchindounTwistedMagusAI(c); }
    explicit AuchindounTwistedMagusAI(Creature* pCreature) : AuchindounCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_MAGUS_ARCANE_BOLT, 4000);
        scriptEvents.addEvent(EVENT_MAGUS_ARCANE_BOMB, 12000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MAGUS_ARCANE_BOLT:
                castOnRandomPlayer(SPELL_MAGUS_ARCANE_BOLT);
                scriptEvents.addEvent(EVENT_MAGUS_ARCANE_BOLT, 15000);
                break;
            case EVENT_MAGUS_ARCANE_BOMB:
                castSpellAOE(SPELL_MAGUS_ARCANE_BOMB);
                scriptEvents.addEvent(EVENT_MAGUS_ARCANE_BOMB, 25000);
                break;
            default:
                break;
        }
    }
};

class AuchindounSpitefulArbiterAI : public AuchindounCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AuchindounSpitefulArbiterAI(c); }
    explicit AuchindounSpitefulArbiterAI(Creature* pCreature) : AuchindounCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_ARBITER_RADIANT_FURY, 8000);
        scriptEvents.addEvent(EVENT_ARBITER_HAMMER, 14000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ARBITER_RADIANT_FURY:
                castSpellAOE(SPELL_ARBITER_RADIANT_FURY);
                scriptEvents.addEvent(EVENT_ARBITER_RADIANT_FURY, Util::getRandomUInt(15000, 20000));
                break;
            case EVENT_ARBITER_HAMMER:
                castOnRandomPlayer(SPELL_ARBITER_HAMMER);
                scriptEvents.addEvent(EVENT_ARBITER_HAMMER, 14000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Azzakel

class AuchindounAzzakelAI : public AuchindounBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AuchindounAzzakelAI(c); }
    explicit AuchindounAzzakelAI(Creature* pCreature) : AuchindounBossAI(pCreature, DATA_AZZAKEL) {}

    void OnCombatStop(Unit* pTarget) override
    {
        leaveClaws();
        AuchindounBossAI::OnCombatStop(pTarget);
    }

protected:
    void onEngage() override
    {
        mDemonCalls = 0;
        scheduleGroundSet();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AZZAKEL_FEL_LASH:
                castSpellOnVictim(SPELL_AZZAKEL_FEL_LASH);
                scriptEvents.addEvent(EVENT_AZZAKEL_FEL_LASH, Util::getRandomUInt(8000, 12000));
                break;
            case EVENT_AZZAKEL_CURTAIN_OF_FLAME:
                castOnRandomPlayer(SPELL_AZZAKEL_CURTAIN_OF_FLAME);
                scriptEvents.addEvent(EVENT_AZZAKEL_CURTAIN_OF_FLAME, Util::getRandomUInt(8000, 15000));
                break;
            case EVENT_AZZAKEL_MALEVOLENT_CRUSH:
                castOnRandomPlayer(SPELL_AZZAKEL_MALEVOLENT_CRUSH);
                scriptEvents.addEvent(EVENT_AZZAKEL_FEL_POOL, 2000);
                scriptEvents.addEvent(EVENT_AZZAKEL_MALEVOLENT_CRUSH, 20000);
                break;
            case EVENT_AZZAKEL_FEL_POOL:
                castSpellOnSelf(SPELL_AZZAKEL_FEL_POOL);
                break;
            case EVENT_AZZAKEL_CLAWS_OF_ARGUS:
                // ground abilities pause while the demon gates are open
                scriptEvents.resetEvents();
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Azzakel casts Claws of Argus!");
                castSpellOnSelf(SPELL_AZZAKEL_CLAWS_OF_ARGUS);
                setRooted(true);
                _setMeleeDisabled(true);
                mDemonCalls = 6;
                scriptEvents.addEvent(EVENT_AZZAKEL_CALL_DEMON, 3000);
                scriptEvents.addEvent(EVENT_AZZAKEL_CLAWS_END, 20000);
                break;
            case EVENT_AZZAKEL_CALL_DEMON:
                callDemon();
                if (mDemonCalls > 0)
                    scriptEvents.addEvent(EVENT_AZZAKEL_CALL_DEMON, 3000);
                break;
            case EVENT_AZZAKEL_CLAWS_END:
                leaveClaws();
                scheduleGroundSet();
                break;
            default:
                break;
        }
    }

private:
    void scheduleGroundSet()
    {
        scriptEvents.addEvent(EVENT_AZZAKEL_FEL_LASH, 8000);
        scriptEvents.addEvent(EVENT_AZZAKEL_CURTAIN_OF_FLAME, 14000);
        scriptEvents.addEvent(EVENT_AZZAKEL_MALEVOLENT_CRUSH, 20000);
        scriptEvents.addEvent(EVENT_AZZAKEL_CLAWS_OF_ARGUS, 45000);
    }

    void callDemon()
    {
        if (mDemonCalls == 0)
            return;

        --mDemonCalls;

        static uint32_t const demonEntries[3] = { NPC_BLAZING_TRICKSTER, NPC_FELGUARD, NPC_CACKLING_PYROMANIAC };
        callAdd(demonEntries[Util::getRandomUInt(2)], AzzakelDemonGates[Util::getRandomUInt(3)]);
    }

    void leaveClaws()
    {
        mDemonCalls = 0;
        setRooted(false);
        _setMeleeDisabled(false);
    }

    uint32_t mDemonCalls = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Teron'gor

class AuchindounTerongorAI : public AuchindounBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AuchindounTerongorAI(c); }
    explicit AuchindounTerongorAI(Creature* pCreature) : AuchindounBossAI(pCreature, DATA_TERONGOR) {}

protected:
    void onEngage() override
    {
        mTransformed = false;

        scriptEvents.addEvent(EVENT_TERONGOR_SHADOW_BOLT, Util::getRandomUInt(8000, 16000));
        scriptEvents.addEvent(EVENT_TERONGOR_CORRUPTION, Util::getRandomUInt(10000, 12000));
        scriptEvents.addEvent(EVENT_TERONGOR_RAIN_OF_FIRE, 21000);
        scriptEvents.addEvent(EVENT_TERONGOR_DRAIN_LIFE, 16000);

        if (isHeroic())
            callAdd(NPC_FELBORNE_ABYSSAL, nearSelf(6.0f, 6.0f));
    }

    void onCombatTick() override
    {
        if (mTransformed || _getHealthPercent() > 75)
            return;

        mTransformed = true;
        scriptEvents.resetEvents();

        // one of the three warlock schools for the rest of the fight
        switch (Util::getRandomUInt(2))
        {
            case 0:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Wither... away!");
                castSpellOnSelf(SPELL_TERONGOR_AFFLICTION_FORM);
                scriptEvents.addEvent(EVENT_TERONGOR_SHADOW_BOLT, Util::getRandomUInt(8000, 10000));
                scriptEvents.addEvent(EVENT_TERONGOR_CURSE_OF_EXHAUSTION, 13000);
                scriptEvents.addEvent(EVENT_TERONGOR_DRAIN_LIFE, Util::getRandomUInt(13000, 16000));
                scriptEvents.addEvent(EVENT_TERONGOR_AGONY, 16000);
                scriptEvents.addEvent(EVENT_TERONGOR_UNSTABLE_AFFLICTION, 20000);
                scriptEvents.addEvent(EVENT_TERONGOR_SEED_OF_MALEVOLENCE, Util::getRandomUInt(22000, 25000));
                break;
            case 1:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Destruction!");
                castSpellOnSelf(SPELL_TERONGOR_DESTRUCTION_FORM);
                scriptEvents.addEvent(EVENT_TERONGOR_CONFLAGRATE, Util::getRandomUInt(8000, 10000));
                scriptEvents.addEvent(EVENT_TERONGOR_IMMOLATE, Util::getRandomUInt(10000, 14000));
                scriptEvents.addEvent(EVENT_TERONGOR_INCINERATE, 16000);
                scriptEvents.addEvent(EVENT_TERONGOR_CHAOS_BOLT, 20000);
                scriptEvents.addEvent(EVENT_TERONGOR_RAIN_OF_FIRE, 24000);
                break;
            default:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "I become something greater!");
                castSpellOnSelf(SPELL_TERONGOR_DEMONOLOGY_FORM);
                scriptEvents.addEvent(EVENT_TERONGOR_DOOM, Util::getRandomUInt(8000, 12000));
                scriptEvents.addEvent(EVENT_TERONGOR_DEMONIC_LEAP, Util::getRandomUInt(10000, 14000));
                scriptEvents.addEvent(EVENT_TERONGOR_CORRUPTION, Util::getRandomUInt(10000, 14000));
                scriptEvents.addEvent(EVENT_TERONGOR_TOUCH_OF_CHAOS, 16000);
                scriptEvents.addEvent(EVENT_TERONGOR_CURSE_OF_EXHAUSTION, 18000);
                scriptEvents.addEvent(EVENT_TERONGOR_CHAOS_BOLT, 25000);
                break;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TERONGOR_SHADOW_BOLT:
                castSpellOnVictim(SPELL_TERONGOR_SHADOW_BOLT);
                scriptEvents.addEvent(eventId, 6000);
                break;
            case EVENT_TERONGOR_CORRUPTION:
                castOnRandomPlayer(SPELL_TERONGOR_CORRUPTION);
                scriptEvents.addEvent(eventId, 9000);
                break;
            case EVENT_TERONGOR_RAIN_OF_FIRE:
                castOnRandomPlayer(SPELL_TERONGOR_RAIN_OF_FIRE);
                scriptEvents.addEvent(eventId, 10000);
                break;
            case EVENT_TERONGOR_DRAIN_LIFE:
                castOnRandomPlayer(SPELL_TERONGOR_DRAIN_LIFE);
                scriptEvents.addEvent(eventId, 16000);
                break;
            case EVENT_TERONGOR_CURSE_OF_EXHAUSTION:
                castOnRandomPlayer(SPELL_TERONGOR_CURSE_OF_EXHAUSTION);
                scriptEvents.addEvent(eventId, 13000);
                break;
            case EVENT_TERONGOR_CHAOS_BOLT:
                castSpellOnVictim(SPELL_TERONGOR_CHAOS_BOLT);
                scriptEvents.addEvent(eventId, 20000);
                break;
            case EVENT_TERONGOR_AGONY:
                castOnRandomPlayer(SPELL_TERONGOR_AGONY);
                scriptEvents.addEvent(eventId, 16000);
                break;
            case EVENT_TERONGOR_UNSTABLE_AFFLICTION:
                castOnRandomPlayer(SPELL_TERONGOR_UNSTABLE_AFFLICTION);
                scriptEvents.addEvent(eventId, 20000);
                break;
            case EVENT_TERONGOR_SEED_OF_MALEVOLENCE:
                castOnRandomPlayer(SPELL_TERONGOR_SEED_OF_MALEVOLENCE);
                scriptEvents.addEvent(eventId, Util::getRandomUInt(22000, 25000));
                break;
            case EVENT_TERONGOR_IMMOLATE:
                castOnRandomPlayer(SPELL_TERONGOR_IMMOLATE);
                scriptEvents.addEvent(eventId, 12000);
                break;
            case EVENT_TERONGOR_INCINERATE:
                castSpellOnVictim(SPELL_TERONGOR_INCINERATE);
                scriptEvents.addEvent(eventId, 6000);
                break;
            case EVENT_TERONGOR_CONFLAGRATE:
                castSpellOnVictim(SPELL_TERONGOR_CONFLAGRATE);
                scriptEvents.addEvent(eventId, Util::getRandomUInt(15000, 20000));
                break;
            case EVENT_TERONGOR_DOOM:
                castOnRandomPlayer(SPELL_TERONGOR_DOOM);
                scriptEvents.addEvent(eventId, Util::getRandomUInt(8000, 12000));
                break;
            case EVENT_TERONGOR_DEMONIC_LEAP:
                castOnRandomPlayer(SPELL_TERONGOR_DEMONIC_LEAP);
                scriptEvents.addEvent(eventId, Util::getRandomUInt(18000, 30000));
                break;
            case EVENT_TERONGOR_TOUCH_OF_CHAOS:
                castSpellOnVictim(SPELL_TERONGOR_TOUCH_OF_CHAOS);
                scriptEvents.addEvent(eventId, 16000);
                break;
            default:
                break;
        }
    }

private:
    bool mTransformed = false;
};

void SetupAuchindoun(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_AUCHINDOUN, &AuchindounInstanceScript::Create);

    mgr->register_creature_script(BOSS_KAATHAR, &AuchindounKaatharAI::Create);
    mgr->register_creature_script(BOSS_KAATHAR_SPAWNED, &AuchindounKaatharAI::Create);
    mgr->register_creature_script(BOSS_NYAMI, &AuchindounNyamiAI::Create);
    mgr->register_creature_script(BOSS_AZZAKEL, &AuchindounAzzakelAI::Create);
    mgr->register_creature_script(BOSS_TERONGOR, &AuchindounTerongorAI::Create);

    mgr->register_creature_script(NPC_MALEFIC_DEFENDER, &AuchindounMaleficDefenderAI::Create);
    mgr->register_creature_script(NPC_TWISTED_MAGUS, &AuchindounTwistedMagusAI::Create);
    mgr->register_creature_script(NPC_SPITEFUL_ARBITER, &AuchindounSpitefulArbiterAI::Create);
}
