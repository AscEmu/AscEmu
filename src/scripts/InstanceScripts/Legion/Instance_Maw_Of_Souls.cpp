/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Maw_Of_Souls.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

enum MawOfSoulsEvents
{
    EVENT_YMIRON_DARK_SLASH = 1,
    EVENT_YMIRON_SCREAMS_OF_THE_DEAD,
    EVENT_YMIRON_WINDS_OF_NORTHREND,
    EVENT_YMIRON_BANE,

    EVENT_HARBARON_COSMIC_SCYTHE,
    EVENT_HARBARON_SHACKLED_SERVITOR,
    EVENT_HARBARON_FRAGMENT,
    EVENT_HARBARON_NETHER_RIP,

    EVENT_SERVITOR_VOID_SNAP,

    EVENT_HELYA_TAINT_OF_THE_SEA,
    EVENT_HELYA_TORRENT,
    EVENT_HELYA_BRACKWATER_BARRAGE,
    EVENT_HELYA_DESTRUCTOR,
    EVENT_HELYA_GRASPING_RETURN,
    EVENT_HELYA_PIERCING_TENTACLE,
    EVENT_HELYA_SURFACE,
    EVENT_HELYA_WING_BUFFET,
    EVENT_HELYA_DECK_TORRENT,
    EVENT_HELYA_CORRUPTED_BELLOW,
    EVENT_HELYA_DIVE,
    EVENT_HELYA_RESURFACE,

    EVENT_DESTRUCTOR_RAPID_RUPTURE,
    EVENT_DESTRUCTOR_SMASH_CHECK
};

namespace
{
    enum class MawDoorKind : uint8_t
    {
        Room,
        Passage,
        SkjalGate
    };

    struct MawDoorRule
    {
        uint32_t entry;
        uint32_t encounterId;
        MawDoorKind kind;
    };

    MawDoorRule const MawDoorRules[] =
    {
        { GO_HARBARON_WALL_OF_SOULS,    DATA_HARBARON,  MawDoorKind::Passage },
        { GO_HARBARON_BARNACLED_DOOR,   DATA_HARBARON,  MawDoorKind::Room },
        { GO_SKJAL_WALL_OF_SOULS,       DATA_HARBARON,  MawDoorKind::SkjalGate },
        { GO_SKJAL_MURKY_FOG_1,         DATA_HARBARON,  MawDoorKind::SkjalGate },
        { GO_SKJAL_MURKY_FOG_2,         DATA_HARBARON,  MawDoorKind::SkjalGate }
    };

    MawDoorRule const* findMawDoor(uint32_t entry)
    {
        for (MawDoorRule const& rule : MawDoorRules)
        {
            if (rule.entry == entry)
                return &rule;
        }

        return nullptr;
    }

    float const MawTwoPi = 6.28318531f;

    uint32_t const YmironPowerCap = 100;
    uint32_t const YmironPowerOnPull = 80;

    uint32_t const HarbaronScytheBlades = 6;
    float const HarbaronScytheSpacing = 5.0f;

    // trap doors on the Naglfar deck, index 0 is the stern pool
    LocationVector const HelyaPools[] =
    {
        { 2933.33f, 960.27f, 510.39f, 1.57f },
        { 2926.73f, 952.15f, 510.39f, 1.57f },
        { 2939.91f, 952.15f, 510.39f, 1.57f },
        { 2933.40f, 941.15f, 510.39f, 1.57f },
        { 2922.20f, 937.07f, 510.39f, 1.57f },
        { 2943.52f, 935.80f, 510.39f, 1.57f },
        { 2927.44f, 928.84f, 510.39f, 1.57f },
        { 2939.53f, 928.46f, 510.39f, 1.57f },
        { 2918.25f, 921.30f, 510.39f, 1.57f },
        { 2933.18f, 920.70f, 510.39f, 1.57f },
        { 2948.52f, 921.45f, 510.39f, 1.57f }
    };
    uint32_t const HelyaPoolCount = 11;

    struct HelyaGraspingSlot
    {
        uint32_t entry;
        LocationVector spot;
    };

