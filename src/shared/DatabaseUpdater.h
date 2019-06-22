/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>
#include "Common.hpp"

class Database;

class DatabaseUpdater
{
public:
    void static initBaseIfNeeded(std::string dbName, std::string dbBaseType, Database& dbPointer);

    void static checkAndApplyDBUpdatesIfNeeded(std::string database, Database& dbPointer);

private:
    void static setupDatabase(std::string database,  Database& dbPointer);
    void static applyUpdatesForDatabase(std::string database, Database& dbPointer);
};
