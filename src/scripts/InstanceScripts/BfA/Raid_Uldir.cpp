/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Uldir.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

enum UldirEvents
{
    // Taloc
    EVENT_ULDIR_TALOC_PLASMA = 1,
    EVENT_ULDIR_TALOC_CUDGEL,
    EVENT_ULDIR_TALOC_POWER_DOWN,
    EVENT_ULDIR_TALOC_POWER_UP,

    // MOTHER
    EVENT_ULDIR_MOTHER_STRIKE,
    EVENT_ULDIR_MOTHER_FLAMES,

    // Fetid Devourer
    EVENT_ULDIR_DEVOURER_THRASH,
    EVENT_ULDIR_DEVOURER_MIASMA,
    EVENT_ULDIR_DEVOURER_REGURGITATION,
    EVENT_ULDIR_DEVOURER_STOMP,
    EVENT_ULDIR_DEVOURER_DISPOSAL,

    // Zek'voz
    EVENT_ULDIR_ZEKVOZ_MIGHT,
    EVENT_ULDIR_ZEKVOZ_LASH,
    EVENT_ULDIR_ZEKVOZ_SPARK,
    EVENT_ULDIR_ZEKVOZ_SPARK_PULSE,
    EVENT_ULDIR_ZEKVOZ_DARKNESS,
    EVENT_ULDIR_ZEKVOZ_DARKNESS_ERUPTS,
    EVENT_ULDIR_ZEKVOZ_EYE_BEAM,
    EVENT_ULDIR_ZEKVOZ_SILITHIDS,
    EVENT_ULDIR_ZEKVOZ_DECEIT,
    EVENT_ULDIR_ZEKVOZ_NERUBIANS,
    EVENT_ULDIR_ZEKVOZ_ORB,
    EVENT_ULDIR_ZEKVOZ_PSIONIC_BLAST,

    // Vectis
    EVENT_ULDIR_VECTIS_VECTOR,
    EVENT_ULDIR_VECTIS_AFFLICTION,
    EVENT_ULDIR_VECTIS_GESTATE,
    EVENT_ULDIR_VECTIS_GESTATE_BURST,
    EVENT_ULDIR_VECTIS_CONTAGION,
    EVENT_ULDIR_VECTIS_LIQUEFY,
    EVENT_ULDIR_VECTIS_GEYSER,
    EVENT_ULDIR_VECTIS_PLAGUE_BOMB,
    EVENT_ULDIR_VECTIS_REFORM,

    // Zul
    EVENT_ULDIR_ZUL_BARRAGE,
    EVENT_ULDIR_ZUL_REVELATION,
    EVENT_ULDIR_ZUL_REVELATION_BURST,
    EVENT_ULDIR_ZUL_CALL_OF_BLOOD,
    EVENT_ULDIR_ZUL_POOL,
    EVENT_ULDIR_ZUL_LOCUS,
    EVENT_ULDIR_ZUL_LOCUS_SPREAD,
    EVENT_ULDIR_ZUL_RUPTURING_BLOOD,
    EVENT_ULDIR_ZUL_DEATHWISH,
    EVENT_ULDIR_BLOODHEXER_CONGEAL,

    // Mythrax
    EVENT_ULDIR_MYTHRAX_SHEAR,
    EVENT_ULDIR_MYTHRAX_RUIN,
    EVENT_ULDIR_MYTHRAX_SPHERES,
    EVENT_ULDIR_MYTHRAX_AWAKENING,
    EVENT_ULDIR_MYTHRAX_VEIL,
    EVENT_ULDIR_MYTHRAX_VISIONS,
    EVENT_ULDIR_MYTHRAX_DESTROYERS,
    EVENT_ULDIR_MYTHRAX_RETURN,

    // G'huun
    EVENT_ULDIR_GHUUN_EXPLOSIVE,
    EVENT_ULDIR_GHUUN_MAWS,
    EVENT_ULDIR_GHUUN_TENDRIL,
    EVENT_ULDIR_GHUUN_MATRIX,
    EVENT_ULDIR_GHUUN_BITE,
    EVENT_ULDIR_GHUUN_WAVE,
    EVENT_ULDIR_GHUUN_FEAST,
    EVENT_ULDIR_GHUUN_GROWTH,
    EVENT_ULDIR_GHUUN_GAZE,
    EVENT_ULDIR_GHUUN_MASS_FADES,

    // timed spells of the adds use one event per table slot
    EVENT_ULDIR_ADD_SPELL_FIRST = 200
};

namespace
{
    float const UldirTwoPi = 6.28318531f;

    uint32_t const UldirBerserkMs = 600000;
    uint32_t const DevourerBerserkMs = 330000;

    // energy bars and the power matrix run as fill times
    uint32_t const DevourerRegurgitationMs = 41000;
    uint32_t const GhuunMatrixDeliveryMs = 25000;
    uint8_t const GhuunMatrixDeliveries = 3;

    size_t const UldirWholeRaid = 40;
    uint32_t const MotherFriendlyFaction = 35;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Positions

    LocationVector const TalocPlatformCentre(-268.0f, -255.0f, 695.302f, 3.27f);

    LocationVector const MotherExitSpot(70.888f, -255.695f, 704.580f, 0.04f);

    LocationVector const DevourerChuteSpots[4] =
    {
        { 290.625f, -622.588f, 694.882f, 3.13f },
        { 216.449f, -623.721f, 694.882f, 3.13f },
        { 234.059f, -719.541f, 694.882f, 3.13f },
        { 278.369f, -719.677f, 694.799f, 2.917f }
    };

    LocationVector const ZekvozOrbSpots[4] =
    {
        { 237.093f, 157.093f, 737.432f, 1.46f },
        { 267.422f, 170.200f, 725.991f, 2.25f },
        { 265.425f, 186.500f, 735.251f, 4.30f },
        { 231.283f, 187.813f, 733.033f, 0.24f }
    };

    LocationVector const VectisBombSpots[12] =
    {
        { 640.71f, -264.544f, 694.698f, 3.85f },
        { 629.74f, -264.857f, 694.698f, 2.73f },
        { 618.33f, -264.711f, 694.698f, 5.43f },
        { 678.93f, -264.877f, 694.698f, 0.73f },
        { 688.71f, -264.808f, 694.698f, 3.85f },
        { 698.46f, -264.965f, 694.698f, 2.73f },
        { 698.86f, -256.172f, 694.698f, 5.43f },
        { 689.23f, -256.232f, 694.698f, 0.73f },
        { 679.75f, -256.292f, 694.698f, 3.85f },
        { 668.16f, -256.365f, 694.698f, 2.73f },
        { 629.98f, -247.196f, 694.698f, 5.43f },
        { 618.10f, -247.182f, 694.698f, 0.73f }
    };

    LocationVector const MythraxCentre(255.149f, -255.744f, 542.909f, 3.65f);

    LocationVector const MythraxDestroyerSpots[2] =
    {
        { 228.434f, -238.122f, 542.908f, 5.736f },
        { 246.924f, -286.016f, 542.908f, 1.136f }
    };

    LocationVector const GhuunPoolCentre(503.035f, -255.701f, 459.482f, 6.26f);

