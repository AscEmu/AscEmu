/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

void WorldSession::sendVoidStorageTransferResult(uint8_t result)
{
#if VERSION_STRING >= Cata
    WorldPacket data(SMSG_VOID_TRANSFER_RESULT, 4);
    data << uint32_t(result);
    SendPacket(&data);
#endif
}

void WorldSession::handleVoidStorageUnlock(WorldPacket& recvData)
{
#if VERSION_STRING >= Cata
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_VOID_STORAGE_UNLOCK");

    Player* player = GetPlayer();

    ObjectGuid npcGuid;
    npcGuid[4] = recvData.readBit();
    npcGuid[5] = recvData.readBit();
    npcGuid[3] = recvData.readBit();
    npcGuid[0] = recvData.readBit();
    npcGuid[2] = recvData.readBit();
    npcGuid[1] = recvData.readBit();
    npcGuid[7] = recvData.readBit();
    npcGuid[6] = recvData.readBit();

    recvData.ReadByteSeq(npcGuid[7]);
    recvData.ReadByteSeq(npcGuid[1]);
    recvData.ReadByteSeq(npcGuid[2]);
    recvData.ReadByteSeq(npcGuid[3]);
    recvData.ReadByteSeq(npcGuid[5]);
    recvData.ReadByteSeq(npcGuid[0]);
    recvData.ReadByteSeq(npcGuid[6]);
    recvData.ReadByteSeq(npcGuid[4]);

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

void WorldSession::handleVoidStorageQuery(WorldPacket& recvData)
{
#if VERSION_STRING >= Cata
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_VOID_STORAGE_QUERY");
    Player* player = GetPlayer();

    ObjectGuid npcGuid;
    npcGuid[4] = recvData.readBit();
    npcGuid[0] = recvData.readBit();
    npcGuid[5] = recvData.readBit();
    npcGuid[7] = recvData.readBit();
    npcGuid[6] = recvData.readBit();
    npcGuid[3] = recvData.readBit();
    npcGuid[1] = recvData.readBit();
    npcGuid[2] = recvData.readBit();

    recvData.ReadByteSeq(npcGuid[5]);
    recvData.ReadByteSeq(npcGuid[6]);
    recvData.ReadByteSeq(npcGuid[3]);
    recvData.ReadByteSeq(npcGuid[7]);
    recvData.ReadByteSeq(npcGuid[1]);
    recvData.ReadByteSeq(npcGuid[0]);
    recvData.ReadByteSeq(npcGuid[4]);
    recvData.ReadByteSeq(npcGuid[2]);

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

    uint8_t count = 0;
    for (uint8_t i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
        if (player->getVoidStorageItem(i))
            ++count;

    WorldPacket data(SMSG_VOID_STORAGE_CONTENTS, 2 * count + (14 + 4 + 4 + 4 + 4) * count);

    data.writeBits(count, 8);

    ByteBuffer itemData((14 + 4 + 4 + 4 + 4) * count);

    for (uint8_t i = 0; i < VOID_STORAGE_MAX_SLOT; ++i)
    {
        VoidStorageItem* item = player->getVoidStorageItem(i);
        if (!item)
            continue;

        ObjectGuid itemId = item->itemId;
        ObjectGuid creatorGuid = item->creatorGuid;

        data.writeBit(creatorGuid[3]);
        data.writeBit(itemId[5]);
        data.writeBit(creatorGuid[6]);
        data.writeBit(creatorGuid[1]);
        data.writeBit(itemId[1]);
        data.writeBit(itemId[3]);
        data.writeBit(itemId[6]);
        data.writeBit(creatorGuid[5]);
        data.writeBit(creatorGuid[2]);
        data.writeBit(itemId[2]);
        data.writeBit(creatorGuid[4]);
        data.writeBit(itemId[0]);
        data.writeBit(itemId[4]);
        data.writeBit(itemId[7]);
        data.writeBit(creatorGuid[0]);
        data.writeBit(creatorGuid[7]);

        itemData.WriteByteSeq(creatorGuid[3]);

        itemData << uint32_t(item->itemSuffixFactor);

        itemData.WriteByteSeq(creatorGuid[4]);

        itemData << uint32_t(i);

        itemData.WriteByteSeq(itemId[0]);
        itemData.WriteByteSeq(itemId[6]);
        itemData.WriteByteSeq(creatorGuid[0]);

        itemData << uint32_t(item->itemRandomPropertyId);

        itemData.WriteByteSeq(itemId[4]);
        itemData.WriteByteSeq(itemId[5]);
        itemData.WriteByteSeq(itemId[2]);
        itemData.WriteByteSeq(creatorGuid[2]);
        itemData.WriteByteSeq(creatorGuid[6]);
        itemData.WriteByteSeq(itemId[1]);
        itemData.WriteByteSeq(itemId[3]);
        itemData.WriteByteSeq(creatorGuid[5]);
        itemData.WriteByteSeq(creatorGuid[7]);

        itemData << uint32_t(item->itemEntry);

        itemData.WriteByteSeq(itemId[7]);
    }

    data.flushBits();
    data.append(itemData);

    SendPacket(&data);
#endif
}

void WorldSession::handleVoidStorageTransfer(WorldPacket& recvData)
{
#if VERSION_STRING >= Cata
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_VOID_STORAGE_TRANSFER");
    Player* player = GetPlayer();

    // Read everything

    ObjectGuid npcGuid;
    npcGuid[1] = recvData.readBit();

    uint32_t countDeposit = recvData.readBits(26);

    if (countDeposit > 9)
    {
        sLogger.debug("handleVoidStorageTransfer - Player (GUID: {}, name: {}) wants to deposit more than 9 items ({}).", player->getGuidLow(), player->getName(), countDeposit);
        return;
    }

    std::vector<ObjectGuid> itemGuids(countDeposit);
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

    std::vector<ObjectGuid> itemIds(countWithdraw);
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
        recvData.ReadByteSeq(itemGuids[i][6]);
        recvData.ReadByteSeq(itemGuids[i][1]);
        recvData.ReadByteSeq(itemGuids[i][0]);
        recvData.ReadByteSeq(itemGuids[i][2]);
        recvData.ReadByteSeq(itemGuids[i][4]);
        recvData.ReadByteSeq(itemGuids[i][5]);
        recvData.ReadByteSeq(itemGuids[i][3]);
        recvData.ReadByteSeq(itemGuids[i][7]);
    }

    recvData.ReadByteSeq(npcGuid[5]);
    recvData.ReadByteSeq(npcGuid[6]);

    for (uint32_t i = 0; i < countWithdraw; ++i)
    {
        recvData.ReadByteSeq(itemIds[i][3]);
        recvData.ReadByteSeq(itemIds[i][1]);
        recvData.ReadByteSeq(itemIds[i][0]);
        recvData.ReadByteSeq(itemIds[i][6]);
        recvData.ReadByteSeq(itemIds[i][2]);
        recvData.ReadByteSeq(itemIds[i][7]);
        recvData.ReadByteSeq(itemIds[i][5]);
        recvData.ReadByteSeq(itemIds[i][4]);
    }

    recvData.ReadByteSeq(npcGuid[1]);
    recvData.ReadByteSeq(npcGuid[4]);
    recvData.ReadByteSeq(npcGuid[7]);
    recvData.ReadByteSeq(npcGuid[3]);
    recvData.ReadByteSeq(npcGuid[2]);
    recvData.ReadByteSeq(npcGuid[0]);

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
    for (std::vector<ObjectGuid>::iterator itr = itemGuids.begin(); itr != itemGuids.end(); ++itr)
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
    for (std::vector<ObjectGuid>::iterator itr = itemIds.begin(); itr != itemIds.end(); ++itr)
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

    WorldPacket data(SMSG_VOID_STORAGE_TRANSFER_CHANGES, ((5 + 5 + (7 + 7) * depositCount +
        7 * withdrawCount) / 8) + 7 * withdrawCount + (7 + 7 + 4 * 4) * depositCount);

    data.writeBits(depositCount, 5);
    data.writeBits(withdrawCount, 5);

    for (uint8_t i = 0; i < depositCount; ++i)
    {
        ObjectGuid itemId = depositItems[i].first.itemId;
        ObjectGuid creatorGuid = depositItems[i].first.creatorGuid;
        data.writeBit(creatorGuid[7]);
        data.writeBit(itemId[7]);
        data.writeBit(itemId[4]);
        data.writeBit(creatorGuid[6]);
        data.writeBit(creatorGuid[5]);
        data.writeBit(itemId[3]);
        data.writeBit(itemId[5]);
        data.writeBit(creatorGuid[4]);
        data.writeBit(creatorGuid[2]);
        data.writeBit(creatorGuid[0]);
        data.writeBit(creatorGuid[3]);
        data.writeBit(creatorGuid[1]);
        data.writeBit(itemId[2]);
        data.writeBit(itemId[0]);
        data.writeBit(itemId[1]);
        data.writeBit(itemId[6]);
    }

    for (uint8_t i = 0; i < withdrawCount; ++i)
    {
        ObjectGuid itemId = withdrawItems[i].itemId;
        data.writeBit(itemId[1]);
        data.writeBit(itemId[7]);
        data.writeBit(itemId[3]);
        data.writeBit(itemId[5]);
        data.writeBit(itemId[6]);
        data.writeBit(itemId[2]);
        data.writeBit(itemId[4]);
        data.writeBit(itemId[0]);
    }

    data.flushBits();

    for (uint8_t i = 0; i < withdrawCount; ++i)
    {
        ObjectGuid itemId = withdrawItems[i].itemId;
        data.WriteByteSeq(itemId[3]);
        data.WriteByteSeq(itemId[1]);
        data.WriteByteSeq(itemId[0]);
        data.WriteByteSeq(itemId[2]);
        data.WriteByteSeq(itemId[7]);
        data.WriteByteSeq(itemId[5]);
        data.WriteByteSeq(itemId[6]);
        data.WriteByteSeq(itemId[4]);
    }

    for (uint8_t i = 0; i < depositCount; ++i)
    {
        ObjectGuid itemId = depositItems[i].first.itemId;
        ObjectGuid creatorGuid = depositItems[i].first.creatorGuid;

        data << uint32_t(depositItems[i].first.itemSuffixFactor);

        data.WriteByteSeq(itemId[6]);
        data.WriteByteSeq(itemId[4]);
        data.WriteByteSeq(creatorGuid[4]);
        data.WriteByteSeq(itemId[2]);
        data.WriteByteSeq(creatorGuid[1]);
        data.WriteByteSeq(creatorGuid[3]);
        data.WriteByteSeq(itemId[3]);
        data.WriteByteSeq(creatorGuid[0]);
        data.WriteByteSeq(itemId[0]);
        data.WriteByteSeq(creatorGuid[6]);
        data.WriteByteSeq(itemId[5]);
        data.WriteByteSeq(creatorGuid[5]);
        data.WriteByteSeq(creatorGuid[7]);

        data << uint32_t(depositItems[i].first.itemEntry);

        data.WriteByteSeq(itemId[1]);

        data << uint32_t(depositItems[i].second); // slot

        data.WriteByteSeq(creatorGuid[2]);
        data.WriteByteSeq(itemId[7]);

        data << uint32_t(depositItems[i].first.itemRandomPropertyId);
    }

    SendPacket(&data);

    sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_NO_ERROR);

    player->saveVoidStorage();
#endif
}

