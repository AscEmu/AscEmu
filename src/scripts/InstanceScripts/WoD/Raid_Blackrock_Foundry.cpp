/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Blackrock_Foundry.hpp"

#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

#include <map>
#include <vector>

enum BlackrockFoundryEvents
{
    EVENT_FOUNDRY_BERSERK = 1,

    EVENT_GRUUL_INFERNO_SLICE,
    EVENT_GRUUL_CAVE_IN,
    EVENT_GRUUL_PETRIFYING_SLAM,
    EVENT_GRUUL_OVERHEAD_SMASH,
    EVENT_GRUUL_RAMPAGE_START,
    EVENT_GRUUL_RAMPAGE_SMASH,
    EVENT_GRUUL_RAMPAGE_END,

    EVENT_OREGORGER_ACID_MAW,
    EVENT_OREGORGER_RETCHED_BLACKROCK,
    EVENT_OREGORGER_EXPLOSIVE_SHARD,
    EVENT_OREGORGER_ACID_TORRENT,
    EVENT_OREGORGER_BLACKROCK_SPINES,
    EVENT_OREGORGER_BLACKROCK_BARRAGE,
    EVENT_OREGORGER_ROLLING_START,
    EVENT_OREGORGER_ROLL_ON,
    EVENT_OREGORGER_ROLLING_END,
    EVENT_OREGORGER_HUNGER_ENRAGE,
    EVENT_OREGORGER_STOMP,

    EVENT_DARMAC_CALL_THE_PACK,
    EVENT_DARMAC_PIN_DOWN,
    EVENT_DARMAC_TANTRUM,
    EVENT_DARMAC_REND_AND_TEAR,
    EVENT_DARMAC_SUPERHEATED_SHRAPNEL,

    EVENT_KAGRAZ_LAVA_SLASH,
    EVENT_KAGRAZ_MOLTEN_TORRENT,
    EVENT_KAGRAZ_CINDER_WOLVES,
    EVENT_KAGRAZ_BLAZING_RADIANCE,
    EVENT_KAGRAZ_FIRESTORM,
    EVENT_KAGRAZ_FIRESTORM_END,

    EVENT_ANVIL_JUMP_SLAM,
    EVENT_ANVIL_DISRUPTING_ROAR,
    EVENT_ANVIL_SKULLCRACKER,
    EVENT_ANVIL_LANDING_SLAM,

    EVENT_THOGAR_ENKINDLE,
    EVENT_THOGAR_PULSE_GRENADE,
    EVENT_THOGAR_REINFORCEMENTS,

    EVENT_FURNACE_PYROCLASM,
    EVENT_FURNACE_RUPTURE,
    EVENT_FURNACE_BLAST,
    EVENT_FURNACE_CREW_WAVE,
    EVENT_FURNACE_CONTAINMENT_FAILSAFE,
    EVENT_FURNACE_SLAG_ELEMENTAL,
    EVENT_FURNACE_FIRECALLERS,
    EVENT_FURNACE_FLOOR_GUARD,
    EVENT_FURNACE_HEAT,
    EVENT_FURNACE_MELT,

    EVENT_KROMOG_STONE_BREATH,
    EVENT_KROMOG_WARPED_ARMOR,
    EVENT_KROMOG_SLAM,
    EVENT_KROMOG_RIPPLING_SMASH,
    EVENT_KROMOG_GRASPING_EARTH,
    EVENT_KROMOG_THUNDERING_BLOWS,

    EVENT_MAIDEN_DREADNAUGHT,
    EVENT_MAIDEN_BOMBARD,
    EVENT_MAIDEN_LEAVE_SHIP,
    EVENT_MAIDEN_SIGNATURE,
    EVENT_MAIDEN_FURY_FIRST,
    EVENT_MAIDEN_FURY_SECOND,

    EVENT_BLACKHAND_SLAG_BOMBS,
    EVENT_BLACKHAND_DEMOLITION,
    EVENT_BLACKHAND_IMPALING_THROW,
    EVENT_BLACKHAND_SHATTERING_SMASH,
    EVENT_BLACKHAND_SIEGEMAKER,
    EVENT_BLACKHAND_IRON_SOLDIERS,
    EVENT_BLACKHAND_SLAG_ERUPTION
};

enum BlackrockFoundryActions
{
    ACTION_DARMAC_UNLEASH_BEAST = 1,
    ACTION_ANVIL_RETREAT,
    ACTION_ANVIL_RETURN,
    ACTION_FURNACE_REGULATOR_LOST,
    ACTION_FURNACE_ELEMENTALIST_LOST,
    ACTION_MAIDEN_BOARD_SHIP,
    ACTION_MAIDEN_IRON_WILL
};

enum BlackrockFoundryCreatureData
{
    FOUNDRY_DATA_BEAST_SLAIN = 1,
    FOUNDRY_DATA_SHIP_VISITED
};

// scriptEvents phase ids
uint32_t const FOUNDRY_STAGE_ONE = 1;
uint32_t const FOUNDRY_STAGE_TWO = 2;
uint32_t const FOUNDRY_STAGE_THREE = 3;

//////////////////////////////////////////////////////////////////////////////////////////
// Positions

LocationVector const FoundryGruulRoomCenter = { 373.618f, 3583.1f, 279.755f, 4.60341f };

LocationVector const FoundryOregorgerRollRoute[] =
{
    { 16.221f, 3537.050f, 280.977f, 0.0f },
    { -11.190f, 3585.790f, 280.977f, 0.0f },
    { 37.201f, 3629.070f, 281.013f, 0.0f },
    { 80.977f, 3579.910f, 280.977f, 0.0f }
};
uint32_t const FoundryOregorgerRollRouteSize = sizeof(FoundryOregorgerRollRoute) / sizeof(FoundryOregorgerRollRoute[0]);

LocationVector const FoundryCinderWolfSpawn = { 131.1977f, 3788.002f, 101.8597f, 0.0f };

LocationVector const FoundryHansgarArenaPos = { 103.434f, 3517.066f, 130.1161f, 0.0f };
LocationVector const FoundryHansgarRetreatPos = { 85.60069f, 3517.861f, 138.235f, 3.141593f };
LocationVector const FoundryFranzokArenaPos = { 102.6545f, 3465.035f, 130.2772f, 0.0f };
LocationVector const FoundryFranzokRetreatPos = { 88.03299f, 3467.769f, 138.4438f, 3.141593f };

LocationVector const FoundryThogarExits[2][6] =
{
    {
        { 493.975f, 3355.412f, 305.8985f, 1.570796f },
        { 500.975f, 3355.412f, 305.9013f, 1.570796f },
        { 493.975f, 3350.412f, 305.9027f, 1.570796f },
        { 500.975f, 3350.412f, 305.9000f, 1.570796f },
        { 493.975f, 3344.412f, 305.9012f, 1.570796f },
        { 500.975f, 3344.412f, 305.8985f, 1.570796f }
    },
    {
        { 493.975f, 3287.412f, 305.8896f, 1.570796f },
        { 500.975f, 3287.412f, 305.8854f, 1.570796f },
        { 493.975f, 3282.412f, 305.8895f, 1.570796f },
        { 500.975f, 3282.412f, 305.8854f, 1.570796f },
        { 493.975f, 3276.412f, 305.8917f, 1.570796f },
        { 500.975f, 3276.412f, 305.8898f, 1.570796f }
    }
};
LocationVector const FoundryThogarManAtArmsExit = { 482.475f, 3312.533f, 302.3777f, 1.570796f };

LocationVector const FoundryFurnaceCrewSpawns[2][3] =
{
    {
        { 127.299f, 3496.977f, 246.085f, 0.7516f },
        { 127.599f, 3504.188f, 246.085f, 0.0054f },
        { 130.231f, 3515.183f, 245.085f, 0.0368f }
    },
    {
        { 265.278f, 3515.901f, 246.551f, 2.2870f },
        { 265.739f, 3520.793f, 246.551f, 3.0881f },
        { 265.634f, 3527.234f, 246.551f, 3.2569f }
    }
};
LocationVector const FoundryBellowsOperatorSpawns[2] =
{
    { 272.4757f, 3591.769f, 246.4359f, 3.071f },
    { 123.9306f, 3592.554f, 246.4359f, 0.106f }
};
LocationVector const FoundryElementalistPositions[4] =
{
    { 181.700f, 3542.40f, 217.517f, 0.0f },
    { 181.884f, 3519.69f, 217.381f, 0.0f },
    { 211.403f, 3516.13f, 217.492f, 0.0f },
    { 217.771f, 3546.35f, 217.408f, 0.0f }
};
LocationVector const FoundryFurnaceFloorGuardPos = { 197.372f, 3498.752f, 217.844f, 1.552f };

LocationVector const FoundryDreadnaughtZipline = { 451.2829f, 3175.634f, 135.4688f, 0.0f };

// spawn offsets around the summoner
struct FoundryOffset
{
    float x;
    float y;
};

FoundryOffset const FoundryRingOffsets[] =
{
    { 20.0f, 0.0f }, { -20.0f, 0.0f }, { 0.0f, 20.0f }, { 0.0f, -20.0f },
    { 14.0f, 14.0f }, { -14.0f, -14.0f }, { 14.0f, -14.0f }, { -14.0f, 14.0f }
};
uint32_t const FoundryRingOffsetCount = sizeof(FoundryRingOffsets) / sizeof(FoundryRingOffsets[0]);

//////////////////////////////////////////////////////////////////////////////////////////
// Encounter tables

struct FoundryExitDoor
{
    uint32_t entry;
    uint32_t dataId;
};

FoundryExitDoor const FoundryExitDoors[] =
{
    { GO_FURNACE_PORTCULLIS,         DATA_OREGORGER },
    { GO_TERMINUS_DOOR,              DATA_BEASTLORD_DARMAC },
    { GO_BURNING_FRONT_DOOR,         DATA_FLAMEBENDER_KAGRAZ },
    { GO_HANSGAR_AND_FRANZOK_EXIT,   DATA_HANSGAR_AND_FRANZOK },
    { GO_FREIGHT_ELEVATOR_DOOR,      DATA_OPERATOR_THOGAR },
    { GO_KROMOGS_DOOR_EAST,          DATA_KROMOG },
    { GO_BLACK_FORGE_PORTCULLIS,     DATA_KROMOG },
    { GO_BLACK_FORGE_GATE,           DATA_KROMOG },
    { GO_KROMOG_DOOR,                DATA_KROMOG }
};

// creatures sharing one encounter slot; all of them are revived after a wipe
struct FoundryGroupMember
{
    uint32_t dataId;
    uint32_t entry;
    bool requiredForKill;
    bool joinsOnPull;
};

FoundryGroupMember const FoundryGroupMembers[] =
{
    { DATA_BEASTLORD_DARMAC,     NPC_CRUELFANG,               false, false },
    { DATA_BEASTLORD_DARMAC,     NPC_DREADWING,               false, false },
    { DATA_BEASTLORD_DARMAC,     NPC_IRONCRUSHER,             false, false },
    { DATA_FLAMEBENDER_KAGRAZ,   NPC_AKNOR_STEELBRINGER,      false, true  },
    { DATA_HANSGAR_AND_FRANZOK,  BOSS_HANSGAR,                true,  true  },
    { DATA_HANSGAR_AND_FRANZOK,  BOSS_FRANZOK,                true,  true  },
    { DATA_BLAST_FURNACE,        BOSS_HEART_OF_THE_MOUNTAIN,  true,  true  },
    { DATA_BLAST_FURNACE,        BOSS_FOREMAN_FELDSPAR,       false, true  },
    { DATA_BLAST_FURNACE,        NPC_HEAT_REGULATOR,          false, true  },
    { DATA_IRON_MAIDENS,         BOSS_ADMIRAL_GARAN,          true,  true  },
    { DATA_IRON_MAIDENS,         BOSS_ENFORCER_SORKA,         true,  true  },
    { DATA_IRON_MAIDENS,         BOSS_MARAK_THE_BLOODED,      true,  true  }
};

