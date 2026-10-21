/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BNetConfig.hpp"

#include "Server/LogonServerDefines.hpp"

namespace AscEmu::Battlenet
{
    void BNetConfig::load()
    {
        Config.MainConfig.tryGetBool("BattleNet", "Enabled", &listen.enabled);
        Config.MainConfig.tryGetString("BattleNet", "Host", &listen.host);
        Config.MainConfig.tryGetInt("BattleNet", "Port", &listen.port);

        Config.MainConfig.tryGetString("TLS", "CertificatesFile", &tls.certificatesFile);
        Config.MainConfig.tryGetString("TLS", "PrivateKeyFile", &tls.privateKeyFile);
        Config.MainConfig.tryGetString("TLS", "PrivateKeyPassword", &tls.privateKeyPassword);

        Config.MainConfig.tryGetString("WebAuth", "Host", &webAuth.host);
        Config.MainConfig.tryGetInt("WebAuth", "Port", &webAuth.port);
        Config.MainConfig.tryGetString("WebAuth", "ExternalAddress", &webAuth.externalAddress);

    }
}
