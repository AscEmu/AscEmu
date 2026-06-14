/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MySQLDatabase.hpp"

#include "../Logging/Logger.hpp"
#include "DatabaseRuntime.hpp"

#include <algorithm>
#include <cstring>
#include <mutex>
#include <sstream>
#include <thread>

namespace
{
    std::mutex g_mysqlClientMutex;
    std::uint32_t g_mysqlClientRefCount = 0;

    void retainMySqlClientLibrary()
    {
        std::scoped_lock lock(g_mysqlClientMutex);
        if (g_mysqlClientRefCount == 0)
            mysql_library_init(0, nullptr, nullptr);

        ++g_mysqlClientRefCount;
    }

    void releaseMySqlClientLibrary()
    {
        std::scoped_lock lock(g_mysqlClientMutex);

        if (g_mysqlClientRefCount == 0)
            return;

        --g_mysqlClientRefCount;
        if (g_mysqlClientRefCount == 0)
            mysql_library_end();
    }
}

MySQLDatabase::MySQLDatabase() : Database()
{
}

MySQLDatabase::~MySQLDatabase()
{
    shutdown();
}

void MySQLDatabase::_beginTransaction(DatabaseConnection* conn)
{
    _sendQuery(conn, "START TRANSACTION", false);
}

void MySQLDatabase::_endTransaction(DatabaseConnection* conn)
{
    _sendQuery(conn, "COMMIT", false);
}

bool MySQLDatabase::initialize(const char* hostname, unsigned int port,
                               const char* username, const char* password, const char* databaseName,
                               uint32_t connectionCount, uint32_t, bool useLegacyAuth)
{
    if (connectionCount == 0)
        return false;

    retainMySqlClientLibrary();

    auto rollbackInit = [this]()
    {
        for (const auto& conn : m_connections)
        {
            if (const auto* mysqlConn = dynamic_cast<const MySQLDatabaseConnection*>(conn.get()))
            {
                if (mysqlConn->MySql != nullptr)
                    mysql_close(mysqlConn->MySql);
            }
        }

        m_connections.clear();
        releaseMySqlClientLibrary();
    };

    m_hostname = hostname != nullptr ? hostname : "";
    m_port = port;
    m_username = username != nullptr ? username : "";
    m_password = password != nullptr ? password : "";
    m_databaseName = databaseName != nullptr ? databaseName : "";
    m_connectionCount = static_cast<int32_t>(connectionCount);

    bool reconnect = true;
    m_connections.clear();
    m_connections.reserve(connectionCount);

    for (uint32_t i = 0; i < connectionCount; ++i)
    {
        MYSQL* mysql = mysql_init(nullptr);
        if (mysql == nullptr)
        {
            rollbackInit();
            return false;
        }

        mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8");
        mysql_options(mysql, MYSQL_OPT_RECONNECT, &reconnect);

        if (!useLegacyAuth)
            mysql_options(mysql, MYSQL_DEFAULT_AUTH, "caching_sha2_password");

        MYSQL* connected = mysql_real_connect(
            mysql,
            m_hostname.c_str(),
            m_username.c_str(),
            m_password.c_str(),
            m_databaseName.c_str(),
            m_port,
            nullptr,
            0);

        if (connected == nullptr)
        {
            sLogger.failure("Connection failed due to: `{}`", mysql_error(mysql));
            mysql_close(mysql);
            rollbackInit();
            return false;
        }

        m_connections.emplace_back(std::make_unique<MySQLDatabaseConnection>(connected));
    }

    const auto hw = std::thread::hardware_concurrency();
    const auto maxWorkers = hw == 0U ? connectionCount : std::min<uint32_t>(connectionCount, hw);
    const auto workerCount = static_cast<uint16_t>(std::max<uint32_t>(1U, maxWorkers));

    m_runtime = std::make_unique<DatabaseRuntime>(*this, workerCount);
    m_runtime->primeConnections(m_connections);
    m_runtime->start();

    return true;
}

void MySQLDatabase::shutdown()
{
    if (m_runtime)
    {
        m_runtime->stop();
        m_runtime.reset();
    }

    const bool hadConnections = !m_connections.empty();

    for (const auto& conn : m_connections)
    {
        if (const auto* mysqlConn = dynamic_cast<const MySQLDatabaseConnection*>(conn.get()))
        {
            if (mysqlConn->MySql != nullptr)
                mysql_close(mysqlConn->MySql);
        }
    }

    m_connections.clear();

    if (hadConnections)
        releaseMySqlClientLibrary();
}

std::string MySQLDatabase::escapeString(std::string escape)
{
    if (escape.empty() || !m_runtime)
        return escape;

    return m_runtime->withConnection([&](DatabaseConnection& connection)
    {
        return escapeString(escape.c_str(), &connection);
    });
}

