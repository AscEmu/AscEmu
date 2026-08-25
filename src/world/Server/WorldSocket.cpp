/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WorldSocket.hpp"

#include "DatabaseDefinition.hpp"
#include "Utilities/Util.hpp"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "World.h"
#include "Packets/SmsgPong.h"
#include "Packets/SmsgAuthChallenge.h"
#include "Packets/SmsgAuthResponse.h"
#include "OpcodeTable.hpp"
#include "WorldSession.h"
#include "Utilities/Random.hpp"
#include "Packets/CmsgAuthSession.h"

using namespace AscEmu::Packets;

#pragma pack(push, 1)
struct ClientPktHeader
{
    uint16_t size;
    uint32_t cmd;

    uint16_t mopCmd;

    static constexpr uint32_t MopOpcodeMask = 0x01FFF;
    static constexpr uint32_t MopMaxPayloadSize = 0x07FFF;

    static ClientPktHeader mopEncrypted(uint32_t raw)
    {
        ClientPktHeader pkt{};

        pkt.size = static_cast<uint16_t>((raw & ~MopOpcodeMask) >> 13);
        pkt.mopCmd = static_cast<uint16_t>(raw & MopOpcodeMask);
        pkt.cmd = pkt.dropMopHighBytes(pkt.mopCmd);

        return pkt;
    }

    static uint16_t dropMopHighBytes(uint16_t opcode)
    {
        return static_cast<uint16_t>(opcode & MopOpcodeMask);
    }

    bool isMopPayloadValid() const { return size <= MopMaxPayloadSize; }
    uint32_t getMopPayloadSize() const { return size; }
    uint16_t getMopOpcode() const { return static_cast<uint16_t>(cmd); }
    uint16_t getRawMopOpcode() const { return mopCmd; }
};

struct ServerPktHeader
{
    uint16_t legacySize{0};
    uint16_t legacyCmd{0};

    static ServerPktHeader legacy(uint16_t size, uint16_t cmd)
    {
        ServerPktHeader pkt;
        pkt.legacySize = size;
        pkt.legacyCmd = cmd;
        return pkt;
    }

    uint8_t cataHeader[6]{};
    uint8_t cataHeaderLength{0};

    static ServerPktHeader cataEncrypted(uint32_t size, uint16_t cmd)
    {
        ServerPktHeader pkt;

        if (size > 0x7FFF)
            pkt.cataHeader[pkt.cataHeaderLength++] = static_cast<uint8_t>(0x80 | ((size >> 16) & 0xFF));

        // Mop = low byte, high byte / Cata = high byte, low byte
        pkt.cataHeader[pkt.cataHeaderLength++] = static_cast<uint8_t>((size >> 8) & 0xFF);
        pkt.cataHeader[pkt.cataHeaderLength++] = static_cast<uint8_t>(size & 0xFF);

        pkt.cataHeader[pkt.cataHeaderLength++] = static_cast<uint8_t>(cmd & 0xFF);
        pkt.cataHeader[pkt.cataHeaderLength++] = static_cast<uint8_t>((cmd >> 8) & 0xFF);

        return pkt;
    }

    static constexpr uint32_t MopOpcodeMask = 0x01FFF;
    uint8_t mopHeader[4]{};
    uint32_t mopHeaderLength{0};

    static ServerPktHeader mopEncrypted(uint32_t size, uint32_t cmd)
    {
        ServerPktHeader pkt;

        const uint32_t raw = (size << 13) | (cmd & MopOpcodeMask);
        memcpy(pkt.mopHeader, &raw, 4);
        pkt.mopHeaderLength = 4;

        return pkt;
    }

    static ServerPktHeader mopUnencrypted(uint32_t size, uint32_t cmd)
    {
        ServerPktHeader header;

        memcpy(&header.mopHeader[0], &size, 2);
        memcpy(&header.mopHeader[2], &cmd, 2);

        header.mopHeaderLength = 4;
        return header;
    }

    const uint8_t* legacyData() const { return reinterpret_cast<const uint8_t*>(&legacySize); }
    const uint8_t* cataData() const { return cataHeader; }
    const uint8_t* mopData() const { return mopHeader; }
};
#pragma pack(pop)

static constexpr uint32_t WORLDSOCKET_SENDBUF_SIZE = 131078;
static constexpr uint32_t WORLDSOCKET_RECVBUF_SIZE = 16384;

