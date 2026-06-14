/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Field.hpp"
#include "CommonTypes.hpp"
#include <string>
#include <mutex>
#include <vector>
#include <memory>

class QueryResult;
class Database;
class SQLCallbackBase;
class DatabaseRuntime;

struct DatabaseConnection
{
    virtual ~DatabaseConnection() = default;
    std::recursive_mutex Busy;
};

struct SERVER_DECL AsyncQueryResult
{
    std::unique_ptr<QueryResult> result;
    std::string query;
};

class SERVER_DECL AsyncQuery
{
    friend class Database;

    std::unique_ptr<SQLCallbackBase> func;
    std::vector<AsyncQueryResult> queries;
    Database* db;

public:
    AsyncQuery(std::unique_ptr<SQLCallbackBase> f);
    ~AsyncQuery();
    void addQuery(const char* format, ...);
    inline void setDB(Database* dbb) { db = dbb; }
};

class SERVER_DECL QueryBuffer
{
    friend class Database;

    std::vector<std::string> queries;

public:
    void addQuery(const char* format, ...);
    void addQueryNA(const char* str);
    void addQueryStr(const std::string& str);
};

class SERVER_DECL Database
{
    friend class AsyncQuery;
    friend class DatabaseRuntime;

public:
    Database();
    virtual ~Database();

    virtual bool initialize(const char* _hostname, unsigned int _port,
        const char* _username, const char* _password, const char* _databaseName,
        uint32_t _connectionCount, uint32_t _bufferSize, bool _useLegacyAuth = false) = 0;

    virtual void shutdown() = 0;
    void endThreads();

    // synchronus queri api
    virtual std::unique_ptr<QueryResult> query(const char* QueryString, ...);
    virtual std::unique_ptr<QueryResult> query(bool* success, const char* QueryString, ...);
    virtual std::unique_ptr<QueryResult> queryNA(const char* QueryString);
    virtual std::unique_ptr<QueryResult> fQuery(const char* QueryString, DatabaseConnection* con);

    virtual void fWaitExecute(const char* QueryString, DatabaseConnection* con);
    virtual bool waitExecute(const char* QueryString, ...);
    virtual bool waitExecuteNA(const char* QueryString);
    virtual bool execute(const char* QueryString, ...);
    virtual bool executeNA(const char* QueryString);

    // asynch / batching
    void queueAsyncQuery(std::unique_ptr<AsyncQuery> query);
    void addQueryBuffer(std::unique_ptr<QueryBuffer> b);

    // escaping
    virtual std::string escapeString(std::string Escape) = 0;
    virtual void escapeLongString(const char* str, uint32_t len, std::stringstream& out) = 0;
    virtual std::string escapeString(const char* esc, DatabaseConnection* con) = 0;

    // capabilities
    virtual bool supportsReplaceInto() = 0;
    virtual bool supportsTableLocking() = 0;

    // metrics
    [[nodiscard]] size_t getQueuedTaskCount() const;
    [[nodiscard]] size_t getWorkerCount() const;
    [[nodiscard]] uint64_t getCompletedTaskCount() const;

    // meta
    const std::string& getHostName() { return m_hostname; }
    const std::string& getDatabaseName() { return m_databaseName; }

    // factory
    static std::unique_ptr<Database> createDatabaseInterface();
    static void cleanupLibs();

protected:
    void _initialize();

    // backend hooks
    virtual void _beginTransaction(DatabaseConnection* conn) = 0;
    virtual void _endTransaction(DatabaseConnection* conn) = 0;
    virtual bool _sendQuery(DatabaseConnection* con, const char* Sql, bool Self) = 0;
    virtual std::unique_ptr<QueryResult> _storeQueryResult(DatabaseConnection* con) = 0;

    // runtime state
    std::unique_ptr<DatabaseRuntime> m_runtime;
    std::vector<std::unique_ptr<DatabaseConnection>> m_connections;

    // meta
    std::string m_hostname;
    std::string m_username;
    std::string m_password;
    std::string m_databaseName;
    uint32_t m_port = 3306;

    int32_t m_connectionCount = -1;
    uint32_t m_counter = 0;
};

class SERVER_DECL QueryResult
{
public:
    QueryResult(uint32_t fields, uint32_t rows) : m_fieldCount(fields), m_rowCount(rows), m_currentRow(nullptr) {}
    virtual ~QueryResult() {}

    virtual bool nextRow() = 0;

    inline Field* fetch() { return m_currentRow.get(); }
    inline uint32_t getFieldCount() const { return m_fieldCount; }
    inline uint32_t getRowCount() const { return m_rowCount; }

protected:
    uint32_t m_fieldCount;
    uint32_t m_rowCount;
    std::unique_ptr<Field[]> m_currentRow;
};