    // slots 0-2 on the left rail, 3-5 on the right rail
    HelyaGraspingSlot const HelyaGraspingSlots[] =
    {
        { NPC_GRASPING_TENTACLE_LEFT_2,  { 2917.30f, 950.65f, 512.26f, 5.73f } },
        { NPC_GRASPING_TENTACLE_LEFT_1,  { 2921.71f, 959.25f, 513.21f, 5.85f } },
        { NPC_GRASPING_TENTACLE_LEFT_2,  { 2925.54f, 965.84f, 512.26f, 5.65f } },
        { NPC_GRASPING_TENTACLE_RIGHT_1, { 2941.37f, 966.34f, 513.79f, 4.08f } },
        { NPC_GRASPING_TENTACLE_RIGHT_2, { 2945.93f, 958.59f, 513.23f, 3.58f } },
        { NPC_GRASPING_TENTACLE_RIGHT_1, { 2948.58f, 952.28f, 513.47f, 3.41f } }
    };
    uint32_t const HelyaGraspingSlotCount = 6;

    LocationVector const HelyaDeckSpot(2933.5f, 961.79f, 512.38f, 4.71f);
    uint32_t const HelyaTentaclesToBreak = 6;
    uint32_t const HelyaRetreatPct = 70;

    uint32_t const HelyaTentacleEntries[] =
    {
        NPC_DESTRUCTOR_TENTACLE,
        NPC_GRASPING_TENTACLE_LEFT_1,
        NPC_GRASPING_TENTACLE_LEFT_2,
        NPC_GRASPING_TENTACLE_RIGHT_1,
        NPC_GRASPING_TENTACLE_RIGHT_2
    };

