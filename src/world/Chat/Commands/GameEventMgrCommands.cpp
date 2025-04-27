/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatHandler.hpp"
#include "Management/GameEvent.hpp"
#include "Management/GameEventMgr.hpp"
#include "Utilities/Util.hpp"

//.event list
bool ChatHandler::HandleEventListEvents(const char* /*args*/, WorldSession* m_session)
{
    SystemMessage(m_session, "--- Current Active Events ---");
    for (const auto& gameEventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = gameEventPair.second.get();
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
    uint32_t eventid = atoi(args);
    if (eventid == 0)
    {
        SystemMessage(m_session, "Invalid argument: %s", args);
        return false;
    }

    for (const auto& eventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = eventPair.second.get();
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
    uint32_t eventid = atoi(args);
    if (eventid == 0)
    {
        SystemMessage(m_session, "Invalid argument: %s", args);
        return false;
    }

    for (const auto& eventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = eventPair.second.get();
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
    uint32_t eventid = atoi(args);
    if (eventid == 0)
    {
        SystemMessage(m_session, "Invalid argument: %s", args);
        return false;
    }

    for (const auto& eventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = eventPair.second.get();
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
    auto startTime = Util::TimeNow();

    // Stop all events, then clear storage
    for (const auto& eventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = eventPair.second.get();
        gameEvent->StopEvent();
    }

    // Clear game events
    sGameEventMgr.mGameEvents.clear();

    // Reload events from DB
    sGameEventMgr.LoadFromDB();

    SystemMessage(m_session, "Reloaded all game events in %u ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}
