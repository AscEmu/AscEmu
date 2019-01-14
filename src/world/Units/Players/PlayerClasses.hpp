/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Player.h"

#define TOTAL_NORMAL_RUNE_TYPES 3
#define TOTAL_USED_RUNES (TOTAL_NORMAL_RUNE_TYPES * 2)
#define MAX_RUNES 6
#define TOTAL_RUNE_TYPES 4
#define MAX_RUNE_VALUE 1

enum SPELL_RUNE_TYPES
{
    RUNE_BLOOD = 0,
    RUNE_FROST = 1,
    RUNE_UNHOLY = 2,
    RUNE_DEATH = 3,
    RUNE_MAX_TYPES = 4
};

const uint8 base_runes[MAX_RUNES] = { RUNE_BLOOD, RUNE_BLOOD, RUNE_FROST, RUNE_FROST, RUNE_UNHOLY, RUNE_UNHOLY };

struct Rune
{
    uint8 type;
    bool is_used;
};

class DeathKnight : public Player
{
    Rune m_runes[MAX_RUNES];

    // Holds last slot used
    uint8 m_last_used_rune_slot;

protected:

    void SendRuneUpdate(uint8 slot);

public:

    explicit DeathKnight(uint32 guid) : Player(guid)
    {
        m_last_used_rune_slot = 0;
        for (uint8 i = 0; i < MAX_RUNES; ++i)
        {
            m_runes[i].type = base_runes[i];
            m_runes[i].is_used = false;
        }
    }

    bool isClassDeathKnight() override { return true; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Runes

    uint8 GetBaseRuneType(uint8 slot);
    uint8 GetRuneType(uint8 slot);
    bool GetRuneIsUsed(uint8 slot);
    void ConvertRune(uint8 slot, uint8 type);
    uint32 HasRunes(uint8 type, uint32 count);
    uint32 TakeRunes(uint8 type, uint32 count);
    void ResetRune(uint8 slot);
    uint8 GetRuneFlags();
    bool IsAllRunesOfTypeInUse(uint8 type);
    uint8 GetLastUsedUnitSlot() { return m_last_used_rune_slot; }
};

class Druid : public Player
{
public:

    explicit Druid(uint32_t guid) : Player(guid) {}

    bool isClassDruid() override { return true; }
};

class Rogue : public Player
{
public:

    explicit Rogue(uint32_t guid) : Player(guid) {}

    bool isClassRogue() override { return true; }
};

class Priest : public Player
{
public:

    explicit Priest(uint32_t guid) : Player(guid) {}

    bool isClassPriest() override { return true; }
};

class Paladin : public Player
{
public:

    explicit Paladin(uint32_t guid) : Player(guid) {}

    bool isClassPaladin() override { return true; }
};

class Warrior : public Player
{
public:

    explicit Warrior(uint32_t guid) : Player(guid) {}

    bool isClassWarrior() override { return true; }
};

class Warlock : public Player
{
public:

    explicit Warlock(uint32_t guid) : Player(guid) {}

    bool isClassWarlock() override { return true; }
};

class Mage : public Player
{
public:

    explicit Mage(uint32_t guid) : Player(guid) {}

    bool isClassMage() override { return true; }
};

class Hunter : public Player
{
public:

    explicit Hunter(uint32_t guid) : Player(guid) {}

    bool isClassHunter() override { return true; }
};

class Shaman : public Player
{
public:

    explicit Shaman(uint32_t guid) : Player(guid) {}

    bool isClassShaman() override { return true; }
};
