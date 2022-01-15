/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Objects/DynamicObject.h"
#include "Macros/ScriptMacros.hpp"
#include "Management/AuctionMgr.h"
#include "Management/QuestMgr.h"
#include "Management/Quest.h"
#include "Management/GameEvent.h"
#include "Management/Skill.hpp"
#include "Management/Battleground/Battleground.h"
#include "Objects/Units/Stats.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Map/MapCell.h"
#include "Map/MapMgr.h"
#include "Map/WorldCreator.h"
#include "Spell/Definitions/PowerType.hpp"
#include "Pet.h"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Storage/MySQLStructures.h"
#include "Management/ObjectMgr.h"
#include "Server/Script/CreatureAIScript.h"
#include "Objects/Units/Creatures/CreatureGroups.h"

using namespace AscEmu::Packets;

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
    m_useAI = true;

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
bool Creature::isTaxi() const { return getNpcFlags() & UNIT_NPC_FLAG_FLIGHTMASTER; }
bool Creature::isCharterGiver() const { return getNpcFlags() & UNIT_NPC_FLAG_PETITIONER; }
bool Creature::isGuildBank() const { return getNpcFlags() & UNIT_NPC_FLAG_GUILD_BANKER; }
bool Creature::isBattleMaster() const { return getNpcFlags() & UNIT_NPC_FLAG_BATTLEMASTER; }
bool Creature::isBanker() const { return getNpcFlags() & UNIT_NPC_FLAG_BANKER; }
bool Creature::isInnkeeper() const { return getNpcFlags() & UNIT_NPC_FLAG_INNKEEPER; }
bool Creature::isSpiritHealer() const { return getNpcFlags() & UNIT_NPC_FLAG_SPIRITHEALER; }
bool Creature::isTabardDesigner() const { return getNpcFlags() & UNIT_NPC_FLAG_TABARDDESIGNER; }
bool Creature::isAuctioneer() const { return getNpcFlags() & UNIT_NPC_FLAG_AUCTIONEER; }
bool Creature::isStableMaster() const { return getNpcFlags() & UNIT_NPC_FLAG_STABLEMASTER; }
bool Creature::isArmorer() const { return getNpcFlags() & UNIT_NPC_FLAG_REPAIR; }
#if VERSION_STRING >= Cata
bool Creature::isTransmog() const { return getNpcFlags() & UNIT_NPC_FLAG_TRANSMOGRIFIER; }
bool Creature::isReforger() const { return getNpcFlags() & UNIT_NPC_FLAG_REFORGER; }
bool Creature::isVoidStorage() const { return getNpcFlags() & UNIT_NPC_FLAG_VAULTKEEPER; }
#endif

bool Creature::isVehicle() const
{
    return creature_properties->vehicleid != 0;
}

bool Creature::isTrainingDummy()
{
    return creature_properties->isTrainingDummy;
}

bool Creature::isPvpFlagSet()
{
    return getPvpFlags() & U_FIELD_BYTES_FLAG_PVP;
}

void Creature::setPvpFlag()
{
    addPvpFlags(U_FIELD_BYTES_FLAG_PVP);
    getSummonInterface()->setPvPFlags(true);
}

void Creature::removePvpFlag()
{
    removePvpFlags(U_FIELD_BYTES_FLAG_PVP);
    getSummonInterface()->setPvPFlags(false);
}

bool Creature::isFfaPvpFlagSet()
{
    return getPvpFlags() & U_FIELD_BYTES_FLAG_FFA_PVP;
}

void Creature::setFfaPvpFlag()
{
    addPvpFlags(U_FIELD_BYTES_FLAG_FFA_PVP);
    getSummonInterface()->setFFAPvPFlags(true);
}

void Creature::removeFfaPvpFlag()
{
    removePvpFlags(U_FIELD_BYTES_FLAG_FFA_PVP);
    getSummonInterface()->setFFAPvPFlags(false);
}

bool Creature::isSanctuaryFlagSet()
{
    return getPvpFlags() & U_FIELD_BYTES_FLAG_SANCTUARY;
}

void Creature::setSanctuaryFlag()
{
    addPvpFlags(U_FIELD_BYTES_FLAG_SANCTUARY);
    getSummonInterface()->setSanctuaryFlags(true);
}

