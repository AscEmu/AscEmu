/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Instance_TheVioletHold.hpp"
#include "Movement/MovementManager.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Azure Saboteur AI
class AzureSaboteurAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit AzureSaboteurAI(Creature* pCreature);

    void OnSummon(Unit* /*summoner*/) override;

    void AIUpdate(unsigned long /*time_passed*/) override;
    void OnReachWP(uint32_t /*type*/, uint32_t /*id*/) override;

    template <size_t N>
    void startSmoothPath(LocationVector const (&path)[N]) { getCreature()->getMovementManager()->moveSmoothPath(POINT_INTRO, &path[0], N, false); }

    void startMovement();

protected:
    InstanceScript* mInstance;
    uint32_t mbossId;
    uint8_t castCounter = 0;
};
