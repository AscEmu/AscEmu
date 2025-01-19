/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LogonConfig.h"
#include "PerformanceCounter.hpp"

class SERVER_DECL Logon
{
    private:
        Logon() = default;
        ~Logon() = default;

    public:
        static Logon& getInstance();

        Logon(Logon&&) = delete;
        Logon(Logon const&) = delete;
        Logon& operator=(Logon&&) = delete;
        Logon& operator=(Logon const&) = delete;

    //////////////////////////////////////////////////////////////////////////////////////////
    // LogonConfig
    public:
        LogonConfig settings;

        void loadLogonConfigValues(bool reload = false);

    //////////////////////////////////////////////////////////////////////////////////////////
    // InfoCore
    private:
        Ascemu::PerformanceCounter perfcounter;

    public:
        float getCPUUsage();
        float getRAMUsage();
};

#define sLogon Logon::getInstance()
#define logonConfig sLogon.settings
