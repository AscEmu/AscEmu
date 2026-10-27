/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_The_Eternal_Palace.hpp"

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
#include <list>
#include <set>
#include <utility>
#include <vector>

enum EternalPalaceEvents : uint32_t
{
    // shared
    EVENT_EP_REPEAT_SPELL = 1,

    // Blackwater Behemoth
    EVENT_BEHEMOTH_FEELERS,
    EVENT_BEHEMOTH_TOXIC_SPINE,
    EVENT_BEHEMOTH_SPINE_VOLLEY,
    EVENT_BEHEMOTH_SHOCK_PULSE,
    EVENT_BEHEMOTH_NEXT_PLATFORM,
    EVENT_BEHEMOTH_CAVITATION,

    // Za'qul
    EVENT_ZAQUL_DARK_BEYOND,
    EVENT_ZAQUL_MIND_TETHER,
    EVENT_ZAQUL_CRUSHING_GRASP,
    EVENT_ZAQUL_DREAD,
    EVENT_ZAQUL_PORTAL_OF_MADNESS,
    EVENT_ZAQUL_MADDENING_ERUPTION,
    EVENT_ZAQUL_MANIFEST_NIGHTMARES,
    EVENT_ZAQUL_DELIRIUM_REALM,
    EVENT_ZAQUL_MANIC_DREAD,
    EVENT_ZAQUL_DARK_PASSAGE,
    EVENT_ZAQUL_DARK_PULSE,
    EVENT_SUMMONER_DARK_TEAR,
    EVENT_SUMMONER_VOID_ERUPTION,
    EVENT_SUMMONER_FEARS_GATE,

    // Abyssal Commander Sivara
    EVENT_SIVARA_CHIMERIC_MARKS,
    EVENT_SIVARA_SPREAD_MARKS,
    EVENT_SIVARA_OVERFLOW,
    EVENT_SIVARA_OVERFLOW_SPILLS,
    EVENT_SIVARA_INVERSION,
    EVENT_SIVARA_INVERT_MARKS,
    EVENT_SIVARA_CRUSHING_REVERBERATION,
    EVENT_SIVARA_BOLT_VOLLEY,
    EVENT_SIVARA_OVERWHELMING_BARRAGE,

    // Queen Azshara
    EVENT_AZSHARA_DRAIN_WARDS,
    EVENT_AZSHARA_BECKON,
    EVENT_AZSHARA_ARCANE_ORBS,
    EVENT_AZSHARA_OVERZEALOUS_HULK,
    EVENT_AZSHARA_ISSUE_DECREES,
    EVENT_AZSHARA_NEXT_STAGE,
    EVENT_AZSHARA_ARCANE_JOLT,
    EVENT_AZSHARA_ARCANE_BURST,
    EVENT_AZSHARA_BURST_MARKS,
    EVENT_AZSHARA_ARCANE_DETONATION,
    EVENT_AZSHARA_SERVANTS,
    EVENT_AZSHARA_REVERSAL_OF_FORTUNE,
    EVENT_AZSHARA_TIDEMISTRESSES,
    EVENT_AZSHARA_LOYAL_MYRMIDON,
    EVENT_AZSHARA_VOID_TOUCHED,
    EVENT_AZSHARA_NETHER_PORTAL,
    EVENT_AZSHARA_PIERCING_GAZE,
    EVENT_AZSHARA_OVERLOAD,
    EVENT_AZSHARA_ENERGY_SPIKE,
    EVENT_LOVER_LONGING,
    EVENT_LOVER_PAINFUL_MEMORIES,
    EVENT_AETHANEL_CHAIN_LIGHTNING,
    EVENT_AETHANEL_LIGHTNING_ORBS,
    EVENT_CYRANUS_SERRATED_EDGE,
    EVENT_CYRANUS_CHARGED_SPEAR,
    EVENT_HULK_SEEK_WARD,
    EVENT_HULK_POUND_DONE,
    EVENT_TIDEMISTRESS_STATIC_SHOCK,
    EVENT_TIDEMISTRESS_CHAIN_LIGHTNING,
    EVENT_TIDEMISTRESS_CRYSTALLINE_SHIELD,
    EVENT_TIDEMISTRESS_DRAIN_WARDS,
    EVENT_LOYAL_MYRMIDON_CHARGED_SPEAR,

    // Orgozoa
    EVENT_ORGOZOA_INCUBATION_FLUID,
    EVENT_ORGOZOA_SOAK_FLUID,
    EVENT_ORGOZOA_ARCING_CURRENT,
    EVENT_ORGOZOA_DESENSITIZING_STING,
    EVENT_ORGOZOA_DRIBBLING_ICHOR,
    EVENT_ORGOZOA_HATCHLINGS,
    EVENT_ORGOZOA_INCUBATOR_FALLBACK,
    EVENT_ORGOZOA_HATCHERY_OPENS,
    EVENT_WITCH_SHOCKING_LIGHTNING,
    EVENT_WITCH_CONDUCTIVE_PULSE,

    // Lady Ashvane
    EVENT_ASHVANE_BARNACLE_BASH,
    EVENT_ASHVANE_UPSURGE,
    EVENT_ASHVANE_CORAL_GROWTH,
    EVENT_ASHVANE_RIPPLING_WAVE,
    EVENT_ASHVANE_BRINY_BUBBLE,
    EVENT_ASHVANE_TRAP_BUBBLES,
    EVENT_ASHVANE_ARCING_AZERITE,
    EVENT_ASHVANE_RESHELL,

    // Radiance of Azshara
    EVENT_RADIANCE_TIDE_FIST,
    EVENT_RADIANCE_ARCANADO_BURST,
    EVENT_RADIANCE_ARCANADO_STRIKES,
    EVENT_RADIANCE_UNSHACKLED_POWER,
    EVENT_RADIANCE_ARCANE_BOMB,
    EVENT_RADIANCE_PLANT_BOMBS,
    EVENT_RADIANCE_ANCIENT_TEMPEST,
    EVENT_RADIANCE_SQUALL_TRAP,
    EVENT_RADIANCE_CALM_RETURNS,
    EVENT_STORMWRAITH_GALE_BUFFET,
    EVENT_STORMWRAITH_STORMLINGS,

    // The Queen's Court
    EVENT_COURT_SEPARATION_CHECK,
    EVENT_COURT_MEASURES_OVER,
    EVENT_SILIVAZ_FRENETIC_CHARGE,
    EVENT_SILIVAZ_CHARGE_LEAP,
    EVENT_SILIVAZ_ZEALOUS_ERUPTION,
    EVENT_PASHMAR_FANATICAL_VERDICT,
    EVENT_PASHMAR_MARK_ZEALOTS,
    EVENT_PASHMAR_POTENT_SPARK,
    EVENT_PASHMAR_SCATTER_SPARKS,
    EVENT_PASHMAR_VIOLENT_OUTBURST
};

namespace
{
    float const PalaceTwoPi = 6.28318531f;

    // follow-up events wait for the cast bar in front of them
    uint32_t const PalaceFollowUpMs = 200;

    // hard enrage
    uint32_t const PalaceBerserkMs = 600000;
    uint32_t const AzsharaBerserkMs = 900000;

    size_t const PalaceRaidSize = 40;

    // movement point ids
    uint32_t const PointServantWard = 1;
    uint32_t const PointHulkWard = 2;
    uint32_t const PointOrgozoaDescent = 10;

    // stages
    uint32_t const RadianceStageCalm = 1;
    uint32_t const RadianceStageTempest = 2;

    uint32_t const AshvaneStageShelled = 1;
    uint32_t const AshvaneStageExposed = 2;

    uint32_t const OrgozoaStageNursery = 1;
    uint32_t const OrgozoaStageDescent = 2;
    uint32_t const OrgozoaStageHatchery = 3;

    uint32_t const AzsharaStageLovers = 1;
    uint32_t const AzsharaStageFirstDecree = 2;
    uint32_t const AzsharaStageSecond = 3;
    uint32_t const AzsharaStageSecondDecree = 4;
    uint32_t const AzsharaStageThird = 5;
    uint32_t const AzsharaStageFourth = 6;

    // Desperate Measures lasts at least this long
    uint32_t const CourtMeasuresMs = 3000;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Positions

    LocationVector const BehemothPlatforms[3] =
    {
        { -4.35f, 974.46f, 1255.90f, 3.21f },
        { -263.46f, 1099.83f, 1211.03f, 4.68f },
        { -448.11f, 788.86f, 1171.09f, 0.85f }
    };

    LocationVector const ZaqulArenaCentre(197.69f, 2012.56f, 755.92f, 0.0f);
    float const ZaqulArenaRadius = 43.0f;

    LocationVector const RadianceStormwraithSpots[2] =
    {
        { 734.86f, 588.79f, 1295.96f, 0.0f },
        { 739.83f, 685.29f, 1295.96f, 0.0f }
    };

    LocationVector const RadianceSquallTraps[17] =
    {
        { 845.45f, 623.99f, 1295.96f, 0.0f },
        { 838.16f, 623.64f, 1295.96f, 0.0f },
        { 831.01f, 623.29f, 1295.96f, 0.0f },
        { 825.02f, 622.99f, 1295.96f, 0.0f },
        { 818.60f, 622.68f, 1295.96f, 0.0f },
        { 811.80f, 622.34f, 1295.96f, 0.0f },
        { 804.77f, 622.02f, 1295.96f, 0.0f },
        { 771.90f, 600.62f, 1295.96f, 0.0f },
        { 770.58f, 593.38f, 1295.96f, 0.0f },
        { 769.34f, 586.53f, 1295.96f, 0.0f },
        { 768.00f, 579.16f, 1295.96f, 0.0f },
        { 766.57f, 571.25f, 1296.20f, 0.0f },
        { 739.56f, 656.40f, 1295.96f, 0.0f },
        { 732.63f, 661.55f, 1295.96f, 0.0f },
        { 726.31f, 666.24f, 1295.96f, 0.0f },
        { 720.05f, 670.88f, 1295.96f, 0.0f },
        { 713.98f, 675.49f, 1296.92f, 0.0f }
    };

    LocationVector const AshvaneArenaCentre(298.22f, 634.11f, 1284.72f, 0.0f);

    LocationVector const OrgozoaNurserySpots[2] =
    {
        { 236.22f, 707.51f, 1060.93f, 0.0f },
        { 290.85f, 700.58f, 1060.94f, 0.0f }
    };

    LocationVector const OrgozoaHatcherySpots[2] =
    {
        { 192.17f, 823.58f, 1010.77f, 0.0f },
        { 252.74f, 818.31f, 1010.98f, 0.0f }
    };

    // path from the nursery down to the hatchery
    LocationVector const OrgozoaDescent[8] =
    {
        { 268.44f, 721.32f, 1060.82f, 0.0f },
        { 272.97f, 741.00f, 1060.25f, 0.0f },
        { 326.45f, 737.38f, 1051.20f, 0.0f },
        { 346.47f, 762.44f, 1043.44f, 0.0f },
        { 326.42f, 798.75f, 1034.46f, 0.0f },
        { 212.08f, 774.73f, 1010.85f, 0.0f },
        { 211.57f, 809.72f, 1010.75f, 0.0f },
        { 220.20f, 818.42f, 1010.81f, 0.0f }
    };

    uint32_t const OrgozoaDescentSteps = 8;

    LocationVector const CourtArenaCentre(196.03f, 1443.84f, 858.91f, 0.0f);
    LocationVector const CourtDecreeSpot(196.29f, 1393.21f, 869.79f, 0.0f);

    LocationVector const AzsharaArenaCentre(216.65f, 2444.94f, 710.65f, 0.0f);
    LocationVector const AzsharaStageFront(223.30f, 2482.45f, 710.81f, 0.0f);
    LocationVector const AzsharaAethanelSpot(229.15f, 2479.40f, 710.80f, 0.0f);
    LocationVector const AzsharaCyranusSpot(197.82f, 2464.69f, 710.77f, 0.0f);

    LocationVector const AzsharaWardSpots[3] =
    {
        { 182.32f, 2454.58f, 710.79f, 0.0f },
        { 250.27f, 2454.62f, 710.79f, 0.0f },
        { 216.27f, 2410.97f, 710.78f, 0.0f }
    };

