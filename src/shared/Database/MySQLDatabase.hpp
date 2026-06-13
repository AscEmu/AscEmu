/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Database.hpp"
#include <string>
#include <mysql.h>


struct MySQLDatabaseConnection : public DatabaseConnection
{
    MySQLDatabaseConnection(MYSQL* _mysql) : MySql(_mysql) {}
    MYSQL* MySql;
};


class SERVER_DECL MySQLDatabase : public Database
{
    public:

        MySQLDatabase();
        ~MySQLDatabase();

        bool Initialize(const char* Hostname, unsigned int port,
                        const char* Username, const char* Password, const char* DatabaseName,
                        uint32_t ConnectionCount, uint32_t BufferSize, bool useLegacyAuth = false);

        void Shutdown();

        std::string EscapeString(std::string Escape);
        void EscapeLongString(const char* str, uint32_t len, std::stringstream & out);
        std::string EscapeString(const char* esc, DatabaseConnection* con);

        bool SupportsReplaceInto() { return true; }
        bool SupportsTableLocking() { return true; }

    protected:

        bool _HandleError(MySQLDatabaseConnection*, uint32_t ErrorNumber);
        bool _SendQuery(DatabaseConnection* con, const char* Sql, bool Self = false);

        void _BeginTransaction(DatabaseConnection* conn);
        void _EndTransaction(DatabaseConnection* conn);
        bool _Reconnect(MySQLDatabaseConnection* conn);

        std::unique_ptr<QueryResult> _StoreQueryResult(DatabaseConnection* con) override;
};


class SERVER_DECL MySQLQueryResult : public QueryResult
{
    public:

        MySQLQueryResult(MYSQL_RES* res, uint32_t FieldCount, uint32_t RowCount);
        ~MySQLQueryResult();

        bool NextRow();

    protected:

        MYSQL_RES* mResult;
};
