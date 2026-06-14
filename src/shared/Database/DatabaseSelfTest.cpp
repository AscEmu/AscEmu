/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "DatabaseSelfTest.hpp"

#include "../Logging/Logger.hpp"
#include "../Utilities/CallBack.h"
#include "RowView.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace AscEmu::AE::DbSelfTest
{
    namespace
    {
        using Clock = std::chrono::steady_clock;

        struct TestRow
        {
            uint32_t id = 0;
            uint32_t value = 0;
            std::string name;
        };

        class AsyncQueryProbe final : public SQLCallbackBase
        {
        public:
            std::future<bool> getFuture()
            {
                return m_promise.get_future();
            }

            void run(QueryResultVector& result) override
            {
                bool ok = false;

                if (!result.empty() && result[0].result)
                {
                    Field* fields = result[0].result->fetch();
                    ok = fields != nullptr && fields[0].asUint32() == 42U;
                }

                m_promise.set_value(ok);
            }

        private:
            std::promise<bool> m_promise;
        };

        void addMessage(Result& result, std::string message)
        {
            result.messages.push_back(std::move(message));
        }

        bool waitUntil(const std::function<bool()>& predicate,
            std::chrono::milliseconds timeout,
            std::chrono::milliseconds pollInterval = std::chrono::milliseconds(20))
        {
            const auto deadline = Clock::now() + timeout;

            while (Clock::now() < deadline)
            {
                if (predicate())
                    return true;

                std::this_thread::sleep_for(pollInterval);
            }

            return predicate();
        }

        bool exec(::Database& db, Result& result, const std::string& sql)
        {
            const bool ok = db.waitExecuteNA(sql.c_str());
            if (!ok)
                addMessage(result, "FAIL exec: " + sql);

            return ok;
        }

        std::optional<uint64_t> queryUint64(::Database& db, Result& result, const std::string& sql)
        {
            auto qr = db.queryNA(sql.c_str());
            if (!qr)
            {
                addMessage(result, "FAIL query: " + sql);
                return std::nullopt;
            }

            Field* fields = qr->fetch();
            if (fields == nullptr)
            {
                addMessage(result, "FAIL query returned no fields: " + sql);
                return std::nullopt;
            }

            return fields[0].asUint64();
        }

        std::string makeTableName(const std::string& prefix)
        {
            const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                Clock::now().time_since_epoch()).count();

            std::ostringstream name;
            name << prefix << "_" << now;
            return name.str();
        }

        bool createTestTable(::Database& db, Result& result, const std::string& tableName)
        {
            return exec(
                db,
                result,
                "CREATE TABLE " + tableName + " ("
                "id INT UNSIGNED NOT NULL PRIMARY KEY, "
                "value INT UNSIGNED NOT NULL, "
                "name VARCHAR(64) NOT NULL"
                ") ENGINE=InnoDB");
        }

        bool seedRows(::Database& db, Result& result, const std::string& tableName)
        {
            return exec(db, result, "INSERT INTO " + tableName + " (id, value, name) VALUES (1, 10, 'alpha')") &&
                exec(db, result, "INSERT INTO " + tableName + " (id, value, name) VALUES (2, 20, 'beta')") &&
                exec(db, result, "INSERT INTO " + tableName + " (id, value, name) VALUES (3, 30, 'gamma')");
        }

        bool smokeTest(::Database& db, Result& result, const std::string& tableName)
        {
            const auto count = queryUint64(db, result, "SELECT COUNT(*) FROM " + tableName);
            if (!count)
                return false;

            if (*count != 3ULL)
            {
                addMessage(result, "FAIL smoke count expected 3, got " + std::to_string(*count));
                return false;
            }

            addMessage(result, "OK smoke test");
            return true;
        }

        bool rowViewTest(::Database& db, Result& result, const std::string& tableName)
        {
            const auto rows = AscEmu::AE::Database::queryMany<TestRow>(
                db,
                "SELECT id, value, name FROM " + tableName + " ORDER BY id",
                [](const AscEmu::AE::Database::RowView& row)
                {
                    return TestRow{
                        row.get<uint32_t>(0),
                        row.get<uint32_t>(1),
                        row.get<std::string>(2)
                    };
                });

            if (rows.size() != 3)
            {
                addMessage(result, "FAIL rowview expected 3 rows, got " + std::to_string(rows.size()));
                return false;
            }

            if (rows[0].id != 1U || rows[0].value != 10U || rows[0].name != "alpha")
            {
                addMessage(result, "FAIL rowview row[0] mismatch");
                return false;
            }

            addMessage(result, "OK rowview mapping");
            return true;
        }

        bool asyncQueryCallbackTest(::Database& db, Result& result)
        {
            auto callback = std::make_unique<AsyncQueryProbe>();
            auto future = callback->getFuture();

            auto query = std::make_unique<AsyncQuery>(std::move(callback));
            query->addQuery("SELECT 42");

            db.queueAsyncQuery(std::move(query));

            if (future.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
            {
                addMessage(result, "FAIL async callback timeout");
                return false;
            }

            if (!future.get())
            {
                addMessage(result, "FAIL async callback wrong result");
                return false;
            }

            addMessage(result, "OK async callback");
            return true;
        }

        bool batchRollbackTest(::Database& db, Result& result, const std::string& tableName)
        {
            const uint64_t completedBefore = db.getCompletedTaskCount();

            auto buffer = std::make_unique<QueryBuffer>();
            buffer->addQueryNA(("INSERT INTO " + tableName + " (id, value, name) VALUES (100, 1000, 'buffer_a')").c_str());
            buffer->addQueryNA(("INSRT INTO " + tableName + " (id, value, name) VALUES (101, 1001, 'broken')").c_str());
            buffer->addQueryNA(("INSERT INTO " + tableName + " (id, value, name) VALUES (102, 1002, 'buffer_b')").c_str());

            db.addQueryBuffer(std::move(buffer));

            const bool finished = waitUntil(
                [&db, completedBefore]()
                {
                    return db.getCompletedTaskCount() > completedBefore;
                },
                std::chrono::seconds(5));

            if (!finished)
            {
                addMessage(result, "FAIL batch rollback timeout");
                return false;
            }

            const auto count = queryUint64(
                db,
                result,
                "SELECT COUNT(*) FROM " + tableName + " WHERE id IN (100, 101, 102)");

            if (!count)
                return false;

            if (*count != 0ULL)
            {
                addMessage(result, "FAIL batch rollback expected 0 rows, got " + std::to_string(*count));
                return false;
            }

            addMessage(result, "OK batch rollback");
            return true;
        }

        bool parallelReadTest(::Database& db, Result& result)
        {
            const size_t workers = db.getWorkerCount();
            if (workers < 2)
            {
                addMessage(result, "SKIP parallel test: workerCount < 2");
                return true;
            }

            const size_t taskCount = std::min<size_t>(workers * 2, 8);

            auto runSerial = [&db, taskCount]() -> std::chrono::milliseconds
                {
                    const auto start = Clock::now();

                    for (size_t i = 0; i < taskCount; ++i)
                    {
                        auto qr = db.queryNA("SELECT SLEEP(1)");
                        (void)qr;
                    }

                    return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start);
                };

            auto runParallel = [&db, taskCount]() -> std::chrono::milliseconds
                {
                    const auto start = Clock::now();
                    std::vector<std::future<void>> futures;
                    futures.reserve(taskCount);

                    for (size_t i = 0; i < taskCount; ++i)
                    {
                        futures.emplace_back(std::async(std::launch::async, [&db]()
                            {
                                auto qr = db.queryNA("SELECT SLEEP(1)");
                                (void)qr;
                            }));
                    }

                    for (auto& future : futures)
                        future.get();

                    return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start);
                };

            const auto serialTime = runSerial();
            const auto parallelTime = runParallel();

            if (parallelTime >= serialTime)
            {
                addMessage(
                    result,
                    "FAIL parallel reads not faster: serial=" + std::to_string(serialTime.count()) +
                    "ms parallel=" + std::to_string(parallelTime.count()) + "ms");
                return false;
            }

            addMessage(
                result,
                "OK parallel reads: serial=" + std::to_string(serialTime.count()) +
                "ms parallel=" + std::to_string(parallelTime.count()) + "ms");
            return true;
        }
    }

    Result run(::Database& db, const std::string& tablePrefix)
    {
        Result result;
        result.success = false;

        addMessage(
            result,
            "AE_DB metrics: workers=" + std::to_string(db.getWorkerCount()) +
            " queued=" + std::to_string(db.getQueuedTaskCount()) +
            " completed=" + std::to_string(db.getCompletedTaskCount()));

        const std::string tableName = makeTableName(tablePrefix);

        const auto cleanup = [&db, &tableName]()
            {
                db.waitExecuteNA(("DROP TABLE IF EXISTS " + tableName).c_str());
            };

        cleanup();

        bool ok = true;
        ok = createTestTable(db, result, tableName) && ok;
        ok = seedRows(db, result, tableName) && ok;
        ok = smokeTest(db, result, tableName) && ok;
        ok = rowViewTest(db, result, tableName) && ok;
        ok = asyncQueryCallbackTest(db, result) && ok;
        ok = batchRollbackTest(db, result, tableName) && ok;
        ok = parallelReadTest(db, result) && ok;

        cleanup();

        result.success = ok;

        if (ok)
            addMessage(result, "AE_DB self-test PASSED");
        else
            addMessage(result, "AE_DB self-test FAILED");

        addMessage(
            result,
            "AE_DB metrics after test: workers=" + std::to_string(db.getWorkerCount()) +
            " queued=" + std::to_string(db.getQueuedTaskCount()) +
            " completed=" + std::to_string(db.getCompletedTaskCount()));

        return result;
    }
}
