/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LogonConfig.h"
#include "Server/LogonServerDefines.hpp"
#include "LogonConf.hpp"
#include "Logging/Logger.hpp"

LogonConfig::LogonConfig()
{
    // logon.conf - LogonDatabase
    logonDb.port = 3306;
    logonDb.isLegacyAuth = false;

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
    Config.MainConfig.tryGetString("LogonDatabase", "Hostname", &logonDb.host);
    Config.MainConfig.tryGetString("LogonDatabase", "Username", &logonDb.user);
    Config.MainConfig.tryGetString("LogonDatabase", "Name", &logonDb.db);
    Config.MainConfig.tryGetString("LogonDatabase", "Password", &logonDb.password);
    Config.MainConfig.tryGetInt("LogonDatabase", "Port", &logonDb.port);
    Config.MainConfig.tryGetInt("LogonDatabase", "Connections", &logonDb.connections);
    Config.MainConfig.tryGetBool("LogonDatabase", "LegacyAuth", &logonDb.isLegacyAuth);

    // logon.conf - Listen
    Config.MainConfig.tryGetString("Listen", "Host", &listen.host);
    Config.MainConfig.tryGetString("Listen", "ISHost", &listen.interServerHost);
    Config.MainConfig.tryGetInt("Listen", "RealmListPort", &listen.realmListPort);
    Config.MainConfig.tryGetInt("Listen", "ServerPort", &listen.port);

    // logon.conf - Logger Settings
    Config.MainConfig.tryGetInt("Logger", "MinimumMessageType", &logger.minimumMessageType);

    // logon.conf - Rates
    Config.MainConfig.tryGetInt("Rates", "AccountRefresh", &rates.accountRefreshTime);

    // logon.conf - LogonServer
    Config.MainConfig.tryGetBool("LogonServer", "DisablePings", &logonServer.disablePings);
    Config.MainConfig.tryGetString("LogonServer", "AllowedIPs", &logonServer.allowedIps);
    Config.MainConfig.tryGetString("LogonServer", "AllowedModIPs", &logonServer.allowedModIps);
}
