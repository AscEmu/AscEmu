/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "DBCSQL.hpp"

namespace DBC::SQL
{
    SqlDbc::SqlDbc(std::string const* dbc_filename, std::string const* dbc_format, std::string const* id_name, char const* format) : format_string(dbc_format), index_name(id_name), sql_index_pos(0)
    {
        /* Convert DBC file name to SQL table name */
        sql_table_name = *dbc_filename;
        for (uint32_t i = 0; i < sql_table_name.size(); ++i)
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
            uint32_t unsigned_index_pos = uint32_t(index_pos);
            for (uint32_t x = 0; x < format_string->size(); ++x)
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
