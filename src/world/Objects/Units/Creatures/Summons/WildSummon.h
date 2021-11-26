/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/Summons/Summon.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implement wild summons. Wild summonned creatures don't follow or
// protect their owner, however they can be hostile, and attack (not the owner)
class WildSummon : public Summon
{
public:

    WildSummon(uint64_t GUID, uint32_t duration);
    ~WildSummon();

    void Load(CreatureProperties const* properties_, Unit* owner, LocationVector & position, uint32_t spellid, int32_t summonslot);
    void OnPushToWorld();
    void OnPreRemoveFromWorld();
};
