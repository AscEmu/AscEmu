/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BattleNetComm.hpp"

#include "BNetConfig.hpp"
#include "BattleNetCommDefines.hpp"
#include "Database/Database.hpp"
#include "Logging/Logger.hpp"
#include "Network/WorldPacket.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <memory>

namespace AscEmu::Battlenet
{
    extern std::unique_ptr<Database> sBNetLogonSQL;

    BattleNetCommManager& BattleNetCommManager::getInstance()
    {
        static BattleNetCommManager instance;
        return instance;
    }

    void BattleNetCommManager::registerRealm(uint32_t realmId, AscEmu::BattlenetComm::RealmRuleset ruleset, BattleNetCommServerSocket* socket)
    {
        BattleNetCommServerSocket* previous = nullptr;
        {
            std::lock_guard lock(m_mutex);
            const auto itr = m_realms.find(realmId);
            if (itr != m_realms.end() && itr->second.socket != socket) previous = itr->second.socket;
            m_realms[realmId] = { socket, ruleset };
        }

        if (previous != nullptr && previous->isConnected())
        {
            sLogger.warning("BattleNetComm: replacing existing connection for realm {}", realmId);
            previous->disconnect();
        }
    }

    void BattleNetCommManager::unregisterSocket(BattleNetCommServerSocket* socket)
    {
        std::lock_guard lock(m_mutex);
        std::erase_if(m_realms, [socket](const auto& entry) { return entry.second.socket == socket; });
    }

    bool BattleNetCommManager::sendPendingSession(const PendingWorldSession& session)
    {
        std::lock_guard lock(m_mutex);
        const auto itr = m_realms.find(session.realmId);
        if (itr == m_realms.end() || itr->second.socket == nullptr || !itr->second.socket->isConnected()) return false;
        return itr->second.socket->sendPendingSession(session);
    }

    AscEmu::BattlenetComm::RealmRuleset BattleNetCommManager::getRealmRuleset(uint32_t realmId) const
    {
        std::lock_guard lock(m_mutex);
        const auto itr = m_realms.find(realmId);
        return itr != m_realms.end() ? itr->second.ruleset : AscEmu::BattlenetComm::RealmRuleset::PvE;
    }

    BattleNetCommServerSocket::BattleNetCommServerSocket(SOCKET fd)
        : Socket(fd, 64 * 1024, 64 * 1024)
    {
    }

    void BattleNetCommServerSocket::onConnect()
    {
        sLogger.info("BattleNetComm: incoming connection from {}:{}", getRemoteIp(), getRemotePort());
    }

    void BattleNetCommServerSocket::onDisconnect()
    {
        if (m_realmId != 0)
            sLogger.info("BattleNetComm: realm {} disconnected", m_realmId);

        sBattleNetCommManager.unregisterSocket(this);
        m_realmId = 0;
        m_authenticated = false;
    }

    void BattleNetCommServerSocket::onRead()
    {
        while (true)
        {
            if (!m_headerReady)
            {
                if (readBuffer.GetSize() < BattlenetComm::FrameHeaderSize)
                    return;

                std::array<uint8_t, BattlenetComm::FrameHeaderSize> headerBytes{};
                if (!readBuffer.Read(headerBytes.data(), headerBytes.size()))
                    return;

                BattlenetComm::FrameHeader header;
                if (!BattlenetComm::decodeFrameHeader(headerBytes, header))
                {
                    sLogger.failure("BattleNetComm: invalid frame magic from {}:{} (first bytes {:02X} {:02X} {:02X} {:02X})", getRemoteIp(), getRemotePort(), headerBytes[0], headerBytes[1], headerBytes[2], headerBytes[3]);
                    disconnect();
                    return;
                }

                if (header.version != BattlenetComm::FrameVersion)
                {
                    sLogger.failure("BattleNetComm: unsupported frame version {} from {}:{} (expected {})", header.version, getRemoteIp(), getRemotePort(), BattlenetComm::FrameVersion);
                    disconnect();
                    return;
                }

                if (header.opcode == 0)
                {
                    sLogger.failure("BattleNetComm: invalid opcode 0 from {}:{}", getRemoteIp(), getRemotePort());
                    disconnect();
                    return;
                }

                if (header.payloadSize > BattlenetComm::MaxPacketSize)
                {
                    sLogger.failure("BattleNetComm: packet too large from {}:{} ({} bytes, max {})", getRemoteIp(), getRemotePort(), header.payloadSize, BattlenetComm::MaxPacketSize);
                    disconnect();
                    return;
                }

                m_opcode = header.opcode;
                m_remaining = header.payloadSize;
                m_headerReady = true;
            }

            if (readBuffer.GetSize() < m_remaining)
                return;

            WorldPacket packet(m_opcode, m_remaining);
            if (m_remaining != 0)
            {
                packet.resize(m_remaining);
                if (!readBuffer.Read(packet.contents(), m_remaining))
                    return;
            }

            m_opcode = 0;
            m_remaining = 0;
            m_headerReady = false;

            handlePacket(packet);
            if (!isConnected())
                return;
        }
    }

