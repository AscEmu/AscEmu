/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Console/BaseConsole.h"
#include <string>

bool handleSendChatAnnounceCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient);
bool handleBanAccountCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient);
bool handleCreateAccountCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient);
bool handleAccountPermission(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient);
bool handleCancelShutdownCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient);
bool handleServerInfoCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient);
bool handleOnlineGmsCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient);
bool handleKickPlayerCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient);
bool handleMotdCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient);
bool handleListOnlinePlayersCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient);
bool handlePlayerInfoCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient);
bool handleShutDownServerCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string consoleInput, bool isWebClient);
bool handleRehashConfigCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient);
bool handleUnbanAccountCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient);
bool handleSendWAnnounceCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient);
bool handleWhisperCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient);
bool handleCreateNameHashCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient);
bool handleRevivePlayerCommand(BaseConsole* baseConsole, int argumentCount, std::string consoleInput, bool isWebClient);
bool handleClearConsoleCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient);
bool handleReloadScriptEngineCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient);
bool handlePrintTimeDateCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient);
bool handleGetAccountsCommand(BaseConsole* baseConsole, int /*argumentCount*/, std::string /*consoleInput*/, bool isWebClient);