void Creature::removeSanctuaryFlag()
{
    removePvpFlags(U_FIELD_BYTES_FLAG_SANCTUARY);
    getSummonInterface()->setSanctuaryFlags(false);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Owner
Player* Creature::getPlayerOwner()
{
    if (getCharmedByGuid() != 0)
    {
        const auto charmerUnit = GetMapMgrUnit(getCharmedByGuid());
        if (charmerUnit != nullptr && charmerUnit->isPlayer())
            return dynamic_cast<Player*>(charmerUnit);
    }

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

std::vector<CreatureItem>* Creature::getSellItems()
{
    return m_SellItems;
}

void Creature::setDeathState(DeathState s)
{
    if (s == ALIVE)
        this->removeUnitFlags(UNIT_FLAG_DEAD);

    if (s == JUST_DIED)
    {
        stopMoving();
        m_deathState = CORPSE;
        m_corpseEvent = true;

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
    }
    else
    {
        m_deathState = s;
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
    if (GetCreatureProperties()->gossipId)
        sScriptMgr.register_creature_gossip(getEntry(), new DatabaseGossip(GetCreatureProperties()->gossipId));
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
    if (getCharmerOrOwnerGUID())
        return;

    // Set the movement flags if the creature is in that mode. (Only fly if actually in air, only swim if in water, etc)
    const float ground = GetMapMgr()->GetLandHeight(GetPositionX(), GetPositionY(), GetPositionZ());

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

    if (time_passed >= m_movementFlagUpdateTimer)
    {
        updateMovementFlags();
        m_movementFlagUpdateTimer = 1000;
    }
    else
    {
        m_movementFlagUpdateTimer -= static_cast<uint16_t>(time_passed);
    }

    if (m_corpseEvent)
    {
        sEventMgr.RemoveEvents(this, EVENT_CREATURE_REMOVE_CORPSE);

        if (loot.gold > 0 || !loot.items.empty())
        {
            switch (this->creature_properties->Rank)
            {
                case ELITE_ELITE:
                    sEventMgr.AddEvent(this, &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, worldConfig.corpseDecay.eliteTimeInSeconds, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                    break;
                case ELITE_RAREELITE:
                    sEventMgr.AddEvent(this, &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, worldConfig.corpseDecay.rareEliteTimeInSeconds, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                    break;
                case ELITE_WORLDBOSS:
                    sEventMgr.AddEvent(this, &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, worldConfig.corpseDecay.worldbossTimeInSeconds, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                    break;
                case ELITE_RARE:
                    sEventMgr.AddEvent(this, &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, worldConfig.corpseDecay.rareTimeInSeconds, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                    break;
                default:
                    sEventMgr.AddEvent(this, &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, worldConfig.corpseDecay.normalTimeInSeconds, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                    break;
            }
        }
        else
        {
            sEventMgr.AddEvent(this, &Creature::OnRemoveCorpse, EVENT_CREATURE_REMOVE_CORPSE, worldConfig.corpseDecay.normalTimeInSeconds, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }

        m_corpseEvent = false;
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
    if (IsInWorld() && (int32)m_mapMgr->GetInstanceID() == m_instanceId)
    {
        sLogger.info("Removing corpse of " I64FMT "...", getGuid());

        setDeathState(DEAD);
        m_position = m_spawnLocation;

        // if corpse was removed during falling, the falling will continue and override relocation to respawn position
        if (IsFalling())
            stopMoving();

        setMoveCanFly(false);

        getMovementManager()->clear();

        if ((GetMapMgr()->GetMapInfo() && GetMapMgr()->GetMapInfo()->isRaid() && creature_properties->isBoss) || m_noRespawn)
        {
            RemoveFromWorld(false, true);
        }
        else
        {
            if (creature_properties->RespawnTime || m_respawnTimeOverride)
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

void Creature::OnRespawn(MapMgr* m)
{
    if (m_noRespawn)
        return;

    // if corpse was removed during falling, the falling will continue and override relocation to respawn position
    if (IsFalling())
        stopMoving();

    getMovementManager()->clear();

    if (m->pInstance)
    {
        auto encounters = sObjectMgr.GetDungeonEncounterList(m->GetMapId(), m->pInstance->m_difficulty);

        Instance* pInstance = m->pInstance;
        if (encounters != NULL && pInstance != NULL)
        {
            bool skip = false;
            for (auto killedNpc : pInstance->m_killedNpcs)
            {
                // is Killed add?
                if (killedNpc == getSpawnId())
                {
                    auto data = sMySQLStore.getSpawnGroupDataBySpawn(killedNpc);

                    // When Our Add is bound to a Boss thats not killed Respawn it
                    if (data && data->bossId)
                    {
                        if (pInstance->m_killedNpcs.find(data->bossId) != pInstance->m_killedNpcs.end())
                        {
                            skip = true;
                            break;
                        }
                        else
                        {
                            skip = false;
                            break;
                        }
                    }
                }

                // Is killed boss?
                if (killedNpc == getEntry())
                {
                    for (DungeonEncounterList::const_iterator itr = encounters->begin(); itr != encounters->end(); ++itr)
                    {
                        DungeonEncounter const* encounter = *itr;
                        if (encounter->creditType == ENCOUNTER_CREDIT_KILL_CREATURE && encounter->creditEntry == killedNpc)
                        {
                            skip = true;
                            break;
                        }
                    }
                }
            }

            if (skip)
            {
                m_noRespawn = true;
                DeleteMe();
                return;
            }
        }

        // Remove from Killed Npcs
        pInstance->m_killedNpcs.erase(getSpawnId());
        sInstanceMgr.SaveInstanceToDB(pInstance);
    }

    sLogger.info("Respawning " I64FMT "...", getGuid());
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

    m_PickPocketed = false;
    PushToWorld(m);
}

void Creature::Create(uint32 mapid, float x, float y, float z, float ang)
{
    Object::_Create(mapid, x, y, z, ang);
}

void Creature::CreateWayPoint(uint32 /*WayPointID*/, uint32 mapid, float x, float y, float z, float ang)
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
        m_spawn->id = spawnid = sObjectMgr.GenerateCreatureSpawnID();
        m_spawn->movetype = getDefaultMovementType();
        m_spawn->displayid = getDisplayId();
        m_spawn->x = m_position.x;
        m_spawn->y = m_position.y;
        m_spawn->z = m_position.z;
        m_spawn->o = m_position.o;
        m_spawn->emote_state = getEmoteState();
        m_spawn->flags = getUnitFlags();
        m_spawn->factionid = getFactionTemplate();
        m_spawn->bytes0 = getBytes0();
        m_spawn->bytes1 = getBytes1();
        m_spawn->bytes2 = getBytes2();
        m_spawn->stand_state = getStandState();
        m_spawn->death_state = 0;
        m_spawn->channel_target_creature = 0;
        m_spawn->channel_target_go = 0;
        m_spawn->channel_spell = 0;
        m_spawn->MountedDisplayID = getMountDisplayId();

        m_spawn->Item1SlotEntry = 0;
        m_spawn->Item2SlotEntry = 0;
        m_spawn->Item3SlotEntry = 0;

        m_spawn->Item1SlotDisplay = getVirtualItemSlotId(MELEE);
        m_spawn->Item2SlotDisplay = getVirtualItemSlotId(OFFHAND);
        m_spawn->Item3SlotDisplay = getVirtualItemSlotId(RANGED);

        if (IsFlying())
            m_spawn->CanFly = 1;
        else
            m_spawn->CanFly = 0;

        m_spawn->phase = m_phase;

        uint32 x = GetMapMgr()->GetPosX(GetPositionX());
        uint32 y = GetMapMgr()->GetPosY(GetPositionY());

        // Add spawn to map
        GetMapMgr()->GetBaseMap()->GetSpawnsListAndCreate(x, y)->CreatureSpawns.push_back(m_spawn);
    }

    std::stringstream ss;

    ss << "DELETE FROM creature_spawns WHERE id = ";
    ss << spawnid;
    ss << " AND min_build <= ";
    ss << VERSION_STRING;
    ss << " AND max_build >= ";
    ss << VERSION_STRING;
    ss << ";";

    WorldDatabase.Execute(ss.str().c_str());

    ss.rdbuf()->str("");

    ss << "INSERT INTO creature_spawns VALUES("
        << spawnid << ","
        << VERSION_STRING << ","
        << VERSION_STRING << ","
        << getEntry() << ","
        << GetMapId() << ","
        << m_position.x << ","
        << m_position.y << ","
        << m_position.z << ","
        << m_position.o << ","
        << getDefaultMovementType() << ","
        << getDisplayId() << ","
        << getFactionTemplate() << ","
        << getUnitFlags() << ","
        << getBytes0() << ","
        << getBytes1() << ","
        << getBytes2() << ","
        << getEmoteState() << ",0,";

    ss << m_spawn->channel_spell << ","
        << m_spawn->channel_target_go << ","
        << m_spawn->channel_target_creature << ",";

    ss << uint32(getStandState()) << ",";

    ss << m_spawn->death_state << ",";

    ss << getMountDisplayId() << ","
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

    WorldDatabase.Execute("DELETE FROM creature_spawns WHERE id = %u AND min_build <= %u AND max_build >= %u", GetSQL_id(), VERSION_STRING, VERSION_STRING);
}


//////////////////////////////////////////////////////////////////////////////////////////
/// Quests

void Creature::AddQuest(QuestRelation* Q)
{
    m_quests->push_back(Q);
}

void Creature::DeleteQuest(QuestRelation* Q)
{
    std::list<QuestRelation*>::iterator it;
    for (it = m_quests->begin(); it != m_quests->end(); ++it)
    {
        if (((*it)->type == Q->type) && ((*it)->qst == Q->qst))
        {
            delete(*it);
            m_quests->erase(it);
            break;
        }
    }
}

QuestProperties const* Creature::FindQuest(uint32 quest_id, uint8 quest_relation)
{
    std::list<QuestRelation*>::iterator it;
    for (it = m_quests->begin(); it != m_quests->end(); ++it)
    {
        QuestRelation* ptr = (*it);

        if ((ptr->qst->id == quest_id) && (ptr->type & quest_relation))
        {
            return ptr->qst;
        }
    }
    return nullptr;
}

uint16 Creature::GetQuestRelation(uint32 quest_id)
{
    uint16 quest_relation = 0;
    std::list<QuestRelation*>::iterator it;

    for (it = m_quests->begin(); it != m_quests->end(); ++it)
    {
        if ((*it)->qst->id == quest_id)
        {
            quest_relation |= (*it)->type;
        }
    }
    return quest_relation;
}

uint32 Creature::NumOfQuests()
{
    return (uint32)m_quests->size();
}

std::list<QuestRelation*>::iterator Creature::QuestsBegin()
{
    return m_quests->begin();
}

std::list<QuestRelation*>::iterator Creature::QuestsEnd()
{
    return m_quests->end();
}

void Creature::SetQuestList(std::list<QuestRelation*>* qst_lst)
{
    m_quests = qst_lst;
}

uint32 Creature::GetHealthFromSpell()
{
    return m_healthfromspell;
}

void Creature::SetHealthFromSpell(uint32 value)
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

bool Creature::HasQuest(uint32 id, uint32 type)
{
    if (!m_quests) return false;
    for (std::list<QuestRelation*>::iterator itr = m_quests->begin(); itr != m_quests->end(); ++itr)
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

void Creature::AddToWorld(MapMgr* pMapMgr)
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
    uint32 delay = 0;
    if (addrespawnevent && (m_respawnTimeOverride > 0 || creature_properties->RespawnTime > 0))
        delay = m_respawnTimeOverride > 0 ? m_respawnTimeOverride : creature_properties->RespawnTime;

    Despawn(0, delay);
}

void Creature::RemoveFromWorld(bool free_guid)
{
    PrepareForRemove();
    Unit::RemoveFromWorld(free_guid);
}

void Creature::EnslaveExpire()
{
    ++m_enslaveCount;

    uint64 charmer = getCharmedByGuid();

    Player* caster = sObjectMgr.GetPlayer(WoWGuid::getGuidLowPartFromUInt64(charmer));
    if (caster)
    {
        caster->setCharmGuid(0);
        caster->setSummonGuid(0);

        WorldPacket data(SMSG_PET_SPELLS, 8);

        data << uint64(0);
        data << uint32(0);

        caster->SendPacket(&data);
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

    getAIInterface()->Init(this, AI_SCRIPT_AGRO);

    updateInRangeOppositeFactionSet();
    updateInRangeSameFactionSet();
}

uint32 Creature::GetEnslaveCount()
{
    return m_enslaveCount;
}

void Creature::SetEnslaveCount(uint32 count)
{
    m_enslaveCount = count;
}

uint32 Creature::GetEnslaveSpell()
{
    return m_enslaveSpell;
}

void Creature::SetEnslaveSpell(uint32 spellId)
{
    m_enslaveSpell = spellId;
}

bool Creature::RemoveEnslave()
{
    return RemoveAura(m_enslaveSpell);
}

void Creature::CalcResistance(uint8_t type)
{
    int32 pos = 0;
    int32 neg = 0;

    if (BaseResistanceModPct[type] < 0)
        neg = (BaseResistance[type] * abs(BaseResistanceModPct[type]) / 100);
    else
        pos = (BaseResistance[type] * BaseResistanceModPct[type]) / 100;

    if (isPet() && isAlive() && IsInWorld())
    {
        Player* owner = static_cast<Pet*>(this)->getPlayerOwner();
        if (type == 0 && owner)
            pos += int32(0.35f * owner->getResistance(type));
        else if (owner)
            pos += int32(0.40f * owner->getResistance(type));
    }

    if (ResistanceModPct[type] < 0)
        neg += (BaseResistance[type] + pos - neg) * abs(ResistanceModPct[type]) / 100;
    else
        pos += (BaseResistance[type] + pos - neg) * ResistanceModPct[type] / 100;

    if (FlatResistanceMod[type] < 0)
        neg += abs(FlatResistanceMod[type]);
    else
        pos += FlatResistanceMod[type];

#if VERSION_STRING > Classic
    setResistanceBuffModPositive(type, pos);
    setResistanceBuffModNegative(type, neg);
#endif

    int32 tot = BaseResistance[type] + pos - neg;

    setResistance(type, tot > 0 ? tot : 0);
}

void Creature::CalcStat(uint8_t type)
{
    int32 pos = 0;
    int32 neg = 0;

    if (StatModPct[type] < 0)
        neg = (BaseStats[type] * abs(StatModPct[type]) / 100);
    else
        pos = (BaseStats[type] * StatModPct[type]) / 100;

    if (isPet())
    {
        Player* owner = static_cast<Pet*>(this)->getPlayerOwner();
        if (type == STAT_STAMINA && owner)
            pos += int32(0.45f * owner->getStat(STAT_STAMINA));
        else if (type == STAT_INTELLECT && owner && getCreatedBySpellId())
            pos += int32(0.30f * owner->getStat(STAT_INTELLECT));
    }

    if (TotalStatModPct[type] < 0)
        neg += (BaseStats[type] + pos - neg) * abs(TotalStatModPct[type]) / 100;
    else
        pos += (BaseStats[type] + pos - neg) * TotalStatModPct[type] / 100;

    if (FlatStatMod[type] < 0)
        neg += abs(FlatStatMod[type]);
    else
        pos += FlatStatMod[type];

#if VERSION_STRING != Classic
    setPosStat(type, pos);
    setNegStat(type, neg);
#endif

    int32 tot = BaseStats[type] + pos - neg;
    setStat(type, tot > 0 ? tot : 0);

    switch (type)
    {
        case STAT_STRENGTH:
        {
            //Attack Power
            if (!isPet())  //We calculate pet's later
            {
                uint32 str = getStat(STAT_STRENGTH);
                int32 AP = (str * 2 - 20);
                if (AP < 0) AP = 0;
                setAttackPower(AP);
            }
            CalcDamage();
        }
        break;
        case STAT_AGILITY:
        {
            //Ranged Attack Power (Does any creature use this?)
            int32 RAP = getLevel() + getStat(STAT_AGILITY) - 10;
            if (RAP < 0)
                RAP = 0;

            setRangedAttackPower(RAP);
        }
        break;
        case STAT_STAMINA:
        {
#if VERSION_STRING != Classic
            //Health
            uint32 hp = getBaseHealth();
            uint32 stat_bonus = getPosStat(STAT_STAMINA)- getNegStat(STAT_STAMINA);
            if (static_cast<int32>(stat_bonus) < 0) stat_bonus = 0;

            uint32 bonus = stat_bonus * 10 + m_healthfromspell;
            uint32 res = hp + bonus;

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
                uint32 mana = getBaseMana();
                uint32 stat_bonus = getPosStat(STAT_INTELLECT) - getNegStat(STAT_INTELLECT);
                if (static_cast<int32>(stat_bonus) < 0) stat_bonus = 0;

                uint32 bonus = stat_bonus * 15;
                uint32 res = mana + bonus;

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

    uint32 cur = getHealth();
    uint32 mh = getMaxHealth();
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

        if (PctRegenModifier)
            amt += (amt * PctRegenModifier) / 100;

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
        cur += (uint32)amt;
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

Trainer* Creature::GetTrainer()
{
    return mTrainer;
}

#if VERSION_STRING < Cata
void Creature::AddVendorItem(uint32 itemid, uint32 amount, DBC::Structures::ItemExtendedCostEntry const* ec)
#else
void Creature::AddVendorItem(uint32 itemid, uint32 amount, DB2::Structures::ItemExtendedCostEntry const* ec)
#endif
{
    CreatureItem ci;
    ci.amount = amount;
    ci.itemid = itemid;
    ci.available_amount = 0;
    ci.max_amount = 0;
    ci.incrtime = 0;
    ci.extended_cost = ec;
    if (!m_SellItems)
    {
        m_SellItems = new std::vector < CreatureItem > ;
        sObjectMgr.SetVendorList(getEntry(), m_SellItems);
    }
    m_SellItems->push_back(ci);
}

void Creature::ModAvItemAmount(uint32 itemid, uint32 value)
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

void Creature::UpdateItemAmount(uint32 itemid)
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

void Creature::ChannelLinkUpGO(uint32 SqlId)
{
    if (!m_mapMgr)        // shouldn't happen
        return;

    GameObject* go = m_mapMgr->GetSqlIdGameObject(SqlId);
    if (go != nullptr)
    {
        event_RemoveEvents(EVENT_CREATURE_CHANNEL_LINKUP);
        setChannelObjectGuid(go->getGuid());
        setChannelSpellId(m_spawn->channel_spell);
    }
}

void Creature::ChannelLinkUpCreature(uint32 SqlId)
{
    if (!m_mapMgr)        // shouldn't happen
        return;

    Creature* creature = m_mapMgr->GetSqlIdCreature(SqlId);
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

uint8 get_byte(uint32 buffer, uint32 index)
{
    uint32 mask = uint32(~0ul);
    if (index > sizeof(uint32) - 1)
        return 0;

    buffer = buffer >> index * 8;
    mask = mask >> 3 * 8;
    buffer = buffer & mask;

    return (uint8)buffer;
}

bool Creature::Teleport(const LocationVector& vec, MapMgr* map)
{
    if (map == nullptr)
        return false;

    if (map->GetCreature(this->getGuidLow()))
    {
        this->SetPosition(vec);
        return true;
    }
    else
    {
        return false;
    }
}

bool Creature::Load(MySQLStructure::CreatureSpawn* spawn, uint8 mode, MySQLStructure::MapInfo const* info)
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

    uint32 health;
    if (creature_properties->MinHealth > creature_properties->MaxHealth)
    {
        sLogger.failure("MinHealth is bigger than MaxHealt! Using MaxHealth value. You should fix this in creature_proto table for entry: %u!", creature_properties->Id);
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

    EventModelChange();

    setLevel(creature_properties->MinLevel + (Util::getRandomUInt(creature_properties->MaxLevel - creature_properties->MinLevel)));

    if (mode && info)
        setLevel(std::min(73 - getLevel(), info->lvl_mod_a));

    for (uint8 i = 0; i < 7; ++i)
        setResistance(i, creature_properties->Resistances[i]);

    setBaseAttackTime(MELEE, creature_properties->AttackTime);

    setMinDamage(creature_properties->MinDamage);
    setMaxDamage(creature_properties->MaxDamage);

    setBaseAttackTime(RANGED, creature_properties->RangedAttackTime);
    setMinRangedDamage(creature_properties->RangedMinDamage);
    setMaxRangedDamage(creature_properties->RangedMaxDamage);

    setVirtualItemSlotId(MELEE, spawn->Item1SlotDisplay);
    setVirtualItemSlotId(OFFHAND, spawn->Item2SlotDisplay);
    setVirtualItemSlotId(RANGED, spawn->Item3SlotDisplay);

#if VERSION_STRING < WotLK
    setVirtualItemInfo(0, 0);
    setVirtualItemInfo(1, 0);
    setVirtualItemInfo(2, 0);

    setVirtualItemInfo(3, 0);
    setVirtualItemInfo(4, 0);
    setVirtualItemInfo(5, 0);
#endif

    setFaction(spawn->factionid);
    setUnitFlags(spawn->flags);
    setEmoteState(spawn->emote_state);
    setBoundingRadius(creature_properties->BoundingRadius);
    setCombatReach(creature_properties->CombatReach);
    original_emotestate = spawn->emote_state;

    // set position
    m_position.ChangeCoords({ spawn->x, spawn->y, spawn->z, spawn->o });
    m_spawnLocation.ChangeCoords({ spawn->x, spawn->y, spawn->z, spawn->o });
    m_aiInterface->timed_emotes = sObjectMgr.GetTimedEmoteList(spawn->id);

    // not a neutral creature
    if (!(m_factionEntry != nullptr && m_factionEntry->RepListId == -1 && m_factionTemplate->HostileMask == 0 && m_factionTemplate->FriendlyMask == 0))
    {
        getAIInterface()->setCanCallForHelp(true);
    }

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
        m_SellItems = sObjectMgr.GetVendorList(getEntry());

    if (isQuestGiver())
        _LoadQuests();

    if (isTrainer() || isProfessionTrainer())
        mTrainer = sObjectMgr.GetTrainer(getEntry());

    if (isAuctioneer())
        auctionHouse = sAuctionMgr.GetAuctionHouse(getEntry());

    //load resistances
    for (uint8 x = 0; x < TOTAL_SPELL_SCHOOLS; ++x)
        BaseResistance[x] = getResistance(x);
    for (uint8 x = 0; x < STAT_COUNT; ++x)
        BaseStats[x] = getStat(x);

    BaseDamage[0] = getMinDamage();
    BaseDamage[1] = getMaxDamage();
    BaseOffhandDamage[0] = getMinOffhandDamage();
    BaseOffhandDamage[1] = getMaxOffhandDamage();
    BaseRangedDamage[0] = getMinRangedDamage();
    BaseRangedDamage[1] = getMaxRangedDamage();
    BaseAttackType = creature_properties->attackSchool;

    setModCastSpeed(1.0f);   // better set this one
    setBytes0(spawn->bytes0);
    setBytes1(spawn->bytes1);
    setBytes2(spawn->bytes2);

    ////////////AI
    getAIInterface()->initialiseScripts(getEntry());
    getAIInterface()->eventOnLoad();

    if (!creature_properties->isTrainingDummy && !isVehicle())
    {
        getAIInterface()->setAllowedToEnterCombat(isattackable(spawn));
    }
    else
    {
        if (!isattackable(spawn))
        {
            getAIInterface()->setAllowedToEnterCombat(false);
            getAIInterface()->setAiScriptType(AI_SCRIPT_PASSIVE);
        }
        else
        {
            getAIInterface()->setAllowedToEnterCombat(true);
        }
    }

    //////////////AI

    myFamily = sCreatureFamilyStore.LookupEntry(creature_properties->Family);


    //HACK!
    if (getDisplayId() == 17743 ||
        getDisplayId() == 20242 ||
        getDisplayId() == 15435 ||
        (creature_properties->Family == UNIT_TYPE_MISC))
    {
        m_useAI = false;
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

    if (creature_properties->invisibility_type > INVIS_FLAG_NORMAL)
        // TODO: currently only invisibility type 15 is used for invisible trigger NPCs
        // these are always invisible to players
        modInvisibilityLevel(InvisibilityFlag(creature_properties->invisibility_type), 1);
    if (spawn->stand_state)
        setStandState((uint8)spawn->stand_state);

    m_aiInterface->eventAiInterfaceParamsetFinish();

    this->m_position.x = spawn->x;
    this->m_position.y = spawn->y;
    this->m_position.z = spawn->z;
    this->m_position.o = spawn->o;

    if (isVehicle())
    {
        createVehicleKit(creature_properties->vehicleid, creature_properties->Id);
        addNpcFlags(UNIT_NPC_FLAG_SPELLCLICK);
        setAItoUse(false);
    }

    if (getMovementTemplate().isRooted())
        setControlled(true, UNIT_STATE_ROOTED);

    return true;
}

void Creature::Load(CreatureProperties const* properties_, float x, float y, float z, float o)
{
    creature_properties = properties_;

    if (creature_properties->isTrainingDummy == 0 && !isVehicle())
    {
        getAIInterface()->setAllowedToEnterCombat(true);
    }
    else
    {
        getAIInterface()->setAllowedToEnterCombat(false);
        getAIInterface()->setAiScriptType(AI_SCRIPT_PASSIVE);
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

    uint32 health = creature_properties->MinHealth + Util::getRandomUInt(creature_properties->MaxHealth - creature_properties->MinHealth);

    setMaxHealth(health);
    setHealth(health);
    setBaseHealth(health);

    setMaxPower(POWER_TYPE_MANA, creature_properties->Mana);
    setBaseMana(creature_properties->Mana);
    setPower(POWER_TYPE_MANA, creature_properties->Mana);

    uint32 model = 0;
    uint8 gender = creature_properties->GetGenderAndCreateRandomDisplayID(&model);
    setGender(gender);

    setDisplayId(model);
    setNativeDisplayId(model);
    setMountDisplayId(0);

    EventModelChange();

    setLevel(creature_properties->MinLevel + (Util::getRandomUInt(creature_properties->MaxLevel - creature_properties->MinLevel)));

    for (uint8 i = 0; i < 7; ++i)
        setResistance(i, creature_properties->Resistances[i]);

    setBaseAttackTime(MELEE, creature_properties->AttackTime);
    setMinDamage(creature_properties->MinDamage);
    setMaxDamage(creature_properties->MaxDamage);


    setFaction(creature_properties->Faction);
    setBoundingRadius(creature_properties->BoundingRadius);
    setCombatReach(creature_properties->CombatReach);

    original_emotestate = 0;

    // set position
    m_position.ChangeCoords({ x, y, z, o });
    m_spawnLocation.ChangeCoords({ x, y, z, o });

    // not a neutral creature
    if (m_factionEntry && !(m_factionEntry->RepListId == -1 && m_factionTemplate->HostileMask == 0 && m_factionTemplate->FriendlyMask == 0))
    {
        getAIInterface()->setCanCallForHelp(true);
    }

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
        m_SellItems = sObjectMgr.GetVendorList(getEntry());

    if (isQuestGiver())
        _LoadQuests();

    if (isTrainer() || isProfessionTrainer())
        mTrainer = sObjectMgr.GetTrainer(getEntry());

    if (isAuctioneer())
        auctionHouse = sAuctionMgr.GetAuctionHouse(getEntry());

    //load resistances
    for (uint8 j = 0; j < TOTAL_SPELL_SCHOOLS; ++j)
        BaseResistance[j] = getResistance(j);
    for (uint8 j = 0; j < STAT_COUNT; ++j)
        BaseStats[j] = getStat(j);

    BaseDamage[0] = getMinDamage();
    BaseDamage[1] = getMaxDamage();
    BaseOffhandDamage[0] = getMinOffhandDamage();
    BaseOffhandDamage[1] = getMaxOffhandDamage();
    BaseRangedDamage[0] = getMinRangedDamage();
    BaseRangedDamage[1] = getMaxRangedDamage();
    BaseAttackType = creature_properties->attackSchool;

    setModCastSpeed(1.0f);   // better set this one

    ////////////AI
    getAIInterface()->initialiseScripts(getEntry());
    getAIInterface()->eventOnLoad();

    //////////////AI

    myFamily = sCreatureFamilyStore.LookupEntry(creature_properties->Family);


    // \todo remove this HACK! already included few lines above
    if (getDisplayId() == 17743 ||
        getDisplayId() == 20242 ||
        getDisplayId() == 15435 ||
        creature_properties->Type == UNIT_TYPE_MISC)
    {
        m_useAI = false;
    }

    setPowerType(POWER_TYPE_MANA);

    /*  // Dont was Used in old AIInterface left the code here if needed at other Date
    if (creature_properties->guardtype == GUARDTYPE_CITY)
        getAIInterface()->setGuard(true);
    else
        getAIInterface()->setGuard(false);*/

    if (creature_properties->guardtype == GUARDTYPE_NEUTRAL)
        getAIInterface()->setGuard(true);
    else
        getAIInterface()->setGuard(false);

    if (creature_properties->invisibility_type > INVIS_FLAG_NORMAL)
        // TODO: currently only invisibility type 15 is used for invisible trigger NPCs
        // these are always invisible to players
        modInvisibilityLevel(InvisibilityFlag(creature_properties->invisibility_type), 1);

    if (isVehicle())
    {
        createVehicleKit(creature_properties->vehicleid, creature_properties->Id);
        addNpcFlags(UNIT_NPC_FLAG_SPELLCLICK);
        setAItoUse(false);
    }

    if (getMovementTemplate().isRooted())
        setControlled(true, UNIT_STATE_ROOTED);
}

void Creature::OnPushToWorld()
{
    if (creature_properties == nullptr)
    {
        sLogger.failure("Something tried to push Creature with entry %u with invalid creature_properties!", getEntry());
        return;
    }

    std::set<uint32>::iterator itr = creature_properties->start_auras.begin();
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

    // Reset Instance Data
    // set encounter state back to NotStarted
    CALL_INSTANCE_SCRIPT_EVENT(GetMapMgr(), setData)(getEntry(), NotStarted);

    Unit::OnPushToWorld();

    if (_myScriptClass)
        _myScriptClass->OnLoad();

    if (m_spawn)
    {
        if (m_spawn->channel_target_creature)
        {
            sEventMgr.AddEvent(this, &Creature::ChannelLinkUpCreature, m_spawn->channel_target_creature, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);    // only 5 attempts
        }

        if (m_spawn->channel_target_go)
        {
            sEventMgr.AddEvent(this, &Creature::ChannelLinkUpGO, m_spawn->channel_target_go, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);    // only 5 attempts
        }
    }

    m_aiInterface->m_is_in_instance = (!m_mapMgr->GetMapInfo()->isNonInstanceMap()) ? true : false;

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
        {
            mEvent->mEventScript->OnCreaturePushToWorld(mEvent, this);
        }
    }
    CALL_INSTANCE_SCRIPT_EVENT(m_mapMgr, OnCreaturePushToWorld)(this);
}

void Creature::Despawn(uint32 delay, uint32 respawntime)
{
    if (delay)
    {
        sEventMgr.AddEvent(this, &Creature::Despawn, (uint32)0, respawntime, EVENT_CREATURE_RESPAWN, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
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
            MapCell* pCell = m_mapMgr->GetCellByCoords(m_spawnLocation.x, m_spawnLocation.y);
            if (pCell == nullptr)
                pCell = GetMapCell();

            if (pCell != nullptr)
            {
                pCell->_respawnObjects.insert(this);

                sEventMgr.RemoveEvents(this);
                sEventMgr.AddEvent(m_mapMgr, &MapMgr::EventRespawnCreature, this, pCell->GetPositionX(), pCell->GetPositionY(), EVENT_CREATURE_RESPAWN, respawntime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

                Unit::RemoveFromWorld(false);

                m_position = m_spawnLocation;
                m_respawnCell = pCell;
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

uint32 Creature::GetLineByFamily(DBC::Structures::CreatureFamilyEntry const* family)
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
    bInvincible = false;
}

uint32 Creature::GetNpcTextId()
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

int32 Creature::GetDamageDoneMod(uint16_t school)
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

    Group* thisGroup = plr->getGroup();
    if (!thisGroup)
        return  (getTaggerGuid() == plr->getGuid());

    switch (thisGroup->GetMethod())
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

uint32 Creature::GetSQL_id()
{
    return spawnid;
};

bool Creature::HasItems()
{
    return ((m_SellItems != NULL) ? true : false);
}

int32 Creature::GetSlotByItemId(uint32 itemid)
{
    uint32 slot = 0;
    for (std::vector<CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
        {
            if (itr->itemid == itemid)
                return slot;
            else
                ++slot;
        }
    return -1;
}

uint32 Creature::GetItemAmountByItemId(uint32 itemid)
{
    for (std::vector<CreatureItem>::iterator itr = m_SellItems->begin(); itr != m_SellItems->end(); ++itr)
        {
            if (itr->itemid == itemid)
                return ((itr->amount < 1) ? 1 : itr->amount);
        }
    return 0;
}

void Creature::GetSellItemBySlot(uint32 slot, CreatureItem& ci)
{
    ci = m_SellItems->at(slot);
}

void Creature::GetSellItemByItemId(uint32 itemid, CreatureItem& ci)
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

#if VERSION_STRING < Cata
DBC::Structures::ItemExtendedCostEntry const* Creature::GetItemExtendedCostByItemId(uint32 itemid)
#else
DB2::Structures::ItemExtendedCostEntry const* Creature::GetItemExtendedCostByItemId(uint32 itemid)
#endif
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

void Creature::RemoveVendorItem(uint32 itemid)
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
    RemoveAllAuras();

    getSummonInterface()->removeAllSummons();

    if (!IsInWorld())
        return;

    if (getCreatedByGuid() != 0)
    {

        Unit* summoner = GetMapMgrUnit(getCreatedByGuid());
        if (summoner != NULL)
        {
#if VERSION_STRING > TBC
            if (summoner->getCritterGuid() == getGuid())
                summoner->setCritterGuid(0);
#endif

            if (getCreatedBySpellId() != 0)
                summoner->RemoveAura(getCreatedBySpellId());
        }
    }

    if (GetMapMgr()->GetMapInfo() && GetMapMgr()->GetMapInfo()->isRaid())
    {
        if (GetCreatureProperties()->Rank == 3)
        {
            GetMapMgr()->RemoveCombatInProgress(getGuid());
        }
    }
}

bool Creature::IsExotic()
{
    if ((GetCreatureProperties()->typeFlags & CREATURE_FLAG1_EXOTIC) != 0)
        return true;

    return false;
}

bool Creature::isCritter()
{
    if (creature_properties->Type == UNIT_TYPE_CRITTER)
        return true;
    else
        return false;
}

void Creature::Die(Unit* pAttacker, uint32 /*damage*/, uint32 spellid)
{
    /*if (getVehicleComponent() != NULL)
    {
        getVehicleComponent()->RemoveAccessories();
        getVehicleComponent()->EjectAllPassengers();
    }

    // Creature falls off vehicle on death
    if ((m_currentVehicle != NULL))
        m_currentVehicle->EjectPassenger(this);*/

    //general hook for die
    if (!sHookInterface.OnPreUnitDie(pAttacker, this))
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

            for (uint8 i = 0; i < 3; i++)
            {
                if (spl->getSpellInfo()->getEffect(i) == SPELL_EFFECT_PERSISTENT_AREA_AURA)
                {
                    uint64 guid = getChannelObjectGuid();
                    DynamicObject* dObj = GetMapMgr()->GetDynamicObject(WoWGuid::getGuidLowPartFromUInt64(guid));
                    if (!dObj)
                        return;

                    dObj->Remove();
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
                if (curSpell != nullptr && curSpell->m_targets.getUnitTarget() == getGuid())
                    attacker->interruptSpellWithSpellType(CurrentSpellType(i));
            }
        }
    }

    smsg_AttackStop(this);
    setHealth(0);

    RemoveAllNonPersistentAuras();

    CALL_SCRIPT_EVENT(pAttacker, _internalOnTargetDied)(this);
    CALL_SCRIPT_EVENT(pAttacker, OnTargetDied)(this);
    pAttacker->getAIInterface()->eventOnTargetDied(this);

    pAttacker->smsg_AttackStop(this);

    getSummonInterface()->removeAllSummons();

    // Clear Threat
    getThreatManager().clearAllThreat();
    getThreatManager().removeMeFromThreatLists();

    getCombatHandler().clearCombat();

    // Add Kills if Player is in Vehicle
    if (pAttacker->isVehicle())
    {
        Unit* vehicle_owner = GetMapMgr()->GetUnit(pAttacker->getCharmedByGuid());

        if (vehicle_owner != nullptr && vehicle_owner->isPlayer())
        {
            sQuestMgr.OnPlayerKill(static_cast<Player*>(vehicle_owner), this, true);
        }
     }

    addUnitFlags(UNIT_FLAG_DEAD);

    Player* looter = nullptr;
    if (getTaggerGuid())
    {
        looter = m_mapMgr->GetUnit(getTaggerGuid())->ToPlayer();
    }
    else if (pAttacker->isPlayer())
    {
        looter = pAttacker->ToPlayer();
    }
    else if (pAttacker->isCreature())
    {
        looter = pAttacker->getPlayerOwner();
    }   
    
    // Setup Loot and Round Robin Player in group case
    if (looter)
    {
        if (Group* group = looter->getGroup())
        {
            if (group->GetLooter())
            {
                // just incase he is not online
                looter = sObjectMgr.GetPlayer(group->GetLooter()->guid);
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
        sLootMgr.fillCreatureLoot(looter, &loot, getEntry(), m_mapMgr->iInstanceMode);

        // Generate Gold
        loot.generateGold(sMySQLStore.getCreatureProperties(getEntry()), getAIInterface()->getDifficultyType());

        if (loot.items.empty())
            return;

        // Master Looting Ninja Checker
        if (worldConfig.player.deactivateMasterLootNinja)
        {
            Player* looter = sObjectMgr.GetPlayer(static_cast<uint32_t>(this->getTaggerGuid()));
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
        sLogger.debug("no loot owner found loot will not be filled for creature %u", getEntry());
    }

    if (getCharmedByGuid())
    {
        //remove owner warlock soul link from caster
        Unit* owner = GetMapMgr()->GetUnit(getCharmedByGuid());

        if (owner != NULL && owner->isPlayer())
            static_cast<Player*>(owner)->EventDismissPet();
    }

    if (getCharmedByGuid() != 0)
    {
        Unit* charmer = m_mapMgr->GetUnit(getCharmedByGuid());
        if (charmer != NULL)
            charmer->UnPossess();
    }

    // Clear health batch on death
    clearHealthBatch();

    // Update movement flags on death
    immediateMovementFlagsUpdate();

    if (m_mapMgr->m_battleground != NULL)
        m_mapMgr->m_battleground->HookOnUnitDied(this);
}

/// \todo implement localization support
// 1. Chat Areas (Area, Map, World)
// 2. WorldPacket... support for MONSTER_SAY
// 3. data resize, map with players (PlayerSession)
// 4. Sending localizations if available... puh
void Creature::SendScriptTextChatMessage(uint32 textid, Unit* target/* = target*/)
{
    SendCreatureChatMessageInRange(this, textid, target);
}

void Creature::SendTimedScriptTextChatMessage(uint32 textid, uint32 delay, Unit* target/* = nullptr*/)
{
    if (delay > 0)
    {
        sEventMgr.AddEvent(this, &Creature::SendTimedScriptTextChatMessage, textid, uint32_t(0), target, EVENT_UNIT_CHAT_MSG, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        return;
    }

    SendCreatureChatMessageInRange(this, textid, target);
}

uint32 Creature::GetType()
{
    return m_Creature_type;
}

void Creature::SetType(uint32 t)
{
    m_Creature_type = t;
}

void Creature::BuildPetSpellList(WorldPacket& data)
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
    if (summoner->isCreature())
        CALL_SCRIPT_EVENT(summoner, onSummonedCreature)(this);

    if (GetScript() != nullptr)
        GetScript()->OnSummon(static_cast<Unit*>(summoner));
}


