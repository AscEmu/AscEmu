/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_The_Emerald_Nightmare.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

enum EmeraldNightmareEvents
{
    // Ursoc
    EVENT_URSOC_OVERWHELM = 1,
    EVENT_URSOC_REND_FLESH,
    EVENT_URSOC_ROARING_CACOPHONY,
    EVENT_URSOC_CHARGE,
    EVENT_URSOC_IMPACT,

    // Nythendra
    EVENT_NYTHENDRA_ROT,
    EVENT_NYTHENDRA_VOLATILE_ROT,
    EVENT_NYTHENDRA_INFESTED_BREATH,

    // Dragons of Nightmare
    EVENT_DRAGON_CORRUPTED_BREATH,
    EVENT_YSONDRE_NIGHTMARE_BLAST,
    EVENT_TAERAR_SEEPING_FOG,
    EVENT_EMERISS_ESSENCE,
    EVENT_SHADE_CORRUPTED_BREATH,
    EVENT_ESSENCE_CORRUPTION,

    // Xavius
    EVENT_XAVIUS_DARKENING_SOUL,
    EVENT_XAVIUS_BLACKENING_SOUL,
    EVENT_XAVIUS_NIGHTMARE_BLADES,
    EVENT_XAVIUS_BLADES_STRIKE,
    EVENT_XAVIUS_CORRUPTION_METEOR,
    EVENT_XAVIUS_NIGHTMARE_INFUSION,
    EVENT_CORRUPTION_HORROR_SWIPE,
    EVENT_CORRUPTION_HORROR_NOVA,

    // Il'gynoth
    EVENT_ILGYNOTH_TENTACLES,
    EVENT_ILGYNOTH_RECONSTITUTION,
    EVENT_ILGYNOTH_CURSED_BLOOD,
    EVENT_ILGYNOTH_EYE_RETURNS,
    EVENT_EYE_NIGHTMARE_GAZE,
    EVENT_EYE_NIGHTMARE_HORROR,
    EVENT_DOMINATOR_FURY,
    EVENT_DOMINATOR_GROUND_SLAM,
    EVENT_DOMINATOR_REACH_CHECK,
    EVENT_DEATHGLARE_MIND_FLAY,
    EVENT_CORRUPTOR_SPEW,

    // Elerethe Renferal
    EVENT_ELERETHE_NECROTIC_VENOM,
    EVENT_ELERETHE_FEEDING_TIME,
    EVENT_ELERETHE_VILE_AMBUSH,
    EVENT_ELERETHE_TWISTING_SHADOWS,
    EVENT_ELERETHE_GATHERING_CLOUDS,
    EVENT_ELERETHE_DARK_STORM,
    EVENT_ELERETHE_RAKING_TALONS,
    EVENT_ELERETHE_RAZOR_WING,

    // Cenarius
    EVENT_CENARIUS_NIGHTMARE_BRAMBLES,
    EVENT_CENARIUS_FORCES_ARRIVE,
    EVENT_CENARIUS_BLAST_SAVER,
    EVENT_CENARIUS_SPEAR,
    EVENT_SISTER_JAVELIN
};

namespace
{
    float const NightmareTwoPi = 6.28318531f;
    uint32_t const NightmareRaidCap = 40;

    // Ursoc
    uint32_t const UrsocBerserkMs = 300000;
    float const UrsocEnergyPerSecond = 3.0f;

    // Nythendra
    float const NythendraDrainPerSecond = 1.0f;
    uint32_t const NythendraSwarmMs = 20000;
    uint32_t const NythendraVerminPerSwarm = 10;

    // Dragons of Nightmare: ground spot, perch and landing spot
    LocationVector const DragonGroundSpot(617.51f, -12807.0f, 4.84f, 3.39f);
    LocationVector const DragonPerch(674.78f, -12875.5f, 42.21f, 2.91f);
    LocationVector const DragonLanding(623.48f, -12845.71f, 4.13f, 2.47f);
    uint32_t const DragonFlightMs = 5000;
    float const YsondreEnergyPerSecond = 2.0f;
    float const DragonCompanionEnergyPerSecond = 1.33f;

    // Xavius: Call of Nightmares ring
    LocationVector const XaviusRoomCentre(-2990.16f, -5029.85f, 147.78f);
    float const XaviusRoomRadius = 55.0f;
    float const XaviusEnergyPerSecond = 5.0f;
    float const XaviusCallingEnergyPerSecond = 2.5f;
    uint32_t const XaviusTerrorsPerEruption = 2;

    // Il'gynoth: court around the Heart
    LocationVector const IlgynothEyeSpot(-12818.1f, 12328.3f, -245.72f, 3.79f);
    LocationVector const IlgynothTentacleSpots[2] =
    {
        { -12843.59f, 12331.40f, -246.01f, 3.56f },
        { -12821.79f, 12302.79f, -246.01f, 3.83f }
    };
    LocationVector const IlgynothHorrorSpot(-12856.12f, 12296.79f, -253.16f, 0.72f);
    float const IchorBlastRange = 10.0f;
    float const NightmareHorrorEnergyPerSecond = 10.0f;

    struct IlgynothTentacleWave
    {
        uint32_t delayMs;
        uint32_t entry;
        uint32_t count;
    };

    // tentacles while the Eye lives, the last two waves repeat
    IlgynothTentacleWave const IlgynothTentacleWaves[] =
    {
        { 20000, NPC_DEATHGLARE_TENTACLE, 1 },
        { 70000, NPC_CORRUPTOR_TENTACLE, 2 },
        { 6000, NPC_DEATHGLARE_TENTACLE, 2 },
        { 84000, NPC_DEATHGLARE_TENTACLE, 1 },
        { 4000, NPC_CORRUPTOR_TENTACLE, 2 },
        { 40000, NPC_CORRUPTOR_TENTACLE, 1 },
        { 15000, NPC_DEATHGLARE_TENTACLE, 1 }
    };
    uint32_t const IlgynothWaveCount = 7;
    uint32_t const IlgynothWaveRepeatFrom = 5;
    uint32_t const IlgynothWaveRepeatDelayMs = 30000;

    // Elerethe Renferal
    uint32_t const EleretheBerserkMs = 540000;
    float const EleretheEnergyPerSecond = 1.1f;
    uint32_t const EleretheSpiderlingsPerFeeding = 4;
    uint32_t const EleretheShadowsPerCast = 2;

    // Cenarius: the grove and the four spots the forces rise from
    LocationVector const CenariusGroveCentre(11366.2f, -12695.1f, 486.91f);
    LocationVector const CenariusForceSpots[4] =
    {
        { 11392.7f, -12723.7f, 487.0f },
        { 11339.5f, -12719.2f, 487.33f },
        { 11340.0f, -12667.8f, 487.45f },
        { 11391.2f, -12670.0f, 487.18f }
    };
    float const CenariusEnergyPerSecond = 1.33f;
    uint32_t const CenariusWaveCount = 6;
    uint32_t const CenariusWispsPerPack = 5;

    struct CenariusForce
    {
        uint32_t entry;
        uint8_t spot;
    };

    // wisps ignore the spot and gather on the grove ring
    CenariusForce const CenariusWaves[CenariusWaveCount][3] =
    {
        { { NPC_CORRUPTED_WISP, 0 }, { NPC_NIGHTMARE_ANCIENT, 1 }, { NPC_ROTTEN_DRAKE, 3 } },
        { { NPC_CORRUPTED_WISP, 0 }, { NPC_TWISTED_SISTER, 2 }, { NPC_ROTTEN_DRAKE, 3 } },
        { { NPC_NIGHTMARE_ANCIENT, 1 }, { NPC_TWISTED_SISTER, 2 }, { NPC_TWISTED_SISTER, 3 } },
        { { NPC_CORRUPTED_WISP, 0 }, { NPC_ROTTEN_DRAKE, 2 }, { NPC_ROTTEN_DRAKE, 3 } },
        { { NPC_NIGHTMARE_ANCIENT, 1 }, { NPC_TWISTED_SISTER, 2 }, { NPC_TWISTED_SISTER, 3 } },
        { { NPC_CORRUPTED_WISP, 0 }, { NPC_TWISTED_SISTER, 2 }, { NPC_TWISTED_SISTER, 3 } }
    };

    enum class NightmareDoorRule : uint8_t
    {
        SealedWhileFighting,
        OpenWhenDone,
        OpenWhileEyeDown
    };

    struct NightmareDoor
    {
        uint32_t entry;
        uint32_t encounterId;
        NightmareDoorRule rule;
    };

    NightmareDoor const NightmareDoors[] =
    {
        { GO_NYTHENDRA_ROOT_WALL_1, DATA_NYTHENDRA, NightmareDoorRule::OpenWhenDone },
        { GO_NYTHENDRA_ROOT_WALL_2, DATA_NYTHENDRA, NightmareDoorRule::OpenWhenDone },
        { GO_NYTHENDRA_ROOT_WALL_3, DATA_NYTHENDRA, NightmareDoorRule::SealedWhileFighting },
        { GO_NYTHENDRA_ROOT_WALL_4, DATA_NYTHENDRA, NightmareDoorRule::SealedWhileFighting },
        { GO_NYTHENDRA_ROOT_WALL_8, DATA_NYTHENDRA, NightmareDoorRule::OpenWhenDone },
        { GO_NYTHENDRA_ROOT_WALL_9, DATA_NYTHENDRA, NightmareDoorRule::OpenWhenDone },
        { GO_RENFERAL_TWISTING_ROOTS, DATA_ELERETHE_RENFERAL, NightmareDoorRule::SealedWhileFighting },
        { GO_URSOC_DOOR_1, DATA_URSOC, NightmareDoorRule::SealedWhileFighting },
        { GO_URSOC_DOOR_2, DATA_URSOC, NightmareDoorRule::SealedWhileFighting },
        { GO_ILGYNOTH_ROOT_WALL, DATA_ILGYNOTH, NightmareDoorRule::SealedWhileFighting },
        { GO_ILGYNOTH_WALL_OF_ROOTS_1, DATA_ILGYNOTH, NightmareDoorRule::SealedWhileFighting },
        { GO_ILGYNOTH_WALL_OF_ROOTS_2, DATA_ILGYNOTH, NightmareDoorRule::SealedWhileFighting },
        { GO_EYE_OF_ILGYNOTH_FORCEFIELD, DATA_ILGYNOTH, NightmareDoorRule::OpenWhileEyeDown }
    };

    LocationVector ringPoint(LocationVector const& centre, float radius, float angle)
    {
        return LocationVector(centre.x + radius * std::cos(angle), centre.y + radius * std::sin(angle), centre.z, angle);
    }