// Hans'gar and Franzok: watcher health decides when mover leaves or returns
struct FoundryAnvilStage
{
    uint32_t watcher;
    int32_t healthPct;
    uint32_t mover;
    bool retreat;
};

FoundryAnvilStage const FoundryAnvilStages[] =
{
    { BOSS_HANSGAR, 85, BOSS_HANSGAR, true  },
    { BOSS_FRANZOK, 70, BOSS_HANSGAR, false },
    { BOSS_FRANZOK, 55, BOSS_FRANZOK, true  },
    { BOSS_HANSGAR, 40, BOSS_FRANZOK, false },
    { BOSS_HANSGAR, 25, BOSS_HANSGAR, true  },
    { BOSS_FRANZOK, 15, BOSS_HANSGAR, false }
};
uint32_t const FoundryAnvilStageCount = sizeof(FoundryAnvilStages) / sizeof(FoundryAnvilStages[0]);

uint32_t const FoundryIronMaidenEntries[3] = { BOSS_ADMIRAL_GARAN, BOSS_ENFORCER_SORKA, BOSS_MARAK_THE_BLOODED };

//////////////////////////////////////////////////////////////////////////////////////////
// Instance

class BlackrockFoundryInstanceScript : public InstanceScript
{
public:
    explicit BlackrockFoundryInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr)
    {
        setBossNumber(BlackrockFoundryEncounterCount);
    }

    static InstanceScript* Create(WorldMap* pMapMgr) { return new BlackrockFoundryInstanceScript(pMapMgr); }

    // a credited member falling before its partners must not end the encounter
    bool setBossState(uint32_t id, EncounterStates state) override
    {
        if (state == EncounterStates::Performed && getBossState(id) == EncounterStates::InProgress && !isEncounterGroupDefeated(id, nullptr))
            return false;

        return InstanceScript::setBossState(id, state);
    }

    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        for (FoundryExitDoor const& door : FoundryExitDoors)
        {
            if (door.entry != pGameObject->getEntry())
                continue;

            mExitDoorGuids[door.dataId].push_back(pGameObject->getGuid());

            // closed until the encounter is done, also on a saved instance
            setDoorOpen(pGameObject, getBossState(door.dataId) == EncounterStates::Performed);
            break;
        }
    }

    void OnEncounterStateChange(uint32_t id, uint32_t state) override
    {
        if (id == DATA_HANSGAR_AND_FRANZOK && state != EncounterStates::InProgress)
            mAnvilStage = 0;

        if (state != EncounterStates::Performed)
            return;

        auto const doors = mExitDoorGuids.find(id);
        if (doors == mExitDoorGuids.end())
            return;

        for (uint64_t guid : doors->second)
        {
            if (GameObject* pDoor = getGameObjectByGuid(guid))
                setDoorOpen(pDoor, true);
        }
    }

    CreatureAIScript* getLivingScript(uint32_t entry)
    {
        for (Creature* creature : getCreatureSetForEntry(entry))
        {
            if (creature->isAlive() && creature->GetScript())
                return creature->GetScript();
        }
        return nullptr;
    }

    uint32_t countLiving(uint32_t entry)
    {
        uint32_t count = 0;
        for (Creature* creature : getCreatureSetForEntry(entry))
        {
            if (creature->isAlive())
                ++count;
        }
        return count;
    }

    bool isEncounterGroupDefeated(uint32_t dataId, Creature* dying)
    {
        for (FoundryGroupMember const& member : FoundryGroupMembers)
        {
            if (member.dataId != dataId || !member.requiredForKill)
                continue;

            for (Creature* creature : getCreatureSetForEntry(member.entry))
            {
                if (creature != dying && creature->isAlive())
                    return false;
            }
        }
        return true;
    }

    void rallyEncounterGroup(uint32_t dataId, Creature* puller)
    {
        // nested pulls from the members' own combat start are ignored
        if (mRallying)
            return;

        mRallying = true;
        for (FoundryGroupMember const& member : FoundryGroupMembers)
        {
            if (member.dataId != dataId || !member.joinsOnPull)
                continue;

            for (Creature* creature : getCreatureSetForEntry(member.entry))
            {
                if (creature == puller || !creature->isAlive())
                    continue;

                if (CreatureAIScript* script = creature->GetScript())
                {
                    if (!script->_isInCombat())
                        script->setZoneWideCombat();
                }
            }
        }
        mRallying = false;
    }

    void reviveEncounterGroup(uint32_t dataId)
    {
        for (FoundryGroupMember const& member : FoundryGroupMembers)
        {
            if (member.dataId != dataId)
                continue;

            for (Creature* creature : getCreatureSetForEntry(member.entry))
            {
                if (creature->isAlive())
                    continue;

                if (CreatureAIScript* script = creature->GetScript())
                    script->despawn(0, 2000);
            }
        }
    }

    uint32_t getAnvilStage() const { return mAnvilStage; }
    void setAnvilStage(uint32_t stage) { mAnvilStage = stage; }

private:
    // the encounter decides, the spawn state of a door is not reliable
    void setDoorOpen(GameObject* pDoor, bool open)
    {
        uint8_t const state = open ? GO_STATE_OPEN : GO_STATE_CLOSED;
        if (pDoor->getState() != state)
            pDoor->setState(state);
    }

    std::map<uint32_t, std::vector<uint64_t>> mExitDoorGuids;
    uint32_t mAnvilStage = 0;
    bool mRallying = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Shared helpers

enum class FoundryCastTarget : uint8_t
{
    Self,
    Victim,
    AnyPlayer,
    NonTank,
    Ranged,
    WoundedAlly,
    Area
};

class FoundryCreatureAI : public CreatureAIScript
{
public:
    explicit FoundryCreatureAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void onSummonedCreature(Creature* summon) override { summons.summon(summon); }

protected:
    BlackrockFoundryInstanceScript* getFoundry()
    {
        if (getCreature()->GetMapId() != MAP_BLACKROCK_FOUNDRY)
            return nullptr;

        return static_cast<BlackrockFoundryInstanceScript*>(getInstanceScript());
    }

    CreatureAIScript* getFoundryScript(uint32_t entry)
    {
        BlackrockFoundryInstanceScript* foundry = getFoundry();
        return foundry ? foundry->getLivingScript(entry) : nullptr;
    }

    Unit* pickPlayer(bool skipTank, float minRange = 0.0f)
    {
        FilterArgs args(TargetFilter_Player);
        if (skipTank)
            args.addFilter(TargetFilter_NotCurrent);
        if (minRange > 0.0f)
            args.setMinMaxRange(minRange, 100.0f);

        if (Unit* target = selectUnitTarget(args))
            return target;

        // fall back to anyone when the raid is stacked
        return selectUnitTarget(FilterArgs(TargetFilter_Player));
    }

    void castAt(FoundryCastTarget target, uint32_t spellId)
    {
        switch (target)
        {
            case FoundryCastTarget::Self:
                castSpellOnSelf(spellId);
                break;
            case FoundryCastTarget::Victim:
                castSpellOnVictim(spellId);
                break;
            case FoundryCastTarget::AnyPlayer:
                if (Unit* player = pickPlayer(false))
                    castSpell(player, spellId);
                break;
            case FoundryCastTarget::NonTank:
                if (Unit* player = pickPlayer(true))
                    castSpell(player, spellId);
                break;
            case FoundryCastTarget::Ranged:
                if (Unit* player = pickPlayer(false, 10.0f))
                    castSpell(player, spellId);
                break;
            case FoundryCastTarget::WoundedAlly:
            {
                Unit* ally = selectUnitTarget(FilterArgs(TargetFilter_WoundedFriendly));
                castSpell(ally ? ally : getCreature(), spellId);
                break;
            }
            case FoundryCastTarget::Area:
                castSpellAOE(spellId);
                break;
        }
    }

    LocationVector ringPosition(uint32_t slot)
    {
        FoundryOffset const& offset = FoundryRingOffsets[slot % FoundryRingOffsetCount];
        return LocationVector(getCreature()->GetPositionX() + offset.x, getCreature()->GetPositionY() + offset.y, getCreature()->GetPositionZ(), getCreature()->GetOrientation());
    }

    // summoned adds join the fight right away
    Creature* summonFighter(uint32_t entry, LocationVector const& position)
    {
        Creature* add = summonCreature(entry, position);
        if (add != nullptr)
            setZoneWideCombat(add);
        return add;
    }

    void raidWarning(std::string const& text)
    {
        sendChatMessage(CHAT_MSG_RAID_BOSS_EMOTE, 0, text);
    }
};

// boss side of an encounter slot
class FoundryEncounterAI : public FoundryCreatureAI
{
public:
    FoundryEncounterAI(Creature* pCreature, uint32_t dataId) : FoundryCreatureAI(pCreature), mDataId(dataId) {}

protected:
    void openEncounter()
    {
        if (InstanceScript* instance = getInstanceScript())
        {
            instance->sendUnitEncounter(EncounterFrameEngage, getCreature());
            instance->setBossState(mDataId, EncounterStates::InProgress);
        }

        if (BlackrockFoundryInstanceScript* foundry = getFoundry())
            foundry->rallyEncounterGroup(mDataId, getCreature());
    }

    void failEncounter()
    {
        if (InstanceScript* instance = getInstanceScript())
        {
            instance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());
            instance->setBossState(mDataId, EncounterStates::Failed);
        }

        scriptEvents.resetEvents();
        summons.despawnAll();

        if (BlackrockFoundryInstanceScript* foundry = getFoundry())
            foundry->reviveEncounterGroup(mDataId);
    }

    void closeEncounter()
    {
        InstanceScript* instance = getInstanceScript();
        if (instance == nullptr)
            return;

        instance->sendUnitEncounter(EncounterFrameDisengaged, getCreature());

        BlackrockFoundryInstanceScript* foundry = getFoundry();
        if (foundry == nullptr || foundry->isEncounterGroupDefeated(mDataId, getCreature()))
            instance->setBossState(mDataId, EncounterStates::Performed);
    }

    uint32_t mDataId;
};

// adds with a fixed list of timed casts
struct FoundryRotationSpell
{
    uint32_t spellId;
    uint32_t firstMs;
    uint32_t repeatMs;
    FoundryCastTarget target;
};

class FoundryRotationAI : public FoundryCreatureAI
{
public:
    explicit FoundryRotationAI(Creature* pCreature) : FoundryCreatureAI(pCreature) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        for (size_t i = 0; i < mRotation.size(); ++i)
            scriptEvents.addEvent(static_cast<uint32_t>(i + 1), mRotation[i].firstMs);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        scriptEvents.resetEvents();
        summons.despawnAll();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        uint32_t const slot = scriptEvents.getFinishedEvent();
        if (slot == 0 || slot > mRotation.size())
            return;

        FoundryRotationSpell const& spell = mRotation[slot - 1];
        castAt(spell.target, spell.spellId);

        if (spell.repeatMs != 0)
            scriptEvents.addEvent(slot, spell.repeatMs);
    }

protected:
    void addRotation(uint32_t spellId, uint32_t firstMs, uint32_t repeatMs, FoundryCastTarget target)
    {
        mRotation.push_back({ spellId, firstMs, repeatMs, target });
    }

    std::vector<FoundryRotationSpell> mRotation;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Gruul

