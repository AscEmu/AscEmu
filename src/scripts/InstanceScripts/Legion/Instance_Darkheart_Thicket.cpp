/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Darkheart_Thicket.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

#include <list>
#include <utility>
#include <vector>

enum DarkheartThicketEvents
{
    EVENT_GLAIDALIS_GRIEVOUS_LEAP = 1,
    EVENT_GLAIDALIS_PRIMAL_RAMPAGE,
    EVENT_GLAIDALIS_NIGHTFALL,

    EVENT_OAKHEART_SHATTERED_EARTH,
    EVENT_OAKHEART_STRANGLING_ROOTS,
    EVENT_OAKHEART_NIGHTMARE_BREATH,
    EVENT_OAKHEART_CRUSHING_GRIP,
    EVENT_OAKHEART_UPROOT,

    EVENT_DRESARON_BREATH_OF_CORRUPTION,
    EVENT_DRESARON_EARTHSHAKING_ROAR,
    EVENT_DRESARON_DOWN_DRAFT,

    EVENT_XAVIUS_FESTERING_RIP,
    EVENT_XAVIUS_NIGHTMARE_BOLT,
    EVENT_XAVIUS_FEED_ON_THE_WEAK,
    EVENT_XAVIUS_INDUCED_PARANOIA
};

namespace
{
    // Glaidalis wakes when a player gets this close to his grove
    float const GlaidalisWatchRange = 48.0f;
    float const GlaidalisPreserverRange = 150.0f;
    uint32_t const GlaidalisLeapIntervalMs = 700;

    // a player this close cracks an egg while Dresaron fights
    float const DresaronEggCrackRange = 3.0f;

