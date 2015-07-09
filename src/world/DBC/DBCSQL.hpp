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

#include "StdAfx.h"

namespace DBC
{
    namespace SQL
    {
        struct SqlDbc
        {
            std::string const* format_string;
            std::string const* index_name;
            std::string sql_table_name;
            int32 index_pos;
            int32 sql_index_pos;
            SqlDbc(std::string const* dbc_filename, std::string const* dbc_format, std::string const* id_name, char const* format);
        private:
            SqlDbc(SqlDbc const& right) = delete;
            SqlDbc& operator=(SqlDbc const& right) = delete;
        };
    }
}

#endif // _DBC_SQL_H