    LocationVector scatterAround(LocationVector const& centre, float minRadius, float maxRadius)
    {
        return ringPoint(centre, Util::getRandomFloat(minRadius, maxRadius), Util::getRandomFloat(NightmareTwoPi));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class EmeraldNightmareInstanceScript : public InstanceScript
{
public:
    explicit EmeraldNightmareInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(EmeraldNightmareEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new EmeraldNightmareInstanceScript(pMapMgr); }

    // a fallen dragon must not end the encounter while another one still lives
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterDown(id))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_URSOC:
            case BOSS_NYTHENDRA:
            case BOSS_YSONDRE:
            case BOSS_LETHON:
            case BOSS_XAVIUS:
            case BOSS_ILGYNOTH:
            case NPC_EYE_OF_ILGYNOTH:
            case BOSS_ELERETHE_RENFERAL:
            case BOSS_CENARIUS:
                mTrackedGuids[pCreature->getEntry()] = pCreature->GetNewGUID();
                break;
            default:
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        NightmareDoor const* pDoorRule = findDoor(pGameObject->getEntry());
        if (pDoorRule == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        alignDoor(pGameObject, *pDoorRule);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t /*state*/) override
    {
        // every state change of Il'gynoth starts with a living Eye
        if (id == DATA_ILGYNOTH)
            mEyeDown = false;

        alignDoors();
    }

    Creature* getTracked(uint32_t entry)
    {
        auto const itr = mTrackedGuids.find(entry);
        return itr != mTrackedGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    // perched dragons count as well
    bool isEncounterDown(uint32_t encounterId)
    {
        if (encounterId != DATA_DRAGONS_OF_NIGHTMARE)
            return true;

        for (Creature* pDragon : getCreatureSetForEntries({ BOSS_YSONDRE, BOSS_TAERAR, BOSS_LETHON, BOSS_EMERISS }))
        {
            if (pDragon->isAlive())
                return false;
        }

        return true;
    }

    // the bond breaks for the whole flight, Ysondre falls last
    void endDragonFlight()
    {
        if (mFlightEnding)
            return;

        mFlightEnding = true;

        Creature* pYsondre = nullptr;
        for (Creature* pDragon : getCreatureSetForEntries({ BOSS_YSONDRE, BOSS_TAERAR, BOSS_LETHON, BOSS_EMERISS }))
        {
            if (!pDragon->isAlive())
                continue;

            if (pDragon->getEntry() == BOSS_YSONDRE)
                pYsondre = pDragon;
            else
                pDragon->dealDamage(pDragon, pDragon->getHealth(), 0);
        }

        if (pYsondre != nullptr && pYsondre->isAlive())
            pYsondre->dealDamage(pYsondre, pYsondre->getHealth(), 0);

        mFlightEnding = false;
    }

    void setEyeDown(bool down)
    {
        mEyeDown = down;
        alignDoors();
    }

private:
    bool isDone(uint32_t id) const { return getBossState(id) == EncounterStates::Performed; }
    bool isFighting(uint32_t id) const { return getBossState(id) == EncounterStates::InProgress; }

    NightmareDoor const* findDoor(uint32_t entry) const
    {
        for (NightmareDoor const& door : NightmareDoors)
        {
            if (door.entry == entry)
                return &door;
        }

        return nullptr;
    }

    bool shouldBeOpen(NightmareDoor const& door) const
    {
        switch (door.rule)
        {
            case NightmareDoorRule::SealedWhileFighting:
                return !isFighting(door.encounterId);
            case NightmareDoorRule::OpenWhenDone:
                return isDone(door.encounterId);
            case NightmareDoorRule::OpenWhileEyeDown:
                return mEyeDown || isDone(door.encounterId);
        }

        return true;
    }

    // changes the state only when it differs
    void alignDoor(GameObject* pDoor, NightmareDoor const& door)
    {
        bool const isOpen = pDoor->getState() != GO_STATE_CLOSED;
        if (isOpen != shouldBeOpen(door))
            pDoor->setState(shouldBeOpen(door) ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    void alignDoors()
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            GameObject* pDoor = getGameObjectByGuid(guid);
            if (pDoor == nullptr)
                continue;

            if (NightmareDoor const* pDoorRule = findDoor(pDoor->getEntry()))
                alignDoor(pDoor, *pDoorRule);
        }
    }

    std::map<uint32_t, WoWGuid> mTrackedGuids;
    std::vector<WoWGuid> mDoorGuids;
    bool mEyeDown = false;
    bool mFlightEnding = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared base: ticks, targeting and add helpers

class EmeraldNightmareCombatAI : public CreatureAIScript
{
public:
    explicit EmeraldNightmareCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override { onEngage(); }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mBerserkLeft = 0;
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        onTick(diff);

        if (!isAlive() || !_isInCombat())
            return;

        scriptEvents.updateEvents(diff, 0);
        updateBerserk(diff);
        onCombatTick(diff);

        if (!isAlive() || (_isCasting() && !mEventsWhileCasting))
            return;

        if (uint32_t eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    // every update, in and out of combat
    virtual void onTick(uint32_t /*diff*/) {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    EmeraldNightmareInstanceScript* getNightmare()
    {
        if (getCreature()->GetMapId() != MAP_THE_EMERALD_NIGHTMARE)
            return nullptr;

        return static_cast<EmeraldNightmareInstanceScript*>(getInstanceScript());
    }

    Creature* tracked(uint32_t entry)
    {
        EmeraldNightmareInstanceScript* pNightmare = getNightmare();
        return pNightmare != nullptr ? pNightmare->getTracked(entry) : nullptr;
    }

    bool isEncounterInProgress(uint32_t encounterId)
    {
        InstanceScript* pInstance = getInstanceScript();
        return pInstance != nullptr && pInstance->getBossState(encounterId) == EncounterStates::InProgress;
    }

    // shuffled living players in sight, the tank only when nobody else is left
    std::vector<Unit*> pickPlayers(uint32_t count, bool skipVictim)
    {
        Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();

        std::vector<Unit*> candidates;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || !pObject->isPlayer() || !pObject->IsWithinLOSInMap(getCreature()))
                continue;

            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (!pPlayer->isAlive() || getRangeToObject(pPlayer) > 100.0f || (skipVictim && pPlayer == pVictim))
                continue;

            candidates.push_back(pPlayer);
        }

        if (candidates.empty() && skipVictim && pVictim != nullptr && pVictim->isPlayer() && pVictim->isAlive())
            candidates.push_back(pVictim);

        Util::randomShuffleVector(candidates);
        if (candidates.size() > count)
            candidates.resize(count);

        return candidates;
    }

    Unit* randomPlayer(bool skipVictim)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipVictim);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* nearestPlayer(float maxRange)
    {
        Unit* pNearest = nullptr;
        float nearest = maxRange;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject == nullptr || !pObject->isPlayer())
                continue;

            Unit* pPlayer = static_cast<Unit*>(pObject);
            if (!pPlayer->isAlive())
                continue;

            float const range = getRangeToObject(pPlayer);
            if (range <= nearest)
            {
                nearest = range;
                pNearest = pPlayer;
            }
        }

        return pNearest;
    }

    uint32_t playerCount() { return static_cast<uint32_t>(pickPlayers(NightmareRaidCap, false).size()); }

    void castOnRandomPlayer(uint32_t spellId, bool skipVictim, bool triggered = false)
    {
        if (Unit* pTarget = randomPlayer(skipVictim))
            castSpell(pTarget, spellId, triggered);
    }

    void castOnPlayers(uint32_t spellId, uint32_t count, bool skipVictim)
    {
        for (Unit* pTarget : pickPlayers(count, skipVictim))
            castSpell(pTarget, spellId, true);
    }

    // summoned add that joins the fight right away
    Creature* callAdd(uint32_t entry, LocationVector const& position)
    {
        Creature* pAdd = summonCreature(entry, position, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // drops all threat and sticks to one player
    void fixateOn(Unit* pTarget)
    {
        _clearHateList();
        addThreat(pTarget, 1000000.0f);
        attackStart(pTarget);
    }

    void joinFight(Creature* pOther)
    {
        if (pOther != nullptr && pOther->isAlive() && !pOther->isInCombat())
            setZoneWideCombat(pOther);
    }

    void tell(Creature* pTarget, int32_t action)
    {
        if (pTarget != nullptr && pTarget->isAlive() && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }

    void showFrame(bool engaged)
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(engaged ? EncounterFrameEngage : EncounterFrameDisengaged, getCreature());
    }

    void raidWarning(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void startBerserk(uint32_t delayMs) { mBerserkLeft = delayMs; }

    static float gain(float perSecond, uint32_t diff) { return perSecond * static_cast<float>(diff) / 1000.0f; }

    // countdown helper, true once when it runs out
    static bool countDown(uint32_t& left, uint32_t diff)
    {
        if (left == 0)
            return false;

        if (left > diff)
        {
            left -= diff;
            return false;
        }

        left = 0;
        return true;
    }

    bool mEventsWhileCasting = false;

private:
    void updateBerserk(uint32_t diff)
    {
        if (countDown(mBerserkLeft, diff))
            castSpellOnSelf(SPELL_EMERALD_NIGHTMARE_BERSERK, true);
    }

    uint32_t mBerserkLeft = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class EmeraldNightmareBossAI : public EmeraldNightmareCombatAI
{
public:
    EmeraldNightmareBossAI(Creature* pCreature, uint32_t encounterId) : EmeraldNightmareCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        EmeraldNightmareCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        // only a living boss means a wipe
        if (!isAlive())
            return;

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::Failed);
        }

        EmeraldNightmareCombatAI::OnCombatStop(pTarget);
        onWipe();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            EmeraldNightmareInstanceScript* pNightmare = getNightmare();
            if (pNightmare == nullptr || pNightmare->isEncounterDown(mEncounterId))
                pInstance->setBossState(mEncounterId, EncounterStates::Performed);
        }
    }

protected:
    // after summons are gone, the boss is back at rest
    virtual void onWipe() {}

    bool isEncounterDone()
    {
        InstanceScript* pInstance = getInstanceScript();
        return pInstance != nullptr && pInstance->getBossState(mEncounterId) == EncounterStates::Performed;
    }

    uint32_t mEncounterId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ursoc

class EmeraldNightmareUrsocAI : public EmeraldNightmareBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareUrsocAI(c); }
    explicit EmeraldNightmareUrsocAI(Creature* pCreature) : EmeraldNightmareBossAI(pCreature, DATA_URSOC) {}