    LocationVector const AzsharaServantSpots[4] =
    {
        { 206.12f, 2408.97f, 710.80f, 0.0f },
        { 200.15f, 2411.34f, 710.80f, 0.0f },
        { 227.28f, 2407.95f, 710.80f, 0.0f },
        { 232.28f, 2409.54f, 710.81f, 0.0f }
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    // Doors

    enum class PalaceDoorKind : uint8_t
    {
        Room,       // shut while the encounter runs
        Passage,    // opens after the encounter
        Wall        // opens after the first wing
    };

    struct PalaceDoorRule
    {
        uint32_t entry;
        uint32_t dataId;
        PalaceDoorKind kind;
    };

    PalaceDoorRule const PalaceDoorRules[] =
    {
        { GO_SIVARA_ROOM_DOOR,      DATA_ABYSSAL_COMMANDER_SIVARA,  PalaceDoorKind::Room },
        { GO_SIVARA_RIGHT_GATE,     DATA_ABYSSAL_COMMANDER_SIVARA,  PalaceDoorKind::Passage },
        { GO_SIVARA_LEFT_GATE,      DATA_ABYSSAL_COMMANDER_SIVARA,  PalaceDoorKind::Passage },
        { GO_ASHVANE_ROOM_DOOR,     DATA_LADY_ASHVANE,              PalaceDoorKind::Room },
        { GO_ASHVANE_ARCANE_WALL,   DATA_LADY_ASHVANE,              PalaceDoorKind::Wall }
    };

    uint32_t const PalaceFirstWing[] = { DATA_ABYSSAL_COMMANDER_SIVARA, DATA_BLACKWATER_BEHEMOTH, DATA_RADIANCE_OF_AZSHARA };

    //////////////////////////////////////////////////////////////////////////////////////////
    // The Queen's Court

    uint32_t const CourtMembers[2] = { BOSS_SILIVAZ_THE_ZEALOUS, BOSS_PASHMAR_THE_FANATICAL };

    struct CourtDecree
    {
        uint32_t spellId;
        char const* text;
    };

    CourtDecree const CourtDecrees[] =
    {
        { SPELL_DECREE_STAND_ALONE,         "Queen Azshara decrees: Stand Alone!" },
        { SPELL_DECREE_REPEAT_PERFORMANCE,  "Queen Azshara decrees: Repeat Performance!" },
        { SPELL_DECREE_DEFERRED_SENTENCE,   "Queen Azshara decrees: Deferred Sentence!" },
        { SPELL_DECREE_OBEY_OR_SUFFER,      "Queen Azshara decrees: Obey or Suffer!" }
    };

    size_t const CourtDecreeCount = sizeof(CourtDecrees) / sizeof(CourtDecrees[0]);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Helpers

    enum class PalaceAim : uint8_t
    {
        Victim,
        RandomPlayer
    };

    // random point on a ring around origin
    LocationVector palaceScatter(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(PalaceTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    std::vector<Player*> palacePlayersNear(Creature* pCentre, float range)
    {
        std::vector<Player*> found;
        WorldMap* pMap = pCentre->getWorldMap();
        if (pMap == nullptr)
            return found;

        LocationVector const centre = pCentre->GetPosition();
        for (auto const& mapPlayer : pMap->getPlayers())
        {
            Player* pPlayer = mapPlayer.second;
            if (pPlayer != nullptr && pPlayer->isAlive() && pPlayer->GetPosition().getExactDist(centre) <= range)
                found.push_back(pPlayer);
        }

        return found;
    }

    // the player puts the aura on himself
    void palaceSelfApply(Player* pPlayer, uint32_t spellId)
    {
        pPlayer->castSpell(pPlayer, spellId, true);
    }

    // energy bar filled in random steps, empty again once full
    class PalaceGauge
    {
    public:
        PalaceGauge(uint32_t stepMs, uint32_t minGain, uint32_t maxGain) : mStepMs(stepMs), mMinGain(minGain), mMaxGain(maxGain) {}

        void reset()
        {
            mValue = 0;
            mStepLeftMs = mStepMs;
        }

        bool fill(uint32_t diff)
        {
            if (mStepLeftMs > diff)
            {
                mStepLeftMs -= diff;
                return false;
            }

            mStepLeftMs = mStepMs;
            mValue += Util::getRandomUInt(mMinGain, mMaxGain);
            if (mValue < 100)
                return false;

            mValue = 0;
            return true;
        }

    private:
        uint32_t mStepMs;
        uint32_t mMinGain;
        uint32_t mMaxGain;
        uint32_t mValue = 0;
        uint32_t mStepLeftMs = 0;
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class EternalPalaceInstanceScript : public InstanceScript
{
public:
    explicit EternalPalaceInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(EternalPalaceEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new EternalPalaceInstanceScript(pMapMgr); }

    // a court member falling first must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isCourtDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        for (size_t i = 0; i < 2; ++i)
        {
            if (pCreature->getEntry() == CourtMembers[i])
                mCourtGuids[i] = pCreature->GetNewGUID();
        }
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (findDoorRule(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.insert(pGameObject->GetNewGUID());
        alignDoor(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor);
        }

        if (id != DATA_THE_QUEENS_COURT)
            return;

        if (state == EncounterStates::InProgress)
        {
            raiseDecreeImage();
            return;
        }

        dismissDecreeImage();

        if (state != EncounterStates::Failed)
            return;

        // a wiped court fights again in full
        for (Creature* pMember : getCourt())
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }
    }

    std::vector<Creature*> getCourt()
    {
        std::vector<Creature*> court;
        for (WoWGuid const& guid : mCourtGuids)
        {
            if (guid.isEmpty())
                continue;

            if (Creature* pMember = getCreatureByGuid(guid))
                court.push_back(pMember);
        }

        return court;
    }

    bool isCourtDown(uint32_t dataId, Creature* pExcept)
    {
        if (dataId != DATA_THE_QUEENS_COURT)
            return true;

        for (Creature* pMember : getCourt())
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    // pulling one member pulls the other
    void rallyCourt(Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (Creature* pMember : getCourt())
        {
            if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat())
                continue;

            if (CreatureAIScript* pScript = pMember->GetScript())
                pScript->setZoneWideCombat();
        }
        mRallying = false;
    }

    Creature* getCourtPartner(Creature* pSelf)
    {
        for (Creature* pMember : getCourt())
        {
            if (pMember != pSelf && pMember->isAlive())
                return pMember;
        }

        return nullptr;
    }

private:
    PalaceDoorRule const* findDoorRule(uint32_t entry) const
    {
        for (PalaceDoorRule const& rule : PalaceDoorRules)
        {
            if (rule.entry == entry)
                return &rule;
        }

        return nullptr;
    }

    bool isDoorOpen(PalaceDoorRule const& rule) const
    {
        switch (rule.kind)
        {
            case PalaceDoorKind::Room:
                return getBossState(rule.dataId) != EncounterStates::InProgress;
            case PalaceDoorKind::Passage:
                return getBossState(rule.dataId) == EncounterStates::Performed;
            case PalaceDoorKind::Wall:
            {
                for (uint32_t dataId : PalaceFirstWing)
                {
                    if (getBossState(dataId) != EncounterStates::Performed)
                        return false;
                }
                return true;
            }
        }

        return false;
    }

    // the encounter decides, the spawn state of a door is not reliable
    void alignDoor(GameObject* pDoor)
    {
        PalaceDoorRule const* pRule = findDoorRule(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        uint8_t const state = isDoorOpen(*pRule) ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != state)
            pDoor->setState(state);
    }

    // a placed image is used, otherwise one is raised for the fight
    void raiseDecreeImage()
    {
        if (!mDecreeImageGuid.isEmpty() && getCreatureByGuid(mDecreeImageGuid) != nullptr)
            return;

        for (Creature* pImage : getCreatureSetForEntry(NPC_COURT_QUEEN_AZSHARA_IMAGE))
        {
            if (pImage->isAlive() && pImage->GetPosition().getExactDist(CourtDecreeSpot) <= 10.0f)
            {
                mDecreeImageGuid = pImage->GetNewGUID();
                mDecreeImageRaised = false;
                return;
            }
        }

        Creature* pImage = spawnCreature(NPC_COURT_QUEEN_AZSHARA_IMAGE, CourtDecreeSpot.x, CourtDecreeSpot.y, CourtDecreeSpot.z, CourtDecreeSpot.o);
        if (pImage == nullptr)
            return;

        mDecreeImageGuid = pImage->GetNewGUID();
        mDecreeImageRaised = true;
    }

    void dismissDecreeImage()
    {
        if (mDecreeImageRaised && !mDecreeImageGuid.isEmpty())
        {
            if (Creature* pImage = getCreatureByGuid(mDecreeImageGuid))
                pImage->despawn(0, 0);
        }

        mDecreeImageGuid.clear();
        mDecreeImageRaised = false;
    }

    std::set<WoWGuid> mDoorGuids;
    WoWGuid mCourtGuids[2];
    WoWGuid mDecreeImageGuid;
    bool mDecreeImageRaised = false;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick, berserk and target helpers

class EternalPalaceCombatAI : public CreatureAIScript
{
public:
    explicit EternalPalaceCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStage = 0;
        mBerserkLeftMs = 0;
        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mStage = 0;
        mBerserkLeftMs = 0;
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

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}

    EternalPalaceInstanceScript* getPalace()
    {
        if (getCreature()->GetMapId() != MAP_THE_ETERNAL_PALACE)
            return nullptr;

        return static_cast<EternalPalaceInstanceScript*>(getInstanceScript());
    }

    Unit* currentVictim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

    std::vector<Player*> playersNear(float range) { return palacePlayersNear(getCreature(), range); }

    // shuffled living players, the current victim is picked last
    std::vector<Player*> pickPlayers(size_t count, bool skipVictim, float range = 100.0f)
    {
        std::vector<Player*> pool = playersNear(range);
        Util::randomShuffleVector(pool);

        if (skipVictim && pool.size() > 1)
        {
            Unit* pVictim = currentVictim();
            for (size_t i = 0; i + 1 < pool.size(); ++i)
            {
                if (pool[i] == pVictim)
                {
                    std::swap(pool[i], pool.back());
                    break;
                }
            }
        }

        if (pool.size() > count)
            pool.resize(count);

        return pool;
    }

    Player* pickPlayer(bool skipVictim)
    {
        std::vector<Player*> const picked = pickPlayers(1, skipVictim);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipVictim = false, bool triggered = false)
    {
        Player* pTarget = pickPlayer(skipVictim);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    Unit* findUnit(WoWGuid const& guid)
    {
        if (guid.isEmpty() || getCreature()->getWorldMap() == nullptr)
            return nullptr;

        return getCreature()->getWorldMap()->getUnit(guid);
    }

    Creature* findCreature(WoWGuid const& guid)
    {
        if (guid.isEmpty() || getCreature()->getWorldMap() == nullptr)
            return nullptr;

        return getCreature()->getWorldMap()->getCreature(guid);
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot, CreatureSummonDespawnType despawnType = CORPSE_DESPAWN, uint32_t durationMs = 0)
    {
        Creature* pAdd = summonCreature(entry, spot, despawnType, durationMs);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // summoned helper that stays out of the fight, lifetime 0 keeps it until despawned
    Creature* placeHelper(uint32_t entry, LocationVector const& spot, uint32_t lifetimeMs)
    {
        return summonCreature(entry, spot, lifetimeMs != 0 ? TIMED_DESPAWN : MANUAL_DESPAWN, lifetimeMs);
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    // a new stage drops every pending event
    void enterStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
    }

    // out of reach: no movement, no melee, not selectable
    void setDormant(bool dormant)
    {
        setRooted(dormant);
        _setMeleeDisabled(dormant);

        if (dormant)
            getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
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
        castSpellOnSelf(SPELL_ETERNAL_PALACE_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state and court pull

class EternalPalaceBossAI : public EternalPalaceCombatAI
{
public:
    EternalPalaceBossAI(Creature* pCreature, uint32_t dataId) : EternalPalaceCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (mDataId == DATA_THE_QUEENS_COURT)
        {
            if (EternalPalaceInstanceScript* pPalace = getPalace())
                pPalace->rallyCourt(getCreature());
        }

        EternalPalaceCombatAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        if (getCreature()->isAlive())
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        EternalPalaceCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            EternalPalaceInstanceScript* pPalace = getPalace();
            if (pPalace == nullptr || pPalace->isCourtDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Helpers outside the fight

// immobile helper that never fights
class EternalPalaceHelperAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceHelperAI(c); }
    explicit EternalPalaceHelperAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        setRooted(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

protected:
    // counts down once, only summoned helpers act
    bool countdown(unsigned long time_passed, uint32_t& leftMs)
    {
        if (!getCreature()->isSummon() || leftMs == 0)
            return false;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (leftMs > diff)
        {
            leftMs -= diff;
            return false;
        }

        leftMs = 0;
        return true;
    }
};

// add with a single spell on a fixed cooldown
class EternalPalaceRepeatCasterAI : public EternalPalaceCombatAI
{
public:
    EternalPalaceRepeatCasterAI(Creature* pCreature, uint32_t spellId, PalaceAim aim, uint32_t firstMs, uint32_t intervalMs, bool triggered)
        : EternalPalaceCombatAI(pCreature), mSpellId(spellId), mAim(aim), mFirstMs(firstMs), mIntervalMs(intervalMs), mTriggered(triggered) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_EP_REPEAT_SPELL, mFirstMs); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_EP_REPEAT_SPELL)
            return;

        if (mAim == PalaceAim::Victim)
            castSpellOnVictim(mSpellId, mTriggered);
        else
            castAtPlayer(mSpellId, false, mTriggered);

        scriptEvents.addEvent(EVENT_EP_REPEAT_SPELL, mIntervalMs);
    }

private:
    uint32_t mSpellId;
    PalaceAim mAim;
    uint32_t mFirstMs;
    uint32_t mIntervalMs;
    bool mTriggered;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Blackwater Behemoth, surfaces at three platforms

class EternalPalaceBehemothAI : public EternalPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceBehemothAI(c); }
    explicit EternalPalaceBehemothAI(Creature* pCreature) : EternalPalaceBossAI(pCreature, DATA_BLACKWATER_BEHEMOTH) {}

protected:
    void onEngage() override
    {
        mPlatform = 0;
        setRooted(true);
        armBerserk(PalaceBerserkMs);
        castSpellOnSelf(SPELL_BEHEMOTH_FEEDING_FRENZY, true);

        scriptEvents.addEvent(EVENT_BEHEMOTH_FEELERS, 4000);
        scriptEvents.addEvent(EVENT_BEHEMOTH_TOXIC_SPINE, 8000);
        scriptEvents.addEvent(EVENT_BEHEMOTH_SHOCK_PULSE, 20000);
        scriptEvents.addEvent(EVENT_BEHEMOTH_NEXT_PLATFORM, 90000);
    }

    void onDisengage() override
    {
        mPlatform = 0;
        setRooted(false);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_BEHEMOTH_FEELERS:
                castSpellOnSelf(SPELL_BEHEMOTH_BIOELECTRIC_FEELERS);
                break;
            case EVENT_BEHEMOTH_TOXIC_SPINE:
                castSpellOnSelf(SPELL_BEHEMOTH_TOXIC_SPINE);
                scriptEvents.addEvent(EVENT_BEHEMOTH_SPINE_VOLLEY, PalaceFollowUpMs);
                scriptEvents.addEvent(EVENT_BEHEMOTH_TOXIC_SPINE, 28000);
                break;
            case EVENT_BEHEMOTH_SPINE_VOLLEY:
                for (Player* pTarget : pickPlayers(5, false))
                    castSpell(pTarget, SPELL_BEHEMOTH_TOXIC_SPINE_MISSILE, true);
                break;
            case EVENT_BEHEMOTH_SHOCK_PULSE:
                warn("The Blackwater Behemoth begins to cast Shock Pulse!");
                castSpellOnVictim(SPELL_BEHEMOTH_SHOCK_PULSE);
                scriptEvents.addEvent(EVENT_BEHEMOTH_SHOCK_PULSE, 20000);
                break;
            case EVENT_BEHEMOTH_NEXT_PLATFORM:
                diveDeeper();
                break;
            case EVENT_BEHEMOTH_CAVITATION:
                castSpellOnSelf(SPELL_BEHEMOTH_CAVITATION);
                break;
            default:
                break;
        }
    }

private:
    // the next platform down, the last one is kept until the end
    void diveDeeper()
    {
        if (mPlatform + 1 >= 3)
            return;

        ++mPlatform;
        warn("The Blackwater Behemoth dives deeper into the abyss!");
        getCreature()->interruptSpell();
        MoveTeleport(BehemothPlatforms[mPlatform]);
        scriptEvents.addEvent(EVENT_BEHEMOTH_CAVITATION, 2000);

        if (mPlatform + 1 < 3)
            scriptEvents.addEvent(EVENT_BEHEMOTH_NEXT_PLATFORM, 90000);
    }

    uint8_t mPlatform = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Za'qul, Harbinger of Ny'alotha

class EternalPalaceZaqulAI : public EternalPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceZaqulAI(c); }
    explicit EternalPalaceZaqulAI(Creature* pCreature) : EternalPalaceBossAI(pCreature, DATA_ZAQUL) {}

protected:
    void onEngage() override
    {
        armBerserk(PalaceBerserkMs);
        mDarkPulse.reset();
        beginStage(1);
    }

    void onCombatTick(uint32_t diff) override
    {
        int32_t const health = _getHealthPercent();
        if (mStage == 1 && health <= 85)
            beginStage(2);
        else if (mStage == 2 && health <= 70)
            beginStage(3);
        else if (mStage == 3 && health <= 50)
            beginStage(4);

        if (mStage == 4 && mDarkPulse.fill(diff))
            scriptEvents.addEvent(EVENT_ZAQUL_DARK_PULSE, 100);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ZAQUL_DARK_BEYOND:
                punishStrays();
                scriptEvents.addEvent(EVENT_ZAQUL_DARK_BEYOND, 2000);
                break;
            case EVENT_ZAQUL_MIND_TETHER:
                castSpellOnVictim(SPELL_ZAQUL_MIND_TETHER);
                break;
            case EVENT_ZAQUL_CRUSHING_GRASP:
                castSpellOnSelf(SPELL_ZAQUL_CRUSHING_GRASP);
                scriptEvents.addEvent(EVENT_ZAQUL_CRUSHING_GRASP, 40000);
                break;
            case EVENT_ZAQUL_DREAD:
                castSpellOnVictim(SPELL_ZAQUL_DREAD);
                scriptEvents.addEvent(EVENT_ZAQUL_DREAD, 25000);
                break;
            case EVENT_ZAQUL_PORTAL_OF_MADNESS:
                openPortal(SPELL_ZAQUL_PORTAL_OF_MADNESS, NPC_ZAQUL_HORRIFIC_SUMMONER);
                scriptEvents.addEvent(EVENT_ZAQUL_PORTAL_OF_MADNESS, 15000);
                break;
            case EVENT_ZAQUL_MADDENING_ERUPTION:
                getCreature()->castSpellLoc(palaceScatter(ZaqulArenaCentre, 0.0f, 30.0f), SPELL_ZAQUL_MADDENING_ERUPTION, false);
                scriptEvents.addEvent(EVENT_ZAQUL_MADDENING_ERUPTION, 35000);
                break;
            case EVENT_ZAQUL_MANIFEST_NIGHTMARES:
                for (Player* pTarget : pickPlayers(2, false))
                    castSpell(pTarget, SPELL_ZAQUL_MANIFEST_NIGHTMARES, true);
                scriptEvents.addEvent(EVENT_ZAQUL_MANIFEST_NIGHTMARES, 45000);
                break;
            case EVENT_ZAQUL_DELIRIUM_REALM:
                castSpellOnSelf(SPELL_ZAQUL_DELIRIUMS_DESCENT);
                scriptEvents.addEvent(EVENT_ZAQUL_DELIRIUM_REALM, 20000);
                break;
            case EVENT_ZAQUL_MANIC_DREAD:
                castSpellOnSelf(SPELL_ZAQUL_MANIC_DREAD);
                scriptEvents.addEvent(EVENT_ZAQUL_MANIC_DREAD, 25000);
                break;
            case EVENT_ZAQUL_DARK_PASSAGE:
                openPortal(SPELL_ZAQUL_DARK_PASSAGE, NPC_ZAQUL_PASSAGE_SUMMONER);
                scriptEvents.addEvent(EVENT_ZAQUL_DARK_PASSAGE, 35000);
                break;
            case EVENT_ZAQUL_DARK_PULSE:
                warn("Za'qul prepares to unleash a Dark Pulse!");
                castSpellOnSelf(SPELL_ZAQUL_DARK_PULSE);
                break;
            default:
                break;
        }
    }

private:
    void beginStage(uint32_t stage)
    {
        enterStage(stage);
        scriptEvents.addEvent(EVENT_ZAQUL_DARK_BEYOND, 2000);
        scriptEvents.addEvent(EVENT_ZAQUL_MIND_TETHER, 3000);
        scriptEvents.addEvent(EVENT_ZAQUL_CRUSHING_GRASP, 40000);

        if (stage == 1)
        {
            scriptEvents.addEvent(EVENT_ZAQUL_PORTAL_OF_MADNESS, 15000);
            scriptEvents.addEvent(EVENT_ZAQUL_DREAD, 25000);
            return;
        }

        if (stage == 4)
        {
            warn("Za'qul draws the madness of all realms into one!");
            mDarkPulse.reset();
            scriptEvents.addEvent(EVENT_ZAQUL_MANIC_DREAD, 25000);
            scriptEvents.addEvent(EVENT_ZAQUL_DARK_PASSAGE, 35000);
            return;
        }

        // the realm stages
        scriptEvents.addEvent(EVENT_ZAQUL_DREAD, 25000);
        scriptEvents.addEvent(EVENT_ZAQUL_MADDENING_ERUPTION, 35000);
        scriptEvents.addEvent(EVENT_ZAQUL_MANIFEST_NIGHTMARES, 45000);

        if (stage == 2)
        {
            warn("Za'qul drags you into the Fear Realm!");
            getCreature()->interruptSpell();
            castSpellOnSelf(SPELL_ZAQUL_OPENING_FEAR_REALM);
            return;
        }

        warn("Za'qul tears open the way to the Delirium Realm!");
        scriptEvents.addEvent(EVENT_ZAQUL_DELIRIUM_REALM, 20000);
    }

    // Dark Beyond for everyone outside the arena ring
    void punishStrays()
    {
        for (Player* pPlayer : playersNear(150.0f))
        {
            bool const outside = pPlayer->GetPosition().getExactDist2d(ZaqulArenaCentre) >= ZaqulArenaRadius;
            bool const marked = pPlayer->hasAurasWithId(SPELL_ZAQUL_DARK_BEYOND);

            if (outside && !marked)
                palaceSelfApply(pPlayer, SPELL_ZAQUL_DARK_BEYOND);
            else if (!outside && marked)
                pPlayer->removeAllAurasById(SPELL_ZAQUL_DARK_BEYOND);
        }
    }

    // the portal missile lands, a summoner steps through
    void openPortal(uint32_t spellId, uint32_t summonerEntry)
    {
        LocationVector const spot = palaceScatter(ZaqulArenaCentre, 5.0f, 30.0f);
        getCreature()->castSpellLoc(spot, spellId, true);
        callAdd(summonerEntry, spot);
    }

    PalaceGauge mDarkPulse{ 1000, 1, 2 };
};

class EternalPalaceHorrificSummonerAI : public EternalPalaceCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceHorrificSummonerAI(c); }
    explicit EternalPalaceHorrificSummonerAI(Creature* pCreature) : EternalPalaceCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_SUMMONER_DARK_TEAR, 1000); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SUMMONER_DARK_TEAR:
                castSpellOnSelf(SPELL_SUMMONER_DARK_TEAR);
                scriptEvents.addEvent(EVENT_SUMMONER_VOID_ERUPTION, PalaceFollowUpMs);
                break;
            case EVENT_SUMMONER_VOID_ERUPTION:
                castSpellOnSelf(SPELL_SUMMONER_VOID_ERUPTION, true);
                break;
            default:
                break;
        }
    }
};

