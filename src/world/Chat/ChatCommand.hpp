/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once 

#include <string>
#include <string_view>
#include <vector>
#include <initializer_list>
#include <functional>
#include <sstream>
#include <iterator>

class ChatHandler;
class WorldSession;

struct ChatCommandNEW
{
    using HandlerFn = std::function<bool(ChatHandler*, std::string_view, WorldSession*)>;

    std::string command;
    std::string commandPermission;
    HandlerFn handler;
    std::string help;

    // Legacy executable with function linker and help (args as const char*)
    ChatCommandNEW(const char* cmd, std::string perm, HandlerFn fn, std::string helpText)
        : command(cmd ? cmd : ""), commandPermission(std::move(perm)),
          handler(std::move(fn)), help(std::move(helpText)) {}

    // Modern executable with function linker and help (args as std::string_view)
    ChatCommandNEW(std::string cmd, std::string perm, HandlerFn fn, std::string helpText)
        : command(std::move(cmd)), commandPermission(std::move(perm)),
          handler(std::move(fn)), help(std::move(helpText)) {}

    // Helper functions for first level commands without handler and help
    ChatCommandNEW(const char* cmd, std::string perm = "0")
        : command(cmd ? cmd : ""), commandPermission(std::move(perm)),
          handler(nullptr), help() {}

    ChatCommandNEW(std::string cmd, std::string perm = "0")
        : command(std::move(cmd)), commandPermission(std::move(perm)),
          handler(nullptr), help() {}
};

// Legacy wrapper 
inline ChatCommandNEW::HandlerFn
wrap(bool (ChatHandler::*pm)(const char*, WorldSession*))
{
    return [pm](ChatHandler* self, std::string_view sv, WorldSession* sess) -> bool {
        std::vector<char> buf;
        buf.reserve(sv.size() + 1);
        buf.insert(buf.end(), sv.begin(), sv.end());
        buf.push_back('\0');

        const char* argz = buf.data();

        return (self->*pm)(argz, sess);
    };
}

// Modern wrapper - bool(ChatHandler::*)(std::string_view, WorldSession*)
inline ChatCommandNEW::HandlerFn
wrap(bool (ChatHandler::*pm)(std::string_view, WorldSession*))
{
    return [pm](ChatHandler* self, std::string_view sv, WorldSession* sess) -> bool {
        return (self->*pm)(sv, sess);
    };
}
