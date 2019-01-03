/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Units/Creatures/Creature.h"
#include "Units/Summons/Summon.h"


class TotemSummon : public Summon
{
public:

    TotemSummon(uint64_t guid);
    ~TotemSummon();

    void Load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector& position, uint32_t spellId, int32_t summonSlot) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Object functions
    void OnPushToWorld() override;
    void OnPreRemoveFromWorld() override;
    bool isTotem() const override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Unit functions
    void Die(Unit* /*pAttacker*/, uint32 /*damage*/, uint32 /*spellid*/) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    //\brief: Sets up the spells the totem will cast. This code was almost directly copied
    //        from SpellEffects.cpp, it requires further refactoring!
    //        For example totems should cast like other units..
    void SetupSpells();
};
