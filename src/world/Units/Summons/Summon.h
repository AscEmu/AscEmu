/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "SummonDefines.hpp"
#include "Units/Creatures/Creature.h"

class Summon : public Creature
{
public:
    Summon(uint64_t guid);
    ~Summon();

    virtual void Load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector& position, uint32_t spellId, int32_t summonSlot);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Object functions
    void OnPushToWorld() override;
    void OnPreRemoveFromWorld() override;
    bool isSummon() const override;
    void onRemoveInRangeObject(Object* object) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Unit functions
    void Die(Unit* pAttacker, uint32_t damage, uint32_t spellid) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
private:
    int32_t m_summonSlot;
    Unit* m_unitOwner;

public:
    bool isSummonedToSlot() const;

    Unit* getUnitOwner() const { return m_unitOwner; }
    Player* getPlayerOwner() override;
};
