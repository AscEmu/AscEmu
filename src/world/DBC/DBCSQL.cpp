/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "DBCSQL.hpp"

namespace DBC
{
    namespace SQL
    {
        SqlDbc::SqlDbc(std::string const* dbc_filename, std::string const* dbc_format, std::string const* id_name, char const* format) : format_string(dbc_format), index_name(id_name), sql_index_pos(0)
        {
            /* Convert DBC file name to SQL table name */
            sql_table_name = *dbc_filename;
            for (uint32 i = 0; i < sql_table_name.size(); ++i)
            {
                if (isalpha(sql_table_name[i]))
                {
                    sql_table_name[i] = char(tolower(sql_table_name[i]));
                }
                else if (sql_table_name[i] == '.')
                {
                    sql_table_name[i] = '_';
                }
            }

            /* Get SQL index position */
            DBC::DBCLoader::GetFormatRecordSize(format, &index_pos);
            if (index_pos >= 0)
            {
                uint32 unsigned_index_pos = uint32(index_pos);
                for (uint32 x = 0; x < format_string->size(); ++x)
                {
                    if ((*format_string)[x] == DBC::DbcFieldFormat::FT_SQL_PRESENT)
                    {
                        if (x == unsigned_index_pos)
                        {
                            break;
                        }

                        ++sql_index_pos;
                    }
                }
            }
        }
    }
}