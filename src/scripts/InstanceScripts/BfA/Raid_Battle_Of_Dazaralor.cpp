/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Battle_Of_Dazaralor.hpp"

#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <map>
#include <vector>

enum BattleOfDazaralorEvents
{
    // Champion of the Light
    EVENT_CHAMPION_WAVE_OF_LIGHT = 1,
    EVENT_CHAMPION_JUDGMENT_RIGHTEOUSNESS,
    EVENT_CHAMPION_CALL_TO_ARMS,
    EVENT_CHAMPION_DIVINE_PROTECTION,
    EVENT_CRUSADER_STRIKE,
    EVENT_CRUSADER_BLINDING_FAITH,
    EVENT_DISCIPLE_DIVINE_BURST,
    EVENT_DISCIPLE_HEAL,
    EVENT_DISCIPLE_PENANCE_MEND,
    EVENT_DISCIPLE_PENANCE_SMITE,

    // Grong and Grong the Revenant
    EVENT_APE_SLAM,
    EVENT_APE_COMBO,
    EVENT_APE_MISSILE,
    EVENT_APE_CALL_HELPER,
    EVENT_APE_ROAR,
    EVENT_CHANNELER_EMPOWER,

    // Jadefire Masters
    EVENT_MAGE_FIREBALL,
    EVENT_MAGE_PYROBLAST,
    EVENT_MAGE_SEARING_EMBERS,
    EVENT_MAGE_MAGMA_TRAP,
    EVENT_MAGE_RISING_FLAMES,
    EVENT_MONK_JADE_STORM,
    EVENT_MONK_SPIRITS_OF_XUEN,
    EVENT_MONK_DRAGONS_BREATH,
    EVENT_XUEN_POUNCE,
    EVENT_XUEN_TIGER_PAW,

    // Opulence
    EVENT_OPULENCE_HOARD_POWER,
    EVENT_OPULENCE_COIN_SWEEP,
    EVENT_OPULENCE_LIQUID_GOLD,
    EVENT_OPULENCE_SPIRITS_OF_GOLD,
    EVENT_OPULENCE_COIN_SHOWER,
    EVENT_OPULENCE_WAIL_OF_GREED,

    // Conclave of the Chosen
    EVENT_PAKU_HASTENING_WINDS,
    EVENT_PAKU_GIFT_OF_WIND,
    EVENT_PAKU_WRATH,
    EVENT_PAKU_KRAGWA,
    EVENT_GONK_RAPTOR_FORM,
    EVENT_GONK_CRAWLING_HEX,
    EVENT_KIMBUL_LACERATING_CLAWS,
    EVENT_KIMBUL_WRATH,
    EVENT_AKUNDA_THUNDERING_STORM,
    EVENT_AKUNDA_MIND_WIPE,
    EVENT_AKUNDA_WRATH,

    // King Rastakhan
    EVENT_RASTAKHAN_SCORCHING_DETONATION,
    EVENT_RASTAKHAN_PLAGUE_OF_TOADS,
    EVENT_RASTAKHAN_SERPENT_TOTEM,
    EVENT_RASTAKHAN_ZOMBIE_DUST_TOTEM,
    EVENT_RASTAKHAN_PLAGUE_OF_FIRE,
    EVENT_RASTAKHAN_BOON,
    EVENT_RASTAKHAN_DEATHS_DOOR,
    EVENT_RASTAKHAN_INEVITABLE_END,
    EVENT_BWONSAMDI_CARESS_OF_DEATH,
    EVENT_ROKA_METEOR_LEAP,
    EVENT_ZALAN_SEAL_OF_PURIFICATION,
    EVENT_GALWANA_GRIEVOUS_AXE,
    EVENT_PHANTOM_STRIKE,
    EVENT_TOTEM_PULSE,

    // High Tinker Mekkatorque
    EVENT_MEKKATORQUE_BUSTER_CANNON,
    EVENT_MEKKATORQUE_GIGAVOLT_CHARGE,
    EVENT_MEKKATORQUE_SPARK_BOTS,
    EVENT_MEKKATORQUE_BLAST_OFF,
    EVENT_MEKKATORQUE_EXPLODING_SHEEP,
    EVENT_MEKKATORQUE_HYPERDRIVE,
    EVENT_SPARK_BOT_PULSE,

    // Stormwall Blockade
    EVENT_KATHERINE_VOLTAIC_FLASH,
    EVENT_KATHERINE_CRACKLING_LIGHTNING,
    EVENT_KATHERINE_JOLTING_VOLLEY,
    EVENT_KATHERINE_ELECTRIC_SHROUD,
    EVENT_JOSEPH_SEA_SWELL,
    EVENT_JOSEPH_TIDAL_VOLLEY,
    EVENT_JOSEPH_TEMPTING_SONG,
    EVENT_JOSEPH_TIDAL_SHROUD,
    EVENT_LAMINARIA_STORMS_WAIL,

    // Lady Jaina Proudmoore
    EVENT_JAINA_ICE_SHARD,
    EVENT_JAINA_FREEZING_BLAST,
    EVENT_JAINA_GRASP_OF_FROST,
    EVENT_JAINA_AVALANCHE,
    EVENT_JAINA_CORSAIR,
    EVENT_JAINA_RING_OF_ICE,
    EVENT_JAINA_GLACIAL_RAY,
    EVENT_JAINA_HAND_OF_FROST,
    EVENT_JAINA_CRYSTALLINE_DUST
};

enum BattleOfDazaralorActions
{
    ACTION_DAZARALOR_JOIN_FIGHT = 1,
    ACTION_DAZARALOR_LEAVE_FIGHT,
    ACTION_DAZARALOR_GUARDS_DOWN,
    ACTION_CHAMPION_ADD_FALLEN,
    ACTION_APE_CHANNEL_HIT,
    ACTION_APE_DETONATION,
    ACTION_CONCLAVE_PARTNER_FALLEN,
    ACTION_BWONSAMDI_MORTAL,
    ACTION_RASTAKHAN_FINAL_STAGE
};

namespace
{
    float const DazaralorTwoPi = 6.28318531f;

    uint32_t const DazaralorBerserkMs = 600000;
    uint32_t const JainaBerserkMs = 900000;

    // energy gain per second
    float const ChampionRetributionGain = 2.5f;
    float const ChampionReckoningGain = 2.0f;
    float const ApeEnergyGain = 1.0f;
    float const JadefireEnergyGain = 1.0f;
    float const GonkEnergyGain = 2.5f;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Positions

    LocationVector const MestrahSideBombSpots[3] =
    {
        { -921.0f, 830.0f, 369.0f, 3.0f },
        { -878.0f, 805.0f, 370.0f, 3.0f },
        { -921.0f, 776.0f, 369.0f, 3.0f }
    };

    LocationVector const MestrahSideXuenSpot(-905.0f, 786.0f, 369.0f, 1.6f);

    LocationVector const MaraSideBombSpots[3] =
    {
        { -1853.448f, 778.556f, 53.784f, 0.025f },
        { -1811.786f, 801.702f, 53.390f, 0.033f },
        { -1854.693f, 831.997f, 53.783f, 3.206f }
    };

    LocationVector const ConclaveCentre(-1095.421f, 804.986f, 203.865f, 1.53f);

    LocationVector const BwonsamdiArrivalSpot(-1254.939f, 805.260f, 359.227f, 6.222f);
    LocationVector const BwonsamdiPlatformSpot(-1232.270f, 805.715f, 351.649f, 0.0f);
    LocationVector const GalwanaSpot(-1232.223f, 807.116f, 351.649f, 0.051f);

