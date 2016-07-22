/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _CHAT_COMMAND_HPP
#define _CHAT_COMMAND_HPP

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

    uint32 NormalValueField;
    uint32 MaxValueField;

    /// ValueType: 0 = nothing, 1 = uint, 2 = float
    uint16 ValueType;
};

#endif // _CHAT_COMMAND_HPP