    uint32_t const DresaronNestEntries[] = { NPC_CORRUPTED_DRAGON_EGG, NPC_CORRUPTED_EGG };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class DarkheartThicketInstanceScript : public InstanceScript
{
public:
    explicit DarkheartThicketInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(DarkheartThicketEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new DarkheartThicketInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        uint32_t encounterId = 0;
        if (!roomDoorEncounter(pGameObject->getEntry(), encounterId))
            return;

        mRoomDoors.emplace_back(pGameObject->GetNewGUID(), encounterId);
        alignDoor(pGameObject, getBossState(encounterId) != EncounterStates::InProgress);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (auto const& roomDoor : mRoomDoors)
        {
            if (roomDoor.second != id)
                continue;

            if (GameObject* pDoor = getGameObjectByGuid(roomDoor.first))
                alignDoor(pDoor, state != EncounterStates::InProgress);
        }

        if (id != DATA_DRESARON)
            return;

        if (state == EncounterStates::Failed)
            tendNest(true);
        else if (state == EncounterStates::Performed)
            tendNest(false);
    }

private:
    // every door here seals its room during the fight
    static bool roomDoorEncounter(uint32_t entry, uint32_t& encounterId)
    {
        if (entry >= GO_GLAIDALIS_GROVE_WALL_FIRST && entry <= GO_GLAIDALIS_GROVE_WALL_LAST)
        {
            encounterId = DATA_ARCHDRUID_GLAIDALIS;
            return true;
        }

        switch (entry)
        {
            case GO_DRESARON_LAIR_DOOR_1:
            case GO_DRESARON_LAIR_DOOR_2:
                encounterId = DATA_DRESARON;
                return true;
            case GO_XAVIUS_ROOT_WALL:
                encounterId = DATA_SHADE_OF_XAVIUS;
                return true;
            default:
                return false;
        }
    }

    // changes the state only when it differs
    void alignDoor(GameObject* pDoor, bool open)
    {
        bool const isOpen = pDoor->getState() != GO_STATE_CLOSED;
        if (isOpen != open)
            pDoor->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    // a wipe refills the nest, a kill clears it
    void tendNest(bool refill)
    {
        for (uint32_t entry : DresaronNestEntries)
        {
            for (Creature* pEgg : getCreatureSetForEntry(entry))
            {
                if (refill && !pEgg->isAlive())
                    pEgg->despawn(1000, 5000);
                else if (!refill && pEgg->isAlive())
                    pEgg->despawn(1000, 0);
            }
        }

        for (Creature* pWhelpling : getCreatureSetForEntry(NPC_HATESPAWN_WHELPLING))
            pWhelpling->despawn(1000, 0);
    }

    std::vector<std::pair<WoWGuid, uint32_t>> mRoomDoors;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick in combat, idle tick outside

class DarkheartThicketCombatAI : public CreatureAIScript
{
public:
    explicit DarkheartThicketCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (!_isInCombat())
        {
            onIdleTick(diff);
            return;
        }

        scriptEvents.updateEvents(diff, 0);
        onCombatTick();

        if (_isCasting())
            return;

        if (uint32_t eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onIdleTick(uint32_t /*diff*/) {}
    virtual void onCombatTick() {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    Unit* randomPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    void castOnRandomPlayer(uint32_t spellId, bool triggered = false)
    {
        if (Unit* pTarget = randomPlayer())
            castSpell(pTarget, spellId, triggered);
    }

    // out of reach and out of combat until released
    void seal(bool sealed)
    {
        setCanEnterCombat(!sealed);
        if (sealed)
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class DarkheartThicketBossAI : public DarkheartThicketCombatAI
{
public:
    DarkheartThicketBossAI(Creature* pCreature, uint32_t encounterId) : DarkheartThicketCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        DarkheartThicketCombatAI::OnCombatStart(pTarget);
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

        DarkheartThicketCombatAI::OnCombatStop(pTarget);
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
// Archdruid Glaidalis, falls upon the preservers before he fights

class DarkheartThicketGlaidalisAI : public DarkheartThicketBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkheartThicketGlaidalisAI(c); }
    explicit DarkheartThicketGlaidalisAI(Creature* pCreature) : DarkheartThicketBossAI(pCreature, DATA_ARCHDRUID_GLAIDALIS) {}

    void OnLoad() override
    {
        mAwake = false;
        mHunting = false;
        mWatchMs = 3000;
        seal(true);
    }

protected:
    void onIdleTick(uint32_t diff) override
    {
        if (mAwake)
            return;

        if (mWatchMs > diff)
        {
            mWatchMs -= diff;
            return;
        }

        mWatchMs = 1000;

        if (mHunting)
        {
            if (leapOnPreserver())
                mWatchMs = GlaidalisLeapIntervalMs;
            else
                awaken();
            return;
        }

        // the preservers are gone already
        if (nearestPreserver() == nullptr)
        {
            awaken();
            return;
        }

        Player* pPlayer = getNearestPlayer();
        if (pPlayer == nullptr || !pPlayer->isAlive() || getCreature()->getDistance(pPlayer) > GlaidalisWatchRange)
            return;

        mHunting = true;
        mWatchMs = 3000;
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Archdruid Glaidalis turns on the druidic preservers!");
    }

    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_GLAIDALIS_GRIEVOUS_LEAP, 6000);
        scriptEvents.addEvent(EVENT_GLAIDALIS_PRIMAL_RAMPAGE, 14000);
        scriptEvents.addEvent(EVENT_GLAIDALIS_NIGHTFALL, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GLAIDALIS_GRIEVOUS_LEAP:
                castSpellAOE(SPELL_GLAIDALIS_GRIEVOUS_LEAP);
                castOnRandomPlayer(SPELL_GLAIDALIS_GRIEVOUS_TEAR, true);
                scriptEvents.addEvent(EVENT_GLAIDALIS_GRIEVOUS_LEAP, 16000);
                break;
            case EVENT_GLAIDALIS_PRIMAL_RAMPAGE:
                if (Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget())
                {
                    castSpellOnSelf(SPELL_GLAIDALIS_RAMPAGE_FORM, true);
                    castSpell(pVictim, SPELL_GLAIDALIS_RAMPAGE_SWIPE, true);
                    castSpell(pVictim, SPELL_GLAIDALIS_RAMPAGE_CHARGE);
                }
                scriptEvents.addEvent(EVENT_GLAIDALIS_PRIMAL_RAMPAGE, 16000);
                break;
            case EVENT_GLAIDALIS_NIGHTFALL:
                castSpellAOE(SPELL_GLAIDALIS_NIGHTFALL);
                scriptEvents.addEvent(EVENT_GLAIDALIS_NIGHTFALL, 16000);
                break;
            default:
                break;
        }
    }

private:
    Creature* nearestPreserver()
    {
        std::list<Creature*> preservers;
        GetCreatureListWithEntryInGrid(preservers, NPC_DRUIDIC_PRESERVER, GlaidalisPreserverRange);

        Creature* pNearest = nullptr;
        for (Creature* pPreserver : preservers)
        {
            if (!pPreserver->isAlive())
                continue;

            if (pNearest == nullptr || getCreature()->getDistance(pPreserver) < getCreature()->getDistance(pNearest))
                pNearest = pPreserver;
        }

        return pNearest;
    }

    bool leapOnPreserver()
    {
        Creature* pPreserver = nearestPreserver();
        if (pPreserver == nullptr)
            return false;

        castSpell(pPreserver, SPELL_GLAIDALIS_PRESERVER_LEAP, true);
        getCreature()->dealDamage(pPreserver, pPreserver->getHealth(), SPELL_GLAIDALIS_PRESERVER_LEAP);
        return true;
    }

    void awaken()
    {
        mAwake = true;
        mHunting = false;
        seal(false);
    }

    bool mAwake = false;
    bool mHunting = false;
    uint32_t mWatchMs = 3000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Oakheart

class DarkheartThicketOakheartAI : public DarkheartThicketBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkheartThicketOakheartAI(c); }
    explicit DarkheartThicketOakheartAI(Creature* pCreature) : DarkheartThicketBossAI(pCreature, DATA_OAKHEART) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_OAKHEART_SHATTERED_EARTH, 6000);
        scriptEvents.addEvent(EVENT_OAKHEART_STRANGLING_ROOTS, 14000);
        scriptEvents.addEvent(EVENT_OAKHEART_NIGHTMARE_BREATH, 22000);
        scriptEvents.addEvent(EVENT_OAKHEART_CRUSHING_GRIP, 30000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_OAKHEART_UPROOT, 34000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_OAKHEART_SHATTERED_EARTH:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Oakheart shatters the earth!");
                castSpellAOE(SPELL_OAKHEART_SHATTERED_EARTH);
                scriptEvents.addEvent(EVENT_OAKHEART_SHATTERED_EARTH, 40000);
                break;
            case EVENT_OAKHEART_STRANGLING_ROOTS:
                castSpellAOE(SPELL_OAKHEART_STRANGLING_ROOTS);
                scriptEvents.addEvent(EVENT_OAKHEART_STRANGLING_ROOTS, 22000);
                break;
            case EVENT_OAKHEART_NIGHTMARE_BREATH:
                castSpellOnVictim(SPELL_OAKHEART_NIGHTMARE_BREATH);
                scriptEvents.addEvent(EVENT_OAKHEART_NIGHTMARE_BREATH, 32000);
                break;
            case EVENT_OAKHEART_CRUSHING_GRIP:
                castSpellOnVictim(SPELL_OAKHEART_CRUSHING_GRIP);
                scriptEvents.addEvent(EVENT_OAKHEART_CRUSHING_GRIP, 30000);
                break;
            case EVENT_OAKHEART_UPROOT:
                castSpellAOE(SPELL_OAKHEART_UPROOT);
                scriptEvents.addEvent(EVENT_OAKHEART_UPROOT, 34000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Dresaron and its nest

class DarkheartThicketDresaronAI : public DarkheartThicketBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkheartThicketDresaronAI(c); }
    explicit DarkheartThicketDresaronAI(Creature* pCreature) : DarkheartThicketBossAI(pCreature, DATA_DRESARON) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_DRESARON_BREATH_OF_CORRUPTION, 7000);
        scriptEvents.addEvent(EVENT_DRESARON_EARTHSHAKING_ROAR, 14000);
        scriptEvents.addEvent(EVENT_DRESARON_DOWN_DRAFT, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DRESARON_BREATH_OF_CORRUPTION:
                castSpellOnVictim(SPELL_DRESARON_BREATH_OF_CORRUPTION);
                scriptEvents.addEvent(EVENT_DRESARON_BREATH_OF_CORRUPTION, 27000);
                break;
            case EVENT_DRESARON_EARTHSHAKING_ROAR:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Dresaron lets out an Earthshaking Roar!");
                castSpellAOE(SPELL_DRESARON_EARTHSHAKING_ROAR);
                scriptEvents.addEvent(EVENT_DRESARON_EARTHSHAKING_ROAR, 19000);
                break;
            case EVENT_DRESARON_DOWN_DRAFT:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Dresaron beats its wings! Down Draft!");
                castSpellAOE(SPELL_DRESARON_DOWN_DRAFT);
                scriptEvents.addEvent(EVENT_DRESARON_DOWN_DRAFT, 30000);
                break;
            default:
                break;
        }
    }
};

// eggs crack under careless feet while the dragon fights
class DarkheartThicketDragonEggAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkheartThicketDragonEggAI(c); }
    explicit DarkheartThicketDragonEggAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        mCracked = false;
        mCheckMs = 500;
        setRooted(true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mCracked || !isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mCheckMs > diff)
        {
            mCheckMs -= diff;
            return;
        }

        mCheckMs = 500;

        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr || pInstance->getBossState(DATA_DRESARON) != EncounterStates::InProgress)
            return;

        Player* pPlayer = getNearestPlayer();
        if (pPlayer == nullptr || !pPlayer->isAlive() || getCreature()->getDistance(pPlayer) > DresaronEggCrackRange)
            return;

        mCracked = true;

        // the whelpling is not bound to the egg's corpse
        Creature* pSelf = getCreature();
        if (Creature* pWhelpling = pInstance->spawnCreature(NPC_HATESPAWN_WHELPLING, pSelf->GetPositionX(), pSelf->GetPositionY(), pSelf->GetPositionZ(), pSelf->GetOrientation()))
            setZoneWideCombat(pWhelpling);

        pSelf->dealDamage(pSelf, pSelf->getHealth(), 0);
    }

