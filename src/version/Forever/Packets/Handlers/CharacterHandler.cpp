#include "version/Forever/Packets/CharacterPackets.hpp"
#include "shared/WoWGuid.hpp"
#include "version/Forever/Packets/Packet.hpp"
#include "version/Forever/World/ProtocolUtils.hpp"
#include "version/Forever/Opcodes.hpp"
#include "version/Forever/World/CharacterSelectBootstrap.hpp"
#include "world/Server/WorldSocket.hpp"
#include "world/Server/WorldSession.h"
#include "world/Server/CharacterErrors.h"
#include "world/Server/DatabaseDefinition.hpp"
#include "world/Objects/Units/Players/PlayerDefines.hpp"
#include "world/Management/ObjectMgr.hpp"
#include "Logging/Logger.hpp"

#include <cstdint>
#include <cstring>
#include <limits>


bool WorldSocket::handleForeverCharEnumOpcode(AscEmu::Version::Forever::Packets::Packet& packet)
{
    if (packet.remaining() != 0)
        sLogger.warning("WorldSocket::Forever: CMSG_ENUM_CHARACTERS expected empty payload, got {} byte(s).", packet.remaining());

    ++m_foreverCharacterEnumRequests;

    if (m_foreverSecondEnumPending && m_foreverCharacterEnumRequests > 2U)
    {
        m_foreverBufferedEnumRequest = true;
        return true;
    }

    if (m_foreverCharacterEnumRequests == 1U)
    {
        return sendForeverEmptyCharacterList();
    }

    if (m_foreverCharacterEnumRequests == 2U)
    {
        m_foreverSecondEnumPending = true;
        m_foreverSecondEnumGateSent = false;
        m_foreverSecondEnumSocialContractSeen = false;
        m_foreverSecondEnumStep = 0;

        return true;
    }


    if (!sendForeverCharacterEnumFromDatabase(true))
        return false;

    if (m_foreverPostCreateEnumRefreshPending)
    {
        m_foreverPostCreateEnumRefreshPending = false;
        m_foreverPostCreateEnumRefreshArmed = true;

    }

    return true;
}

bool WorldSocket::handleForeverCheckCharacterNameOpcode(AscEmu::Version::Forever::Packets::Packet& packet)
{
    using namespace AscEmu::Version::Forever;
    using namespace AscEmu::Version::Forever::Packets;

    CheckCharacterNameRequest request;
    if (!parseCheckCharacterName(packet.contents(), packet.size(), request))
    {
        sLogger.warning("WorldSocket::Forever: malformed CMSG_CHECK_CHARACTER_NAME_AVAILABILITY.");
        return true;
    }

    const CharacterErrorCodes validation = VerifyName(request.name);
    uint32_t result = toCharacterResult(validation);
    if (validation == E_CHAR_NAME_SUCCESS)
        result = sObjectMgr.getCachedCharacterInfoByName(request.name) != nullptr ? toCharacterResult(E_CHAR_CREATE_NAME_IN_USE) : toCharacterResult(E_CHAR_NAME_SUCCESS);

    ByteBuffer response;
    response << request.sequenceIndex << result;


    return sendForeverPacket(Opcode::SMSG_CHECK_CHARACTER_NAME_AVAILABILITY_RESULT, response.contents(), static_cast<uint32_t>(response.size()));
}

bool WorldSocket::handleForeverPlayerLoginOpcode(AscEmu::Version::Forever::Packets::Packet& packet)
{
    using namespace AscEmu::Version::Forever;

    WoWGuid guid;
    size_t consumed = 0;
    if (!WoWGuid::unpackModern(packet.contents(), packet.size(), guid, consumed) || guid.getModernHighType() != ModernHighGuid::Player || guid.getModernRealmId() != m_foreverRealmId || guid.getModernLow() == 0 || guid.getModernLow() > std::numeric_limits<uint32_t>::max())
    {
        sLogger.warning("WorldSocket::Forever: malformed CMSG_PLAYER_LOGIN size={} bytes=[{}].", packet.size(), bytesToHex(packet.contents(), packet.size()));
        return true;
    }

    if (packet.size() != consumed + sizeof(float) + sizeof(uint8_t))
    {
        sLogger.warning("WorldSocket::Forever: CMSG_PLAYER_LOGIN unexpected layout guidBytes={} totalBytes={} bytes=[{}].", consumed, packet.size(), bytesToHex(packet.contents(), packet.size()));
        return true;
    }

    const uint32_t guidLow = static_cast<uint32_t>(guid.getModernLow());

    if (m_session == nullptr)
        return false;

    const auto ownership = CharacterDatabase.query("SELECT guid FROM characters WHERE guid = %u AND acct = %u", guidLow, m_session->GetAccountId());
    if (ownership == nullptr)
    {
        sLogger.warning("WorldSocket::Forever: CMSG_PLAYER_LOGIN rejected guidLow={} because it does not belong to account={}.", guidLow, m_session->GetAccountId());
        return true;
    }

    return beginForeverInstanceLogin(guidLow);
}

