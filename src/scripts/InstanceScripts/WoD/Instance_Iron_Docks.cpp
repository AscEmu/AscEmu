/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Iron_Docks.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <map>
#include <set>
#include <vector>

enum IronDocksEvents
{
    EVENT_NOKGAR_BURNING_ARROWS = 1,
    EVENT_NOKGAR_BARBED_ARROW_BARRAGE,
    EVENT_NOKGAR_RECKLESS_PROVOCATION,

    EVENT_DREADFANG_SAVAGE_MAULING,
    EVENT_DREADFANG_SHREDDING_SWIPES,
    EVENT_DREADFANG_BLOODLETTING_HOWL,

    EVENT_MAKOGG_FLAMING_SLASH,
    EVENT_MAKOGG_LAVA_SWEEP,
    EVENT_NEESA_GUTSHOT,
    EVENT_NEESA_OGRE_TRAPS,
    EVENT_NEESA_JUMPER_CABLES,
    EVENT_NEESA_BOMBSQUAD,
    EVENT_DUGRU_BLOOD_BOLT,
    EVENT_DUGRU_TAINTED_BLOOD,
    EVENT_DUGRU_SANGUINE_SPHERE,

    EVENT_OSHIR_PRIMAL_ASSAULT,
    EVENT_OSHIR_RENDING_SLASHES,
    EVENT_OSHIR_BREAKOUT,
    EVENT_OSHIR_TIME_TO_FEED,

    EVENT_SKULLOC_GRONN_SMASH,
    EVENT_SKULLOC_CANNON_BARRAGE,
    EVENT_KORAMAR_SHATTERING_BLADE,
    EVENT_KORAMAR_BERSERKER_LEAP,
    EVENT_ZOGGOSH_RAPID_FIRE
};

// cages around Oshir's kennel
static LocationVector const OshirCages[8] =
{
    { 6905.50f, -1096.70f, 4.91667f },
    { 6905.34f, -1108.77f, 4.91667f },
    { 6904.55f, -1133.03f, 4.91667f },
    { 6962.14f, -1074.74f, 4.91667f },
    { 6950.18f, -1074.69f, 4.91667f },
    { 6934.44f, -1076.20f, 4.91667f },
    { 6983.62f, -1123.81f, 4.91667f },
    { 6984.08f, -1111.06f, 4.91667f }
};

class IronDocksInstanceScript : public InstanceScript
{
public:
    explicit IronDocksInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(IronDocksEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new IronDocksInstanceScript(pMapMgr); }

