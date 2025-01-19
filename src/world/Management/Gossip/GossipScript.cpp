/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GossipScript.hpp"
#include "GossipMenu.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestMgr.h"
#include "Objects/GameObject.h"
#include "Storage/MySQLDataStore.hpp"
#include "Objects/Item.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/Script/ScriptMgr.hpp"

void GossipScript::destroy()
{
    delete this;
}

GossipScript* GossipScript::getInterface(Creature* creature)
{
    if (const auto script = sScriptMgr.get_creature_gossip(creature->getEntry()))
        return script;

    return nullptr;
}

GossipScript* GossipScript::getInterface(Item* item)
{
    return sScriptMgr.get_item_gossip(item->getEntry());
}

GossipScript* GossipScript::getInterface(GameObject* gameObject)
{
    return sScriptMgr.get_go_gossip(gameObject->getEntry());
}

void GossipSpiritHealer::onHello(Object* object, Player* player)
{
    if (const auto creature = dynamic_cast<Creature*>(object))
        player->getSession()->sendSpiritHealerRequest(creature);
}

void GossipVendor::onHello(Object* object, Player* player)
{
    if (auto creature = dynamic_cast<Creature*>(object))
    {
        auto gossipTextId = sMySQLStore.getGossipTextIdForNpc(creature->getEntry());
        if (!sMySQLStore.getNpcGossipText(gossipTextId))
            gossipTextId = DefaultGossipTextId;

        GossipMenu menu(creature->getGuid(), gossipTextId, player->getSession()->language);

        const auto vendorRestrictions = sMySQLStore.getVendorRestriction(creature->GetCreatureProperties()->Id);
        if (!player->canBuyAt(vendorRestrictions))
            menu.setTextID(vendorRestrictions->cannotbuyattextid);
        else
            menu.addItem(GOSSIP_ICON_VENDOR, VENDOR, 1);

        sQuestMgr.FillQuestMenu(creature, player, menu);

        menu.sendGossipPacket(player);
    }
}

void GossipVendor::onSelectOption(Object* object, Player* player, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    if (const auto creature = dynamic_cast<Creature*>(object))
        player->getSession()->sendInventoryList(creature);
}

void GossipTrainer::onHello(Object* object, Player* player)
{
    if (auto creature = dynamic_cast<Creature*>(object))
    {
        auto gossipTextId = sMySQLStore.getGossipTextIdForNpc(creature->getEntry());
        if (!sMySQLStore.getNpcGossipText(gossipTextId))
            gossipTextId = DefaultGossipTextId;

        GossipMenu menu(creature->getGuid(), gossipTextId, player->getSession()->language);

        if (const auto trainer = creature->GetTrainer())
        {
            if (!player->canTrainAt(trainer))
            {
                menu.setTextID(trainer->Cannot_Train_GossipTextId);
            }
            else
            {
                std::string name = creature->GetCreatureProperties()->Name;
                std::string::size_type pos = name.find(' ');

                if (pos != std::string::npos)
                    name = name.substr(0, pos);

                auto msg = std::string(player->getSession()->LocalizedGossipOption(ISEEK));
                msg += std::string(player->getSession()->LocalizedGossipOption(TRAINING)) + ", " + name + ".";
                menu.addItem(GOSSIP_ICON_TRAINER, 0, 1, msg);

                if (creature->isVendor())
                {
                    const auto vendorRestrictions = sMySQLStore.getVendorRestriction(creature->GetCreatureProperties()->Id);
                    if (player->canBuyAt(vendorRestrictions))
                        menu.addItem(GOSSIP_ICON_VENDOR, VENDOR, 2);
                }
            }
        }

        sQuestMgr.FillQuestMenu(creature, player, menu);

        menu.sendGossipPacket(player);
    }
}

