/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Nerubar_Palace.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <cmath>
#include <cstddef>
#include <map>
#include <set>
#include <vector>

enum NerubarPalaceEvents
{
    // Sikran
    EVENT_NP_SIKRAN_FLOURISH = 1,
    EVENT_NP_SIKRAN_PHASE_LUNGE,
    EVENT_NP_SIKRAN_PHASE_BLADES,
    EVENT_NP_SIKRAN_RAIN_OF_ARROWS,
    EVENT_NP_SIKRAN_DECIMATE,

    // Ulgrax the Devourer
    EVENT_NP_ULGRAX_BRUTAL_CRUSH,
    EVENT_NP_ULGRAX_VENOMOUS_LASH,
    EVENT_NP_ULGRAX_STALKERS_WEBBING,
    EVENT_NP_ULGRAX_DIGESTIVE_ACID,
    EVENT_NP_ULGRAX_HULKING_CRASH,
    EVENT_NP_ULGRAX_CARNIVOROUS_CONTEST,
    EVENT_NP_ULGRAX_JUGGERNAUT_CHARGE,
    EVENT_NP_ULGRAX_RAVENOUS_HOST,
    EVENT_NP_ULGRAX_HUNGERING_BELLOWS,
    EVENT_NP_ULGRAX_FRENZY_ENDS,

    // The Bloodbound Horror
    EVENT_NP_HORROR_GRASP_FROM_BEYOND,
    EVENT_NP_HORROR_GRUESOME_DISGORGE,
    EVENT_NP_HORROR_CRIMSON_RAIN,
    EVENT_NP_HORROR_MANIFEST,
    EVENT_NP_HORROR_SPEWING_HEMORRHAGE,
    EVENT_NP_HARBINGER_MANIFEST_HORROR,

    // Rasha'nan
    EVENT_NP_RASHANAN_SAVAGE_ASSAULT,
    EVENT_NP_RASHANAN_SPINNERETS_STRANDS,
    EVENT_NP_RASHANAN_ROLLING_ACID,
    EVENT_NP_RASHANAN_INFESTED_SPAWN,
    EVENT_NP_RASHANAN_EROSIVE_SPRAY,
    EVENT_NP_RASHANAN_CAUSTIC_HAIL,

    // Broodtwister Ovi'nax
    EVENT_NP_OVINAX_VOLATILE_CONCOCTION,
    EVENT_NP_OVINAX_EXPERIMENTAL_DOSAGE,
    EVENT_NP_OVINAX_EGGS_HATCH,
    EVENT_NP_OVINAX_STICKY_WEB,
    EVENT_NP_OVINAX_VILE_DISCHARGE,
    EVENT_NP_OVINAX_SANGUINE_OVERFLOW,
    EVENT_NP_PARASITE_REACH,

    // Nexus-Princess Ky'veza
    EVENT_NP_KYVEZA_VOID_SHREDDERS,
    EVENT_NP_KYVEZA_ASSASSINATION,
    EVENT_NP_KYVEZA_NETHER_RIFT,
    EVENT_NP_KYVEZA_TWILIGHT_MASSACRE,
    EVENT_NP_KYVEZA_NEXUS_DAGGERS,
    EVENT_NP_KYVEZA_REGICIDE,
    EVENT_NP_KYVEZA_NIGHT_FADES,

    // Anub'arash
    EVENT_NP_ANUBARASH_PIERCING_STRIKE,
    EVENT_NP_ANUBARASH_IMPALING_ERUPTION,
    EVENT_NP_ANUBARASH_CALL_OF_THE_SWARM,
    EVENT_NP_ANUBARASH_RECKLESS_CHARGE,
    EVENT_NP_ANUBARASH_BURROWED_ERUPTION,
    EVENT_NP_ANUBARASH_STINGING_SWARM,
    EVENT_NP_ANUBARASH_SEISMIC_UPHEAVAL,
    EVENT_NP_ANUBARASH_SPIKE_ERUPTION,
    EVENT_NP_ANUBARASH_UNLEASHED_SWARM,

    // Skeinspinner Takazj
    EVENT_NP_TAKAZJ_POISON_BOLT,
    EVENT_NP_TAKAZJ_WEB_BOMB,
    EVENT_NP_TAKAZJ_VENOMOUS_RAIN,
    EVENT_NP_TAKAZJ_SKITTERING_LEAP,
    EVENT_NP_TAKAZJ_ENTROPIC_BARRAGE,
    EVENT_NP_TAKAZJ_VOID_BOLT,
    EVENT_NP_TAKAZJ_STRANDS_OF_REALITY,
    EVENT_NP_TAKAZJ_VOID_STEP,
    EVENT_NP_TAKAZJ_WEB_VORTEX,
    EVENT_NP_TAKAZJ_ENTROPIC_DESOLATION,
    EVENT_NP_TAKAZJ_CATACLYSMIC_ENTROPY,

    // Queen Ansurek
    EVENT_NP_ANSUREK_LIQUEFY,
    EVENT_NP_ANSUREK_FEAST,
    EVENT_NP_ANSUREK_SILKEN_TOMB,
    EVENT_NP_ANSUREK_REACTIVE_TOXIN,
    EVENT_NP_ANSUREK_WEB_BLADES,
    EVENT_NP_ANSUREK_VENOM_NOVA,
    EVENT_NP_ANSUREK_PARALYZING_VENOM,
    EVENT_NP_ANSUREK_WREST,
    EVENT_NP_ANSUREK_PREDATION_ENDS,
    EVENT_NP_ANSUREK_COURT_WAVE,
    EVENT_NP_ANSUREK_ACID_BOLT,
    EVENT_NP_ANSUREK_ASCENDED_WREST,
    EVENT_NP_ANSUREK_ASCENSION_ENDS,
    EVENT_NP_ANSUREK_GORGE,
    EVENT_NP_ANSUREK_INFEST,
    EVENT_NP_ANSUREK_GLOOM_BURST,
    EVENT_NP_ANSUREK_ABYSSAL_INFUSION,
    EVENT_NP_ANSUREK_QUEENS_SUMMONS,
    EVENT_NP_ANSUREK_FROTHING_GLUTTONY,
    EVENT_NP_ANSUREK_ROYAL_CONDEMNATION,

    // adds with a fixed spell kit, one event per kit entry
    EVENT_NP_KIT_FIRST = 200
};

namespace
{
    float const NerubarFullCircle = 6.2831853f;

    uint32_t const SikranBerserkMs = 360000;
    uint32_t const UlgraxBerserkMs = 420000;
    uint32_t const BloodboundBerserkMs = 480000;
    uint32_t const RashananBerserkMs = 420000;
    uint32_t const OvinaxBerserkMs = 450000;
    uint32_t const SilkenCourtBerserkMs = 540000;
    uint32_t const AnsurekBerserkMs = 600000;

    // milliseconds per energy point
    uint32_t const SikranEnergyStepMs = 850;
    uint32_t const UlgraxHungerStepMs = 900;
    uint32_t const BloodboundEnergyStepMs = 600;
    uint32_t const RashananEnergyStepMs = 700;
    uint32_t const OvinaxEnergyStepMs = 800;
    uint32_t const KyvezaEnergyStepMs = 1000;
    uint32_t const CourtEnergyStepMs = 900;

    uint32_t const SimulacrumLifetimeMs = 90000;
    uint32_t const NetherPhantomLifetimeMs = 30000;

    // entrance hall door and the reach of its guards
    LocationVector const HallDoorSpot(-2719.96f, -396.54f, -1158.7f);
    float const HallGuardReach = 120.0f;

    // Ulgrax waits on the ledge until the raid enters his arena
    LocationVector const UlgraxArenaSpot(-2862.74f, -251.91f, -1189.6f, 5.55f);
    float const UlgraxArenaReach = 100.0f;
    float const UlgraxArenaHeight = 45.0f;

    // descent steps in instance ticks
    uint32_t const UlgraxRoarTick = 1;
    uint32_t const UlgraxLandTick = 3;
    uint32_t const UlgraxReadyTick = 6;

    uint32_t const UlgraxGleefulBrutality = 1;
    uint32_t const UlgraxFeedingFrenzy = 2;
    uint32_t const UlgraxJuggernautCharges = 4;

    uint32_t const KyvezaPhantomBlade = 1;
    uint32_t const KyvezaStarlessNight = 2;
    uint32_t const KyvezaNightsBeforeEternal = 3;

    // Silken Court stages, intermissions in between
    uint32_t const CourtClashOfRivals = 1;
    uint32_t const CourtVoidAscension = 2;
    uint32_t const CourtGraspOfTheVoid = 3;
    uint32_t const CourtRagingFury = 4;
    uint32_t const CourtUnleashedRage = 5;
    uint32_t const CourtIntermissionMs = 20000;

    uint32_t const CourtLords[2] = { BOSS_ANUBARASH, BOSS_TAKAZJ };

    uint32_t const AnsurekQueensVenom = 1;
    uint32_t const AnsurekSpidersWeb = 2;
    uint32_t const AnsurekRoyalAscension = 3;
    uint32_t const AnsurekParanoiasFeast = 4;
    int32_t const AnsurekPredationPct = 65;

    // Experimental Dosage hatches spiders, worms and parasites in turn
    uint32_t const BroodEntries[3] = { NPC_COLOSSAL_SPIDER, NPC_VORACIOUS_WORM, NPC_BLOOD_PARASITE };
    uint32_t const BroodCounts[3] = { 1, 2, 3 };

    struct AnsurekCourtWave
    {
        uint32_t firstEntry;
        uint32_t firstCount;
        uint32_t secondEntry;
        uint32_t secondCount;
    };

    AnsurekCourtWave const AnsurekCourtWaves[3] =
    {
        { NPC_CHAMBER_GUARDIAN,     2, NPC_CHAMBER_EXPELLER,    1 },
        { NPC_ASCENDED_VOIDSPEAKER, 1, NPC_DEVOTED_WORSHIPPER,  2 },
        { NPC_CAUSTIC_SKITTERER,    3, NPC_CHAMBER_ACOLYTE,     2 }
    };

