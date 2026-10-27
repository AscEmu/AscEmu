/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Cathedral_Of_Eternal_Night.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <array>
#include <vector>

enum CathedralEternalNightEvents
{
    EVENT_CEN_AGRONOX_TIMBER_SMASH = 1,
    EVENT_CEN_AGRONOX_LASHER_WAVE,
    EVENT_CEN_AGRONOX_POISONOUS_SPORES,
    EVENT_CEN_AGRONOX_CHOKING_VINES,

    EVENT_CEN_THRASHBITE_PULVERIZING_CUDGEL,
    EVENT_CEN_THRASHBITE_HEAVE_CUDGEL,
    EVENT_CEN_THRASHBITE_SCORNFUL_GAZE,
    EVENT_CEN_THRASHBITE_SCORNFUL_CHARGE,
    EVENT_CEN_THRASHBITE_SHAKE_OFF,

    EVENT_CEN_DOMATRAX_FELSOUL_CLEAVE,
    EVENT_CEN_DOMATRAX_PORTAL_SPAWN,

    EVENT_CEN_MEPHISTROTH_DEMONIC_UPHEAVAL,
    EVENT_CEN_MEPHISTROTH_DARK_SOLITUDE,
    EVENT_CEN_MEPHISTROTH_CARRION_SWARM,
    EVENT_CEN_MEPHISTROTH_SHADOW_RISE,
    EVENT_CEN_MEPHISTROTH_EMERGE
};

namespace
{
    std::array<LocationVector, 5> const AgronoxLasherSpots =
    { {
        { -500.26f, 2541.83f, 388.13f, 4.49f },
        { -497.23f, 2534.99f, 388.13f, 4.01f },
        { -536.12f, 2551.06f, 388.13f, 4.89f },
        { -545.49f, 2548.70f, 388.13f, 4.99f },
        { -548.39f, 2539.13f, 388.13f, 5.13f }
    } };

    std::array<LocationVector, 4> const AgronoxVineSpots =
    { {
        { -500.55f, 2536.22f, 388.27f, 4.31f },
        { -550.22f, 2534.58f, 388.13f, 5.28f },
        { -550.64f, 2501.10f, 388.17f, 5.98f },
        { -493.73f, 2507.01f, 388.16f, 3.95f }
    } };

    std::array<LocationVector, 4> const DomatraxPortalSpots =
    { {
        { -519.67f, 2548.22f, 533.94f, 3.94f },
        { -522.06f, 2504.01f, 533.94f, 2.24f },
        { -564.17f, 2503.62f, 533.94f, 0.77f },
        { -561.85f, 2549.77f, 533.94f, 5.45f }
    } };

    std::array<LocationVector, 8> const MephistrothShadowSpots =
    { {
        { -515.09f, 2511.28f, 533.94f, 2.64f },
        { -530.12f, 2496.94f, 533.94f, 1.95f },
        { -552.74f, 2496.88f, 533.94f, 1.24f },
        { -570.11f, 2510.35f, 533.94f, 0.50f },
        { -570.77f, 2541.98f, 533.94f, 5.77f },
        { -555.74f, 2555.67f, 533.94f, 5.11f },
        { -531.29f, 2555.90f, 533.94f, 4.36f },
        { -515.62f, 2545.86f, 533.94f, 3.79f }
    } };

    // a bookcase this close to the gaze victim takes the charge
    float const ThrashbiteBookcaseReach = 8.0f;

    // Shadow Fade lasts this long without the Aegis event
    uint32_t const MephistrothFadeDuration = 30000;

    // Domatrax portals call one demon per portal this often
    uint32_t const DomatraxPortalPulse = 15000;

    bool isBookcase(uint32_t entry)
    {
        return entry == GO_CEN_BOOKCASE_1 || entry == GO_CEN_BOOKCASE_2 || entry == GO_CEN_BOOKCASE_3 || entry == GO_CEN_BOOKCASE_4;
    }
}

class CathedralEternalNightInstanceScript : public InstanceScript
{
public:
    explicit CathedralEternalNightInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(CathedralEternalNightEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new CathedralEternalNightInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        uint32_t const entry = pGameObject->getEntry();
        if (isBookcase(entry))
        {
            mBookcaseGuids.push_back(pGameObject->GetNewGUID());
            if (getBossState(DATA_THRASHBITE) != EncounterStates::Performed)
                pGameObject->setState(GO_STATE_CLOSED);
            return;
        }

        switch (entry)
        {
            case GO_CEN_AGRONOX_GARDEN_DOOR:
            case GO_CEN_THRASHBITE_ENTRANCE:
            case GO_CEN_THRASHBITE_EXIT:
            case GO_CEN_SANCTUM_DOOR:
                mDoorGuids.push_back(pGameObject->GetNewGUID());
                alignDoor(pGameObject);
                break;
            default:
                break;
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor);
        }

