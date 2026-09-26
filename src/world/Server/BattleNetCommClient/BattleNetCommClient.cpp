/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BattleNetCommClient.hpp"

#include "BattleNetCommDefines.hpp"
#include "Logging/Logger.hpp"
#include "Network/WorldPacket.hpp"
#include "Server/World.h"
#include "Threading/Thread.hpp"
#include "Threading/ThreadPool.hpp"

#include <algorithm>
#include <array>
#include <chrono>

namespace AscEmu::BattlenetComm
{
    BattleNetCommClientSocket::BattleNetCommClientSocket(SOCKET fd)
        : Socket(fd, 64 * 1024, 64 * 1024)
    {
    }

    void BattleNetCommClientSocket::onConnect()
    {
        sBattleNetCommClient.setSocket(this);

        WorldPacket packet(CMSG_REGISTER_REALM, 128);
        packet << ProtocolVersion;
        packet << worldConfig.battleNetComm.realmId;
        packet << worldConfig.battleNetComm.realmName;
        packet << static_cast<uint8_t>(worldConfig.battleNetComm.ruleset);
        packet << worldConfig.battleNetComm.sharedSecret;

        if (!sendPacket(packet))
        {
            sLogger.failure("BattleNetCommClient: failed to send realm registration");
            disconnect();
            return;
        }

        sLogger.info("BattleNetCommClient: connected to {}:{}; registering realm {} ('{}') ruleset={}", worldConfig.battleNetComm.host, worldConfig.battleNetComm.port, worldConfig.battleNetComm.realmId, worldConfig.battleNetComm.realmName, static_cast<uint32_t>(worldConfig.battleNetComm.ruleset));
    }

    void BattleNetCommClientSocket::onDisconnect()
    {
        sBattleNetCommClient.clearSocket(this);
        sLogger.info("BattleNetCommClient: disconnected from Battle.net server");
    }

