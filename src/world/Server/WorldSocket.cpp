/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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

#include "StdAfx.h"
#include "AuthCodes.h"
#include "Management/AddonMgr.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/MainServerDefines.h"
#include "Auth/Sha1.h"
#include "World.h"
#include "World.Legacy.h"

#pragma pack(push, 1)
struct ClientPktHeader
{
    uint16 size;
    uint32 cmd;
};

// MIT
struct ServerPktHeader
{
#if VERSION_STRING == Cata
    ServerPktHeader(uint32_t _size, uint16_t _cmd) : size(_size)
    {
        headerLength = 0;
        if (size > 0x7FFF)
            header[headerLength++] = 0x80 | (0xFF & (_size >> 16));
        header[headerLength++] = 0xFF & (_size >> 8);
        header[headerLength++] = 0xFF & _size;
        header[headerLength++] = 0xFF & _cmd;
        header[headerLength++] = 0xFF & (_cmd >> 8); 
    }

    uint8_t getHeaderLength() { return headerLength; }
    bool isLargePacket() { return ((headerLength == 4) ? true : false); }
    const uint32_t size;
    uint8_t header[5];
    uint8_t headerLength;
#else
    uint16_t size;
    uint16_t cmd;
#endif
};
// MIT End
#pragma pack(pop)

WorldSocket::WorldSocket(SOCKET fd)
    :
    Socket(fd, WORLDSOCKET_SENDBUF_SIZE, WORLDSOCKET_RECVBUF_SIZE),
    Authed(false),
    mOpcode(0),
    mRemaining(0),
    mSize(0),
    mSeed(Util::getRandomUInt(RAND_MAX)),
    mRequestID(0),
    mSession(NULL),
    pAuthenticationPacket(NULL),
    _latency(0),
    mQueued(false),
    m_nagleEanbled(false),
    mClientSeed(0),
    mClientBuild(0),
    m_fullAccountName(NULL)
{

}

WorldSocket::~WorldSocket()
{
    WorldPacket* pck;
    queueLock.Acquire();
    while ((pck = _queue.Pop()) != NULL)
    {
        delete pck;
    }
    queueLock.Release();

    if (pAuthenticationPacket)
        delete pAuthenticationPacket;

    if (mSession)
    {
        mSession->SetSocket(NULL);
        mSession = NULL;
    }

    if (m_fullAccountName != NULL)
    {
        delete m_fullAccountName;
        m_fullAccountName = NULL;
    }
}