protected:
    void onEngage() override
    {
        mEnergy = 0.0f;
        mCharging = false;
        mFrenzied = false;
        mGazeTarget = 0;

        castSpellOnSelf(SPELL_URSOC_FEROCIOUS_YELL, true);
        startBerserk(UrsocBerserkMs);

        scriptEvents.addEvent(EVENT_URSOC_OVERWHELM, 10000);
        scriptEvents.addEvent(EVENT_URSOC_REND_FLESH, 13000);
        scriptEvents.addEvent(EVENT_URSOC_ROARING_CACOPHONY, 18000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mFrenzied && _getHealthPercent() < 31)
        {
            mFrenzied = true;
            raidWarning("Ursoc goes into a Blood Frenzy!");
            castSpellOnSelf(SPELL_URSOC_BLOOD_FRENZY, true);
        }

        // full energy sends him barreling at a marked player
        if (mCharging)
            return;

        mEnergy += gain(UrsocEnergyPerSecond, diff);
        if (mEnergy >= 100.0f)
            markCharge();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_URSOC_OVERWHELM:
                castSpellOnVictim(SPELL_URSOC_OVERWHELM);
                scriptEvents.addEvent(EVENT_URSOC_OVERWHELM, 10000);
                break;
            case EVENT_URSOC_REND_FLESH:
                castSpellOnVictim(SPELL_URSOC_REND_FLESH);
                scriptEvents.addEvent(EVENT_URSOC_REND_FLESH, 20000);
                break;
            case EVENT_URSOC_ROARING_CACOPHONY:
                raidWarning("Ursoc lets out a Roaring Cacophony!");
                castSpellAOE(SPELL_URSOC_ROARING_CACOPHONY);
                scriptEvents.addEvent(EVENT_URSOC_ROARING_CACOPHONY, 20000);
                break;
            case EVENT_URSOC_CHARGE:
                charge();
                break;
            case EVENT_URSOC_IMPACT:
                castSpellAOE(SPELL_URSOC_BARRELING_IMPACT, true);
                mCharging = false;
                break;
            default:
                break;
        }
    }

private:
    void markCharge()
    {
        mEnergy = 0.0f;

        Unit* pTarget = randomPlayer(true);
        if (pTarget == nullptr)
            return;

        mCharging = true;
        mGazeTarget = pTarget->getGuid();
        castSpell(pTarget, SPELL_URSOC_FOCUSED_GAZE, true);
        raidWarning("Ursoc fixes his Focused Gaze on his next victim!");
        scriptEvents.addEvent(EVENT_URSOC_CHARGE, 6000);
    }

    void charge()
    {
        Unit* pTarget = getCreature()->getWorldMapUnit(mGazeTarget);
        if (pTarget != nullptr && pTarget->isAlive())
        {
            castSpell(pTarget, SPELL_URSOC_TRAMPLING_SLAM, true);
            moveCharge(pTarget->GetPosition());
        }

        scriptEvents.addEvent(EVENT_URSOC_IMPACT, 1500);
    }

    float mEnergy = 0.0f;
    bool mCharging = false;
    bool mFrenzied = false;
    uint64_t mGazeTarget = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nythendra

class EmeraldNightmareNythendraAI : public EmeraldNightmareBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareNythendraAI(c); }
    explicit EmeraldNightmareNythendraAI(Creature* pCreature) : EmeraldNightmareBossAI(pCreature, DATA_NYTHENDRA) {}

protected:
    void onEngage() override
    {
        mEnergy = 100.0f;
        mSwarmLeft = 0;
        mBreathTicks = 0;
        mBreathTickIn = 0;

        // more players, more Rot from the start
        uint32_t const raid = playerCount();
        mRotTargets = raid < 15 ? 1 : (raid < 25 ? 2 : 3);

        startRotation(true);
    }

    void onWipe() override
    {
        mSwarmLeft = 0;
        mBreathTicks = 0;
        releaseRoot();
    }

    void onCombatTick(uint32_t diff) override
    {
        updateBreath(diff);

        if (mSwarmLeft > 0)
        {
            if (countDown(mSwarmLeft, diff))
                endSwarm();
            return;
        }

        mEnergy -= gain(NythendraDrainPerSecond, diff);
        if (mEnergy <= 0.0f)
            beginSwarm();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NYTHENDRA_ROT:
                castOnPlayers(SPELL_NYTHENDRA_ROT, mRotTargets, true);
                scriptEvents.addEvent(EVENT_NYTHENDRA_ROT, 16000);
                break;
            case EVENT_NYTHENDRA_VOLATILE_ROT:
                castSpellOnVictim(SPELL_NYTHENDRA_VOLATILE_ROT);
                scriptEvents.addEvent(EVENT_NYTHENDRA_VOLATILE_ROT, 25000);
                break;
            case EVENT_NYTHENDRA_INFESTED_BREATH:
                exhale();
                scriptEvents.addEvent(EVENT_NYTHENDRA_INFESTED_BREATH, 36000);
                break;
            default:
                break;
        }
    }

private:
    void startRotation(bool opening)
    {
        scriptEvents.addEvent(EVENT_NYTHENDRA_ROT, opening ? 6000 : 14000);
        scriptEvents.addEvent(EVENT_NYTHENDRA_VOLATILE_ROT, opening ? 24000 : 35000);
        scriptEvents.addEvent(EVENT_NYTHENDRA_INFESTED_BREATH, opening ? Util::getRandomUInt(35000, 40000) : 47000);
    }

    void exhale()
    {
        Unit* pTarget = randomPlayer(false);
        if (pTarget == nullptr)
            return;

        raidWarning("Nythendra begins to exhale an Infested Breath!");
        getCreature()->setFacingToObject(pTarget);
        castSpell(pTarget, SPELL_NYTHENDRA_INFESTED_BREATH);
        mBreathTicks = 5;
        mBreathTickIn = 3000;
    }

    // five bursts while the breath is channelled
    void updateBreath(uint32_t diff)
    {
        if (mBreathTicks == 0 || !countDown(mBreathTickIn, diff))
            return;

        castSpellAOE(SPELL_NYTHENDRA_INFESTED_BREATH_DAMAGE, true);
        if (--mBreathTicks > 0)
            mBreathTickIn = 1000;
    }

    void beginSwarm()
    {
        mEnergy = 0.0f;
        mSwarmLeft = NythendraSwarmMs;
        mBreathTicks = 0;
        ++mRotTargets;
        scriptEvents.resetEvents();

        setRooted(true);
        _setMeleeDisabled(true);
        raidWarning("Nythendra calls upon the Heart of the Swarm!");
        castSpellOnSelf(SPELL_NYTHENDRA_HEART_OF_THE_SWARM);

        for (uint32_t i = 0; i < NythendraVerminPerSwarm; ++i)
            summonCreature(NPC_CORRUPTED_VERMIN, scatterAround(getCreature()->GetPosition(), 8.0f, 30.0f), MANUAL_DESPAWN);
    }

    void endSwarm()
    {
        mEnergy = 100.0f;
        releaseRoot();
        summons.despawnEntry(NPC_CORRUPTED_VERMIN);
        startRotation(false);
    }

    void releaseRoot()
    {
        setRooted(false);
        _setMeleeDisabled(false);
    }

    float mEnergy = 100.0f;
    uint32_t mSwarmLeft = 0;
    uint32_t mRotTargets = 1;
    uint32_t mBreathTicks = 0;
    uint32_t mBreathTickIn = 0;
};

// crawls to the closest player and bursts there three times
class EmeraldNightmareCorruptedVerminAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareCorruptedVerminAI(c); }
    explicit EmeraldNightmareCorruptedVerminAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        mThinkIn = 1000;
    }

protected:
    void onTick(uint32_t diff) override
    {
        if (!countDown(mThinkIn, diff))
            return;

        if (mBursts > 0)
        {
            castSpellAOE(SPELL_VERMIN_BURST_OF_CORRUPTION, true);
            mThinkIn = 2000;
            if (--mBursts == 0)
            {
                setRooted(false);
                mThinkIn = 18000;
            }
            return;
        }

        mThinkIn = 1000;

        Unit* pPlayer = nearestPlayer(40.0f);
        if (pPlayer == nullptr)
            return;

        if (getRangeToObject(pPlayer) <= 2.5f)
        {
            stopMovement();
            setRooted(true);
            mBursts = 3;
            mThinkIn = 2000;
            return;
        }

        moveTo(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), false);
    }

private:
    uint32_t mThinkIn = 0;
    uint32_t mBursts = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Dragons of Nightmare

class EmeraldNightmareYsondreAI : public EmeraldNightmareBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareYsondreAI(c); }
    explicit EmeraldNightmareYsondreAI(Creature* pCreature) : EmeraldNightmareBossAI(pCreature, DATA_DRAGONS_OF_NIGHTMARE) {}

    void OnDied(Unit* pKiller) override
    {
        mBond.clear();
        if (EmeraldNightmareInstanceScript* pNightmare = getNightmare())
            pNightmare->endDragonFlight();

        EmeraldNightmareBossAI::OnDied(pKiller);
    }

protected:
    // the flight is called once Ysondre stands in the world
    void onTick(uint32_t /*diff*/) override
    {
        if (mFlightCalled || _isInCombat())
            return;

        mFlightCalled = true;
        if (!isEncounterDone())
            summonFlight();
    }

    void onEngage() override
    {
        mEnergy = 0.0f;
        mSwapStage = 0;
        mPool = 1.0f;
        mBond.clear();

        bind(getCreature());
        castSpellOnSelf(SPELL_YSONDRE_MARK, true);

        // the grounded companion shares the pool from the first blow
        if (Creature* pCompanion = companion(0))
        {
            bind(pCompanion);
            joinFight(pCompanion);
        }

        scriptEvents.addEvent(EVENT_DRAGON_CORRUPTED_BREATH, 16000);
        scriptEvents.addEvent(EVENT_YSONDRE_NIGHTMARE_BLAST, 40000);
    }

    void onWipe() override
    {
        mBond.clear();
        summonFlight();
    }

    void onCombatTick(uint32_t diff) override
    {
        shareWounds();
        if (!isAlive())
            return;

        checkSwap();

        mEnergy += gain(YsondreEnergyPerSecond, diff);
        if (mEnergy >= 100.0f && !_isCasting())
        {
            mEnergy = 0.0f;
            callDefiledSpirits();
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DRAGON_CORRUPTED_BREATH:
                castSpellOnVictim(SPELL_DRAGON_CORRUPTED_BREATH);
                scriptEvents.addEvent(EVENT_DRAGON_CORRUPTED_BREATH, 32000);
                break;
            case EVENT_YSONDRE_NIGHTMARE_BLAST:
                castSpellOnVictim(SPELL_YSONDRE_NIGHTMARE_BLAST);
                scriptEvents.addEvent(EVENT_YSONDRE_NIGHTMARE_BLAST, 15000);
                break;
            default:
                break;
        }
    }