    bool isHelyaTentacle(uint32_t entry)
    {
        for (uint32_t tentacle : HelyaTentacleEntries)
        {
            if (tentacle == entry)
                return true;
        }

        return false;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class MawOfSoulsInstanceScript : public InstanceScript
{
public:
    explicit MawOfSoulsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(MawOfSoulsEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new MawOfSoulsInstanceScript(pMapMgr); }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        switch (pCreature->getEntry())
        {
            case BOSS_HELYA:
                mHelyaGuid = pCreature->GetNewGUID();
                break;
            case NPC_SKJAL:
                mSkjalGuid = pCreature->GetNewGUID();
                alignDoors();
                break;
            default:
                break;
        }
    }

    void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
    {
        if (pVictim->getEntry() != NPC_SKJAL)
            return;

        mSkjalSlain = true;
        saveToDB();
        alignDoors();
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findMawDoor(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        alignDoor(pGameObject);
    }

    void OnEncounterStateChange(uint32_t /*id*/, uint32_t /*state*/) override { alignDoors(); }

    void writeSaveDataExtended(std::ostringstream& data) override { data << (mSkjalSlain ? 1 : 0) << ' '; }

    void readSaveDataExtended(std::istringstream& data) override
    {
        uint32_t slain = 0;
        if (data >> slain)
            mSkjalSlain = slain != 0;
    }

    Creature* getHelya() { return getCreatureByGuid(mHelyaGuid); }

private:
    void alignDoors()
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor);
        }
    }

    // without a Skjal spawn the gate follows Harbaron
    bool isSkjalGateOpen()
    {
        if (mSkjalSlain)
            return true;

        Creature* pSkjal = getCreatureByGuid(mSkjalGuid);
        if (pSkjal != nullptr && pSkjal->isAlive())
            return false;

        return getBossState(DATA_HARBARON) == EncounterStates::Performed;
    }

    void alignDoor(GameObject* pDoor)
    {
        MawDoorRule const* rule = findMawDoor(pDoor->getEntry());
        if (rule == nullptr)
            return;

        EncounterStates const state = getBossState(rule->encounterId);
        bool wantOpen = false;
        switch (rule->kind)
        {
            case MawDoorKind::Room:
                wantOpen = state != EncounterStates::InProgress;
                break;
            case MawDoorKind::Passage:
                wantOpen = state == EncounterStates::Performed;
                break;
            case MawDoorKind::SkjalGate:
                wantOpen = isSkjalGateOpen();
                break;
        }

        bool const isOpen = pDoor->getState() != GO_STATE_CLOSED;
        if (wantOpen != isOpen)
            pDoor->setState(wantOpen ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    WoWGuid mHelyaGuid;
    WoWGuid mSkjalGuid;
    std::vector<WoWGuid> mDoorGuids;
    bool mSkjalSlain = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event clock while in combat

class MawOfSoulsCombatAI : public CreatureAIScript
{
public:
    explicit MawOfSoulsCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

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

    MawOfSoulsInstanceScript* getMawOfSouls()
    {
        if (getCreature()->GetMapId() != MAP_MAW_OF_SOULS)
            return nullptr;

        return static_cast<MawOfSoulsInstanceScript*>(getInstanceScript());
    }

    Unit* anyPlayer() { return getBestPlayerTarget(TargetFilter_None); }

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

    // random point on a circle around a position
    static LocationVector aroundSpot(LocationVector const& origin, float radius)
    {
        float const angle = Util::getRandomFloat(MawTwoPi);
        return LocationVector(origin.x + radius * std::cos(angle), origin.y + radius * std::sin(angle), origin.z, angle);
    }

    void warn(std::string const& text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class MawOfSoulsBossAI : public MawOfSoulsCombatAI
{
public:
    MawOfSoulsBossAI(Creature* pCreature, uint32_t encounterId) : MawOfSoulsCombatAI(pCreature), mEncounterId(encounterId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mEncounterId, EncounterStates::InProgress);
        }

        MawOfSoulsCombatAI::OnCombatStart(pTarget);
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

        MawOfSoulsCombatAI::OnCombatStop(pTarget);
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
// Ymiron, the Fallen King: power fills each second, a full bar ends in Dark Slash

class MawOfSoulsYmironAI : public MawOfSoulsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MawOfSoulsYmironAI(c); }
    explicit MawOfSoulsYmironAI(Creature* pCreature) : MawOfSoulsBossAI(pCreature, DATA_YMIRON) {}

protected:
    void onEngage() override
    {
        mPower = YmironPowerOnPull;
        mPowerClock = 0;
        mPowerTicks = 0;
        scriptEvents.addEvent(EVENT_YMIRON_SCREAMS_OF_THE_DEAD, 6000);
        scriptEvents.addEvent(EVENT_YMIRON_WINDS_OF_NORTHREND, 16000);
        scriptEvents.addEvent(EVENT_YMIRON_BANE, 22000);
    }

    void onCombatTick(uint32_t diff) override
    {
        mPowerClock += diff;
        while (mPowerClock >= 1000)
        {
            mPowerClock -= 1000;
            if (mPower >= YmironPowerCap)
            {
                mPower = 0;
                scriptEvents.addEvent(EVENT_YMIRON_DARK_SLASH, 100);
                continue;
            }

            // two steps of seven, then one of six
            mPower += (mPowerTicks % 3 == 2) ? 6 : 7;
            ++mPowerTicks;
            if (mPower > YmironPowerCap)
                mPower = YmironPowerCap;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_YMIRON_DARK_SLASH:
                castSpellOnVictim(SPELL_YMIRON_DARK_SLASH);
                break;
            case EVENT_YMIRON_SCREAMS_OF_THE_DEAD:
                castSpellAOE(SPELL_YMIRON_SCREAMS_OF_THE_DEAD);
                scriptEvents.addEvent(EVENT_YMIRON_SCREAMS_OF_THE_DEAD, 23000);
                break;
            case EVENT_YMIRON_WINDS_OF_NORTHREND:
                castSpellAOE(SPELL_YMIRON_WINDS_OF_NORTHREND);
                scriptEvents.addEvent(EVENT_YMIRON_WINDS_OF_NORTHREND, 24000);
                break;
            case EVENT_YMIRON_BANE:
                warn("Ymiron calls forth the essences of Bane!");
                castSpellAOE(SPELL_YMIRON_BANE);
                scriptEvents.addEvent(EVENT_YMIRON_BANE, 50000);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mPower = 0;
    uint32_t mPowerClock = 0;
    uint32_t mPowerTicks = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Harbaron

class MawOfSoulsHarbaronAI : public MawOfSoulsBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MawOfSoulsHarbaronAI(c); }
    explicit MawOfSoulsHarbaronAI(Creature* pCreature) : MawOfSoulsBossAI(pCreature, DATA_HARBARON) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_HARBARON_COSMIC_SCYTHE, 4000);
        scriptEvents.addEvent(EVENT_HARBARON_SHACKLED_SERVITOR, 7000);
        scriptEvents.addEvent(EVENT_HARBARON_FRAGMENT, 18000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_HARBARON_NETHER_RIP, 13000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HARBARON_COSMIC_SCYTHE:
                if (Unit* pTarget = anyPlayer())
                {
                    castSpell(pTarget, SPELL_HARBARON_COSMIC_SCYTHE);
                    layScythes(pTarget);
                }
                scriptEvents.addEvent(EVENT_HARBARON_COSMIC_SCYTHE, 8000);
                break;
            case EVENT_HARBARON_SHACKLED_SERVITOR:
                warn("Harbaron summons a Shackled Servitor!");
                raiseAdd(NPC_SHACKLED_SERVITOR, aroundSpot(getCreature()->GetPosition(), 15.0f));
                scriptEvents.addEvent(EVENT_HARBARON_SHACKLED_SERVITOR, 23000);
                break;
            case EVENT_HARBARON_FRAGMENT:
                if (Unit* pTarget = anyPlayer())
                {
                    warn("Harbaron tears a soul into Fragments!");
                    castSpell(pTarget, SPELL_HARBARON_FRAGMENT);
                    splitSoul(pTarget);
                }
                scriptEvents.addEvent(EVENT_HARBARON_FRAGMENT, 30000);
                break;
            case EVENT_HARBARON_NETHER_RIP:
                castSpellOnSelf(SPELL_HARBARON_NETHER_RIP, true);
                scriptEvents.addEvent(EVENT_HARBARON_NETHER_RIP, 13000);
                break;
            default:
                break;
        }
    }

private:
    // blades line up from Harbaron towards the target
    void layScythes(Unit* pTarget)
    {
        LocationVector const origin = getCreature()->GetPosition();
        float const angle = getCreature()->getAbsoluteAngle(pTarget);

        for (uint32_t i = 1; i <= HarbaronScytheBlades; ++i)
        {
            float const range = HarbaronScytheSpacing * static_cast<float>(i);
            LocationVector const spot(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
            summonCreature(NPC_COSMIC_SCYTHE, spot, TIMED_DESPAWN, 4000);
        }
    }

    // two pieces of the soul flank the victim
    void splitSoul(Unit* pTarget)
    {
        LocationVector const centre = pTarget->GetPosition();
        float const angle = Util::getRandomFloat(MawTwoPi);

        for (uint32_t i = 0; i < 2; ++i)
        {
            float const side = angle + static_cast<float>(i) * (MawTwoPi / 2.0f);
            LocationVector const spot(centre.x + 1.5f * std::cos(side), centre.y + 1.5f * std::sin(side), centre.z, side);
            summonCreature(NPC_SOUL_FRAGMENT, spot, CORPSE_DESPAWN);
        }
    }
};

// glows briefly, then strikes its own spot
class MawOfSoulsCosmicScytheAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MawOfSoulsCosmicScytheAI(c); }
    explicit MawOfSoulsCosmicScytheAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_SCYTHE_GLOW, true);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mStrikeClock == 0)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mStrikeClock > diff)
        {
            mStrikeClock -= diff;
            return;
        }

        mStrikeClock = 0;
        getCreature()->castSpellLoc(getCreature()->GetPosition(), SPELL_SCYTHE_STRIKE, true);
        despawn(500, 0);
    }

