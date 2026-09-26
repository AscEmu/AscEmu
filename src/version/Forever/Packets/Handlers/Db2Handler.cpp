#include "version/Forever/World/Db2Registry.hpp"
#include "world/Server/Opcodes.hpp"
#include "world/Server/WorldSocket.hpp"
#include "Logging/Logger.hpp"

class Creature;
#include "Storage/MySQLDataStore.hpp"

#include <ctime>

bool WorldSocket::handleForeverDbQueryBulkOpcode(WorldPacket& request)
{
    using namespace AscEmu::Version::Forever;

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
        sLogger.warning("WorldSocket::Forever: malformed CMSG_DB_QUERY_BULK table=0x{:08X}, count={}, remaining={}.", tableHash, queryCount, request.remaining());
        return true;
    }

    const uint32_t timestamp = static_cast<uint32_t>(std::time(nullptr));

    // The status field is three bits wide. Forever captures established 4 as
    // the invalid/missing-record status. Status 0 is the normal record reply.
    constexpr uint8_t ForeverDb2StatusValid = 0U;
    constexpr uint8_t ForeverDb2StatusInvalid = 4U;

    for (uint32_t i = 0; i < queryCount; ++i)
    {
        uint32_t recordId = 0;
        request >> recordId;
        Db2::RecordView record;
        bool found = Db2::getRecord(tableHash, recordId, record);

        // Verified from the 1.60.1.70009 retail capture for Marshal McBride:
        //
        //   SMSG_GOSSIP_MESSAGE: RandomTextID = 7590
        //   CMSG_DB_QUERY_BULK : tableHash = 0x021826BB, recordId = 7590
        //   SMSG_DB_REPLY      : 164-byte BroadcastText record
        //
        // The reply record is:
        //   CString Text
        //   CString Text1
        //   uint32  ID
        //   42-byte fixed tail
        //
        // AscEmu allows custom npc_gossip_texts ids. When such an id does not
        // exist in the extracted BroadcastText.db2, expose the DB text as a
        // synthetic BroadcastText record using the exact 70009 wire shape.
        constexpr uint32_t BroadcastTextTableHash = 0x021826BBU;
        ByteBuffer syntheticRecord;

        if (!found && tableHash == BroadcastTextTableHash)
        {
            if (const auto* gossipText = sMySQLStore.getNpcGossipText(recordId))
            {
                const std::string& defaultText = gossipText->textHolder[0].texts[0];
                const std::string& alternateText = gossipText->textHolder[0].texts[1];

                // ByteBuffer's std::string serializer is the same nul-terminated
                // CString shape observed in the retail SMSG_DB_REPLY.
                syntheticRecord << defaultText;
                syntheticRecord << alternateText;
                syntheticRecord << uint32_t(recordId);

                // Exact fixed tail observed after ID=7590 in the 70009 retail
                // BroadcastText record for this NPC. Plain custom greetings use
                // the same neutral/default values; only the record ID and strings
                // are replaced.
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
                syntheticRecord.append(BroadcastText70009Tail, sizeof(BroadcastText70009Tail));

                found = true;

                sLogger.info(
                    "WorldSocket::Forever: synthetic BroadcastText.db2 record id={} from npc_gossip_texts, textLen={}, altLen={}, recordSize={}.",
                    recordId, defaultText.size(), alternateText.size(), syntheticRecord.size());
            }
        }

        ByteBuffer response;
        response << tableHash << recordId << timestamp;
        response.writeBits<uint8_t>(found ? ForeverDb2StatusValid : ForeverDb2StatusInvalid, 3);
        response.flushBits();

        if (found)
        {
            if (syntheticRecord.size() != 0)
            {
                response << static_cast<uint32_t>(syntheticRecord.size());
                response.append(syntheticRecord);
            }
            else
            {
                response << static_cast<uint32_t>(record.data.size());
                response.append(record.data.data(), record.data.size());
            }
        }
        else
        {
            response << uint32_t(0);
        }

        if (!sendForeverPacket(SMSG_DB_REPLY, response.contents(), static_cast<uint32_t>(response.size())))
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

bool WorldSocket::handleForeverHotfixRequestOpcode(WorldPacket& /*packet*/)
{
    using namespace AscEmu::Version::Forever;

    return true;
}
