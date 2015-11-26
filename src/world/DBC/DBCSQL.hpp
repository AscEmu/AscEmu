/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2011 <http://www.ArcEmu.org/>
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

#ifndef _DBC_SQL_H
#define _DBC_SQL_H

#include <string>
#include "Common.h"
#include "DBCLoader.hpp" // Used in constructor

namespace DBC
{
    namespace SQL
    {
        using std::string;

        struct SqlDbc
        {
            string const* format_string;
            string const* index_name;
            string sql_table_name;
            int32 index_pos;
            int32 sql_index_pos;
            SqlDbc(string const* dbc_filename, string const* dbc_format, string const* id_name, char const* format);

            private:

                SqlDbc(SqlDbc const& right) = delete;
                SqlDbc& operator=(SqlDbc const& right) = delete;
        };
    }
}

#endif // _DBC_SQL_H