    // a credited member falling before its partners must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_NOKGAR:
            case NPC_DREADFANG:
            case BOSS_MAKOGG:
            case BOSS_MAKOGG_ALT:
            case BOSS_NEESA_NOX:
            case BOSS_AHRIOK_DUGRU:
            case BOSS_SKULLOC:
            case BOSS_KORAMAR:
            case BOSS_ZOGGOSH:
                mTrackedGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
                break;
            default:
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_OSHIR_ENCOUNTER_GATE && mOshirGateGuids.insert(pGameObject->GetNewGUID()).second)
            setDoorOpen(pGameObject, getBossState(DATA_OSHIR) != EncounterStates::InProgress);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id == DATA_OSHIR)
        {
            // gate is closed while the fight runs
            for (WoWGuid const& guid : mOshirGateGuids)
            {
                if (GameObject* pGate = getGameObjectByGuid(guid))
                    setDoorOpen(pGate, state != EncounterStates::InProgress);
            }
            return;
        }

        // a wiped group fights again in full
        if (state == EncounterStates::Failed)
        {
            for (Creature* pMember : getGroup(id))
            {
                if (!pMember->isAlive())
                    pMember->despawn(1000, 5000);
            }
        }
    }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getGroup(uint32_t encounterId)
    {
        std::vector<uint32_t> entries;
        if (encounterId == DATA_ENFORCERS)
            entries = { BOSS_MAKOGG, BOSS_MAKOGG_ALT, BOSS_NEESA_NOX, BOSS_AHRIOK_DUGRU };
        else if (encounterId == DATA_SKULLOC)
            entries = { BOSS_SKULLOC, BOSS_KORAMAR, BOSS_ZOGGOSH };

        std::vector<Creature*> group;
        for (uint32_t entry : entries)
        {
            if (Creature* pMember = getTracked(entry))
                group.push_back(pMember);
        }

        return group;
    }

    bool isGroupDown(uint32_t encounterId, Creature* pExcept)
    {
        for (Creature* pMember : getGroup(encounterId))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    void sendAction(uint32_t entry, int32_t action)
    {
        Creature* pTarget = getTracked(entry);
        if (pTarget != nullptr && pTarget->isAlive() && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }

private:
    // the encounter decides, the spawn state of a door is not reliable
    void setDoorOpen(GameObject* pDoor, bool open)
    {
        uint8_t const state = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != state)
            pDoor->setState(state);
    }

    std::map<uint32_t, WoWGuid> mTrackedGuids;
    std::set<WoWGuid> mOshirGateGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick while in combat

class IronDocksCombatAI : public CreatureAIScript
{
public:
    explicit IronDocksCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    IronDocksInstanceScript* getIronDocks()
    {
        if (getCreature()->GetMapId() != MAP_IRON_DOCKS)
            return nullptr;

        return static_cast<IronDocksInstanceScript*>(getInstanceScript());
    }

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    void castOnRandomPlayer(uint32_t spellId)
    {
        if (Unit* pTarget = randomPlayer())
            castSpell(pTarget, spellId);
    }

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

    void notify(uint32_t entry, int32_t action)
    {
        if (IronDocksInstanceScript* pIronDocks = getIronDocks())
            pIronDocks->sendAction(entry, action);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and group pull

class IronDocksBossAI : public IronDocksCombatAI
{
public:
    IronDocksBossAI(Creature* pCreature, uint32_t encounterId) : IronDocksCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        // the rest of the group joins
        if (IronDocksInstanceScript* pIronDocks = getIronDocks())
        {
            for (Creature* pMember : pIronDocks->getGroup(mEncounterId))
            {
                if (pMember != getCreature() && pMember->isAlive() && !pMember->isInCombat())
                    setZoneWideCombat(pMember);
            }
        }

        IronDocksCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Failed);
        }

        IronDocksCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            IronDocksInstanceScript* pIronDocks = getIronDocks();
            if (pIronDocks == nullptr || pIronDocks->isGroupDown(mEncounterId, getCreature()))
                pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }

        onDefeat();
    }

protected:
    virtual void onDefeat() {}

    // random living member of the own group, self included
    Creature* randomGroupMember()
    {
        std::vector<Creature*> alive;
        if (IronDocksInstanceScript* pIronDocks = getIronDocks())
        {
            for (Creature* pMember : pIronDocks->getGroup(mEncounterId))
            {
                if (pMember->isAlive())
                    alive.push_back(pMember);
            }
        }

        if (alive.empty())
            return getCreature();

        return alive[Util::getRandomUInt(static_cast<uint32_t>(alive.size() - 1))];
    }

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fleshrender Nok'gar, out of reach while Dreadfang is fresh