    LocationVector const MekkatorqueClawSpots[3] =
    {
        { -1588.186f, 789.644f, 126.965f, 0.0f },
        { -1605.073f, 788.459f, 126.965f, 0.0f },
        { -1605.872f, 816.764f, 126.965f, 0.0f }
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    // Doors and encounter groups

    enum class DazaralorDoorKind : uint8_t
    {
        Room,       // shut while the encounter runs
        Passage     // opens after the encounter
    };

    struct DazaralorDoorRule
    {
        uint32_t entry;
        uint32_t dataId;
        DazaralorDoorKind kind;
    };

    DazaralorDoorRule const DazaralorDoorRules[] =
    {
        { GO_FRIDA_WALL_OF_SPEARS,        DATA_CHAMPION_OF_THE_LIGHT,     DazaralorDoorKind::Room },
        { GO_RAWANI_WALL_OF_SPEARS,       DATA_CHAMPION_OF_THE_LIGHT,     DazaralorDoorKind::Room },
        { GO_RAWANI_COLLISION_DOOR,       DATA_CHAMPION_OF_THE_LIGHT,     DazaralorDoorKind::Passage },
        { GO_GRONG_WALL_OF_SPEARS,        DATA_GRONG,                     DazaralorDoorKind::Room },
        { GO_GRONG_WALL_OF_SPEARS,        DATA_GRONG_THE_REVENANT,        DazaralorDoorKind::Room },
        { GO_JADEFIRE_ENTRANCE_WALL,      DATA_JADEFIRE_MASTERS_MESTRAH,  DazaralorDoorKind::Room },
        { GO_JADEFIRE_EXIT_WALL,          DATA_JADEFIRE_MASTERS_MESTRAH,  DazaralorDoorKind::Passage },
        { GO_JADEFIRE_BARRIER,            DATA_JADEFIRE_MASTERS_MESTRAH,  DazaralorDoorKind::Passage },
        { GO_JADEFIRE_MAIN_WALL,          DATA_JADEFIRE_MASTERS_MESTRAH,  DazaralorDoorKind::Passage },
        { GO_CONCLAVE_ENTRANCE_DOOR,      DATA_CONCLAVE_OF_THE_CHOSEN,    DazaralorDoorKind::Room },
        { GO_CONCLAVE_EXIT_DOOR,          DATA_CONCLAVE_OF_THE_CHOSEN,    DazaralorDoorKind::Passage },
        { GO_RASTAKHAN_HARBOR_DOOR,       DATA_KING_RASTAKHAN,            DazaralorDoorKind::Passage },
        { GO_MEKKATORQUE_ENTRANCE_WALL,   DATA_MEKKATORQUE,               DazaralorDoorKind::Room },
        { GO_MEKKATORQUE_EXIT_WALL,       DATA_MEKKATORQUE,               DazaralorDoorKind::Passage },
        { GO_MEKKATORQUE_COLLISION_DOOR,  DATA_MEKKATORQUE,               DazaralorDoorKind::Passage }
    };

    // wave 0 is pulled together, a later wave enters once every earlier one fell
    struct DazaralorMemberSlot
    {
        uint32_t dataId;
        uint32_t entry;
        uint8_t wave;
        bool required;
    };

    DazaralorMemberSlot const DazaralorMemberSlots[] =
    {
        { DATA_JADEFIRE_MASTERS_MESTRAH,  BOSS_MESTRAH,             0, true },
        { DATA_JADEFIRE_MASTERS_MESTRAH,  BOSS_MANCEROY_FLAMEFIST,  0, true },
        { DATA_JADEFIRE_MASTERS_MARA,     BOSS_MARA_GRIMFANG,       0, true },
        { DATA_JADEFIRE_MASTERS_MARA,     BOSS_ANATHOS_FIRECALLER,  0, true },
        { DATA_CONCLAVE_OF_THE_CHOSEN,    BOSS_PAKUS_ASPECT,        0, true },
        { DATA_CONCLAVE_OF_THE_CHOSEN,    BOSS_GONKS_ASPECT,        0, true },
        { DATA_CONCLAVE_OF_THE_CHOSEN,    BOSS_KIMBULS_ASPECT,      1, true },
        { DATA_CONCLAVE_OF_THE_CHOSEN,    BOSS_AKUNDAS_ASPECT,      2, true },
        { DATA_KING_RASTAKHAN,            BOSS_KING_RASTAKHAN,      0, true },
        { DATA_KING_RASTAKHAN,            NPC_SIEGEBREAKER_ROKA,    0, false },
        { DATA_KING_RASTAKHAN,            NPC_PRELATE_ZALAN,        0, false },
        { DATA_STORMWALL_BLOCKADE,        BOSS_SISTER_KATHERINE,    0, true },
        { DATA_STORMWALL_BLOCKADE,        BOSS_BROTHER_JOSEPH,      0, true },
        { DATA_STORMWALL_BLOCKADE,        BOSS_LAMINARIA,           1, true }
    };

    DazaralorMemberSlot const* findMemberSlot(uint32_t entry)
    {
        for (DazaralorMemberSlot const& slot : DazaralorMemberSlots)
        {
            if (slot.entry == entry)
                return &slot;
        }

        return nullptr;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Shared helpers

    LocationVector dazaralorRingPoint(LocationVector const& origin, float minRange, float maxRange)
    {
        float const angle = Util::getRandomFloat(DazaralorTwoPi);
        float const range = Util::getRandomFloat(minRange, maxRange);
        return LocationVector(origin.x + range * std::cos(angle), origin.y + range * std::sin(angle), origin.z, angle);
    }

    // distinct random players, the tank only when nobody else is around
    std::vector<Unit*> dazaralorPickPlayers(CreatureAIScript* pScript, size_t count, bool skipTank)
    {
        std::vector<Unit*> picked;
        for (uint8_t pass = skipTank ? 0 : 1; pass < 2 && picked.empty(); ++pass)
        {
            FilterArgs args(TargetFilter_Player);
            if (pass == 0)
                args.addFilter(TargetFilter_NotCurrent);

            for (size_t attempt = 0; attempt < count * 4 && picked.size() < count; ++attempt)
            {
                Unit* pTarget = pScript->selectUnitTarget(args);
                if (pTarget == nullptr)
                    break;

                if (std::find(picked.begin(), picked.end(), pTarget) == picked.end())
                    picked.push_back(pTarget);
            }
        }

        return picked;
    }

    // energy bar kept as a plain counter
    class DazaralorEnergyBar
    {
    public:
        void reset(float gainPerSecond, float start = 0.0f)
        {
            mGainPerSecond = gainPerSecond;
            mValue = start;
        }

        void fill(uint32_t diffMs) { add(mGainPerSecond * static_cast<float>(diffMs) / 1000.0f); }
        void add(float points) { mValue = std::min(100.0f, mValue + points); }
        float value() const { return mValue; }
        bool isFull() const { return mValue >= 100.0f; }

    private:
        float mGainPerSecond = 0.0f;
        float mValue = 0.0f;
    };

    // spell released on a marked unit after a delay, no unit means around the caster
    struct DazaralorDelayedStrike
    {
        WoWGuid target;
        uint32_t spellId;
        uint32_t leftMs;
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class BattleOfDazaralorInstanceScript : public InstanceScript
{
public:
    explicit BattleOfDazaralorInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(BattleOfDazaralorEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new BattleOfDazaralorInstanceScript(pMapMgr); }

    // the kill credit member must not close a group fight early
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isGroupDown(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        if (!hasDoorRule(pGameObject->getEntry()))
            return;

        mDoorGuids.push_back(pGameObject->GetNewGUID());
        applyDoorRules(pGameObject);
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        for (WoWGuid const& guid : mDoorGuids)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                applyDoorRules(pDoor);
        }

        if (state == EncounterStates::InProgress)
        {
            mEngagedAtMs[id] = Util::getMSTime();
            return;
        }

        if (state != EncounterStates::Failed)
            return;

        // a wiped group returns in full, late waves wait again
        for (Creature* pMember : collectMembers(id, false))
        {
            if (!pMember->isAlive())
                pMember->despawn(1000, 5000);
        }

        sendToGroup(id, ACTION_DAZARALOR_LEAVE_FIGHT, 0xFF, false);
    }

    std::vector<Creature*> collectMembers(uint32_t dataId, bool requiredOnly)
    {
        std::vector<Creature*> members;
        for (DazaralorMemberSlot const& slot : DazaralorMemberSlots)
        {
            if (slot.dataId != dataId || (requiredOnly && !slot.required))
                continue;

            for (Creature* pMember : getCreatureSetForEntry(slot.entry))
                members.push_back(pMember);
        }

        return members;
    }

    bool isGroupDown(uint32_t dataId, Creature* pExcept)
    {
        for (Creature* pMember : collectMembers(dataId, true))
        {
            if (pMember != pExcept && pMember->isAlive())
                return false;
        }

        return true;
    }

    // the first wave answers a pull together
    void rallyGroup(uint32_t dataId, Creature* pPuller)
    {
        if (mRallying)
            return;

        mRallying = true;
        for (DazaralorMemberSlot const& slot : DazaralorMemberSlots)
        {
            if (slot.dataId != dataId || slot.wave != 0)
                continue;

            for (Creature* pMember : getCreatureSetForEntry(slot.entry))
            {
                if (pMember == pPuller || !pMember->isAlive() || pMember->isInCombat())
                    continue;

                if (CreatureAIScript* pScript = pMember->GetScript())
                    pScript->setZoneWideCombat();
            }
        }
        mRallying = false;
    }

    // called after each fallen member: next wave, fallen guards
    void onMemberDown(uint32_t dataId)
    {
        if (getBossState(dataId) != EncounterStates::InProgress)
            return;

        uint8_t lowestWave = 0xFF;
        bool hasGuards = false;
        bool guardsStanding = false;
        for (DazaralorMemberSlot const& slot : DazaralorMemberSlots)
        {
            if (slot.dataId != dataId)
                continue;

            if (!slot.required)
                hasGuards = true;

            for (Creature* pMember : getCreatureSetForEntry(slot.entry))
            {
                if (!pMember->isAlive())
                    continue;

                lowestWave = std::min(lowestWave, slot.wave);
                if (!slot.required)
                    guardsStanding = true;
            }
        }

        if (lowestWave != 0 && lowestWave != 0xFF)
            sendToGroup(dataId, ACTION_DAZARALOR_JOIN_FIGHT, lowestWave, false);

        if (hasGuards && !guardsStanding)
            sendToGroup(dataId, ACTION_DAZARALOR_GUARDS_DOWN, 0xFF, true);
    }

    uint32_t getElapsedMs(uint32_t dataId) const
    {
        auto const itr = mEngagedAtMs.find(dataId);
        return itr != mEngagedAtMs.end() ? Util::getMSTime() - itr->second : 0;
    }

    Creature* findAlive(uint32_t entry)
    {
        for (Creature* pCreature : getCreatureSetForEntry(entry))
        {
            if (pCreature->isAlive())
                return pCreature;
        }

        return nullptr;
    }

private:
    // wave 0xFF reaches every wave
    void sendToGroup(uint32_t dataId, int32_t action, uint8_t wave, bool requiredOnly)
    {
        for (DazaralorMemberSlot const& slot : DazaralorMemberSlots)
        {
            if (slot.dataId != dataId || (wave != 0xFF && slot.wave != wave) || (requiredOnly && !slot.required))
                continue;

            for (Creature* pMember : getCreatureSetForEntry(slot.entry))
            {
                if (pMember->isAlive() && pMember->GetScript() != nullptr)
                    pMember->GetScript()->DoAction(action);
            }
        }
    }

    bool hasDoorRule(uint32_t entry) const
    {
        for (DazaralorDoorRule const& rule : DazaralorDoorRules)
        {
            if (rule.entry == entry)
                return true;
        }

        return false;
    }

    // every rule of a door has to agree before it opens
    bool isDoorOpen(uint32_t entry) const
    {
        for (DazaralorDoorRule const& rule : DazaralorDoorRules)
        {
            if (rule.entry != entry)
                continue;

            EncounterStates const state = getBossState(rule.dataId);
            if (rule.kind == DazaralorDoorKind::Room && state == EncounterStates::InProgress)
                return false;

            if (rule.kind == DazaralorDoorKind::Passage && state != EncounterStates::Performed)
                return false;
        }

        return true;
    }

    // changes the state only when it differs
    void applyDoorRules(GameObject* pDoor)
    {
        uint8_t const wanted = isDoorOpen(pDoor->getEntry()) ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != wanted)
            pDoor->setState(wanted);
    }

    std::vector<WoWGuid> mDoorGuids;
    std::map<uint32_t, uint32_t> mEngagedAtMs;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared combat base: event tick, berserk, delayed strikes and target helpers

class BattleOfDazaralorFightAI : public CreatureAIScript
{
public:
    BattleOfDazaralorFightAI(Creature* pCreature, bool framed) : CreatureAIScript(pCreature), mFramed(framed) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        clearFight();
        mStage = 1;
        if (mFramed)
            showFrame(true);

        onEngage();
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (mFramed && getCreature()->isAlive())
            showFrame(false);

        clearFight();
        mStage = 0;
        onDisengage();
    }

    void OnDied(Unit* pKiller) override
    {
        if (mFramed)
            showFrame(false);

        clearFight();
        onDefeat(pKiller);
    }

    void OnCastSpell(uint32_t spellId) override { onSpellDone(spellId); }

    void onSummonedCreature(Creature* pSummon) override { summons.summon(pSummon); }

    void AIUpdate(unsigned long time_passed) override
    {
        if (!_isInCombat())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        scriptEvents.updateEvents(diff, 0);
        tickBerserk(diff);
        tickStrikes(diff);
        onCombatTick(diff);

        if (_isCasting())
            return;

        if (uint32_t const eventId = scriptEvents.getFinishedEvent())
            onEvent(eventId);
    }

protected:
    virtual void onEngage() {}
    virtual void onDisengage() {}
    virtual void onDefeat(Unit* /*pKiller*/) {}
    virtual void onCombatTick(uint32_t /*diff*/) {}
    virtual void onEvent(uint32_t /*eventId*/) {}
    virtual void onSpellDone(uint32_t /*spellId*/) {}

    BattleOfDazaralorInstanceScript* getRaid()
    {
        if (getCreature()->GetMapId() != MAP_BATTLE_OF_DAZARALOR)
            return nullptr;

        return static_cast<BattleOfDazaralorInstanceScript*>(getInstanceScript());
    }

    Unit* pickPlayer(bool skipTank = false)
    {
        std::vector<Unit*> const picked = dazaralorPickPlayers(this, 1, skipTank);
        return picked.empty() ? nullptr : picked.front();
    }

    std::vector<Unit*> pickPlayers(size_t count, bool skipTank = false) { return dazaralorPickPlayers(this, count, skipTank); }

    Unit* castAtPlayer(uint32_t spellId, bool skipTank = false, bool triggered = false)
    {
        Unit* pTarget = pickPlayer(skipTank);
        if (pTarget != nullptr)
            castSpell(pTarget, spellId, triggered);

        return pTarget;
    }

    Unit* currentVictim() { return getCreature()->getAIInterface()->getCurrentTarget(); }

    Unit* findUnit(WoWGuid const& guid)
    {
        if (guid.isEmpty() || getCreature()->getWorldMap() == nullptr)
            return nullptr;

        return getCreature()->getWorldMap()->getUnit(guid);
    }

    // living creature of this entry, instance wide when possible
    Creature* findAlive(uint32_t entry)
    {
        if (BattleOfDazaralorInstanceScript* pRaid = getRaid())
            return pRaid->findAlive(entry);

        Creature* pCreature = findNearestCreature(entry, 200.0f);
        return pCreature != nullptr && pCreature->isAlive() ? pCreature : nullptr;
    }

    void signal(uint32_t entry, int32_t action)
    {
        Creature* pTarget = findAlive(entry);
        if (pTarget != nullptr && pTarget->GetScript() != nullptr)
            pTarget->GetScript()->DoAction(action);
    }

    // summoned add that joins the fight at once
    Creature* callAdd(uint32_t entry, LocationVector const& spot)
    {
        Creature* pAdd = summonCreature(entry, spot, CORPSE_DESPAWN);
        if (pAdd != nullptr)
            setZoneWideCombat(pAdd);

        return pAdd;
    }

    // summoned helper with a fixed lifetime
    Creature* placeHelper(uint32_t entry, LocationVector const& spot, uint32_t lifetimeMs)
    {
        return summonCreature(entry, spot, TIMED_DESPAWN, lifetimeMs);
    }

    LocationVector aroundSelf(float minRange, float maxRange) { return dazaralorRingPoint(getCreature()->GetPosition(), minRange, maxRange); }

    void warn(char const* text) { sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text); }

    void armBerserk(uint32_t delayMs) { mBerserkLeftMs = delayMs; }

    // a new stage drops every pending event
    void enterStage(uint32_t stage)
    {
        mStage = stage;
        scriptEvents.resetEvents();
    }

    // a lost target drops the strike
    void strikeLater(Unit* pTarget, uint32_t spellId, uint32_t delayMs)
    {
        if (pTarget != nullptr)
            queueStrike(pTarget->GetNewGUID(), spellId, delayMs);
    }

    void blastLater(uint32_t spellId, uint32_t delayMs) { queueStrike(WoWGuid(), spellId, delayMs); }

    // out of the fight until called: no combat, not selectable
    void setBenched(bool benched)
    {
        mBenched = benched;
        setCanEnterCombat(!benched);

        if (benched)
            getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        else
            getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    uint32_t mStage = 0;
    bool mBenched = false;

private:
    void showFrame(bool engaged)
    {
        if (InstanceScript* pInstance = getInstanceScript())
            pInstance->sendUnitEncounter(engaged ? EncounterFrameEngage : EncounterFrameDisengaged, getCreature());
    }

    void queueStrike(WoWGuid const& target, uint32_t spellId, uint32_t delayMs)
    {
        DazaralorDelayedStrike strike;
        strike.target = target;
        strike.spellId = spellId;
        strike.leftMs = delayMs;
        mStrikes.push_back(strike);
    }

    void clearFight()
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
        mStrikes.clear();
        mBerserkLeftMs = 0;
    }

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
        castSpellOnSelf(SPELL_DAZARALOR_BERSERK, true);
    }

    // due strikes are collected first, a cast may queue new ones
    void tickStrikes(uint32_t diff)
    {
        if (mStrikes.empty())
            return;

        std::vector<DazaralorDelayedStrike> due;
        for (auto itr = mStrikes.begin(); itr != mStrikes.end();)
        {
            if (itr->leftMs > diff)
            {
                itr->leftMs -= diff;
                ++itr;
                continue;
            }

            due.push_back(*itr);
            itr = mStrikes.erase(itr);
        }

        for (DazaralorDelayedStrike const& strike : due)
        {
            if (strike.target.isEmpty())
            {
                castSpellAOE(strike.spellId, true);
                continue;
            }

            Unit* pTarget = findUnit(strike.target);
            if (pTarget != nullptr && pTarget->isAlive())
                castSpell(pTarget, strike.spellId, true);
        }
    }

    bool mFramed;
    uint32_t mBerserkLeftMs = 0;
    std::vector<DazaralorDelayedStrike> mStrikes;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter base: frame, boss state, group pull and late waves

class BattleOfDazaralorEncounterAI : public BattleOfDazaralorFightAI
{
public:
    BattleOfDazaralorEncounterAI(Creature* pCreature, uint32_t dataId, uint32_t berserkMs)
        : BattleOfDazaralorFightAI(pCreature, false), mDataId(dataId), mBerserkLimitMs(berserkMs)
    {
        if (DazaralorMemberSlot const* pSlot = findMemberSlot(pCreature->getEntry()))
            mWave = pSlot->wave;
    }