    // energy kept by the script, one point per step
    class NerubarMeter
    {
    public:
        void reset(uint32_t points, uint32_t stepMs)
        {
            mPoints = points;
            mStepMs = stepMs;
            mCarryMs = 0;
        }

        // true once the meter is full
        bool fill(uint32_t diff)
        {
            if (mPoints >= 100)
                return true;

            mCarryMs += diff;
            while (mCarryMs >= mStepMs && mPoints < 100)
            {
                mCarryMs -= mStepMs;
                ++mPoints;
            }

            return mPoints >= 100;
        }

        // true once the meter is empty
        bool drain(uint32_t diff)
        {
            if (mPoints == 0)
                return true;

            mCarryMs += diff;
            while (mCarryMs >= mStepMs && mPoints > 0)
            {
                mCarryMs -= mStepMs;
                --mPoints;
            }

            return mPoints == 0;
        }

    private:
        uint32_t mPoints = 0;
        uint32_t mStepMs = 1000;
        uint32_t mCarryMs = 0;
    };

    LocationVector nerubarPointNear(LocationVector const& centre, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(NerubarFullCircle);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(centre.x + range * std::cos(angle), centre.y + range * std::sin(angle), centre.z, angle);
    }

    // how a kit spell picks its target
    enum class NerubarAim : uint8_t
    {
        Victim,
        Self,
        Area,
        AnyPlayer
    };

    struct NerubarKitSpell
    {
        uint32_t spellId;
        NerubarAim aim;
        uint32_t firstMs;
        uint32_t repeatMs;      // 0 casts once
        char const* warning;    // nullptr stays silent
    };

    NerubarKitSpell const RavenousSpawnKit[] =
    {
        { SPELL_RAVENOUS_SPAWN_DISEMBOWEL, NerubarAim::Victim, 3000, 7000, nullptr }
    };

    NerubarKitSpell const LostWatcherKit[] =
    {
        { SPELL_LOST_WATCHER_SPECTRAL_SLAM, NerubarAim::Victim, 6000, 14000, nullptr },
        { SPELL_LOST_WATCHER_BLACK_BULWARK, NerubarAim::Self, 12000, 25000, nullptr }
    };

    NerubarKitSpell const InfestedSpawnKit[] =
    {
        { SPELL_INFESTED_SPAWN_INFESTED_BITE, NerubarAim::Victim, 1500, 15000, nullptr }
    };

    NerubarKitSpell const ColossalSpiderKit[] =
    {
        { SPELL_COLOSSAL_SPIDER_VICIOUS_BITE, NerubarAim::Victim, 5000, 10000, nullptr }
    };

    NerubarKitSpell const VoraciousWormKit[] =
    {
        { SPELL_VORACIOUS_WORM_POISON_BURST, NerubarAim::Area, 6000, 12000, nullptr }
    };

    NerubarKitSpell const NetherPhantomKit[] =
    {
        { SPELL_KYVEZA_NEXUS_DAGGERS, NerubarAim::Area, 4000, 12000, nullptr },
        { SPELL_KYVEZA_NETHER_RIFT, NerubarAim::Area, 9000, 15000, nullptr }
    };

    NerubarKitSpell const ChamberGuardianKit[] =
    {
        { SPELL_CHAMBER_GUARDIAN_OUST, NerubarAim::Victim, 5000, 15000, nullptr }
    };

    NerubarKitSpell const ChamberExpellerKit[] =
    {
        { SPELL_CHAMBER_EXPELLER_EXPULSION_BEAM, NerubarAim::Area, 6000, 14000, nullptr }
    };

    NerubarKitSpell const ChamberAcolyteKit[] =
    {
        { SPELL_CHAMBER_ACOLYTE_DARK_DETONATION, NerubarAim::Area, 8000, 12000, nullptr }
    };

    NerubarKitSpell const VoidspeakerKit[] =
    {
        { SPELL_VOIDSPEAKER_SHADOWBLAST, NerubarAim::Victim, 3000, 6000, nullptr },
        { SPELL_VOIDSPEAKER_RADIATING_GLOOM, NerubarAim::Self, 12000, 22000, nullptr }
    };

    NerubarKitSpell const WorshipperKit[] =
    {
        { SPELL_WORSHIPPER_GLOOM_TOUCH, NerubarAim::Self, 5000, 15000, nullptr },
        { SPELL_WORSHIPPER_PROTECTION, NerubarAim::Self, 10000, 25000, nullptr },
        { SPELL_WORSHIPPER_COSMIC_APOCALYPSE, NerubarAim::Area, 45000, 0, "A Devoted Worshipper calls down a Cosmic Apocalypse!" }
    };

    NerubarKitSpell const CausticSkittererKit[] =
    {
        { SPELL_SKITTERER_CAUSTIC_FANGS, NerubarAim::Self, 500, 0, nullptr }
    };

    NerubarKitSpell const SummonedAcolyteKit[] =
    {
        { SPELL_SUMMONED_ACOLYTE_DARK_BARRIER, NerubarAim::Self, 500, 0, nullptr },
        { SPELL_SUMMONED_ACOLYTE_NULL_DETONATION, NerubarAim::Area, 20000, 20000, nullptr }
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class NerubarPalaceInstanceScript : public InstanceScript
{
public:
    explicit NerubarPalaceInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(NerubarPalaceEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new NerubarPalaceInstanceScript(pMapMgr); }

    // the court is down only with both lords
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (id == DATA_SILKEN_COURT && state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && isCourtStanding())
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        uint32_t const entry = pCreature->getEntry();
        switch (entry)
        {
            case BOSS_SIKRAN:
            case BOSS_ULGRAX:
            case BOSS_BLOODBOUND_HORROR:
            case BOSS_RASHANAN:
            case BOSS_OVINAX:
            case BOSS_KYVEZA:
            case BOSS_ANUBARASH:
            case BOSS_TAKAZJ:
            case BOSS_QUEEN_ANSUREK:
                mBossGuids[entry] = pCreature->GetNewGUID();
                break;
            case NPC_SCARAB_CAPTAIN_VULAKAN:
            case NPC_KAHETI_SENTRY:
            case NPC_WOVEN_THREADMANCER:
                if (pCreature->GetPosition().getExactDist2d(HallDoorSpot) <= HallGuardReach)
                    mHallGuardGuids.insert(pCreature->GetNewGUID());
                break;
            default:
                break;
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() != GO_NERUBAR_HALL_DOOR)
            return;

        mHallDoorGuid = pGameObject->GetNewGUID();
        setDoorOpen(pGameObject, isHallOpen());
    }

    void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
    {
        if (mHallCleared || mHallGuardGuids.find(pVictim->GetNewGUID()) == mHallGuardGuids.end())
            return;

        for (WoWGuid const& guid : mHallGuardGuids)
        {
            Creature* pGuard = getCreatureByGuid(guid);
            if (pGuard != nullptr && pGuard != pVictim && pGuard->isAlive())
                return;
        }

        mHallCleared = true;
        refreshHallDoor();
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        refreshHallDoor();

        if (id != DATA_SILKEN_COURT)
            return;

        // every pull starts with the clash of rivals
        if (state == EncounterStates::InProgress || state == EncounterStates::Failed)
        {
            mCourtStage = CourtClashOfRivals;
            mCourtIntermissionLeftMs = 0;
        }

        // a wiped court fights again in full
        if (state == EncounterStates::Failed)
        {
            for (Creature* pLord : getCourt())
            {
                if (!pLord->isAlive())
                    pLord->despawn(1000, 5000);
            }
        }
    }

    void UpdateEvent() override
    {
        callUlgraxDown();
        tickCourtIntermission();
    }

    Creature* getBoss(uint32_t entry)
    {
        auto const itr = mBossGuids.find(entry);
        return itr != mBossGuids.end() ? getCreatureByGuid(itr->second) : nullptr;
    }

    std::vector<Creature*> getCourt()
    {
        std::vector<Creature*> court;
        for (uint32_t entry : CourtLords)
        {
            if (Creature* pLord = getBoss(entry))
                court.push_back(pLord);
        }

        return court;
    }

    Creature* getCourtPartner(Creature* pLord)
    {
        for (Creature* pMember : getCourt())
        {
            if (pMember != pLord)
                return pMember;
        }

        return nullptr;
    }

    bool isCourtStanding()
    {
        for (Creature* pLord : getCourt())
        {
            if (pLord->isAlive())
                return true;
        }

        return false;
    }

    // pulling one lord pulls the other
    void rallyCourt(CreatureAIScript* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pLord : getCourt())
        {
            if (pLord != pPuller->getCreature() && pLord->isAlive() && !pLord->isInCombat())
                pPuller->setZoneWideCombat(pLord);
        }
        mRallying = false;
    }

    uint32_t getCourtStage() const { return mCourtStage; }

    // a full lord meter starts the next intermission
    void openCourtIntermission(uint32_t intermission)
    {
        if (intermission <= mCourtStage || getBossState(DATA_SILKEN_COURT) != EncounterStates::InProgress)
            return;

        mCourtStage = intermission;
        mCourtIntermissionLeftMs = CourtIntermissionMs;
        announceCourtStage();
    }

private:
    void tickCourtIntermission()
    {
        if (mCourtIntermissionLeftMs == 0)
            return;

        if (getBossState(DATA_SILKEN_COURT) != EncounterStates::InProgress)
        {
            mCourtIntermissionLeftMs = 0;
            return;
        }

        uint32_t const step = getUpdateFrequency();
        if (mCourtIntermissionLeftMs > step)
        {
            mCourtIntermissionLeftMs -= step;
            return;
        }

        // the lords rejoin in the following stage
        mCourtIntermissionLeftMs = 0;
        ++mCourtStage;
        announceCourtStage();
    }

    void announceCourtStage()
    {
        for (Creature* pLord : getCourt())
        {
            if (pLord->isAlive() && pLord->GetScript() != nullptr)
                pLord->GetScript()->DoAction(ACTION_NP_COURT_STAGE);
        }
    }