void GossipTrainer::onSelectOption(Object* object, Player* player, uint32_t Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    if (const auto creature = dynamic_cast<Creature*>(object))
    {
        if (1 == Id)
            player->getSession()->sendTrainerList(creature);
        else
            player->getSession()->sendInventoryList(creature);
    }
}

void GossipFlightMaster::onHello(Object* object, Player* player)
{
    if (const auto creature = dynamic_cast<Creature*>(object))
    {
        auto gossipTextId = sMySQLStore.getGossipTextIdForNpc(creature->getEntry());
        if (!sMySQLStore.getNpcGossipText(gossipTextId))
            gossipTextId = DefaultGossipTextId;

        GossipMenu menu(object->getGuid(), gossipTextId, player->getSession()->language);

        player->getSession()->sendLearnNewTaxiNode(creature);

        menu.addItem(GOSSIP_ICON_FLIGHTMASTER, FLIGHTMASTER, 1);

        sQuestMgr.FillQuestMenu(creature, player, menu);

        menu.sendGossipPacket(player);
    }
}

void GossipFlightMaster::onSelectOption(Object* object, Player* player, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    if (const auto creature = dynamic_cast<Creature*>(object))
        player->getSession()->sendTaxiMenu(creature);
}

void GossipAuctioneer::onHello(Object* object, Player* player)
{
    if (const auto creature = dynamic_cast<Creature*>(object))
    {
        auto gossipTextId = sMySQLStore.getGossipTextIdForNpc(creature->getEntry());
        if (!sMySQLStore.getNpcGossipText(gossipTextId))
            gossipTextId = DefaultGossipTextId;

        GossipMenu::sendQuickMenu(object->getGuid(), gossipTextId, player, 1, GOSSIP_ICON_VENDOR, player->getSession()->LocalizedGossipOption(AUCTIONEER));
    }
}

void GossipAuctioneer::onSelectOption(Object* object, Player* player, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    player->getSession()->sendAuctionList(dynamic_cast<Creature*>(object));
}

void GossipInnKeeper::onHello(Object* object, Player* player)
{
    if (auto creature = dynamic_cast<Creature*>(object))
    {
        auto gossipTextId = sMySQLStore.getGossipTextIdForNpc(creature->getEntry());
        if (!sMySQLStore.getNpcGossipText(gossipTextId))
            gossipTextId = DefaultGossipTextId;

        GossipMenu menu(object->getGuid(), gossipTextId, player->getSession()->language);

        menu.addItem(GOSSIP_ICON_CHAT, INNKEEPER, 1);

        if (creature->isVendor())
        {
            const auto vendorRestrictions = sMySQLStore.getVendorRestriction(creature->GetCreatureProperties()->Id);
            if (player->canBuyAt(vendorRestrictions))
                menu.addItem(GOSSIP_ICON_VENDOR, VENDOR, 2);
        }

        sQuestMgr.FillQuestMenu(creature, player, menu);

        menu.sendGossipPacket(player);
    }
}

void GossipInnKeeper::onSelectOption(Object* object, Player* player, uint32_t Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    if (const auto creature = dynamic_cast<Creature*>(object))
    {
        if (1 == Id)
            player->getSession()->sendInnkeeperBind(creature);
        else
            player->getSession()->sendInventoryList(creature);
    }
}

void GossipBattleMaster::onHello(Object* object, Player* player)
{
    if (const auto creature = dynamic_cast<Creature*>(object))
    {
        auto gossipTextId = sMySQLStore.getGossipTextIdForNpc(creature->getEntry());
        if (!sMySQLStore.getNpcGossipText(gossipTextId))
            gossipTextId = DefaultGossipTextId;

        GossipMenu menu(creature->getGuid(), gossipTextId, player->getSession()->language);

        menu.addItem(GOSSIP_ICON_BATTLE, BATTLEMASTER, 1);

        sQuestMgr.FillQuestMenu(creature, player, menu);

        menu.sendGossipPacket(player);
    }
}