private:
    uint32_t mStrikeClock = 2000;
};

class MawOfSoulsShackledServitorAI : public MawOfSoulsCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MawOfSoulsShackledServitorAI(c); }
    explicit MawOfSoulsShackledServitorAI(Creature* pCreature) : MawOfSoulsCombatAI(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        castSpellOnSelf(SPELL_SERVITOR_SHACKLED, true);
    }

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_SERVITOR_VOID_SNAP, 2000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SERVITOR_VOID_SNAP)
            return;

        castSpellAOE(SPELL_SERVITOR_VOID_SNAP);
        scriptEvents.addEvent(EVENT_SERVITOR_VOID_SNAP, 6000);
    }
};

class MawOfSoulsSoulFragmentAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MawOfSoulsSoulFragmentAI(c); }
    explicit MawOfSoulsSoulFragmentAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override { setReactState(REACT_PASSIVE); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Helya: tentacles first, then the deck of the Naglfar

class MawOfSoulsHelyaAI : public MawOfSoulsBossAI
{
    enum class HelyaStage : uint8_t
    {
        Tentacles,
        Submerged,
        Deck,
        DeckSubmerged
    };

public:
    static CreatureAIScript* Create(Creature* c) { return new MawOfSoulsHelyaAI(c); }
    explicit MawOfSoulsHelyaAI(Creature* pCreature) : MawOfSoulsBossAI(pCreature, DATA_HELYA) {}

    void OnLoad() override
    {
        lurkBelowDeck();

        InstanceScript* pInstance = getInstanceScript();
        if (isAlive() && (pInstance == nullptr || pInstance->getBossState(DATA_HELYA) != EncounterStates::Performed))
            raiseFirstTentacles();
    }

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || !_isInCombat() || mStage != HelyaStage::Tentacles || !isHelyaTentacle(pSummon->getEntry()))
            return;

        loseTentacle(pSummon);
    }

