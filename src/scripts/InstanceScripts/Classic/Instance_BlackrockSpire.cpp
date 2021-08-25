/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_BlackrockSpire.h"

#include "Server/Script/CreatureAIScript.h"
#include "Macros/ScriptMacros.hpp"

class BlackrockSpireInstanceScript : public InstanceScript
{
public:

    explicit BlackrockSpireInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new BlackrockSpireInstanceScript(pMapMgr); }
};

class GythAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GythAI)
    explicit GythAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        HasSummoned = false;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        HasSummoned = false;
    }

    void AIUpdate() override
    {
        if (!HasSummoned && getCreature()->getHealthPct() <= 8)
        {
            Unit* Warchief = spawnCreature(CN_REND_BLACKHAND, 157.366516f, -419.779358f, 110.472336f, 3.056772f);
            if (Warchief != NULL)
            {
                if (getCreature()->getThreatManager().getCurrentVictim() != NULL)
                {
                    Warchief->GetAIInterface()->onHostileAction(getCreature()->getThreatManager().getCurrentVictim());
                }
            }

            HasSummoned = true;
        }
    }

protected:

    bool HasSummoned;
};

class HalyconAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HalyconAI)
    explicit HalyconAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        HasSummoned = false;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        HasSummoned = false;
    }

    void AIUpdate() override
    {
        if (!HasSummoned && getCreature()->getHealthPct() <= 25)
        {
            Unit* cGizrul = spawnCreature(CN_GIZRUL, -195.100006f, -321.970001f, 65.424400f, 0.016500f);
            if (cGizrul != NULL)
            {
                if (getCreature()->getThreatManager().getCurrentVictim() != NULL)
                {
                    cGizrul->GetAIInterface()->onHostileAction(getCreature()->getThreatManager().getCurrentVictim());
                }
            }

            HasSummoned = true;
        }
    }

protected:

    bool HasSummoned;
};


class OverlordWyrmthalakAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(OverlordWyrmthalakAI)
    explicit OverlordWyrmthalakAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        HasSummoned = false;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        HasSummoned = false;
    }

    void AIUpdate() override
    {
        if (!HasSummoned && getCreature()->getHealthPct() <= 50)
        {
            Unit* Warlord1 = spawnCreature(CN_SPIRESTONE_WARLORD, -30.675352f, -493.231750f, 90.610725f, 3.123542f);
            Unit* Warlord2 = spawnCreature(CN_SPIRESTONE_WARLORD, -30.433489f, -479.833923f, 90.535606f, 3.123542f);
            if (getCreature()->getThreatManager().getCurrentVictim() != NULL)
            {
                if (Warlord1 != NULL)
                {
                    Warlord1->GetAIInterface()->onHostileAction(getCreature()->getThreatManager().getCurrentVictim());
                }
                if (Warlord2 != NULL)
                {
                    Warlord2->GetAIInterface()->onHostileAction(getCreature()->getThreatManager().getCurrentVictim());
                }
            }

            HasSummoned = true; // Indicates that the spawns have been summoned
        }
    }

protected:

    bool HasSummoned;
};

void SetupBlackrockSpire(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKROCK_SPIRE, &BlackrockSpireInstanceScript::Create);
    mgr->register_creature_script(CN_GYTH, &GythAI::Create);
    mgr->register_creature_script(CN_HALYCON, &HalyconAI::Create);
    mgr->register_creature_script(CN_OVERLORD_WYRMTHALAK, &OverlordWyrmthalakAI::Create);
}
