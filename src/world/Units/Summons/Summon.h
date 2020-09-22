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
    Summon(uint64_t guid, uint32_t duration);
    ~Summon();

    virtual void Load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector& position, uint32_t spellId, int32_t summonSlot);

    virtual void unSummon();

    uint32_t getTimeLeft() const;
    void setTimeLeft(uint32_t time);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Object functions
    void OnPushToWorld() override;
    void OnPreRemoveFromWorld() override;
    bool isSummon() const override;
    void onRemoveInRangeObject(Object* object) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Unit functions
    void Die(Unit* pAttacker, uint32 damage, uint32 spellid) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
protected:
    int32_t m_summonSlot = -1;
    Unit* m_unitOwner = nullptr;

    uint32_t m_duration = 0;

public:
    bool isSummonedToSlot() const;

    Unit* getUnitOwner() const { return m_unitOwner; }
    Player* getPlayerOwner() override;
};
