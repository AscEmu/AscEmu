/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/GameEvent.h"
#include "Chat/ChatHandler.hpp"

//.event list
bool ChatHandler::HandleEventListEvents(const char* /*args*/, WorldSession* m_session)
{
    SystemMessage(m_session, "--- Current Active Events ---");
    auto eventList = sGameEventMgr.mGameEvents;
    for (auto gameEventPair : eventList)
    {
        auto gameEvent = gameEventPair.second;
        if (gameEvent->GetState() == GAMEEVENT_ACTIVE)
        {
            SystemMessage(m_session, "%u - %s", gameEvent->event_id, gameEvent->description.c_str());
        }
        else if (gameEvent->GetState() == GAMEEVENT_ACTIVE_FORCED)
        {
            SystemMessage(m_session, "[Force Activated] %u - %s", gameEvent->event_id, gameEvent->description.c_str());
        }
    }
    SystemMessage(m_session, "--- End List ---");
    return true;
}

//.event start
bool ChatHandler::HandleEventStartEvent(const char* args, WorldSession* m_session)
{
    uint32 eventid = atoi(args);
    if (eventid == 0)
    {
        SystemMessage(m_session, "Invalid argument: %s", args);
        return false;
    }

    for (auto eventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = eventPair.second;
        if (gameEvent->event_id == eventid)
        {
            SystemMessage(m_session, "Force starting event %u (%s)", gameEvent->event_id, gameEvent->description.c_str());
            gameEvent->StartEvent(true);
            return true;
        }
    }

    SystemMessage(m_session, "Game event with ID %u not found", eventid);
    return false;
}

//.event stop
bool ChatHandler::HandleEventStopEvent(const char* args, WorldSession* m_session)
{
    uint32 eventid = atoi(args);
    if (eventid == 0)
    {
        SystemMessage(m_session, "Invalid argument: %s", args);
        return false;
    }

    for (auto eventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = eventPair.second;
        if (gameEvent->event_id == eventid)
        {
            SystemMessage(m_session, "Force stopping event %u (%s)", gameEvent->event_id, gameEvent->description.c_str());
            gameEvent->StopEvent(true);
            return true;
        }
    }

    SystemMessage(m_session, "Game event with ID %u not found", eventid);
    return false;
}

//.event reset
bool ChatHandler::HandleEventResetEvent(const char* args, WorldSession* m_session)
{
    uint32 eventid = atoi(args);
    if (eventid == 0)
    {
        SystemMessage(m_session, "Invalid argument: %s", args);
        return false;
    }

    for (auto eventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = eventPair.second;
        if (gameEvent->event_id == eventid)
        {
            SystemMessage(m_session, "Clearing flags for event %u (%s)", gameEvent->event_id, gameEvent->description.c_str());
            if (gameEvent->GetState() == GAMEEVENT_ACTIVE_FORCED)
                gameEvent->SetState(GAMEEVENT_ACTIVE);
            else
                gameEvent->SetState(GAMEEVENT_INACTIVE);
            return true;
        }
    }

    SystemMessage(m_session, "Game event with ID %u not found", eventid);
    return false;
}

//.event reload
bool ChatHandler::HandleEventReloadAllEvents(const char* /*args*/, WorldSession* m_session)
{
    SystemMessage(m_session, "Beginning reload of all game events");
    auto start = time(0);
    // Wait for GameEventMgr thread to stop
    if (!sGameEventMgrThread.Pause())
        return false;

    // Stop all events, then clear storage
    for (auto eventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = eventPair.second;
        gameEvent->StopEvent();
        delete gameEvent;
    }

    // Clear game events
    sGameEventMgr.mGameEvents.clear();

    // Reload events from DB
    sGameEventMgr.LoadFromDB();

    // Restart GameEventMgr thread
    sGameEventMgrThread.Resume();

    SystemMessage(m_session, "Reloaded all game events in %u ms", time(0)-start);
    return true;
}
