/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WorldObjectRegistry.hpp"
#include <type_traits>
#include <mutex>
#include "Objects/Object.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/GameObject.h"
#include "Objects/DynamicObject.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Transporter.hpp"


namespace world
{
    void WorldObjectRegistry::snapshotCreatures(std::vector<Creature*>& out) const
    {
        std::shared_lock lock(m_mutex);
        out.clear();
        out.reserve(m_creatures.size());
        for (auto& entry : m_creatures)
        {
            out.push_back(entry.second);
        }
    }

    void WorldObjectRegistry::snapshotGameObjects(std::vector<GameObject*>& out) const
    {
        std::shared_lock lock(m_mutex);
        out.clear();
        out.reserve(m_gameObjects.size());
        for (auto& entry : m_gameObjects)
        {
            out.push_back(entry.second);
        }
    }

    void WorldObjectRegistry::snapshotTransporters(std::vector<Transporter*>& out) const
    {
        std::shared_lock lock(m_mutex);
        out.clear();
        out.reserve(m_transporters.size());
        for (auto& entry : m_transporters)
        {
            out.push_back(entry.second);
        }
    }

    void WorldObjectRegistry::snapshotDynamicObjects(std::vector<DynamicObject*>& out) const
    {
        std::shared_lock lock(m_mutex);
        out.clear();
        out.reserve(m_dynamicObjects.size());
        for (auto& entry : m_dynamicObjects)
        {
            out.push_back(entry.second);
        }
    }

    void WorldObjectRegistry::snapshotPlayers(std::vector<Player*>& out) const
    {
        std::shared_lock lock(m_mutex);
        out.clear();
        out.reserve(m_players.size());
        for (auto& entry : m_players)
        {
            out.push_back(entry.second);
        }
    }

    void WorldObjectRegistry::snapshotPets(std::vector<Pet*>& out) const
    {
        std::shared_lock lock(m_mutex);
        out.clear();
        out.reserve(m_pets.size());
        for (auto& entry : m_pets)
        {
            out.push_back(entry.second);
        }
    }

    void WorldObjectRegistry::snapshotCorpses(std::vector<Corpse*>& out) const
    {
        std::shared_lock lock(m_mutex);
        out.clear();
        out.reserve(m_corpses.size());
        for (auto& entry : m_corpses)
        {
            out.push_back(entry.second);
        }
    }

    void WorldObjectRegistry::insert(Object* obj)
    {
        if (!obj)
        {
            return;
        }

        const KeyType key = keyOf(obj->GetNewGUID());
        std::unique_lock lock(m_mutex);

        if (auto it = m_objects.find(key); it != m_objects.end() && it->second != obj)
        {
            sLogger.warning("Registry: GUID {} already mapped to {}, overwriting with {}",
                             key, (void*)it->second, (void*)obj);
        }

        m_objects[key] = obj;

        if (auto* player = dynamic_cast<Player*>(obj))
        {
            m_players[key] = player;
        }
        else if (auto* pet = dynamic_cast<Pet*>(obj))
        {
            m_pets[key] = pet;
        }
        else if (auto* creature = dynamic_cast<Creature*>(obj))
        {
            m_creatures[key] = creature;
        }
        else if (auto* transporter = dynamic_cast<Transporter*>(obj))
        {
            m_transporters[key] = transporter;
        }
        else if (auto* gameObject = dynamic_cast<GameObject*>(obj))
        {
            m_gameObjects[key] = gameObject;
        }
        else if (auto* dynamicObject = dynamic_cast<DynamicObject*>(obj))
        {
            m_dynamicObjects[key] = dynamicObject;
        }
        else if (auto* corpse = dynamic_cast<Corpse*>(obj))
        {
            m_corpses[key] = corpse;
        }
    }

    void WorldObjectRegistry::erase(Object* obj)
    {
        if (!obj)
            return;

        erase(obj->GetNewGUID());
    }

    void WorldObjectRegistry::erase(const WoWGuid& g)
    {
        const KeyType key = keyOf(g);
        std::unique_lock lock(m_mutex);
        m_objects.erase(key);
        m_players.erase(key);
        m_creatures.erase(key);
        m_gameObjects.erase(key);
        m_dynamicObjects.erase(key);
        m_pets.erase(key);
        m_corpses.erase(key);
        m_transporters.erase(key);
    }

