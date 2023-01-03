/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Objects/Units/UnitDefines.hpp"

#include <vector>
#include <set>

class Summon;
class TotemSummon;

// There are 3 types of summons
// - Combat pet : this is a controllable pet. Hunter pets and warlock minions for example.
// - Guardian pet : this pet cannot be controlled and they act like combat pet would with aggressive react state. Totem is a subtype of guardian.
// - Non-Combat pet : this is known as vanity pet, companion pet or critter. Cannot be attacked or attack.

// There can be only one combat pet and one non-combat pet. Guardian pets have no limits.
// Technically Death Knight can have 17 summons at time; 10 ghouls from Army of the Dead, 1 permanent risen ghoul,
// 4 bloodworms, 1 dancing rune weapon and a companion pet.

//////////////////////////////////////////////////////////////////////////////////////////
// Manages all summons for Unit class
class SERVER_DECL SummonHandler
{
public:
    SummonHandler();
    ~SummonHandler();

    // Removes all guardians and totems, but not permanent pet
    void removeAllSummons(bool totemsOnly = false);

    void setPvPFlags(bool set);
    void setFFAPvPFlags(bool set);
    void setSanctuaryFlags(bool set);

    bool hasTotemInSlot(SummonSlot slot) const;
    TotemSummon* getTotemInSlot(SummonSlot slot) const;
    Summon* getSummonWithEntry(uint32_t entry) const;

    void getTotemSpellIds(std::vector<uint32_t> &spellIds);

    void Init(Unit* owner);

    uint32_t m_SummonSlot[MAX_SUMMON_SLOT];

protected:
    Unit* m_Owner = nullptr;
};
