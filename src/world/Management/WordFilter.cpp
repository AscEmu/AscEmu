/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/WordFilter.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"

std::unique_ptr<WordFilter> g_chatFilter;

WordFilter::WordFilter() {}

WordFilter::~WordFilter() {}

//!\brief case sensitive
//!\TODO replace it with boost::regex or c++14/17 std::regex
bool WordFilter::isBlockedOrReplaceWord(std::string& chatMessage)
{
    std::list<MySQLStructure::WordFilterChat>::const_iterator iterator;
    for (iterator = sMySQLStore._wordFilterChatStore.begin(); iterator != sMySQLStore._wordFilterChatStore.end(); ++iterator)
    {
        size_t pos = 0;
        while ((pos = chatMessage.find(iterator->word, pos)) != std::string::npos)
        {
            if (iterator->blockMessage)
            {
                return true;
            }

            chatMessage.replace(pos, iterator->word.length(), iterator->wordReplace);
            pos += iterator->wordReplace.length();
        }
    }
    return false;
}
