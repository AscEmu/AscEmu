/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once 

#include <string>

class ChatHandler;
class WorldSession;

class ChatCommand
{
public:

    const char* Name;

    char CommandGroup;

    bool (ChatHandler::*Handler)(const char* args, WorldSession* m_session);

    std::string Help;

    ChatCommand* ChildCommands;
};