class IronDocksNokgarAI : public IronDocksBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IronDocksNokgarAI(c); }
    explicit IronDocksNokgarAI(Creature* pCreature) : IronDocksBossAI(pCreature, DATA_NOKGAR) {}

    void OnCombatStop(Unit* pTarget) override
    {
        mMounted = false;
        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        IronDocksBossAI::OnCombatStop(pTarget);
    }

    void DoAction(int32_t action) override
    {
        // called in by Dreadfang, he fights on foot from the start
        if (action == ACTION_NOKGAR_RIDERLESS)
        {
            mRiderless = true;
            return;
        }

        if (action != ACTION_NOKGAR_DISMOUNT || !mMounted || !_isInCombat())
            return;

        mMounted = false;
        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Fleshrender Nok'gar leaps down from Dreadfang!");
        scriptEvents.addEvent(EVENT_NOKGAR_RECKLESS_PROVOCATION, 10000);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_NOKGAR_BURNING_ARROWS, 16000);
        scriptEvents.addEvent(EVENT_NOKGAR_BARBED_ARROW_BARRAGE, 44000);

        if (mRiderless)
        {
            scriptEvents.addEvent(EVENT_NOKGAR_RECKLESS_PROVOCATION, 10000);
            return;
        }

        mMounted = true;
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        bringMount();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NOKGAR_BURNING_ARROWS:
                castOnRandomPlayer(SPELL_NOKGAR_BURNING_ARROWS);
                scriptEvents.addEvent(EVENT_NOKGAR_BURNING_ARROWS, 18000);
                break;
            case EVENT_NOKGAR_BARBED_ARROW_BARRAGE:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Fleshrender Nok'gar calls for a Barbed Arrow Barrage!");
                castOnRandomPlayer(SPELL_NOKGAR_BARBED_ARROW_BARRAGE);
                scriptEvents.addEvent(EVENT_NOKGAR_BARBED_ARROW_BARRAGE, 42000);
                break;
            case EVENT_NOKGAR_RECKLESS_PROVOCATION:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Fleshrender Nok'gar uses Reckless Provocation! Stop attacking him!");
                castSpellOnSelf(SPELL_NOKGAR_RECKLESS_PROVOCATION);
                scriptEvents.addEvent(EVENT_NOKGAR_RECKLESS_PROVOCATION, 42000);
                break;
            default:
                break;
        }
    }

private:
    // a placed Dreadfang is used, otherwise the mount is called in
    void bringMount()
    {
        Creature* pMount = findNearestCreature(NPC_DREADFANG, 40.0f);
        if (pMount == nullptr || !pMount->isAlive())
            pMount = summonCreature(NPC_DREADFANG, nearSelf(2.0f, 2.0f), CORPSE_DESPAWN);

        if (pMount != nullptr && !pMount->isInCombat())
            setZoneWideCombat(pMount);
    }

    bool mMounted = false;
    bool mRiderless = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Dreadfang

class IronDocksDreadfangAI : public IronDocksCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IronDocksDreadfangAI(c); }
    explicit IronDocksDreadfangAI(Creature* pCreature) : IronDocksCombatAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());

        if (IronDocksInstanceScript* pIronDocks = getIronDocks())
        {
            Creature* pNokgar = pIronDocks->getTracked(BOSS_NOKGAR);
            if (pNokgar != nullptr && pNokgar->isAlive() && !pNokgar->isInCombat())
                setZoneWideCombat(pNokgar);
        }

        IronDocksCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        IronDocksCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        releaseRider();
    }

protected:
    void onEngage() override
    {
        mRiderReleased = false;
        scriptEvents.addEvent(EVENT_DREADFANG_SAVAGE_MAULING, 10000);
        scriptEvents.addEvent(EVENT_DREADFANG_SHREDDING_SWIPES, 36000);
        scriptEvents.addEvent(EVENT_DREADFANG_BLOODLETTING_HOWL, 44000);
    }

    void onCombatTick() override
    {
        if (!mRiderReleased && _getHealthPercent() <= 50)
            releaseRider();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DREADFANG_SAVAGE_MAULING:
                castOnRandomPlayer(SPELL_DREADFANG_SAVAGE_MAULING);
                scriptEvents.addEvent(EVENT_DREADFANG_SAVAGE_MAULING, Util::getRandomUInt(8000, 12000));
                break;
            case EVENT_DREADFANG_SHREDDING_SWIPES:
                castSpellAOE(SPELL_DREADFANG_SHREDDING_SWIPES);
                scriptEvents.addEvent(EVENT_DREADFANG_SHREDDING_SWIPES, 36000);
                break;
            case EVENT_DREADFANG_BLOODLETTING_HOWL:
                castSpellAOE(SPELL_DREADFANG_BLOODLETTING_HOWL);
                scriptEvents.addEvent(EVENT_DREADFANG_BLOODLETTING_HOWL, 44000);
                break;
            default:
                break;
        }
    }

