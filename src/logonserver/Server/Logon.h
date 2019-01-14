/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LogonConfig.h"
#include "Singleton.h"
#include "PerformanceCounter.hpp"

class SERVER_DECL Logon : public Singleton<Logon>
{
    public:
        Logon();
        ~Logon();

    //////////////////////////////////////////////////////////////////////////////////////////
    // LogonConfig
    public:
        LogonConfig settings;

        void loadLogonConfigValues(bool reload = false);

    //////////////////////////////////////////////////////////////////////////////////////////
    // InfoCore
    private:
        Arcemu::PerformanceCounter perfcounter;

    public:
        float getCPUUsage();
        float getRAMUsage();
};

#define sLogon Logon::getSingleton()
#define logonConfig sLogon.settings