    // Ulgrax comes down once a living player stands in his arena
    void callUlgraxDown()
    {
        if (getBossState(DATA_ULGRAX) == EncounterStates::Performed)
            return;

        Creature* pUlgrax = getBoss(BOSS_ULGRAX);
        if (pUlgrax == nullptr || !pUlgrax->isAlive() || pUlgrax->isInCombat() || pUlgrax->GetScript() == nullptr)
            return;

        for (auto const& mapPlayer : getWorldMap()->getPlayers())
        {
            Player* pPlayer = mapPlayer.second;
            if (pPlayer == nullptr || !pPlayer->isAlive())
                continue;

            LocationVector const position = pPlayer->GetPosition();
            if (position.getExactDist2d(UlgraxArenaSpot) <= UlgraxArenaReach && std::fabs(position.z - UlgraxArenaSpot.z) <= UlgraxArenaHeight)
            {
                pUlgrax->GetScript()->DoAction(ACTION_NP_ULGRAX_DESCEND);
                return;
            }
        }
    }

    // a saved run past any boss keeps the hall open
    bool isHallOpen() const
    {
        if (mHallCleared)
            return true;

        for (uint32_t id = 0; id < NerubarPalaceEncounterCount; ++id)
        {
            if (getBossState(id) == EncounterStates::Performed)
                return true;
        }

        return false;
    }

    void refreshHallDoor()
    {
        if (GameObject* pDoor = getGameObjectByGuid(mHallDoorGuid))
            setDoorOpen(pDoor, isHallOpen());
    }

    // the encounter decides, the spawn state of a door is not reliable
    static void setDoorOpen(GameObject* pDoor, bool open)
    {
        uint8_t const state = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != state)
            pDoor->setState(state);
    }

    std::map<uint32_t, WoWGuid> mBossGuids;
    std::set<WoWGuid> mHallGuardGuids;
    WoWGuid mHallDoorGuid;
    bool mHallCleared = false;
    bool mRallying = false;
    uint32_t mCourtStage = CourtClashOfRivals;
    uint32_t mCourtIntermissionLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick, berserk and target helpers