private:
    void releaseRider()
    {
        mRiderReleased = true;
        callMissingRider();
        notify(BOSS_NOKGAR, ACTION_NOKGAR_DISMOUNT);
    }

    // spawn data with Dreadfang alone: Nok'gar joins when the mount falls back
    void callMissingRider()
    {
        IronDocksInstanceScript* pIronDocks = getIronDocks();
        if (pIronDocks == nullptr)
            return;

        Creature* pNokgar = pIronDocks->getTracked(BOSS_NOKGAR);
        if (pNokgar != nullptr && pNokgar->isAlive())
            return;

        pNokgar = spawnCreature(BOSS_NOKGAR, nearSelf(2.0f, 2.0f));
        if (pNokgar == nullptr || pNokgar->GetScript() == nullptr)
            return;

        pNokgar->GetScript()->DoAction(ACTION_NOKGAR_RIDERLESS);
        setZoneWideCombat(pNokgar);
    }

    bool mRiderReleased = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Grimrail Enforcers

class IronDocksMakoggAI : public IronDocksBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IronDocksMakoggAI(c); }
    explicit IronDocksMakoggAI(Creature* pCreature) : IronDocksBossAI(pCreature, DATA_ENFORCERS) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_MAKOGG_FLAMING_SLASH, 4000);
        scriptEvents.addEvent(EVENT_MAKOGG_LAVA_SWEEP, 14000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MAKOGG_FLAMING_SLASH:
                castSpellOnVictim(SPELL_MAKOGG_FLAMING_SLASH);
                scriptEvents.addEvent(EVENT_MAKOGG_FLAMING_SLASH, 28000);
                break;
            case EVENT_MAKOGG_LAVA_SWEEP:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Makogg Emberblade begins a Lava Sweep!");
                castSpellAOE(SPELL_MAKOGG_LAVA_SWEEP);
                scriptEvents.addEvent(EVENT_MAKOGG_LAVA_SWEEP, 28000);
                break;
            default:
                break;
        }
    }
};

class IronDocksNeesaNoxAI : public IronDocksBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IronDocksNeesaNoxAI(c); }
    explicit IronDocksNeesaNoxAI(Creature* pCreature) : IronDocksBossAI(pCreature, DATA_ENFORCERS) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_NEESA_GUTSHOT, 2000);
        scriptEvents.addEvent(EVENT_NEESA_BOMBSQUAD, 5000);
        scriptEvents.addEvent(EVENT_NEESA_OGRE_TRAPS, 12000);
        scriptEvents.addEvent(EVENT_NEESA_JUMPER_CABLES, 28000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NEESA_GUTSHOT:
                castSpellOnVictim(SPELL_NEESA_GUTSHOT);
                scriptEvents.addEvent(EVENT_NEESA_GUTSHOT, 3000);
                break;
            case EVENT_NEESA_BOMBSQUAD:
                callAdd(NPC_BOMBSQUAD, nearSelf(3.0f, 3.0f));
                scriptEvents.addEvent(EVENT_NEESA_BOMBSQUAD, 14000);
                break;
            case EVENT_NEESA_OGRE_TRAPS:
                castSpellAOE(SPELL_NEESA_OGRE_TRAPS);
                scriptEvents.addEvent(EVENT_NEESA_OGRE_TRAPS, 30000);
                break;
            case EVENT_NEESA_JUMPER_CABLES:
                castSpellOnVictim(SPELL_NEESA_JUMPER_CABLES);
                scriptEvents.addEvent(EVENT_NEESA_JUMPER_CABLES, 28000);
                break;
            default:
                break;
        }
    }
};

