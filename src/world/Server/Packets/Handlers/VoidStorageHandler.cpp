/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/WorldSession.h"
#include "Objects/Units/Players/Player.hpp"

// Cata
#include "Logging/Logger.hpp"
#include "Macros/GuildMacros.hpp"
#include "Management/ItemInterface.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Management/ObjectMgr.hpp"
#include "Objects/Item.hpp"
#include "Server/Packets/SmsgVoidTransferResult.h"
#include "Server/Packets/SmsgVoidStorageContents.h"
#include "Server/Packets/SmsgVoidStorageTransferChanges.h"
#include "Server/Packets/SmsgVoidItemSwapResponse.h"

using namespace AscEmu::Packets;

void WorldSession::sendVoidStorageTransferResult([[maybe_unused]] uint8_t result)
{
#if VERSION_STRING >= Cata
    SmsgVoidTransferResult managedPacket{ uint32_t(result) };
    sendManagedPacket(managedPacket);
#endif
}

void WorldSession::handleVoidStorageUnlock([[maybe_unused]] WorldPacket& recvData)
{
#if VERSION_STRING >= Cata
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_VOID_STORAGE_UNLOCK");

    Player* player = GetPlayer();

    WoWGuid npcGuid;
    npcGuid[4] = recvData.readBit();
    npcGuid[5] = recvData.readBit();
    npcGuid[3] = recvData.readBit();
    npcGuid[0] = recvData.readBit();
    npcGuid[2] = recvData.readBit();
    npcGuid[1] = recvData.readBit();
    npcGuid[7] = recvData.readBit();
    npcGuid[6] = recvData.readBit();

    recvData.readByteSeq(npcGuid[7]);
    recvData.readByteSeq(npcGuid[1]);
    recvData.readByteSeq(npcGuid[2]);
    recvData.readByteSeq(npcGuid[3]);
    recvData.readByteSeq(npcGuid[5]);
    recvData.readByteSeq(npcGuid[0]);
    recvData.readByteSeq(npcGuid[6]);
    recvData.readByteSeq(npcGuid[4]);

    Creature* creature = player->getWorldMapCreature(npcGuid);
    if (!creature)
    {
        sLogger.debug("handleVoidStorageUnlock - Unit (GUID: {}) not found.", uint64_t(npcGuid));
        return;
    }

    // Validate
    if (!creature->isVoidStorage() && creature->getDistance(player) > 5.0f)
    {
        sLogger.debug("handleVoidStorageUnlock - Unit (GUID: {}) can't interact with it or is no Void Storage.", uint64_t(npcGuid));
        return;
    }

    if (player->isVoidStorageUnlocked())
    {
        sLogger.debug("handleVoidStorageUnlock - Player (GUID: {}, name: {}) tried to unlock void storage a 2nd time.", player->getGuidLow(), player->getName());
        return;
    }

    player->modCoinage(-int64_t(VOID_STORAGE_UNLOCK));
    player->unlockVoidStorage();
#endif
}

void WorldSession::handleVoidStorageQuery([[maybe_unused]] WorldPacket& recvData)
{
#if VERSION_STRING >= Cata
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_VOID_STORAGE_QUERY");
    Player* player = GetPlayer();

    WoWGuid npcGuid;
    npcGuid[4] = recvData.readBit();
    npcGuid[0] = recvData.readBit();
    npcGuid[5] = recvData.readBit();
    npcGuid[7] = recvData.readBit();
    npcGuid[6] = recvData.readBit();
    npcGuid[3] = recvData.readBit();
    npcGuid[1] = recvData.readBit();
    npcGuid[2] = recvData.readBit();

    recvData.readByteSeq(npcGuid[5]);
    recvData.readByteSeq(npcGuid[6]);
    recvData.readByteSeq(npcGuid[3]);
    recvData.readByteSeq(npcGuid[7]);
    recvData.readByteSeq(npcGuid[1]);
    recvData.readByteSeq(npcGuid[0]);
    recvData.readByteSeq(npcGuid[4]);
    recvData.readByteSeq(npcGuid[2]);

    Creature* creature = player->getWorldMapCreature(npcGuid);
    if (!creature)
    {
        sLogger.debug("handleVoidStorageQuery - Unit (GUID: {}) not found.", uint64_t(npcGuid));
        return;
    }

    // Validate
    if (!creature->isVoidStorage() && creature->getDistance(player) > 5.0f)
    {
        sLogger.debug("handleVoidStorageQuery - Unit (GUID: {}) can't interact with it or is no Void Storage.", uint64_t(npcGuid));
        return;
    }

    if (!player->isVoidStorageUnlocked())
    {
        sLogger.debug("handleVoidStorageQuery - Player (GUID: {}, name: {}) queried void storage without unlocking it.", player->getGuidLow(), player->getName());
        return;
    }

    SmsgVoidStorageContents managedPacket(player);
    sendManagedPacket(managedPacket);
#endif
}

