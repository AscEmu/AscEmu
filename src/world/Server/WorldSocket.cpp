/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Class WorldSocket - Main network code functions, handles
// reading/writing of all packets.

#include "WorldSocket.h"

#include "DatabaseDefinition.hpp"
#include "Utilities/Util.hpp"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Cryptography/Sha1.hpp"
#include "World.h"
#include "Management/AddonMgr.h"
#include "Packets/SmsgPong.h"
#include "Packets/SmsgAuthChallenge.h"
#include "Packets/SmsgAuthResponse.h"
#include "OpcodeTable.hpp"
#include "WorldSession.h"
#include "Utilities/Random.hpp"

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

    bool isMopPayloadValid() const
    {
        return size <= MopMaxPayloadSize;
    }

    uint32_t getMopPayloadSize() const
    {
        return size;
    }

    uint16_t getMopOpcode() const
    {
        return static_cast<uint16_t>(cmd);
    }

    uint16_t getRawMopOpcode() const
    {
        return mopCmd;
    }
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

    const uint8_t* legacyData() const
    {
        return reinterpret_cast<const uint8_t*>(&legacySize);
    }

    const uint8_t* cataData() const
    {
        return cataHeader;
    }

    const uint8_t* mopData() const
    {
        return mopHeader;
    }
};

// MIT End
#pragma pack(pop)

WorldSocket::WorldSocket(SOCKET fd)
    :
    Socket(fd, WORLDSOCKET_SENDBUF_SIZE, WORLDSOCKET_RECVBUF_SIZE), AuthDigest{},
    mClientBuild(0),
    mOpcode(0),
    mRemaining(0),
    mSize(0),
    mSeed(Util::getRandomUInt(RAND_MAX)),
    mClientSeed(0),
    mRequestID(0),
    mSession(nullptr),
    pAuthenticationPacket(nullptr),
    _latency(0),
    mQueued(false),
    m_nagleEanbled(false),
    m_fullAccountName(nullptr)
{
    //todo Zyres: This is temp until we moved the supported version from makro to config
    WoW::ClientProtocolState protocol;
    switch (sOpcodeTables.getVersionIdForAEVersion())
    {
        case 0:
            protocol.expansion = WoW::Expansion::_Classic;
            break;
        case 1:
            protocol.expansion = WoW::Expansion::_TBC;
            break;
        case 2:
            protocol.expansion = WoW::Expansion::_WotLK;
            break;
        case 3:
            protocol.expansion = WoW::Expansion ::_Cata;
            break;
        case 4:
            protocol.expansion = WoW::Expansion::_Mop;
            break;
        default:
            protocol.expansion = WoW::Expansion::Unknown;
            break;
    }

    setClientProtocol(protocol);

    sLogger.debug("Processing client protokol for version {}", m_protocol.expansion);
}

WorldSocket::~WorldSocket()
{
    while (auto pck = _queue.tryPop())
    {
    }

    pAuthenticationPacket = nullptr;

    if (mSession)
    {
        mSession->SetSocket(nullptr);
        mSession = nullptr;
    }
}

void WorldSocket::onDisconnect()
{
    if (!_queue.hasItems())
        return;

    while (auto pck = _queue.tryPop())
    {
    }

    if (mSession)
    {
        mSession->SetSocket(nullptr);
        mSession = nullptr;
    }

    if (mRequestID != 0)
    {
        sLogonCommHandler.removeUnauthedClientSocketClose(mRequestID);
        mRequestID = 0;
    }

    if (mQueued)
    {
        sWorld.removeQueuedSocket(this);    // Remove from queued sockets.
        mQueued = false;
    }
}

