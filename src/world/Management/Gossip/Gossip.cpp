/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Server/Packets/SmsgGossipMessage.h"

using namespace Arcemu;
using namespace AscEmu::Packets;

Gossip::Menu::Menu(uint64 Creature_Guid, uint32 Text_Id, uint32 language, uint32 gossip_id) : 
textid_(Text_Id), guid_(Creature_Guid), language_(language), gossipId(gossip_id)
{}

//MIT
void Gossip::Menu::addItem(uint8_t icon, uint32_t textId, uint32_t id, std::string text /*= ""*/, uint32_t boxMoney /*= 0*/, std::string boxMessage /*= ""*/, bool isCoded /*= false*/)
{
    const GossipItem item(icon, text, textId, isCoded, boxMoney, boxMessage);
    this->itemlist_.insert(std::make_pair(id, item));
}

//MIT
void Gossip::Menu::removeItem(uint32_t id)
{
    const auto itr = itemlist_.find(id);
    if (itr != itemlist_.end())
        itemlist_.erase(itr);
}

void Gossip::Menu::AddQuest(QuestProperties const* quest, uint8 icon)
{
    const GossipQuestItem questItem(icon, quest->min_level, quest->quest_flags);
    this->questlist_.insert(std::make_pair(quest->id, questItem));
}

void Gossip::Menu::RemoveQuest(uint32_t id)
{
    const auto itr = questlist_.find(id);
    if (itr != questlist_.end())
        questlist_.erase(itr);
}

//MIT
void Gossip::Menu::sendGossipPacket(Player* player) const
{
    player->GetSession()->SendPacket(SmsgGossipMessage(guid_, gossipId, textid_, language_, itemlist_, questlist_).serialise().get());
}

//MIT
void Gossip::Menu::sendSimpleMenu(uint64_t guid, uint32_t txt_id, Player* plr)
{
    plr->GetSession()->SendPacket(SmsgGossipMessage(guid, 0, txt_id, 0, {}, {}).serialise().get());
}

//MIT
void Gossip::Menu::sendQuickMenu(uint64_t guid, uint32_t textid, Player* Plr, uint32_t itemid, uint8_t itemicon, std::string itemtext, uint32_t requiredmoney/*=0*/, std::string moneytext/*=""*/, bool extra/*=false*/)
{
    std::map<uint32_t, GossipItem> tempItemList;
    const GossipItem tempItem(itemicon, itemtext, 0, extra, requiredmoney, moneytext);
    tempItemList.insert(std::make_pair(itemid, tempItem));

    Plr->GetSession()->SendPacket(SmsgGossipMessage(guid, 0, textid, 0, tempItemList, {}).serialise().get());
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
    Gossip::Script* script = sScriptMgr.get_creature_gossip(creature->getEntry());
    if (script != NULL)
        return script;

    if (creature->isSpiritHealer())
        return &sScriptMgr.spirithealerScript_;
    else if (creature->isInnkeeper())
        return &sScriptMgr.innkeeperScript_;
    else if (creature->isBanker())
        return &sScriptMgr.bankerScript_;
    else if (creature->isClassTrainer())
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
    else if (creature->isAuctioneer())
        return &sScriptMgr.auctioneerScript_;
    else if (creature->isCharterGiver())
        return &sScriptMgr.chartergiverScript_;
    else if (creature->isVendor())
        return &sScriptMgr.vendorScript_;

    return &sScriptMgr.genericScript_;
}
Gossip::Script* Gossip::Script::GetInterface(::Item* item)
{
    return sScriptMgr.get_item_gossip(item->getEntry());
}
Gossip::Script* Gossip::Script::GetInterface(GameObject* go)
{
    return sScriptMgr.get_go_gossip(go->getEntry());
}

// SPIRIT HEALER
void Arcemu::Gossip::SpiritHealer::OnHello(Object* pObject, Player* Plr)
{
    Plr->GetSession()->sendSpiritHealerRequest(static_cast<Creature*>(pObject));
}