class NerubarPalaceCombatAI : public CreatureAIScript
{
public:
    explicit NerubarPalaceCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStage = 1;
        mBerserkLeftMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        endFight();
        onDisengage();
    }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        tickBerserk(diff);
        onCombatTick(diff);

        // due events wait while a cast runs
        while (!_isCasting())
        {
            uint32_t const eventId = scriptEvents.getFinishedEvent();
            if (eventId == 0)
                break;

            onEvent(eventId);
        }
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    NerubarPalaceInstanceScript* getPalace()
    {
        if (getCreature()->GetMapId() != MAP_NERUBAR_PALACE)
            return nullptr;

        return static_cast<NerubarPalaceInstanceScript*>(getInstanceScript());
    }

    void endFight()
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mStage = 0;
        mBerserkLeftMs = 0;
    }

    // a new stage drops what the old one had pending
    void switchStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
    }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    // distinct random players, the tank only when nobody else is left
    std::vector<Unit*> pickPlayers(size_t count, bool skipTank = false)
    {
        TargetFilter const filter = skipTank ? TargetFilter(TargetFilter_Player | TargetFilter_NotCurrent) : TargetFilter_Player;
        FilterArgs const args(filter);

        std::vector<Unit*> pool;
        for (Object* pObject : getCreature()->getInRangePlayersSet())
        {
            if (pObject != nullptr && isValidUnitTarget(pObject, filter, args))
                pool.push_back(static_cast<Unit*>(pObject));
        }

        if (pool.empty() && skipTank)
            return pickPlayers(count, false);

        std::vector<Unit*> picked;
        while (!pool.empty() && picked.size() < count)
        {
            size_t const index = Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1));
            picked.push_back(pool[index]);
            pool[index] = pool.back();
            pool.pop_back();
        }

        return picked;
    }

    Unit* pickPlayer(bool skipTank = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false)
    {
        Unit* pTarget = pickPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId);

        return pTarget;
    }

    void castAtPlayers(uint32_t spellId, size_t count, bool skipTank = false)
    {
        for (Unit* pTarget : pickPlayers(count, skipTank))
            castSpell(pTarget, spellId, true);
    }

    // destination spell aimed at the ground under a random player
    void castBelowPlayer(uint32_t spellId, bool skipTank = false)
    {
        if (Unit* pTarget = pickPlayer(skipTank))
            getCreature()->castSpellLoc(pTarget->GetPosition(), spellId, false);
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, durationMs);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // adds raised by an add belong to the boss and leave with its fight
    Creature* callAddForBoss(uint32_t bossEntry, uint32_t entry, LocationVector const& spot)
    {
        Creature* pBoss = nullptr;
        if (NerubarPalaceInstanceScript* pPalace = getPalace())
            pBoss = pPalace->getBoss(bossEntry);

        if (pBoss == nullptr || !pBoss->isAlive())
            return callAdd(entry, spot);

        Creature* pAdd = pBoss->summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    LocationVector nearSelf(float minRange, float maxRange)
    {
        return nerubarPointNear(getCreature()->GetPosition(), minRange, maxRange);
    }

    uint32_t mStage = 0;

private:
    void tickBerserk(uint32_t diff)
    {
        if (mBerserkLeftMs == 0)
            return;

        if (mBerserkLeftMs > diff)
        {
            mBerserkLeftMs -= diff;
            return;
        }

        mBerserkLeftMs = 0;
        castSpellOnSelf(SPELL_NERUBAR_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and court pull

class NerubarPalaceBossAI : public NerubarPalaceCombatAI
{
public:
    NerubarPalaceBossAI(Creature* pCreature, uint32_t dataId) : NerubarPalaceCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (NerubarPalaceInstanceScript* pPalace = getPalace())
        {
            pPalace->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pPalace->setBossState(mDataId, EncounterStates::InProgress);

            if (mDataId == DATA_SILKEN_COURT)
                pPalace->rallyCourt(this);
        }

        NerubarPalaceCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (isAlive())
        {
            if (NerubarPalaceInstanceScript* pPalace = getPalace())
            {
                pPalace->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pPalace->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        NerubarPalaceCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        // the instance holds the court back while a lord still stands
        if (NerubarPalaceInstanceScript* pPalace = getPalace())
        {
            pPalace->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pPalace->setBossState(mDataId, EncounterStates::Performed);
        }

        endFight();
        onDefeat();
    }

protected:
    virtual void onDefeat() {}

    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adds that only repeat a fixed spell kit

class NerubarPalaceKitAI : public NerubarPalaceCombatAI
{
public:
    template <size_t N>
    NerubarPalaceKitAI(Creature* pCreature, NerubarKitSpell const (&kit)[N]) : NerubarPalaceCombatAI(pCreature), mKit(kit), mKitSize(N) {}

    static CreatureAIScript* CreateRavenousSpawn(Creature* c) { return new NerubarPalaceKitAI(c, RavenousSpawnKit); }
    static CreatureAIScript* CreateLostWatcher(Creature* c) { return new NerubarPalaceKitAI(c, LostWatcherKit); }
    static CreatureAIScript* CreateInfestedSpawn(Creature* c) { return new NerubarPalaceKitAI(c, InfestedSpawnKit); }
    static CreatureAIScript* CreateColossalSpider(Creature* c) { return new NerubarPalaceKitAI(c, ColossalSpiderKit); }
    static CreatureAIScript* CreateVoraciousWorm(Creature* c) { return new NerubarPalaceKitAI(c, VoraciousWormKit); }
    static CreatureAIScript* CreateNetherPhantom(Creature* c) { return new NerubarPalaceKitAI(c, NetherPhantomKit); }
    static CreatureAIScript* CreateChamberGuardian(Creature* c) { return new NerubarPalaceKitAI(c, ChamberGuardianKit); }
    static CreatureAIScript* CreateChamberExpeller(Creature* c) { return new NerubarPalaceKitAI(c, ChamberExpellerKit); }
    static CreatureAIScript* CreateChamberAcolyte(Creature* c) { return new NerubarPalaceKitAI(c, ChamberAcolyteKit); }
    static CreatureAIScript* CreateVoidspeaker(Creature* c) { return new NerubarPalaceKitAI(c, VoidspeakerKit); }
    static CreatureAIScript* CreateWorshipper(Creature* c) { return new NerubarPalaceKitAI(c, WorshipperKit); }
    static CreatureAIScript* CreateCausticSkitterer(Creature* c) { return new NerubarPalaceKitAI(c, CausticSkittererKit); }
    static CreatureAIScript* CreateSummonedAcolyte(Creature* c) { return new NerubarPalaceKitAI(c, SummonedAcolyteKit); }

protected:
    void onEngage() override
    {
        for (size_t i = 0; i < mKitSize; ++i)
            scriptEvents.addEvent(EVENT_NP_KIT_FIRST + static_cast<uint32_t>(i), mKit[i].firstMs);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId < EVENT_NP_KIT_FIRST || eventId >= EVENT_NP_KIT_FIRST + mKitSize)
            return;

        NerubarKitSpell const& spell = mKit[eventId - EVENT_NP_KIT_FIRST];
        if (spell.warning != nullptr)
            warn(spell.warning);

        switch (spell.aim)
        {
            case NerubarAim::Victim:
                castSpellOnVictim(spell.spellId);
                break;
            case NerubarAim::Self:
                castSpellOnSelf(spell.spellId);
                break;
            case NerubarAim::AnyPlayer:
                castAtPlayer(spell.spellId);
                break;
            default:
                castSpellAOE(spell.spellId);
                break;
        }

        if (spell.repeatMs != 0)
            scriptEvents.addEvent(eventId, spell.repeatMs);
    }

private:
    NerubarKitSpell const* mKit;
    size_t mKitSize;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Sikran, Captain of the Sureki

class NerubarPalaceSikranAI : public NerubarPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NerubarPalaceSikranAI(c); }
    explicit NerubarPalaceSikranAI(Creature* pCreature) : NerubarPalaceBossAI(pCreature, DATA_SIKRAN) {}

protected:
    void onEngage() override
    {
        armBerserk(SikranBerserkMs);
        mEnergy.reset(0, SikranEnergyStepMs);

        scriptEvents.addEvent(EVENT_NP_SIKRAN_FLOURISH, 6000);
        scriptEvents.addEvent(EVENT_NP_SIKRAN_PHASE_BLADES, 16000);
        scriptEvents.addEvent(EVENT_NP_SIKRAN_RAIN_OF_ARROWS, 24000);
        scriptEvents.addEvent(EVENT_NP_SIKRAN_DECIMATE, 32000);
    }

    // full energy ends in a Shattering Sweep
    void onCombatTick(uint32_t diff) override
    {
        if (!mEnergy.fill(diff) || _isCasting())
            return;

        mEnergy.reset(0, SikranEnergyStepMs);
        warn("Sikran unleashes a Shattering Sweep!");
        castSpellAOE(SPELL_SIKRAN_SHATTERING_SWEEP);
        shatterSimulacra();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NP_SIKRAN_FLOURISH:
                // Captain's Flourish opens with Expose and ends in Phase Lunge
                castSpellOnVictim(SPELL_SIKRAN_EXPOSE);
                scriptEvents.addEvent(EVENT_NP_SIKRAN_PHASE_LUNGE, 2500);
                scriptEvents.addEvent(EVENT_NP_SIKRAN_FLOURISH, 24000);
                break;
            case EVENT_NP_SIKRAN_PHASE_LUNGE:
                castSpellOnVictim(SPELL_SIKRAN_PHASE_LUNGE);
                break;
            case EVENT_NP_SIKRAN_PHASE_BLADES:
                throwPhaseBlades();
                scriptEvents.addEvent(EVENT_NP_SIKRAN_PHASE_BLADES, 30000);
                break;
            case EVENT_NP_SIKRAN_RAIN_OF_ARROWS:
                castSpellAOE(SPELL_SIKRAN_RAIN_OF_ARROWS);
                scriptEvents.addEvent(EVENT_NP_SIKRAN_RAIN_OF_ARROWS, 32000);
                break;
            case EVENT_NP_SIKRAN_DECIMATE:
                castAtPlayer(SPELL_SIKRAN_DECIMATE, true);
                shatterSimulacra();
                scriptEvents.addEvent(EVENT_NP_SIKRAN_DECIMATE, 36000);
                break;
            default:
                break;
        }
    }

private:
    // a simulacrum stays behind every struck player on heroic, behind the first one otherwise
    void throwPhaseBlades()
    {
        bool const heroic = isHeroic();
        std::vector<Unit*> const targets = pickPlayers(heroic ? 3 : 2, true);
        if (targets.empty())
            return;

        castSpell(targets.front(), SPELL_SIKRAN_PHASE_BLADES);
        for (size_t i = 0; i < targets.size(); ++i)
        {
            if (i > 0 && !heroic)
                break;

            summonCreature(NPC_COSMIC_SIMULACRUM, targets[i]->GetPosition(), TIMED_DESPAWN, SimulacrumLifetimeMs);
        }
    }

    // destroyed simulacra shower the raid with Cosmic Shards
    void shatterSimulacra()
    {
        if (!summons.hasEntry(NPC_COSMIC_SIMULACRUM))
            return;

        summons.despawnEntry(NPC_COSMIC_SIMULACRUM);
        castSpellAOE(SPELL_SIKRAN_COSMIC_SHARDS, true);
    }

    NerubarMeter mEnergy;
};

// a fixed tear that pulses cosmic damage until it is destroyed
class NerubarPalaceCosmicSimulacrumAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NerubarPalaceCosmicSimulacrumAI(c); }
    explicit NerubarPalaceCosmicSimulacrumAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setIgnoreAllCombat(true);
        setRooted(true);
        _setMeleeDisabled(true);
        castSpellOnSelf(SPELL_SIMULACRUM_PRESENCE, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ulgrax the Devourer

class NerubarPalaceUlgraxAI : public NerubarPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NerubarPalaceUlgraxAI(c); }
    explicit NerubarPalaceUlgraxAI(Creature* pCreature) : NerubarPalaceBossAI(pCreature, DATA_ULGRAX) {}

    void OnLoad() override
    {
        // a spawn inside the arena needs no descent
        if (getCreature()->GetPosition().getExactDist2d(UlgraxArenaSpot) <= UlgraxArenaReach)
            finishDescent();
    }

    // sent each instance tick while the raid stands in the arena: roar, Swallowing Darkness, drop, fight
    void DoAction(int32_t action) override
    {
        if (action != ACTION_NP_ULGRAX_DESCEND || mDescended || _isInCombat())
            return;

        ++mDescentTicks;
        if (mDescentTicks == UlgraxRoarTick)
        {
            getCreature()->emote(EMOTE_ONESHOT_BATTLEROAR);
            castSpellOnSelf(SPELL_ULGRAX_DESCENT_DARKNESS);
        }
        else if (mDescentTicks == UlgraxLandTick)
        {
            MoveTeleport(UlgraxArenaSpot);
            getCreature()->SetSpawnLocation(UlgraxArenaSpot);
        }
        else if (mDescentTicks >= UlgraxReadyTick)
        {
            finishDescent();
        }
    }

protected:
    void onEngage() override
    {
        mDescended = true;
        armBerserk(UlgraxBerserkMs);
        beginGleefulBrutality();
    }

    void onDisengage() override { _removeAura(SPELL_ULGRAX_READY_TO_FEED); }

    // the hunger meter drains, at zero he goes into a Feeding Frenzy
    void onCombatTick(uint32_t diff) override
    {
        if (mStage == UlgraxGleefulBrutality && mHunger.drain(diff) && !_isCasting())
            beginFeedingFrenzy();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NP_ULGRAX_BRUTAL_CRUSH:
                castSpellOnVictim(SPELL_ULGRAX_BRUTAL_CRUSH);
                scriptEvents.addEvent(EVENT_NP_ULGRAX_BRUTAL_CRUSH, 15000);
                break;
            case EVENT_NP_ULGRAX_VENOMOUS_LASH:
                castSpellAOE(SPELL_ULGRAX_VENOMOUS_LASH);
                scriptEvents.addEvent(EVENT_NP_ULGRAX_VENOMOUS_LASH, 25000);
                break;
            case EVENT_NP_ULGRAX_STALKERS_WEBBING:
                castAtPlayer(SPELL_ULGRAX_STALKERS_WEBBING, true);
                scriptEvents.addEvent(EVENT_NP_ULGRAX_STALKERS_WEBBING, 30000);
                break;
            case EVENT_NP_ULGRAX_DIGESTIVE_ACID:
                castSpellAOE(SPELL_ULGRAX_DIGESTIVE_ACID);
                scriptEvents.addEvent(EVENT_NP_ULGRAX_DIGESTIVE_ACID, 30000);
                break;
            case EVENT_NP_ULGRAX_HULKING_CRASH:
                castSpellAOE(SPELL_ULGRAX_HULKING_CRASH);
                scriptEvents.addEvent(EVENT_NP_ULGRAX_HULKING_CRASH, 35000);
                break;
            case EVENT_NP_ULGRAX_CARNIVOROUS_CONTEST:
                if (castAtPlayer(SPELL_ULGRAX_CARNIVOROUS_CONTEST, true) != nullptr)
                    warn("Ulgrax the Devourer drags a victim into a Carnivorous Contest!");
                scriptEvents.addEvent(EVENT_NP_ULGRAX_CARNIVOROUS_CONTEST, 40000);
                break;
            case EVENT_NP_ULGRAX_JUGGERNAUT_CHARGE:
                castAtPlayer(SPELL_ULGRAX_JUGGERNAUT_CHARGE, true);
                if (mChargesLeft > 0 && --mChargesLeft > 0)
                    scriptEvents.addEvent(EVENT_NP_ULGRAX_JUGGERNAUT_CHARGE, 3500);
                break;
            case EVENT_NP_ULGRAX_RAVENOUS_HOST:
                warn("Ulgrax the Devourer unleashes a host of Ravenous Spawn!");
                for (uint8_t i = 0; i < (isHeroic() ? 6 : 4); ++i)
                    callAdd(NPC_RAVENOUS_SPAWN, nearSelf(10.0f, 20.0f));
                break;
            case EVENT_NP_ULGRAX_HUNGERING_BELLOWS:
                castSpellAOE(SPELL_ULGRAX_HUNGERING_BELLOWS);
                scriptEvents.addEvent(EVENT_NP_ULGRAX_HUNGERING_BELLOWS, 8000);
                break;
            case EVENT_NP_ULGRAX_FRENZY_ENDS:
                castSpellAOE(SPELL_ULGRAX_SWALLOWING_DARKNESS);
                beginGleefulBrutality();
                break;
            default:
                break;
        }
    }

private:
    void beginGleefulBrutality()
    {
        switchStage(UlgraxGleefulBrutality);
        mHunger.reset(100, UlgraxHungerStepMs);
        _removeAura(SPELL_ULGRAX_READY_TO_FEED);

        scriptEvents.addEvent(EVENT_NP_ULGRAX_BRUTAL_CRUSH, 5000);
        scriptEvents.addEvent(EVENT_NP_ULGRAX_VENOMOUS_LASH, 9000);
        scriptEvents.addEvent(EVENT_NP_ULGRAX_STALKERS_WEBBING, 14000);
        scriptEvents.addEvent(EVENT_NP_ULGRAX_DIGESTIVE_ACID, 20000);
        scriptEvents.addEvent(EVENT_NP_ULGRAX_HULKING_CRASH, 26000);
        scriptEvents.addEvent(EVENT_NP_ULGRAX_CARNIVOROUS_CONTEST, 33000);
    }

    void beginFeedingFrenzy()
    {
        switchStage(UlgraxFeedingFrenzy);
        mChargesLeft = UlgraxJuggernautCharges;
        warn("Ulgrax the Devourer goes mad with hunger!");
        castSpellOnSelf(SPELL_ULGRAX_READY_TO_FEED, true);

        scriptEvents.addEvent(EVENT_NP_ULGRAX_JUGGERNAUT_CHARGE, 2000);
        scriptEvents.addEvent(EVENT_NP_ULGRAX_RAVENOUS_HOST, 16000);
        scriptEvents.addEvent(EVENT_NP_ULGRAX_HUNGERING_BELLOWS, 20000);
        scriptEvents.addEvent(EVENT_NP_ULGRAX_STALKERS_WEBBING, 24000);
        scriptEvents.addEvent(EVENT_NP_ULGRAX_FRENZY_ENDS, 45000);
    }

    void finishDescent()
    {
        mDescended = true;
        setIgnoreAllCombat(false);
    }

    bool mDescended = false;
    uint32_t mDescentTicks = 0;
    uint32_t mChargesLeft = 0;
    NerubarMeter mHunger;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Bloodbound Horror