void WorldSocket::OnDisconnect()
{
    if (mSession)
    {
        mSession->SetSocket(0);
        mSession = NULL;
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

void WorldSocket::OutPacket(uint16 opcode, size_t len, const void* data)
{
    OUTPACKET_RESULT res;
    if ((len + 10) > WORLDSOCKET_SENDBUF_SIZE)
    {
        LOG_ERROR("WARNING: Tried to send a packet of %u bytes (which is too large) to a socket. Opcode was: %u (0x%03X)", (unsigned int)len, (unsigned int)opcode, (unsigned int)opcode);
        return;
    }

    res = _OutPacket(opcode, len, data);
    if (res == OUTPACKET_RESULT_SUCCESS)
        return;

    if (res == OUTPACKET_RESULT_NO_ROOM_IN_BUFFER)
    {
        /* queue the packet */
        queueLock.Acquire();
        WorldPacket* pck = new WorldPacket(opcode, len);
        if (len) pck->append((const uint8*)data, len);
        _queue.Push(pck);
        queueLock.Release();
    }
}

void WorldSocket::UpdateQueuedPackets()
{
    queueLock.Acquire();
    if (!_queue.HasItems())
    {
        queueLock.Release();
        return;
    }

    WorldPacket* pck;
    while ((pck = _queue.front()) != NULL)
    {
        /* try to push out as many as you can */
        switch (_OutPacket(pck->GetOpcode(), pck->size(), pck->size() ? pck->contents() : NULL))
        {
            case OUTPACKET_RESULT_SUCCESS:
            {
                delete pck;
                _queue.pop_front();
            }
            break;

            case OUTPACKET_RESULT_NO_ROOM_IN_BUFFER:
            {
                /* still connected */
                queueLock.Release();
                return;
            }
            break;

            default:
            {
                /* kill everything in the buffer */
                while ((pck == _queue.Pop()) != 0)
                {
                    delete pck;
                }
                queueLock.Release();
                return;
            }
            break;
        }
    }
    queueLock.Release();
}

OUTPACKET_RESULT WorldSocket::_OutPacket(uint16 opcode, size_t len, const void* data)
{
    bool rv;
    if (!IsConnected())
        return OUTPACKET_RESULT_NOT_CONNECTED;

    BurstBegin();
    //if ((m_writeByteCount + len + 4) >= m_writeBufferSize)
    if (writeBuffer.GetSpace() < (len + 4))
    {
        BurstEnd();
        return OUTPACKET_RESULT_NO_ROOM_IN_BUFFER;
    }

    // Packet logger :)
    sWorldPacketLog.logPacket((uint32)len, opcode, (const uint8*)data, 1, (mSession ? mSession->GetAccountId() : 0));

#if VERSION_STRING == Cata
    ServerPktHeader Header(uint32(len + 2), static_cast<uint16_t>(opcode));
#else
    // Encrypt the packet
    // First, create the header.
    ServerPktHeader Header;

    Header.cmd = opcode;
    Header.size = ntohs((uint16)len + 2);
#endif

#if VERSION_STRING < WotLK
    _crypt.encryptTbcSend((uint8*)&Header, sizeof(ServerPktHeader));
#elif VERSION_STRING == WotLK
    _crypt.encryptWotlkSend((uint8*)&Header, sizeof(ServerPktHeader));
#elif VERSION_STRING == Cata
    _crypt.encryptWotlkSend(((uint8*)Header.header), Header.getHeaderLength());
#endif

#if VERSION_STRING == Cata
    rv = BurstSend((const uint8*)&Header.header, Header.getHeaderLength());
#else
    // Pass the header to our send buffer
    rv = BurstSend((const uint8*)&Header, 4);
#endif

    // Pass the rest of the packet to our send buffer (if there is any)
    if (len > 0 && rv)
    {
        rv = BurstSend((const uint8*)data, (uint32)len);
    }

    if (rv) BurstPush();
    BurstEnd();
    return rv ? OUTPACKET_RESULT_SUCCESS : OUTPACKET_RESULT_SOCKET_ERROR;
}

void WorldSocket::OnConnect()
{
    sWorld.increaseAcceptedConnections();
    _latency = Util::getMSTime();

#if VERSION_STRING < WotLK
    OutPacket(SMSG_AUTH_CHALLENGE, 4, &mSeed);
#elif VERSION_STRING == WotLK
    WorldPacket wp(SMSG_AUTH_CHALLENGE, 24);

    wp << uint32(1);
    wp << uint32(mSeed);
    wp << uint32(0xC0FFEEEE);
    wp << uint32(0x00BABE00);
    wp << uint32(0xDF1697E5);
    wp << uint32(0x1234ABCD);

    SendPacket(&wp);
#elif VERSION_STRING == Cata
    WorldPacket packet(MSG_WOW_CONNECTION, 46);
    packet << "RLD OF WARCRAFT CONNECTION - SERVER TO CLIENT";
    SendPacket(&packet);
#endif
}

#if VERSION_STRING == Cata
void WorldSocket::OnConnectTwo()
{
    WorldPacket packet(SMSG_AUTH_CHALLENGE, 37);
    for (int i = 0; i < 8; ++i)
        packet << uint32(0);
    
    packet << mSeed;
    packet << uint8(1);

    SendPacket(&packet);
}
#endif

void WorldSocket::_HandleAuthSession(WorldPacket* recvPacket)
{
#if VERSION_STRING == Cata
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
        *recvPacket >> mClientBuild;
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
            recvPacket->read((uint8_t*)mAddonInfoBuffer.contents(), addonSize);
        }
        
        recvPacket->readBit();
        uint32_t accountNameLength = recvPacket->readBits(12);
        account = recvPacket->ReadString(accountNameLength);
    }
#else
    std::string account;
    uint32 unk2, unk3;
    uint64 unk4;
    uint32 unk5, unk6, unk7;

    _latency = Util::getMSTime() - _latency;

    try
    {
#if VERSION_STRING < WotLK
        *recvPacket >> mClientBuild;
        *recvPacket >> unk2;
        *recvPacket >> account;
        *recvPacket >> mClientSeed;
#else
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
        LOG_DETAIL("Incomplete copy of AUTH_SESSION Received.");
        return;
    }

    // Send out a request for this account.
    mRequestID = sLogonCommHandler.clientConnectionId(account, this);

    if (mRequestID == 0xFFFFFFFF)
    {
        Disconnect();
        return;
    }

    // shitty hash !
    m_fullAccountName = new std::string(account);

    // Set the authentication packet
    pAuthenticationPacket = recvPacket;
}

