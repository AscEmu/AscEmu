/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/Socket.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct ssl_st;
using SSL = ssl_st;

namespace AscEmu::Battlenet
{
    class BNetSocket final : public Socket
    {
    public:
        explicit BNetSocket(SOCKET fd);
        ~BNetSocket() override;

        void onConnect() override;
        void onRead() override;
        void onDisconnect() override;

    private:
        bool initializeTls();
        bool processTls();
        bool flushTlsOutput();
        bool processPlainText(const uint8_t* data, size_t size);
        bool processRpcFrames();
        bool dispatchRpcRequest(uint32_t serviceHash, uint32_t methodId, uint32_t token, const uint8_t* payload, size_t payloadSize);
        void logUnhandledRpc(uint32_t serviceHash, uint32_t methodId, uint32_t token, const uint8_t* payload, size_t payloadSize) const;
        bool handleConnectionConnect(uint32_t token, const uint8_t* payload, size_t payloadSize);
        bool handleConnectionKeepAlive(uint32_t token, const uint8_t* payload, size_t payloadSize);
        bool handleConnectionRequestDisconnect(uint32_t token, const uint8_t* payload, size_t payloadSize);
        bool handleAuthenticationV2Logon(uint32_t token, const uint8_t* payload, size_t payloadSize);
        bool handleAuthenticationV2VerifyWebCredentials(uint32_t token, const uint8_t* payload, size_t payloadSize);
        bool sendExternalChallenge();
        bool sendLogonComplete(const std::string& login, const std::string& loginTicket);
        bool handleAccountServiceV2Request(uint32_t methodId, uint32_t token, const uint8_t* payload, size_t payloadSize);
        bool handleGameUtilitiesRequest(uint32_t methodId, uint32_t token, const uint8_t* payload, size_t payloadSize);
        bool handleGameUtilitiesGetAllValues(uint32_t token, const std::string& attributeName, const uint8_t* payload, size_t payloadSize);
        bool handleRealmListTicketRequest(uint32_t token, const std::string& commandName, const uint8_t* payload, size_t payloadSize);
        bool handleRealmListRequest(uint32_t token, const std::string& commandName);
        bool handleRealmJoinRequest(uint32_t token, const uint8_t* payload, size_t payloadSize);
        bool handleFetchBleepProxiesRequest(uint32_t token, const std::string& commandName);
        bool handleSuperDistrictListRequest(uint32_t token, const std::string& commandName);
        bool handleLastCharPlayedRequest(uint32_t token, const std::string& commandName);
        bool sendRpcRequest(uint32_t serviceHash, uint32_t methodId, uint32_t token, const std::vector<uint8_t>& payload);
        bool sendRpcResponse(uint32_t token, const std::vector<uint8_t>& payload);
        bool writeTlsPlainText(const uint8_t* data, size_t size);
        void logTlsError(const char* operation, int result) const;
        void releaseTls();

        uint64_t m_connectionId = 0;
        size_t m_receivedBytes = 0;
        size_t m_sentBytes = 0;
        std::chrono::steady_clock::time_point m_connectedAt{};

        SSL* m_ssl = nullptr;
        bool m_tlsHandshakeComplete = false;
        std::vector<uint8_t> m_plainTextBuffer;
        struct LinkedGameAccount
        {
            uint32_t id = 0;
            std::string name;
        };

        uint32_t m_battleNetAccountId = 0;
        uint32_t m_selectedGameAccountId = 0;
        uint32_t m_clientBuild = 0;
        std::string m_selectedGameAccountName;
        std::vector<LinkedGameAccount> m_linkedGameAccounts;
        std::array<uint8_t, 32> m_realmListClientSecret{};
        bool m_realmListClientSecretValid = false;

        // Request currently being dispatched. This lets generic response logging
        // print the semantic service/method name without duplicating logs in every handler.
        uint32_t m_currentRpcServiceHash = 0;
        uint32_t m_currentRpcMethodId = 0;

        // Set by ConnectionService::RequestDisconnect. processRpcFrames() first
        // retires the current frame, processTls() flushes any remaining TLS
        // output, and onRead() finally arms delayedDisconnect(). This prevents
        // onDisconnect()/releaseTls() from freeing m_ssl or clearing the parser
        // buffer while the current read/TLS stack is still using them.
        bool m_delayCloseAfterReadCallback = false;
    };
}
