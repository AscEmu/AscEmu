#include "version/Forever/Packets/ClientConfigPackets.hpp"
#include "version/Forever/World/ProtocolUtils.hpp"
#include "world/Server/WorldSocket.hpp"
#include "world/Server/WorldSession.h"
#include "world/Server/DatabaseDefinition.hpp"
#include "Logging/Logger.hpp"
#include <array>

bool WorldSocket::handleForeverUpdateAccountDataOpcode(AscEmu::Version::Forever::Packets::Packet& packet)
{
    using namespace AscEmu::Version::Forever;
    using namespace AscEmu::Version::Forever::Packets;

    UpdateAccountDataRequest request;
    if (!parseUpdateAccountData(packet, request))
    {
        sLogger.warning("WorldSocket::Forever: malformed CMSG_UPDATE_ACCOUNT_DATA payload={} byte(s).", packet.size());
        return true;
    }


    if (request.dataType != 16)
        return true;

    if (m_session == nullptr)
        return false;

    const uint32_t accountId = m_session->GetAccountId();

    // Type 16 is the Forever character-order account data observed in the
    // official 69913 sniff. Persist the resolved positions server-side so our
    // DB-backed enum can use the same order after reconnect/restart.
    for (const CharacterOrderEntry& entry : request.characterOrder)
    {
        CharacterDatabase.waitExecute("INSERT INTO character_list_order (acct, guid, listPosition) " "VALUES (%u, %llu, %u) " "ON DUPLICATE KEY UPDATE listPosition=VALUES(listPosition)", accountId, static_cast<unsigned long long>(entry.guidLow), static_cast<uint32_t>(entry.position));

    }

    // Official 69913 sends SMSG 0x004601B4 after CMSG_UPDATE_ACCOUNT_DATA
    // type 16.  The observed payload is exactly:
    //   00 00               packed empty ObjectGuid
    //   10 00 00 00         data type = 16
    //   00 00 00 00         success/result = 0
    //
    // Without this completion packet the client keeps the account-data update
    // outstanding and later character-order drag operations may no longer
    // produce another CMSG_UPDATE_ACCOUNT_DATA.
    std::array<uint8_t, 10> complete{
        0x00, 0x00,
        0x10, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_UPDATE_ACCOUNT_DATA_COMPLETE, complete.data(), static_cast<uint32_t>(complete.size())))
        return false;


    return true;
}