protected:
    void onEngage() override
    {
        mStage = HelyaStage::Tentacles;
        mTentaclesLost = 0;
        mHealthBites = 0;
        mGraspingPending = 0;
        mRetreated = false;
        rouseTentacles();

        scriptEvents.addEvent(EVENT_HELYA_TAINT_OF_THE_SEA, 500);
        scriptEvents.addEvent(EVENT_HELYA_DESTRUCTOR, 26000);
        scriptEvents.addEvent(EVENT_HELYA_PIERCING_TENTACLE, 8000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_HELYA_BRACKWATER_BARRAGE, 15000);
        else
            scriptEvents.addEvent(EVENT_HELYA_TORRENT, 3000);
    }

    void onReset() override
    {
        mStage = HelyaStage::Tentacles;
        MoveTeleport(getCreature()->GetSpawnPosition());
        lurkBelowDeck();
        raiseFirstTentacles();
    }

    // she abandons the fight once wounded enough on deck
    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mRetreated || mStage != HelyaStage::Deck || _getHealthPercent() > static_cast<int32_t>(HelyaRetreatPct))
            return;

        mRetreated = true;
        warn("Helya retreats into the depths of Helheim!");
        getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HELYA_TAINT_OF_THE_SEA:
                castAt(anyPlayer(), SPELL_HELYA_TAINT_OF_THE_SEA);
                scriptEvents.addEvent(EVENT_HELYA_TAINT_OF_THE_SEA, 12000);
                break;
            case EVENT_HELYA_TORRENT:
                castAt(anyPlayer(), SPELL_HELYA_TORRENT);
                scriptEvents.addEvent(EVENT_HELYA_TORRENT, 5000);
                break;
            case EVENT_HELYA_BRACKWATER_BARRAGE:
                warn("Helya's tentacles unleash a Brackwater Barrage!");
                castSpellAOE(SPELL_HELYA_BRACKWATER_BARRAGE);
                scriptEvents.addEvent(EVENT_HELYA_BRACKWATER_BARRAGE, 22000);
                break;
            case EVENT_HELYA_DESTRUCTOR:
                raiseDestructor();
                scriptEvents.addEvent(EVENT_HELYA_DESTRUCTOR, 26000);
                break;
            case EVENT_HELYA_GRASPING_RETURN:
                while (mGraspingPending > 0)
                {
                    --mGraspingPending;
                    raiseGrasping(true);
                }
                break;
            case EVENT_HELYA_PIERCING_TENTACLE:
                summonCreature(NPC_PIERCING_TENTACLE, HelyaPools[1 + Util::getRandomUInt(HelyaPoolCount - 2)], TIMED_DESPAWN, 6000);
                if (mStage == HelyaStage::Tentacles)
                    scriptEvents.addEvent(EVENT_HELYA_PIERCING_TENTACLE, 8000);
                else if (mStage != HelyaStage::Deck)
                    scriptEvents.addEvent(EVENT_HELYA_PIERCING_TENTACLE, 2000);
                break;
            case EVENT_HELYA_SURFACE:
                climbOnDeck();
                break;
            case EVENT_HELYA_WING_BUFFET:
                // punishes an empty melee range
                if (getBestPlayerTarget(TargetFilter_InMeleeRange) == nullptr)
                    castSpellAOE(SPELL_HELYA_WING_BUFFET);
                scriptEvents.addEvent(EVENT_HELYA_WING_BUFFET, 2000);
                break;
            case EVENT_HELYA_DECK_TORRENT:
                castAt(anyPlayer(), SPELL_HELYA_DECK_TORRENT);
                scriptEvents.addEvent(EVENT_HELYA_DECK_TORRENT, 8000);
                break;
            case EVENT_HELYA_CORRUPTED_BELLOW:
                warn("Helya unleashes a Corrupted Bellow!");
                castSpellAOE(SPELL_HELYA_CORRUPTED_BELLOW);
                scriptEvents.addEvent(EVENT_HELYA_CORRUPTED_BELLOW, 21000);
                break;
            case EVENT_HELYA_DIVE:
                dive(HelyaStage::DeckSubmerged, EVENT_HELYA_RESURFACE);
                break;
            case EVENT_HELYA_RESURFACE:
                resurface(14000, 16000);
                break;
            default:
                break;
        }
    }

