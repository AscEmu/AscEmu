/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <string>

struct WordFilterMatch
{
    char* szMatch;
    char* szIgnoreMatch;
    void* pCompiledExpression;
    void* pCompiledIgnoreExpression;
    void* pCompiledExpressionOptions;
    void* pCompiledIgnoreExpressionOptions;
    int iType;
};

class WordFilter
{
    WordFilterMatch** m_filters;
    size_t m_filterCount;

    bool CompileExpression(const char* szExpression, void** pOutput, void** pExtraOutput);

public:
    WordFilter() : m_filters(nullptr), m_filterCount(0)
    {
    }

    ~WordFilter();

    void Load(const char* szTableName);
    bool Parse(std::string& sMessage, bool bAllowReplace = true);
    bool ParseEscapeCodes(char* sMessage, bool bAllowLinks);
};

extern WordFilter* g_characterNameFilter;
extern WordFilter* g_chatFilter;