class IronDocksAhriokDugruAI : public IronDocksBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IronDocksAhriokDugruAI(c); }
    explicit IronDocksAhriokDugruAI(Creature* pCreature) : IronDocksBossAI(pCreature, DATA_ENFORCERS) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_DUGRU_BLOOD_BOLT, 2000);
        scriptEvents.addEvent(EVENT_DUGRU_TAINTED_BLOOD, 20000);
        scriptEvents.addEvent(EVENT_DUGRU_SANGUINE_SPHERE, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DUGRU_BLOOD_BOLT:
                castSpellOnVictim(SPELL_DUGRU_BLOOD_BOLT);
                scriptEvents.addEvent(EVENT_DUGRU_BLOOD_BOLT, 4000);
                break;
            case EVENT_DUGRU_TAINTED_BLOOD:
                castSpellOnVictim(SPELL_DUGRU_TAINTED_BLOOD);
                scriptEvents.addEvent(EVENT_DUGRU_TAINTED_BLOOD, 28000);
                break;
            case EVENT_DUGRU_SANGUINE_SPHERE:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Ahri'ok Dugru shields an ally with a Sanguine Sphere!");
                castSpell(randomGroupMember(), SPELL_DUGRU_SANGUINE_SPHERE);
                scriptEvents.addEvent(EVENT_DUGRU_SANGUINE_SPHERE, 60000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Oshir

class IronDocksOshirAI : public IronDocksBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IronDocksOshirAI(c); }
    explicit IronDocksOshirAI(Creature* pCreature) : IronDocksBossAI(pCreature, DATA_OSHIR) {}

protected:
    void onEngage() override
    {
        mWolvesNext = Util::getRandomUInt(1) == 0;
        scriptEvents.addEvent(EVENT_OSHIR_PRIMAL_ASSAULT, 4000);
        scriptEvents.addEvent(EVENT_OSHIR_BREAKOUT, 17000);
        scriptEvents.addEvent(EVENT_OSHIR_TIME_TO_FEED, 36000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_OSHIR_PRIMAL_ASSAULT:
                castOnRandomPlayer(SPELL_OSHIR_PRIMAL_ASSAULT);
                scriptEvents.addEvent(EVENT_OSHIR_RENDING_SLASHES, 3000);
                scriptEvents.addEvent(EVENT_OSHIR_PRIMAL_ASSAULT, 46000);
                break;
            case EVENT_OSHIR_RENDING_SLASHES:
                castSpellOnSelf(SPELL_OSHIR_RENDING_SLASHES);
                break;
            case EVENT_OSHIR_BREAKOUT:
                breakOpenCage();
                scriptEvents.addEvent(EVENT_OSHIR_BREAKOUT, 45000);
                break;
            case EVENT_OSHIR_TIME_TO_FEED:
                castOnRandomPlayer(SPELL_OSHIR_TIME_TO_FEED);
                scriptEvents.addEvent(EVENT_OSHIR_TIME_TO_FEED, 60000);
                break;
            default:
                break;
        }
    }

private:
    // wolf pack and rylak take turns
    void breakOpenCage()
    {
        LocationVector const& cage = OshirCages[Util::getRandomUInt(7)];

        if (mWolvesNext)
        {
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Oshir breaks open a cage of Ravenous Wolves!");
            for (uint8_t i = 0; i < 3; ++i)
                callAdd(NPC_RAVENOUS_WOLF, cage);
        }
        else
        {
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Oshir breaks open a Rylak Skyterror cage!");
            callAdd(NPC_RYLAK_SKYTERROR, cage);
        }

        mWolvesNext = !mWolvesNext;
    }

    bool mWolvesNext = true;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Skulloc, Koramar and Zoggosh

class IronDocksSkullocAI : public IronDocksBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IronDocksSkullocAI(c); }
    explicit IronDocksSkullocAI(Creature* pCreature) : IronDocksBossAI(pCreature, DATA_SKULLOC) {}

protected:
    void onEngage() override
    {
        mTurretReleased = false;
        mBarrageLeft = 0;
        scriptEvents.addEvent(EVENT_SKULLOC_GRONN_SMASH, 30000);
    }

    void onCombatTick() override
    {
        // the turret crew gives up below a quarter
        if (!mTurretReleased && _getHealthPercent() <= 25)
        {
            mTurretReleased = true;
            notify(BOSS_ZOGGOSH, ACTION_ZOGGOSH_LEAVE_TURRET);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SKULLOC_GRONN_SMASH:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Skulloc prepares a Cannon Barrage! Take cover!");
                castSpellAOE(SPELL_SKULLOC_GRONN_SMASH);
                notify(BOSS_KORAMAR, ACTION_KORAMAR_BLADESTORM);
                mBarrageLeft = 5;
                scriptEvents.addEvent(EVENT_SKULLOC_CANNON_BARRAGE, 3000);
                scriptEvents.addEvent(EVENT_SKULLOC_GRONN_SMASH, 60000);
                break;
            case EVENT_SKULLOC_CANNON_BARRAGE:
                castSpellAOE(SPELL_SKULLOC_CANNON_BARRAGE);
                if (mBarrageLeft > 0 && --mBarrageLeft > 0)
                    scriptEvents.addEvent(EVENT_SKULLOC_CANNON_BARRAGE, 3000);
                break;
            default:
                break;
        }
    }

