/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"


GossipMenu::GossipMenu(uint64 Creature_Guid, uint32 Text_Id) : TextId(Text_Id), CreatureGuid(Creature_Guid)
{
}

void GossipMenu::AddItem(uint8 Icon, const char* Text, int32 Id /* = -1 */, int8 Extra /* = 0 */)
{
    GossipMenuItem Item;

    Item.Icon = Icon;
    Item.Extra = Extra;
    Item.Text = Text;
    Item.m_gBoxMessage = "";
    Item.m_gBoxMoney = 0;
    Item.Id = uint32(Menu.size());

    if (Id > 0)
        Item.IntId = Id;
    else
        Item.IntId = Item.Id;

    Menu.push_back(Item);
}

void GossipMenu::AddMenuItem(uint8 Icon, const char* Message, uint32 dtSender, uint32 dtAction, const char* BoxMessage, uint32 BoxMoney, bool Coded)
{
    GossipMenuItem Item;

    Item.Icon = Icon;
    Item.Extra = Coded;
    Item.Text = Message;
    Item.m_gBoxMessage = BoxMessage;
    Item.m_gBoxMoney = BoxMoney;
    Item.Id = uint32(Menu.size());
    Item.IntId = dtAction;

    Menu.push_back(Item);
}

GossipMenuItem GossipMenu::GetItem(uint32 Id)
{
    if (Id >= Menu.size())
    {
        GossipMenuItem k;
        k.IntId = 1;
        k.Extra = 0;

        k.Id = 0;
        k.Icon = 0;
        k.m_gSender = 0;
        k.m_gAction = 0;
        k.m_gBoxMoney = 0;

        return k;
    }
    else
    {
        return Menu[Id];
    }
}

void GossipMenu::AddItem(GossipMenuItem* GossipItem)
{
    Menu.push_back(*GossipItem);
}

void GossipMenu::BuildPacket(WorldPacket& Packet)
{
    Packet << CreatureGuid;
    Packet << uint32(0);            // some new menu type in 2.4?
    Packet << TextId;
    Packet << uint32(Menu.size());

    for (std::vector<GossipMenuItem>::iterator iter = Menu.begin();
        iter != Menu.end(); ++iter)
    {
        Packet << iter->Id;
        Packet << iter->Icon;
        Packet << iter->Extra;
        Packet << uint32(iter->m_gBoxMoney);    // money required to open menu, 2.0.3
        Packet << iter->Text;
        Packet << iter->m_gBoxMessage;          // accept text (related to money) pop up box, 2.0.3
    }
}

void GossipMenu::SendTo(Player* Plr)
{
    WorldPacket data(SMSG_GOSSIP_MESSAGE, Menu.size() * 50 + 24);
    BuildPacket(data);
    data << uint32(0);  // 0 quests obviously
    Plr->GetSession()->SendPacket(&data);
}
