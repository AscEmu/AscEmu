/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2009-2010 ArcEmu Team <http://www.ArcEmu.org/>
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
#include "Management/TaxiMgr.h"

class ScryingOrb : public GameObjectAIScript
{
public:

    explicit ScryingOrb(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new ScryingOrb(GO); }

    void OnActivate(Player* pPlayer)
    {
        if (auto* questLog = pPlayer->getQuestLogByQuestId(11490))
        {
            float SSX = pPlayer->GetPositionX();
            float SSY = pPlayer->GetPositionY();
            float SSZ = pPlayer->GetPositionZ();

            GameObject* pOrb = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(SSX, SSY, SSZ, 187578);
            if (pOrb)
            {
                pOrb->setState(GO_STATE_OPEN);
                questLog->setMobCountForIndex(0, 1);
                questLog->SendUpdateAddKill(0);
                questLog->updatePlayerFields();
            }
        }
        else
        {
            pPlayer->BroadcastMessage("Missing required quest : The Scryer's Scryer");
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Ayren Cloudbreaker Gossip
class AyrenCloudbreaker_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* pPlayer) override
    {
        GossipMenu menu(pObject->getGuid(), 12252, pPlayer->GetSession()->language);
        if (pPlayer->hasQuestInQuestLog(11532) || pPlayer->hasQuestInQuestLog(11533))
            menu.addItem(GOSSIP_ICON_CHAT, 466, 1);     // Speaking of action, I've been ordered to undertake an air strike.

        if (pPlayer->hasQuestInQuestLog(11543) || pPlayer->hasQuestInQuestLog(11542))
            menu.addItem(GOSSIP_ICON_CHAT, 467, 2);     // I need to intercept the Dawnblade reinforcements.

        menu.sendGossipPacket(pPlayer);
    }

    void onSelectOption(Object* /*pObject*/, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                TaxiPath* pPath = sTaxiMgr.GetTaxiPath(779);
                pPlayer->TaxiStart(pPath, 22840, 0);
                pPlayer->removeUnitFlags(UNIT_FLAG_MOUNTED_TAXI);
            } break;
            case 2:
            {
                TaxiPath* pPath = sTaxiMgr.GetTaxiPath(784);
                pPlayer->TaxiStart(pPath, 22840, 0);
                pPlayer->removeUnitFlags(UNIT_FLAG_MOUNTED_TAXI);
            } break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Unrestrained Dragonhawk Gossip
class SCRIPT_DECL UnrestrainedDragonhawk_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* pPlayer) override
    {
        GossipMenu menu(pObject->getGuid(), 12371, pPlayer->GetSession()->language);
        if (pPlayer->hasQuestInQuestLog(11543) || pPlayer->hasQuestInQuestLog(11542))
            menu.addItem(GOSSIP_ICON_CHAT, 468, 1); // <Ride the dragonhawk to Sun's Reach>

        menu.sendGossipPacket(pPlayer);
    }

    void onSelectOption(Object* /*pObject*/, Player* pPlayer, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        TaxiPath* pPath = sTaxiMgr.GetTaxiPath(788);
        pPlayer->TaxiStart(pPath, 22840, 0);
        pPlayer->removeUnitFlags(UNIT_FLAG_MOUNTED_TAXI);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// The Battle for the Sun's Reach Armory
class TheBattleForTheSunReachArmory : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TheBattleForTheSunReachArmory)
    explicit TheBattleForTheSunReachArmory(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* pKiller) override
    {
        if (pKiller->isPlayer())
        {
            Player* player = static_cast<Player*>(pKiller);

            player->AddQuestKill(11537, 0, 0);
            player->AddQuestKill(11538, 0, 0);
        }
    }
};

void SetupIsleOfQuelDanas(ScriptMgr* mgr)
{
    mgr->register_gameobject_script(187578, &ScryingOrb::Create);
    mgr->register_creature_script(24999, &TheBattleForTheSunReachArmory::Create);
    mgr->register_creature_script(25001, &TheBattleForTheSunReachArmory::Create);
    mgr->register_creature_script(25002, &TheBattleForTheSunReachArmory::Create);

    mgr->register_creature_gossip(25059, new AyrenCloudbreaker_Gossip());
    mgr->register_creature_gossip(25236, new UnrestrainedDragonhawk_Gossip());
}