        // smashed shelves stand again unless Thrashbite is down
        if (id == DATA_THRASHBITE && state != EncounterStates::Performed)
        {
            for (WoWGuid const& guid : mBookcaseGuids)
            {
                if (GameObject* pBookcase = getGameObjectByGuid(guid))
                    pBookcase->setState(GO_STATE_CLOSED);
            }
        }
    }

    GameObject* intactBookcaseNear(Unit* pTarget, float reach)
    {
        GameObject* pClosest = nullptr;
        float closestDistance = reach;

        for (WoWGuid const& guid : mBookcaseGuids)
        {
            GameObject* pBookcase = getGameObjectByGuid(guid);
            if (pBookcase == nullptr || pBookcase->getState() != GO_STATE_CLOSED)
                continue;

            float const distance = pTarget->getDistance(pBookcase);
            if (distance <= closestDistance)
            {
                closestDistance = distance;
                pClosest = pBookcase;
            }
        }

        return pClosest;
    }

private:
    bool isFighting(uint32_t id) const { return getBossState(id) == EncounterStates::InProgress; }
    bool isDefeated(uint32_t id) const { return getBossState(id) == EncounterStates::Performed; }

    bool shouldBeOpen(uint32_t entry) const
    {
        switch (entry)
        {
            case GO_CEN_AGRONOX_GARDEN_DOOR:
                return isDefeated(DATA_AGRONOX);
            case GO_CEN_THRASHBITE_ENTRANCE:
                return !isFighting(DATA_THRASHBITE);
            case GO_CEN_THRASHBITE_EXIT:
                return isDefeated(DATA_THRASHBITE);
            case GO_CEN_SANCTUM_DOOR:
                return !isFighting(DATA_DOMATRAX) && !isFighting(DATA_MEPHISTROTH);
            default:
                return true;
        }
    }

    void alignDoor(GameObject* pDoor)
    {
        pDoor->setState(shouldBeOpen(pDoor->getEntry()) ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    std::vector<WoWGuid> mDoorGuids;
    std::vector<WoWGuid> mBookcaseGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared fighter base: event clock while engaged

class CathedralEternalNightFighterAI : public CreatureAIScript
{
public:
    explicit CathedralEternalNightFighterAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        onPull();
    }

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

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        onFightTick(diff);

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onPull() {}
    virtual void onFightTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    CathedralEternalNightInstanceScript* getCathedral()
    {
        if (getCreature()->GetMapId() != MAP_CATHEDRAL_OF_ETERNAL_NIGHT)
            return nullptr;

        return static_cast<CathedralEternalNightInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return selectUnitTarget(FilterArgs(TargetFilter_Player)); }

    Unit* anyPlayerButTank()
    {
        if (Unit* pTarget = selectUnitTarget(FilterArgs(TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent))))
            return pTarget;

        return anyPlayer();
    }

    Creature* bringAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // the add only cares for one player
    void fixateOn(Creature* pAdd, Unit* pTarget, uint32_t fixateSpell)
    {
        if (pAdd == nullptr || pTarget == nullptr)
            return;

        if (fixateSpell != 0)
            pAdd->castSpell(pTarget, fixateSpell, true);

        addThreat(pTarget, 1000000.0f, pAdd);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class CathedralEternalNightBossAI : public CathedralEternalNightFighterAI
{
public:
    CathedralEternalNightBossAI(Creature* pCreature, uint32_t encounterId) : CathedralEternalNightFighterAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        CathedralEternalNightFighterAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (!isAlive())
            return;

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Failed);
        }

        onReset();
        CathedralEternalNightFighterAI::OnCombatStop(pTarget);
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
    virtual void onReset() {}

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Agronox

class CathedralEternalNightAgronoxAI : public CathedralEternalNightBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CathedralEternalNightAgronoxAI(c); }
    explicit CathedralEternalNightAgronoxAI(Creature* pCreature) : CathedralEternalNightBossAI(pCreature, DATA_AGRONOX) {}

