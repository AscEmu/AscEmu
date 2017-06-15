/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>

class WordFilter
{
    public:

        WordFilter();
        ~WordFilter();

        bool isBlockedOrReplaceWord(std::string& chatMessage);
        bool isCharacterNameAllowed(std::string charName);

};

extern WordFilter* g_characterNameFilter;
extern WordFilter* g_chatFilter;