private:
    bool mTurretReleased = false;
    uint32_t mBarrageLeft = 0;
};

class IronDocksKoramarAI : public IronDocksBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IronDocksKoramarAI(c); }
    explicit IronDocksKoramarAI(Creature* pCreature) : IronDocksBossAI(pCreature, DATA_SKULLOC) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_KORAMAR_BLADESTORM && _isInCombat())
            castSpellOnSelf(SPELL_KORAMAR_BLADESTORM);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KORAMAR_SHATTERING_BLADE, 4000);
        scriptEvents.addEvent(EVENT_KORAMAR_BERSERKER_LEAP, 10000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KORAMAR_SHATTERING_BLADE:
                castSpellOnVictim(SPELL_KORAMAR_SHATTERING_BLADE);
                scriptEvents.addEvent(EVENT_KORAMAR_SHATTERING_BLADE, 4000);
                break;
            case EVENT_KORAMAR_BERSERKER_LEAP:
                castOnRandomPlayer(SPELL_KORAMAR_BERSERKER_LEAP);
                scriptEvents.addEvent(EVENT_KORAMAR_BERSERKER_LEAP, 12000);
                break;
            default:
                break;
        }
    }

    // without the captain the first mate leaves the turret
    void onDefeat() override { notify(BOSS_ZOGGOSH, ACTION_ZOGGOSH_LEAVE_TURRET); }
};

class IronDocksZoggoshAI : public IronDocksBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new IronDocksZoggoshAI(c); }
    explicit IronDocksZoggoshAI(Creature* pCreature) : IronDocksBossAI(pCreature, DATA_SKULLOC) {}

    void OnCombatStop(Unit* pTarget) override
    {
        leaveTurret();
        IronDocksBossAI::OnCombatStop(pTarget);
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_ZOGGOSH_LEAVE_TURRET || !mOnTurret || !_isInCombat())
            return;

        leaveTurret();
        scriptEvents.removeEvent(EVENT_ZOGGOSH_RAPID_FIRE);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Zoggosh leaves Blackhand's Might Turret!");
    }

protected:
    void onEngage() override
    {
        // turret gunner until released
        mOnTurret = true;
        setRooted(true);
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_ZOGGOSH_RAPID_FIRE, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_ZOGGOSH_RAPID_FIRE || !mOnTurret)
            return;

        castOnRandomPlayer(SPELL_ZOGGOSH_RAPID_FIRE);
        scriptEvents.addEvent(EVENT_ZOGGOSH_RAPID_FIRE, 10000);
    }

private:
    void leaveTurret()
    {
        mOnTurret = false;
        setRooted(false);
        _setMeleeDisabled(false);
    }

    bool mOnTurret = false;
};

void SetupIronDocks(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_IRON_DOCKS, &IronDocksInstanceScript::Create);

    mgr->register_creature_script(BOSS_NOKGAR, &IronDocksNokgarAI::Create);
    mgr->register_creature_script(NPC_DREADFANG, &IronDocksDreadfangAI::Create);

    mgr->register_creature_script(BOSS_MAKOGG, &IronDocksMakoggAI::Create);
    mgr->register_creature_script(BOSS_MAKOGG_ALT, &IronDocksMakoggAI::Create);
    mgr->register_creature_script(BOSS_NEESA_NOX, &IronDocksNeesaNoxAI::Create);
    mgr->register_creature_script(BOSS_AHRIOK_DUGRU, &IronDocksAhriokDugruAI::Create);

    mgr->register_creature_script(BOSS_OSHIR, &IronDocksOshirAI::Create);

    mgr->register_creature_script(BOSS_SKULLOC, &IronDocksSkullocAI::Create);
    mgr->register_creature_script(BOSS_KORAMAR, &IronDocksKoramarAI::Create);
    mgr->register_creature_script(BOSS_ZOGGOSH, &IronDocksZoggoshAI::Create);
}
