/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <memory>
#include <string>

class WordFilter
{
public:
    WordFilter();
    ~WordFilter();

    bool isBlockedOrReplaceWord(std::string& chatMessage);
};

extern std::unique_ptr<WordFilter> g_chatFilter;
