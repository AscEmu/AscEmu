/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Shadowmoon_Burial_Grounds.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <vector>

enum ShadowmoonBurialGroundsEvents
{
    EVENT_SADANA_DEATHSPIKE = 1,
    EVENT_SADANA_DAGGERFALL,
    EVENT_SADANA_DARK_STAR,
    EVENT_SADANA_DARK_COMMUNION,

    EVENT_SBG_SPIRIT_APPROACH,
    EVENT_SBG_DAGGER_IMPACT,

    EVENT_NHALLISH_VOID_BLAST,
    EVENT_NHALLISH_PLANAR_SHIFT,
    EVENT_NHALLISH_VOID_VORTEX,
    EVENT_NHALLISH_SOUL_STEAL,
    EVENT_NHALLISH_VOID_DEVASTATION,

    EVENT_SBG_SOUL_SHRED,

    EVENT_BONEMAW_CORPSE_BREATH,
    EVENT_BONEMAW_FETID_SPIT,
    EVENT_BONEMAW_NECROTIC_PITCH,
    EVENT_BONEMAW_BODY_SLAM,

    EVENT_SBG_WORM_SPIT,
    EVENT_SBG_WORM_SLAM,

    EVENT_NERZHUL_MALEVOLENCE,
    EVENT_NERZHUL_OMEN_OF_DEATH,
    EVENT_NERZHUL_RITUAL_OF_BONES,

    EVENT_SBG_OMEN_STRIKE,
    EVENT_SBG_BONES_MARCH
};

enum ShadowmoonBurialGroundsActions
{
    ACTION_SBG_SPIRIT_COMMUNE = 1
};

enum ShadowmoonBurialGroundsCreatureData
{
    CDATA_SBG_SPIRIT_BUSY = 1
};

static LocationVector const sadanaSpiritSpots[3] =
{
    { 1795.85f,   2.427f, 282.14f, 4.67f },
    { 1772.43f, -44.29f,  281.39f, 0.74f },
    { 1812.56f, -49.17f,  280.96f, 1.99f }
};

static LocationVector const bonemawWormSpots[4] =
{
    { 1785.77f, -425.45f, 201.21f, 0.46f },
    { 1826.28f, -473.06f, 200.74f, 3.56f },
    { 1850.34f, -493.49f, 198.88f, 4.16f },
    { 1808.75f, -506.52f, 201.69f, 5.63f }
};

static LocationVector const nerzhulBoneSpots[6] =
{
    { 1679.81f, -829.47f, 73.85f, 0.1f },
    { 1679.28f, -815.14f, 73.85f, 0.1f },
    { 1681.24f, -843.43f, 73.85f, 0.1f },
    { 1679.23f, -802.71f, 73.85f, 0.1f },
    { 1693.66f, -854.61f, 73.85f, 0.1f },
    { 1689.09f, -790.68f, 73.85f, 0.1f }
};

static LocationVector const nerzhulPortalSpot = { 1727.25f, -810.65f, 73.80f, 3.56f };

class ShadowmoonBurialGroundsInstanceScript : public InstanceScript
{
public:
    explicit ShadowmoonBurialGroundsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(ShadowmoonBurialGroundsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ShadowmoonBurialGroundsInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case GO_SBG_SADANA_DOOR:
            case GO_SBG_SADANA_PROGRESS_DOOR:
            case GO_SBG_NHALLISH_DOOR:
            case GO_SBG_BONEMAW_ENTRANCE:
            case GO_SBG_BONEMAW_EXIT:
            case GO_SBG_NERZHUL_DOOR:
                mDoorGuids.push_back(pGameObject->GetNewGUID());
                applyDoorState(pGameObject);
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t bossId, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoorState(pDoor);
        }

        if (bossId == DATA_NERZHUL && state == Performed)
            spawnExitPortal();
    }

    void OnPlayerEnter(Player* /*pPlayer*/) override
    {
        if (getBossState(DATA_NERZHUL) == Performed)
            spawnExitPortal();
    }

