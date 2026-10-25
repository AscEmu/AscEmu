/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "LogonCommBattlePets.hpp"
#include "WoWGuid.hpp"

#include <cstdint>

namespace BattlePets
{
    constexpr uint16_t MaxLevel = 25;
    constexpr uint8_t SlotCount = 3;
    constexpr uint8_t MaxPetsPerSpecies = 3;
    constexpr uint32_t MaxPets = 1000;
    constexpr uint32_t MaxNameLength = 16;
    constexpr uint16_t DefaultBreed = 3;                // the balanced breed of a species without breed data

    // the summon spell of a species without its own spell, the creature comes from the species
    constexpr uint32_t SpellSummonBattlePet = 118301;

    enum class Quality : uint8_t
    {
        Poor        = 0,
        Common      = 1,
        Uncommon    = 2,
        Rare        = 3,
        Epic        = 4,
        Legendary   = 5,
        Count
    };

    // BattlePetSpecies flags, the same bits since 5.x
    enum SpeciesFlags : uint32_t
    {
        SPECIES_FLAG_NO_RENAME              = 0x00001,
        SPECIES_FLAG_WELL_KNOWN             = 0x00002,  // can be learned
        SPECIES_FLAG_NOT_ACCOUNT_WIDE       = 0x00004,
        SPECIES_FLAG_CAPTURABLE             = 0x00008,
        SPECIES_FLAG_NOT_TRADABLE           = 0x00010,
        SPECIES_FLAG_HIDE_FROM_JOURNAL      = 0x00020,
        SPECIES_FLAG_LEGACY_ACCOUNT_UNIQUE  = 0x00040,  // one pet of the species per account
        SPECIES_FLAG_CANT_BATTLE            = 0x00080,
        SPECIES_FLAG_HORDE_ONLY             = 0x00100,
        SPECIES_FLAG_ALLIANCE_ONLY          = 0x00200,
        SPECIES_FLAG_BOSS                   = 0x00400,
        SPECIES_FLAG_RANDOM_DISPLAY         = 0x00800
    };

    // flags of a pet in the journal
    enum PetFlags : uint16_t
    {
        PET_FLAG_FAVORITE                   = 0x001,
        PET_FLAG_CONVERTED                  = 0x002,
        PET_FLAG_REVOKED                    = 0x004,
        PET_FLAG_LOCKED_FOR_CONVERT         = 0x008,
        PET_FLAG_ABILITY_0_SELECTION        = 0x010,
        PET_FLAG_ABILITY_1_SELECTION        = 0x020,
        PET_FLAG_ABILITY_2_SELECTION        = 0x040,
        PET_FLAG_FANFARE_NEEDED             = 0x080,
        PET_FLAG_DISPLAY_OVERRIDDEN         = 0x100,

        // the flags a client may change
        PET_FLAGS_CLIENT                    = PET_FLAG_FAVORITE | PET_FLAG_ABILITY_0_SELECTION | PET_FLAG_ABILITY_1_SELECTION | PET_FLAG_ABILITY_2_SELECTION
    };

    // BattlePetState ids of the stats
    enum State : uint32_t
    {
        STATE_STAT_POWER                    = 18,
        STATE_STAT_STAMINA                  = 19,
        STATE_STAT_SPEED                    = 20
    };

    enum class Error : uint8_t
    {
        CantHaveMorePetsOfType  = 3,
        CantHaveMorePets        = 4,
        TooHighLevelToUncage    = 7
    };

    // a pet of the journal: the stored record and the values this realm derives from its client data
    struct BattlePet
    {
        LogonComm::BattlePetRecord record;
        bool known = false;                             // the species exists in the client data of this realm
        uint32_t creatureId = 0;
        uint32_t displayId = 0;                         // the stored display when this realm has it, else one of its own
        uint32_t power = 0;
        uint32_t maxHealth = 0;
        uint32_t speed = 0;
        bool noRename = false;
    };

    // the server guid of a pet: its id for 5.x, a typed guid the 6.x and later clients receive as 128 bit guid
    inline uint64_t petGuid(uint32_t id)
    {
#if VERSION_STRING >= WoD
        return id != 0 ? WoWGuid(id, 0, HIGHGUID_TYPE_BATTLE_PET).getRawGuid() : 0;
#else
        return id;
#endif
    }
}
