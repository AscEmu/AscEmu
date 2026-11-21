/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Server/Packets/CmsgAuctionListOwnerItems.h"
#include "Server/Packets/CmsgAuctionListItems.h"
#include "Server/Packets/CmsgAuctionRemoveItem.h"
#include "Server/Packets/SmsgAuctionCommandResult.h"
#include "Server/Packets/CmsgAuctionListIBidderItemse.h"
#include "Server/Packets/CmsgAuctionListIPendingSales.h"
#include "Server/Packets/CmsgAuctionPlaceBid.h"
#include "Server/Packets/CmsgAuctionSellItem.h"
#include "Server/WorldSession.h"
#include "Objects/Units/Players/Player.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Management/AuctionMgr.hpp"
#include "Management/ItemInterface.h"
#include "Management/MailMgr.h"
#include "Map/Maps/WorldMap.hpp"
#include "Server/Definitions.h"

using namespace AscEmu::Packets;

void WorldSession::handleAuctionListOwnerItems(WorldPacket& recvPacket)
{
    CmsgAuctionListOwnerItems srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_AUCTION_LIST_OWNER_ITEMS {} (guidLow).", srlPacket.guid.getCounter());

    const auto creature = _player->getWorldMapCreature(srlPacket.guid.getRawGuid());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    creature->auctionHouse->sendOwnerListPacket(_player);
}

void WorldSession::handleAuctionListItems(WorldPacket& recvPacket)
{
    CmsgAuctionListItems srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_AUCTION_LIST_ITEMS {} (guidLow).", srlPacket.guid.getCounter());

    const auto creature = _player->getWorldMapCreature(srlPacket.guid.getRawGuid());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    creature->auctionHouse->sendAuctionList(_player, srlPacket);
}

void WorldSession::handleCancelAuction(WorldPacket& recvPacket)
{
    CmsgAuctionRemoveItem srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_AUCTION_REMOVE_ITEM {} (auctionId).", srlPacket.auctionId);

    const auto creature = _player->getWorldMapCreature(srlPacket.guid.getRawGuid());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    const auto auction = creature->auctionHouse->getAuction(srlPacket.auctionId);
    if (auction == nullptr)
        return;

    creature->auctionHouse->queueDeletion(auction, AUCTION_REMOVE_CANCELLED);

    SmsgAuctionCommandResult managedPacket(srlPacket.auctionId, AUCTION_ACTION_CANCEL, AUCTION_ERROR_NONE);
    sendManagedPacket(managedPacket);

    creature->auctionHouse->sendOwnerListPacket(_player);
}

void WorldSession::handleAuctionListBidderItems(WorldPacket& recvPacket)
{
    CmsgAuctionListIBidderItemse srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_AUCTION_LIST_BIDDER_ITEMS {} (lowguid).", srlPacket.guid.getCounter());

    const auto creature = _player->getWorldMapCreature(srlPacket.guid.getRawGuid());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    creature->auctionHouse->sendBidListPacket(_player);
}

void WorldSession::handleAuctionListPendingSales([[maybe_unused]] WorldPacket& recvPacket)
{
    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK))
        return;

    CmsgAuctionListIPendingSales srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_AUCTION_LIST_PRENDING_SALES {} (lowguid).", srlPacket.guid.getCounter());

    //\todo SMSG_AUCTION_LIST_PENDING_SALES needs to be researched!
}

void WorldSession::handleAuctionSellItem(WorldPacket& recvPacket)
{
    CmsgAuctionSellItem srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_AUCTION_SELL_ITEM.");

    if (!srlPacket.bidMoney || !srlPacket.expireTime)
        return;

    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata))
    {
        constexpr uint64_t maxPreCataMoney = std::numeric_limits<int32_t>::max();
        if (srlPacket.bidMoney > maxPreCataMoney || srlPacket.buyoutPrice > maxPreCataMoney)
            return;
    }

    const auto creature = _player->getWorldMapCreature(srlPacket.auctioneerGuid.getRawGuid());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    const auto auctionHouse = creature->auctionHouse;

    const bool isTbcOrNewer = WoW::isServerExpansionAtLeast(WoW::Expansion::_TBC);
    const uint32_t minAuctionTime = getMinAuctionTime(isTbcOrNewer);
    const uint32_t auctionDuration = srlPacket.expireTime * MINUTE;

    if (auctionDuration != minAuctionTime &&
        auctionDuration != 2 * minAuctionTime &&
        auctionDuration != 4 * minAuctionTime)
    {
        return;
    }

    Item* items[MAX_AUCTION_ITEMS];

    uint32_t finalCount = 0;

    for (uint32_t i = 0; i < srlPacket.itemsCount; ++i)
    {
        const auto item = _player->getItemInterface()->GetItemByGUID(srlPacket.itemGuids[i]);
        if (!item)
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_ACTION_CREATE, AUCTION_ERROR_ITEM);
            return;
        }

        if (item->getStackCount() < srlPacket.count[i])
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_ACTION_CREATE, AUCTION_ERROR_INTERNAL);
            return;
        }

        items[i] = item;
        finalCount += srlPacket.count[i];
    }

    if (!finalCount)
    {
        _player->sendAuctionCommandResult(nullptr, AUCTION_ACTION_CREATE, AUCTION_ERROR_INTERNAL);
        return;
    }

    for (uint32_t i = 0; i < srlPacket.itemsCount; ++i)
    {
        if (items[i] == nullptr)
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_ACTION_CREATE, AUCTION_ERROR_INTERNAL);
            return;
        }

        const uint64_t itemWorth = static_cast<uint64_t>(items[i]->getItemProperties()->SellPrice) * items[i]->getStackCount();
        const uint32_t durationMultiplier = srlPacket.expireTime / (isTbcOrNewer ? 240u : 120u);
        const uint32_t itemDeposit = static_cast<uint32_t>(static_cast<double>(itemWorth) * auctionHouse->depositPercent * durationMultiplier);

        if (!_player->hasEnoughCoinage(itemDeposit))
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_ACTION_CREATE, AUCTION_ERROR_MONEY);
            return;
        }

        _player->modCoinage(-static_cast<int64_t>(itemDeposit));

        auto item = _player->getItemInterface()->SafeRemoveAndRetreiveItemByGuid(srlPacket.itemGuids[i], false);
        if (!item)
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_ACTION_CREATE, AUCTION_ERROR_ITEM);
            return;
        }

        if (item->IsInWorld())
            item->removeFromWorld();

        item->setOwner(nullptr);
        item->m_isDirty = true;
        item->saveToDB(INVENTORY_SLOT_NOT_SET, 0, true, nullptr);

        auto auction = std::make_unique<Auction>();
        auction->buyoutPrice = srlPacket.buyoutPrice;
        auction->expireTime = static_cast<uint32_t>(UNIXTIME) + srlPacket.expireTime * MINUTE;
        auction->startPrice = srlPacket.bidMoney;
        auction->highestBid = 0;
        auction->highestBidderGuid = 0;
        auction->Id = sAuctionMgr.generateAuctionId();
        auction->ownerGuid = _player->getGuidLow();
        auction->auctionItem = std::move(item);
        auction->isRemoved = false;
        auction->removedType = AUCTION_REMOVE_EXPIRED;
        auction->depositAmount = itemDeposit;

        auction->saveToDB(auctionHouse->getId());
        auctionHouse->addAuction(std::move(auction));

        _player->sendAuctionCommandResult(nullptr, AUCTION_ACTION_CREATE, AUCTION_ERROR_NONE);
    }

    auctionHouse->sendOwnerListPacket(_player);
}