    void OnLoad() override
    {
        if (mWave != 0 && getCreature()->GetMapId() == MAP_BATTLE_OF_DAZARALOR)
            setBenched(true);
    }

    void OnCombatStart(Unit* pTarget) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            pInstance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (BattleOfDazaralorInstanceScript* pRaid = getRaid())
            pRaid->rallyGroup(mDataId, getCreature());

        BattleOfDazaralorFightAI::OnCombatStart(pTarget);
        armEncounterBerserk();
    }

    void OnCombatStop(Unit* pTarget) override
    {
        bool const alive = getCreature()->isAlive();
        if (alive)
        {
            if (InstanceScript* pInstance = getInstanceScript())
            {
                pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
                pInstance->setBossState(mDataId, EncounterStates::Failed);
            }
        }

        BattleOfDazaralorFightAI::OnCombatStop(pTarget);

        if (alive && mWave != 0 && getCreature()->GetMapId() == MAP_BATTLE_OF_DAZARALOR)
            setBenched(true);
    }

    void OnDied(Unit* pKiller) override
    {
        if (InstanceScript* pInstance = getInstanceScript())
        {
            pInstance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

            BattleOfDazaralorInstanceScript* pRaid = getRaid();
            if (pRaid == nullptr || pRaid->isGroupDown(mDataId, getCreature()))
                pInstance->setBossState(mDataId, EncounterStates::Performed);
        }

        BattleOfDazaralorFightAI::OnDied(pKiller);

        if (BattleOfDazaralorInstanceScript* pRaid = getRaid())
            pRaid->onMemberDown(mDataId);
    }

    void DoAction(int32_t action) override
    {
        switch (action)
        {
            case ACTION_DAZARALOR_JOIN_FIGHT:
                if (!mBenched || !getCreature()->isAlive())
                    return;

                setBenched(false);
                onJoinFight();
                setZoneWideCombat();
                return;
            case ACTION_DAZARALOR_LEAVE_FIGHT:
                // released but never engaged before the wipe
                if (mWave != 0 && !mBenched && !_isInCombat())
                    setBenched(true);
                return;
            default:
                onAction(action);
                return;
        }
    }

protected:
    virtual void onAction(int32_t /*action*/) {}
    virtual void onJoinFight() {}

    uint32_t mDataId;

private:
    // late members only get the time that is left
    void armEncounterBerserk()
    {
        if (mBerserkLimitMs == 0)
            return;

        uint32_t elapsed = 0;
        if (BattleOfDazaralorInstanceScript* pRaid = getRaid())
            elapsed = pRaid->getElapsedMs(mDataId);

        armBerserk(elapsed < mBerserkLimitMs ? mBerserkLimitMs - elapsed : 1000);
    }

    uint32_t mBerserkLimitMs;
    uint8_t mWave = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Summoned helpers: idle until their steps run, then gone

class BattleOfDazaralorHelperAI : public CreatureAIScript
{
public:
    BattleOfDazaralorHelperAI(Creature* pCreature, uint32_t firstMs, uint32_t everyMs, uint8_t steps, uint32_t lingerMs, bool targetable)
        : CreatureAIScript(pCreature), mLeftMs(firstMs), mEveryMs(everyMs), mSteps(steps), mLingerMs(lingerMs), mTargetable(targetable) {}

    void OnLoad() override
    {
        if (getCreature()->GetMapId() != MAP_BATTLE_OF_DAZARALOR)
            return;

        setReactState(REACT_PASSIVE);
        setCanEnterCombat(false);
        setRooted(true);
        _setMeleeDisabled(true);

        if (!mTargetable)
            getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mStep >= mSteps || !getCreature()->isSummon() || !getCreature()->isAlive())
            return;

        uint32_t const diff = static_cast<uint32_t>(time_passed);
        if (mLeftMs > diff)
        {
            mLeftMs -= diff;
            return;
        }

        mLeftMs = mEveryMs;
        onStep(mStep++);

        if (mStep >= mSteps && mLingerMs != 0)
            despawn(mLingerMs, 0);
    }

protected:
    virtual void onStep(uint8_t /*step*/) {}

    Unit* randomPlayer()
    {
        std::vector<Unit*> const picked = dazaralorPickPlayers(this, 1, false);
        return picked.empty() ? nullptr : picked.front();
    }

    std::vector<Unit*> randomPlayers(size_t count) { return dazaralorPickPlayers(this, count, false); }

private:
    uint32_t mLeftMs;
    uint32_t mEveryMs;
    uint8_t mSteps;
    uint32_t mLingerMs;
    bool mTargetable;
    uint8_t mStep = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Champion of the Light: Frida Ironbellows or Ra'wani Kanae

namespace
{
    struct DazaralorChampionSide
    {
        uint32_t crusaderEntry;
        uint32_t discipleEntry;
        LocationVector reinforcementSpot;
    };

    DazaralorChampionSide const FridaSide = { NPC_DARKFORGED_CRUSADER, NPC_ANOINTED_DISCIPLE, { -631.0f, 1089.0f, 321.0f, 0.77f } };
    DazaralorChampionSide const RawaniSide = { NPC_ZANDALARI_CRUSADER, NPC_REZANI_DISCIPLE, { -2023.0f, 833.0f, 6.0f, 4.73f } };
}

class BattleOfDazaralorChampionAI : public BattleOfDazaralorEncounterAI
{
public:
    BattleOfDazaralorChampionAI(Creature* pCreature, DazaralorChampionSide const& side)
        : BattleOfDazaralorEncounterAI(pCreature, DATA_CHAMPION_OF_THE_LIGHT, DazaralorBerserkMs), mSide(side) {}

protected:
    void onAction(int32_t action) override
    {
        // every fallen follower fuels the champion
        if (action == ACTION_CHAMPION_ADD_FALLEN && _isInCombat())
            castSpellOnSelf(SPELL_CHAMPION_ZEALOTRY, true);
    }

    void onEngage() override
    {
        mReckoning = false;
        mWrathDone = false;
        mEnergy.reset(ChampionRetributionGain);
        castSpellOnSelf(SPELL_CHAMPION_SEAL_OF_RETRIBUTION, true);

        scriptEvents.addEvent(EVENT_CHAMPION_WAVE_OF_LIGHT, 12000);
        scriptEvents.addEvent(EVENT_CHAMPION_JUDGMENT_RIGHTEOUSNESS, 50000);
        scriptEvents.addEvent(EVENT_CHAMPION_CALL_TO_ARMS, 65000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_CHAMPION_DIVINE_PROTECTION, 15000);
    }

    void onCombatTick(uint32_t diff) override
    {
        if (!mWrathDone && _getHealthPercent() <= 30)
        {
            mWrathDone = true;
            castSpellOnSelf(SPELL_CHAMPION_AVENGING_WRATH, true);
        }

        mEnergy.fill(diff);
        if (mEnergy.isFull() && !_isCasting())
            changeSeal();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CHAMPION_WAVE_OF_LIGHT:
                castAtPlayer(SPELL_CHAMPION_WAVE_OF_LIGHT);
                scriptEvents.addEvent(EVENT_CHAMPION_WAVE_OF_LIGHT, 20000);
                break;
            case EVENT_CHAMPION_JUDGMENT_RIGHTEOUSNESS:
                if (Creature* pFollower = randomFollower())
                    castSpell(pFollower, SPELL_CHAMPION_JUDGMENT_RIGHTEOUSNESS);
                scriptEvents.addEvent(EVENT_CHAMPION_JUDGMENT_RIGHTEOUSNESS, 30000);
                break;
            case EVENT_CHAMPION_CALL_TO_ARMS:
                castSpellOnSelf(SPELL_CHAMPION_CALL_TO_ARMS);
                callAdd(mSide.crusaderEntry, mSide.reinforcementSpot);
                callAdd(mSide.discipleEntry, mSide.reinforcementSpot);
                scriptEvents.addEvent(EVENT_CHAMPION_CALL_TO_ARMS, 35000);
                break;
            case EVENT_CHAMPION_DIVINE_PROTECTION:
                protectWoundedFollowers();
                scriptEvents.addEvent(EVENT_CHAMPION_DIVINE_PROTECTION, 15000);
                break;
            default:
                break;
        }
    }

private:
    // full bar: the seals take turns, the reckoning seal ends with a judgment
    void changeSeal()
    {
        if (!mReckoning)
        {
            _removeAura(SPELL_CHAMPION_SEAL_OF_RETRIBUTION);
            castSpellOnSelf(SPELL_CHAMPION_SEAL_OF_RECKONING, true);
            mEnergy.reset(ChampionReckoningGain);
            mReckoning = true;
            return;
        }

        _removeAura(SPELL_CHAMPION_SEAL_OF_RECKONING);
        castSpellOnSelf(SPELL_CHAMPION_SEAL_OF_RETRIBUTION, true);
        mEnergy.reset(ChampionRetributionGain);
        mReckoning = false;

        warn("The Champion of the Light unleashes Judgment: Reckoning!");
        castSpellAOE(SPELL_CHAMPION_JUDGMENT_RECKONING);
    }

    Creature* randomFollower()
    {
        bool const crusaderFirst = Util::getRandomUInt(1) == 0;
        uint32_t const entries[2] =
        {
            crusaderFirst ? mSide.crusaderEntry : mSide.discipleEntry,
            crusaderFirst ? mSide.discipleEntry : mSide.crusaderEntry
        };

        for (uint32_t entry : entries)
        {
            Creature* pFollower = findNearestCreature(entry, 100.0f);
            if (pFollower != nullptr && pFollower->isAlive())
                return pFollower;
        }

        return nullptr;
    }

    void protectWoundedFollowers()
    {
        uint32_t const entries[2] = { mSide.crusaderEntry, mSide.discipleEntry };
        for (uint32_t entry : entries)
        {
            Creature* pFollower = findNearestCreature(entry, 100.0f);
            if (pFollower == nullptr || !pFollower->isAlive() || pFollower->getHealthPct() >= 99)
                continue;

            if (!pFollower->hasAurasWithId(SPELL_CHAMPION_DIVINE_PROTECTION))
                castSpell(pFollower, SPELL_CHAMPION_DIVINE_PROTECTION, true);
        }
    }

    DazaralorChampionSide const& mSide;
    DazaralorEnergyBar mEnergy;
    bool mReckoning = false;
    bool mWrathDone = false;
};

class BattleOfDazaralorFridaAI : public BattleOfDazaralorChampionAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorFridaAI(c); }
    explicit BattleOfDazaralorFridaAI(Creature* pCreature) : BattleOfDazaralorChampionAI(pCreature, FridaSide) {}
};

class BattleOfDazaralorRawaniAI : public BattleOfDazaralorChampionAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorRawaniAI(c); }
    explicit BattleOfDazaralorRawaniAI(Creature* pCreature) : BattleOfDazaralorChampionAI(pCreature, RawaniSide) {}
};

// followers of both champions
class BattleOfDazaralorFollowerAI : public BattleOfDazaralorFightAI
{
public:
    explicit BattleOfDazaralorFollowerAI(Creature* pCreature) : BattleOfDazaralorFightAI(pCreature, true) {}

protected:
    void onDefeat(Unit* /*pKiller*/) override
    {
        signal(BOSS_FRIDA_IRONBELLOWS, ACTION_CHAMPION_ADD_FALLEN);
        signal(BOSS_RAWANI_KANAE, ACTION_CHAMPION_ADD_FALLEN);
    }

    Creature* livingChampion()
    {
        Creature* pChampion = findAlive(BOSS_FRIDA_IRONBELLOWS);
        return pChampion != nullptr ? pChampion : findAlive(BOSS_RAWANI_KANAE);
    }
};

class BattleOfDazaralorCrusaderAI : public BattleOfDazaralorFollowerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorCrusaderAI(c); }
    explicit BattleOfDazaralorCrusaderAI(Creature* pCreature) : BattleOfDazaralorFollowerAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_CRUSADER_STRIKE, 3000);
        scriptEvents.addEvent(EVENT_CRUSADER_BLINDING_FAITH, 13000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_CRUSADER_STRIKE:
                castSpellOnVictim(SPELL_CRUSADER_STRIKE);
                scriptEvents.addEvent(EVENT_CRUSADER_STRIKE, 15000);
                break;
            case EVENT_CRUSADER_BLINDING_FAITH:
                castSpellAOE(SPELL_CRUSADER_BLINDING_FAITH);
                scriptEvents.addEvent(EVENT_CRUSADER_BLINDING_FAITH, 25000);
                break;
            default:
                break;
        }
    }

    // the flash disorients whoever faces the crusader
    void onSpellDone(uint32_t spellId) override
    {
        if (spellId == SPELL_CRUSADER_BLINDING_FAITH)
            strikeLater(currentVictim(), SPELL_CRUSADER_BLINDING_FAITH_DISORIENT, 100);
    }
};