private:
    struct BondShare
    {
        uint64_t guid;
        float fraction;
    };

    // two of the three companions, one at her side and one circling above
    void summonFlight()
    {
        std::vector<uint32_t> companions = { BOSS_TAERAR, BOSS_LETHON, BOSS_EMERISS };
        Util::randomShuffleVector(companions);

        mCompanionGuids[0] = 0;
        mCompanionGuids[1] = 0;

        if (Creature* pGrounded = summonCreature(companions[0], DragonGroundSpot))
            mCompanionGuids[0] = pGrounded->getGuid();

        if (Creature* pAloft = summonCreature(companions[1], DragonPerch))
        {
            mCompanionGuids[1] = pAloft->getGuid();
            tell(pAloft, ACTION_DRAGON_PERCH_AT_ONCE);
        }
    }

    Creature* companion(uint8_t slot)
    {
        if (mCompanionGuids[slot] == 0)
            return nullptr;

        Creature* pCompanion = getCreature()->getWorldMapCreature(mCompanionGuids[slot]);
        return pCompanion != nullptr && pCompanion->isAlive() ? pCompanion : nullptr;
    }

    void bind(Creature* pDragon)
    {
        for (BondShare const& share : mBond)
        {
            if (share.guid == pDragon->getGuid())
                return;
        }

        mBond.push_back({ pDragon->getGuid(), applyPool(pDragon) });
    }

    void unbind(Creature* pDragon)
    {
        for (auto itr = mBond.begin(); itr != mBond.end(); ++itr)
        {
            if (itr->guid == pDragon->getGuid())
            {
                mBond.erase(itr);
                return;
            }
        }
    }

    // returns the fraction the dragon really holds afterwards
    float applyPool(Creature* pDragon)
    {
        uint32_t const maxHealth = pDragon->getMaxHealth();
        if (maxHealth == 0)
            return mPool;

        uint32_t health = static_cast<uint32_t>(static_cast<float>(maxHealth) * mPool);
        health = std::max<uint32_t>(1, std::min(health, maxHealth));
        pDragon->setHealth(health);
        return static_cast<float>(health) / static_cast<float>(maxHealth);
    }

    // damage on any bonded dragon drains one common pool
    void shareWounds()
    {
        float loss = 0.0f;
        for (BondShare const& share : mBond)
        {
            Creature* pDragon = getCreature()->getWorldMapCreature(share.guid);
            if (pDragon == nullptr || !pDragon->isAlive() || pDragon->getMaxHealth() == 0)
                continue;

            loss += share.fraction - static_cast<float>(pDragon->getHealth()) / static_cast<float>(pDragon->getMaxHealth());
        }

        if (loss == 0.0f)
            return;

        mPool = std::min(1.0f, mPool - loss);
        if (mPool <= 0.0f)
        {
            mBond.clear();
            if (EmeraldNightmareInstanceScript* pNightmare = getNightmare())
                pNightmare->endDragonFlight();
            return;
        }

        for (BondShare& share : mBond)
        {
            Creature* pDragon = getCreature()->getWorldMapCreature(share.guid);
            if (pDragon != nullptr && pDragon->isAlive())
                share.fraction = applyPool(pDragon);
        }
    }

    // companions trade places at 70% and 40%
    void checkSwap()
    {
        float const percent = mPool * 100.0f;
        if (mSwapStage == 0 && percent < 71.0f)
        {
            mSwapStage = 1;
            swapCompanions(0, 1);
        }
        else if (mSwapStage == 1 && percent < 41.0f)
        {
            mSwapStage = 2;
            swapCompanions(1, 0);
        }
    }

    void swapCompanions(uint8_t leaving, uint8_t arriving)
    {
        raidWarning("The Dragons of Nightmare trade places!");

        if (Creature* pLeaving = companion(leaving))
        {
            unbind(pLeaving);
            tell(pLeaving, ACTION_DRAGON_TAKE_OFF);
        }

        if (Creature* pArriving = companion(arriving))
        {
            bind(pArriving);
            tell(pArriving, ACTION_DRAGON_LAND);
        }
    }

    void callDefiledSpirits()
    {
        raidWarning("Ysondre calls forth Defiled Spirits!");
        for (Unit* pPlayer : pickPlayers(playerCount() / 5 + 1, false))
            summonCreature(NPC_DEFILED_DRUID_SPIRIT, pPlayer->GetPosition(), TIMED_DESPAWN, 8000);
    }

    float mEnergy = 0.0f;
    float mPool = 1.0f;
    uint8_t mSwapStage = 0;
    bool mFlightCalled = false;
    uint64_t mCompanionGuids[2] = { 0, 0 };
    std::vector<BondShare> mBond;
};

// Taerar, Lethon and Emeriss
class EmeraldNightmareDragonAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareDragonAI(c); }
    explicit EmeraldNightmareDragonAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

    void OnCombatStart(Unit* pTarget) override
    {
        // a dragon in the air does not fight
        if (mAloft)
            return;

        showFrame(true);
        joinFight(tracked(BOSS_YSONDRE));
        EmeraldNightmareCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive() && !mAloft)
            showFrame(false);

        EmeraldNightmareCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        showFrame(false);
        if (EmeraldNightmareInstanceScript* pNightmare = getNightmare())
            pNightmare->endDragonFlight();
    }

    void DoAction(int32_t action) override
    {
        switch (action)
        {
            case ACTION_DRAGON_PERCH_AT_ONCE:
                setAloft(true);
                setRooted(true);
                break;
            case ACTION_DRAGON_TAKE_OFF:
                takeOff();
                break;
            case ACTION_DRAGON_LAND:
                descend();
                break;
            default:
                break;
        }
    }

protected:
    void onEngage() override { startGroundKit(); }

    void onTick(uint32_t diff) override
    {
        if (countDown(mPerchIn, diff))
            setRooted(true);

        if (countDown(mTouchdownIn, diff))
            touchdown();
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mAloft)
            return;

        mEnergy += gain(DragonCompanionEnergyPerSecond, diff);
        if (mEnergy >= 100.0f && !_isCasting())
        {
            mEnergy = 0.0f;
            unleashSpecial();
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DRAGON_CORRUPTED_BREATH:
                castSpellOnVictim(SPELL_DRAGON_CORRUPTED_BREATH);
                scriptEvents.addEvent(EVENT_DRAGON_CORRUPTED_BREATH, 32000);
                break;
            case EVENT_TAERAR_SEEPING_FOG:
                castSpellOnSelf(SPELL_TAERAR_SEEPING_FOG, true);
                scriptEvents.addEvent(EVENT_TAERAR_SEEPING_FOG, 16000);
                break;
            case EVENT_EMERISS_ESSENCE:
                callAdd(NPC_ESSENCE_OF_CORRUPTION, scatterAround(getCreature()->GetPosition(), 5.0f, 12.0f));
                scriptEvents.addEvent(EVENT_EMERISS_ESSENCE, 30000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t markSpell()
    {
        switch (getCreature()->getEntry())
        {
            case BOSS_TAERAR:
                return SPELL_TAERAR_MARK;
            case BOSS_LETHON:
                return SPELL_LETHON_MARK;
            default:
                return SPELL_EMERISS_MARK;
        }
    }

    void startGroundKit()
    {
        scriptEvents.resetEvents();
        mEnergy = 0.0f;
        castSpellOnSelf(markSpell(), true);
        scriptEvents.addEvent(EVENT_DRAGON_CORRUPTED_BREATH, 16000);

        switch (getCreature()->getEntry())
        {
            case BOSS_TAERAR:
                scriptEvents.addEvent(EVENT_TAERAR_SEEPING_FOG, 26000);
                break;
            case BOSS_LETHON:
                castSpellOnSelf(SPELL_LETHON_GLOOM, true);
                break;
            case BOSS_EMERISS:
                scriptEvents.addEvent(EVENT_EMERISS_ESSENCE, 30000);
                break;
            default:
                break;
        }
    }

    void setAloft(bool aloft)
    {
        mAloft = aloft;
        if (aloft)
        {
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            setReactState(REACT_PASSIVE);
            _setMeleeDisabled(true);
            setFlyMode(true);
        }
        else
        {
            removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            setReactState(REACT_AGGRESSIVE);
            _setMeleeDisabled(false);
            setFlyMode(false);
        }
    }

    void takeOff()
    {
        scriptEvents.resetEvents();
        attackStop();
        setAloft(true);
        _removeAura(markSpell());
        _removeAura(SPELL_LETHON_GLOOM);
        showFrame(false);

        mTouchdownIn = 0;
        mPerchIn = DragonFlightMs;
        movePoint(1, DragonPerch, false);
    }

    void descend()
    {
        mPerchIn = 0;
        setRooted(false);
        mTouchdownIn = DragonFlightMs;
        movePoint(2, DragonLanding, false);
    }

    void touchdown()
    {
        setAloft(false);

        // still in combat from before the flight: resume here
        if (_isInCombat())
        {
            showFrame(true);
            startGroundKit();
        }

        setZoneWideCombat();
    }

    void unleashSpecial()
    {
        switch (getCreature()->getEntry())
        {
            case BOSS_TAERAR:
                raidWarning("Taerar calls forth a Shade of Taerar!");
                callAdd(NPC_SHADE_OF_TAERAR, scatterAround(getCreature()->GetPosition(), 5.0f, 10.0f));
                break;
            case BOSS_LETHON:
                raidWarning("Lethon siphons the spirits of his foes!");
                castSpellAOE(SPELL_LETHON_SIPHON_SPIRIT);
                for (Unit* pPlayer : pickPlayers(playerCount() / 5 + 1, false))
                    summonCreature(NPC_SPIRIT_SHADE, pPlayer->GetPosition(), CORPSE_DESPAWN);
                break;
            case BOSS_EMERISS:
                raidWarning("Emeriss spreads a Volatile Infection!");
                castOnPlayers(SPELL_EMERISS_VOLATILE_INFECTION, 2, true);
                break;
            default:
                break;
        }
    }

    float mEnergy = 0.0f;
    bool mAloft = false;
    uint32_t mPerchIn = 0;
    uint32_t mTouchdownIn = 0;
};

// roots in place and erupts
class EmeraldNightmareDefiledSpiritAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareDefiledSpiritAI(c); }
    explicit EmeraldNightmareDefiledSpiritAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_DEFILED_SPIRIT_ROOT, true);
        mEruptIn = 500;
    }

protected:
    void onTick(uint32_t diff) override
    {
        if (countDown(mEruptIn, diff))
            castSpellAOE(SPELL_DEFILED_SPIRIT_ERUPTION);
    }

private:
    uint32_t mEruptIn = 0;
};

// drifts to Lethon and feeds the flight
class EmeraldNightmareSpiritShadeAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareSpiritShadeAI(c); }
    explicit EmeraldNightmareSpiritShadeAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        mStepIn = 500;
    }

protected:
    void onTick(uint32_t diff) override
    {
        if (!countDown(mStepIn, diff))
            return;

        mStepIn = 2000;

        Creature* pLethon = tracked(BOSS_LETHON);
        if (!isEncounterInProgress(DATA_DRAGONS_OF_NIGHTMARE) || pLethon == nullptr || !pLethon->isAlive() || pLethon->hasUnitFlags(UNIT_FLAG_NOT_SELECTABLE))
        {
            mStepIn = 0;
            despawn(100);
            return;
        }

        if (getRangeToObject(pLethon) < 5.0f)
        {
            castSpellOnSelf(SPELL_SPIRIT_SHADE_OFFERING_LETHON, true);
            castSpellOnSelf(SPELL_SPIRIT_SHADE_OFFERING_YSONDRE, true);
            mStepIn = 0;
            despawn(1000);
            return;
        }

        moveTo(pLethon->GetPositionX(), pLethon->GetPositionY(), pLethon->GetPositionZ(), false);
    }

private:
    uint32_t mStepIn = 0;
};

class EmeraldNightmareShadeOfTaerarAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareShadeOfTaerarAI(c); }
    explicit EmeraldNightmareShadeOfTaerarAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

    void OnLoad() override { castSpellOnSelf(SPELL_SHADE_NIGHTMARE_VISAGE, true); }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_SHADE_CORRUPTED_BREATH, 5000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SHADE_CORRUPTED_BREATH)
            return;

        castSpellOnVictim(SPELL_SHADE_CORRUPTED_BREATH);
        scriptEvents.addEvent(EVENT_SHADE_CORRUPTED_BREATH, 30000);
    }
};

class EmeraldNightmareEssenceOfCorruptionAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareEssenceOfCorruptionAI(c); }
    explicit EmeraldNightmareEssenceOfCorruptionAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_ESSENCE_CORRUPTION, 2000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_ESSENCE_CORRUPTION)
            return;

        castOnRandomPlayer(SPELL_ESSENCE_CORRUPTION, false);
        scriptEvents.addEvent(EVENT_ESSENCE_CORRUPTION, 6000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Xavius

class EmeraldNightmareXaviusAI : public EmeraldNightmareBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareXaviusAI(c); }
    explicit EmeraldNightmareXaviusAI(Creature* pCreature) : EmeraldNightmareBossAI(pCreature, DATA_XAVIUS) {}

protected:
    void onEngage() override
    {
        mEnergy = 30.0f;
        mEruptions = 0;
        castSpellOnSelf(SPELL_XAVIUS_UNFATHOMABLE_REALITY, true);
        enterPhase(1);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mPhase == 1 && _getHealthPercent() < 65)
        {
            raidWarning("Xavius draws deeper on the Nightmare!");
            enterPhase(2);
        }
        else if (mPhase == 2 && _getHealthPercent() < 30)
        {
            raidWarning("Xavius unleashes the full might of the Nightmare!");
            enterPhase(3);
        }

        mEnergy += gain(mPhase == 2 ? XaviusCallingEnergyPerSecond : XaviusEnergyPerSecond, diff);
        if (mEnergy >= 100.0f && !_isCasting())
        {
            mEnergy = 0.0f;
            unleash();
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_XAVIUS_DARKENING_SOUL:
                castSpellOnVictim(SPELL_XAVIUS_DARKENING_SOUL);
                scriptEvents.addEvent(EVENT_XAVIUS_DARKENING_SOUL, 8000);
                break;
            case EVENT_XAVIUS_BLACKENING_SOUL:
                castSpellOnVictim(SPELL_XAVIUS_BLACKENING_SOUL);
                scriptEvents.addEvent(EVENT_XAVIUS_BLACKENING_SOUL, 7000);
                break;
            case EVENT_XAVIUS_NIGHTMARE_BLADES:
                markBlades();
                scriptEvents.addEvent(EVENT_XAVIUS_NIGHTMARE_BLADES, 16000);
                break;
            case EVENT_XAVIUS_BLADES_STRIKE:
                strikeBlades();
                break;
            case EVENT_XAVIUS_CORRUPTION_METEOR:
                raidWarning("Xavius hurls a Corruption Meteor!");
                castOnRandomPlayer(SPELL_XAVIUS_CORRUPTION_METEOR, true);
                scriptEvents.addEvent(EVENT_XAVIUS_CORRUPTION_METEOR, 29000);
                break;
            case EVENT_XAVIUS_NIGHTMARE_INFUSION:
                castSpellOnSelf(SPELL_XAVIUS_NIGHTMARE_INFUSION, true);
                scriptEvents.addEvent(EVENT_XAVIUS_NIGHTMARE_INFUSION, 62000);
                break;
            default:
                break;
        }
    }

private:
    void enterPhase(uint8_t phase)
    {
        mPhase = phase;
        mBladeTargets.clear();
        scriptEvents.resetEvents();

        if (phase == 1)
        {
            scriptEvents.addEvent(EVENT_XAVIUS_DARKENING_SOUL, 7000);
            scriptEvents.addEvent(EVENT_XAVIUS_NIGHTMARE_BLADES, 18000);
            return;
        }

        scriptEvents.addEvent(EVENT_XAVIUS_BLACKENING_SOUL, 7000);
        scriptEvents.addEvent(EVENT_XAVIUS_CORRUPTION_METEOR, 21000);
        scriptEvents.addEvent(EVENT_XAVIUS_NIGHTMARE_INFUSION, 32000);

        if (phase == 3)
            scriptEvents.addEvent(EVENT_XAVIUS_NIGHTMARE_BLADES, 18000);
    }

    // phase one: three eruptions, then a horror
    void unleash()
    {
        switch (mPhase)
        {
            case 1:
                if (mEruptions < 3)
                {
                    ++mEruptions;
                    for (Unit* pPlayer : pickPlayers(XaviusTerrorsPerEruption, true))
                        summonCreature(NPC_LURKING_TERROR, scatterAround(pPlayer->GetPosition(), 6.0f, 10.0f), CORPSE_DESPAWN);
                }
                else
                {
                    mEruptions = 0;
                    raidWarning("Xavius manifests a Corruption Horror!");
                    callAdd(NPC_CORRUPTION_HORROR, scatterAround(getCreature()->GetPosition(), 5.0f, 10.0f));
                }
                break;
            case 2:
                raidWarning("Xavius issues a Call of Nightmares!");
                summonCreature(NPC_INCONCEIVABLE_HORROR, ringPoint(XaviusRoomCentre, XaviusRoomRadius, Util::getRandomFloat(NightmareTwoPi)), CORPSE_DESPAWN);
                break;
            default:
                raidWarning("Xavius calls upon the Writhing Deep!");
                castSpellOnSelf(SPELL_XAVIUS_WRITHING_DEEP);
                break;
        }
    }

    void markBlades()
    {
        mBladeTargets.clear();
        for (Unit* pPlayer : pickPlayers(2, true))
        {
            castSpell(pPlayer, SPELL_XAVIUS_NIGHTMARE_BLADES_MARK, true);
            mBladeTargets.push_back(pPlayer->getGuid());
        }

        if (!mBladeTargets.empty())
            scriptEvents.addEvent(EVENT_XAVIUS_BLADES_STRIKE, 4000);
    }

    void strikeBlades()
    {
        for (uint64_t guid : mBladeTargets)
        {
            Unit* pTarget = getCreature()->getWorldMapUnit(guid);
            if (pTarget != nullptr && pTarget->isAlive())
                castSpell(pTarget, SPELL_XAVIUS_NIGHTMARE_BLADES, true);
        }

        mBladeTargets.clear();
    }

    uint8_t mPhase = 1;
    float mEnergy = 30.0f;
    uint32_t mEruptions = 0;
    std::vector<uint64_t> mBladeTargets;
};

// fixates a player and infects on contact
class EmeraldNightmareLurkingTerrorAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareLurkingTerrorAI(c); }
    explicit EmeraldNightmareLurkingTerrorAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        mStepIn = 2000;
    }

protected:
    void onTick(uint32_t diff) override
    {
        if (!countDown(mStepIn, diff))
            return;

        mStepIn = 500;

        if (!isEncounterInProgress(DATA_XAVIUS))
        {
            mStepIn = 0;
            despawn(100);
            return;
        }

        Unit* pQuarry = getCreature()->getWorldMapUnit(mQuarry);
        if (pQuarry == nullptr || !pQuarry->isAlive())
        {
            pQuarry = randomPlayer(false);
            if (pQuarry == nullptr)
                return;

            mQuarry = pQuarry->getGuid();
            castSpell(pQuarry, SPELL_LURKING_TERROR_FIXATION, true);
        }

        if (getRangeToObject(pQuarry) <= 3.0f)
        {
            castSpellAOE(SPELL_LURKING_TERROR_INFECTION, true);
            mStepIn = 0;
            despawn(1000);
            return;
        }

        moveTo(pQuarry->GetPositionX(), pQuarry->GetPositionY(), pQuarry->GetPositionZ(), false);
    }

private:
    uint32_t mStepIn = 0;
    uint64_t mQuarry = 0;
};

class EmeraldNightmareCorruptionHorrorAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareCorruptionHorrorAI(c); }
    explicit EmeraldNightmareCorruptionHorrorAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

    void OnLoad() override { castSpellOnSelf(SPELL_CORRUPTION_HORROR_BIRTH, true); }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_CORRUPTION_HORROR_SWIPE, 9000);
        scriptEvents.addEvent(EVENT_CORRUPTION_HORROR_NOVA, 14000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CORRUPTION_HORROR_SWIPE:
                castSpellOnVictim(SPELL_CORRUPTION_HORROR_SWIPE);
                scriptEvents.addEvent(EVENT_CORRUPTION_HORROR_SWIPE, 10000);
                break;
            case EVENT_CORRUPTION_HORROR_NOVA:
                castSpellAOE(SPELL_CORRUPTION_HORROR_NOVA);
                scriptEvents.addEvent(EVENT_CORRUPTION_HORROR_NOVA, 20000);
                break;
            default:
                break;
        }
    }
};

// creeps to Xavius and ruins the raid from his side
class EmeraldNightmareInconceivableHorrorAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareInconceivableHorrorAI(c); }
    explicit EmeraldNightmareInconceivableHorrorAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        mStepIn = 1000;
    }

protected:
    void onTick(uint32_t diff) override
    {
        if (!countDown(mStepIn, diff))
            return;

        mStepIn = 1000;

        Creature* pXavius = tracked(BOSS_XAVIUS);
        if (!isEncounterInProgress(DATA_XAVIUS) || pXavius == nullptr || !pXavius->isAlive())
        {
            mStepIn = 0;
            despawn(100);
            return;
        }

        if (getRangeToObject(pXavius) > 10.0f)
        {
            moveTo(pXavius->GetPositionX(), pXavius->GetPositionY(), pXavius->GetPositionZ(), false);
            return;
        }

        stopMovement();
        castSpellAOE(SPELL_INCONCEIVABLE_DARK_RUINATION);
        mStepIn = 5000;
    }

private:
    uint32_t mStepIn = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Il'gynoth, the Heart of Corruption

class EmeraldNightmareIlgynothAI : public EmeraldNightmareBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareIlgynothAI(c); }
    explicit EmeraldNightmareIlgynothAI(Creature* pCreature) : EmeraldNightmareBossAI(pCreature, DATA_ILGYNOTH)
    {
        // Cursed Blood keeps flowing during the long reconstitution
        mEventsWhileCasting = true;
    }

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        shield(true);
    }

    void DoAction(int32_t action) override
    {
        if (!_isInCombat())
            return;

        if (action == ACTION_ILGYNOTH_EYE_FELL)
            exposeHeart();
        else if (action == ACTION_ILGYNOTH_CALL_HORROR)
            callHorror();
    }

    // blood of fallen tentacles crawls on toward the Heart
    void spawnIchors(LocationVector const& where, uint32_t count)
    {
        for (uint32_t i = 0; i < count; ++i)
            callAdd(NPC_NIGHTMARE_ICHOR, scatterAround(where, 1.0f, 4.0f));
    }

