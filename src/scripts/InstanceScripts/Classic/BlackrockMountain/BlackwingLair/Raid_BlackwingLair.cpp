/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Raid_BlackwingLair.h"

#include "Setup.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

class BlackwingLairInstanceScript : public InstanceScript
{
public:
    explicit BlackwingLairInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) { }
    static InstanceScript* Create(WorldMap* pMapMgr) { return new BlackwingLairInstanceScript(pMapMgr); }
};

class VaelastraszAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new VaelastraszAI(c); }
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

    mgr->register_creature_script(BlackwinfLair::CN_VAELASTRASZ, &VaelastraszAI::Create);

    mgr->register_creature_gossip(BlackwinfLair::CN_VAELASTRASZ, new VaelastraszGossip()); //\todo  Vael Gossip change the flag to agressive
}
