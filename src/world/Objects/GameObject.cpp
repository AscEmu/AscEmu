/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "Management/GameEvent.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include <G3D/Quat.h>
#include "Macros/ScriptMacros.hpp"
#include "Map/Cells/MapCell.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Management/Faction.h"
#include "Spell/Definitions/ProcFlags.hpp"
#include "Spell/Definitions/SpellEffectTarget.hpp"
#include "Spell/SpellMgr.hpp"
#include "Data/WoWGameObject.hpp"
#include "Management/Battleground/Battleground.h"
#include "Server/Packets/SmsgGameobjectCustomAnim.h"
#include "Server/Packets/SmsgGameobjectPagetext.h"
#include "Server/Packets/SmsgTriggerCinematic.h"
#include "Server/Packets/SmsgFishEscaped.h"
#include "Server/Packets/SmsgFishNotHooked.h"
#include "Server/Packets/SmsgEnableBarberShop.h"
#include "Server/Packets/SmsgDestructibleBuildingDamage.h"
#include "Server/Script/ScriptMgr.h"
#include "Map/Maps/MapScriptInterface.h"
#include "GameObjectModel.h"
#include "Server/Definitions.h"

// MIT

using namespace AscEmu::Packets;

bool QuaternionData::isUnit() const
{
    return fabs(x * x + y * y + z * z + w * w - 1.0f) < 1e-5f;
}

void QuaternionData::toEulerAnglesZYX(float& Z, float& Y, float& X) const
{
    G3D::Matrix3(G3D::Quat(x, y, z, w)).toEulerAnglesZYX(Z, Y, X);
}

QuaternionData QuaternionData::fromEulerAnglesZYX(float Z, float Y, float X)
{
    G3D::Quat quat(G3D::Matrix3::fromEulerAnglesZYX(Z, Y, X));
    return QuaternionData(quat.x, quat.y, quat.z, quat.w);
}

GameObject::GameObject(uint64_t guid)
{
    //////////////////////////////////////////////////////////////////////////
    m_objectType |= TYPE_GAMEOBJECT;
    m_objectTypeId = TYPEID_GAMEOBJECT;
    m_valuesCount = getSizeOfStructure(WoWGameObject);
    //////////////////////////////////////////////////////////////////////////

#if VERSION_STRING == Classic
    m_updateFlag = (UPDATEFLAG_ALL | UPDATEFLAG_HAS_POSITION);
#endif
#if VERSION_STRING == TBC
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION);
#endif
#if VERSION_STRING == WotLK
    m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_POSITION | UPDATEFLAG_ROTATION);
#endif
#if VERSION_STRING == Cata
    m_updateFlag = (UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);
#endif
#if VERSION_STRING == Mop
    m_updateFlag = (UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);
#endif

    //\todo Why is there a pointer to the same thing in a derived class? ToDo: sort this out..
    m_uint32Values = _fields;

    std::fill(m_uint32Values, &m_uint32Values[getSizeOfStructure(WoWGameObject)], 0);
    m_updateMask.SetCount(getSizeOfStructure(WoWGameObject));

    setOType(TYPE_GAMEOBJECT | TYPE_OBJECT);
    setGuid(guid);

    setAnimationProgress(100);
    setScale(1);
}

GameObject::~GameObject()
{
    sEventMgr.RemoveEvents(this);

    delete m_model;

    if (myScript)
    {
        myScript->Destroy();
        myScript = nullptr;
    }

    if (m_respawnCell)
        m_respawnCell->_respawnObjects.erase(this);
}

//////////////////////////////////////////////////////////////////////////////////////////
// WoWData

uint64_t GameObject::getCreatedByGuid() const { return gameObjectData()->object_field_created_by.guid; }
void GameObject::setCreatedByGuid(uint64_t guid) { write(gameObjectData()->object_field_created_by.guid, guid); }

uint32_t GameObject::getDisplayId() const { return gameObjectData()->display_id; }
void GameObject::setDisplayId(uint32_t id)
{
    write(gameObjectData()->display_id, id);
    updateModel();
}

uint32_t GameObject::getFlags() const { return gameObjectData()->flags; }
void GameObject::setFlags(uint32_t flags) { write(gameObjectData()->flags, flags); }
void GameObject::addFlags(uint32_t flags) { setFlags(getFlags() | flags); }
void GameObject::removeFlags(uint32_t flags) { setFlags(getFlags() & ~flags); }
bool GameObject::hasFlags(uint32_t flags) const { return (getFlags() & flags) != 0; }

float GameObject::getParentRotation(uint8_t type) const { return gameObjectData()->rotation[type]; }
void GameObject::setParentRotation(QuaternionData const& rotation)
{
    write(gameObjectData()->rotation[0], rotation.x);
    write(gameObjectData()->rotation[1], rotation.y);
    write(gameObjectData()->rotation[2], rotation.z);
    write(gameObjectData()->rotation[3], rotation.w);
}

#if VERSION_STRING < WotLK
uint32_t GameObject::getDynamicFlags() const { return gameObjectData()->dynamic; }
void GameObject::setDynamicFlags(uint32_t dynamicFlags) { write(gameObjectData()->dynamic, dynamicFlags); }
#elif VERSION_STRING < Mop
uint32_t GameObject::getDynamicField() const { return gameObjectData()->dynamic; }
uint16_t GameObject::getDynamicFlags() const { return gameObjectData()->dynamic_field_parts.dyn_flag; }
int16_t GameObject::getDynamicPathProgress() const { return gameObjectData()->dynamic_field_parts.path_progress; }
void GameObject::setDynamicField(uint32_t dynamic) { write(gameObjectData()->dynamic, dynamic); }
void GameObject::setDynamicField(uint16_t dynamicFlags, int16_t pathProgress) { setDynamicField(static_cast<uint32_t>(pathProgress) << 16 | dynamicFlags); }
void GameObject::setDynamicFlags(uint16_t dynamicFlags) { setDynamicField(dynamicFlags, getDynamicPathProgress()); }
void GameObject::setDynamicPathProgress(int16_t pathProgress) { setDynamicField(getDynamicFlags(), pathProgress); }
#endif

uint32_t GameObject::getFactionTemplate() const { return gameObjectData()->faction_template; }
void GameObject::setFactionTemplate(uint32_t id) { write(gameObjectData()->faction_template, id); }

uint32_t GameObject::getLevel() const { return gameObjectData()->level; }
void GameObject::setLevel(uint32_t level) { write(gameObjectData()->level, level); }

//bytes1
uint8_t GameObject::getState() const
{
#if VERSION_STRING <= TBC
    return gameObjectData()->state;
#elif VERSION_STRING >= WotLK
    return gameObjectData()->bytes_1_gameobject.state;
#endif
}
void GameObject::setState(uint8_t state)
{
#if VERSION_STRING <= TBC
    write(gameObjectData()->state, static_cast<uint32_t>(state));
#elif VERSION_STRING >= WotLK
    write(gameObjectData()->bytes_1_gameobject.state, state);
#endif
}

uint8_t GameObject::getGoType() const
{
#if VERSION_STRING <= TBC
    return gameObjectData()->type;
#elif VERSION_STRING >= WotLK
    return gameObjectData()->bytes_1_gameobject.type;
#endif
}
void GameObject::setGoType(uint8_t type)
{
#if VERSION_STRING <= TBC
    write(gameObjectData()->type, static_cast<uint32_t>(type));
#elif VERSION_STRING >= WotLK
    write(gameObjectData()->bytes_1_gameobject.type, type);
#endif
}

#if VERSION_STRING < Mop
uint8_t GameObject::getArtKit() const
{
#if VERSION_STRING <= TBC
    return gameObjectData()->art_kit;
#elif VERSION_STRING >= WotLK
    return gameObjectData()->bytes_1_gameobject.art_kit;
#endif
}
void GameObject::setArtKit(uint8_t artkit)
{
#if VERSION_STRING <= TBC
    write(gameObjectData()->art_kit, static_cast<uint32_t>(artkit));
#elif VERSION_STRING >= WotLK
    write(gameObjectData()->bytes_1_gameobject.art_kit, artkit);
#endif
}
#else
uint8_t GameObject::getArtKit() const
{
    return gameObjectData()->bytes_2_gameobject.art_kit;
}
void GameObject::setArtKit(uint8_t artkit)
{
    write(gameObjectData()->bytes_2_gameobject.art_kit, artkit);
}
#endif