protected:
    void onPull() override
    {
        mLasherWavesLeft = 2;
        scriptEvents.addEvent(EVENT_CEN_AGRONOX_TIMBER_SMASH, 5000);
        scriptEvents.addEvent(EVENT_CEN_AGRONOX_LASHER_WAVE, 10000);
        scriptEvents.addEvent(EVENT_CEN_AGRONOX_POISONOUS_SPORES, 10000);
        scriptEvents.addEvent(EVENT_CEN_AGRONOX_CHOKING_VINES, 24000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CEN_AGRONOX_TIMBER_SMASH:
                castSpellOnVictim(SPELL_AGRONOX_TIMBER_SMASH);
                scriptEvents.addEvent(EVENT_CEN_AGRONOX_TIMBER_SMASH, 31000);
                break;
            case EVENT_CEN_AGRONOX_LASHER_WAVE:
                raiseLashers();
                if (mLasherWavesLeft > 0 && --mLasherWavesLeft > 0)
                    scriptEvents.addEvent(EVENT_CEN_AGRONOX_LASHER_WAVE, 30000);
                break;
            case EVENT_CEN_AGRONOX_POISONOUS_SPORES:
                castSpellAOE(SPELL_AGRONOX_POISONOUS_SPORES);
                scriptEvents.addEvent(EVENT_CEN_AGRONOX_POISONOUS_SPORES, 20000);
                break;
            case EVENT_CEN_AGRONOX_CHOKING_VINES:
                growVines();
                scriptEvents.addEvent(EVENT_CEN_AGRONOX_CHOKING_VINES, 35000);
                break;
            default:
                break;
        }
    }

private:
    // one Fulminating Lasher per player, each hunts its own victim
    void raiseLashers()
    {
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Agronox calls forth Fulminating Lashers!");
        castSpellAOE(SPELL_AGRONOX_FULMINATING_LASHERS);

        size_t spot = 0;
        for (auto const& playerPair : getCreature()->getWorldMap()->getPlayers())
        {
            Player* pPlayer = playerPair.second;
            if (pPlayer == nullptr || !pPlayer->isAlive())
                continue;

            if (spot >= AgronoxLasherSpots.size())
                break;

            fixateOn(bringAdd(NPC_FULMINATING_LASHER, AgronoxLasherSpots[spot++]), pPlayer, SPELL_LASHER_FIXATE);
        }
    }

    void growVines()
    {
        Unit* pTarget = anyPlayerButTank();
        if (pTarget == nullptr)
            return;

        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Choking Vines burst from the ground!");
        LocationVector const& spot = AgronoxVineSpots[Util::getRandomUInt(static_cast<uint32_t>(AgronoxVineSpots.size() - 1))];
        if (Creature* pVines = summonCreature(NPC_CHOKING_VINES, spot, CORPSE_DESPAWN))
            pVines->castSpell(pTarget, SPELL_CHOKING_VINES_GRIP, true);
    }

    uint32_t mLasherWavesLeft = 0;
};

class CathedralEternalNightChokingVinesAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CathedralEternalNightChokingVinesAI(c); }
    explicit CathedralEternalNightChokingVinesAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Thrashbite the Scornful

class CathedralEternalNightThrashbiteAI : public CathedralEternalNightBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CathedralEternalNightThrashbiteAI(c); }
    explicit CathedralEternalNightThrashbiteAI(Creature* pCreature) : CathedralEternalNightBossAI(pCreature, DATA_THRASHBITE) {}

