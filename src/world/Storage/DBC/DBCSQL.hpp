/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>
#include "Common.hpp"
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