class FoundryGruulAI : public FoundryEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryGruulAI(c); }
    explicit FoundryGruulAI(Creature* pCreature) : FoundryEncounterAI(pCreature, DATA_GRUUL) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        openEncounter();

        mRampaging = false;
        castSpellOnSelf(SPELL_GRUUL_OVERWHELMING_BLOWS, true);

        scriptEvents.addEvent(EVENT_GRUUL_INFERNO_SLICE, 17000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_GRUUL_CAVE_IN, 12000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_GRUUL_PETRIFYING_SLAM, 22000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_GRUUL_OVERHEAD_SMASH, 44000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_GRUUL_RAMPAGE_START, 100000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_FOUNDRY_BERSERK, 480000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        failEncounter();
        mRampaging = false;
        setReactState(REACT_AGGRESSIVE);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        closeEncounter();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mRampaging ? FOUNDRY_STAGE_TWO : FOUNDRY_STAGE_ONE);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_GRUUL_INFERNO_SLICE:
                castSpellOnVictim(SPELL_GRUUL_INFERNO_SLICE);
                scriptEvents.addEvent(EVENT_GRUUL_INFERNO_SLICE, 17000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_GRUUL_CAVE_IN:
                if (Unit* target = pickPlayer(false, 10.0f))
                    summonCreature(NPC_CAVE_IN, target->GetPosition(), TIMED_DESPAWN, 90000);
                scriptEvents.addEvent(EVENT_GRUUL_CAVE_IN, 30000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_GRUUL_PETRIFYING_SLAM:
                castSpellOnSelf(SPELL_GRUUL_PETRIFYING_SLAM);
                scriptEvents.addEvent(EVENT_GRUUL_PETRIFYING_SLAM, 60000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_GRUUL_OVERHEAD_SMASH:
                castAt(FoundryCastTarget::AnyPlayer, SPELL_GRUUL_OVERHEAD_SMASH);
                scriptEvents.addEvent(EVENT_GRUUL_OVERHEAD_SMASH, 34000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_GRUUL_RAMPAGE_START:
                raidWarning("Gruul begins a Destructive Rampage!");
                castSpellOnSelf(SPELL_GRUUL_DESTRUCTIVE_RAMPAGE, true);
                mRampaging = true;
                setReactState(REACT_PASSIVE);
                moveTo(FoundryGruulRoomCenter.x, FoundryGruulRoomCenter.y, FoundryGruulRoomCenter.z);
                scriptEvents.addEvent(EVENT_GRUUL_RAMPAGE_SMASH, 3500, FOUNDRY_STAGE_TWO);
                scriptEvents.addEvent(EVENT_GRUUL_RAMPAGE_END, 30000, FOUNDRY_STAGE_TWO);
                // stage one timers are frozen for the 30s rampage, 80s + 30s = 110s cycle
                scriptEvents.addEvent(EVENT_GRUUL_RAMPAGE_START, 80000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_GRUUL_RAMPAGE_SMASH:
                castSpellOnSelf(SPELL_GRUUL_OVERHEAD_SMASH);
                scriptEvents.addEvent(EVENT_GRUUL_RAMPAGE_SMASH, 3500, FOUNDRY_STAGE_TWO);
                break;
            case EVENT_GRUUL_RAMPAGE_END:
                scriptEvents.removeEvent(EVENT_GRUUL_RAMPAGE_SMASH);
                _removeAura(SPELL_GRUUL_DESTRUCTIVE_RAMPAGE);
                mRampaging = false;
                setReactState(REACT_AGGRESSIVE);
                if (Unit* target = pickPlayer(false))
                    attackStart(target);
                break;
            case EVENT_FOUNDRY_BERSERK:
                castSpellOnSelf(SPELL_FOUNDRY_BERSERK, true);
                break;
            default:
                break;
        }
    }

private:
    bool mRampaging = false;
};

class FoundryCaveInAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryCaveInAI(c); }
    explicit FoundryCaveInAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnLoad() override
    {
        setReactState(REACT_PASSIVE);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        setRooted(true);
        castSpellOnSelf(SPELL_GRUUL_CAVE_IN, true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Oregorger

class FoundryOregorgerAI : public FoundryEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryOregorgerAI(c); }
    explicit FoundryOregorgerAI(Creature* pCreature) : FoundryEncounterAI(pCreature, DATA_OREGORGER) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        openEncounter();

        mRolling = false;
        mBarrageLeft = 0;
        mRouteIndex = 0;

        armFeedingKit();
        scriptEvents.addEvent(EVENT_OREGORGER_HUNGER_ENRAGE, 420000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        failEncounter();
        leaveRolling();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        closeEncounter();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mRolling ? FOUNDRY_STAGE_TWO : FOUNDRY_STAGE_ONE);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_OREGORGER_ACID_MAW:
                castSpellOnVictim(SPELL_OREGORGER_ACID_MAW, true);
                scriptEvents.addEvent(EVENT_OREGORGER_ACID_MAW, 6000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_OREGORGER_RETCHED_BLACKROCK:
                castAt(FoundryCastTarget::Ranged, SPELL_OREGORGER_RETCHED_BLACKROCK);
                scriptEvents.addEvent(EVENT_OREGORGER_RETCHED_BLACKROCK, 15000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_OREGORGER_EXPLOSIVE_SHARD:
            {
                FilterArgs meleeRange(TargetFilter_Player, 0.0f, 8.0f);
                if (Unit* target = selectUnitTarget(meleeRange))
                    castSpell(target, SPELL_OREGORGER_EXPLOSIVE_SHARD, true);
                scriptEvents.addEvent(EVENT_OREGORGER_EXPLOSIVE_SHARD, 15000, FOUNDRY_STAGE_ONE);
                break;
            }
            case EVENT_OREGORGER_ACID_TORRENT:
                castSpellOnVictim(SPELL_OREGORGER_ACID_TORRENT);
                scriptEvents.addEvent(EVENT_OREGORGER_ACID_TORRENT, 13000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_OREGORGER_BLACKROCK_SPINES:
                castSpellOnSelf(SPELL_OREGORGER_BLACKROCK_SPINES, true);
                mBarrageLeft = 3;
                scriptEvents.addEvent(EVENT_OREGORGER_BLACKROCK_BARRAGE, 1000, FOUNDRY_STAGE_ONE);
                scriptEvents.addEvent(EVENT_OREGORGER_BLACKROCK_SPINES, 19000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_OREGORGER_BLACKROCK_BARRAGE:
                if (mBarrageLeft > 0)
                {
                    --mBarrageLeft;
                    castSpellAOE(SPELL_OREGORGER_BLACKROCK_BARRAGE);
                    if (mBarrageLeft > 0)
                        scriptEvents.addEvent(EVENT_OREGORGER_BLACKROCK_BARRAGE, 2000, FOUNDRY_STAGE_ONE);
                    else
                        _removeAura(SPELL_OREGORGER_BLACKROCK_SPINES);
                }
                break;
            case EVENT_OREGORGER_ROLLING_START:
                enterRolling();
                break;
            case EVENT_OREGORGER_ROLL_ON:
            {
                LocationVector const& point = FoundryOregorgerRollRoute[mRouteIndex % FoundryOregorgerRollRouteSize];
                ++mRouteIndex;
                moveTo(point.x, point.y, point.z);
                scriptEvents.addEvent(EVENT_OREGORGER_ROLL_ON, 5000, FOUNDRY_STAGE_TWO);
                break;
            }
            case EVENT_OREGORGER_ROLLING_END:
                leaveRolling();
                armFeedingKit();
                if (Unit* target = pickPlayer(false))
                    attackStart(target);
                break;
            case EVENT_OREGORGER_HUNGER_ENRAGE:
                // starving: no more rolling, endless stomps
                if (mRolling)
                    leaveRolling();
                scriptEvents.removeEvent(EVENT_OREGORGER_ROLLING_START);
                scriptEvents.addEvent(EVENT_OREGORGER_STOMP, 1000);
                break;
            case EVENT_OREGORGER_STOMP:
                castSpellAOE(SPELL_OREGORGER_EARTHSHAKING_STOMP);
                scriptEvents.addEvent(EVENT_OREGORGER_STOMP, 1500);
                break;
            default:
                break;
        }
    }

private:
    void armFeedingKit()
    {
        scriptEvents.addEvent(EVENT_OREGORGER_ACID_MAW, 3000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_OREGORGER_RETCHED_BLACKROCK, 6000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_OREGORGER_EXPLOSIVE_SHARD, 9000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_OREGORGER_ACID_TORRENT, 12000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_OREGORGER_BLACKROCK_SPINES, 14000, FOUNDRY_STAGE_ONE);
        // stands in for the empty mana bar
        scriptEvents.addEvent(EVENT_OREGORGER_ROLLING_START, 100000, FOUNDRY_STAGE_ONE);
    }

    void enterRolling()
    {
        mRolling = true;
        mBarrageLeft = 0;
        _removeAura(SPELL_OREGORGER_BLACKROCK_SPINES);

        raidWarning("Oregorger rolls through the room in search of ore!");
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        castSpellAOE(SPELL_OREGORGER_WALLSHAKING_ROAR, true);
        castSpellOnSelf(SPELL_OREGORGER_ROLLING_FURY, true);
        castSpellOnSelf(SPELL_OREGORGER_HUNGER_DRIVE, true);

        scriptEvents.addEvent(EVENT_OREGORGER_ROLL_ON, 2000, FOUNDRY_STAGE_TWO);
        scriptEvents.addEvent(EVENT_OREGORGER_ROLLING_END, 35000, FOUNDRY_STAGE_TWO);
    }

    void leaveRolling()
    {
        mRolling = false;
        scriptEvents.removeEvent(EVENT_OREGORGER_ROLL_ON);
        scriptEvents.removeEvent(EVENT_OREGORGER_ROLLING_END);
        _removeAura(SPELL_OREGORGER_ROLLING_FURY);
        _removeAura(SPELL_OREGORGER_HUNGER_DRIVE);
        _setMeleeDisabled(false);
        setReactState(REACT_AGGRESSIVE);
    }

    bool mRolling = false;
    uint32_t mBarrageLeft = 0;
    uint32_t mRouteIndex = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Beastlord Darmac

struct FoundryDarmacBeast
{
    uint32_t entry;
    uint32_t spiritSpell;
    uint32_t riderEvent;
    uint32_t riderFirstMs;
    char const* mountText;
};

FoundryDarmacBeast const FoundryDarmacBeasts[3] =
{
    { NPC_IRONCRUSHER, SPELL_DARMAC_FURY_OF_THE_ELEKK,   EVENT_DARMAC_TANTRUM,              22000, "Beastlord Darmac mounts Ironcrusher!" },
    { NPC_CRUELFANG,   SPELL_DARMAC_CUNNING_OF_THE_WOLF, EVENT_DARMAC_REND_AND_TEAR,        20000, "Beastlord Darmac mounts Cruelfang!" },
    { NPC_DREADWING,   SPELL_DARMAC_SPIRIT_OF_THE_RYLAK, EVENT_DARMAC_SUPERHEATED_SHRAPNEL, 12000, "Beastlord Darmac mounts Dreadwing!" }
};

int32_t const FoundryDarmacMountHealth[3] = { 85, 65, 45 };

class FoundryDarmacAI : public FoundryEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryDarmacAI(c); }
    explicit FoundryDarmacAI(Creature* pCreature) : FoundryEncounterAI(pCreature, DATA_BEASTLORD_DARMAC) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        openEncounter();

        mMountStage = 0;
        mRiddenMask = 0;

        scriptEvents.addEvent(EVENT_DARMAC_CALL_THE_PACK, 8000);
        scriptEvents.addEvent(EVENT_DARMAC_PIN_DOWN, 9500);
        scriptEvents.addEvent(EVENT_FOUNDRY_BERSERK, 720000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        failEncounter();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        closeEncounter();
    }

    // a ridden beast fell, Darmac keeps its power
    void SetCreatureData(uint32_t type, uint32_t data) override
    {
        if (type != FOUNDRY_DATA_BEAST_SLAIN || !_isInCombat())
            return;

        for (FoundryDarmacBeast const& beast : FoundryDarmacBeasts)
        {
            if (beast.entry != data)
                continue;

            castSpellOnSelf(beast.spiritSpell, true);
            scriptEvents.addEvent(beast.riderEvent, beast.riderFirstMs);
            break;
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (mMountStage < 3 && _getHealthPercent() <= FoundryDarmacMountHealth[mMountStage])
        {
            ++mMountStage;
            mountNextBeast();
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_DARMAC_CALL_THE_PACK:
                castSpellOnSelf(SPELL_DARMAC_CALL_THE_PACK, true);
                for (uint32_t i = 0; i < 2; ++i)
                    summonFighter(NPC_PACK_BEAST, ringPosition(mPackSlot++));
                scriptEvents.addEvent(EVENT_DARMAC_CALL_THE_PACK, 30000);
                break;
            case EVENT_DARMAC_PIN_DOWN:
                castAt(FoundryCastTarget::Ranged, SPELL_DARMAC_PIN_DOWN);
                scriptEvents.addEvent(EVENT_DARMAC_PIN_DOWN, 19700);
                break;
            case EVENT_DARMAC_TANTRUM:
                castSpellOnSelf(SPELL_DARMAC_TANTRUM);
                scriptEvents.addEvent(EVENT_DARMAC_TANTRUM, 23000);
                break;
            case EVENT_DARMAC_REND_AND_TEAR:
                castAt(FoundryCastTarget::AnyPlayer, SPELL_DARMAC_REND_AND_TEAR);
                scriptEvents.addEvent(EVENT_DARMAC_REND_AND_TEAR, 12000);
                break;
            case EVENT_DARMAC_SUPERHEATED_SHRAPNEL:
                castAt(FoundryCastTarget::NonTank, SPELL_DARMAC_SUPERHEATED_SHRAPNEL);
                scriptEvents.addEvent(EVENT_DARMAC_SUPERHEATED_SHRAPNEL, 25000);
                break;
            case EVENT_FOUNDRY_BERSERK:
                castSpellOnSelf(SPELL_FOUNDRY_BERSERK, true);
                break;
            default:
                break;
        }
    }

private:
    void mountNextBeast()
    {
        for (uint32_t i = 0; i < 3; ++i)
        {
            if (mRiddenMask & (1u << i))
                continue;

            CreatureAIScript* beast = getFoundryScript(FoundryDarmacBeasts[i].entry);
            if (beast == nullptr)
                continue;

            mRiddenMask |= (1u << i);
            raidWarning(FoundryDarmacBeasts[i].mountText);
            beast->DoAction(ACTION_DARMAC_UNLEASH_BEAST);
            return;
        }
    }

    uint32_t mMountStage = 0;
    uint32_t mRiddenMask = 0;
    uint32_t mPackSlot = 0;
};

class FoundryDarmacBeastAI : public FoundryRotationAI
{
public:
    explicit FoundryDarmacBeastAI(Creature* pCreature) : FoundryRotationAI(pCreature) {}

    void DoAction(int32_t action) override
    {
        if (action != ACTION_DARMAC_UNLEASH_BEAST)
            return;

        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IGNORE_PLAYER_COMBAT);
        setReactState(REACT_AGGRESSIVE);
        setZoneWideCombat();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (CreatureAIScript* darmac = getFoundryScript(BOSS_BEASTLORD_DARMAC))
            darmac->SetCreatureData(FOUNDRY_DATA_BEAST_SLAIN, getCreature()->getEntry());
    }
};

class FoundryCruelfangAI : public FoundryDarmacBeastAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryCruelfangAI(c); }
    explicit FoundryCruelfangAI(Creature* pCreature) : FoundryDarmacBeastAI(pCreature)
    {
        addRotation(SPELL_CRUELFANG_REND_AND_TEAR, 13000, 12000, FoundryCastTarget::AnyPlayer);
        addRotation(SPELL_CRUELFANG_SAVAGE_HOWL, 17000, 26000, FoundryCastTarget::Self);
    }
};