#if VERSION_STRING < Mop
uint8_t GameObject::getAnimationProgress() const
{
#if VERSION_STRING <= TBC
    return gameObjectData()->animation_progress;
#elif VERSION_STRING >= WotLK
    return gameObjectData()->bytes_1_gameobject.animation_progress;
#endif
}
void GameObject::setAnimationProgress(uint8_t progress)
{
#if VERSION_STRING <= TBC
    write(gameObjectData()->animation_progress, static_cast<uint32_t>(progress));
#elif VERSION_STRING >= WotLK
    write(gameObjectData()->bytes_1_gameobject.animation_progress, progress);
#endif
}
#else
uint8_t GameObject::getAnimationProgress() const
{
    return gameObjectData()->bytes_2_gameobject.animation_progress;
}
void GameObject::setAnimationProgress(uint8_t progress)
{
    write(gameObjectData()->bytes_2_gameobject.animation_progress, progress);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Type helper

bool GameObject::isQuestGiver() const { return getGoType() == GAMEOBJECT_TYPE_QUESTGIVER; }
bool GameObject::isFishingNode() const { return getGoType() == GAMEOBJECT_TYPE_FISHINGNODE; }

bool GameObject::loadFromDB(uint32_t spawnId, WorldMap* map, bool addToWorld)
{
    MySQLStructure::GameobjectSpawn* data = sMySQLStore.getGameObjectSpawn(spawnId);

    if (!data)
    {
        sLogger.failure("Gameobject (spawnId: %u) not found in table gameobject_spawns, cant load.");
        return false;
    }

    if (!map || !map->getBaseMap())
    {
        sLogger.failure("Gameobject (spawnId: %u) invalid WorldMap or base data Invalid, cant load.");
        return false;
    }

    uint32_t entry = data->entry;
    GameObject_State gameobjectState = data->state;

    m_spawnId = spawnId;
    if (!create(entry, map, data->phase, data->spawnPoint, data->rotation, data->state))
        return false;

    if (data->spawntimesecs >= 0)
    {
        m_spawnedByDefault = true;

        if (!GetGameObjectProperties()->getDespawnPossibility() && !GetGameObjectProperties()->isDespawnAtAction())
        {
            setFlags(GO_FLAG_NEVER_DESPAWN);
            m_respawnDelayTime = 0;
            m_respawnTime = 0;
        }
        else
        {
            m_respawnDelayTime = data->spawntimesecs;
            m_respawnTime = map->getGORespawnTime(data->id);

            // ready to respawn
            if (m_respawnTime && m_respawnTime <= Util::getTimeNow())
            {
                m_respawnTime = 0;
                map->removeRespawnTime(SPAWN_TYPE_GAMEOBJECT, data->id);
            }
        }
    }
    else
    {
        m_spawnedByDefault = false;
        m_respawnDelayTime = -data->spawntimesecs;
        m_respawnTime = 0;
    }

    m_spawn = data;

    // add to insert Pool
    if (addToWorld)
        map->AddObject(this);

    return true;
}

void GameObject::deleteFromDB()
{
    if (m_spawn != nullptr)
        WorldDatabase.Execute("DELETE FROM gameobject_spawns WHERE id = %u AND min_build <= %u AND max_build >= %u ", m_spawn->id, VERSION_STRING, VERSION_STRING);
}

void GameObject::saveToDB()
{
    if (m_spawn == nullptr)
    {
        // Create spawn instance
        MySQLStructure::GameobjectSpawn* m_spawnTemp = new MySQLStructure::GameobjectSpawn;
        m_spawnTemp->entry = getEntry();
        m_spawnTemp->id = sObjectMgr.GenerateGameObjectSpawnID();
        m_spawnTemp->map = GetMapId();
        m_spawnTemp->spawnPoint = GetPosition();
        m_spawnTemp->phase = GetPhase();
        m_spawnTemp->rotation = m_localRotation;
        m_spawnTemp->spawntimesecs = m_spawnedByDefault ? m_respawnDelayTime : -(int32)m_respawnDelayTime;
        m_spawnTemp->state = GameObject_State(getState());

        uint32_t cx = getWorldMap()->getPosX(GetPositionX());
        uint32_t cy = getWorldMap()->getPosY(GetPositionY());

        getWorldMap()->getBaseMap()->getSpawnsListAndCreate(cx, cy)->GameobjectSpawns.push_back(m_spawnTemp);

        m_spawn = m_spawnTemp;
    }
    std::stringstream ss;

    ss << "DELETE FROM gameobject_spawns WHERE id = ";
    ss << m_spawn->id;
    ss << " AND min_build <= ";
    ss << VERSION_STRING;
    ss << " AND max_build >= ";
    ss << VERSION_STRING;
    ss << ";";

    WorldDatabase.ExecuteNA(ss.str().c_str());

    ss.rdbuf()->str("");

    ss << "INSERT INTO gameobject_spawns VALUES("
        << m_spawn->id << ","
        << getEntry() << ","
        << GetMapId() << ","
        << GetPhase() << ","
        << GetPositionX() << ","
        << GetPositionY() << ","
        << GetPositionZ() << ","
        << GetOrientation() << ","
        << getParentRotation(0) << ","
        << getParentRotation(1) << ","
        << getParentRotation(2) << ","
        << getParentRotation(3) << ","
        << int32_t(m_respawnDelayTime) << ","
        << getState() << ","
        << "0,"            // event
        << VERSION_STRING << ","
        << VERSION_STRING << ")";
    WorldDatabase.Execute(ss.str().c_str());
}

bool GameObject::create(uint32_t entry, WorldMap* map, uint32_t phase, LocationVector const& position, QuaternionData const& rotation, GameObject_State state, uint32_t spawnId)
{
    gameobject_properties = sMySQLStore.getGameObjectProperties(entry);
    if (gameobject_properties == nullptr)
    {
        sLogger.failure("Something tried to create a GameObject with invalid entry %u", entry);
        return false;
    }

    if (gameobject_properties->type == GAMEOBJECT_TYPE_MO_TRANSPORT)
    {
        sLogger.failure("Gameobject (GUID: %u Entry: %u) not created: gameobject type GAMEOBJECT_TYPE_MO_TRANSPORT cannot be manually created.", getGuidLow(), entry);
        return false;
    }

    Object::_Create(map->getBaseMap()->getMapId(), position.x, position.y, position.z, position.o);
    setEntry(entry);
    SetPosition(position);
    setDisplayId(gameobject_properties->display_id);
    m_phase = phase;

    setLocalRotation(rotation.x, rotation.y, rotation.z, rotation.w);
    MySQLStructure::GameObjectSpawnExtra const* gameObjectAddon = sMySQLStore.getGameObjectExtra(getSpawnId());

    // For most of gameobjects is (0, 0, 0, 1) quaternion, there are only some transports with not standard rotation
    QuaternionData parentRotation;
    if (gameObjectAddon)
        parentRotation = gameObjectAddon->parentRotation;

    setParentRotation(parentRotation);

    setScale(gameobject_properties->size);

    // Spawn Overrides
    if (MySQLStructure::GameObjectSpawnOverrides const* goOverride = sMySQLStore.getGameObjectOverride(getSpawnId()))
    {
        SetFaction(goOverride->faction);
        setFlags(goOverride->flags);
    }

    m_model = createModel();

    setGoType(static_cast<uint8>(gameobject_properties->type));
    m_prevGoState = state;
    setState(state);
    setArtKit(0);

    switch (gameobject_properties->type)
    {
        case GAMEOBJECT_TYPE_FISHINGHOLE:
        {
            setAnimationProgress(0);
            m_goValue.FishingHole.MaxOpens = Util::getRandomUInt(gameobject_properties->fishinghole.max_success_opens, gameobject_properties->fishinghole.max_success_opens);
        } break;
        case GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING:
        {
            m_goValue.Building.Health = gameobject_properties->destructible_building.intact_num_hits + gameobject_properties->destructible_building.damaged_num_hits;
            m_goValue.Building.MaxHealth = m_goValue.Building.Health;
            setAnimationProgress(255);
        } break;
        case GAMEOBJECT_TYPE_TRANSPORT:
        {
            m_overrides = GAMEOBJECT_INFVIS | GAMEOBJECT_ONMOVEWIDE; //Make it forever visible on the same map;

            #if VERSION_STRING == Classic
                    m_updateFlag = (m_updateFlag | UPDATEFLAG_TRANSPORT);
            #endif
            #if VERSION_STRING == TBC
                    m_updateFlag = (m_updateFlag | UPDATEFLAG_TRANSPORT);
            #endif
            #if VERSION_STRING == WotLK
                    m_updateFlag = (m_updateFlag | UPDATEFLAG_TRANSPORT) & ~UPDATEFLAG_POSITION;
            #endif
            #if VERSION_STRING == Cata
                    m_updateFlag = (m_updateFlag | UPDATEFLAG_TRANSPORT) & ~UPDATEFLAG_POSITION;
            #endif
            #if VERSION_STRING == Mop
                    m_updateFlag = (m_updateFlag | UPDATEFLAG_TRANSPORT) & ~UPDATEFLAG_POSITION;
            #endif

            setLevel(gameobject_properties->transport.pause);
            setState(gameobject_properties->transport.startOpen ? GO_STATE_OPEN : GO_STATE_CLOSED);
            setAnimationProgress(0);
            m_goValue.Transport.CurrentSeg = 0;
            m_goValue.Transport.AnimationInfo = sTransportHandler.getTransportAnimInfo(entry);
            m_goValue.Transport.PathProgress = 0;
        } break;
        case GAMEOBJECT_TYPE_FISHINGNODE:
        {
            setAnimationProgress(0);
        } break;
        default:
            setAnimationProgress(100);
            break;
    }

    InitAI();

    if (spawnId)
        m_spawnId = spawnId;

    if (uint32_t linkedEntry = gameobject_properties->getLinkedGameObjectEntry())
    {
        GameObject* linkedGO = map->createGameObject(linkedEntry);
        if (linkedGO)
        {
            if (linkedGO->create(linkedEntry, map, phase, position, rotation, GO_STATE_CLOSED))
            {
                setLinkedTrap(linkedGO);
                linkedGO->PushToWorld(map);
            }
            else
            {
                delete linkedGO;
            }
        }
    }

    // Check if GameObject is Large
    if (gameobject_properties->isLargeGameObject())
        m_overrides = GAMEOBJECT_AREAWIDE;  // not implemented yet

    // Check if GameObject is Infinite
    if (gameobject_properties->isInfiniteGameObject())
        m_overrides = GAMEOBJECT_MAPWIDE;

    return true;
}

void GameObject::setRespawnTime(int32_t respawn)
{
    m_respawnTime = respawn > 0 ? Util::getTimeNow() + respawn : 0;
    m_respawnDelayTime = respawn > 0 ? respawn : 0;
}

void GameObject::respawn()
{
    if (m_spawnedByDefault && m_respawnTime > 0)
    {
        m_respawnTime = Util::getTimeNow();

        if (getSpawnId())
        {
            MapCell* pCell = getWorldMap()->getCellByCoords(GetSpawnX(), GetSpawnY());
            if (pCell == nullptr)
                pCell = GetMapCell();

            getWorldMap()->doRespawn(SPAWN_TYPE_GAMEOBJECT, this, getSpawnId(), pCell->getPositionX(), pCell->getPositionY());
        }
    }
}

void GameObject::setLocalRotation(float qx, float qy, float qz, float qw)
{
    G3D::Quat rotation(qx, qy, qz, qw);
    rotation.unitize();
    m_localRotation.x = rotation.x;
    m_localRotation.y = rotation.y;
    m_localRotation.z = rotation.z;
    m_localRotation.w = rotation.w;
    updatePackedRotation();
}

void GameObject::setLocalRotationAngles(float z_rot, float y_rot, float x_rot)
{
    G3D::Quat quat(G3D::Matrix3::fromEulerAnglesZYX(z_rot, y_rot, x_rot));
    setLocalRotation(quat.x, quat.y, quat.z, quat.w);
}

QuaternionData GameObject::getWorldRotation() const
{
    QuaternionData localRotation = getLocalRotation();
    if (Transporter* transport = GetTransport())
    {
        QuaternionData worldRotation = transport->getWorldRotation();

        G3D::Quat worldRotationQuat(worldRotation.x, worldRotation.y, worldRotation.z, worldRotation.w);
        G3D::Quat localRotationQuat(localRotation.x, localRotation.y, localRotation.z, localRotation.w);

        G3D::Quat resultRotation = localRotationQuat * worldRotationQuat;

        return QuaternionData(resultRotation.x, resultRotation.y, resultRotation.z, resultRotation.w);
    }
    return localRotation;
}

void GameObject::updatePackedRotation()
{
    static const int32 PACK_YZ = 1 << 20;
    static const int32 PACK_X = PACK_YZ << 1;

    static const int32 PACK_YZ_MASK = (PACK_YZ << 1) - 1;
    static const int32 PACK_X_MASK = (PACK_X << 1) - 1;

    int8 w_sign = (m_localRotation.w >= 0.f ? 1 : -1);
    int64 x = int32(m_localRotation.x * PACK_X) * w_sign & PACK_X_MASK;
    int64 y = int32(m_localRotation.y * PACK_YZ) * w_sign & PACK_YZ_MASK;
    int64 z = int32(m_localRotation.z * PACK_YZ) * w_sign & PACK_YZ_MASK;
    m_packedRotation = z | (y << 21) | (x << 42);
}

void GameObject::setLootState(LootState state, Unit* unit)
{
    m_lootState = state;
    if (unit)
        m_lootStateUnitGUID = unit->getGuid();
    else
        m_lootStateUnitGUID = 0;

    // Start restock timer if the chest is partially looted or not looted at all
    if (getGoType() == GAMEOBJECT_TYPE_CHEST && state == GO_ACTIVATED && GetGameObjectProperties()->chest.restock_time > 0 && m_restockTime == 0)
        m_restockTime = Util::getTimeNow() + GetGameObjectProperties()->chest.restock_time;

    if (getGoType() == GAMEOBJECT_TYPE_DOOR) // only set collision for doors on SetGoState
        return;

    if (m_model)
    {
        bool collision = false;
        // Use the current go state
        if ((getGoType() != GO_STATE_CLOSED && (state == GO_ACTIVATED || state == GO_JUST_DEACTIVATED)) || state == GO_READY)
            collision = !collision;

        enableCollision(collision);
    }
}

void GameObject::enableCollision(bool enable)
{
    if (!m_model)
        return;

    m_model->enable(enable ? GetPhase() : 0);
}

uint32_t GameObject::getTransportPeriod() const
{
    if (getGoType() != GAMEOBJECT_TYPE_TRANSPORT)
        return 0;

    if (getGOValue()->Transport.AnimationInfo)
        return getGOValue()->Transport.AnimationInfo->TotalTime;

    return 0;
}

class GameObjectModelOwnerImpl : public GameObjectModelOwnerBase
{
public:
    explicit GameObjectModelOwnerImpl(GameObject const* owner) : _owner(owner) { }

    bool IsSpawned() const override { return _owner->isSpawned(); }
    uint32 GetDisplayId() const override { return _owner->getDisplayId(); }
    uint32 GetPhaseMask() const override { return _owner->GetPhase(); }
    G3D::Vector3 GetPosition() const override { return G3D::Vector3(_owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ()); }
    float GetOrientation() const override { return _owner->GetOrientation(); }
    float GetScale() const override { return _owner->getScale(); }
    void DebugVisualizeCorner(G3D::Vector3 const& corner) const override { const_cast<GameObject*>(_owner)->getWorldMap()->createAndSpawnCreature(1, LocationVector(corner.x, corner.y, corner.z, 0)); }

private:
    GameObject const* _owner;
};

GameObjectModel* GameObject::createModel()
{
    return GameObjectModel::Create(std::make_unique<GameObjectModelOwnerImpl>(this), worldConfig.server.dataDir);
}

void GameObject::updateModelPosition()
{
    if (!m_model)
        return;

    if (getWorldMap()->containsGameObjectModel(*m_model))
    {
        getWorldMap()->removeGameObjectModel(*m_model);
        m_model->UpdatePosition();
        getWorldMap()->insertGameObjectModel(*m_model);
    }
}

void GameObject::updateModel()
{
    if (!IsInWorld())
        return;
    if (m_model)
        if (getWorldMap()->containsGameObjectModel(*m_model))
            getWorldMap()->removeGameObjectModel(*m_model);
    delete m_model;
    m_model = createModel();
    if (m_model)
        getWorldMap()->insertGameObjectModel(*m_model);
}

void GameObject::Update(unsigned long time_passed)
{
    if (m_event_Instanceid != m_instanceId)
    {
        event_Relocate();
        return;
    }

    if (!IsInWorld() || m_deleted)
        return;

    // Respawn Handling
    if (m_despawnDelay)
    {
        if (m_despawnDelay > time_passed)
            m_despawnDelay -= time_passed;
        else
        {
            m_despawnDelay = 0;
            despawn(0, m_despawnRespawnTime);
        }
    }

    switch (m_lootState)
    {
        case GO_NOT_READY:
        {
            switch (getGoType())
            {
                case GAMEOBJECT_TYPE_TRAP:
                {
                    GameObjectProperties const* goInfo = GetGameObjectProperties();
                    // Bombs
                    if (goInfo->trap.charges == 2)
                        // Hardcoded tooltip value
                        m_cooldownTime = Util::getMSTime() + 10 * IN_MILLISECONDS;
                    else if (Unit* owner = getOwner())
                        if (owner->isInCombat())
                            m_cooldownTime = Util::getMSTime() + goInfo->trap.start_delay * IN_MILLISECONDS;

                    setLootState(GO_READY);
                } break;
                case GAMEOBJECT_TYPE_TRANSPORT:
                {
                    if (!m_goValue.Transport.AnimationInfo)
                        break;

                    if (getState() == GO_STATE_CLOSED)
                    {
                        m_goValue.Transport.PathProgress += time_passed;
                    }
                } break;
                case GAMEOBJECT_TYPE_FISHINGNODE:
                {
                    // fishing code (bobber ready)
                    if (Util::getTimeNow() > m_respawnTime - 5)
                    {
                        // splash bobber (bobber ready now)
                        Unit* caster = getOwner();
                        if (caster && caster->isPlayer())
                        {
                            setState(GO_STATE_OPEN);
                            setFlags(GO_FLAG_NEVER_DESPAWN);

                            ByteBuffer data(2500);
                            uint32_t count = 0;
                            count = BuildValuesUpdateBlockForPlayer(&data, caster->ToPlayer());
                            caster->ToPlayer()->getUpdateMgr().pushUpdateData(&data, count);

                            sendGameobjectCustomAnim(0);
                        }

                        m_lootState = GO_READY;                 // can be successfully open with some chance
                    }
                } return;
                case GAMEOBJECT_TYPE_CHEST:
                {
                    if (m_restockTime > Util::getTimeNow())
                        return;
                    // If there is no restock timer, or if the restock timer passed, the chest becomes ready to loot
                    m_restockTime = 0;
                    m_lootState = GO_READY;
                } break;
                default:
                {
                    m_lootState = GO_READY; // for other GOis same switched without delay to GO_READY
                } break;
            }
        } [[fallthrough]];
        case GO_READY:
        {
            if (!m_loadedFromDB)
            {
                if (m_respawnTime > 0) // timer on
                {
                    time_t now = Util::getTimeNow();
                    if (m_respawnTime <= now) // timer expired
                    {
                        m_respawnTime = 0;
                        m_usetimes = 0;

                        switch (getGoType())
                        {
                            case GAMEOBJECT_TYPE_FISHINGNODE: //  can't fish now
                            {
                                Unit* caster = getOwner();
                                if (caster && caster->isPlayer())
                                {
                                    caster->removeGameObject(this, false);
                                    caster->sendPacket(SmsgFishEscaped().serialise().get());

                                    // Fishing is channeled spell
                                    auto channelledSpell = caster->getCurrentSpell(CURRENT_CHANNELED_SPELL);
                                    if (channelledSpell != nullptr)
                                    {
                                        channelledSpell->sendChannelUpdate(0);
                                        channelledSpell->finish(true);
                                    }
                                }

                                // can be delete
                                m_lootState = GO_JUST_DEACTIVATED;
                                return;
                            }
                            case GAMEOBJECT_TYPE_DOOR:
                            case GAMEOBJECT_TYPE_BUTTON:
                            {
                                // We need to open doors if they are closed (add there another condition if this code breaks some usage, but it need to be here for battlegrounds)
                                if (getState() != GO_STATE_CLOSED)
                                    resetDoorOrButton();
                            } break;
                            case GAMEOBJECT_TYPE_FISHINGHOLE:
                            {
                                // Initialize a new max fish count on respawn
                                m_goValue.FishingHole.MaxOpens = Util::getRandomUInt(GetGameObjectProperties()->fishinghole.min_success_opens, GetGameObjectProperties()->fishinghole.max_success_opens);
                            } break;
                            default:
                                break;
                        }

                        // Despawn timer
                        if (!m_spawnedByDefault)
                        {
                            // Can be despawned or destroyed
                            setLootState(GO_JUST_DEACTIVATED);
                            return;
                        }

                        // Respawn timer
                        getWorldMap()->AddObject(this);
                    }
                }
            }

            // Set respawn timer
            if (m_loadedFromDB && m_respawnTime > 0)
                saveRespawnTime();

            if (isSpawned())
            {
                GameObjectProperties const* goInfo = GetGameObjectProperties();
                if (goInfo->type == GAMEOBJECT_TYPE_TRAP)
                {
                    if (Util::getMSTime() < m_cooldownTime)
                        break;

                    // Type 2 (bomb) does not need to be triggered by a unit and despawns after casting its spell.
                    if (goInfo->trap.charges == 2)
                    {
                        setLootState(GO_ACTIVATED);
                        break;
                    }

                    // Type 0 despawns after being triggered, type 1 does not.
                    /// @todo This is activation radius. Casting radius must be selected from spell data.
                    float radius;
                    if (!goInfo->trap.radius)
                    {
                        // Battleground traps: data2 == 0 && data5 == 3
                        if (goInfo->trap.cooldown != 3)
                            break;

                        radius = 3.f;
                    }
                    else
                        radius = goInfo->trap.radius / 2.f;

                    // Pointer to appropriate target if found any
                    Unit* target = nullptr;

                    /// @todo this hack with search required until GO casting not implemented
                    if (getOwner())
                    {
                        // Hunter trap: Search units which are unfriendly to the trap's owner
                        for (auto itr : getInRangeObjectsSet())
                        {
                            if (radius > itr->getDistance(this->GetPosition()))
                            {
                                if (itr->isCreatureOrPlayer() && isHostile(this, itr))
                                    target = itr->ToUnit();
                            }
                        }
                    }
                    else
                    {
                        // Environmental trap: Any player
                        Player* player = nullptr;
                        for (auto itr : getInRangePlayersSet())
                        {
                            if (radius > itr->getDistance(this->GetPosition()))
                                target = player;
                        }
                    }

                    if (target)
                        setLootState(GO_ACTIVATED, target);

                }
                else if (uint32_t max_charges = goInfo->getCharges())
                {
                    if (m_usetimes >= max_charges)
                    {
                        m_usetimes = 0;
                        setLootState(GO_JUST_DEACTIVATED);      // can be despawned or destroyed
                    }
                }
            }
        } break;
        case GO_ACTIVATED:
        {
            switch (getGoType())
            {
                case GAMEOBJECT_TYPE_DOOR:
                case GAMEOBJECT_TYPE_BUTTON:
                {
                    if (m_cooldownTime && Util::getMSTime() >= m_cooldownTime)
                        resetDoorOrButton();
                } break;
                case GAMEOBJECT_TYPE_GOOBER:
                {
                    if (Util::getMSTime() >= m_cooldownTime)
                    {
                        removeFlags(GO_FLAG_NONSELECTABLE);
                        setLootState(GO_JUST_DEACTIVATED);
                    }
                } break;
                case GAMEOBJECT_TYPE_CHEST:
                {
                    // Non-consumable chest was partially looted and restock time passed, restock all loot now
                    if (GetGameObjectProperties()->chest.consumable == 0 && Util::getTimeNow() >= m_restockTime)
                    {
                        m_restockTime = 0;
                        m_lootState = GO_READY;
                    }
                } break;
                case GAMEOBJECT_TYPE_TRAP:
                {
                    GameObjectProperties const* goInfo = GetGameObjectProperties();
                    if (goInfo->trap.charges == 2 && goInfo->trap.spell_id)
                    {
                        /// @todo nullptr target won't work for target type 1
                        CastSpell(0, goInfo->trap.spell_id);
                        setLootState(GO_JUST_DEACTIVATED);
                    }
                    else if (Unit* target = getWorldMapUnit(m_lootStateUnitGUID))
                    {
                        // Some traps do not have a spell but should be triggered
                        if (goInfo->trap.spell_id)
                            CastSpell(target->getGuid(), goInfo->trap.spell_id);

                        // Template value or 4 seconds
                        m_cooldownTime = Util::getMSTime() + (goInfo->trap.cooldown ? goInfo->trap.cooldown : uint32(4)) * IN_MILLISECONDS;

                        if (goInfo->trap.charges == 1)
                            setLootState(GO_JUST_DEACTIVATED);
                        else if (!goInfo->trap.charges)
                            setLootState(GO_READY);
                    }
                } break;
                default:
                    break;
            }
        } break;
        case GO_JUST_DEACTIVATED:
        {
            // If nearby linked trap exists, despawn it
            if (GameObject* linkedTrap = getLinkedTrap())
                linkedTrap->despawn(0, 0);

            //if Gameobject should cast spell, then this, but some GOs (type = 10) should be destroyed
            if (getGoType() == GAMEOBJECT_TYPE_GOOBER)
            {
                uint32_t spellId = GetGameObjectProperties()->goober.spell_id;

                if (spellId)
                {
                    for (auto it = m_unique_users.begin(); it != m_unique_users.end(); ++it)
                        // m_unique_users can contain only player GUIDs
                        if (Player* owner = getWorldMapPlayer(*it))
                            owner->castSpell(owner, spellId, false);

                    m_unique_users.clear();
                    m_usetimes = 0;
                }

                // Only goobers with a lock id or a reset time may reset their go state
                if (GetGameObjectProperties()->getLockId() || GetGameObjectProperties()->getAutoCloseTime())
                    setState(GO_STATE_CLOSED);
            }

            switch (getGoType())
            {
                case GAMEOBJECT_TYPE_CHEST:
                case GAMEOBJECT_TYPE_FISHINGNODE:
                case GAMEOBJECT_TYPE_FISHINGHOLE:
                    static_cast<GameObject_Lootable*>(this)->loot.clear();
                    break;
            }

            // Do not delete chests or goobers that are not consumed on loot, while still allowing them to despawn when they expire if summoned
            bool isSummonedAndExpired = (getOwner() || getSpellId()) && m_respawnTime == 0;
            if ((getGoType() == GAMEOBJECT_TYPE_CHEST || getGoType() == GAMEOBJECT_TYPE_GOOBER) && !GetGameObjectProperties()->isDespawnAtAction() && !isSummonedAndExpired)
            {
                if (getGoType() == GAMEOBJECT_TYPE_CHEST && GetGameObjectProperties()->chest.restock_time > 0)
                {
                    // Start restock timer when the chest is fully looted
                    m_restockTime = Util::getTimeNow() + GetGameObjectProperties()->chest.restock_time;
                    setLootState(GO_NOT_READY);
                }
                else
                {
                    setLootState(GO_READY);
                }

                return;
            }
            else if (getOwnerGUID() || getSpellId())
            {
                setRespawnTime(0);
                Delete();
                return;
            }

            setLootState(GO_NOT_READY);

            //burning flags in some battlegrounds, if you find better condition, just add it
            if (GetGameObjectProperties()->isDespawnAtAction() || getAnimationProgress() > 0)
            {
                sendGameobjectDespawnAnim();
                //reset flags
                if (MySQLStructure::GameObjectSpawnOverrides const* goOverride = sMySQLStore.getGameObjectOverride(m_spawnId))
                    setFlags(goOverride->flags);
            }

            if (!m_respawnDelayTime)
                return;

            if (!m_spawnedByDefault)
            {
                m_respawnTime = 0;

                if (m_spawnId)
                    despawn(0, 0);
                else
                    Delete();

                return;
            }

            uint32_t respawnDelay = m_respawnDelayTime;
            m_respawnTime = Util::getTimeNow() + respawnDelay;

            // if option not set then object will be saved at grid unload
            // Otherwise just save respawn time to map object memory
            saveRespawnTime();
            despawn(0, 0);
        } break;
    }

    _UpdateSpells(time_passed);
}

void GameObject::despawn(uint32_t delay, uint32_t forceRespawntime)
{
    if (delay > 0)
    {
        if (!m_despawnDelay || m_despawnDelay > delay)
        {
            m_despawnDelay = delay;
            m_despawnRespawnTime = forceRespawntime;
        }
    }
    else
    {
        if (!IsInWorld())
            return;

        CALL_GO_SCRIPT_EVENT(this, OnDespawn)();

        if (m_spawn && m_loadedFromDB)
        {
            uint32_t const respawnDelay = (forceRespawntime > 0) ? forceRespawntime : m_spawn->spawntimesecs;
            saveRespawnTime(respawnDelay);
        }
        else if (!m_loadedFromDB) // Respawning for non Database Loaded Objects
        {
            /* Get our originating mapcell */
            if (MapCell* pCell = GetMapCell())
            {
                pCell->_respawnObjects.insert(this);
                sEventMgr.RemoveEvents(this);

                m_respawnCell = pCell;
                uint32_t const respawnDelay = (forceRespawntime > 0) ? forceRespawntime : 0;
                saveRespawnTime(respawnDelay);
                RemoveFromWorld(false);
            }
            else
            {
                sLogger.failure("GameObject::Despawn tries to respawn go %u without a valid MapCell, return!", this->getEntry());
            }
            return;
        }

        RemoveFromWorld(true);
        expireAndDelete();
    }
}

void GameObject::saveRespawnTime(uint32_t forceDelay)
{
    if (m_spawn && (forceDelay || m_respawnTime > Util::getTimeNow()) && m_spawnedByDefault)
    {
        // for Gameobjects not Loaded from Database
        if (!m_loadedFromDB)
        {
            RespawnInfo ri;
            ri.type = SPAWN_TYPE_GAMEOBJECT;
            ri.spawnId = getSpawnId();
            ri.entry = getEntry();
            ri.time = m_respawnTime;
            ri.obj = this;
            ri.cellX = m_spawnLocation.x;
            ri.cellY = m_spawnLocation.y;

            bool success = getWorldMap()->addRespawn(ri);
            if (success)
                getWorldMap()->saveRespawnDB(ri);
            return;
        }

        time_t thisRespawnTime = forceDelay ? Util::getTimeNow() + forceDelay / IN_MILLISECONDS : m_respawnTime;
        getWorldMap()->saveRespawnTime(SPAWN_TYPE_GAMEOBJECT, m_spawnId, getEntry(), thisRespawnTime, m_respawnCell->getPositionX(), m_respawnCell->getPositionY());
    }
}

void GameObject::resetDoorOrButton()
{
    if (m_lootState == GO_READY || m_lootState == GO_JUST_DEACTIVATED)
        return;

    removeFlags(GO_FLAG_NONSELECTABLE);
    setState(m_prevGoState);

    setLootState(GO_JUST_DEACTIVATED);
    m_cooldownTime = 0;
}

bool GameObject::isUseable()
{
    // If cooldown data present in template
    if (uint32_t cooldown = GetGameObjectProperties()->getCooldown())
    {
        if (Util::getMSTime() < m_cooldownTime)
            return false;

        m_cooldownTime = Util::getMSTime() + cooldown * IN_MILLISECONDS;
    }

    return true;
}

void GameObject::useDoorOrButton(uint32_t time_to_restore, bool alternative /* = false */, Unit* user /*=nullptr*/)
{
    if (m_lootState != GO_READY)
        return;

    if (!time_to_restore)
        time_to_restore = GetGameObjectProperties()->getAutoCloseTime();

    switchDoorOrButton(true, alternative);
    setLootState(GO_ACTIVATED, user);

    m_cooldownTime = time_to_restore ? (Util::getMSTime() + time_to_restore) : 0;
}

void GameObject::switchDoorOrButton(bool activate, bool alternative /* = false */)
{
    if (activate)
        setFlags(GO_FLAG_NONSELECTABLE);
    else
        removeFlags(GO_FLAG_NONSELECTABLE);

    if (getState() == GO_STATE_CLOSED)
        setState(alternative ? GO_STATE_ALTERNATIVE_OPEN : GO_STATE_OPEN);
    else
        setState(GO_STATE_CLOSED);
}

void GameObject::triggerLinkedGameObject(uint32_t trapEntry, Unit* target)
{
    GameObjectProperties const* trapInfo = sMySQLStore.getGameObjectProperties(trapEntry);
    if (!trapInfo || trapInfo->type != GAMEOBJECT_TYPE_TRAP)
        return;

    SpellInfo const* trapSpell = sSpellMgr.getSpellInfo(trapInfo->trap.spell_id);
    if (!trapSpell)                                          // checked at load already
        return;

    if (GameObject* trapGO = getLinkedTrap())
        trapGO->CastSpell(target->getGuid(), trapSpell->getId());
}

GameObject* GameObject::getLinkedTrap()
{
    return getWorldMapGameObject(m_linkedTrap);
}

void GameObject::addUniqueUse(Player* player)
{
    addUse();
    m_unique_users.insert(player->getGuid());
}

//////////////////////////////////////////////////////////////////////////////////////////
// Summoned Go's
//////////////////////////////////////////////////////////////////////////////////////////

void GameObject::expireAndDelete()
{
    if (m_deleted)
        return;

    m_deleted = true;

    setLootState(GO_NOT_READY);
    sendGameobjectDespawnAnim();

    setState(GO_STATE_CLOSED);

    // remove any events
    sEventMgr.RemoveEvents(this);
    if (IsInWorld())
    {
        removeFromWorld(true);
        delete this;
    }
    else
    {
        delete this;
    }
}

void GameObject::removeFromWorld(bool free_guid)
{
    if (IsInWorld())
    {
        if (m_model)
            if (getWorldMap()->containsGameObjectModel(*m_model))
                getWorldMap()->removeGameObjectModel(*m_model);

        sEventMgr.RemoveEvents(this);
        Object::RemoveFromWorld(free_guid);
    }
}

// MIT End

GameObjectProperties const* GameObject::GetGameObjectProperties() const
{
    return gameobject_properties;
}

void GameObject::SaveToFile(std::stringstream & name)
{

    std::stringstream ss;

    ss << "INSERT INTO gameobject_spawns VALUES("
        << ((m_spawn == NULL) ? 0 : m_spawn->id) << ","
        << VERSION_STRING << ","
        << VERSION_STRING << ","
        << getEntry() << ","
        << GetMapId() << ","
        << GetPositionX() << ","
        << GetPositionY() << ","
        << GetPositionZ() << ","
        << GetOrientation() << ","
        << getParentRotation(0) << ","
        << getParentRotation(1) << ","
        << getParentRotation(2) << ","
        << getParentRotation(3) << ","
        << uint32(getState()) << ","
        << getFlags() << ","
        << getFactionTemplate() << ","
        << getScale() << ","
        << "0,"             // respawnNpcLink
        << m_phase << ","
        << m_overrides << ","
        << "0)";            // event

    FILE* OutFile;

    OutFile = fopen(name.str().c_str(), "wb");
    if (!OutFile) return;
    fwrite(ss.str().c_str(), 1, ss.str().size(), OutFile);
    fclose(OutFile);

}

void GameObject::InitAI()
{
    if (myScript == NULL)
        myScript = sScriptMgr.CreateAIScriptClassForGameObject(getEntry(), this);
}

void GameObject::CallScriptUpdate()
{
    if (myScript)
        myScript->AIUpdate();
    else
        sLogger.failure("GameObject::CallScriptUpdate tries to call, but go %u has no valid script (nullptr)", this->getEntry());
}

void GameObject::OnPushToWorld()
{
    if (m_model)
    {
        if (Transporter* trans = ToTransport())
            trans->setDelayedAddModelToMap();
        else
            getWorldMap()->insertGameObjectModel(*m_model);
    }

    Object::OnPushToWorld();
    if (mEvent != nullptr)
    {
        if (mEvent->mEventScript != nullptr)
        {
            mEvent->mEventScript->OnGameObjectPushToWorld(mEvent, this);
        }
    }

    CALL_GO_SCRIPT_EVENT(this, OnCreate)();
    CALL_GO_SCRIPT_EVENT(this, OnSpawn)();
    CALL_INSTANCE_SCRIPT_EVENT(m_WorldMap, OnGameObjectPushToWorld)(this);

    if (gameobject_properties->type == GAMEOBJECT_TYPE_CHEST)
    {
        //close if open (happenes after respawn)
        if (this->getState() == GO_STATE_OPEN)
            this->setState(GO_STATE_CLOSED);
    }
}

void GameObject::onRemoveInRangeObject(Object* pObj)
{
    Object::onRemoveInRangeObject(pObj);
    if (m_summonedGo && getOwner() == pObj)
    {
        for (uint8_t i = 0; i < 4; i++)
            if (getOwner()->m_ObjectSlots[i] == getGuidLow())
                getOwner()->m_ObjectSlots[i] = 0;

        expireAndDelete();
    }
}

uint32_t GameObject::GetGOReqSkill()
{
    // Here we check the SpellFocus table against the dbcs
    auto lock = sLockStore.LookupEntry(GetGameObjectProperties()->raw.parameter_0);
    if (!lock)
        return 0;

    for (uint8_t i = 0; i < LOCK_NUM_CASES; i++)
    {
        if (lock->locktype[i] == 2 && lock->minlockskill[i])
            return lock->minlockskill[i];
    }
    return 0;
}

void GameObject::CastSpell(uint64_t TargetGUID, SpellInfo const* sp)
{
    Spell* s = sSpellMgr.newSpell(this, sp, true, nullptr);

    SpellCastTargets tgt(TargetGUID);

    // TODO: Is this meant to be set source?
    tgt.setDestination(GetPosition());

    s->prepare(&tgt);
}

void GameObject::CastSpell(uint64_t TargetGUID, uint32_t SpellID)
{
    SpellInfo const* sp = sSpellMgr.getSpellInfo(SpellID);
    if (sp == nullptr)
    {
        sLogger.failure("GameObject %u tried to cast a non-existing Spell %u.", gameobject_properties->entry, SpellID);
        return;
    }

    CastSpell(TargetGUID, sp);
}

//MIT
void GameObject::sendGameobjectCustomAnim(uint32_t anim)
{
    sendMessageToSet(SmsgGameobjectCustomAnim(getGuid(), anim).serialise().get(), false, false);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Door
GameObject_Door::GameObject_Door(uint64 GUID) : GameObject(GUID)
{ }

GameObject_Door::~GameObject_Door()
{ }

void GameObject_Door::InitAI()
{
    GameObject::InitAI();
}

void GameObject_Door::onUse(Player* player)
{
    if (!isUseable())
        return;

    useDoorOrButton(0, false, player);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Button
GameObject_Button::GameObject_Button(uint64 GUID) : GameObject(GUID)
{
    spell = nullptr;
}

GameObject_Button::~GameObject_Button()
{ }

void GameObject_Button::InitAI()
{
    GameObject::InitAI();

    if (gameobject_properties->button.linked_trap_id != 0)
    {
        GameObjectProperties const* gameobject_info = sMySQLStore.getGameObjectProperties(gameobject_properties->button.linked_trap_id);

        if (gameobject_info != nullptr)
        {
            if (gameobject_info->trap.spell_id != 0)
                spell = sSpellMgr.getSpellInfo(gameobject_info->trap.spell_id);
        }
    }
}

void GameObject_Button::onUse(Player* player)
{
    if (!isUseable())
        return;

    useDoorOrButton(0, false, player);

    if (getState() == GO_STATE_CLOSED)
    {
        if (spell != nullptr)
            CastSpell(player->getGuid(), spell);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_QuestGiver
GameObject_QuestGiver::GameObject_QuestGiver(uint64 GUID) : GameObject(GUID)
{
    m_quests = NULL;
}

GameObject_QuestGiver::~GameObject_QuestGiver()
{ }

void GameObject_QuestGiver::InitAI()
{
    LoadQuests();
    GameObject::InitAI();
}

void GameObject_QuestGiver::onUse(Player* player)
{
    if (!isUseable())
        return;

    if (HasQuests())
        sQuestMgr.OnActivateQuestGiver(this, player);
}

void GameObject_QuestGiver::DeleteQuest(QuestRelation* Q)
{
    for (std::list<QuestRelation*>::iterator itr = m_quests->begin(); itr != m_quests->end(); ++itr)
    {
        QuestRelation* qr = *itr;

        if ((qr->type == Q->type) && (qr->qst == Q->qst))
        {
            delete qr;
            m_quests->erase(itr);
            break;
        }
    }
}

QuestProperties const* GameObject_QuestGiver::FindQuest(uint32_t quest_id, uint8_t quest_relation)
{
    for (std::list<QuestRelation*>::iterator itr = m_quests->begin(); itr != m_quests->end(); ++itr)
    {
        QuestRelation* qr = *itr;

        if ((qr->qst->id == quest_id) && ((qr->type & quest_relation) != 0))
        {
            return qr->qst;
        }
    }
    return nullptr;
}

uint16 GameObject_QuestGiver::GetQuestRelation(uint32_t quest_id)
{
    uint16 quest_relation = 0;

    for (std::list<QuestRelation*>::iterator itr = m_quests->begin(); itr != m_quests->end(); ++itr)
    {
        QuestRelation* qr = *itr;

        if ((qr != nullptr) && (qr->qst->id == quest_id))
            quest_relation |= qr->type;
    }

    return quest_relation;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Chest
GameObject_Chest::GameObject_Chest(uint64 GUID) : GameObject_Lootable(GUID)
{
    spell = nullptr;
}

GameObject_Chest::~GameObject_Chest()
{ }

void GameObject_Chest::InitAI()
{
    GameObject::InitAI();

    if (gameobject_properties->chest.linked_trap_id != 0)
    {
        GameObjectProperties const* gameobject_info = sMySQLStore.getGameObjectProperties(gameobject_properties->chest.linked_trap_id);

        if (gameobject_info != nullptr)
        {
            if (gameobject_info->trap.spell_id != 0)
                spell = sSpellMgr.getSpellInfo(gameobject_info->trap.spell_id);
        }
    }
}

bool GameObject_Chest::HasLoot()
{
    if (!loot.isLooted())
        return true;

    return false;
}

void GameObject_Chest::Open()
{
    setState(GO_STATE_OPEN);
}

void GameObject_Chest::Close()
{
    setState(GO_STATE_CLOSED);
}

void GameObject_Chest::onUse(Player* player)
{
    if (!isUseable())
        return;

    if (getState() == GO_STATE_CLOSED)
    {
        Open();

        if (spell != NULL)
            CastSpell(player->getGuid(), spell);

        //open chest spell?
        SpellCastTargets targets(getGuid());
        auto spellInfo = sSpellMgr.getSpellInfo(11437);
        auto spellOpen = sSpellMgr.newSpell(player, spellInfo, true, nullptr);
        spellOpen->prepare(&targets);
    }
    else
    {
        Close();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Trap
GameObject_Trap::GameObject_Trap(uint64 GUID) : GameObject(GUID)
{
    spell = NULL;
}

GameObject_Trap::~GameObject_Trap()
{ }

void GameObject_Trap::InitAI()
{
    ///\brief prevent traps from casting periodic spells. This can be removed until proper event handling.
    // e.g. BootyBay
    switch (gameobject_properties->entry)
    {
        case 171941:
        case 175791:
        case 176214:
        case 179557:
        case 179560:
        case 180391:
            return;
    }

    spell = sSpellMgr.getSpellInfo(gameobject_properties->trap.spell_id);

    if (gameobject_properties->trap.stealthed != 0)
    {
        inStealth = true;
        stealthFlag = STEALTH_FLAG_TRAP;
    }

    if (gameobject_properties->trap.invisible != 0)
    {
        invisible = true;
        invisibilityFlag = INVIS_FLAG_TRAP;
    }

    GameObject::InitAI();
}

void GameObject_Trap::onUse(Player* player)
{
    if (!isUseable())
        return;

    GameObjectProperties const* goInfo = GetGameObjectProperties();
    if (goInfo->trap.spell_id)
        CastSpell(player->getGuid(), goInfo->trap.spell_id);

    m_cooldownTime = Util::getMSTime() + (goInfo->trap.cooldown ? goInfo->trap.cooldown : uint32(4)) * IN_MILLISECONDS;   // template or 4 seconds

    if (goInfo->trap.charges == 1)         // Deactivate after trigger
        setLootState(GO_JUST_DEACTIVATED);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Chair

void GameObject_Chair::onUse(Player* player)
{
    if (!isUseable())
        return;

    // todo: parameter_1 defines the height!
    player->safeTeleport(player->GetMapId(), player->GetInstanceID(), GetPositionNC());
    player->setStandState(STANDSTATE_SIT_MEDIUM_CHAIR);

    player->updateSpeed();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_SpellFocus
GameObject_SpellFocus::GameObject_SpellFocus(uint64 GUID) : GameObject(GUID)
{ }

GameObject_SpellFocus::~GameObject_SpellFocus()
{ }

void GameObject_SpellFocus::OnPushToWorld()
{
    GameObject::OnPushToWorld();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Goober
GameObject_Goober::GameObject_Goober(uint64 GUID) : GameObject(GUID)
{
    spell = NULL;
}

GameObject_Goober::~GameObject_Goober()
{ }

void GameObject_Goober::InitAI()
{
    GameObject::InitAI();

    if (gameobject_properties->goober.linked_trap_id != 0)
    {
        GameObjectProperties const* gameobject_info = sMySQLStore.getGameObjectProperties(gameobject_properties->goober.linked_trap_id);
        if (gameobject_info != nullptr)
        {
            if (gameobject_info->trap.spell_id != 0)
                spell = sSpellMgr.getSpellInfo(gameobject_info->trap.spell_id);
        }
    }
}

void GameObject_Goober::onUse(Player* player)
{
    if (!isUseable())
        return;

    GameObjectProperties const* info = GetGameObjectProperties();

    if (player)
    {
        if (info->goober.page_id)    // show page...
        {
            player->sendPacket(SmsgGameobjectPagetext(getGuid()).serialise().get());
        }
        else if (info->goober.gossip_id)
        {
            // todo
        }

        // possible quest objective for active quests
        if (info->goober.quest_id && sMySQLStore.getQuestProperties(info->goober.quest_id))
        {
            //Quest require to be active for GO using
            if (QuestLogEntry* quest = player->getQuestLogByQuestId(info->goober.quest_id))
            {
                if (quest->getQuestState() != QUEST_INCOMPLETE)
                    return;
            }
        }

        // Quest Completion
        if (Group* group = player->getGroup())
        {
            // todo
        }
        //else

    }

    if (uint32_t trapEntry = info->goober.linked_trap_id)
        triggerLinkedGameObject(trapEntry, player);

    setFlags(GO_FLAG_NONSELECTABLE);
    setLootState(GO_ACTIVATED, player);

    if (info->goober.custom_anim)
        SmsgGameobjectCustomAnim(getGuid(), getAnimationProgress());
    else
        setState(GO_STATE_OPEN);

    m_cooldownTime = Util::getMSTime() + info->getAutoCloseTime();

    if (spell != nullptr)
        CastSpell(player->getGuid(), spell);

    player->castSpell(getGuid(), gameobject_properties->goober.spell_id, false);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Camera
void GameObject_Camera::onUse(Player* player)
{
    if (!isUseable())
        return;

    if (gameobject_properties->camera.cinematic_id != 0)
        player->getSession()->SendPacket(SmsgTriggerCinematic(gameobject_properties->camera.cinematic_id).serialise().get());
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_FishingNode
GameObject_FishingNode::GameObject_FishingNode(uint64 GUID) : GameObject_Lootable(GUID)
{
}

GameObject_FishingNode::~GameObject_FishingNode()
{ }

void GameObject_FishingNode::onUse(Player* player)
{
    if (!isUseable())
        return;

    if (player->getGuid() != getOwnerGUID())
        return;

    switch (getLootState())
    {
        case GO_READY: // ready for loot
        {
            uint32_t zone, subzone;
            m_WorldMap->getZoneAndAreaId(GetPhase(), zone, subzone, GetPosition());

            MySQLStructure::FishingZones const* zone_skill = sMySQLStore.getFishingZone(subzone);
            if (!zone_skill)
                zone_skill = sMySQLStore.getFishingZone(zone);

            //provide error, no fishable zone or area should be 0
            if (!zone_skill)
                sLogger.failure("Fishable areaId %u are not found in `fishing` table.", subzone);

            int32_t skill = player->getSkillLineCurrent(SKILL_FISHING, false);

            int32 chance = 100;
            if (skill < zone_skill->maxSkill)
            {
                chance = int32(pow((double)skill / zone_skill->maxSkill, 2) * 100);
                if (chance < 1)
                    chance = 1;
            }

            int32_t roll = Util::getRandomInt(1, 100);

            // Advance
            player->advanceSkillLine(SKILL_FISHING, static_cast<uint16_t>(float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE))));

            GameObject_FishingHole* school = nullptr;
            GameObject* fishingPool = getWorldMap()->getInterface()->findNearestGoWithType(this, GAMEOBJECT_TYPE_FISHINGHOLE);;
            if (fishingPool != nullptr)
            {
                school = dynamic_cast<GameObject_FishingHole*>(fishingPool);

                if (!isInRange(school, static_cast<float>(school->GetGameObjectProperties()->fishinghole.radius)))
                    school = nullptr;
            }

            // Fishing pools have no skill requirement as of patch 3.3.0
            if (chance >= roll || school)
            {
                if (school)
                {
                    school->onUse(player);
                    setLootState(GO_JUST_DEACTIVATED);
                }
                else
                {
                    player->sendLoot(getGuid(), LOOT_FISHING, player->GetMapId());
                }
            }
            else // If fishing skill is too low, send junk loot.
            {
                player->sendLoot(getGuid(), LOOT_FISHING, player->GetMapId());
            }
        } break;
        case GO_JUST_DEACTIVATED: // nothing to do, will be deleted at next update
            break;
        default:
        {
            setLootState(GO_JUST_DEACTIVATED);
            player->sendPacket(SmsgFishNotHooked().serialise().get());
            break;
        }
    }

    // Fishing is channeled spell
    auto channelledSpell = player->getCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (channelledSpell != nullptr)
    {
        channelledSpell->sendChannelUpdate(0);
        channelledSpell->finish(true);
    }
}

bool GameObject_FishingNode::HasLoot()
{
    if (!loot.isLooted())
        return true;

    return false;
}

void GameObject::getFishLoot(Loot* fishloot, Player* loot_owner)
{
    fishloot->clear();

    uint32_t zone, subzone;
    uint32_t defaultzone = 1;
    m_WorldMap->getZoneAndAreaId(GetPhase(), zone, subzone, GetPosition());

    // if subzone loot exist use it
    fishloot->fillLoot(subzone, sLootMgr.FishingLoot, loot_owner, true);
    if (fishloot->empty())
    {
        //subzone no result,use zone loot
        fishloot->fillLoot(zone, sLootMgr.FishingLoot, loot_owner, true);
        //use zone 1 as default, somewhere fishing got nothing,becase subzone and zone not set, like Off the coast of Storm Peaks.
        if (fishloot->empty())
            fishloot->fillLoot(defaultzone, sLootMgr.FishingLoot, loot_owner, true);
    }
}

void GameObject::getFishLootJunk(Loot* fishloot, Player* loot_owner)
{
    fishloot->clear();

    uint32_t zone, subzone;
    uint32_t defaultzone = 1;
    m_WorldMap->getZoneAndAreaId(GetPhase(), zone, subzone, GetPosition());

    // if subzone loot exist use it
    fishloot->fillLoot(subzone, sLootMgr.FishingLoot, loot_owner, true, LOOT_MODE_JUNK_FISH);
    if (fishloot->empty())  //use this becase if zone or subzone has normal mask drop, then fishloot->FillLoot return true.
    {
        //use zone loot
        fishloot->fillLoot(zone, sLootMgr.FishingLoot, loot_owner, true, LOOT_MODE_JUNK_FISH);
        if (fishloot->empty())
            //use zone 1 as default
            fishloot->fillLoot(defaultzone, sLootMgr.FishingLoot, loot_owner, true, LOOT_MODE_JUNK_FISH);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Ritual
GameObject_Ritual::GameObject_Ritual(uint64 GUID) : GameObject(GUID)
{
}

GameObject_Ritual::~GameObject_Ritual()
{
    delete Ritual;
    Ritual = nullptr;
}

void GameObject_Ritual::InitAI()
{
    Ritual = new RitualStruct(gameobject_properties->summoning_ritual.req_participants);
}

void GameObject_Ritual::onUse(Player* player)
{
    if (!isUseable())
        return;

    SpellCastTargets targets;
    Spell* spell = nullptr;

    // store the members in the ritual, cast sacrifice spell, and summon.
    if (GetRitual()->IsFinished() || GetRitual()->GetCasterGUID() == 0)
        return;

    // If we clicked on the ritual we are already in, remove us, otherwise add us as a ritual member
    if (GetRitual()->HasMember(player->getGuidLow()))
    {
        GetRitual()->RemoveMember(player->getGuidLow());
        player->setChannelSpellId(0);
        player->setChannelObjectGuid(0);
        return;
    }
    else
    {
        GetRitual()->AddMember(player->getGuidLow());
        player->setChannelSpellId(GetRitual()->GetSpellID());
        player->setChannelObjectGuid(getGuid());
    }

    // If we were the last required member, proceed with the ritual!
    if (!GetRitual()->HasFreeSlots())
    {
        GetRitual()->Finish();
        Player* plr = nullptr;

        unsigned long MaxMembers = GetRitual()->GetMaxMembers();
        for (unsigned long i = 0; i < MaxMembers; i++)
        {
            plr = player->getWorldMap()->getPlayer(GetRitual()->GetMemberGUIDBySlot(i));
            if (plr != nullptr)
            {
                plr->setChannelObjectGuid(0);
                plr->setChannelSpellId(0);
            }
        }

        if (gameobject_properties->entry == 36727 || gameobject_properties->entry == 194108)   // summon portal
        {
            if (!GetRitual()->GetTargetGUID() == 0)
                return;

            SpellInfo const* info = sSpellMgr.getSpellInfo(gameobject_properties->summoning_ritual.spell_id);
            if (info == nullptr)
                return;

            Player* target = sObjectMgr.GetPlayer(GetRitual()->GetTargetGUID());
            if (target == nullptr || !target->IsInWorld())
                return;

            spell = sSpellMgr.newSpell(player->getWorldMap()->getPlayer(GetRitual()->GetCasterGUID()), info, true, nullptr);
            targets.setUnitTarget(target->getGuid());
            spell->prepare(&targets);
        }
        else if (gameobject_properties->entry == 177193)    // doom portal
        {
            uint32_t victimid = Util::getRandomUInt(GetRitual()->GetMaxMembers() - 1);

            // kill the sacrifice player
            Player* psacrifice = player->getWorldMap()->getPlayer(GetRitual()->GetMemberGUIDBySlot(victimid));
            Player* pCaster = getWorldMap()->getPlayer(GetRitual()->GetCasterGUID());
            if (!psacrifice || !pCaster)
                return;

            SpellInfo const* info = sSpellMgr.getSpellInfo(gameobject_properties->summoning_ritual.caster_target_spell);
            if (!info)
                return;

            spell = sSpellMgr.newSpell(psacrifice, info, true, nullptr);
            targets.setUnitTarget(psacrifice->getGuid());
            spell->prepare(&targets);

            // summons demon
            info = sSpellMgr.getSpellInfo(gameobject_properties->summoning_ritual.spell_id);
            spell = sSpellMgr.newSpell(pCaster, info, true, nullptr);

            SpellCastTargets targets2(pCaster->getGuid());
            spell->prepare(&targets2);
        }
        else if (gameobject_properties->entry == 179944)    // Summoning portal for meeting stones
        {
            plr = player->getWorldMap()->getPlayer(GetRitual()->GetTargetGUID());
            if (!plr)
                return;

            Player* pleader = player->getWorldMap()->getPlayer(GetRitual()->GetCasterGUID());
            if (!pleader)
                return;

            SpellInfo const* info = sSpellMgr.getSpellInfo(gameobject_properties->summoning_ritual.spell_id);
            spell = sSpellMgr.newSpell(pleader, info, true, nullptr);
            SpellCastTargets targets2(plr->getGuid());
            spell->prepare(&targets2);

            // expire the gameobject
            expireAndDelete();
        }
        else if (gameobject_properties->entry == 186811 || gameobject_properties->entry == 181622)
        {
            SpellInfo const* info = sSpellMgr.getSpellInfo(gameobject_properties->summoning_ritual.spell_id);
            if (info == NULL)
                return;

            spell = sSpellMgr.newSpell(player->getWorldMap()->getPlayer(GetRitual()->GetCasterGUID()), info, true, nullptr);
            SpellCastTargets targets2(GetRitual()->GetCasterGUID());
            spell->prepare(&targets2);
            expireAndDelete();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_SpellCaster
GameObject_SpellCaster::GameObject_SpellCaster(uint64 GUID) : GameObject(GUID)
{
    spell = nullptr;
}

GameObject_SpellCaster::~GameObject_SpellCaster()
{ }

void GameObject_SpellCaster::InitAI()
{
    spell = sSpellMgr.getSpellInfo(gameobject_properties->spell_caster.spell_id);
    if (spell == nullptr)
        sLogger.failure("GameObject %u ( %s ) has a nonexistant spellID in the database.", gameobject_properties->entry, gameobject_properties->name.c_str());
}

void GameObject_SpellCaster::onUse(Player* player)
{
    if (!isUseable())
        return;

    if (GetGameObjectProperties()->spell_caster.party_only != 0)
    {
        if (getOwner() != nullptr && getOwner()->isPlayer())
        {
            Player* summoner = static_cast<Player*>(getOwner());
            if (summoner->getGuid() != player->getGuid())
            {
                if (!player->isInGroup())
                    return;

                if (player->getGroup() != summoner->getGroup())
                    return;
            }
        }
    }

    if (spell == nullptr)
        return;

    CastSpell(player->getGuid(), spell);

    addUse();

    if (getUseCount() == 0)
        expireAndDelete();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Meetingstone
void GameObject_Meetingstone::onUse(Player* player)
{
    if (!isUseable())
        return;

    // Use selection
    Player* pPlayer = sObjectMgr.GetPlayer(static_cast<uint32>(player->getTargetGuid()));
    if (pPlayer == nullptr)
        return;

    // If we are not in a group we can't summon anyone
    if (!player->isInGroup())
        return;

    // We can only summon someone if they are in our raid/group
    if (player->getGroup() != pPlayer->getGroup())
        return;

    // We can't summon ourselves!
    if (pPlayer->getGuid() == player->getGuid())
        return;

    // Create the summoning portal
    GameObject* pGo = player->getWorldMap()->createGameObject(179944);
    if (pGo == nullptr)
        return;

    GameObject_Ritual* rGo = static_cast<GameObject_Ritual*>(pGo);

    QuaternionData rot = QuaternionData::fromEulerAnglesZYX(player->GetOrientation(), 0.f, 0.f);

    rGo->create(179944, player->getWorldMap(), player->GetPositionX(), player->GetPosition(), rot, GO_STATE_CLOSED);
    rGo->GetRitual()->Setup(player->getGuidLow(), pPlayer->getGuidLow(), 18540);
    rGo->PushToWorld(player->getWorldMap());

    player->setChannelObjectGuid(rGo->getGuid());
    player->setChannelSpellId(rGo->GetRitual()->GetSpellID());

    // expire after 2mins
    despawn(120000, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_FlagStand
void GameObject_FlagStand::onUse(Player* player)
{
    if (!isUseable())
        return;

    if (player->getBattleground() != nullptr)
        player->getBattleground()->HookFlagStand(player, this);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_FishingHole
GameObject_FishingHole::GameObject_FishingHole(uint64 GUID) : GameObject_Lootable(GUID)
{

}

GameObject_FishingHole::~GameObject_FishingHole()
{ }

void GameObject_FishingHole::InitAI()
{

}

void GameObject_FishingHole::onUse(Player* player)
{
    if (!isUseable())
        return;

    if (!player)
        return;

    player->sendLoot(getGuid(), LOOT_FISHINGHOLE, player->GetMapId());
    // todo achievemnt
    return;
}

bool GameObject_FishingHole::HasLoot()
{
    if (!loot.isLooted())
        return true;

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_FlagDrop
void GameObject_FlagDrop::onUse(Player* player)
{
    if (!isUseable())
        return;

    if (player->getBattleground())
        player->getBattleground()->HookFlagDrop(player, this);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_BarberChair
void GameObject_BarberChair::onUse(Player* player)
{
    if (!isUseable())
        return;

#if VERSION_STRING > TBC
    //parameter_0 defines the height!
    player->safeTeleport(player->GetMapId(), player->GetInstanceID(), GetPositionNC());
    player->updateSpeed();

    //send barber shop menu to player
    player->sendPacket(SmsgEnableBarberShop().serialise().get());

    player->setStandState(STANDSTATE_SIT_HIGH_CHAIR);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Destructible
GameObject_Destructible::GameObject_Destructible(uint64 GUID) : GameObject(GUID)
{
    hitpoints = 0;
    maxhitpoints = 0;
}

GameObject_Destructible::~GameObject_Destructible()
{ }

void GameObject_Destructible::InitAI()
{
    GameObject::InitAI();
    Rebuild();
}

void GameObject_Destructible::Damage(uint32_t damage, uint64_t AttackerGUID, uint64_t ControllerGUID, uint32_t SpellID)
{
    // If we are already destroyed there's nothing to damage!
    if (hitpoints == 0)
        return;

    if (damage >= hitpoints)
    {
        // Instant destruction
        hitpoints = 0;

        setFlags(GO_FLAG_DESTROYED);
        removeFlags(GO_FLAG_DAMAGED);
        setDisplayId(gameobject_properties->destructible_building.destroyed_display_id);   // destroyed display id

        CALL_GO_SCRIPT_EVENT(this, OnDestroyed)();

    }
    else
    {
        // Simply damaging
        hitpoints -= damage;

        if (!hasFlags(GO_FLAG_DAMAGED))
        {
            // Intact  ->  Damaged

            // Are we below the intact-damaged transition treshold?
            if (hitpoints <= (maxhitpoints - gameobject_properties->destructible_building.intact_num_hits))
            {
                setFlags(GO_FLAG_DAMAGED);
                setDisplayId(gameobject_properties->destructible_building.damaged_display_id); // damaged display id
            }
        }
        else
        {
            if (hitpoints == 0)
            {
                removeFlags(GO_FLAG_DAMAGED);
                setFlags(GO_FLAG_DESTROYED);
                setDisplayId(gameobject_properties->destructible_building.destroyed_display_id);
            }
        }

        CALL_GO_SCRIPT_EVENT(this, OnDamaged)(damage);
    }

    uint8_t animprogress = static_cast<uint8>(std::round(hitpoints / float(maxhitpoints)) * 255);
    setAnimationProgress(animprogress);
    SendDamagePacket(damage, AttackerGUID, ControllerGUID, SpellID);
}

void GameObject_Destructible::SendDamagePacket(uint32_t damage, uint64_t AttackerGUID, uint64_t ControllerGUID, uint32_t SpellID)
{
#if VERSION_STRING > TBC
    sendMessageToSet(SmsgDestructibleBuildingDamage(GetNewGUID(), AttackerGUID, ControllerGUID, damage, SpellID).serialise().get(), false, false);
#endif
}

void GameObject_Destructible::Rebuild()
{
    removeFlags(GO_FLAG_DAMAGED | GO_FLAG_DESTROYED);
    setDisplayId(gameobject_properties->display_id);
    maxhitpoints = gameobject_properties->destructible_building.intact_num_hits + gameobject_properties->destructible_building.damaged_num_hits;
    hitpoints = maxhitpoints;
}
