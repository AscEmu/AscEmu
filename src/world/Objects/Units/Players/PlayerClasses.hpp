/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Player.hpp"

enum SPELL_RUNE_TYPES
{
    RUNE_BLOOD = 0,
    RUNE_FROST = 1,
    RUNE_UNHOLY = 2,
    RUNE_DEATH = 3,
    RUNE_MAX_TYPES = 4
};

const uint8_t base_runes[MAX_RUNES] = { RUNE_BLOOD, RUNE_BLOOD, RUNE_FROST, RUNE_FROST, RUNE_UNHOLY, RUNE_UNHOLY };

struct Rune
{
    uint8_t type;
    bool is_used;
};

class DeathKnight : public Player
{
    Rune m_runes[MAX_RUNES];

    // Holds last slot used
    uint8_t m_last_used_rune_slot = 0;

protected:
    void SendRuneUpdate(uint8_t slot);

public:
    explicit DeathKnight(uint32_t guid) : Player(guid)
    {
        for (uint8_t i = 0; i < MAX_RUNES; ++i)
        {
            m_runes[i].type = base_runes[i];
            m_runes[i].is_used = false;
        }
    }

    bool isClassDeathKnight() const override { return true; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Runes

    uint8_t GetBaseRuneType(uint8_t slot);
    uint8_t GetRuneType(uint8_t slot);
    bool GetRuneIsUsed(uint8_t slot);
    void ConvertRune(uint8_t slot, uint8_t type);
    uint32_t HasRunes(uint8_t type, uint32_t count);
    uint32_t TakeRunes(uint8_t type, uint32_t count);
    void ResetRune(uint8_t slot);
    uint8_t GetRuneFlags();
    bool IsAllRunesOfTypeInUse(uint8_t type);
    uint8_t GetLastUsedUnitSlot() { return m_last_used_rune_slot; }
};

class Druid : public Player
{
public:
    explicit Druid(uint32_t guid) : Player(guid) {}

    bool isClassDruid() const override { return true; }
};

class Rogue : public Player
{
public:
    explicit Rogue(uint32_t guid) : Player(guid) {}

    bool isClassRogue() const override { return true; }
};

class Priest : public Player
{
public:
    explicit Priest(uint32_t guid) : Player(guid) {}

    bool isClassPriest() const override { return true; }
};

class Paladin : public Player
{
public:
    explicit Paladin(uint32_t guid) : Player(guid) {}

    bool isClassPaladin() const override { return true; }
};

class Warrior : public Player
{
public:
    explicit Warrior(uint32_t guid) : Player(guid) {}

    bool isClassWarrior() const override { return true; }
};

class Warlock : public Player
{
public:
    explicit Warlock(uint32_t guid) : Player(guid) {}

    bool isClassWarlock() const override { return true; }
};

class Mage : public Player
{
public:
    explicit Mage(uint32_t guid) : Player(guid) {}

    bool isClassMage() const override { return true; }
};

class Monk : public Player
{
public:
    explicit Monk(uint32_t guid) : Player(guid) {}

    bool isClassMonk() const override { return true; }
};

class Hunter : public Player
{
public:
    explicit Hunter(uint32_t guid) : Player(guid) {}

    bool isClassHunter() const override { return true; }
};

class Shaman : public Player
{
public:
    explicit Shaman(uint32_t guid) : Player(guid) {}

    bool isClassShaman() const override { return true; }
};
