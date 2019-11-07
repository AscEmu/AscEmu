/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Units/Summons/Summon.h"
#include "Units/Creatures/CreatureDefines.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implements guardians
// Guardians are summons that follow and protect their owner
class GuardianSummon : public Summon
{
public:
    GuardianSummon(uint64_t GUID);
    ~GuardianSummon();

    void Load(CreatureProperties const* properties_, Unit* owner, LocationVector & position, uint32_t spellid, int32_t summonslot);
    void OnPushToWorld();
    void OnPreRemoveFromWorld();
};
