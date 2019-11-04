/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Units/Summons/Summon.h"
#include "Units/Creatures/CreatureDefines.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Class implementing companions/vanity pets/critterpets
// These are totally passive and inattackable, they only serve iCandy purposes
class CompanionSummon : public Summon
{
public:
    CompanionSummon(uint64_t GUID);
    ~CompanionSummon();

    virtual void Load(CreatureProperties const* properties_, Unit* owner, LocationVector& position, uint32_t spellid, int32_t summonslot);

    void OnPushToWorld();
    void OnPreRemoveFromWorld();
};