    // G'huun's brood, every Reorigination Blast stuns it
    uint32_t const GhuunBrood[] =
    {
        NPC_GHUUN_CYCLOPEAN_TERROR, NPC_GHUUN_CYCLOPEAN_TERROR_ALT, NPC_GHUUN_DARK_YOUNG, NPC_GHUUN_DARK_YOUNG_ALT,
        NPC_GHUUN_GIBBERING_HORROR, NPC_GHUUN_BLIGHTSPREADER_TENDRIL
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    // Doors

    enum class UldirDoorKind : uint8_t
    {
        Room,       // shut while the encounter runs
        Passage     // opens after the encounter
    };

    struct UldirDoorRule
    {
        uint32_t entry;
        uint32_t dataId;
        UldirDoorKind kind;
    };

    UldirDoorRule const UldirDoorRules[] =
    {
        { GO_ZUL_CHAMBER_DOOR_1,    DATA_ZUL,       UldirDoorKind::Room },
        { GO_ZUL_CHAMBER_DOOR_2,    DATA_ZUL,       UldirDoorKind::Room },
        { GO_ZUL_CHAMBER_DOOR_3,    DATA_ZUL,       UldirDoorKind::Room },
        { GO_ZUL_DESCENT_HATCH,     DATA_ZUL,       UldirDoorKind::Passage },
        { GO_MYTHRAX_EXIT_DOOR,     DATA_MYTHRAX,   UldirDoorKind::Passage }
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    // Timed spells of the adds

    enum class UldirAim : uint8_t
    {
        Victim,
        RandomPlayer,
        Area,
        Self
    };

    // a zero repeat fires once
    struct UldirAddSpell
    {
        uint32_t spellId;
        UldirAim aim;
        uint32_t firstMs;
        uint32_t minRepeatMs;
        uint32_t maxRepeatMs;
        bool triggered;
    };

    std::vector<UldirAddSpell> const SilithidWarriorSpells =
    {
        { SPELL_SILITHID_SKITTERING_SWARM,  UldirAim::Self,         500,    0,      0,      true },
        { SPELL_SILITHID_JAGGED_MANDIBLE,   UldirAim::Victim,       2000,   2000,   2000,   true }
    };

    std::vector<UldirAddSpell> const NerubianVoidweaverSpells =
    {
        { SPELL_VOIDWEAVER_VOID_BOLT,       UldirAim::Area,         3000,   6000,   6000,   false }
    };

    std::vector<UldirAddSpell> const PlagueAmalgamSpells =
    {
        { SPELL_AMALGAM_IMMUNOSUPPRESSION,  UldirAim::Area,         5000,   10000,  10000,  false }
    };

    std::vector<UldirAddSpell> const NazmaniCrusherSpells =
    {
        { SPELL_CRUSHER_BLOODY_CLEAVE,      UldirAim::Victim,       3000,   15000,  18000,  false },
        { SPELL_CRUSHER_THRUMMING_PULSE,    UldirAim::Area,         20000,  25000,  25000,  false }
    };

    std::vector<UldirAddSpell> const NazmaniBloodhexerSpells =
    {
        { SPELL_BLOODHEXER_BLOODSHARD,      UldirAim::RandomPlayer, 3000,   2000,   2000,   false }
    };

    std::vector<UldirAddSpell> const VisionOfMadnessSpells =
    {
        { SPELL_VISION_MIND_FLAY,           UldirAim::RandomPlayer, 2000,   5000,   5000,   false }
    };

    std::vector<UldirAddSpell> const NraqiDestroyerSpells =
    {
        { SPELL_DESTROYER_VOID_VOLLEY,      UldirAim::RandomPlayer, 4000,   8000,   8000,   false }
    };

    std::vector<UldirAddSpell> const CyclopeanTerrorSpells =
    {
        { SPELL_TERROR_TORMENT,             UldirAim::RandomPlayer, 5000,   20000,  20000,  false }
    };

    std::vector<UldirAddSpell> const DarkYoungSpells =
    {
        { SPELL_YOUNG_MASSIVE_SMASH,        UldirAim::Victim,       5000,   20000,  20000,  false },
        { SPELL_YOUNG_DARK_BARGAIN,         UldirAim::Area,         30000,  30000,  30000,  false }
    };

    std::vector<UldirAddSpell> const GibberingHorrorSpells =
    {
        { SPELL_HORROR_MIND_NUMBING_CHATTER, UldirAim::Area,        5000,   15000,  15000,  false }
    };

    std::vector<UldirAddSpell> const BlightspreaderTendrilSpells =
    {
        { SPELL_TENDRIL_DECAYING_ERUPTION,  UldirAim::Area,         5000,   15000,  15000,  false }
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    // Helpers

    // random point on a ring around origin
    LocationVector uldirRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(UldirTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    Creature* uldirLivingCreature(InstanceScript* pInstance, uint32_t entry)
    {
        if (pInstance == nullptr)
            return nullptr;

        for (Creature* pCreature : pInstance->getCreatureSetForEntry(entry))
        {
            if (pCreature->isAlive())
                return pCreature;
        }

        return nullptr;
    }

    // a player picked for a delayed effect
    struct UldirMark
    {
        WoWGuid guid;
        LocationVector spot;
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class UldirInstanceScript : public InstanceScript
{
public:
    explicit UldirInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(UldirEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new UldirInstanceScript(pMapMgr); }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (pGameObject->getEntry() == GO_MOTHER_REWARD_CACHE)
        {
            mCacheGuids.push_back(pGameObject->GetNewGUID());
            guardCache(pGameObject);
            return;
        }

        if (findDoorRule(pGameObject->getEntry()) == nullptr)
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        alignDoor(pGameObject);
    }

    void OnEncounterStateChange(uint32_t /*id*/, uint32_t /*state*/) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                alignDoor(pDoor);
        }

        for (WoWGuid const& guid : mCacheGuids)
        {
            if (GameObject* pCache = getGameObjectByGuid(guid))
                guardCache(pCache);
        }
    }

private:
    UldirDoorRule const* findDoorRule(uint32_t entry) const
    {
        for (UldirDoorRule const& rule : UldirDoorRules)
        {
            if (rule.entry == entry)
                return &rule;
        }

        return nullptr;
    }

    bool isDoorOpen(UldirDoorRule const& rule) const
    {
        if (rule.kind == UldirDoorKind::Room)
            return getBossState(rule.dataId) != EncounterStates::InProgress;

        return getBossState(rule.dataId) == EncounterStates::Performed;
    }

    // changes the state only when it differs
    void alignDoor(GameObject* pDoor)
    {
        UldirDoorRule const* pRule = findDoorRule(pDoor->getEntry());
        if (pRule == nullptr)
            return;

        bool const open = isDoorOpen(*pRule);
        bool const isOpen = pDoor->getState() != GO_STATE_CLOSED;
        if (isOpen != open)
            pDoor->setState(open ? GO_STATE_OPEN : GO_STATE_CLOSED);
    }

    // the cache can be looted once MOTHER stood down
    void guardCache(GameObject* pCache)
    {
        if (getBossState(DATA_MOTHER) == EncounterStates::Performed)
            pCache->removeFlags(GO_FLAG_NOT_SELECTABLE);
        else
            pCache->addFlags(GO_FLAG_NOT_SELECTABLE);
    }

    std::vector<WoWGuid> mDoorGuids;
    std::vector<WoWGuid> mCacheGuids;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick, berserk and target helpers

class UldirCombatAI : public CreatureAIScript
{
public:
    explicit UldirCombatAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStage = 1;
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

        // every due event runs unless a cast holds the queue
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

        for (size_t i = 0; i < pool.size() && i < count; ++i)
        {
            size_t const pick = i + Util::getRandomUInt(static_cast<uint32_t>(pool.size() - 1 - i));
            std::swap(pool[i], pool[pick]);
        }

        if (pool.size() > count)
            pool.resize(count);

        return pool;
    }

    Unit* pickPlayer(bool skipTank = false)
    {
        std::vector<Unit*> const picked = pickPlayers(1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        Unit* pTarget = pickPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    void castAtPlayers(uint32_t spellId, size_t count, bool skipTank = false, bool triggered = true)
    {
        for (Unit* pTarget : pickPlayers(count, skipTank))
            castSpell(pTarget, spellId, triggered);
    }

    // debuffs aimed at their bearer are cast by the bearer
    void afflict(Unit* pTarget, uint32_t spellId)
    {
        if (pTarget != nullptr && pTarget->isAlive())
            pTarget->castSpell(pTarget, spellId, true);
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // summoned helper that stays out of the fight
    Creature* placeHelper(uint32_t entry, LocationVector const& spot)
    {
        return summonCreature(entry, spot, MANUAL_DESPAWN);
    }

    LocationVector aroundSelf(float minRange, float maxRange)
    {
        return uldirRingPoint(getCreature()->GetPosition(), minRange, maxRange);
    }

    UldirMark markOf(Unit* pTarget) const
    {
        UldirMark mark;
        mark.guid = pTarget->GetNewGUID();
        mark.spot = pTarget->GetPosition();
        return mark;
    }

    // current spot of a marked player, the last known one when he is gone
    LocationVector markSpot(UldirMark& mark)
    {
        if (!mark.guid.isEmpty() && getCreature()->getWorldMap() != nullptr)
        {
            Unit* pTarget = getCreature()->getWorldMap()->getUnit(mark.guid);
            if (pTarget != nullptr && pTarget->isAlive())
                mark.spot = pTarget->GetPosition();
        }

        return mark.spot;
    }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    // a new stage drops every pending event
    void enterStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
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
        castSpellOnSelf(SPELL_ULDIR_BERSERK, true);
    }

    uint32_t mBerserkLeftMs = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame and boss state

class UldirBossAI : public UldirCombatAI
{
public:
    UldirBossAI(Creature* pCreature, uint32_t dataId) : UldirCombatAI(pCreature), mDataId(dataId) {}

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        UldirCombatAI::OnCombatStart(pTarget);
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

        UldirCombatAI::OnCombatStop(pTarget);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();
    }

protected:
    uint32_t mDataId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Adds driven by a spell table

class UldirAddAI : public UldirCombatAI
{
public:
    UldirAddAI(Creature* pCreature, std::vector<UldirAddSpell> const& spells, bool rooted)
        : UldirCombatAI(pCreature), mSpells(spells), mRooted(rooted) {}

protected:
    void onEngage() override
    {
        if (mRooted)
            setRooted(true);

        for (size_t i = 0; i < mSpells.size(); ++i)
            scriptEvents.addEvent(EVENT_ULDIR_ADD_SPELL_FIRST + static_cast<uint32_t>(i), mSpells[i].firstMs);
    }

    void onDisengage() override
    {
        if (mRooted)
            setRooted(false);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId < EVENT_ULDIR_ADD_SPELL_FIRST)
            return;

        size_t const slot = eventId - EVENT_ULDIR_ADD_SPELL_FIRST;
        if (slot >= mSpells.size())
            return;

        UldirAddSpell const& entry = mSpells[slot];
        switch (entry.aim)
        {
            case UldirAim::Victim:
                castSpellOnVictim(entry.spellId, entry.triggered);
                break;
            case UldirAim::RandomPlayer:
                castAtPlayer(entry.spellId, false, entry.triggered);
                break;
            case UldirAim::Area:
                castSpellAOE(entry.spellId, entry.triggered);
                break;
            case UldirAim::Self:
                castSpellOnSelf(entry.spellId, entry.triggered);
                break;
        }

        if (entry.maxRepeatMs == 0)
            return;

        uint32_t const delay = entry.maxRepeatMs > entry.minRepeatMs ? Util::getRandomUInt(entry.minRepeatMs, entry.maxRepeatMs) : entry.minRepeatMs;
        scriptEvents.addEvent(eventId, delay);
    }

private:
    std::vector<UldirAddSpell> const& mSpells;
    bool mRooted;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Helpers outside the fight

// immobile helper that never fights
class UldirHelperAI : public CreatureAIScript
{
public:
    explicit UldirHelperAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        setRooted(true);
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

protected:
    // counts down once, only summoned helpers act
    bool countdown(uint32_t diff, uint32_t& leftMs)
    {
        if (!getCreature()->isSummon() || leftMs == 0)
            return false;

        if (leftMs > diff)
        {
            leftMs -= diff;
            return false;
        }

        leftMs = 0;
        return true;
    }

    // nearest living player inside range
    Player* playerWithin(float range)
    {
        Player* pPlayer = getNearestPlayer();
        if (pPlayer == nullptr || !pPlayer->isAlive() || getRangeToObject(pPlayer) > range)
            return nullptr;

        return pPlayer;
    }
};

// fades after a while
class UldirFadingHelperAI : public UldirHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirFadingHelperAI(c); }
    explicit UldirFadingHelperAI(Creature* pCreature) : UldirHelperAI(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (countdown(static_cast<uint32_t>(time_passed), mFadeLeftMs))
            despawn(100, 0);
    }

private:
    uint32_t mFadeLeftMs = 10000;
};

// attackable add that walks to its boss and acts once it arrives
class UldirSeekerAI : public CreatureAIScript
{
public:
    UldirSeekerAI(Creature* pCreature, uint32_t bossEntry, uint32_t releaseSpell, float reach)
        : CreatureAIScript(pCreature), mBossEntry(bossEntry), mReleaseSpell(releaseSpell), mReach(reach) {}

    void OnLoad() override { setReactState(REACT_PASSIVE); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mArrived || !getCreature()->isAlive())
            return;

        if (!mReleased)
        {
            mReleased = true;
            castSpellOnSelf(mReleaseSpell, true);
        }

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mStepLeftMs > diff)
        {
            mStepLeftMs -= diff;
            return;
        }

        mStepLeftMs = 1000;

        Creature* pBoss = uldirLivingCreature(getInstanceScript(), mBossEntry);
        if (pBoss == nullptr || !pBoss->isInCombat())
            return;

        if (getRangeToObject(pBoss) > mReach)
        {
            moveTo(pBoss->GetPositionX(), pBoss->GetPositionY(), pBoss->GetPositionZ(), false);
            return;
        }

        mArrived = true;
        onArrival(pBoss);
        despawn(500, 0);
    }

protected:
    virtual void onArrival(Creature* /*pBoss*/) {}

private:
    uint32_t mBossEntry;
    uint32_t mReleaseSpell;
    float mReach;
    uint32_t mStepLeftMs = 0;
    bool mReleased = false;
    bool mArrived = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Taloc

class UldirTalocAI : public UldirBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirTalocAI(c); }
    explicit UldirTalocAI(Creature* pCreature) : UldirBossAI(pCreature, DATA_TALOC) {}

protected:
    void onEngage() override
    {
        armBerserk(UldirBerserkMs);
        startAssault();
    }

    void onDisengage() override { setReactState(REACT_AGGRESSIVE); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        // the platform starts to sink below 35%
        if (mStage != 1 || _getHealthPercent() > 35)
            return;

        enterStage(2);
        warn("Taloc's power falters and he shuts down!");
        setReactState(REACT_PASSIVE);
        attackStop();
        moveTo(TalocPlatformCentre.x, TalocPlatformCentre.y, TalocPlatformCentre.z);
        scriptEvents.addEvent(EVENT_ULDIR_TALOC_POWER_DOWN, 4000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ULDIR_TALOC_PLASMA:
                castAtPlayers(SPELL_TALOC_PLASMA_DISCHARGE, 3);
                scriptEvents.addEvent(EVENT_ULDIR_TALOC_PLASMA, 20000);
                break;
            case EVENT_ULDIR_TALOC_CUDGEL:
                if (castAtPlayer(SPELL_TALOC_CUDGEL_OF_GORE, true) != nullptr)
                    warn("Taloc raises the Cudgel of Gore!");
                scriptEvents.addEvent(EVENT_ULDIR_TALOC_CUDGEL, 30000);
                break;
            case EVENT_ULDIR_TALOC_POWER_DOWN:
                castSpellOnSelf(SPELL_TALOC_POWERED_DOWN, true);
                scriptEvents.addEvent(EVENT_ULDIR_TALOC_POWER_UP, 20000);
                break;
            case EVENT_ULDIR_TALOC_POWER_UP:
                _removeAura(SPELL_TALOC_POWERED_DOWN);
                setReactState(REACT_AGGRESSIVE);
                warn("Taloc powers up again!");
                startAssault();
                break;
            default:
                break;
        }
    }

private:
    void startAssault()
    {
        scriptEvents.addEvent(EVENT_ULDIR_TALOC_PLASMA, 5000);
        scriptEvents.addEvent(EVENT_ULDIR_TALOC_CUDGEL, 50000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// MOTHER, stands down at 2% instead of dying

class UldirMotherAI : public UldirBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirMotherAI(c); }
    explicit UldirMotherAI(Creature* pCreature) : UldirBossAI(pCreature, DATA_MOTHER) {}

    void OnLoad() override
    {
        InstanceScript* pInstance = getInstanceScript();
        if (pInstance != nullptr && pInstance->getBossState(DATA_MOTHER) == EncounterStates::Performed)
            despawn(1000, 0);
    }

    void OnCombatStop(Unit* pTarget) override
    {
        // a stood down MOTHER leaves combat without failing the encounter
        if (mStoodDown)
        {
            UldirCombatAI::OnCombatStop(pTarget);
            return;
        }

        UldirBossAI::OnCombatStop(pTarget);
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (damage == nullptr)
            return;

        if (mStoodDown)
        {
            *damage = 0;
            return;
        }

        uint32_t const health = getCreature()->getHealth();
        uint32_t const yieldHealth = std::max<uint32_t>(getCreature()->getMaxHealth() / 50, 1);
        if (health > *damage + yieldHealth)
            return;

        *damage = health > yieldHealth ? health - yieldHealth : 0;
        mYieldPending = true;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!mStoodDown)
        {
            UldirBossAI::AIUpdate(time_passed);
            return;
        }

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (tick(diff, mWalkLeftMs))
            moveTo(MotherExitSpot.x, MotherExitSpot.y, MotherExitSpot.z, false);

        if (tick(diff, mVanishLeftMs))
            despawn(500, 0);
    }

protected:
    void onEngage() override
    {
        mYieldPending = false;
        armBerserk(UldirBerserkMs);
        scriptEvents.addEvent(EVENT_ULDIR_MOTHER_STRIKE, 3000);
        scriptEvents.addEvent(EVENT_ULDIR_MOTHER_FLAMES, 6000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mYieldPending && !mStoodDown)
            standDown();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ULDIR_MOTHER_STRIKE:
                castSpellOnVictim(SPELL_MOTHER_SANITIZING_STRIKE);
                scriptEvents.addEvent(EVENT_ULDIR_MOTHER_STRIKE, 20000);
                break;
            case EVENT_ULDIR_MOTHER_FLAMES:
                warn("MOTHER releases the Purifying Flame!");
                for (Unit* pTarget : pickPlayers(UldirWholeRaid))
                    placeHelper(NPC_MOTHER_PURIFYING_FLAME, pTarget->GetPosition());
                scriptEvents.addEvent(EVENT_ULDIR_MOTHER_FLAMES, 25000);
                break;
            default:
                break;
        }
    }

private:
    void standDown()
    {
        mYieldPending = false;
        mStoodDown = true;
        scriptEvents.resetEvents();
        summons.despawnAll();
        warn("MOTHER's defense protocols shut down!");

        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            pInstance->setBossState(DATA_MOTHER, EncounterStates::Performed);
        }