class FoundryDreadwingAI : public FoundryDarmacBeastAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryDreadwingAI(c); }
    explicit FoundryDreadwingAI(Creature* pCreature) : FoundryDarmacBeastAI(pCreature)
    {
        addRotation(SPELL_DREADWING_INFERNO_BREATH, 5000, 20000, FoundryCastTarget::NonTank);
        addRotation(SPELL_DREADWING_CONFLAGRATION, 12000, 20000, FoundryCastTarget::AnyPlayer);
    }
};

class FoundryIroncrusherAI : public FoundryDarmacBeastAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryIroncrusherAI(c); }
    explicit FoundryIroncrusherAI(Creature* pCreature) : FoundryDarmacBeastAI(pCreature)
    {
        addRotation(SPELL_IRONCRUSHER_CRUSH_ARMOR, 8000, 8000, FoundryCastTarget::Victim);
        addRotation(SPELL_IRONCRUSHER_STAMPEDE, 15000, 20000, FoundryCastTarget::Self);
        addRotation(SPELL_IRONCRUSHER_TANTRUM, 25000, 23000, FoundryCastTarget::Self);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Flamebender Ka'graz

class FoundryKagrazAI : public FoundryEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryKagrazAI(c); }
    explicit FoundryKagrazAI(Creature* pCreature) : FoundryEncounterAI(pCreature, DATA_FLAMEBENDER_KAGRAZ) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        openEncounter();

        mMoltenEnergy = 0;
        mEnergyTimer = 0;
        mFirestorm = false;

        scriptEvents.addEvent(EVENT_KAGRAZ_LAVA_SLASH, 11000);
        scriptEvents.addEvent(EVENT_FOUNDRY_BERSERK, 420000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        failEncounter();
        mFirestorm = false;
        setRooted(false);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        closeEncounter();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        // one molten energy every 1.2s, new abilities at 25/50/75/100
        if (!mFirestorm)
        {
            mEnergyTimer += static_cast<uint32_t>(time_passed);
            while (mEnergyTimer >= 1200 && mMoltenEnergy < 100)
            {
                mEnergyTimer -= 1200;
                ++mMoltenEnergy;

                if (mMoltenEnergy == 25)
                    scriptEvents.addEvent(EVENT_KAGRAZ_MOLTEN_TORRENT, 1);
                else if (mMoltenEnergy == 50)
                    scriptEvents.addEvent(EVENT_KAGRAZ_CINDER_WOLVES, 1);
                else if (mMoltenEnergy == 75)
                    scriptEvents.addEvent(EVENT_KAGRAZ_BLAZING_RADIANCE, 1);
                else if (mMoltenEnergy == 100)
                    scriptEvents.addEvent(EVENT_KAGRAZ_FIRESTORM, 1);
            }
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KAGRAZ_LAVA_SLASH:
                castAt(FoundryCastTarget::Ranged, SPELL_KAGRAZ_LAVA_SLASH);
                scriptEvents.addEvent(EVENT_KAGRAZ_LAVA_SLASH, 14500);
                break;
            case EVENT_KAGRAZ_MOLTEN_TORRENT:
                castAt(FoundryCastTarget::Ranged, SPELL_KAGRAZ_MOLTEN_TORRENT);
                scriptEvents.addEvent(EVENT_KAGRAZ_MOLTEN_TORRENT, 14500);
                break;
            case EVENT_KAGRAZ_CINDER_WOLVES:
            {
                raidWarning("Flamebender Ka'graz calls forth her Cinder Wolves!");
                Creature* first = summonFighter(NPC_CINDER_WOLF, FoundryCinderWolfSpawn);
                summonFighter(NPC_CINDER_WOLF, FoundryCinderWolfSpawn);
                if (first != nullptr && first->GetScript())
                    first->GetScript()->castSpellOnSelf(SPELL_CINDER_WOLF_OVERHEATED, true);
                break;
            }
            case EVENT_KAGRAZ_BLAZING_RADIANCE:
                castAt(FoundryCastTarget::Ranged, SPELL_KAGRAZ_BLAZING_RADIANCE);
                scriptEvents.addEvent(EVENT_KAGRAZ_BLAZING_RADIANCE, 12000);
                break;
            case EVENT_KAGRAZ_FIRESTORM:
                raidWarning("Flamebender Ka'graz unleashes a Firestorm!");
                mFirestorm = true;
                scriptEvents.removeEvent(EVENT_KAGRAZ_MOLTEN_TORRENT);
                scriptEvents.removeEvent(EVENT_KAGRAZ_BLAZING_RADIANCE);
                scriptEvents.delayEvent(EVENT_KAGRAZ_LAVA_SLASH, 14000);
                setRooted(true);
                castSpellOnSelf(SPELL_KAGRAZ_FIRESTORM, true);
                castSpellOnSelf(SPELL_KAGRAZ_FIRESTORM_PULSE);
                scriptEvents.addEvent(EVENT_KAGRAZ_FIRESTORM_END, 14000);
                break;
            case EVENT_KAGRAZ_FIRESTORM_END:
                _removeAura(SPELL_KAGRAZ_FIRESTORM);
                castSpellOnSelf(SPELL_KAGRAZ_FLAMEFURY, true);
                setRooted(false);
                mMoltenEnergy = 0;
                mEnergyTimer = 0;
                mFirestorm = false;
                break;
            case EVENT_FOUNDRY_BERSERK:
                castSpellOnSelf(SPELL_FOUNDRY_BERSERK, true);
                break;
            default:
                break;
        }
    }

private:
    uint32_t mMoltenEnergy = 0;
    uint32_t mEnergyTimer = 0;
    bool mFirestorm = false;
};

class FoundryAknorAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryAknorAI(c); }
    explicit FoundryAknorAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_AKNOR_DEVASTATING_SLAM, 6000, 8000, FoundryCastTarget::Victim);
        addRotation(SPELL_AKNOR_DROP_THE_HAMMER, 10000, 8000, FoundryCastTarget::AnyPlayer);
    }
};

class FoundryCinderWolfAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryCinderWolfAI(c); }
    explicit FoundryCinderWolfAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_CINDER_WOLF_FIXATE, 1000, 30000, FoundryCastTarget::AnyPlayer);
        addRotation(SPELL_CINDER_WOLF_CHARRING_BREATH, 7000, 7000, FoundryCastTarget::Victim);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Hans'gar and Franzok

class FoundryAnvilBrotherAI : public FoundryEncounterAI
{
public:
    FoundryAnvilBrotherAI(Creature* pCreature, LocationVector const& arenaPos, LocationVector const& retreatPos) :
        FoundryEncounterAI(pCreature, DATA_HANSGAR_AND_FRANZOK), mArenaPos(arenaPos), mRetreatPos(retreatPos) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        openEncounter();
        mOutOfArena = false;
        armArenaKit();
        scriptEvents.addEvent(EVENT_FOUNDRY_BERSERK, 600000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        failEncounter();
        if (mOutOfArena)
            setInArena(true);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        closeEncounter();
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_ANVIL_RETREAT && !mOutOfArena)
        {
            raidWarning("The brothers trade places at the assembly line!");
            setInArena(false);
            moveJump(mRetreatPos, 20.0f, 10.0f);
        }
        else if (action == ACTION_ANVIL_RETURN && mOutOfArena)
        {
            setInArena(true);
            moveJump(mArenaPos, 20.0f, 10.0f);
            scriptEvents.addEvent(EVENT_ANVIL_LANDING_SLAM, 2500);
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mOutOfArena ? FOUNDRY_STAGE_TWO : FOUNDRY_STAGE_ONE);

        checkAnvilStage();

        if (_isCasting())
            return;

