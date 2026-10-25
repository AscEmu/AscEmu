/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/ByteBuffer.hpp"

#include <cstdint>
#include <string>
#include <vector>

// Account wide battle pet journal in the logonserver: REQUEST loads, ADD creates with a logon id, SAVE writes changes
namespace LogonComm
{
    struct BattlePetRecord
    {
        uint32_t id = 0;                    // unique in the account
        uint32_t species = 0;
        uint16_t breed = 0;
        uint32_t displayId = 0;
        uint16_t level = 1;
        uint16_t experience = 0;
        uint32_t health = 0;
        uint8_t quality = 0;
        uint16_t flags = 0;
        std::string name;
        uint64_t nameTimestamp = 0;
    };

    // result of LRCMSG_BATTLE_PET_ADD
    enum BattlePetAddResult : uint8_t
    {
        BattlePetAddOk                  = 0,
        BattlePetAddTooManyOfSpecies    = 1,
        BattlePetAddTooManyPets         = 2,
        BattlePetAddFailed              = 3
    };

    struct BattlePetSlotRecord
    {
        uint8_t index = 0;
        uint32_t petId = 0;
        bool locked = true;
    };

    inline void writeBattlePet(ByteBuffer& buffer, BattlePetRecord const& pet)
    {
        buffer << pet.id << pet.species << pet.breed << pet.displayId << pet.level << pet.experience;
        buffer << pet.health << pet.quality << pet.flags << pet.name << pet.nameTimestamp;
    }

    inline BattlePetRecord readBattlePet(ByteBuffer& buffer)
    {
        BattlePetRecord pet;
        buffer >> pet.id >> pet.species >> pet.breed >> pet.displayId >> pet.level >> pet.experience;
        buffer >> pet.health >> pet.quality >> pet.flags >> pet.name >> pet.nameTimestamp;
        return pet;
    }

    inline void writeBattlePetSlots(ByteBuffer& buffer, std::vector<BattlePetSlotRecord> const& slots)
    {
        buffer << uint8_t(slots.size());
        for (const auto& slot : slots)
            buffer << slot.index << slot.petId << uint8_t(slot.locked ? 1 : 0);
    }

    inline std::vector<BattlePetSlotRecord> readBattlePetSlots(ByteBuffer& buffer)
    {
        uint8_t count = 0;
        buffer >> count;

        std::vector<BattlePetSlotRecord> slots(count);
        for (auto& slot : slots)
        {
            uint8_t locked = 0;
            buffer >> slot.index >> slot.petId >> locked;
            slot.locked = locked != 0;
        }
        return slots;
    }
}
