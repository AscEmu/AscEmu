/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/Summons/Summon.h"

class TotemSummon : public Summon
{
public:
    TotemSummon(uint64_t guid, uint32_t duration);
    ~TotemSummon();

    void Load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector& position, uint32_t spellId, int32_t summonSlot) override;

    void unSummon() override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Object functions
    void OnPushToWorld() override;
    void OnPreRemoveFromWorld() override;
    bool isTotem() const override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Unit functions
    void Die(Unit* /*pAttacker*/, uint32_t /*damage*/, uint32_t /*spellid*/) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    //\brief: Sets up the spells the totem will cast. This code was almost directly copied
    //        from SpellEffects.cpp, it requires further refactoring!
    //        For example totems should cast like other units..
    void SetupSpells();
};