void WorldSocket::OutPacket(uint32_t opcode, size_t len, const void* data)
{
    if ((len + 10) > WORLDSOCKET_SENDBUF_SIZE)
    {
        sLogger.failure("WARNING: Tried to send a packet of {} bytes (which is too large) to a socket. Opcode was: {} (0x{:03X})", static_cast<unsigned int>(len), static_cast<unsigned int>(opcode), static_cast<unsigned int>(opcode));
        return;
    }

    OUTPACKET_RESULT res = _OutPacket(opcode, len, data);
    if (res == OUTPACKET_RESULT_SUCCESS)
        return;

    if (res == OUTPACKET_RESULT_NO_ROOM_IN_BUFFER)
    {
        /* queue the packet */
        auto packet = std::make_unique<WorldPacket>(opcode, len);
        if (len)
            packet->append(static_cast<const uint8_t*>(data), len);

        _queue.push(std::move(packet));
    }
}

void WorldSocket::UpdateQueuedPackets()
{
    if (!_queue.hasItems())
        return;

    while (auto itr = _queue.tryPop())
    {
        const auto& pck = itr.value();
        // try to push out as many as you can
        switch (_OutPacket(pck->getOpcode(), pck->size(), pck->size() ? pck->contents() : nullptr))
        {
            case OUTPACKET_RESULT_SUCCESS:
                break;

            // still connected
            case OUTPACKET_RESULT_NO_ROOM_IN_BUFFER:
                return;

            // kill everything in the buffer
            default:
                {
                    while (auto remainingPacket = _queue.tryPop())
                    {
                    }
                    return;
                }
        }
    }
}

OUTPACKET_RESULT WorldSocket::_OutPacket(uint32_t opcode, size_t len, const void* data)
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
        static_cast<const uint8_t*>(data), 1, (mSession ? mSession->GetAccountId() : 0));

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
        if (_crypt.isInitialized())
        {
            ServerPktHeader header = ServerPktHeader::mopEncrypted(static_cast<uint32_t>(len),
                static_cast<uint32_t>(sOpcodeTables.getHexValueForVersionId(opcode)));

            _crypt.encryptWotlkSend(header.mopHeader, header.mopHeaderLength);
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

        _crypt.encryptWotlkSend(header.cataHeader, header.cataHeaderLength);
        rv = burstSend(header.cataData(), header.cataHeaderLength);
    }
    else
    {
        ServerPktHeader header = ServerPktHeader::legacy(ntohs(static_cast<uint16_t>(len + 2)),
            static_cast<uint16_t>(sOpcodeTables.getHexValueForVersionId(opcode)));

        if (isLegacy)
            _crypt.encryptLegacySend(reinterpret_cast<uint8_t*>(&header.legacySize), 4);
        else if (isWotlk)
            _crypt.encryptWotlkSend(reinterpret_cast<uint8_t*>(&header.legacySize), 4);

        rv = burstSend(header.legacyData(), 4);
    }

    if (len > 0 && rv)
        rv = burstSend(static_cast<const uint8_t*>(data), static_cast<uint32_t>(len));

    if (rv)
        burstPush();

    burstEnd();
    return rv ? OUTPACKET_RESULT_SUCCESS : OUTPACKET_RESULT_SOCKET_ERROR;
}

void WorldSocket::onConnect()
{
    sWorld.increaseAcceptedConnections();
    _latency = Util::getMSTime();

    if (m_protocol.expansion <= WoW::Expansion::_WotLK)
        sendAuthChallengePacket();
    else
        sendVerifyConnectPacket();
}