protected:
    // the court is raised once the Heart stands in the world
    void onTick(uint32_t /*diff*/) override
    {
        if (mCourtRaised || _isInCombat())
            return;

        mCourtRaised = true;
        if (!isEncounterDone())
            raiseCourt();
    }

    void onEngage() override
    {
        mFinalTorpor = false;
        mWaveStep = 0;
        shield(true);

        joinFight(tracked(NPC_EYE_OF_ILGYNOTH));
        for (uint64_t guid : mDominatorGuids)
            joinFight(getCreature()->getWorldMapCreature(guid));

        scriptEvents.addEvent(EVENT_ILGYNOTH_TENTACLES, IlgynothTentacleWaves[0].delayMs);
    }

    void onWipe() override
    {
        shield(true);
        raiseCourt();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ILGYNOTH_TENTACLES:
                raiseTentacles();
                break;
            case EVENT_ILGYNOTH_RECONSTITUTION:
                raidWarning(mFinalTorpor ? "Il'gynoth sinks into Final Torpor!" : "Il'gynoth begins its Dark Reconstitution!");
                castSpellOnSelf(mFinalTorpor ? SPELL_ILGYNOTH_FINAL_TORPOR : SPELL_ILGYNOTH_DARK_RECONSTITUTION);
                mFinalTorpor = true;
                scriptEvents.addEvent(EVENT_ILGYNOTH_EYE_RETURNS, 51000);
                break;
            case EVENT_ILGYNOTH_CURSED_BLOOD:
                castOnRandomPlayer(SPELL_ILGYNOTH_CURSED_BLOOD, false, true);
                scriptEvents.addEvent(EVENT_ILGYNOTH_CURSED_BLOOD, 15000);
                break;
            case EVENT_ILGYNOTH_EYE_RETURNS:
                restoreEye();
                break;
            default:
                break;
        }
    }

private:
    void shield(bool on)
    {
        if (on)
            addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void raiseCourt()
    {
        summonCreature(NPC_EYE_OF_ILGYNOTH, IlgynothEyeSpot, CORPSE_DESPAWN);

        for (uint8_t i = 0; i < 2; ++i)
        {
            mDominatorGuids[i] = 0;
            if (Creature* pDominator = summonCreature(NPC_DOMINATOR_TENTACLE, IlgynothTentacleSpots[i], CORPSE_DESPAWN))
                mDominatorGuids[i] = pDominator->getGuid();
        }
    }

    void raiseTentacles()
    {
        IlgynothTentacleWave const& wave = IlgynothTentacleWaves[mWaveStep];
        for (uint32_t i = 0; i < wave.count; ++i)
            callAdd(wave.entry, scatterAround(IlgynothTentacleSpots[Util::getRandomUInt(1)], 0.0f, 5.0f));

        uint32_t delayMs = 0;
        if (++mWaveStep >= IlgynothWaveCount)
        {
            mWaveStep = IlgynothWaveRepeatFrom;
            delayMs = IlgynothWaveRepeatDelayMs;
        }
        else
        {
            delayMs = IlgynothTentacleWaves[mWaveStep].delayMs;
        }

        scriptEvents.addEvent(EVENT_ILGYNOTH_TENTACLES, delayMs);
    }

    void exposeHeart()
    {
        scriptEvents.resetEvents();
        shield(false);
        raidWarning("The Eye of Il'gynoth falls, the Heart of Corruption lies exposed!");
        scriptEvents.addEvent(EVENT_ILGYNOTH_RECONSTITUTION, 10000);
        scriptEvents.addEvent(EVENT_ILGYNOTH_CURSED_BLOOD, 15000);
    }

    void restoreEye()
    {
        scriptEvents.resetEvents();
        shield(true);

        if (Creature* pEye = summonCreature(NPC_EYE_OF_ILGYNOTH, IlgynothEyeSpot, CORPSE_DESPAWN))
            setZoneWideCombat(pEye);

        if (EmeraldNightmareInstanceScript* pNightmare = getNightmare())
            pNightmare->setEyeDown(false);

        raidWarning("The Eye of Il'gynoth opens again!");
        mWaveStep = 0;
        scriptEvents.addEvent(EVENT_ILGYNOTH_TENTACLES, IlgynothTentacleWaves[0].delayMs);
    }

    void callHorror()
    {
        raidWarning("A Nightmare Horror emerges!");
        summonCreature(NPC_NIGHTMARE_HORROR, IlgynothHorrorSpot, CORPSE_DESPAWN);
    }

    bool mCourtRaised = false;
    bool mFinalTorpor = false;
    uint32_t mWaveStep = 0;
    uint64_t mDominatorGuids[2] = { 0, 0 };
};

// shared lookup of the Heart for its court
class EmeraldNightmareIlgynothCourtAI : public EmeraldNightmareCombatAI
{
public:
    explicit EmeraldNightmareIlgynothCourtAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

protected:
    EmeraldNightmareIlgynothAI* heart()
    {
        Creature* pHeart = tracked(BOSS_ILGYNOTH);
        if (pHeart == nullptr || !pHeart->isAlive())
            return nullptr;

        return dynamic_cast<EmeraldNightmareIlgynothAI*>(pHeart->GetScript());
    }

    void bleedIchors(uint32_t count)
    {
        if (EmeraldNightmareIlgynothAI* pHeart = heart())
            pHeart->spawnIchors(getCreature()->GetPosition(), count);
    }
};

class EmeraldNightmareEyeOfIlgynothAI : public EmeraldNightmareIlgynothCourtAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareEyeOfIlgynothAI(c); }
    explicit EmeraldNightmareEyeOfIlgynothAI(Creature* pCreature) : EmeraldNightmareIlgynothCourtAI(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void OnCombatStart(Unit* pTarget) override
    {
        showFrame(true);
        joinFight(tracked(BOSS_ILGYNOTH));
        EmeraldNightmareIlgynothCourtAI::OnCombatStart(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        showFrame(false);

        if (EmeraldNightmareInstanceScript* pNightmare = getNightmare())
            pNightmare->setEyeDown(true);

        tell(tracked(BOSS_ILGYNOTH), ACTION_ILGYNOTH_EYE_FELL);
    }

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_EYE_NIGHTMARE_GAZE, 2000);
        scriptEvents.addEvent(EVENT_EYE_NIGHTMARE_HORROR, 65000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_EYE_NIGHTMARE_GAZE:
                castOnRandomPlayer(SPELL_EYE_NIGHTMARE_GAZE, false);
                scriptEvents.addEvent(EVENT_EYE_NIGHTMARE_GAZE, 2000);
                break;
            case EVENT_EYE_NIGHTMARE_HORROR:
                tell(tracked(BOSS_ILGYNOTH), ACTION_ILGYNOTH_CALL_HORROR);
                scriptEvents.addEvent(EVENT_EYE_NIGHTMARE_HORROR, 220000);
                break;
            default:
                break;
        }
    }
};

// Dominator, Deathglare and Corruptor Tentacles
class EmeraldNightmareIlgynothTentacleAI : public EmeraldNightmareIlgynothCourtAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareIlgynothTentacleAI(c); }
    explicit EmeraldNightmareIlgynothTentacleAI(Creature* pCreature) : EmeraldNightmareIlgynothCourtAI(pCreature) {}

    void OnLoad() override { setRooted(true); }

    void OnCombatStart(Unit* pTarget) override
    {
        if (isDominator())
            showFrame(true);

        joinFight(tracked(BOSS_ILGYNOTH));
        EmeraldNightmareIlgynothCourtAI::OnCombatStart(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (isDominator())
            showFrame(false);

        bleedIchors(getCreature()->getEntry() == NPC_CORRUPTOR_TENTACLE ? 1 : 2);
    }

protected:
    void onEngage() override
    {
        switch (getCreature()->getEntry())
        {
            case NPC_DOMINATOR_TENTACLE:
                scriptEvents.addEvent(EVENT_DOMINATOR_FURY, 7000);
                scriptEvents.addEvent(EVENT_DOMINATOR_GROUND_SLAM, 13000);
                scriptEvents.addEvent(EVENT_DOMINATOR_REACH_CHECK, 4000);
                break;
            case NPC_DEATHGLARE_TENTACLE:
                scriptEvents.addEvent(EVENT_DEATHGLARE_MIND_FLAY, 2000);
                break;
            case NPC_CORRUPTOR_TENTACLE:
                scriptEvents.addEvent(EVENT_CORRUPTOR_SPEW, 2000);
                break;
            default:
                break;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DOMINATOR_FURY:
                castSpellOnSelf(SPELL_DOMINATOR_NIGHTMARISH_FURY);
                scriptEvents.addEvent(EVENT_DOMINATOR_FURY, 11000);
                break;
            case EVENT_DOMINATOR_GROUND_SLAM:
                castOnRandomPlayer(SPELL_DOMINATOR_GROUND_SLAM, false);
                scriptEvents.addEvent(EVENT_DOMINATOR_GROUND_SLAM, 22000);
                break;
            case EVENT_DOMINATOR_REACH_CHECK:
            {
                // roars when nobody stands in its reach
                Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
                if (pVictim == nullptr || getRangeToObject(pVictim) > 8.0f)
                    castSpellAOE(SPELL_DOMINATOR_RUPTURING_ROAR);

                scriptEvents.addEvent(EVENT_DOMINATOR_REACH_CHECK, 3000);
                break;
            }
            case EVENT_DEATHGLARE_MIND_FLAY:
                castOnRandomPlayer(SPELL_DEATHGLARE_MIND_FLAY, false);
                scriptEvents.addEvent(EVENT_DEATHGLARE_MIND_FLAY, Util::getRandomUInt(9000, 12000));
                break;
            case EVENT_CORRUPTOR_SPEW:
                castSpellOnSelf(SPELL_CORRUPTOR_SPEW_CORRUPTION, true);
                scriptEvents.addEvent(EVENT_CORRUPTOR_SPEW, 10000);
                break;
            default:
                break;
        }
    }

private:
    bool isDominator() { return getCreature()->getEntry() == NPC_DOMINATOR_TENTACLE; }
};

class EmeraldNightmareNightmareHorrorAI : public EmeraldNightmareIlgynothCourtAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareNightmareHorrorAI(c); }
    explicit EmeraldNightmareNightmareHorrorAI(Creature* pCreature) : EmeraldNightmareIlgynothCourtAI(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        mWakeIn = 5000;
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        showFrame(false);
        bleedIchors(4);
    }

protected:
    // rises out of reach and joins after a few seconds
    void onTick(uint32_t diff) override
    {
        if (!countDown(mWakeIn, diff))
            return;

        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);
        showFrame(true);
        setZoneWideCombat();
    }

    void onEngage() override { mEnergy = 0.0f; }

    void onCombatTick(uint32_t diff) override
    {
        if (mWakeIn != 0)
            return;

        mEnergy += gain(NightmareHorrorEnergyPerSecond, diff);
        if (mEnergy >= 100.0f && !_isCasting())
        {
            mEnergy = 0.0f;
            castSpellOnVictim(SPELL_NIGHTMARE_HORROR_EYE_OF_FATE);
        }
    }

