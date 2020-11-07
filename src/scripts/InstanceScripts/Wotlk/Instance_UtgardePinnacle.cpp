/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_UtgardePinnacle.h"

class UtgardePinnacleInstanceScript : public InstanceScript
{
public:

    explicit UtgardePinnacleInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new UtgardePinnacleInstanceScript(pMapMgr); }


};

class SvalaSorrowgraveAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SvalaSorrowgraveAI)
    explicit SvalaSorrowgraveAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }
};

class GortokPalehoofAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GortokPalehoofAI)
    explicit GortokPalehoofAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addEmoteForEvent(Event_OnCombatStart, SAY_GROTOK_PALEHOOF_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_GROTOK_PALEHOOF_02);
        addEmoteForEvent(Event_OnTargetDied, SAY_GROTOK_PALEHOOF_03);
    }
};

class SkadiTheRuthlessAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SkadiTheRuthlessAI)
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
    ADD_CREATURE_FACTORY_FUNCTION(KingYmironAI)
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
