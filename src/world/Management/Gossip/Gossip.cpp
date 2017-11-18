/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Management/Item.h"

using namespace Arcemu;

Gossip::Item::Item(size_t itemid, uint8 icon)
{
    id_ = uint16(itemid);
    icon_ = icon;
    text_ = "";
    boxmessage_ = "";
    boxmoney_ = 0;
    coded_ = false;
}

Gossip::Item::Item(size_t itemid, uint8 icon, const char* text, bool coded/*= false*/, size_t boxmoney/*=0*/, const char* boxmessage/*=NULL*/)
{
    id_ = uint16(itemid);
    icon_ = icon;
    text_ = (text != NULL) ? text : "";
    coded_ = coded;
    boxmoney_ = uint32(boxmoney);
    boxmessage_ = (boxmessage != NULL) ? boxmessage : "";
}

WorldPacket& operator<<(WorldPacket& packet, const Gossip::Item & item)
{
    packet << uint32(item.id_);
    packet << item.icon_;
    packet << item.coded_;
    packet << item.boxmoney_;
    packet << item.text_;
    packet << item.boxmessage_;
    return packet;
}

template<uint32 size>
StackBuffer<size>& operator<<(StackBuffer<size>& packet, const Gossip::Item & item)
{
    packet << uint32(item.id_);
    packet << item.icon_;
    packet << item.coded_;
    packet << item.boxmoney_;
    packet << item.text_;
    packet << item.boxmessage_;
    return packet;
}


Gossip::Menu::Menu(uint64 Creature_Guid, uint32 Text_Id, uint32 language, uint32 gossip_id) : textid_(Text_Id), guid_(Creature_Guid), language_(language), gossipId(gossip_id)
{}

Gossip::Menu::Menu(Object* obj, uint32 textid, uint32 language, uint32 gossip_id)
{
    guid_ = obj->GetGUID();
    textid_ = textid;
    language_ = language;
    gossipId = gossip_id;
}

void Gossip::Menu::AddItem(uint8 icon, const char* itemtext, uint32 itemid, bool coded/*=false*/)
{
    Gossip::Item item(itemid, icon);
    item.text_ = (itemtext != NULL) ? itemtext : "";
    item.coded_ = coded;
    this->itemlist_.push_back(item);
}

void Gossip::Menu::AddItem(uint8 icon, const char* itemtext, uint32 itemid, uint32 boxmoney/*=0*/, const char* boxtext/*=NULL*/, bool coded/*=false*/)
{
    Gossip::Item item(itemid, icon, itemtext, coded, boxmoney, boxtext);
    this->itemlist_.push_back(item);
}

void Gossip::Menu::RemoveItem(uint32 id)
{
    for (Gossip::ItemList::iterator itr = itemlist_.begin(); itr != itemlist_.end(); ++itr)
    {
        if ((*itr).id_ == id)
        {
            itemlist_.erase(itr);
            break;
        }
    }
}

void Gossip::Menu::AddQuest(QuestProperties const* quest, uint8 icon)
{
    this->questlist_.insert(std::make_pair(quest, icon));
}

void Gossip::Menu::RemoveQuest(QuestProperties const* quest)
{
    Gossip::QuestList::iterator itr = questlist_.find(quest);
    if (itr != questlist_.end())
        questlist_.erase(itr);
}

WorldPacket& operator<<(WorldPacket& packet, const Gossip::Menu & menu)
{
    packet << menu.guid_;
    packet << menu.gossipId;
    packet << menu.textid_;
    packet << uint32(menu.itemlist_.size());
    {
        for (Gossip::ItemList::const_iterator itr = menu.itemlist_.begin(); itr != menu.itemlist_.end(); ++itr)
            packet << *itr;
    }
    packet << uint32(menu.questlist_.size());
    {
        for (Gossip::QuestList::const_iterator itr = menu.questlist_.begin(); itr != menu.questlist_.end(); ++itr)
        {
            packet << itr->first->id;
            packet << uint32(itr->second);
            packet << itr->first->min_level;
            packet << itr->first->quest_flags;
            packet << uint8(0);

            MySQLStructure::LocalesQuest const* lq = (menu.language_ > 0) ? sMySQLStore.getLocalizedQuest(itr->first->id, menu.language_) : nullptr;
            if (lq != nullptr)
            {
                packet << lq->title;
            }
            else
            {
                packet << itr->first->title;
            }
        }
    }
    return packet;
}

