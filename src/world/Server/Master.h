/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include "Config/Config.h"
#include "Database/DatabaseEnv.h"
#include "MainServerDefines.h"
#include "../shared/AscemuServerDefines.hpp"

class Master : public Singleton<Master>
{
    public:

        Master();
        ~Master();

        bool Run(int argc, char** argv);
        void PrintBanner();
        bool LoadWorldConfiguration(char* config_file);
        void OpenCheatLogFiles();
        void StartNetworkSubsystem();
        void StartRemoteConsole();
        void WritePidFile();

        void ShutdownThreadPools(bool listnersockcreate);
        void ShutdownLootSystem();
        bool m_ShutdownEvent;
        uint32_t m_ShutdownTimer;

        static volatile bool m_stopEvent;
        bool m_restartEvent;

    private:

        bool _StartDB();
        void _StopDB();
        bool _CheckDBVersion();

        void _HookSignals();
        void _UnhookSignals();

        static void _OnSignal(int s);
};

#define sMaster Master::getSingleton()
