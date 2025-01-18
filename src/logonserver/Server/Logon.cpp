/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logon.h"

Logon& Logon::getInstance()
{
    static Logon mInstance;
    return mInstance;
}

void Logon::loadLogonConfigValues(bool reload /*false*/)
{
    settings.loadConfigValues(reload);
}

float Logon::getCPUUsage()
{
    return perfcounter.GetCurrentCPUUsage();
}

float Logon::getRAMUsage()
{
    return perfcounter.GetCurrentRAMUsage();
}
