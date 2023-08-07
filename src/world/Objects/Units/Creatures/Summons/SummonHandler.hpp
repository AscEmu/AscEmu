/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "SummonDefines.hpp"

#include <vector>

class Summon;
class TotemSummon;
class Unit;

class SERVER_DECL SummonHandler
{
public:
    SummonHandler();
    ~SummonHandler();

    void removeAllSummons(bool totemsOnly = false);

    void setPvPFlags(bool set);
    void setFFAPvPFlags(bool set);
    void setSanctuaryFlags(bool set);

    bool hasTotemInSlot(SummonSlot slot) const;
    TotemSummon* getTotemInSlot(SummonSlot slot) const;
    Summon* getSummonWithEntry(uint32_t entry) const;

    void getTotemSpellIds(std::vector<uint32_t> &spellIds);

    uint32_t m_SummonSlot[MAX_SUMMON_SLOT];

    void setUnitOwner(Unit* owner);

protected:
    Unit* m_Owner = nullptr;
};
