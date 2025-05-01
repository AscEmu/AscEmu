/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "GameEventDefines.hpp"
#include "Threading/AEThread.h"

class GameEventMgr
{
    GameEventMgr() = default;
    ~GameEventMgr() = default;

public:
    class GameEventMgrThread
    {
    private:
        GameEventMgrThread() = default;
        ~GameEventMgrThread() = default;

    public:
        static GameEventMgrThread& getInstance();
        void initialize();
        void finalize();

        GameEventMgrThread(GameEventMgrThread&&) = delete;
        GameEventMgrThread(GameEventMgrThread const&) = delete;
        GameEventMgrThread& operator=(GameEventMgrThread&&) = delete;
        GameEventMgrThread& operator=(GameEventMgrThread const&) = delete;

        bool m_IsActive = false;

        void Update();

        std::unique_ptr<AscEmu::Threading::AEThread> m_reloadThread;
        uint32_t m_reloadTime;
    };

    static GameEventMgr& getInstance();
    void initialize();

    GameEventMgr(GameEventMgr&&) = delete;
    GameEventMgr(GameEventMgr const&) = delete;
    GameEventMgr& operator=(GameEventMgr&&) = delete;
    GameEventMgr& operator=(GameEventMgr const&) = delete;

    ActiveEvents const& GetActiveEventList() const { return mActiveEvents; }
    void StartArenaEvents();
    void LoadFromDB();
    GameEvent* GetEventById(uint32_t pEventId);

    GameEvents mGameEvents;
    ActiveEvents mActiveEvents;
    NPCGuidList mNPCGuidList;
    GOBGuidList mGOBGuidList;
};

#define sGameEventMgr GameEventMgr::getInstance()
#define sGameEventMgrThread GameEventMgr::GameEventMgrThread::getInstance()
