/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ObjectFactory.hpp"
#include "GuidAllocator.hpp"
#include "WorldObjectRegistry.hpp"
#include "Map/Visibility/VisibilitySystem.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Server/World.h"

/// Engine
#include "Objects/Object.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/GameObject.h"
#include "Objects/Transporter.hpp"
#include "Objects/DynamicObject.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Corpse.hpp"

using Maker = GameObject* (*)(uint64_t);

ObjectFactory::ObjectFactory(WorldMap& map, visibility::VisibilitySystem& visibilitySystem, world::WorldObjectRegistry& reg, GuidAllocator& guids)
    : m_worldMap(map), m_visibilitySystem(visibilitySystem), m_objectRegistry(reg), m_guidAllocator(guids)
{
}

template <class T> static GameObject* make(uint64_t g) { return new T(g); }
static const std::unordered_map<int, Maker> registry =
{
    { GAMEOBJECT_TYPE_DOOR,                  &make<GameObject_Door> },
    { GAMEOBJECT_TYPE_BUTTON,                &make<GameObject_Button> },
    { GAMEOBJECT_TYPE_QUESTGIVER,            &make<GameObject_QuestGiver> },
    { GAMEOBJECT_TYPE_CHEST,                 &make<GameObject_Chest> },
    { GAMEOBJECT_TYPE_TRAP,                  &make<GameObject_Trap> },
    { GAMEOBJECT_TYPE_CHAIR,                 &make<GameObject_Chair> },
    { GAMEOBJECT_TYPE_SPELL_FOCUS,           &make<GameObject_SpellFocus> },
    { GAMEOBJECT_TYPE_GOOBER,                &make<GameObject_Goober> },
    { GAMEOBJECT_TYPE_TRANSPORT,             &make<GameObject_Transport> },
    { GAMEOBJECT_TYPE_CAMERA,                &make<GameObject_Camera> },
    { GAMEOBJECT_TYPE_FISHINGNODE,           &make<GameObject_FishingNode> },
    { GAMEOBJECT_TYPE_RITUAL,                &make<GameObject_Ritual> },
    { GAMEOBJECT_TYPE_SPELLCASTER,           &make<GameObject_SpellCaster> },
    { GAMEOBJECT_TYPE_MEETINGSTONE,          &make<GameObject_Meetingstone> },
    { GAMEOBJECT_TYPE_FLAGSTAND,             &make<GameObject_FlagStand> },
    { GAMEOBJECT_TYPE_FISHINGHOLE,           &make<GameObject_FishingHole> },
    { GAMEOBJECT_TYPE_FLAGDROP,              &make<GameObject_FlagDrop> },
    { GAMEOBJECT_TYPE_BARBER_CHAIR,          &make<GameObject_BarberChair> },
    { GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING, &make<GameObject_Destructible> },
};

visibility::ObjectMeta ObjectFactory::makeMeta(Object* obj, const WoWGuid& g) const
{
    visibility::ObjectMeta m;
    m.guid = g;

    if (dynamic_cast<Player*>(obj))
    {
        m.container = visibility::Container::Players;
    }
    else if (dynamic_cast<Pet*>(obj))
    {
        m.container = visibility::Container::Pets;
    }
    else if (dynamic_cast<Creature*>(obj))
    {
        m.container = visibility::Container::Creatures;
    }
    else if (dynamic_cast<Transporter*>(obj))
    {
        m.container = visibility::Container::Transporter;
    }
    else if (dynamic_cast<GameObject*>(obj))
    {
        m.container = visibility::Container::GameObjects;
    }
    else if (dynamic_cast<DynamicObject*>(obj))
    {
        m.container = visibility::Container::DynamicObjects;
    }
    else if (dynamic_cast<Corpse*>(obj))
    {
        m.container = visibility::Container::Corpses;
    }
    else
    {
        m.container = visibility::Container::Unknown;
    }
    return m;
}

void ObjectFactory::removeAndDestroy(Object* obj, bool recycleGuid)
{
    if (!obj)
        return;

    const WoWGuid guid = obj->GetNewGUID();

    if (obj->IsInWorld())
    {
        if (obj->getWorldMap() != &m_worldMap)
        {
            sLogger.failure("ObjectFactory::removeAndDestroy: guid={} belongs to another map", guid.getRawGuid());
            return;
        }

        detachFromWorld(obj, /*keepRegistry=*/false, /*soft=*/false);
    }
    else if (m_objectRegistry.getAny(guid) == obj)
    {
        /// Soft-detached runtime objects intentionally remain registered so the
        /// same instance can be reattached later. If such an object is finally
        /// destroyed while detached, remove that retained registry entry first.
        m_objectRegistry.erase(guid);
    }

    recycleAndDestroy(obj, recycleGuid);
}

