/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string>

namespace AscEmu::Battlenet
{
    // logon.conf sections of the Battle.net login (6.x and later clients)
    struct BNetConfig
    {
        struct ListenSettings
        {
            // 0 turns the Battle.net login and the web login off
            bool enabled = true;
            std::string host = "0.0.0.0";
            uint32_t port = 1119;
        } listen;

        struct TlsSettings
        {
            std::string certificatesFile = "certs/bnetserver.cert.pem";
            std::string privateKeyFile = "certs/bnetserver.key.pem";
            std::string privateKeyPassword;
        } tls;

        struct WebAuthSettings
        {
            std::string host = "0.0.0.0";
            uint32_t port = 8081;
            // host name or address the clients reach the web login at
            std::string externalAddress = "bnet.ascemu.local";

            // http(s)://<externalAddress>:<port>/bnetserver/login/<build>/
            // The build of the client is part of the url so the web login knows which form the client expects;
            // an unknown build (0) leaves it out. 10.x clients verify the certificate of the web login against
            // the system store, they get the plain http form of the url and the web auth listener answers them without TLS.
            [[nodiscard]] std::string loginUrl(bool secure, uint32_t clientBuild) const
            {
                std::string url = std::string(secure ? "https://" : "http://") + externalAddress + ":" + std::to_string(port) + "/bnetserver/login/";
                if (clientBuild != 0)
                    url += std::to_string(clientBuild) + "/";
                return url;
            }
        } webAuth;

        // reads the sections from the loaded logon.conf
        void load();
    };

    inline BNetConfig bnetConfig;
}