    void BattleNetCommClientSocket::onRead()
    {
        while (true)
        {
            if (!m_headerReady)
            {
                if (readBuffer.GetSize() < FrameHeaderSize)
                    return;

                std::array<uint8_t, FrameHeaderSize> headerBytes{};
                if (!readBuffer.Read(headerBytes.data(), headerBytes.size()))
                    return;

                FrameHeader header;
                if (!decodeFrameHeader(headerBytes, header))
                {
                    sLogger.failure("BattleNetCommClient: invalid frame magic (first bytes {:02X} {:02X} {:02X} {:02X})", headerBytes[0], headerBytes[1], headerBytes[2], headerBytes[3]);
                    disconnect();
                    return;
                }

                if (header.version != FrameVersion)
                {
                    sLogger.failure("BattleNetCommClient: unsupported frame version {} (expected {})", header.version, FrameVersion);
                    disconnect();
                    return;
                }

                if (header.opcode == 0)
                {
                    sLogger.failure("BattleNetCommClient: invalid opcode 0");
                    disconnect();
                    return;
                }

                if (header.payloadSize > MaxPacketSize)
                {
                    sLogger.failure("BattleNetCommClient: packet too large ({} bytes, max {})", header.payloadSize, MaxPacketSize);
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

    bool BattleNetCommClientSocket::sendPacket(WorldPacket& packet)
    {
        if (!isConnected() || isDeleted())
            return false;

        if (packet.size() > MaxPacketSize)
        {
            sLogger.failure("BattleNetCommClient: refusing to send oversized packet opcode={} size={} max={}", packet.getOpcode(), packet.size(), MaxPacketSize);
            return false;
        }

        std::array<uint8_t, FrameHeaderSize> header{};
        encodeFrameHeader(header, static_cast<uint16_t>(packet.getOpcode()), static_cast<uint32_t>(packet.size()));

        burstBegin();
        bool ok = burstSend(header.data(), static_cast<uint32_t>(header.size()));
        if (ok && packet.size() != 0)
            ok = burstSend(packet.contents(), static_cast<uint32_t>(packet.size()));
        if (ok)
            burstPush();
        burstEnd();
        return ok;
    }

    void BattleNetCommClientSocket::handlePacket(WorldPacket& packet)
    {
        switch (packet.getOpcode())
        {
            case SMSG_REGISTER_REALM_RESULT:
                handleRegisterResult(packet);
                break;
            case SMSG_PENDING_WORLD_SESSION:
                handlePendingWorldSession(packet);
                break;
            default:
                sLogger.failure("BattleNetCommClient: unknown opcode {}", packet.getOpcode());
                disconnect();
                break;
        }
    }

    void BattleNetCommClientSocket::handleRegisterResult(WorldPacket& packet)
    {
        uint8_t result = 0xFF;
        uint32_t realmId = 0;
        packet >> result;
        packet >> realmId;

        if (packet.hadReadFailure() || packet.rpos() != packet.size())
        {
            sLogger.failure("BattleNetCommClient: malformed realm registration result (payload={} bytes, consumed={} bytes)", packet.size(), packet.rpos());
            disconnect();
            return;
        }

        if (result != 0)
        {
            sLogger.failure("BattleNetCommClient: realm {} registration rejected with result {}", realmId, result);
            disconnect();
            return;
        }

        sLogger.info("BattleNetCommClient: realm {} registration accepted", realmId);
    }

    void BattleNetCommClientSocket::handlePendingWorldSession(WorldPacket& packet)
    {
        PendingWorldSession session;
        packet >> session.accountId;
        packet >> session.gameAccountId;
        packet >> session.realmId;
        packet >> session.clientBuild;
        packet >> session.region;
        packet >> session.expiresAt;
        packet >> session.gameAccountName;
        packet >> session.realmJoinTicket;

        if (packet.hadReadFailure())
        {
            sLogger.failure("BattleNetCommClient: malformed pending world session header");
            disconnect();
            return;
        }

        uint32_t worldAuthKeySize = 0;
        packet >> worldAuthKeySize;
        if (worldAuthKeySize != session.worldAuthKeyData.size() || packet.rpos() + worldAuthKeySize > packet.size())
        {
            sLogger.failure("BattleNetCommClient: invalid world auth KeyData length {}", worldAuthKeySize);
            disconnect();
            return;
        }

        packet.read(session.worldAuthKeyData.data(), worldAuthKeySize);

        uint32_t joinSecretSize = 0;
        packet >> joinSecretSize;
        if (joinSecretSize != session.joinSecret.size() || packet.rpos() + joinSecretSize > packet.size())
        {
            sLogger.failure("BattleNetCommClient: invalid JoinSecret length {}", joinSecretSize);
            disconnect();
            return;
        }

        packet.read(session.joinSecret.data(), joinSecretSize);
        if (packet.hadReadFailure() || packet.rpos() != packet.size())
        {
            sLogger.failure("BattleNetCommClient: malformed pending world session payload (payload={} bytes, consumed={} bytes)", packet.size(), packet.rpos());
            disconnect();
            return;
        }

        sBattleNetCommClient.storePendingSession(std::move(session));
    }

    BattleNetCommClientManager& BattleNetCommClientManager::getInstance()
    {
        static BattleNetCommClientManager instance;
        return instance;
    }

    void BattleNetCommClientManager::start(AscEmu::Threading::AEThreadPool& threadPool)
    {
        m_running.store(true);
        threadPool.addDedicatedThread("BattleNetCommWatcher", [this](AscEmu::Threading::AEThread& thread) { run(thread); });
    }

    void BattleNetCommClientManager::finalize()
    {
        m_running.store(false);

        BattleNetCommClientSocket* socket = nullptr;
        {
            std::lock_guard lock(m_socketMutex);
            socket = m_socket;
            m_socket = nullptr;
        }

        if (socket != nullptr && socket->isConnected())
            socket->disconnect();

        std::lock_guard pendingLock(m_pendingMutex);
        m_pendingSessions.clear();
    }

    void BattleNetCommClientManager::run(AscEmu::Threading::AEThread& thread)
    {
        while (m_running.load() && !thread.isKilled())
        {
            bool connected = false;
            {
                std::lock_guard lock(m_socketMutex);
                connected = m_socket != nullptr && m_socket->isConnected();
            }

            if (!connected)
            {
                sLogger.info("BattleNetCommClient: attempting connection to {}:{}", worldConfig.battleNetComm.host, worldConfig.battleNetComm.port);

                if (auto* socket = ConnectTCPSocket<BattleNetCommClientSocket>(worldConfig.battleNetComm.host.c_str(), static_cast<u_short>(worldConfig.battleNetComm.port)); socket == nullptr)
                {
                    sLogger.warning("BattleNetCommClient: connection failed; retrying in 5 seconds");
                }
            }

            cleanupExpiredSessions();

            for (uint32_t i = 0; i < 50 && m_running.load() && !thread.isKilled(); ++i)
                AscEmu::Threading::sleep(100);
        }
    }

    void BattleNetCommClientManager::setSocket(BattleNetCommClientSocket* socket)
    {
        std::lock_guard lock(m_socketMutex);
        m_socket = socket;
    }

    void BattleNetCommClientManager::clearSocket(BattleNetCommClientSocket* socket)
    {
        std::lock_guard lock(m_socketMutex);
        if (m_socket == socket)
            m_socket = nullptr;
    }

    void BattleNetCommClientManager::storePendingSession(PendingWorldSession session)
    {
        const std::string ticket = session.realmJoinTicket;
        size_t pendingCount = 0;
        {
            std::lock_guard lock(m_pendingMutex);
            m_pendingSessions[ticket] = std::move(session);
            pendingCount = m_pendingSessions.size();
        }

    }

    bool BattleNetCommClientManager::getPendingSession(const std::string& realmJoinTicket, PendingWorldSession& session, bool consume)
    {
        std::lock_guard lock(m_pendingMutex);
        const auto itr = m_pendingSessions.find(realmJoinTicket);
        if (itr == m_pendingSessions.end())
            return false;

        if (itr->second.expiresAt < static_cast<uint64_t>(UNIXTIME))
        {
            m_pendingSessions.erase(itr);
            return false;
        }

        session = itr->second;
        if (consume)
            m_pendingSessions.erase(itr);
        return true;
    }

    void BattleNetCommClientManager::cleanupExpiredSessions()
    {
        const uint64_t now = static_cast<uint64_t>(UNIXTIME);
        std::lock_guard lock(m_pendingMutex);
        std::erase_if(m_pendingSessions, [now](const auto& entry) { return entry.second.expiresAt < now; });
    }
}
