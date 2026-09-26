/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/Socket.hpp"
#include "BattleNetCommDefines.hpp"

#include <array>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class WorldPacket;

namespace AscEmu::Battlenet
{
    class BattleNetCommServerSocket;

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

    class BattleNetCommManager
    {
    public:
        static BattleNetCommManager& getInstance();

        void registerRealm(uint32_t realmId, AscEmu::BattlenetComm::RealmRuleset ruleset, BattleNetCommServerSocket* socket);
        void unregisterSocket(BattleNetCommServerSocket* socket);
        bool sendPendingSession(const PendingWorldSession& session);
        [[nodiscard]] AscEmu::BattlenetComm::RealmRuleset getRealmRuleset(uint32_t realmId) const;

    private:
        struct RegisteredRealm
        {
            BattleNetCommServerSocket* socket = nullptr;
            AscEmu::BattlenetComm::RealmRuleset ruleset = AscEmu::BattlenetComm::RealmRuleset::PvE;
        };

        mutable std::mutex m_mutex;
        std::unordered_map<uint32_t, RegisteredRealm> m_realms;
    };

    class BattleNetCommServerSocket final : public Socket
    {
    public:
        explicit BattleNetCommServerSocket(SOCKET fd);
        ~BattleNetCommServerSocket() override = default;

        void onConnect() override;
        void onRead() override;
        void onDisconnect() override;

        bool sendPacket(WorldPacket& packet);
        bool sendPendingSession(const PendingWorldSession& session);

    private:
        void handlePacket(WorldPacket& packet);
        void handleRegisterRealm(WorldPacket& packet);

        uint32_t m_remaining = 0;
        uint16_t m_opcode = 0;
        bool m_headerReady = false;
        uint32_t m_realmId = 0;
        bool m_authenticated = false;
    };

    inline BattleNetCommManager& sBattleNetCommManager = BattleNetCommManager::getInstance();
}