class EternalPalacePassageSummonerAI : public EternalPalaceCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalacePassageSummonerAI(c); }
    explicit EternalPalacePassageSummonerAI(Creature* pCreature) : EternalPalaceCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_SUMMONER_SHROUD_OF_FEAR, true);
        scriptEvents.addEvent(EVENT_SUMMONER_FEARS_GATE, 1000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId == EVENT_SUMMONER_FEARS_GATE)
            castSpellOnSelf(SPELL_SUMMONER_FEARS_GATE);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Abyssal Commander Sivara

class EternalPalaceSivaraAI : public EternalPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceSivaraAI(c); }
    explicit EternalPalaceSivaraAI(Creature* pCreature) : EternalPalaceBossAI(pCreature, DATA_ABYSSAL_COMMANDER_SIVARA) {}

protected:
    void onEngage() override
    {
        armBerserk(PalaceBerserkMs);
        mBarrage.reset();
        mBoltsLeft = 2;
        castSpellOnSelf(SPELL_SIVARA_FROSTVENOM_TIPPED, true);

        scriptEvents.addEvent(EVENT_SIVARA_CHIMERIC_MARKS, 3000);
        scriptEvents.addEvent(EVENT_SIVARA_CRUSHING_REVERBERATION, 35000);
        scriptEvents.addEvent(EVENT_SIVARA_INVERSION, 35000);
        scriptEvents.addEvent(EVENT_SIVARA_OVERFLOW, 40000);
        scriptEvents.addEvent(EVENT_SIVARA_BOLT_VOLLEY, 50000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mBarrage.fill(diff))
            scriptEvents.addEvent(EVENT_SIVARA_OVERWHELMING_BARRAGE, 100);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SIVARA_CHIMERIC_MARKS:
                castSpellOnSelf(SPELL_SIVARA_CHIMERIC_MARKS);
                scriptEvents.addEvent(EVENT_SIVARA_SPREAD_MARKS, PalaceFollowUpMs);
                break;
            case EVENT_SIVARA_SPREAD_MARKS:
                spreadMarks();
                break;
            case EVENT_SIVARA_OVERFLOW:
                castSpellOnSelf(SPELL_SIVARA_OVERFLOW);
                scriptEvents.addEvent(EVENT_SIVARA_OVERFLOW_SPILLS, PalaceFollowUpMs);
                scriptEvents.addEvent(EVENT_SIVARA_OVERFLOW, 40000);
                break;
            case EVENT_SIVARA_OVERFLOW_SPILLS:
                spill(SPELL_SIVARA_FROST_MARK, SPELL_SIVARA_OVERFLOWING_CHILL);
                spill(SPELL_SIVARA_TOXIC_BRAND, SPELL_SIVARA_OVERFLOWING_VENOM);
                break;
            case EVENT_SIVARA_INVERSION:
                castSpellOnSelf(SPELL_SIVARA_INVERSION);
                scriptEvents.addEvent(EVENT_SIVARA_INVERT_MARKS, PalaceFollowUpMs);
                scriptEvents.addEvent(EVENT_SIVARA_INVERSION, 35000);
                break;
            case EVENT_SIVARA_INVERT_MARKS:
                invertMarks();
                break;
            case EVENT_SIVARA_CRUSHING_REVERBERATION:
                castSpellOnVictim(SPELL_SIVARA_CRUSHING_REVERBERATION);
                scriptEvents.addEvent(EVENT_SIVARA_CRUSHING_REVERBERATION, 35000);
                break;
            case EVENT_SIVARA_BOLT_VOLLEY:
                hurlBolt();
                if (--mBoltsLeft > 0)
                {
                    scriptEvents.addEvent(EVENT_SIVARA_BOLT_VOLLEY, 5000);
                    break;
                }
                mBoltsLeft = 2;
                scriptEvents.addEvent(EVENT_SIVARA_BOLT_VOLLEY, 50000);
                break;
            case EVENT_SIVARA_OVERWHELMING_BARRAGE:
                warn("Abyssal Commander Sivara unleashes an Overwhelming Barrage!");
                castSpellOnSelf(SPELL_SIVARA_OVERWHELMING_BARRAGE);
                break;
            default:
                break;
        }
    }

