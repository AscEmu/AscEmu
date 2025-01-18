/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Instance_UtgardePinnacle.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

class UtgardePinnacleInstanceScript : public InstanceScript
{
public:
    explicit UtgardePinnacleInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new UtgardePinnacleInstanceScript(pMapMgr); }
};

class SvalaSorrowgraveAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SvalaSorrowgraveAI(c); }
    explicit SvalaSorrowgraveAI(Creature* pCreature) : CreatureAIScript(pCreature) 
    {
        mInstance = getInstanceScript();
    }
protected:
    InstanceScript* mInstance;
};

class GortokPalehoofAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new GortokPalehoofAI(c); }
    explicit GortokPalehoofAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addEmoteForEvent(Event_OnCombatStart, SAY_GROTOK_PALEHOOF_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_GROTOK_PALEHOOF_02);
        addEmoteForEvent(Event_OnTargetDied, SAY_GROTOK_PALEHOOF_03);
    }
};

class SkadiTheRuthlessAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SkadiTheRuthlessAI(c); }
    explicit SkadiTheRuthlessAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addEmoteForEvent(Event_OnCombatStart, SAY_SKADI_RUTHLESS_START);
        addEmoteForEvent(Event_OnTargetDied, SAY_SKADI_RUTHLESS_KILL_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_SKADI_RUTHLESS_KILL_02);
        addEmoteForEvent(Event_OnDied, SAY_SKADI_RUTHLESS_DIE);
    }
};

class KingYmironAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new KingYmironAI(c); }
    explicit KingYmironAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addEmoteForEvent(Event_OnCombatStart, SAY_KING_YMIRON_START);
        addEmoteForEvent(Event_OnTargetDied, SAY_KING_YMIRON_KILL_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_KING_YMIRON_KILL_02);
        addEmoteForEvent(Event_OnTargetDied, SAY_KING_YMIRON_KILL_03);
        addEmoteForEvent(Event_OnTargetDied, SAY_KING_YMIRON_KILL_04);
        addEmoteForEvent(Event_OnDied, SAY_KING_YMIRON_DIE);
    }
};

void SetupUtgardePinnacle(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_UTGARDE_PINNACLE, &UtgardePinnacleInstanceScript::Create);

    //Bosses
    mgr->register_creature_script(CN_SVALA_SORROWGRAVE, &SvalaSorrowgraveAI::Create);
    mgr->register_creature_script(CN_GORTOK_PALEHOOF, &GortokPalehoofAI::Create);
    mgr->register_creature_script(CN_SKADI_THE_RUTHLESS, &SkadiTheRuthlessAI::Create);
    mgr->register_creature_script(CN_KING_YMIRON, &KingYmironAI::Create);
}
