/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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