void GossipBattleMaster::onSelectOption(Object* object, Player* player, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    player->getSession()->sendBattlegroundList(dynamic_cast<Creature*>(object), 0);
}

void GossipBanker::onHello(Object* object, Player* player)
{
    player->getSession()->sendBankerList(dynamic_cast<Creature*>(object));
}

void GossipBanker::onSelectOption(Object* /*object*/, Player* /*player*/, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
}

void GossipCharterGiver::onHello(Object* object, Player* player)
{
    if (const auto creature = dynamic_cast<Creature*>(object))
    {
        auto gossipTextId = sMySQLStore.getGossipTextIdForNpc(creature->getEntry());
        if (!sMySQLStore.getNpcGossipText(gossipTextId))
            gossipTextId = DefaultGossipTextId;

        if (creature->isTabardDesigner())
            GossipMenu::sendQuickMenu(object->getGuid(), gossipTextId, player, 1, GOSSIP_ICON_CHAT, player->getSession()->LocalizedGossipOption(FOUND_GUILD));
        else
            GossipMenu::sendQuickMenu(object->getGuid(), gossipTextId, player, 1, GOSSIP_ICON_CHAT, player->getSession()->LocalizedGossipOption(FOUND_ARENATEAM));
    }
}

void GossipCharterGiver::onSelectOption(Object* object, Player* player, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    player->getSession()->sendCharterRequest(dynamic_cast<Creature*>(object));
}

void GossipTabardDesigner::onHello(Object* object, Player* player)
{
    if (auto creature = dynamic_cast<Creature*>(object))
    {
        auto gossipTextId = sMySQLStore.getGossipTextIdForNpc(creature->getEntry());
        if (!sMySQLStore.getNpcGossipText(gossipTextId))
            gossipTextId = DefaultGossipTextId;

        GossipMenu menu(creature->getGuid(), gossipTextId, player->getSession()->language);

        menu.addItem(GOSSIP_ICON_TABARD, TABARD, 1);

        if (creature->isCharterGiver())
            menu.addItem(GOSSIP_ICON_CHAT, FOUND_GUILD, 2);

        if (creature->isVendor())
        {
            const auto vendorRestrictions = sMySQLStore.getVendorRestriction(creature->GetCreatureProperties()->Id);
            if (player->canBuyAt(vendorRestrictions))
                menu.addItem(GOSSIP_ICON_VENDOR, VENDOR, 3);
        }

        menu.sendGossipPacket(player);
    }
}

void GossipTabardDesigner::onSelectOption(Object* object, Player* player, uint32_t Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    switch (Id)
    {
        case 1:
            player->getSession()->sendTabardHelp(dynamic_cast<Creature*>(object));
            break;
        case 2:
        {
            if (const auto creature = dynamic_cast<Creature*>(object))
                if (creature->isCharterGiver())
                    player->getSession()->sendCharterRequest(creature);
        } break;
        case 3:
        {
            if (const auto creature = dynamic_cast<Creature*>(object))
                player->getSession()->sendInventoryList(creature);
        } break;
        default: 
            break;
    }
}

void GossipStableMaster::onHello(Object* object, Player* player)
{
    uint32_t gossipTextId = 0;
    if (const auto creature = dynamic_cast<Creature*>(object))
        gossipTextId = sMySQLStore.getGossipTextIdForNpc(creature->getEntry());

    if (!sMySQLStore.getNpcGossipText(gossipTextId))
        gossipTextId = DefaultGossipTextId;

    if (player->getClass() == ::HUNTER)
        GossipMenu::sendQuickMenu(object->getGuid(), gossipTextId, player, 1, GOSSIP_ICON_CHAT, player->getSession()->LocalizedGossipOption(STABLE_MY_PET));
    else
        GossipMenu::sendSimpleMenu(object->getGuid(), gossipTextId, player);
}

void GossipStableMaster::onSelectOption(Object* object, Player* player, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    player->getSession()->sendStabledPetList(object->getGuid());
}