        setReactState(REACT_PASSIVE);
        attackStop();
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->setFaction(MotherFriendlyFaction);
        setCanEnterCombat(false);

        mWalkLeftMs = 4000;
        mVanishLeftMs = 14000;

        if (_isInCombat())
            getCreature()->getAIInterface()->enterEvadeMode();
    }

    static bool tick(uint32_t diff, uint32_t& leftMs)
    {
        if (leftMs == 0)
            return false;

        if (leftMs > diff)
        {
            leftMs -= diff;
            return false;
        }

        leftMs = 0;
        return true;
    }

    bool mYieldPending = false;
    bool mStoodDown = false;
    uint32_t mWalkLeftMs = 0;
    uint32_t mVanishLeftMs = 0;
};

// ignites under a player shortly after it appears
class UldirPurifyingFlameAI : public UldirHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirPurifyingFlameAI(c); }
    explicit UldirPurifyingFlameAI(Creature* pCreature) : UldirHelperAI(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (countdown(diff, mIgniteLeftMs))
            castSpellAOE(SPELL_MOTHER_PURIFYING_FLAME);

        if (countdown(diff, mFadeLeftMs))
            despawn(100, 0);
    }

private:
    uint32_t mIgniteLeftMs = 500;
    uint32_t mFadeLeftMs = 4500;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Fetid Devourer

