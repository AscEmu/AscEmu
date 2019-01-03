/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* spawncheckcr = mTarget->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), mTarget->GetPositionZ(), 7750);

        if (!spawncheckcr)
        {
            Creature* general = mTarget->GetMapMgr()->CreateAndSpawnCreature(7750, -10619, -2997, 28.8f, 4);
            general->Despawn(3 * 60 * 1000, 0);
        }

        GameObject* spawncheckgobj = mTarget->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), mTarget->GetPositionZ(), 141980);

        if (!spawncheckgobj)
        {
            GameObject* generalsbox = mTarget->GetMapMgr()->CreateAndSpawnGameObject(141980, -10622, -2994, 28.6f, 4, 4);
            if (generalsbox != nullptr)
                generalsbox->Despawn(3 * 60 * 1000, 0);
        }
    }
};


class HeroesofOld1 : public Arcemu::Gossip::Script
{
public:

    void OnHello(Object* pObject, Player* plr) override
    {
        if (!plr)
            return;

        Creature* general = static_cast<Creature*>(pObject);
        if (general == nullptr)
            return;

        Arcemu::Gossip::Menu menu(pObject->getGuid(), 1);
        if (plr->HasQuest(2702) || plr->HasFinishedQuest(2702))
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(453), 1);     // I need to speak with Corporal.

        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*EnteredCode*/, uint32 /*gossipId*/) override
    {
        if (!plr)
            return;

        Creature* general = static_cast<Creature*>(pObject);
        if (general == nullptr)
            return;

        switch (Id)
        {
            case 1:
            {
                Creature* spawncheckcr = plr->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), 7750);
                if (!spawncheckcr)
                {
                    general = plr->GetMapMgr()->CreateAndSpawnCreature(7750, -10619, -2997, 28.8f, 4);
                    general->Despawn(3 * 60 * 1000, 0);
                }

                GameObject* spawncheckgobj = plr->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), 141980);
                if (!spawncheckgobj)
                {
                    GameObject* generalsbox = plr->GetMapMgr()->CreateAndSpawnGameObject(141980, -10622, -2994, 28.6f, 4, 4);
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

    Arcemu::Gossip::Script* gossip1 = new HeroesofOld1();
    mgr->register_creature_gossip(7572, gossip1);
}