void GossipPetTrainer::onHello(Object* object, Player* player)
{
    if (const auto creature = dynamic_cast<Creature*>(object))
    {
        auto gossipTextId = sMySQLStore.getGossipTextIdForNpc(creature->getEntry());
        if (!sMySQLStore.getNpcGossipText(gossipTextId))
            gossipTextId = DefaultGossipTextId;

        GossipMenu menu(object->getGuid(), gossipTextId, player->getSession()->language);

        menu.addItem(GOSSIP_ICON_TRAINER, BEASTTRAINING, 1);

        if (player->getClass() == ::HUNTER && player->getPet() != nullptr)
            menu.addItem(GOSSIP_ICON_CHAT, PETTRAINER_TALENTRESET, 2);

        sQuestMgr.FillQuestMenu(creature, player, menu);

        menu.sendGossipPacket(player);
    }
}

void GossipPetTrainer::onSelectOption(Object* object, Player* player, uint32_t Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    switch(Id)
    {
        case 1:
        {
            if (const auto creature = dynamic_cast<Creature*>(object))
                player->getSession()->sendTrainerList(creature);
        } break;
        case 2:
        {
            GossipMenu::sendQuickMenu(object->getGuid(), TXTID_PETUNTRAIN, player, 3, GOSSIP_ICON_CHAT, player->getSession()->LocalizedGossipOption(PETTRAINER_TALENTRESET));
        } break;
        default:
        {
            GossipMenu::senGossipComplete(player);
            player->sendPetUnlearnConfirmPacket();
        } break;
    }
}

void GossipClassTrainer::onHello(Object* object, Player* player)
{
    if (auto creature = dynamic_cast<Creature*>(object))
    {
        const auto playerSession = player->getSession();

        auto gossipTextId = sMySQLStore.getGossipTextIdForNpc(creature->getEntry());
        if (!sMySQLStore.getNpcGossipText(gossipTextId))
            gossipTextId = DefaultGossipTextId;

        GossipMenu menu(object->getGuid(), gossipTextId, playerSession->language);

        const auto playerClass = player->getClass();
        auto trainer = creature->GetTrainer();

        if (trainer)
        {
            if (trainer->RequiredClass != playerClass)
            {
                menu.setTextID(trainer->Cannot_Train_GossipTextId);
            }
            else
            {
                menu.setTextID(trainer->Can_Train_Gossip_TextId);

                std::string menuItemName = playerSession->LocalizedGossipOption(ISEEK);
                std::string creatureName = creature->GetCreatureProperties()->Name;

                std::string::size_type pos = creatureName.find(' ');

                if (pos != std::string::npos)
                    creatureName = creatureName.substr(0, pos);

                switch (playerClass)
                {
                    case ::MAGE:
                        menuItemName += std::string(playerSession->LocalizedGossipOption(GI_MAGE));
                        break;
                    case ::SHAMAN:
                        menuItemName += std::string(playerSession->LocalizedGossipOption(GI_SHAMAN));
                        break;
                    case ::WARRIOR:
                        menuItemName += std::string(playerSession->LocalizedGossipOption(GI_WARRIOR));
                        break;
                    case ::PALADIN:
                        menuItemName += std::string(playerSession->LocalizedGossipOption(GI_PALADIN));
                        break;
                    case ::WARLOCK:
                        menuItemName += std::string(playerSession->LocalizedGossipOption(GI_WARLOCK));
                        break;
                    case ::HUNTER:
                        menuItemName += std::string(playerSession->LocalizedGossipOption(GI_HUNTER));
                        break;
                    case ::ROGUE:
                        menuItemName += std::string(playerSession->LocalizedGossipOption(GI_ROGUE));
                        break;
                    case ::DRUID:
                        menuItemName += std::string(playerSession->LocalizedGossipOption(GI_DRUID));
                        break;
                    case ::PRIEST:
                        menuItemName += std::string(playerSession->LocalizedGossipOption(GI_PRIEST));
                        break;
    #if VERSION_STRING > TBC
                    case ::DEATHKNIGHT:
                        menuItemName += std::string(playerSession->LocalizedGossipOption(GI_DEATHKNIGHT));
                        break;
    #endif
                    default:
                        break;
                }
                menuItemName += " ";
                menuItemName += std::string(playerSession->LocalizedGossipOption(TRAINING)) + ", " + creatureName + ".";

                menu.addItem(GOSSIP_ICON_TRAINER, 0, 1, menuItemName);

                if (creature->getLevel() > TrainerTalentResetMinLevel && player->getLevel() > worldConfig.player.minTalentResetLevel && creature->GetTrainer()->RequiredClass == playerClass)
                {
                    menu.addItem(GOSSIP_ICON_CHAT, CLASSTRAINER_TALENTRESET, 2);

                    if (player->getLevel() >= worldConfig.player.minDualSpecLevel && player->m_talentSpecsCount < 2)
                        menu.addItem(GOSSIP_ICON_CHAT, LEARN_DUAL_TS, 4);
                }
            }
        }

        sQuestMgr.FillQuestMenu(creature, player, menu);

        menu.sendGossipPacket(player);
    }
}

