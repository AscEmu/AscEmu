/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Console/BaseConsole.h"

bool handleSendChatAnnounceCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput);
bool handleBanAccountCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput);
bool handleCancelShutdownCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/);
bool handServerleInfoCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/);
bool handleOnlineGmsCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/);
bool handleKickPlayerCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput);
bool handleMotdCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput);
bool handleListOnlinePlayersCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/);
bool handlePlayerInfoCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput);
bool handleShutDownServerCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string consoleInput);
bool handleRehashConfigCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/);
bool handleUnbanAccountCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput);
bool handleSendWAnnounceCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput);
bool handleWhisperCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput);
bool handleCreateNameHashCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput);
bool handleRevivePlayerCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput);
bool handleClearConsoleCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/);
bool handleReloadScriptEngineCommand(BaseConsole* /*baseConsole*/, int /*argumentCount*/, std::string /*consoleInput*/);
bool handlePrintTimeDateCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/);