void WorldSocket::InformationRetreiveCallback(WorldPacket & recvData, uint32 requestid)
{
    if (requestid != mRequestID)
        return;

    uint32 error;
    recvData >> error;

    if (error != 0 || pAuthenticationPacket == nullptr)
    {
        // something happened wrong @ the logon server
        OutPacket(SMSG_AUTH_RESPONSE, 1, "\x0D");
        return;
    }

    // Extract account information from the packet.
    std::string AccountName;
    const std::string* ForcedPermissions;
    uint32 AccountID;
    std::string GMFlags;
    uint8 AccountFlags;
    std::string lang;

    recvData >> AccountID;
    recvData >> AccountName;
    recvData >> GMFlags;
    recvData >> AccountFlags;

    ForcedPermissions = sLogonCommHandler.getPermissionStringForAccountId(AccountID);
    if (ForcedPermissions != nullptr)
        GMFlags.assign(ForcedPermissions->c_str());

    LOG_DEBUG(" >> got information packet from logon: `%s` ID %u (request %u)", AccountName.c_str(), AccountID, mRequestID);

    mRequestID = 0;

    // Pull the sessionkey we generated during the logon - client handshake
    uint8 K[40];
    recvData.read(K, 40);

#if VERSION_STRING < WotLK
    BigNumber BNK;
    BNK.SetBinary(K, 40);

    uint8 *key = new uint8[20];
    WowCrypt::generateTbcKey(key, K);

    _crypt.setTbcKey(key, 20);
    _crypt.initTbcCrypt();
    delete[] key;
#else
    _crypt.initWotlkCrypt(K);
#endif

    recvData >> lang;

    //checking if player is already connected
    //disconnect current player and login this one(blizzlike)
    WorldSession* session = sWorld.getSessionByAccountId(AccountID);
    if (session)
    {
        // AUTH_FAILED = 0x0D
        session->Disconnect();

        // clear the logout timer so he times out straight away
        session->SetLogoutTimer(1);

        // we must send authentication failed here.
        // the stupid newb can relog his client.
        // otherwise accounts dupe up and disasters happen.
        OutPacket(SMSG_AUTH_RESPONSE, 1, "\x15");
        return;
    }

    Sha1Hash sha;
#if VERSION_STRING != Cata
    uint8 digest[20];
    pAuthenticationPacket->read(digest, 20);
#endif
    uint32 t = 0;
    if (m_fullAccountName == nullptr) // should never happen !
        sha.UpdateData(AccountName);
    else
    {
        sha.UpdateData(*m_fullAccountName);

        // this is unused now. we may as well free up the memory.
        delete m_fullAccountName;
        m_fullAccountName = nullptr;
    }

    sha.UpdateData((uint8*)&t, 4);
    sha.UpdateData((uint8*)&mClientSeed, 4);
    sha.UpdateData((uint8*)&mSeed, 4);
#if VERSION_STRING < WotLK
    sha.UpdateBigNumbers(&BNK, NULL);
#else
    sha.UpdateData((uint8*)&K, 40);
#endif
    sha.Finalize();

#if VERSION_STRING != Cata
    if (memcmp(sha.GetDigest(), digest, 20))
#else
    if (memcmp(sha.GetDigest(), AuthDigest, 20))
#endif
    {
        // AUTH_UNKNOWN_ACCOUNT = 21
        OutPacket(SMSG_AUTH_RESPONSE, 1, "\x15");
        return;
    }

    // Allocate session
    WorldSession* pSession = new WorldSession(AccountID, AccountName, this);
    mSession = pSession;
    ARCEMU_ASSERT(mSession != nullptr);
    // aquire delete mutex
    pSession->deleteMutex.Acquire();

    // Set session properties
    pSession->SetClientBuild(mClientBuild);

#if VERSION_STRING == Cata
    pSession->readAddonInfoPacket(mAddonInfoBuffer);
#endif

    pSession->LoadSecurity(GMFlags);
    pSession->SetAccountFlags(AccountFlags);
    pSession->m_lastPing = (uint32)UNIXTIME;
    pSession->language = Util::getLanguagesIdFromString(lang);

    recvData >> pSession->m_muted;

    for (uint8_t i = 0; i < 8; ++i)
        pSession->SetAccountData(i, nullptr, true, 0);

    if (worldConfig.server.useAccountData)
    {
        QueryResult* pResult = CharacterDatabase.Query("SELECT * FROM account_data WHERE acct = %u", AccountID);
        if (pResult == nullptr)
            CharacterDatabase.Execute("INSERT INTO account_data VALUES(%u, '', '', '', '', '', '', '', '', '')", AccountID);
        else
        {
            size_t len;
            const char* data;
            char* d;
            for (uint8_t i = 0; i < 8; ++i)
            {
                data = pResult->Fetch()[1 + i].GetString();
                len = data ? strlen(data) : 0;
                if (len > 1)
                {
                    d = new char[len + 1];
                    memcpy(d, data, len + 1);
                    pSession->SetAccountData(i, d, true, (uint32)len);
                }
            }

            delete pResult;
        }
    }

    LOG_DEBUG("%s from %s:%u [%ums]", AccountName.c_str(), GetRemoteIP().c_str(), GetRemotePort(), _latency);
#ifdef SESSION_CAP
    if (sWorld.GetSessionCount() >= SESSION_CAP)
    {
        OutPacket(SMSG_AUTH_RESPONSE, 1, "\x0D");
        Disconnect();
        return;
    }
#endif

    // Check for queue.
    uint32 playerLimit = worldConfig.getPlayerLimit();
    if ((sWorld.getSessionCount() < playerLimit) || pSession->HasGMPermissions())
    {
        Authenticate();
    }
    else if (playerLimit > 0)
    {
        // Queued, sucker.
        uint32 Position = sWorld.addQueuedSocket(this);
        mQueued = true;
        LOG_DEBUG("%s added to queue in position %u", AccountName.c_str(), Position);

        // Send packet so we know what we're doing
        UpdateQueuePosition(Position);
    }
    else
    {
        OutPacket(SMSG_AUTH_RESPONSE, 1, "\x0E"); // AUTH_REJECT = 14
        Disconnect();
    }

    // release delete mutex
    pSession->deleteMutex.Release();
}