void WorldSession::handleVoidStorageTransfer([[maybe_unused]] WorldPacket& recvData)
{
#if VERSION_STRING >= Cata
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_VOID_STORAGE_TRANSFER");
    Player* player = GetPlayer();

    // Read everything

    WoWGuid npcGuid;
    npcGuid[1] = recvData.readBit();

    uint32_t countDeposit = recvData.readBits(26);

    if (countDeposit > 9)
    {
        sLogger.debug("handleVoidStorageTransfer - Player (GUID: {}, name: {}) wants to deposit more than 9 items ({}).", player->getGuidLow(), player->getName(), countDeposit);
        return;
    }

    std::vector<WoWGuid> itemGuids(countDeposit);
    for (uint32_t i = 0; i < countDeposit; ++i)
    {
        itemGuids[i][4] = recvData.readBit();
        itemGuids[i][6] = recvData.readBit();
        itemGuids[i][7] = recvData.readBit();
        itemGuids[i][0] = recvData.readBit();
        itemGuids[i][1] = recvData.readBit();
        itemGuids[i][5] = recvData.readBit();
        itemGuids[i][3] = recvData.readBit();
        itemGuids[i][2] = recvData.readBit();
    }

    npcGuid[2] = recvData.readBit();
    npcGuid[0] = recvData.readBit();
    npcGuid[3] = recvData.readBit();
    npcGuid[5] = recvData.readBit();
    npcGuid[6] = recvData.readBit();
    npcGuid[4] = recvData.readBit();

    uint32_t countWithdraw = recvData.readBits(26);

    if (countWithdraw > 9)
    {
        sLogger.debug("handleVoidStorageTransfer - Player (GUID: {}, name: {}) wants to withdraw more than 9 items ({}).", player->getGuidLow(), player->getName(), countWithdraw);
        return;
    }

    std::vector<WoWGuid> itemIds(countWithdraw);
    for (uint32_t i = 0; i < countWithdraw; ++i)
    {
        itemIds[i][4] = recvData.readBit();
        itemIds[i][7] = recvData.readBit();
        itemIds[i][1] = recvData.readBit();
        itemIds[i][0] = recvData.readBit();
        itemIds[i][2] = recvData.readBit();
        itemIds[i][3] = recvData.readBit();
        itemIds[i][5] = recvData.readBit();
        itemIds[i][6] = recvData.readBit();
    }

    npcGuid[7] = recvData.readBit();

    recvData.flushBits();

    for (uint32_t i = 0; i < countDeposit; ++i)
    {
        recvData.readByteSeq(itemGuids[i][6]);
        recvData.readByteSeq(itemGuids[i][1]);
        recvData.readByteSeq(itemGuids[i][0]);
        recvData.readByteSeq(itemGuids[i][2]);
        recvData.readByteSeq(itemGuids[i][4]);
        recvData.readByteSeq(itemGuids[i][5]);
        recvData.readByteSeq(itemGuids[i][3]);
        recvData.readByteSeq(itemGuids[i][7]);
    }

    recvData.readByteSeq(npcGuid[5]);
    recvData.readByteSeq(npcGuid[6]);

    for (uint32_t i = 0; i < countWithdraw; ++i)
    {
        recvData.readByteSeq(itemIds[i][3]);
        recvData.readByteSeq(itemIds[i][1]);
        recvData.readByteSeq(itemIds[i][0]);
        recvData.readByteSeq(itemIds[i][6]);
        recvData.readByteSeq(itemIds[i][2]);
        recvData.readByteSeq(itemIds[i][7]);
        recvData.readByteSeq(itemIds[i][5]);
        recvData.readByteSeq(itemIds[i][4]);
    }

    recvData.readByteSeq(npcGuid[1]);
    recvData.readByteSeq(npcGuid[4]);
    recvData.readByteSeq(npcGuid[7]);
    recvData.readByteSeq(npcGuid[3]);
    recvData.readByteSeq(npcGuid[2]);
    recvData.readByteSeq(npcGuid[0]);

    Creature* creature = player->getWorldMapCreature(npcGuid);
    if (!creature)
    {
        sLogger.debug("handleVoidStorageTransfer - Unit (GUID: {}) not found.", uint64_t(npcGuid));
        return;
    }

    // Validate
    if (!creature->isVoidStorage() && creature->getDistance(player) > 5.0f)
    {
        sLogger.debug("handleVoidStorageTransfer - Unit (GUID: {}) can't interact with it or is no Void Storage.", uint64_t(npcGuid));
        return;
    }

    if (!player->isVoidStorageUnlocked())
    {
        sLogger.debug("handleVoidStorageTransfer - Player (GUID: {}, name: {}) queried void storage without unlocking it.", player->getGuidLow(), player->getName());
        return;
    }

    if (itemGuids.size() > player->getNumOfVoidStorageFreeSlots())
    {
        sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_FULL);
        return;
    }

    uint32_t freeBagSlots = 0;
    if (itemIds.size() != 0)
    {
        freeBagSlots = player->getItemInterface()->CalculateFreeSlots(nullptr);
    }

    if (itemIds.size() > freeBagSlots)
    {
        sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INVENTORY_FULL);
        return;
    }

    if (player->getCoinage() < uint64_t(itemGuids.size() * VOID_STORAGE_STORE_ITEM))
    {
        sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_NOT_ENOUGH_MONEY);
        return;
    }

    std::pair<VoidStorageItem, uint8_t> depositItems[VOID_STORAGE_MAX_DEPOSIT];
    uint8_t depositCount = 0;
    for (std::vector<WoWGuid>::iterator itr = itemGuids.begin(); itr != itemGuids.end(); ++itr)
    {
        Item* item = player->getItemInterface()->GetItemByGUID(*itr);
        if (!item)
        {
            sLogger.debug("handleVoidStorageTransfer - Player (GUID: {}, name: {}) wants to deposit an invalid item (item guid: %I64u).", player->getGuidLow(), player->getName(), uint64_t(*itr));
            continue;
        }

        VoidStorageItem itemVS(sObjectMgr.generateVoidStorageItemId(), item->getEntry(), static_cast<uint32_t>(item->getCreatorGuid()), item->getRandomPropertiesId(), item->getPropertySeed());

        uint8_t slot = player->addVoidStorageItem(itemVS);

        depositItems[depositCount++] = std::make_pair(itemVS, slot);

        player->getItemInterface()->SafeFullRemoveItemByGuid(*itr);
        item->deleteFromDB();
    }

    int64_t cost = depositCount * VOID_STORAGE_STORE_ITEM;

    player->modCoinage(-cost);

    VoidStorageItem withdrawItems[VOID_STORAGE_MAX_WITHDRAW];
    uint8_t withdrawCount = 0;
    for (std::vector<WoWGuid>::iterator itr = itemIds.begin(); itr != itemIds.end(); ++itr)
    {
        uint8_t slot;
        VoidStorageItem* itemVS = player->getVoidStorageItem(*itr, slot);
        if (!itemVS)
        {
            sLogger.debug("handleVoidStorageTransfer - Player (GUID: {}, name: {}) tried to withdraw an invalid item (id: %I64u)", player->getGuidLow(), player->getName(), uint64_t(*itr));
            continue;
        }

        auto itemHolder = sObjectMgr.createItem(itemVS->itemEntry, player);

        auto* item = itemHolder.get();
        const auto [msg, _] = player->getItemInterface()->AddItemToFreeSlot(std::move(itemHolder));
        if (msg != ADD_ITEM_RESULT_OK)
        {
            sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INVENTORY_FULL);
            sLogger.debug("handleVoidStorageTransfer - Player (GUID: {}, name: {}) couldn't withdraw item id %I64u because inventory was full.", player->getGuidLow(), player->getName(), uint64_t(*itr));
            return;
        }

        item->setCreatorGuid(uint64_t(itemVS->creatorGuid));
        item->addFlags(ITEM_FLAG_SOULBOUND);

        withdrawItems[withdrawCount++] = *itemVS;

        player->deleteVoidStorageItem(slot);
    }

    SmsgVoidStorageTransferChanges managedPacket(depositItems, depositCount, withdrawItems, withdrawCount);
    sendManagedPacket(managedPacket);

    sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_NO_ERROR);

    player->saveVoidStorage();
