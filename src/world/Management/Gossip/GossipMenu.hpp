/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Gossip/GossipDefines.hpp"
#include "Management/Gossip/Gossip.h"

//class SERVER_DECL GossipMenu
//{
//public:
//
//    GossipMenu(uint64 Creature_Guid, uint32 Text_Id, uint32_t gossip_id = 0);
//    void AddItem(GossipMenuItem* GossipItem);
//    void AddItem(uint8 Icon, const char* Text, int32 Id = -1, int8 Extra = 0);
//    void AddMenuItem(uint8 Icon, const char* Message, uint32 dtSender, uint32 dtAction, const char* BoxMessage, uint32 BoxMoney, bool Coded = false);
//    void BuildPacket(WorldPacket& Packet);
//    void SendTo(Player* Plr);
//    GossipMenuItem GetItem(uint32 Id);
//    inline void SetTextID(uint32 TID) { TextId = TID; }
//
//protected:
//
//    uint32 TextId;
//    uint32_t mGossipId;
//    uint64 CreatureGuid;
//    std::vector<GossipMenuItem> Menu;
//};