template<uint32 size>
StackBuffer<size>& operator<<(StackBuffer<size> & packet, const Gossip::Menu & menu)
{
    packet << menu.guid_;
    packet << menu.gossipId;
    packet << menu.textid_;
    packet << uint32(menu.itemlist_.size());
    {
        for (Gossip::ItemList::const_iterator itr = menu.itemlist_.begin(); itr != menu.itemlist_.end(); ++itr)
            packet << *itr;
    }
    packet << uint32(menu.questlist_.size());
    {
        std::string title;
        for (Gossip::QuestList::const_iterator itr = menu.questlist_.begin(); itr != menu.questlist_.end(); ++itr)
        {
            packet << itr->first->id;
            packet << uint32(itr->second);
            packet << itr->first->min_level;
            packet << itr->first->quest_flags;
            packet << uint8(0);
            MySQLStructure::LocalesQuest const* lq = (menu.language_ > 0) ? sMySQLStore.getLocalizedQuest(itr->first->id, menu.language_) : nullptr;
            if (lq != nullptr)
            {
                title = lq->title;
            }
            else
            {
                title = itr->first->title;
            }
            packet << title;
        }
    }
    return packet;
}

void Gossip::Menu::BuildPacket(WorldPacket& packet) const
{
    packet << *this;
}
void Gossip::Menu::BuildPacket(WorldPacket* packet) const
{
    *packet << *this;
}

void Gossip::Menu::Send(Player* plr) const
{
    WorldPacket packet(SMSG_GOSSIP_MESSAGE, 512);
    BuildPacket(packet);
    plr->GetSession()->SendPacket(&packet);
}

template<uint32 size>
void Gossip::Menu::StackSend(Player* plr) const
{
    StackWorldPacket<size> packet(SMSG_GOSSIP_MESSAGE);
    packet << static_cast<Gossip::Menu>(*this);
    plr->GetSession()->SendPacket(&packet);
}

void Gossip::Menu::SendSimpleMenu(uint64 guid, size_t txt_id, Player* plr)
{
    StackWorldPacket<32> packet(SMSG_GOSSIP_MESSAGE);
    packet << guid;
    packet << uint32(0);
    packet << uint32(txt_id);
    packet << uint32(0);
    packet << uint32(0);
    plr->GetSession()->SendPacket(&packet);
}

void Gossip::Menu::SendQuickMenu(uint64 guid, size_t textid, Player* Plr, size_t itemid, uint8 itemicon, const char* itemtext, size_t requiredmoney/*=0*/, const char* moneytext/*=NULL*/, uint8 extra/*=0*/)
{
    StackWorldPacket<64> packet(SMSG_GOSSIP_MESSAGE);
    std::string itemtexts = (itemtext != NULL) ? itemtext : "";
    std::string moneytexts = (moneytext != NULL) ? moneytext : "";
    packet << guid;
    packet << uint32(0);
    packet << uint32(textid);
    packet << uint32(1);
    packet << uint32(itemid);
    packet << itemicon;
    packet << extra;
    packet << uint32(requiredmoney);
    packet << itemtexts;

    if (moneytext != NULL)
        packet << moneytexts;
    else
        packet << uint8(0);

    packet << uint32(0);
    Plr->GetSession()->SendPacket(&packet);
}

void Gossip::Menu::Complete(Player* plr)
{
    plr->GetSession()->OutPacket(SMSG_GOSSIP_COMPLETE, 0, NULL);
}

void Gossip::Script::Destroy()
{
    delete this;
}