class UldirFetidDevourerAI : public UldirBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirFetidDevourerAI(c); }
    explicit UldirFetidDevourerAI(Creature* pCreature) : UldirBossAI(pCreature, DATA_FETID_DEVOURER) {}

protected:
    void onEngage() override
    {
        mFrenzied = false;
        armBerserk(DevourerBerserkMs);
        scriptEvents.addEvent(EVENT_ULDIR_DEVOURER_THRASH, 4000);
        scriptEvents.addEvent(EVENT_ULDIR_DEVOURER_MIASMA, 8000);
        scriptEvents.addEvent(EVENT_ULDIR_DEVOURER_DISPOSAL, 16000);
        scriptEvents.addEvent(EVENT_ULDIR_DEVOURER_REGURGITATION, DevourerRegurgitationMs);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_ULDIR_DEVOURER_STOMP, 12000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mFrenzied || _getHealthPercent() > 50)
            return;

        mFrenzied = true;
        warn("Fetid Devourer goes into a Fetid Frenzy!");
        castSpellOnSelf(SPELL_DEVOURER_FETID_FRENZY, true);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ULDIR_DEVOURER_THRASH:
                castSpellOnVictim(SPELL_DEVOURER_TERRIBLE_THRASH);
                scriptEvents.addEvent(EVENT_ULDIR_DEVOURER_THRASH, 4000);
                break;
            case EVENT_ULDIR_DEVOURER_MIASMA:
                for (Unit* pTarget : pickPlayers(3))
                    afflict(pTarget, SPELL_DEVOURER_MALODOROUS_MIASMA);
                scriptEvents.addEvent(EVENT_ULDIR_DEVOURER_MIASMA, 20000);
                break;
            case EVENT_ULDIR_DEVOURER_REGURGITATION:
                if (castAtPlayer(SPELL_DEVOURER_ROTTING_REGURGITATION) != nullptr)
                    warn("Fetid Devourer begins Rotting Regurgitation!");
                scriptEvents.addEvent(EVENT_ULDIR_DEVOURER_REGURGITATION, DevourerRegurgitationMs);
                break;
            case EVENT_ULDIR_DEVOURER_STOMP:
                castSpellAOE(SPELL_DEVOURER_SHOCKWAVE_STOMP);
                scriptEvents.addEvent(EVENT_ULDIR_DEVOURER_STOMP, 25000);
                break;
            case EVENT_ULDIR_DEVOURER_DISPOSAL:
                warn("A waste disposal unit releases a Corruption Corpuscle!");
                summonCreature(NPC_DEVOURER_CORRUPTION_CORPUSCLE, DevourerChuteSpots[Util::getRandomUInt(3)], CORPSE_DESPAWN);
                scriptEvents.addEvent(EVENT_ULDIR_DEVOURER_DISPOSAL, 30000);
                break;
            default:
                break;
        }
    }

private:
    bool mFrenzied = false;
};

// crawls to the Devourer, who feeds on it unless it is killed first
class UldirCorpuscleAI : public UldirSeekerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirCorpuscleAI(c); }
    explicit UldirCorpuscleAI(Creature* pCreature) : UldirSeekerAI(pCreature, BOSS_FETID_DEVOURER, SPELL_CORPUSCLE_ENTICING_ESSENCE, 6.0f) {}

protected:
    void onArrival(Creature* pBoss) override
    {
        pBoss->castSpell(getCreature(), SPELL_DEVOURER_CONSUME_CORRUPTION, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Zek'voz, Herald of N'Zoth

class UldirZekvozAI : public UldirBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirZekvozAI(c); }
    explicit UldirZekvozAI(Creature* pCreature) : UldirBossAI(pCreature, DATA_ZEKVOZ) {}

protected:
    void onEngage() override
    {
        mSparkPulsesLeft = 0;
        armBerserk(UldirBerserkMs);
        startStage();
        scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_EYE_BEAM, 51000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_SILITHIDS, 62000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        int32_t const health = _getHealthPercent();
        if (mStage == 1 && health <= 65)
        {
            enterStage(2);
            warn("Zek'voz calls upon the deceit of Yogg-Saron!");
            startStage();
            scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_DECEIT, 1000);
            scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_NERUBIANS, 10000);
        }
        else if (mStage == 2 && health <= 30)
        {
            enterStage(3);
            warn("Zek'voz unleashes the corruption of N'Zoth!");
            startStage();
            scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_ORB, 1000);
            scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_PSIONIC_BLAST, 3000);
        }
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ULDIR_ZEKVOZ_MIGHT:
                castSpellOnSelf(SPELL_ZEKVOZ_MIGHT_OF_THE_VOID);
                scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_LASH, 3500);
                scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_MIGHT, 45000);
                break;
            case EVENT_ULDIR_ZEKVOZ_LASH:
                castSpellOnVictim(SPELL_ZEKVOZ_VOID_LASH, true);
                castSpellOnVictim(SPELL_ZEKVOZ_SHATTER, true);
                break;
            case EVENT_ULDIR_ZEKVOZ_SPARK:
                mSparkPulsesLeft = 5;
                scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_SPARK_PULSE, 100);
                scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_SPARK, mStage == 3 ? 10000 : 25000);
                break;
            case EVENT_ULDIR_ZEKVOZ_SPARK_PULSE:
                castAtPlayers(SPELL_ZEKVOZ_TITAN_SPARK, 10);
                if (mSparkPulsesLeft > 0 && --mSparkPulsesLeft > 0)
                    scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_SPARK_PULSE, 2000);
                break;
            case EVENT_ULDIR_ZEKVOZ_DARKNESS:
                warn("Zek'voz gathers Surging Darkness!");
                castSpellAOE(SPELL_ZEKVOZ_SURGING_DARKNESS);
                scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_DARKNESS_ERUPTS, 7000);
                scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_DARKNESS, 20000);
                break;
            case EVENT_ULDIR_ZEKVOZ_DARKNESS_ERUPTS:
                castSpellAOE(SPELL_ZEKVOZ_SURGING_DARKNESS_BLAST, true);
                break;
            case EVENT_ULDIR_ZEKVOZ_EYE_BEAM:
                // C'Thun's eye only watches the first stage
                if (mStage != 1)
                    break;
                castAtPlayer(SPELL_ZEKVOZ_EYE_BEAM, false, true);
                scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_EYE_BEAM, 27000);
                break;
            case EVENT_ULDIR_ZEKVOZ_SILITHIDS:
                if (mStage != 1)
                    break;
                for (uint8_t i = 0; i < 7; ++i)
                    callAdd(NPC_ZEKVOZ_SILITHID_WARRIOR, aroundSelf(5.0f, 15.0f));
                scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_SILITHIDS, 120000);
                break;
            case EVENT_ULDIR_ZEKVOZ_DECEIT:
                castAtPlayers(SPELL_ZEKVOZ_ROILING_DECEIT, 3);
                scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_DECEIT, Util::getRandomUInt(20000, 25000));
                break;
            case EVENT_ULDIR_ZEKVOZ_NERUBIANS:
                for (uint8_t i = 0; i < 3; ++i)
                    callAdd(NPC_ZEKVOZ_NERUBIAN_VOIDWEAVER, aroundSelf(10.0f, 20.0f));
                scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_NERUBIANS, Util::getRandomUInt(30000, 35000));
                break;
            case EVENT_ULDIR_ZEKVOZ_ORB:
                warn("An Orb of Corruption falls toward the ground!");
                placeHelper(NPC_ZEKVOZ_ORB_OF_CORRUPTION, ZekvozOrbSpots[Util::getRandomUInt(3)]);
                scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_ORB, 35000);
                break;
            case EVENT_ULDIR_ZEKVOZ_PSIONIC_BLAST:
                castSpellAOE(SPELL_ZEKVOZ_PSIONIC_BLAST);
                scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_PSIONIC_BLAST, 40000);
                break;
            default:
                break;
        }
    }

