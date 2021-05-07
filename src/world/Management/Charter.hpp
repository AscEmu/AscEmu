/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "Units/Players/PlayerDefines.hpp"

class Field;

class Charter
{
public:

    uint32_t GetNumberOfSlotsByType()
    {
        switch (CharterType)
        {
        case CHARTER_TYPE_GUILD:
            return 9;

        case CHARTER_TYPE_ARENA_2V2:
            return 1;

        case CHARTER_TYPE_ARENA_3V3:
            return 2;

        case CHARTER_TYPE_ARENA_5V5:
            return 4;

        default:
            return 9;
        }
    }

    uint32_t SignatureCount;
    uint32_t* Signatures;
    uint8_t CharterType;
    uint32_t Slots;
    uint32_t LeaderGuid;
    uint64_t ItemGuid;
    uint32_t CharterId;
    std::string GuildName;

    /************************************************************************/
    /* Developer Fields                                                     */
    /************************************************************************/
    uint32_t PetitionSignerCount;

    Charter(Field* fields);
    Charter(uint32_t id, uint32_t leader, uint8_t type) : CharterType(type), LeaderGuid(leader), CharterId(id)
    {
        SignatureCount = 0;
        ItemGuid = 0;
        Slots = GetNumberOfSlotsByType();
        Signatures = new uint32_t[Slots];
        memset(Signatures, 0, sizeof(uint32_t) * Slots);
        PetitionSignerCount = 0;
    }

    ~Charter()
    {
        delete[] Signatures;
    }

    void SaveToDB();
    void Destroy();         // When item is deleted.

    void AddSignature(uint32_t PlayerGuid);
    void RemoveSignature(uint32_t PlayerGuid);

    uint32_t GetLeader() { return LeaderGuid; }
    uint32_t GetID() { return CharterId; }

    bool IsFull() { return (SignatureCount == Slots); }
};