protected:
    void onPull() override
    {
        mGazeTarget = WoWGuid();
        scriptEvents.addEvent(EVENT_CEN_THRASHBITE_PULVERIZING_CUDGEL, 5000);
        scriptEvents.addEvent(EVENT_CEN_THRASHBITE_HEAVE_CUDGEL, 14000);
        scriptEvents.addEvent(EVENT_CEN_THRASHBITE_SCORNFUL_GAZE, 29000);
    }

    void onReset() override { shakeOff(); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CEN_THRASHBITE_PULVERIZING_CUDGEL:
                sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Thrashbite the Scornful raises his cudgel for a Pulverizing blow!");
                castSpellAOE(SPELL_THRASHBITE_PULVERIZING_CUDGEL);
                scriptEvents.addEvent(EVENT_CEN_THRASHBITE_PULVERIZING_CUDGEL, 34000);
                break;
            case EVENT_CEN_THRASHBITE_HEAVE_CUDGEL:
                castSpellAOE(SPELL_THRASHBITE_HEAVE_CUDGEL);
                scriptEvents.addEvent(EVENT_CEN_THRASHBITE_HEAVE_CUDGEL, 36000);
                break;
            case EVENT_CEN_THRASHBITE_SCORNFUL_GAZE:
                if (Unit* pTarget = anyPlayerButTank())
                {
                    sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Thrashbite the Scornful fixes his Scornful Gaze on a victim! Hide behind a bookcase!");
                    castSpell(pTarget, SPELL_THRASHBITE_SCORNFUL_GAZE);
                    mGazeTarget = pTarget->GetNewGUID();
                    scriptEvents.addEvent(EVENT_CEN_THRASHBITE_SCORNFUL_CHARGE, 6000);
                }
                scriptEvents.addEvent(EVENT_CEN_THRASHBITE_SCORNFUL_GAZE, 36000);
                break;
            case EVENT_CEN_THRASHBITE_SCORNFUL_CHARGE:
                chargeGazeTarget();
                break;
            case EVENT_CEN_THRASHBITE_SHAKE_OFF:
                shakeOff();
                break;
            default:
                break;
        }
    }

private:
    // a bookcase in the way takes the hit and leaves him dazed
    void chargeGazeTarget()
    {
        Unit* pTarget = getCreature()->getWorldMap()->getUnit(mGazeTarget);
        mGazeTarget = WoWGuid();
        if (pTarget == nullptr || !pTarget->isAlive())
            return;

        castSpell(pTarget, SPELL_THRASHBITE_SCORNFUL_CHARGE, true);

        CathedralEternalNightInstanceScript* pCathedral = getCathedral();
        GameObject* pBookcase = pCathedral != nullptr ? pCathedral->intactBookcaseNear(pTarget, ThrashbiteBookcaseReach) : nullptr;
        if (pBookcase == nullptr)
            return;

        pBookcase->setState(GO_STATE_OPEN);
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Thrashbite the Scornful smashes into a bookcase and is dazed!");
        castSpellOnSelf(SPELL_THRASHBITE_WILD_THRASH, true);
        setRooted(true);
        _setMeleeDisabled(true);
        scriptEvents.delayAllEvents(8000);
        scriptEvents.addEvent(EVENT_CEN_THRASHBITE_SHAKE_OFF, 8000);
    }

    void shakeOff()
    {
        setRooted(false);
        _setMeleeDisabled(false);
    }

    WoWGuid mGazeTarget;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Domatrax

class CathedralEternalNightDomatraxAI : public CathedralEternalNightBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CathedralEternalNightDomatraxAI(c); }
    explicit CathedralEternalNightDomatraxAI(Creature* pCreature) : CathedralEternalNightBossAI(pCreature, DATA_DOMATRAX) {}

protected:
    void onPull() override
    {
        mStage = 0;
        mChaos = 0;
        mChaosClock = 0;
        mPortals.clear();
        scriptEvents.addEvent(EVENT_CEN_DOMATRAX_FELSOUL_CLEAVE, 8000);
    }

    void onReset() override { mPortals.clear(); }

    void onFightTick(uint32_t diff) override
    {
        // chaotic energy fills by two to four a second
        mChaosClock += diff;
        while (mChaosClock >= 1000)
        {
            mChaosClock -= 1000;
            mChaos += Util::getRandomUInt(2, 4);
        }

        if (mChaos >= 100 && !_isCasting())
        {
            mChaos = 0;
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Domatrax unleashes Chaotic Energy! Seek the Aegis of Aggramar!");
            castSpellOnSelf(SPELL_DOMATRAX_CHAOTIC_ENERGY_AURA, true);
            castSpellAOE(SPELL_DOMATRAX_CHAOTIC_ENERGY);
        }

        // portals at 90 and 50, frenzy at 25
        int32_t const health = _getHealthPercent();
        if (mStage == 0 && health <= 90)
        {
            mStage = 1;
            openPortals(NPC_DOMATRAX_IMP);
        }
        else if (mStage == 1 && health <= 50)
        {
            mStage = 2;
            openPortals(NPC_DOMATRAX_FELGUARD);
        }
        else if (mStage == 2 && health <= 25)
        {
            mStage = 3;
            sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Domatrax goes into a frenzy!");
            castSpellOnSelf(SPELL_DOMATRAX_FRENZY, true);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CEN_DOMATRAX_FELSOUL_CLEAVE:
                castSpellOnVictim(SPELL_DOMATRAX_FELSOUL_CLEAVE);
                scriptEvents.addEvent(EVENT_CEN_DOMATRAX_FELSOUL_CLEAVE, 18000);
                break;
            case EVENT_CEN_DOMATRAX_PORTAL_SPAWN:
                pulsePortals();
                scriptEvents.addEvent(EVENT_CEN_DOMATRAX_PORTAL_SPAWN, DomatraxPortalPulse);
                break;
            default:
                break;
        }
    }