private:
    // abilities kept through every stage
    void startStage()
    {
        scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_MIGHT, 6000);
        scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_SPARK, 9000);
        scriptEvents.addEvent(EVENT_ULDIR_ZEKVOZ_DARKNESS, 25000);
    }

    uint8_t mSparkPulsesLeft = 0;
};

class UldirSilithidWarriorAI : public UldirAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirSilithidWarriorAI(c); }
    explicit UldirSilithidWarriorAI(Creature* pCreature) : UldirAddAI(pCreature, SilithidWarriorSpells, false) {}
};

// bound in place on heroic
class UldirNerubianVoidweaverAI : public UldirAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirNerubianVoidweaverAI(c); }
    explicit UldirNerubianVoidweaverAI(Creature* pCreature) : UldirAddAI(pCreature, NerubianVoidweaverSpells, false) {}

protected:
    void onEngage() override
    {
        if (isHeroic())
        {
            setRooted(true);
            castSpellOnSelf(SPELL_VOIDWEAVER_SHADOWBIND, true);
        }

        UldirAddAI::onEngage();
    }

    void onDisengage() override
    {
        setRooted(false);
        UldirAddAI::onDisengage();
    }
};

// lands after a while, the interceptor takes Corruptor's Pact, otherwise it pulses
class UldirCorruptionOrbAI : public UldirHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirCorruptionOrbAI(c); }
    explicit UldirCorruptionOrbAI(Creature* pCreature) : UldirHelperAI(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (!countdown(static_cast<uint32_t>(time_passed), mImpactLeftMs))
            return;

        Player* pInterceptor = playerWithin(8.0f);
        Creature* pZekvoz = uldirLivingCreature(getInstanceScript(), BOSS_ZEKVOZ);
        if (pInterceptor != nullptr && pZekvoz != nullptr)
        {
            pZekvoz->castSpell(pInterceptor, SPELL_ZEKVOZ_CORRUPTORS_PACT, true);
            despawn(1000, 0);
            return;
        }

        castSpellOnSelf(SPELL_ORB_CORRUPTION_PULSE, true);
    }

private:
    uint32_t mImpactLeftMs = 17000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Vectis

class UldirVectisAI : public UldirBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirVectisAI(c); }
    explicit UldirVectisAI(Creature* pCreature) : UldirBossAI(pCreature, DATA_VECTIS) {}

protected:
    void onEngage() override
    {
        armBerserk(UldirBerserkMs);
        scriptEvents.addEvent(EVENT_ULDIR_VECTIS_VECTOR, 1000);
        startInfection();
    }

    void onDisengage() override { setRooted(false); }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ULDIR_VECTIS_VECTOR:
                // every vector jump leaves a Lingering Infection behind
                for (Unit* pHost : pickPlayers(3))
                {
                    castSpell(pHost, SPELL_VECTIS_OMEGA_VECTOR, true);
                    castSpell(pHost, SPELL_VECTIS_OMEGA_VECTOR_HIT, true);
                    castSpell(pHost, SPELL_VECTIS_LINGERING_INFECTION, true);
                }
                scriptEvents.addEvent(EVENT_ULDIR_VECTIS_VECTOR, 10000);
                break;
            case EVENT_ULDIR_VECTIS_AFFLICTION:
                castSpellOnVictim(SPELL_VECTIS_EVOLVING_AFFLICTION, true);
                scriptEvents.addEvent(EVENT_ULDIR_VECTIS_AFFLICTION, 9500);
                break;
            case EVENT_ULDIR_VECTIS_GESTATE:
                if (Unit* pHost = pickPlayer(true))
                {
                    castSpell(pHost, SPELL_VECTIS_GESTATE, true);
                    mGestateMark = markOf(pHost);
                    scriptEvents.addEvent(EVENT_ULDIR_VECTIS_GESTATE_BURST, 5000);
                }
                scriptEvents.addEvent(EVENT_ULDIR_VECTIS_GESTATE, 25000);
                break;
            case EVENT_ULDIR_VECTIS_GESTATE_BURST:
                callAdd(NPC_VECTIS_PLAGUE_AMALGAM, markSpot(mGestateMark));
                break;
            case EVENT_ULDIR_VECTIS_CONTAGION:
                castSpellAOE(SPELL_VECTIS_CONTAGION);
                scriptEvents.addEvent(EVENT_ULDIR_VECTIS_CONTAGION, 22500);
                break;
            case EVENT_ULDIR_VECTIS_LIQUEFY:
                liquefy();
                break;
            case EVENT_ULDIR_VECTIS_GEYSER:
                castSpellOnSelf(SPELL_VECTIS_BLOOD_GEYSER, true);
                castAtPlayers(SPELL_VECTIS_BLOOD_GEYSER_HIT, 3);
                break;
            case EVENT_ULDIR_VECTIS_PLAGUE_BOMB:
                castSpellOnSelf(SPELL_VECTIS_PLAGUE_BOMB, true);
                dropPathogenBombs();
                scriptEvents.addEvent(EVENT_ULDIR_VECTIS_PLAGUE_BOMB, 10000);
                break;
            case EVENT_ULDIR_VECTIS_REFORM:
                reform();
                break;
            default:
                break;
        }
    }

private:
    void startInfection()
    {
        scriptEvents.addEvent(EVENT_ULDIR_VECTIS_AFFLICTION, 3000);
        scriptEvents.addEvent(EVENT_ULDIR_VECTIS_GESTATE, 9000);
        scriptEvents.addEvent(EVENT_ULDIR_VECTIS_CONTAGION, 20000);
        scriptEvents.addEvent(EVENT_ULDIR_VECTIS_LIQUEFY, 90000);
    }

    void liquefy()
    {
        enterStage(2);
        warn("Vectis liquefies and seeds Plague Bombs!");
        castSpellOnSelf(SPELL_VECTIS_LIQUEFY);
        setRooted(true);
        scriptEvents.addEvent(EVENT_ULDIR_VECTIS_VECTOR, 10000);
        scriptEvents.addEvent(EVENT_ULDIR_VECTIS_GEYSER, 5000);
        scriptEvents.addEvent(EVENT_ULDIR_VECTIS_PLAGUE_BOMB, 10000);
        scriptEvents.addEvent(EVENT_ULDIR_VECTIS_REFORM, 35000);
    }

    void reform()
    {
        castSpellOnSelf(SPELL_VECTIS_LIQUEFY_CANCEL, true);
        _removeAura(SPELL_VECTIS_LIQUEFY);
        setRooted(false);
        enterStage(1);
        warn("Vectis takes form again!");
        scriptEvents.addEvent(EVENT_ULDIR_VECTIS_VECTOR, 10000);
        startInfection();
    }

    // two bombs on distinct spots
    void dropPathogenBombs()
    {
        uint32_t const spotCount = static_cast<uint32_t>(sizeof(VectisBombSpots) / sizeof(VectisBombSpots[0]));
        uint32_t const first = Util::getRandomUInt(spotCount - 1);
        uint32_t const second = (first + 1 + Util::getRandomUInt(spotCount - 2)) % spotCount;
        placeHelper(NPC_VECTIS_PATHOGEN_BOMB, VectisBombSpots[first]);
        placeHelper(NPC_VECTIS_PATHOGEN_BOMB, VectisBombSpots[second]);
    }

    UldirMark mGestateMark;
};

class UldirPlagueAmalgamAI : public UldirAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirPlagueAmalgamAI(c); }
    explicit UldirPlagueAmalgamAI(Creature* pCreature) : UldirAddAI(pCreature, PlagueAmalgamSpells, false) {}
};

