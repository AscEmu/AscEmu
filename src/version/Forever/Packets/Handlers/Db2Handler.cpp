#include "version/Forever/World/BroadcastTextId.hpp"
#include "version/Forever/World/Db2Registry.hpp"
#include "world/Server/Opcodes.hpp"
#include "world/Server/WorldSocket.hpp"
#include "Logging/Logger.hpp"

class Creature;
#include "Storage/MySQLDataStore.hpp"

#include <ctime>

namespace
{
    constexpr uint32_t BroadcastTextTableHash = 0x021826BBU;

    // The status field is three bits wide. In the verified 70009 retail
    // BroadcastText reply a valid record is encoded as value 1 (wire byte 0x20
    // with the client's MSB-first bit order). Missing records use value 4.
    constexpr uint8_t ForeverDb2StatusValid = 1U;
    constexpr uint8_t ForeverDb2StatusInvalid = 4U;
}

bool WorldSocket::handleForeverDbQueryBulkOpcode(WorldPacket& request)
{
    if (request.remaining() < sizeof(uint32_t))
    {
        sLogger.warning("WorldSocket::Forever: malformed CMSG_DB_QUERY_BULK.");
        return true;
    }

    uint32_t tableHash = 0;
    request >> tableHash;
    const uint32_t queryCount = request.readBits(13);

    if (request.rpos() + static_cast<size_t>(queryCount) * sizeof(uint32_t) > request.size())
    {
        sLogger.warning(
            "WorldSocket::Forever: malformed CMSG_DB_QUERY_BULK table=0x{:08X}, count={}, remaining={}.",
            tableHash, queryCount, request.remaining());
        return true;
    }

    for (uint32_t i = 0; i < queryCount; ++i)
    {
        uint32_t recordId = 0;
        request >> recordId;

        if (!handleForeverDbQueryRecord(tableHash, recordId))
            return false;
    }

    ++m_foreverDbQueryBulkCount;

    if (m_foreverPostDbEnumRefreshPending && !m_foreverPostDbEnumRefreshSent && m_foreverDbQueryBulkCount >= 2U)
    {
        m_foreverPostDbEnumRefreshSent = true;
        m_foreverPostDbEnumRefreshPending = false;
        return sendForeverCharacterEnumFromDatabase(true);
    }

    return true;
}

bool WorldSocket::handleForeverDbQueryRecord(uint32_t tableHash, uint32_t recordId)
{
    switch (tableHash)
    {
        case BroadcastTextTableHash:
            return handleForeverBroadcastTextDbQuery(recordId);

        default:
            return handleForeverGenericDb2Query(tableHash, recordId);
    }
}

