/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "VioletHold.h"

class VioletHold : public InstanceScript
{
public:

    explicit VioletHold(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        addData(608, NotStarted);
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new VioletHold(pMapMgr); }

    void OnPlayerEnter(Player* player)
    {
        player->BroadcastMessage("Welcome to %s", mInstance->GetMapInfo()->name.c_str());

        setData(608, PreProgress);
    }
};

#define SINCLARI_SAY_1 "Prison guards, we are leaving! These adventurers are taking over! Go go go!"
#define SINCLARY_SAY_2 "I'm locking the door. Good luck, and thank you for doing this."

class SinclariAI : public CreatureAIScript
{
public:

    explicit SinclariAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }

    static CreatureAIScript* Create(Creature* creature) { return new SinclariAI(creature); }

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        switch (iWaypointId)
        {
            case 2:
            {
                OnRescuePrisonGuards();
            } break;
            case 4:
            {
                getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, SINCLARY_SAY_2);
                getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
            } break;
            case 5:
            {
                VioletHold* pInstance = (VioletHold*)getCreature()->GetMapMgr()->GetScript();
                pInstance->setData(608, InProgress);
                GameObject* pVioletHoldDoor = pInstance->getClosestGameObjectForPosition(191723, 1822.59f, 803.93f, 44.36f);
                if (pVioletHoldDoor != nullptr)
                    pVioletHoldDoor->setState(GO_STATE_CLOSED);
            } break;
        }
    }

    void OnRescuePrisonGuards()
    {
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, SINCLARI_SAY_1);

        VioletHold* pInstance = (VioletHold*)getCreature()->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        auto guardSet = pInstance->getCreatureSetForEntry(30659);
        for (auto guard : guardSet)
        {
            guard->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
        }
    }
};

class SinclariGossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* pPlayer) override
    {
        VioletHold* pInstance = (VioletHold*)pPlayer->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        //Page 1: Textid and first menu item
        if (pInstance->getData(608) == PreProgress)
        {
            GossipMenu menu(pObject->getGuid(), 13853, 0);
            menu.addItem(GOSSIP_ICON_CHAT, (600), 1);
            menu.sendGossipPacket(pPlayer);
        }

        //If VioletHold is started, Sinclari has this item for people who aould join.
        if (pInstance->getData(608) == InProgress)
        {
            GossipMenu menu(pObject->getGuid(), 13853, 0);
            menu.addItem(GOSSIP_ICON_CHAT, (602), 3);
            menu.sendGossipPacket(pPlayer);
        }
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        VioletHold* pInstance = (VioletHold*)pPlayer->GetMapMgr()->GetScript();
        if (!pInstance)
            return;

        if (!pObject->isCreature())
            return;

        Creature* sinclari = static_cast<Creature*>(pObject);
        switch (Id)
        {
            case 1:
            {
                GossipMenu menu(pObject->getGuid(), 13854, 0);
                menu.addItem(GOSSIP_ICON_CHAT, (601), 2);
                menu.sendGossipPacket(pPlayer);
            } break;
            case 2:
            {
                static_cast<Creature*>(pObject)->setNpcFlags(UNIT_NPC_FLAG_NONE);
                sinclari->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
            } break;
            case 3:
            {
                GossipMenu::senGossipComplete(pPlayer);
                pPlayer->SafeTeleport(pPlayer->GetInstanceID(), 608, 1830.531006f, 803.939758f, 44.340508f, 6.281611f);
            } break;
        }
    }
};

class VHGuardsAI : public CreatureAIScript
{
public:

    explicit VHGuardsAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }

    static CreatureAIScript* Create(Creature* creature) { return new VHGuardsAI(creature); }
};


void VioletHoldScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(608, &VioletHold::Create);

    scriptMgr->register_creature_script(30659, &VHGuardsAI::Create);

    scriptMgr->register_creature_script(30658, &SinclariAI::Create);

    GossipScript* GSinclari = new SinclariGossip();
    scriptMgr->register_creature_gossip(30658, GSinclari);
}