class BattleOfDazaralorDiscipleAI : public BattleOfDazaralorFollowerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorDiscipleAI(c); }
    explicit BattleOfDazaralorDiscipleAI(Creature* pCreature) : BattleOfDazaralorFollowerAI(pCreature) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_DISCIPLE_DIVINE_BURST, 3000);
        scriptEvents.addEvent(EVENT_DISCIPLE_HEAL, 8000);
        scriptEvents.addEvent(EVENT_DISCIPLE_PENANCE_MEND, 13000);
        scriptEvents.addEvent(EVENT_DISCIPLE_PENANCE_SMITE, 18000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_DISCIPLE_DIVINE_BURST:
                castAtPlayer(SPELL_DISCIPLE_DIVINE_BURST);
                scriptEvents.addEvent(EVENT_DISCIPLE_DIVINE_BURST, 15000);
                break;
            case EVENT_DISCIPLE_HEAL:
            {
                // the champion first, otherwise itself
                Creature* pChampion = livingChampion();
                Unit* pPatient = pChampion != nullptr && pChampion->getHealthPct() < 100 ? static_cast<Unit*>(pChampion) : getCreature();
                castSpell(pPatient, SPELL_DISCIPLE_HEAL);
                scriptEvents.addEvent(EVENT_DISCIPLE_HEAL, 20000);
                break;
            }
            case EVENT_DISCIPLE_PENANCE_MEND:
                if (Creature* pCrusader = woundedCrusader())
                    castSpell(pCrusader, SPELL_DISCIPLE_PENANCE_MEND);
                scriptEvents.addEvent(EVENT_DISCIPLE_PENANCE_MEND, 25000);
                break;
            case EVENT_DISCIPLE_PENANCE_SMITE:
                castAtPlayer(SPELL_DISCIPLE_PENANCE_SMITE, false, true);
                scriptEvents.addEvent(EVENT_DISCIPLE_PENANCE_SMITE, 30000);
                break;
            default:
                break;
        }
    }

private:
    Creature* woundedCrusader()
    {
        uint32_t const entries[2] = { NPC_DARKFORGED_CRUSADER, NPC_ZANDALARI_CRUSADER };
        for (uint32_t entry : entries)
        {
            Creature* pCrusader = findNearestCreature(entry, 100.0f);
            if (pCrusader != nullptr && pCrusader->isAlive() && pCrusader->getHealthPct() < 99)
                return pCrusader;
        }

        return nullptr;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Grong and Grong the Revenant

namespace
{
    struct DazaralorApeKit
    {
        uint32_t dataId;
        uint32_t channelerEntry;
        uint32_t slamSpell;
        uint32_t tantrumSpell;
        bool gnomishGear;   // missile and the three bestial finishers
    };

    DazaralorApeKit const GrongKit = { DATA_GRONG, NPC_APETAGONIZER_3000, SPELL_GRONG_REVERBERATING_SLAM, SPELL_GRONG_TANTRUM, true };
    DazaralorApeKit const RevenantKit = { DATA_GRONG_THE_REVENANT, NPC_DEATH_SPECTER, SPELL_REVENANT_DEATHLY_SLAM, SPELL_REVENANT_DEATH_KNELL, false };
}

class BattleOfDazaralorApeAI : public BattleOfDazaralorEncounterAI
{
public:
    BattleOfDazaralorApeAI(Creature* pCreature, DazaralorApeKit const& kit)
        : BattleOfDazaralorEncounterAI(pCreature, kit.dataId, DazaralorBerserkMs), mKit(kit) {}

protected:
    void onAction(int32_t action) override
    {
        if (!_isInCombat())
            return;

        if (action == ACTION_APE_CHANNEL_HIT)
            mEnergy.add(isHeroic() ? 10.0f : 5.0f);
        else if (action == ACTION_APE_DETONATION)
            mEnergy.add(isHeroic() ? 50.0f : 25.0f);
    }

    void onEngage() override
    {
        mEnergy.reset(ApeEnergyGain);

        scriptEvents.addEvent(EVENT_APE_SLAM, 3000);
        scriptEvents.addEvent(EVENT_APE_COMBO, 11000);
        scriptEvents.addEvent(EVENT_APE_CALL_HELPER, 30000);
        if (mKit.gnomishGear)
            scriptEvents.addEvent(EVENT_APE_MISSILE, 25000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_APE_ROAR, 8000);
    }

    void onCombatTick(uint32_t diff) override
    {
        mEnergy.fill(diff);
        if (!mEnergy.isFull() || _isCasting())
            return;

        mEnergy.reset(ApeEnergyGain);
        warn("Grong flies into a rage!");
        castSpellAOE(mKit.tantrumSpell);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_APE_SLAM:
                castAtPlayer(mKit.slamSpell);
                scriptEvents.addEvent(EVENT_APE_SLAM, 20000);
                break;
            case EVENT_APE_COMBO:
                performCombo();
                scriptEvents.addEvent(EVENT_APE_COMBO, 25000);
                break;
            case EVENT_APE_MISSILE:
                castAtPlayer(SPELL_GRONG_MEGATOMIC_SEEKER_MISSILE, false, true);
                scriptEvents.addEvent(EVENT_APE_MISSILE, 25000);
                break;
            case EVENT_APE_CALL_HELPER:
                callAdd(mKit.channelerEntry, aroundSelf(3.0f, 6.0f));
                scriptEvents.addEvent(EVENT_APE_CALL_HELPER, 45000);
                break;
            case EVENT_APE_ROAR:
                castSpellAOE(SPELL_GRONG_FEROCIOUS_ROAR);
                scriptEvents.addEvent(EVENT_APE_ROAR, Util::getRandomUInt(30000, 45000));
                break;
            default:
                break;
        }
    }

    void onSpellDone(uint32_t spellId) override
    {
        if (spellId == SPELL_GRONG_RENDING_BITE)
            strikeLater(currentVictim(), SPELL_GRONG_RENDING_BITE_BLEED, 100);
    }

private:
    void performCombo()
    {
        if (!mKit.gnomishGear)
        {
            castSpellOnSelf(SPELL_REVENANT_NECROTIC_COMBO, true);
            castSpellOnVictim(SPELL_REVENANT_NECROTIC_SMASH);
            return;
        }

        castSpellOnSelf(SPELL_GRONG_BESTIAL_COMBO, true);
        switch (Util::getRandomUInt(2))
        {
            case 0:
                castSpellOnVictim(SPELL_GRONG_BESTIAL_SMASH);
                break;
            case 1:
                castSpellAOE(SPELL_GRONG_RENDING_BITE);
                break;
            default:
            {
                // the tank is hurled into another player
                Unit* pTank = currentVictim();
                if (pTank == nullptr)
                    break;

                castSpell(pTank, SPELL_GRONG_BESTIAL_THROW);
                if (Unit* pLanding = pickPlayer(true))
                    strikeLater(pLanding, SPELL_GRONG_BESTIAL_IMPACT, 6000);
                break;
            }
        }
    }

    DazaralorApeKit const& mKit;
    DazaralorEnergyBar mEnergy;
};

class BattleOfDazaralorGrongAI : public BattleOfDazaralorApeAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorGrongAI(c); }
    explicit BattleOfDazaralorGrongAI(Creature* pCreature) : BattleOfDazaralorApeAI(pCreature, GrongKit) {}
};

class BattleOfDazaralorGrongRevenantAI : public BattleOfDazaralorApeAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorGrongRevenantAI(c); }
    explicit BattleOfDazaralorGrongRevenantAI(Creature* pCreature) : BattleOfDazaralorApeAI(pCreature, RevenantKit) {}
};

// stationary add that keeps feeding the ape's energy
class BattleOfDazaralorApeChannelerAI : public BattleOfDazaralorFightAI
{
public:
    BattleOfDazaralorApeChannelerAI(Creature* pCreature, uint32_t apeEntry, uint32_t channelSpell, uint32_t deathSpell)
        : BattleOfDazaralorFightAI(pCreature, true), mApeEntry(apeEntry), mChannelSpell(channelSpell), mDeathSpell(deathSpell) {}

protected:
    void onEngage() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_CHANNELER_EMPOWER, 5000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_CHANNELER_EMPOWER)
            return;

        if (Creature* pApe = findAlive(mApeEntry))
        {
            castSpell(pApe, mChannelSpell);
            if (pApe->GetScript() != nullptr)
                pApe->GetScript()->DoAction(ACTION_APE_CHANNEL_HIT);
        }

        scriptEvents.addEvent(EVENT_CHANNELER_EMPOWER, 5000);
    }

    void onDefeat(Unit* /*pKiller*/) override
    {
        if (mDeathSpell == 0)
            return;

        castSpellAOE(mDeathSpell, true);
        signal(mApeEntry, ACTION_APE_DETONATION);
    }

private:
    uint32_t mApeEntry;
    uint32_t mChannelSpell;
    uint32_t mDeathSpell;
};

class BattleOfDazaralorApetagonizerAI : public BattleOfDazaralorApeChannelerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorApetagonizerAI(c); }
    explicit BattleOfDazaralorApetagonizerAI(Creature* pCreature)
        : BattleOfDazaralorApeChannelerAI(pCreature, BOSS_GRONG, SPELL_APETAGONIZER_APETAGONIZE, SPELL_APETAGONIZER_LIGHTNING_DETONATION) {}
};

class BattleOfDazaralorDeathSpecterAI : public BattleOfDazaralorApeChannelerAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorDeathSpecterAI(c); }
    explicit BattleOfDazaralorDeathSpecterAI(Creature* pCreature)
        : BattleOfDazaralorApeChannelerAI(pCreature, BOSS_GRONG_THE_REVENANT, SPELL_DEATH_SPECTER_DEATH_EMPOWERMENT, 0) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// Jadefire Masters: a fire mage and a monk on each side

class BattleOfDazaralorJadefireMageAI : public BattleOfDazaralorEncounterAI
{
public:
    BattleOfDazaralorJadefireMageAI(Creature* pCreature, uint32_t dataId, LocationVector const* bombSpots)
        : BattleOfDazaralorEncounterAI(pCreature, dataId, DazaralorBerserkMs), mBombSpots(bombSpots) {}

protected:
    void onEngage() override
    {
        mPhoenix = false;
        mMistCalled = false;
        mEnergy.reset(JadefireEnergyGain);

        scriptEvents.addEvent(EVENT_MAGE_FIREBALL, 1000);
        scriptEvents.addEvent(EVENT_MAGE_PYROBLAST, 15000);
        scriptEvents.addEvent(EVENT_MAGE_SEARING_EMBERS, 20000);
        scriptEvents.addEvent(EVENT_MAGE_MAGMA_TRAP, 25000);
    }

    void onCombatTick(uint32_t diff) override
    {
        mEnergy.fill(diff);

        // team attack at a third of the bar
        if (!mMistCalled && mEnergy.value() >= 30.0f)
        {
            mMistCalled = true;
            callFireFromMist();
        }

        if (!mEnergy.isFull() || _isCasting())
            return;

        mEnergy.reset(JadefireEnergyGain);
        mMistCalled = false;
        if (!mPhoenix)
            becomePhoenix();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MAGE_FIREBALL:
                castSpellOnVictim(SPELL_JADEFIRE_FIREBALL);
                scriptEvents.addEvent(EVENT_MAGE_FIREBALL, 3000);
                break;
            case EVENT_MAGE_PYROBLAST:
                castSpellOnSelf(SPELL_JADEFIRE_FIRE_SHIELD, true);
                castAtPlayer(SPELL_JADEFIRE_PYROBLAST);
                scriptEvents.addEvent(EVENT_MAGE_PYROBLAST, 20000);
                break;
            case EVENT_MAGE_SEARING_EMBERS:
                for (Unit* pTarget : pickPlayers(5))
                    castSpell(pTarget, SPELL_JADEFIRE_SEARING_EMBERS, true);
                scriptEvents.addEvent(EVENT_MAGE_SEARING_EMBERS, 25000);
                break;
            case EVENT_MAGE_MAGMA_TRAP:
                for (Unit* pTarget : pickPlayers(3))
                    castSpell(pTarget, SPELL_JADEFIRE_MAGMA_TRAP, true);
                scriptEvents.addEvent(EVENT_MAGE_MAGMA_TRAP, 35000);
                break;
            case EVENT_MAGE_RISING_FLAMES:
                castSpellOnVictim(SPELL_JADEFIRE_RISING_FLAMES);
                scriptEvents.addEvent(EVENT_MAGE_RISING_FLAMES, 3000);
                break;
            default:
                break;
        }
    }

private:
    void callFireFromMist()
    {
        warn("Living Bombs drop from the mist!");
        castSpellOnSelf(SPELL_JADEFIRE_FIRE_FROM_MIST, true);
        for (uint8_t i = 0; i < 2; ++i)
            placeHelper(NPC_LIVING_BOMB, mBombSpots[i], 15000);
    }

    // a full bar turns the mage into a phoenix for the rest of the fight
    void becomePhoenix()
    {
        mPhoenix = true;
        scriptEvents.removeEvent(EVENT_MAGE_FIREBALL);
        scriptEvents.removeEvent(EVENT_MAGE_PYROBLAST);
        scriptEvents.removeEvent(EVENT_MAGE_SEARING_EMBERS);

        castSpellOnSelf(SPELL_JADEFIRE_BLAZING_PHOENIX, true);
        scriptEvents.addEvent(EVENT_MAGE_RISING_FLAMES, 3000);
    }

    LocationVector const* mBombSpots;
    DazaralorEnergyBar mEnergy;
    bool mPhoenix = false;
    bool mMistCalled = false;
};

class BattleOfDazaralorJadefireMonkAI : public BattleOfDazaralorEncounterAI
{
public:
    BattleOfDazaralorJadefireMonkAI(Creature* pCreature, uint32_t dataId, LocationVector const* xuenSpot)
        : BattleOfDazaralorEncounterAI(pCreature, dataId, DazaralorBerserkMs), mXuenSpot(xuenSpot) {}

protected:
    void onEngage() override
    {
        mTransformed = false;
        mEnergy.reset(JadefireEnergyGain);

        scriptEvents.addEvent(EVENT_MONK_JADE_STORM, 3000);
        scriptEvents.addEvent(EVENT_MONK_SPIRITS_OF_XUEN, 20000);
    }

