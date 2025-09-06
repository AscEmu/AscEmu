/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatCommandHandler.hpp"
#include "Management/GameEvent.hpp"
#include "Management/GameEventMgr.hpp"
#include "Utilities/Util.hpp"

//.event list
bool ChatCommandHandler::HandleEventListEvents(const char* /*args*/, WorldSession* m_session)
{
    systemMessage(m_session, "--- Current Active Events ---");
    for (const auto& gameEventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = gameEventPair.second.get();
        if (gameEvent->GetState() == GAMEEVENT_ACTIVE)
        {
            systemMessage(m_session, "{} - {}", gameEvent->event_id, gameEvent->description);
        }
        else if (gameEvent->GetState() == GAMEEVENT_ACTIVE_FORCED)
        {
            systemMessage(m_session, "[Force Activated] {} - {}", gameEvent->event_id, gameEvent->description);
        }
    }
    systemMessage(m_session, "--- End List ---");
    return true;
}

//.event start
bool ChatCommandHandler::HandleEventStartEvent(const char* args, WorldSession* m_session)
{
    uint32_t eventid = atoi(args);
    if (eventid == 0)
    {
        systemMessage(m_session, "Invalid argument: {}", args);
        return false;
    }

    for (const auto& eventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = eventPair.second.get();
        if (gameEvent->event_id == eventid)
        {
            systemMessage(m_session, "Force starting event {} ({})", gameEvent->event_id, gameEvent->description);
            gameEvent->StartEvent(true);
            return true;
        }
    }

    systemMessage(m_session, "Game event with ID {} not found", eventid);
    return false;
}

//.event stop
bool ChatCommandHandler::HandleEventStopEvent(const char* args, WorldSession* m_session)
{
    uint32_t eventid = atoi(args);
    if (eventid == 0)
    {
        systemMessage(m_session, "Invalid argument: {}", args);
        return false;
    }

    for (const auto& eventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = eventPair.second.get();
        if (gameEvent->event_id == eventid)
        {
            systemMessage(m_session, "Force stopping event {} ({})", gameEvent->event_id, gameEvent->description);
            gameEvent->StopEvent(true);
            return true;
        }
    }

    systemMessage(m_session, "Game event with ID {} not found", eventid);
    return false;
}

//.event reset
bool ChatCommandHandler::HandleEventResetEvent(const char* args, WorldSession* m_session)
{
    uint32_t eventid = atoi(args);
    if (eventid == 0)
    {
        systemMessage(m_session, "Invalid argument: {}", args);
        return false;
    }

    for (const auto& eventPair : sGameEventMgr.mGameEvents)
    {
        auto gameEvent = eventPair.second.get();
        if (gameEvent->event_id == eventid)
        {
            systemMessage(m_session, "Clearing flags for event {} ({})", gameEvent->event_id, gameEvent->description);
            if (gameEvent->GetState() == GAMEEVENT_ACTIVE_FORCED)
                gameEvent->SetState(GAMEEVENT_ACTIVE);
            else
                gameEvent->SetState(GAMEEVENT_INACTIVE);
            return true;
        }
    }

    systemMessage(m_session, "Game event with ID {} not found", eventid);
    return false;
}

//.event reload
bool ChatCommandHandler::HandleEventReloadAllEvents(const char* /*args*/, WorldSession* m_session)
{
    systemMessage(m_session, "Beginning reload of all game events");
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

    systemMessage(m_session, "Reloaded all game events in {} ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}