bool WorldSocket::handleForeverBroadcastTextDbQuery(uint32_t recordId)
{
    using namespace AscEmu::Version::Forever;

    const auto type = BroadcastTextId::getType(recordId);
    const uint32_t sourceId = BroadcastTextId::getSourceId(recordId);

    switch (type)
    {
        case BroadcastTextId::Type::Native:
        {
            Db2::RecordView record;
            if (!Db2::getRecord(BroadcastTextTableHash, recordId, record))
            {
                sLogger.info(
                    "WorldSocket::Forever: BroadcastText NATIVE MISSING id={}.",
                    recordId);
                return sendForeverDb2MissingReply(BroadcastTextTableHash, recordId);
            }

            sLogger.info(
                "WorldSocket::Forever: BroadcastText NATIVE id={}, table={}, layout=0x{:08X}.",
                recordId, record.tableName, record.layoutHash);

            return sendForeverDb2Reply(
                BroadcastTextTableHash,
                recordId,
                record.data.data(),
                record.data.size());
        }

        case BroadcastTextId::Type::Gossip:
        {
            ByteBuffer syntheticRecord;
            if (!buildForeverGossipBroadcastTextRecord(syntheticRecord, recordId, sourceId))
            {
                sLogger.info(
                    "WorldSocket::Forever: BroadcastText GOSSIP MISSING wireId={} sourceId={}.",
                    recordId, sourceId);
                return sendForeverDb2MissingReply(BroadcastTextTableHash, recordId);
            }

            sLogger.info(
                "WorldSocket::Forever: BroadcastText GOSSIP wireId={} sourceId={} recordSize={}.",
                recordId, sourceId, syntheticRecord.size());

            return sendForeverDb2Reply(
                BroadcastTextTableHash,
                recordId,
                syntheticRecord.contents(),
                syntheticRecord.size());
        }

        case BroadcastTextId::Type::CreatureText:
        case BroadcastTextId::Type::QuestText:
        case BroadcastTextId::Type::ScriptText:
        case BroadcastTextId::Type::GameObjectText:
            sLogger.info(
                "WorldSocket::Forever: BroadcastText unsupported namespace={} wireId={} sourceId={}.",
                static_cast<uint32_t>(type), recordId, sourceId);
            return sendForeverDb2MissingReply(BroadcastTextTableHash, recordId);

        default:
            sLogger.warning(
                "WorldSocket::Forever: BroadcastText unknown namespace={} wireId={} sourceId={}.",
                static_cast<uint32_t>(type), recordId, sourceId);
            return sendForeverDb2MissingReply(BroadcastTextTableHash, recordId);
    }
}

bool WorldSocket::handleForeverGenericDb2Query(uint32_t tableHash, uint32_t recordId)
{
    using namespace AscEmu::Version::Forever;

    Db2::RecordView record;
    if (!Db2::getRecord(tableHash, recordId, record))
        return sendForeverDb2MissingReply(tableHash, recordId);

    return sendForeverDb2Reply(
        tableHash,
        recordId,
        record.data.data(),
        record.data.size());
}

bool WorldSocket::buildForeverGossipBroadcastTextRecord(ByteBuffer& buffer, uint32_t wireId, uint32_t gossipTextId)
{
    const auto* gossipText = sMySQLStore.getNpcGossipText(gossipTextId);
    if (gossipText == nullptr)
        return false;

    const std::string& defaultText = gossipText->textHolder[0].texts[0];
    const std::string& alternateText = gossipText->textHolder[0].texts[1];

    buffer << defaultText;
    buffer << alternateText;

    // The record ID must match the encoded ID requested by the client, not the
    // underlying AscEmu npc_gossip_texts source ID.
    buffer << uint32_t(wireId);

    // Neutral/default fixed fields taken from the verified 70009 Marshal
    // McBride BroadcastText reply. The two strings and ID above remain custom.
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

bool WorldSocket::sendForeverDb2Reply(
    uint32_t tableHash,
    uint32_t recordId,
    const uint8_t* data,
    size_t dataSize)
{
    ByteBuffer response;
    response << tableHash << recordId << static_cast<uint32_t>(std::time(nullptr));
    response.writeBits<uint8_t>(ForeverDb2StatusValid, 3);
    response.flushBits();

    response << static_cast<uint32_t>(dataSize);
    if (dataSize != 0)
        response.append(data, dataSize);

    return sendForeverPacket(
        SMSG_DB_REPLY,
        response.contents(),
        static_cast<uint32_t>(response.size()));
}

bool WorldSocket::sendForeverDb2MissingReply(uint32_t tableHash, uint32_t recordId)
{
    ByteBuffer response;
    response << tableHash << recordId << static_cast<uint32_t>(std::time(nullptr));
    response.writeBits<uint8_t>(ForeverDb2StatusInvalid, 3);
    response.flushBits();
    response << uint32_t(0);

    return sendForeverPacket(
        SMSG_DB_REPLY,
        response.contents(),
        static_cast<uint32_t>(response.size()));
}

bool WorldSocket::handleForeverHotfixRequestOpcode(WorldPacket& /*packet*/)
{
    return true;
}
