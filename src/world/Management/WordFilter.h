/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

};

extern WordFilter* g_chatFilter;