class NerubarPalaceBloodboundHorrorAI : public NerubarPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NerubarPalaceBloodboundHorrorAI(c); }
    explicit NerubarPalaceBloodboundHorrorAI(Creature* pCreature) : NerubarPalaceBossAI(pCreature, DATA_BLOODBOUND_HORROR) {}

protected:
    void onEngage() override
    {
        armBerserk(BloodboundBerserkMs);
        mEnergy.reset(0, BloodboundEnergyStepMs);

        scriptEvents.addEvent(EVENT_NP_HORROR_GRASP_FROM_BEYOND, 9000);
        scriptEvents.addEvent(EVENT_NP_HORROR_GRUESOME_DISGORGE, 14000);
        scriptEvents.addEvent(EVENT_NP_HORROR_CRIMSON_RAIN, 20000);
        scriptEvents.addEvent(EVENT_NP_HORROR_MANIFEST, 25000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_NP_HORROR_SPEWING_HEMORRHAGE, 35000);
    }

    // full energy assails the raid with Goresplatter
    void onCombatTick(uint32_t diff) override
    {
        if (!mEnergy.fill(diff) || _isCasting())
            return;

        mEnergy.reset(0, BloodboundEnergyStepMs);
        warn("The Bloodbound Horror splatters the chamber with gore!");
        castSpellAOE(SPELL_BLOODBOUND_GORESPLATTER);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NP_HORROR_GRASP_FROM_BEYOND:
                castAtPlayer(SPELL_BLOODBOUND_GRASP_FROM_BEYOND, true);
                scriptEvents.addEvent(EVENT_NP_HORROR_GRASP_FROM_BEYOND, 28000);
                break;
            case EVENT_NP_HORROR_GRUESOME_DISGORGE:
                warn("The Bloodbound Horror begins a Gruesome Disgorge!");
                castSpellOnVictim(SPELL_BLOODBOUND_GRUESOME_DISGORGE);
                scriptEvents.addEvent(EVENT_NP_HORROR_GRUESOME_DISGORGE, 40000);
                break;
            case EVENT_NP_HORROR_CRIMSON_RAIN:
                castAtPlayers(SPELL_BLOODBOUND_CRIMSON_RAIN, 3);
                scriptEvents.addEvent(EVENT_NP_HORROR_CRIMSON_RAIN, 30000);
                break;
            case EVENT_NP_HORROR_MANIFEST:
                warn("Creatures of The Unseeming manifest around The Bloodbound Horror!");
                for (uint8_t i = 0; i < 2; ++i)
                    callAdd(NPC_LOST_WATCHER, nearSelf(10.0f, 18.0f));
                callAdd(NPC_FORGOTTEN_HARBINGER, nearSelf(15.0f, 25.0f));
                scriptEvents.addEvent(EVENT_NP_HORROR_MANIFEST, 60000);
                break;
            case EVENT_NP_HORROR_SPEWING_HEMORRHAGE:
                castSpellAOE(SPELL_BLOODBOUND_SPEWING_HEMORRHAGE);
                scriptEvents.addEvent(EVENT_NP_HORROR_SPEWING_HEMORRHAGE, 50000);
                break;
            default:
                break;
        }
    }

private:
    NerubarMeter mEnergy;
};

// keeps calling Blood Horrors until it is defeated
class NerubarPalaceForgottenHarbingerAI : public NerubarPalaceCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NerubarPalaceForgottenHarbingerAI(c); }
    explicit NerubarPalaceForgottenHarbingerAI(Creature* pCreature) : NerubarPalaceCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_NP_HARBINGER_MANIFEST_HORROR, 5000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_NP_HARBINGER_MANIFEST_HORROR)
            return;

        callAddForBoss(BOSS_BLOODBOUND_HORROR, NPC_BLOOD_HORROR, nearSelf(2.0f, 5.0f));
        scriptEvents.addEvent(EVENT_NP_HARBINGER_MANIFEST_HORROR, 15000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rasha'nan

class NerubarPalaceRashananAI : public NerubarPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NerubarPalaceRashananAI(c); }
    explicit NerubarPalaceRashananAI(Creature* pCreature) : NerubarPalaceBossAI(pCreature, DATA_RASHANAN) {}

protected:
    void onEngage() override
    {
        armBerserk(RashananBerserkMs);
        mEnergy.reset(0, RashananEnergyStepMs);
        mHailLeft = 0;

        scriptEvents.addEvent(EVENT_NP_RASHANAN_SAVAGE_ASSAULT, 7000);
        scriptEvents.addEvent(EVENT_NP_RASHANAN_SPINNERETS_STRANDS, 12000);
        scriptEvents.addEvent(EVENT_NP_RASHANAN_ROLLING_ACID, 18000);
        scriptEvents.addEvent(EVENT_NP_RASHANAN_INFESTED_SPAWN, 25000);
        scriptEvents.addEvent(EVENT_NP_RASHANAN_EROSIVE_SPRAY, 32000);
    }

    // full energy: Acidic Eruption, then a volley of Caustic Hail
    void onCombatTick(uint32_t diff) override
    {
        if (!mEnergy.fill(diff) || _isCasting())
            return;

        mEnergy.reset(0, RashananEnergyStepMs);
        warn("Rasha'nan erupts in acid!");
        castSpellAOE(SPELL_RASHANAN_ACIDIC_ERUPTION);
        mHailLeft = 3;
        scriptEvents.addEvent(EVENT_NP_RASHANAN_CAUSTIC_HAIL, 6000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NP_RASHANAN_SAVAGE_ASSAULT:
                castSpellOnVictim(SPELL_RASHANAN_SAVAGE_ASSAULT);
                scriptEvents.addEvent(EVENT_NP_RASHANAN_SAVAGE_ASSAULT, 16000);
                break;
            case EVENT_NP_RASHANAN_SPINNERETS_STRANDS:
                castSpellAOE(SPELL_RASHANAN_SPINNERETS_STRANDS);
                scriptEvents.addEvent(EVENT_NP_RASHANAN_SPINNERETS_STRANDS, 30000);
                break;
            case EVENT_NP_RASHANAN_ROLLING_ACID:
                castBelowPlayer(SPELL_RASHANAN_ROLLING_ACID);
                scriptEvents.addEvent(EVENT_NP_RASHANAN_ROLLING_ACID, 35000);
                break;
            case EVENT_NP_RASHANAN_INFESTED_SPAWN:
                releaseInfestedSpawn();
                scriptEvents.addEvent(EVENT_NP_RASHANAN_INFESTED_SPAWN, 40000);
                break;
            case EVENT_NP_RASHANAN_EROSIVE_SPRAY:
                castSpellAOE(SPELL_RASHANAN_EROSIVE_SPRAY);
                scriptEvents.addEvent(EVENT_NP_RASHANAN_EROSIVE_SPRAY, 45000);
                break;
            case EVENT_NP_RASHANAN_CAUSTIC_HAIL:
                castBelowPlayer(SPELL_RASHANAN_CAUSTIC_HAIL);
                if (mHailLeft > 0 && --mHailLeft > 0)
                    scriptEvents.addEvent(EVENT_NP_RASHANAN_CAUSTIC_HAIL, 3000);
                break;
            default:
                break;
        }
    }

private:
    // marked players get a spawn dropped on them
    void releaseInfestedSpawn()
    {
        for (Unit* pTarget : pickPlayers(isHeroic() ? 3 : 2, true))
        {
            castSpell(pTarget, SPELL_RASHANAN_INFESTED_SPAWN, true);
            callAdd(NPC_INFESTED_SPAWN, nerubarPointNear(pTarget->GetPosition(), 2.0f, 4.0f));
        }
    }

    NerubarMeter mEnergy;
    uint32_t mHailLeft = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Broodtwister Ovi'nax

class NerubarPalaceOvinaxAI : public NerubarPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NerubarPalaceOvinaxAI(c); }
    explicit NerubarPalaceOvinaxAI(Creature* pCreature) : NerubarPalaceBossAI(pCreature, DATA_OVINAX) {}

protected:
    void onEngage() override
    {
        armBerserk(OvinaxBerserkMs);
        mEnergy.reset(0, OvinaxEnergyStepMs);
        mBroodTurn = 0;
        mDosedSpots.clear();

        scriptEvents.addEvent(EVENT_NP_OVINAX_VOLATILE_CONCOCTION, 8000);
        scriptEvents.addEvent(EVENT_NP_OVINAX_EXPERIMENTAL_DOSAGE, 14000);
        scriptEvents.addEvent(EVENT_NP_OVINAX_STICKY_WEB, 22000);
        scriptEvents.addEvent(EVENT_NP_OVINAX_VILE_DISCHARGE, 30000);
    }

    void onDisengage() override { mDosedSpots.clear(); }

    // full energy: he ingests Black Blood and it overflows shortly after
    void onCombatTick(uint32_t diff) override
    {
        if (!mEnergy.fill(diff) || _isCasting())
            return;

        mEnergy.reset(0, OvinaxEnergyStepMs);
        warn("Broodtwister Ovi'nax ingests Black Blood!");
        castSpellOnSelf(SPELL_OVINAX_INGEST_BLACK_BLOOD);
        scriptEvents.addEvent(EVENT_NP_OVINAX_SANGUINE_OVERFLOW, 4000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NP_OVINAX_VOLATILE_CONCOCTION:
                castSpellOnVictim(SPELL_OVINAX_VOLATILE_CONCOCTION);
                scriptEvents.addEvent(EVENT_NP_OVINAX_VOLATILE_CONCOCTION, 20000);
                break;
            case EVENT_NP_OVINAX_EXPERIMENTAL_DOSAGE:
                castSpellAOE(SPELL_OVINAX_EXPERIMENTAL_DOSAGE);
                mDosedSpots.clear();
                for (Unit* pTarget : pickPlayers(3, true))
                    mDosedSpots.push_back(pTarget->GetPosition());
                scriptEvents.addEvent(EVENT_NP_OVINAX_EGGS_HATCH, 8000);
                scriptEvents.addEvent(EVENT_NP_OVINAX_EXPERIMENTAL_DOSAGE, 50000);
                break;
            case EVENT_NP_OVINAX_EGGS_HATCH:
                hatchEggs();
                break;
            case EVENT_NP_OVINAX_STICKY_WEB:
                castAtPlayers(SPELL_OVINAX_STICKY_WEB, 2, true);
                scriptEvents.addEvent(EVENT_NP_OVINAX_STICKY_WEB, 30000);
                break;
            case EVENT_NP_OVINAX_VILE_DISCHARGE:
                castBelowPlayer(SPELL_OVINAX_VILE_DISCHARGE);
                scriptEvents.addEvent(EVENT_NP_OVINAX_VILE_DISCHARGE, 25000);
                break;
            case EVENT_NP_OVINAX_SANGUINE_OVERFLOW:
                castAtPlayers(SPELL_OVINAX_SANGUINE_OVERFLOW, 2);
                break;
            default:
                break;
        }
    }

