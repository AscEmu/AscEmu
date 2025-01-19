/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Server/Script/CreatureAIScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Intro Portal AI
class IntroPortalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit IntroPortalAI(Creature* pCreature);

    void OnLoad() override;

    void AIUpdate(unsigned long /*time_passed*/) override;
    void SetCreatureData(uint32_t /*type*/, uint32_t /*data*/) override;

    void OnSummon(Unit* /*summoner*/) override;
    void onSummonedCreature(Creature* /*summon*/) override;
    void OnSummonDespawn(Creature* /*summon*/) override;

protected:
    InstanceScript* mInstance;
    uint8_t portalLocation;
};
