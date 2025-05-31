/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
 *
 */

#include "MySQLDatabase.h"

#include <sstream>

#include "Logging/Logger.hpp"

MySQLDatabase::~MySQLDatabase()
{
    for (const auto& conn : Connections)
    {
        if (const auto mysqlConn = dynamic_cast<MySQLDatabaseConnection const*>(conn.get()))
            mysql_close(mysqlConn->MySql);
    }
}

MySQLDatabase::MySQLDatabase() : Database()
{

}

void MySQLDatabase::_BeginTransaction(DatabaseConnection* conn)
{
    _SendQuery(conn, "START TRANSACTION", false);
}

void MySQLDatabase::_EndTransaction(DatabaseConnection* conn)
{
    _SendQuery(conn, "COMMIT", false);
}

bool MySQLDatabase::Initialize(const char* Hostname, unsigned int port, const char* Username, const char* Password, const char* DatabaseName, uint32_t ConnectionCount, uint32_t /*BufferSize*/, bool useLegacyAuth)
{
    uint32_t i;
    MYSQL* temp = NULL;
    MYSQL* temp2 = NULL;
    bool my_true = true;

    mHostname = std::string(Hostname);
    mConnectionCount = ConnectionCount;
    mUsername = std::string(Username);
    mPassword = std::string(Password);
    mDatabaseName = std::string(DatabaseName);

    sLogger.info("MySQLDatabase : Connecting to `{}`, database `{}`...", Hostname, DatabaseName);

    Connections.reserve(ConnectionCount);
    for(i = 0; i < ConnectionCount; ++i)
    {
        temp = mysql_init(NULL);
        if(temp == NULL)
            continue;

        if(mysql_options(temp, MYSQL_SET_CHARSET_NAME, "utf8"))
            sLogger.failure("Could not set utf8 character set.");

        if(mysql_options(temp, MYSQL_OPT_RECONNECT, &my_true))
            sLogger.failure("MYSQL_OPT_RECONNECT could not be set, connection drops may occur but will be counteracted.");

        // Check if we want to use MySQL 8 legacy authentication otherwise use new auth
        if (!useLegacyAuth)
        {
            // Set authentication to caching_sha2_password (new method)
            if (mysql_options(temp, MYSQL_DEFAULT_AUTH, "caching_sha2_password"))
            {
                sLogger.failure("Could not set default authentication plugin to caching_sha2_password.");
                mysql_close(temp);
                return false;
            }
            sLogger.info("Using new MySQL 8 authentication method (caching_sha2_password).");
        }
        else
        {
            // No explicit authentication setting; use legacy authentication
            sLogger.info("Using legacy MySQL authentication method.");
        }

        temp2 = mysql_real_connect(temp, Hostname, Username, Password, DatabaseName, port, NULL, 0);
        if(temp2 == NULL)
        {
            sLogger.failure("Connection failed due to: `{}`", mysql_error(temp));
            mysql_close(temp);
            return false;
        }

        Connections.emplace_back(std::make_unique<MySQLDatabaseConnection>(temp2));
    }

    Database::_Initialize();
    return true;
}

std::string MySQLDatabase::EscapeString(std::string Escape)
{
    char a2[16384] = { 0 };

    DatabaseConnection* con = GetFreeConnection();
    std::string ret;
    if(mysql_real_escape_string(static_cast<MySQLDatabaseConnection*>(con)->MySql, a2, Escape.c_str(), (unsigned long)Escape.length()) == 0)
        ret = std::move(Escape);
    else
        ret = a2;

    con->Busy.release();

    return std::string(ret);
}

void MySQLDatabase::EscapeLongString(const char* str, uint32_t len, std::stringstream & out)
{
    char a2[65536 * 3] = { 0 };

    DatabaseConnection* con = GetFreeConnection();
    const char* ret;
    if(mysql_real_escape_string(static_cast<MySQLDatabaseConnection*>(con)->MySql, a2, str, (unsigned long)len) == 0)
        ret = str;
    else
        ret = a2;

    out.write(a2, (std::streamsize)strlen(a2));
    con->Busy.release();
}