private:
    // each dosed spot hatches the next kind of experiment
    void hatchEggs()
    {
        if (mDosedSpots.empty())
            return;

        warn("The Experimental Dosage hatches nearby eggs!");
        for (LocationVector const& spot : mDosedSpots)
        {
            uint32_t const kind = mBroodTurn % 3;
            ++mBroodTurn;

            for (uint32_t i = 0; i < BroodCounts[kind]; ++i)
                callAdd(BroodEntries[kind], nerubarPointNear(spot, 2.0f, 6.0f));
        }

        mDosedSpots.clear();
    }

    NerubarMeter mEnergy;
    uint32_t mBroodTurn = 0;
    std::vector<LocationVector> mDosedSpots;
};

// fixates a player and infests it on arrival
class NerubarPalaceBloodParasiteAI : public NerubarPalaceCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NerubarPalaceBloodParasiteAI(c); }
    explicit NerubarPalaceBloodParasiteAI(Creature* pCreature) : NerubarPalaceCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        if (Unit* pTarget = pickPlayer(true))
        {
            castSpell(pTarget, SPELL_BLOOD_PARASITE_FIXATE, true);
            addThreat(pTarget, 100000.0f);
        }

        scriptEvents.addEvent(EVENT_NP_PARASITE_REACH, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_NP_PARASITE_REACH)
            return;

        Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
        if (pVictim != nullptr && getCreature()->getDistance(pVictim) <= 4.0f)
        {
            castSpell(pVictim, SPELL_BLOOD_PARASITE_INFEST, true);
            despawn(1500, 0);
            return;
        }

        scriptEvents.addEvent(EVENT_NP_PARASITE_REACH, 1000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Nexus-Princess Ky'veza

class NerubarPalaceKyvezaAI : public NerubarPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NerubarPalaceKyvezaAI(c); }
    explicit NerubarPalaceKyvezaAI(Creature* pCreature) : NerubarPalaceBossAI(pCreature, DATA_KYVEZA) {}

protected:
    void onEngage() override
    {
        mNightsFallen = 0;
        castSpellOnSelf(SPELL_KYVEZA_REAPER, true);
        beginPhantomBlade();
    }

    // full energy brings a Starless Night
    void onCombatTick(uint32_t diff) override
    {
        if (mStage == KyvezaPhantomBlade && mEnergy.fill(diff) && !_isCasting())
            beginStarlessNight();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NP_KYVEZA_VOID_SHREDDERS:
                castSpellOnVictim(SPELL_KYVEZA_VOID_SHREDDERS);
                scriptEvents.addEvent(EVENT_NP_KYVEZA_VOID_SHREDDERS, 16000);
                break;
            case EVENT_NP_KYVEZA_ASSASSINATION:
                markForAssassination();
                scriptEvents.addEvent(EVENT_NP_KYVEZA_ASSASSINATION, 50000);
                break;
            case EVENT_NP_KYVEZA_NETHER_RIFT:
                castSpellAOE(SPELL_KYVEZA_NETHER_RIFT);
                scriptEvents.addEvent(EVENT_NP_KYVEZA_NETHER_RIFT, 30000);
                break;
            case EVENT_NP_KYVEZA_TWILIGHT_MASSACRE:
                warn("Nexus-Princess Ky'veza begins a Twilight Massacre!");
                castSpellAOE(SPELL_KYVEZA_TWILIGHT_MASSACRE);
                scriptEvents.addEvent(EVENT_NP_KYVEZA_TWILIGHT_MASSACRE, 34000);
                break;
            case EVENT_NP_KYVEZA_NEXUS_DAGGERS:
                castSpellAOE(SPELL_KYVEZA_NEXUS_DAGGERS);
                scriptEvents.addEvent(EVENT_NP_KYVEZA_NEXUS_DAGGERS, 30000);
                break;
            case EVENT_NP_KYVEZA_REGICIDE:
                castSpellAOE(SPELL_KYVEZA_REGICIDE);
                scriptEvents.addEvent(EVENT_NP_KYVEZA_REGICIDE, 8000);
                break;
            case EVENT_NP_KYVEZA_NIGHT_FADES:
                beginPhantomBlade();
                break;
            default:
                break;
        }
    }

private:
    void beginPhantomBlade()
    {
        switchStage(KyvezaPhantomBlade);
        mEnergy.reset(0, KyvezaEnergyStepMs);

        scriptEvents.addEvent(EVENT_NP_KYVEZA_VOID_SHREDDERS, 6000);
        scriptEvents.addEvent(EVENT_NP_KYVEZA_ASSASSINATION, 14000);
        scriptEvents.addEvent(EVENT_NP_KYVEZA_NETHER_RIFT, 22000);
        scriptEvents.addEvent(EVENT_NP_KYVEZA_TWILIGHT_MASSACRE, 34000);
        scriptEvents.addEvent(EVENT_NP_KYVEZA_NEXUS_DAGGERS, 46000);
    }

    // the third night never ends
    void beginStarlessNight()
    {
        switchStage(KyvezaStarlessNight);
        ++mNightsFallen;
        scriptEvents.addEvent(EVENT_NP_KYVEZA_REGICIDE, 6000);

        if (mNightsFallen >= KyvezaNightsBeforeEternal)
        {
            warn("Nexus-Princess Ky'veza calls an Eternal Night!");
            castSpellAOE(SPELL_KYVEZA_ETERNAL_NIGHT);
            return;
        }

        warn("Nexus-Princess Ky'veza invokes a Starless Night!");
        castSpellAOE(SPELL_KYVEZA_STARLESS_NIGHT);
        scriptEvents.addEvent(EVENT_NP_KYVEZA_NIGHT_FADES, 25000);
    }

    // each marked player draws a Nether Phantom out of her shadow
    void markForAssassination()
    {
        castSpellAOE(SPELL_KYVEZA_ASSASSINATION);
        for (Unit* pTarget : pickPlayers(isHeroic() ? 3 : 2, true))
            callAdd(NPC_NETHER_PHANTOM, nerubarPointNear(pTarget->GetPosition(), 3.0f, 6.0f), TIMED_DESPAWN, NetherPhantomLifetimeMs);
    }

    NerubarMeter mEnergy;
    uint32_t mNightsFallen = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Silken Court: the instance keeps the stage, both lords follow it

class NerubarPalaceCourtLordAI : public NerubarPalaceBossAI
{
public:
    explicit NerubarPalaceCourtLordAI(Creature* pCreature) : NerubarPalaceBossAI(pCreature, DATA_SILKEN_COURT) {}

    void DoAction(int32_t action) override
    {
        if (action == ACTION_NP_COURT_STAGE && _isInCombat())
            enterCourtStage(courtStage());
    }

protected:
    void onEngage() override
    {
        armBerserk(SilkenCourtBerserkMs);
        mMeter.reset(0, CourtEnergyStepMs);

        // the Queen binds both fates together
        if (NerubarPalaceInstanceScript* pPalace = getPalace())
        {
            Creature* pPartner = pPalace->getCourtPartner(getCreature());
            if (pPartner != nullptr && pPartner->isAlive())
                castSpell(pPartner, SPELL_COURT_QUEENS_PROCLAMATION, true);
        }

        enterCourtStage(courtStage());
    }

    virtual void onCourtStage(uint32_t previous, uint32_t stage) = 0;

    uint32_t courtStage()
    {
        NerubarPalaceInstanceScript* pPalace = getPalace();
        return pPalace != nullptr ? pPalace->getCourtStage() : CourtClashOfRivals;
    }

    void enterCourtStage(uint32_t stage)
    {
        uint32_t const previous = mStage;
        switchStage(stage);
        onCourtStage(previous, stage);
    }

    // a full meter in the own stage opens the next intermission
    void fillCourtMeter(uint32_t diff, uint32_t ownStage, uint32_t intermission)
    {
        if (mStage != ownStage || !mMeter.fill(diff) || _isCasting())
            return;

        mMeter.reset(0, CourtEnergyStepMs);
        if (NerubarPalaceInstanceScript* pPalace = getPalace())
            pPalace->openCourtIntermission(intermission);
    }

    NerubarMeter mMeter;
};

class NerubarPalaceAnubarashAI : public NerubarPalaceCourtLordAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NerubarPalaceAnubarashAI(c); }
    explicit NerubarPalaceAnubarashAI(Creature* pCreature) : NerubarPalaceCourtLordAI(pCreature) {}