void WorldSocket::Authenticate()
{
    ARCEMU_ASSERT(pAuthenticationPacket != NULL);
    mQueued = false;

    if (mSession == NULL)
        return;

#if VERSION_STRING != Cata
    if (mSession->HasFlag(ACCOUNT_FLAG_XPACK_02))
        OutPacket(SMSG_AUTH_RESPONSE, 11, "\x0C\x30\x78\x00\x00\x00\x00\x00\x00\x00\x02");
    else if (mSession->HasFlag(ACCOUNT_FLAG_XPACK_01))
        OutPacket(SMSG_AUTH_RESPONSE, 11, "\x0C\x30\x78\x00\x00\x00\x00\x00\x00\x00\x01");
    else
        OutPacket(SMSG_AUTH_RESPONSE, 11, "\x0C\x30\x78\x00\x00\x00\x00\x00\x00\x00\x00");

    sAddonMgr.SendAddonInfoPacket(pAuthenticationPacket, static_cast<uint32>(pAuthenticationPacket->rpos()), mSession);
#else
    WorldPacket data(SMSG_AUTH_RESPONSE, 17);
    data.writeBit(false);
    data.writeBit(true);
    data << uint32_t(0);                          // BillingTimeRemaining
    data << uint8_t(3);                           // 0 - normal, 1 - TBC, 2 - WOTLK, 3 - CATA
    data << uint32_t(0);
    data << uint8_t(3);
    data << uint32_t(0);                          // BillingTimeRested
    data << uint8_t(0);                           // BillingPlanFlags
    data << uint8_t(0x0C);                        // 0x0C = 12 (AUTH_OK)
    SendPacket(&data);

    mSession->SendClientCacheVersion(15595);

    // send addon info here
    mSession->sendAddonInfo();
#endif


    mSession->_latency = _latency;

    delete pAuthenticationPacket;
    pAuthenticationPacket = NULL;

    sWorld.addSession(mSession);
    sWorld.addGlobalSession(mSession);

#if VERSION_STRING > TBC
#if VERSION_STRING != Cata
    mSession->SendClientCacheVersion(12340);
#endif
#endif

}