private:
    struct FelPortal
    {
        WoWGuid guid;
        uint32_t demonEntry;
    };

    // two neighbouring portal spots, each guardian keeps calling one demon kind
    void openPortals(uint32_t demonEntry)
    {
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Domatrax opens Fel Portals!");

        size_t spot = Util::getRandomUInt(static_cast<uint32_t>(DomatraxPortalSpots.size() - 1));
        for (uint8_t i = 0; i < 2; ++i)
        {
            if (Creature* pGuardian = summonCreature(NPC_FEL_PORTAL_GUARDIAN, DomatraxPortalSpots[spot], CORPSE_DESPAWN))
            {
                mPortals.push_back({ pGuardian->GetNewGUID(), demonEntry });
                if (InstanceScript* pInstance = getInstanceScript())
                    pInstance->sendUnitEncounter(EncounterFrameEngage, pGuardian);
            }

            spot = (spot + 1) % DomatraxPortalSpots.size();
        }

        pulsePortals();
        scriptEvents.removeEvent(EVENT_CEN_DOMATRAX_PORTAL_SPAWN);
        scriptEvents.addEvent(EVENT_CEN_DOMATRAX_PORTAL_SPAWN, DomatraxPortalPulse);
    }

    void pulsePortals()
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr)
            return;

        for (FelPortal const& portal : mPortals)
        {
            Creature* pGuardian = pInstance->getCreatureByGuid(portal.guid);
            if (pGuardian == nullptr || !pGuardian->isAlive())
                continue;

            LocationVector spot = pGuardian->GetPosition();
            spot.x += Util::getRandomFloat(-2.0f, 2.0f);
            spot.y += Util::getRandomFloat(-2.0f, 2.0f);
            bringAdd(portal.demonEntry, spot);
        }
    }

    std::vector<FelPortal> mPortals;
    uint32_t mStage = 0;
    uint32_t mChaos = 0;
    uint32_t mChaosClock = 0;
};

class CathedralEternalNightFelPortalGuardianAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CathedralEternalNightFelPortalGuardianAI(c); }
    explicit CathedralEternalNightFelPortalGuardianAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_FEL_DISSOLVE_IN, true);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mephistroth

class CathedralEternalNightMephistrothAI : public CathedralEternalNightBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CathedralEternalNightMephistrothAI(c); }
    explicit CathedralEternalNightMephistrothAI(Creature* pCreature) : CathedralEternalNightBossAI(pCreature, DATA_MEPHISTROTH) {}

    // out of reach while faded
    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (mFaded)
            *damage = 0;
    }

protected:
    void onPull() override
    {
        mFaded = false;
        mDarkness = 50;
        mDarknessClock = 0;
        mShadowSlots.fill(WoWGuid());
        armShadowStrikes();
    }

    void onReset() override { stepOutOfShadow(); }

    void onFightTick(uint32_t diff) override
    {
        if (mFaded)
            return;

        // one or two energy a second, a full bar sends him into the shadows
        mDarknessClock += diff;
        while (mDarknessClock >= 1000)
        {
            mDarknessClock -= 1000;
            mDarkness += Util::getRandomUInt(1, 2);
        }

        if (mDarkness >= 100 && !_isCasting())
            fadeAway();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CEN_MEPHISTROTH_DEMONIC_UPHEAVAL:
                castSpellAOE(SPELL_MEPHISTROTH_DEMONIC_UPHEAVAL);
                scriptEvents.addEvent(EVENT_CEN_MEPHISTROTH_DEMONIC_UPHEAVAL, 27000);
                break;
            case EVENT_CEN_MEPHISTROTH_DARK_SOLITUDE:
                castSpellAOE(SPELL_MEPHISTROTH_DARK_SOLITUDE);
                scriptEvents.addEvent(EVENT_CEN_MEPHISTROTH_DARK_SOLITUDE, 12000);
                break;
            case EVENT_CEN_MEPHISTROTH_CARRION_SWARM:
                castSpellOnVictim(SPELL_MEPHISTROTH_CARRION_SWARM);
                scriptEvents.addEvent(EVENT_CEN_MEPHISTROTH_CARRION_SWARM, 18000);
                break;
            case EVENT_CEN_MEPHISTROTH_SHADOW_RISE:
                raiseShadow();
                scriptEvents.addEvent(EVENT_CEN_MEPHISTROTH_SHADOW_RISE, 5000);
                break;
            case EVENT_CEN_MEPHISTROTH_EMERGE:
                stepOutOfShadow();
                summons.despawnEntry(NPC_SHADOW_OF_MEPHISTROTH);
                mShadowSlots.fill(WoWGuid());
                mDarkness = 0;
                mDarknessClock = 0;
                armShadowStrikes();
                if (Unit* pTarget = anyPlayer())
                    attackStart(pTarget);
                break;
            default:
                break;
        }
    }

