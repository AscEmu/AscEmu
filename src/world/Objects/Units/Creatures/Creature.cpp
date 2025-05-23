/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Objects/DynamicObject.hpp"
#include "Management/AuctionMgr.hpp"
#include "Management/QuestMgr.h"
#include "Management/QuestProperties.hpp"
#include "Management/GameEvent.hpp"
#include "Management/Skill.hpp"
#include "Management/Battleground/Battleground.hpp"
#include "Management/Loot/LootMgr.hpp"
#include "Management/Loot/LootRoll.hpp"
#include "Objects/Units/Stats.h"
#include "Storage/MySQLDataStore.hpp"
#include "Map/Cells/MapCell.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Pet.h"
#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Storage/MySQLStructures.h"
#include "Management/ObjectMgr.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Movement/MovementManager.h"
#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Objects/Units/Creatures/CreatureGroups.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/Definitions.h"
#include "Server/EventMgr.h"
#include "Server/Opcodes.hpp"
#include "Server/World.h"
#include "Server/Script/EventScript.hpp"
#include "Server/Script/HookInterface.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Summons/SummonHandler.hpp"
#include "Utilities/Random.hpp"

uint8_t CreatureProperties::generateRandomDisplayIdAndReturnGender(uint32_t* displayId) const
{
    if (isTriggerNpc)
    {
        // Trigger npcs should load their invisible model
        *displayId = getInvisibleModelForTriggerNpc();
        return 0;
    }

    const uint32_t models[] = { Male_DisplayID, Male_DisplayID2, Female_DisplayID, Female_DisplayID2 };
    if (!models[0] && !models[1] && !models[2] && !models[3])
    {
        // All models are invalid
        sLogger.failure("CreatureProperties : All display IDs are invalid for creature entry {}", Id);
        return 0;
    }

    while (true)
    {
        const auto res = Util::getRandomUInt(3);
        if (models[res])
        {
            *displayId = models[res];
            return res < 2 ? 0U : 1U;
        }
    }
}

uint32_t CreatureProperties::getRandomModelId() const
{
    if (isTriggerNpc)
    {
        // Trigger npcs should load their invisible model
        return getInvisibleModelForTriggerNpc();
    }

    std::vector<uint32_t> modelIds;

    if (Male_DisplayID)
        modelIds.push_back(Male_DisplayID);
    if (Male_DisplayID2)
        modelIds.push_back(Male_DisplayID2);
    if (Female_DisplayID)
        modelIds.push_back(Female_DisplayID);
    if (Female_DisplayID2)
        modelIds.push_back(Female_DisplayID2);

    if (modelIds.empty())
    {
        sLogger.failure("CreatureProperties : All display IDs are invalid for creature entry {}", Id);
        return 0;
    }

    Util::randomShuffleVector(&modelIds);
    return modelIds.front();
}

uint32_t CreatureProperties::getInvisibleModelForTriggerNpc() const
{
    const auto* displayInfo = sObjectMgr.getCreatureDisplayInfoData(Male_DisplayID);
    if (displayInfo != nullptr && displayInfo->isModelInvisibleStalker)
        return Male_DisplayID;

    displayInfo = sObjectMgr.getCreatureDisplayInfoData(Male_DisplayID2);
    if (displayInfo != nullptr && displayInfo->isModelInvisibleStalker)
        return Male_DisplayID2;

    displayInfo = sObjectMgr.getCreatureDisplayInfoData(Female_DisplayID);
    if (displayInfo != nullptr && displayInfo->isModelInvisibleStalker)
        return Female_DisplayID;

    displayInfo = sObjectMgr.getCreatureDisplayInfoData(Female_DisplayID2);
    if (displayInfo != nullptr && displayInfo->isModelInvisibleStalker)
        return Female_DisplayID2;

    // Incase invisible model not found, return default trigger invisible model
    return 11686;
}

uint32_t CreatureProperties::getVisibleModelForTriggerNpc() const
{
    const auto* displayInfo = sObjectMgr.getCreatureDisplayInfoData(Male_DisplayID);
    if (displayInfo != nullptr && !displayInfo->isModelInvisibleStalker)
        return Male_DisplayID;

    displayInfo = sObjectMgr.getCreatureDisplayInfoData(Male_DisplayID2);
    if (displayInfo != nullptr && !displayInfo->isModelInvisibleStalker)
        return Male_DisplayID2;

    displayInfo = sObjectMgr.getCreatureDisplayInfoData(Female_DisplayID);
    if (displayInfo != nullptr && !displayInfo->isModelInvisibleStalker)
        return Female_DisplayID;

    displayInfo = sObjectMgr.getCreatureDisplayInfoData(Female_DisplayID2);
    if (displayInfo != nullptr && !displayInfo->isModelInvisibleStalker)
        return Female_DisplayID2;

    // Incase visible model not found, return default trigger visible model
    return 17519;
}

bool CreatureProperties::isExotic() const
{
    return (typeFlags & CREATURE_FLAG1_EXOTIC) != 0;
}

Creature::Creature(uint64_t guid)
{
    //////////////////////////////////////////////////////////////////////////
    m_objectTypeId = TYPEID_UNIT;
    m_valuesCount = getSizeOfStructure(WoWUnit);
    //////////////////////////////////////////////////////////////////////////

    //\todo Why is there a pointer to the same thing in a derived class? ToDo: sort this out..
    m_uint32Values = _fields;

    memset(m_uint32Values, 0, ((getSizeOfStructure(WoWUnit)) * sizeof(uint32_t)));
    m_updateMask.SetCount(getSizeOfStructure(WoWUnit));

    setOType(TYPE_UNIT | TYPE_OBJECT);
    setGuid(guid);

    setAttackPowerMultiplier(0.0f);
    setRangedAttackPowerMultiplier(0.0f);

    //override class "Unit" initialisation
    setAItoUse(true);

    // Override initialization from Unit class
    getThreatManager().initialize();
}

Creature::~Creature()
{
    sEventMgr.RemoveEvents(this);

    if (_myScriptClass)
    {
        _myScriptClass->Destroy();
        _myScriptClass = nullptr;
    }

    if (m_respawnCell)
        m_respawnCell->_respawnObjects.erase(this);

    if (m_escorter)
        m_escorter = nullptr;
}

bool Creature::isVendor() const { return getNpcFlags() & UNIT_NPC_FLAG_VENDOR; }
bool Creature::isTrainer() const { return getNpcFlags() & UNIT_NPC_FLAG_TRAINER; }
bool Creature::isClassTrainer() const { return getNpcFlags() & UNIT_NPC_FLAG_TRAINER_CLASS; }
bool Creature::isProfessionTrainer() const { return getNpcFlags() & UNIT_NPC_FLAG_TRAINER_PROFESSION; }
bool Creature::isQuestGiver() const { return getNpcFlags() & UNIT_NPC_FLAG_QUESTGIVER; }
bool Creature::isGossip() const{ return getNpcFlags() & UNIT_NPC_FLAG_GOSSIP; }
bool Creature::isTaxi() const { return getNpcFlags() & UNIT_NPC_FLAG_TAXI; }
bool Creature::isCharterGiver() const { return getNpcFlags() & UNIT_NPC_FLAG_CHARTERGIVER; }
bool Creature::isGuildBank() const { return getNpcFlags() & UNIT_NPC_FLAG_GUILD_BANKER; }
bool Creature::isBattleMaster() const { return getNpcFlags() & UNIT_NPC_FLAG_BATTLEMASTER; }
bool Creature::isBanker() const { return getNpcFlags() & UNIT_NPC_FLAG_BANKER; }
bool Creature::isInnkeeper() const { return getNpcFlags() & UNIT_NPC_FLAG_INNKEEPER; }
bool Creature::isSpiritHealer() const { return getNpcFlags() & UNIT_NPC_FLAG_SPIRITHEALER; }
bool Creature::isTabardDesigner() const { return getNpcFlags() & UNIT_NPC_FLAG_TABARDDESIGNER; }
bool Creature::isAuctioneer() const { return getNpcFlags() & UNIT_NPC_FLAG_AUCTIONEER; }
bool Creature::isStableMaster() const { return getNpcFlags() & UNIT_NPC_FLAG_STABLEMASTER; }
bool Creature::isArmorer() const { return getNpcFlags() & UNIT_NPC_FLAG_ARMORER; }
#if VERSION_STRING >= Cata
bool Creature::isTransmog() const { return getNpcFlags() & UNIT_NPC_FLAG_TRANSMOGRIFIER; }
bool Creature::isReforger() const { return getNpcFlags() & UNIT_NPC_FLAG_REFORGER; }
bool Creature::isVoidStorage() const { return getNpcFlags() & UNIT_NPC_FLAG_VOID_STORAGE; }
#endif

bool Creature::isVehicle() const
{
    return creature_properties->vehicleid != 0;
}

bool Creature::isTrainingDummy()
{
    return creature_properties->isTrainingDummy;
}

bool Creature::isPvpFlagSet() const
{
#if VERSION_STRING > TBC
    return getPvpFlags() & PVP_STATE_FLAG_PVP;
#else
    return getUnitFlags() & UNIT_FLAG_PVP;
#endif
}

void Creature::setPvpFlag()
{
#if VERSION_STRING > TBC
    addPvpFlags(PVP_STATE_FLAG_PVP);
#else
    addUnitFlags(UNIT_FLAG_PVP);
#endif
    getSummonInterface()->setPvPFlags(true);
}

void Creature::removePvpFlag()
{
#if VERSION_STRING > TBC
    removePvpFlags(PVP_STATE_FLAG_PVP);
#else
    removeUnitFlags(UNIT_FLAG_PVP);
#endif
    getSummonInterface()->setPvPFlags(false);
}

bool Creature::isFfaPvpFlagSet() const
{
#if VERSION_STRING > TBC
    return getPvpFlags() & PVP_STATE_FLAG_FFA_PVP;
#else
    return false;
#endif
}

void Creature::setFfaPvpFlag()
{
#if VERSION_STRING > TBC
    addPvpFlags(PVP_STATE_FLAG_FFA_PVP);
#endif
    getSummonInterface()->setFFAPvPFlags(true);
}

void Creature::removeFfaPvpFlag()
{
#if VERSION_STRING > TBC
    removePvpFlags(PVP_STATE_FLAG_FFA_PVP);
#endif
    getSummonInterface()->setFFAPvPFlags(false);
}

bool Creature::isSanctuaryFlagSet() const
{
#if VERSION_STRING > TBC
    return getPvpFlags() & PVP_STATE_FLAG_SANCTUARY;
#else
    return false;
#endif
}

void Creature::setSanctuaryFlag()
{
#if VERSION_STRING > TBC
    addPvpFlags(PVP_STATE_FLAG_SANCTUARY);
#endif
    getSummonInterface()->setSanctuaryFlags(true);
}