void WorldSession::handleAuctionPlaceBid(WorldPacket& recvPacket)
{
    CmsgAuctionPlaceBid srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_AUCTION_PLACE_BID: {} (auctionId), {} (price).", srlPacket.auctionId, srlPacket.price);

    if (!srlPacket.price)
        return;

    if (!WoW::isServerExpansionAtLeast(WoW::Expansion::_Cata) && srlPacket.price > std::numeric_limits<int32_t>::max())
        return;

    const auto creature = _player->getWorldMapCreature(srlPacket.guid.getRawGuid());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    const auto auctionHouse = creature->auctionHouse;
    const auto auction = auctionHouse->getAuction(srlPacket.auctionId);
    if (auction == nullptr || !auction->ownerGuid)
    {
        _player->sendAuctionCommandResult(nullptr, AUCTION_ACTION_BID, AUCTION_ERROR_INTERNAL, 0);
        return;
    }

    if (auction->ownerGuid == _player->getGuid())
    {
        _player->sendAuctionCommandResult(nullptr, AUCTION_ACTION_BID, AUCTION_ERROR_BID_OWN_AUCTION, 0);
        return;
    }

    const uint64_t minRequiredBid = auction->highestBid ? (auction->highestBid + auction->getAuctionOutBid()) : auction->startPrice;
    if (srlPacket.price < minRequiredBid && srlPacket.price != auction->buyoutPrice)
    {
        _player->sendAuctionCommandResult(auction, AUCTION_ACTION_BID, AUCTION_ERROR_INTERNAL, 0);
        return;
    }

    if (auction->buyoutPrice > 0 && srlPacket.price > auction->buyoutPrice)
        srlPacket.price = auction->buyoutPrice;

    if (!_player->hasEnoughCoinage(srlPacket.price))
    {
        _player->sendAuctionCommandResult(nullptr, AUCTION_ACTION_BID, AUCTION_ERROR_MONEY, 0);
        return;
    }

    _player->modCoinage(-static_cast<int64_t>(srlPacket.price));


    if (auction->highestBidderGuid.getLowGuid() != 0)
    {
        char subject[100];
        snprintf(subject, 100, "%u:0:0", static_cast<int>(auction->auctionItem->getEntry()));
        sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, auctionHouse->getId(), auction->highestBidderGuid, subject, "", static_cast<uint32_t>(auction->highestBid), 0, 0, MAIL_STATIONERY_AUCTION);

        if (auction->highestBidderGuid != _player->getGuid())
            auctionHouse->sendAuctionOutBidNotificationPacket(auction, _player->getGuid(), static_cast<uint32_t>(srlPacket.price));
    }

    if (auction->buyoutPrice == srlPacket.price)
    {
        auction->highestBidderGuid = _player->getGuid();
        auction->highestBid = srlPacket.price;

        auctionHouse->queueDeletion(auction, AUCTION_REMOVE_WON);

        _player->sendAuctionCommandResult(auction, AUCTION_ACTION_BID, AUCTION_ERROR_NONE, 0);
        auctionHouse->sendAuctionBuyOutNotificationPacket(auction);
    }
    else
    {
        auction->highestBidderGuid = _player->getGuidLow();
        auction->highestBid = srlPacket.price;
        auction->updateInDB();

        _player->sendAuctionCommandResult(auction, AUCTION_ACTION_BID, AUCTION_ERROR_NONE, 0);
    }
}
