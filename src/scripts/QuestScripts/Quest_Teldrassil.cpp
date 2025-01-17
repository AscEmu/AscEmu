/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Chat/ChatDefines.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/Script/QuestScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class Zenn_Foulhoof : public QuestScript
{
public:
    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        if (Creature* creature = mTarget->getWorldMap()->getSqlIdCreature(43727))
        {
            creature->setDisplayId(901);
            creature->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Ribbit! No! This cannot...ribbit...be! You have duped me with...ribbit..your foul trickery! Ribbit!");

            sEventMgr.AddEvent(static_cast<Unit*>(creature), &Unit::setDisplayId, static_cast<uint32_t>(10035), EVENT_UNK, 50000, 0, 1);
        }
    }
};

void SetupTeldrassil(ScriptMgr* mgr)
{
    mgr->register_quest_script(489, new Zenn_Foulhoof());
}