void WorldSocket::sendAuthChallengePacket()
{
    SendPacket(SmsgAuthChallenge(mSeed).serialise().get());
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

void WorldSocket::Authenticate(std::unique_ptr<WorldSession> sessionHolder)
{
    if (pAuthenticationPacket != nullptr)
    {
        mQueued = false;

        if (mSession == nullptr || sessionHolder == nullptr)
            return;

        SendPacket(SmsgAuthResponse(AuthOkay, ARST_ACCOUNT_DATA).serialise().get());

#if VERSION_STRING < Cata
        sAddonMgr.SendAddonInfoPacket(pAuthenticationPacket.get(), static_cast<uint32_t>(pAuthenticationPacket->rpos()), mSession);
#else
        mSession->sendAddonInfo();
#endif

#if VERSION_STRING > TBC
        mSession->sendClientCacheVersion(BUILD_VERSION);
#endif
        mSession->_latency = _latency;

        pAuthenticationPacket = nullptr;

        sWorld.addGlobalSession(mSession);
        sWorld.addSession(std::move(sessionHolder));
    }
    else
    {
        sLogger.failure("WorldSocket::Authenticate something tried to Authenticate but packet is invalid (nullptr)");
        SendPacket(SmsgAuthResponse(AuthRejected, ARST_ONLY_ERROR).serialise().get());
        disconnect();
    }
}

void WorldSocket::UpdateQueuePosition(uint32_t Position)
{
    SendPacket(SmsgAuthResponse(0, ARST_QUEUE, Position).serialise().get());
}

void WorldSocket::onRead()
{
    for (;;)
    {
        if (mRemaining == 0 && !processHeader())
        {
            return;
        }

        if (mRemaining > 0 && readBuffer.GetSize() < mRemaining)
        {
            return;
        }

        auto packet = std::make_unique<WorldPacket>(sOpcodeTables.getHexValueForVersionId(mOpcode), mSize);
        packet->resize(mSize);

        if (mRemaining > 0)
        {
            readBuffer.Read(packet->contents(), mRemaining);
        }

        sWorldPacketLog.logPacket(mSize, static_cast<uint16_t>(mOpcode), mSize ? packet->contents() : nullptr, 0, (mSession ? mSession->GetAccountId() : 0));

        mRemaining = mSize = 0;

        dispatchPacket(std::move(packet));
    }
}

bool WorldSocket::processHeader()
{
    const auto version = m_protocol.expansion;
    const bool isClassic = version == WoW::Expansion::_Classic;
    const bool isTBC = version == WoW::Expansion::_TBC;
    const bool legacyDecrypt = isClassic || isTBC;
    const bool isCata = version == WoW::Expansion::_Cata;
    const bool isMop = version == WoW::Expansion::_Mop;
    const bool isHandshakeRequired = isCata || isMop;

    if (isHandshakeRequired && !m_HandshakeReceived)
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

        mRemaining = mSize = size;
        mOpcode = MSG_VERIFY_CONNECTIVITY;
        m_HandshakeReceived = true;

        sLogger.debug("WorldSocket::processHeader(): Received handshake header. Raw: {:02X} {:02X}, size: {}",
            header[0], header[1], mSize);

        return true;
    }

    if (isMop && _crypt.isInitialized())
    {
        if (readBuffer.GetSize() < 4)
            return false;

        uint32_t rawHeader = 0;
        readBuffer.Read(reinterpret_cast<uint8_t*>(&rawHeader), 4);
        _crypt.decryptWotlkReceive(reinterpret_cast<uint8_t*>(&rawHeader), 4);

        const ClientPktHeader header = ClientPktHeader::mopEncrypted(rawHeader);

        if (!header.isMopPayloadValid())
        {
            sLogger.failure("WorldSocket::processHeader(): Received broken MoP packet from client. Size: {}, Opcode: {}",
                header.getMopPayloadSize(), header.getRawMopOpcode());

            disconnect();
            return false;
        }

        mRemaining = mSize = header.getMopPayloadSize();
        mOpcode = sOpcodeTables.getInternalIdForHex(header.getMopOpcode());

        return true;
    }

    if (readBuffer.GetSize() < 6)
        return false;

    ClientPktHeader header;
    readBuffer.Read(&header, 6);

    if (legacyDecrypt)
    {
        _crypt.decryptLegacyReceive(reinterpret_cast<uint8_t*>(&header), sizeof(ClientPktHeader));
        mRemaining = mSize = ntohs(header.size) - 4;
    }
    else
    {
        _crypt.decryptWotlkReceive(reinterpret_cast<uint8_t*>(&header), sizeof(ClientPktHeader));
        if (isMop)
            mRemaining = mSize = header.size - 4;
        else
            mRemaining = mSize = ntohs(header.size) - 4;
    }

    mOpcode = sOpcodeTables.getInternalIdForHex(static_cast<uint16_t>(header.cmd));
    return true;
}