private:
    bool mCracked = false;
    uint32_t mCheckMs = 500;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shade of Xavius

class DarkheartThicketXaviusAI : public DarkheartThicketBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DarkheartThicketXaviusAI(c); }
    explicit DarkheartThicketXaviusAI(Creature* pCreature) : DarkheartThicketBossAI(pCreature, DATA_SHADE_OF_XAVIUS) {}

protected:
    void onEngage() override
    {
        mNightmareUnleashed = false;
        scriptEvents.addEvent(EVENT_XAVIUS_FESTERING_RIP, 3000);
        scriptEvents.addEvent(EVENT_XAVIUS_NIGHTMARE_BOLT, 9000);
        scriptEvents.addEvent(EVENT_XAVIUS_FEED_ON_THE_WEAK, 14000);
        scriptEvents.addEvent(EVENT_XAVIUS_INDUCED_PARANOIA, 21000);
    }

    void onCombatTick() override
    {
        if (mNightmareUnleashed || _getHealthPercent() > 50)
            return;

        mNightmareUnleashed = true;
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Shade of Xavius unleashes an Apocalyptic Nightmare!");
        castSpellOnSelf(SPELL_XAVIUS_APOCALYPTIC_NIGHTMARE, true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_XAVIUS_FESTERING_RIP:
                castSpellOnVictim(SPELL_XAVIUS_FESTERING_RIP);
                scriptEvents.addEvent(EVENT_XAVIUS_FESTERING_RIP, 17000);
                break;
            case EVENT_XAVIUS_NIGHTMARE_BOLT:
                castSpellAOE(SPELL_XAVIUS_NIGHTMARE_BOLT);
                scriptEvents.addEvent(EVENT_XAVIUS_NIGHTMARE_BOLT, 17000);
                break;
            case EVENT_XAVIUS_FEED_ON_THE_WEAK:
                castOnRandomPlayer(SPELL_XAVIUS_FEED_ON_THE_WEAK);
                scriptEvents.addEvent(EVENT_XAVIUS_FEED_ON_THE_WEAK, 30000);
                break;
            case EVENT_XAVIUS_INDUCED_PARANOIA:
                castSpellAOE(SPELL_XAVIUS_INDUCED_PARANOIA);
                scriptEvents.addEvent(EVENT_XAVIUS_INDUCED_PARANOIA, 21000);
                break;
            default:
                break;
        }
    }

private:
    bool mNightmareUnleashed = false;
};

void SetupDarkheartThicket(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_DARKHEART_THICKET, &DarkheartThicketInstanceScript::Create);

    mgr->register_creature_script(BOSS_ARCHDRUID_GLAIDALIS, &DarkheartThicketGlaidalisAI::Create);
    mgr->register_creature_script(BOSS_OAKHEART, &DarkheartThicketOakheartAI::Create);

    mgr->register_creature_script(BOSS_DRESARON, &DarkheartThicketDresaronAI::Create);
    mgr->register_creature_script(NPC_CORRUPTED_DRAGON_EGG, &DarkheartThicketDragonEggAI::Create);
    mgr->register_creature_script(NPC_CORRUPTED_EGG, &DarkheartThicketDragonEggAI::Create);

    mgr->register_creature_script(BOSS_SHADE_OF_XAVIUS, &DarkheartThicketXaviusAI::Create);
}
