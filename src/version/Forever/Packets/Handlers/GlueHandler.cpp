#include "version/Forever/Opcodes.hpp"
#include "version/Forever/OpcodeTable.hpp"
#include "version/Forever/Packets/Packet.hpp"
#include "version/Forever/World/CharacterSelectBootstrap.hpp"
#include "version/Forever/World/Protocol.hpp"
#include "version/Forever/World/ProtocolUtils.hpp"
#include "world/Server/WorldSocket.hpp"
#include "Logging/Logger.hpp"

#include <array>
#include <ctime>


bool WorldSocket::processForeverGlueState(AscEmu::Version::Forever::Packets::Packet& packet, bool& consumed)
{
    using namespace AscEmu::Version::Forever;
    consumed = false;

    if (!m_foreverSecondEnumPending)
        return true;

    const Opcode opcode = packet.getOpcode();

    if (!m_foreverSecondEnumGateSent)
    {
        static constexpr std::array<Opcode, 4> RequiredPrefix =
        {
            Opcode::CMSG_BATTLE_PAY_GET_PURCHASE_LIST,
            Opcode::CMSG_BATTLE_PAY_GET_PURCHASE_LIST,
            Opcode::CMSG_BATTLE_PAY_GET_PRODUCT_LIST,
            Opcode::CMSG_UPDATE_VAS_PURCHASE_STATES
        };

        if (m_foreverSecondEnumStep < RequiredPrefix.size() && opcode == RequiredPrefix[m_foreverSecondEnumStep])
        {
            ++m_foreverSecondEnumStep;
            consumed = true;


            if (m_foreverSecondEnumStep == RequiredPrefix.size() && !sendForeverHotfixBootstrap())
                return false;

            return true;
        }

        if (m_foreverSecondEnumStep == RequiredPrefix.size())
        {
            if (opcode == Opcode::CMSG_SOCIAL_CONTRACT_REQUEST)
            {
                m_foreverSecondEnumSocialContractSeen = true;
                consumed = true;
                return true;
            }

            if (opcode == Opcode::CMSG_BATTLE_PAY_GET_PURCHASE_LIST)
            {
                consumed = true;
                if (!sendForeverPacket(Opcode::SMSG_CHARACTER_SELECT_GATE, CharacterSelectBootstrap::CharacterSelectGate460382.data(), static_cast<uint32_t>(CharacterSelectBootstrap::CharacterSelectGate460382.size())))
                    return false;

                m_foreverSecondEnumGateSent = true;
                return true;
            }
        }
    }

    if (opcode == Opcode::CMSG_CHARACTER_SELECT_GATE_ACK)
    {
        consumed = true;

        if (!sendForeverSecondEnumCompletion())
            return false;

        // Restore the exact state transition from the last known-good
        // monolithic implementation. The refactor previously returned
        // immediately after sendForeverSecondEnumCompletion(), leaving
        // m_foreverSecondEnumPending=true forever. That caused every later
        // CMSG_ENUM_CHARACTERS to be buffered indefinitely and the client to
        // remain on "Retrieving character list" / fall back to create.
        m_foreverSecondEnumPending = false;
        m_foreverSecondEnumGateSent = false;
        m_foreverSecondEnumStep = 0;

        if (m_foreverBufferedEnumRequest)
        {
            m_foreverBufferedEnumRequest = false;


            if (!sendForeverCharacterEnumFromDatabase(true))
                return false;

            // Preserve the proven timing workaround: after the first DB-backed
            // enum the client performs its initial DB2 bulk phase. Refresh once
            // after batch #2 so all characters are visible on first entry.
            m_foreverDbQueryBulkCount = 0;
            m_foreverPostDbEnumRefreshPending = true;
            m_foreverPostDbEnumRefreshSent = false;
        }

        m_foreverSecondEnumSocialContractSeen = false;
        return true;
    }

    return true;
}

bool WorldSocket::handleForeverIgnoredGlueOpcode(AscEmu::Version::Forever::Packets::Packet&)
{
    using namespace AscEmu::Version::Forever;


    return true;
}

bool WorldSocket::handleForeverQuickJoinOpcode(AscEmu::Version::Forever::Packets::Packet&)
{
    using namespace AscEmu::Version::Forever;
    return true;
}

bool WorldSocket::handleForeverLastCatalogFetchOpcode(AscEmu::Version::Forever::Packets::Packet& packet)
{
    if (packet.remaining() != 0)
        sLogger.warning("WorldSocket::Forever: CMSG_GET_LAST_CATALOG_FETCH expected empty payload.");
    return true;
}