protected:
    void onDisengage() override { _removeAura(SPELL_ANUBARASH_SPIKE_STORM); }

    // his rage builds during Grasp of the Void
    void onCombatTick(uint32_t diff) override { fillCourtMeter(diff, CourtGraspOfTheVoid, CourtRagingFury); }

    void onCourtStage(uint32_t previous, uint32_t stage) override
    {
        if (previous == CourtRagingFury && stage != CourtRagingFury)
            _removeAura(SPELL_ANUBARASH_SPIKE_STORM);

        switch (stage)
        {
            case CourtClashOfRivals:
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_PIERCING_STRIKE, 6000);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_IMPALING_ERUPTION, 12000);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_CALL_OF_THE_SWARM, 18000);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_RECKLESS_CHARGE, 30000);
                break;
            case CourtVoidAscension:
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_PIERCING_STRIKE, 6000);
                break;
            case CourtGraspOfTheVoid:
                mMeter.reset(0, CourtEnergyStepMs);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_PIERCING_STRIKE, 6000);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_STINGING_SWARM, 15000);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_IMPALING_ERUPTION, 22000);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_CALL_OF_THE_SWARM, 30000);
                break;
            case CourtRagingFury:
                warn("Anub'arash erupts in a Spike Storm!");
                castSpellOnSelf(SPELL_ANUBARASH_SPIKE_STORM);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_SEISMIC_UPHEAVAL, 4000);
                break;
            default:
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_PIERCING_STRIKE, 6000);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_STINGING_SWARM, 12000);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_RECKLESS_CHARGE, 18000);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_SPIKE_ERUPTION, 24000);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_UNLEASHED_SWARM, 36000);
                break;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NP_ANUBARASH_PIERCING_STRIKE:
                castSpellOnVictim(SPELL_ANUBARASH_PIERCING_STRIKE);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_PIERCING_STRIKE, 14000);
                break;
            case EVENT_NP_ANUBARASH_IMPALING_ERUPTION:
                castSpellOnVictim(SPELL_ANUBARASH_IMPALING_ERUPTION);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_IMPALING_ERUPTION, 22000);
                break;
            case EVENT_NP_ANUBARASH_CALL_OF_THE_SWARM:
                castSpellAOE(SPELL_ANUBARASH_CALL_OF_THE_SWARM);
                for (uint8_t i = 0; i < (isHeroic() ? 3 : 2); ++i)
                    callAdd(NPC_SHATTERSHELL_SCARAB, nearSelf(5.0f, 12.0f));
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_CALL_OF_THE_SWARM, 45000);
                break;
            case EVENT_NP_ANUBARASH_RECKLESS_CHARGE:
                warn("Anub'arash prepares a Reckless Charge!");
                castBelowPlayer(mStage == CourtUnleashedRage ? SPELL_ANUBARASH_RAGING_CHARGE : SPELL_ANUBARASH_RECKLESS_CHARGE, true);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_BURROWED_ERUPTION, 3000);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_RECKLESS_CHARGE, mStage == CourtUnleashedRage ? 35000 : 40000);
                break;
            case EVENT_NP_ANUBARASH_BURROWED_ERUPTION:
                castSpellAOE(SPELL_ANUBARASH_BURROWED_ERUPTION);
                break;
            case EVENT_NP_ANUBARASH_STINGING_SWARM:
                castSpellAOE(SPELL_ANUBARASH_STINGING_SWARM);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_STINGING_SWARM, 45000);
                break;
            case EVENT_NP_ANUBARASH_SEISMIC_UPHEAVAL:
                castSpellAOE(SPELL_ANUBARASH_SEISMIC_UPHEAVAL);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_SEISMIC_UPHEAVAL, 5000);
                break;
            case EVENT_NP_ANUBARASH_SPIKE_ERUPTION:
                castBelowPlayer(SPELL_ANUBARASH_SPIKE_ERUPTION);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_SPIKE_ERUPTION, 30000);
                break;
            case EVENT_NP_ANUBARASH_UNLEASHED_SWARM:
                warn("Anub'arash unleashes the swarm!");
                castSpellAOE(SPELL_ANUBARASH_UNLEASHED_SWARM);
                scriptEvents.addEvent(EVENT_NP_ANUBARASH_UNLEASHED_SWARM, 40000);
                break;
            default:
                break;
        }
    }
};

class NerubarPalaceTakazjAI : public NerubarPalaceCourtLordAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NerubarPalaceTakazjAI(c); }
    explicit NerubarPalaceTakazjAI(Creature* pCreature) : NerubarPalaceCourtLordAI(pCreature) {}

protected:
    void onDisengage() override { _removeAura(SPELL_TAKAZJ_SHATTER_EXISTENCE); }

    // her void energy builds during the Clash of Rivals
    void onCombatTick(uint32_t diff) override { fillCourtMeter(diff, CourtClashOfRivals, CourtVoidAscension); }

    void onCourtStage(uint32_t previous, uint32_t stage) override
    {
        if (previous == CourtVoidAscension && stage != CourtVoidAscension)
            _removeAura(SPELL_TAKAZJ_SHATTER_EXISTENCE);

        switch (stage)
        {
            case CourtClashOfRivals:
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_POISON_BOLT, 3000);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_WEB_BOMB, 15000);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_VENOMOUS_RAIN, 22000);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_SKITTERING_LEAP, 28000);
                break;
            case CourtVoidAscension:
                warn("Skeinspinner Takazj ascends into void form!");
                castSpellOnSelf(SPELL_TAKAZJ_SHATTER_EXISTENCE);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_ENTROPIC_BARRAGE, 3000);
                break;
            case CourtRagingFury:
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_VOID_BOLT, 3000);
                break;
            default:
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_VOID_BOLT, 3000);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_STRANDS_OF_REALITY, 12000);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_VOID_STEP, 18000);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_WEB_VORTEX, 20000);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_ENTROPIC_DESOLATION, 28000);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_CATACLYSMIC_ENTROPY, 40000);
                break;
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NP_TAKAZJ_POISON_BOLT:
                castSpellOnVictim(SPELL_TAKAZJ_POISON_BOLT);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_POISON_BOLT, 5000);
                break;
            case EVENT_NP_TAKAZJ_WEB_BOMB:
                castBelowPlayer(SPELL_TAKAZJ_WEB_BOMB, true);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_WEB_BOMB, 35000);
                break;
            case EVENT_NP_TAKAZJ_VENOMOUS_RAIN:
                castAtPlayer(SPELL_TAKAZJ_VENOMOUS_RAIN, true);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_VENOMOUS_RAIN, 30000);
                break;
            case EVENT_NP_TAKAZJ_SKITTERING_LEAP:
                castBelowPlayer(SPELL_TAKAZJ_SKITTERING_LEAP, true);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_SKITTERING_LEAP, 25000);
                break;
            case EVENT_NP_TAKAZJ_ENTROPIC_BARRAGE:
                castAtPlayers(SPELL_TAKAZJ_ENTROPIC_BARRAGE, 2);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_ENTROPIC_BARRAGE, 4000);
                break;
            case EVENT_NP_TAKAZJ_VOID_BOLT:
                castSpellOnVictim(SPELL_TAKAZJ_VOID_BOLT);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_VOID_BOLT, 5000);
                break;
            case EVENT_NP_TAKAZJ_STRANDS_OF_REALITY:
                castSpellAOE(SPELL_TAKAZJ_STRANDS_OF_REALITY);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_STRANDS_OF_REALITY, 30000);
                break;
            case EVENT_NP_TAKAZJ_VOID_STEP:
                castBelowPlayer(SPELL_TAKAZJ_VOID_STEP, true);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_VOID_STEP, 25000);
                break;
            case EVENT_NP_TAKAZJ_WEB_VORTEX:
                castSpellAOE(SPELL_TAKAZJ_WEB_VORTEX);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_WEB_VORTEX, 40000);
                break;
            case EVENT_NP_TAKAZJ_ENTROPIC_DESOLATION:
                castSpellAOE(SPELL_TAKAZJ_ENTROPIC_DESOLATION);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_ENTROPIC_DESOLATION, 35000);
                break;
            case EVENT_NP_TAKAZJ_CATACLYSMIC_ENTROPY:
                warn("Skeinspinner Takazj channels Cataclysmic Entropy!");
                castSpellAOE(SPELL_TAKAZJ_CATACLYSMIC_ENTROPY);
                scriptEvents.addEvent(EVENT_NP_TAKAZJ_CATACLYSMIC_ENTROPY, 50000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Queen Ansurek

class NerubarPalaceQueenAnsurekAI : public NerubarPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NerubarPalaceQueenAnsurekAI(c); }
    explicit NerubarPalaceQueenAnsurekAI(Creature* pCreature) : NerubarPalaceBossAI(pCreature, DATA_QUEEN_ANSUREK) {}

