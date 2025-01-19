/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "SummonDefines.hpp"

#include <array>
#include <vector>

class Pet;
class Summon;
class TotemSummon;
class Unit;
enum UnitSpeedType : uint8_t;

class SERVER_DECL SummonHandler
{
public:
    SummonHandler(Unit* owner);
    ~SummonHandler();

    SummonHandler& operator=(SummonHandler const&);

    // Adds summon to interface
    // Should only be called from Summon::OnPushToWorld
    void addSummonToHandler(Summon* summon);
    // Removes summon from interface
    // Should only be called from Summon::unSummon
    void removeSummonFromHandler(Summon* summon);

    Pet* getPet() const;
    std::vector<Summon*> const& getSummons() const;

    // Unsummons all pets, guardians and totems
    void removeAllSummons();

    void setPvPFlags(bool set);
    void setFFAPvPFlags(bool set);
    void setSanctuaryFlags(bool set);

    void setPhase(uint8_t command, uint32_t newPhase);

    void notifyOnOwnerSpeedChange(UnitSpeedType type, float_t rate, bool current);
    void notifyOnOwnerAttacked(Unit* attacker);
    void notifyOnPetDeath(Summon* pet);

    Summon* getSummonWithEntry(uint32_t entry) const;
    Summon* getSummonInSlot(SummonSlot slot) const;

    bool hasTotemInSlot(SummonSlot slot) const;
    void killAllTotems() const;

private:
    Unit* m_owner = nullptr;

    // Current permanent pet
    Pet* m_pet = nullptr;
    // Summons with slot (i.e. totems)
    // Skip SUMMON_SLOT_NONE
    std::array<Summon*, (MAX_SUMMON_SLOT - 1)> m_summonSlots = { nullptr };
    // All guardians, pets, totems, minipets etc
    // Note; contains also current permanent pet and all summons from m_summonSlots
    // TODO: minipet probably should not be in summons vector?
    std::vector<Summon*> m_summons;
    // TODO: unit summoned gameobjects
};
