/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../Database.h"

#include <string>
#include <vector>

namespace AscEmu::AE::DbSelfTest
{
    struct Result
    {
        bool success = false;
        std::vector<std::string> messages;
    };

    Result run(::Database& db, const std::string& tablePrefix = "ae_db_selftest");
}
