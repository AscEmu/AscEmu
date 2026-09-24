/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string>

namespace AscEmu::Battlenet
{
    struct BNetConfig
    {
        struct ListenSettings
        {
            std::string host = "0.0.0.0";
            uint32_t port = 1119;
        } listen;

        struct TlsSettings
        {
            std::string certificatesFile = "bnetserver.cert.pem";
            std::string privateKeyFile = "bnetserver.key.pem";
            std::string privateKeyPassword;
        } tls;

        struct WebAuthSettings
        {
            std::string host = "0.0.0.0";
            uint32_t port = 8081;
        } webAuth;

        struct LogonDatabaseSettings
        {
            std::string host = "127.0.0.1";
            std::string user = "root";
            std::string name = "ascemu_logon";
            std::string password;
            uint32_t port = 3306;
            uint32_t connections = 2;
            bool legacyAuth = false;
        } logonDatabase;

        struct CharacterDatabaseSettings
        {
            std::string host = "127.0.0.1";
            std::string user = "root";
            std::string name = "ascemu_char";
            std::string password;
            uint32_t port = 3306;
            uint32_t connections = 2;
            bool legacyAuth = false;
        } characterDatabase;

        struct WorldSettings
        {
            std::string host = "127.0.0.1";
            uint32_t port = 8129;
        } world;

        struct BattleNetCommSettings
        {
            std::string host = "127.0.0.1";
            uint32_t port = 1120;
            std::string sharedSecret = "ascemu-bnetcomm";
        } battleNetComm;

        struct LoggerSettings
        {
            uint8_t minimumMessageType = 2;
        } logger;

        void load();
    };

    inline BNetConfig bnetConfig;
}
