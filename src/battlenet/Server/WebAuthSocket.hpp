/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/Socket.hpp"

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>

struct ssl_st;
using SSL = ssl_st;

namespace AscEmu::Battlenet
{
    class WebAuthSocket final : public Socket
    {
    public:
        explicit WebAuthSocket(SOCKET fd);
        ~WebAuthSocket() override;

        void onConnect() override;
        void onRead() override;
        void onDisconnect() override;

    private:
        bool initializeTls();
        // creates the TLS session once; the first client data can arrive before onConnect ran
        bool ensureTls();
        bool processTls();
        bool flushTlsOutput();
        bool processHttpData(const uint8_t* data, size_t size);
        bool handlePasswordLogin(const std::string& login, const std::string& password);
        bool sendLoginFormResponse();
        bool sendJsonResponse(const std::string& body, const char* status = "200 OK");
        bool writeTlsPlainText(const uint8_t* data, size_t size);
        void logTlsError(const char* operation, int result) const;
        void releaseTls();

        uint64_t m_connectionId = 0;
        SSL* m_ssl = nullptr;
        std::mutex m_tlsInitMutex;
        bool m_tlsInitAttempted = false;
        bool m_tlsHandshakeComplete = false;
        bool m_responseSent = false;
        std::string m_httpBuffer;
    };
}
