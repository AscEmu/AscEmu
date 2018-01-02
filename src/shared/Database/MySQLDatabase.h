/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
 */

#ifndef _MYSQLDATABASE_H
#define _MYSQLDATABASE_H

#include <string>
#include <mysql.h>


struct MySQLDatabaseConnection : public DatabaseConnection
{
    MYSQL* MySql;
};


class SERVER_DECL MySQLDatabase : public Database
{
    friend class QueryThread;
    friend class AsyncQuery;

    public:

        MySQLDatabase();
        ~MySQLDatabase();

        bool Initialize(const char* Hostname, unsigned int port,
                        const char* Username, const char* Password, const char* DatabaseName,
                        uint32 ConnectionCount, uint32 BufferSize);

        void Shutdown();

        std::string EscapeString(std::string Escape);
        void EscapeLongString(const char* str, uint32 len, std::stringstream & out);
        std::string EscapeString(const char* esc, DatabaseConnection* con);

        bool SupportsReplaceInto() { return true; }
        bool SupportsTableLocking() { return true; }

    protected:

        bool _HandleError(MySQLDatabaseConnection*, uint32 ErrorNumber);
        bool _SendQuery(DatabaseConnection* con, const char* Sql, bool Self = false);

        void _BeginTransaction(DatabaseConnection* conn);
        void _EndTransaction(DatabaseConnection* conn);
        bool _Reconnect(MySQLDatabaseConnection* conn);

        QueryResult* _StoreQueryResult(DatabaseConnection* con);
};


class SERVER_DECL MySQLQueryResult : public QueryResult
{
    public:

        MySQLQueryResult(MYSQL_RES* res, uint32 FieldCount, uint32 RowCount);
        ~MySQLQueryResult();

        bool NextRow();

    protected:

        MYSQL_RES* mResult;
};

#endif        // _MYSQLDATABASE_H