Gossip::Script* Gossip::Script::GetInterface(Creature* creature)
{
    Gossip::Script* script = sScriptMgr.get_creature_gossip(creature->GetEntry());
    if (script != NULL)
        return script;

    if (creature->isSpiritHealer())
        return &sScriptMgr.spirithealerScript_;
    else if (creature->isInnkeeper())
        return &sScriptMgr.innkeeperScript_;
    else if (creature->isBanker())
        return &sScriptMgr.bankerScript_;
    else if (creature->isClass())
        return &sScriptMgr.classtrainerScript_;
    else if (creature->isTrainer())
    {
        ::Trainer* traininfo = creature->GetTrainer();

        if (traininfo != NULL)    //Seems to happen.
        {
            if (traininfo->TrainerType == TRAINER_TYPE_PET)
                return &sScriptMgr.pettrainerScript_;
            else
                return &sScriptMgr.trainerScript_;
        }
    }
    else if (creature->isTabardDesigner())
        return &sScriptMgr.tabardScript_;
    else if (creature->isTaxi())
        return &sScriptMgr.flightmasterScript_;
    else if (creature->isStableMaster())
        return &sScriptMgr.stablemasterScript_;
    else if (creature->isBattleMaster())
        return &sScriptMgr.battlemasterScript_;
    else if (creature->isAuctioner())
        return &sScriptMgr.auctioneerScript_;
    else if (creature->isCharterGiver())
        return &sScriptMgr.chartergiverScript_;
    else if (creature->isVendor())
        return &sScriptMgr.vendorScript_;

    return &sScriptMgr.genericScript_;
}
Gossip::Script* Gossip::Script::GetInterface(::Item* item)
{
    return sScriptMgr.get_item_gossip(item->GetEntry());
}
Gossip::Script* Gossip::Script::GetInterface(GameObject* go)
{
    return sScriptMgr.get_go_gossip(go->GetEntry());
}

// SPIRIT HEALER
void Arcemu::Gossip::SpiritHealer::OnHello(Object* pObject, Player* Plr)
{
    Plr->GetSession()->SendSpiritHealerRequest(static_cast<Creature*>(pObject));
}