void ObjectFactory::recycleAndDestroy(Object* obj, bool recycleGuid)
{
    if (!obj)
        return;

    /// Final destruction must never bypass the detach lifecycle. Attached objects
    /// still own spatial, visibility, collision and script state that has to be
    /// released by detachFromWorld() first.
    if (obj->IsInWorld())
    {
        sLogger.failure("ObjectFactory::recycleAndDestroy: refusing to destroy attached object guid={}", obj->GetNewGUID().getRawGuid());
        return;
    }

    const WoWGuid guid = obj->GetNewGUID();

    /// A detached object must no longer be reachable through the live world
    /// registry. Keeping it registered while deferring deletion would leave a
    /// dangling pointer as soon as drainDeferredDestroy() runs.
    if (m_objectRegistry.getAny(guid) == obj)
    {
        sLogger.failure("ObjectFactory::recycleAndDestroy: refusing to destroy registered object guid={}", guid.getRawGuid());
        return;
    }

    /// Queue first so duplicate destruction requests cannot recycle the same GUID
    /// more than once. The WorldMap set owns the final delete from this point on.
    if (!m_worldMap.deferDestroy(obj))
        return;

    if (recycleGuid)
        m_guidAllocator.release(guid.getRawGuid());
}


void ObjectFactory::attachToWorld(Object* obj)
{
    if (!obj)
        return;

    const WoWGuid guid = obj->GetNewGUID();

    if (obj->IsInWorld())
    {
        if (obj->getWorldMap() == &m_worldMap)
        {
            if (m_worldMap.getSpatialIndex().handleByGuid(guid).id)
            {
                sLogger.warning("ObjectFactory::attachToWorld: guid={} already attached", guid.getRawGuid());
                return;
            }
        }
        else
        {
            sLogger.warning("ObjectFactory::attachToWorld: guid={} already belongs to another map", guid.getRawGuid());
            return;
        }
    }

    /// Registry-Index
    if (!m_objectRegistry.contains(obj->GetNewGUID()))
    {
        m_objectRegistry.insert(obj);
    }

    /// Register the Object to this World
    obj->registerToWorld(m_worldMap);

    /// Object Created and Ready. Call Events before Attached to Visibility
    obj->onPreAttachToWorld();

    auto meta = makeMeta(obj, obj->GetNewGUID());
    auto h = m_worldMap.getSpatialIndex().addObject(meta, obj->GetPosition(), obj);
    m_worldMap.onGridMaterialized(visibility::SpatialIndex::packGridFromPos(obj->GetPosition()));
    m_visibilitySystem.onObjectAdded(h);

    auto interest = m_visibilitySystem.buildInterestProfile(obj);

    m_visibilitySystem.applyInterestProfile(h, interest);

    /// Player::onAttachToWorld() immediately calls processPendingUpdates() while the
    /// client is still loading. Flush this player's initial visibility batch now so
    /// login/worldport keeps the old synchronous behavior for the entering player.
    if (obj->isPlayer())
        m_worldMap.processPendingVisibilityChangesForViewer(obj->GetNewGUID(), 256, 256);

    /// Call Events for Object after Attaching to Visibility
    obj->onAttachToWorld();
}