    void onCombatTick(uint32_t diff) override
    {
        mEnergy.fill(diff);
        if (!mEnergy.isFull() || _isCasting())
            return;

        mEnergy.reset(JadefireEnergyGain);
        if (mTransformed)
            return;

        // full bar: the monk trades the jade storm for dragon's breath
        mTransformed = true;
        scriptEvents.removeEvent(EVENT_MONK_JADE_STORM);
        scriptEvents.removeEvent(EVENT_MONK_SPIRITS_OF_XUEN);
        scriptEvents.addEvent(EVENT_MONK_DRAGONS_BREATH, 3000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MONK_JADE_STORM:
                castSpellAOE(SPELL_JADEFIRE_WHIRLING_JADE_STORM);
                scriptEvents.addEvent(EVENT_MONK_JADE_STORM, 15000);
                break;
            case EVENT_MONK_SPIRITS_OF_XUEN:
                castSpellAOE(SPELL_JADEFIRE_SPIRITS_OF_XUEN);
                callAdd(NPC_SPIRIT_OF_XUEN, mXuenSpot != nullptr ? *mXuenSpot : aroundSelf(3.0f, 10.0f));
                scriptEvents.addEvent(EVENT_MONK_SPIRITS_OF_XUEN, 30000);
                break;
            case EVENT_MONK_DRAGONS_BREATH:
                castSpellOnSelf(SPELL_JADEFIRE_DRAGONS_BREATH);
                scriptEvents.addEvent(EVENT_MONK_DRAGONS_BREATH, 15000);
                break;
            default:
                break;
        }
    }

private:
    LocationVector const* mXuenSpot;
    DazaralorEnergyBar mEnergy;
    bool mTransformed = false;
};

class BattleOfDazaralorManceroyAI : public BattleOfDazaralorJadefireMageAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorManceroyAI(c); }
    explicit BattleOfDazaralorManceroyAI(Creature* pCreature)
        : BattleOfDazaralorJadefireMageAI(pCreature, DATA_JADEFIRE_MASTERS_MESTRAH, MestrahSideBombSpots) {}
};

class BattleOfDazaralorMestrahAI : public BattleOfDazaralorJadefireMonkAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorMestrahAI(c); }
    explicit BattleOfDazaralorMestrahAI(Creature* pCreature)
        : BattleOfDazaralorJadefireMonkAI(pCreature, DATA_JADEFIRE_MASTERS_MESTRAH, &MestrahSideXuenSpot) {}
};

class BattleOfDazaralorAnathosAI : public BattleOfDazaralorJadefireMageAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorAnathosAI(c); }
    explicit BattleOfDazaralorAnathosAI(Creature* pCreature)
        : BattleOfDazaralorJadefireMageAI(pCreature, DATA_JADEFIRE_MASTERS_MARA, MaraSideBombSpots) {}
};

class BattleOfDazaralorMaraAI : public BattleOfDazaralorJadefireMonkAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorMaraAI(c); }
    explicit BattleOfDazaralorMaraAI(Creature* pCreature)
        : BattleOfDazaralorJadefireMonkAI(pCreature, DATA_JADEFIRE_MASTERS_MARA, nullptr) {}
};

// explodes after its cast unless killed first
class BattleOfDazaralorLivingBombAI : public BattleOfDazaralorHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorLivingBombAI(c); }
    explicit BattleOfDazaralorLivingBombAI(Creature* pCreature) : BattleOfDazaralorHelperAI(pCreature, 1000, 0, 1, 8000, true) {}

protected:
    void onStep(uint8_t /*step*/) override { castSpellAOE(SPELL_LIVING_BOMB_EXPLOSION); }
};

// hunts one player
class BattleOfDazaralorSpiritOfXuenAI : public BattleOfDazaralorFightAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorSpiritOfXuenAI(c); }
    explicit BattleOfDazaralorSpiritOfXuenAI(Creature* pCreature) : BattleOfDazaralorFightAI(pCreature, false) {}

protected:
    void onEngage() override
    {
        if (Unit* pPrey = pickPlayer(true))
        {
            castSpell(pPrey, SPELL_XUEN_STALKING, true);
            addThreat(pPrey, 1000000.0f);
        }

        scriptEvents.addEvent(EVENT_XUEN_POUNCE, 3000);
        scriptEvents.addEvent(EVENT_XUEN_TIGER_PAW, 6000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_XUEN_POUNCE:
                castSpellAOE(SPELL_XUEN_POUNCE);
                scriptEvents.addEvent(EVENT_XUEN_POUNCE, 5000);
                break;
            case EVENT_XUEN_TIGER_PAW:
                castSpellOnVictim(SPELL_XUEN_TIGER_PAW);
                scriptEvents.addEvent(EVENT_XUEN_TIGER_PAW, 5000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Opulence

class BattleOfDazaralorOpulenceAI : public BattleOfDazaralorEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorOpulenceAI(c); }
    explicit BattleOfDazaralorOpulenceAI(Creature* pCreature) : BattleOfDazaralorEncounterAI(pCreature, DATA_OPULENCE, DazaralorBerserkMs) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_OPULENCE_HOARD_POWER, 1000);
        scriptEvents.addEvent(EVENT_OPULENCE_COIN_SWEEP, 7000);
        scriptEvents.addEvent(EVENT_OPULENCE_LIQUID_GOLD, 12000);
        scriptEvents.addEvent(EVENT_OPULENCE_SPIRITS_OF_GOLD, 17000);
        scriptEvents.addEvent(EVENT_OPULENCE_COIN_SHOWER, 20000);
        scriptEvents.addEvent(EVENT_OPULENCE_WAIL_OF_GREED, 23000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_OPULENCE_HOARD_POWER:
                castSpellAOE(SPELL_OPULENCE_HOARD_POWER);
                break;
            case EVENT_OPULENCE_COIN_SWEEP:
                castSpellOnVictim(SPELL_OPULENCE_COIN_SWEEP);
                scriptEvents.addEvent(EVENT_OPULENCE_COIN_SWEEP, 10000);
                break;
            case EVENT_OPULENCE_LIQUID_GOLD:
                for (Unit* pTarget : pickPlayers(2))
                    castSpell(pTarget, SPELL_OPULENCE_LIQUID_GOLD, true);
                scriptEvents.addEvent(EVENT_OPULENCE_LIQUID_GOLD, 15000);
                break;
            case EVENT_OPULENCE_SPIRITS_OF_GOLD:
                castSpellOnSelf(SPELL_OPULENCE_SPIRITS_OF_GOLD);
                for (uint8_t i = 0; i < 8; ++i)
                    callAdd(NPC_SPIRIT_OF_GOLD, aroundSelf(2.0f, 6.0f));
                scriptEvents.addEvent(EVENT_OPULENCE_SPIRITS_OF_GOLD, 60000);
                break;
            case EVENT_OPULENCE_COIN_SHOWER:
                warn("Opulence showers the raid with coins!");
                for (Unit* pTarget : pickPlayers(10))
                    castSpell(pTarget, SPELL_OPULENCE_COIN_SHOWER, true);
                scriptEvents.addEvent(EVENT_OPULENCE_COIN_SHOWER, 20000);
                break;
            case EVENT_OPULENCE_WAIL_OF_GREED:
                castSpellOnSelf(SPELL_OPULENCE_GREED, true);
                castSpellAOE(SPELL_OPULENCE_WAIL_OF_GREED);
                scriptEvents.addEvent(EVENT_OPULENCE_WAIL_OF_GREED, 35000);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Conclave of the Chosen: Pa'ku and Gonk, then Kimbul, then Akunda

class BattleOfDazaralorAspectAI : public BattleOfDazaralorEncounterAI
{
public:
    BattleOfDazaralorAspectAI(Creature* pCreature, uint32_t partnerEntry)
        : BattleOfDazaralorEncounterAI(pCreature, DATA_CONCLAVE_OF_THE_CHOSEN, DazaralorBerserkMs), mPartnerEntry(partnerEntry) {}

protected:
    void onAction(int32_t action) override
    {
        if (action != ACTION_CONCLAVE_PARTNER_FALLEN || !_isInCombat())
            return;

        _removeAura(SPELL_CONCLAVE_LOAS_PACT);
        castSpellOnSelf(SPELL_CONCLAVE_LOAS_WRATH, true);
    }

    void onCombatTick(uint32_t diff) override
    {
        onAspectTick(diff);
        if (mPartnerEntry == 0)
            return;

        if (mPactCheckMs > diff)
        {
            mPactCheckMs -= diff;
            return;
        }
        mPactCheckMs = 1000;

        // partners standing together share the pact
        Creature* pPartner = findAlive(mPartnerEntry);
        bool const together = pPartner != nullptr && getRangeToObject(pPartner) <= 7.0f;
        bool const bound = hasAura(SPELL_CONCLAVE_LOAS_PACT);
        if (together && !bound)
            castSpellOnSelf(SPELL_CONCLAVE_LOAS_PACT, true);
        else if (!together && bound)
            _removeAura(SPELL_CONCLAVE_LOAS_PACT);
    }

    void onDefeat(Unit* /*pKiller*/) override
    {
        if (mPartnerEntry != 0)
            signal(mPartnerEntry, ACTION_CONCLAVE_PARTNER_FALLEN);
    }

    virtual void onAspectTick(uint32_t /*diff*/) {}

private:
    uint32_t mPartnerEntry;
    uint32_t mPactCheckMs = 1000;
};

class BattleOfDazaralorPakuAI : public BattleOfDazaralorAspectAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorPakuAI(c); }
    explicit BattleOfDazaralorPakuAI(Creature* pCreature) : BattleOfDazaralorAspectAI(pCreature, BOSS_GONKS_ASPECT) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_PAKU_HASTENING_WINDS, 2000);
        scriptEvents.addEvent(EVENT_PAKU_GIFT_OF_WIND, 18000);
        scriptEvents.addEvent(EVENT_PAKU_WRATH, 20000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_PAKU_KRAGWA, 35000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_PAKU_HASTENING_WINDS:
                castSpellOnSelf(SPELL_PAKU_HASTENING_WINDS, true);
                scriptEvents.addEvent(EVENT_PAKU_HASTENING_WINDS, 2000);
                break;
            case EVENT_PAKU_GIFT_OF_WIND:
                castSpellAOE(SPELL_PAKU_GIFT_OF_WIND);
                scriptEvents.addEvent(EVENT_PAKU_GIFT_OF_WIND, Util::getRandomUInt(15000, 18000));
                break;
            case EVENT_PAKU_WRATH:
                warn("Pa'ku's Wrath! Gather beneath Pa'ku!");
                placeHelper(NPC_PAKU, dazaralorRingPoint(ConclaveCentre, 0.0f, 30.0f), 12000);
                scriptEvents.addEvent(EVENT_PAKU_WRATH, 30000);
                break;
            case EVENT_PAKU_KRAGWA:
                placeHelper(NPC_KRAGWA, ConclaveCentre, 10000);
                scriptEvents.addEvent(EVENT_PAKU_KRAGWA, 70000);
                break;
            default:
                break;
        }
    }
};

class BattleOfDazaralorGonkAI : public BattleOfDazaralorAspectAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorGonkAI(c); }
    explicit BattleOfDazaralorGonkAI(Creature* pCreature) : BattleOfDazaralorAspectAI(pCreature, BOSS_PAKUS_ASPECT) {}

protected:
    void onEngage() override
    {
        mEnergy.reset(GonkEnergyGain);
        scriptEvents.addEvent(EVENT_GONK_RAPTOR_FORM, 15000);
        scriptEvents.addEvent(EVENT_GONK_CRAWLING_HEX, 50000);
    }

    void onAspectTick(uint32_t diff) override
    {
        mEnergy.fill(diff);
        if (!mEnergy.isFull() || _isCasting())
            return;

        // the wrath spends three quarters of the bar
        mEnergy.reset(GonkEnergyGain, 25.0f);
        warn("Gonk's Wrath! Ravenous Stalkers join the hunt!");
        placeHelper(NPC_GONK, ConclaveCentre, 4000);
        for (uint8_t i = 0; i < 5; ++i)
            callAdd(NPC_RAVENOUS_STALKER, dazaralorRingPoint(ConclaveCentre, 5.0f, 30.0f));
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_GONK_RAPTOR_FORM:
                castSpellOnSelf(SPELL_GONK_WILD_MAUL);
                scriptEvents.addEvent(EVENT_GONK_RAPTOR_FORM, Util::getRandomUInt(15000, 18000));
                break;
            case EVENT_GONK_CRAWLING_HEX:
                for (Unit* pTarget : pickPlayers(3, true))
                    castSpell(pTarget, SPELL_GONK_CRAWLING_HEX, true);
                scriptEvents.addEvent(EVENT_GONK_CRAWLING_HEX, 50000);
                break;
            default:
                break;
        }
    }

private:
    DazaralorEnergyBar mEnergy;
};

class BattleOfDazaralorKimbulAI : public BattleOfDazaralorAspectAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorKimbulAI(c); }
    explicit BattleOfDazaralorKimbulAI(Creature* pCreature) : BattleOfDazaralorAspectAI(pCreature, 0) {}

