/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WebAuthSocket.hpp"

#include "AEVersion.hpp"
#include "BNetAuthTicketStore.hpp"
#include "BNetConfig.hpp"
#include "BNetProtocol.hpp"
#include "BNetTlsContext.hpp"
#include "Master.hpp"
#include "Cryptography/BNetSrpV1.hpp"
#include "Database/Database.hpp"
#include "Logging/Logger.hpp"

#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <climits>
#include <cctype>
#include <chrono>
#include <ctime>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace AscEmu::Battlenet
{
    namespace
    {
        constexpr uint32_t WEB_AUTH_SOCKET_BUFFER_SIZE = 64 * 1024;
        constexpr size_t TLS_IO_BUFFER_SIZE = 16 * 1024;
        constexpr size_t MAX_HTTP_REQUEST_SIZE = 64 * 1024;

        std::atomic<uint64_t> nextWebAuthConnectionId{ 1 };

        // random session id in the 8-4-4-4-12 uuid notation
        std::string createSessionId()
        {
            std::array<uint8_t, 16> randomBytes{};
            if (RAND_bytes(randomBytes.data(), static_cast<int>(randomBytes.size())) != 1)
                return {};

            static constexpr char hexDigits[] = "0123456789abcdef";
            std::string id;
            id.reserve(36);
            for (size_t i = 0; i < randomBytes.size(); ++i)
            {
                if (i == 4 || i == 6 || i == 8 || i == 10)
                    id += '-';

                id += hexDigits[randomBytes[i] >> 4];
                id += hexDigits[randomBytes[i] & 0x0F];
            }

            return id;
        }

        // Date header value, e.g. "Sat, 10 Oct 2026 16:12:05 GMT"
        std::string httpDate()
        {
            const std::time_t now = std::time(nullptr);
            std::tm utc{};
#if defined(_WIN32)
            gmtime_s(&utc, &now);
#else
            gmtime_r(&now, &utc);
#endif
            char buffer[64]{};
            std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &utc);
            return buffer;
        }

        struct PendingSrpChallenge
        {
            AscEmu::Cryptography::BNetSrpV1::ServerChallenge challenge;
            std::chrono::steady_clock::time_point createdAt;
        };

        std::mutex pendingSrpMutex;
        std::unordered_map<std::string, PendingSrpChallenge> pendingSrpChallenges;

        std::string upperAscii(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) { return static_cast<char>(std::toupper(character)); });

            return value;
        }

        void prunePendingSrpChallenges()
        {
            const auto cutoff = std::chrono::steady_clock::now() - std::chrono::minutes(2);

            for (auto itr = pendingSrpChallenges.begin(); itr != pendingSrpChallenges.end();)
            {
                if (itr->second.createdAt < cutoff)
                    itr = pendingSrpChallenges.erase(itr);
                else
                    ++itr;
            }
        }

        void storePendingSrpChallenge(const std::string& login, const AscEmu::Cryptography::BNetSrpV1::ServerChallenge& challenge)
        {
            std::lock_guard<std::mutex> guard(pendingSrpMutex);
            prunePendingSrpChallenges();

            pendingSrpChallenges[upperAscii(login)] =
                PendingSrpChallenge{
                    challenge,
                    std::chrono::steady_clock::now()
                };
        }

        bool takePendingSrpChallenge(const std::string& login, AscEmu::Cryptography::BNetSrpV1::ServerChallenge& challenge)
        {
            std::lock_guard<std::mutex> guard(pendingSrpMutex);
            prunePendingSrpChallenges();

            const auto itr = pendingSrpChallenges.find(upperAscii(login));
            if (itr == pendingSrpChallenges.end())
                return false;

            challenge = std::move(itr->second.challenge);
            pendingSrpChallenges.erase(itr);
            return true;
        }

        std::string makeLoginTicket()
        {
            std::array<uint8_t, 20> randomBytes{};
            if (RAND_bytes(randomBytes.data(), static_cast<int>(randomBytes.size())) != 1)
                return {};

            static constexpr char digits[] = "0123456789ABCDEF";

            std::string ticket = "AE-";
            ticket.reserve(3 + randomBytes.size() * 2);

            for (uint8_t value : randomBytes)
            {
                ticket.push_back(digits[value >> 4]);
                ticket.push_back(digits[value & 0x0F]);
            }

            return ticket;
        }

        // position of the quote that closes the JSON string starting at _start, npos when it stays open
        size_t jsonStringEnd(const std::string& body, size_t start)
        {
            for (size_t i = start; i < body.size(); ++i)
            {
                if (body[i] == '\\')
                {
                    ++i;
                    continue;
                }

                if (body[i] == '"')
                    return i;
            }

            return std::string::npos;
        }

        // Range of the raw "value" text in the object of the login form whose "input_id" is inputId.
        // Clients write the two members in either order, so the whole object is read before it is judged.
        bool findLoginInputValue(const std::string& body, const std::string& inputId, size_t& valueStart, size_t& valueEnd)
        {
            bool idMatches = false;
            bool hasValue = false;
            std::string pendingKey;

            for (size_t i = 0; i < body.size(); ++i)
            {
                const char current = body[i];
                if (current == '{')
                {
                    idMatches = false;
                    hasValue = false;
                    pendingKey.clear();
                }
                else if (current == '}')
                {
                    if (idMatches && hasValue)
                        return true;

                    idMatches = false;
                    hasValue = false;
                    pendingKey.clear();
                }
                else if (current == ',')
                {
                    pendingKey.clear();
                }
                else if (current == '"')
                {
                    const size_t textStart = i + 1;
                    const size_t textEnd = jsonStringEnd(body, textStart);
                    if (textEnd == std::string::npos)
                        return false;

                    size_t next = textEnd + 1;
                    while (next < body.size() && (body[next] == ' ' || body[next] == '\t' || body[next] == '\r' || body[next] == '\n'))
                        ++next;

                    if (next < body.size() && body[next] == ':')
                    {
                        pendingKey = body.substr(textStart, textEnd - textStart);
                    }
                    else
                    {
                        if (pendingKey == "input_id")
                        {
                            idMatches = body.compare(textStart, textEnd - textStart, inputId) == 0;
                        }
                        else if (pendingKey == "value")
                        {
                            valueStart = textStart;
                            valueEnd = textEnd;
                            hasValue = true;
                        }

                        pendingKey.clear();
                    }

                    i = textEnd;
                }
            }

            return false;
        }

        std::string redactSensitiveLoginValues(std::string body)
        {
            size_t valueStart = 0;
            size_t valueEnd = 0;
            if (findLoginInputValue(body, "password", valueStart, valueEnd))
                body.replace(valueStart, valueEnd - valueStart, "<redacted>");

            return body;
        }

        std::string extractLoginInputValue(const std::string& body, const std::string& inputId)
        {
            size_t valueStart = 0;
            size_t valueEnd = 0;
            if (!findLoginInputValue(body, inputId, valueStart, valueEnd))
                return {};

            return body.substr(valueStart, valueEnd - valueStart);
        }