enum OutPacketResult : uint8_t
{
    OUTPACKET_RESULT_SUCCESS = 1,
    OUTPACKET_RESULT_NO_ROOM_IN_BUFFER = 2,
    OUTPACKET_RESULT_NOT_CONNECTED = 3,
    OUTPACKET_RESULT_SOCKET_ERROR = 4,
};

WorldSocket::WorldSocket(SOCKET fd)
    :
    Socket(fd, WORLDSOCKET_SENDBUF_SIZE, WORLDSOCKET_RECVBUF_SIZE),
    m_socketSeed(Util::getRandomUInt(RAND_MAX))
{
}

WorldSocket::~WorldSocket()
{
    while (auto pck = m_queue.tryPop())
    {
    }

    if (m_session)
    {
        m_session->SetSocket(nullptr);
        m_session = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// virtual functions (Socket)
void WorldSocket::onRead()
{
    for (;;)
    {
        if (m_remaining == 0 && !processHeader())
            return;

        if (m_remaining > 0 && readBuffer.GetSize() < m_remaining)
            return;

        auto packet = std::make_unique<WorldPacket>(sOpcodeTables.getHexValueForVersionId(m_opcode), m_size);
        packet->resize(m_size);

        if (m_remaining > 0)
            readBuffer.Read(packet->contents(), m_remaining);

        sWorldPacketLog.logPacket(m_size, static_cast<uint16_t>(m_opcode), m_size ? packet->contents() : nullptr, 0, (m_session ? m_session->GetAccountId() : 0));

        m_remaining = m_size = 0;

        dispatchPacket(std::move(packet));
    }
}

void WorldSocket::onConnect()
{
    sWorld.increaseAcceptedConnections();
    m_latency = Util::getMSTime();

    if (m_protocolSetByLogonComm)
    {
        // If the client reached us over loopback, it's on the same host as this world server.
        // If the logon server is also local, that's exactly what it would have recorded at auth
        // time too, so loopback is still the correct lookup key - leave it alone. But if the logon
        // server lives on a different host, it only ever sees the client's real network address, so
        // asking it about "127.0.0.1"/"::1" would never match. In that case substitute our own
        // configured realm address instead - that's the same address the client used to reach the
        // logon server to get to us in the first place.
        std::string requestIp = getRemoteIp();
        if (requestIp == "127.0.0.1" || requestIp == "::1")
        {
            const std::string& logonAddress = worldConfig.logonServer.address;
            const bool logonIsLocal = logonAddress.empty() || logonAddress == "127.0.0.1" || logonAddress == "::1" || logonAddress == "localhost";

            if (!logonIsLocal)
            {
                std::string realmAddress = sLogonCommHandler.getRealmAddress();
                const auto portSeparator = realmAddress.find(':');
                if (portSeparator != std::string::npos)
                    realmAddress.resize(portSeparator);

                if (!realmAddress.empty() && realmAddress != "127.0.0.1" && realmAddress != "::1")
                {
                    sLogger.debug("WorldSocket::onConnect(): client connected via loopback and LogonServer ({}) is remote, using configured realm address {} instead for the build request.", logonAddress, realmAddress);
                    requestIp = realmAddress;
                }
            }
        }

        sLogger.debug("WorldSocket::onConnect(): sending build request for ip {}", requestIp);
        sLogonCommHandler.sendBuildRequest(requestIp, this);
    }
    else
    {
        setCurrentVersionAsProtocol();
        sendClientConnectionPacket();
    }
}

void WorldSocket::onDisconnect()
{
    if (!m_queue.hasItems())
        return;

    while (auto pck = m_queue.tryPop())
    {
    }

    if (m_session)
    {
        m_session->SetSocket(nullptr);
        m_session = nullptr;
    }

    if (m_requestId != 0)
    {
        sLogonCommHandler.removeUnauthedClientSocketClose(m_requestId);
        m_requestId = 0;
    }

    if (m_queued)
    {
        sWorld.removeQueuedSocket(this);
        m_queued = false;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// helper for protocol setup
void WorldSocket::setClientProtocol(WoW::ClientProtocol protocol)
{
    m_protocol = protocol;
}

WoW::ClientProtocol WorldSocket::getClientProtocol()
{
    return m_protocol;
}

void WorldSocket::setCurrentVersionAsProtocol()
{
    //Zyres: this is a fallback when the client version is not set by the client information
    WoW::ClientProtocol protocol;
    switch (sOpcodeTables.getVersionIdForAEVersion())
    {
        case 0: protocol.expansion = WoW::Expansion::_Classic; break;
        case 1: protocol.expansion = WoW::Expansion::_TBC; break;
        case 2: protocol.expansion = WoW::Expansion::_WotLK; break;
        case 3: protocol.expansion = WoW::Expansion::_Cata; break;
        case 4: protocol.expansion = WoW::Expansion::_Mop; break;

        default: protocol.expansion = WoW::Expansion::Unknown; break;
    }

    setClientProtocol(protocol);
}

void WorldSocket::setClientProtocolByBuild(uint32_t build)
{
    WoW::ClientProtocol protocol;

    switch (build)
    {
        case 5875: protocol.expansion = WoW::Expansion::_Classic; break;
        case 8606: protocol.expansion = WoW::Expansion::_TBC; break;
        case 12340: protocol.expansion = WoW::Expansion::_WotLK; break;
        case 15595: protocol.expansion = WoW::Expansion::_Cata; break;
        case 18414: protocol.expansion = WoW::Expansion::_Mop; break;

        default: protocol.expansion = WoW::Expansion::Unknown; break;
    }

    setClientProtocol(protocol);
}

//////////////////////////////////////////////////////////////////////////////////////////
// packet sending SERVER->CLIENT
void WorldSocket::outPacket(uint32_t opcode, size_t len, const void* data)
{
    if ((len + 10) > WORLDSOCKET_SENDBUF_SIZE)
    {
        sLogger.failure("WARNING: Tried to send a packet of {} bytes (which is too large) to a socket. Opcode was: {} (0x{:03X})",
            static_cast<unsigned int>(len), static_cast<unsigned int>(opcode), static_cast<unsigned int>(opcode));
        return;
    }

    uint8_t res = _outPacket(opcode, len, data);
    if (res == OUTPACKET_RESULT_SUCCESS)
        return;

    if (res == OUTPACKET_RESULT_NO_ROOM_IN_BUFFER)
    {
        auto packet = std::make_unique<WorldPacket>(opcode, len);
        if (len)
            packet->append(static_cast<const uint8_t*>(data), len);

        m_queue.push(std::move(packet));
    }
}

uint8_t WorldSocket::_outPacket(uint32_t opcode, size_t len, const void* data)
{
    if (!isConnected())
        return OUTPACKET_RESULT_NOT_CONNECTED;

    burstBegin();

    if (writeBuffer.GetSpace() < (len + 4))
    {
        burstEnd();
        return OUTPACKET_RESULT_NO_ROOM_IN_BUFFER;
    }

    sWorldPacketLog.logPacket(static_cast<uint32_t>(len), static_cast<uint16_t>(opcode),
        static_cast<const uint8_t*>(data), 1, (m_session ? m_session->GetAccountId() : 0));

    const auto version = m_protocol.expansion;
    const bool isClassic = version == WoW::Expansion::_Classic;
    const bool isTbc = version == WoW::Expansion::_TBC;
    const bool isWotlk = version == WoW::Expansion::_WotLK;
    const bool isCata = version == WoW::Expansion::_Cata;
    const bool isMop = version == WoW::Expansion::_Mop;
    const bool isLegacy = isClassic || isTbc;

    bool rv = false;

    if (isMop)
    {
        if (m_crypt.isInitialized())
        {
            ServerPktHeader header = ServerPktHeader::mopEncrypted(static_cast<uint32_t>(len),
                static_cast<uint32_t>(sOpcodeTables.getHexValueForVersionId(opcode)));

            m_crypt.encryptWotlkSend(header.mopHeader, header.mopHeaderLength);
            rv = burstSend(header.mopData(), header.mopHeaderLength);
        }
        else
        {
            ServerPktHeader header = ServerPktHeader::mopUnencrypted(static_cast<uint32_t>(len + 2),
                static_cast<uint32_t>(sOpcodeTables.getHexValueForVersionId(opcode)));

            rv = burstSend(header.mopData(), header.mopHeaderLength);
        }
    }
    else if (isCata)
    {
        ServerPktHeader header = ServerPktHeader::cataEncrypted(static_cast<uint32_t>(len + 2),
            static_cast<uint16_t>(sOpcodeTables.getHexValueForVersionId(opcode)));

        m_crypt.encryptWotlkSend(header.cataHeader, header.cataHeaderLength);
        rv = burstSend(header.cataData(), header.cataHeaderLength);
    }
    else
    {
        ServerPktHeader header = ServerPktHeader::legacy(ntohs(static_cast<uint16_t>(len + 2)),
            static_cast<uint16_t>(sOpcodeTables.getHexValueForVersionId(opcode)));

        if (m_crypt.isInitialized())
        {
            if (isLegacy)
                m_crypt.encryptLegacySend(reinterpret_cast<uint8_t*>(&header.legacySize), 4);
            else if (isWotlk)
                m_crypt.encryptWotlkSend(reinterpret_cast<uint8_t*>(&header.legacySize), 4);
        }

        rv = burstSend(header.legacyData(), 4);
    }

    if (len > 0 && rv)
        rv = burstSend(static_cast<const uint8_t*>(data), static_cast<uint32_t>(len));

    if (rv)
        burstPush();

    burstEnd();
    return rv ? OUTPACKET_RESULT_SUCCESS : OUTPACKET_RESULT_SOCKET_ERROR;
}

void WorldSocket::updateQueuedPackets()
{
    if (!m_queue.hasItems())
        return;

    while (auto itr = m_queue.tryPop())
    {
        const auto& pck = itr.value();

        switch (_outPacket(pck->getOpcode(), pck->size(), pck->size() ? pck->contents() : nullptr))
        {
            case OUTPACKET_RESULT_SUCCESS:
                break;

            case OUTPACKET_RESULT_NO_ROOM_IN_BUFFER:
                return;

            // pop remaining packets
            default:
                {
                    while (auto remainingPacket = m_queue.tryPop())
                    {
                    }
                    return;
                }
        }
    }
}

void WorldSocket::sendPacket(WorldPacket* packet)
{
    if (!packet)
        return;

    outPacket(packet->getOpcode(), packet->size(), (packet->size() ? (const void*)packet->contents() : nullptr));
}

void WorldSocket::sendUpdateQueuePosition(uint32_t Position)
{
    SmsgAuthResponse response(0, ARST_QUEUE, Position);
    sendManagedPacket(response);
}

void WorldSocket::sendAuthenticated(std::unique_ptr<WorldSession> sessionHolder)
{
    m_queued = false;

    if (m_session == nullptr || sessionHolder == nullptr)
        return;

    SmsgAuthResponse response(AuthOkay, ARST_ACCOUNT_DATA);
    sendManagedPacket(response);

    m_session->sendAddonInfo();

    if (m_protocol.expansion > WoW::Expansion::_TBC)
        m_session->sendClientCacheVersion(BUILD_VERSION);

    m_session->_latency = m_latency;

    sWorld.addGlobalSession(m_session);
    sWorld.addSession(std::move(sessionHolder));
}

void WorldSocket::sendClientConnectionPacket()
{
    if (m_protocol.expansion <= WoW::Expansion::_WotLK)
        sendAuthChallengePacket();
    else
        sendVerifyConnectPacket();
}

void WorldSocket::sendAuthChallengePacket()
{
    SmsgAuthChallenge challengePacket(m_socketSeed);
    sendManagedPacket(challengePacket);
}

void WorldSocket::sendVerifyConnectPacket()
{
    const std::string handshake = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT";
    uint16_t handshaleLength = static_cast<uint16_t>(handshake.length());
    uint8_t sizeBytes[2];

    if (m_protocol.expansion == WoW::Expansion::_Mop)
    {
        sizeBytes[0] = handshaleLength & 0xFF;
        sizeBytes[1] = (handshaleLength >> 8) & 0xFF;
    }
    else // Cata
    {
        sizeBytes[0] = (handshaleLength >> 8) & 0xFF;
        sizeBytes[1] = handshaleLength & 0xFF;
    }

    burstBegin();
    burstSend(sizeBytes, 2);
    burstSend(reinterpret_cast<const uint8_t*>(handshake.c_str()), static_cast<uint32_t>(handshake.length()));
    burstPush();
    burstEnd();
}

//////////////////////////////////////////////////////////////////////////////////////////
// packet receiving CLIENT->SERVER (after onRead from Socket class)
bool WorldSocket::processHeader()
{
    const auto version = m_protocol.expansion;
    const bool isClassic = version == WoW::Expansion::_Classic;
    const bool isTBC = version == WoW::Expansion::_TBC;
    const bool legacyDecrypt = isClassic || isTBC;
    const bool isWotLK = version == WoW::Expansion::_WotLK;
    const bool isCata = version == WoW::Expansion::_Cata;
    const bool isMop = version == WoW::Expansion::_Mop;
    const bool isHandshakeRequired = isCata || isMop;

    if (isHandshakeRequired && !m_handshakeReceived)
    {
        if (readBuffer.GetSize() < 2)
            return false;

        sLogger.debug("WorldSocket::processHeader(): Received handshake size: {}", readBuffer.GetSize());

        uint8_t header[2];
        readBuffer.Read(header, 2);
        uint16_t size = 0;

        if (isCata)
            size = (static_cast<uint16_t>(header[0]) << 8) | static_cast<uint16_t>(header[1]);
        if (isMop)
            size = static_cast<uint16_t>(header[0]) | (static_cast<uint16_t>(header[1]) << 8);

        m_remaining = m_size = size;
        m_opcode = MSG_VERIFY_CONNECTIVITY;
        m_handshakeReceived = true;

        sLogger.debug("WorldSocket::processHeader(): Received handshake header. Raw: {:02X} {:02X}, size: {}",
            header[0], header[1], m_size);

        return true;
    }

    if (isMop && m_crypt.isInitialized())
    {
        if (readBuffer.GetSize() < 4)
            return false;

        uint32_t rawHeader = 0;
        readBuffer.Read(reinterpret_cast<uint8_t*>(&rawHeader), 4);
        m_crypt.decryptWotlkReceive(reinterpret_cast<uint8_t*>(&rawHeader), 4);

        const ClientPktHeader header = ClientPktHeader::mopEncrypted(rawHeader);

        if (!header.isMopPayloadValid())
        {
            sLogger.failure("WorldSocket::processHeader(): Received broken MoP packet from client. Size: {}, Opcode: {}",
                header.getMopPayloadSize(), header.getRawMopOpcode());

            disconnect();
            return false;
        }

        m_remaining = m_size = header.getMopPayloadSize();
        m_opcode = sOpcodeTables.getInternalIdForHex(header.getMopOpcode());

        const auto opcodeState = sOpcodeTables.getStateForInternalId(m_opcode);

        if (opcodeState == OpcodeDevelopmentState::NotUsed)
        {
            m_opcode = MSG_NULL_ACTION;
            return true;
        }

        if (m_opcode == 0)
            sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "WorldSocket::processHeader(): Unmapped MoP opcode 0x{:04X} (payload size {}) - falling back to MSG_NULL_ACTION.",
                header.getMopOpcode(), header.getMopPayloadSize());

        return true;
    }

    if (readBuffer.GetSize() < 6)
        return false;

    ClientPktHeader header;
    readBuffer.Read(&header, 6);

    if (legacyDecrypt)
    {
        m_crypt.decryptLegacyReceive(reinterpret_cast<uint8_t*>(&header), 6);
        m_remaining = m_size = ntohs(header.size) - 4;
    }
    else if (isCata || isWotLK)
    {
        m_crypt.decryptWotlkReceive(reinterpret_cast<uint8_t*>(&header), 6);
        m_remaining = m_size = ntohs(header.size) - 4;
    }
    else
    {
        m_crypt.decryptWotlkReceive(reinterpret_cast<uint8_t*>(&header), 8);
        m_remaining = m_size = header.size - 4;
    }

    m_opcode = sOpcodeTables.getInternalIdForHex(static_cast<uint16_t>(header.cmd));
    return true;
}

void WorldSocket::dispatchPacket(std::unique_ptr<WorldPacket> packet)
{
    switch (sOpcodeTables.getInternalIdForHex(packet->getOpcode()))
    {
        case CMSG_PING:
            handlePing(std::move(packet));
            break;
        case CMSG_AUTH_SESSION:
            handleAuthSession(std::move(packet));
            break;
        case MSG_VERIFY_CONNECTIVITY:
            handleMsgVerifyConnection(std::move(packet));
            break;
        default:
        {
            if (m_session)
                m_session->QueuePacket(std::move(packet));
        } break;
    }
}

void WorldSocket::handlePing(std::unique_ptr<WorldPacket> recvPacket)
{
    uint32_t ping;
    if (recvPacket->size() < 4)
    {
        sLogger.failure("WorldSocket::handlePing: Socket closed due to incomplete ping packet.");
        disconnect();
        return;
    }

    if (m_protocol.expansion <= WoW::Expansion::_Cata)
    {
        *recvPacket >> ping;
        *recvPacket >> m_latency;
    }
    else
    {
        *recvPacket >> m_latency;
        *recvPacket >> ping;
    }

    if (m_session)
    {
        m_session->_latency = m_latency;
        m_session->m_lastPing = static_cast<uint32_t>(UNIXTIME);
        m_session->m_clientTimeDelay = 0;
    }

    sendPacket(SmsgPong(ping).serialise().get());

#ifdef WIN32
    // Dynamically change nagle buffering status based on latency.
    //if (_latency >= 250)
    // I think 350 is better, in a MMO 350 latency isn't that big that we need to worry about reducing the number of packets being sent.
    if (m_latency >= 350)
    {
        if (!m_nagleEanbled)
        {
            u_long arg = 0;
            setsockopt(getFd(), 0x6, 0x1, reinterpret_cast<const char*>(&arg), sizeof(arg));
            m_nagleEanbled = true;
        }
    }
    else
    {
        if (m_nagleEanbled)
        {
            u_long arg = 1;
            setsockopt(getFd(), 0x6, 0x1, reinterpret_cast<const char*>(&arg), sizeof(arg));
            m_nagleEanbled = false;
        }
    }
#endif
}

void WorldSocket::handleAuthSession(std::unique_ptr<WorldPacket> recvPacket)
{
    CmsgAuthSession authSession;
    if (!parsePacket(*recvPacket, authSession))
    {
        sLogger.failure("WorldSocket::handleAuthSession: Error {}", authSession.errorMsg);

        disconnect();
        return;
    }

    m_latency = Util::getMSTime() - m_latency;

    m_accountName = authSession.accountName;
    std::copy(authSession.authDigest, authSession.authDigest + 20, m_authDigest);
    m_clientSeed = authSession.clientSeed;
    m_clientBuild = authSession.clientBuild;
    m_addonInfoBuffer = std::move(authSession.addonInfoBuffer);

    // Send out a request for this account.
    m_requestId = sLogonCommHandler.clientConnectionId(m_accountName, this);

    if (m_requestId == 0xFFFFFFFF)
    {
        disconnect();
        return;
    }
}

void WorldSocket::handleMsgVerifyConnection(std::unique_ptr<WorldPacket> recvPacket)
{
    std::string ClientToServerMsg;
    *recvPacket >> ClientToServerMsg;

    sLogger.debugOpcode("WorldSocket::handleMsgVerifyConnection Received client Message: {}.", ClientToServerMsg);

    m_handshakeReceived = true;

    sendAuthChallengePacket();
}

void WorldSocket::informationRetreiveCallback(WorldPacket& recvData, uint32_t requestid)
{
    if (requestid != m_requestId)
        return;

    uint32_t error;
    recvData >> error;

    if (error != 0)
    {
        SmsgAuthResponse response(AuthFailed, ARST_ONLY_ERROR);
        sendManagedPacket(response);
        return;
    }

    // Extract account information from the packet.
    std::string AccountName;
    uint32_t AccountID;
    std::string GMFlags;
    uint8_t AccountFlags;
    uint8_t K[40];
    std::string lang;
    uint32_t muted;

    recvData >> AccountID;
    recvData >> AccountName;
    recvData >> GMFlags;
    recvData >> AccountFlags;
    recvData.read(K, 40);
    recvData >> lang;
    recvData >> muted;

    sLogger.debug("WorldSocket::informationRetreiveCallback: Account: '{}', ID: {}, GMFlags: '{}', Flags: {}",
        AccountName, AccountID, GMFlags, AccountFlags);

    std::string forcedPermissions = sLogonCommHandler.getPermissionStringForAccountId(AccountID);
    if (!forcedPermissions.empty())
        GMFlags = forcedPermissions;

    sLogger.debug("WorldSocket::informationRetreiveCallback: got information packet from logon: `{}` ID {} (request {})",
        AccountName, AccountID, m_requestId);

    m_requestId = 0;

    const std::string& authAccountName = m_accountName.empty() ? AccountName : m_accountName;

    m_crypt.initForClientVersion(static_cast<uint8_t>(m_protocol.expansion), K);
    if (!m_crypt.isInitialized())
    {
        SmsgAuthResponse response(AuthFailed, ARST_ONLY_ERROR);
        sendManagedPacket(response);
        return;
    }

    if (!m_crypt.verifyWorldAuthDigest(static_cast<uint8_t>(m_protocol.expansion), authAccountName,
        m_clientSeed, m_socketSeed, K, m_authDigest))
    {
        SmsgAuthResponse response(AuthUnknownAccount, ARST_ONLY_ERROR);
        sendManagedPacket(response);
        return;
    }

    // disconnect current player and login this one(blizzlike)
    WorldSession* oldSession = sWorld.getSessionByAccountId(AccountID);
    if (oldSession)
    {
        if (oldSession->GetSocket() == nullptr)
        {
            sLogger.info("WorldSocket::informationRetreiveCallback: reconnect session for account {} (ID: {})", AccountName, AccountID);
            if (oldSession->GetPlayer() && !oldSession->IsLoggingOut())
                oldSession->LogoutPlayer(true);

            sWorld.deleteSession(oldSession);
        }
        else
        {
            sLogger.info("WorldSocket::informationRetreiveCallback: account {} (ID: {}) already connected from {}; rejecting new connection",
                AccountName, AccountID, oldSession->GetSocket() ? oldSession->GetSocket()->getRemoteIp().c_str() : "unknown");
            
            oldSession->Disconnect();
            oldSession->SetLogoutTimer(1);

            SmsgAuthResponse response(AuthUnknownAccount, ARST_ONLY_ERROR);
            sendManagedPacket(response);
            return;
        }
    }

    auto pSession = std::make_unique<WorldSession>(AccountID, AccountName, this);

    m_session = pSession.get();

    std::lock_guard guard(pSession->deleteMutex);

    pSession->SetClientBuild(static_cast<uint16_t>(m_clientBuild));
    pSession->readAddonInfoPacket(m_addonInfoBuffer);
    pSession->LoadSecurity(GMFlags);
    pSession->SetAccountFlags(AccountFlags);
    pSession->m_lastPing = static_cast<uint32_t>(UNIXTIME);
    pSession->language = Util::getLanguagesIdFromString(lang);
    pSession->m_muted = muted;

    for (uint8_t i = 0; i < 8; ++i)
        pSession->SetAccountData(i, nullptr, true, 0);

    if (worldConfig.server.useAccountData)
    {
        auto pResult = CharacterDatabase.query("SELECT * FROM account_data WHERE acct = %u", AccountID);
        if (pResult == nullptr)
        {
            CharacterDatabase.execute("INSERT INTO account_data VALUES(%u, '', '', '', '', '', '', '', '', '')", AccountID);
        }
        else
        {
            Field* fields = pResult->fetch();
            for (uint8_t i = 0; i < 8; ++i)
            {
                const char* data = fields[1 + i].asCString();
                size_t len = data ? strlen(data) : 0;
                if (len > 1)
                {
                    auto d = std::make_unique<char[]>(len + 1);
                    memcpy(d.get(), data, len + 1);
                    pSession->SetAccountData(i, std::move(d), true, static_cast<uint32_t>(len));
                }
            }
        }
    }

    sLogger.debug("WorldSocket::informationRetreiveCallback: {} from {}:{} [{}ms]", AccountName, getRemoteIp(), getRemotePort(), m_latency);

    uint32_t playerLimit = worldConfig.getPlayerLimit();
    if ((sWorld.getSessionCount() < playerLimit) || pSession->HasGMPermissions())
    {
        sendAuthenticated(std::move(pSession));
    }
    else if (playerLimit > 0)
    {
        uint32_t Position = sWorld.addQueuedSocket(this, std::move(pSession));
        m_queued = true;
        sLogger.debug("WorldSocket::informationRetreiveCallback: {} added to queue in position {}", AccountName, Position);

        sendUpdateQueuePosition(Position);
    }
    else
    {
        SmsgAuthResponse response(AuthRejected, ARST_ONLY_ERROR);
        sendManagedPacket(response);
        disconnect();
    }
}

void WorldPacketLog::logPacket(uint32_t len, uint16_t opcode, const uint8_t* data, uint8_t direction, uint32_t accountid)
{
    switch (opcode)
    {
        //stop spaming opcodes here
        case SMSG_MONSTER_MOVE:
        case MSG_MOVE_HEARTBEAT:
        case SMSG_ATTACKERSTATEUPDATE:
        case SMSG_EMOTE:
        case SMSG_TIME_SYNC_REQUEST:
        case CMSG_TIME_SYNC_RESPONSE:
        case SMSG_PONG:
        case CMSG_PING:
        case SMSG_UPDATE_OBJECT:
            break;
        default:
        {
            sLogger.debugOpcode("[{}]: {} {} (0x{:03X}) of {} bytes.", direction ? "SERVER" : "CLIENT", direction ? "sent" : "received",
                sOpcodeTables.getNameForInternalId(opcode), sOpcodeTables.getHexValueForVersionId(opcode), len);
        } break;
    }

    if (isLogEnabled)
    {
        std::lock_guard lock(mPacketLogMutex);

        unsigned int line = 1;
        unsigned int countpos = 0;
        uint16_t lenght = static_cast<uint16_t>(len);

        fprintf(mPacketLogFile, "{%s} Packet: (0x%04X) %s PacketSize = %u stamp = %u accountid = %u\n", (direction ? "SERVER" : "CLIENT"), 
            sOpcodeTables.getHexValueForVersionId(opcode),
            sOpcodeTables.getNameForInternalId(opcode).c_str(), lenght, Util::getMSTime(), accountid);

        fprintf(mPacketLogFile, "|------------------------------------------------|----------------|\n");
        fprintf(mPacketLogFile, "|00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |0123456789ABCDEF|\n");
        fprintf(mPacketLogFile, "|------------------------------------------------|----------------|\n");

        if (lenght > 0)
        {
            fprintf(mPacketLogFile, "|");
            for (unsigned int count = 0; count < lenght; count++)
            {
                if (countpos == 16)
                {
                    countpos = 0;

                    fprintf(mPacketLogFile, "|");

                    for (unsigned int a = count - 16; a < count; a++)
                    {
                        if ((data[a] < 32) || (data[a] > 126))
                        {
                            fprintf(mPacketLogFile, ".");
                        }
                        else
                        {
                            fprintf(mPacketLogFile, "%c", data[a]);
                        }
                    }

                    fprintf(mPacketLogFile, "|\n");

                    line++;
                    fprintf(mPacketLogFile, "|");
                }

                fprintf(mPacketLogFile, "%02X ", data[count]);

                //FIX TO PARSE PACKETS WITH LENGTH < OR = TO 16 BYTES.
                if (count + 1 == lenght && lenght <= 16)
                {
                    for (unsigned int b = countpos + 1; b < 16; b++)
                    {
                        fprintf(mPacketLogFile, "   ");
                    }

                    fprintf(mPacketLogFile, "|");

                    for (unsigned int a = 0; a < lenght; a++)
                    {
                        if ((data[a] < 32) || (data[a] > 126))
                        {
                            fprintf(mPacketLogFile, ".");
                        }
                        else
                        {
                            fprintf(mPacketLogFile, "%c", data[a]);
                        }
                    }

                    for (unsigned int c = count; c < 15; c++)
                    {
                        fprintf(mPacketLogFile, " ");
                    }

                    fprintf(mPacketLogFile, "|\n");
                }

                //FIX TO PARSE THE LAST LINE OF THE PACKETS WHEN THE LENGTH IS > 16 AND ITS IN THE LAST LINE.
                if (count + 1 == lenght && lenght > 16)
                {
                    for (unsigned int b = countpos + 1; b < 16; b++)
                    {
                        fprintf(mPacketLogFile, "   ");
                    }

                    fprintf(mPacketLogFile, "|");

                    unsigned short _print = 0;

                    for (unsigned int a = line * 16 - 16; a < lenght; a++)
                    {
                        if ((data[a] < 32) || (data[a] > 126))
                        {
                            fprintf(mPacketLogFile, ".");
                        }
                        else
                        {
                            fprintf(mPacketLogFile, "%c", data[a]);
                        }

                        _print++;
                    }

                    for (unsigned int c = _print; c < 16; c++)
                    {
                        fprintf(mPacketLogFile, " ");
                    }

                    fprintf(mPacketLogFile, "|\n");
                }

                countpos++;
            }
        }

        fprintf(mPacketLogFile, "-------------------------------------------------------------------\n\n");
        fflush(mPacketLogFile);
    }
}
