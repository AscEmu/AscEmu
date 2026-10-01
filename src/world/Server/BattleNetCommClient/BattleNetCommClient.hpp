/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/Socket.hpp"

#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class WorldPacket;

namespace AscEmu::Threading
{
    class AEThread;
    class AEThreadPool;
}

namespace AscEmu::BattlenetComm
{
    struct PendingWorldSession
    {
        uint32_t accountId = 0;
        uint32_t gameAccountId = 0;
        uint32_t realmId = 0;
        uint32_t clientBuild = 0;
        uint32_t region = 0;
        uint64_t expiresAt = 0;
        std::string gameAccountName;
        std::string realmJoinTicket;
        std::array<uint8_t, 64> worldAuthKeyData{};
        std::array<uint8_t, 32> joinSecret{};
    };

    class BattleNetCommClientSocket final : public Socket
    {
    public:
        explicit BattleNetCommClientSocket(SOCKET fd);
        ~BattleNetCommClientSocket() override = default;

        void onConnect() override;
        void onRead() override;
        void onDisconnect() override;

        bool sendPacket(WorldPacket& packet);

    private:
        void handlePacket(WorldPacket& packet);
        void handleRegisterResult(WorldPacket& packet);
        void handlePendingWorldSession(WorldPacket& packet);

        uint32_t m_remaining = 0;
        uint16_t m_opcode = 0;
        bool m_headerReady = false;
    };

    class BattleNetCommClientManager
    {
    public:
        static BattleNetCommClientManager& getInstance();

        void start(AscEmu::Threading::AEThreadPool& threadPool);
        void finalize();
        void setSocket(BattleNetCommClientSocket* socket);
        void clearSocket(BattleNetCommClientSocket* socket);
        void storePendingSession(PendingWorldSession session);
        bool getPendingSession(const std::string& realmJoinTicket, PendingWorldSession& session, bool consume = false);

    private:
        void run(AscEmu::Threading::AEThread& thread);
        void cleanupExpiredSessions();

        std::atomic<bool> m_running{ false };
        std::mutex m_socketMutex;
        BattleNetCommClientSocket* m_socket = nullptr;
        std::mutex m_pendingMutex;
        std::unordered_map<std::string, PendingWorldSession> m_pendingSessions;
    };

    inline BattleNetCommClientManager& sBattleNetCommClient = BattleNetCommClientManager::getInstance();
}
