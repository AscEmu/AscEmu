/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/WorldSession.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WorldStrings.h"
#include "Macros/ItemMacros.hpp"
#include "Server/Packets/CmsgRequestHotfix.h"
#include "Server/Packets/SmsgDbReply.h"

using namespace AscEmu::Packets;

#define DB2_REPLY_SPARSE    2442913102
#define DB2_REPLY_ITEM      1344507586
#define DB2_REPLY_BROADCAST   35137211

void WorldSession::handleRequestHotfix(WorldPacket& recvPacket)
{
    CmsgRequestHotfix srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    auto const protocol = _socket->getClientProtocol();
    if (protocol.isLegion())
    {
        // 7.3.5 keeps the item data on the server: Item and ItemSparse records come from the item properties,
        // every other table is reported as not available
        for (const uint32_t recordId : srlPacket.entries)
        {
            ByteBuffer record;
            switch (srlPacket.type)
            {
                case DB2_REPLY_ITEM:
                    writeItemRecordLegion(recordId, record);
                    break;
                case DB2_REPLY_SPARSE:
                    writeItemSparseRecordLegion(recordId, record);
                    break;
                default:
                    sLogger.debug("Received hotfix request for table 0x{:08X} record {}, not served", srlPacket.type, recordId);
                    break;
            }

            SmsgDbReply managedPacket(recordId, srlPacket.type, record);
            sendManagedPacket(managedPacket);
        }
        return;
    }

    if (protocol.isWoD())
    {
        // the client data tables of this version are not loaded: every record is reported as not available
        for (const uint32_t recordId : srlPacket.entries)
        {
            SmsgDbReply managedPacket(recordId, srlPacket.type, ByteBuffer());
            sendManagedPacket(managedPacket);
        }
        return;
    }

    if (protocol.isDragonflight())
    {
        // 10.x: the records of the client data tables are not served yet. The 5.x item record the older clients
        // get passes the size check of the 10.x client and leaves it with an unreadable record, so every record
        // is reported as not available; the client asks on the character screen as well and waits for the answers
        for (const uint32_t recordId : srlPacket.entries)
        {
            SmsgDbReply managedPacket(recordId, srlPacket.type, ByteBuffer());
            sendManagedPacket(managedPacket);
        }
        return;
    }

    // older clients only ask in the world
    if (_player == nullptr)
        return;

    switch (srlPacket.type)
    {
        case DB2_REPLY_ITEM:
        {
            sendItemDb2Reply(srlPacket.entry);
        } break;
        case DB2_REPLY_SPARSE:
        {
            sendItemSparseDb2Reply(srlPacket.entry);
        } break;
        case DB2_REPLY_BROADCAST:
        {
            if (protocol.isMop())
                sendBroadcastDb2Reply(srlPacket.entry);
        } break;
        default:
        {
            sLogger.debug("Received unknown hotfix type {} entry {}", srlPacket.type, srlPacket.entry);
            recvPacket.clear();
        } break;
    }
}

// Item record of 7.3.5 (layout 0x0DFCC83D) without its id: icon, class, sub class, sound override, material,
// inventory type, sheathe type, group sounds. Icons are not known, the client falls back to its own appearance data.
void WorldSession::writeItemRecordLegion(uint32_t entry, ByteBuffer& record)
{
    ItemProperties const* proto = sMySQLStore.getItemProperties(entry);
    if (proto == nullptr)
        return;

    record << uint32_t(0);                                          // icon file data id
    record << uint8_t(proto->Class);
    record << uint8_t(proto->SubClass);
    record << int8_t(-1);                                           // sound override sub class
    record << int8_t(proto->LockMaterial);                          // material
    record << uint8_t(proto->InventoryType);
    record << uint8_t(proto->SheathID);
    record << uint8_t(0);                                           // item group sounds
}