private:
    void armShadowStrikes()
    {
        scriptEvents.addEvent(EVENT_CEN_MEPHISTROTH_DEMONIC_UPHEAVAL, 3200);
        scriptEvents.addEvent(EVENT_CEN_MEPHISTROTH_DARK_SOLITUDE, 8000);
        scriptEvents.addEvent(EVENT_CEN_MEPHISTROTH_CARRION_SWARM, 15000);
    }

    void fadeAway()
    {
        mFaded = true;
        scriptEvents.resetEvents();
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, "Mephistroth fades into the shadows!");

        castSpellOnSelf(SPELL_MEPHISTROTH_SHADOW_FADE, true);
        castSpellOnSelf(SPELL_MEPHISTROTH_CREEPING_SHADOWS, true);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        _setMeleeDisabled(true);
        setReactState(REACT_PASSIVE);
        stopMovement();

        scriptEvents.addEvent(EVENT_CEN_MEPHISTROTH_SHADOW_RISE, 2000);
        scriptEvents.addEvent(EVENT_CEN_MEPHISTROTH_EMERGE, MephistrothFadeDuration);
    }

    void stepOutOfShadow()
    {
        mFaded = false;
        _removeAura(SPELL_MEPHISTROTH_SHADOW_FADE);
        _removeAura(SPELL_MEPHISTROTH_CREEPING_SHADOWS);
        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        _setMeleeDisabled(false);
        setReactState(REACT_AGGRESSIVE);
    }

    // a shadow rises on a free spot around the room
    void raiseShadow()
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr)
            return;

        std::vector<size_t> freeSlots;
        for (size_t i = 0; i < mShadowSlots.size(); ++i)
        {
            Creature* pShadow = pInstance->getCreatureByGuid(mShadowSlots[i]);
            if (pShadow == nullptr || !pShadow->isAlive())
                freeSlots.push_back(i);
        }

        if (freeSlots.empty())
            return;

        size_t const slot = freeSlots[Util::getRandomUInt(static_cast<uint32_t>(freeSlots.size() - 1))];
        if (Creature* pShadow = bringAdd(NPC_SHADOW_OF_MEPHISTROTH, MephistrothShadowSpots[slot]))
            mShadowSlots[slot] = pShadow->GetNewGUID();
    }

    std::array<WoWGuid, 8> mShadowSlots;
    bool mFaded = false;
    uint32_t mDarkness = 0;
    uint32_t mDarknessClock = 0;
};

void SetupCathedralOfEternalNight(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_CATHEDRAL_OF_ETERNAL_NIGHT, &CathedralEternalNightInstanceScript::Create);

    mgr->register_creature_script(BOSS_AGRONOX, &CathedralEternalNightAgronoxAI::Create);
    mgr->register_creature_script(NPC_CHOKING_VINES, &CathedralEternalNightChokingVinesAI::Create);

    mgr->register_creature_script(BOSS_THRASHBITE_THE_SCORNFUL, &CathedralEternalNightThrashbiteAI::Create);

    mgr->register_creature_script(BOSS_DOMATRAX, &CathedralEternalNightDomatraxAI::Create);
    mgr->register_creature_script(NPC_FEL_PORTAL_GUARDIAN, &CathedralEternalNightFelPortalGuardianAI::Create);

    mgr->register_creature_script(BOSS_MEPHISTROTH, &CathedralEternalNightMephistrothAI::Create);
}
