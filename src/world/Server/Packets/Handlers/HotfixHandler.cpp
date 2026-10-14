/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/WorldSession.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WorldStrings.h"
#include "Macros/ItemMacros.hpp"
#include "Server/Packets/CmsgRequestHotfix.h"
#include "Server/Packets/CmsgDbQueryBulk.h"
#include "Server/Packets/SmsgDbReply.h"
#if defined(AE_FOREVER)
#include "version/Forever/World/BroadcastTextId.hpp"
#include "version/Forever/World/Db2Registry.hpp"
#include "Logging/Logger.hpp"
#endif

using namespace AscEmu::Packets;

#define DB2_REPLY_SPARSE    2442913102
#define DB2_REPLY_ITEM      1344507586
#define DB2_REPLY_BROADCAST   35137211

#if defined(AE_FOREVER)
namespace
{
    constexpr uint32_t BroadcastTextTableHash = 0x021826BBU;
    constexpr uint32_t TactKeyTableHash = 0xDF2F53CFU;
}
#endif

void WorldSession::handleDbQueryBulkOpcode(WorldPacket& recvPacket)
{
    CmsgDbQueryBulk request;
    if (!parsePacket(recvPacket, request))
        return;

#if defined(AE_FOREVER)
    if (getClientProtocol().isForever())
    {
        uint32_t tactKeyServed = 0;
        uint32_t tactKeyMissing = 0;

        for (uint32_t recordId : request.recordIds)
        {
            bool found = false;
            if (!sendDbQueryRecord(request.tableHash, recordId, &found))
            {
                sLogger.warning("WorldSession: failed to process CMSG_DB_QUERY_BULK table=0x{:08X} record={}.", request.tableHash, recordId);
                return;
            }

            if (request.tableHash == TactKeyTableHash)
            {
                if (found)
                    ++tactKeyServed;
                else
                    ++tactKeyMissing;
            }
        }

        if (request.tableHash == TactKeyTableHash)
            sLogger.info("WorldSession: TactKey query records={} served={} missing={}.", request.recordIds.size(), tactKeyServed, tactKeyMissing);

        return;
    }
#endif

    (void)request;
}

void WorldSession::handleRequestHotfix(WorldPacket& recvPacket)
{
    CmsgRequestHotfix srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    auto const protocol = _socket->getClientProtocol();
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

#if defined(AE_FOREVER)
bool WorldSession::sendDbQueryRecord(uint32_t tableHash, uint32_t recordId, bool* found)
{
    using namespace AscEmu::Version::Forever;

    if (found != nullptr)
        *found = false;

    if (tableHash == BroadcastTextTableHash)
    {
        const auto type = BroadcastTextId::getType(recordId);
        const uint32_t sourceId = BroadcastTextId::getSourceId(recordId);

        switch (type)
        {
            case BroadcastTextId::Type::Native:
            {
                Db2::RecordView record;
                if (!Db2::getRecord(BroadcastTextTableHash, recordId, record))
                {
                    sLogger.info("WorldSession: BroadcastText NATIVE MISSING id={}.", recordId);
                    return sendDb2Reply(BroadcastTextTableHash, recordId, ByteBuffer{}, false);
                }

                ByteBuffer buffer;
                buffer.append(record.data.data(), record.data.size());
                sLogger.info("WorldSession: BroadcastText NATIVE id={}, table={}, layout=0x{:08X}.", recordId, record.tableName, record.layoutHash);
                if (found != nullptr)
                    *found = true;
                return sendDb2Reply(BroadcastTextTableHash, recordId, std::move(buffer), true);
            }

            case BroadcastTextId::Type::Gossip:
            {
                ByteBuffer buffer;
                if (!buildGossipBroadcastTextRecord(buffer, recordId, sourceId))
                {
                    sLogger.info("WorldSession: BroadcastText GOSSIP MISSING wireId={} sourceId={}.", recordId, sourceId);
                    return sendDb2Reply(BroadcastTextTableHash, recordId, ByteBuffer{}, false);
                }

                sLogger.info("WorldSession: BroadcastText GOSSIP wireId={} sourceId={} recordSize={}.", recordId, sourceId, buffer.size());
                if (found != nullptr)
                    *found = true;
                return sendDb2Reply(BroadcastTextTableHash, recordId, std::move(buffer), true);
            }

            case BroadcastTextId::Type::CreatureText:
            case BroadcastTextId::Type::QuestText:
            case BroadcastTextId::Type::ScriptText:
            case BroadcastTextId::Type::GameObjectText:
                sLogger.info("WorldSession: BroadcastText unsupported namespace={} wireId={} sourceId={}.", static_cast<uint32_t>(type), recordId, sourceId);
                return sendDb2Reply(BroadcastTextTableHash, recordId, ByteBuffer{}, false);

            default:
                sLogger.warning("WorldSession: BroadcastText unknown namespace={} wireId={} sourceId={}.", static_cast<uint32_t>(type), recordId, sourceId);
                return sendDb2Reply(BroadcastTextTableHash, recordId, ByteBuffer{}, false);
        }
    }

    Db2::RecordView record;
    const bool recordFound = Db2::getRecord(tableHash, recordId, record);
    if (found != nullptr)
        *found = recordFound;

    if (!recordFound)
        return sendDb2Reply(tableHash, recordId, ByteBuffer{}, false);

    ByteBuffer buffer;
    buffer.append(record.data.data(), record.data.size());
    return sendDb2Reply(tableHash, recordId, std::move(buffer), true);
}

bool WorldSession::buildGossipBroadcastTextRecord(ByteBuffer& buffer, uint32_t wireId, uint32_t gossipTextId)
{
    const auto* gossipText = sMySQLStore.getNpcGossipText(gossipTextId);
    if (gossipText == nullptr)
        return false;

    const std::string& defaultText = gossipText->textHolder[0].texts[0];
    const std::string& alternateText = gossipText->textHolder[0].texts[1];

    buffer << defaultText;
    buffer << alternateText;
    buffer << uint32_t(wireId);

    static constexpr uint8_t BroadcastText70009Tail[42] =
    {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
    };

    buffer.append(BroadcastText70009Tail, sizeof(BroadcastText70009Tail));
    return true;
}

bool WorldSession::sendDb2Reply(uint32_t tableHash, uint32_t recordId, ByteBuffer buffer, bool found)
{
    AscEmu::Packets::SmsgDbReply response(recordId, tableHash, std::move(buffer), found);
    sendManagedPacket(response);
    return true;
}
#endif