// a bomb nobody soaks grows into a Plague Amalgam
class UldirPathogenBombAI : public UldirHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirPathogenBombAI(c); }
    explicit UldirPathogenBombAI(Creature* pCreature) : UldirHelperAI(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (!countdown(static_cast<uint32_t>(time_passed), mBurstLeftMs))
            return;

        if (playerWithin(4.0f) == nullptr)
        {
            if (Creature* pVectis = uldirLivingCreature(getInstanceScript(), BOSS_VECTIS))
            {
                if (Creature* pAmalgam = pVectis->summonCreature(NPC_VECTIS_PLAGUE_AMALGAM, getCreature()->GetPosition(), CORPSE_DESPAWN))
                    setZoneWideCombat(pAmalgam);
            }
        }

        despawn(500, 0);
    }

private:
    uint32_t mBurstLeftMs = 6000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Zul, Reborn

class UldirZulAI : public UldirBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirZulAI(c); }
    explicit UldirZulAI(Creature* pCreature) : UldirBossAI(pCreature, DATA_ZUL) {}

protected:
    void onEngage() override
    {
        mRevelations.clear();
        armBerserk(UldirBerserkMs);
        scriptEvents.addEvent(EVENT_ULDIR_ZUL_BARRAGE, 3000);
        scriptEvents.addEvent(EVENT_ULDIR_ZUL_CALL_OF_BLOOD, 15000);
        scriptEvents.addEvent(EVENT_ULDIR_ZUL_REVELATION, 29000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_ULDIR_ZUL_POOL, 20000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage != 1 || _getHealthPercent() > 40)
            return;

        enterStage(2);
        warn("Zul's Locus of Corruption awakens!");
        scriptEvents.addEvent(EVENT_ULDIR_ZUL_LOCUS, 3000);
        scriptEvents.addEvent(EVENT_ULDIR_ZUL_BARRAGE, 6000);
        scriptEvents.addEvent(EVENT_ULDIR_ZUL_REVELATION, 10000);
        scriptEvents.addEvent(EVENT_ULDIR_ZUL_CALL_OF_BLOOD, 25000);
        scriptEvents.addEvent(EVENT_ULDIR_ZUL_DEATHWISH, 30000);
        scriptEvents.addEvent(EVENT_ULDIR_ZUL_RUPTURING_BLOOD, 35000);
        scriptEvents.addEvent(EVENT_ULDIR_ZUL_POOL, 40000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ULDIR_ZUL_BARRAGE:
                castAtPlayer(SPELL_ZUL_SHADOW_BARRAGE);
                scriptEvents.addEvent(EVENT_ULDIR_ZUL_BARRAGE, 2000);
                break;
            case EVENT_ULDIR_ZUL_REVELATION:
                mRevelations.clear();
                for (Unit* pTarget : pickPlayers(isHeroic() ? 2 : 1))
                    mRevelations.push_back(markOf(pTarget));
                warn("Zul unveils a Dark Revelation!");
                scriptEvents.addEvent(EVENT_ULDIR_ZUL_REVELATION_BURST, 10000);
                scriptEvents.addEvent(EVENT_ULDIR_ZUL_REVELATION, 20000);
                break;
            case EVENT_ULDIR_ZUL_REVELATION_BURST:
                for (UldirMark& mark : mRevelations)
                {
                    LocationVector const spot = markSpot(mark);
                    getCreature()->castSpellLoc(spot, SPELL_ZUL_DARK_REVELATION_BURST, true);
                    for (uint8_t i = 0; i < 3; ++i)
                        callAdd(NPC_ZUL_MINION_OF_ZUL, uldirRingPoint(spot, 1.0f, 4.0f));
                }
                mRevelations.clear();
                break;
            case EVENT_ULDIR_ZUL_CALL_OF_BLOOD:
                warn("Zul calls the Nazmani to his side!");
                castSpellOnSelf(SPELL_ZUL_CALL_OF_BLOOD);
                callAdd(NPC_ZUL_NAZMANI_CRUSHER, aroundSelf(10.0f, 30.0f));
                callAdd(NPC_ZUL_NAZMANI_BLOODHEXER, aroundSelf(10.0f, 30.0f));
                for (uint8_t i = 0; i < 6; ++i)
                    callAdd(NPC_ZUL_BLOODTHIRSTY_CRAWG, aroundSelf(10.0f, 25.0f));
                scriptEvents.addEvent(EVENT_ULDIR_ZUL_CALL_OF_BLOOD, 30000);
                break;
            case EVENT_ULDIR_ZUL_POOL:
                if (Unit* pTarget = pickPlayer())
                    placeHelper(NPC_ZUL_POOL_OF_DARKNESS, pTarget->GetPosition());
                scriptEvents.addEvent(EVENT_ULDIR_ZUL_POOL, 50000);
                break;
            case EVENT_ULDIR_ZUL_LOCUS:
                castSpellAOE(SPELL_ZUL_LOCUS_OF_CORRUPTION);
                scriptEvents.addEvent(EVENT_ULDIR_ZUL_LOCUS_SPREAD, 3100);
                break;
            case EVENT_ULDIR_ZUL_LOCUS_SPREAD:
                for (Unit* pTarget : pickPlayers(UldirWholeRaid))
                    afflict(pTarget, SPELL_ZUL_CORRUPTED_BLOOD);
                break;
            case EVENT_ULDIR_ZUL_RUPTURING_BLOOD:
                castSpellOnVictim(SPELL_ZUL_RUPTURING_BLOOD, true);
                scriptEvents.addEvent(EVENT_ULDIR_ZUL_RUPTURING_BLOOD, 15000);
                break;
            case EVENT_ULDIR_ZUL_DEATHWISH:
                warn("Zul curses his foes with Deathwish!");
                castAtPlayers(SPELL_ZUL_DEATHWISH, 2);
                scriptEvents.addEvent(EVENT_ULDIR_ZUL_DEATHWISH, 45000);
                break;
            default:
                break;
        }
    }

private:
    std::vector<UldirMark> mRevelations;
};

class UldirNazmaniCrusherAI : public UldirAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirNazmaniCrusherAI(c); }
    explicit UldirNazmaniCrusherAI(Creature* pCreature) : UldirAddAI(pCreature, NazmaniCrusherSpells, false) {}
};

// congeals blood into an ichor that runs to Zul
class UldirNazmaniBloodhexerAI : public UldirAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirNazmaniBloodhexerAI(c); }
    explicit UldirNazmaniBloodhexerAI(Creature* pCreature) : UldirAddAI(pCreature, NazmaniBloodhexerSpells, false) {}

protected:
    void onEngage() override
    {
        UldirAddAI::onEngage();
        scriptEvents.addEvent(EVENT_ULDIR_BLOODHEXER_CONGEAL, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_ULDIR_BLOODHEXER_CONGEAL)
        {
            UldirAddAI::onEvent(eventId);
            return;
        }

        // Zul owns the ichor so it goes away with his fight
        if (Creature* pZul = uldirLivingCreature(getInstanceScript(), BOSS_ZUL))
            pZul->summonCreature(NPC_ZUL_ANIMATED_ICHOR, aroundSelf(1.0f, 3.0f), CORPSE_DESPAWN);

        scriptEvents.addEvent(EVENT_ULDIR_BLOODHEXER_CONGEAL, 25000);
    }
};

// heals Zul once it reaches him
class UldirAnimatedIchorAI : public UldirSeekerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirAnimatedIchorAI(c); }
    explicit UldirAnimatedIchorAI(Creature* pCreature) : UldirSeekerAI(pCreature, BOSS_ZUL, SPELL_ANIMATED_ICHOR_DRIPPING_BLOOD, 5.0f) {}

protected:
    void onArrival(Creature* pBoss) override
    {
        castSpell(pBoss, SPELL_ANIMATED_ICHOR_TRANSFUSION, true);
    }
};

// a soaker absorbs the darkness, otherwise it erupts
class UldirPoolOfDarknessAI : public UldirHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirPoolOfDarknessAI(c); }
    explicit UldirPoolOfDarknessAI(Creature* pCreature) : UldirHelperAI(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (countdown(diff, mFormLeftMs))
            castSpellOnSelf(SPELL_POOL_FORMING, true);

        if (!countdown(diff, mPulseLeftMs))
            return;

        mPulseLeftMs = 2000;
        if (Player* pSoaker = playerWithin(5.0f))
        {
            if (!pSoaker->hasAurasWithId(SPELL_POOL_ABSORBED_IN_DARKNESS))
                castSpell(pSoaker, SPELL_POOL_ABSORBED_IN_DARKNESS, true);
            return;
        }

        castSpellAOE(SPELL_POOL_ERUPTING_DARKNESS, true);
    }

private:
    uint32_t mFormLeftMs = 100;
    uint32_t mPulseLeftMs = 3000;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mythrax the Unraveler

