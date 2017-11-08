/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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


//SvalaSorrowgrave
class SvalaSorrowgraveAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(SvalaSorrowgraveAI, MoonScriptCreatureAI);
        SvalaSorrowgraveAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
        }
};

//GortokPalehoof
class GortokPalehoofAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(GortokPalehoofAI, MoonScriptCreatureAI);
        GortokPalehoofAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
        }

        void OnCombatStart(Unit* mTarget)
        {
            sendDBChatMessage(SAY_GROTOK_PALEHOOF_01);
        }

        void OnTargetDied(Unit* mKiller)
        {
            switch (RandomUInt(2))
            {
                case 0:
                    sendDBChatMessage(SAY_GROTOK_PALEHOOF_02);
                    break;
                case 1:
                    sendDBChatMessage(SAY_GROTOK_PALEHOOF_03);
                    break;
                default:
                    break;
            }
        }
};

//SkadiTheRuthless
class SkadiTheRuthlessAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(SkadiTheRuthlessAI, MoonScriptCreatureAI);
        SkadiTheRuthlessAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
        }

        void OnCombatStart(Unit* mTarget)
        {
            sendDBChatMessage(SAY_SKADI_RUTHLESS_START);
        }

        void OnTargetDied(Unit* mKiller)
        {
            switch (RandomUInt(2))
            {
                case 0:
                    sendDBChatMessage(SAY_SKADI_RUTHLESS_KILL_01);
                    break;
                case 1:
                    sendDBChatMessage(SAY_SKADI_RUTHLESS_KILL_02);
                    break;
                default:
                    break;
            }
        }

        void OnDied(Unit* mKiller)
        {
            sendDBChatMessage(SAY_SKADI_RUTHLESS_DIE);
        }
};

//KingYmiron
class KingYmironAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(KingYmironAI, MoonScriptCreatureAI);
        KingYmironAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
        }

        void OnCombatStart(Unit* mTarget)
        {
            sendDBChatMessage(SAY_KING_YMIRON_START);
        }

        void OnTargetDied(Unit* mKiller)
        {
            switch (RandomUInt(5))
            {
                case 0:
                    sendDBChatMessage(SAY_KING_YMIRON_KILL_01);
                    break;
                case 1:
                    sendDBChatMessage(SAY_KING_YMIRON_KILL_02);
                    break;
                case 2:
                    sendDBChatMessage(SAY_KING_YMIRON_KILL_03);
                    break;
                case 3:
                    sendDBChatMessage(SAY_KING_YMIRON_KILL_04);
                    break;
                default:
                    break;
            }
        }

        void OnDied(Unit* mKiller)
        {
            sendDBChatMessage(SAY_KING_YMIRON_DIE);
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
