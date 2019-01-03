/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logon.h"

initialiseSingleton(Logon);

Logon::Logon()
{
    
}

Logon::~Logon()
{}

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
