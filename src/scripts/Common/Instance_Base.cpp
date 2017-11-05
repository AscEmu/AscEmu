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

GameObject* MoonInstanceScript::GetGameObjectByGuid(uint32 pGuid)
{
    if (pGuid == 0)
        return NULL;

    return mInstance->GetGameObject(pGuid);
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