        uint32_t const eventId = scriptEvents.getFinishedEvent();
        if (eventId == EVENT_FOUNDRY_BERSERK)
        {
            castSpellOnSelf(SPELL_FOUNDRY_BERSERK, true);
            return;
        }

        if (eventId == EVENT_ANVIL_LANDING_SLAM)
        {
            castAt(FoundryCastTarget::AnyPlayer, SPELL_ANVIL_BODY_SLAM);
            if (Unit* target = pickPlayer(false))
                attackStart(target);
            return;
        }

        handleArenaEvent(eventId);
    }

protected:
    virtual void armArenaKit() = 0;
    virtual void handleArenaEvent(uint32_t eventId) = 0;

private:
    void setInArena(bool inArena)
    {
        mOutOfArena = !inArena;
        if (inArena)
        {
            removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            _setMeleeDisabled(false);
            setReactState(REACT_AGGRESSIVE);
        }
        else
        {
            stopMovement();
            setReactState(REACT_PASSIVE);
            _setMeleeDisabled(true);
            addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        }
    }

    void checkAnvilStage()
    {
        BlackrockFoundryInstanceScript* foundry = getFoundry();
        if (foundry == nullptr || mOutOfArena)
            return;

        uint32_t const stage = foundry->getAnvilStage();
        if (stage >= FoundryAnvilStageCount)
            return;

        FoundryAnvilStage const& step = FoundryAnvilStages[stage];
        if (step.watcher != getCreature()->getEntry() || _getHealthPercent() > step.healthPct)
            return;

        foundry->setAnvilStage(stage + 1);
        if (CreatureAIScript* mover = getFoundryScript(step.mover))
            mover->DoAction(step.retreat ? ACTION_ANVIL_RETREAT : ACTION_ANVIL_RETURN);
    }

    LocationVector mArenaPos;
    LocationVector mRetreatPos;
    bool mOutOfArena = false;
};

class FoundryHansgarAI : public FoundryAnvilBrotherAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryHansgarAI(c); }
    explicit FoundryHansgarAI(Creature* pCreature) : FoundryAnvilBrotherAI(pCreature, FoundryHansgarArenaPos, FoundryHansgarRetreatPos) {}

protected:
    void armArenaKit() override
    {
        scriptEvents.addEvent(EVENT_ANVIL_JUMP_SLAM, 20500, FOUNDRY_STAGE_ONE);
    }

    void handleArenaEvent(uint32_t eventId) override
    {
        if (eventId != EVENT_ANVIL_JUMP_SLAM)
            return;

        castAt(FoundryCastTarget::AnyPlayer, SPELL_HANSGAR_JUMP_SLAM);
        scriptEvents.addEvent(EVENT_ANVIL_JUMP_SLAM, 25000, FOUNDRY_STAGE_ONE);
    }
};

class FoundryFranzokAI : public FoundryAnvilBrotherAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryFranzokAI(c); }
    explicit FoundryFranzokAI(Creature* pCreature) : FoundryAnvilBrotherAI(pCreature, FoundryFranzokArenaPos, FoundryFranzokRetreatPos) {}

protected:
    void armArenaKit() override
    {
        scriptEvents.addEvent(EVENT_ANVIL_SKULLCRACKER, 20000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_ANVIL_DISRUPTING_ROAR, 45000, FOUNDRY_STAGE_ONE);
    }

    void handleArenaEvent(uint32_t eventId) override
    {
        if (eventId == EVENT_ANVIL_SKULLCRACKER)
        {
            castAt(FoundryCastTarget::Ranged, SPELL_FRANZOK_SKULLCRACKER);
            scriptEvents.addEvent(EVENT_ANVIL_SKULLCRACKER, 21000, FOUNDRY_STAGE_ONE);
        }
        else if (eventId == EVENT_ANVIL_DISRUPTING_ROAR)
        {
            castSpellAOE(SPELL_FRANZOK_DISRUPTING_ROAR);
            scriptEvents.addEvent(EVENT_ANVIL_DISRUPTING_ROAR, 46000, FOUNDRY_STAGE_ONE);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Operator Thogar

enum class FoundryThogarWaveKind : uint8_t
{
    Troops,
    Elite
};

struct FoundryThogarWave
{
    uint32_t atMs;
    FoundryThogarWaveKind kind;
};

// timed reinforcements in place of the add trains
FoundryThogarWave const FoundryThogarWaves[] =
{
    { 32000,  FoundryThogarWaveKind::Troops },
    { 82000,  FoundryThogarWaveKind::Elite  },
    { 122000, FoundryThogarWaveKind::Troops },
    { 197000, FoundryThogarWaveKind::Troops },
    { 252000, FoundryThogarWaveKind::Elite  },
    { 372000, FoundryThogarWaveKind::Troops },
    { 372000, FoundryThogarWaveKind::Elite  },
    { 462000, FoundryThogarWaveKind::Troops }
};
uint32_t const FoundryThogarWaveCount = sizeof(FoundryThogarWaves) / sizeof(FoundryThogarWaves[0]);

class FoundryThogarAI : public FoundryEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryThogarAI(c); }
    explicit FoundryThogarAI(Creature* pCreature) : FoundryEncounterAI(pCreature, DATA_OPERATOR_THOGAR) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        openEncounter();

        mNextWave = 0;

        scriptEvents.addEvent(EVENT_THOGAR_PULSE_GRENADE, 7000);
        scriptEvents.addEvent(EVENT_THOGAR_ENKINDLE, 16000);
        scriptEvents.addEvent(EVENT_THOGAR_REINFORCEMENTS, FoundryThogarWaves[0].atMs);
        scriptEvents.addEvent(EVENT_FOUNDRY_BERSERK, 480000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        failEncounter();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        closeEncounter();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_THOGAR_ENKINDLE:
                castSpellOnVictim(SPELL_THOGAR_ENKINDLE);
                scriptEvents.addEvent(EVENT_THOGAR_ENKINDLE, 16000);
                break;
            case EVENT_THOGAR_PULSE_GRENADE:
                castAt(FoundryCastTarget::Ranged, SPELL_THOGAR_PROTOTYPE_PULSE_GRENADE);
                scriptEvents.addEvent(EVENT_THOGAR_PULSE_GRENADE, 12000);
                break;
            case EVENT_THOGAR_REINFORCEMENTS:
                sendWave();
                break;
            case EVENT_FOUNDRY_BERSERK:
                castSpellOnSelf(SPELL_FOUNDRY_BERSERK, true);
                break;
            default:
                break;
        }
    }

private:
    void sendWave()
    {
        if (mNextWave >= FoundryThogarWaveCount)
            return;

        FoundryThogarWave const& wave = FoundryThogarWaves[mNextWave];
        LocationVector const* exits = FoundryThogarExits[mNextWave % 2];

        if (wave.kind == FoundryThogarWaveKind::Troops)
        {
            raidWarning("Iron Horde reinforcements jump off the train!");
            for (uint32_t i = 0; i < 4; ++i)
                summonFighter(NPC_THOGAR_IRON_RAIDER, exits[i]);
            summonFighter(NPC_THOGAR_IRON_CRACK_SHOT, exits[4]);
            summonFighter(NPC_THOGAR_IRON_CRACK_SHOT, exits[5]);
        }
        else
        {
            summonFighter(NPC_THOGAR_IRON_CRACK_SHOT, exits[0]);
            summonFighter(NPC_THOGAR_IRON_CRACK_SHOT, exits[1]);
            summonFighter(NPC_THOGAR_GROMKAR_FIREMENDER, exits[2]);
            summonFighter(NPC_THOGAR_GROMKAR_FIREMENDER, exits[3]);
            summonFighter(NPC_THOGAR_GROMKAR_MAN_AT_ARMS, FoundryThogarManAtArmsExit);
            summonFighter(NPC_THOGAR_GROMKAR_MAN_AT_ARMS, LocationVector(FoundryThogarManAtArmsExit.x, FoundryThogarManAtArmsExit.y + 4.0f, FoundryThogarManAtArmsExit.z, FoundryThogarManAtArmsExit.o));
        }

        ++mNextWave;
        if (mNextWave < FoundryThogarWaveCount)
        {
            uint32_t const gap = FoundryThogarWaves[mNextWave].atMs - wave.atMs;
            scriptEvents.addEvent(EVENT_THOGAR_REINFORCEMENTS, gap > 0 ? gap : 1);
        }
    }

    uint32_t mNextWave = 0;
};

class FoundryThogarIronRaiderAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryThogarIronRaiderAI(c); }
    explicit FoundryThogarIronRaiderAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_IRON_RAIDER_THROW_GRENADE, 3500, 13500, FoundryCastTarget::AnyPlayer);
        addRotation(SPELL_IRON_RAIDER_SERRATED_SLASH, 6000, 12000, FoundryCastTarget::Victim);
    }
};

class FoundryThogarCrackShotAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryThogarCrackShotAI(c); }
    explicit FoundryThogarCrackShotAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_CRACK_SHOT_SHOOT_RIFLE, 1500, 3000, FoundryCastTarget::Victim);
        addRotation(SPELL_IRON_RAIDER_THROW_GRENADE, 3500, 13500, FoundryCastTarget::AnyPlayer);
    }
};

class FoundryThogarFiremenderAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryThogarFiremenderAI(c); }
    explicit FoundryThogarFiremenderAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_FIREMENDER_CAUTERIZING_BOLT, 5000, 20000, FoundryCastTarget::WoundedAlly);
    }
};

class FoundryThogarManAtArmsAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryThogarManAtArmsAI(c); }
    explicit FoundryThogarManAtArmsAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_MAN_AT_ARMS_IRON_BELLOW, 1000, 10000, FoundryCastTarget::Area);
        addRotation(SPELL_MAN_AT_ARMS_RECKLESS_SLASH, 3000, 10000, FoundryCastTarget::AnyPlayer);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Blast Furnace

class FoundryFeldsparAI : public FoundryEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryFeldsparAI(c); }
    explicit FoundryFeldsparAI(Creature* pCreature) : FoundryEncounterAI(pCreature, DATA_BLAST_FURNACE) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        openEncounter();

        castSpellOnSelf(SPELL_FELDSPAR_HOT_BLOODED, true);
        scriptEvents.addEvent(EVENT_FURNACE_PYROCLASM, 18000);
        scriptEvents.addEvent(EVENT_FURNACE_RUPTURE, 26000);
        scriptEvents.addEvent(EVENT_FOUNDRY_BERSERK, 780000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        failEncounter();
    }

    // only the Heart of the Mountain completes the encounter
    void OnDied(Unit* /*pKiller*/) override
    {
        closeEncounter();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_FURNACE_PYROCLASM:
                castSpellOnSelf(SPELL_FELDSPAR_PYROCLASM);
                scriptEvents.addEvent(EVENT_FURNACE_PYROCLASM, 20000);
                break;
            case EVENT_FURNACE_RUPTURE:
                castAt(FoundryCastTarget::Ranged, SPELL_FELDSPAR_RUPTURE);
                scriptEvents.addEvent(EVENT_FURNACE_RUPTURE, 25000);
                break;
            case EVENT_FOUNDRY_BERSERK:
                castSpellOnSelf(SPELL_FOUNDRY_BERSERK, true);
                break;
            default:
                break;
        }
    }
};