// VENDORS
void Arcemu::Gossip::Vendor::OnHello(Object* pObject, Player* Plr)
{
    Creature* creature = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(creature->getEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    MySQLStructure::VendorRestrictions const* vendor = sMySQLStore.getVendorRestriction(creature->GetCreatureProperties()->Id);

    Gossip::Menu menu(creature->getGuid(), Text, Plr->GetSession()->language);

    if (!Plr->CanBuyAt(vendor))
        menu.setTextID(vendor->cannotbuyattextid);
    else
        menu.addItem(GOSSIP_ICON_VENDOR, VENDOR, 1);

    sQuestMgr.FillQuestMenu(creature, Plr, menu); //add any quests we have.

    menu.sendGossipPacket(Plr);
}

void Arcemu::Gossip::Vendor::OnSelectOption(Object* pObject, Player* Plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    Plr->GetSession()->sendInventoryList(static_cast<Creature*>(pObject));
}

//TRAINER
void Arcemu::Gossip::Trainer::OnHello(Object* pObject, Player* Plr)
{
    Creature* trainer = static_cast<Creature*>(pObject);
    ::Trainer* trainerinfo = trainer->GetTrainer();
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(trainer->getEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    std::string name = trainer->GetCreatureProperties()->Name;
    std::string::size_type pos = name.find(" ");      // only take first name

    if (pos != std::string::npos)
        name = name.substr(0, pos);

    Gossip::Menu menu(trainer->getGuid(), Text, Plr->GetSession()->language);
    if (trainerinfo != NULL)
    {
        if (!Plr->CanTrainAt(trainerinfo))
            menu.setTextID(trainerinfo->Cannot_Train_GossipTextId);
        else
        {
            // I seek
            std::string msg = std::string(Plr->GetSession()->LocalizedGossipOption(ISEEK));
            msg += std::string(Plr->GetSession()->LocalizedGossipOption(TRAINING)) + ", " + name + ".";
            menu.addItem(GOSSIP_ICON_TRAINER, 0, 1, msg);

            if (trainer->isVendor())
            {
                MySQLStructure::VendorRestrictions const* vendor = sMySQLStore.getVendorRestriction(trainer->GetCreatureProperties()->Id);
                if (Plr->CanBuyAt(vendor))
                    menu.addItem(GOSSIP_ICON_VENDOR, VENDOR, 2);
            }
        }
    }
    sQuestMgr.FillQuestMenu(trainer, Plr, menu);
    menu.sendGossipPacket(Plr);
}

void Arcemu::Gossip::Trainer::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    if (1 == Id)
        Plr->GetSession()->sendTrainerList(static_cast<Creature*>(pObject));
    else
        Plr->GetSession()->sendInventoryList(static_cast<Creature*>(pObject));
}

// TAXIMASTER

void Arcemu::Gossip::FlightMaster::OnHello(Object* pObject, Player* Plr)
{
    Creature* flightmaster = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(flightmaster->getEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(pObject->getGuid(), Text, Plr->GetSession()->language);
    menu.addItem(GOSSIP_ICON_FLIGHTMASTER, FLIGHTMASTER, 1);
    sQuestMgr.FillQuestMenu(flightmaster, Plr, menu);

    menu.sendGossipPacket(Plr);
}

void Arcemu::Gossip::FlightMaster::OnSelectOption(Object* pObject, Player* Plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    Plr->GetSession()->sendTaxiList(static_cast<Creature*>(pObject));
}

// AUCTIONEER
void Arcemu::Gossip::Auctioneer::OnHello(Object* pObject, Player* Plr)
{
    Creature* auctioneer = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(auctioneer->getEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;
    //auctioneers don't offer quests.
    Gossip::Menu::sendQuickMenu(pObject->getGuid(), Text, Plr, 1, GOSSIP_ICON_VENDOR, Plr->GetSession()->LocalizedGossipOption(AUCTIONEER));
}

void Arcemu::Gossip::Auctioneer::OnSelectOption(Object* pObject, Player* Plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    Plr->GetSession()->sendAuctionList(static_cast<Creature*>(pObject));
}

// INN KEEPERS
void Arcemu::Gossip::InnKeeper::OnHello(Object* pObject, Player* Plr)
{
    Creature* innkeeper = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(innkeeper->getEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(pObject->getGuid(), Text, Plr->GetSession()->language);
    menu.addItem(GOSSIP_ICON_CHAT, INNKEEPER, 1);
    //inn keepers can sell stuff
    if (innkeeper->isVendor())
    {
        MySQLStructure::VendorRestrictions const* vendor = sMySQLStore.getVendorRestriction(innkeeper->GetCreatureProperties()->Id);
        if (Plr->CanBuyAt(vendor))
            menu.addItem(GOSSIP_ICON_VENDOR, VENDOR, 2);
    }
    sQuestMgr.FillQuestMenu(innkeeper, Plr, menu);
    menu.sendGossipPacket(Plr);
}

void Arcemu::Gossip::InnKeeper::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    if (1 == Id)
        Plr->GetSession()->sendInnkeeperBind(static_cast<Creature*>(pObject));
    else
        Plr->GetSession()->sendInventoryList(static_cast<Creature*>(pObject));
}

//BATTLE MASTER
void Arcemu::Gossip::BattleMaster::OnHello(Object* pObject, Player* Plr)
{
    Creature* battlemaster = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(battlemaster->getEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(battlemaster->getGuid(), Text, Plr->GetSession()->language);
    menu.addItem(GOSSIP_ICON_BATTLE, BATTLEMASTER, 1);
    sQuestMgr.FillQuestMenu(battlemaster, Plr, menu);
    menu.sendGossipPacket(Plr);
}

void Arcemu::Gossip::BattleMaster::OnSelectOption(Object* pObject, Player* Plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    Plr->GetSession()->sendBattlegroundList(static_cast<Creature*>(pObject), 0);
}

//BANKER
void Arcemu::Gossip::Banker::OnHello(Object* pObject, Player* Plr)
{
    Plr->GetSession()->sendBankerList(static_cast<Creature*>(pObject));
}

void Arcemu::Gossip::Banker::OnSelectOption(Object* /*pObject*/, Player* /*Plr*/, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{

}

//CHARTER GIVER
void Arcemu::Gossip::CharterGiver::OnHello(Object* pObject, Player* Plr)
{
    Creature* chartergiver = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(chartergiver->getEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    if (chartergiver->isTabardDesigner())
        Gossip::Menu::sendQuickMenu(pObject->getGuid(), Text, Plr, 1, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(FOUND_GUILD));
    else
        Gossip::Menu::sendQuickMenu(pObject->getGuid(), Text, Plr, 1, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(FOUND_ARENATEAM));
}

void Arcemu::Gossip::CharterGiver::OnSelectOption(Object* pObject, Player* Plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    Plr->GetSession()->sendCharterRequest(static_cast<Creature*>(pObject));
}

//TABARD DESIGNER
void Arcemu::Gossip::TabardDesigner::OnHello(Object* pObject, Player* Plr)
{
    Creature* chartergiver = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(chartergiver->getEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(chartergiver->getGuid(), Text, Plr->GetSession()->language);
    menu.addItem(GOSSIP_ICON_TABARD, TABARD, 1);
    if (chartergiver->isCharterGiver())
        menu.addItem(GOSSIP_ICON_CHAT, FOUND_GUILD, 2);

    if (chartergiver->isVendor())
    {
        MySQLStructure::VendorRestrictions const* vendor = sMySQLStore.getVendorRestriction(chartergiver->GetCreatureProperties()->Id);
        if (Plr->CanBuyAt(vendor))
            menu.addItem(GOSSIP_ICON_VENDOR, VENDOR, 3);
    }
    menu.sendGossipPacket(Plr);
}

void Arcemu::Gossip::TabardDesigner::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    switch (Id)
    {
        case 1:
            Plr->GetSession()->sendTabardHelp(static_cast<Creature*>(pObject));
            break;
        case 2:
            if (static_cast<Creature*>(pObject)->isCharterGiver())
                Plr->GetSession()->sendCharterRequest(static_cast<Creature*>(pObject));
            break;
        case 3:
            Plr->GetSession()->sendInventoryList(static_cast<Creature*>(pObject));
            break;
    }
}

// STABLED MASTER
void Arcemu::Gossip::StableMaster::OnHello(Object* pObject, Player* Plr)
{
    Creature* stablemaster = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(stablemaster->getEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    if (Plr->getClass() == ::HUNTER)
        Gossip::Menu::sendQuickMenu(pObject->getGuid(), Text, Plr, 1, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(STABLE_MY_PET));
    else
        Gossip::Menu::sendSimpleMenu(pObject->getGuid(), Text, Plr);
}

void Arcemu::Gossip::StableMaster::OnSelectOption(Object* pObject, Player* Plr, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    Plr->GetSession()->sendStabledPetList(pObject->getGuid());
}


// PET TRAINER
void Arcemu::Gossip::PetTrainer::OnHello(Object* pObject, Player* Plr)
{
    Creature* petrain = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(petrain->getEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(pObject->getGuid(), Text, Plr->GetSession()->language);
    menu.addItem(GOSSIP_ICON_TRAINER, BEASTTRAINING, 1);
    if (Plr->getClass() == ::HUNTER && Plr->GetSummon() != NULL)
        menu.addItem(GOSSIP_ICON_CHAT, PETTRAINER_TALENTRESET, 2);
    sQuestMgr.FillQuestMenu(petrain, Plr, menu);

    menu.sendGossipPacket(Plr);
}

void Arcemu::Gossip::PetTrainer::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    if (1 == Id)
        Plr->GetSession()->sendTrainerList(static_cast<Creature*>(pObject));
    else if (2 == Id)
        Gossip::Menu::sendQuickMenu(pObject->getGuid(), TXTID_PETUNTRAIN, Plr, 3, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(PETTRAINER_TALENTRESET));
    else
    {
        Gossip::Menu::Complete(Plr);
        Plr->sendPetUnlearnConfirmPacket();
    }

}

// CLASS TRAINER
void Arcemu::Gossip::ClassTrainer::OnHello(Object* pObject, Player* Plr)
{
    Creature* trainer = static_cast<Creature*>(pObject);
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(trainer->getEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(pObject->getGuid(), Text, Plr->GetSession()->language);
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
#if VERSION_STRING > TBC
                case ::DEATHKNIGHT:
                    itemname += std::string(Plr->GetSession()->LocalizedGossipOption(GI_DEATHKNIGHT));
                    break;
#endif
                default:
                    break;
            }
            itemname += " ";
            itemname += std::string(Plr->GetSession()->LocalizedGossipOption(TRAINING)) + ", " + name + ".";

            menu.addItem(GOSSIP_ICON_TRAINER, 0, 1, itemname);

            //talent reset option.
            if (trainer->getLevel() > TrainerTalentResetMinLevel && Plr->getLevel() > worldConfig.player.minTalentResetLevel && trainer->GetTrainer()->RequiredClass == playerclass)
            {
                menu.addItem(GOSSIP_ICON_CHAT, CLASSTRAINER_TALENTRESET, 2);
                //dual speciliazation option.
                if (Plr->getLevel() >= worldConfig.player.minDualSpecLevel && Plr->m_talentSpecsCount < 2)
                    menu.addItem(GOSSIP_ICON_CHAT, LEARN_DUAL_TS, 4);
            }
        }
    }
    sQuestMgr.FillQuestMenu(trainer, Plr, menu);
    menu.sendGossipPacket(Plr);
}

void Arcemu::Gossip::ClassTrainer::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    const char* purchaseconfirm;
    switch (Id)
    {
        case 1:
            Plr->GetSession()->sendTrainerList(static_cast<Creature*>(pObject));
            break;
        case 2:
            Gossip::Menu::sendQuickMenu(pObject->getGuid(), TXTID_TALENTRESET, Plr, 3, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(CLASSTRAINER_TALENTCONFIRM), 3);
            break;
        case 3:
            Gossip::Menu::Complete(Plr);
            Plr->sendTalentResetConfirmPacket();
            break;
        case 4:
            purchaseconfirm = Plr->GetSession()->LocalizedWorldSrv(SURE_TO_PURCHASE_DTS);
            Gossip::Menu::sendQuickMenu(pObject->getGuid(), TXTID_DUALSPECPURCHASE, Plr, 5, GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedWorldSrv(PURCHASE_DTS), 10000000, purchaseconfirm);
            break;
        case 5:
            if (!Plr->hasEnoughCoinage(10000000))
            {
                Gossip::Menu::Complete(Plr);
                Plr->GetSession()->SendNotification(Plr->GetSession()->LocalizedWorldSrv(NOT_ENOUGH_MONEY_DTS)); // I know this is not correct
            }
            else
            {
                Gossip::Menu::Complete(Plr);
                Plr->modCoinage(-10000000);
                Plr->m_talentSpecsCount = 2;
                Plr->castSpell(Plr, 63624, true); // Show activate spec buttons
                Plr->castSpell(Plr, 63706, true); // Allow primary spec to be activated
                Plr->castSpell(Plr, 63707, true); // Allow secondary spec to be activated
                Plr->SaveToDB(false); // hai gm i bought dual spec but no werk plis gief mi 1000g back - GTFO you never bought anything
            }
    }
}

void Arcemu::Gossip::Generic::OnHello(Object* pObject, Player* Plr)
{
    //Simply send quests.
    uint32 Text = sMySQLStore.getGossipTextIdForNpc(pObject->getEntry());
    if (sMySQLStore.getNpcText(Text) == nullptr)
        Text = DefaultGossipTextId;

    Gossip::Menu menu(pObject->getGuid(), Text, Plr->GetSession()->language);
    sQuestMgr.FillQuestMenu(static_cast<Creature*>(pObject), Plr, menu);
    menu.sendGossipPacket(Plr);
}

void Arcemu::Gossip::Generic::OnSelectOption(Object* /*pObject*/, Player* /*Plr*/, uint32 /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{}
