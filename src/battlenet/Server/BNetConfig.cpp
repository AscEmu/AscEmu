/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BNetConfig.hpp"
#include "BNetServerDefines.hpp"

namespace AscEmu::Battlenet
{
    void BNetConfig::load()
    {
        configManager.mainConfig.tryGetString("Listen", "Host", &listen.host);
        configManager.mainConfig.tryGetInt("Listen", "Port", &listen.port);

        configManager.mainConfig.tryGetString("TLS", "CertificatesFile", &tls.certificatesFile);
        configManager.mainConfig.tryGetString("TLS", "PrivateKeyFile", &tls.privateKeyFile);
        configManager.mainConfig.tryGetString("TLS", "PrivateKeyPassword", &tls.privateKeyPassword);

        configManager.mainConfig.tryGetString("WebAuth", "Host", &webAuth.host);
        configManager.mainConfig.tryGetInt("WebAuth", "Port", &webAuth.port);

        configManager.mainConfig.tryGetString("LogonDatabase", "Hostname", &logonDatabase.host);
        configManager.mainConfig.tryGetString("LogonDatabase", "Username", &logonDatabase.user);
        configManager.mainConfig.tryGetString("LogonDatabase", "Name", &logonDatabase.name);
        configManager.mainConfig.tryGetString("LogonDatabase", "Password", &logonDatabase.password);
        configManager.mainConfig.tryGetInt("LogonDatabase", "Port", &logonDatabase.port);
        configManager.mainConfig.tryGetInt("LogonDatabase", "Connections", &logonDatabase.connections);
        configManager.mainConfig.tryGetBool("LogonDatabase", "LegacyAuth", &logonDatabase.legacyAuth);

        // Character DB normally uses the same MySQL endpoint/credentials as the
        // logon DB.  Inherit them so existing bnetserver.conf files keep working;
        // only the database name differs unless explicitly overridden below.
        characterDatabase.host = logonDatabase.host;
        characterDatabase.user = logonDatabase.user;
        characterDatabase.password = logonDatabase.password;
        characterDatabase.port = logonDatabase.port;
        characterDatabase.legacyAuth = logonDatabase.legacyAuth;

        configManager.mainConfig.tryGetString("CharacterDatabase", "Hostname", &characterDatabase.host);
        configManager.mainConfig.tryGetString("CharacterDatabase", "Username", &characterDatabase.user);
        configManager.mainConfig.tryGetString("CharacterDatabase", "Name", &characterDatabase.name);
        configManager.mainConfig.tryGetString("CharacterDatabase", "Password", &characterDatabase.password);
        configManager.mainConfig.tryGetInt("CharacterDatabase", "Port", &characterDatabase.port);
        configManager.mainConfig.tryGetInt("CharacterDatabase", "Connections", &characterDatabase.connections);
        configManager.mainConfig.tryGetBool("CharacterDatabase", "LegacyAuth", &characterDatabase.legacyAuth);

        configManager.mainConfig.tryGetString("World", "Host", &world.host);
        configManager.mainConfig.tryGetInt("World", "Port", &world.port);

        configManager.mainConfig.tryGetString("BattleNetComm", "Host", &battleNetComm.host);
        configManager.mainConfig.tryGetInt("BattleNetComm", "Port", &battleNetComm.port);
        configManager.mainConfig.tryGetString("BattleNetComm", "SharedSecret", &battleNetComm.sharedSecret);

        configManager.mainConfig.tryGetInt("Logger", "MinimumMessageType", &logger.minimumMessageType);
    }
}