void MySQLDatabase::escapeLongString(const char* str, uint32_t len, std::stringstream& out)
{
    if (str == nullptr || len == 0 || !m_runtime)
        return;

    auto writeEscaped = [&](DatabaseConnection& connection)
    {
        std::string escaped(static_cast<size_t>(len) * 2U + 1U, '\0');
        const auto written = mysql_real_escape_string(
            static_cast<MySQLDatabaseConnection*>(&connection)->MySql,
            escaped.data(),
            str,
            static_cast<unsigned long>(len));

        out.write(escaped.data(), static_cast<std::streamsize>(written));
    };

    m_runtime->withConnection(writeEscaped);
}

std::string MySQLDatabase::escapeString(const char* esc, DatabaseConnection* con)
{
    if (esc == nullptr || con == nullptr)
        return {};

    const auto inputLength = std::strlen(esc);
    std::string escaped(inputLength * 2U + 1U, '\0');

    const auto written = mysql_real_escape_string(
        static_cast<MySQLDatabaseConnection*>(con)->MySql,
        escaped.data(),
        esc,
        static_cast<unsigned long>(inputLength));

    escaped.resize(static_cast<std::size_t>(written));
    return escaped;
}

bool MySQLDatabase::_sendQuery(DatabaseConnection* con, const char* sql, bool self)
{
    auto* dbConnection = static_cast<MySQLDatabaseConnection*>(con);
    int result = mysql_query(dbConnection->MySql, sql);

    if (result > 0)
    {
        if (!self && _handleError(dbConnection, mysql_errno(dbConnection->MySql)))
            result = _sendQuery(con, sql, true);
        else
            sLogger.failure("Sql query failed due to [{}], Query: [{}]", mysql_error(dbConnection->MySql), sql);
    }

    return result == 0;
}

bool MySQLDatabase::_handleError(MySQLDatabaseConnection* con, uint32_t errorNumber)
{
    switch (errorNumber)
    {
        case 2006:
        case 2008:
        case 2013:
        case 2055:
            return _reconnect(con);
        default:
            return false;
    }
}

bool MySQLDatabase::_reconnect(MySQLDatabaseConnection* conn)
{
    MYSQL* mysql = mysql_init(nullptr);
    if (mysql == nullptr)
        return false;

    MYSQL* connected = mysql_real_connect(
        mysql,
        m_hostname.c_str(),
        m_username.c_str(),
        m_password.c_str(),
        m_databaseName.c_str(),
        m_port,
        nullptr,
        0);

    if (connected == nullptr)
    {
        sLogger.failure("Could not reconnect to database because of `{}`", mysql_error(mysql));
        mysql_close(mysql);
        return false;
    }

    if (conn->MySql != nullptr)
        mysql_close(conn->MySql);

    conn->MySql = connected;
    return true;
}

MySQLQueryResult::MySQLQueryResult(MYSQL_RES* res, uint32_t fieldCount, uint32_t rowCount)
    : QueryResult(fieldCount, rowCount), m_result(res)
{
    m_currentRow = std::make_unique<Field[]>(fieldCount);
}

MySQLQueryResult::~MySQLQueryResult()
{
    if (m_result != nullptr)
        mysql_free_result(m_result);
}

bool MySQLQueryResult::nextRow()
{
    MYSQL_ROW row = mysql_fetch_row(m_result);
    if (row == nullptr)
        return false;

    unsigned long* lengths = mysql_fetch_lengths(m_result);

    for (uint32_t i = 0; i < m_fieldCount; ++i)
    {
        const std::size_t fieldLength = lengths != nullptr ? static_cast<std::size_t>(lengths[i]) : 0U;
        m_currentRow[i].setValue(row[i], fieldLength);
    }

    return true;
}

std::unique_ptr<QueryResult> MySQLDatabase::_storeQueryResult(DatabaseConnection* con)
{
    auto* db = static_cast<MySQLDatabaseConnection*>(con);
    MYSQL_RES* result = mysql_store_result(db->MySql);

    const uint32_t fieldCount = static_cast<uint32_t>(mysql_field_count(db->MySql));
    if (fieldCount == 0U || result == nullptr)
    {
        if (result != nullptr)
            mysql_free_result(result);

        return nullptr;
    }

    const uint32_t rowCount = static_cast<uint32_t>(mysql_num_rows(result));
    if (rowCount == 0U)
    {
        mysql_free_result(result);
        return nullptr;
    }

    auto queryResult = std::make_unique<MySQLQueryResult>(result, fieldCount, rowCount);
    queryResult->nextRow();
    return queryResult;
}
