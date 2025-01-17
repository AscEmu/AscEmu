/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace Erekem
{
    enum Spells
    {
        SPELL_BLOODLUST         = 54516,
        SPELL_BREAK_BONDS       = 59463,
        SPELL_CHAIN_HEAL        = 54481,
        SPELL_EARTH_SHIELD      = 54479,
        SPELL_EARTH_SHOCK       = 54511,
        SPELL_LIGHTNING_BOLT    = 53044,
        SPELL_STORMSTRIKE       = 51876,
        SPELL_WINDFURY          = 54493
    };

    enum GuardSpells
    {
        SPELL_GUSHING_WOUND     = 39215,
        SPELL_HOWLING_SCREECH   = 54462,
        SPELL_STRIKE            = 14516
    };

    enum Yells
    {
        SAY_AGGRO               = 4524,
        SAY_SLAY1               = 4525,
        SAY_SLAY2               = 4526,
        SAY_SLAY3               = 4527,
        SAY_DEATH               = 4528,
        SAY_SPAWN               = 4529,
        SAY_ADD_KILLED          = 4530,
        SAY_BOTH_ADDS_KILLED    = 4531
    };
}

#include "Instance_TheVioletHold.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
//  Erekem AI
class ErekemAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit ErekemAI(Creature* pCreature);

    void OnLoad() override;
    void AIUpdate(unsigned long /*time_passed*/) override;
    void OnDied(Unit* /*_killer*/) override;
    void OnReachWP(uint32_t type, uint32_t iWaypointId) override;
    void justReachedSpawn() override;

    bool checkGuardsAlive();
    bool checkGuardAuras(Creature* guard);

protected:
    InstanceScript* mInstance;
    CreatureAISpells* windfury = nullptr;
    CreatureAISpells* breakBonds = nullptr;
};

class ErekemGuardAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit ErekemGuardAI(Creature* pCreature);
};