bool WorldSocket::handleForeverCharCreateOpcode(AscEmu::Version::Forever::Packets::Packet& packet)
{
    return handleForeverCreateCharacter(packet.contents(), static_cast<uint32_t>(packet.size()));
}

bool WorldSocket::handleForeverCharDeleteOpcode(AscEmu::Version::Forever::Packets::Packet& packet)
{
    using namespace AscEmu::Version::Forever;

    WoWGuid guid;
    if (!WoWGuid::unpackModern(packet.contents(), packet.size(), guid) || guid.getModernHighType() != ModernHighGuid::Player || guid.getModernRealmId() != m_foreverRealmId || guid.getModernLow() == 0 || guid.getModernLow() > UINT32_MAX)
    {
        sLogger.warning("WorldSocket::Forever: malformed CMSG_CHAR_DELETE payload size={} bytes=[{}].", packet.size(), bytesToHex(packet.contents(), packet.size()));

        ByteBuffer response;
        response << AscEmu::Version::Forever::Packets::toDeleteCharacterResult(E_CHAR_DELETE_FAILED);
        return sendForeverPacket(Opcode::SMSG_DELETE_CHAR, response.contents(), static_cast<uint32_t>(response.size()));
    }

    const uint64_t guidLow = guid.getModernLow();

    WorldSession* session = getSession();
    if (session == nullptr)
        return false;


    const CharacterErrorCodes coreResult = static_cast<CharacterErrorCodes>(session->deleteCharacter(WoWGuid(guidLow)));
    const uint32_t result = AscEmu::Version::Forever::Packets::toDeleteCharacterResult(coreResult);

    if (coreResult == E_CHAR_DELETE_SUCCESS)
    {
        // Forever-specific glue data is not known to the legacy character
        // deletion path. Remove it only after the core deletion succeeded.
        CharacterDatabase.execute("DELETE FROM character_list_order WHERE acct=%u AND guid=%llu", session->GetAccountId(), static_cast<unsigned long long>(guidLow));
        CharacterDatabase.execute("DELETE FROM character_customizations WHERE guid=%llu", static_cast<unsigned long long>(guidLow));
    }

    ByteBuffer response;
    response << result;


    return sendForeverPacket(Opcode::SMSG_DELETE_CHAR, response.contents(), static_cast<uint32_t>(response.size()));
}

bool WorldSocket::handleForeverUndeleteCooldownOpcode(AscEmu::Version::Forever::Packets::Packet& packet)
{
    using namespace AscEmu::Version::Forever;
    if (packet.remaining() != 0)
        sLogger.warning("WorldSocket::Forever: CMSG_GET_UNDELETE_CHARACTER_COOLDOWN_STATUS expected empty payload.");

    return sendForeverPacket(Opcode::SMSG_UNDELETE_COOLDOWN_STATUS_RESPONSE, CharacterSelectBootstrap::UndeleteCooldown460276.data(), static_cast<uint32_t>(CharacterSelectBootstrap::UndeleteCooldown460276.size()));
}


bool WorldSocket::handleForeverCharacterListAckOpcode(AscEmu::Version::Forever::Packets::Packet& packet)
{
    using namespace AscEmu::Version::Forever;


    // In the observed 69893 flow the payload is:
    //   uint32 characterCount
    //   packed ObjectGuid[characterCount]
    //
    // The client sends this only after it has consumed the complete enum.
    // Refreshing before this point is too early after character creation and
    // can leave one cached row visually missing until a reconnect.
    if (m_foreverPostCreateEnumRefreshArmed)
    {
        m_foreverPostCreateEnumRefreshArmed = false;


        if (!sendForeverCharacterEnumFromDatabase(true))
            return false;
    }

    return true;
}