private:
    // half the raid gets frost, the other half venom
    void spreadMarks()
    {
        std::vector<Player*> const targets = pickPlayers(PalaceRaidSize, false);
        for (size_t i = 0; i < targets.size(); ++i)
            castSpell(targets[i], (i % 2) == 0 ? SPELL_SIVARA_FROST_MARK_MISSILE : SPELL_SIVARA_TOXIC_BRAND_MISSILE, true);
    }

    // two marked players overflow, the tank is spared
    void spill(uint32_t markId, uint32_t spillId)
    {
        Unit* pVictim = currentVictim();
        uint8_t spilled = 0;
        for (Player* pPlayer : pickPlayers(PalaceRaidSize, false))
        {
            if (spilled >= 2)
                break;

            if (pPlayer == pVictim || !pPlayer->hasAurasWithId(markId))
                continue;

            palaceSelfApply(pPlayer, spillId);
            ++spilled;
        }
    }

    // frost turns to venom and venom to frost
    void invertMarks()
    {
        std::vector<Player*> frosted;
        std::vector<Player*> branded;
        for (Player* pPlayer : playersNear(100.0f))
        {
            if (pPlayer->hasAurasWithId(SPELL_SIVARA_FROST_MARK))
                frosted.push_back(pPlayer);
            else if (pPlayer->hasAurasWithId(SPELL_SIVARA_TOXIC_BRAND))
                branded.push_back(pPlayer);
        }

        for (Player* pPlayer : frosted)
        {
            pPlayer->removeAllAurasById(SPELL_SIVARA_FROST_MARK);
            castSpell(pPlayer, SPELL_SIVARA_TOXIC_BRAND_MISSILE, true);
        }

        for (Player* pPlayer : branded)
        {
            pPlayer->removeAllAurasById(SPELL_SIVARA_TOXIC_BRAND);
            castSpell(pPlayer, SPELL_SIVARA_FROST_MARK_MISSILE, true);
        }
    }

    // the bolt carries the element opposite to the target's mark
    void hurlBolt()
    {
        Player* pTarget = pickPlayer(true);
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, pTarget->hasAurasWithId(SPELL_SIVARA_FROST_MARK) ? SPELL_SIVARA_TOXIC_BOLT : SPELL_SIVARA_FROST_BOLT);
    }

    PalaceGauge mBarrage{ 4000, 0, 3 };
    uint8_t mBoltsLeft = 2;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Queen Azshara

class EternalPalaceAzsharaAI : public EternalPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceAzsharaAI(c); }
    explicit EternalPalaceAzsharaAI(Creature* pCreature) : EternalPalaceBossAI(pCreature, DATA_QUEEN_AZSHARA) {}

    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon == nullptr || mStage != AzsharaStageLovers || !_isInCombat())
            return;

        uint32_t const entry = pSummon->getEntry();
        if (entry != NPC_AZSHARA_AETHANEL && entry != NPC_AZSHARA_CYRANUS)
            return;

        if (++mLoversDown >= 2)
            issueDecree(AzsharaStageSecond);
    }

protected:
    void onEngage() override
    {
        armBerserk(AzsharaBerserkMs);
        mDetonation.reset();
        beginLovers();
    }

    void onDisengage() override
    {
        setDormant(false);
        mLoversDown = 0;
        mWardGuids.clear();
    }

    void onCombatTick(uint32_t diff) override
    {
        int32_t const health = _getHealthPercent();
        if (mStage == AzsharaStageSecond && health <= 70)
            issueDecree(AzsharaStageThird);
        else if (mStage == AzsharaStageThird && health <= 45)
            beginFourth();

        if ((mStage == AzsharaStageSecond || mStage == AzsharaStageThird) && mDetonation.fill(diff))
            scriptEvents.addEvent(EVENT_AZSHARA_ARCANE_DETONATION, 100);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AZSHARA_DRAIN_WARDS:
                for (WoWGuid const& guid : mWardGuids)
                {
                    if (Creature* pWard = findCreature(guid))
                        castSpell(pWard, SPELL_AZSHARA_DRAIN_ANCIENT_WARD, true);
                }
                scriptEvents.addEvent(EVENT_AZSHARA_BECKON, 10000);
                break;
            case EVENT_AZSHARA_BECKON:
                castAtPlayer(SPELL_AZSHARA_BECKON, true);
                scriptEvents.addEvent(EVENT_AZSHARA_DRAIN_WARDS, 45000);
                break;
            case EVENT_AZSHARA_ARCANE_ORBS:
                for (uint8_t i = 0; i < 5; ++i)
                    getCreature()->castSpellLoc(palaceScatter(AzsharaArenaCentre, 0.0f, 15.0f), SPELL_AZSHARA_ARCANE_ORB, true);
                scriptEvents.addEvent(EVENT_AZSHARA_ARCANE_ORBS, 50000);
                break;
            case EVENT_AZSHARA_OVERZEALOUS_HULK:
                callAdd(NPC_AZSHARA_OVERZEALOUS_HULK, AzsharaArenaCentre);
                scriptEvents.addEvent(EVENT_AZSHARA_OVERZEALOUS_HULK, 45000);
                break;
            case EVENT_AZSHARA_ISSUE_DECREES:
                handOutDecrees();
                break;
            case EVENT_AZSHARA_NEXT_STAGE:
                if (mStageAfterDecree == AzsharaStageSecond)
                    beginSecond();
                else
                    beginThird();
                break;
            case EVENT_AZSHARA_ARCANE_JOLT:
                castSpellOnVictim(SPELL_AZSHARA_ARCANE_JOLT);
                scriptEvents.addEvent(EVENT_AZSHARA_ARCANE_JOLT, 15000);
                break;
            case EVENT_AZSHARA_ARCANE_BURST:
                castSpellOnSelf(SPELL_AZSHARA_ARCANE_BURST);
                scriptEvents.addEvent(EVENT_AZSHARA_BURST_MARKS, PalaceFollowUpMs);
                scriptEvents.addEvent(EVENT_AZSHARA_ARCANE_BURST, 25000);
                break;
            case EVENT_AZSHARA_BURST_MARKS:
                for (Player* pTarget : pickPlayers(2, true))
                    palaceSelfApply(pTarget, SPELL_AZSHARA_ARCANE_BURST_MARK);
                break;
            case EVENT_AZSHARA_ARCANE_DETONATION:
                warn("Queen Azshara unleashes an Arcane Detonation!");
                castSpellOnSelf(SPELL_AZSHARA_ARCANE_DETONATION);
                break;
            case EVENT_AZSHARA_SERVANTS:
                callServants();
                scriptEvents.addEvent(EVENT_AZSHARA_SERVANTS, 50000);
                break;
            case EVENT_AZSHARA_REVERSAL_OF_FORTUNE:
                castSpellOnSelf(SPELL_AZSHARA_REVERSAL_OF_FORTUNE);
                scriptEvents.addEvent(EVENT_AZSHARA_REVERSAL_OF_FORTUNE, 80000);
                break;
            case EVENT_AZSHARA_TIDEMISTRESSES:
                callAdd(NPC_AZSHARA_LADY_VENOMTONGUE, AzsharaServantSpots[1]);
                callAdd(NPC_AZSHARA_SERENA_SCARSCALE, AzsharaServantSpots[2]);
                if (isHeroic())
                    callAdd(NPC_AZSHARA_VENNA_SEASTORM, AzsharaServantSpots[3]);
                break;
            case EVENT_AZSHARA_LOYAL_MYRMIDON:
                callAdd(NPC_AZSHARA_LOYAL_MYRMIDON, AzsharaArenaCentre);
                scriptEvents.addEvent(EVENT_AZSHARA_LOYAL_MYRMIDON, 90000);
                break;
            case EVENT_AZSHARA_VOID_TOUCHED:
                castSpellOnVictim(SPELL_AZSHARA_VOID_TOUCHED);
                scriptEvents.addEvent(EVENT_AZSHARA_VOID_TOUCHED, 25000);
                break;
            case EVENT_AZSHARA_NETHER_PORTAL:
                getCreature()->castSpellLoc(palaceScatter(AzsharaArenaCentre, 5.0f, 50.0f), SPELL_AZSHARA_NETHER_PORTAL, true);
                scriptEvents.addEvent(EVENT_AZSHARA_NETHER_PORTAL, 30000);
                break;
            case EVENT_AZSHARA_PIERCING_GAZE:
                getCreature()->castSpellLoc(AzsharaArenaCentre, SPELL_AZSHARA_PIERCING_GAZE, true);
                scriptEvents.addEvent(EVENT_AZSHARA_PIERCING_GAZE, 50000);
                break;
            case EVENT_AZSHARA_OVERLOAD:
                overload();
                scriptEvents.addEvent(EVENT_AZSHARA_ENERGY_SPIKE, 40000);
                break;
            case EVENT_AZSHARA_ENERGY_SPIKE:
                if (Creature* pConsole = findNearestCreature(NPC_AZSHARA_TITAN_CONSOLE, 100.0f))
                    pConsole->castSpell(pConsole, SPELL_CONSOLE_MASSIVE_ENERGY_SPIKE, true);
                scriptEvents.addEvent(EVENT_AZSHARA_OVERLOAD, 55000);
                break;
            default:
                break;
        }
    }