class FoundryHeartOfTheMountainAI : public FoundryEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryHeartOfTheMountainAI(c); }
    explicit FoundryHeartOfTheMountainAI(Creature* pCreature) : FoundryEncounterAI(pCreature, DATA_BLAST_FURNACE) {}

    void OnLoad() override
    {
        setContained(true);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        openEncounter();
        setContained(true);

        mStage = FOUNDRY_STAGE_ONE;
        mBlastInterval = 30000;
        mCrewSide = 0;
        mElementalistsLeft = 0;

        BlackrockFoundryInstanceScript* foundry = getFoundry();
        mRegulatorsLeft = foundry ? foundry->countLiving(NPC_HEAT_REGULATOR) : 0;
        // without regulator spawns the furnace still breaks open
        if (mRegulatorsLeft == 0)
            scriptEvents.addEvent(EVENT_FURNACE_CONTAINMENT_FAILSAFE, 90000, FOUNDRY_STAGE_ONE);

        scriptEvents.addEvent(EVENT_FURNACE_BLAST, mBlastInterval);
        scriptEvents.addEvent(EVENT_FURNACE_CREW_WAVE, 60000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_FOUNDRY_BERSERK, 780000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        failEncounter();
        setContained(true);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        closeEncounter();
    }

    void DoAction(int32_t action) override
    {
        if (!_isInCombat())
            return;

        if (action == ACTION_FURNACE_REGULATOR_LOST && mStage == FOUNDRY_STAGE_ONE)
        {
            if (mRegulatorsLeft > 0)
                --mRegulatorsLeft;
            if (mRegulatorsLeft == 0)
                enterElementalStage();
        }
        else if (action == ACTION_FURNACE_ELEMENTALIST_LOST && mStage == FOUNDRY_STAGE_TWO)
        {
            if (mElementalistsLeft > 0)
                --mElementalistsLeft;
            if (mElementalistsLeft == 0)
                enterFreedomStage();
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mStage);

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_FURNACE_BLAST:
                // heat builds up, every blast comes faster
                castSpellAOE(SPELL_HEART_BLAST, true);
                if (mBlastInterval > 12000)
                    mBlastInterval -= 2000;
                scriptEvents.addEvent(EVENT_FURNACE_BLAST, mBlastInterval);
                break;
            case EVENT_FURNACE_CREW_WAVE:
                sendFurnaceCrew();
                scriptEvents.addEvent(EVENT_FURNACE_CREW_WAVE, 60000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_FURNACE_CONTAINMENT_FAILSAFE:
                enterElementalStage();
                break;
            case EVENT_FURNACE_SLAG_ELEMENTAL:
                summonFighter(NPC_SLAG_ELEMENTAL, FoundryElementalistPositions[mCrewSide++ % 4]);
                scriptEvents.addEvent(EVENT_FURNACE_SLAG_ELEMENTAL, 55000, FOUNDRY_STAGE_TWO);
                break;
            case EVENT_FURNACE_FIRECALLERS:
                summonFighter(NPC_FIRECALLER, FoundryElementalistPositions[mCrewSide++ % 4]);
                summonFighter(NPC_FIRECALLER, FoundryElementalistPositions[mCrewSide++ % 4]);
                scriptEvents.addEvent(EVENT_FURNACE_FIRECALLERS, 45000, FOUNDRY_STAGE_TWO);
                break;
            case EVENT_FURNACE_FLOOR_GUARD:
                summonFighter(NPC_SECURITY_GUARD, FoundryFurnaceFloorGuardPos);
                scriptEvents.addEvent(EVENT_FURNACE_FLOOR_GUARD, 40000, FOUNDRY_STAGE_TWO);
                break;
            case EVENT_FURNACE_HEAT:
                castSpellOnVictim(SPELL_HEART_TEMPERED, true);
                castSpellOnVictim(SPELL_HEART_HEAT, true);
                scriptEvents.addEvent(EVENT_FURNACE_HEAT, 11000, FOUNDRY_STAGE_THREE);
                break;
            case EVENT_FURNACE_MELT:
                castAt(FoundryCastTarget::AnyPlayer, SPELL_HEART_MELT);
                scriptEvents.addEvent(EVENT_FURNACE_MELT, 10000, FOUNDRY_STAGE_THREE);
                break;
            case EVENT_FOUNDRY_BERSERK:
                castSpellOnSelf(SPELL_FOUNDRY_BERSERK, true);
                break;
            default:
                break;
        }
    }

private:
    void setContained(bool contained)
    {
        if (contained)
        {
            setReactState(REACT_PASSIVE);
            _setMeleeDisabled(true);
            addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            setRooted(true);
        }
        else
        {
            removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            setRooted(false);
            _setMeleeDisabled(false);
            setReactState(REACT_AGGRESSIVE);
        }
    }

    void sendFurnaceCrew()
    {
        for (uint32_t side = 0; side < 2; ++side)
        {
            summonFighter(NPC_FURNACE_ENGINEER, FoundryFurnaceCrewSpawns[side][mCrewSide % 3]);
            summonFighter(NPC_SECURITY_GUARD, FoundryFurnaceCrewSpawns[side][(mCrewSide + 1) % 3]);
            summonFighter(NPC_BELLOWS_OPERATOR, FoundryBellowsOperatorSpawns[side]);
        }
        ++mCrewSide;
    }

    void enterElementalStage()
    {
        if (mStage != FOUNDRY_STAGE_ONE)
            return;

        mStage = FOUNDRY_STAGE_TWO;
        raidWarning("The Heart of the Mountain breaks loose!");
        castSpellOnSelf(SPELL_HEART_BUST_LOOSE, true);

        mElementalistsLeft = 0;
        for (LocationVector const& position : FoundryElementalistPositions)
        {
            if (summonFighter(NPC_PRIMAL_ELEMENTALIST, position) != nullptr)
                ++mElementalistsLeft;
        }

        if (mElementalistsLeft == 0)
        {
            enterFreedomStage();
            return;
        }

        scriptEvents.addEvent(EVENT_FURNACE_SLAG_ELEMENTAL, 13000, FOUNDRY_STAGE_TWO);
        scriptEvents.addEvent(EVENT_FURNACE_FLOOR_GUARD, 71500, FOUNDRY_STAGE_TWO);
        scriptEvents.addEvent(EVENT_FURNACE_FIRECALLERS, 76000, FOUNDRY_STAGE_TWO);
    }

    void enterFreedomStage()
    {
        mStage = FOUNDRY_STAGE_THREE;
        raidWarning("The Heart of the Mountain is free!");
        setContained(false);

        scriptEvents.addEvent(EVENT_FURNACE_HEAT, 6000, FOUNDRY_STAGE_THREE);
        scriptEvents.addEvent(EVENT_FURNACE_MELT, 11000, FOUNDRY_STAGE_THREE);

        if (Unit* target = pickPlayer(false))
            attackStart(target);
    }

    uint32_t mStage = FOUNDRY_STAGE_ONE;
    uint32_t mBlastInterval = 30000;
    uint32_t mCrewSide = 0;
    uint32_t mRegulatorsLeft = 0;
    uint32_t mElementalistsLeft = 0;
};

class FoundryHeatRegulatorAI : public FoundryCreatureAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryHeatRegulatorAI(c); }
    explicit FoundryHeatRegulatorAI(Creature* pCreature) : FoundryCreatureAI(pCreature) {}

    void OnLoad() override
    {
        setRooted(true);
        _setMeleeDisabled(true);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (CreatureAIScript* heart = getFoundryScript(BOSS_HEART_OF_THE_MOUNTAIN))
            heart->DoAction(ACTION_FURNACE_REGULATOR_LOST);
    }
};

class FoundryPrimalElementalistAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryPrimalElementalistAI(c); }
    explicit FoundryPrimalElementalistAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_ELEMENTALIST_REACTIVE_SHIELD, 8000, 15000, FoundryCastTarget::Self);
    }

    void OnLoad() override
    {
        setRooted(true);
    }

    void OnCombatStart(Unit* pTarget) override
    {
        FoundryRotationAI::OnCombatStart(pTarget);
        castSpellOnSelf(SPELL_ELEMENTALIST_DAMAGE_SHIELD, true);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (CreatureAIScript* heart = getFoundryScript(BOSS_HEART_OF_THE_MOUNTAIN))
            heart->DoAction(ACTION_FURNACE_ELEMENTALIST_LOST);
    }
};

class FoundryFurnaceEngineerAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryFurnaceEngineerAI(c); }
    explicit FoundryFurnaceEngineerAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_ENGINEER_ELECTROCUTION, 5000, 10000, FoundryCastTarget::AnyPlayer);
        addRotation(SPELL_ENGINEER_BOMB, 10000, 15000, FoundryCastTarget::AnyPlayer);
    }
};

class FoundrySecurityGuardAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundrySecurityGuardAI(c); }
    explicit FoundrySecurityGuardAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_SECURITY_GUARD_DEFENSE, 5000, 40000, FoundryCastTarget::Self);
    }
};

class FoundryBellowsOperatorAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryBellowsOperatorAI(c); }
    explicit FoundryBellowsOperatorAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_BELLOWS_OPERATOR_DEAFENING_ROAR, 7500, 20000, FoundryCastTarget::Area);
    }
};

class FoundrySlagElementalAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundrySlagElementalAI(c); }
    explicit FoundrySlagElementalAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_SLAG_ELEMENTAL_FIXATE, 2000, 30000, FoundryCastTarget::AnyPlayer);
        addRotation(SPELL_SLAG_ELEMENTAL_BURN, 5000, 5000, FoundryCastTarget::Area);
    }
};

class FoundryFirecallerAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryFirecallerAI(c); }
    explicit FoundryFirecallerAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_FIRECALLER_VOLATILE_FIRE, 5000, 15000, FoundryCastTarget::AnyPlayer);
        addRotation(SPELL_FIRECALLER_LAVA_BURST, 10000, 10000, FoundryCastTarget::AnyPlayer);
        addRotation(SPELL_FIRECALLER_CAUTERIZE_WOUNDS, 15000, 20000, FoundryCastTarget::WoundedAlly);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Kromog

class FoundryKromogAI : public FoundryEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryKromogAI(c); }
    explicit FoundryKromogAI(Creature* pCreature) : FoundryEncounterAI(pCreature, DATA_KROMOG) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        openEncounter();

        mFrenzied = false;
        castSpellOnSelf(SPELL_KROMOG_FISTS_OF_STONE, true);

        scriptEvents.addEvent(EVENT_KROMOG_STONE_BREATH, 9000);
        scriptEvents.addEvent(EVENT_KROMOG_WARPED_ARMOR, 14000);
        scriptEvents.addEvent(EVENT_KROMOG_SLAM, 18000);
        scriptEvents.addEvent(EVENT_KROMOG_RIPPLING_SMASH, 23500);
        scriptEvents.addEvent(EVENT_KROMOG_GRASPING_EARTH, 50000);
        scriptEvents.addEvent(EVENT_FOUNDRY_BERSERK, 540000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        failEncounter();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        closeEncounter();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), 0);

        if (!mFrenzied && _getHealthPercent() <= 30)
        {
            mFrenzied = true;
            castSpellOnSelf(SPELL_KROMOG_FRENZY, true);
        }

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_KROMOG_STONE_BREATH:
                castSpellOnSelf(SPELL_KROMOG_STONE_BREATH);
                scriptEvents.addEvent(EVENT_KROMOG_STONE_BREATH, 24000);
                break;
            case EVENT_KROMOG_WARPED_ARMOR:
                castSpellOnVictim(SPELL_KROMOG_WARPED_ARMOR, true);
                scriptEvents.addEvent(EVENT_KROMOG_WARPED_ARMOR, mFrenzied ? 10200 : 14000);
                break;
            case EVENT_KROMOG_SLAM:
                castSpellOnSelf(SPELL_KROMOG_SLAM);
                scriptEvents.addEvent(EVENT_KROMOG_SLAM, 23000);
                break;
            case EVENT_KROMOG_RIPPLING_SMASH:
                castSpellOnSelf(SPELL_KROMOG_RIPPLING_SMASH);
                scriptEvents.addEvent(EVENT_KROMOG_RIPPLING_SMASH, mFrenzied ? 18200 : 21000);
                break;
            case EVENT_KROMOG_GRASPING_EARTH:
                raidWarning("Kromog calls upon the Rune of Grasping Earth!");
                castSpellOnSelf(SPELL_KROMOG_GRASPING_EARTH);
                // nothing else overlaps the Thundering Blows window
                scriptEvents.addEvent(EVENT_KROMOG_THUNDERING_BLOWS, 12000);
                scriptEvents.addEvent(EVENT_KROMOG_STONE_BREATH, 31000);
                scriptEvents.addEvent(EVENT_KROMOG_RIPPLING_SMASH, 35000);
                scriptEvents.delayEvent(EVENT_KROMOG_SLAM, 31000);
                scriptEvents.delayEvent(EVENT_KROMOG_WARPED_ARMOR, 31000);
                scriptEvents.addEvent(EVENT_KROMOG_GRASPING_EARTH, 112000);
                break;
            case EVENT_KROMOG_THUNDERING_BLOWS:
                castSpellOnSelf(SPELL_KROMOG_THUNDERING_BLOWS);
                break;
            case EVENT_FOUNDRY_BERSERK:
                castSpellOnSelf(SPELL_FOUNDRY_BERSERK, true);
                break;
            default:
                break;
        }
    }