void WorldSocket::UpdateQueuePosition(uint32 Position)
{
#if VERSION_STRING != Cata
    // cebernic: Displays re-correctly until 2.4.3,there will not be always 0
    WorldPacket QueuePacket(SMSG_AUTH_RESPONSE, 16);
    QueuePacket << uint8(0x1B) << uint8(0x2C) << uint8(0x73) << uint8(0) << uint8(0);
    QueuePacket << uint32(0) << uint8(0);// << uint8(0);
    QueuePacket << Position;
    //    QueuePacket << uint8(1);
#else
    WorldPacket QueuePacket(SMSG_AUTH_RESPONSE, 21);    // 17 + 4 if queued
    QueuePacket.writeBit(true);                         // has queue
    QueuePacket.writeBit(false);                        // unk queue-related
    QueuePacket.writeBit(true);                         // has account data
    QueuePacket << uint32_t(0);                         // Unknown - 4.3.2
    QueuePacket << uint8_t(3);                          // 0 - normal, 1 - TBC, 2 - WotLK, 3 - CT. must be set in database manually for each account
    QueuePacket << uint32_t(0);                         // BillingTimeRemaining
    QueuePacket << uint8_t(3);                          // 0 - normal, 1 - TBC, 2 - WotLK, 3 - CT. Must be set in database manually for each account.
    QueuePacket << uint32_t(0);                         // BillingTimeRested
    QueuePacket << uint8_t(0);                          // BillingPlanFlags
    QueuePacket << uint8_t(0x1B);                       // Waiting in queue (AUTH_WAIT_QUEUE I think)
    QueuePacket << uint32_t(Position);                  // position in queue
#endif
    SendPacket(&QueuePacket);
}

void WorldSocket::_HandlePing(WorldPacket* recvPacket)
{
    uint32 ping;
    if (recvPacket->size() < 4)
    {
        LOG_ERROR("Socket closed due to incomplete ping packet.");
        Disconnect();
        return;
    }

#if VERSION_STRING != Cata
    *recvPacket >> ping;
    *recvPacket >> _latency;
#else
    *recvPacket >> _latency;
    *recvPacket >> ping;
#endif

    if (mSession)
    {
        mSession->_latency = _latency;
        mSession->m_lastPing = (uint32)UNIXTIME;

        // reset the move time diff calculator, don't worry it will be re-calculated next movement packet.
        mSession->m_clientTimeDelay = 0;
    }

    OutPacket(SMSG_PONG, 4, &ping);

#ifdef WIN32
    // Dynamically change nagle buffering status based on latency.
    //if (_latency >= 250)
    // I think 350 is better, in a MMO 350 latency isn't that big that we need to worry about reducing the number of packets being sent.
    if (_latency >= 350)
    {
        if (!m_nagleEanbled)
        {
            u_long arg = 0;
            setsockopt(GetFd(), 0x6, 0x1, (const char*)&arg, sizeof(arg));
            m_nagleEanbled = true;
        }
    }
    else
    {
        if (m_nagleEanbled)
        {
            u_long arg = 1;
            setsockopt(GetFd(), 0x6, 0x1, (const char*)&arg, sizeof(arg));
            m_nagleEanbled = false;
        }
    }
#endif
}