    bool BattleNetCommServerSocket::sendPacket(WorldPacket& packet)
    {
        if (!isConnected() || isDeleted())
            return false;

        if (packet.size() > BattlenetComm::MaxPacketSize)
        {
            sLogger.failure("BattleNetComm: refusing to send oversized packet opcode={} size={} max={}", packet.getOpcode(), packet.size(), BattlenetComm::MaxPacketSize);
            return false;
        }

        std::array<uint8_t, BattlenetComm::FrameHeaderSize> header{};
        BattlenetComm::encodeFrameHeader(header, static_cast<uint16_t>(packet.getOpcode()), static_cast<uint32_t>(packet.size()));

        burstBegin();
        bool ok = burstSend(header.data(), static_cast<uint32_t>(header.size()));
        if (ok && packet.size() != 0)
            ok = burstSend(packet.contents(), static_cast<uint32_t>(packet.size()));
        if (ok)
            burstPush();
        burstEnd();
        return ok;
    }

    void BattleNetCommServerSocket::handlePacket(WorldPacket& packet)
    {
        if (!m_authenticated && packet.getOpcode() != BattlenetComm::CMSG_REGISTER_REALM)
        {
            sLogger.failure("BattleNetComm: unauthenticated world sent opcode {}", packet.getOpcode());
            disconnect();
            return;
        }

        switch (packet.getOpcode())
        {
            case BattlenetComm::CMSG_REGISTER_REALM:
                handleRegisterRealm(packet);
                break;
            default:
                sLogger.failure("BattleNetComm: unknown world opcode {}", packet.getOpcode());
                disconnect();
                break;
        }
    }

    void BattleNetCommServerSocket::handleRegisterRealm(WorldPacket& packet)
    {
        uint32_t protocolVersion = 0;
        uint32_t realmId = 0;
        std::string realmName;
        uint8_t rulesetValue = 0;
        std::string sharedSecret;

        packet >> protocolVersion;
        packet >> realmId;
        packet >> realmName;
        packet >> rulesetValue;
        packet >> sharedSecret;

        if (packet.hadReadFailure() || packet.rpos() != packet.size())
        {
            sLogger.failure("BattleNetComm: malformed realm registration from {}:{} (payload={} bytes, consumed={} bytes)", getRemoteIp(), getRemotePort(), packet.size(), packet.rpos());
            disconnect();
            return;
        }

        uint8_t result = 0;
        if (protocolVersion != BattlenetComm::ProtocolVersion)
        {
            sLogger.failure("BattleNetComm: realm {} uses unsupported protocol version {}", realmId, protocolVersion);
            result = 1;
        }
        else if (!AscEmu::BattlenetComm::isValidRealmRuleset(rulesetValue)) { sLogger.failure("BattleNetComm: realm {} sent invalid ruleset {}", realmId, rulesetValue); result = 4; }
        else if (sharedSecret != bnetConfig.battleNetComm.sharedSecret)
        {
            sLogger.failure("BattleNetComm: realm {} authentication failed from {}:{}", realmId, getRemoteIp(), getRemotePort());
            result = 2;
        }
        else if (realmId == 0 || !sBNetLogonSQL || !sBNetLogonSQL->query("SELECT id FROM realms WHERE id = %u LIMIT 1", realmId))
        {
            sLogger.failure("BattleNetComm: realm {} is not present in the logon database", realmId);
            result = 3;
        }

        WorldPacket response(BattlenetComm::SMSG_REGISTER_REALM_RESULT, 16);
        response << result;
        response << realmId;
        sendPacket(response);

        if (result != 0)
        {
            disconnect();
            return;
        }

        m_realmId = realmId;
        m_authenticated = true;
        const auto ruleset = static_cast<AscEmu::BattlenetComm::RealmRuleset>(rulesetValue);
        sBattleNetCommManager.registerRealm(realmId, ruleset, this);
        sLogger.info("BattleNetComm: registered realm {} ('{}') ruleset={} from {}:{}", realmId, realmName, rulesetValue, getRemoteIp(), getRemotePort());
    }

    bool BattleNetCommServerSocket::sendPendingSession(const PendingWorldSession& session)
    {
        WorldPacket packet(BattlenetComm::SMSG_PENDING_WORLD_SESSION, 256);
        packet << session.accountId;
        packet << session.gameAccountId;
        packet << session.realmId;
        packet << session.clientBuild;
        packet << session.region;
        packet << session.expiresAt;
        packet << session.gameAccountName;
        packet << session.realmJoinTicket;

        packet << static_cast<uint32_t>(session.worldAuthKeyData.size());
        packet.append(session.worldAuthKeyData.data(), session.worldAuthKeyData.size());

        packet << static_cast<uint32_t>(session.joinSecret.size());
        packet.append(session.joinSecret.data(), session.joinSecret.size());

        const bool sent = sendPacket(packet);
        if (sent)
        {
            sLogger.debug("BattleNetComm: queued pending world session account={} realm={} build={} ticket_bytes={} expires={}", session.accountId, session.realmId, session.clientBuild, session.realmJoinTicket.size(), session.expiresAt);
        }
        return sent;
    }
}