std::string MySQLDatabase::EscapeString(const char* esc, DatabaseConnection* con)
{
    char a2[16384] = { 0 };
    const char* ret;
    if(mysql_real_escape_string(static_cast<MySQLDatabaseConnection*>(con)->MySql, a2, (char*)esc, (unsigned long)strlen(esc)) == 0)
        ret = esc;
    else
        ret = a2;

    return std::string(ret);
}

void MySQLDatabase::Shutdown()
{
    mysql_library_end();
}

bool MySQLDatabase::_SendQuery(DatabaseConnection* con, const char* Sql, bool Self)
{
    //dunno what it does ...leaving untouched
    int result = mysql_query(static_cast<MySQLDatabaseConnection*>(con)->MySql, Sql);
    if(result > 0)
    {
        if(Self == false && _HandleError(static_cast<MySQLDatabaseConnection*>(con), mysql_errno(static_cast<MySQLDatabaseConnection*>(con)->MySql)))
        {
            // Re-send the query, the connection was successful.
            // The true on the end will prevent an endless loop here, as it will
            // stop after sending the query twice.
            result = _SendQuery(con, Sql, true);
        }
        else
            sLogger.failure("Sql query failed due to [{}], Query: [{}]", mysql_error(static_cast<MySQLDatabaseConnection*>(con)->MySql), Sql);
    }

    return (result == 0 ? true : false);
}

bool MySQLDatabase::_HandleError(MySQLDatabaseConnection* con, uint32_t ErrorNumber)
{
    // Handle errors that should cause a reconnect to the Database.
    switch(ErrorNumber)
    {
        case 2006:  // Mysql server has gone away
        case 2008:  // Client ran out of memory
        case 2013:  // Lost connection to sql server during query
        case 2055:  // Lost connection to sql server - system error
            {
                // Let's instruct a reconnect to the db when we encounter these errors.
                return _Reconnect(con);
            }
            break;
    }

    return false;
}

MySQLQueryResult::MySQLQueryResult(MYSQL_RES* res, uint32_t FieldCount, uint32_t RowCount) : QueryResult(FieldCount, RowCount), mResult(res)
{
    mCurrentRow = std::make_unique<Field[]>(FieldCount);
}

MySQLQueryResult::~MySQLQueryResult()
{
    mysql_free_result(mResult);
}

bool MySQLQueryResult::NextRow()
{
    MYSQL_ROW row = mysql_fetch_row(mResult);
    if(row == NULL)
        return false;

    for(uint32_t i = 0; i < mFieldCount; ++i)
        mCurrentRow[i].setValue(row[i]);

    return true;
}

std::unique_ptr<QueryResult> MySQLDatabase::_StoreQueryResult(DatabaseConnection* con)
{
    MySQLDatabaseConnection* db = static_cast<MySQLDatabaseConnection*>(con);
    MYSQL_RES* pRes = mysql_store_result(db->MySql);
    uint32_t uRows = (uint32_t)mysql_affected_rows(db->MySql);
    uint32_t uFields = (uint32_t)mysql_field_count(db->MySql);

    if(uRows == 0 || uFields == 0 || pRes == 0)
    {
        if(pRes != NULL)
            mysql_free_result(pRes);

        return NULL;
    }

    auto res = std::make_unique<MySQLQueryResult>(pRes, uFields, uRows);
    res->NextRow();

    return res;
}

bool MySQLDatabase::_Reconnect(MySQLDatabaseConnection* conn)
{
    MYSQL* temp, *temp2;

    temp = mysql_init(NULL);
    temp2 = mysql_real_connect(temp, mHostname.c_str(), mUsername.c_str(), mPassword.c_str(), mDatabaseName.c_str(), mPort, NULL , 0);
    if(temp2 == NULL)
    {
        sLogger.failure("Could not reconnect to database because of `{}`", mysql_error(temp));
        mysql_close(temp);
        return false;
    }

    if(conn->MySql != NULL)
        mysql_close(conn->MySql);

    conn->MySql = temp;
    return true;
}