void WorldSocket::OnRead()
{
    for (;;)
    {
        // Check for the header if we don't have any bytes to wait for.
        if (mRemaining == 0)
        {
            if (readBuffer.GetSize() < 6)
            {
                // No header in the packet, let's wait.
                return;
            }

            // Copy from packet buffer into header local var
            ClientPktHeader Header;
            readBuffer.Read((uint8*)&Header, 6);

            // Decrypt the header
#if VERSION_STRING < WotLK
            _crypt.decryptTbcReceive((uint8*)&Header, sizeof(ClientPktHeader));
#else
            _crypt.decryptWotlkReceive((uint8*)&Header, sizeof(ClientPktHeader));
#endif

            mRemaining = mSize = ntohs(Header.size) - 4;
            mOpcode = Header.cmd;
        }

        WorldPacket* Packet;

        if (mRemaining > 0)
        {
            if (readBuffer.GetSize() < mRemaining)
            {
                // We have a fragmented packet. Wait for the complete one before proceeding.
                return;
            }
        }

        Packet = new WorldPacket(static_cast<uint16>(mOpcode), mSize);
        Packet->resize(mSize);

        if (mRemaining > 0)
        {
            // Copy from packet buffer into our actual buffer.
            ///Read(mRemaining, (uint8*)Packet->contents());
            readBuffer.Read((uint8*)Packet->contents(), mRemaining);
        }

        sWorldPacketLog.logPacket(mSize, static_cast<uint16>(mOpcode), mSize ? Packet->contents() : NULL, 0, (mSession ? mSession->GetAccountId() : 0));
        mRemaining = mSize = mOpcode = 0;

        // Check for packets that we handle
        switch (Packet->GetOpcode())
        {
            case CMSG_PING:
            {
                _HandlePing(Packet);
                delete Packet;
            }
            break;
#if VERSION_STRING == Cata
            case MSG_WOW_CONNECTION:
            {
                HandleWoWConnection(Packet);
            }
            break;
#endif
            case CMSG_AUTH_SESSION:
            {
                _HandleAuthSession(Packet);
            }
            break;
            default:
            {
                if (mSession)
                    mSession->QueuePacket(Packet);
                else
                    delete Packet;
            }
            break;
        }
    }
}

#if VERSION_STRING == Cata
void WorldSocket::HandleWoWConnection(WorldPacket* recvPacket)
{
    std::string ClientToServerMsg;
    *recvPacket >> ClientToServerMsg;

    OnConnectTwo();
}

void WorldSocket::SendAuthResponseError(uint8_t code)
{
    WorldPacket packet(SMSG_AUTH_RESPONSE, 1);
    packet.writeBit(0);                         // has queue info
    packet.writeBit(0);                         // has account info
    packet << uint8_t(code);                    // the error code
		  
    SendPacket(&packet);
}
#endif

#if VERSION_STRING != Cata
void WorldPacketLog::logPacket(uint32_t len, uint16_t opcode, const uint8_t* data, uint8_t direction, uint32_t accountid)
#else
void WorldPacketLog::logPacket(uint32_t len, uint32_t opcode, const uint8_t* data, uint8_t direction, uint32_t accountid)
#endif
{
    if (worldConfig.log.worldDebugFlags & LF_OPCODE)
    {
        switch (opcode)
        {
            //stop spaming opcodes here
            case SMSG_MONSTER_MOVE:
            case MSG_MOVE_HEARTBEAT:
            {
            } break;
            default:
            {
                LogDebugFlag(LF_OPCODE, "[%s]: %s %s (0x%03X) of %u bytes.", direction ? "SERVER" : "CLIENT", direction ? "sent" : "received",
                    getOpcodeName(opcode).c_str(), opcode, len);
            } break;
        }
}

    if (isLogEnabled)
    {
        mPacketLogMutex.Acquire();
        unsigned int line = 1;
        unsigned int countpos = 0;
        uint16_t lenght = static_cast<uint16_t>(len);
        unsigned int count = 0;

        fprintf(mPacketLogFile, "{%s} Packet: (0x%04X) %s PacketSize = %u stamp = %u accountid = %u\n", (direction ? "SERVER" : "CLIENT"), opcode,
            getOpcodeName(opcode).c_str(), lenght, Util::getMSTime(), accountid);
        fprintf(mPacketLogFile, "|------------------------------------------------|----------------|\n");
        fprintf(mPacketLogFile, "|00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |0123456789ABCDEF|\n");
        fprintf(mPacketLogFile, "|------------------------------------------------|----------------|\n");

        if (lenght > 0)
        {
            fprintf(mPacketLogFile, "|");
            for (count = 0; count < lenght; count++)
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

                    unsigned short print = 0;

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

                        print++;
                    }

                    for (unsigned int c = print; c < 16; c++)
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
        mPacketLogMutex.Release();
    }
}