void ObjectFactory::detachFromWorld(Object* obj, bool keepRegistry /*= false*/, bool soft /*= false*/)
{
    if (!obj)
        return;

    if (auto* script = m_worldMap.getScript())
        script->removeObject(obj);

    /// Call Event before we Invalidate all Info
    obj->onPreDetachFromWorld();

    const WoWGuid guid = obj->GetNewGUID();

    if (auto h = m_worldMap.getSpatialIndex().handleByGuid(guid); h.id)
    {
        /// Farsight can add visibility that belongs only to the remote viewer.
        Player* remoteRecipient = nullptr;
        if (Unit* viewerUnit = obj->ToUnit())
            remoteRecipient = viewerUnit->m_playerControler;
        else if (auto* viewerDyn = dynamic_cast<DynamicObject*>(obj); viewerDyn && viewerDyn->getDynamicType() == DYNAMIC_OBJECT_FARSIGHT_FOCUS)
            remoteRecipient = m_worldMap.getPlayer(WoWGuid(viewerDyn->getCasterGuid()));

        if (remoteRecipient)
            m_worldMap.clearVisibilitySourceForRecipient(guid, remoteRecipient);

        m_visibilitySystem.onObjectRemoving(h);
        const bool removed = m_worldMap.getSpatialIndex().removeObject(h);
        if (!removed || m_worldMap.getSpatialIndex().handleByGuid(guid).id)
            sLogger.warning("vis sanity: spatial detach incomplete guid={} removed={} handleStillPresent={}", guid.getRawGuid(), removed, m_worldMap.getSpatialIndex().handleByGuid(guid).id != 0);

        /// Flush farsight removals while the viewer can still be resolved.
        const Unit* viewerUnit = obj->ToUnit();
        const auto* viewerDyn = dynamic_cast<DynamicObject*>(obj);
        const bool temporaryRemoteViewer =
            (viewerUnit && viewerUnit->m_playerControler) ||
            (viewerDyn && viewerDyn->getDynamicType() == DYNAMIC_OBJECT_FARSIGHT_FOCUS);
        if (temporaryRemoteViewer)
        {
            m_worldMap.processPendingVisibilityChangesForViewer(guid, 4096, 4096);
            m_worldMap.clearVisibilityRecipientForViewer(guid);
        }

        m_worldMap.purgePendingVisibilityForGuid(guid);
    }

    /// Call Events for Object after Detaching from Visibility
    if (!soft)
        obj->onDetachFromWorld();

    /// Unregister us from Worldmap
    obj->unregisterFromWorld();

    if (!keepRegistry)
        m_objectRegistry.erase(guid);
}

void ObjectFactory::transferWorld(Object* obj, WorldMap* newMap)
{
    if (!obj || !newMap || newMap == &m_worldMap)
        return;

    const WoWGuid guid = obj->GetNewGUID();
    const LocationVector transferPosition = obj->GetPosition();
    const uint32_t oldMapId = m_worldMap.getBaseMap()->getMapId();
    const uint32_t newMapId = newMap->getBaseMap()->getMapId();

    /// Remove the object from the old map before handing it to the new one.
    auto removeHandle = m_worldMap.getSpatialIndex().handleByGuid(guid);
    if (removeHandle.id)
    {
        m_visibilitySystem.onObjectRemoving(removeHandle);
        m_worldMap.flushVisibilityRemovalForObject(guid);

        const bool removed = m_worldMap.getSpatialIndex().removeObject(removeHandle);
        if (!removed || m_worldMap.getSpatialIndex().handleByGuid(guid).id)
        {
            sLogger.warning(
                "vis sanity: transfer detach incomplete guid={} removed={} handleStillPresent={}",
                guid.getRawGuid(),
                removed,
                m_worldMap.getSpatialIndex().handleByGuid(guid).id != 0);
        }

        m_worldMap.purgePendingVisibilityForGuid(guid);
    }

    if (auto* script = m_worldMap.getScript())
        script->removeObject(obj);

    m_objectRegistry.erase(guid);
    obj->unregisterFromWorld();

    /// Finish the attach on the target map thread.
    const bool isTransporter = obj->isTransporter();

    newMap->queueMapTask(
        [obj, newMap, transferPosition, isTransporter, oldMapId, newMapId]()
        {
            if (!obj || !newMap || obj->IsInWorld())
                return;

            newMap->getObjectFactory().finishTransferWorld(obj, transferPosition);

            if (isTransporter)
            {
                auto* transporter = reinterpret_cast<Transporter*>(obj);

                // Only teleport passengers after the transporter is fully attached
                // to the destination map. This guarantees that players can see and
                // remain attached to their transport when WORLDPORT_ACK is handled.
                transporter->TeleportPlayers(
                    transferPosition.x,
                    transferPosition.y,
                    transferPosition.z,
                    transferPosition.o,
                    newMapId,
                    oldMapId,
                    true,
                    newMap);
            }
        });
}

