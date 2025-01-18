/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_TheStockade.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

class InstanceStormwindStockadeScript : public InstanceScript
{
public:
    explicit InstanceStormwindStockadeScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new InstanceStormwindStockadeScript(pMapMgr); }
};

class DeepfuryAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DeepfuryAI(c); }
    explicit DeepfuryAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHIELD_SLAM, 100.0f, TARGET_ATTACKING, 0, 8);
        addAISpell(IMPROVED_BLOCKING, 100.0f, TARGET_SELF, 0, 20);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 7164, false); // Defensive Stance
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 15 && getAIAgent() != AGENT_FLEE)
        {
            setAIAgent(AGENT_FLEE);
            _setMeleeDisabled(false);
            _setRangedDisabled(true);
            _setCastDisabled(true);
            moveTo(float(105.693390), float(-58.426674), float(-34.856178), true);
        }
    }
};

class HamhockAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HamhockAI(c); }
    explicit HamhockAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BLOODLUST, 100.0f, TARGET_RANDOM_FRIEND, 0, 60);
        addAISpell(CHAINLIGHT, 50.0f, TARGET_ATTACKING, 2, 7);

        addEmoteForEvent(Event_OnCombatStart, 8759);
    }
};

class BazilAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BazilAI(c); }
    explicit BazilAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SMOKE_BOMB, 100.0f, TARGET_ATTACKING, 9, 15);
        addAISpell(BATTLE_SHOUT, 100.0f, TARGET_SELF, 3, 30);

        addEmoteForEvent(Event_OnCombatStart, 8760);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 674, false); // Dual Wield
    }
};

class DextrenAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new DextrenAI(c); }
    explicit DextrenAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(FRIGHTENING_SHOUT, 33.0f, TARGET_ATTACKING, 8, 30);
        addAISpell(STRIKE, 33.0f, TARGET_SELF, 0, 10);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 7165, false); // Battle Stance
    }
};

class TargorrTheDreadAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new TargorrTheDreadAI(c); }
    explicit TargorrTheDreadAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        Enraged = addAISpell(ENRAGE, 0.0f, TARGET_SELF);
        addAISpell(THRASH, 50.0f, TARGET_SELF, 0, 8);
        Enrage = false;
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() < 50 && !Enrage)
        {
            Enrage = true;
            _castAISpell(Enraged);
        }
    }

    bool Enrage;
    CreatureAISpells* Enraged;

};

class InmateAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new InmateAI(c); }
    explicit InmateAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(REND, 100.0f, TARGET_ATTACKING, 5, 16);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 7165, false); // Battle Stance
    }
};

class InsurgentAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new InsurgentAI(c); }
    explicit InsurgentAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DEMORALIZING_SHOUT, 100.0f, TARGET_SELF, 7, 25);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 9128, false); // Battle Shout
    }
};

class PrisonerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new PrisonerAI(c); }
    explicit PrisonerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(KICK, 100.0f, TARGET_ATTACKING, 5, 16);
        addAISpell(DISARM, 100.0f, TARGET_ATTACKING, 10, 14);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 7165, false); // Battle Stance
    }
};

class ConvictAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ConvictAI(c); }
    explicit ConvictAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BACKHAND, 100.0f, TARGET_ATTACKING, 5, 12);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        getCreature()->castSpell(getCreature(), 674, false); // Dual Wield
    }
};

void SetupTheStockade(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_THE_STOCKADE, &InstanceStormwindStockadeScript::Create);

    //Creatures
    mgr->register_creature_script(CN_KAMDEEPFURY, &DeepfuryAI::Create);
    mgr->register_creature_script(CN_HAMHOCK, &HamhockAI::Create);
    mgr->register_creature_script(CN_BAZILTHREDD, &BazilAI::Create);
    mgr->register_creature_script(CN_DEXTRENWARD, &DextrenAI::Create);
    mgr->register_creature_script(CN_TARGORR_THE_DREAD, &TargorrTheDreadAI::Create);
    mgr->register_creature_script(CN_DEFINMATE, &InmateAI::Create);
    mgr->register_creature_script(CN_DEFINSURGENT, &InsurgentAI::Create);
    mgr->register_creature_script(CN_DEFPRISONER, &PrisonerAI::Create);
    mgr->register_creature_script(CN_DEFCONVICT, &ConvictAI::Create);
}