private:
    void lurkBelowDeck()
    {
        setRooted(true);
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void raiseFirstTentacles()
    {
        mGraspingSlots.clear();
        for (uint32_t i = 0; i < HelyaGraspingSlotCount; ++i)
            mGraspingSlots.push_back(i);

        mFreePools.clear();
        for (uint32_t i = 1; i < HelyaPoolCount; ++i)
            mFreePools.push_back(i);

        // one tentacle on each rail and one in the stern pool
        takeGraspingSlot(Util::getRandomUInt(2), false);
        takeGraspingSlot(3 + Util::getRandomUInt(2), false);
        summonCreature(NPC_DESTRUCTOR_TENTACLE, HelyaPools[0], CORPSE_DESPAWN);
    }

    void rouseTentacles()
    {
        for (uint32_t entry : HelyaTentacleEntries)
        {
            std::list<Creature*> tentacles;
            GetCreatureListWithEntryInGrid(tentacles, entry, 150.0f);
            for (Creature* pTentacle : tentacles)
            {
                if (pTentacle->isAlive() && !pTentacle->isInCombat())
                    setZoneWideCombat(pTentacle);
            }
        }
    }

    void takeGraspingSlot(uint32_t slot, bool engage)
    {
        for (auto itr = mGraspingSlots.begin(); itr != mGraspingSlots.end(); ++itr)
        {
            if (*itr != slot)
                continue;

            mGraspingSlots.erase(itr);
            HelyaGraspingSlot const& grasp = HelyaGraspingSlots[slot];
            Creature* pTentacle = summonCreature(grasp.entry, grasp.spot, CORPSE_DESPAWN);
            if (pTentacle != nullptr && engage)
                setZoneWideCombat(pTentacle);
            return;
        }
    }

    void raiseGrasping(bool engage)
    {
        if (mGraspingSlots.empty())
            return;

        takeGraspingSlot(mGraspingSlots[Util::getRandomUInt(static_cast<uint32_t>(mGraspingSlots.size() - 1))], engage);
    }

    void raiseDestructor()
    {
        if (mFreePools.empty())
            return;

        uint32_t const pick = Util::getRandomUInt(static_cast<uint32_t>(mFreePools.size() - 1));
        uint32_t const pool = mFreePools[pick];
        mFreePools.erase(mFreePools.begin() + pick);

        if (Creature* pTentacle = summonCreature(NPC_DESTRUCTOR_TENTACLE, HelyaPools[pool], CORPSE_DESPAWN))
            setZoneWideCombat(pTentacle);
    }

    uint32_t nearestPool(LocationVector const& position) const
    {
        uint32_t nearest = 0;
        float nearestRange = -1.0f;
        for (uint32_t i = 0; i < HelyaPoolCount; ++i)
        {
            float const range = HelyaPools[i].distanceSquare(position);
            if (nearestRange < 0.0f || range < nearestRange)
            {
                nearestRange = range;
                nearest = i;
            }
        }

        return nearest;
    }

    // every lost tentacle tears a piece off Helya herself
    void loseTentacle(Creature* pTentacle)
    {
        ++mTentaclesLost;

        uint32_t const biteCount = mHealthBites++;
        uint32_t const pct = (biteCount % 3 == 2) ? 4 : 3;
        uint32_t const bite = getCreature()->getMaxHealth() / 100 * pct;
        uint32_t const health = getCreature()->getHealth();
        getCreature()->setHealth(health > bite ? health - bite : 1);

        if (mTentaclesLost >= HelyaTentaclesToBreak)
        {
            for (uint32_t entry : HelyaTentacleEntries)
                summons.despawnEntry(entry);

            warn("Helya's tentacles are broken! She dives beneath the Naglfar!");
            dive(HelyaStage::Submerged, EVENT_HELYA_SURFACE);
            return;
        }

        if (pTentacle->getEntry() == NPC_DESTRUCTOR_TENTACLE)
        {
            uint32_t const pool = nearestPool(pTentacle->GetPosition());
            if (pool != 0)
                mFreePools.push_back(pool);
            return;
        }

        ++mGraspingPending;
        scriptEvents.addEvent(EVENT_HELYA_GRASPING_RETURN, 5000);
    }

    void dive(HelyaStage stage, uint32_t returnEvent)
    {
        mStage = stage;
        scriptEvents.resetEvents();
        attackStop();
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        castSpellOnSelf(SPELL_HELYA_SUBMERGED, true);

        scriptEvents.addEvent(EVENT_HELYA_PIERCING_TENTACLE, 2000);
        scriptEvents.addEvent(returnEvent, 15000);
    }

    void climbOnDeck()
    {
        MoveTeleport(HelyaDeckSpot);
        castSpellOnSelf(SPELL_HELYA_TRUE_FORM, true);
        castSpellAOE(SPELL_HELYA_SMASH, true);
        warn("Helya rises onto the deck of the Naglfar!");
        resurface(16000, 15000);
    }

    void resurface(uint32_t torrentDelay, uint32_t bellowDelay)
    {
        mStage = HelyaStage::Deck;
        scriptEvents.removeEvent(EVENT_HELYA_PIERCING_TENTACLE);
        removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        setReactState(REACT_AGGRESSIVE);

        scriptEvents.addEvent(EVENT_HELYA_WING_BUFFET, 10000);
        scriptEvents.addEvent(EVENT_HELYA_DECK_TORRENT, torrentDelay);
        scriptEvents.addEvent(EVENT_HELYA_CORRUPTED_BELLOW, bellowDelay);
        scriptEvents.addEvent(EVENT_HELYA_DIVE, 70000);
    }

    HelyaStage mStage = HelyaStage::Tentacles;
    uint32_t mTentaclesLost = 0;
    uint32_t mHealthBites = 0;
    uint32_t mGraspingPending = 0;
    bool mRetreated = false;
    std::vector<uint32_t> mGraspingSlots;
    std::vector<uint32_t> mFreePools;
};