void Creature::removeSanctuaryFlag()
{
#if VERSION_STRING > TBC
    removePvpFlags(PVP_STATE_FLAG_SANCTUARY);
#endif
    getSummonInterface()->setSanctuaryFlags(false);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Owner
Unit* Creature::getUnitOwner()
{
    if (getCharmedByGuid() != 0)
        return getWorldMapUnit(getCharmedByGuid());

    return nullptr;
}

Unit const* Creature::getUnitOwner() const
{
    if (getCharmedByGuid() != 0)
        return getWorldMapUnit(getCharmedByGuid());

    return nullptr;
}

Unit* Creature::getUnitOwnerOrSelf()
{
    if (auto* const unitOwner = getUnitOwner())
        return unitOwner;

    return this;
}

Unit const* Creature::getUnitOwnerOrSelf() const
{
    if (auto* const unitOwner = getUnitOwner())
        return unitOwner;

    return this;
}

Player* Creature::getPlayerOwner()
{
    if (getCharmedByGuid() != 0)
        return getWorldMapPlayer(getCharmedByGuid());

    return nullptr;
}

Player const* Creature::getPlayerOwner() const
{
    if (getCharmedByGuid() != 0)
        return getWorldMapPlayer(getCharmedByGuid());

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
float_t Creature::getMaxWanderDistance() const
{
    return m_wanderDistance;
}

void Creature::setMaxWanderDistance(float_t dist)
{
    m_wanderDistance = dist;
}

#if VERSION_STRING < WotLK
uint32_t Creature::getVirtualItemEntry(uint8_t slot) const
{
    if (slot >= TOTAL_WEAPON_DAMAGE_TYPES)
        return 0;

    return m_virtualItemEntry[slot];
}

void Creature::setVirtualItemEntry(uint8_t slot, uint32_t itemId)
{
    if (slot >= TOTAL_WEAPON_DAMAGE_TYPES)
        return;

    m_virtualItemEntry[slot] = itemId;
}
#endif

void Creature::toggleDualwield(bool enable)
{
    setDualWield(enable);
    if (enable)
    {
        setBaseAttackTime(OFFHAND, getBaseAttackTime(MELEE));
        // Creatures deal 50% damage on offhand hits
        setMinOffhandDamage(getMinDamage() * 0.5f);
        setMaxOffhandDamage(getMaxDamage() * 0.5f);
    }
    else
    {
        setBaseAttackTime(OFFHAND, 0);
        setMinOffhandDamage(0.0f);
        setMaxOffhandDamage(0.0f);
    }
}

std::vector<CreatureItem>* Creature::getSellItems()
{
    return m_SellItems;
}

void Creature::setDeathState(DeathState s)
{
    Unit::setDeathState(s);

    if (s == ALIVE)
        this->removeUnitFlags(UNIT_FLAG_DEAD);

    if (s == JUST_DIED)
    {
        // Respawn Handling
        const auto now = Util::getTimeNow();

        m_corpseRemoveTime = now + m_corpseDelay;

        uint32_t respawnDelay = m_respawnDelay;

        if (isDungeonBoss() && !m_respawnDelay)
            m_respawnTime = std::numeric_limits<time_t>::max(); // never respawn in this instance
        else
            m_respawnTime = now + respawnDelay + m_corpseDelay;

        saveRespawnTime();

        setTargetGuid(0);

        setUnitFlags(UNIT_NPC_FLAG_NONE);

        setMountDisplayId(0);

        getAIInterface()->setNoSearchAssistance(false);

        if (m_enslaveSpell)
            RemoveEnslave();

        //Dismiss group if is leader
        if (m_formation && m_formation->getLeader() == this)
            m_formation->formationReset(true);

        for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
            interruptSpellWithSpellType(static_cast<CurrentSpellType>(i));

        // if it's not a Pet, and not a summon and it has skinningloot then we will allow skinning
        if (getCreatedByGuid() == 0 && getSummonedByGuid() == 0 && sLootMgr.isSkinnable(creature_properties->Id))
            addUnitFlags(UNIT_FLAG_SKINNABLE);

        bool needsFalling = (IsFlying() || isHovering()) && !isUnderWater();
        setMoveHover(false);
        setMoveDisableGravity(false);

        if (needsFalling)
            getMovementManager()->moveFall();

        Unit::setDeathState(CORPSE);
    }
    else if (s == JUST_RESPAWNED)
    {
        if (isPet())
        {
            setFullHealth();
        }
        else
        {
            uint32_t curhealth = getMaxHealth();
            setPower(POWER_TYPE_MANA, getMaxPower(POWER_TYPE_MANA));
            setHealth((m_deathState == ALIVE || m_deathState == JUST_RESPAWNED) ? curhealth : 0);
        }

        setTaggerGuid(0);

        getAIInterface()->setCannotReachTarget(false);
        updateMovementFlags();

        removeUnitStateFlag(UNIT_STATE_ALL_ERASABLE);

        OnRespawn(getWorldMap());

        motion_Initialize();
        Unit::setDeathState(ALIVE);
    }
}

void Creature::addToInRangeObjects(Object* pObj)
{
    Unit::addToInRangeObjects(pObj);
}

void Creature::onRemoveInRangeObject(Object* pObj)
{
    if (m_escorter == pObj)
    {
        stopMoving();

        m_escorter = nullptr;
        Despawn(1000, 1000);
    }

    Unit::onRemoveInRangeObject(pObj);
}

void Creature::clearInRangeSets()
{
    Unit::clearInRangeSets();
}

//! brief: used to generate gossip menu based on db values from table gossip_menu, gossip_menu_item and gossip_menu_option. WIP.
class DatabaseGossip : public GossipScript
{
    uint32_t m_gossipMenuId;

public:
    DatabaseGossip(uint32_t gossipId) : m_gossipMenuId(gossipId) {}

    void onHello(Object* object, Player* player) override
    {
        sObjectMgr.generateDatabaseGossipMenu(object, m_gossipMenuId, player);
    }

    void onSelectOption(Object* object, Player* player, uint32_t gossipItemId, const char* /*Code*/, uint32_t gossipMenuId) override
    {
        if (gossipItemId > 0)
        {
            if (gossipMenuId != 0)
                sObjectMgr.generateDatabaseGossipOptionAndSubMenu(object, player, gossipItemId, gossipMenuId);
            else
                sObjectMgr.generateDatabaseGossipOptionAndSubMenu(object, player, gossipItemId, m_gossipMenuId);
        }
    }
};

void Creature::registerDatabaseGossip()
{
    if (isGossip())
    {
        if (GetCreatureProperties()->gossipId)
        {
            sScriptMgr.register_creature_gossip(getEntry(), new DatabaseGossip(GetCreatureProperties()->gossipId));
        }
        else
        {
            if (isSpiritHealer())
                sScriptMgr.register_creature_gossip(getEntry(), new GossipSpiritHealer());

            if (isInnkeeper())
                sScriptMgr.register_creature_gossip(getEntry(), new GossipInnKeeper());

            if (isBanker())
                sScriptMgr.register_creature_gossip(getEntry(), new GossipBanker());

            if (isClassTrainer())
                sScriptMgr.register_creature_gossip(getEntry(), new GossipClassTrainer());

            if (isTrainer())
            {
                if (const auto trainer = GetTrainer())
                {
                    if (trainer->TrainerType == TRAINER_TYPE_PET)
                        sScriptMgr.register_creature_gossip(getEntry(), new GossipPetTrainer());
                    else
                        sScriptMgr.register_creature_gossip(getEntry(), new GossipTrainer());
                }
            }
            else if (isTabardDesigner())
                sScriptMgr.register_creature_gossip(getEntry(), new GossipTabardDesigner());
            else if (isTaxi())
                sScriptMgr.register_creature_gossip(getEntry(), new GossipFlightMaster());
            else if (isStableMaster())
                sScriptMgr.register_creature_gossip(getEntry(), new GossipStableMaster());
            else if (isBattleMaster())
                sScriptMgr.register_creature_gossip(getEntry(), new GossipBattleMaster());
            else if (isAuctioneer())
                sScriptMgr.register_creature_gossip(getEntry(), new GossipAuctioneer());
            else if (isCharterGiver())
                sScriptMgr.register_creature_gossip(getEntry(), new GossipCharterGiver());
            else if (isVendor())
                sScriptMgr.register_creature_gossip(getEntry(), new GossipVendor());

            sScriptMgr.register_creature_gossip(getEntry(), new GossipGeneric());
        }
    }
}

bool Creature::isReturningHome() const
{
    if (getMovementManager()->getCurrentMovementGeneratorType() == HOME_MOTION_TYPE)
        return true;

    return false;
}

void Creature::searchFormation()
{
    if (isSummon())
        return;

    uint32_t lowguid = getSpawnId();
    if (!lowguid)
        return;

    if (FormationInfo const* formationInfo = sFormationMgr->getFormationInfo(lowguid))
        sFormationMgr->addCreatureToGroup(formationInfo->LeaderSpawnId, this);
}

bool Creature::isFormationLeader() const
{
    if (!m_formation)
        return false;

    return m_formation->isLeader(this);
}

void Creature::signalFormationMovement()
{
    if (!m_formation)
        return;

    if (!m_formation->isLeader(this))
        return;

    m_formation->leaderStartedMoving();
}

bool Creature::isFormationLeaderMoveAllowed() const
{
    if (!m_formation)
        return false;

    return m_formation->canLeaderStartMoving();
}

void Creature::motion_Initialize()
{
    if (m_formation)
    {
        if (m_formation->getLeader() == this)
            m_formation->formationReset(false);
        else if (m_formation->isFormed())
        {
            getMovementManager()->moveIdle(); // wait the order of leader
            return;
        }
    }

    getMovementManager()->initialize();
}

void Creature::immediateMovementFlagsUpdate()
{
    updateMovementFlags();
    // reset timer
    m_movementFlagUpdateTimer = 1000;
}

void Creature::updateMovementFlags()
{
    // Do not update movement flags if creature is controlled by a player (charm/vehicle)
    if (getUnitOwner())
        return;

    // Set the movement flags if the creature is in that mode. (Only fly if actually in air, only swim if in water, etc)
    const float ground = getFloorZ();

    bool isInAir;

#if VERSION_STRING < WotLK
    isInAir = (G3D::fuzzyGt(GetPositionZ(), ground + GROUND_HEIGHT_TOLERANCE) || G3D::fuzzyLt(GetPositionZ(), ground - GROUND_HEIGHT_TOLERANCE)); // Can be underground too, prevent the falling
#else
    isInAir = (G3D::fuzzyGt(GetPositionZ(), ground + (canHover() ? getHoverHeight() : 0.0f) + GROUND_HEIGHT_TOLERANCE) || G3D::fuzzyLt(GetPositionZ(), ground - GROUND_HEIGHT_TOLERANCE)); // Can be underground too, prevent the falling
#endif

    if (isInAir && !IsFalling())
    {
        auto needsFalling = false;
        if (getMovementTemplate().isFlightAllowed())
        {
            if (isAlive())
            {
                if (getMovementTemplate().Flight == CreatureFlightMovementType::CanFly)
                {
                    if (!hasUnitMovementFlag(MOVEFLAG_CAN_FLY))
                        setMoveCanFly(true);
                }
                else
                {
                    if (!hasUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY))
                        setMoveDisableGravity(true);
                }

                if (isHovering() && hasAuraWithAuraEffect(SPELL_AURA_HOVER))
                    setMoveHover(false);
            }
            else
            {
                if (hasUnitMovementFlag(MOVEFLAG_CAN_FLY))
                    setMoveCanFly(false);

                if (hasUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY))
                    setMoveDisableGravity(false);

                // Unit is in air and is dead => needs to fall down
                needsFalling = true;
            }
        }

        if (isHovering() && (!hasAuraWithAuraEffect(SPELL_AURA_HOVER) || !isAlive()))
        {
            setMoveHover(false);
            needsFalling = true;
        }

        // Make unit fall
        if (needsFalling && !isUnderWater())
            getMovementManager()->moveFall();
    }

    if (!isInAir)
    {
        removeUnitMovementFlag(MOVEFLAG_FALLING);

        if (hasUnitMovementFlag(MOVEFLAG_CAN_FLY))
            setMoveCanFly(false);

        if (hasUnitMovementFlag(MOVEFLAG_DISABLEGRAVITY))
            setMoveDisableGravity(false);

        if (!isHovering() && isAlive() && (canHover() || hasAuraWithAuraEffect(SPELL_AURA_HOVER)))
            setMoveHover(true);
    }

    // Swimming flag
    if (isInWater() && getMovementTemplate().isSwimAllowed())
    {
        if (!hasUnitMovementFlag(MOVEFLAG_SWIMMING))
            setMoveSwim(true);
    }
    else
    {
        if (hasUnitMovementFlag(MOVEFLAG_SWIMMING))
            setMoveSwim(false);
    }
}