class UldirMythraxAI : public UldirBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirMythraxAI(c); }
    explicit UldirMythraxAI(Creature* pCreature) : UldirBossAI(pCreature, DATA_MYTHRAX) {}

protected:
    void onEngage() override
    {
        mAwakenings = 0;
        armBerserk(UldirBerserkMs);
        startRuin();
    }

    void onDisengage() override
    {
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage != 1)
            return;

        // Xalzaix answers at 67% and 34%
        int32_t const health = _getHealthPercent();
        if ((mAwakenings == 0 && health <= 67) || (mAwakenings == 1 && health <= 34))
            beginAwakening();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ULDIR_MYTHRAX_SHEAR:
                castSpellOnVictim(SPELL_MYTHRAX_ESSENCE_SHEAR);
                scriptEvents.addEvent(EVENT_ULDIR_MYTHRAX_SHEAR, 14000);
                break;
            case EVENT_ULDIR_MYTHRAX_RUIN:
                castAtPlayers(SPELL_MYTHRAX_IMMINENT_RUIN, 4);
                scriptEvents.addEvent(EVENT_ULDIR_MYTHRAX_RUIN, 20000);
                break;
            case EVENT_ULDIR_MYTHRAX_SPHERES:
                warn("Mythrax conjures Oblivion Spheres!");
                for (Unit* pTarget : pickPlayers(3))
                    placeHelper(NPC_MYTHRAX_OBLIVION_SPHERE, uldirRingPoint(pTarget->GetPosition(), 4.0f, 10.0f));
                scriptEvents.addEvent(EVENT_ULDIR_MYTHRAX_SPHERES, 30000);
                break;
            case EVENT_ULDIR_MYTHRAX_AWAKENING:
                setRooted(true);
                castSpellAOE(SPELL_MYTHRAX_XALZAIXS_AWAKENING);
                scriptEvents.addEvent(EVENT_ULDIR_MYTHRAX_VEIL, 8000);
                break;
            case EVENT_ULDIR_MYTHRAX_VEIL:
                castSpellOnSelf(SPELL_MYTHRAX_OBLIVION_VEIL, true);
                scriptEvents.addEvent(EVENT_ULDIR_MYTHRAX_VISIONS, 5000);
                scriptEvents.addEvent(EVENT_ULDIR_MYTHRAX_DESTROYERS, 15000);
                scriptEvents.addEvent(EVENT_ULDIR_MYTHRAX_RETURN, 60000);
                break;
            case EVENT_ULDIR_MYTHRAX_VISIONS:
                for (Unit* pTarget : pickPlayers(5))
                    callAdd(NPC_MYTHRAX_VISION_OF_MADNESS, pTarget->GetPosition());
                break;
            case EVENT_ULDIR_MYTHRAX_DESTROYERS:
                warn("N'raqi Destroyers emerge from the void!");
                for (LocationVector const& spot : MythraxDestroyerSpots)
                    callAdd(NPC_MYTHRAX_NRAQI_DESTROYER, spot);
                break;
            case EVENT_ULDIR_MYTHRAX_RETURN:
                endAwakening();
                break;
            default:
                break;
        }
    }

private:
    void startRuin()
    {
        scriptEvents.addEvent(EVENT_ULDIR_MYTHRAX_SHEAR, 3000);
        scriptEvents.addEvent(EVENT_ULDIR_MYTHRAX_RUIN, 5000);
        scriptEvents.addEvent(EVENT_ULDIR_MYTHRAX_SPHERES, 9000);
    }

    void beginAwakening()
    {
        ++mAwakenings;
        enterStage(2);
        warn("Mythrax withdraws and awakens Xalzaix!");
        setReactState(REACT_PASSIVE);
        attackStop();
        moveTo(MythraxCentre.x, MythraxCentre.y, MythraxCentre.z);
        scriptEvents.addEvent(EVENT_ULDIR_MYTHRAX_AWAKENING, 4000);
    }

    void endAwakening()
    {
        _removeAura(SPELL_MYTHRAX_OBLIVION_VEIL);
        setRooted(false);
        setReactState(REACT_AGGRESSIVE);

        if (isHeroic())
            castSpellAOE(SPELL_MYTHRAX_ESSENCE_SHATTER);

        enterStage(1);
        warn("Mythrax returns to the fight!");
        startRuin();
    }

    uint8_t mAwakenings = 0;
};

// explodes when a player touches it
class UldirOblivionSphereAI : public UldirHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirOblivionSphereAI(c); }
    explicit UldirOblivionSphereAI(Creature* pCreature) : UldirHelperAI(pCreature) {}

    void AIUpdate(unsigned long time_passed) override
    {
        if (mBurst || !countdown(static_cast<uint32_t>(time_passed), mCheckLeftMs))
            return;

        mCheckLeftMs = 500;
        if (playerWithin(4.0f) == nullptr)
            return;

        mBurst = true;
        castSpellAOE(SPELL_SPHERE_CRASHING_OBLIVION, true);
        despawn(500, 0);
    }

private:
    uint32_t mCheckLeftMs = 2000;
    bool mBurst = false;
};

class UldirVisionOfMadnessAI : public UldirAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirVisionOfMadnessAI(c); }
    explicit UldirVisionOfMadnessAI(Creature* pCreature) : UldirAddAI(pCreature, VisionOfMadnessSpells, false) {}
};

class UldirNraqiDestroyerAI : public UldirAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirNraqiDestroyerAI(c); }
    explicit UldirNraqiDestroyerAI(Creature* pCreature) : UldirAddAI(pCreature, NraqiDestroyerSpells, false) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// G'huun: shielded in his pool until the Reorigination Drive is charged, then he emerges

class UldirGhuunAI : public UldirBossAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirGhuunAI(c); }
    explicit UldirGhuunAI(Creature* pCreature) : UldirBossAI(pCreature, DATA_GHUUN) {}

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        // untouchable while anchored in the pool
        if (damage != nullptr && mStage == 1)
            *damage = 0;
    }

protected:
    void onEngage() override
    {
        mDeliveries = 0;
        mWaves = 0;
        armBerserk(UldirBerserkMs);
        setRooted(true);
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_EXPLOSIVE, 5000);
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_MAWS, 23000);
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_MATRIX, GhuunMatrixDeliveryMs);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_ULDIR_GHUUN_TENDRIL, 28000);
    }

    void onDisengage() override { setRooted(false); }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 2 && _getHealthPercent() <= 20)
            collapse();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_ULDIR_GHUUN_EXPLOSIVE:
                castAtPlayers(SPELL_GHUUN_EXPLOSIVE_CORRUPTION, mStage == 3 ? 1 : 3);
                scriptEvents.addEvent(EVENT_ULDIR_GHUUN_EXPLOSIVE, 26000);
                break;
            case EVENT_ULDIR_GHUUN_MAWS:
                callThousandMaws();
                scriptEvents.addEvent(EVENT_ULDIR_GHUUN_MAWS, 30000);
                break;
            case EVENT_ULDIR_GHUUN_TENDRIL:
                callAdd(NPC_GHUUN_BLIGHTSPREADER_TENDRIL, uldirRingPoint(GhuunPoolCentre, 5.0f, 30.0f));
                scriptEvents.addEvent(EVENT_ULDIR_GHUUN_TENDRIL, 45000);
                break;
            case EVENT_ULDIR_GHUUN_MATRIX:
                deliverMatrix();
                break;
            case EVENT_ULDIR_GHUUN_BITE:
                castSpellOnVictim(SPELL_GHUUN_CORRUPTING_BITE, true);
                castSpellOnSelf(SPELL_GHUUN_GROWING_CORRUPTION, true);
                scriptEvents.addEvent(EVENT_ULDIR_GHUUN_BITE, 1500);
                break;
            case EVENT_ULDIR_GHUUN_WAVE:
                castAtPlayers(isHeroic() ? SPELL_GHUUN_WAVE_OF_CORRUPTION_HC : SPELL_GHUUN_WAVE_OF_CORRUPTION, 3);
                // every second wave ends in a Blood Feast, every wave once Uldir collapsed
                if (++mWaves >= 2)
                {
                    if (mStage != 3)
                        mWaves = 0;
                    bloodFeast();
                }
                scriptEvents.addEvent(EVENT_ULDIR_GHUUN_WAVE, 30000);
                break;
            case EVENT_ULDIR_GHUUN_FEAST:
                warn("A Gibbering Horror bursts out of the Blood Feast!");
                callAdd(NPC_GHUUN_GIBBERING_HORROR, markSpot(mFeastMark));
                break;
            case EVENT_ULDIR_GHUUN_GROWTH:
                for (Unit* pTarget : pickPlayers(5))
                    getCreature()->castSpellLoc(pTarget->GetPosition(), SPELL_GHUUN_MALIGNANT_GROWTH, true);
                scriptEvents.addEvent(EVENT_ULDIR_GHUUN_GROWTH, 40000);
                break;
            case EVENT_ULDIR_GHUUN_GAZE:
                warn("G'huun turns his terrible Gaze upon the raid!");
                castSpellOnSelf(SPELL_GHUUN_GAZE_OF_GHUUN);
                scriptEvents.addEvent(EVENT_ULDIR_GHUUN_GAZE, 30000);
                break;
            case EVENT_ULDIR_GHUUN_MASS_FADES:
                _removeAura(SPELL_GHUUN_CORPULENT_MASS);
                break;
            default:
                break;
        }
    }

