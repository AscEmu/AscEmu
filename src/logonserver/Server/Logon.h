/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LogonConfig.h"
#include "Singleton.h"

class SERVER_DECL Logon : public Singleton<Logon>
{
    public:
        Logon();
        ~Logon();

    private:
    //////////////////////////////////////////////////////////////////////////////////////////
    // LogonConfig
    public:
        LogonConfig settings;

        void loadLogonConfigValues(bool reload = false);
};

#define sLogon Logon::getSingleton()
#define logonConfig sLogon.settings