// rooted tentacles drag Helya into the fight when touched
class MawOfSoulsTentacleAI : public MawOfSoulsCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MawOfSoulsTentacleAI(c); }
    explicit MawOfSoulsTentacleAI(Creature* pCreature) : MawOfSoulsCombatAI(pCreature) {}

    void OnLoad() override { setRooted(true); }

    void OnCombatStart(Unit* pTarget) override
    {
        if (MawOfSoulsInstanceScript* pSouls = getMawOfSouls())
        {
            Creature* pHelya = pSouls->getHelya();
            if (pHelya != nullptr && pHelya->isAlive() && !pHelya->isInCombat())
                setZoneWideCombat(pHelya);
        }

        MawOfSoulsCombatAI::OnCombatStart(pTarget);
    }
};

class MawOfSoulsDestructorTentacleAI : public MawOfSoulsTentacleAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MawOfSoulsDestructorTentacleAI(c); }
    explicit MawOfSoulsDestructorTentacleAI(Creature* pCreature) : MawOfSoulsTentacleAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_DESTRUCTOR_RAPID_RUPTURE, 2000);
        scriptEvents.addEvent(EVENT_DESTRUCTOR_SMASH_CHECK, 2000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DESTRUCTOR_RAPID_RUPTURE:
                if (Unit* pTarget = getBestPlayerTarget(TargetFilter_InMeleeRange))
                {
                    castSpell(pTarget, SPELL_DESTRUCTOR_RAPID_RUPTURE);
                    scriptEvents.addEvent(EVENT_DESTRUCTOR_RAPID_RUPTURE, 12000);
                }
                else
                {
                    scriptEvents.addEvent(EVENT_DESTRUCTOR_RAPID_RUPTURE, 3000);
                }
                break;
            case EVENT_DESTRUCTOR_SMASH_CHECK:
                // nobody within reach, the tentacle slams the deck
                if (getBestPlayerTarget(TargetFilter_InMeleeRange) == nullptr)
                    castSpellAOE(SPELL_DESTRUCTOR_SMASH);
                scriptEvents.addEvent(EVENT_DESTRUCTOR_SMASH_CHECK, 2000);
                break;
            default:
                break;
        }
    }
};

