/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/ObjectDefines.h"
#include "Objects/Object.h"
#include "Objects/Units/Creatures/Creature.h"

#include "Map/Cells/MapCell.hpp"
#include "Map/Maps/WorldMap.hpp"

class GameObject;
class Creature;
class Unit;
class Player;

//////////////////////////////////////////////////////////////////////////////////////////
/// Class MapScriptInterface
/// Provides an interface to WorldMap for scripts, to obtain objects, get players, etc.
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL MapScriptInterface
{
public:
    MapScriptInterface(WorldMap& mgr);
    ~MapScriptInterface();

    template<class T, uint32_t TypeId> T* getObjectNearestCoords(uint32_t Entry, float x, float y, float z = 0.0f)
    {
        MapCell* pCell = m_worldMap.getCell(m_worldMap.getPosX(x), m_worldMap.getPosY(y));
        if (pCell == nullptr)
            return 0;

        T* ClosestObject = nullptr;
        float ClosestDist = 999999.0f;
        float CurrentDist = 0;

        ObjectSet::const_iterator iter = pCell->Begin();
        for (; iter != pCell->End(); ++iter)
        {
            CurrentDist = (*iter)->CalcDistance(x, y, (z != 0.0f ? z : (*iter)->GetPositionZ()));
            if (CurrentDist < ClosestDist && (*iter)->getObjectTypeId() == TypeId)
            {
                if ((Entry && (*iter)->getEntry() == Entry) || !Entry)
                {
                    ClosestDist = CurrentDist;
                    ClosestObject = ((T*)(*iter));
                }
            }
        }

        return ClosestObject;
    }

    inline GameObject* getGameObjectNearestCoords(float x, float y, float z = 0.0f, uint32_t Entry = 0)
    {
        return getObjectNearestCoords<GameObject, TYPEID_GAMEOBJECT>(Entry, x, y, z);
    }

    inline Creature* getCreatureNearestCoords(float x, float y, float z = 0.0f, uint32_t Entry = 0)
    {
        return getObjectNearestCoords<Creature, TYPEID_UNIT>(Entry, x, y, z);
    }

    inline GameObject* findNearestGoWithType(Object* o, uint32_t type)
    {
        GameObject* go = nullptr;
        float r = FLT_MAX;

        for (const auto& itr : o->getInRangeObjectsSet())
        {
            Object* iro = itr;
            if (!iro || !iro->isGameObject())
                continue;

            GameObject* irgo = static_cast<GameObject*>(iro);

            if (irgo->getGoType() != type)
                continue;

            if ((irgo->GetPhase() & o->GetPhase()) == 0)
                continue;

            float range = o->getDistanceSq(iro);

            if (range < r)
            {
                r = range;
                go = irgo;
            }
        }

        return go;
    }

    inline Creature* findNearestCreature(Object* pObject, uint32_t entry, float maxSearchRange /*= 250.0f*/) const
    {
        MapCell* pCell = m_worldMap.getCell(m_worldMap.getPosX(pObject->GetPositionX()), m_worldMap.getPosY(pObject->GetPositionY()));
        if (pCell == nullptr)
            return nullptr;

        float CurrentDist = 0;
        float r = FLT_MAX;
        Creature* target = nullptr;

        ObjectSet::const_iterator iter = pCell->Begin();
        for (; iter != pCell->End(); ++iter)
        {
            if ((*iter)->isCreature() && (*iter)->getEntry() == entry)
            {
                CurrentDist = (*iter)->CalcDistance(pObject);
                if (CurrentDist <= maxSearchRange)
                {
                    if (CurrentDist < r)
                    {
                        r = CurrentDist;
                        target = static_cast<Creature*>((*iter));
                    }
                }
            }
        }
        return target;
    }

    inline void getCreatureListWithEntryInRange(Creature* pCreature, std::list<Creature*>& container, uint32_t entry, float maxSearchRange /*= 250.0f*/) const
    {
        float CurrentDist = 0;

        for (auto const& target : m_worldMap.activeCreatures)
        {
            if (target->isCreature() && target->getEntry() == entry)
            {
                CurrentDist = target->CalcDistance(pCreature);
                if (CurrentDist <= maxSearchRange)
                    container.push_back(target);
            }
        }
    }

    inline Creature* getNearestAssistCreatureInCell(Creature* pCreature, Unit* enemy, float range /*= 250.0f*/) const
    {
        MapCell* pCell = m_worldMap.getCell(m_worldMap.getPosX(pCreature->GetPositionX()), m_worldMap.getPosY(pCreature->GetPositionY()));
        if (pCell == nullptr)
            return nullptr;

        float CurrentDist = 0;
        ObjectSet::const_iterator iter = pCell->Begin();
        for (; iter != pCell->End(); ++iter)
        {
            if ((*iter)->isCreature())
            {
                Creature* helper = (*iter)->ToCreature();
                if (pCreature != helper)
                {
                    CurrentDist = (*iter)->CalcDistance(pCreature);
                    if (CurrentDist <= range)
                    {
                        if (helper->getAIInterface()->canAssistTo(pCreature, enemy))
                        {
                            return helper;
                        }
                    }
                }
            }
        }
        return nullptr;
    }

    inline void getGameObjectListWithEntryInRange(Creature* pCreature, std::list<GameObject*>& container, uint32_t entry, float maxSearchRange /*= 250.0f*/) const
    {
        float CurrentDist = 0;

        for (auto const& target : m_worldMap.activeGameObjects)
        {
            if (target->isGameObject() && target->getEntry() == entry)
            {
                CurrentDist = target->CalcDistance(pCreature);
                if (CurrentDist <= maxSearchRange)
                    container.push_back(target);
            }
        }
    }

    inline GameObject* findNearestGameObject(Object* pObject, uint32_t entry, float maxSearchRange /*= 250.0f*/) const
    {
        MapCell* pCell = m_worldMap.getCell(m_worldMap.getPosX(pObject->GetPositionX()), m_worldMap.getPosY(pObject->GetPositionY()));
        if (pCell == nullptr)
            return nullptr;

        float CurrentDist = 0;
        float r = FLT_MAX;
        GameObject* target = nullptr;

        ObjectSet::const_iterator iter = pCell->Begin();
        for (; iter != pCell->End(); ++iter)
        {
            if ((*iter)->isGameObject() && (*iter)->getEntry() == entry)
            {
                CurrentDist = (*iter)->CalcDistance(pObject);
                if (CurrentDist <= maxSearchRange)
                {
                    if (CurrentDist < r)
                    {
                        r = CurrentDist;
                        target = static_cast<GameObject*>((*iter));
                    }
                }
            }
        }
        return target;
    }

    inline Player* getPlayerNearestCoords(float x, float y, float z = 0.0f, uint32_t Entry = 0)
    {
        return getObjectNearestCoords<Player, TYPEID_PLAYER>(Entry, x, y, z);
    }

    uint32_t getPlayerCountInRadius(float x, float y, float z = 0.0f, float radius = 5.0f);

    GameObject* spawnGameObject(uint32_t Entry, LocationVector pos, bool AddToWorld, uint32_t Misc1, uint32_t Misc2, uint32_t phase = 0xFFFFFFF);
    GameObject* spawnGameObject(MySQLStructure::GameobjectSpawn* gs, bool AddToWorld);
    Creature* spawnCreature(uint32_t Entry, LocationVector pos, bool AddToWorld, bool tmplate, uint32_t Misc1, uint32_t Misc2, uint32_t phase = 0xFFFFFFF);
    Creature* spawnCreature(MySQLStructure::CreatureSpawn* sp, bool AddToWorld);

    void deleteGameObject(GameObject* ptr);
    void deleteCreature(Creature* ptr);

private:
    WorldMap& m_worldMap;
};
