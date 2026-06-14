/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Database.hpp"
#include <string>
#include <mysql.h>
#include <memory>

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

    bool initialize(const char* Hostname, unsigned int port,
        const char* Username, const char* Password, const char* DatabaseName,
        uint32_t ConnectionCount, uint32_t BufferSize, bool useLegacyAuth = false);

    void shutdown();

    std::string escapeString(std::string Escape);
    void escapeLongString(const char* str, uint32_t len, std::stringstream& out);
    std::string escapeString(const char* esc, DatabaseConnection* con);

    bool supportsReplaceInto() { return true; }
    bool supportsTableLocking() { return true; }

protected:
    bool _handleError(MySQLDatabaseConnection*, uint32_t ErrorNumber);
    bool _sendQuery(DatabaseConnection* con, const char* Sql, bool Self = false);

    void _beginTransaction(DatabaseConnection* conn);
    void _endTransaction(DatabaseConnection* conn);
    bool _reconnect(MySQLDatabaseConnection* conn);

    std::unique_ptr<QueryResult> _storeQueryResult(DatabaseConnection* con) override;
};

class SERVER_DECL MySQLQueryResult : public QueryResult
{
public:
    MySQLQueryResult(MYSQL_RES* res, uint32_t FieldCount, uint32_t RowCount);
    ~MySQLQueryResult();

    bool nextRow();

protected:
    MYSQL_RES* m_result;
};
