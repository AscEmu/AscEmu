/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_BlackrockSpire.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

class BlackrockSpireInstanceScript : public InstanceScript
{
public:
    explicit BlackrockSpireInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new BlackrockSpireInstanceScript(pMapMgr); }
};

class GythAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GythAI(c); }

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
            Unit* Warchief = spawnCreature(BlackrockSpire::CN_REND_BLACKHAND, 157.366516f, -419.779358f, 110.472336f, 3.056772f);
            if (Warchief != nullptr)
            {
                if (getCreature()->getThreatManager().getCurrentVictim() != nullptr)
                {
                    Warchief->getAIInterface()->onHostileAction(getCreature()->getThreatManager().getCurrentVictim());
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
public:
    static CreatureAIScript* Create(Creature* c) { return new HalyconAI(c); }
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
            Unit* cGizrul = spawnCreature(BlackrockSpire::CN_GIZRUL, -195.100006f, -321.970001f, 65.424400f, 0.016500f);
            if (cGizrul != nullptr)
            {
                if (getCreature()->getThreatManager().getCurrentVictim() != nullptr)
                {
                    cGizrul->getAIInterface()->onHostileAction(getCreature()->getThreatManager().getCurrentVictim());
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
public:
    static CreatureAIScript* Create(Creature* c) { return new OverlordWyrmthalakAI(c); }
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
            Unit* Warlord1 = spawnCreature(BlackrockSpire::CN_SPIRESTONE_WARLORD, -30.675352f, -493.231750f, 90.610725f, 3.123542f);
            Unit* Warlord2 = spawnCreature(BlackrockSpire::CN_SPIRESTONE_WARLORD, -30.433489f, -479.833923f, 90.535606f, 3.123542f);
            if (getCreature()->getThreatManager().getCurrentVictim() != nullptr)
            {
                if (Warlord1 != nullptr)
                {
                    Warlord1->getAIInterface()->onHostileAction(getCreature()->getThreatManager().getCurrentVictim());
                }
                if (Warlord2 != nullptr)
                {
                    Warlord2->getAIInterface()->onHostileAction(getCreature()->getThreatManager().getCurrentVictim());
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
    mgr->register_creature_script(BlackrockSpire::CN_GYTH, &GythAI::Create);
    mgr->register_creature_script(BlackrockSpire::CN_HALYCON, &HalyconAI::Create);
    mgr->register_creature_script(BlackrockSpire::CN_OVERLORD_WYRMTHALAK, &OverlordWyrmthalakAI::Create);
}