void GossipClassTrainer::onSelectOption(Object* object, Player* player, uint32_t Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
    const auto playerSession = player->getSession();

    switch (Id)
    {
        case 1:
        {
            if (const auto creature = dynamic_cast<Creature*>(object))
                playerSession->sendTrainerList(creature);
        } break;
        case 2:
        {
            GossipMenu::sendQuickMenu(object->getGuid(), TXTID_TALENTRESET, player, 3, GOSSIP_ICON_CHAT, playerSession->LocalizedGossipOption(CLASSTRAINER_TALENTCONFIRM), 3);
        } break;
        case 3:
        {
            GossipMenu::senGossipComplete(player);
            player->sendTalentResetConfirmPacket();
        } break;
        case 4:
        {
            GossipMenu::sendQuickMenu(object->getGuid(), TXTID_DUALSPECPURCHASE, player, 5, GOSSIP_ICON_CHAT, playerSession->LocalizedWorldSrv(PURCHASE_DTS), 10000000, playerSession->LocalizedWorldSrv(SURE_TO_PURCHASE_DTS));
        } break;
        case 5:
        {
            if (!player->hasEnoughCoinage(10000000))
            {
                GossipMenu::senGossipComplete(player);
                playerSession->SendNotification(playerSession->LocalizedWorldSrv(NOT_ENOUGH_MONEY_DTS));
            }
            else
            {
                GossipMenu::senGossipComplete(player);

                player->modCoinage(-10000000);
                player->m_talentSpecsCount = 2;
                player->castSpell(player, 63624, true); // Show activate spec buttons
                player->castSpell(player, 63706, true); // Allow primary spec to be activated
                player->castSpell(player, 63707, true); // Allow secondary spec to be activated
                player->saveToDB(false);
            }
        } break;
        default:
            break;
    }
}

void GossipGeneric::onHello(Object* object, Player* player)
{
    auto gossipTextId = sMySQLStore.getGossipTextIdForNpc(object->getEntry());
    if (!sMySQLStore.getNpcGossipText(gossipTextId))
        gossipTextId = DefaultGossipTextId;

    if (const auto creature = dynamic_cast<Creature*>(object))
    {
        GossipMenu menu(object->getGuid(), gossipTextId, player->getSession()->language);

        sQuestMgr.FillQuestMenu(creature, player, menu);

        menu.sendGossipPacket(player);
    }
}

void GossipGeneric::onSelectOption(Object* /*object*/, Player* /*player*/, uint32_t /*Id*/, const char* /*EnteredCode*/, uint32_t /*gossipId*/)
{
}