protected:
    void onJoinFight() override { warn("Kimbul's Aspect joins the fight!"); }

    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KIMBUL_LACERATING_CLAWS, 15000);
        scriptEvents.addEvent(EVENT_KIMBUL_WRATH, 20000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KIMBUL_LACERATING_CLAWS:
                castSpellOnVictim(SPELL_KIMBUL_LACERATING_CLAWS, true);
                scriptEvents.addEvent(EVENT_KIMBUL_LACERATING_CLAWS, 20000);
                break;
            case EVENT_KIMBUL_WRATH:
                placeHelper(NPC_KIMBUL, ConclaveCentre, 8000);
                scriptEvents.addEvent(EVENT_KIMBUL_WRATH, 60000);
                break;
            default:
                break;
        }
    }
};

class BattleOfDazaralorAkundaAI : public BattleOfDazaralorAspectAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorAkundaAI(c); }
    explicit BattleOfDazaralorAkundaAI(Creature* pCreature) : BattleOfDazaralorAspectAI(pCreature, 0) {}

protected:
    void onJoinFight() override { warn("Akunda's Aspect joins the fight!"); }

    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_AKUNDA_THUNDERING_STORM, 15000);
        scriptEvents.addEvent(EVENT_AKUNDA_MIND_WIPE, 20000);
        scriptEvents.addEvent(EVENT_AKUNDA_WRATH, 25000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_AKUNDA_THUNDERING_STORM:
                castSpellAOE(SPELL_AKUNDA_THUNDERING_STORM);
                scriptEvents.addEvent(EVENT_AKUNDA_THUNDERING_STORM, 20000);
                break;
            case EVENT_AKUNDA_MIND_WIPE:
                for (Unit* pTarget : pickPlayers(2, true))
                {
                    castSpell(pTarget, SPELL_AKUNDA_MIND_WIPE_VISUAL, true);
                    castSpell(pTarget, SPELL_AKUNDA_MIND_WIPE, true);
                }
                scriptEvents.addEvent(EVENT_AKUNDA_MIND_WIPE, 50000);
                break;
            case EVENT_AKUNDA_WRATH:
                placeHelper(NPC_AKUNDA, ConclaveCentre, 10000);
                scriptEvents.addEvent(EVENT_AKUNDA_WRATH, 60000);
                break;
            default:
                break;
        }
    }
};

// Pa'ku circles over one spot and pulses
class BattleOfDazaralorPakuWrathAI : public BattleOfDazaralorHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorPakuWrathAI(c); }
    explicit BattleOfDazaralorPakuWrathAI(Creature* pCreature) : BattleOfDazaralorHelperAI(pCreature, 1000, 0, 1, 9000, false) {}

protected:
    void onStep(uint8_t /*step*/) override { castSpellOnSelf(SPELL_PAKU_PAKUS_WRATH); }
};

class BattleOfDazaralorGonkWrathAI : public BattleOfDazaralorHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorGonkWrathAI(c); }
    explicit BattleOfDazaralorGonkWrathAI(Creature* pCreature) : BattleOfDazaralorHelperAI(pCreature, 500, 0, 1, 3000, false) {}

protected:
    void onStep(uint8_t /*step*/) override { castSpellAOE(SPELL_GONK_GONKS_WRATH); }
};

// four pounces on random players
class BattleOfDazaralorKimbulWrathAI : public BattleOfDazaralorHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorKimbulWrathAI(c); }
    explicit BattleOfDazaralorKimbulWrathAI(Creature* pCreature) : BattleOfDazaralorHelperAI(pCreature, 1000, 1000, 4, 2000, false) {}

protected:
    void onStep(uint8_t /*step*/) override
    {
        Unit* pTarget = randomPlayer();
        if (pTarget == nullptr)
            return;

        castSpell(pTarget, SPELL_KIMBUL_KIMBULS_WRATH, true);
        castSpell(pTarget, SPELL_KIMBUL_BLEEDING_WOUNDS, true);
    }
};

class BattleOfDazaralorAkundaWrathAI : public BattleOfDazaralorHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorAkundaWrathAI(c); }
    explicit BattleOfDazaralorAkundaWrathAI(Creature* pCreature) : BattleOfDazaralorHelperAI(pCreature, 500, 0, 1, 8000, false) {}

protected:
    void onStep(uint8_t /*step*/) override
    {
        for (Unit* pTarget : randomPlayers(3))
            castSpell(pTarget, SPELL_AKUNDA_AKUNDAS_WRATH, true);
    }
};

class BattleOfDazaralorKragwaAI : public BattleOfDazaralorHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorKragwaAI(c); }
    explicit BattleOfDazaralorKragwaAI(Creature* pCreature) : BattleOfDazaralorHelperAI(pCreature, 1000, 1000, 4, 3000, false) {}

protected:
    void onStep(uint8_t /*step*/) override
    {
        if (Unit* pTarget = randomPlayer())
            castSpell(pTarget, SPELL_KRAGWA_KRAGWAS_WRATH, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// King Rastakhan: guards, Bwonsamdi's arrival, Bwonsamdi joins, the final stage

class BattleOfDazaralorRastakhanAI : public BattleOfDazaralorEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorRastakhanAI(c); }
    explicit BattleOfDazaralorRastakhanAI(Creature* pCreature) : BattleOfDazaralorEncounterAI(pCreature, DATA_KING_RASTAKHAN, DazaralorBerserkMs) {}

protected:
    void onAction(int32_t action) override
    {
        if (!_isInCombat())
            return;

        if (action == ACTION_DAZARALOR_GUARDS_DOWN && mStage == 1)
            beginSecondStage();
        else if (action == ACTION_RASTAKHAN_FINAL_STAGE && mStage == 3)
            beginFinalStage();
    }

    void onEngage() override
    {
        // the guards' souls shield the king
        castSpellOnSelf(SPELL_RASTAKHAN_BIND_SOULS, true);
        scriptEvents.addEvent(EVENT_RASTAKHAN_SCORCHING_DETONATION, 7000);
        scriptEvents.addEvent(EVENT_RASTAKHAN_PLAGUE_OF_TOADS, 10000);

        if (isHeroic())
        {
            scriptEvents.addEvent(EVENT_RASTAKHAN_SERPENT_TOTEM, 5000);
            callAdd(NPC_HEADHUNTER_GALWANA, GalwanaSpot);
        }

        if (findAlive(NPC_SIEGEBREAKER_ROKA) == nullptr && findAlive(NPC_PRELATE_ZALAN) == nullptr)
            beginSecondStage();
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 2 && _getHealthPercent() <= 60)
            beginThirdStage();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_RASTAKHAN_SCORCHING_DETONATION:
                if (Unit* pTank = currentVictim())
                {
                    mDetonationTarget = pTank->GetNewGUID();
                    castSpell(pTank, SPELL_RASTAKHAN_SCORCHING_DETONATION);
                }
                scriptEvents.addEvent(EVENT_RASTAKHAN_SCORCHING_DETONATION, 20000);
                break;
            case EVENT_RASTAKHAN_PLAGUE_OF_TOADS:
                castAtPlayer(SPELL_RASTAKHAN_PLAGUE_OF_TOADS);
                scriptEvents.addEvent(EVENT_RASTAKHAN_PLAGUE_OF_TOADS, 25000);
                break;
            case EVENT_RASTAKHAN_SERPENT_TOTEM:
                callAdd(NPC_GREATER_SERPENT_TOTEM, aroundSelf(10.0f, 26.0f));
                scriptEvents.addEvent(EVENT_RASTAKHAN_SERPENT_TOTEM, 18000);
                break;
            case EVENT_RASTAKHAN_ZOMBIE_DUST_TOTEM:
                callAdd(NPC_ZOMBIE_DUST_TOTEM, aroundSelf(5.0f, 14.0f));
                scriptEvents.addEvent(EVENT_RASTAKHAN_ZOMBIE_DUST_TOTEM, 40000);
                break;
            case EVENT_RASTAKHAN_PLAGUE_OF_FIRE:
                castSpellAOE(SPELL_RASTAKHAN_PLAGUE_OF_FIRE);
                scriptEvents.addEvent(EVENT_RASTAKHAN_PLAGUE_OF_FIRE, 30000);
                break;
            case EVENT_RASTAKHAN_BOON:
                castSpellOnSelf(SPELL_RASTAKHAN_BWONSAMDIS_BOON, true);
                scriptEvents.addEvent(EVENT_RASTAKHAN_BOON, 15000);
                break;
            case EVENT_RASTAKHAN_DEATHS_DOOR:
                castAtPlayer(SPELL_RASTAKHAN_DEATHS_DOOR, false, true);
                scriptEvents.addEvent(EVENT_RASTAKHAN_DEATHS_DOOR, 50000);
                break;
            case EVENT_RASTAKHAN_INEVITABLE_END:
                warn("King Rastakhan calls for the Inevitable End!");
                castSpellAOE(SPELL_RASTAKHAN_INEVITABLE_END);
                scriptEvents.addEvent(EVENT_RASTAKHAN_INEVITABLE_END, 60000);
                break;
            default:
                break;
        }
    }

    void onSpellDone(uint32_t spellId) override
    {
        if (spellId == SPELL_RASTAKHAN_SCORCHING_DETONATION)
        {
            strikeLater(findUnit(mDetonationTarget), SPELL_RASTAKHAN_SCORCHING_BLAST, 5000);
            return;
        }

        // three players carry the flames
        if (spellId == SPELL_RASTAKHAN_PLAGUE_OF_FIRE)
        {
            for (Unit* pTarget : pickPlayers(3))
                strikeLater(pTarget, SPELL_RASTAKHAN_PLAGUE_OF_FIRE_MARK, 100);
        }
    }

private:
    void beginSecondStage()
    {
        enterStage(2);
        warn("King Rastakhan calls upon Bwonsamdi!");
        _removeAura(SPELL_RASTAKHAN_BIND_SOULS);
        summonCreature(NPC_BWONSAMDI, BwonsamdiArrivalSpot, MANUAL_DESPAWN);

        scheduleCurses();
        scriptEvents.addEvent(EVENT_RASTAKHAN_ZOMBIE_DUST_TOTEM, 15000);
    }

    void beginThirdStage()
    {
        enterStage(3);
        warn("Bwonsamdi joins the battle!");
        signal(NPC_BWONSAMDI, ACTION_BWONSAMDI_MORTAL);
        castSpellOnSelf(SPELL_RASTAKHAN_ALL_ENCOMPASSING_DEATH, true);
        callAdd(NPC_PHANTOM_OF_RAGE, aroundSelf(10.0f, 30.0f));
        callAdd(NPC_PHANTOM_OF_RETRIBUTION, aroundSelf(10.0f, 30.0f));

        scheduleCurses();
        scriptEvents.addEvent(EVENT_RASTAKHAN_ZOMBIE_DUST_TOTEM, 15000);
        scriptEvents.addEvent(EVENT_RASTAKHAN_BOON, 5000);
    }

    void beginFinalStage()
    {
        enterStage(4);
        warn("King Rastakhan stands at Death's Door!");

        scheduleCurses();
        scriptEvents.addEvent(EVENT_RASTAKHAN_BOON, 5000);
        scriptEvents.addEvent(EVENT_RASTAKHAN_DEATHS_DOOR, 30000);
        scriptEvents.addEvent(EVENT_RASTAKHAN_INEVITABLE_END, 40000);
    }

    // shared by every stage after the guards
    void scheduleCurses()
    {
        scriptEvents.addEvent(EVENT_RASTAKHAN_SCORCHING_DETONATION, 7000);
        scriptEvents.addEvent(EVENT_RASTAKHAN_PLAGUE_OF_TOADS, 10000);
        scriptEvents.addEvent(EVENT_RASTAKHAN_PLAGUE_OF_FIRE, 20000);
    }

    WoWGuid mDetonationTarget;
};

// immune until the king falls below sixty percent
class BattleOfDazaralorBwonsamdiAI : public BattleOfDazaralorFightAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorBwonsamdiAI(c); }
    explicit BattleOfDazaralorBwonsamdiAI(Creature* pCreature) : BattleOfDazaralorFightAI(pCreature, true) {}

    void OnLoad() override
    {
        if (!getCreature()->isSummon())
            return;

        setBenched(true);
        mArrivalLeftMs = 12000;
    }

    void AIUpdate(unsigned long time_passed) override
    {
        if (mArrivalLeftMs != 0)
            tickArrival(static_cast<uint32_t>(time_passed));

        BattleOfDazaralorFightAI::AIUpdate(time_passed);
    }

    void DamageTaken(Unit* /*pAttacker*/, uint32_t* damage) override
    {
        if (mUnliving && damage != nullptr)
            *damage = 0;
    }

    void DoAction(int32_t action) override
    {
        if (action != ACTION_BWONSAMDI_MORTAL || !mUnliving)
            return;

        mUnliving = false;
        _removeAura(SPELL_BWONSAMDI_UNLIVING);
        if (Creature* pKing = findAlive(BOSS_KING_RASTAKHAN))
            castSpell(pKing, SPELL_BWONSAMDI_BOON_GIFT, true);
    }

protected:
    void onEngage() override
    {
        mFinalCalled = false;
        scriptEvents.addEvent(EVENT_BWONSAMDI_CARESS_OF_DEATH, 7000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mUnliving || mFinalCalled || _getHealthPercent() > 50)
            return;

        mFinalCalled = true;
        signal(BOSS_KING_RASTAKHAN, ACTION_RASTAKHAN_FINAL_STAGE);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_BWONSAMDI_CARESS_OF_DEATH)
            return;

        castSpellOnVictim(SPELL_BWONSAMDI_CARESS_OF_DEATH);
        scriptEvents.addEvent(EVENT_BWONSAMDI_CARESS_OF_DEATH, 20000);
    }