private:
    // out of reach while the cursed lovers fight
    void beginLovers()
    {
        enterStage(AzsharaStageLovers);
        setDormant(true);
        warn("Queen Azshara calls the cursed lovers to the stage!");

        mWardGuids.clear();
        for (LocationVector const& spot : AzsharaWardSpots)
        {
            if (Creature* pWard = placeHelper(NPC_AZSHARA_ANCIENT_WARD, spot, 0))
                mWardGuids.push_back(pWard->GetNewGUID());
        }

        scriptEvents.addEvent(EVENT_AZSHARA_DRAIN_WARDS, 45000);
        scriptEvents.addEvent(EVENT_AZSHARA_OVERZEALOUS_HULK, 45000);
        scriptEvents.addEvent(EVENT_AZSHARA_ARCANE_ORBS, 50000);

        // a lover that cannot be called counts as fallen
        mLoversDown = 0;
        if (callAdd(NPC_AZSHARA_AETHANEL, AzsharaAethanelSpot) == nullptr)
            ++mLoversDown;
        if (callAdd(NPC_AZSHARA_CYRANUS, AzsharaCyranusSpot) == nullptr)
            ++mLoversDown;

        if (mLoversDown >= 2)
            issueDecree(AzsharaStageSecond);
    }

    // intermission: the queen steps in and hands out her decrees
    void issueDecree(uint32_t stageAfter)
    {
        enterStage(stageAfter == AzsharaStageSecond ? AzsharaStageFirstDecree : AzsharaStageSecondDecree);
        mStageAfterDecree = stageAfter;
        setDormant(false);
        getCreature()->interruptSpell();

        warn("Queen Azshara issues a Queen's Decree!");
        castSpellOnSelf(SPELL_AZSHARA_QUEENS_DECREE);
        scriptEvents.addEvent(EVENT_AZSHARA_ISSUE_DECREES, PalaceFollowUpMs);
        scriptEvents.addEvent(EVENT_AZSHARA_NEXT_STAGE, 20000);
    }

    // one stance decree each, heroic adds Obey or Suffer
    void handOutDecrees()
    {
        for (Player* pPlayer : playersNear(150.0f))
        {
            palaceSelfApply(pPlayer, Util::getRandomUInt(1) == 0 ? SPELL_AZSHARA_DECREE_STAND_ALONE : SPELL_AZSHARA_DECREE_STAND_TOGETHER);

            if (isHeroic())
                palaceSelfApply(pPlayer, Util::getRandomUInt(1) == 0 ? SPELL_AZSHARA_DECREE_OBEY : SPELL_AZSHARA_DECREE_SUFFER);
        }
    }

    void beginSecond()
    {
        enterStage(AzsharaStageSecond);
        warn("Queen Azshara descends to face her challengers!");
        MoveTeleport(AzsharaStageFront);
        castSpellOnSelf(SPELL_AZSHARA_ARCANE_MASTERY, true);
        mDetonation.reset();

        scriptEvents.addEvent(EVENT_AZSHARA_BECKON, 10000);
        scriptEvents.addEvent(EVENT_AZSHARA_ARCANE_JOLT, 15000);
        scriptEvents.addEvent(EVENT_AZSHARA_ARCANE_BURST, 25000);
        scriptEvents.addEvent(EVENT_AZSHARA_SERVANTS, 50000);
    }

    void beginThird()
    {
        enterStage(AzsharaStageThird);
        warn("Queen Azshara calls her tidemistresses to the arena!");
        castSpellOnSelf(SPELL_AZSHARA_ARCANE_MASTERY, true);
        mDetonation.reset();

        scriptEvents.addEvent(EVENT_AZSHARA_LOYAL_MYRMIDON, 2000);
        scriptEvents.addEvent(EVENT_AZSHARA_TIDEMISTRESSES, 5000);
        scriptEvents.addEvent(EVENT_AZSHARA_BECKON, 10000);
        scriptEvents.addEvent(EVENT_AZSHARA_ARCANE_JOLT, 15000);
        scriptEvents.addEvent(EVENT_AZSHARA_ARCANE_BURST, 25000);
        scriptEvents.addEvent(EVENT_AZSHARA_REVERSAL_OF_FORTUNE, 80000);
    }

    void beginFourth()
    {
        enterStage(AzsharaStageFourth);
        warn("Queen Azshara draws on the power of N'Zoth!");

        scriptEvents.addEvent(EVENT_AZSHARA_BECKON, 10000);
        scriptEvents.addEvent(EVENT_AZSHARA_VOID_TOUCHED, 25000);
        scriptEvents.addEvent(EVENT_AZSHARA_NETHER_PORTAL, 30000);
        scriptEvents.addEvent(EVENT_AZSHARA_PIERCING_GAZE, 50000);
        scriptEvents.addEvent(EVENT_AZSHARA_OVERLOAD, 55000);
        scriptEvents.addEvent(EVENT_AZSHARA_REVERSAL_OF_FORTUNE, 80000);
    }

    // one Indomitable or three Devoted march on the wards
    void callServants()
    {
        if (Util::getRandomUInt(1) == 0)
        {
            summonCreature(NPC_AZSHARA_INDOMITABLE, AzsharaServantSpots[1], CORPSE_DESPAWN);
            return;
        }

        summonCreature(NPC_AZSHARA_DEVOTED, AzsharaServantSpots[0], CORPSE_DESPAWN);
        summonCreature(NPC_AZSHARA_DEVOTED, AzsharaServantSpots[2], CORPSE_DESPAWN);
        summonCreature(NPC_AZSHARA_DEVOTED, AzsharaServantSpots[3], CORPSE_DESPAWN);
    }

    // the console overloads into a shock, an essence or a drained ward
    void overload()
    {
        warn("The Titan Console begins to Overload!");

        switch (Util::getRandomUInt(2))
        {
            case 0:
                if (Player* pTarget = pickPlayer(false))
                    palaceSelfApply(pTarget, SPELL_AZSHARA_SYSTEM_SHOCK);
                break;
            case 1:
                if (Player* pTarget = pickPlayer(false))
                    palaceSelfApply(pTarget, SPELL_AZSHARA_ESSENCE_OF_AZEROTH);
                break;
            default:
                if (!mWardGuids.empty())
                {
                    WoWGuid const& guid = mWardGuids[Util::getRandomUInt(static_cast<uint32_t>(mWardGuids.size() - 1))];
                    if (Creature* pWard = findCreature(guid))
                        castSpell(pWard, SPELL_AZSHARA_DRAIN_ANCIENT_WARD, true);
                }
                break;
        }
    }

    PalaceGauge mDetonation{ 1000, 2, 3 };
    uint8_t mLoversDown = 0;
    uint32_t mStageAfterDecree = 0;
    std::vector<WoWGuid> mWardGuids;
};

// Aethanel and Cyranus share Longing and Painful Memories
class EternalPalaceLoverAI : public EternalPalaceCombatAI
{
public:
    EternalPalaceLoverAI(Creature* pCreature, uint32_t firstLongingMs) : EternalPalaceCombatAI(pCreature), mFirstLongingMs(firstLongingMs) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_LOVER_LONGING, mFirstLongingMs);
        onLoverEngage();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_LOVER_LONGING:
                castSpellOnSelf(SPELL_LOVERS_LONGING);
                scriptEvents.addEvent(EVENT_LOVER_PAINFUL_MEMORIES, 8000);
                break;
            case EVENT_LOVER_PAINFUL_MEMORIES:
                castSpellOnSelf(SPELL_LOVERS_PAINFUL_MEMORIES);
                scriptEvents.addEvent(EVENT_LOVER_LONGING, 20000);
                break;
            default:
                onLoverEvent(eventId);
                break;
        }
    }

    virtual void onLoverEngage() {}
    virtual void onLoverEvent(uint32_t /*eventId*/) {}

private:
    uint32_t mFirstLongingMs;
};

class EternalPalaceAethanelAI : public EternalPalaceLoverAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceAethanelAI(c); }
    explicit EternalPalaceAethanelAI(Creature* pCreature) : EternalPalaceLoverAI(pCreature, 8000) {}

protected:
    void onLoverEngage() override
    {
        scriptEvents.addEvent(EVENT_AETHANEL_CHAIN_LIGHTNING, 15000);
        scriptEvents.addEvent(EVENT_AETHANEL_LIGHTNING_ORBS, 35000);
    }

    void onLoverEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AETHANEL_CHAIN_LIGHTNING:
                castAtPlayer(SPELL_AETHANEL_CHAIN_LIGHTNING);
                scriptEvents.addEvent(EVENT_AETHANEL_CHAIN_LIGHTNING, 15000);
                break;
            case EVENT_AETHANEL_LIGHTNING_ORBS:
                for (uint8_t i = 0; i < 3; ++i)
                    placeHelper(NPC_AZSHARA_LIGHTNING_ORB, palaceScatter(getCreature()->GetPosition(), 5.0f, 25.0f), 25000);
                scriptEvents.addEvent(EVENT_AETHANEL_LIGHTNING_ORBS, 35000);
                break;
            default:
                break;
        }
    }
};

class EternalPalaceCyranusAI : public EternalPalaceLoverAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceCyranusAI(c); }
    explicit EternalPalaceCyranusAI(Creature* pCreature) : EternalPalaceLoverAI(pCreature, 20000) {}

protected:
    void onLoverEngage() override
    {
        scriptEvents.addEvent(EVENT_CYRANUS_SERRATED_EDGE, 3000);
        scriptEvents.addEvent(EVENT_CYRANUS_CHARGED_SPEAR, 30000);
    }

    void onLoverEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CYRANUS_SERRATED_EDGE:
                castSpellOnVictim(SPELL_CYRANUS_SERRATED_EDGE, true);
                scriptEvents.addEvent(EVENT_CYRANUS_SERRATED_EDGE, 2000);
                break;
            case EVENT_CYRANUS_CHARGED_SPEAR:
                if (Unit* pVictim = currentVictim())
                {
                    warn("Cyranus aims a Charged Spear!");
                    getCreature()->castSpellLoc(pVictim->GetPosition(), SPELL_CYRANUS_CHARGED_SPEAR, false);
                }
                scriptEvents.addEvent(EVENT_CYRANUS_CHARGED_SPEAR, 30000);
                break;
            default:
                break;
        }
    }
};

// bounces lightning around itself four times
class EternalPalaceLightningOrbAI : public EternalPalaceHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceLightningOrbAI(c); }
    explicit EternalPalaceLightningOrbAI(Creature* pCreature) : EternalPalaceHelperAI(pCreature) {}

    void OnLoad() override
    {
        EternalPalaceHelperAI::OnLoad();
        _applyAura(SPELL_LIGHTNING_ORB_VISUAL);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mPulsesLeft == 0 || !countdown(time_passed, mPulseLeftMs))
            return;

        getCreature()->castSpellLoc(palaceScatter(getCreature()->GetPosition(), 0.0f, 15.0f), SPELL_LIGHTNING_ORB_DISCHARGE, true);
        if (--mPulsesLeft > 0)
            mPulseLeftMs = 5000;
    }

private:
    uint8_t mPulsesLeft = 4;
    uint32_t mPulseLeftMs = 1000;
};

// walks to a ward and pounds it, then fights
class EternalPalaceOverzealousHulkAI : public EternalPalaceCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceOverzealousHulkAI(c); }
    explicit EternalPalaceOverzealousHulkAI(Creature* pCreature) : EternalPalaceCombatAI(pCreature) {}

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type != POINT_MOTION_TYPE || id != PointHulkWard)
            return;

        if (Creature* pWard = findCreature(mWardGuid))
            castSpell(pWard, SPELL_HULK_GROUND_POUND);

        scriptEvents.addEvent(EVENT_HULK_POUND_DONE, PalaceFollowUpMs);
    }

protected:
    void onEngage() override
    {
        mWardGuid.clear();
        scriptEvents.addEvent(EVENT_HULK_SEEK_WARD, 1000);
    }

    void onDisengage() override { setReactState(REACT_AGGRESSIVE); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_HULK_SEEK_WARD:
            {
                Creature* pWard = findNearestCreature(NPC_AZSHARA_ANCIENT_WARD, 100.0f);
                if (pWard == nullptr)
                    break;

                mWardGuid = pWard->GetNewGUID();
                attackStop();
                setReactState(REACT_PASSIVE);
                movePoint(PointHulkWard, pWard->GetPosition());
                break;
            }
            case EVENT_HULK_POUND_DONE:
                setReactState(REACT_AGGRESSIVE);
                break;
            default:
                break;
        }
    }

private:
    WoWGuid mWardGuid;
};

// Azshara's Indomitable and Azshara's Devoted empower the ward they reach
class EternalPalaceDevotedServantAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceDevotedServantAI(c); }
    explicit EternalPalaceDevotedServantAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        if (!getCreature()->isSummon())
            return;

        setReactState(REACT_PASSIVE);
        mMarchLeftMs = 2000;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mMarchLeftMs == 0)
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mMarchLeftMs > diff)
        {
            mMarchLeftMs -= diff;
            return;
        }

        mMarchLeftMs = 0;
        if (Creature* pWard = findNearestCreature(NPC_AZSHARA_ANCIENT_WARD, 100.0f))
        {
            mWardGuid = pWard->GetNewGUID();
            movePoint(PointServantWard, pWard->GetPosition());
        }
    }

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type != POINT_MOTION_TYPE || id != PointServantWard || getCreature()->getWorldMap() == nullptr)
            return;

        if (Creature* pWard = getCreature()->getWorldMap()->getCreature(mWardGuid))
            castSpell(pWard, SPELL_SERVANT_ENERGIZE_WARD, true);

        despawn(1000, 0);
    }

private:
    uint32_t mMarchLeftMs = 0;
    WoWGuid mWardGuid;
};

