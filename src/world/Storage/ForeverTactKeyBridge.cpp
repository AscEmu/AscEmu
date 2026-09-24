/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ForeverTactKeyBridge.hpp"

#include "world/Server/DatabaseDefinition.hpp"

namespace AscEmu::World::Storage
{
    std::vector<ForeverTactKeyRecord> loadForeverTactKeys(uint32_t maxBuild)
    {
        std::vector<ForeverTactKeyRecord> records;

        auto result = WorldDatabase.query(
            "SELECT ID, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, "
            "Key9, Key10, Key11, Key12, Key13, Key14, Key15, Key16, VerifiedBuild "
            "FROM tact_key WHERE VerifiedBuild=0 OR (VerifiedBuild>0 AND VerifiedBuild<=%u) "
            "ORDER BY ID ASC, VerifiedBuild ASC",
            maxBuild);

        if (!result)
            return records;

        do
        {
            Field* fields = result->fetch();

            ForeverTactKeyRecord record;
            record.id = fields[0].asUint32();
            for (size_t index = 0; index < record.key.size(); ++index)
                record.key[index] = fields[index + 1U].asUint8();
            record.verifiedBuild = fields[17].asInt32();

            records.emplace_back(record);
        }
        while (result->nextRow());

        return records;
    }
}
