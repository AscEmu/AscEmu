/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LogonConfig.h"
#include "Server/LogonServerDefines.hpp"
#include "LogonConf.h"
#include "Logging/Logger.hpp"

LogonConfig::LogonConfig()
{
    // logon.conf - LogonDatabase
    logonDb.port = 3306;

    // logon.conf - Listen
    listen.port = 8093;

    // logon.conf - Logger
    logger.minimumMessageType = 2;

    // logon.conf - Rates
    rates.accountRefreshTime = 600;
}

void LogonConfig::loadConfigValues(bool reload /*false*/)
{
    if (reload)
    {
        if (Config.MainConfig.openAndLoadConfigFile(CONFDIR "/logon.conf"))
        {
            sLogger.info("Config : " CONFDIR "/logon.conf reloaded");
        }
        else
        {
            sLogger.failure("Config : error occurred loading " CONFDIR "/logon.conf");
            return;
        }
    }

    // logon.conf - LogonDatabase
    ASSERT(Config.MainConfig.tryGetString("LogonDatabase", "Hostname", &logonDb.host));
    ASSERT(Config.MainConfig.tryGetString("LogonDatabase", "Username", &logonDb.user));
    ASSERT(Config.MainConfig.tryGetString("LogonDatabase", "Name", &logonDb.db));
    ASSERT(Config.MainConfig.tryGetString("LogonDatabase", "Password", &logonDb.password));
    ASSERT(Config.MainConfig.tryGetInt("LogonDatabase", "Port", &logonDb.port));
    ASSERT(Config.MainConfig.tryGetInt("LogonDatabase", "Connections", &logonDb.connections));

    // logon.conf - Listen
    ASSERT(Config.MainConfig.tryGetString("Listen", "Host", &listen.host));
    ASSERT(Config.MainConfig.tryGetString("Listen", "ISHost", &listen.interServerHost));
    ASSERT(Config.MainConfig.tryGetInt("Listen", "RealmListPort", &listen.realmListPort));
    ASSERT(Config.MainConfig.tryGetInt("Listen", "ServerPort", &listen.port));

    // logon.conf - Logger Settings
    ASSERT(Config.MainConfig.tryGetInt("Logger", "MinimumMessageType", &logger.minimumMessageType));

    // logon.conf - Rates
    ASSERT(Config.MainConfig.tryGetInt("Rates", "AccountRefresh", &rates.accountRefreshTime));

    // logon.conf - LogonServer
    ASSERT(Config.MainConfig.tryGetBool("LogonServer", "DisablePings", &logonServer.disablePings));
    ASSERT(Config.MainConfig.tryGetString("LogonServer", "AllowedIPs", &logonServer.allowedIps));
    ASSERT(Config.MainConfig.tryGetString("LogonServer", "AllowedModIPs", &logonServer.allowedModIps));
}
