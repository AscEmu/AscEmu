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
        // answers the first complete request in the buffer; handled tells whether one was there
        bool handleBufferedHttpRequest(bool& handled);
        std::string buildHttpResponse(const char* status, const std::string& body);
        bool handlePasswordLogin(const std::string& login, const std::string& password);
        bool sendLoginFormResponse();
        bool sendJsonResponse(const std::string& body, const char* status = "200 OK");
        // sends a http response over the transport of the connection (TLS or plain)
        bool writeResponse(const uint8_t* data, size_t size);
        bool writeTlsPlainText(const uint8_t* data, size_t size);
        void logTlsError(const char* operation, int result) const;
        void releaseTls();

        uint64_t m_connectionId = 0;
        SSL* m_ssl = nullptr;
        std::mutex m_tlsInitMutex;
        bool m_tlsInitAttempted = false;
        bool m_tlsHandshakeComplete = false;
        // the first byte decides: a TLS record (0x16) or a plain http request line
        bool m_transportDecided = false;
        bool m_plainHttp = false;
        // HTTP/1.1 keeps the connection for the following requests unless the client asks to close it
        bool m_keepAlive = true;
        // JSESSIONID of the connection: taken from the request cookie or created with the first request,
        // the client sends it back with the SRP challenge and proof
        std::string m_sessionId;
        std::string m_setCookie;
        std::string m_httpBuffer;
    };
}