private:
    bool isDoorOpen(uint32_t entry) const
    {
        switch (entry)
        {
            case GO_SBG_SADANA_DOOR:
                return getBossState(DATA_SADANA) != InProgress;
            case GO_SBG_SADANA_PROGRESS_DOOR:
                return getBossState(DATA_SADANA) == Performed;
            case GO_SBG_NHALLISH_DOOR:
                return getBossState(DATA_NHALLISH) != InProgress;
            case GO_SBG_BONEMAW_ENTRANCE:
                return getBossState(DATA_BONEMAW) != InProgress;
            case GO_SBG_BONEMAW_EXIT:
            case GO_SBG_NERZHUL_DOOR:
                return getBossState(DATA_BONEMAW) == Performed;
            default:
                return true;
        }
    }

    void applyDoorState(GameObject* pDoor)
    {
        pDoor->setState(isDoorOpen(pDoor->getEntry()) ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    void spawnExitPortal()
    {
        if (mPortalSpawned)
            return;

        mPortalSpawned = true;
        spawnCreature(NPC_SBG_SHADOWMOON_PORTAL, nerzhulPortalSpot.x, nerzhulPortalSpot.y, nerzhulPortalSpot.z, nerzhulPortalSpot.o);
    }

    std::vector<WoWGuid> mDoorGuids;
    bool mPortalSpawned = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sadana Bloodfury

class ShadowmoonBurialGroundsSadanaAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadowmoonBurialGroundsSadanaAI(c); }
    explicit ShadowmoonBurialGroundsSadanaAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_SADANA, EncounterStates::InProgress);

        for (LocationVector const& spot : sadanaSpiritSpots)
            summonCreature(NPC_SBG_DEFILED_SPIRIT, spot);

        scriptEvents.addEvent(EVENT_SADANA_DEATHSPIKE, 8000);
        scriptEvents.addEvent(EVENT_SADANA_DAGGERFALL, 12000);
        scriptEvents.addEvent(EVENT_SADANA_DARK_STAR, 20000);
        scriptEvents.addEvent(EVENT_SADANA_DARK_COMMUNION, 30000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SADANA, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_SADANA, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SADANA_DEATHSPIKE:
                castSpellOnSelf(SPELL_SADANA_DEATHSPIKE, true);
                scriptEvents.addEvent(EVENT_SADANA_DEATHSPIKE, 15000);
                break;
            case EVENT_SADANA_DAGGERFALL:
            {
                uint32_t const daggers = isHeroic() ? 2 : 1;
                for (uint32_t i = 0; i < daggers; ++i)
                {
                    if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                        summonCreature(NPC_SBG_DAGGERFALL, pTarget->GetPosition(), TIMED_DESPAWN, 8000);
                }
                scriptEvents.addEvent(EVENT_SADANA_DAGGERFALL, 28000);
            } break;
            case EVENT_SADANA_DARK_STAR:
                castSpellAOE(SPELL_SADANA_WHISPERS_DARK_STAR);
                scriptEvents.addEvent(EVENT_SADANA_DARK_STAR, 30000);
                break;
            case EVENT_SADANA_DARK_COMMUNION:
                if (Creature* pSpirit = pickIdleSpirit())
                {
                    sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Sadana Bloodfury calls a Defiled Spirit to feed her!");
                    castSpell(pSpirit, SPELL_SADANA_DARK_COMMUNION);
                    pSpirit->GetScript()->DoAction(ACTION_SBG_SPIRIT_COMMUNE);
                }
                scriptEvents.addEvent(EVENT_SADANA_DARK_COMMUNION, 30000);
                break;
            default:
                break;
        }
    }

private:
    Creature* pickIdleSpirit()
    {
        std::list<Creature*> spirits;
        GetCreatureListWithEntryInGrid(spirits, NPC_SBG_DEFILED_SPIRIT, 80.0f);

        std::vector<Creature*> idle;
        for (Creature* pSpirit : spirits)
        {
            if (pSpirit->isAlive() && pSpirit->GetScript() && pSpirit->GetScript()->GetCreatureData(CDATA_SBG_SPIRIT_BUSY) == 0)
                idle.push_back(pSpirit);
        }

        if (idle.empty())
            return nullptr;

        return Util::selectRandomContainerElement(idle);
    }
};

// Defiled Spirit, walks to Sadana on Dark Communion and heals her on arrival
class ShadowmoonBurialGroundsDefiledSpiritAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadowmoonBurialGroundsDefiledSpiritAI(c); }
    explicit ShadowmoonBurialGroundsDefiledSpiritAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_SBG_SPIRIT_COMMUNE)
            return;

        mCommuning = true;
        setReactState(REACT_PASSIVE);
        scriptEvents.addEvent(EVENT_SBG_SPIRIT_APPROACH, 500);
    }

    uint32_t GetCreatureData(uint32_t type) const override
    {
        return type == CDATA_SBG_SPIRIT_BUSY && mCommuning ? 1 : 0;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (scriptEvents.getFinishedEvent() != EVENT_SBG_SPIRIT_APPROACH)
            return;

        Creature* pSadana = findNearestCreature(BOSS_SADANA_BLOODFURY, 100.0f);
        if (pSadana == nullptr || !pSadana->isAlive())
        {
            mCommuning = false;
            return;
        }

        if (getRangeToObject(pSadana) <= 4.0f)
        {
            castSpell(pSadana, SPELL_SADANA_COMMUNION_HEAL, true);
            despawn(500, 0);
            return;
        }

        moveTo(pSadana->GetPositionX(), pSadana->GetPositionY(), pSadana->GetPositionZ(), false);
        scriptEvents.addEvent(EVENT_SBG_SPIRIT_APPROACH, 1000);
    }

