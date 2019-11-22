/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include "Config/Config.h"
#include "Database/DatabaseEnv.h"
#include "MainServerDefines.h"
#include "../shared/AscemuServerDefines.hpp"
#include <iostream>

class SERVER_DECL Master
{
    private:

        Master() = default;
        ~Master() = default;

    public:

        static Master& getInstance();
        void initialize();

        Master(Master&&) = delete;
        Master(Master const&) = delete;
        Master& operator=(Master&&) = delete;
        Master& operator=(Master const&) = delete;

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

        //lib Log
        void libLog(const char* format, ...)
        {
            char message_buffer[32768];
            va_list ap;

            va_start(ap, format);
            vsnprintf(message_buffer, 32768, format, ap);
            va_end(ap);

            std::cout << message_buffer << std::endl;
        }

    private:

        bool _StartDB();
        void _StopDB();
        bool _CheckDBVersion();

        void _HookSignals();
        void _UnhookSignals();

        static void _OnSignal(int s);
};

#define sMaster Master::getInstance()

#define DLLLogDetail(msg, ...) sMaster.libLog(msg, ##__VA_ARGS__)