// Lady Venomtongue, Serena Scarscale and Venna Seastorm
class EternalPalaceTidemistressAI : public EternalPalaceCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceTidemistressAI(c); }
    explicit EternalPalaceTidemistressAI(Creature* pCreature) : EternalPalaceCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        setRooted(true);
        scriptEvents.addEvent(EVENT_TIDEMISTRESS_CHAIN_LIGHTNING, 15000);
        scriptEvents.addEvent(EVENT_TIDEMISTRESS_STATIC_SHOCK, 25000);
        scriptEvents.addEvent(EVENT_TIDEMISTRESS_CRYSTALLINE_SHIELD, Util::getRandomUInt(30000, 50000));
        scriptEvents.addEvent(EVENT_TIDEMISTRESS_DRAIN_WARDS, 45000);
    }

    void onDisengage() override { setRooted(false); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_TIDEMISTRESS_CHAIN_LIGHTNING:
                castAtPlayer(SPELL_AETHANEL_CHAIN_LIGHTNING);
                scriptEvents.addEvent(EVENT_TIDEMISTRESS_CHAIN_LIGHTNING, 15000);
                break;
            case EVENT_TIDEMISTRESS_STATIC_SHOCK:
                for (Player* pTarget : pickPlayers(4, false))
                    palaceSelfApply(pTarget, SPELL_TIDEMISTRESS_STATIC_SHOCK);
                scriptEvents.addEvent(EVENT_TIDEMISTRESS_STATIC_SHOCK, 25000);
                break;
            case EVENT_TIDEMISTRESS_CRYSTALLINE_SHIELD:
                castSpellOnSelf(SPELL_TIDEMISTRESS_CRYSTALLINE_SHIELD, true);
                scriptEvents.addEvent(EVENT_TIDEMISTRESS_CRYSTALLINE_SHIELD, Util::getRandomUInt(30000, 50000));
                break;
            case EVENT_TIDEMISTRESS_DRAIN_WARDS:
            {
                std::list<Creature*> wards;
                GetCreatureListWithEntryInGrid(wards, NPC_AZSHARA_ANCIENT_WARD, 100.0f);
                for (Creature* pWard : wards)
                    castSpell(pWard, SPELL_AZSHARA_DRAIN_ANCIENT_WARD, true);
                scriptEvents.addEvent(EVENT_TIDEMISTRESS_DRAIN_WARDS, 45000);
                break;
            }
            default:
                break;
        }
    }
};

// its spear breaks a tidemistress' Crystalline Shield
class EternalPalaceLoyalMyrmidonAI : public EternalPalaceCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceLoyalMyrmidonAI(c); }
    explicit EternalPalaceLoyalMyrmidonAI(Creature* pCreature) : EternalPalaceCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_LOYAL_MYRMIDON_CHARGED_SPEAR, 30000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_LOYAL_MYRMIDON_CHARGED_SPEAR)
            return;

        Unit* pTarget = findShieldedTidemistress();
        if (pTarget == nullptr)
            pTarget = currentVictim();

        if (pTarget != nullptr)
            getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_CYRANUS_CHARGED_SPEAR, false);

        scriptEvents.addEvent(EVENT_LOYAL_MYRMIDON_CHARGED_SPEAR, 30000);
    }

private:
    Creature* findShieldedTidemistress()
    {
        uint32_t const mistresses[3] = { NPC_AZSHARA_LADY_VENOMTONGUE, NPC_AZSHARA_SERENA_SCARSCALE, NPC_AZSHARA_VENNA_SEASTORM };
        for (uint32_t entry : mistresses)
        {
            std::list<Creature*> found;
            GetCreatureListWithEntryInGrid(found, entry, 100.0f);
            for (Creature* pMistress : found)
            {
                if (pMistress->isAlive() && pMistress->hasAurasWithId(SPELL_TIDEMISTRESS_CRYSTALLINE_SHIELD))
                    return pMistress;
            }
        }

        return nullptr;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Orgozoa

class EternalPalaceOrgozoaAI : public EternalPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceOrgozoaAI(c); }
    explicit EternalPalaceOrgozoaAI(Creature* pCreature) : EternalPalaceBossAI(pCreature, DATA_ORGOZOA) {}

    void OnReachWP(uint32_t type, uint32_t id) override
    {
        if (type != POINT_MOTION_TYPE || mStage != OrgozoaStageDescent || mDescentStep >= OrgozoaDescentSteps || id != PointOrgozoaDescent + mDescentStep)
            return;

        if (++mDescentStep < OrgozoaDescentSteps)
            movePoint(PointOrgozoaDescent + mDescentStep, OrgozoaDescent[mDescentStep], false);
        else
            startIncubator();
    }

protected:
    void onEngage() override
    {
        armBerserk(PalaceBerserkMs);
        mIchor.reset();
        beginBrood(OrgozoaStageNursery);
    }

    void onDisengage() override
    {
        mDescentStep = 0;
        setReactState(REACT_AGGRESSIVE);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mStage == OrgozoaStageNursery && _getHealthPercent() <= 40)
        {
            beginDescent();
            return;
        }

        if (mStage != OrgozoaStageDescent && mIchor.fill(diff))
            scriptEvents.addEvent(EVENT_ORGOZOA_DRIBBLING_ICHOR, 100);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ORGOZOA_INCUBATION_FLUID:
                castSpellOnSelf(SPELL_ORGOZOA_INCUBATION_FLUID);
                scriptEvents.addEvent(EVENT_ORGOZOA_SOAK_FLUID, PalaceFollowUpMs);
                scriptEvents.addEvent(EVENT_ORGOZOA_ARCING_CURRENT, 9000);
                break;
            case EVENT_ORGOZOA_SOAK_FLUID:
                for (Player* pTarget : pickPlayers(3, false))
                    castSpell(pTarget, SPELL_ORGOZOA_INCUBATION_FLUID_AURA, true);
                break;
            case EVENT_ORGOZOA_ARCING_CURRENT:
                arcCurrent();
                scriptEvents.addEvent(EVENT_ORGOZOA_INCUBATION_FLUID, 23000);
                break;
            case EVENT_ORGOZOA_DESENSITIZING_STING:
                castSpellOnVictim(SPELL_ORGOZOA_DESENSITIZING_STING);
                scriptEvents.addEvent(EVENT_ORGOZOA_DESENSITIZING_STING, 7000);
                break;
            case EVENT_ORGOZOA_DRIBBLING_ICHOR:
                castSpellOnSelf(SPELL_ORGOZOA_DRIBBLING_ICHOR);
                scriptEvents.addEvent(EVENT_ORGOZOA_HATCHLINGS, 15000);
                break;
            case EVENT_ORGOZOA_HATCHLINGS:
                hatch();
                break;
            case EVENT_ORGOZOA_INCUBATOR_FALLBACK:
                mDescentStep = OrgozoaDescentSteps;
                MoveTeleport(OrgozoaDescent[OrgozoaDescentSteps - 1]);
                startIncubator();
                break;
            case EVENT_ORGOZOA_HATCHERY_OPENS:
                setReactState(REACT_AGGRESSIVE);
                beginBrood(OrgozoaStageHatchery);
                break;
            default:
                break;
        }
    }

private:
    void beginBrood(uint32_t stage)
    {
        enterStage(stage);
        scriptEvents.addEvent(EVENT_ORGOZOA_DESENSITIZING_STING, 7000);
        scriptEvents.addEvent(EVENT_ORGOZOA_HATCHLINGS, 15000);
        scriptEvents.addEvent(EVENT_ORGOZOA_INCUBATION_FLUID, 23000);
    }

    // retreat along the path, the incubator waits at its end
    void beginDescent()
    {
        enterStage(OrgozoaStageDescent);
        warn("Orgozoa retreats to the hatchery!");
        getCreature()->interruptSpell();
        attackStop();
        setReactState(REACT_PASSIVE);

        mDescentStep = 0;
        movePoint(PointOrgozoaDescent, OrgozoaDescent[0], false);
        scriptEvents.addEvent(EVENT_ORGOZOA_INCUBATOR_FALLBACK, 40000);
    }

    void startIncubator()
    {
        scriptEvents.removeEvent(EVENT_ORGOZOA_INCUBATOR_FALLBACK);
        castSpellOnSelf(SPELL_ORGOZOA_MASSIVE_INCUBATOR);
        scriptEvents.addEvent(EVENT_ORGOZOA_HATCHERY_OPENS, PalaceFollowUpMs);
    }

    // a player soaked in fluid draws the current first
    void arcCurrent()
    {
        Player* pTarget = nullptr;
        for (Player* pPlayer : pickPlayers(PalaceRaidSize, false))
        {
            if (pPlayer->hasAurasWithId(SPELL_ORGOZOA_INCUBATION_FLUID_AURA))
            {
                pTarget = pPlayer;
                break;
            }
        }

        if (pTarget == nullptr)
            pTarget = pickPlayer(false);

        if (pTarget != nullptr)
            castSpell(pTarget, SPELL_ORGOZOA_ARCING_CURRENT, true);
    }

    void hatch()
    {
        if (mStage == OrgozoaStageNursery)
        {
            callAdd(NPC_ORGOZOA_ZOATROID, OrgozoaNurserySpots[0]);
            callAdd(NPC_ORGOZOA_ZOATROID, OrgozoaNurserySpots[0]);
            callAdd(NPC_ORGOZOA_ZOATROID, OrgozoaNurserySpots[1]);
            return;
        }

        if (mStage != OrgozoaStageHatchery)
            return;

        // two kinds of hatchery guards, two of each
        std::vector<uint32_t> kinds = { NPC_ORGOZOA_AZSHARI_WITCH, NPC_ORGOZOA_ZANJIR_MYRMIDON, NPC_ORGOZOA_DREADCOIL_HULK };
        if (isHeroic())
            kinds.push_back(NPC_ORGOZOA_ZOATROID);

        Util::randomShuffleVector(kinds);
        for (size_t i = 0; i < 2 && i < kinds.size(); ++i)
        {
            for (LocationVector const& spot : OrgozoaHatcherySpots)
                callAdd(kinds[i], spot);
        }
    }

    PalaceGauge mIchor{ 2000, 1, 2 };
    uint32_t mDescentStep = 0;
};

class EternalPalaceZoatroidAI : public EternalPalaceRepeatCasterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceZoatroidAI(c); }
    explicit EternalPalaceZoatroidAI(Creature* pCreature) : EternalPalaceRepeatCasterAI(pCreature, SPELL_ZOATROID_PERVASIVE_SHOCK, PalaceAim::Victim, 10000, 10000, true) {}
};

class EternalPalaceDreadcoilHulkAI : public EternalPalaceRepeatCasterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceDreadcoilHulkAI(c); }
    explicit EternalPalaceDreadcoilHulkAI(Creature* pCreature) : EternalPalaceRepeatCasterAI(pCreature, SPELL_DREADCOIL_POWERFUL_STOMP, PalaceAim::Victim, 30000, 30000, false) {}
};

class EternalPalaceZanjirMyrmidonAI : public EternalPalaceRepeatCasterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceZanjirMyrmidonAI(c); }
    explicit EternalPalaceZanjirMyrmidonAI(Creature* pCreature) : EternalPalaceRepeatCasterAI(pCreature, SPELL_ZANJIR_AQUA_LANCE, PalaceAim::RandomPlayer, 15000, 15000, false) {}
};

// Shocking Lightning and Conductive Pulse take turns
class EternalPalaceAzshariWitchAI : public EternalPalaceCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceAzshariWitchAI(c); }
    explicit EternalPalaceAzshariWitchAI(Creature* pCreature) : EternalPalaceCombatAI(pCreature) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_WITCH_SHOCKING_LIGHTNING, 15000); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_WITCH_SHOCKING_LIGHTNING:
                castSpellOnVictim(SPELL_WITCH_SHOCKING_LIGHTNING);
                scriptEvents.addEvent(EVENT_WITCH_CONDUCTIVE_PULSE, 15000);
                break;
            case EVENT_WITCH_CONDUCTIVE_PULSE:
                castSpellOnVictim(SPELL_WITCH_CONDUCTIVE_PULSE);
                scriptEvents.addEvent(EVENT_WITCH_SHOCKING_LIGHTNING, 15000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lady Ashvane, shelled by her Hardened Carapace until it breaks

class EternalPalaceAshvaneAI : public EternalPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceAshvaneAI(c); }
    explicit EternalPalaceAshvaneAI(Creature* pCreature) : EternalPalaceBossAI(pCreature, DATA_LADY_ASHVANE) {}

