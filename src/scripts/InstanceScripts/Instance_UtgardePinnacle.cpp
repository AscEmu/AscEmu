/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Instance_UtgardePinnacle.h"


class SvalaSorrowgraveAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SvalaSorrowgraveAI);
        SvalaSorrowgraveAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
        }
};

class GortokPalehoofAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GortokPalehoofAI);
        GortokPalehoofAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addEmoteForEvent(Event_OnCombatStart, SAY_GROTOK_PALEHOOF_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_GROTOK_PALEHOOF_02);
            addEmoteForEvent(Event_OnTargetDied, SAY_GROTOK_PALEHOOF_03);
        }
};

class SkadiTheRuthlessAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SkadiTheRuthlessAI);
        SkadiTheRuthlessAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addEmoteForEvent(Event_OnCombatStart, SAY_SKADI_RUTHLESS_START);
            addEmoteForEvent(Event_OnTargetDied, SAY_SKADI_RUTHLESS_KILL_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_SKADI_RUTHLESS_KILL_02);
            addEmoteForEvent(Event_OnDied, SAY_SKADI_RUTHLESS_DIE);
        }
};

class KingYmironAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(KingYmironAI);
        KingYmironAI(Creature* pCreature) : CreatureAIScript(pCreature)
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
    //Bosses
    mgr->register_creature_script(CN_SVALA_SORROWGRAVE, &SvalaSorrowgraveAI::Create);
    mgr->register_creature_script(CN_GORTOK_PALEHOOF, &GortokPalehoofAI::Create);
    mgr->register_creature_script(CN_SKADI_THE_RUTHLESS, &SkadiTheRuthlessAI::Create);
    mgr->register_creature_script(CN_KING_YMIRON, &KingYmironAI::Create);
}
