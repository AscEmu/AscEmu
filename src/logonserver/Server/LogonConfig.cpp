/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LogonConfig.h"
#include "Server/LogonServerDefines.hpp"
#include "LogonConf.h"
#include "Log.hpp"

LogonConfig::LogonConfig()
{
    // logon.conf - LogonDatabase
    logonDb.port = 3306;

    // logon.conf - Listen
    listen.port = 8093;

    // logon.conf - LogLevel
    logLevel.file = 0;

    // logon.conf - Rates
    rates.accountRefreshTime = 600;
}

void LogonConfig::loadConfigValues(bool reload /*false*/)
{
    if (reload)
    {
        if (Config.MainConfig.openAndLoadConfigFile(CONFDIR "/logon.conf"))
        {
            LogDetail("Config : " CONFDIR "/logon.conf reloaded");
        }
        else
        {
            LogError("Config : error occurred loading " CONFDIR "/logon.conf");
            return;
        }
    }

    // logon.conf - LogonDatabase
    logonDb.host = Config.MainConfig.getStringDefault("LogonDatabase", "Hostname", "");
    logonDb.user = Config.MainConfig.getStringDefault("LogonDatabase", "Username", "");
    logonDb.db = Config.MainConfig.getStringDefault("LogonDatabase", "Name", "");
    logonDb.password = Config.MainConfig.getStringDefault("LogonDatabase", "Password", "");
    logonDb.port = Config.MainConfig.getIntDefault("LogonDatabase", "Port", 3306);
    logonDb.connections = Config.MainConfig.getIntDefault("LogonDatabase", "ConnectionCount", 5);

    // logon.conf - Listen
    listen.host = Config.MainConfig.getStringDefault("Listen", "Host", "0.0.0.0");
    listen.interServerHost = Config.MainConfig.getStringDefault("Listen", "ISHost", "0.0.0.0");
    listen.realmListPort = Config.MainConfig.getIntDefault("Listen", "RealmListPort", 3724);
    listen.port = Config.MainConfig.getIntDefault("Listen", "ServerPort", 8093);

    // logon.conf - LogLevel
    logLevel.file = Config.MainConfig.getIntDefault("LogLevel", "File", 0);

    // logon.conf - Rates
    rates.accountRefreshTime = Config.MainConfig.getIntDefault("Rates", "AccountRefresh", 600);

    // logon.conf - LogonServer
    logonServer.remotePassword = Config.MainConfig.getStringDefault("LogonServer", "RemotePassword", "");
    logonServer.allowedIps = Config.MainConfig.getStringDefault("LogonServer", "AllowedIPs", "127.0.0.1/24");
    logonServer.allowedModIps = Config.MainConfig.getStringDefault("LogonServer", "AllowedModIPs", "127.0.0.1/24");
}