private:
    bool mFrenzied = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Iron Maidens

class FoundryIronMaidenAI : public FoundryEncounterAI
{
public:
    explicit FoundryIronMaidenAI(Creature* pCreature) : FoundryEncounterAI(pCreature, DATA_IRON_MAIDENS) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        openEncounter();

        mIronFury = 0;
        mFuryTimer = 0;
        mShipVisited = false;
        mAboard = false;
        mIronWill = false;

        armSignature();
        scriptEvents.addEvent(EVENT_MAIDEN_DREADNAUGHT, 60000);
        scriptEvents.addEvent(EVENT_FOUNDRY_BERSERK, 600000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        failEncounter();
        if (mAboard)
            leaveShip();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        closeEncounter();
    }

    uint32_t GetCreatureData(uint32_t type) const override
    {
        return (type == FOUNDRY_DATA_SHIP_VISITED && mShipVisited) ? 1 : 0;
    }

    void DoAction(int32_t action) override
    {
        if (action == ACTION_MAIDEN_IRON_WILL && !mIronWill)
        {
            mIronWill = true;
            castSpellOnSelf(SPELL_MAIDEN_IRON_WILL, true);
        }
        else if (action == ACTION_MAIDEN_BOARD_SHIP && !mAboard)
        {
            boardShip();
        }
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mAboard ? FOUNDRY_STAGE_TWO : FOUNDRY_STAGE_ONE);

        tickIronFury(static_cast<uint32_t>(time_passed));

        if (!mIronWill && _getHealthPercent() <= 20)
        {
            for (uint32_t entry : FoundryIronMaidenEntries)
            {
                if (CreatureAIScript* maiden = getFoundryScript(entry))
                    maiden->DoAction(ACTION_MAIDEN_IRON_WILL);
            }
        }

        if (_isCasting())
            return;

        uint32_t const eventId = scriptEvents.getFinishedEvent();
        switch (eventId)
        {
            case 0:
                break;
            case EVENT_MAIDEN_DREADNAUGHT:
                if (leadsTheMaidens())
                    sendSomeoneToShip();
                scriptEvents.addEvent(EVENT_MAIDEN_DREADNAUGHT, 198000);
                break;
            case EVENT_MAIDEN_BOMBARD:
                castAt(FoundryCastTarget::AnyPlayer, SPELL_MAIDEN_BOMBARDMENT_ALPHA);
                scriptEvents.addEvent(EVENT_MAIDEN_BOMBARD, 12000, FOUNDRY_STAGE_TWO);
                break;
            case EVENT_MAIDEN_LEAVE_SHIP:
                leaveShip();
                break;
            case EVENT_FOUNDRY_BERSERK:
                castSpellOnSelf(SPELL_FOUNDRY_BERSERK, true);
                break;
            default:
                handleSignatureEvent(eventId);
                break;
        }
    }

protected:
    virtual void armSignature() = 0;
    virtual void handleSignatureEvent(uint32_t eventId) = 0;
    virtual void reachFuryTier(uint32_t tier) = 0;

private:
    // one Iron Fury every 6.33s, abilities unlock at 30 and 100
    void tickIronFury(uint32_t diff)
    {
        if (mIronFury >= 100)
            return;

        mFuryTimer += diff;
        while (mFuryTimer >= 6333 && mIronFury < 100)
        {
            mFuryTimer -= 6333;
            ++mIronFury;

            if (mIronFury == 30)
                reachFuryTier(1);
            else if (mIronFury == 100)
                reachFuryTier(2);
        }
    }

    // first living maiden in table order runs the Dreadnaught schedule
    bool leadsTheMaidens()
    {
        for (uint32_t entry : FoundryIronMaidenEntries)
        {
            if (CreatureAIScript* maiden = getFoundryScript(entry))
                return maiden == this;
        }
        return false;
    }

    void sendSomeoneToShip()
    {
        CreatureAIScript* chosen = nullptr;
        for (uint32_t entry : FoundryIronMaidenEntries)
        {
            CreatureAIScript* maiden = getFoundryScript(entry);
            if (maiden == nullptr || maiden->GetCreatureData(FOUNDRY_DATA_SHIP_VISITED) != 0)
                continue;

            if (chosen == nullptr || maiden->_getHealthPercent() < chosen->_getHealthPercent())
                chosen = maiden;
        }

        if (chosen != nullptr)
            chosen->DoAction(ACTION_MAIDEN_BOARD_SHIP);
    }

    void boardShip()
    {
        mAboard = true;
        mShipVisited = true;

        raidWarning("An Iron Maiden heads for the Dreadnaught!");
        setReactState(REACT_PASSIVE);
        _setMeleeDisabled(true);
        addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        moveTo(FoundryDreadnaughtZipline.x, FoundryDreadnaughtZipline.y, FoundryDreadnaughtZipline.z);

        scriptEvents.addEvent(EVENT_MAIDEN_BOMBARD, 10000, FOUNDRY_STAGE_TWO);
        scriptEvents.addEvent(EVENT_MAIDEN_LEAVE_SHIP, 60000, FOUNDRY_STAGE_TWO);
    }

    void leaveShip()
    {
        mAboard = false;
        scriptEvents.removeEvent(EVENT_MAIDEN_BOMBARD);
        scriptEvents.removeEvent(EVENT_MAIDEN_LEAVE_SHIP);

        removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        _setMeleeDisabled(false);
        setReactState(REACT_AGGRESSIVE);

        if (_isInCombat())
        {
            if (Unit* target = pickPlayer(false))
                attackStart(target);
        }
    }

    uint32_t mIronFury = 0;
    uint32_t mFuryTimer = 0;
    bool mShipVisited = false;
    bool mAboard = false;
    bool mIronWill = false;
};

class FoundryGaranAI : public FoundryIronMaidenAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryGaranAI(c); }
    explicit FoundryGaranAI(Creature* pCreature) : FoundryIronMaidenAI(pCreature) {}

protected:
    void armSignature() override
    {
        scriptEvents.addEvent(EVENT_MAIDEN_SIGNATURE, 19000, FOUNDRY_STAGE_ONE);
    }

    void reachFuryTier(uint32_t tier) override
    {
        scriptEvents.addEvent(tier == 1 ? EVENT_MAIDEN_FURY_FIRST : EVENT_MAIDEN_FURY_SECOND, 1, FOUNDRY_STAGE_ONE);
    }

    void handleSignatureEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MAIDEN_SIGNATURE:
                castAt(FoundryCastTarget::NonTank, SPELL_GARAN_RAPID_FIRE);
                scriptEvents.addEvent(EVENT_MAIDEN_SIGNATURE, 30000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_MAIDEN_FURY_FIRST:
                castAt(FoundryCastTarget::Ranged, SPELL_GARAN_PENETRATING_SHOT);
                scriptEvents.addEvent(EVENT_MAIDEN_FURY_FIRST, 30000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_MAIDEN_FURY_SECOND:
                castSpellOnSelf(SPELL_GARAN_DEPLOY_TURRET, true);
                scriptEvents.addEvent(EVENT_MAIDEN_FURY_SECOND, 21300, FOUNDRY_STAGE_ONE);
                break;
            default:
                break;
        }
    }
};

class FoundrySorkaAI : public FoundryIronMaidenAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundrySorkaAI(c); }
    explicit FoundrySorkaAI(Creature* pCreature) : FoundryIronMaidenAI(pCreature) {}

protected:
    void armSignature() override
    {
        scriptEvents.addEvent(EVENT_MAIDEN_SIGNATURE, 11000, FOUNDRY_STAGE_ONE);
    }

    void reachFuryTier(uint32_t tier) override
    {
        scriptEvents.addEvent(tier == 1 ? EVENT_MAIDEN_FURY_FIRST : EVENT_MAIDEN_FURY_SECOND, 1, FOUNDRY_STAGE_ONE);
    }

    void handleSignatureEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MAIDEN_SIGNATURE:
                castAt(FoundryCastTarget::NonTank, SPELL_SORKA_BLADE_DASH);
                scriptEvents.addEvent(EVENT_MAIDEN_SIGNATURE, 20000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_MAIDEN_FURY_FIRST:
                castAt(FoundryCastTarget::AnyPlayer, SPELL_SORKA_CONVULSIVE_SHADOWS);
                scriptEvents.addEvent(EVENT_MAIDEN_FURY_FIRST, 56000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_MAIDEN_FURY_SECOND:
                castAt(FoundryCastTarget::AnyPlayer, SPELL_SORKA_DARK_HUNT);
                scriptEvents.addEvent(EVENT_MAIDEN_FURY_SECOND, 13500, FOUNDRY_STAGE_ONE);
                break;
            default:
                break;
        }
    }
};

class FoundryMarakAI : public FoundryIronMaidenAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryMarakAI(c); }
    explicit FoundryMarakAI(Creature* pCreature) : FoundryIronMaidenAI(pCreature) {}