    void WorldObjectRegistry::rekey(Object* obj, const WoWGuid& oldGuid, const WoWGuid& newGuid)
    {
        if (!obj)
            return;

        const KeyType oldKey = keyOf(oldGuid);
        const KeyType newKey = keyOf(newGuid);
        if (oldKey == newKey)
            return;

        std::unique_lock lock(m_mutex);

        auto eraseIfSame = [obj, oldKey](auto& map)
        {
            auto it = map.find(oldKey);
            if (it != map.end() && it->second == obj)
                map.erase(it);
        };

        eraseIfSame(m_objects);
        eraseIfSame(m_players);
        eraseIfSame(m_creatures);
        eraseIfSame(m_gameObjects);
        eraseIfSame(m_dynamicObjects);
        eraseIfSame(m_pets);
        eraseIfSame(m_corpses);
        eraseIfSame(m_transporters);

        if (auto it = m_objects.find(newKey); it != m_objects.end() && it->second != obj)
        {
            sLogger.warning("Registry: GUID {} already mapped to {}, overwriting with {} during rekey",
                            newKey, (void*)it->second, (void*)obj);
        }

        m_objects[newKey] = obj;

        if (auto* player = dynamic_cast<Player*>(obj))
        {
            m_players[newKey] = player;
        }
        else if (auto* pet = dynamic_cast<Pet*>(obj))
        {
            m_pets[newKey] = pet;
        }
        else if (auto* creature = dynamic_cast<Creature*>(obj))
        {
            m_creatures[newKey] = creature;
        }
        else if (auto* transporter = dynamic_cast<Transporter*>(obj))
        {
            m_transporters[newKey] = transporter;
        }
        else if (auto* gameObject = dynamic_cast<GameObject*>(obj))
        {
            m_gameObjects[newKey] = gameObject;
        }
        else if (auto* dynamicObject = dynamic_cast<DynamicObject*>(obj))
        {
            m_dynamicObjects[newKey] = dynamicObject;
        }
        else if (auto* corpse = dynamic_cast<Corpse*>(obj))
        {
            m_corpses[newKey] = corpse;
        }
    }

    bool WorldObjectRegistry::contains(const WoWGuid& g) const
    {
        std::shared_lock lock(m_mutex);
        return m_objects.find(keyOf(g)) != m_objects.end();
    }

    template <class Map>
    static auto findByLow32(const Map& map, uint32_t low)
    {
        for (const auto& [_, obj] : map)
        {
            if (obj && obj->GetNewGUID().getLowGuid() == low)
                return obj;
        }

        return typename Map::mapped_type{};
    }

    Creature* WorldObjectRegistry::getCreature(const WoWGuid& guid) const
    {
        std::shared_lock lock(m_mutex);
        auto itr = m_creatures.find(keyOf(guid));
        return itr == m_creatures.end() ? nullptr : itr->second;
    }
    Creature* WorldObjectRegistry::findCreatureByLow32(uint32_t low) const
    {
        std::shared_lock lock(m_mutex);
        return findByLow32(m_creatures, low);
    }

    GameObject* WorldObjectRegistry::getGameObject(const WoWGuid& guid) const
    {
        std::shared_lock lock(m_mutex);
        auto itr = m_gameObjects.find(keyOf(guid));
        return itr == m_gameObjects.end() ? nullptr : itr->second;
    }
    GameObject* WorldObjectRegistry::findGameObjectByLow32(uint32_t low) const
    {
        std::shared_lock lock(m_mutex);
        return findByLow32(m_gameObjects, low);
    }

