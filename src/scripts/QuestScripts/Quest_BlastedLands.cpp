/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008 WEmu Team
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

class HeroesofOld : public QuestScript
{
public:
    void OnQuestStart(Player* pPlayer, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* pCreature = pPlayer->MAP_GET_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 7750);
        if (!pCreature)
        {
            Creature* general = pPlayer->GetMapMgr()->CreateAndSpawnCreature(7750, -10619, -2997, 28.8f, 4);
            general->Despawn(3 * 60 * 1000, 0);
        }

        GameObject* spawncheckgobj = pPlayer->MAP_GET_GAMEOBJECT_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 141980);
        if (!spawncheckgobj)
        {
            GameObject* generalsbox = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(141980, -10622, -2994, 28.6f, 4, 4);
            if (generalsbox != nullptr)
                generalsbox->Despawn(3 * 60 * 1000, 0);
        }
    }
};

class HeroesofOld1 : public GossipScript
{
public:
    void onHello(Object* pObject, Player* pPlayer) override
    {
        if (!pPlayer)
            return;

        Creature* general = static_cast<Creature*>(pObject);
        if (general == nullptr)
            return;

        GossipMenu menu(pObject->getGuid(), 1);
        if (pPlayer->hasQuestInQuestLog(2702) || pPlayer->HasFinishedQuest(2702))
            menu.addItem(GOSSIP_ICON_CHAT, 453, 1); // I need to speak with Corporal.

        menu.sendGossipPacket(pPlayer);
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        if (!pPlayer)
            return;

        Creature* general = static_cast<Creature*>(pObject);
        if (general == nullptr)
            return;

        switch (Id)
        {
            case 1:
            {
                Creature* pCreature = pPlayer->MAP_GET_CREATURE_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 7750);
                if (!pCreature)
                {
                    general = pPlayer->GetMapMgr()->CreateAndSpawnCreature(7750, -10619, -2997, 28.8f, 4);
                    general->Despawn(3 * 60 * 1000, 0);
                }

                GameObject* spawncheckgobj = pPlayer->MAP_GET_GAMEOBJECT_NEAREST_COORDS(pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), 141980);
                if (!spawncheckgobj)
                {
                    GameObject* generalsbox = pPlayer->GetMapMgr()->CreateAndSpawnGameObject(141980, -10622, -2994, 28.6f, 4, 4);
                    if (generalsbox != nullptr)
                        generalsbox->Despawn(3 * 60 * 1000, 0);
                }
            }
        }
    }
};

void SetupBlastedLands(ScriptMgr* mgr)
{
    QuestScript* HeroesoO = new HeroesofOld();
    mgr->register_quest_script(2702, HeroesoO);

    GossipScript* gossip1 = new HeroesofOld1();
    mgr->register_creature_gossip(7572, gossip1);
}
