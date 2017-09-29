/*
 * Moon++ Scripts for Ascent MMORPG Server
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Server/MainServerDefines.h"
#include "Map/WorldCreator.h"

MoonInstanceScript::MoonInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
{
    mUpdateFrequency = DEFAULT_UPDATE_FREQUENCY;
    mTimerIdCounter = 0;
    mSpawnsCreated = false;
};

MoonInstanceScript::~MoonInstanceScript()
{
};


Creature* MoonInstanceScript::GetCreatureByGuid(uint32 pGuid)
{
    if (pGuid == 0)
        return NULL;

    return mInstance->GetCreature(pGuid);
};

Creature* MoonInstanceScript::SpawnCreature(uint32 pEntry, float pX, float pY, float pZ, float pO, uint32 pFactionId)
{
    Creature* NewCreature = mInstance->GetInterface()->SpawnCreature(pEntry, pX, pY, pZ, pO, true, true, 0, 0);
    if (NewCreature != nullptr)
        NewCreature->SetFaction(pFactionId);

    return NewCreature;
};

Creature* MoonInstanceScript::PushCreature(uint32 pEntry, float pX, float pY, float pZ, float pO, uint32 pFaction)
{
    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(pEntry);
    if (cp == nullptr)
    {
        LOG_ERROR("PushCreature: tried to push a invalid creature with entry %u!", pEntry);
        return nullptr;
    }

    Creature* c = mInstance->CreateCreature(pEntry);

    Arcemu::Util::ArcemuAssert(c != NULL);

    c->Load(cp, pX, pY, pZ, pO);

    if (pFaction != 0)
        c->SetFaction(pFaction);

    c->PushToWorld(mInstance);
    return c;
}



GameObject* MoonInstanceScript::FindClosestGameObjectOnMap(uint32 pEntry, float pX, float pY, float pZ)
{
    GameObjectSet GameObjects = FindGameObjectsOnMap(pEntry);

    if (GameObjects.size() == 0)
        return NULL;
    if (GameObjects.size() == 1)
        return *(GameObjects.begin());

    GameObject* NearestObject = NULL;
    float Distance, NearestDistance = 99999;
    for (GameObjectSet::iterator Iter = GameObjects.begin(); Iter != GameObjects.end(); ++Iter)
    {
        Distance = GetRangeToObject(*Iter, pX, pY, pZ);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestObject = (*Iter);
        };
    };

    return NearestObject;
};

GameObject* MoonInstanceScript::SpawnGameObject(uint32 pEntry, float pX, float pY, float pZ, float pO)
{
    GameObject* pNewGO = mInstance->GetInterface()->SpawnGameObject(pEntry, pX, pY, pZ, pO, true, 0, 0);
    return pNewGO;
};

GameObjectSet MoonInstanceScript::FindGameObjectsOnMap(uint32 pEntry)
{
    GameObject* CurrentObject = NULL;
    GameObjectSet ReturnSet;
    for (std::vector< GameObject* >::iterator GOIter = mInstance->GOStorage.begin(); GOIter != mInstance->GOStorage.end(); ++GOIter)
    {
        CurrentObject = (*GOIter);
        if (CurrentObject != NULL)
        {
            if (CurrentObject->GetEntry() == pEntry)
                ReturnSet.insert(CurrentObject);
        };
    };

    return ReturnSet;
};

GameObject* MoonInstanceScript::GetGameObjectByGuid(uint32 pGuid)
{
    if (pGuid == 0)
        return NULL;

    return mInstance->GetGameObject(pGuid);
};

void MoonInstanceScript::AddGameObjectStateByEntry(uint32 pEntry, GameObjectState pState, bool pUseQuery)
{
    if (pEntry == 0)
        return;

    GameObjectEntryMap::iterator Iter = mGameObjects.find(pEntry);
    if (Iter != mGameObjects.end())
        (*Iter).second = pState;
    else
        mGameObjects.insert(GameObjectEntryMap::value_type(pEntry, pState));

    GameObject* CurrentObject = NULL;
    if (!pUseQuery)
    {
        for (std::vector< GameObject* >::iterator GOIter = mInstance->GOStorage.begin(); GOIter != mInstance->GOStorage.end(); ++GOIter)
        {
            CurrentObject = (*GOIter);
            if (CurrentObject != NULL)
            {
                if (CurrentObject->GetEntry() == pEntry)
                    CurrentObject->SetState(pState);
            };
        };
    }
    else
    {
        QueryResult* Result = WorldDatabase.Query("SELECT id FROM gameobject_spawns WHERE entry = %u", pEntry);
        if (Result != NULL)
        {
            do
            {
                CurrentObject = getGameObjectBySpawnId(Result->Fetch()[0].GetUInt32());
                if (CurrentObject != NULL)
                    CurrentObject->SetState(pState);
            }
            while (Result->NextRow());

            delete Result;
        };
    };
};

float MoonInstanceScript::GetRangeToObject(Object* pObject, float pX, float pY, float pZ)
{
    if (pObject == NULL)
        return 0.0f;

    LocationVector pos = pObject->GetPosition();
    float dX = pos.x - pX;
    float dY = pos.y - pY;
    float dZ = pos.z - pZ;

    return sqrtf(dX * dX + dY * dY + dZ * dZ);
};

int32 MoonInstanceScript::AddTimer(int32 pDurationMillisec)
{
    int32 Index = mTimerIdCounter++;
    mTimers.push_back(std::make_pair(Index, pDurationMillisec));
    return Index;
}

int32 MoonInstanceScript::GetTimer(int32 pTimerId)
{
    for (TimerArray::iterator TimerIter = mTimers.begin(); TimerIter != mTimers.end(); ++TimerIter)
    {
        if (TimerIter->first == pTimerId)
            return TimerIter->second;
    };

    return 0;
};

void MoonInstanceScript::RemoveTimer(int32 & pTimerId)
{
    for (TimerArray::iterator TimerIter = mTimers.begin(); TimerIter != mTimers.end(); ++TimerIter)
    {
        if (TimerIter->first == pTimerId)
        {
            mTimers.erase(TimerIter);
            pTimerId = INVALIDATE_TIMER;
            break;
        };
    };
};

void MoonInstanceScript::ResetTimer(int32 pTimerId, int32 pDurationMillisec)
{
    for (TimerArray::iterator TimerIter = mTimers.begin(); TimerIter != mTimers.end(); ++TimerIter)
    {
        if (TimerIter->first == pTimerId)
        {
            TimerIter->second = pDurationMillisec;
            break;
        };
    };
};

bool MoonInstanceScript::IsTimerFinished(int32 pTimerId)
{
    for (TimerArray::iterator TimerIter = mTimers.begin(); TimerIter != mTimers.end(); ++TimerIter)
    {
        if (TimerIter->first == pTimerId)
            return (TimerIter->second <= 0) ? true : false;
    };

    return false;
};

void MoonInstanceScript::CancelAllTimers()
{
    mTimers.clear();
    mTimerIdCounter = 0;
};

void MoonInstanceScript::SetCellForcedStates(float pMinX, float pMaxX, float pMinY, float pMaxY, bool pActivate)
{
    if (pMinX == pMaxX || pMinY == pMaxY)
        return;

    float Y = pMinY;
    while (pMinX < pMaxX)
    {
        while (pMinY < pMaxY)
        {
            MapCell* CurrentCell = mInstance->GetCellByCoords(pMinX, pMinY);
            if (pActivate && CurrentCell == NULL)
            {
                CurrentCell = mInstance->CreateByCoords(pMinX, pMinY);
                if (CurrentCell != NULL)
                    CurrentCell->Init(mInstance->GetPosX(pMinX), mInstance->GetPosY(pMinY), mInstance);
            };

            if (CurrentCell != NULL)
            {
                if (pActivate)
                    mInstance->AddForcedCell(CurrentCell);
                else
                    mInstance->RemoveForcedCell(CurrentCell);
            };

            pMinY += 40.0f;
        };

        pMinY = Y;
        pMinX += 40.0f;
    };
};

void MoonInstanceScript::UpdateEvent()
{
    //uint32 CurrentTime = static_cast< uint32 >(time(NULL));
    for (TimerArray::iterator TimerIter = mTimers.begin(); TimerIter != mTimers.end(); ++TimerIter)
    {
        TimerIter->second -= mUpdateFrequency;
    };
};

void MoonInstanceScript::Destroy()
{
    delete this;
};

void MoonInstanceScript::BuildEncounterMap()
{
    if (mInstance->pInstance == NULL)
        return;

    QueryResult* KillResult = WorldDatabase.Query("SELECT id, entry FROM creature_spawns WHERE map = %u AND entry IN (SELECT entry FROM creature_properties WHERE encounter = 1)", mInstance->GetMapId());
    if (KillResult != NULL)
    {
        uint32 Id = 0, Entry = 0;
        Field* CurrentField = NULL;
        EncounterMap::iterator EncounterIter;
        EncounterState State = State_NotStarted;
        bool StartedInstance = mInstance->pInstance->m_killedNpcs.size() > 0;
        do
        {
            CurrentField = KillResult->Fetch();
            Id = CurrentField[0].GetUInt32();
            Entry = CurrentField[1].GetUInt32();

            EncounterIter = mEncounters.find(Entry);
            if (EncounterIter != mEncounters.end())
                continue;

            if (StartedInstance)
            {
                if (mInstance->pInstance->m_killedNpcs.find(Entry) != mInstance->pInstance->m_killedNpcs.end())
                    State = State_Finished;
                else
                    State = State_NotStarted;
            };

            mEncounters.insert(EncounterMap::value_type(Entry, BossData(Id, 0, State)));
        }
        while (KillResult->NextRow());

        delete KillResult;
    };
};
