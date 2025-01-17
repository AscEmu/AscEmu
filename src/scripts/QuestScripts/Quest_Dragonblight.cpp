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
#include "Management/ItemInterface.h"
#include "Management/Gossip/GossipScript.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Utilities/Random.hpp"

class WoodlandWalker : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new WoodlandWalker(c); }
    explicit WoodlandWalker(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pCreature->setFaction(35);
    }
};

class WoodlandWalkerGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        Creature* pCreature = (pObject->isCreature()) ? (static_cast<Creature*>(pObject)) : nullptr;
        if (pCreature == nullptr)
            return;

        uint32_t chance = Util::getRandomUInt(1);
        if (chance == 0)
        {
            pCreature->setFaction(14);
            pCreature->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "The Woodlands Walker is angered by your request and attacks!");
        }
        else
        {
            plr->getItemInterface()->AddItemById(36786, 1, 0);
            pCreature->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Breaking off a piece of its bark, the Woodlands Walker hands it to you before departing.");
        }
    }
};

class WrathGateQuestCinema : public QuestScript
{
public:
    void OnQuestComplete(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
#if VERSION_STRING > TBC
        // send packet for movie
        uint32_t id = 14;
        mTarget->sendMovie(id);
#endif
    }
};

void SetupDragonblight(ScriptMgr* mgr)
{
    mgr->register_creature_script(26421, &WoodlandWalker::Create);

    mgr->register_creature_gossip(26421, new WoodlandWalkerGossip());

    mgr->register_quest_script(12499, new WrathGateQuestCinema());
    mgr->register_quest_script(12500, new WrathGateQuestCinema());
}