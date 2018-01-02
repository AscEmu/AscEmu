/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Chat/ChatCommand.hpp"
#include "Singleton.h"

class SERVER_DECL CommandTableStorage : public Singleton<CommandTableStorage>
{
    ChatCommand* _modifyCommandTable;
    ChatCommand* _debugCommandTable;
    ChatCommand* _eventCommandTable;
    ChatCommand* _waypointCommandTable;
    ChatCommand* _GMTicketCommandTable;
    ChatCommand* _TicketCommandTable;
    ChatCommand* _GuildCommandTable;
    ChatCommand* _GameObjectSetCommandTable;
    ChatCommand* _GameObjectCommandTable;
    ChatCommand* _BattlegroundCommandTable;
    ChatCommand* _NPCSetCommandTable;
    ChatCommand* _NPCCommandTable;
    ChatCommand* _CheatCommandTable;
    ChatCommand* _accountCommandTable;
    ChatCommand* _petCommandTable;
    ChatCommand* _recallCommandTable;
    ChatCommand* _questCommandTable;
    ChatCommand* _serverCommandTable;
    ChatCommand* _reloadTableCommandTable;
    ChatCommand* _gmCommandTable;
    ChatCommand* _characterAddCommandTable;
    ChatCommand* _characterSetCommandTable;
    ChatCommand* _characterListCommandTable;
    ChatCommand* _characterCommandTable;
    ChatCommand* _lookupCommandTable;
    ChatCommand* _adminCommandTable;
    ChatCommand* _kickCommandTable;
    ChatCommand* _banCommandTable;
    ChatCommand* _unbanCommandTable;
    ChatCommand* _instanceCommandTable;
    ChatCommand* _arenaCommandTable;
    ChatCommand* _achievementCommandTable;
    ChatCommand* _vehicleCommandTable;
    ChatCommand* _transportCommandTable;
    ChatCommand* _commandTable;

    ChatCommand* GetSubCommandTable(const char* name);
    ChatCommand* GetCharSubCommandTable(const char* name);
    ChatCommand* GetNPCSubCommandTable(const char* name);
    ChatCommand* GetGOSubCommandTable(const char* name);
    ChatCommand* GetReloadCommandTable(const char* name);

public:

    void Init();
    void Dealloc();
    void Load();
    void Override(const char* command, const char* level);
    inline ChatCommand* Get() { return _commandTable; }
};

#define sCommandTableStorag CommandTableStorage::getSingleton()
