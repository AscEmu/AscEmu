/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>

class Database;

class DatabaseUpdater
{
public:
    void static initBaseIfNeeded(const std::string& dbName, const std::string& dbBaseType, Database& dbPointer);

    void static checkAndApplyDBUpdatesIfNeeded(const std::string& database, Database& dbPointer);

private:
    void static setupDatabase(const std::string& database,  Database& dbPointer);
    void static applyUpdatesForDatabase(const std::string& database, Database& dbPointer);
};