private:
    // steps down to the platform after the summoning
    void tickArrival(uint32_t diff)
    {
        if (mArrivalLeftMs > diff)
        {
            mArrivalLeftMs -= diff;
            return;
        }

        mArrivalLeftMs = 0;
        MoveTeleport(BwonsamdiPlatformSpot);
        setBenched(false);
        castSpellOnSelf(SPELL_BWONSAMDI_UNLIVING, true);
        setZoneWideCombat();
    }

    uint32_t mArrivalLeftMs = 0;
    bool mUnliving = true;
    bool mFinalCalled = false;
};

class BattleOfDazaralorRokaAI : public BattleOfDazaralorEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorRokaAI(c); }
    explicit BattleOfDazaralorRokaAI(Creature* pCreature) : BattleOfDazaralorEncounterAI(pCreature, DATA_KING_RASTAKHAN, 0) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_ROKA_METEOR_LEAP, 5000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_ROKA_METEOR_LEAP)
            return;

        castAtPlayer(SPELL_ROKA_METEOR_LEAP, true);
        scriptEvents.addEvent(EVENT_ROKA_METEOR_LEAP, 25000);
    }

    // back to the tank after the meteor
    void onSpellDone(uint32_t spellId) override
    {
        if (spellId == SPELL_ROKA_METEOR_LEAP)
            strikeLater(currentVictim(), SPELL_ROKA_CRUSHING_LEAP, 100);
    }
};

class BattleOfDazaralorZalanAI : public BattleOfDazaralorEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorZalanAI(c); }
    explicit BattleOfDazaralorZalanAI(Creature* pCreature) : BattleOfDazaralorEncounterAI(pCreature, DATA_KING_RASTAKHAN, 0) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_ZALAN_SEAL_OF_PURIFICATION, 5000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_ZALAN_SEAL_OF_PURIFICATION)
            return;

        castSpellAOE(SPELL_ZALAN_SEAL_OF_PURIFICATION, true);
        scriptEvents.addEvent(EVENT_ZALAN_SEAL_OF_PURIFICATION, 30000);
    }
};

class BattleOfDazaralorGalwanaAI : public BattleOfDazaralorFightAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorGalwanaAI(c); }
    explicit BattleOfDazaralorGalwanaAI(Creature* pCreature) : BattleOfDazaralorFightAI(pCreature, true) {}

protected:
    void onEngage() override { scriptEvents.addEvent(EVENT_GALWANA_GRIEVOUS_AXE, 5000); }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_GALWANA_GRIEVOUS_AXE)
            return;

        castAtPlayer(SPELL_GALWANA_GRIEVOUS_AXE);
        scriptEvents.addEvent(EVENT_GALWANA_GRIEVOUS_AXE, Util::getRandomUInt(15000, 20000));
    }
};

// phantoms of the death realm, one signature attack each
class BattleOfDazaralorPhantomAI : public BattleOfDazaralorFightAI
{
public:
    BattleOfDazaralorPhantomAI(Creature* pCreature, uint32_t spellId, bool onVictim)
        : BattleOfDazaralorFightAI(pCreature, true), mSpellId(spellId), mOnVictim(onVictim) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_PHANTOM_UNDYING_RELENTLESSNESS, true);
        scriptEvents.addEvent(EVENT_PHANTOM_STRIKE, 5000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_PHANTOM_STRIKE)
            return;

        if (mOnVictim)
            castSpellOnVictim(mSpellId);
        else
            castAtPlayer(mSpellId);

        scriptEvents.addEvent(EVENT_PHANTOM_STRIKE, Util::getRandomUInt(15000, 20000));
    }

private:
    uint32_t mSpellId;
    bool mOnVictim;
};

class BattleOfDazaralorPhantomOfRageAI : public BattleOfDazaralorPhantomAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorPhantomOfRageAI(c); }
    explicit BattleOfDazaralorPhantomOfRageAI(Creature* pCreature) : BattleOfDazaralorPhantomAI(pCreature, SPELL_PHANTOM_NECROTIC_SMASH, true) {}
};

class BattleOfDazaralorPhantomOfRetributionAI : public BattleOfDazaralorPhantomAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorPhantomOfRetributionAI(c); }
    explicit BattleOfDazaralorPhantomOfRetributionAI(Creature* pCreature) : BattleOfDazaralorPhantomAI(pCreature, SPELL_PHANTOM_GRAVE_BOLT, false) {}
};

// rooted totem with one periodic spell
class BattleOfDazaralorTotemAI : public BattleOfDazaralorFightAI
{
public:
    BattleOfDazaralorTotemAI(Creature* pCreature, uint32_t spellId) : BattleOfDazaralorFightAI(pCreature, false), mSpellId(spellId) {}

protected:
    void onEngage() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
        scriptEvents.addEvent(EVENT_TOTEM_PULSE, 5000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_TOTEM_PULSE)
            return;

        castAtPlayer(mSpellId);
        scriptEvents.addEvent(EVENT_TOTEM_PULSE, Util::getRandomUInt(20000, 25000));
    }

private:
    uint32_t mSpellId;
};

class BattleOfDazaralorZombieDustTotemAI : public BattleOfDazaralorTotemAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorZombieDustTotemAI(c); }
    explicit BattleOfDazaralorZombieDustTotemAI(Creature* pCreature) : BattleOfDazaralorTotemAI(pCreature, SPELL_TOTEM_ZOMBIE_DUST) {}
};

class BattleOfDazaralorSerpentTotemAI : public BattleOfDazaralorTotemAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorSerpentTotemAI(c); }
    explicit BattleOfDazaralorSerpentTotemAI(Creature* pCreature) : BattleOfDazaralorTotemAI(pCreature, SPELL_TOTEM_SERPENTS_BREATH) {}
};

//////////////////////////////////////////////////////////////////////////////////////////
// High Tinker Mekkatorque

class BattleOfDazaralorMekkatorqueAI : public BattleOfDazaralorEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorMekkatorqueAI(c); }
    explicit BattleOfDazaralorMekkatorqueAI(Creature* pCreature) : BattleOfDazaralorEncounterAI(pCreature, DATA_MEKKATORQUE, DazaralorBerserkMs) {}

protected:
    void onEngage() override
    {
        mSheepCalled = false;
        castSpellOnSelf(SPELL_MEKKATORQUE_ELECTROSHOCK_STRIKES, true);

        scriptEvents.addEvent(EVENT_MEKKATORQUE_BUSTER_CANNON, 13000);
        scriptEvents.addEvent(EVENT_MEKKATORQUE_GIGAVOLT_CHARGE, 21000);
        scriptEvents.addEvent(EVENT_MEKKATORQUE_SPARK_BOTS, 31000);
        scriptEvents.addEvent(EVENT_MEKKATORQUE_BLAST_OFF, 40000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mSheepCalled || _getHealthPercent() > 40)
            return;

        // second stage: sheep at once, hyperdrive later
        mSheepCalled = true;
        scriptEvents.addEvent(EVENT_MEKKATORQUE_EXPLODING_SHEEP, 3000);
        scriptEvents.addEvent(EVENT_MEKKATORQUE_HYPERDRIVE, 66000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MEKKATORQUE_BUSTER_CANNON:
                if (Unit* pTarget = castAtPlayer(SPELL_MEKKATORQUE_BUSTER_CANNON))
                    mCannonTarget = pTarget->GetNewGUID();
                scriptEvents.addEvent(EVENT_MEKKATORQUE_BUSTER_CANNON, 20000);
                break;
            case EVENT_MEKKATORQUE_GIGAVOLT_CHARGE:
                for (Unit* pTarget : pickPlayers(isHeroic() ? 3 : 2))
                    castSpell(pTarget, SPELL_MEKKATORQUE_GIGAVOLT_CHARGE, true);
                scriptEvents.addEvent(EVENT_MEKKATORQUE_GIGAVOLT_CHARGE, 40000);
                break;
            case EVENT_MEKKATORQUE_SPARK_BOTS:
                for (LocationVector const& spot : MekkatorqueClawSpots)
                {
                    placeHelper(NPC_GNOMISH_SUPPORT_CLAW, spot, 3000);
                    callAdd(NPC_SPARK_BOT, spot);
                }
                scriptEvents.addEvent(EVENT_MEKKATORQUE_SPARK_BOTS, 60000);
                break;
            case EVENT_MEKKATORQUE_BLAST_OFF:
                warn("High Tinker Mekkatorque blasts off!");
                castSpellAOE(SPELL_MEKKATORQUE_BLAST_OFF);
                scriptEvents.addEvent(EVENT_MEKKATORQUE_BLAST_OFF, 30000);
                break;
            case EVENT_MEKKATORQUE_EXPLODING_SHEEP:
                warn("High Tinker Mekkatorque signals the Explosive Sheep!");
                castSpellOnSelf(SPELL_MEKKATORQUE_SIGNAL_SHEEP, true);
                for (uint8_t i = 0; i < 7; ++i)
                    placeHelper(NPC_EXPLOSIVE_SHEEP, aroundSelf(5.0f, 30.0f), 15000);
                break;
            case EVENT_MEKKATORQUE_HYPERDRIVE:
                castSpellOnSelf(SPELL_MEKKATORQUE_HYPERDRIVE, true);
                break;
            default:
                break;
        }
    }

    void onSpellDone(uint32_t spellId) override
    {
        if (spellId == SPELL_MEKKATORQUE_BUSTER_CANNON)
            strikeLater(findUnit(mCannonTarget), SPELL_MEKKATORQUE_BUSTER_CANNON_BLAST, 100);
        else if (spellId == SPELL_MEKKATORQUE_BLAST_OFF)
            blastLater(SPELL_MEKKATORQUE_CRASH_DOWN, 3600);
    }

private:
    WoWGuid mCannonTarget;
    bool mSheepCalled = false;
};

class BattleOfDazaralorSparkBotAI : public BattleOfDazaralorFightAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorSparkBotAI(c); }
    explicit BattleOfDazaralorSparkBotAI(Creature* pCreature) : BattleOfDazaralorFightAI(pCreature, false) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_SPARK_BOT_FORCE_SHIELD, true);
        scriptEvents.addEvent(EVENT_SPARK_BOT_PULSE, 3000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_SPARK_BOT_PULSE)
            return;

        castSpellAOE(SPELL_SPARK_BOT_SPARK_PULSE);
        scriptEvents.addEvent(EVENT_SPARK_BOT_PULSE, 15000);
    }
};

// only carries the spark bot in
class BattleOfDazaralorSupportClawAI : public BattleOfDazaralorHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorSupportClawAI(c); }
    explicit BattleOfDazaralorSupportClawAI(Creature* pCreature) : BattleOfDazaralorHelperAI(pCreature, 0, 0, 0, 0, false) {}
};

class BattleOfDazaralorExplosiveSheepAI : public BattleOfDazaralorHelperAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorExplosiveSheepAI(c); }
    explicit BattleOfDazaralorExplosiveSheepAI(Creature* pCreature) : BattleOfDazaralorHelperAI(pCreature, 500, 0, 1, 10000, true) {}

protected:
    void onStep(uint8_t /*step*/) override { castSpellAOE(SPELL_SHEEP_CRITTER_EXPLOSION); }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Stormwall Blockade: Sister Katherine and Brother Joseph, then Laminaria

class BattleOfDazaralorKatherineAI : public BattleOfDazaralorEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorKatherineAI(c); }
    explicit BattleOfDazaralorKatherineAI(Creature* pCreature) : BattleOfDazaralorEncounterAI(pCreature, DATA_STORMWALL_BLOCKADE, DazaralorBerserkMs) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_KATHERINE_VOLTAIC_FLASH, 8000);
        scriptEvents.addEvent(EVENT_KATHERINE_CRACKLING_LIGHTNING, 12000);
        scriptEvents.addEvent(EVENT_KATHERINE_JOLTING_VOLLEY, 20000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_KATHERINE_ELECTRIC_SHROUD, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_KATHERINE_VOLTAIC_FLASH:
                castAtPlayer(SPELL_KATHERINE_VOLTAIC_FLASH);
                scriptEvents.addEvent(EVENT_KATHERINE_VOLTAIC_FLASH, 16000);
                break;
            case EVENT_KATHERINE_CRACKLING_LIGHTNING:
                castAtPlayer(SPELL_KATHERINE_CRACKLING_LIGHTNING, true);
                scriptEvents.addEvent(EVENT_KATHERINE_CRACKLING_LIGHTNING, 18000);
                break;
            case EVENT_KATHERINE_JOLTING_VOLLEY:
                castSpellAOE(SPELL_KATHERINE_JOLTING_VOLLEY);
                scriptEvents.addEvent(EVENT_KATHERINE_JOLTING_VOLLEY, 25000);
                break;
            case EVENT_KATHERINE_ELECTRIC_SHROUD:
                castSpellOnSelf(SPELL_KATHERINE_ELECTRIC_SHROUD, true);
                scriptEvents.addEvent(EVENT_KATHERINE_ELECTRIC_SHROUD, 45000);
                break;
            default:
                break;
        }
    }
};

class BattleOfDazaralorJosephAI : public BattleOfDazaralorEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorJosephAI(c); }
    explicit BattleOfDazaralorJosephAI(Creature* pCreature) : BattleOfDazaralorEncounterAI(pCreature, DATA_STORMWALL_BLOCKADE, DazaralorBerserkMs) {}

