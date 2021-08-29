/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/Script/CreatureAIScript.h"
#include "Macros/ScriptMacros.hpp"

class BlackwingLairInstanceScript : public InstanceScript
{
public:

    explicit BlackwingLairInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new BlackwingLairInstanceScript(pMapMgr); }
};

class VaelastraszAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VaelastraszAI)
    explicit VaelastraszAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->setHealth((uint32_t)(getCreature()->getMaxHealth() * 0.3f));
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        getCreature()->sendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "At last the agony ends. I have failed you my Queen... I have failed us all...");
    }
};

class VaelastraszGossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* Plr) override
    {
        GossipMenu menu(pObject->getGuid(), 9903, 0);
        menu.sendGossipPacket(Plr);
    }
};


void SetupBlackwingLair(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKWING_LAIR, &BlackwingLairInstanceScript::Create);

    mgr->register_creature_script(13020, &VaelastraszAI::Create);

    mgr->register_creature_gossip(13020, new VaelastraszGossip()); //\todo  Vael Gossip change the flag to agressive
}
