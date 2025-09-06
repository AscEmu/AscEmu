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

class ChatCommandHandler;
class WorldSession;

struct ChatCommand
{
    using HandlerFn = std::function<bool(ChatCommandHandler*, std::string_view, WorldSession*)>;

    std::string command;
    std::string commandPermission;
    size_t minArgCount;
    HandlerFn handler;
    std::string help;

    // Legacy executable with function linker and help (args as const char*)
    ChatCommand(const char* cmd, std::string perm, size_t args, HandlerFn fn, std::string helpText)
        : command(cmd ? cmd : ""), commandPermission(std::move(perm)), minArgCount(args),
          handler(std::move(fn)), help(std::move(helpText)) {}

    // Modern executable with function linker and help (args as std::string_view)
    ChatCommand(std::string cmd, std::string perm, size_t args, HandlerFn fn, std::string helpText)
        : command(std::move(cmd)), commandPermission(std::move(perm)), minArgCount(args),
          handler(std::move(fn)), help(std::move(helpText)) {}

    // Helper functions for first level commands without handler and help
    ChatCommand(const char* cmd, std::string perm, size_t args = 0)
        : command(cmd ? cmd : ""), commandPermission(std::move(perm)), minArgCount(args),
          handler(nullptr), help() {}

    ChatCommand(std::string cmd, std::string perm, size_t args = 0)
        : command(std::move(cmd)), commandPermission(std::move(perm)), minArgCount(args),
          handler(nullptr), help() {}
};

inline size_t countWords(const char* argz)
{
    if (!argz)
        return 0;

    size_t count = 0;
    bool inWord = false;

    for (const char* p = argz; *p; ++p)
    {
        if (std::isspace(static_cast<unsigned char>(*p)))
        {
            inWord = false;
        }
        else if (!inWord)
        {
            inWord = true;
            ++count;
        }
    }
    return count;
}

// Legacy wrapper 
inline ChatCommand::HandlerFn
wrap(bool (ChatCommandHandler::*pm)(const char*, WorldSession*))
{
    return [pm](ChatCommandHandler* self, std::string_view sv, WorldSession* sess) -> bool {
        std::vector<char> buf;
        buf.reserve(sv.size() + 1);
        buf.insert(buf.end(), sv.begin(), sv.end());
        buf.push_back('\0');

        const char* argz = buf.data();

        return (self->*pm)(argz, sess);
    };
}

inline size_t countWords(std::string_view sv)
{
    size_t count = 0;
    bool inWord = false;

    for (char c : sv)
    {
        if (std::isspace(static_cast<unsigned char>(c)))
        {
            inWord = false;
        }
        else if (!inWord)
        {
            inWord = true;
            ++count;
        }
    }
    return count;
}

// Modern wrapper - bool(ChatCommandHandler::*)(std::string_view, WorldSession*)
inline ChatCommand::HandlerFn
wrap(bool (ChatCommandHandler::*pm)(std::string_view, WorldSession*))
{
    return [pm](ChatCommandHandler* self, std::string_view sv, WorldSession* sess) -> bool {
        return (self->*pm)(sv, sess);
    };
}