bool WorldSocket::sendForeverHotfixBootstrap()
{
    using namespace AscEmu::Version::Forever;

    if (m_foreverHotfixBootstrapSent)
        return true;

    {
        ByteBuffer cacheVersion;
        cacheVersion << uint32_t(0);
        if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_CACHE_VERSION, cacheVersion.contents(), static_cast<uint32_t>(cacheVersion.size())))
            return false;
    }

    {
        // Same virtual-realm construction used by our working Midnight core.
        const uint32_t virtualRealmAddress =
            ((m_foreverRegionId & 0xFFU) << 24U) |
            ((m_foreverBattlegroupId & 0xFFU) << 16U) |
            (m_foreverRealmId & 0xFFFFU);

        ByteBuffer availableHotfixes;
        availableHotfixes << int32_t(virtualRealmAddress);
        availableHotfixes << uint32_t(0);

        if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_AVAILABLE_HOTFIXES, availableHotfixes.contents(), static_cast<uint32_t>(availableHotfixes.size())))
            return false;

    }

    m_foreverHotfixBootstrapSent = true;
    return true;
}

bool WorldSocket::sendForeverSecondEnumCompletion()
{
    using namespace AscEmu::Version::Forever;

    // Exact static ordering from the official Forever 1.60.1.69893 capture
    // following CMSG 0x0044013A. Keep the corrected pre-enum bootstrap from
    // the previous bootstrap, but use the capture-derived 69893 payloads in this completion
    // block so the two changes can be tested independently.
    for (uint32_t i = 0; i < 2; ++i)
    {
        if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_CHARACTER_ENUM_PRELUDE, CharacterSelectBootstrap::EnumPrelude46021D.data(), static_cast<uint32_t>(CharacterSelectBootstrap::EnumPrelude46021D.size())))
            return false;
    }

    if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_CHARACTER_ENUM_PRELUDE_EXTENDED, CharacterSelectBootstrap::EnumPreludeExtended46021C.data(), static_cast<uint32_t>(CharacterSelectBootstrap::EnumPreludeExtended46021C.size())))
        return false;

    if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_CHARACTER_ENUM_PRELUDE, CharacterSelectBootstrap::EnumPrelude46021D.data(), static_cast<uint32_t>(CharacterSelectBootstrap::EnumPrelude46021D.size())))
        return false;

    if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_UNDELETE_COOLDOWN_STATUS_RESPONSE, CharacterSelectBootstrap::UndeleteCooldown460276.data(), static_cast<uint32_t>(CharacterSelectBootstrap::UndeleteCooldown460276.size())))
        return false;

    if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_CHARACTER_SELECT_STATUS, CharacterSelectBootstrap::CharacterSelectStatus46029D.data(), static_cast<uint32_t>(CharacterSelectBootstrap::CharacterSelectStatus46029D.size())))
        return false;

    // The official capture contained 0x00460325 because that session first
    // emitted CMSG_SOCIAL_CONTRACT_REQUEST. Do not send an unsolicited social
    // contract response when this account/session skipped that request.
    if (m_foreverSecondEnumSocialContractSeen)
    {
        if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_SOCIAL_CONTRACT_REQUEST_RESPONSE, CharacterSelectBootstrap::SocialContract460325.data(), static_cast<uint32_t>(CharacterSelectBootstrap::SocialContract460325.size())))
            return false;
    }

    if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_ENUM_CHARACTERS_RESULT, CharacterSelectBootstrap::EmptyCharacterList.data(), static_cast<uint32_t>(CharacterSelectBootstrap::EmptyCharacterList.size())))
        return false;

    // Capture-derived 69893 AccountDataTimes payload. Only the observed Unix
    // timestamp field at bytes 2..5 is patched for the current session.
    auto postEnumState = CharacterSelectBootstrap::PostEnum4601B5Template;
    const auto now = static_cast<uint32_t>(std::time(nullptr));
    std::memcpy(postEnumState.data() + 2, &now, sizeof(now));

    if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_ACCOUNT_DATA_TIMES, postEnumState.data(), static_cast<uint32_t>(postEnumState.size())))
        return false;

    // Use the exact 69893 collection payload here rather than the Midnight
    // semantic empty serializer. The pre-enum bootstrap remains Midnight-style.
    if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_ACCOUNT_ITEM_COLLECTION_DATA, CharacterSelectBootstrap::AccountItemCollection460362.data(), static_cast<uint32_t>(CharacterSelectBootstrap::AccountItemCollection460362.size())))
        return false;

    return true;
}
