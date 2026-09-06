/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <unordered_map>
#include <cstdint>
#include <shared_mutex>
#include <vector>
#include "WoWGuid.hpp"

class Object;
class Creature;
class GameObject;
class DynamicObject;
class Player;
class Pet;
class AreaTrigger;
class Corpse;
class Transporter;

struct Summary
{
    size_t any{ 0 };
    size_t creatures{ 0 };
    size_t gameObjects{ 0 };
    size_t dynamics{ 0 };
    size_t players{ 0 };
    size_t pets{ 0 };
    size_t corpses{ 0 };
    size_t transporters{ 0 };
    size_t total{ 0 };
};

namespace world
{
    class WorldObjectRegistry
    {
    public:
        WorldObjectRegistry() = default;

        void snapshotCreatures(std::vector<Creature*>& out) const;
        void snapshotGameObjects(std::vector<GameObject*>& out) const;
        void snapshotTransporters(std::vector<Transporter*>& out) const;
        void snapshotDynamicObjects(std::vector<DynamicObject*>& out) const;
        void snapshotPlayers(std::vector<Player*>& out) const;
        void snapshotPets(std::vector<Pet*>& out) const;
        void snapshotCorpses(std::vector<Corpse*>& out) const;

        template <typename T, typename Fn>
        void forEachPinned(std::vector<T*>& scratch, Fn&& fn) const;

        template <class Fn>
        void forEachPinnedByGuids(const std::vector<WoWGuid>& guids, Fn&& fn);

        template <class T, class Fn>
        void forEachPinnedByGuidsT(const std::vector<WoWGuid>& guids, Fn&& fn);

        /// Registry access is protected because sessions and map handoffs can overlap.
        void insert(Object* obj);
        void erase(Object* obj);
        void erase(const WoWGuid& g);
        void rekey(Object* obj, const WoWGuid& oldGuid, const WoWGuid& newGuid);
        bool contains(const WoWGuid& g) const;

        Creature*       getCreature     (const WoWGuid& g) const;
        Creature*       findCreatureByLow32(uint32_t low) const;
        GameObject*     getGameObject   (const WoWGuid& g) const;
        GameObject*     findGameObjectByLow32(uint32_t low) const;
        Transporter*    getTransporter  (const WoWGuid& g) const;
        DynamicObject*  getDynamicObject(const WoWGuid& g) const;
        Player*         getPlayer       (const WoWGuid& g) const;
        Pet*            getPet          (const WoWGuid& g) const;
        Corpse*         getCorpse       (const WoWGuid& g) const;
        Corpse*         getCorpseByOwner(uint32_t ownerLowGuid) const;
        Object*         getAny          (const WoWGuid& g) const; // returns base if any

        void   clearAll();

        size_t countAny()        const; // = m_objects.size()
        size_t countCreatures() const;
        size_t countGameObjects() const;
        size_t countDynamics() const;
        size_t countPlayers() const;
        size_t countPets() const;
        size_t countCorpses() const;
        size_t countTransporters() const;

        size_t countAll()        const;
        Summary counts() const;


    private:
        using KeyType = std::uint64_t;
        static inline KeyType keyOf(const WoWGuid& guid)
        {
            return guid.getRawGuid();
        }

        mutable std::shared_mutex m_mutex;
        std::unordered_map<KeyType, Object*>        m_objects;
        std::unordered_map<KeyType, Creature*>      m_creatures;
        std::unordered_map<KeyType, GameObject*>    m_gameObjects;
        std::unordered_map<KeyType, Transporter*>   m_transporters;
        std::unordered_map<KeyType, DynamicObject*> m_dynamicObjects;
        std::unordered_map<KeyType, Player*>        m_players;
        std::unordered_map<KeyType, Pet*>           m_pets;
        std::unordered_map<KeyType, Corpse*>        m_corpses;
    };

    template<typename T, typename Fn>
    inline void WorldObjectRegistry::forEachPinned(std::vector<T*>& scratch, Fn&& fn) const
    {
        for (T* obj : scratch)
        {
            if (!obj)
                continue;
            typename T::UpdatePin _pin(obj);   // RAII-Pin
            fn(*obj);
        }
    }
    template<class Fn>
    inline void WorldObjectRegistry::forEachPinnedByGuids(const std::vector<WoWGuid>& guids, Fn&& fn)
    {
        for (const WoWGuid& g : guids)
        {
            Object* obj = nullptr;

            {
                std::shared_lock lk(m_mutex);
                auto it = m_objects.find(keyOf(g));
                if (it == m_objects.end() || !it->second)
                    continue;

                obj = it->second;

                Object::UpdatePin _pin(obj); // RAII-Pin
                lk.unlock();
                fn(*obj);

                }
        }

    }

    template<class T, class Fn>
    inline void WorldObjectRegistry::forEachPinnedByGuidsT(const std::vector<WoWGuid>& guids, Fn&& fn)
    {
        for (const WoWGuid& g : guids)
        {
            T* obj = nullptr;

            {
                std::shared_lock lk(m_mutex);

                auto it = m_objects.find(keyOf(g));
                if (it == m_objects.end() || !it->second)
                    continue;

                obj = dynamic_cast<T*>(it->second);
                if (!obj)
                    continue;

                typename T::UpdatePin _pin(obj); // RAII-Pin
                lk.unlock();
                fn(*obj);

                }
        }
    }
}
