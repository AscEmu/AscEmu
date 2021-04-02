/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2009 WhyScripts Team <http://www.whydb.org/>
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

enum
{
    DALARAN_TELEPORT_SPELL = 68328
};

class Lady_Jaina : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        if (plr->hasQuestInQuestLog(558))
        {
            GossipMenu menu(pObject->getGuid(), 7012, plr->GetSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 505, 1);     // Lady Jaina, this may sound like an odd request... but I have a young ward who is quite shy. You are a hero to him, and he asked me to get your autograph.
            menu.sendGossipPacket(plr);
        }
    }

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        plr->castSpell(plr, sSpellMgr.getSpellInfo(23122), true);
        GossipMenu::senGossipComplete(plr);
    }
};

class Cairne : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        if (plr->hasQuestInQuestLog(925))
        {
            GossipMenu menu(pObject->getGuid(), 7013, plr->GetSession()->language);
            menu.addItem(GOSSIP_ICON_CHAT, 506, 1);     // Give me hoofprint.
            menu.sendGossipPacket(plr);
        }
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        GossipMenu::sendSimpleMenu(pObject->getGuid(), 7014, plr);
        plr->castSpell(plr, sSpellMgr.getSpellInfo(23123), true);
    }
};

class TeleportQ_Gossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* plr) override
    {
        uint32_t Text = sMySQLStore.getGossipTextIdForNpc(static_cast<Creature*>(pObject)->getEntry());
        if (sMySQLStore.getNpcText(Text) == nullptr)
            Text = DefaultGossipTextId;

        GossipMenu menu(pObject->getGuid(), Text, plr->GetSession()->language);
        sQuestMgr.FillQuestMenu(static_cast<Creature*>(pObject), plr, menu);

        if ((plr->hasQuestInQuestLog(12791) || plr->hasQuestInQuestLog(12794) || plr->hasQuestInQuestLog(12796)) && plr->hasItem(39740))
        {
            menu.addItem(GOSSIP_ICON_CHAT, 514, 1);        // Teleport me to Dalaran.
        }
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* /*pObject*/, Player* plr, uint32_t /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        plr->castSpell(plr, DALARAN_TELEPORT_SPELL, true);
    }
};

void SetupQuestGossip(ScriptMgr* mgr)
{
    mgr->register_creature_gossip(4968, new Lady_Jaina());
    mgr->register_creature_gossip(3057, new Cairne());

    // Dalaran quests start
    GossipScript* gs = new TeleportQ_Gossip();

    // Horde
    mgr->register_creature_gossip(26471, gs);
    mgr->register_creature_gossip(29155, gs);
    mgr->register_creature_gossip(29159, gs);
    mgr->register_creature_gossip(29160, gs);
    mgr->register_creature_gossip(29162, gs);
    // Alliance
    mgr->register_creature_gossip(23729, gs);
    mgr->register_creature_gossip(26673, gs);
    mgr->register_creature_gossip(27158, gs);
    mgr->register_creature_gossip(29158, gs);
    mgr->register_creature_gossip(29161, gs);
    // Both
    mgr->register_creature_gossip(29169, gs);
}
