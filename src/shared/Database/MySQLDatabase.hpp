/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"
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
        uint32_t ConnectionCount, uint32_t BufferSize, bool useLegacyAuth = false) override;

    void shutdown() override;

    std::string escapeString(std::string Escape) override;
    void escapeLongString(const char* str, uint32_t len, std::stringstream& out) override;
    std::string escapeString(const char* esc, DatabaseConnection* con) override;

    bool supportsReplaceInto() override { return true; }
    bool supportsTableLocking() override { return true; }

protected:
    bool _handleError(MySQLDatabaseConnection*, uint32_t ErrorNumber);
    bool _sendQuery(DatabaseConnection* con, const char* Sql, bool Self = false) override;

    void _beginTransaction(DatabaseConnection* conn) override;
    void _endTransaction(DatabaseConnection* conn) override;
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