void ObjectFactory::finishTransferWorld(Object* obj, const LocationVector& transferPosition)
{
    if (!obj || obj->IsInWorld())
        return;

    const WoWGuid guid = obj->GetNewGUID();

    if (!m_objectRegistry.contains(guid))
        m_objectRegistry.insert(obj);

    obj->registerToWorld(m_worldMap);

    auto meta = makeMeta(obj, guid);
    auto h = m_worldMap.getSpatialIndex().addObject(meta, obj->GetPosition(), obj);
    if (!h.id)
    {
        m_objectRegistry.erase(guid);
        obj->unregisterFromWorld();

        sLogger.failure(
            "ObjectFactory::finishTransferWorld: failed to attach object {} to map {}",
            guid.getRawGuid(),
            m_worldMap.getBaseMap()->getMapId());
        return;
    }

    m_visibilitySystem.onObjectAdded(h);

    auto interest = m_visibilitySystem.buildInterestProfile(obj);

    if (interest.viewer && interest.viewerSubscribeCells <= 0)
        interest.viewerSubscribeCells = worldConfig.server.mapCellNumber;

    if (interest.activator && interest.activatorSubscribeCells <= 0)
        interest.activatorSubscribeCells = worldConfig.server.mapCellNumber;

    m_visibilitySystem.applyInterestProfile(h, interest);

    if (auto* script = m_worldMap.getScript())
    {
        if (obj->isGameObject())
            script->OnGameObjectPushToWorld(obj->ToGameObject());
        else if (obj->isCreature())
            script->OnCreaturePushToWorld(obj->ToCreature());

        script->addObject(obj);
    }

    if (obj->isTransporter())
    {
        auto* transporter = reinterpret_cast<Transporter*>(obj);

        /// Players already received their destination position before the
        /// worldport. UpdatePassengerPositions skips players that are not yet
        /// attached to the new WorldMap and passengers still carrying the old
        /// map id.
        transporter->UpdatePosition(
            transferPosition.x,
            transferPosition.y,
            transferPosition.z,
            transferPosition.o);

        transporter->LoadStaticPassengers();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Creatures
//////////////////////////////////////////////////////////////////////////////////////////
Creature* ObjectFactory::createCreature(uint32_t entry)
{
    bool isVehicle = false;

    if (auto props = sMySQLStore.getCreatureProperties(entry))
        isVehicle = props->vehicleid != 0;

    uint64_t guid = m_guidAllocator.allocCreature(entry, true, isVehicle);
    return new Creature(guid);
}

Creature* ObjectFactory::createCreature(uint32_t entry, const LocationVector& pos)
{
    Creature* creature = createCreature(entry);
    if (!creature)
        return nullptr;

    const auto* creatureInfo = sMySQLStore.getCreatureProperties(entry);
    if (!creatureInfo)
    {
        sLogger.failure("ObjectFactory::createCreature: invalid entry {}", entry);
        delete creature;
        return nullptr;
    }

    creature->Load(creatureInfo, pos.x, pos.y, pos.z, pos.o);
    return creature;
}

Creature* ObjectFactory::createCreatureFromSpawns(const MySQLStructure::CreatureSpawn& rowIn)
{
    Creature* creature = createCreature(rowIn.entry);
    if (!creature)
        return nullptr;

    creature->m_loadedFromDB = true;
    if (!creature->LoadFromDB(const_cast<MySQLStructure::CreatureSpawn*>(&rowIn), &m_worldMap, /*addToWorldLater=*/false) || !creature->CanAddToWorld())
    {
        sLogger.failure("ObjectFactory::createCreatureFromSpawns: failed to load spawn {}", rowIn.id);
        delete creature;
        return nullptr;
    }

    return creature;
}

Creature* ObjectFactory::createAndSpawnCreature(uint32_t entry, const LocationVector& pos)
{
    Creature* creature = createCreature(entry, pos);
    if (!creature)
        return nullptr;

    attachToWorld(creature);
    return creature;
}

Creature* ObjectFactory::createAndSpawnCreatureFromSpawns(const MySQLStructure::CreatureSpawn& rowIn)
{
    Creature* creature = createCreatureFromSpawns(rowIn);
    if (!creature)
        return nullptr;

    attachToWorld(creature);
    return creature;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// GameObjects
//////////////////////////////////////////////////////////////////////////////////////////
GameObject* ObjectFactory::createGameObject(uint32_t entry)
{
    GameObjectProperties const* gameobjectProperties = sMySQLStore.getGameObjectProperties(entry);
    if (!gameobjectProperties)
        return nullptr;

    uint64_t guid = m_guidAllocator.allocGameObject(entry, true);

    GameObject* gameObject = nullptr;
    if (const auto it = registry.find(gameobjectProperties->type); it != registry.end())
        gameObject = it->second(guid);
    else
        gameObject = new GameObject(guid);

    gameObject->SetGameObjectProperties(gameobjectProperties);
    return gameObject;
}

GameObject* ObjectFactory::createGameObject(uint32_t entry, const LocationVector& pos, const QuaternionData& rotation)
{
    GameObject* gameObject = createGameObject(entry);
    if (!gameObject)
        return nullptr;

    if (!gameObject->create(entry, &m_worldMap, gameObject->GetPhase(), pos, rotation, GO_STATE_CLOSED, 0))
    {
        sLogger.failure("ObjectFactory::createGameObject: failed to initialize entry {}", entry);
        delete gameObject;
        return nullptr;
    }

    return gameObject;
}

GameObject* ObjectFactory::createGameObjectFromSpawns(const MySQLStructure::GameobjectSpawn& rowIn)
{
    GameObject* go = createGameObject(rowIn.entry);
    if (!go)
        return nullptr;

    go->m_loadedFromDB = true;
    if (!go->loadFromDB(const_cast<MySQLStructure::GameobjectSpawn*>(&rowIn), &m_worldMap, /*addToWorldLater=*/false))
    {
        sLogger.failure("ObjectFactory::createGameObjectFromSpawns: failed to load spawn {}", rowIn.id);
        delete go;
        return nullptr;
    }

    return go;
}

GameObject* ObjectFactory::createAndSpawnGameObject(uint32_t entry, const LocationVector& pos, const QuaternionData& rotation)
{
    GameObject* gameObject = createGameObject(entry, pos, rotation);
    if (!gameObject)
        return nullptr;

    attachToWorld(gameObject);
    return gameObject;
}

GameObject* ObjectFactory::createAndSpawnGameObjectFromSpawns(const MySQLStructure::GameobjectSpawn& rowIn)
{
    GameObject* go = createGameObjectFromSpawns(rowIn);
    if (!go)
        return nullptr;

    attachToWorld(go);
    return go;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Transporters
//////////////////////////////////////////////////////////////////////////////////////////
Transporter* ObjectFactory::createTransporter(uint32_t entry, uint32_t mapId, const LocationVector& pos)
{
    uint64_t guid = m_guidAllocator.allocTransporter(entry, true);

    Transporter* trans = new Transporter(guid);
    if (!trans->Create(entry, mapId, pos.x, pos.y, pos.z, pos.o, 255))
    {
        delete trans;
        return nullptr;
    }

    return trans;
}

Transporter* ObjectFactory::createAndSpawnTransporter(uint32_t entry, uint32_t mapId, const LocationVector& pos)
{
    Transporter* trans = createTransporter(entry, mapId, pos);
    if (!trans)
        return nullptr;

    attachToWorld(trans);
    return trans;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// DynamicObjects
//////////////////////////////////////////////////////////////////////////////////////////
DynamicObject* ObjectFactory::createDynamic()
{
    uint64_t guid = m_guidAllocator.allocDynamicObject(0, false);

    DynamicObject* dynamicObject = new DynamicObject(guid);

    if (!dynamicObject)
        return nullptr;

    return dynamicObject;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Corpses
//////////////////////////////////////////////////////////////////////////////////////////
Corpse* ObjectFactory::createCorpse(uint64_t rawGuid)
{
    const uint64_t guid = rawGuid ? rawGuid : m_guidAllocator.allocCorpse(true);

    Corpse* corpse = new Corpse(guid);
    if (!corpse)
        return nullptr;

    return corpse;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// GUID Passthroughs
//////////////////////////////////////////////////////////////////////////////////////////
uint64_t ObjectFactory::generateCreatureGuid(uint32_t entry, bool reuse) const { return const_cast<GuidAllocator&>(m_guidAllocator).allocCreature(entry, reuse, sMySQLStore.getCreatureProperties(entry) && sMySQLStore.getCreatureProperties(entry)->vehicleid != 0); }
uint64_t ObjectFactory::generateGameObjectGuid(uint32_t entry, bool reuse) const { return const_cast<GuidAllocator&>(m_guidAllocator).allocGameObject(entry, reuse); }
uint64_t ObjectFactory::generateDynamicGuid(bool reuse) const { return const_cast<GuidAllocator&>(m_guidAllocator).allocDynamicObject(0, reuse); }
uint64_t ObjectFactory::generateCorpseGuid(bool reuse) const { return const_cast<GuidAllocator&>(m_guidAllocator).allocCorpse(reuse); }
