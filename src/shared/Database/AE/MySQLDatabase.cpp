/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../MySQLDatabase.h"

#include "../../Logging/Logger.hpp"
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
    Shutdown();
}

void MySQLDatabase::_BeginTransaction(DatabaseConnection* conn)
{
    _SendQuery(conn, "START TRANSACTION", false);
}

void MySQLDatabase::_EndTransaction(DatabaseConnection* conn)
{
    _SendQuery(conn, "COMMIT", false);
}

bool MySQLDatabase::Initialize(const char* hostname, unsigned int port,
                               const char* username, const char* password, const char* databaseName,
                               uint32_t connectionCount, uint32_t, bool useLegacyAuth)
{
    if (connectionCount == 0)
        return false;

    retainMySqlClientLibrary();

    auto rollbackInit = [this]()
    {
        for (const auto& conn : Connections)
        {
            if (const auto* mysqlConn = dynamic_cast<const MySQLDatabaseConnection*>(conn.get()))
            {
                if (mysqlConn->MySql != nullptr)
                    mysql_close(mysqlConn->MySql);
            }
        }

        Connections.clear();
        releaseMySqlClientLibrary();
    };

    mHostname = hostname != nullptr ? hostname : "";
    mPort = port;
    mUsername = username != nullptr ? username : "";
    mPassword = password != nullptr ? password : "";
    mDatabaseName = databaseName != nullptr ? databaseName : "";
    mConnectionCount = static_cast<int32_t>(connectionCount);

    bool reconnect = true;
    Connections.clear();
    Connections.reserve(connectionCount);

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
            mHostname.c_str(),
            mUsername.c_str(),
            mPassword.c_str(),
            mDatabaseName.c_str(),
            mPort,
            nullptr,
            0);

        if (connected == nullptr)
        {
            sLogger.failure("Connection failed due to: `{}`", mysql_error(mysql));
            mysql_close(mysql);
            rollbackInit();
            return false;
        }

        Connections.emplace_back(std::make_unique<MySQLDatabaseConnection>(connected));
    }

#ifdef ASCEMU_USE_AE_DATABASE
    const auto hw = std::thread::hardware_concurrency();
    const auto maxWorkers = hw == 0U ? connectionCount : std::min<uint32_t>(connectionCount, hw);
    const auto workerCount = static_cast<uint16_t>(std::max<uint32_t>(1U, maxWorkers));

    m_runtime = std::make_unique<DatabaseRuntime>(*this, workerCount);
    m_runtime->primeConnections(Connections);
    m_runtime->start();
#endif

    return true;
}

void MySQLDatabase::Shutdown()
{
#ifdef ASCEMU_USE_AE_DATABASE
    if (m_runtime)
    {
        m_runtime->stop();
        m_runtime.reset();
    }
#endif

    const bool hadConnections = !Connections.empty();

    for (const auto& conn : Connections)
    {
        if (const auto* mysqlConn = dynamic_cast<const MySQLDatabaseConnection*>(conn.get()))
        {
            if (mysqlConn->MySql != nullptr)
                mysql_close(mysqlConn->MySql);
        }
    }

    Connections.clear();

    if (hadConnections)
        releaseMySqlClientLibrary();
}

std::string MySQLDatabase::EscapeString(std::string escape)
{
    if (escape.empty())
        return escape;

    if (m_runtime)
    {
        return m_runtime->withConnection([&](DatabaseConnection& connection)
        {
            return EscapeString(escape.c_str(), &connection);
        });
    }

    DatabaseConnection* connection = GetFreeConnection();
    std::unique_lock<std::recursive_mutex> connectionLock(connection->Busy, std::adopt_lock);
    return EscapeString(escape.c_str(), connection);
}

void MySQLDatabase::EscapeLongString(const char* str, uint32_t len, std::stringstream& out)
{
    if (str == nullptr || len == 0)
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

    if (m_runtime)
    {
        m_runtime->withConnection(writeEscaped);
        return;
    }

    DatabaseConnection* connection = GetFreeConnection();
    std::unique_lock<std::recursive_mutex> connectionLock(connection->Busy, std::adopt_lock);
    writeEscaped(*connection);
}

std::string MySQLDatabase::EscapeString(const char* esc, DatabaseConnection* con)
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

bool MySQLDatabase::_SendQuery(DatabaseConnection* con, const char* sql, bool self)
{
    auto* dbConnection = static_cast<MySQLDatabaseConnection*>(con);
    int result = mysql_query(dbConnection->MySql, sql);

    if (result > 0)
    {
        if (!self && _HandleError(dbConnection, mysql_errno(dbConnection->MySql)))
            result = _SendQuery(con, sql, true);
        else
            sLogger.failure("Sql query failed due to [{}], Query: [{}]", mysql_error(dbConnection->MySql), sql);
    }

    return result == 0;
}

bool MySQLDatabase::_HandleError(MySQLDatabaseConnection* con, uint32_t errorNumber)
{
    switch (errorNumber)
    {
        case 2006:
        case 2008:
        case 2013:
        case 2055:
            return _Reconnect(con);
        default:
            return false;
    }
}

bool MySQLDatabase::_Reconnect(MySQLDatabaseConnection* conn)
{
    MYSQL* mysql = mysql_init(nullptr);
    if (mysql == nullptr)
        return false;

    MYSQL* connected = mysql_real_connect(
        mysql,
        mHostname.c_str(),
        mUsername.c_str(),
        mPassword.c_str(),
        mDatabaseName.c_str(),
        mPort,
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
    : QueryResult(fieldCount, rowCount), mResult(res)
{
    mCurrentRow = std::make_unique<Field[]>(fieldCount);
}

MySQLQueryResult::~MySQLQueryResult()
{
    if (mResult != nullptr)
        mysql_free_result(mResult);
}

bool MySQLQueryResult::NextRow()
{
    MYSQL_ROW row = mysql_fetch_row(mResult);
    if (row == nullptr)
        return false;

    unsigned long* lengths = mysql_fetch_lengths(mResult);

    for (uint32_t i = 0; i < mFieldCount; ++i)
    {
        const std::size_t fieldLength = lengths != nullptr ? static_cast<std::size_t>(lengths[i]) : 0U;
        mCurrentRow[i].setValue(row[i], fieldLength);
    }

    return true;
}

std::unique_ptr<QueryResult> MySQLDatabase::_StoreQueryResult(DatabaseConnection* con)
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
    queryResult->NextRow();
    return queryResult;
}