private:
    void callThousandMaws()
    {
        if (mStage != 1)
            return;

        warn("G'huun calls forth a Thousand Maws!");
        castSpellOnSelf(SPELL_GHUUN_THOUSAND_MAWS);

        uint8_t const terrors = static_cast<uint8_t>(3 + Util::getRandomUInt(1));
        for (uint8_t i = 0; i < terrors; ++i)
            callAdd(NPC_GHUUN_CYCLOPEAN_TERROR, uldirRingPoint(GhuunPoolCentre, 10.0f, 30.0f));

        callAdd(NPC_GHUUN_DARK_YOUNG, uldirRingPoint(GhuunPoolCentre, 10.0f, 30.0f));
    }

    // a timed matrix delivery charges the drive by a third
    void deliverMatrix()
    {
        if (mStage == 3)
            return;

        if (++mDeliveries < GhuunMatrixDeliveries)
        {
            warn("A Power Matrix charges the Reorigination Drive!");
            scriptEvents.addEvent(EVENT_ULDIR_GHUUN_MATRIX, GhuunMatrixDeliveryMs);
            return;
        }

        mDeliveries = 0;
        reoriginationBlast();

        if (mStage == 1)
            emerge();
        else
            scriptEvents.addEvent(EVENT_ULDIR_GHUUN_MATRIX, GhuunMatrixDeliveryMs);
    }

    void reoriginationBlast()
    {
        warn("The Reorigination Drive unleashes a Reorigination Blast!");

        InstanceScript* pInstance = getInstanceScript();
        if (pInstance == nullptr)
            return;

        for (uint32_t entry : GhuunBrood)
        {
            for (Creature* pAdd : pInstance->getCreatureSetForEntry(entry))
            {
                if (pAdd->isAlive())
                    pAdd->castSpell(pAdd, SPELL_GHUUN_REORIGINATION_BLAST, true);
            }
        }
    }

    void emerge()
    {
        enterStage(2);
        setRooted(false);
        warn("G'huun rises from his pool of blood!");
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_BITE, 1500);
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_EXPLOSIVE, 5000);
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_WAVE, 20000);
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_MATRIX, GhuunMatrixDeliveryMs);
    }

    void collapse()
    {
        enterStage(3);
        warn("Uldir collapses around G'huun!");
        castSpellOnSelf(SPELL_GHUUN_COLLAPSE, true);
        castSpellOnSelf(SPELL_GHUUN_CORPULENT_MASS, true);
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_BITE, 1500);
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_EXPLOSIVE, 5000);
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_MASS_FADES, 19000);
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_WAVE, 20000);
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_GAZE, 30000);

        if (isHeroic())
            scriptEvents.addEvent(EVENT_ULDIR_GHUUN_GROWTH, 30000);
    }

    void bloodFeast()
    {
        Unit* pTarget = pickPlayer();
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, SPELL_GHUUN_BLOOD_FEAST, true);
        mFeastMark = markOf(pTarget);
        scriptEvents.addEvent(EVENT_ULDIR_GHUUN_FEAST, 8500);
    }

    uint8_t mDeliveries = 0;
    uint8_t mWaves = 0;
    UldirMark mFeastMark;
};

class UldirCyclopeanTerrorAI : public UldirAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirCyclopeanTerrorAI(c); }
    explicit UldirCyclopeanTerrorAI(Creature* pCreature) : UldirAddAI(pCreature, CyclopeanTerrorSpells, true) {}
};

class UldirDarkYoungAI : public UldirAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirDarkYoungAI(c); }
    explicit UldirDarkYoungAI(Creature* pCreature) : UldirAddAI(pCreature, DarkYoungSpells, false) {}
};

class UldirGibberingHorrorAI : public UldirAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirGibberingHorrorAI(c); }
    explicit UldirGibberingHorrorAI(Creature* pCreature) : UldirAddAI(pCreature, GibberingHorrorSpells, false) {}
};

class UldirBlightspreaderTendrilAI : public UldirAddAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new UldirBlightspreaderTendrilAI(c); }
    explicit UldirBlightspreaderTendrilAI(Creature* pCreature) : UldirAddAI(pCreature, BlightspreaderTendrilSpells, true) {}
};

void SetupUldir(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ULDIR, &UldirInstanceScript::Create);

    // Taloc
    mgr->register_creature_script(BOSS_TALOC, &UldirTalocAI::Create);

    // MOTHER
    mgr->register_creature_script(BOSS_MOTHER, &UldirMotherAI::Create);
    mgr->register_creature_script(NPC_MOTHER_PURIFYING_FLAME, &UldirPurifyingFlameAI::Create);

    // Fetid Devourer
    mgr->register_creature_script(BOSS_FETID_DEVOURER, &UldirFetidDevourerAI::Create);
    mgr->register_creature_script(NPC_DEVOURER_CORRUPTION_CORPUSCLE, &UldirCorpuscleAI::Create);

    // Zek'voz
    mgr->register_creature_script(BOSS_ZEKVOZ, &UldirZekvozAI::Create);
    mgr->register_creature_script(NPC_ZEKVOZ_SILITHID_WARRIOR, &UldirSilithidWarriorAI::Create);
    mgr->register_creature_script(NPC_ZEKVOZ_NERUBIAN_VOIDWEAVER, &UldirNerubianVoidweaverAI::Create);
    mgr->register_creature_script(NPC_ZEKVOZ_SURGING_DARKNESS, &UldirFadingHelperAI::Create);
    mgr->register_creature_script(NPC_ZEKVOZ_ORB_OF_CORRUPTION, &UldirCorruptionOrbAI::Create);

    // Vectis
    mgr->register_creature_script(BOSS_VECTIS, &UldirVectisAI::Create);
    mgr->register_creature_script(NPC_VECTIS_PLAGUE_AMALGAM, &UldirPlagueAmalgamAI::Create);
    mgr->register_creature_script(NPC_VECTIS_PATHOGEN_BOMB, &UldirPathogenBombAI::Create);

    // Zul, Reborn
    mgr->register_creature_script(BOSS_ZUL, &UldirZulAI::Create);
    mgr->register_creature_script(NPC_ZUL_NAZMANI_CRUSHER, &UldirNazmaniCrusherAI::Create);
    mgr->register_creature_script(NPC_ZUL_NAZMANI_BLOODHEXER, &UldirNazmaniBloodhexerAI::Create);
    mgr->register_creature_script(NPC_ZUL_ANIMATED_ICHOR, &UldirAnimatedIchorAI::Create);
    mgr->register_creature_script(NPC_ZUL_POOL_OF_DARKNESS, &UldirPoolOfDarknessAI::Create);

    // Mythrax the Unraveler
    mgr->register_creature_script(BOSS_MYTHRAX, &UldirMythraxAI::Create);
    mgr->register_creature_script(NPC_MYTHRAX_OBLIVION_SPHERE, &UldirOblivionSphereAI::Create);
    mgr->register_creature_script(NPC_MYTHRAX_VISION_OF_MADNESS, &UldirVisionOfMadnessAI::Create);
    mgr->register_creature_script(NPC_MYTHRAX_NRAQI_DESTROYER, &UldirNraqiDestroyerAI::Create);

    // G'huun
    mgr->register_creature_script(BOSS_GHUUN, &UldirGhuunAI::Create);
    mgr->register_creature_script(NPC_GHUUN_CYCLOPEAN_TERROR, &UldirCyclopeanTerrorAI::Create);
    mgr->register_creature_script(NPC_GHUUN_CYCLOPEAN_TERROR_ALT, &UldirCyclopeanTerrorAI::Create);
    mgr->register_creature_script(NPC_GHUUN_DARK_YOUNG, &UldirDarkYoungAI::Create);
    mgr->register_creature_script(NPC_GHUUN_DARK_YOUNG_ALT, &UldirDarkYoungAI::Create);
    mgr->register_creature_script(NPC_GHUUN_GIBBERING_HORROR, &UldirGibberingHorrorAI::Create);
    mgr->register_creature_script(NPC_GHUUN_BLIGHTSPREADER_TENDRIL, &UldirBlightspreaderTendrilAI::Create);
}