private:
    uint32_t mWakeIn = 0;
    float mEnergy = 0.0f;
};

// fixates a player, bursts when slain, feeds the Heart when it reaches it
class EmeraldNightmareNightmareIchorAI : public EmeraldNightmareIlgynothCourtAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareNightmareIchorAI(c); }
    explicit EmeraldNightmareNightmareIchorAI(Creature* pCreature) : EmeraldNightmareIlgynothCourtAI(pCreature) {}

    void OnLoad() override { mCheckIn = 1000; }

    void DamageTaken(Unit* pAttacker, uint32_t* damage) override
    {
        // own blast passes, a lethal blow starts the blast instead
        if (damage == nullptr || pAttacker == getCreature())
            return;

        if (mBursting || mReabsorbed)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        *damage = health > 1 ? health - 1 : 0;
        startBurst();
    }

protected:
    void onTick(uint32_t diff) override
    {
        if (mBursting)
        {
            if (countDown(mBurstIn, diff))
                finishBurst();
            return;
        }

        if (mReabsorbed || !countDown(mCheckIn, diff))
            return;

        mCheckIn = 2000;

        if (!isEncounterInProgress(DATA_ILGYNOTH))
        {
            mReabsorbed = true;
            despawn(100);
            return;
        }

        Creature* pHeart = tracked(BOSS_ILGYNOTH);
        if (pHeart != nullptr && pHeart->isAlive() && getRangeToObject(pHeart) <= 20.0f)
        {
            reabsorb();
            return;
        }

        keepFixate();
    }

private:
    void keepFixate()
    {
        Unit* pQuarry = getCreature()->getWorldMapUnit(mQuarry);
        if (pQuarry != nullptr && pQuarry->isAlive())
            return;

        pQuarry = randomPlayer(false);
        if (pQuarry == nullptr)
            return;

        mQuarry = pQuarry->getGuid();
        castSpell(pQuarry, SPELL_ICHOR_FIXATE, true);
        fixateOn(pQuarry);
    }

    void startBurst()
    {
        mBursting = true;
        mBurstIn = 3000;
        attackStop();
        setRooted(true);
        castSpellAOE(SPELL_ICHOR_NIGHTMARE_EXPLOSION);
    }

    // the Eye takes a share of a blast close to it
    void finishBurst()
    {
        Creature* pEye = tracked(NPC_EYE_OF_ILGYNOTH);
        if (pEye != nullptr && pEye->isAlive() && getRangeToObject(pEye) <= IchorBlastRange)
            getCreature()->dealDamage(pEye, pEye->getMaxHealth() / 20, SPELL_ICHOR_NIGHTMARE_EXPLOSION_EYE);

        getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
    }

    void reabsorb()
    {
        mReabsorbed = true;
        attackStop();
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_ICHOR_REABSORPTION, true);
        despawn(1000);
    }

    bool mBursting = false;
    bool mReabsorbed = false;
    uint32_t mBurstIn = 0;
    uint32_t mCheckIn = 0;
    uint64_t mQuarry = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Elerethe Renferal

class EmeraldNightmareEleretheAI : public EmeraldNightmareBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareEleretheAI(c); }
    explicit EmeraldNightmareEleretheAI(Creature* pCreature) : EmeraldNightmareBossAI(pCreature, DATA_ELERETHE_RENFERAL) {}

protected:
    void onEngage() override
    {
        mEnergy = 0.0f;
        mDarkwing = false;
        castSpellOnSelf(SPELL_ELERETHE_SPIDER_TRANSFORM, true);
        startBerserk(EleretheBerserkMs);
        spiderKit();
    }

    // a full bar switches between spider and darkwing
    void onCombatTick(uint32_t diff) override
    {
        mEnergy += gain(EleretheEnergyPerSecond, diff);
        if (mEnergy >= 100.0f && !_isCasting())
            shiftForm();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ELERETHE_NECROTIC_VENOM:
                castOnPlayers(SPELL_ELERETHE_NECROTIC_VENOM, 2, true);
                scriptEvents.addEvent(EVENT_ELERETHE_NECROTIC_VENOM, 22000);
                break;
            case EVENT_ELERETHE_FEEDING_TIME:
                feedingTime();
                scriptEvents.addEvent(EVENT_ELERETHE_FEEDING_TIME, 50000);
                break;
            case EVENT_ELERETHE_VILE_AMBUSH:
                castOnRandomPlayer(SPELL_ELERETHE_VILE_AMBUSH, true, true);
                break;
            case EVENT_ELERETHE_TWISTING_SHADOWS:
                for (Unit* pPlayer : pickPlayers(EleretheShadowsPerCast, false))
                    summonCreature(NPC_TWISTING_SHADOWS, pPlayer->GetPosition(), MANUAL_DESPAWN);
                scriptEvents.addEvent(EVENT_ELERETHE_TWISTING_SHADOWS, 40000);
                break;
            case EVENT_ELERETHE_GATHERING_CLOUDS:
                raidWarning("Elerethe Renferal gathers the clouds!");
                castSpellAOE(SPELL_ELERETHE_GATHERING_CLOUDS);
                break;
            case EVENT_ELERETHE_DARK_STORM:
                raidWarning("A Dark Storm forms around Elerethe Renferal!");
                castSpellAOE(SPELL_ELERETHE_DARK_STORM);
                break;
            case EVENT_ELERETHE_RAKING_TALONS:
                castSpellOnVictim(SPELL_ELERETHE_RAKING_TALONS);
                scriptEvents.addEvent(EVENT_ELERETHE_RAKING_TALONS, 30000);
                break;
            case EVENT_ELERETHE_RAZOR_WING:
                raidWarning("Elerethe Renferal prepares a Razor Wing!");
                castOnRandomPlayer(SPELL_ELERETHE_RAZOR_WING, false);
                scriptEvents.addEvent(EVENT_ELERETHE_RAZOR_WING, 32000);
                break;
            default:
                break;
        }
    }

private:
    void spiderKit()
    {
        scriptEvents.addEvent(EVENT_ELERETHE_NECROTIC_VENOM, 12000);
        scriptEvents.addEvent(EVENT_ELERETHE_FEEDING_TIME, 16000);
    }

    void darkwingKit()
    {
        scriptEvents.addEvent(EVENT_ELERETHE_TWISTING_SHADOWS, 7000);
        scriptEvents.addEvent(EVENT_ELERETHE_GATHERING_CLOUDS, 16000);
        scriptEvents.addEvent(EVENT_ELERETHE_DARK_STORM, 26000);
        scriptEvents.addEvent(EVENT_ELERETHE_RAKING_TALONS, 33000);
        scriptEvents.addEvent(EVENT_ELERETHE_RAZOR_WING, 59000);
    }

    void shiftForm()
    {
        mEnergy = 0.0f;
        mDarkwing = !mDarkwing;
        scriptEvents.resetEvents();

        if (mDarkwing)
        {
            raidWarning("Elerethe Renferal takes her Darkwing Form!");
            _removeAura(SPELL_ELERETHE_SPIDER_TRANSFORM);
            castSpellOnSelf(SPELL_ELERETHE_BIRD_TRANSFORM, true);
            castSpellOnSelf(SPELL_ELERETHE_DARKWING_FORM);
            darkwingKit();
            return;
        }

        raidWarning("Elerethe Renferal returns to her Arachnid Form!");
        summons.despawnEntry(NPC_TWISTING_SHADOWS);
        _removeAura(SPELL_ELERETHE_BIRD_TRANSFORM);
        castSpellOnSelf(SPELL_ELERETHE_SPIDER_TRANSFORM, true);
        castSpellOnSelf(SPELL_ELERETHE_ARACHNID_FORM);
        spiderKit();
    }

    void feedingTime()
    {
        raidWarning("Elerethe Renferal calls her brood: Feeding Time!");
        castSpellOnSelf(SPELL_ELERETHE_FEEDING_TIME, true);

        for (uint32_t i = 0; i < EleretheSpiderlingsPerFeeding; ++i)
            callAdd(NPC_VENOMOUS_SPIDERLING, ringPoint(getCreature()->GetPosition(), 12.0f, static_cast<float>(i) * NightmareTwoPi / static_cast<float>(EleretheSpiderlingsPerFeeding)));

        scriptEvents.addEvent(EVENT_ELERETHE_VILE_AMBUSH, 4000);
    }

    float mEnergy = 0.0f;
    bool mDarkwing = false;
};

class EmeraldNightmareTwistingShadowsAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareTwistingShadowsAI(c); }
    explicit EmeraldNightmareTwistingShadowsAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_TWISTING_SHADOWS_AREA, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Cenarius

class EmeraldNightmareCenariusAI : public EmeraldNightmareBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareCenariusAI(c); }
    explicit EmeraldNightmareCenariusAI(Creature* pCreature) : EmeraldNightmareBossAI(pCreature, DATA_CENARIUS) {}

protected:
    void onEngage() override
    {
        mEnergy = 90.0f;
        mWaves = 0;
        mArrivingWave = 0;
        mOverwhelmed = false;

        castSpellOnSelf(SPELL_CENARIUS_CREEPING_NIGHTMARES, true);
        scriptEvents.addEvent(EVENT_CENARIUS_NIGHTMARE_BRAMBLES, 25000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mOverwhelmed && _getHealthPercent() < 35)
        {
            enterOverwhelmed();
            return;
        }

        // six waves at most, none once overwhelmed
        if (mOverwhelmed || mWaves >= CenariusWaveCount)
            return;

        mEnergy += gain(CenariusEnergyPerSecond, diff);
        if (mEnergy >= 100.0f && !_isCasting())
            callForces();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CENARIUS_NIGHTMARE_BRAMBLES:
                if (Unit* pTarget = randomPlayer(true))
                {
                    raidWarning("Nightmare Brambles burst from the ground!");
                    summonCreature(NPC_NIGHTMARE_BRAMBLES, pTarget->GetPosition(), MANUAL_DESPAWN);
                }
                scriptEvents.addEvent(EVENT_CENARIUS_NIGHTMARE_BRAMBLES, 30000);
                break;
            case EVENT_CENARIUS_FORCES_ARRIVE:
                raiseForces(mArrivingWave);
                break;
            case EVENT_CENARIUS_BLAST_SAVER:
                castSpellOnSelf(SPELL_CENARIUS_NIGHTMARE_BLAST_SAVER, true);
                break;
            case EVENT_CENARIUS_SPEAR:
                castSpellOnVictim(SPELL_CENARIUS_SPEAR_OF_NIGHTMARES);
                scriptEvents.addEvent(EVENT_CENARIUS_SPEAR, 18000);
                break;
            default:
                break;
        }
    }

