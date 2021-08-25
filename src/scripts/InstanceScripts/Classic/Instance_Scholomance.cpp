/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Scholomance.h"

#include "Server/Script/CreatureAIScript.h"
#include "Macros/ScriptMacros.hpp"

class ScholomanceInstanceScript : public InstanceScript
{
public:

    explicit ScholomanceInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ScholomanceInstanceScript(pMapMgr); }
};

class DoctorTheolenKrastinovAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DoctorTheolenKrastinovAI)
    explicit DoctorTheolenKrastinovAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        frenzy = addAISpell(SP_DR_THEOL_FRENZY, 0.0f, TARGET_SELF, 0, 0, false, true);
        frenzy->setAttackStopTimer(1000);
    }

    void AIUpdate() override
    {
        if (getCreature()->getHealthPct() <= 50 && getScriptPhase() == 1)
        {
            getCreature()->castSpell(getCreature(), frenzy->mSpellInfo, true);
            setScriptPhase(2);
        }
    }

protected:

    CreatureAISpells* frenzy;
};

class VectusAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VectusAI)
    explicit VectusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        frenzy = addAISpell(SP_VECTUS_FRENZY, 0.0f, TARGET_SELF, 0, 0, false, true);
        frenzy->setAttackStopTimer(1000);
    }

    void AIUpdate() override
    {
        if (getCreature()->getHealthPct() <= 25 && getScriptPhase() == 1)
        {
            getCreature()->castSpell(getCreature(), frenzy->mSpellInfo, true);
            setScriptPhase(2);
        }
    }

protected:

    CreatureAISpells* frenzy;
};

void SetupScholomance(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SCHOLOMANCE, &ScholomanceInstanceScript::Create);

    mgr->register_creature_script(CN_DOCTOR_THEOLEN_KRASTINOV, &DoctorTheolenKrastinovAI::Create);
    mgr->register_creature_script(CN_VECTUS, &VectusAI::Create);
}
