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

    if (uint32_t guid = static_cast<uint32_t>(getCreatedByGuid()))
    {
        Player* player = sObjectMgr.GetPlayer(guid);
        if (player && player->getSummonedObject() == this)
            player->setSummonedObject(nullptr);

        if (player == m_summoner)
            m_summoner = nullptr;
    }

    if (m_respawnCell)
        m_respawnCell->_respawnObjects.erase(this);

    if (m_summonedGo && m_summoner)
        for (uint8_t i = 0; i < 4; ++i)
            if (m_summoner->m_ObjectSlots[i] == getGuidLow())
                m_summoner->m_ObjectSlots[i] = 0;
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

Player* GameObject::getPlayerOwner()
{
    if (m_summoner != nullptr && m_summoner->isPlayer())
        return dynamic_cast<Player*>(m_summoner);

    return nullptr;
}

bool GameObject::loadFromDB(uint32_t spawnId, WorldMap* map, bool addToWorld)
{
    MySQLStructure::GameobjectSpawn const* data = sMySQLStore.getGameObjectSpawn(spawnId);

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
    if (!create(entry, map->getBaseMap()->getMapId(), data->phase, data->spawnPoint, data->rotation, data->state))
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
            m_respawnTime = getWorldMap()->getGORespawnTime(data->id);

            // ready to respawn
            if (m_respawnTime && m_respawnTime <= Util::getTimeNow())
            {
                m_respawnTime = 0;
                getWorldMap()->removeRespawnTime(SPAWN_TYPE_GAMEOBJECT, data->id);
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
        getWorldMap()->AddObject(this);

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

bool GameObject::create(uint32_t entry, uint32_t mapId, uint32_t phase, LocationVector const& position, QuaternionData const& rotation, GameObject_State state, uint32_t spawnId)
{
    gameobject_properties = sMySQLStore.getGameObjectProperties(entry);
    if (gameobject_properties == nullptr)
    {
        sLogger.failure("Something tried to create a GameObject with invalid entry %u", entry);
        return false;
    }

    Object::_Create(mapId, position.x, position.y, position.z, position.o);
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
        case GAMEOBJECT_TYPE_MO_TRANSPORT:
        {
            m_overrides = GAMEOBJECT_INFVIS | GAMEOBJECT_ONMOVEWIDE; //Make it forever visible on the same map;
            setFlags(GO_FLAG_TRANSPORT | GO_FLAG_NEVER_DESPAWN);
            setState(gameobject_properties->mo_transport.can_be_stopped ? GO_STATE_CLOSED : GO_STATE_OPEN);
            m_goValue.Transport.PathProgress = 0;
        } break;
        case GAMEOBJECT_TYPE_FISHINGNODE:
        {
            setAnimationProgress(0);
        } break;
        default:
            setAnimationProgress(0);
            break;
    }

    InitAI();

    if (spawnId)
        m_spawnId = spawnId;

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

    bool IsSpawned() const override { return true; }
    uint32 GetDisplayId() const override { return _owner->getDisplayId(); }
    uint32 GetPhaseMask() const override { return 1; }       // todo our gameobjects dont support phases?
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

    if (!IsInWorld())
        return;

    if (m_deleted)
        return;

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
                uint32_t const respawnDelay = (forceRespawntime > 0) ? forceRespawntime : m_spawn->spawntimesecs;
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
        RemoveFromWorld(true);
        delete this;
    }
    else
    {
        delete this;
    }
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
    if (m_summonedGo && m_summoner == pObj)
    {
        for (uint8 i = 0; i < 4; i++)
            if (m_summoner->m_ObjectSlots[i] == getGuidLow())
                m_summoner->m_ObjectSlots[i] = 0;

        m_summoner = 0;
        expireAndDelete();
    }
}
// Remove gameobject from world, using their despawn animation.
void GameObject::RemoveFromWorld(bool free_guid)
{
    sendGameobjectDespawnAnim();

    if (m_model)
        if (getWorldMap()->containsGameObjectModel(*m_model))
            getWorldMap()->removeGameObjectModel(*m_model);

    sEventMgr.RemoveEvents(this);
    Object::RemoveFromWorld(free_guid);
}

uint32 GameObject::GetGOReqSkill()
{
    // Here we check the SpellFocus table against the dbcs
    auto lock = sLockStore.LookupEntry(GetGameObjectProperties()->raw.parameter_0);
    if (!lock)
        return 0;

    for (uint8 i = 0; i < LOCK_NUM_CASES; i++)
    {
        if (lock->locktype[i] == 2 && lock->minlockskill[i])
            return lock->minlockskill[i];
    }
    return 0;
}