protected:
    void onEngage() override
    {
        armBerserk(PalaceBerserkMs);
        shellUp(false);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (mStage != AshvaneStageShelled)
            return;

        // the aura needs a moment before its absence counts as broken
        if (mShellGraceMs > diff)
        {
            mShellGraceMs -= diff;
            return;
        }

        mShellGraceMs = 0;
        if (!hasAura(SPELL_ASHVANE_HARDENED_CARAPACE))
            shatter();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ASHVANE_BARNACLE_BASH:
                castSpellOnVictim(SPELL_ASHVANE_BARNACLE_BASH);
                scriptEvents.addEvent(EVENT_ASHVANE_BARNACLE_BASH, 15000);
                break;
            case EVENT_ASHVANE_UPSURGE:
                for (Player* pTarget : pickPlayers(3, true))
                    castSpell(pTarget, SPELL_ASHVANE_UPSURGE, true);
                scriptEvents.addEvent(EVENT_ASHVANE_UPSURGE, 18000);
                break;
            case EVENT_ASHVANE_CORAL_GROWTH:
                for (uint8_t i = 0; i < 2; ++i)
                    summonCreature(NPC_ASHVANE_CORAL_GROWTH, palaceScatter(AshvaneArenaCentre, 5.0f, 35.0f), CORPSE_DESPAWN);
                scriptEvents.addEvent(EVENT_ASHVANE_CORAL_GROWTH, 50000);
                break;
            case EVENT_ASHVANE_RIPPLING_WAVE:
                castSpellOnSelf(SPELL_ASHVANE_RIPPLING_WAVE);
                scriptEvents.addEvent(EVENT_ASHVANE_RIPPLING_WAVE, 15000);
                break;
            case EVENT_ASHVANE_BRINY_BUBBLE:
                castSpellOnSelf(SPELL_ASHVANE_BRINY_BUBBLE);
                scriptEvents.addEvent(EVENT_ASHVANE_TRAP_BUBBLES, PalaceFollowUpMs);
                scriptEvents.addEvent(EVENT_ASHVANE_BRINY_BUBBLE, 45000);
                break;
            case EVENT_ASHVANE_TRAP_BUBBLES:
                for (Player* pTarget : pickPlayers(2, true))
                    palaceSelfApply(pTarget, SPELL_ASHVANE_BRINY_BUBBLE_MARK);
                break;
            case EVENT_ASHVANE_ARCING_AZERITE:
                warn("Lady Ashvane charges the arena with Arcing Azerite!");
                chargeAzerite();
                scriptEvents.addEvent(EVENT_ASHVANE_ARCING_AZERITE, 40000);
                break;
            case EVENT_ASHVANE_RESHELL:
                shellUp(true);
                break;
            default:
                break;
        }
    }

private:
    void shellUp(bool again)
    {
        enterStage(AshvaneStageShelled);
        mShellGraceMs = 3000;
        castSpellOnSelf(SPELL_ASHVANE_HARDENED_CARAPACE, true);
        if (again)
            warn("Lady Ashvane is encased in a Hardened Carapace once more!");

        scriptEvents.addEvent(EVENT_ASHVANE_UPSURGE, 3000);
        scriptEvents.addEvent(EVENT_ASHVANE_BARNACLE_BASH, 7000);
        scriptEvents.addEvent(EVENT_ASHVANE_RIPPLING_WAVE, 15000);
        scriptEvents.addEvent(EVENT_ASHVANE_CORAL_GROWTH, 25000);
        scriptEvents.addEvent(EVENT_ASHVANE_BRINY_BUBBLE, 45000);
    }

    void shatter()
    {
        enterStage(AshvaneStageExposed);
        warn("Lady Ashvane's Hardened Carapace shatters!");

        scriptEvents.addEvent(EVENT_ASHVANE_UPSURGE, 3000);
        scriptEvents.addEvent(EVENT_ASHVANE_BARNACLE_BASH, 7000);
        scriptEvents.addEvent(EVENT_ASHVANE_ARCING_AZERITE, 40000);
        scriptEvents.addEvent(EVENT_ASHVANE_BRINY_BUBBLE, 45000);
        scriptEvents.addEvent(EVENT_ASHVANE_RESHELL, 100000);
    }

    // three colours, two players each
    void chargeAzerite()
    {
        uint32_t const marks[3] = { SPELL_ASHVANE_ARCING_AZERITE_YELLOW, SPELL_ASHVANE_ARCING_AZERITE_RED, SPELL_ASHVANE_ARCING_AZERITE_BLUE };
        std::vector<Player*> const targets = pickPlayers(6, false);
        for (size_t i = 0; i < targets.size(); ++i)
            castSpell(targets[i], marks[i / 2], true);
    }

    uint32_t mShellGraceMs = 0;
};

class EternalPalaceCoralGrowthAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceCoralGrowthAI(c); }
    explicit EternalPalaceCoralGrowthAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        _applyAura(SPELL_CORAL_GROWTH_VISUAL);
        castSpellOnSelf(SPELL_CORAL_CUTTING_CORAL, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Radiance of Azshara

class EternalPalaceRadianceAI : public EternalPalaceBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceRadianceAI(c); }
    explicit EternalPalaceRadianceAI(Creature* pCreature) : EternalPalaceBossAI(pCreature, DATA_RADIANCE_OF_AZSHARA) {}

    // the slain Stormwraith ends the tempest
    void OnSummonDies(Creature* pSummon, Unit* /*pKiller*/) override
    {
        if (pSummon != nullptr && pSummon->getEntry() == NPC_RADIANCE_STORMWRAITH && mStage == RadianceStageTempest && _isInCombat())
            beginCalm();
    }

protected:
    void onEngage() override
    {
        setRooted(true);
        armBerserk(PalaceBerserkMs);
        beginCalm();
    }

    void onDisengage() override
    {
        setRooted(false);
        _removeAura(SPELL_RADIANCE_ANCIENT_TEMPEST);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RADIANCE_TIDE_FIST:
                castSpellOnVictim(SPELL_RADIANCE_TIDE_FIST);
                scriptEvents.addEvent(EVENT_RADIANCE_TIDE_FIST, 18000);
                break;
            case EVENT_RADIANCE_ARCANADO_BURST:
                castSpellOnSelf(SPELL_RADIANCE_ARCANADO_BURST);
                scriptEvents.addEvent(EVENT_RADIANCE_ARCANADO_STRIKES, PalaceFollowUpMs);
                scriptEvents.addEvent(EVENT_RADIANCE_ARCANADO_BURST, 25000);
                break;
            case EVENT_RADIANCE_ARCANADO_STRIKES:
                for (Player* pTarget : pickPlayers(3, false))
                    getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_RADIANCE_ARCANADO_MISSILE, true);
                break;
            case EVENT_RADIANCE_UNSHACKLED_POWER:
                castSpellOnVictim(SPELL_RADIANCE_UNSHACKLED_POWER);
                scriptEvents.addEvent(EVENT_RADIANCE_UNSHACKLED_POWER, 20000);
                break;
            case EVENT_RADIANCE_ARCANE_BOMB:
                castSpellOnSelf(SPELL_RADIANCE_ARCANE_BOMB);
                scriptEvents.addEvent(EVENT_RADIANCE_PLANT_BOMBS, PalaceFollowUpMs);
                scriptEvents.addEvent(EVENT_RADIANCE_ARCANE_BOMB, 17000);
                break;
            case EVENT_RADIANCE_PLANT_BOMBS:
                for (Player* pTarget : pickPlayers(3, true))
                    placeHelper(NPC_RADIANCE_ARCANE_BOMB, pTarget->GetPosition(), 8000);
                break;
            case EVENT_RADIANCE_ANCIENT_TEMPEST:
                beginTempest();
                break;
            case EVENT_RADIANCE_SQUALL_TRAP:
                for (LocationVector const& spot : RadianceSquallTraps)
                    getCreature()->castSpellLoc(spot, SPELL_RADIANCE_SQUALL_TRAP, true);
                break;
            case EVENT_RADIANCE_CALM_RETURNS:
                beginCalm();
                break;
            default:
                break;
        }
    }

private:
    void beginCalm()
    {
        enterStage(RadianceStageCalm);
        _removeAura(SPELL_RADIANCE_ANCIENT_TEMPEST);

        scriptEvents.addEvent(EVENT_RADIANCE_ARCANE_BOMB, 17000);
        scriptEvents.addEvent(EVENT_RADIANCE_TIDE_FIST, 18000);
        scriptEvents.addEvent(EVENT_RADIANCE_UNSHACKLED_POWER, 20000);
        scriptEvents.addEvent(EVENT_RADIANCE_ARCANADO_BURST, 25000);
        scriptEvents.addEvent(EVENT_RADIANCE_ANCIENT_TEMPEST, 105000);
    }

    void beginTempest()
    {
        enterStage(RadianceStageTempest);
        warn("Radiance of Azshara begins to summon an Ancient Tempest!");
        castSpellOnSelf(SPELL_RADIANCE_ANCIENT_TEMPEST);

        scriptEvents.addEvent(EVENT_RADIANCE_SQUALL_TRAP, 2000);
        scriptEvents.addEvent(EVENT_RADIANCE_ARCANE_BOMB, 17000);

        // without its Stormwraith the tempest blows over on its own
        if (callAdd(NPC_RADIANCE_STORMWRAITH, RadianceStormwraithSpots[Util::getRandomUInt(1)]) == nullptr)
            scriptEvents.addEvent(EVENT_RADIANCE_CALM_RETURNS, 30000);
    }
};

// detonates where it was planted
class EternalPalaceArcaneBombAI : public EternalPalaceHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceArcaneBombAI(c); }
    explicit EternalPalaceArcaneBombAI(Creature* pCreature) : EternalPalaceHelperAI(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (countdown(time_passed, mFuseMs))
            castSpellAOE(SPELL_ARCANE_BOMB_DETONATION);
    }

private:
    uint32_t mFuseMs = 100;
};

class EternalPalaceStormwraithAI : public EternalPalaceCombatAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceStormwraithAI(c); }
    explicit EternalPalaceStormwraithAI(Creature* pCreature) : EternalPalaceCombatAI(pCreature) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_STORMWRAITH_FOCUS_POWER);
        scriptEvents.addEvent(EVENT_STORMWRAITH_GALE_BUFFET, 10000);
        scriptEvents.addEvent(EVENT_STORMWRAITH_STORMLINGS, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_STORMWRAITH_GALE_BUFFET:
                castSpellOnVictim(SPELL_STORMWRAITH_GALE_BUFFET);
                scriptEvents.addEvent(EVENT_STORMWRAITH_GALE_BUFFET, 10000);
                break;
            case EVENT_STORMWRAITH_STORMLINGS:
                // stormlings outlive the wraith and leave once the fight is over
                for (uint8_t i = 0; i < 3; ++i)
                    callAdd(NPC_RADIANCE_STORMLING, palaceScatter(getCreature()->GetPosition(), 5.0f, 10.0f), TIMED_DESPAWN_OUT_OF_COMBAT, 5000);
                scriptEvents.addEvent(EVENT_STORMWRAITH_STORMLINGS, 20000);
                break;
            default:
                break;
        }
    }
};

class EternalPalaceStormlingAI : public EternalPalaceRepeatCasterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceStormlingAI(c); }
    explicit EternalPalaceStormlingAI(Creature* pCreature) : EternalPalaceRepeatCasterAI(pCreature, SPELL_STORMLING_CHAIN_LIGHTNING, PalaceAim::RandomPlayer, 10000, 10000, false) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Queen's Court: Silivaz the Zealous and Pashmar the Fanatical

// a member at death's door holds on until the other one falls as well
class EternalPalaceCourtMemberAI : public EternalPalaceBossAI
{
public:
    explicit EternalPalaceCourtMemberAI(Creature* pCreature) : EternalPalaceBossAI(pCreature, DATA_THE_QUEENS_COURT) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mFalling)
            return;

        // nothing hurts while Desperate Measures run
        if (mHoldingOn)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        if (*damage < health)
            return;

        EternalPalaceCourtMemberAI* pPartner = partnerScript();
        if (pPartner == nullptr)
            return;

        // the partner was holding on, both fall together
        if (pPartner->isHoldingOn())
        {
            pPartner->fallTogether();
            return;
        }

        *damage = health > 1 ? health - 1 : 0;
        holdOn();
    }

    bool isHoldingOn() const { return mHoldingOn; }

    void fallTogether()
    {
        mHoldingOn = false;
        mFalling = true;
        getCreature()->dealDamage(getCreature(), getCreature()->getHealth(), 0);
    }

protected:
    void onEngage() override
    {
        mHoldingOn = false;
        mFalling = false;
        armBerserk(PalaceBerserkMs);
        scriptEvents.addEvent(EVENT_COURT_SEPARATION_CHECK, 2000);
        onCourtEngage();
    }

    void onDisengage() override
    {
        mHoldingOn = false;
        mFalling = false;
        _removeAura(SPELL_COURT_SEPARATION_OF_POWER);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mHoldingOn)
            onCourtTick(diff);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_COURT_SEPARATION_CHECK:
                checkSeparation();
                scriptEvents.addEvent(EVENT_COURT_SEPARATION_CHECK, 2000);
                break;
            case EVENT_COURT_MEASURES_OVER:
                mHoldingOn = false;
                scriptEvents.addEvent(EVENT_COURT_SEPARATION_CHECK, 2000);
                onCourtRecover();
                break;
            default:
                onCourtEvent(eventId);
                break;
        }
    }

    virtual void onCourtEngage() {}
    virtual void onCourtRecover() {}
    virtual void onCourtTick(uint32_t /*diff*/) {}
    virtual void onCourtEvent(uint32_t /*eventId*/) {}