    Transporter* WorldObjectRegistry::getTransporter(const WoWGuid& guid) const
    {
        std::shared_lock lock(m_mutex);
        auto itr = m_transporters.find(keyOf(guid));
        return itr == m_transporters.end() ? nullptr : itr->second;
    }
    DynamicObject* WorldObjectRegistry::getDynamicObject(const WoWGuid& guid) const
    {
        std::shared_lock lock(m_mutex);
        auto itr = m_dynamicObjects.find(keyOf(guid));
        return itr == m_dynamicObjects.end() ? nullptr : itr->second;
    }
    Player* WorldObjectRegistry::getPlayer(const WoWGuid& guid) const
    {
        std::shared_lock lock(m_mutex);
        auto itr = m_players.find(keyOf(guid));
        return itr == m_players.end() ? nullptr : itr->second;
    }
    Pet* WorldObjectRegistry::getPet(const WoWGuid& guid) const
    {
        std::shared_lock lock(m_mutex);
        auto itr = m_pets.find(keyOf(guid));
        return itr == m_pets.end() ? nullptr : itr->second;
    }
    Corpse* WorldObjectRegistry::getCorpse(const WoWGuid& guid) const
    {
        std::shared_lock lock(m_mutex);
        auto itr = m_corpses.find(keyOf(guid));
        return itr == m_corpses.end() ? nullptr : itr->second;
    }
    Corpse* WorldObjectRegistry::getCorpseByOwner(uint32_t ownerLowGuid) const
    {
        std::shared_lock lock(m_mutex);

        Corpse* bestBody = nullptr;
        Corpse* bestAny = nullptr;

        for (const auto& [_, corpse] : m_corpses)
        {
            if (!corpse)
                continue;

            WoWGuid owner;
            owner.init(corpse->getOwnerGuid());
            if (owner.getCounter() != ownerLowGuid)
                continue;

            /// Prefer the newest body if old duplicate corpses still exist.
            if (!bestAny || corpse->getGuidLow() > bestAny->getGuidLow())
                bestAny = corpse;

            if (corpse->getCorpseState() == CORPSE_STATE_BODY)
            {
                if (!bestBody || corpse->getGuidLow() > bestBody->getGuidLow())
                    bestBody = corpse;
            }
        }

        return bestBody ? bestBody : bestAny;
    }

    Object* WorldObjectRegistry::getAny(const WoWGuid& guid) const
    {
        std::shared_lock lock(m_mutex);
        auto itr = m_objects.find(keyOf(guid));
        return itr == m_objects.end() ? nullptr : itr->second;
    }

    void WorldObjectRegistry::clearAll()
    {
        std::unique_lock lock(m_mutex);
        m_objects.clear();
        m_players.clear();
        m_creatures.clear();
        m_gameObjects.clear();
        m_dynamicObjects.clear();
        m_pets.clear();
        m_corpses.clear();
        m_transporters.clear();
    }

    size_t WorldObjectRegistry::countAny() const
    {
        std::shared_lock lock(m_mutex);
        return m_objects.size();
    }
    size_t WorldObjectRegistry::countCreatures() const
    {
        std::shared_lock lock(m_mutex);
        return m_creatures.size();
    }
    size_t WorldObjectRegistry::countGameObjects() const
    {
        std::shared_lock lock(m_mutex);
        return m_gameObjects.size();
    }
    size_t WorldObjectRegistry::countDynamics() const
    {
        std::shared_lock lock(m_mutex);
        return m_dynamicObjects.size();
    }
    size_t WorldObjectRegistry::countPlayers() const
    {
        std::shared_lock lock(m_mutex);
        return m_players.size();
    }
    size_t WorldObjectRegistry::countPets() const
    {
        std::shared_lock lock(m_mutex);
        return m_pets.size();
    }
    size_t WorldObjectRegistry::countCorpses() const
    {
        std::shared_lock lock(m_mutex);
        return m_corpses.size();
    }
    size_t WorldObjectRegistry::countTransporters() const
    {
        std::shared_lock lock(m_mutex);
        return m_transporters.size();
    }

    size_t WorldObjectRegistry::countAll() const
    {
        std::shared_lock lock(m_mutex);
        return m_creatures.size() + m_gameObjects.size() + m_dynamicObjects.size()  + m_players.size() + m_pets.size() + m_corpses.size() + m_transporters.size();
    }

    Summary WorldObjectRegistry::counts() const
    {
        std::shared_lock lock(m_mutex);
        Summary s;
        s.any = m_objects.size();
        s.creatures = m_creatures.size();
        s.gameObjects = m_gameObjects.size();
        s.transporters = m_transporters.size();
        s.dynamics = m_dynamicObjects.size();
        s.players = m_players.size();
        s.pets = m_pets.size();
        s.corpses = m_corpses.size();
        s.total = s.creatures + s.gameObjects + s.transporters + s.dynamics + s.players + s.pets + s.corpses;
        return s;
    }

} /// namespace world
