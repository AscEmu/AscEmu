/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/QuestScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class HeroesofOld : public QuestScript
{
public:
    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        Creature* spawncheckcr = mTarget->getWorldMap()->getInterface()->getCreatureNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), mTarget->GetPositionZ(), 7750);

        if (!spawncheckcr)
        {
            Creature* general = mTarget->getWorldMap()->createAndSpawnCreature(7750, LocationVector(-10619, -2997, 28.8f, 4));
            general->Despawn(3 * 60 * 1000, 0);
        }

        GameObject* spawncheckgobj = mTarget->getWorldMap()->getInterface()->getGameObjectNearestCoords(mTarget->GetPositionX(), mTarget->GetPositionY(), mTarget->GetPositionZ(), 141980);

        if (!spawncheckgobj)
        {
            GameObject* generalsbox = mTarget->getWorldMap()->createAndSpawnGameObject(141980, LocationVector(-10622, -2994, 28.6f, 4), 4);
            if (generalsbox != nullptr)
                generalsbox->despawn(3 * 60 * 1000, 0);
        }
    }
};


class HeroesofOld1 : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        if (!plr)
            return;

        Creature* general = static_cast<Creature*>(pObject);
        if (general == nullptr)
            return;

        GossipMenu menu(pObject->getGuid(), 1);
        if (plr->hasQuestInQuestLog(2702) || plr->hasQuestFinished(2702))
            menu.addItem(GOSSIP_ICON_CHAT, 453, 1);     // I need to speak with Corporal.

        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
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
                Creature* spawncheckcr = plr->getWorldMap()->getInterface()->getCreatureNearestCoords(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), 7750);
                if (!spawncheckcr)
                {
                    general = plr->getWorldMap()->createAndSpawnCreature(7750, LocationVector(-10619, -2997, 28.8f, 4));
                    general->Despawn(3 * 60 * 1000, 0);
                }

                GameObject* spawncheckgobj = plr->getWorldMap()->getInterface()->getGameObjectNearestCoords(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), 141980);
                if (!spawncheckgobj)
                {
                    GameObject* generalsbox = plr->getWorldMap()->createAndSpawnGameObject(141980, LocationVector(-10622, -2994, 28.6f, 4), 4);
                    if (generalsbox != nullptr)
                        generalsbox->despawn(3 * 60 * 1000, 0);
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