// VENDORS
void Arcemu::Gossip::Vendor::OnHello(Object* pObject, Player* Plr)
{
    Creature* creature = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(creature->GetEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    MySQLStructure::VendorRestrictions const* vendor = sMySQLStore.getVendorRestriction(creature->GetCreatureProperties()->Id);

    Gossip::Menu menu(creature->GetGUID(), Text, Plr->GetSession()->language);

    if (!Plr->CanBuyAt(vendor))
        menu.setTextID(vendor->cannotbuyattextid);
    else
        menu.AddItem(GOSSIP_ICON_VENDOR, Plr->GetSession()->LocalizedGossipOption(VENDOR), 1, false);

    sQuestMgr.FillQuestMenu(creature, Plr, menu); //add any quests we have.

    menu.StackSend<256>(Plr);
}

void Arcemu::Gossip::Vendor::OnSelectOption(Object* pObject, Player* Plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    Plr->GetSession()->SendInventoryList(static_cast<Creature*>(pObject));
}

//TRAINER
void Arcemu::Gossip::Trainer::OnHello(Object* pObject, Player* Plr)
{
    Creature* trainer = static_cast<Creature*>(pObject);
    ::Trainer* trainerinfo = trainer->GetTrainer();
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(trainer->GetEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    std::string name = trainer->GetCreatureProperties()->Name;
    std::string::size_type pos = name.find(" ");      // only take first name

    if (pos != std::string::npos)
        name = name.substr(0, pos);

    Gossip::Menu menu(trainer->GetGUID(), Text, Plr->GetSession()->language);
    if (trainerinfo != NULL)
    {
        if (!Plr->CanTrainAt(trainerinfo))
            menu.setTextID(trainerinfo->Cannot_Train_GossipTextId);
        else
        {
            // I seek
            std::string msg = std::string(Plr->GetSession()->LocalizedGossipOption(ISEEK));
            msg += std::string(Plr->GetSession()->LocalizedGossipOption(TRAINING)) + ", " + name + ".";
            menu.AddItem(GOSSIP_ICON_TRAINER, msg.c_str(), 1);

            if (trainer->isVendor())
            {
                MySQLStructure::VendorRestrictions const* vendor = sMySQLStore.getVendorRestriction(trainer->GetCreatureProperties()->Id);
                if (Plr->CanBuyAt(vendor))
                    menu.AddItem(GOSSIP_ICON_VENDOR, Plr->GetSession()->LocalizedGossipOption(VENDOR), 2);
            }
        }
    }
    sQuestMgr.FillQuestMenu(trainer, Plr, menu);
    menu.StackSend<256>(Plr);
}

void Arcemu::Gossip::Trainer::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    if (1 == Id)
        Plr->GetSession()->SendTrainerList(static_cast<Creature*>(pObject));
    else
        Plr->GetSession()->SendInventoryList(static_cast<Creature*>(pObject));
}

// TAXIMASTER

void Arcemu::Gossip::FlightMaster::OnHello(Object* pObject, Player* Plr)
{
    Creature* flightmaster = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(flightmaster->GetEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(pObject->GetGUID(), Text, Plr->GetSession()->language);
    menu.AddItem(GOSSIP_ICON_FLIGHTMASTER, Plr->GetSession()->LocalizedGossipOption(FLIGHTMASTER), 1);
    sQuestMgr.FillQuestMenu(flightmaster, Plr, menu);

    menu.StackSend<256>(Plr);
}

void Arcemu::Gossip::FlightMaster::OnSelectOption(Object* pObject, Player* Plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    Plr->GetSession()->SendTaxiList(static_cast<Creature*>(pObject));
}

// AUCTIONEER
void Arcemu::Gossip::Auctioneer::OnHello(Object* pObject, Player* Plr)
{
    Creature* auctioneer = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(auctioneer->GetEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;
    //auctioneers don't offer quests.
    Gossip::Menu::SendQuickMenu(pObject->GetGUID(), Text, Plr, 1, GOSSIP_ICON_VENDOR, Plr->GetSession()->LocalizedGossipOption(AUCTIONEER));
}

void Arcemu::Gossip::Auctioneer::OnSelectOption(Object* pObject, Player* Plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    Plr->GetSession()->SendAuctionList(static_cast<Creature*>(pObject));
}

// INN KEEPERS
void Arcemu::Gossip::InnKeeper::OnHello(Object* pObject, Player* Plr)
{
    Creature* innkeeper = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(innkeeper->GetEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(pObject->GetGUID(), Text, Plr->GetSession()->language);
    menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(INNKEEPER), 1);
    //inn keepers can sell stuff
    if (innkeeper->isVendor())
    {
        MySQLStructure::VendorRestrictions const* vendor = sMySQLStore.getVendorRestriction(innkeeper->GetCreatureProperties()->Id);
        if (Plr->CanBuyAt(vendor))
            menu.AddItem(GOSSIP_ICON_VENDOR, Plr->GetSession()->LocalizedGossipOption(VENDOR), 2);
    }
    sQuestMgr.FillQuestMenu(innkeeper, Plr, menu);
    menu.StackSend<256>(Plr);
}

void Arcemu::Gossip::InnKeeper::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    if (1 == Id)
        Plr->GetSession()->SendInnkeeperBind(static_cast<Creature*>(pObject));
    else
        Plr->GetSession()->SendInventoryList(static_cast<Creature*>(pObject));
}

//BATTLE MASTER
void Arcemu::Gossip::BattleMaster::OnHello(Object* pObject, Player* Plr)
{
    Creature* battlemaster = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(battlemaster->GetEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(battlemaster->GetGUID(), Text, Plr->GetSession()->language);
    menu.AddItem(GOSSIP_ICON_BATTLE, Plr->GetSession()->LocalizedGossipOption(BATTLEMASTER), 1);
    sQuestMgr.FillQuestMenu(battlemaster, Plr, menu);
    menu.StackSend<256>(Plr);
}

void Arcemu::Gossip::BattleMaster::OnSelectOption(Object* pObject, Player* Plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    Plr->GetSession()->SendBattlegroundList(static_cast<Creature*>(pObject), 0);
}

//BANKER
void Arcemu::Gossip::Banker::OnHello(Object* pObject, Player* Plr)
{
    Plr->GetSession()->SendBankerList(static_cast<Creature*>(pObject));
}

void Arcemu::Gossip::Banker::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId)
{

}

//CHARTER GIVER
void Arcemu::Gossip::CharterGiver::OnHello(Object* pObject, Player* Plr)
{
    Creature* chartergiver = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(chartergiver->GetEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    if (chartergiver->isTabardDesigner())
        Gossip::Menu::SendQuickMenu(pObject->GetGUID(), Text, Plr, 1, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(FOUND_GUILD));
    else
        Gossip::Menu::SendQuickMenu(pObject->GetGUID(), Text, Plr, 1, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(FOUND_ARENATEAM));
}

void Arcemu::Gossip::CharterGiver::OnSelectOption(Object* pObject, Player* Plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    Plr->GetSession()->SendCharterRequest(static_cast<Creature*>(pObject));
}

//TABARD DESIGNER
void Arcemu::Gossip::TabardDesigner::OnHello(Object* pObject, Player* Plr)
{
    Creature* chartergiver = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(chartergiver->GetEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(chartergiver->GetGUID(), Text, Plr->GetSession()->language);
    menu.AddItem(GOSSIP_ICON_TABARD, Plr->GetSession()->LocalizedGossipOption(TABARD), 1);
    if (chartergiver->isCharterGiver())
        menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(FOUND_GUILD), 2);

    if (chartergiver->isVendor())
    {
        MySQLStructure::VendorRestrictions const* vendor = sMySQLStore.getVendorRestriction(chartergiver->GetCreatureProperties()->Id);
        if (Plr->CanBuyAt(vendor))
            menu.AddItem(GOSSIP_ICON_VENDOR, Plr->GetSession()->LocalizedGossipOption(VENDOR), 3);
    }
    menu.Send(Plr);
}

void Arcemu::Gossip::TabardDesigner::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId)
{
    switch (Id)
    {
        case 1:
            Plr->GetSession()->SendTabardHelp(static_cast<Creature*>(pObject));
            break;
        case 2:
            if (static_cast<Creature*>(pObject)->isCharterGiver())
                Plr->GetSession()->SendCharterRequest(static_cast<Creature*>(pObject));
            break;
        case 3:
            Plr->GetSession()->SendInventoryList(static_cast<Creature*>(pObject));
            break;
    }
}

// STABLED MASTER
void Arcemu::Gossip::StableMaster::OnHello(Object* pObject, Player* Plr)
{
    Creature* stablemaster = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(stablemaster->GetEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    if (Plr->getClass() == ::HUNTER)
        Gossip::Menu::SendQuickMenu(pObject->GetGUID(), Text, Plr, 1, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(STABLE_MY_PET));
    else
        Gossip::Menu::SendSimpleMenu(pObject->GetGUID(), Text, Plr);
}

void Arcemu::Gossip::StableMaster::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId)
{
    Plr->GetSession()->SendStabledPetList(pObject->GetGUID());
}


// PET TRAINER
void Arcemu::Gossip::PetTrainer::OnHello(Object* pObject, Player* Plr)
{
    Creature* petrain = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(petrain->GetEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(pObject->GetGUID(), Text, Plr->GetSession()->language);
    menu.AddItem(GOSSIP_ICON_TRAINER, Plr->GetSession()->LocalizedGossipOption(BEASTTRAINING), 1);
    if (Plr->getClass() == ::HUNTER && Plr->GetSummon() != NULL)
        menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(PETTRAINER_TALENTRESET), 2);
    sQuestMgr.FillQuestMenu(petrain, Plr, menu);

    menu.StackSend<256>(Plr);
}

void Arcemu::Gossip::PetTrainer::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId)
{
    if (1 == Id)
        Plr->GetSession()->SendTrainerList(static_cast<Creature*>(pObject));
    else if (2 == Id)
        Gossip::Menu::SendQuickMenu(pObject->GetGUID(), TXTID_PETUNTRAIN, Plr, 3, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(PETTRAINER_TALENTRESET));
    else
    {
        Gossip::Menu::Complete(Plr);
        Plr->SendPetUntrainConfirm();
    }

}

// CLASS TRAINER
void Arcemu::Gossip::ClassTrainer::OnHello(Object* pObject, Player* Plr)
{
    Creature* trainer = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(trainer->GetEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(pObject->GetGUID(), Text, Plr->GetSession()->language);
    uint8 playerclass = Plr->getClass();
    ::Trainer* traininfo = trainer->GetTrainer();

    if (traininfo != NULL)    //Seems to happen
    {
        if (traininfo->RequiredClass != playerclass)
            menu.setTextID(traininfo->Cannot_Train_GossipTextId);
        else
        {
            menu.setTextID(traininfo->Can_Train_Gossip_TextId);
            std::string itemname = Plr->GetSession()->LocalizedGossipOption(ISEEK);
            std::string name = trainer->GetCreatureProperties()->Name;

            std::string::size_type pos = name.find(" ");      // only take first name

            if (pos != std::string::npos)
                name = name.substr(0, pos);

            switch (playerclass)
            {
                case ::MAGE:
                    itemname += std::string(Plr->GetSession()->LocalizedGossipOption(GI_MAGE));
                    break;
                case ::SHAMAN:
                    itemname += std::string(Plr->GetSession()->LocalizedGossipOption(GI_SHAMAN));
                    break;
                case ::WARRIOR:
                    itemname += std::string(Plr->GetSession()->LocalizedGossipOption(GI_WARRIOR));
                    break;
                case ::PALADIN:
                    itemname += std::string(Plr->GetSession()->LocalizedGossipOption(GI_PALADIN));
                    break;
                case ::WARLOCK:
                    itemname += std::string(Plr->GetSession()->LocalizedGossipOption(GI_WARLOCK));
                    break;
                case ::HUNTER:
                    itemname += std::string(Plr->GetSession()->LocalizedGossipOption(GI_HUNTER));
                    break;
                case ::ROGUE:
                    itemname += std::string(Plr->GetSession()->LocalizedGossipOption(GI_ROGUE));
                    break;
                case ::DRUID:
                    itemname += std::string(Plr->GetSession()->LocalizedGossipOption(GI_DRUID));
                    break;
                case ::PRIEST:
                    itemname += std::string(Plr->GetSession()->LocalizedGossipOption(GI_PRIEST));
                    break;
                case ::DEATHKNIGHT:
                    itemname += std::string(Plr->GetSession()->LocalizedGossipOption(GI_DEATHKNIGHT));
                    break;
                default:
                    break;
            }
            itemname += " ";
            itemname += std::string(Plr->GetSession()->LocalizedGossipOption(TRAINING)) + ", " + name + ".";

            menu.AddItem(GOSSIP_ICON_TRAINER, itemname.c_str(), 1);

            //talent reset option.
            if (trainer->getLevel() > TrainerTalentResetMinLevel && Plr->getLevel() > worldConfig.player.minTalentResetLevel && trainer->GetTrainer()->RequiredClass == playerclass)
            {
                menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(CLASSTRAINER_TALENTRESET), 2);
                //dual speciliazation option.
                if (Plr->getLevel() >= worldConfig.player.minDualSpecLevel && Plr->m_talentSpecsCount < 2)
                    menu.AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(LEARN_DUAL_TS), 4);
            }
        }
    }
    sQuestMgr.FillQuestMenu(trainer, Plr, menu);
    menu.StackSend<256>(Plr);
}