// bursts out of a trap door once and sinks again
class MawOfSoulsPiercingTentacleAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new MawOfSoulsPiercingTentacleAI(c); }
    explicit MawOfSoulsPiercingTentacleAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mPierceClock == 0)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mPierceClock > diff)
        {
            mPierceClock -= diff;
            return;
        }

        mPierceClock = 0;
        castSpellOnSelf(SPELL_PIERCING_TENTACLE);
    }

private:
    uint32_t mPierceClock = 1000;
};

void SetupMawOfSouls(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MAW_OF_SOULS, &MawOfSoulsInstanceScript::Create);

    mgr->register_creature_script(BOSS_YMIRON, &MawOfSoulsYmironAI::Create);

    mgr->register_creature_script(BOSS_HARBARON, &MawOfSoulsHarbaronAI::Create);
    mgr->register_creature_script(NPC_COSMIC_SCYTHE, &MawOfSoulsCosmicScytheAI::Create);
    mgr->register_creature_script(NPC_SHACKLED_SERVITOR, &MawOfSoulsShackledServitorAI::Create);
    mgr->register_creature_script(NPC_SOUL_FRAGMENT, &MawOfSoulsSoulFragmentAI::Create);

    mgr->register_creature_script(BOSS_HELYA, &MawOfSoulsHelyaAI::Create);
    mgr->register_creature_script(NPC_DESTRUCTOR_TENTACLE, &MawOfSoulsDestructorTentacleAI::Create);
    mgr->register_creature_script(NPC_GRASPING_TENTACLE_LEFT_1, &MawOfSoulsTentacleAI::Create);
    mgr->register_creature_script(NPC_GRASPING_TENTACLE_LEFT_2, &MawOfSoulsTentacleAI::Create);
    mgr->register_creature_script(NPC_GRASPING_TENTACLE_RIGHT_1, &MawOfSoulsTentacleAI::Create);
    mgr->register_creature_script(NPC_GRASPING_TENTACLE_RIGHT_2, &MawOfSoulsTentacleAI::Create);
    mgr->register_creature_script(NPC_PIERCING_TENTACLE, &MawOfSoulsPiercingTentacleAI::Create);
}