protected:
    void armSignature() override
    {
        scriptEvents.addEvent(EVENT_MAIDEN_SIGNATURE, 5000, FOUNDRY_STAGE_ONE);
    }

    void reachFuryTier(uint32_t tier) override
    {
        // the second tier is a permanent buff
        if (tier == 1)
            scriptEvents.addEvent(EVENT_MAIDEN_FURY_FIRST, 1, FOUNDRY_STAGE_ONE);
        else
            castSpellOnSelf(SPELL_MARAK_SANGUINE_STRIKES, true);
    }

    void handleSignatureEvent(uint32_t eventId) override
    {
        switch (eventId)
        {
            case EVENT_MAIDEN_SIGNATURE:
                castAt(FoundryCastTarget::NonTank, SPELL_MARAK_BLOOD_RITUAL);
                scriptEvents.addEvent(EVENT_MAIDEN_SIGNATURE, 20000, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_MAIDEN_FURY_FIRST:
                castAt(FoundryCastTarget::AnyPlayer, SPELL_MARAK_BLOODSOAKED_HEARTSEEKER);
                scriptEvents.addEvent(EVENT_MAIDEN_FURY_FIRST, 70000, FOUNDRY_STAGE_ONE);
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Blackhand

class FoundryBlackhandAI : public FoundryEncounterAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryBlackhandAI(c); }
    explicit FoundryBlackhandAI(Creature* pCreature) : FoundryEncounterAI(pCreature, DATA_BLACKHAND) {}

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        openEncounter();

        mStage = FOUNDRY_STAGE_ONE;
        mSoldierSide = 0;
        mSiegemakerSlot = 0;

        scriptEvents.addEvent(EVENT_BLACKHAND_SLAG_BOMBS, 6000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_BLACKHAND_DEMOLITION, 15500, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_BLACKHAND_SHATTERING_SMASH, 30000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_BLACKHAND_IMPALING_THROW, 36000, FOUNDRY_STAGE_ONE);
        scriptEvents.addEvent(EVENT_FOUNDRY_BERSERK, 600000);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        failEncounter();
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        closeEncounter();
    }

    void AIUpdate(unsigned long time_passed) override
    {
        scriptEvents.updateEvents(static_cast<uint32_t>(time_passed), mStage);

        if (mStage == FOUNDRY_STAGE_ONE && _getHealthPercent() <= 70)
            enterStorageWarehouse();
        else if (mStage == FOUNDRY_STAGE_TWO && _getHealthPercent() <= 30)
            enterIronCrucible();

        if (_isCasting())
            return;

        switch (scriptEvents.getFinishedEvent())
        {
            case EVENT_BLACKHAND_SLAG_BOMBS:
                if (mStage == FOUNDRY_STAGE_THREE)
                {
                    castSpellOnSelf(SPELL_BLACKHAND_ATTACH_SLAG_BOMBS, true);
                    castSpellOnVictim(SPELL_BLACKHAND_SLAG_BOMBS_TANK, true);
                }
                else
                {
                    castSpellOnSelf(SPELL_BLACKHAND_THROW_SLAG_BOMBS, true);
                }
                scriptEvents.addEvent(EVENT_BLACKHAND_SLAG_BOMBS, 25000, mStage);
                break;
            case EVENT_BLACKHAND_DEMOLITION:
                raidWarning("Blackhand brings down the ceiling with Demolition!");
                castSpellAOE(SPELL_BLACKHAND_DEMOLITION);
                scriptEvents.addEvent(EVENT_BLACKHAND_DEMOLITION, 45500, FOUNDRY_STAGE_ONE);
                break;
            case EVENT_BLACKHAND_SHATTERING_SMASH:
                if (mStage == FOUNDRY_STAGE_THREE)
                {
                    raidWarning("Blackhand prepares a Massive Shattering Smash!");
                    castSpellOnVictim(SPELL_BLACKHAND_MASSIVE_SMASH);
                    scriptEvents.addEvent(EVENT_BLACKHAND_SHATTERING_SMASH, 25000, FOUNDRY_STAGE_THREE);
                }
                else
                {
                    castSpellOnVictim(SPELL_BLACKHAND_SHATTERING_SMASH);
                    scriptEvents.addEvent(EVENT_BLACKHAND_SHATTERING_SMASH, mStage == FOUNDRY_STAGE_ONE ? 30000 : 44000, mStage);
                }
                break;
            case EVENT_BLACKHAND_IMPALING_THROW:
                castAt(FoundryCastTarget::Ranged, SPELL_BLACKHAND_IMPALING_THROW);
                scriptEvents.addEvent(EVENT_BLACKHAND_IMPALING_THROW, 16000, mStage);
                break;
            case EVENT_BLACKHAND_SIEGEMAKER:
                summonFighter(NPC_SIEGEMAKER, ringPosition(mSiegemakerSlot++));
                scriptEvents.addEvent(EVENT_BLACKHAND_SIEGEMAKER, 50000, FOUNDRY_STAGE_TWO);
                break;
            case EVENT_BLACKHAND_IRON_SOLDIERS:
                // six soldiers, alternating between two sides of the ring
                for (uint32_t i = 0; i < 6; ++i)
                    summonFighter(NPC_IRON_SOLDIER, ringPosition(mSoldierSide + i * 2));
                mSoldierSide ^= 1;
                scriptEvents.addEvent(EVENT_BLACKHAND_IRON_SOLDIERS, 30000, FOUNDRY_STAGE_TWO);
                break;
            case EVENT_BLACKHAND_SLAG_ERUPTION:
                castSpellAOE(SPELL_BLACKHAND_SLAG_ERUPTION);
                scriptEvents.addEvent(EVENT_BLACKHAND_SLAG_ERUPTION, 33000, FOUNDRY_STAGE_THREE);
                break;
            case EVENT_FOUNDRY_BERSERK:
                castSpellOnSelf(SPELL_FOUNDRY_BERSERK, true);
                break;
            default:
                break;
        }
    }

private:
    void enterStorageWarehouse()
    {
        mStage = FOUNDRY_STAGE_TWO;
        raidWarning("Blackhand moves the fight into the Storage Warehouse!");

        scriptEvents.removeEvent(EVENT_BLACKHAND_DEMOLITION);
        scriptEvents.addEvent(EVENT_BLACKHAND_IRON_SOLDIERS, 11000, FOUNDRY_STAGE_TWO);
        scriptEvents.addEvent(EVENT_BLACKHAND_SLAG_BOMBS, 12000, FOUNDRY_STAGE_TWO);
        scriptEvents.addEvent(EVENT_BLACKHAND_SIEGEMAKER, 16000, FOUNDRY_STAGE_TWO);
        scriptEvents.addEvent(EVENT_BLACKHAND_IMPALING_THROW, 26000, FOUNDRY_STAGE_TWO);
        scriptEvents.addEvent(EVENT_BLACKHAND_SHATTERING_SMASH, 44000, FOUNDRY_STAGE_TWO);
    }

    void enterIronCrucible()
    {
        mStage = FOUNDRY_STAGE_THREE;
        raidWarning("Blackhand descends into the Iron Crucible!");

        summons.despawnAll();
        scriptEvents.removeEvent(EVENT_BLACKHAND_SIEGEMAKER);
        scriptEvents.removeEvent(EVENT_BLACKHAND_IRON_SOLDIERS);
        scriptEvents.addEvent(EVENT_BLACKHAND_SLAG_BOMBS, 12000, FOUNDRY_STAGE_THREE);
        scriptEvents.addEvent(EVENT_BLACKHAND_IMPALING_THROW, 16000, FOUNDRY_STAGE_THREE);
        scriptEvents.addEvent(EVENT_BLACKHAND_SHATTERING_SMASH, 25000, FOUNDRY_STAGE_THREE);
        scriptEvents.addEvent(EVENT_BLACKHAND_SLAG_ERUPTION, 31500, FOUNDRY_STAGE_THREE);
    }

    uint32_t mStage = FOUNDRY_STAGE_ONE;
    uint32_t mSoldierSide = 0;
    uint32_t mSiegemakerSlot = 0;
};

class FoundrySiegemakerAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundrySiegemakerAI(c); }
    explicit FoundrySiegemakerAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_SIEGEMAKER_FIXATE, 1000, 20000, FoundryCastTarget::AnyPlayer);
        addRotation(SPELL_SIEGEMAKER_MORTAR, 10000, 12000, FoundryCastTarget::Ranged);
    }

    void OnCombatStart(Unit* pTarget) override
    {
        FoundryRotationAI::OnCombatStart(pTarget);
        castSpellOnSelf(SPELL_SIEGEMAKER_BLACKIRON_PLATING, true);
        castSpellOnSelf(SPELL_SIEGEMAKER_BATTERING_RAM, true);
    }
};

class FoundryIronSoldierAI : public FoundryRotationAI
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FoundryIronSoldierAI(c); }
    explicit FoundryIronSoldierAI(Creature* pCreature) : FoundryRotationAI(pCreature)
    {
        addRotation(SPELL_IRON_SOLDIER_EXPLOSIVE_ROUND, 3000, 6000, FoundryCastTarget::AnyPlayer);
    }

    void OnLoad() override
    {
        setRooted(true);
    }
};

void SetupBlackrockFoundry(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKROCK_FOUNDRY, &BlackrockFoundryInstanceScript::Create);

    // Gruul
    mgr->register_creature_script(BOSS_GRUUL, &FoundryGruulAI::Create);
    mgr->register_creature_script(NPC_CAVE_IN, &FoundryCaveInAI::Create);

    // Oregorger
    mgr->register_creature_script(BOSS_OREGORGER, &FoundryOregorgerAI::Create);

    // Beastlord Darmac
    mgr->register_creature_script(BOSS_BEASTLORD_DARMAC, &FoundryDarmacAI::Create);
    mgr->register_creature_script(NPC_CRUELFANG, &FoundryCruelfangAI::Create);
    mgr->register_creature_script(NPC_DREADWING, &FoundryDreadwingAI::Create);
    mgr->register_creature_script(NPC_IRONCRUSHER, &FoundryIroncrusherAI::Create);

    // Flamebender Ka'graz
    mgr->register_creature_script(BOSS_FLAMEBENDER_KAGRAZ, &FoundryKagrazAI::Create);
    mgr->register_creature_script(NPC_AKNOR_STEELBRINGER, &FoundryAknorAI::Create);
    mgr->register_creature_script(NPC_CINDER_WOLF, &FoundryCinderWolfAI::Create);

    // Hans'gar and Franzok
    mgr->register_creature_script(BOSS_HANSGAR, &FoundryHansgarAI::Create);
    mgr->register_creature_script(BOSS_FRANZOK, &FoundryFranzokAI::Create);

    // Operator Thogar
    mgr->register_creature_script(BOSS_OPERATOR_THOGAR, &FoundryThogarAI::Create);
    mgr->register_creature_script(NPC_THOGAR_IRON_RAIDER, &FoundryThogarIronRaiderAI::Create);
    mgr->register_creature_script(NPC_THOGAR_IRON_CRACK_SHOT, &FoundryThogarCrackShotAI::Create);
    mgr->register_creature_script(NPC_THOGAR_GROMKAR_FIREMENDER, &FoundryThogarFiremenderAI::Create);
    mgr->register_creature_script(NPC_THOGAR_GROMKAR_MAN_AT_ARMS, &FoundryThogarManAtArmsAI::Create);

    // The Blast Furnace
    mgr->register_creature_script(BOSS_FOREMAN_FELDSPAR, &FoundryFeldsparAI::Create);
    mgr->register_creature_script(BOSS_HEART_OF_THE_MOUNTAIN, &FoundryHeartOfTheMountainAI::Create);
    mgr->register_creature_script(NPC_HEAT_REGULATOR, &FoundryHeatRegulatorAI::Create);
    mgr->register_creature_script(NPC_PRIMAL_ELEMENTALIST, &FoundryPrimalElementalistAI::Create);
    mgr->register_creature_script(NPC_FURNACE_ENGINEER, &FoundryFurnaceEngineerAI::Create);
    mgr->register_creature_script(NPC_SECURITY_GUARD, &FoundrySecurityGuardAI::Create);
    mgr->register_creature_script(NPC_BELLOWS_OPERATOR, &FoundryBellowsOperatorAI::Create);
    mgr->register_creature_script(NPC_SLAG_ELEMENTAL, &FoundrySlagElementalAI::Create);
    mgr->register_creature_script(NPC_FIRECALLER, &FoundryFirecallerAI::Create);

    // Kromog
    mgr->register_creature_script(BOSS_KROMOG, &FoundryKromogAI::Create);

    // The Iron Maidens
    mgr->register_creature_script(BOSS_ADMIRAL_GARAN, &FoundryGaranAI::Create);
    mgr->register_creature_script(BOSS_ENFORCER_SORKA, &FoundrySorkaAI::Create);
    mgr->register_creature_script(BOSS_MARAK_THE_BLOODED, &FoundryMarakAI::Create);

    // Blackhand
    mgr->register_creature_script(BOSS_BLACKHAND, &FoundryBlackhandAI::Create);
    mgr->register_creature_script(NPC_SIEGEMAKER, &FoundrySiegemakerAI::Create);
    mgr->register_creature_script(NPC_IRON_SOLDIER, &FoundryIronSoldierAI::Create);
}