// ItemSparse record of 7.3.5 (layout 0x4007DE16) without its id, in the field order of the client data
void WorldSession::writeItemSparseRecordLegion(uint32_t entry, ByteBuffer& record)
{
    ItemProperties const* proto = sMySQLStore.getItemProperties(entry);
    if (proto == nullptr)
        return;

    record << int64_t(proto->AllowableRace);
    record << proto->Name;                                          // display name, 4 variants
    record << proto->Name;
    record << proto->Name;
    record << proto->Name;
    record << proto->Description;
    record << uint32_t(proto->Flags);
    record << uint32_t(proto->Flags2);
    record << uint32_t(0);                                          // flags 3
    record << uint32_t(0);                                          // flags 4
    record << float(1.0f);                                          // price random value
    record << float(1.0f);                                          // price variance
    record << int32_t(1);                                           // vendor stack count
    record << int32_t(proto->BuyPrice);
    record << int32_t(proto->SellPrice);
    record << int32_t(proto->RequiredSpell);                        // required ability
    record << int32_t(proto->Unique);                               // max count
    record << int32_t(proto->MaxCount);                             // stackable

    for (uint8_t i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
        record << int32_t(0);                                       // stat percent editor
    for (uint8_t i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
        record << float(0.0f);                                      // stat percentage of socket

    record << float(proto->Range);
    record << int32_t(proto->BagFamily);
    record << float(1.0f);                                          // quality modifier
    record << int32_t(proto->ExistingDuration);                     // duration in inventory
    record << float(0.0f);                                          // damage variance
    record << int16_t(proto->AllowableClass);
    record << uint16_t(proto->ItemLevel);
    record << uint16_t(proto->RequiredSkill);
    record << uint16_t(proto->RequiredSkillRank);
    record << uint16_t(proto->RequiredFaction);

    // stat values; the stat types follow further down
    uint8_t statCount = 0;
    for (const auto& [statType, statValue] : proto->generalStatsMap)
    {
        if (statCount == MAX_ITEM_PROTO_STATS)
            break;
        record << int16_t(statValue);
        ++statCount;
    }
    for (; statCount < MAX_ITEM_PROTO_STATS; ++statCount)
        record << int16_t(0);

    record << uint16_t(proto->ScalingStatsEntry);
    record << uint16_t(proto->Delay);
    record << uint16_t(proto->PageId);
    record << uint16_t(proto->QuestId);
    record << uint16_t(proto->LockId);
    record << uint16_t(proto->RandomPropId);                        // random select
    record << uint16_t(proto->RandomSuffixId);                      // random suffix group
    record << uint16_t(proto->ItemSet);
    record << uint16_t(proto->ZoneNameID);
    record << uint16_t(proto->MapID);
    record << uint16_t(proto->TotemCategory);
    record << uint16_t(proto->SocketBonus);
    record << uint16_t(proto->GemProperties);
    record << uint16_t(proto->ItemLimitCategory);
    record << uint16_t(proto->HolidayId);
    record << uint16_t(0);                                          // required transmog holiday
    record << uint16_t(0);                                          // item name description
    record << uint8_t(proto->Quality);
    record << uint8_t(proto->InventoryType);
    record << int8_t(proto->RequiredLevel);
    record << uint8_t(proto->RequiredPlayerRank1);
    record << uint8_t(proto->RequiredPlayerRank2);
    record << uint8_t(proto->RequiredFactionStanding);
    record << uint8_t(proto->ContainerSlots);

    statCount = 0;
    for (const auto& [statType, statValue] : proto->generalStatsMap)
    {
        if (statCount == MAX_ITEM_PROTO_STATS)
            break;
        record << int8_t(statType);
        ++statCount;
    }
    for (; statCount < MAX_ITEM_PROTO_STATS; ++statCount)
        record << int8_t(-1);

    record << uint8_t(proto->Damage[0].Type);
    record << uint8_t(proto->Bonding);
    record << uint8_t(proto->PageLanguage);
    record << uint8_t(proto->PageMaterial);
    record << int8_t(proto->LockMaterial);                          // material
    record << uint8_t(proto->SheathID);
    for (uint8_t i = 0; i < MAX_ITEM_PROTO_SOCKETS; ++i)
        record << uint8_t(proto->Sockets[i].SocketColor);
    record << uint8_t(0);                                           // spell weight category
    record << uint8_t(0);                                           // spell weight
    record << uint8_t(0);                                           // artifact
    record << uint8_t(0);                                           // expansion
}

void WorldSession::sendItemDb2Reply(uint32_t entry)
{
    ItemProperties const* proto = sMySQLStore.getItemProperties(entry);
    if (proto)
    {
        ByteBuffer buff;

        buff << uint32_t(entry);
        buff << uint32_t(proto->Class);
        buff << uint32_t(proto->SubClass);
        buff << int32_t(0);                                         // unk?
        buff << uint32_t(proto->LockMaterial);
        buff << uint32_t(proto->DisplayInfoID);
        buff << uint32_t(proto->InventoryType);
        buff << uint32_t(proto->SheathID);

        SmsgDbReply replyPacket(entry, DB2_REPLY_ITEM, buff);
        sendManagedPacket(replyPacket);
    }
}

void WorldSession::sendItemSparseDb2Reply(uint32_t entry)
{
    ItemProperties const* proto = sMySQLStore.getItemProperties(entry);
    if (proto)
    {
        ByteBuffer buff;

        buff << uint32_t(entry);
        buff << uint32_t(proto->Quality);
        buff << uint32_t(proto->Flags);
        buff << uint32_t(proto->Flags2);
        buff << float(1.0f);
        buff << float(1.0f);
        buff << uint32_t(proto->MaxCount);
        buff << int32_t(proto->BuyPrice);
        buff << uint32_t(proto->SellPrice);
        buff << uint32_t(proto->InventoryType);
        buff << int32_t(proto->AllowableClass);
        buff << int32_t(proto->AllowableRace);
        buff << uint32_t(proto->ItemLevel);
        buff << uint32_t(proto->RequiredLevel);
        buff << uint32_t(proto->RequiredSkill);
        buff << uint32_t(proto->RequiredSkillRank);
        buff << uint32_t(0);                                        // req spell
        buff << uint32_t(proto->RequiredPlayerRank1);
        buff << uint32_t(proto->RequiredPlayerRank2);
        buff << uint32_t(proto->RequiredFactionStanding);
        buff << uint32_t(proto->RequiredFaction);
        buff << int32_t(proto->MaxCount);
        buff << int32_t(0);                                         // stackable
        buff << uint32_t(proto->ContainerSlots);

        auto it = proto->generalStatsMap.begin();
        for (uint8_t i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
        {
            if (it != proto->generalStatsMap.end())
            {
                buff << it->first;
                ++it;
            }
            else
            {
                buff << uint32_t(0);
            }
        }

        auto it2 = proto->generalStatsMap.begin();
        for (uint8_t i = 0; i < MAX_ITEM_PROTO_STATS; ++i)
        {
            if (it2 != proto->generalStatsMap.end())
            {
                buff << it2->second;
                ++it;
            }
            else
            {
                buff << int32_t(0);
            }
        }

        for (uint32_t x = 0; x < MAX_ITEM_PROTO_STATS; ++x)
            buff << int32_t(0);                                     // unk

        for (uint32_t x = 0; x < MAX_ITEM_PROTO_STATS; ++x)
            buff << int32_t(0);                                     // unk

        buff << uint32_t(proto->ScalingStatsEntry);
        buff << uint32_t(0);                                        // damage type
        buff << uint32_t(proto->Delay);
        buff << float(40);                                          // ranged range

        for (uint32_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
            buff << int32_t(0);

        for (uint32_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
            buff << uint32_t(0);

        for (uint32_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
            buff << int32_t(0);

        for (uint32_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
            buff << int32_t(0);

        for (uint32_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
            buff << uint32_t(0);

        for (uint32_t x = 0; x < MAX_ITEM_PROTO_SPELLS; ++x)
            buff << int32_t(0);

        buff << uint32_t(proto->Bonding);

        // item name
        utf8_string name = proto->Name;
        buff << uint16_t(name.length());
        if (name.length())
            buff << name;

        for (uint32_t i = 0; i < 3; ++i)                            // other 3 names
            buff << uint16_t(0);

        std::string desc = proto->Description;
        buff << uint16_t(desc.length());
        if (desc.length())
            buff << desc;

        buff << uint32_t(proto->PageId);
        buff << uint32_t(proto->PageLanguage);
        buff << uint32_t(proto->PageMaterial);
        buff << uint32_t(proto->QuestId);
        buff << uint32_t(proto->LockId);
        buff << int32_t(proto->LockMaterial);
        buff << uint32_t(proto->SheathID);
        buff << int32_t(proto->RandomPropId);
        buff << int32_t(proto->RandomSuffixId);
        buff << uint32_t(proto->ItemSet);

        buff << uint32_t(0);// area
        buff << uint32_t(proto->MapID);
        buff << uint32_t(proto->BagFamily);
        buff << uint32_t(proto->TotemCategory);

        for (uint32_t x = 0; x < MAX_ITEM_PROTO_SOCKETS; ++x)
            buff << uint32_t(proto->Sockets[x].SocketColor);

        for (uint32_t x = 0; x < MAX_ITEM_PROTO_SOCKETS; ++x)
            buff << uint32_t(proto->Sockets[x].Unk);

        buff << uint32_t(proto->SocketBonus);
        buff << uint32_t(proto->GemProperties);
        buff << float(proto->ArmorDamageModifier);
        buff << int32_t(proto->ExistingDuration);
        buff << uint32_t(proto->ItemLimitCategory);
        buff << uint32_t(proto->HolidayId);
        buff << float(proto->ScalingStatsFlag);                     // StatScalingFactor
        buff << uint32_t(0);                                        // archaeology unk
        buff << uint32_t(0);                                        // archaeology findinds count

        SmsgDbReply replyPacket(entry, DB2_REPLY_SPARSE, buff);
        sendManagedPacket(replyPacket);
    }
}

void WorldSession::sendBroadcastDb2Reply(uint32_t entry)
{
    ByteBuffer buffer;

    std::string defaultText = localizedWorldSrv(ServerString::SS_HEY_HOW_CAN_I_HELP_YOU);
    std::string alternativeText = localizedWorldSrv(ServerString::SS_HEY_HOW_CAN_I_HELP_YOU);

    const auto localesNpcText = (language > 0) ? sMySQLStore.getLocalizedNpcGossipText(entry, language) : nullptr;
    const auto pGossip = sMySQLStore.getNpcGossipText(entry);

    if (localesNpcText)
    {
        defaultText = localesNpcText->texts[0][0];
        alternativeText = localesNpcText->texts[0][1];
    }
    else if (pGossip)
    {
        defaultText = pGossip->textHolder[0].texts[0];
        alternativeText = pGossip->textHolder[0].texts[1];
    }

    uint16_t defaultTextLength = static_cast<uint16_t>(defaultText.length());
    uint16_t altTextLength = static_cast<uint16_t>(alternativeText.length());

    buffer << uint32_t(entry);
    buffer << uint32_t(pGossip ? pGossip->textHolder[0].language : 0);
    buffer << uint16_t(defaultTextLength);

    if (defaultTextLength)
        buffer << std::string(defaultText);

    buffer << uint16_t(altTextLength);

    if (altTextLength)
        buffer << std::string(alternativeText);

    for (uint8_t j = 0; j < 8; j++)
        buffer << uint32_t(0);

    buffer << uint32_t(1);

    SmsgDbReply replyPacket(entry, DB2_REPLY_BROADCAST, buffer);
    sendManagedPacket(replyPacket);
}