protected:
    void onEngage() override
    {
        armBerserk(AnsurekBerserkMs);
        beginQueensVenom();
    }

    void onDisengage() override
    {
        setRooted(false);
        _removeAura(SPELL_ANSUREK_ACRID_PRESENCE);
        _removeAura(SPELL_ANSUREK_DREADFUL_PRESENCE);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == AnsurekQueensVenom && _getHealthPercent() <= AnsurekPredationPct && !_isCasting())
            beginSpidersWeb();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_NP_ANSUREK_LIQUEFY:
                castSpellOnVictim(SPELL_ANSUREK_LIQUEFY);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_FEAST, 3000);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_LIQUEFY, 40000);
                break;
            case EVENT_NP_ANSUREK_FEAST:
                castSpellOnVictim(SPELL_ANSUREK_FEAST);
                break;
            case EVENT_NP_ANSUREK_SILKEN_TOMB:
                castSpellAOE(SPELL_ANSUREK_SILKEN_TOMB);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_SILKEN_TOMB, 50000);
                break;
            case EVENT_NP_ANSUREK_REACTIVE_TOXIN:
                castSpellAOE(SPELL_ANSUREK_REACTIVE_TOXIN);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_REACTIVE_TOXIN, 50000);
                break;
            case EVENT_NP_ANSUREK_WEB_BLADES:
                castAtPlayer(SPELL_ANSUREK_WEB_BLADES);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_WEB_BLADES, mStage == AnsurekQueensVenom ? 30000 : 35000);
                break;
            case EVENT_NP_ANSUREK_VENOM_NOVA:
                warn("Queen Ansurek unleashes a Venom Nova!");
                castSpellAOE(SPELL_ANSUREK_VENOM_NOVA);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_VENOM_NOVA, 55000);
                break;
            case EVENT_NP_ANSUREK_PARALYZING_VENOM:
                castSpellAOE(SPELL_ANSUREK_PARALYZING_VENOM);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_PARALYZING_VENOM, 6000);
                break;
            case EVENT_NP_ANSUREK_WREST:
                castSpellAOE(SPELL_ANSUREK_WREST);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_WREST, 7000);
                break;
            case EVENT_NP_ANSUREK_PREDATION_ENDS:
                beginRoyalAscension();
                break;
            case EVENT_NP_ANSUREK_COURT_WAVE:
                callCourtWave();
                break;
            case EVENT_NP_ANSUREK_ACID_BOLT:
                castAtPlayer(SPELL_ANSUREK_ACID_BOLT);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_ACID_BOLT, 3500);
                break;
            case EVENT_NP_ANSUREK_ASCENDED_WREST:
                castBelowPlayer(SPELL_ANSUREK_ASCENDED_WREST);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_ASCENDED_WREST, 15000);
                break;
            case EVENT_NP_ANSUREK_ASCENSION_ENDS:
                beginParanoiasFeast();
                break;
            case EVENT_NP_ANSUREK_GORGE:
                castSpellOnVictim(SPELL_ANSUREK_GORGE);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_GORGE, 20000);
                break;
            case EVENT_NP_ANSUREK_INFEST:
                castSpellOnVictim(SPELL_ANSUREK_INFEST);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_GLOOM_BURST, 6000);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_INFEST, 35000);
                break;
            case EVENT_NP_ANSUREK_GLOOM_BURST:
                burstGloomHatchlings();
                break;
            case EVENT_NP_ANSUREK_ABYSSAL_INFUSION:
                castSpellAOE(SPELL_ANSUREK_ABYSSAL_INFUSION);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_ABYSSAL_INFUSION, 60000);
                break;
            case EVENT_NP_ANSUREK_QUEENS_SUMMONS:
                warn("Queen Ansurek issues the Queen's Summons!");
                for (uint8_t i = 0; i < 4; ++i)
                    callAdd(NPC_SUMMONED_ACOLYTE, nearSelf(15.0f, 25.0f));
                scriptEvents.addEvent(EVENT_NP_ANSUREK_QUEENS_SUMMONS, 70000);
                break;
            case EVENT_NP_ANSUREK_FROTHING_GLUTTONY:
                warn("Queen Ansurek begins Frothing Gluttony!");
                castSpellAOE(SPELL_ANSUREK_FROTHING_GLUTTONY);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_FROTHING_GLUTTONY, 70000);
                break;
            case EVENT_NP_ANSUREK_ROYAL_CONDEMNATION:
                warn("Queen Ansurek pronounces Royal Condemnation!");
                castSpellAOE(SPELL_ANSUREK_ROYAL_CONDEMNATION);
                scriptEvents.addEvent(EVENT_NP_ANSUREK_ROYAL_CONDEMNATION, 55000);
                break;
            default:
                break;
        }
    }

private:
    void beginQueensVenom()
    {
        switchStage(AnsurekQueensVenom);
        castSpellOnSelf(SPELL_ANSUREK_ACRID_PRESENCE, true);

        scriptEvents.addEvent(EVENT_NP_ANSUREK_LIQUEFY, 8000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_SILKEN_TOMB, 12000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_REACTIVE_TOXIN, 18000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_WEB_BLADES, 22000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_VENOM_NOVA, 30000);
    }

    // she holds still in her web and feeds
    void beginSpidersWeb()
    {
        switchStage(AnsurekSpidersWeb);
        _removeAura(SPELL_ANSUREK_ACRID_PRESENCE);
        warn("Queen Ansurek shrouds herself in webbing and begins to feed!");
        setRooted(true);
        castSpellAOE(SPELL_ANSUREK_PREDATION);

        scriptEvents.addEvent(EVENT_NP_ANSUREK_PARALYZING_VENOM, 4000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_WREST, 7000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_PREDATION_ENDS, 20000);
    }

    // casts and chamber guard waves only, no platform travel
    void beginRoyalAscension()
    {
        switchStage(AnsurekRoyalAscension);
        setRooted(false);
        mCourtWave = 0;
        warn("Queen Ansurek ascends and calls her chamber guard!");

        scriptEvents.addEvent(EVENT_NP_ANSUREK_COURT_WAVE, 2000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_ACID_BOLT, 4000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_ASCENDED_WREST, 10000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_ASCENSION_ENDS, 60000);
    }

    void beginParanoiasFeast()
    {
        switchStage(AnsurekParanoiasFeast);
        warn("Queen Ansurek embraces the void as paranoia takes hold!");
        castSpellOnSelf(SPELL_ANSUREK_DREADFUL_PRESENCE, true);

        scriptEvents.addEvent(EVENT_NP_ANSUREK_GORGE, 6000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_INFEST, 12000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_ABYSSAL_INFUSION, 15000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_WEB_BLADES, 20000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_QUEENS_SUMMONS, 25000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_FROTHING_GLUTTONY, 35000);
        scriptEvents.addEvent(EVENT_NP_ANSUREK_ROYAL_CONDEMNATION, 45000);
    }

    void callCourtWave()
    {
        if (mCourtWave >= 3)
            return;

        AnsurekCourtWave const& wave = AnsurekCourtWaves[mCourtWave];
        for (uint32_t i = 0; i < wave.firstCount; ++i)
            callAdd(wave.firstEntry, nearSelf(12.0f, 22.0f));
        for (uint32_t i = 0; i < wave.secondCount; ++i)
            callAdd(wave.secondEntry, nearSelf(12.0f, 22.0f));

        if (++mCourtWave < 3)
            scriptEvents.addEvent(EVENT_NP_ANSUREK_COURT_WAVE, 18000);
    }

    // the infested tank bursts into Gloom Hatchlings
    void burstGloomHatchlings()
    {
        Unit* pVictim = getCreature()->getAIInterface()->getCurrentTarget();
        LocationVector const spot = pVictim != nullptr ? pVictim->GetPosition() : getCreature()->GetPosition();
        for (uint8_t i = 0; i < 2; ++i)
            callAdd(NPC_GLOOM_HATCHLING, nerubarPointNear(spot, 2.0f, 5.0f));
    }

    uint32_t mCourtWave = 0;
};

void SetupNerubarPalace(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_NERUBAR_PALACE, &NerubarPalaceInstanceScript::Create);

    // Sikran, Captain of the Sureki
    mgr->register_creature_script(BOSS_SIKRAN, &NerubarPalaceSikranAI::Create);
    mgr->register_creature_script(BOSS_SIKRAN_ALT, &NerubarPalaceSikranAI::Create);
    mgr->register_creature_script(NPC_COSMIC_SIMULACRUM, &NerubarPalaceCosmicSimulacrumAI::Create);

    // Ulgrax the Devourer
    mgr->register_creature_script(BOSS_ULGRAX, &NerubarPalaceUlgraxAI::Create);
    mgr->register_creature_script(NPC_RAVENOUS_SPAWN, &NerubarPalaceKitAI::CreateRavenousSpawn);

    // The Bloodbound Horror
    mgr->register_creature_script(BOSS_BLOODBOUND_HORROR, &NerubarPalaceBloodboundHorrorAI::Create);
    mgr->register_creature_script(NPC_LOST_WATCHER, &NerubarPalaceKitAI::CreateLostWatcher);
    mgr->register_creature_script(NPC_FORGOTTEN_HARBINGER, &NerubarPalaceForgottenHarbingerAI::Create);

    // Rasha'nan
    mgr->register_creature_script(BOSS_RASHANAN, &NerubarPalaceRashananAI::Create);
    mgr->register_creature_script(BOSS_RASHANAN_ALT, &NerubarPalaceRashananAI::Create);
    mgr->register_creature_script(NPC_INFESTED_SPAWN, &NerubarPalaceKitAI::CreateInfestedSpawn);

    // Broodtwister Ovi'nax
    mgr->register_creature_script(BOSS_OVINAX, &NerubarPalaceOvinaxAI::Create);
    mgr->register_creature_script(NPC_COLOSSAL_SPIDER, &NerubarPalaceKitAI::CreateColossalSpider);
    mgr->register_creature_script(NPC_VORACIOUS_WORM, &NerubarPalaceKitAI::CreateVoraciousWorm);
    mgr->register_creature_script(NPC_BLOOD_PARASITE, &NerubarPalaceBloodParasiteAI::Create);

    // Nexus-Princess Ky'veza
    mgr->register_creature_script(BOSS_KYVEZA, &NerubarPalaceKyvezaAI::Create);
    mgr->register_creature_script(BOSS_KYVEZA_ALT, &NerubarPalaceKyvezaAI::Create);
    mgr->register_creature_script(NPC_NETHER_PHANTOM, &NerubarPalaceKitAI::CreateNetherPhantom);

    // The Silken Court
    mgr->register_creature_script(BOSS_ANUBARASH, &NerubarPalaceAnubarashAI::Create);
    mgr->register_creature_script(BOSS_TAKAZJ, &NerubarPalaceTakazjAI::Create);

    // Queen Ansurek
    mgr->register_creature_script(BOSS_QUEEN_ANSUREK, &NerubarPalaceQueenAnsurekAI::Create);
    mgr->register_creature_script(NPC_CHAMBER_GUARDIAN, &NerubarPalaceKitAI::CreateChamberGuardian);
    mgr->register_creature_script(NPC_CHAMBER_EXPELLER, &NerubarPalaceKitAI::CreateChamberExpeller);
    mgr->register_creature_script(NPC_CHAMBER_ACOLYTE, &NerubarPalaceKitAI::CreateChamberAcolyte);
    mgr->register_creature_script(NPC_ASCENDED_VOIDSPEAKER, &NerubarPalaceKitAI::CreateVoidspeaker);
    mgr->register_creature_script(NPC_DEVOTED_WORSHIPPER, &NerubarPalaceKitAI::CreateWorshipper);
    mgr->register_creature_script(NPC_CAUSTIC_SKITTERER, &NerubarPalaceKitAI::CreateCausticSkitterer);
    mgr->register_creature_script(NPC_SUMMONED_ACOLYTE, &NerubarPalaceKitAI::CreateSummonedAcolyte);
}
