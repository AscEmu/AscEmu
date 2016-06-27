/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _WORLD_MASTER_H
#define _WORLD_MASTER_H

#include "Common.h"
#include "Config/ConfigEnv.h"
#include "Database/DatabaseEnv.h"
#include "MainServerDefines.h"
#include "../shared/AscemuServerDefines.hpp"

static const char* REQUIRED_CHAR_DB_VERSION = "2016-03-28_01_gm_survey";
static const char* REQUIRED_WORLD_DB_VERSION = "2016-06-27_01_drop_creature_tables";

class Master : public Singleton<Master>
{
    public:

        Master();
        ~Master();

        bool Run(int argc, char** argv);
        void PrintBanner();
        bool LoadWorldConfiguration(char* config_file, char* optional_config_file, char* realm_config_file);
        void OpenCheatLogFiles();
        void StartNetworkSubsystem();
        void StartRemoteConsole();
        void WritePidFile();

        void ShutdownThreadPools(bool listnersockcreate);
        void ShutdownLootSystem();
        bool m_ShutdownEvent;
        uint32 m_ShutdownTimer;

        static volatile bool m_stopEvent;
        bool m_restartEvent;

    private:

        bool _StartDB();
        void _StopDB();
        bool _CheckDBVersion();

        void _HookSignals();
        void _UnhookSignals();

        static void _OnSignal(int s);
};

#define sMaster Master::getSingleton()

#endif // _WORLD_MASTER_H