#if AE_BNET_PASSWORD_WEB_LOGIN
        // resolves the escapes a JSON string value may contain, \u sequences are kept as they are
        std::string unescapeJsonValue(const std::string& value)
        {
            std::string result;
            result.reserve(value.size());

            for (size_t i = 0; i < value.size(); ++i)
            {
                if (value[i] != '\\' || i + 1 >= value.size())
                {
                    result.push_back(value[i]);
                    continue;
                }

                const char escaped = value[++i];
                switch (escaped)
                {
                    case '"': result.push_back('"'); break;
                    case '\\': result.push_back('\\'); break;
                    case '/': result.push_back('/'); break;
                    case 'b': result.push_back('\b'); break;
                    case 'f': result.push_back('\f'); break;
                    case 'n': result.push_back('\n'); break;
                    case 'r': result.push_back('\r'); break;
                    case 't': result.push_back('\t'); break;
                    default:
                        result.push_back('\\');
                        result.push_back(escaped);
                        break;
                }
            }

            return result;
        }
#endif
    }

    WebAuthSocket::WebAuthSocket(SOCKET fd)
        : Socket(fd, WEB_AUTH_SOCKET_BUFFER_SIZE, WEB_AUTH_SOCKET_BUFFER_SIZE)
        , m_connectionId(nextWebAuthConnectionId.fetch_add(1, std::memory_order_relaxed))
    {
    }

    WebAuthSocket::~WebAuthSocket()
    {
        releaseTls();
    }

    void WebAuthSocket::onConnect()
    {
        m_httpBuffer.clear();

        sLogger.info("BNet WebAuth: connection #{} from {}:{}", m_connectionId, getRemoteIp(), getRemotePort());
    }

    bool WebAuthSocket::ensureTls()
    {
        std::lock_guard lock(m_tlsInitMutex);
        if (m_tlsInitAttempted)
            return m_ssl != nullptr;

        m_tlsInitAttempted = true;
        return initializeTls();
    }

    bool WebAuthSocket::initializeTls()
    {
        SSL_CTX* context = BNetTlsContext::getInstance().getContext();
        if (context == nullptr)
            return false;

        m_ssl = SSL_new(context);
        if (m_ssl == nullptr)
            return false;

        BIO* readBio = BIO_new(BIO_s_mem());
        BIO* writeBio = BIO_new(BIO_s_mem());
        if (readBio == nullptr || writeBio == nullptr)
        {
            if (readBio != nullptr)
                BIO_free(readBio);
            if (writeBio != nullptr)
                BIO_free(writeBio);

            releaseTls();
            return false;
        }

        SSL_set_bio(m_ssl, readBio, writeBio);
        SSL_set_accept_state(m_ssl);
        return true;
    }

    void WebAuthSocket::onRead()
    {
        const size_t available = readBuffer.GetSize();
        if (available == 0)
            return;

        std::vector<uint8_t> encryptedData(available);
        if (!readBuffer.Read(encryptedData.data(), encryptedData.size()))
        {
            sLogger.failure("BNet WebAuth: connection #{} failed to read {} byte(s)", m_connectionId, available);
            disconnect();
            return;
        }

        if (!m_transportDecided)
        {
            m_transportDecided = true;
            m_plainHttp = encryptedData[0] != 0x16;

            if (m_plainHttp)
                sLogger.info("BNet WebAuth: connection #{} plain http request", m_connectionId);
            else
                sLogger.info("BNet WebAuth: connection #{} TLS handshake started", m_connectionId);
        }

        if (m_plainHttp)
        {
            if (!processHttpData(encryptedData.data(), encryptedData.size()))
                disconnect();

            return;
        }

        if (!ensureTls())
        {
            sLogger.failure("BNet WebAuth: connection #{} could not initialize TLS", m_connectionId);
            disconnect();
            return;
        }

        BIO* readBio = SSL_get_rbio(m_ssl);
        if (readBio == nullptr)
        {
            disconnect();
            return;
        }

        const int written = BIO_write(readBio, encryptedData.data(), static_cast<int>(encryptedData.size()));

        if (written <= 0 || static_cast<size_t>(written) != encryptedData.size())
        {
            sLogger.failure("BNet WebAuth: connection #{} failed to pass TLS input to OpenSSL", m_connectionId);
            disconnect();
            return;
        }

        if (!processTls())
            disconnect();
    }

    bool WebAuthSocket::processTls()
    {
        if (!m_tlsHandshakeComplete)
        {
            const int result = SSL_do_handshake(m_ssl);
            if (!flushTlsOutput())
                return false;

            if (result == 1)
            {
                m_tlsHandshakeComplete = true;

                const char* serverName = SSL_get_servername(m_ssl, TLSEXT_NAMETYPE_host_name);

                sLogger.info("BNet WebAuth: connection #{} TLS handshake complete " "(version: {}, cipher: {}, SNI: '{}')", m_connectionId, SSL_get_version(m_ssl), SSL_get_cipher_name(m_ssl), serverName != nullptr ? serverName : "<none>");
            }
            else
            {
                const int error = SSL_get_error(m_ssl, result);
                if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
                    return true;

                logTlsError("handshake", result);
                return false;
            }
        }

        std::array<uint8_t, TLS_IO_BUFFER_SIZE> plainText{};

        while (m_tlsHandshakeComplete)
        {
            const int result = SSL_read(m_ssl, plainText.data(), static_cast<int>(plainText.size()));

            if (result > 0)
            {
                if (!processHttpData(plainText.data(), static_cast<size_t>(result)))
                    return false;

                continue;
            }

            const int error = SSL_get_error(m_ssl, result);
            if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
                break;

            if (error == SSL_ERROR_ZERO_RETURN)
                return false;

            logTlsError("read", result);
            return false;
        }

        return flushTlsOutput();
    }

    bool WebAuthSocket::processHttpData(const uint8_t* data, size_t size)
    {
        if (size == 0)
            return true;

        if (m_httpBuffer.size() + size > MAX_HTTP_REQUEST_SIZE)
        {
            sLogger.failure("BNet WebAuth: connection #{} HTTP request exceeded {} byte(s)", m_connectionId, MAX_HTTP_REQUEST_SIZE);
            return false;
        }

        m_httpBuffer.append(reinterpret_cast<const char*>(data), size);

        // a kept connection delivers the requests one after another
        bool handled = true;
        while (handled)
        {
            if (!handleBufferedHttpRequest(handled))
                return false;
        }

        return true;
    }

    bool WebAuthSocket::handleBufferedHttpRequest(bool& handled)
    {
        handled = false;

        const size_t headerEnd = m_httpBuffer.find("\r\n\r\n");
        if (headerEnd == std::string::npos)
            return true;

        const size_t headersSize = headerEnd + 4;
        const std::string headers = m_httpBuffer.substr(0, headersSize);

        size_t contentLength = 0;
        const std::string contentLengthName = "content-length:";
        const std::string connectionName = "connection:";
        const std::string hostName = "host:";
        const std::string cookieName = "cookie:";
        std::string connectionValue;
        std::string hostValue;
        std::string cookieValue;

        size_t lineStart = 0;
        while (lineStart < headerEnd)
        {
            const size_t lineEnd = m_httpBuffer.find("\r\n", lineStart);
            if (lineEnd == std::string::npos || lineEnd > headerEnd)
                break;

            std::string line = m_httpBuffer.substr(lineStart, lineEnd - lineStart);
            std::string lowerLine = line;
            std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), [](unsigned char value) { return static_cast<char>(std::tolower(value)); });

            if (lowerLine.rfind(contentLengthName, 0) == 0)
            {
                const std::string value = line.substr(contentLengthName.size());
                try
                {
                    contentLength = static_cast<size_t>(std::stoull(value));
                }
                catch (...)
                {
                    sLogger.failure("BNet WebAuth: connection #{} invalid Content-Length '{}',", m_connectionId, value);
                    return false;
                }
            }
            else if (lowerLine.rfind(connectionName, 0) == 0)
            {
                connectionValue = lowerLine.substr(connectionName.size());
            }
            else if (lowerLine.rfind(hostName, 0) == 0)
            {
                hostValue = line.substr(hostName.size());
            }
            else if (lowerLine.rfind(cookieName, 0) == 0)
            {
                cookieValue = line.substr(cookieName.size());
            }

            lineStart = lineEnd + 2;
        }

        if (headersSize + contentLength > MAX_HTTP_REQUEST_SIZE)
        {
            sLogger.failure("BNet WebAuth: connection #{} HTTP request requires {} byte(s), limit is {}", m_connectionId, headersSize + contentLength, MAX_HTTP_REQUEST_SIZE);
            return false;
        }

        // Do not answer as soon as the header is complete. The WoW client may
        // deliver the JSON body in a later TLS record.
        if (m_httpBuffer.size() < headersSize + contentLength)
            return true;

        const size_t requestLineEnd = m_httpBuffer.find("\r\n");
        if (requestLineEnd == std::string::npos)
            return false;

        const std::string requestLine = m_httpBuffer.substr(0, requestLineEnd);
        const std::string body = m_httpBuffer.substr(headersSize, contentLength);

        // the request is taken out of the buffer, the next one may already follow
        m_httpBuffer.erase(0, headersSize + contentLength);
        handled = true;

        m_keepAlive = requestLine.find(" HTTP/1.1") != std::string::npos;
        if (connectionValue.find("close") != std::string::npos)
            m_keepAlive = false;
        else if (connectionValue.find("keep-alive") != std::string::npos)
            m_keepAlive = true;

        // the session cookie: a request without one gets a new id with its response
        const size_t cookieStart = cookieValue.find("JSESSIONID=");
        if (cookieStart != std::string::npos)
        {
            const size_t valueStart = cookieStart + 11;
            const size_t valueEnd = cookieValue.find(';', valueStart);
            m_sessionId = cookieValue.substr(valueStart, valueEnd == std::string::npos ? std::string::npos : valueEnd - valueStart);
        }
        else if (m_sessionId.empty())
        {
            m_sessionId = createSessionId();

            std::string domain = hostValue;
            domain.erase(0, domain.find_first_not_of(" \t"));
            domain.erase(domain.find_last_not_of(" \t\r") + 1);
            if (const size_t portStart = domain.find(':'); portStart != std::string::npos)
                domain.erase(portStart);
            if (domain.empty())
                domain = bnetConfig.webAuth.externalAddress;

            m_setCookie = "JSESSIONID=" + m_sessionId + "; Path=/bnetserver; Domain=" + domain + "; Secure; HttpOnly; SameSite=None";
        }

        sLogger.debug("BNet WebAuth: connection #{} HTTP request:\n{}", m_connectionId, headers);

        if (!body.empty())
        {
            const std::string diagnosticBody = redactSensitiveLoginValues(body);

            sLogger.debug("BNet WebAuth: connection #{} HTTP body ({} byte(s)):\n{}", m_connectionId, body.size(), diagnosticBody);
        }

        if (requestLine.rfind("GET /bnetserver/login/ ", 0) == 0)
            return sendLoginFormResponse();

        if (requestLine.rfind("POST /bnetserver/login/ ", 0) == 0)
        {
            const std::string login = extractLoginInputValue(body, "account_name");
            const std::string useSrp = extractLoginInputValue(body, "use_srp");
            const std::string publicA = extractLoginInputValue(body, "public_A");
            const std::string clientM1 = extractLoginInputValue(body, "client_evidence_M1");

#if AE_BNET_PASSWORD_WEB_LOGIN
            // clients up to 9.x post the plain password over the TLS connection
            if (useSrp.empty())
                return handlePasswordLogin(login, unescapeJsonValue(extractLoginInputValue(body, "password")));
#endif

            if (login.empty() || useSrp != "true" || publicA.empty() || clientM1.empty())
            {
                sLogger.info("BNet WebAuth: connection #{} POST /bnetserver/login/ is not a complete SRP proof", m_connectionId);
                return sendJsonResponse("{\"authentication_state\":\"DONE\"}");
            }

            AscEmu::Cryptography::BNetSrpV1::ServerChallenge challenge;
            if (!takePendingSrpChallenge(login, challenge))
            {
                sLogger.info("BNet WebAuth: connection #{} has no pending SRP challenge for '{}'", m_connectionId, login);
                return sendJsonResponse("{\"authentication_state\":\"DONE\"}");
            }

            std::string serverM2;
            AscEmu::Cryptography::BNetSrpV1::EvidenceDiagnostics evidenceDiagnostics;
            if (!AscEmu::Cryptography::BNetSrpV1::verifyClientEvidence(challenge, publicA, clientM1, serverM2, &evidenceDiagnostics))
            {
                sLogger.info("BNet WebAuth: connection #{} SRP client evidence rejected for '{}'", m_connectionId, login);
                sLogger.debug("BNet WebAuth SRP diagnostic: client_M1={} expected_M1={}", clientM1, evidenceDiagnostics.expectedM1);
                sLogger.debug("BNet WebAuth SRP diagnostic variants: fixed128={} littleEvidence={} littleU={}", evidenceDiagnostics.fixedWidthEvidenceM1, evidenceDiagnostics.littleEndianEvidenceM1, evidenceDiagnostics.littleEndianUExpectedM1);
                return sendJsonResponse("{\"authentication_state\":\"DONE\"}");
            }

            const std::string loginTicket = makeLoginTicket();
            if (loginTicket.empty())
            {
                sLogger.failure("BNet WebAuth: connection #{} could not generate login ticket", m_connectionId);
                return sendJsonResponse("{}", "500 Internal Server Error");
            }

            storeWebAuthTicket(loginTicket, login);

            const std::string response =
                "{\"authentication_state\":\"DONE\","
                "\"login_ticket\":\"" + loginTicket + "\","
                "\"server_evidence_M2\":\"" + serverM2 + "\"}";

            sLogger.info("BNet WebAuth: connection #{} authentication successful for '{}'.", m_connectionId, login);

            return sendJsonResponse(response);
        }

        if (requestLine.rfind("POST /bnetserver/login/srp/ ", 0) == 0)
        {
            const std::string login = extractLoginInputValue(body, "account_name");
            if (login.empty())
            {
                sLogger.failure("BNet WebAuth: connection #{} SRP challenge request has no account_name", m_connectionId);
                return sendJsonResponse("{\"authentication_state\":\"DONE\",\"error_code\":\"UNKNOWN_ACCOUNT\"}");
            }

            if (!sBNetLogonSQL)
            {
                sLogger.failure("BNet WebAuth: connection #{} logon database is unavailable", m_connectionId);
                return sendJsonResponse("{}", "500 Internal Server Error");
            }

            const std::string escapedLogin = sBNetLogonSQL->escapeString(login);
            auto result = sBNetLogonSQL->query("SELECT email, srp_version, srp_salt, srp_verifier " "FROM battlenet_accounts WHERE UPPER(email) = UPPER('%s') LIMIT 1", escapedLogin.c_str());

            if (!result)
            {
                sLogger.info("BNet WebAuth: connection #{} SRP account '{}' not found", m_connectionId, login);
                return sendJsonResponse("{\"authentication_state\":\"DONE\",\"error_code\":\"UNKNOWN_ACCOUNT\"}");
            }

            Field* fields = result->fetch();
            const std::string accountEmail = fields[0].asCString() != nullptr ? fields[0].asCString() : "";
            const std::string srpIdentity = accountEmail;
            const uint8_t srpVersion = fields[1].asUint8(true);
            const std::string saltHex = fields[2].asCString() != nullptr ? fields[2].asCString() : "";
            const std::string verifierHex = fields[3].asCString() != nullptr ? fields[3].asCString() : "";

            if (srpVersion != 1 || saltHex.empty() || verifierHex.empty())
            {
                sLogger.info("BNet WebAuth: connection #{} account '{}' has no Battle.net SRP v1 credentials; " "reset the account password once after applying the Stage 13 SQL update", m_connectionId, accountEmail);
                return sendJsonResponse("{\"authentication_state\":\"DONE\",\"error_code\":\"SRP_CREDENTIALS_MISSING\"}");
            }

            AscEmu::Cryptography::BNetSrpV1::ServerChallenge challenge;
            if (!AscEmu::Cryptography::BNetSrpV1::createServerChallenge(srpIdentity, saltHex, verifierHex, challenge))
            {
                sLogger.failure("BNet WebAuth: connection #{} failed to build SRP v1 challenge for account '{}'", m_connectionId, accountEmail);
                return sendJsonResponse("{}", "500 Internal Server Error");
            }

            const std::string challengeJson =
                "{\"version\":" + std::to_string(challenge.version) +
                ",\"iterations\":" + std::to_string(challenge.iterations) +
                ",\"modulus\":\"" + challenge.modulus +
                "\",\"generator\":\"" + challenge.generator +
                "\",\"hash_function\":\"" + challenge.hashFunction +
                "\",\"username\":\"" + challenge.username +
                "\",\"salt\":\"" + challenge.salt +
                "\",\"public_B\":\"" + challenge.publicB + "\"}";

            storePendingSrpChallenge(login, challenge);

            sLogger.info("BNet WebAuth: connection #{} POST /bnetserver/login/srp/ -> " "SRP v1 challenge [legacy little-endian] for account '{}' using identity '{}' " "(username={}, salt={} byte(s), B={} byte(s)); ephemeral proof state cached", m_connectionId, accountEmail, srpIdentity, challenge.username, challenge.salt.size() / 2, challenge.publicB.size() / 2);

            return sendJsonResponse(challengeJson);
        }

        sLogger.failure("BNet WebAuth: connection #{} unsupported HTTP request '{}',", m_connectionId, requestLine);
        return sendJsonResponse("{}", "404 Not Found");
    }

    bool WebAuthSocket::handlePasswordLogin(const std::string& login, const std::string& password)
    {
        if (login.empty() || password.empty())
        {
            sLogger.info("BNet WebAuth: connection #{} POST /bnetserver/login/ without account name or password", m_connectionId);
            return sendJsonResponse("{\"authentication_state\":\"DONE\"}");
        }

        if (!sBNetLogonSQL)
        {
            sLogger.failure("BNet WebAuth: connection #{} logon database is unavailable", m_connectionId);
            return sendJsonResponse("{}", "500 Internal Server Error");
        }

        const std::string escapedLogin = sBNetLogonSQL->escapeString(login);
        auto result = sBNetLogonSQL->query("SELECT email, srp_version, srp_salt, srp_verifier FROM battlenet_accounts WHERE UPPER(email) = UPPER('%s') LIMIT 1", escapedLogin.c_str());
        if (!result)
        {
            sLogger.info("BNet WebAuth: connection #{} password login for unknown account '{}'", m_connectionId, login);
            return sendJsonResponse("{\"authentication_state\":\"DONE\"}");
        }

        Field* fields = result->fetch();
        const std::string accountEmail = fields[0].asCString() != nullptr ? fields[0].asCString() : "";
        const uint8_t srpVersion = fields[1].asUint8(true);
        const std::string saltHex = fields[2].asCString() != nullptr ? fields[2].asCString() : "";
        const std::string verifierHex = fields[3].asCString() != nullptr ? fields[3].asCString() : "";

        if (srpVersion != 1 || !AscEmu::Cryptography::BNetSrpV1::verifyPassword(accountEmail, password, saltHex, verifierHex))
        {
            sLogger.info("BNet WebAuth: connection #{} password rejected for '{}' (srp_version {})", m_connectionId, accountEmail, srpVersion);
            return sendJsonResponse("{\"authentication_state\":\"DONE\"}");
        }

        const std::string loginTicket = makeLoginTicket();
        if (loginTicket.empty())
        {
            sLogger.failure("BNet WebAuth: connection #{} could not generate login ticket", m_connectionId);
            return sendJsonResponse("{}", "500 Internal Server Error");
        }

        storeWebAuthTicket(loginTicket, accountEmail);

        sLogger.info("BNet WebAuth: connection #{} password authentication successful for '{}'.", m_connectionId, accountEmail);
        return sendJsonResponse("{\"authentication_state\":\"DONE\",\"login_ticket\":\"" + loginTicket + "\"}");
    }

    bool WebAuthSocket::sendLoginFormResponse()
    {
        // Matches the Battle.net WebAuth LoginForm input contract used by current WoW clients.
        // /bnetserver/login/ endpoint. Keep the original proto field names:
        // type, inputs, input_id, max_length and srp_url.
#if AE_BNET_PASSWORD_WEB_LOGIN
        // clients up to 9.x send the password with the form, the SRP url is left out
        const std::string body = std::string(
            "{"
                "\"type\":\"LOGIN_FORM\","
                "\"inputs\":["
                    "{"
                        "\"input_id\":\"account_name\","
                        "\"type\":\"text\","
                        "\"label\":\"E-mail\","
                        "\"max_length\":320"
                    "},"
                    "{"
                        "\"input_id\":\"password\","
                        "\"type\":\"password\","
                        "\"label\":\"Password\","
                        "\"max_length\":16"
                    "},"
                    "{"
                        "\"input_id\":\"log_in_submit\","
                        "\"type\":\"submit\","
                        "\"label\":\"Log In\""
                    "}"
                "]"
            "}");
#else
        const std::string body = std::string(
            "{"
                "\"type\":\"LOGIN_FORM\","
                "\"inputs\":["
                    "{"
                        "\"input_id\":\"account_name\","
                        "\"type\":\"text\","
                        "\"label\":\"E-mail\","
                        "\"max_length\":320"
                    "},"
                    "{"
                        "\"input_id\":\"password\","
                        "\"type\":\"password\","
                        "\"label\":\"Password\","
                        "\"max_length\":128"
                    "},"
                    "{"
                        "\"input_id\":\"log_in_submit\","
                        "\"type\":\"submit\","
                        "\"label\":\"Log In\""
                    "}"
                "],"
                "\"srp_url\":\"") + bnetConfig.webAuth.loginUrl(!m_plainHttp) + "srp/\""
            "}";
#endif

        const std::string response = buildHttpResponse("200 OK", body);

        sLogger.info("BNet WebAuth: connection #{} GET /bnetserver/login/ -> " "HTTP 200 LOGIN_FORM with SRP URL ({} byte(s))", m_connectionId, body.size());

        return writeResponse(reinterpret_cast<const uint8_t*>(response.data()), response.size());
    }

    bool WebAuthSocket::sendJsonResponse(const std::string& body, const char* status)
    {
        const std::string response = buildHttpResponse(status, body);

        return writeResponse(reinterpret_cast<const uint8_t*>(response.data()), response.size());
    }

    std::string WebAuthSocket::buildHttpResponse(const char* status, const std::string& body)
    {
        // the connection stays open for the next request unless the client asked to close it
        std::string response =
            "HTTP/1.1 " + std::string(status) + "\r\n"
            "Date: " + httpDate() + "\r\n"
            "Server: AscEmu\r\n"
            "Content-Type: application/json;charset=utf-8\r\n"
            "Cache-Control: no-store\r\n";

        if (!m_keepAlive)
            response += "Connection: close\r\n";

        // a new session id goes out once, with the response to the request that had none
        if (!m_setCookie.empty())
        {
            response += "Set-Cookie: " + m_setCookie + "\r\n";
            m_setCookie.clear();
        }

        response += "Content-Length: " + std::to_string(body.size()) + "\r\n";

        sLogger.debug("BNet WebAuth: connection #{} HTTP response:\n{}", m_connectionId, response);

        response += "\r\n" + body;
        return response;
    }

    bool WebAuthSocket::writeResponse(const uint8_t* data, size_t size)
    {
        if (m_plainHttp)
            return send(data, static_cast<uint32_t>(size));

        return writeTlsPlainText(data, size);
    }

    bool WebAuthSocket::writeTlsPlainText(const uint8_t* data, size_t size)
    {
        size_t offset = 0;

        while (offset < size)
        {
            const size_t remaining = size - offset;
            const int chunkSize = static_cast<int>(std::min(remaining, static_cast<size_t>(INT_MAX)));

            const int result = SSL_write(m_ssl, data + offset, chunkSize);
            if (result > 0)
            {
                offset += static_cast<size_t>(result);

                if (!flushTlsOutput())
                    return false;

                continue;
            }

            const int error = SSL_get_error(m_ssl, result);
            if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
            {
                if (!flushTlsOutput())
                    return false;

                continue;
            }

            logTlsError("write", result);
            return false;
        }

        return flushTlsOutput();
    }

    bool WebAuthSocket::flushTlsOutput()
    {
        BIO* writeBio = SSL_get_wbio(m_ssl);
        if (writeBio == nullptr)
            return false;

        std::array<uint8_t, TLS_IO_BUFFER_SIZE> encryptedData{};

        while (BIO_ctrl_pending(writeBio) > 0)
        {
            const int bytesRead = BIO_read(writeBio, encryptedData.data(), static_cast<int>(encryptedData.size()));

            if (bytesRead <= 0)
                return false;

            if (!send(encryptedData.data(), static_cast<uint32_t>(bytesRead)))
                return false;
        }

        return true;
    }

    void WebAuthSocket::logTlsError(const char* operation, int result) const
    {
        const int sslError = SSL_get_error(m_ssl, result);
        const unsigned long openSslError = ERR_get_error();

        if (openSslError == 0)
        {
            sLogger.failure("BNet WebAuth: connection #{} TLS {} failed (SSL error {})", m_connectionId, operation, sslError);
            return;
        }

        char errorBuffer[256]{};
        ERR_error_string_n(openSslError, errorBuffer, sizeof(errorBuffer));

        sLogger.failure("BNet WebAuth: connection #{} TLS {} failed (SSL error {}): {}", m_connectionId, operation, sslError, errorBuffer);
    }

    void WebAuthSocket::releaseTls()
    {
        if (m_ssl == nullptr)
            return;

        SSL_free(m_ssl);
        m_ssl = nullptr;
        m_tlsHandshakeComplete = false;
    }

    void WebAuthSocket::onDisconnect()
    {
        sLogger.info("BNet WebAuth: connection #{} disconnected", m_connectionId);

        releaseTls();
    }
}
