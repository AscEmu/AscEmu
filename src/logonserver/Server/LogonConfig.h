/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <string>
#include <cstdint>

class SERVER_DECL LogonConfig
{
    public:

        LogonConfig();

        void loadConfigValues(bool reload = false);

        // logon.conf - LogonDatabase
        struct LogonDatabase
        {
            std::string host;
            std::string user;
            std::string db;
            std::string password;
            uint32_t port;
            int connections;
        } logonDb;

        // logon.conf - Listen
        struct Listen
        {
            std::string host;
            std::string interServerHost;
            uint32_t realmListPort;
            uint32_t port;
        } listen;

        // logon.conf - LogLevel
        struct LogLevel
        {
            uint32_t file;
        } logLevel;

        // logon.conf - Rates
        struct Rates
        {
            uint32_t accountRefreshTime;
        } rates;

        // logon.conf - LogonServer
        struct LogonServer
        {
            bool disablePings;
            std::string allowedIps;
            std::string allowedModIps;
        } logonServer;
};