private:
    void callForces()
    {
        mEnergy = 0.0f;
        mArrivingWave = mWaves++;

        raidWarning("Cenarius calls the Forces of Nightmare!");
        castSpellOnSelf(SPELL_CENARIUS_FORCES_OF_NIGHTMARE);
        castSpellOnSelf(SPELL_CENARIUS_AURA_OF_DREAD_THORNS, true);
        scriptEvents.addEvent(EVENT_CENARIUS_FORCES_ARRIVE, 10000);
    }

    void raiseForces(uint32_t wave)
    {
        if (wave >= CenariusWaveCount)
            return;

        for (CenariusForce const& force : CenariusWaves[wave])
        {
            if (force.entry == NPC_CORRUPTED_WISP)
            {
                LocationVector const gathering = ringPoint(CenariusGroveCentre, 30.0f, Util::getRandomFloat(NightmareTwoPi));
                for (uint32_t i = 0; i < CenariusWispsPerPack; ++i)
                    callAdd(NPC_CORRUPTED_WISP, scatterAround(gathering, 0.0f, 3.0f));
                continue;
            }

            callAdd(force.entry, CenariusForceSpots[force.spot]);
        }
    }

    void enterOverwhelmed()
    {
        mOverwhelmed = true;
        raidWarning("Cenarius is consumed by an Overwhelming Nightmare!");
        castSpellOnSelf(SPELL_CENARIUS_OVERWHELMING_NIGHTMARE, true);
        scriptEvents.addEvent(EVENT_CENARIUS_BLAST_SAVER, 4000);
        scriptEvents.addEvent(EVENT_CENARIUS_SPEAR, 21000);
    }

    float mEnergy = 90.0f;
    uint32_t mWaves = 0;
    uint32_t mArrivingWave = 0;
    bool mOverwhelmed = false;
};

// Nightmare Ancient, Rotten Drake and Twisted Sister: energy driven specials
class EmeraldNightmareNightmareForceAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareNightmareForceAI(c); }
    explicit EmeraldNightmareNightmareForceAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

    void OnLoad() override
    {
        switch (getCreature()->getEntry())
        {
            case NPC_NIGHTMARE_ANCIENT:
                mEnergy = 50.0f;
                mEnergyPerSecond = 3.33f;
                break;
            case NPC_ROTTEN_DRAKE:
                mEnergy = 30.0f;
                mEnergyPerSecond = 4.0f;
                break;
            default:
                mEnergy = 80.0f;
                mEnergyPerSecond = 5.0f;
                break;
        }
    }

    void OnDied(Unit* /*pKiller*/) override { showFrame(false); }

protected:
    void onEngage() override
    {
        showFrame(true);
        mBuffeted = false;

        if (getCreature()->getEntry() == NPC_TWISTED_SISTER)
            scriptEvents.addEvent(EVENT_SISTER_JAVELIN, 3000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mBuffeted && getCreature()->getEntry() == NPC_ROTTEN_DRAKE && _getHealthPercent() < 46)
        {
            mBuffeted = true;
            castSpellOnSelf(SPELL_DRAKE_NIGHTMARE_BUFFET, true);
        }

        mEnergy += gain(mEnergyPerSecond, diff);
        if (mEnergy < 100.0f || _isCasting())
            return;

        mEnergy = 0.0f;
        switch (getCreature()->getEntry())
        {
            case NPC_NIGHTMARE_ANCIENT:
                castSpellAOE(SPELL_ANCIENT_DESICCATING_STOMP);
                break;
            case NPC_ROTTEN_DRAKE:
                castOnRandomPlayer(SPELL_DRAKE_ROTTEN_BREATH, false);
                break;
            default:
                castOnRandomPlayer(SPELL_SISTER_SCORNED_TOUCH, false);
                break;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SISTER_JAVELIN)
            return;

        castOnRandomPlayer(SPELL_SISTER_NIGHTMARE_JAVELIN, false);
        scriptEvents.addEvent(EVENT_SISTER_JAVELIN, Util::getRandomUInt(5000, 8000));
    }

private:
    float mEnergy = 0.0f;
    float mEnergyPerSecond = 0.0f;
    bool mBuffeted = false;
};

// hunts one player after another and hurts them on contact
class EmeraldNightmareCorruptedWispAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareCorruptedWispAI(c); }
    explicit EmeraldNightmareCorruptedWispAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

    void OnLoad() override
    {
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_WISP_DESTRUCTIVE_NIGHTMARES, true);
        mStepIn = 1000;
    }

protected:
    void onTick(uint32_t diff) override
    {
        if (!countDown(mStepIn, diff))
            return;

        mStepIn = 1000;

        Unit* pQuarry = getCreature()->getWorldMapUnit(mQuarry);
        if (pQuarry == nullptr || !pQuarry->isAlive())
        {
            chooseQuarry(0);
            return;
        }

        if (getRangeToObject(pQuarry) <= 3.0f)
        {
            castSpell(pQuarry, SPELL_WISP_DESTRUCTIVE_NIGHTMARES_HIT, true);
            chooseQuarry(mQuarry);
        }
    }

private:
    void chooseQuarry(uint64_t previous)
    {
        Unit* pChosen = nullptr;
        for (Unit* pPlayer : pickPlayers(NightmareRaidCap, true))
        {
            pChosen = pPlayer;
            if (pPlayer->getGuid() != previous)
                break;
        }

        if (pChosen == nullptr)
            return;

        mQuarry = pChosen->getGuid();
        fixateOn(pChosen);
    }

    uint32_t mStepIn = 0;
    uint64_t mQuarry = 0;
};

// rolls after a player, harming everyone in its way
class EmeraldNightmareNightmareBramblesAI : public EmeraldNightmareCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EmeraldNightmareNightmareBramblesAI(c); }
    explicit EmeraldNightmareNightmareBramblesAI(Creature* pCreature) : EmeraldNightmareCombatAI(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        mArmIn = 1000;
    }

protected:
    void onTick(uint32_t diff) override
    {
        if (countDown(mArmIn, diff))
        {
            castSpellOnSelf(SPELL_BRAMBLES_AREA, true);
            castSpellOnSelf(SPELL_BRAMBLES_PERIODIC, true);
            mStepIn = 1000;
            return;
        }

        if (!countDown(mStepIn, diff))
            return;

        mStepIn = 1000;
        if (!isEncounterInProgress(DATA_CENARIUS))
            return;

        // first the player it sprouted under, then anyone
        Unit* pQuarry = getCreature()->getWorldMapUnit(mQuarry);
        if (pQuarry == nullptr || !pQuarry->isAlive())
        {
            pQuarry = mQuarry == 0 ? nearestPlayer(10.0f) : nullptr;
            if (pQuarry == nullptr)
                pQuarry = randomPlayer(false);
            if (pQuarry == nullptr)
                return;

            mQuarry = pQuarry->getGuid();
        }

        moveTo(pQuarry->GetPositionX(), pQuarry->GetPositionY(), pQuarry->GetPositionZ(), false);
    }

private:
    uint32_t mArmIn = 0;
    uint32_t mStepIn = 0;
    uint64_t mQuarry = 0;
};

void SetupTheEmeraldNightmare(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_EMERALD_NIGHTMARE, &EmeraldNightmareInstanceScript::Create);

    // Ursoc
    mgr->register_creature_script(BOSS_URSOC, &EmeraldNightmareUrsocAI::Create);

    // Nythendra
    mgr->register_creature_script(BOSS_NYTHENDRA, &EmeraldNightmareNythendraAI::Create);
    mgr->register_creature_script(NPC_CORRUPTED_VERMIN, &EmeraldNightmareCorruptedVerminAI::Create);

    // Dragons of Nightmare
    mgr->register_creature_script(BOSS_YSONDRE, &EmeraldNightmareYsondreAI::Create);
    mgr->register_creature_script(BOSS_TAERAR, &EmeraldNightmareDragonAI::Create);
    mgr->register_creature_script(BOSS_LETHON, &EmeraldNightmareDragonAI::Create);
    mgr->register_creature_script(BOSS_EMERISS, &EmeraldNightmareDragonAI::Create);
    mgr->register_creature_script(NPC_DEFILED_DRUID_SPIRIT, &EmeraldNightmareDefiledSpiritAI::Create);
    mgr->register_creature_script(NPC_SPIRIT_SHADE, &EmeraldNightmareSpiritShadeAI::Create);
    mgr->register_creature_script(NPC_SHADE_OF_TAERAR, &EmeraldNightmareShadeOfTaerarAI::Create);
    mgr->register_creature_script(NPC_ESSENCE_OF_CORRUPTION, &EmeraldNightmareEssenceOfCorruptionAI::Create);

    // Xavius
    mgr->register_creature_script(BOSS_XAVIUS, &EmeraldNightmareXaviusAI::Create);
    mgr->register_creature_script(NPC_LURKING_TERROR, &EmeraldNightmareLurkingTerrorAI::Create);
    mgr->register_creature_script(NPC_CORRUPTION_HORROR, &EmeraldNightmareCorruptionHorrorAI::Create);
    mgr->register_creature_script(NPC_INCONCEIVABLE_HORROR, &EmeraldNightmareInconceivableHorrorAI::Create);

    // Il'gynoth
    mgr->register_creature_script(BOSS_ILGYNOTH, &EmeraldNightmareIlgynothAI::Create);
    mgr->register_creature_script(NPC_EYE_OF_ILGYNOTH, &EmeraldNightmareEyeOfIlgynothAI::Create);
    mgr->register_creature_script(NPC_DOMINATOR_TENTACLE, &EmeraldNightmareIlgynothTentacleAI::Create);
    mgr->register_creature_script(NPC_DEATHGLARE_TENTACLE, &EmeraldNightmareIlgynothTentacleAI::Create);
    mgr->register_creature_script(NPC_CORRUPTOR_TENTACLE, &EmeraldNightmareIlgynothTentacleAI::Create);
    mgr->register_creature_script(NPC_NIGHTMARE_HORROR, &EmeraldNightmareNightmareHorrorAI::Create);
    mgr->register_creature_script(NPC_NIGHTMARE_ICHOR, &EmeraldNightmareNightmareIchorAI::Create);

    // Elerethe Renferal
    mgr->register_creature_script(BOSS_ELERETHE_RENFERAL, &EmeraldNightmareEleretheAI::Create);
    mgr->register_creature_script(NPC_TWISTING_SHADOWS, &EmeraldNightmareTwistingShadowsAI::Create);

    // Cenarius
    mgr->register_creature_script(BOSS_CENARIUS, &EmeraldNightmareCenariusAI::Create);
    mgr->register_creature_script(NPC_NIGHTMARE_ANCIENT, &EmeraldNightmareNightmareForceAI::Create);
    mgr->register_creature_script(NPC_ROTTEN_DRAKE, &EmeraldNightmareNightmareForceAI::Create);
    mgr->register_creature_script(NPC_TWISTED_SISTER, &EmeraldNightmareNightmareForceAI::Create);
    mgr->register_creature_script(NPC_CORRUPTED_WISP, &EmeraldNightmareCorruptedWispAI::Create);
    mgr->register_creature_script(NPC_NIGHTMARE_BRAMBLES, &EmeraldNightmareNightmareBramblesAI::Create);
}