private:
    bool mCommuning = false;
};

// Daggerfall, drops on the marked spot and leaves a damaging zone
class ShadowmoonBurialGroundsDaggerfallAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadowmoonBurialGroundsDaggerfallAI(c); }
    explicit ShadowmoonBurialGroundsDaggerfallAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_SADANA_DAGGERFALL_VISUAL, true);
        scriptEvents.addEvent(EVENT_SBG_DAGGER_IMPACT, 2500);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (scriptEvents.getFinishedEvent() == EVENT_SBG_DAGGER_IMPACT)
        {
            castSpellAOE(SPELL_SADANA_DAGGERFALL_IMPACT, true);
            castSpellOnSelf(SPELL_SADANA_DAGGERFALL_AURA, true);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nhallish

class ShadowmoonBurialGroundsNhallishAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadowmoonBurialGroundsNhallishAI(c); }
    explicit ShadowmoonBurialGroundsNhallishAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_NHALLISH, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_NHALLISH_VOID_BLAST, 12000);
        scriptEvents.addEvent(EVENT_NHALLISH_PLANAR_SHIFT, 24000);
        scriptEvents.addEvent(EVENT_NHALLISH_SOUL_STEAL, 34000);
        scriptEvents.addEvent(EVENT_NHALLISH_VOID_DEVASTATION, 66000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_NHALLISH, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_NHALLISH, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_NHALLISH_VOID_BLAST:
                castSpellOnVictim(SPELL_NHALLISH_VOID_BLAST);
                scriptEvents.addEvent(EVENT_NHALLISH_VOID_BLAST, 24000);
                break;
            case EVENT_NHALLISH_PLANAR_SHIFT:
                castSpellAOE(SPELL_NHALLISH_PLANAR_SHIFT);
                scriptEvents.addEvent(EVENT_NHALLISH_VOID_VORTEX, 1500);
                scriptEvents.addEvent(EVENT_NHALLISH_PLANAR_SHIFT, 70000);
                break;
            case EVENT_NHALLISH_VOID_VORTEX:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Nhallish summons a Void Vortex!");
                castSpellAOE(SPELL_NHALLISH_VOID_VORTEX);
                break;
            case EVENT_NHALLISH_SOUL_STEAL:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Nhallish tears the souls from his enemies!");
                castSpellAOE(SPELL_NHALLISH_SOUL_STEAL);
                summonStolenSouls();
                scriptEvents.addEvent(EVENT_NHALLISH_SOUL_STEAL, 70000);
                break;
            case EVENT_NHALLISH_VOID_DEVASTATION:
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Nhallish begins to cast Void Devastation!");
                castSpellAOE(SPELL_NHALLISH_VOID_DEVASTATION);
                scriptEvents.addEvent(EVENT_NHALLISH_VOID_DEVASTATION, 70000);
                break;
            default:
                break;
        }
    }

private:
    // one possessed soul next to every living player in the room
    void summonStolenSouls()
    {
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (pPlayer == nullptr || !pPlayer->isAlive() || getRangeToObject(pPlayer) > 60.0f)
                continue;

            summonCreature(NPC_SBG_POSSESSED_SOUL, pPlayer->GetPositionX() + 2.0f, pPlayer->GetPositionY(), pPlayer->GetPositionZ(), pPlayer->GetOrientation(), TIMED_DESPAWN, 60000);
        }
    }
};