private:
    Creature* partner()
    {
        EternalPalaceInstanceScript* pPalace = getPalace();
        return pPalace != nullptr ? pPalace->getCourtPartner(getCreature()) : nullptr;
    }

    EternalPalaceCourtMemberAI* partnerScript()
    {
        Creature* pPartner = partner();
        if (pPartner == nullptr || pPartner->GetScript() == nullptr)
            return nullptr;

        return dynamic_cast<EternalPalaceCourtMemberAI*>(pPartner->GetScript());
    }

    // the window to strike down the other member
    void holdOn()
    {
        mHoldingOn = true;
        scriptEvents.resetEvents();
        getCreature()->interruptSpell();

        warn("A member of the Queen's Court resorts to Desperate Measures!");
        castSpellOnSelf(SPELL_COURT_DESPERATE_MEASURES);
        scriptEvents.addEvent(EVENT_COURT_MEASURES_OVER, CourtMeasuresMs);
    }

    // standing close together empowers both
    void checkSeparation()
    {
        Creature* pPartner = partner();
        bool const close = pPartner != nullptr && getRangeToObject(pPartner) <= 21.0f;
        bool const empowered = hasAura(SPELL_COURT_SEPARATION_OF_POWER);

        if (close && !empowered)
            castSpellOnSelf(SPELL_COURT_SEPARATION_OF_POWER, true);
        else if (!close && empowered)
            _removeAura(SPELL_COURT_SEPARATION_OF_POWER);
    }

    bool mHoldingOn = false;
    bool mFalling = false;
};

class EternalPalaceSilivazAI : public EternalPalaceCourtMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceSilivazAI(c); }
    explicit EternalPalaceSilivazAI(Creature* pCreature) : EternalPalaceCourtMemberAI(pCreature) {}

protected:
    void onCourtEngage() override
    {
        mEruption.reset();
        castSpellOnSelf(SPELL_SILIVAZ_COMMANDERS_FURY, true);
        scriptEvents.addEvent(EVENT_SILIVAZ_FRENETIC_CHARGE, 30000);
    }

    void onCourtRecover() override { scriptEvents.addEvent(EVENT_SILIVAZ_FRENETIC_CHARGE, 30000); }

    void onCourtTick(uint32_t diff) override
    {
        if (mEruption.fill(diff))
            scriptEvents.addEvent(EVENT_SILIVAZ_ZEALOUS_ERUPTION, 100);
    }

    void onCourtEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_SILIVAZ_FRENETIC_CHARGE:
                if (Player* pTarget = pickPlayer(true))
                {
                    palaceSelfApply(pTarget, SPELL_SILIVAZ_FRENETIC_CHARGE_MARK);
                    castSpell(pTarget, SPELL_SILIVAZ_FRENETIC_CHARGE);
                    mChargeTarget = pTarget->GetNewGUID();
                    scriptEvents.addEvent(EVENT_SILIVAZ_CHARGE_LEAP, PalaceFollowUpMs);
                }
                scriptEvents.addEvent(EVENT_SILIVAZ_FRENETIC_CHARGE, 30000);
                break;
            case EVENT_SILIVAZ_CHARGE_LEAP:
                if (Unit* pTarget = findUnit(mChargeTarget))
                    castSpell(pTarget, SPELL_SILIVAZ_FRENETIC_CHARGE_LEAP, true);
                mChargeTarget.clear();
                break;
            case EVENT_SILIVAZ_ZEALOUS_ERUPTION:
                warn("Silivaz the Zealous begins to cast Zealous Eruption!");
                castSpellOnSelf(SPELL_SILIVAZ_ZEALOUS_ERUPTION);
                break;
            default:
                break;
        }
    }

private:
    PalaceGauge mEruption{ 1000, 1, 2 };
    WoWGuid mChargeTarget;
};

class EternalPalacePashmarAI : public EternalPalaceCourtMemberAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalacePashmarAI(c); }
    explicit EternalPalacePashmarAI(Creature* pCreature) : EternalPalaceCourtMemberAI(pCreature) {}

protected:
    void onCourtEngage() override
    {
        mOutburst.reset();
        castSpellOnSelf(SPELL_PASHMAR_PASHMARS_TOUCH, true);
        onCourtRecover();
    }

    void onCourtRecover() override
    {
        scriptEvents.addEvent(EVENT_PASHMAR_FANATICAL_VERDICT, 30000);
        scriptEvents.addEvent(EVENT_PASHMAR_POTENT_SPARK, 90000);
    }

    void onCourtTick(uint32_t diff) override
    {
        if (mOutburst.fill(diff))
            scriptEvents.addEvent(EVENT_PASHMAR_VIOLENT_OUTBURST, 100);
    }

    void onCourtEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PASHMAR_FANATICAL_VERDICT:
                castSpellOnSelf(SPELL_PASHMAR_FANATICAL_VERDICT);
                scriptEvents.addEvent(EVENT_PASHMAR_MARK_ZEALOTS, PalaceFollowUpMs);
                scriptEvents.addEvent(EVENT_PASHMAR_FANATICAL_VERDICT, 30000);
                break;
            case EVENT_PASHMAR_MARK_ZEALOTS:
                for (Player* pTarget : pickPlayers(3, true))
                    palaceSelfApply(pTarget, SPELL_PASHMAR_FANATICAL_VERDICT_MARK);
                break;
            case EVENT_PASHMAR_POTENT_SPARK:
                castSpellOnSelf(SPELL_PASHMAR_POTENT_SPARK);
                scriptEvents.addEvent(EVENT_PASHMAR_SCATTER_SPARKS, PalaceFollowUpMs);
                scriptEvents.addEvent(EVENT_PASHMAR_POTENT_SPARK, 90000);
                break;
            case EVENT_PASHMAR_SCATTER_SPARKS:
                for (uint8_t i = 0; i < 3; ++i)
                    summonCreature(NPC_COURT_POTENT_SPARK, palaceScatter(CourtArenaCentre, 5.0f, 30.0f), CORPSE_DESPAWN);
                break;
            case EVENT_PASHMAR_VIOLENT_OUTBURST:
                warn("Pashmar the Fanatical begins to cast Violent Outburst!");
                castSpellOnSelf(SPELL_PASHMAR_VIOLENT_OUTBURST);
                break;
            default:
                break;
        }
    }

private:
    PalaceGauge mOutburst{ 1000, 1, 2 };
};

// bursts when it is destroyed
class EternalPalacePotentSparkAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalacePotentSparkAI(c); }
    explicit EternalPalacePotentSparkAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setRooted(true);
        _setMeleeDisabled(true);
        _applyAura(SPELL_SPARK_POTENT_SPARK);
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr || mDischarged || *damage < getCreature()->getHealth())
            return;

        mDischarged = true;
        castSpellAOE(isHeroic() ? SPELL_SPARK_DISCHARGE_HEROIC : SPELL_SPARK_DISCHARGE, true);
    }

private:
    bool mDischarged = false;
};

// the queen's image above the court enacts a decree on a timer
class EternalPalaceDecreeImageAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new EternalPalaceDecreeImageAI(c); }
    explicit EternalPalaceDecreeImageAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (!isCourtImage())
            return;

        if (!mPrepared)
        {
            mPrepared = true;
            setReactState(REACT_PASSIVE);
            setCanEnterCombat(false);
            setRooted(true);
            getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }

        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr || pInstance->getBossState(DATA_THE_QUEENS_COURT) != EncounterStates::InProgress)
        {
            mNextDecree = 0;
            mDecreeLeftMs = 30000;
            return;
        }

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mDecreeLeftMs > diff)
        {
            mDecreeLeftMs -= diff;
            return;
        }

        enactDecree();
        mDecreeLeftMs = isHeroic() ? 45000 : 60000;
    }

private:
    // the intro image elsewhere in the palace stays idle
    bool isCourtImage()
    {
        return getCreature()->GetMapId() == MAP_THE_ETERNAL_PALACE && getCreature()->GetPosition().getExactDist(CourtDecreeSpot) <= 10.0f;
    }

    void enactDecree()
    {
        CourtDecree const& decree = CourtDecrees[mNextDecree];
        mNextDecree = (mNextDecree + 1) % CourtDecreeCount;

        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, decree.text);
        for (Player* pPlayer : palacePlayersNear(getCreature(), 150.0f))
            palaceSelfApply(pPlayer, decree.spellId);
    }

    bool mPrepared = false;
    size_t mNextDecree = 0;
    uint32_t mDecreeLeftMs = 30000;
};

void SetupTheEternalPalace(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_ETERNAL_PALACE, &EternalPalaceInstanceScript::Create);

    // Blackwater Behemoth
    mgr->register_creature_script(BOSS_BLACKWATER_BEHEMOTH, &EternalPalaceBehemothAI::Create);

    // Za'qul
    mgr->register_creature_script(BOSS_ZAQUL, &EternalPalaceZaqulAI::Create);
    mgr->register_creature_script(NPC_ZAQUL_HORRIFIC_SUMMONER, &EternalPalaceHorrificSummonerAI::Create);
    mgr->register_creature_script(NPC_ZAQUL_PASSAGE_SUMMONER, &EternalPalacePassageSummonerAI::Create);

    // Abyssal Commander Sivara
    mgr->register_creature_script(BOSS_ABYSSAL_COMMANDER_SIVARA, &EternalPalaceSivaraAI::Create);

    // Queen Azshara
    mgr->register_creature_script(BOSS_QUEEN_AZSHARA, &EternalPalaceAzsharaAI::Create);
    mgr->register_creature_script(NPC_AZSHARA_AETHANEL, &EternalPalaceAethanelAI::Create);
    mgr->register_creature_script(NPC_AZSHARA_CYRANUS, &EternalPalaceCyranusAI::Create);
    mgr->register_creature_script(NPC_AZSHARA_ANCIENT_WARD, &EternalPalaceHelperAI::Create);
    mgr->register_creature_script(NPC_AZSHARA_LIGHTNING_ORB, &EternalPalaceLightningOrbAI::Create);
    mgr->register_creature_script(NPC_AZSHARA_OVERZEALOUS_HULK, &EternalPalaceOverzealousHulkAI::Create);
    mgr->register_creature_script(NPC_AZSHARA_INDOMITABLE, &EternalPalaceDevotedServantAI::Create);
    mgr->register_creature_script(NPC_AZSHARA_DEVOTED, &EternalPalaceDevotedServantAI::Create);
    mgr->register_creature_script(NPC_AZSHARA_LADY_VENOMTONGUE, &EternalPalaceTidemistressAI::Create);
    mgr->register_creature_script(NPC_AZSHARA_SERENA_SCARSCALE, &EternalPalaceTidemistressAI::Create);
    mgr->register_creature_script(NPC_AZSHARA_VENNA_SEASTORM, &EternalPalaceTidemistressAI::Create);
    mgr->register_creature_script(NPC_AZSHARA_LOYAL_MYRMIDON, &EternalPalaceLoyalMyrmidonAI::Create);

    // Orgozoa
    mgr->register_creature_script(BOSS_ORGOZOA, &EternalPalaceOrgozoaAI::Create);
    mgr->register_creature_script(NPC_ORGOZOA_ZOATROID, &EternalPalaceZoatroidAI::Create);
    mgr->register_creature_script(NPC_ORGOZOA_ZANJIR_MYRMIDON, &EternalPalaceZanjirMyrmidonAI::Create);
    mgr->register_creature_script(NPC_ORGOZOA_AZSHARI_WITCH, &EternalPalaceAzshariWitchAI::Create);
    mgr->register_creature_script(NPC_ORGOZOA_DREADCOIL_HULK, &EternalPalaceDreadcoilHulkAI::Create);

    // Lady Ashvane
    mgr->register_creature_script(BOSS_LADY_ASHVANE, &EternalPalaceAshvaneAI::Create);
    mgr->register_creature_script(NPC_ASHVANE_CORAL_GROWTH, &EternalPalaceCoralGrowthAI::Create);

    // Radiance of Azshara
    mgr->register_creature_script(BOSS_RADIANCE_OF_AZSHARA, &EternalPalaceRadianceAI::Create);
    mgr->register_creature_script(NPC_RADIANCE_ARCANE_BOMB, &EternalPalaceArcaneBombAI::Create);
    mgr->register_creature_script(NPC_RADIANCE_STORMWRAITH, &EternalPalaceStormwraithAI::Create);
    mgr->register_creature_script(NPC_RADIANCE_STORMLING, &EternalPalaceStormlingAI::Create);

    // The Queen's Court
    mgr->register_creature_script(BOSS_SILIVAZ_THE_ZEALOUS, &EternalPalaceSilivazAI::Create);
    mgr->register_creature_script(BOSS_PASHMAR_THE_FANATICAL, &EternalPalacePashmarAI::Create);
    mgr->register_creature_script(NPC_COURT_POTENT_SPARK, &EternalPalacePotentSparkAI::Create);
    mgr->register_creature_script(NPC_COURT_QUEEN_AZSHARA_IMAGE, &EternalPalaceDecreeImageAI::Create);
}