void Arcemu::Gossip::ClassTrainer::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId)
{
    const char* purchaseconfirm;
    switch (Id)
    {
        case 1:
            Plr->GetSession()->SendTrainerList(static_cast<Creature*>(pObject));
            break;
        case 2:
            Gossip::Menu::SendQuickMenu(pObject->GetGUID(), TXTID_TALENTRESET, Plr, 3, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(CLASSTRAINER_TALENTCONFIRM), 3);
            break;
        case 3:
            Gossip::Menu::Complete(Plr);
            Plr->SendTalentResetConfirm();
            break;
        case 4:
            purchaseconfirm = Plr->GetSession()->LocalizedWorldSrv(Gossip::SURE_TO_PURCHASE_DTS);
            Gossip::Menu::SendQuickMenu(pObject->GetGUID(), TXTID_DUALSPECPURCHASE, Plr, 5, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedWorldSrv(Gossip::PURCHASE_DTS), 10000000, purchaseconfirm);
            break;
        case 5:
            if (!Plr->HasGold(10000000))
            {
                Gossip::Menu::Complete(Plr);
                Plr->GetSession()->SendNotification(Plr->GetSession()->LocalizedWorldSrv(Gossip::NOT_ENOUGH_MONEY_DTS)); // I know this is not correct
            }
            else
            {
                Gossip::Menu::Complete(Plr);
                Plr->ModGold(-10000000);
                Plr->m_talentSpecsCount = 2;
                Plr->Reset_Talents();
                Plr->CastSpell(Plr, 63624, true); // Show activate spec buttons
                Plr->CastSpell(Plr, 63706, true); // Allow primary spec to be activated
                Plr->CastSpell(Plr, 63707, true); // Allow secondary spec to be activated
                Plr->SaveToDB(false); // hai gm i bought dual spec but no werk plis gief mi 1000g back - GTFO you never bought anything
            }
    }
}

void Arcemu::Gossip::Generic::OnHello(Object* pObject, Player* Plr)
{
    //Simply send quests.
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(pObject->GetEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(pObject->GetGUID(), Text, Plr->GetSession()->language);
    sQuestMgr.FillQuestMenu(static_cast<Creature*>(pObject), Plr, menu);
    menu.StackSend<256>(Plr);
}

void Arcemu::Gossip::Generic::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId)
{}