void WorldSession::handleVoidSwapItem(WorldPacket& recvData)
{
#if VERSION_STRING >= Cata
    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_VOID_SWAP_ITEM");

    Player* player = GetPlayer();
    uint32_t newSlot;
    ObjectGuid npcGuid;
    ObjectGuid itemId;

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

    recvData.ReadByteSeq(npcGuid[1]);
    recvData.ReadByteSeq(itemId[3]);
    recvData.ReadByteSeq(itemId[2]);
    recvData.ReadByteSeq(itemId[4]);
    recvData.ReadByteSeq(npcGuid[3]);
    recvData.ReadByteSeq(npcGuid[0]);
    recvData.ReadByteSeq(itemId[6]);
    recvData.ReadByteSeq(itemId[1]);
    recvData.ReadByteSeq(npcGuid[5]);
    recvData.ReadByteSeq(itemId[5]);
    recvData.ReadByteSeq(npcGuid[6]);
    recvData.ReadByteSeq(itemId[0]);
    recvData.ReadByteSeq(npcGuid[2]);
    recvData.ReadByteSeq(npcGuid[7]);
    recvData.ReadByteSeq(npcGuid[4]);
    recvData.ReadByteSeq(itemId[7]);

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
    bool usedDestSlot = player->getVoidStorageItem(newSlot) != nullptr;
    ObjectGuid itemIdDest;
    if (usedDestSlot)
        itemIdDest = player->getVoidStorageItem(newSlot)->itemId;

    if (!player->swapVoidStorageItem(oldSlot, newSlot))
    {
        sendVoidStorageTransferResult(VOID_TRANSFER_ERROR_INTERNAL_ERROR_1);
        return;
    }

    WorldPacket data(SMSG_VOID_ITEM_SWAP_RESPONSE, 1 + (usedSrcSlot + usedDestSlot) * (1 + 7 + 4));

    data.writeBit(!usedDestSlot);
    data.writeBit(!usedSrcSlot);

    if (usedSrcSlot)
    {
        data.writeBit(itemId[5]);
        data.writeBit(itemId[2]);
        data.writeBit(itemId[1]);
        data.writeBit(itemId[4]);
        data.writeBit(itemId[0]);
        data.writeBit(itemId[6]);
        data.writeBit(itemId[7]);
        data.writeBit(itemId[3]);
    }

    data.writeBit(!usedDestSlot); // unk

    if (usedDestSlot)
    {
        data.writeBit(itemIdDest[7]);
        data.writeBit(itemIdDest[3]);
        data.writeBit(itemIdDest[4]);
        data.writeBit(itemIdDest[0]);
        data.writeBit(itemIdDest[5]);
        data.writeBit(itemIdDest[1]);
        data.writeBit(itemIdDest[2]);
        data.writeBit(itemIdDest[6]);
    }

    data.writeBit(!usedSrcSlot); // unk

    data.flushBits();

    if (usedDestSlot)
    {
        data.WriteByteSeq(itemIdDest[4]);
        data.WriteByteSeq(itemIdDest[6]);
        data.WriteByteSeq(itemIdDest[5]);
        data.WriteByteSeq(itemIdDest[2]);
        data.WriteByteSeq(itemIdDest[3]);
        data.WriteByteSeq(itemIdDest[1]);
        data.WriteByteSeq(itemIdDest[7]);
        data.WriteByteSeq(itemIdDest[0]);
    }

    if (usedSrcSlot)
    {
        data.WriteByteSeq(itemId[6]);
        data.WriteByteSeq(itemId[3]);
        data.WriteByteSeq(itemId[5]);
        data.WriteByteSeq(itemId[0]);
        data.WriteByteSeq(itemId[1]);
        data.WriteByteSeq(itemId[2]);
        data.WriteByteSeq(itemId[4]);
        data.WriteByteSeq(itemId[7]);
    }

    if (usedDestSlot)
        data << uint32_t(oldSlot);

    if (usedSrcSlot)
        data << uint32_t(newSlot);

    SendPacket(&data);

    player->saveVoidStorage();
#endif
}