// Possessed Soul, hunts the nearest player and shreds its soul
class ShadowmoonBurialGroundsPossessedSoulAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadowmoonBurialGroundsPossessedSoulAI(c); }
    explicit ShadowmoonBurialGroundsPossessedSoulAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        if (Player* pPlayer = getNearestPlayer())
        {
            addThreat(pPlayer, 1000.0f);
            attackStart(pPlayer);
        }
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SBG_SOUL_SHRED, 3000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        if (scriptEvents.getFinishedEvent() == EVENT_SBG_SOUL_SHRED)
        {
            castSpellOnVictim(SPELL_NHALLISH_SOUL_SHRED);
            scriptEvents.addEvent(EVENT_SBG_SOUL_SHRED, 8000);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Bonemaw

class ShadowmoonBurialGroundsBonemawAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadowmoonBurialGroundsBonemawAI(c); }
    explicit ShadowmoonBurialGroundsBonemawAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_BONEMAW, EncounterStates::InProgress);

        setRooted(true);
        mSecondWormsCalled = false;

        summonCreature(NPC_SBG_CARRION_WORM, bonemawWormSpots[0]);
        summonCreature(NPC_SBG_CARRION_WORM, bonemawWormSpots[1]);

        scriptEvents.addEvent(EVENT_BONEMAW_FETID_SPIT, 5000);
        scriptEvents.addEvent(EVENT_BONEMAW_CORPSE_BREATH, 6000);
        scriptEvents.addEvent(EVENT_BONEMAW_NECROTIC_PITCH, 8000);
        scriptEvents.addEvent(EVENT_BONEMAW_BODY_SLAM, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        setRooted(false);
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_BONEMAW, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_BONEMAW, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
        setZoneWideCombat(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mSecondWormsCalled && _getHealthPercent() <= 50)
        {
            mSecondWormsCalled = true;
            sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Carrion Worms burst from the water!");
            summonCreature(NPC_SBG_CARRION_WORM, bonemawWormSpots[2]);
            summonCreature(NPC_SBG_CARRION_WORM, bonemawWormSpots[3]);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BONEMAW_FETID_SPIT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_BONEMAW_FETID_SPIT);
                scriptEvents.addEvent(EVENT_BONEMAW_FETID_SPIT, 6000);
                break;
            case EVENT_BONEMAW_CORPSE_BREATH:
                castSpellOnVictim(SPELL_BONEMAW_CORPSE_BREATH);
                scriptEvents.addEvent(EVENT_BONEMAW_CORPSE_BREATH, 28000);
                break;
            case EVENT_BONEMAW_NECROTIC_PITCH:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_BONEMAW_NECROTIC_PITCH);
                scriptEvents.addEvent(EVENT_BONEMAW_NECROTIC_PITCH, 12000);
                break;
            case EVENT_BONEMAW_BODY_SLAM:
                castSpellOnVictim(SPELL_BONEMAW_BODY_SLAM);
                scriptEvents.addEvent(EVENT_BONEMAW_BODY_SLAM, 30000);
                break;
            default:
                break;
        }
    }

private:
    bool mSecondWormsCalled = false;
};

// Carrion Worm, stationary spitter of the Bonemaw fight
class ShadowmoonBurialGroundsCarrionWormAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadowmoonBurialGroundsCarrionWormAI(c); }
    explicit ShadowmoonBurialGroundsCarrionWormAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        setRooted(true);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.addEvent(EVENT_SBG_WORM_SPIT, 5000);
        scriptEvents.addEvent(EVENT_SBG_WORM_SLAM, 14000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_SBG_WORM_SPIT:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    castSpell(pTarget, SPELL_CARRION_WORM_FETID_SPIT);
                scriptEvents.addEvent(EVENT_SBG_WORM_SPIT, 3000);
                break;
            case EVENT_SBG_WORM_SLAM:
                castSpellOnVictim(SPELL_CARRION_WORM_BODY_SLAM);
                scriptEvents.addEvent(EVENT_SBG_WORM_SLAM, 16000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ner'zhul

class ShadowmoonBurialGroundsNerzhulAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadowmoonBurialGroundsNerzhulAI(c); }
    explicit ShadowmoonBurialGroundsNerzhulAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameEngage, getCreature());
        getInstanceScript()->setBossState(DATA_NERZHUL, EncounterStates::InProgress);

        scriptEvents.addEvent(EVENT_NERZHUL_MALEVOLENCE, 8000);
        scriptEvents.addEvent(EVENT_NERZHUL_OMEN_OF_DEATH, 12000);
        scriptEvents.addEvent(EVENT_NERZHUL_RITUAL_OF_BONES, 25000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (!isAlive())
            return;

        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_NERZHUL, EncounterStates::Failed);
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        getInstanceScript()->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
        getInstanceScript()->setBossState(DATA_NERZHUL, EncounterStates::Performed);
    }

    void onSummonedCreature(Creature* summon) override
    {
        summons.summon(summon);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_NERZHUL_MALEVOLENCE:
                castSpellOnVictim(SPELL_NERZHUL_MALEVOLENCE);
                scriptEvents.addEvent(EVENT_NERZHUL_MALEVOLENCE, 12000);
                break;
            case EVENT_NERZHUL_OMEN_OF_DEATH:
                if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter_Player)))
                    summonCreature(NPC_SBG_OMEN_OF_DEATH, pTarget->GetPosition(), TIMED_DESPAWN, 8000);
                scriptEvents.addEvent(EVENT_NERZHUL_OMEN_OF_DEATH, 14000);
                break;
            case EVENT_NERZHUL_RITUAL_OF_BONES:
            {
                sendChatMessage(CHAT_MSG_MONSTER_EMOTE, 0, "Ner'zhul begins the Ritual of Bones! Find the gap!");

                // the wall of bones keeps one random gap
                uint32_t const gap = Util::getRandomUInt(0, 5);
                for (uint32_t i = 0; i < 6; ++i)
                {
                    if (i != gap)
                        summonCreature(NPC_SBG_RITUAL_OF_BONES, nerzhulBoneSpots[i], TIMED_DESPAWN, 32000);
                }
                scriptEvents.addEvent(EVENT_NERZHUL_RITUAL_OF_BONES, 45000);
            } break;
            default:
                break;
        }
    }
};

