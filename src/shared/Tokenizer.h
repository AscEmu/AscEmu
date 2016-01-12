/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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
 *
 *
 * STL String Tokenizer from http://www.codeproject.com/KB/recipes/stringtok.aspx with a few modifications
*/
#pragma once

#include "Common.h"
namespace Arcemu
{
    namespace Utility
    {
        class IsDelimiter : public std::unary_function<char, bool>
        {
            public:
                //Constructor specifying the separators
                IsDelimiter(std::string const & rostr) : m_ostr(rostr) {}
                bool operator()(char c) const
                {
                    return m_ostr.find(c) != std::string::npos;
                }

            private:
            std::string m_ostr;
        };

        class CTokenizer
        {
                IsDelimiter roPred;
            std::vector<std::string> tokens_;
                void Tokenize(std::string const & rostr)
                {
                    std::string::const_iterator it, itTokenEnd;
                    it = itTokenEnd = rostr.begin();
                    while(it != rostr.end())
                    {
                        while(this->roPred(*it))
                            ++it;
                        //Find next token

                        itTokenEnd = find_if(it, rostr.end(), roPred);
                        //Append token to result

                        if(it < itTokenEnd)
                            tokens_.push_back(std::string(it, itTokenEnd));
                        it = itTokenEnd;
                    }
                }
            public:
                typedef std::vector<std::string>::iterator iterator;
                CTokenizer(std::string const & input, std::string const & delimiter) : roPred(delimiter) { Tokenize(input); }
                iterator start() { return tokens_.begin(); }
                iterator end() { return tokens_.end(); }
        };
    }
}