protected:
    void onEngage() override
    {
        scriptEvents.addEvent(EVENT_JOSEPH_SEA_SWELL, 10000);
        scriptEvents.addEvent(EVENT_JOSEPH_TIDAL_VOLLEY, 15000);
        scriptEvents.addEvent(EVENT_JOSEPH_TEMPTING_SONG, 30000);
        if (isHeroic())
            scriptEvents.addEvent(EVENT_JOSEPH_TIDAL_SHROUD, 30000);
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_JOSEPH_SEA_SWELL:
                castAtPlayer(SPELL_JOSEPH_SEA_SWELL);
                scriptEvents.addEvent(EVENT_JOSEPH_SEA_SWELL, 20000);
                break;
            case EVENT_JOSEPH_TIDAL_VOLLEY:
                castSpellAOE(SPELL_JOSEPH_TIDAL_VOLLEY);
                scriptEvents.addEvent(EVENT_JOSEPH_TIDAL_VOLLEY, 25000);
                break;
            case EVENT_JOSEPH_TEMPTING_SONG:
                warn("A Tempting Siren begins to sing!");
                castSpellAOE(SPELL_JOSEPH_TEMPTING_SONG);
                callAdd(NPC_TEMPTING_SIREN, aroundSelf(10.0f, 20.0f));
                scriptEvents.addEvent(EVENT_JOSEPH_TEMPTING_SONG, 40000);
                break;
            case EVENT_JOSEPH_TIDAL_SHROUD:
                castSpellOnSelf(SPELL_JOSEPH_TIDAL_SHROUD, true);
                scriptEvents.addEvent(EVENT_JOSEPH_TIDAL_SHROUD, 45000);
                break;
            default:
                break;
        }
    }
};

class BattleOfDazaralorLaminariaAI : public BattleOfDazaralorEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorLaminariaAI(c); }
    explicit BattleOfDazaralorLaminariaAI(Creature* pCreature) : BattleOfDazaralorEncounterAI(pCreature, DATA_STORMWALL_BLOCKADE, DazaralorBerserkMs) {}

protected:
    void onJoinFight() override { warn("Laminaria rises from the depths!"); }

    void onEngage() override
    {
        castSpellOnSelf(SPELL_LAMINARIA_KELP_WRAPPED_FISTS, true);
        scriptEvents.addEvent(EVENT_LAMINARIA_STORMS_WAIL, 15000);
    }

    void onEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_LAMINARIA_STORMS_WAIL)
            return;

        castAtPlayer(SPELL_LAMINARIA_STORMS_WAIL, true);
        scriptEvents.addEvent(EVENT_LAMINARIA_STORMS_WAIL, 30000);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Lady Jaina Proudmoore: three stages split at sixty and thirty percent

class BattleOfDazaralorJainaAI : public BattleOfDazaralorEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BattleOfDazaralorJainaAI(c); }
    explicit BattleOfDazaralorJainaAI(Creature* pCreature) : BattleOfDazaralorEncounterAI(pCreature, DATA_LADY_JAINA_PROUDMOORE, JainaBerserkMs) {}

protected:
    void onEngage() override
    {
        castSpellOnSelf(SPELL_JAINA_CHILLING_TOUCH, true);

        scriptEvents.addEvent(EVENT_JAINA_ICE_SHARD, 6000);
        scriptEvents.addEvent(EVENT_JAINA_FREEZING_BLAST, 10000);
        scriptEvents.addEvent(EVENT_JAINA_GRASP_OF_FROST, 17000);
        scriptEvents.addEvent(EVENT_JAINA_AVALANCHE, 20000);
        scriptEvents.addEvent(EVENT_JAINA_CORSAIR, 25000);
        scriptEvents.addEvent(EVENT_JAINA_RING_OF_ICE, 40000);
    }

    void onCombatTick(uint32_t /*diff*/) override
    {
        if (mStage == 1 && _getHealthPercent() <= 60)
            beginSecondStage();
        else if (mStage == 2 && _getHealthPercent() <= 30)
            beginThirdStage();
    }

    void onEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_JAINA_ICE_SHARD:
                castSpellOnVictim(SPELL_JAINA_ICE_SHARD);
                scriptEvents.addEvent(EVENT_JAINA_ICE_SHARD, 12000);
                break;
            case EVENT_JAINA_FREEZING_BLAST:
                castAtPlayer(SPELL_JAINA_FREEZING_BLAST);
                scriptEvents.addEvent(EVENT_JAINA_FREEZING_BLAST, 15000);
                break;
            case EVENT_JAINA_GRASP_OF_FROST:
                castSpellAOE(SPELL_JAINA_GRASP_OF_FROST);
                scriptEvents.addEvent(EVENT_JAINA_GRASP_OF_FROST, 25000);
                break;
            case EVENT_JAINA_AVALANCHE:
                // marked player, the snow lands a few seconds later
                if (Unit* pTarget = castAtPlayer(SPELL_JAINA_AVALANCHE, false, true))
                    strikeLater(pTarget, SPELL_JAINA_AVALANCHE_IMPACT, 3000);
                scriptEvents.addEvent(EVENT_JAINA_AVALANCHE, 30000);
                break;
            case EVENT_JAINA_CORSAIR:
                callAdd(NPC_KUL_TIRAN_CORSAIR, aroundSelf(15.0f, 25.0f));
                scriptEvents.addEvent(EVENT_JAINA_CORSAIR, 45000);
                break;
            case EVENT_JAINA_RING_OF_ICE:
                warn("Lady Jaina Proudmoore begins to cast Ring of Ice!");
                castSpellAOE(SPELL_JAINA_RING_OF_ICE);
                scriptEvents.addEvent(EVENT_JAINA_RING_OF_ICE, 60000);
                break;
            case EVENT_JAINA_GLACIAL_RAY:
                warn("Lady Jaina Proudmoore channels a Glacial Ray!");
                castSpellAOE(SPELL_JAINA_GLACIAL_RAY);
                scriptEvents.addEvent(EVENT_JAINA_GLACIAL_RAY, 40000);
                break;
            case EVENT_JAINA_HAND_OF_FROST:
                castAtPlayer(SPELL_JAINA_HAND_OF_FROST, true);
                scriptEvents.addEvent(EVENT_JAINA_HAND_OF_FROST, 20000);
                break;
            case EVENT_JAINA_CRYSTALLINE_DUST:
                castSpellOnVictim(SPELL_JAINA_CRYSTALLINE_DUST);
                scriptEvents.addEvent(EVENT_JAINA_CRYSTALLINE_DUST, 25000);
                break;
            default:
                break;
        }
    }

private:
    void beginSecondStage()
    {
        enterStage(2);
        castSpellOnSelf(SPELL_JAINA_HOWLING_WINDS, true);

        scriptEvents.addEvent(EVENT_JAINA_ICE_SHARD, 6000);
        scriptEvents.addEvent(EVENT_JAINA_FREEZING_BLAST, 10000);
        scriptEvents.addEvent(EVENT_JAINA_GLACIAL_RAY, 18000);
        scriptEvents.addEvent(EVENT_JAINA_GRASP_OF_FROST, 25000);
        scriptEvents.addEvent(EVENT_JAINA_AVALANCHE, 30000);
    }

    void beginThirdStage()
    {
        enterStage(3);
        for (Unit* pTarget : pickPlayers(3))
            castSpell(pTarget, SPELL_JAINA_ARCANE_BARRAGE, true);

        scriptEvents.addEvent(EVENT_JAINA_ICE_SHARD, 6000);
        scriptEvents.addEvent(EVENT_JAINA_HAND_OF_FROST, 10000);
        scriptEvents.addEvent(EVENT_JAINA_CRYSTALLINE_DUST, 15000);
        scriptEvents.addEvent(EVENT_JAINA_GLACIAL_RAY, 25000);
        scriptEvents.addEvent(EVENT_JAINA_RING_OF_ICE, 35000);
    }
};

void SetupBattleOfDazarAlor(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BATTLE_OF_DAZARALOR, &BattleOfDazaralorInstanceScript::Create);

    // Champion of the Light
    mgr->register_creature_script(BOSS_FRIDA_IRONBELLOWS, &BattleOfDazaralorFridaAI::Create);
    mgr->register_creature_script(BOSS_RAWANI_KANAE, &BattleOfDazaralorRawaniAI::Create);
    mgr->register_creature_script(NPC_DARKFORGED_CRUSADER, &BattleOfDazaralorCrusaderAI::Create);
    mgr->register_creature_script(NPC_ZANDALARI_CRUSADER, &BattleOfDazaralorCrusaderAI::Create);
    mgr->register_creature_script(NPC_ANOINTED_DISCIPLE, &BattleOfDazaralorDiscipleAI::Create);
    mgr->register_creature_script(NPC_REZANI_DISCIPLE, &BattleOfDazaralorDiscipleAI::Create);

    // Grong and Grong the Revenant
    mgr->register_creature_script(BOSS_GRONG, &BattleOfDazaralorGrongAI::Create);
    mgr->register_creature_script(NPC_APETAGONIZER_3000, &BattleOfDazaralorApetagonizerAI::Create);
    mgr->register_creature_script(BOSS_GRONG_THE_REVENANT, &BattleOfDazaralorGrongRevenantAI::Create);
    mgr->register_creature_script(NPC_DEATH_SPECTER, &BattleOfDazaralorDeathSpecterAI::Create);

    // Jadefire Masters, both sides
    mgr->register_creature_script(BOSS_MANCEROY_FLAMEFIST, &BattleOfDazaralorManceroyAI::Create);
    mgr->register_creature_script(BOSS_MESTRAH, &BattleOfDazaralorMestrahAI::Create);
    mgr->register_creature_script(BOSS_ANATHOS_FIRECALLER, &BattleOfDazaralorAnathosAI::Create);
    mgr->register_creature_script(BOSS_MARA_GRIMFANG, &BattleOfDazaralorMaraAI::Create);
    mgr->register_creature_script(NPC_LIVING_BOMB, &BattleOfDazaralorLivingBombAI::Create);
    mgr->register_creature_script(NPC_SPIRIT_OF_XUEN, &BattleOfDazaralorSpiritOfXuenAI::Create);

    // Opulence
    mgr->register_creature_script(BOSS_OPULENCE, &BattleOfDazaralorOpulenceAI::Create);

    // Conclave of the Chosen
    mgr->register_creature_script(BOSS_PAKUS_ASPECT, &BattleOfDazaralorPakuAI::Create);
    mgr->register_creature_script(BOSS_GONKS_ASPECT, &BattleOfDazaralorGonkAI::Create);
    mgr->register_creature_script(BOSS_KIMBULS_ASPECT, &BattleOfDazaralorKimbulAI::Create);
    mgr->register_creature_script(BOSS_AKUNDAS_ASPECT, &BattleOfDazaralorAkundaAI::Create);
    mgr->register_creature_script(NPC_PAKU, &BattleOfDazaralorPakuWrathAI::Create);
    mgr->register_creature_script(NPC_GONK, &BattleOfDazaralorGonkWrathAI::Create);
    mgr->register_creature_script(NPC_KIMBUL, &BattleOfDazaralorKimbulWrathAI::Create);
    mgr->register_creature_script(NPC_AKUNDA, &BattleOfDazaralorAkundaWrathAI::Create);
    mgr->register_creature_script(NPC_KRAGWA, &BattleOfDazaralorKragwaAI::Create);

    // King Rastakhan
    mgr->register_creature_script(BOSS_KING_RASTAKHAN, &BattleOfDazaralorRastakhanAI::Create);
    mgr->register_creature_script(NPC_SIEGEBREAKER_ROKA, &BattleOfDazaralorRokaAI::Create);
    mgr->register_creature_script(NPC_PRELATE_ZALAN, &BattleOfDazaralorZalanAI::Create);
    mgr->register_creature_script(NPC_HEADHUNTER_GALWANA, &BattleOfDazaralorGalwanaAI::Create);
    mgr->register_creature_script(NPC_BWONSAMDI, &BattleOfDazaralorBwonsamdiAI::Create);
    mgr->register_creature_script(NPC_PHANTOM_OF_RAGE, &BattleOfDazaralorPhantomOfRageAI::Create);
    mgr->register_creature_script(NPC_PHANTOM_OF_RETRIBUTION, &BattleOfDazaralorPhantomOfRetributionAI::Create);
    mgr->register_creature_script(NPC_ZOMBIE_DUST_TOTEM, &BattleOfDazaralorZombieDustTotemAI::Create);
    mgr->register_creature_script(NPC_GREATER_SERPENT_TOTEM, &BattleOfDazaralorSerpentTotemAI::Create);

    // High Tinker Mekkatorque
    mgr->register_creature_script(BOSS_HIGH_TINKER_MEKKATORQUE, &BattleOfDazaralorMekkatorqueAI::Create);
    mgr->register_creature_script(NPC_SPARK_BOT, &BattleOfDazaralorSparkBotAI::Create);
    mgr->register_creature_script(NPC_GNOMISH_SUPPORT_CLAW, &BattleOfDazaralorSupportClawAI::Create);
    mgr->register_creature_script(NPC_EXPLOSIVE_SHEEP, &BattleOfDazaralorExplosiveSheepAI::Create);

    // Stormwall Blockade
    mgr->register_creature_script(BOSS_SISTER_KATHERINE, &BattleOfDazaralorKatherineAI::Create);
    mgr->register_creature_script(BOSS_BROTHER_JOSEPH, &BattleOfDazaralorJosephAI::Create);
    mgr->register_creature_script(BOSS_LAMINARIA, &BattleOfDazaralorLaminariaAI::Create);

    // Lady Jaina Proudmoore
    mgr->register_creature_script(BOSS_LADY_JAINA_PROUDMOORE, &BattleOfDazaralorJainaAI::Create);
}
