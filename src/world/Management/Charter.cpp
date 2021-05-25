/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Charter.hpp"
#include "Objects/ObjectMgr.h"

#include "shared/Database/Field.hpp"

Charter::Charter(Field* fields)
{
    uint32_t f = 0;
    CharterId = fields[f++].GetUInt32();
    CharterType = fields[f++].GetUInt8();
    LeaderGuid = fields[f++].GetUInt32();
    GuildName = fields[f++].GetString();
    ItemGuid = fields[f++].GetUInt64();
    SignatureCount = 0;
    Slots = GetNumberOfSlotsByType();
    Signatures = new uint32_t[Slots];

    for (uint32_t i = 0; i < Slots; ++i)
    {
        Signatures[i] = fields[f++].GetUInt32();
        if (Signatures[i])
            ++SignatureCount;
    }

    PetitionSignerCount = 0;
}

void Charter::AddSignature(uint32_t PlayerGuid)
{
    if (SignatureCount >= Slots)
        return;

    ++SignatureCount;
    uint32_t i = 0;
    for (; i < Slots; ++i)
    {
        if (Signatures[i] == 0)
        {
            Signatures[i] = PlayerGuid;
            break;
        }
    }

    ARCEMU_ASSERT(i != Slots);
}

void Charter::RemoveSignature(uint32_t PlayerGuid)
{
    for (uint32_t i = 0; i < Slots; ++i)
    {
        if (Signatures[i] == PlayerGuid)
        {
            Signatures[i] = 0;
            --SignatureCount;
            SaveToDB();
            break;
        }
    }
}

void Charter::Destroy()
{
    sObjectMgr.RemoveCharter(this);

    CharacterDatabase.Execute("DELETE FROM charters WHERE charterId = %u", CharterId);

    for (uint32_t i = 0; i < Slots; ++i)
    {
        if (!Signatures[i])
            continue;

        Player* p = sObjectMgr.GetPlayer(Signatures[i]);
        if (p != nullptr)
            p->unsetCharter(CharterType);
    }

    delete this;
}

void Charter::SaveToDB()
{
    std::stringstream ss;
    uint32_t i;

    ss << "DELETE FROM charters WHERE charterId = ";
    ss << CharterId;
    ss << ";";

    CharacterDatabase.Execute(ss.str().c_str());

    ss.rdbuf()->str("");

    ss << "INSERT INTO charters VALUES(" << CharterId << "," << CharterType << "," << LeaderGuid << ",'" << GuildName << "'," << ItemGuid;

    for (i = 0; i < Slots; ++i)
        ss << "," << Signatures[i];

    for (; i < 9; ++i)
        ss << ",0";

    ss << ")";
    CharacterDatabase.Execute(ss.str().c_str());
}