//MIT end

void Creature::Update(unsigned long time_passed)
{
    Unit::Update(time_passed);

    // Update Movement
    if (time_passed >= m_movementFlagUpdateTimer)
    {
        updateMovementFlags();
        m_movementFlagUpdateTimer = 1000;
    }
    else
    {
        m_movementFlagUpdateTimer -= static_cast<uint16_t>(time_passed);
    }

    const auto now = Util::getTimeNow();

    // Update DeathState
    switch (m_deathState)
    {
        case DEAD:
        {
            if (m_respawnTime <= now)
                respawn();
        } break;
        case CORPSE:
        {
            if (m_corpseRemoveTime <= now)
                OnRemoveCorpse();
        } break;
        default:
            break;
    }
}

void Creature::SafeDelete()
{
    sEventMgr.RemoveEvents(this);

    delete this;
}

void Creature::DeleteMe()
{
    if (IsInWorld())
        RemoveFromWorld(false, true);
    else
        SafeDelete();
}

void Creature::OnRemoveCorpse()
{
    // time to respawn!
    if (IsInWorld() && (int32_t)m_WorldMap->getInstanceId() == m_instanceId)
    {
        sLogger.info("Removing corpse of {}...", std::to_string(getGuid()));

        setDeathState(DEAD);
        m_position = m_spawnLocation;

        // Respawn Handling
        const auto now = Util::getTimeNow();
        m_corpseRemoveTime = now;

        // if corpse was removed during falling, the falling will continue and override relocation to respawn position
        if (IsFalling())
            stopMoving();

        setMoveCanFly(false);

        getMovementManager()->clear();

        if ((getWorldMap()->getBaseMap()->getMapInfo() && getWorldMap()->getBaseMap()->getMapInfo()->isRaid() && creature_properties->isBoss) || m_noRespawn)
        {
            RemoveFromWorld(false, true);
        }
        else
        {
            if (m_respawnTime)
                RemoveFromWorld(true, false);
            else
                RemoveFromWorld(false, true);
        }
    }
    else
    {
        sLogger.failure("Creature::OnRemoveCorpse but Creature is not in World or in instance");
    }
}

void Creature::OnRespawn(WorldMap* m)
{
    if (m_noRespawn)
        return;

    // if corpse was removed during falling, the falling will continue and override relocation to respawn position
    if (IsFalling())
        stopMoving();

    getMovementManager()->clear();

    sLogger.info("Respawning {}...", std::to_string(getGuid()));
    setHealth(getMaxHealth());

    if (m_spawn)
    {
        setNpcFlags(creature_properties->NPCFLags);
        setEmoteState(m_spawn->emote_state);

        // creature's death state
        if (m_spawn->death_state == CREATURE_STATE_APPEAR_DEAD)
        {
            m_limbostate = true;
            setDeathState(ALIVE);   // we are not actually dead, we just appear dead
            setDynamicFlags(U_DYN_FLAG_DEAD);
        }
        else if (m_spawn->death_state == CREATURE_STATE_DEAD)
        {
            setHealth(0);
            m_limbostate = true;
            setDeathState(CORPSE);
        }
        else
            setDeathState(ALIVE);
    }

    removeUnitFlags(UNIT_FLAG_SKINNABLE);

    setTaggerGuid(0);

    //empty loot
    loot.items.clear();

    // Init Movement Handlers
    getMovementManager()->initializeDefault();

    // Re-initialize reactstate that could be altered by movementgenerators
    getAIInterface()->initializeReactState();

#ifdef FT_VEHICLES
    // Init Vehicle
    createVehicleKit(GetCreatureProperties()->vehicleid, getEntry());
#endif

    m_PickPocketed = false;
    PushToWorld(m);
}

void Creature::Create(uint32_t mapid, float x, float y, float z, float ang)
{
    Object::_Create(mapid, x, y, z, ang);
}

