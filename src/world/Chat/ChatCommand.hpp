/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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

    uint16_t NormalValueField;
    uint16_t MaxValueField;

    /// ValueType: 0 = nothing, 1 = uint, 2 = float
    uint16_t ValueType;
};