void GameObject::CastSpell(uint64 TargetGUID, SpellInfo const* sp)
{
    Spell* s = sSpellMgr.newSpell(this, sp, true, nullptr);

    SpellCastTargets tgt(TargetGUID);

    // TODO: Is this meant to be set source?
    tgt.setDestination(GetPosition());

    s->prepare(&tgt);
}

void GameObject::CastSpell(uint64 TargetGUID, uint32 SpellID)
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

    if (gameobject_properties->door.start_open != 0)
        setState(GO_STATE_OPEN);
    else
        setState(GO_STATE_CLOSED);
}

void GameObject_Door::Open()
{
    setState(GO_STATE_OPEN);
    if (gameobject_properties->door.auto_close_time != 0)
        sEventMgr.AddEvent(this, &GameObject_Door::Close, 0, gameobject_properties->door.auto_close_time, 1, 0);
}

void GameObject_Door::Close()
{
    sEventMgr.RemoveEvents(this, EVENT_GAMEOBJECT_CLOSE);
    setState(GO_STATE_CLOSED);
}

void GameObject_Door::SpecialOpen()
{
    setState(GO_STATE_ALTERNATIVE_OPEN);
}

void GameObject_Door::onUse(Player* /*player*/)
{
    if (getState() == GO_STATE_CLOSED)
        Open();
    else
        Close();
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

    if (gameobject_properties->button.start_open != 0)
        setState(GO_STATE_OPEN);

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

void GameObject_Button::Open()
{
    setState(GO_STATE_OPEN);
    if (gameobject_properties->button.auto_close_time != 0)
        sEventMgr.AddEvent(this, &GameObject_Button::Close, EVENT_GAMEOBJECT_CLOSE, gameobject_properties->button.auto_close_time, 1, 0);
}

void GameObject_Button::Close()
{
    sEventMgr.RemoveEvents(this, EVENT_GAMEOBJECT_CLOSE);
    setState(GO_STATE_CLOSED);
}

void GameObject_Button::onUse(Player* player)
{
    sLogger.failure("Player uses Button.");
    if (getState() == GO_STATE_CLOSED)
    {
        Open();

        if (spell != nullptr)
            CastSpell(player->getGuid(), spell);
    }
    else
    {
        Close();
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

QuestProperties const* GameObject_QuestGiver::FindQuest(uint32 quest_id, uint8 quest_relation)
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

uint16 GameObject_QuestGiver::GetQuestRelation(uint32 quest_id)
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
    targetupdatetimer = 0;
    maxdistance = 0.0f;
    cooldown = 0;
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
    charges = gameobject_properties->trap.charges;

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

    cooldown = gameobject_properties->trap.cooldown * 1000;
    if (cooldown < 1000)
        cooldown = 1000;

    maxdistance = std::sqrt(float(gameobject_properties->trap.radius));
    if (maxdistance == 0.0f)
        maxdistance = 1.0f;

    GameObject::InitAI();
}

void GameObject_Trap::Update(unsigned long time_passed)
{
    if (m_deleted)
        return;

    if (m_event_Instanceid != m_instanceId)
    {
        event_Relocate();
        return;
    }

    if (!IsInWorld())
        return;

    if (spell == NULL)
        return;

    if (getState() == 1)
    {
        targetupdatetimer += time_passed;

        if (targetupdatetimer > cooldown)   // Update targets only if cooldown finished
            targetupdatetimer = 0;

        if (targetupdatetimer != 0)
            return;

        for (const auto& itr : getInRangeObjectsSet())
        {
            float dist;

            Object* o = itr;

            if (!o || !o->isCreatureOrPlayer())
                continue;

            if ((m_summoner != NULL) && (o->getGuid() == m_summoner->getGuid()))
                continue;

            dist = getDistanceSq(o);

            if (dist <= maxdistance)
            {

                if (m_summonedGo)
                {
                    if (!m_summoner)
                    {
                        expireAndDelete();
                        return;
                    }

                    if (!isAttackable(m_summoner, o))
                        continue;
                }

                CastSpell(o->getGuid(), spell);

                if (m_summoner != NULL)
                {
                    m_summoner->HandleProc(PROC_ON_TRAP_ACTIVATION, reinterpret_cast<Unit*>(o), spell, DamageInfo(), false);
                }

                if (charges != 0)
                    charges--;

                if (m_summonedGo && gameobject_properties->trap.charges != 0 && charges == 0)
                {
                    expireAndDelete();
                    return;
                }
                                                                                                    //Zyres: This is the same XD
                if (spell->getEffectImplicitTargetA(0) == EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT /*|| spell->getEffectImplicitTargetA(0) == EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT*/)
                {
                    return; // on area don't continue.
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Chair

void GameObject_Chair::onUse(Player* player)
{
    sLogger.failure("Player uses Chair.");

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
    SpawnLinkedTrap();
}

void GameObject_SpellFocus::SpawnLinkedTrap()
{
    uint32 trapid = gameobject_properties->spell_focus.linked_trap_id;
    if (trapid == 0)
        return;

    GameObject* go = m_WorldMap->createGameObject(trapid);
    if (go == nullptr)
    {
        sLogger.failure("Failed to create linked trap (entry: %u) for GameObject %u ( %s ). Missing GOProperties!", trapid, gameobject_properties->entry, gameobject_properties->name.c_str());
        return;
    }

    QuaternionData rot = QuaternionData::fromEulerAnglesZYX(m_position.getOrientation(), 0.f, 0.f);

    if (!go->create(trapid, m_mapId, m_phase, m_position, rot, GO_STATE_CLOSED))
    {
        sLogger.failure("Failed CreateFromProto for linked trap of GameObject %u ( %s ).", gameobject_properties->entry, gameobject_properties->name.c_str());
        return;
    }

    go->SetFaction(getFactionTemplate());
    go->setCreatedByGuid(getGuid());
    go->PushToWorld(m_WorldMap);
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

void GameObject_Goober::Open()
{
    setState(GO_STATE_OPEN);
    if (gameobject_properties->goober.auto_close_time != 0)
        sEventMgr.AddEvent(this, &GameObject_Goober::Close, EVENT_GAMEOBJECT_CLOSE, gameobject_properties->goober.auto_close_time, 1, 0);
}

void GameObject_Goober::Close()
{
    sEventMgr.RemoveEvents(this, EVENT_GAMEOBJECT_CLOSE);
    setState(GO_STATE_CLOSED);
}

void GameObject_Goober::onUse(Player* player)
{
    sLogger.failure("Player uses Goober.");
    if (getState() == GO_STATE_CLOSED)
    {
        Open();

        if (spell != nullptr)
            CastSpell(player->getGuid(), spell);

        player->castSpell(getGuid(), gameobject_properties->goober.spell_id, false);

        if (gameobject_properties->goober.page_id)
            player->sendPacket(SmsgGameobjectPagetext(getGuid()).serialise().get());
    }
    else
    {
        Close();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Camera
void GameObject_Camera::onUse(Player* player)
{
    if (gameobject_properties->camera.cinematic_id != 0)
        player->getSession()->SendPacket(SmsgTriggerCinematic(gameobject_properties->camera.cinematic_id).serialise().get());
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_FishingNode
GameObject_FishingNode::GameObject_FishingNode(uint64 GUID) : GameObject_Lootable(GUID)
{
    FishHooked = false;
}

GameObject_FishingNode::~GameObject_FishingNode()
{ }

void GameObject_FishingNode::OnPushToWorld()
{
    const uint32 zone = GetZoneId();

    // Only set a 'splash' if there is any loot in this area / zone
    if (sLootMgr.isFishable(zone))
    {
        uint32 seconds[] = { 0, 4, 10, 14 };
        uint32 rnd = Util::getRandomUInt(3);
        sEventMgr.AddEvent(this, &GameObject_FishingNode::EventFishHooked, EVENT_GAMEOBJECT_FISH_HOOKED, seconds[rnd] * 1000, 1, 0);

    }
    sEventMgr.AddEvent(this, &GameObject_FishingNode::EndFishing, true, EVENT_GAMEOBJECT_END_FISHING, 17 * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void GameObject_FishingNode::onUse(Player* player)
{
    sEventMgr.RemoveEvents(player, EVENT_STOP_CHANNELING);

    bool success = UseNode();

    if (success)
    {
        uint32 zone = player->getAreaId();
        if (zone == 0)                  // If the player's area ID is 0, use the zone ID instead
            zone = player->GetZoneId();

        MySQLStructure::FishingZones const* entry = sMySQLStore.getFishingZone(zone);
        if (entry == nullptr)
        {
            sLogger.failure("ERROR: Fishing zone information for zone %u not found!", zone);
            EndFishing(true);
            return;
        }

        const uint32 maxskill = entry->maxSkill;
        const uint32 minskill = entry->minSkill;

        if (player->getSkillLineCurrent(SKILL_FISHING, false) < maxskill)
            player->advanceSkillLine(SKILL_FISHING, static_cast<uint16_t>(float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE))));

        GameObject_FishingHole* school = nullptr;
        GameObject* go = getWorldMap()->getInterface()->findNearestGoWithType(this, GAMEOBJECT_TYPE_FISHINGHOLE);
        if (go != nullptr)
        {
            school = dynamic_cast<GameObject_FishingHole*>(go);

            if (!isInRange(school, static_cast<float>(school->GetGameObjectProperties()->fishinghole.radius)))
                school = nullptr;
        }

        if (school != nullptr)
        {
            sLootMgr.fillGOLoot(player, &school->loot, school->GetGameObjectProperties()->raw.parameter_1, getWorldMap()->getDifficulty());
            player->sendLoot(school->getGuid(), LOOT_FISHING, school->GetMapId());
            EndFishing(false);
            school->CatchFish();

        }
        else if (maxskill != 0 && Util::checkChance(((player->getSkillLineCurrent(SKILL_FISHING, true) - minskill) * 100) / maxskill))
        {
            sLootMgr.fillFishingLoot(player, &this->loot, zone, getWorldMap()->getDifficulty());
            player->sendLoot(getGuid(), LOOT_FISHING, GetMapId());
            EndFishing(false);
        }
        else
        {
            player->sendPacket(SmsgFishEscaped().serialise().get());
            EndFishing(true);
        }
    }
    else
    {
        player->sendPacket(SmsgFishNotHooked().serialise().get());
    }

    // Fishing is channeled spell
    auto channelledSpell = player->getCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (channelledSpell != nullptr)
    {
        if (success)
        {
            channelledSpell->sendChannelUpdate(0);
            channelledSpell->finish(true);
        }
        else
        {
            channelledSpell->sendChannelUpdate(0);
            channelledSpell->finish(false);
        }
    }
}

bool GameObject_FishingNode::UseNode()
{
    sEventMgr.RemoveEvents(this);

    // Clicking on the bobber before something is hooked
    if (!FishHooked)
    {
        EndFishing(true);
        return false;
    }
    return true;
}

void GameObject_FishingNode::EndFishing(bool abort)
{
    if (!abort)
        despawn(10000, 0);
    else
        expireAndDelete();
}

void GameObject_FishingNode::EventFishHooked()
{
    sendGameobjectCustomAnim();
    FishHooked = true;
}

bool GameObject_FishingNode::HasLoot()
{
    if (!loot.isLooted())
        return true;

    return false;
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
            uint32 victimid = Util::getRandomUInt(GetRitual()->GetMaxMembers() - 1);

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
    charges = gameobject_properties->spell_caster.charges;

    spell = sSpellMgr.getSpellInfo(gameobject_properties->spell_caster.spell_id);
    if (spell == nullptr)
        sLogger.failure("GameObject %u ( %s ) has a nonexistant spellID in the database.", gameobject_properties->entry, gameobject_properties->name.c_str());
}

void GameObject_SpellCaster::onUse(Player* player)
{
    if (GetGameObjectProperties()->spell_caster.party_only != 0)
    {
        if (m_summoner != nullptr && m_summoner->isPlayer())
        {
            Player* summoner = static_cast<Player*>(m_summoner);
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

    if (charges > 0 && --charges == 0)
        expireAndDelete();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Meetingstone
void GameObject_Meetingstone::onUse(Player* player)
{
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

    rGo->create(179944, player->GetMapId(), player->GetPositionX(), player->GetPosition(), rot, GO_STATE_CLOSED);
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
    if (player->getBattleground() != nullptr)
        player->getBattleground()->HookFlagStand(player, this);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_FishingHole
GameObject_FishingHole::GameObject_FishingHole(uint64 GUID) : GameObject_Lootable(GUID)
{
    usage_remaining = 0;
}

GameObject_FishingHole::~GameObject_FishingHole()
{ }

void GameObject_FishingHole::InitAI()
{
    CalcFishRemaining(true);
}

bool GameObject_FishingHole::CanFish()
{
    if (usage_remaining > 0)
        return true;
    else
        return false;
}

void GameObject_FishingHole::CatchFish()
{
    ASSERT(usage_remaining > 0);
    usage_remaining--;
    if (usage_remaining == 0)
        despawn(0, 1800 + Util::getRandomUInt(3600)); // respawn in 30 - 90 minutes
}

void GameObject_FishingHole::CalcFishRemaining(bool force)
{
    if (force || (usage_remaining == 0))
        usage_remaining = gameobject_properties->fishinghole.min_success_opens + Util::getRandomUInt(gameobject_properties->fishinghole.max_success_opens - gameobject_properties->fishinghole.min_success_opens) - 1;
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
    if (player->getBattleground())
        player->getBattleground()->HookFlagDrop(player, this);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_BarberChair
void GameObject_BarberChair::onUse(Player* player)
{
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

void GameObject_Destructible::Damage(uint32 damage, uint64 AttackerGUID, uint64 ControllerGUID, uint32 SpellID)
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

    uint8 animprogress = static_cast<uint8>(std::round(hitpoints / float(maxhitpoints)) * 255);
    setAnimationProgress(animprogress);
    SendDamagePacket(damage, AttackerGUID, ControllerGUID, SpellID);
}

void GameObject_Destructible::SendDamagePacket(uint32 damage, uint64 AttackerGUID, uint64 ControllerGUID, uint32 SpellID)
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