void Creature::CreateWayPoint(uint32_t /*WayPointID*/, uint32_t mapid, float x, float y, float z, float ang)
{
    Object::_Create(mapid, x, y, z, ang);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Looting

void Creature::SaveToDB()
{
    if (m_spawn == NULL)
    {
        m_spawn = new MySQLStructure::CreatureSpawn;
        m_spawn->entry = getEntry();
        m_spawn->id = spawnid = sObjectMgr.generateCreatureSpawnId();
        m_spawn->movetype = getDefaultMovementType();
        m_spawn->displayid = getDisplayId();
        m_spawn->x = m_position.x;
        m_spawn->y = m_position.y;
        m_spawn->z = m_position.z;
        m_spawn->o = m_position.o;
        m_spawn->emote_state = getEmoteState();
        m_spawn->flags = getUnitFlags();
        m_spawn->pvp_flagged = isPvpFlagSet() ? 1 : 0;
        m_spawn->factionid = getFactionTemplate();
        m_spawn->bytes0 = getBytes0();
        m_spawn->stand_state = getStandState();
        m_spawn->death_state = 0;
        m_spawn->channel_target_creature = 0;
        m_spawn->channel_target_go = 0;
        m_spawn->channel_spell = 0;
        m_spawn->MountedDisplayID = getMountDisplayId();
        m_spawn->sheath_state = getSheathType();

#if VERSION_STRING < WotLK
        m_spawn->Item1SlotEntry = getVirtualItemEntry(MELEE);
        m_spawn->Item2SlotEntry = getVirtualItemEntry(OFFHAND);
        m_spawn->Item3SlotEntry = getVirtualItemEntry(RANGED);
#else
        m_spawn->Item1SlotEntry = getVirtualItemSlotId(MELEE);
        m_spawn->Item2SlotEntry = getVirtualItemSlotId(OFFHAND);
        m_spawn->Item3SlotEntry = getVirtualItemSlotId(RANGED);
#endif

        if (IsFlying())
            m_spawn->CanFly = 1;
        else
            m_spawn->CanFly = 0;

        m_spawn->phase = m_phase;

        uint32_t x = getWorldMap()->getPosX(GetPositionX());
        uint32_t y = getWorldMap()->getPosY(GetPositionY());

        // Add spawn to map
        getWorldMap()->getBaseMap()->getSpawnsListAndCreate(x, y)->CreatureSpawns.push_back(m_spawn);
    }

    std::stringstream ss;

    ss << "DELETE FROM " << m_spawn->origine << " WHERE id = ";
    ss << spawnid;
    ss << " AND min_build <= ";
    ss << VERSION_STRING;
    ss << " AND max_build >= ";
    ss << VERSION_STRING;
    ss << ";";

    WorldDatabase.Execute(ss.str().c_str());

    ss.rdbuf()->str("");

    ss << "INSERT INTO " << m_spawn->origine << " VALUES("
        << spawnid << ","
        << VERSION_STRING << ","
        << VERSION_STRING << ","
        << getEntry() << ","
        << GetMapId() << ","
        << m_position.x << ","
        << m_position.y << ","
        << m_position.z << ","
        << m_position.o << ","
        << static_cast<uint32_t>(getDefaultMovementType()) << ","
        << getDisplayId() << ","
        << getFactionTemplate() << ","
        << getUnitFlags() << ","
        << (isPvpFlagSet() ? "1" : "0") << ","
        << getBytes0() << ","
        << getEmoteState() << ",0,";

    ss << m_spawn->channel_spell << ","
        << m_spawn->channel_target_go << ","
        << m_spawn->channel_target_creature << ",";

    ss << uint32_t(getStandState()) << ",";

    ss << m_spawn->death_state << ",";

    ss << getMountDisplayId() << ","
        << std::to_string(getSheathType()) << ","
        << m_spawn->Item1SlotEntry << ","
        << m_spawn->Item2SlotEntry << ","
        << m_spawn->Item3SlotEntry << ",";

    if (IsFlying())
        ss << 1 << ",";
    else
        ss << 0 << ",";

    ss << m_phase << ","
        << "0"  // event_entry
        << ",0"  // wander distance
        << ",0" // waypoint_group
        << ")";

    WorldDatabase.Execute(ss.str().c_str());
}

void Creature::LoadScript()
{
    _myScriptClass = sScriptMgr.CreateAIScriptClassForEntry(this);
}

void Creature::DeleteFromDB()
{
    if (!GetSQL_id())
        return;

    WorldDatabase.Execute("DELETE FROM %s WHERE id = %u AND min_build <= %u AND max_build >= %u", m_spawn->origine.c_str(), GetSQL_id(), VERSION_STRING, VERSION_STRING);
}


//////////////////////////////////////////////////////////////////////////////////////////
/// Quests

void Creature::AddQuest(std::unique_ptr<QuestRelation> Q)
{
    m_quests->push_back(std::move(Q));
}

void Creature::DeleteQuest(QuestRelation const* Q)
{
    for (auto it = m_quests->begin(); it != m_quests->end(); ++it)
    {
        if (((*it)->type == Q->type) && ((*it)->qst == Q->qst))
        {
            m_quests->erase(it);
            break;
        }
    }
}

QuestProperties const* Creature::FindQuest(uint32_t quest_id, uint8_t quest_relation)
{
    for (auto it = m_quests->begin(); it != m_quests->end(); ++it)
    {
        const auto& ptr = (*it);

        if ((ptr->qst->id == quest_id) && (ptr->type & quest_relation))
        {
            return ptr->qst;
        }
    }
    return nullptr;
}

uint16_t Creature::GetQuestRelation(uint32_t quest_id)
{
    uint16_t quest_relation = 0;

    for (auto it = m_quests->begin(); it != m_quests->end(); ++it)
    {
        if ((*it)->qst->id == quest_id)
        {
            quest_relation |= (*it)->type;
        }
    }
    return quest_relation;
}

uint32_t Creature::NumOfQuests()
{
    return (uint32_t)m_quests->size();
}

std::list<std::unique_ptr<QuestRelation>>::iterator Creature::QuestsBegin()
{
    return m_quests->begin();
}

std::list<std::unique_ptr<QuestRelation>>::iterator Creature::QuestsEnd()
{
    return m_quests->end();
}

void Creature::SetQuestList(std::list<std::unique_ptr<QuestRelation>>* qst_lst)
{
    m_quests = qst_lst;
}

uint32_t Creature::GetHealthFromSpell()
{
    return m_healthfromspell;
}

void Creature::SetHealthFromSpell(uint32_t value)
{
    m_healthfromspell = value;
}

void Creature::_LoadQuests()
{
    sQuestMgr.LoadNPCQuests(this);
}

bool Creature::HasQuests()
{
    return m_quests != NULL;
}

bool Creature::HasQuest(uint32_t id, uint32_t type)
{
    if (!m_quests) return false;
    for (auto itr = m_quests->begin(); itr != m_quests->end(); ++itr)
        {
            if ((*itr)->qst->id == id && (*itr)->type & type)
                return true;
        }
    return false;
}

void Creature::AddToWorld()
{
    // force set faction
    if (m_factionTemplate == NULL || m_factionEntry == NULL)
        setServersideFaction();

    if (creature_properties == nullptr)
        creature_properties = sMySQLStore.getCreatureProperties(getEntry());

    if (creature_properties == nullptr)
        return;

    if (m_factionTemplate == NULL || m_factionEntry == NULL)
        return;

    Object::AddToWorld();
    searchFormation();   
    motion_Initialize();
    immediateMovementFlagsUpdate();

    if (getMovementTemplate().isRooted())
        setControlled(true, UNIT_STATE_ROOTED);
}

void Creature::AddToWorld(WorldMap* pMapMgr)
{
    // force set faction
    if (m_factionTemplate == NULL || m_factionEntry == NULL)
        setServersideFaction();

    if (creature_properties == nullptr)
        creature_properties = sMySQLStore.getCreatureProperties(getEntry());

    if (creature_properties == nullptr)
        return;

    if (m_factionTemplate == NULL || m_factionEntry == NULL)
        return;

    Object::AddToWorld(pMapMgr);
    searchFormation();
    motion_Initialize();
    immediateMovementFlagsUpdate();

    if (getMovementTemplate().isRooted())
        setControlled(true, UNIT_STATE_ROOTED);
}

bool Creature::CanAddToWorld()
{
    if (m_factionEntry == NULL || m_factionTemplate == NULL)
        setServersideFaction();

    if (m_factionTemplate == NULL || m_factionEntry == NULL || creature_properties == nullptr)
        return false;

    return true;
}

void Creature::RemoveFromWorld(bool addrespawnevent, bool /*free_guid*/)
{
    if (addrespawnevent && m_respawnTime > 0)
        despawn(0);
    else
        Despawn(0, 0);
}

void Creature::RemoveFromWorld(bool free_guid)
{
    PrepareForRemove();
    Unit::RemoveFromWorld(free_guid);
}

void Creature::EnslaveExpire()
{
    ++m_enslaveCount;

    uint64_t charmer = getCharmedByGuid();

    Player* caster = sObjectMgr.getPlayer(WoWGuid::getGuidLowPartFromUInt64(charmer));
    if (caster)
    {
        caster->setCharmGuid(0);
        caster->setSummonGuid(0);

        WorldPacket data(SMSG_PET_SPELLS, 8);

        data << uint64_t(0);
        data << uint32_t(0);

        caster->sendPacket(&data);
    }
    setCharmedByGuid(0);
    setSummonedByGuid(0);

    resetCurrentSpeeds();

    switch (GetCreatureProperties()->Type)
    {
        case UNIT_TYPE_DEMON:
            setFaction(90);
            break;
        default:
            setFaction(954);
            break;
    };

    getAIInterface()->Init(this);

    updateInRangeOppositeFactionSet();
    updateInRangeSameFactionSet();
}

uint32_t Creature::GetEnslaveCount()
{
    return m_enslaveCount;
}

void Creature::SetEnslaveCount(uint32_t count)
{
    m_enslaveCount = count;
}

uint32_t Creature::GetEnslaveSpell()
{
    return m_enslaveSpell;
}

void Creature::SetEnslaveSpell(uint32_t spellId)
{
    m_enslaveSpell = spellId;
}

bool Creature::RemoveEnslave()
{
    return removeAllAurasByIdReturnCount(m_enslaveSpell) > 0;
}

void Creature::CalcResistance(uint8_t type)
{
    int32_t pos = 0;
    int32_t neg = 0;

    if (BaseResistanceModPct[type] < 0)
        neg = (m_baseResistance[type] * abs(BaseResistanceModPct[type]) / 100);
    else
        pos = (m_baseResistance[type] * BaseResistanceModPct[type]) / 100;

    if (isPet() && isAlive() && IsInWorld())
    {
        Player* owner = static_cast<Pet*>(this)->getPlayerOwner();
        if (type == 0 && owner)
            pos += int32_t(0.35f * owner->getResistance(type));
        else if (owner)
            pos += int32_t(0.40f * owner->getResistance(type));
    }

    if (ResistanceModPct[type] < 0)
        neg += (m_baseResistance[type] + pos - neg) * abs(ResistanceModPct[type]) / 100;
    else
        pos += (m_baseResistance[type] + pos - neg) * ResistanceModPct[type] / 100;

    if (FlatResistanceMod[type] < 0)
        neg += abs(FlatResistanceMod[type]);
    else
        pos += FlatResistanceMod[type];

#if VERSION_STRING > Classic
    setResistanceBuffModPositive(type, pos);
    setResistanceBuffModNegative(type, neg);
#endif

    int32_t tot = m_baseResistance[type] + pos - neg;

    setResistance(type, tot > 0 ? tot : 0);
}

void Creature::CalcStat(uint8_t type)
{
    int32_t pos = 0;
    int32_t neg = 0;

    if (StatModPct[type] < 0)
        neg = (m_baseStats[type] * abs(StatModPct[type]) / 100);
    else
        pos = (m_baseStats[type] * StatModPct[type]) / 100;

    if (isPet())
    {
        Player* owner = static_cast<Pet*>(this)->getPlayerOwner();
        if (type == STAT_STAMINA && owner)
            pos += int32_t(0.45f * owner->getStat(STAT_STAMINA));
        else if (type == STAT_INTELLECT && owner && getCreatedBySpellId())
            pos += int32_t(0.30f * owner->getStat(STAT_INTELLECT));
    }

    if (TotalStatModPct[type] < 0)
        neg += (m_baseStats[type] + pos - neg) * abs(TotalStatModPct[type]) / 100;
    else
        pos += (m_baseStats[type] + pos - neg) * TotalStatModPct[type] / 100;

    if (FlatStatMod[type] < 0)
        neg += abs(FlatStatMod[type]);
    else
        pos += FlatStatMod[type];

#if VERSION_STRING != Classic
    setPosStat(type, pos);
    setNegStat(type, neg);
#endif

    int32_t tot = m_baseStats[type] + pos - neg;
    setStat(type, tot > 0 ? tot : 0);

    switch (type)
    {
        case STAT_STRENGTH:
        {
            //Attack Power
            if (!isPet())  //We calculate pet's later
            {
                uint32_t str = getStat(STAT_STRENGTH);
                int32_t AP = (str * 2 - 20);
                if (AP < 0) AP = 0;
                setAttackPower(AP);
            }
            calculateDamage();
        }
        break;
        case STAT_AGILITY:
        {
            //Ranged Attack Power (Does any creature use this?)
            int32_t RAP = getLevel() + getStat(STAT_AGILITY) - 10;
            if (RAP < 0)
                RAP = 0;

            setRangedAttackPower(RAP);
        }
        break;
        case STAT_STAMINA:
        {
#if VERSION_STRING != Classic
            //Health
            uint32_t hp = getBaseHealth();
            uint32_t stat_bonus = getPosStat(STAT_STAMINA)- getNegStat(STAT_STAMINA);
            if (static_cast<int32_t>(stat_bonus) < 0) stat_bonus = 0;

            uint32_t bonus = stat_bonus * 10 + m_healthfromspell;
            uint32_t res = hp + bonus;

            if (res < hp)
                res = hp;
            setMaxHealth(res);
            if (getHealth() > getMaxHealth())
                setHealth(getMaxHealth());
#endif
        }
        break;
        case STAT_INTELLECT:
        {
#if VERSION_STRING != Classic
            if (getPowerType() == POWER_TYPE_MANA)
            {
                uint32_t mana = getBaseMana();
                uint32_t stat_bonus = getPosStat(STAT_INTELLECT) - getNegStat(STAT_INTELLECT);
                if (static_cast<int32_t>(stat_bonus) < 0) stat_bonus = 0;

                uint32_t bonus = stat_bonus * 15;
                uint32_t res = mana + bonus;

                if (res < mana) res = mana;
                setMaxPower(POWER_TYPE_MANA, res);
            }
#endif
        }
        break;
    }
}

void Creature::RegenerateHealth()
{
    if (m_limbostate || !m_canRegenerateHP)
        return;

    uint32_t cur = getHealth();
    uint32_t mh = getMaxHealth();
    if (cur >= mh)return;

    float amt = 0.0f;

    // While polymorphed health is regenerated rapidly
    // Exact value is yet unknown but it's roughly 10% of health per sec
    if (hasUnitStateFlag(UNIT_STATE_POLYMORPHED))
    {
        amt = getMaxHealth() * 0.10f;
    }
    else if (!getCombatHandler().isInCombat())
    {
        // 25% of max health per tick
        amt = getMaxHealth() * 0.25f;
        amt *= getTotalPctMultiplierForAuraEffect(SPELL_AURA_MOD_HEALTH_REGEN_PERCENT);

        //Apply shit from conf file
        amt *= worldConfig.getFloatRate(RATE_HEALTH);
    }
    else
    {
        return;
    }

    if (amt <= 1.0f) //this fixes regen like 0.98
        cur++;
    else
        cur += (uint32_t)amt;
    setHealth((cur >= mh) ? mh : cur);
}

void Creature::CallScriptUpdate(unsigned long time_passed)
{
    if (_myScriptClass)
        _myScriptClass->_internalAIUpdate(time_passed);
}

CreatureProperties const* Creature::GetCreatureProperties()
{
    return creature_properties;
}

void Creature::SetCreatureProperties(CreatureProperties const* cp)
{
    creature_properties = cp;
}

Trainer const* Creature::GetTrainer()
{
    return mTrainer;
}

void Creature::AddVendorItem(uint32_t itemid, uint32_t amount, WDB::Structures::ItemExtendedCostEntry const* ec)
{
    CreatureItem ci;
    ci.amount = amount;
    ci.itemid = itemid;
    ci.available_amount = 0;
    ci.max_amount = 0;
    ci.incrtime = 0;
    ci.extended_cost = ec;
    if (m_SellItems == nullptr)
        m_SellItems = sObjectMgr.createVendorList(getEntry());

    m_SellItems->push_back(ci);
}

void Creature::ModAvItemAmount(uint32_t itemid, uint32_t value)
{
    for (std::vector<CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
    {
        if (itr->itemid == itemid)
        {
            if (itr->available_amount)
            {
                if (value > itr->available_amount)    // shouldn't happen
                {
                    itr->available_amount = 0;
                    return;
                }
                else
                    itr->available_amount -= value;

                if (!event_HasEvent(EVENT_ITEM_UPDATE))
                    sEventMgr.AddEvent(this, &Creature::UpdateItemAmount, itr->itemid, EVENT_ITEM_UPDATE, itr->incrtime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }
            return;
        }
    }
}

void Creature::UpdateItemAmount(uint32_t itemid)
{
    for (std::vector<CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
    {
        if (itr->itemid == itemid)
        {
            if (itr->max_amount == 0)        // shouldn't happen
                itr->available_amount = 0;
            else
            {
                itr->available_amount = itr->max_amount;
            }
            return;
        }
    }
}

void Creature::ChannelLinkUpGO(uint32_t SqlId)
{
    if (!m_WorldMap)        // shouldn't happen
        return;

    GameObject* go = m_WorldMap->getSqlIdGameObject(SqlId);
    if (go != nullptr)
    {
        event_RemoveEvents(EVENT_CREATURE_CHANNEL_LINKUP);
        setChannelObjectGuid(go->getGuid());
        setChannelSpellId(m_spawn->channel_spell);
    }
}

void Creature::ChannelLinkUpCreature(uint32_t SqlId)
{
    if (!m_WorldMap)        // shouldn't happen
        return;

    Creature* creature = m_WorldMap->getSqlIdCreature(SqlId);
    if (creature != nullptr)
    {
        event_RemoveEvents(EVENT_CREATURE_CHANNEL_LINKUP);
        setChannelObjectGuid(creature->getGuid());
        setChannelSpellId(m_spawn->channel_spell);
    }
}

bool Creature::isattackable(MySQLStructure::CreatureSpawn* spawn)
{
    if (spawn == nullptr)
        return false;

    if ((spawn->flags & 2) || (spawn->flags & 128) || (spawn->flags & 256) || (spawn->flags & 65536))
        return false;
    else
        return true;
}

uint8_t get_byte(uint32_t buffer, uint32_t index)
{
    uint32_t mask = uint32_t(~0ul);
    if (index > sizeof(uint32_t) - 1)
        return 0;

    buffer = buffer >> index * 8;
    mask = mask >> 3 * 8;
    buffer = buffer & mask;

    return (uint8_t)buffer;
}

bool Creature::teleport(const LocationVector& vec, WorldMap* map)
{
    if (map == nullptr)
        return false;

    if (map->getCreature(this->getGuidLow()))
    {
        this->SetPosition(vec);
        return true;
    }
    else
    {
        return false;
    }
}

bool Creature::Load(MySQLStructure::CreatureSpawn* spawn, uint8_t mode, MySQLStructure::MapInfo const* info)
{
    m_spawn = spawn;
    creature_properties = sMySQLStore.getCreatureProperties(spawn->entry);
    if (creature_properties == nullptr)
        return false;

    spawnid = spawn->id;
    m_phase = spawn->phase;

    setSpeedRate(TYPE_WALK, creature_properties->walk_speed, false);
    setSpeedRate(TYPE_RUN, creature_properties->run_speed, false);
    setSpeedRate(TYPE_FLY, creature_properties->fly_speed, false);
    resetCurrentSpeeds();

    //Set fields
    setEntry(creature_properties->Id);
    setScale(creature_properties->Scale);

#if VERSION_STRING > TBC
    setHoverHeight(creature_properties->Scale);
#endif

    uint32_t health;
    if (creature_properties->MinHealth > creature_properties->MaxHealth)
    {
        sLogger.failure("MinHealth is bigger than MaxHealt! Using MaxHealth value. You should fix this in creature_proto table for entry: {}!", creature_properties->Id);
        health = creature_properties->MaxHealth - Util::getRandomUInt(10);
    }
    else
    {
        health = creature_properties->MinHealth + Util::getRandomUInt(creature_properties->MaxHealth - creature_properties->MinHealth);
    }

    setMaxHealth(health);
    setHealth(health);
    setBaseHealth(health);

    setMaxPower(POWER_TYPE_MANA, creature_properties->Mana);
    setBaseMana(creature_properties->Mana);
    setPower(POWER_TYPE_MANA, creature_properties->Mana);


    setDisplayId(spawn->displayid);
    setNativeDisplayId(spawn->displayid);
    setMountDisplayId(spawn->MountedDisplayID);

    eventModelChange();

    setLevel(creature_properties->MinLevel + (Util::getRandomUInt(creature_properties->MaxLevel - creature_properties->MinLevel)));

    if (mode && info)
        setLevel(std::min(73 - getLevel(), info->lvl_mod_a));

    for (uint8_t i = 0; i < 7; ++i)
        setResistance(i, creature_properties->Resistances[i]);

    setBaseAttackTime(MELEE, creature_properties->AttackTime);

    setMinDamage(creature_properties->MinDamage);
    setMaxDamage(creature_properties->MaxDamage);

    setBaseAttackTime(RANGED, creature_properties->RangedAttackTime);
    setMinRangedDamage(creature_properties->RangedMinDamage);
    setMaxRangedDamage(creature_properties->RangedMaxDamage);

    setVirtualItemSlotId(MELEE, spawn->Item1SlotEntry);
    setVirtualItemSlotId(OFFHAND, spawn->Item2SlotEntry);
    setVirtualItemSlotId(RANGED, spawn->Item3SlotEntry);

    setFaction(spawn->factionid);
    setUnitFlags(spawn->flags);
    setEmoteState(spawn->emote_state);
    setBoundingRadius(creature_properties->BoundingRadius);
    setCombatReach(creature_properties->CombatReach);
    original_emotestate = spawn->emote_state;

#if VERSION_STRING == TBC
    // Summons are set in Summon::load
    if (!isSummon())
        setPositiveAuraLimit(POS_AURA_LIMIT_CREATURE);
#endif

    // set position
    m_position.ChangeCoords({ spawn->x, spawn->y, spawn->z, spawn->o });
    m_spawnLocation.ChangeCoords({ spawn->x, spawn->y, spawn->z, spawn->o });
    m_aiInterface->timed_emotes = sObjectMgr.getTimedEmoteList(spawn->id);

    // not a neutral creature
    if (!(m_factionEntry != nullptr && m_factionEntry->RepListId == -1 && m_factionTemplate->HostileMask == 0 && m_factionTemplate->FriendlyMask == 0))
    {
        getAIInterface()->setCanCallForHelp(true);
    }

    getAIInterface()->initializeReactState();

    // set if creature can shoot or not.
    if (creature_properties->CanRanged == 1)
        getAIInterface()->m_canRangedAttack = true;
    else
        m_aiInterface->m_canRangedAttack = false;

    // checked at loading
    m_defaultMovementType = MovementGeneratorType(spawn->movetype);

    setMaxWanderDistance(static_cast<float_t>(spawn->wander_distance));
    if (getMaxWanderDistance() == 0.0f && m_defaultMovementType == RANDOM_MOTION_TYPE)
        m_defaultMovementType = IDLE_MOTION_TYPE;

    _waypointPathId = spawn->waypoint_id;

    //SETUP NPC FLAGS
    setNpcFlags(creature_properties->NPCFLags);

    if (isVendor())
        m_SellItems = sObjectMgr.getVendorList(getEntry());

    if (isQuestGiver())
        _LoadQuests();

    if (isTrainer() || isProfessionTrainer())
        mTrainer = sObjectMgr.getTrainer(getEntry());

    if (isAuctioneer())
        auctionHouse = sAuctionMgr.getAuctionHouse(getEntry());

    //load resistances
    for (uint8_t x = 0; x < TOTAL_SPELL_SCHOOLS; ++x)
        m_baseResistance[x] = getResistance(x);
    for (uint8_t x = 0; x < STAT_COUNT; ++x)
        m_baseStats[x] = getStat(x);

    m_baseDamage[0] = getMinDamage();
    m_baseDamage[1] = getMaxDamage();
    m_baseOffhandDamage[0] = getMinOffhandDamage();
    m_baseOffhandDamage[1] = getMaxOffhandDamage();
    m_baseRangedDamage[0] = getMinRangedDamage();
    m_baseRangedDamage[1] = getMaxRangedDamage();
    BaseAttackType = creature_properties->attackSchool;

    setModCastSpeed(1.0f);   // better set this one

    // Bytes 0
    setBytes0(spawn->bytes0);

    // Bytes 1
    setBytes1(0);
    setStandState(spawn->stand_state);

    // Bytes 2
    setBytes2(0);
    setSheathType(spawn->sheath_state);
    if (spawn->pvp_flagged == 1)
        setPvpFlag();

    ////////////AI

    if (isattackable(spawn))
        getAIInterface()->setAllowedToEnterCombat(true);

    //////////////AI

    myFamily = sCreatureFamilyStore.lookupEntry(creature_properties->Family);


    //HACK!
    if (getDisplayId() == 17743 ||
        getDisplayId() == 20242 ||
        getDisplayId() == 15435 ||
        (creature_properties->Family == UNIT_TYPE_MISC))
    {
        setAItoUse(false);
    }

    // more hacks!
    if (creature_properties->Mana != 0)
        setPowerType(POWER_TYPE_MANA);
    else
        setPowerType(0);

    /*  // Dont was Used in old AIInterface left the code here if needed at other Date
    if (creature_properties->guardtype == GUARDTYPE_CITY)
        getAIInterface()->setGuard(true);
    else
        getAIInterface()->setGuard(false);*/

    if (creature_properties->guardtype == GUARDTYPE_NEUTRAL)
        getAIInterface()->setGuard(true);
    else
        getAIInterface()->setGuard(false);


    // creature death state
    if (spawn->death_state == CREATURE_STATE_APPEAR_DEAD)
    {
        m_limbostate = true;
        setDynamicFlags(U_DYN_FLAG_DEAD);
    }
    else if (spawn->death_state == CREATURE_STATE_DEAD)
    {
        setHealth(0);
        m_limbostate = true;
        setDeathState(CORPSE);
    }

    m_aiInterface->eventAiInterfaceParamsetFinish();

    this->m_position.x = spawn->x;
    this->m_position.y = spawn->y;
    this->m_position.z = spawn->z;
    this->m_position.o = spawn->o;

    // Set spell immunities
    if (creature_properties->modImmunities != 0)
    {
        const auto immunityMask = static_cast<SpellImmunityMask>(creature_properties->modImmunities);
        addSpellImmunity(immunityMask, true);
    }

#ifdef FT_VEHICLES
    if (isVehicle())
    {
        createVehicleKit(creature_properties->vehicleid, creature_properties->Id);
        addNpcFlags(UNIT_NPC_FLAG_SPELLCLICK);
        setAItoUse(false);
    }
#endif

    if (getMovementTemplate().isRooted())
        setControlled(true, UNIT_STATE_ROOTED);

    // Respawn
    m_respawnDelay = creature_properties->RespawnTime / IN_MILLISECONDS;

    if (isDungeonBoss())
        m_respawnDelay = 0; // special value, prevents respawn for dungeon bosses unless overridden

    switch (this->creature_properties->Rank)
    {
        case ELITE_ELITE:
            m_corpseDelay = worldConfig.corpseDecay.eliteTimeInSeconds;
            break;
        case ELITE_RAREELITE:
            m_corpseDelay = worldConfig.corpseDecay.rareEliteTimeInSeconds;
            break;
        case ELITE_WORLDBOSS:
            m_corpseDelay = worldConfig.corpseDecay.worldbossTimeInSeconds;
            break;
        case ELITE_RARE:
            m_corpseDelay = worldConfig.corpseDecay.rareTimeInSeconds;
            break;
        default:
            m_corpseDelay = worldConfig.corpseDecay.normalTimeInSeconds;
            break;
    }

    return true;
}

void Creature::Load(CreatureProperties const* properties_, float x, float y, float z, float o)
{
    creature_properties = properties_;

    if (creature_properties->isTrainingDummy == 0 && !isVehicle())
    {
        getAIInterface()->setAllowedToEnterCombat(true);
    }

    setSpeedRate(TYPE_WALK, creature_properties->walk_speed, false);
    setSpeedRate(TYPE_RUN, creature_properties->run_speed, false);
    setSpeedRate(TYPE_FLY, creature_properties->fly_speed, false);
    resetCurrentSpeeds();

    //Set fields
    setEntry(creature_properties->Id);
    setScale(creature_properties->Scale);

#if VERSION_STRING > TBC
    setHoverHeight(creature_properties->Scale);
#endif

    uint32_t health = creature_properties->MinHealth + Util::getRandomUInt(creature_properties->MaxHealth - creature_properties->MinHealth);

    setMaxHealth(health);
    setHealth(health);
    setBaseHealth(health);

    setMaxPower(POWER_TYPE_MANA, creature_properties->Mana);
    setBaseMana(creature_properties->Mana);
    setPower(POWER_TYPE_MANA, creature_properties->Mana);

    uint32_t model = 0;
    uint8_t gender = creature_properties->generateRandomDisplayIdAndReturnGender(&model);
    setGender(gender);

    setDisplayId(model);
    setNativeDisplayId(model);
    setMountDisplayId(0);

    eventModelChange();

    setLevel(creature_properties->MinLevel + (Util::getRandomUInt(creature_properties->MaxLevel - creature_properties->MinLevel)));

    for (uint8_t i = 0; i < 7; ++i)
        setResistance(i, creature_properties->Resistances[i]);

    setBaseAttackTime(MELEE, creature_properties->AttackTime);
    setMinDamage(creature_properties->MinDamage);
    setMaxDamage(creature_properties->MaxDamage);

    setFaction(creature_properties->Faction);
    setBoundingRadius(creature_properties->BoundingRadius);
    setCombatReach(creature_properties->CombatReach);

#if VERSION_STRING == TBC
    // Summons are set in Summon::load
    if (!isSummon())
        setPositiveAuraLimit(POS_AURA_LIMIT_CREATURE);
#endif

    original_emotestate = 0;

    // set position
    m_position.ChangeCoords({ x, y, z, o });
    m_spawnLocation.ChangeCoords({ x, y, z, o });

    // not a neutral creature
    if (m_factionEntry && !(m_factionEntry->RepListId == -1 && m_factionTemplate->HostileMask == 0 && m_factionTemplate->FriendlyMask == 0))
    {
        getAIInterface()->setCanCallForHelp(true);
    }

    getAIInterface()->initializeReactState();

    // set if creature can shoot or not.
    if (creature_properties->CanRanged == 1)
        getAIInterface()->m_canRangedAttack = true;
    else
        getAIInterface()->m_canRangedAttack = false;

    // checked at loading
    m_defaultMovementType = MovementGeneratorType(IDLE_MOTION_TYPE);

    if (getMaxWanderDistance() == 0.0f && m_defaultMovementType == RANDOM_MOTION_TYPE)
        m_defaultMovementType = IDLE_MOTION_TYPE;

    _waypointPathId = 0;

    //SETUP NPC FLAGS
    setNpcFlags(creature_properties->NPCFLags);

    if (isVendor())
        m_SellItems = sObjectMgr.getVendorList(getEntry());

    if (isQuestGiver())
        _LoadQuests();

    if (isTrainer() || isProfessionTrainer())
        mTrainer = sObjectMgr.getTrainer(getEntry());

    if (isAuctioneer())
        auctionHouse = sAuctionMgr.getAuctionHouse(getEntry());

    //load resistances
    for (uint8_t j = 0; j < TOTAL_SPELL_SCHOOLS; ++j)
        m_baseResistance[j] = getResistance(j);
    for (uint8_t j = 0; j < STAT_COUNT; ++j)
        m_baseStats[j] = getStat(j);

    m_baseDamage[0] = getMinDamage();
    m_baseDamage[1] = getMaxDamage();
    m_baseOffhandDamage[0] = getMinOffhandDamage();
    m_baseOffhandDamage[1] = getMaxOffhandDamage();
    m_baseRangedDamage[0] = getMinRangedDamage();
    m_baseRangedDamage[1] = getMaxRangedDamage();
    BaseAttackType = creature_properties->attackSchool;

    setModCastSpeed(1.0f);   // better set this one

    ////////////AI
    getAIInterface()->initialiseScripts(getEntry());
    getAIInterface()->eventOnLoad();

    //////////////AI

    myFamily = sCreatureFamilyStore.lookupEntry(creature_properties->Family);


    // \todo remove this HACK! already included few lines above
    if (getDisplayId() == 17743 ||
        getDisplayId() == 20242 ||
        getDisplayId() == 15435 ||
        creature_properties->Type == UNIT_TYPE_MISC)
    {
        setAItoUse(false);
    }

    setPowerType(POWER_TYPE_MANA);

    // Equipment
    setVirtualItemSlotId(MELEE, creature_properties->itemslot_1);
    setVirtualItemSlotId(OFFHAND, creature_properties->itemslot_2);
    setVirtualItemSlotId(RANGED, creature_properties->itemslot_3);

    /*  // Dont was Used in old AIInterface left the code here if needed at other Date
    if (creature_properties->guardtype == GUARDTYPE_CITY)
        getAIInterface()->setGuard(true);
    else
        getAIInterface()->setGuard(false);*/

    if (creature_properties->guardtype == GUARDTYPE_NEUTRAL)
        getAIInterface()->setGuard(true);
    else
        getAIInterface()->setGuard(false);

    // Set spell immunities
    if (creature_properties->modImmunities != 0)
    {
        const auto immunityMask = static_cast<SpellImmunityMask>(creature_properties->modImmunities);
        addSpellImmunity(immunityMask, true);
    }

#ifdef FT_VEHICLES
    if (isVehicle())
    {
        createVehicleKit(creature_properties->vehicleid, creature_properties->Id);
        addNpcFlags(UNIT_NPC_FLAG_SPELLCLICK);
        setAItoUse(false);
    }
#endif

    if (getMovementTemplate().isRooted())
        setControlled(true, UNIT_STATE_ROOTED);

    // Respawn
    m_respawnDelay = creature_properties->RespawnTime / IN_MILLISECONDS;

    if (isDungeonBoss())
        m_respawnDelay = 0; // special value, prevents respawn for dungeon bosses unless overridden

    switch (this->creature_properties->Rank)
    {
        case ELITE_ELITE:
            m_corpseDelay = worldConfig.corpseDecay.eliteTimeInSeconds;
            break;
        case ELITE_RAREELITE:
            m_corpseDelay = worldConfig.corpseDecay.rareEliteTimeInSeconds;
            break;
        case ELITE_WORLDBOSS:
            m_corpseDelay = worldConfig.corpseDecay.worldbossTimeInSeconds;
            break;
        case ELITE_RARE:
            m_corpseDelay = worldConfig.corpseDecay.rareTimeInSeconds;
            break;
        default:
            m_corpseDelay = worldConfig.corpseDecay.normalTimeInSeconds;
            break;
    }
}

void Creature::OnLoaded()
{
    getAIInterface()->initialiseScripts(getEntry());
    getAIInterface()->eventOnLoad();
}

void Creature::OnPrePushToWorld()
{
    immediateMovementFlagsUpdate();
    Unit::OnPrePushToWorld();
}

void Creature::OnPushToWorld()
{
    if (creature_properties == nullptr)
    {
        sLogger.failure("Something tried to push Creature with entry {} with invalid creature_properties!", getEntry());
        return;
    }

    OnLoaded();

    // Send initial power regen modifiers
    // TODO: missing mana regen update for creatures
    //updateManaRegeneration(true);
    updateFocusRegeneration(true);
    updateEnergyRegeneration(true);

    std::set<uint32_t>::iterator itr = creature_properties->start_auras.begin();
    for (; itr != creature_properties->start_auras.end(); ++itr)
    {
        SpellInfo const* sp = sSpellMgr.getSpellInfo((*itr));
        if (sp == nullptr)
            continue;

        castSpell(this, sp, 0);
    }

    if (!sScriptMgr.has_creature_gossip(getEntry()))
        registerDatabaseGossip();

    if (GetScript() == NULL)
    {
        LoadScript();
    }

    // Formations
    searchFormation();
    motion_Initialize();

    Unit::OnPushToWorld();

    if (_myScriptClass)
    {
        _myScriptClass->OnLoad();
        _myScriptClass->InitOrReset();
    }

    if (m_spawn)
    {
        if (m_spawn->channel_target_creature)
            sEventMgr.AddEvent(this, &Creature::ChannelLinkUpCreature, m_spawn->channel_target_creature, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);    // only 5 attempts

        if (m_spawn->channel_target_go)
            sEventMgr.AddEvent(this, &Creature::ChannelLinkUpGO, m_spawn->channel_target_go, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);    // only 5 attempts
    }

    if (m_WorldMap)
        m_aiInterface->m_is_in_instance = (!m_WorldMap->getBaseMap()->getMapInfo()->isNonInstanceMap()) ? true : false;
    else
        m_aiInterface->m_is_in_instance = false;

    if (this->HasItems())
    {
        for (std::vector<CreatureItem>::iterator itr2 = m_SellItems->begin(); itr2 != m_SellItems->end(); ++itr2)
        {
            if (itr2->max_amount == 0)
                itr2->available_amount = 0;
            else if (itr2->available_amount < itr2->max_amount)
                sEventMgr.AddEvent(this, &Creature::UpdateItemAmount, itr2->itemid, EVENT_ITEM_UPDATE, vendorItemsUpdate, 1, 0);
        }

    }

    getAIInterface()->setCreatureProtoDifficulty(creature_properties->Id);

    if (mEvent != nullptr)
    {
        if (mEvent->mEventScript != nullptr)
            mEvent->mEventScript->OnCreaturePushToWorld(mEvent, this);
    }

    if (m_WorldMap && m_WorldMap->getScript())
    {
        m_WorldMap->getScript()->OnCreaturePushToWorld(this);
        m_WorldMap->getScript()->addObject(this);
    }
}

void Creature::respawn(bool force)
{
    if (force)
    {
        if (isAlive())
            setDeathState(JUST_DIED);
        else if (getDeathState() != CORPSE)
            setDeathState(CORPSE);
    }

    // do this for now delete the part when we are only respawning with spawnid
    if (true)
    {
        OnRemoveCorpse();

        if (getDeathState() == DEAD)
        {
            sLogger.debug("Respawning creature {} ({})", GetCreatureProperties()->Name, getGuid());
            m_respawnTime = 0;
            loot.clear();

            auto minlevel = std::min(GetCreatureProperties()->MaxLevel, GetCreatureProperties()->MinLevel);
            auto maxlevel = std::max(GetCreatureProperties()->MaxLevel, GetCreatureProperties()->MinLevel);
            auto level = minlevel == maxlevel ? minlevel : Util::getRandomUInt(minlevel, maxlevel);
            setLevel(level);

            setDeathState(JUST_RESPAWNED);

            uint32_t displayID = getNativeDisplayId();
            uint8_t gender = GetCreatureProperties()->generateRandomDisplayIdAndReturnGender(&displayID);

            setGender(gender);
            setDisplayId(displayID);
            setNativeDisplayId(displayID);

            getMovementManager()->initializeDefault();

            // Re-initialize reactstate that could be altered by movementgenerators
            getAIInterface()->initializeReactState();
        }
    }
    else
    {
        if (getSpawnId())
        {
            MapCell* pCell = getWorldMap()->getCellByCoords(GetSpawnX(), GetSpawnY());
            if (pCell == nullptr)
                pCell = GetMapCell();

            getWorldMap()->doRespawn(SPAWN_TYPE_CREATURE, nullptr, getSpawnId(), pCell->getPositionX(), pCell->getPositionY());
        }
    }

    sLogger.debug("Respawning creature {} ({})", GetCreatureProperties()->Name, getGuid());
}

void Creature::Despawn(uint32_t delay, uint32_t respawntime)
{
    if (delay)
    {
        sEventMgr.AddEvent(this, &Creature::Despawn, (uint32_t)0, respawntime, EVENT_CREATURE_RESPAWN, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    else
    {
        PrepareForRemove();

        if (!IsInWorld())
            return;

        if (_myScriptClass != NULL)
            _myScriptClass->OnDespawn();

        if (respawntime && !m_noRespawn)
        {
            // get the cell with our SPAWN location. if we've moved cell this might break :P
            MapCell* pCell = m_WorldMap->getCellByCoords(m_spawnLocation.x, m_spawnLocation.y);
            if (pCell == nullptr)
                pCell = GetMapCell();

            if (pCell != nullptr)
            {
                pCell->_respawnObjects.insert(this);

                sEventMgr.RemoveEvents(this);

                m_position = m_spawnLocation;
                m_respawnCell = pCell;

                saveRespawnTime(respawntime);
                Unit::RemoveFromWorld(false);
            }
            else
            {
                sLogger.failure("Creature::Despawn not able to get a valid MapCell (nullptr)");
            }
        }
        else
        {
            Unit::RemoveFromWorld(true);
            SafeDelete();
        }
    }
}

void Creature::despawn(uint32_t delay)
{
    if (delay)
    {
        sEventMgr.AddEvent(this, &Creature::despawn, (uint32_t)0, EVENT_CREATURE_RESPAWN, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    else
    {
        PrepareForRemove();

        if (!IsInWorld())
            return;

        if (_myScriptClass != nullptr)
            _myScriptClass->OnDespawn();

        if (!m_noRespawn)
        {
            // get the cell with our SPAWN location. if we've moved cell this might break :P
            MapCell* pCell = m_WorldMap->getCellByCoords(m_spawnLocation.x, m_spawnLocation.y);
            if (pCell == nullptr)
                pCell = GetMapCell();

            if (pCell != nullptr)
            {
                pCell->_respawnObjects.insert(this);

                sEventMgr.RemoveEvents(this);

                m_position = m_spawnLocation;
                m_respawnCell = pCell;

                saveRespawnTime();
                Unit::RemoveFromWorld(false);
            }
            else
            {
                sLogger.failure("Creature::Despawn not able to get a valid MapCell (nullptr)");
            }
        }
        else
        {
            Unit::RemoveFromWorld(true);
            SafeDelete();
        }
    }
}

void Creature::saveRespawnTime(uint32_t forceDelay)
{
    if (isSummon() || !getSpawnId() || !getWorldMap())
        return;

    const auto now = Util::getTimeNow();

    // do this for now delete the part when we are only respawning with spawnid
    if (true)
    {
        RespawnInfo ri;
        ri.type = SPAWN_TYPE_CREATURE;
        ri.spawnId = getSpawnId();
        ri.entry = getEntry();
        ri.time = forceDelay ? now + forceDelay / IN_MILLISECONDS : m_respawnTime;
        ri.cellX = m_spawnLocation.x;
        ri.cellY = m_spawnLocation.y;
        ri.obj = this;

        bool success = getWorldMap()->addRespawn(ri);
        if (success)
            getWorldMap()->saveRespawnDB(ri);
        return;
    }

    time_t thisRespawnTime = forceDelay ? now + forceDelay / IN_MILLISECONDS : m_respawnTime;
    getWorldMap()->saveRespawnTime(SPAWN_TYPE_CREATURE, getSpawnId(), getEntry(), thisRespawnTime, m_respawnCell->getPositionX(), m_respawnCell->getPositionY());
}

void Creature::TriggerScriptEvent(int fRef)
{
    if (_myScriptClass)
        _myScriptClass->StringFunctionCall(fRef);
}

bool Creature::IsInLimboState()
{
    return m_limbostate;
}

void Creature::SetLimboState(bool set)
{
    m_limbostate = set;
}

uint32_t Creature::GetLineByFamily(WDB::Structures::CreatureFamilyEntry const* family)
{
    return family->skilline ? family->skilline : 0;
}

void Creature::RemoveLimboState(Unit* /*healer*/)
{
    if (!m_limbostate != true)
        return;

    m_limbostate = false;
    setEmoteState(m_spawn ? m_spawn->emote_state : EMOTE_ONESHOT_NONE);
    setHealth(getMaxHealth());
    m_isInvincible = false;
}

uint32_t Creature::GetNpcTextId()
{
    return sMySQLStore.getGossipTextIdForNpc(this->getEntry());
}

float Creature::GetBaseParry()
{
    //\todo what are the parry rates for mobs?
    // FACT: bosses have varying parry rates (used to tune the difficulty of boss fights)

    // for now return a base of 5%, later get from dbase?
    return 5.0f;
}

int32_t Creature::GetDamageDoneMod(uint16_t school)
{
    if (school >= TOTAL_SPELL_SCHOOLS)
        return 0;

    return ModDamageDone[ school ];
}

float Creature::GetDamageDonePctMod(uint16_t school)
{
    if (school >= TOTAL_SPELL_SCHOOLS)
        return 0;

    return ModDamageDonePct[ school ];
}

bool Creature::IsPickPocketed()
{
    return m_PickPocketed;
}

void Creature::SetPickPocketed(bool val)
{
    m_PickPocketed = val;
}

CreatureAIScript* Creature::GetScript()
{
    return _myScriptClass;
}

bool Creature::HasLootForPlayer(Player* plr)
{   
    if (loot.isLooted()) // nothing to loot or everything looted.
        return false;

    if (const auto group = plr->getGroup())
    {
        switch (group->GetMethod())
        {
            case PARTY_LOOT_FREE_FOR_ALL:
                return true;
            case PARTY_LOOT_ROUND_ROBIN:
            case PARTY_LOOT_MASTER_LOOTER:
                // only loot if the player is Plunder Master or Round Robbin Player
                if (loot.roundRobinPlayer == 0 || loot.roundRobinPlayer == plr->getGuid())
                    return true;
                // or when it has Personal loot
                return loot.hasItemFor(plr);
            case PARTY_LOOT_GROUP:
            case PARTY_LOOT_NEED_BEFORE_GREED:
                // only loot when no Round Robbin or is Round Robber 
                if (loot.roundRobinPlayer == 0 || loot.roundRobinPlayer == plr->getGuid())
                    return true;
                // or if Items is under Group Threshold research this also grey items are under threshold which means free loot ?
                if (loot.hasOverThresholdItem())
                    return true;
                // or when it has Personal loot
                return loot.hasItemFor(plr);
        }
    }
    else
    {
        return getTaggerGuid() == plr->getGuid();
    }

    return false;
}

uint16_t Creature::GetRequiredLootSkill()
{
    if (GetCreatureProperties()->typeFlags & CREATURE_FLAG1_HERBLOOT)
        return SKILL_HERBALISM;     // herbalism
    else if (GetCreatureProperties()->typeFlags & CREATURE_FLAG1_MININGLOOT)
        return SKILL_MINING;        // mining
    else if (GetCreatureProperties()->typeFlags & CREATURE_FLAG1_ENGINEERLOOT)
        return SKILL_ENGINEERING;
    else
        return SKILL_SKINNING;      // skinning
}

uint32_t Creature::GetSQL_id()
{
    return spawnid;
};

bool Creature::HasItems()
{
    return ((m_SellItems != NULL) ? true : false);
}

int32_t Creature::GetSlotByItemId(uint32_t itemid)
{
    uint32_t slot = 0;
    for (std::vector<CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
        {
            if (itr->itemid == itemid)
                return slot;
            else
                ++slot;
        }
    return -1;
}

uint32_t Creature::GetItemAmountByItemId(uint32_t itemid)
{
    for (std::vector<CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
        {
            if (itr->itemid == itemid)
                return ((itr->amount < 1) ? 1 : itr->amount);
        }
    return 0;
}

void Creature::GetSellItemBySlot(uint32_t slot, CreatureItem& ci)
{
    ci = m_SellItems->at(slot);
}

void Creature::GetSellItemByItemId(uint32_t itemid, CreatureItem& ci)
{
    for (std::vector<CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
        {
            if (itr->itemid == itemid)
            {
                ci = (*itr);
                return;
            }
        }
    ci.amount = 0;
    ci.max_amount = 0;
    ci.available_amount = 0;
    ci.incrtime = 0;
    ci.itemid = 0;
}

WDB::Structures::ItemExtendedCostEntry const* Creature::GetItemExtendedCostByItemId(uint32_t itemid)
{
    for (auto itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
    {
        if (itr->itemid == itemid)
          return itr->extended_cost;
    }
    return nullptr;
}

std::vector<CreatureItem>::iterator Creature::GetSellItemBegin()
{
    return m_SellItems->begin();
}

std::vector<CreatureItem>::iterator Creature::GetSellItemEnd()
{
    return m_SellItems->end();
}


size_t Creature::GetSellItemCount()
{
    return m_SellItems->size();
}

void Creature::RemoveVendorItem(uint32_t itemid)
{
    for (auto itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
    {
        if (itr->itemid == itemid)
        {
            m_SellItems->erase(itr);
            return;
        }
    }
}

void Creature::PrepareForRemove()
{
    removeAllAuras();

    getSummonInterface()->removeAllSummons();

    if (!IsInWorld())
        return;

    if (getCreatedByGuid() != 0)
    {

        Unit* summoner = getWorldMapUnit(getCreatedByGuid());
        if (summoner != NULL)
        {
#if VERSION_STRING > TBC
            if (summoner->getCritterGuid() == getGuid())
                summoner->setCritterGuid(0);
#endif

            if (getCreatedBySpellId() != 0)
                summoner->removeAllAurasById(getCreatedBySpellId());
        }
    }

    if (getWorldMap()->getBaseMap()->getMapInfo() && getWorldMap()->getBaseMap()->getMapInfo()->isRaid())
    {
        if (GetCreatureProperties()->Rank == 3)
        {
            getWorldMap()->removeCombatInProgress(getGuid());
        }
    }
}

bool Creature::IsExotic()
{
    return creature_properties->isExotic();
}

bool Creature::isCritter()
{
    if (creature_properties->Type == UNIT_TYPE_CRITTER)
        return true;
    else
        return false;
}

void Creature::die(Unit* pAttacker, uint32_t /*damage*/, uint32_t spellid)
{
#ifdef FT_VEHICLES
    // Exit Vehicle
    callExitVehicle();
#endif

    //general hook for die
    if (pAttacker != nullptr && !sHookInterface.OnPreUnitDie(pAttacker, this))
        return;

    // on die and an target die proc
    {
        SpellInfo const* killerspell;
        if (spellid)
            killerspell = sSpellMgr.getSpellInfo(spellid);
        else killerspell = NULL;
    }

    setDeathState(JUST_DIED);
    getAIInterface()->enterEvadeMode();

    if (getChannelObjectGuid() != 0)
    {
        Spell* spl = getCurrentSpell(CURRENT_CHANNELED_SPELL);

        if (spl != NULL)
        {

            for (uint8_t i = 0; i < 3; i++)
            {
                if (spl->getSpellInfo()->getEffect(i) == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                {
                    uint64_t guid = getChannelObjectGuid();
                    DynamicObject* dObj = getWorldMap()->getDynamicObject(WoWGuid::getGuidLowPartFromUInt64(guid));
                    if (!dObj)
                        return;

                    dObj->remove();
                }
            }

            if (spl->getSpellInfo()->getChannelInterruptFlags() == 48140)
                interruptSpell(spl->getSpellInfo()->getId());
        }
    }

    // Stop players from casting
    for (const auto& itr : getInRangePlayersSet())
    {
        Unit* attacker = static_cast<Unit*>(itr);
        if (attacker && attacker->isCastingSpell())
        {
            for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
            {
                Spell* curSpell = attacker->getCurrentSpell(CurrentSpellType(i));
                if (curSpell != nullptr && curSpell->m_targets.getUnitTargetGuid() == getGuid())
                    attacker->interruptSpellWithSpellType(CurrentSpellType(i));
            }
        }
    }

    smsg_AttackStop(this);
    setHealth(0);

    removeAllNonPersistentAuras();

    if (pAttacker != nullptr)
    {
        if (pAttacker->IsInWorld() && pAttacker->isCreature() && dynamic_cast<Creature*>(pAttacker)->GetScript())
        {
            dynamic_cast<Creature*>(pAttacker)->GetScript()->_internalOnTargetDied(this);
            dynamic_cast<Creature*>(pAttacker)->GetScript()->OnTargetDied(this);
        }

        pAttacker->getAIInterface()->eventOnTargetDied(this);
        pAttacker->smsg_AttackStop(this);
    }

    // TODO: npc summons and pets should not unsummon on owner death
    // correct behaviour needs more investigation
    getSummonInterface()->removeAllSummons();

    // Clear Threat
    getThreatManager().clearAllThreat();
    getThreatManager().removeMeFromThreatLists();

    getCombatHandler().clearCombat();

    // Add Kills if Player is in Vehicle
    if (pAttacker != nullptr && pAttacker->isVehicle())
    {
        Unit* vehicle_owner = getWorldMap()->getUnit(pAttacker->getCharmedByGuid());

        if (vehicle_owner != nullptr && vehicle_owner->isPlayer())
        {
            sQuestMgr.OnPlayerKill(static_cast<Player*>(vehicle_owner), this, true);
        }
     }

    addUnitFlags(UNIT_FLAG_DEAD);

    Player* looter = nullptr;
    if (getTaggerGuid())
    {
        if (Unit* tagger = m_WorldMap->getUnit(getTaggerGuid()))
            looter = tagger->ToPlayer();
    }
    else if (pAttacker != nullptr && pAttacker->isPlayer())
    {
        looter = pAttacker->ToPlayer();
    }
    else if (pAttacker != nullptr && pAttacker->isCreature())
    {
        looter = pAttacker->getPlayerOwner();
    }   
    
    // Setup Loot and Round Robin Player in group case
    if (looter)
    {
        if (const auto group = looter->getGroup())
        {
            if (group->GetLooter())
            {
                // just incase he is not online
                looter = sObjectMgr.getPlayer(group->GetLooter()->guid);
                if (looter)
                {
                    setTaggerGuid(looter->getGuid()); // set Tagger to the allowed looter.
                    group->sendLooter(this, looter);
                }
                else
                {
                    group->sendLooter(this, nullptr);
                }
            }
            else
            {
                group->sendLooter(this, nullptr);
            }

            group->updateLooterGuid(this);
        }
        else
        {
            looter->sendLooter(this);
        }

        // Generate Loot
        sLootMgr.fillCreatureLoot(looter, &loot, getEntry(), m_WorldMap->getDifficulty());

        // Generate Gold
        loot.generateGold(sMySQLStore.getCreatureProperties(getEntry()), getAIInterface()->getDifficultyType());

        if (loot.items.empty())
            return;

        // Master Looting Ninja Checker
        if (worldConfig.player.deactivateMasterLootNinja)
        {
            looter = sObjectMgr.getPlayer(static_cast<uint32_t>(this->getTaggerGuid()));
            if (looter && looter->getGroup() && looter->getGroup()->GetMethod() == PARTY_LOOT_MASTER_LOOTER)
            {
                uint16_t lootThreshold = looter->getGroup()->GetThreshold();

                for (auto itr = loot.items.begin(); itr != loot.items.end(); ++itr)
                {
                    if (itr->itemproto->Quality < lootThreshold)
                        continue;

                    // Master Loot Stuff - Let the rest of the raid know what dropped..
                    ///\todo Shouldn't we move this array to a global position? Or maybe it already exists^^ (VirtualAngel) --- I can see (dead) talking pigs...^^
                    const char* itemColours[8] = { "9d9d9d", "ffffff", "1eff00", "0070dd", "a335ee", "ff8000", "e6cc80", "e6cc80" };
                    char buffer[256];
                    sprintf(buffer, "\174cff%s\174Hitem:%u:0:0:0:0:0:0:0\174h[%s]\174h\174r", itemColours[itr->itemproto->Quality], itr->itemproto->ItemId, itr->itemproto->Name.c_str());
                    this->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, buffer);
                }
            }
        }
    }
    else
    {
        sLogger.debug("no loot owner found loot will not be filled for creature {}", getEntry());
    }

    if (getCharmedByGuid())
    {
        //remove owner warlock soul link from caster
        Unit* owner = getWorldMap()->getUnit(getCharmedByGuid());

        if (owner != NULL && owner->isPlayer())
            static_cast<Player*>(owner)->eventDismissPet();
    }

    if (getCharmedByGuid() != 0)
    {
        Unit* charmer = m_WorldMap->getUnit(getCharmedByGuid());
        if (charmer != NULL)
            charmer->unPossess();
    }

    // Clear health batch on death
    clearHealthBatch();

    // Update movement flags on death
    immediateMovementFlagsUpdate();

    if (m_WorldMap->getBaseMap()->isBattlegroundOrArena() && reinterpret_cast<BattlegroundMap*>(m_WorldMap)->getBattleground())
        reinterpret_cast<BattlegroundMap*>(m_WorldMap)->getBattleground()->HookOnUnitDied(this);
}

/// \todo implement localization support
// 1. Chat Areas (Area, Map, World)
// 2. WorldPacket... support for MONSTER_SAY
// 3. data resize, map with players (PlayerSession)
// 4. Sending localizations if available... puh
void Creature::SendScriptTextChatMessage(uint32_t textid, Unit* target/* = target*/)
{
    SendCreatureChatMessageInRange(this, textid, target);
}

void Creature::SendScriptTextChatMessageByIndex(uint32_t textid, Unit* target/* = target*/)
{
    auto text = sMySQLStore.getNpcScriptTextById(getEntry(), textid);

    if (text)
        SendCreatureChatMessageInRange(this, text->id, target);
    else
        sLogger.failure("Creature::SendScriptTextChatMessageByIndex: Invalid textId");
}

void Creature::SendTimedScriptTextChatMessage(uint32_t textid, uint32_t delay, Unit* target/* = nullptr*/)
{
    if (delay > 0)
    {
        sEventMgr.AddEvent(this, &Creature::SendTimedScriptTextChatMessage, textid, uint32_t(0), target, EVENT_UNIT_CHAT_MSG, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        return;
    }

    SendCreatureChatMessageInRange(this, textid, target);
}

uint32_t Creature::GetType()
{
    return m_Creature_type;
}

void Creature::SetType(uint32_t t)
{
    m_Creature_type = t;
}

void Creature::setRespawnTime(uint32_t respawn)
{
    m_respawnTime = respawn != 0 ? std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) + respawn : 0;
}

void Creature::buildPetSpellList(WorldPacket& data)
{
    data << uint64_t(getGuid());
    data << uint16_t(creature_properties->Family);
    data << uint32_t(0);

    if (!isVehicle())
        data << uint32_t(0);
    else
        data << uint32_t(0x8000101);

    std::vector<uint32_t>::const_iterator itr = creature_properties->castable_spells.begin();

    // Send the actionbar
    for (uint8_t i = 0; i < 10; ++i)
    {
        if (itr != creature_properties->castable_spells.end())
        {
            const auto spell = *itr;
            const uint32_t actionButton = uint32_t(spell) | uint32_t(i + 8) << 24;
            data << uint32_t(actionButton);
            ++itr;
        }
        else
        {
            data << uint16_t(0);
            data << uint8_t(0);
            data << uint8_t(i + 8);
        }
    }

    data << uint8_t(0);
    // cooldowns
    data << uint8_t(0);
}

CreatureMovementData const& Creature::getMovementTemplate()
{
    if (CreatureMovementData const* movementOverride = sObjectMgr.getCreatureMovementOverride(spawnid))
        return *movementOverride;

    return GetCreatureProperties()->Movement;
}

void Creature::InitSummon(Object* summoner)
{
    if (summoner)
    {
        if (summoner->IsInWorld() && summoner->isCreature() && static_cast<Creature*>(summoner)->GetScript())
            static_cast<Creature*>(summoner)->GetScript()->onSummonedCreature(this);

        if (GetScript() != nullptr)
            GetScript()->OnSummon(static_cast<Unit*>(summoner));
    }
}

bool Creature::updateEntry(uint32_t entry)
{
    CreatureProperties const* cInfo = sMySQLStore.getCreatureProperties(entry);
    if (!cInfo)
        return false;

    if (_myScriptClass)
    {
        _myScriptClass->Destroy();
        _myScriptClass = nullptr;
    }

    Load(cInfo, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
    LoadScript();

    motion_Initialize();

    // When we are Still Alive call OnLoad for new CreatureAIScript
    if (isAlive())
    {
        if (_myScriptClass)
            _myScriptClass->OnLoad();
    }

    return true;
}