#endif
}

void WorldSession::handleVoidSwapItem([[maybe_unused]] WorldPacket& recvData)
{
#if VERSION_STRING >= Cata
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_VOID_SWAP_ITEM");

    Player* player = GetPlayer();
    uint32_t newSlot;
    WoWGuid npcGuid;
    WoWGuid itemId;

    recvData >> newSlot;

    npcGuid[2] = recvData.readBit();
    npcGuid[4] = recvData.readBit();
    npcGuid[0] = recvData.readBit();
    itemId[2] = recvData.readBit();
    itemId[6] = recvData.readBit();
    itemId[5] = recvData.readBit();
    npcGuid[1] = recvData.readBit();
    npcGuid[7] = recvData.readBit();
    itemId[3] = recvData.readBit();
    itemId[7] = recvData.readBit();
    itemId[0] = recvData.readBit();
    npcGuid[6] = recvData.readBit();
    npcGuid[5] = recvData.readBit();
    npcGuid[3] = recvData.readBit();
    itemId[1] = recvData.readBit();
    itemId[4] = recvData.readBit();

    recvData.readByteSeq(npcGuid[1]);
    recvData.readByteSeq(itemId[3]);
    recvData.readByteSeq(itemId[2]);
    recvData.readByteSeq(itemId[4]);
    recvData.readByteSeq(npcGuid[3]);
    recvData.readByteSeq(npcGuid[0]);
    recvData.readByteSeq(itemId[6]);
    recvData.readByteSeq(itemId[1]);
    recvData.readByteSeq(npcGuid[5]);
    recvData.readByteSeq(itemId[5]);
    recvData.readByteSeq(npcGuid[6]);
    recvData.readByteSeq(itemId[0]);
    recvData.readByteSeq(npcGuid[2]);
    recvData.readByteSeq(npcGuid[7]);
    recvData.readByteSeq(npcGuid[4]);
    recvData.readByteSeq(itemId[7]);

    Creature* creature = player->getWorldMapCreature(npcGuid);
    if (!creature)
    {
        sLogger.debug("handleVoidSwapItem - Unit (GUID: {}) not found.", uint64_t(npcGuid));
        return;
    }

    // Validate
    if (!creature->isVoidStorage() && creature->getDistance(player) > 5.0f)
    {
        sLogger.debug("handleVoidSwapItem - Unit (GUID: {}) can't interact with it or is no Void Storage.", uint64_t(npcGuid));
        return;
    }

    if (!player->isVoidStorageUnlocked())
    {
        sLogger.debug("handleVoidSwapItem - Player (GUID: {}, name: {}) queried void storage without unlocking it.", player->getGuidLow(), player->getName());
        return;
    }

    uint8_t oldSlot;
    if (!player->getVoidStorageItem(itemId, oldSlot))
    {
        sLogger.debug("handleVoidSwapItem - Player (GUID: {}, name: {}) requested swapping an invalid item (slot: {}, itemid: %I64u).", player->getGuidLow(), player->getName(), newSlot, uint64_t(itemId));
        return;
    }

    bool usedSrcSlot = player->getVoidStorageItem(oldSlot) != nullptr; // should be always true
    bool usedDestSlot = player->getVoidStorageItem(static_cast<uint8_t>(newSlot)) != nullptr;
    WoWGuid itemIdDest;
    if (usedDestSlot)
        itemIdDest = player->getVoidStorageItem(static_cast<uint8_t>(newSlot))->itemId;

    if (!player->swapVoidStorageItem(oldSlot, static_cast<uint8_t>(newSlot)))
    {
        sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INTERNAL_ERROR_1);
        return;
    }

    SmsgVoidItemSwapResponse managedPacket(usedSrcSlot, usedDestSlot, itemId, itemIdDest, oldSlot, newSlot);
    sendManagedPacket(managedPacket);

    player->saveVoidStorage();
#endif
}