// Omen of Death, marks a spot and strikes it with lightning
class ShadowmoonBurialGroundsOmenOfDeathAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadowmoonBurialGroundsOmenOfDeathAI(c); }
    explicit ShadowmoonBurialGroundsOmenOfDeathAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_NERZHUL_OMEN_VISUAL, true);
        scriptEvents.addEvent(EVENT_SBG_OMEN_STRIKE, 2000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (scriptEvents.getFinishedEvent() == EVENT_SBG_OMEN_STRIKE)
            castSpellAOE(SPELL_NERZHUL_OMEN_LIGHTNING, true);
    }
};

// Ritual of Bones, marches straight across the room
class ShadowmoonBurialGroundsRitualOfBonesAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadowmoonBurialGroundsRitualOfBonesAI(c); }
    explicit ShadowmoonBurialGroundsRitualOfBonesAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnSummon(Unit* /*summoner*/) override
    {
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_NERZHUL_RITUAL_PERIODIC, true);
        scriptEvents.addEvent(EVENT_SBG_BONES_MARCH, 3000);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (scriptEvents.getFinishedEvent() != EVENT_SBG_BONES_MARCH)
            return;

        castSpellOnSelf(SPELL_NERZHUL_RITUAL_OF_BONES, true);

        float const facing = getCreature()->GetOrientation();
        moveTo(getCreature()->GetPositionX() + 68.0f * std::cos(facing), getCreature()->GetPositionY() + 68.0f * std::sin(facing), getCreature()->GetPositionZ(), false);
    }
};

void SetupShadowmoonBurialGrounds(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SHADOWMOON_BURIAL_GROUNDS, &ShadowmoonBurialGroundsInstanceScript::Create);

    mgr->register_creature_script(BOSS_SADANA_BLOODFURY, &ShadowmoonBurialGroundsSadanaAI::Create);
    mgr->register_creature_script(BOSS_NHALLISH, &ShadowmoonBurialGroundsNhallishAI::Create);
    mgr->register_creature_script(BOSS_BONEMAW, &ShadowmoonBurialGroundsBonemawAI::Create);
    mgr->register_creature_script(BOSS_NERZHUL, &ShadowmoonBurialGroundsNerzhulAI::Create);

    mgr->register_creature_script(NPC_SBG_DEFILED_SPIRIT, &ShadowmoonBurialGroundsDefiledSpiritAI::Create);
    mgr->register_creature_script(NPC_SBG_DAGGERFALL, &ShadowmoonBurialGroundsDaggerfallAI::Create);
    mgr->register_creature_script(NPC_SBG_POSSESSED_SOUL, &ShadowmoonBurialGroundsPossessedSoulAI::Create);
    mgr->register_creature_script(NPC_SBG_CARRION_WORM, &ShadowmoonBurialGroundsCarrionWormAI::Create);
    mgr->register_creature_script(NPC_SBG_OMEN_OF_DEATH, &ShadowmoonBurialGroundsOmenOfDeathAI::Create);
    mgr->register_creature_script(NPC_SBG_RITUAL_OF_BONES, &ShadowmoonBurialGroundsRitualOfBonesAI::Create);
}