void WorldSocket::dispatchPacket(std::unique_ptr<WorldPacket> packet)
{
    switch (sOpcodeTables.getInternalIdForHex(packet->getOpcode()))
    {
        case CMSG_PING:
            _handlePing(std::move(packet));
            break;
        case CMSG_AUTH_SESSION:
            _handleAuthSession(std::move(packet));
            break;
        case MSG_VERIFY_CONNECTIVITY:
            _handleMsgVerifyConnection(std::move(packet));
            break;
        default:
            if (mSession)
                mSession->QueuePacket(std::move(packet));
            break;
    }
}

void WorldSocket::_handlePing(std::unique_ptr<WorldPacket> recvPacket)
{
    uint32_t ping;
    if (recvPacket->size() < 4)
    {
        sLogger.failure("Socket closed due to incomplete ping packet.");
        disconnect();
        return;
    }

#if VERSION_STRING < Cata
    *recvPacket >> ping;
    *recvPacket >> _latency;
#else
    *recvPacket >> _latency;
    *recvPacket >> ping;
#endif

    if (mSession)
    {
        mSession->_latency = _latency;
        mSession->m_lastPing = static_cast<uint32_t>(UNIXTIME);

        // reset the move time diff calculator, don't worry it will be re-calculated next movement packet.
        mSession->m_clientTimeDelay = 0;
    }

    SendPacket(SmsgPong(ping).serialise().get());

#ifdef WIN32
    // Dynamically change nagle buffering status based on latency.
    //if (_latency >= 250)
    // I think 350 is better, in a MMO 350 latency isn't that big that we need to worry about reducing the number of packets being sent.
    if (_latency >= 350)
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

void WorldSocket::_handleAuthSession(std::unique_ptr<WorldPacket> recvPacket)
{
#if VERSION_STRING == Mop
    std::string account;
    uint32_t addonSize;

    _latency = Util::getMSTime() - _latency;

    try
    {
        recvPacket->read<uint32_t>();
        recvPacket->read<uint32_t>();
        *recvPacket >> AuthDigest[18];
        *recvPacket >> AuthDigest[14];
        *recvPacket >> AuthDigest[3];
        *recvPacket >> AuthDigest[4];
        *recvPacket >> AuthDigest[0];
        recvPacket->read<uint32_t>();
        *recvPacket >> AuthDigest[11];
        *recvPacket >> mClientSeed;
        *recvPacket >> AuthDigest[19];
        recvPacket->read<uint8_t>();
        recvPacket->read<uint8_t>();
        *recvPacket >> AuthDigest[2];
        *recvPacket >> AuthDigest[9];
        *recvPacket >> AuthDigest[12];
        recvPacket->read<uint64_t>();
        recvPacket->read<uint32_t>();
        *recvPacket >> AuthDigest[16];
        *recvPacket >> AuthDigest[5];
        *recvPacket >> AuthDigest[6];
        *recvPacket >> AuthDigest[8];

        uint16_t build;
        *recvPacket >> build;
        mClientBuild = static_cast<uint32_t>(build);

        *recvPacket >> AuthDigest[17];
        *recvPacket >> AuthDigest[7];
        *recvPacket >> AuthDigest[13];
        *recvPacket >> AuthDigest[15];
        *recvPacket >> AuthDigest[1];
        *recvPacket >> AuthDigest[10];

        *recvPacket >> addonSize;

        if (addonSize)
        {
            if (recvPacket->rpos() + addonSize > recvPacket->size())
            {
                sLogger.failure("Addon size overflow packet size!");
                return;
            }
            mAddonInfoBuffer.resize(addonSize);
            recvPacket->read(static_cast<uint8_t*>(mAddonInfoBuffer.contents()), addonSize);
        }

        recvPacket->readBit();

        const auto accountNameLength = recvPacket->readBits(11);
        account = recvPacket->readString(accountNameLength);
    }
#elif VERSION_STRING == Cata
    std::string account;
    uint32_t addonSize;

    _latency = Util::getMSTime() - _latency;

    try
    {
        recvPacket->read<uint32_t>();
        recvPacket->read<uint32_t>();
        recvPacket->read<uint8_t>();
        *recvPacket >> AuthDigest[10];
        *recvPacket >> AuthDigest[18];
        *recvPacket >> AuthDigest[12];
        *recvPacket >> AuthDigest[5];
        recvPacket->read<uint64_t>();
        *recvPacket >> AuthDigest[15];
        *recvPacket >> AuthDigest[9];
        *recvPacket >> AuthDigest[19];
        *recvPacket >> AuthDigest[4];
        *recvPacket >> AuthDigest[7];
        *recvPacket >> AuthDigest[16];
        *recvPacket >> AuthDigest[3];

        uint16_t build;
        *recvPacket >> build;
        mClientBuild = static_cast<uint32_t>(build);

        *recvPacket >> AuthDigest[8];
        recvPacket->read<uint32_t>();
        recvPacket->read<uint8_t>();
        *recvPacket >> AuthDigest[17];
        *recvPacket >> AuthDigest[6];
        *recvPacket >> AuthDigest[0];
        *recvPacket >> AuthDigest[1];
        *recvPacket >> AuthDigest[11];
        *recvPacket >> mClientSeed;
        *recvPacket >> AuthDigest[2];
        recvPacket->read<uint32_t>();
        *recvPacket >> AuthDigest[14];
        *recvPacket >> AuthDigest[13];

        *recvPacket >> addonSize;
        if (addonSize)
        {
            mAddonInfoBuffer.resize(addonSize);
            recvPacket->read(static_cast<uint8_t*>(mAddonInfoBuffer.contents()), addonSize);
        }
        
        recvPacket->readBit();
        uint32_t accountNameLength = recvPacket->readBits(12);
        account = recvPacket->readString(accountNameLength);
    }
#else
    std::string account;
    uint32_t unk2;

    _latency = Util::getMSTime() - _latency;

    try
    {
#if VERSION_STRING < WotLK
        *recvPacket >> mClientBuild;
        *recvPacket >> unk2;
        *recvPacket >> account;
        *recvPacket >> mClientSeed;
#else
        uint32_t unk3;
        uint64_t unk4;
        uint32_t unk5, unk6, unk7;

        *recvPacket >> mClientBuild;
        *recvPacket >> unk2;
        *recvPacket >> account;
        *recvPacket >> unk3;
        *recvPacket >> mClientSeed;
        *recvPacket >> unk4;
        *recvPacket >> unk5;
        *recvPacket >> unk6;
        *recvPacket >> unk7;
#endif
    }
#endif
    catch (ByteBuffer::error &)
    {
        sLogger.info("Incomplete copy of AUTH_SESSION Received.");
        return;
    }

    // Send out a request for this account.
    mRequestID = sLogonCommHandler.clientConnectionId(account, this);

    if (mRequestID == 0xFFFFFFFF)
    {
        disconnect();
        return;
    }

    // shitty hash !
    m_fullAccountName = std::make_unique<std::string>(account);

    // Set the authentication packet
    pAuthenticationPacket = std::move(recvPacket);
}

void WorldSocket::_handleMsgVerifyConnection(std::unique_ptr<WorldPacket> recvPacket)
{
    std::string ClientToServerMsg;
    *recvPacket >> ClientToServerMsg;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received client Message: {}", ClientToServerMsg);

    m_HandshakeReceived = true;

    sendAuthChallengePacket();
}

void WorldSocket::informationRetreiveCallback(WorldPacket& recvData, uint32_t requestid)
{
    if (requestid != mRequestID)
        return;

    uint32_t error;
    recvData >> error;

    if (error != 0 || pAuthenticationPacket == nullptr)
    {
        // something happened wrong @ the logon server
        SendPacket(SmsgAuthResponse(AuthFailed, ARST_ONLY_ERROR).serialise().get());
        return;
    }

    // Extract account information from the packet.
    std::string AccountName;
    uint32_t AccountID;
    std::string GMFlags;
    uint8_t AccountFlags;
    std::string lang;

    recvData >> AccountID;
    recvData >> AccountName;
    recvData >> GMFlags;
    recvData >> AccountFlags;

    sLogger.debug("InfoRetreiveCallback - Account: '{}', ID: {}, GMFlags: '{}', Flags: {}", AccountName, AccountID, GMFlags, AccountFlags);

    std::string forcedPermissions = sLogonCommHandler.getPermissionStringForAccountId(AccountID);
    if (!forcedPermissions.empty())
        GMFlags = forcedPermissions;

    sLogger.debug("InformationRetreiveCallback : got information packet from logon: `{}` ID {} (request {})", AccountName, AccountID, mRequestID);

    mRequestID = 0;

    // Pull the sessionkey we generated during the logon - client handshake
    uint8_t K[40];
    recvData.read(K, 40);

#if VERSION_STRING < WotLK
    BigNumber BNK;
    BNK.SetBinary(K, 40);

#if VERSION_STRING == TBC
    auto key = std::make_unique<uint8_t[]>(20);
    WowCrypt::generateTbcKey(key.get(), K);

    _crypt.setLegacyKey(key.get(), 20);
    _crypt.initLegacyCrypt();
#elif VERSION_STRING == Classic
    static constexpr uint8_t classicAuthKey[16] = { 0x38, 0xA7, 0x83, 0x15, 
                                                    0xF8, 0x92, 0x25, 0x30, 
                                                    0x71, 0x98, 0x67, 0xB1, 
                                                    0x8C, 0x04, 0xE2, 0xAA };
    uint8_t abuf[64], bbuf[64];
    memset(abuf, 0x36, 64);
    memset(bbuf, 0x5C, 64);
    for (int i = 0; i<16; ++i)
    {
        abuf[i] ^= classicAuthKey[i];
        bbuf[i] ^= classicAuthKey[i];
    }

    Sha1Hash hasher;
    uint8_t buffer[104];
    hasher.initialize();
    memcpy(buffer, abuf, 64);
    memcpy(&buffer[64], K, 40);
    hasher.updateData(buffer, 104);
    hasher.finalize();
    memcpy(buffer, bbuf, 64);
    memcpy(&buffer[64], hasher.getDigest(), 20);
    hasher.initialize();
    hasher.updateData(buffer, 84);
    hasher.finalize();

    _crypt.setLegacyKey(K, 40);
    _crypt.initLegacyCrypt();
#endif
#elif VERSION_STRING < Mop
    _crypt.initWotlkCrypt(K);
#else
    _crypt.initMopCrypt(K);

    BigNumber BNK;
    BNK.SetBinary(K, 40);
#endif

#if VERSION_STRING != Mop
    recvData >> lang;
#else
    if (recvData.rpos() != recvData.wpos())
    {
        lang.resize(4);
        recvData.read(reinterpret_cast<uint8_t*>(lang.data()), 4);
    }
#endif

    //checking if player is already connected
    //disconnect current player and login this one(blizzlike)
    WorldSession* oldSession = sWorld.getSessionByAccountId(AccountID);
    if (oldSession)
    {
        if (oldSession->GetSocket() == nullptr)
        {
            sLogger.info("Reconnect: removing zombie session for account {} (ID: {})", AccountName, AccountID);
            if (oldSession->GetPlayer() && !oldSession->IsLoggingOut())
                oldSession->LogoutPlayer(true);
            sWorld.deleteSession(oldSession);
            // Continue with new session
        }
        else
        {
            sLogger.info("Reconnect: account {} (ID: {}) already connected from {}; rejecting new connection",
                AccountName, AccountID,
                oldSession->GetSocket() ? oldSession->GetSocket()->getRemoteIp().c_str() : "unknown");
            // AUTH_FAILED = 0x0D
            oldSession->Disconnect();

            // clear the logout timer so he times out straight away
            oldSession->SetLogoutTimer(1);

            // we must send authentication failed here.
            // the stupid newb can relog his client.
            // otherwise accounts dupe up and disasters happen.
            SendPacket(SmsgAuthResponse(AuthUnknownAccount, ARST_ONLY_ERROR).serialise().get());
            return;
        }
    }

    Sha1Hash sha;
#if VERSION_STRING < Cata
    uint8_t digest[20];
    pAuthenticationPacket->read(digest, 20);
#endif
    uint32_t t = 0;
    if (m_fullAccountName == nullptr) // should never happen !
        sha.updateData(AccountName);
    else
    {
        sha.updateData(*m_fullAccountName);

        // this is unused now. we may as well free up the memory.
        m_fullAccountName = nullptr;
    }

    sha.updateData(reinterpret_cast<uint8_t*>(&t), 4);
    sha.updateData(reinterpret_cast<uint8_t*>(&mClientSeed), 4);
    sha.updateData(reinterpret_cast<uint8_t*>(&mSeed), 4);
#if VERSION_STRING < WotLK
    sha.updateBigNumbers(&BNK, NULL);
#else
    sha.updateData(reinterpret_cast<uint8_t*>(&K), 40);
#endif
    sha.finalize();

#if VERSION_STRING < Cata
    if (memcmp(sha.getDigest(), digest, 20))
#else
    if (memcmp(sha.getDigest(), AuthDigest, 20))
#endif
    {
        // AUTH_UNKNOWN_ACCOUNT = 21
        SendPacket(SmsgAuthResponse(AuthUnknownAccount, ARST_ONLY_ERROR).serialise().get());
        return;
    }

    // Allocate session
    auto pSession = std::make_unique<WorldSession>(AccountID, AccountName, this);

    mSession = pSession.get();

    // aquire delete mutex
    std::lock_guard guard(pSession->deleteMutex);

    // Set session properties
    pSession->SetClientBuild(mClientBuild);

#if VERSION_STRING >= Cata
    pSession->readAddonInfoPacket(mAddonInfoBuffer);
#endif

    pSession->LoadSecurity(GMFlags);
    pSession->SetAccountFlags(AccountFlags);
    pSession->m_lastPing = static_cast<uint32_t>(UNIXTIME);
    pSession->language = Util::getLanguagesIdFromString(lang);

#if VERSION_STRING != Mop
    recvData >> pSession->m_muted;
#else
    if (recvData.rpos() != recvData.wpos())
        recvData >> pSession->m_muted;
#endif

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

    sLogger.debug("{} from {}:{} [{}ms]", AccountName, getRemoteIp(), getRemotePort(), _latency);

    // Check for queue.
    uint32_t playerLimit = worldConfig.getPlayerLimit();
    if ((sWorld.getSessionCount() < playerLimit) || pSession->HasGMPermissions())
    {
        Authenticate(std::move(pSession));
    }
    else if (playerLimit > 0)
    {
        // Queued, sucker.
        uint32_t Position = sWorld.addQueuedSocket(this, std::move(pSession));
        mQueued = true;
        sLogger.debug("{} added to queue in position {}", AccountName, Position);

        // Send packet so we know what we're doing
        UpdateQueuePosition(Position);
    }
    else
    {
        SendPacket(SmsgAuthResponse(AuthRejected, ARST_ONLY_ERROR).serialise().get());
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
            sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "[{}]: {} {} (0x{:03X}) of {} bytes.", direction ? "SERVER" : "CLIENT", direction ? "sent" : "received",
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
