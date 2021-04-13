/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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
//#include "Server/CharacterErrors.h"
//#include "Management/AddonMgr.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/MainServerDefines.h"
#include "Auth/Sha1.h"
#include "World.h"
#include "Management/AddonMgr.h"
#include "Packets/SmsgPong.h"
#include "Packets/SmsgAuthChallenge.h"
#include "Packets/SmsgAuthResponse.h"
#include "OpcodeTable.hpp"
//#include "World.Legacy.h"

using namespace AscEmu::Packets;

#pragma pack(push, 1)
struct ClientPktHeader
{
    uint16 size;
    uint32 cmd;
};

struct AuthPktHeader
{
    AuthPktHeader(uint32_t _size, uint32_t _opcode) : raw(_size << 13 | _opcode & 0x01FFF) {}

    uint16_t getOpcode() const
    {
        return uint16_t(raw & 0x01FFF);
    }

    uint32_t getSize() const
    {
        return raw >> 13;
    };

    uint32_t raw;
};

// MIT
#if VERSION_STRING != Mop
struct ServerPktHeader
{
#if VERSION_STRING >= Cata
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
    bool isLargePacket() { return (headerLength == 4); }
    const uint32_t size;
    uint8_t header[5]{};
    uint8_t headerLength;
#else
    uint16_t size;
    uint16_t cmd;
#endif
};
#else
struct ServerPktHeader
{
    ServerPktHeader(uint32_t _size, uint32_t _cmd) : headerLength(0)
    {
        if (_size > 0x7FFF)
            header[headerLength++] = 0x80 | 0xFF & _size >> 16;

        header[headerLength++] = 0xFF & _size;
        header[headerLength++] = 0xFF & _size >> 8;
        header[headerLength++] = 0xFF & _cmd;
        header[headerLength++] = 0xFF & _cmd >> 8;
    }

    uint32_t getOpcode() const
    {
        uint8_t length = headerLength;
        uint32_t opcode = uint32_t(header[--length]) << 8;
        opcode |= uint32_t(header[--length]);

        return opcode;
    }

    uint32_t getSize() const
    {
        uint32_t size = 0;

        uint8_t length = 0;
        if (header[length] & 0x80)
            size |= uint32_t(header[length++] & 0x7F) << 16;

        size |= uint32_t(header[length++] & 0xFF) << 8;
        size |= uint32_t(header[length] & 0xFF);

        return size;
    }

    uint8_t headerLength;
    uint8_t header[6]{};
};
#endif

// MIT End
#pragma pack(pop)

WorldSocket::WorldSocket(SOCKET fd)
    :
    Socket(fd, WORLDSOCKET_SENDBUF_SIZE, WORLDSOCKET_RECVBUF_SIZE), AuthDigest{},
    mClientBuild(0),
    Authed(false),
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
}

WorldSocket::~WorldSocket()
{
    WorldPacket* pck;
    queueLock.Acquire();
    while ((pck = _queue.Pop()) != nullptr)
    {
        delete pck;
    }
    queueLock.Release();

    delete pAuthenticationPacket;

    if (mSession)
    {
        mSession->SetSocket(nullptr);
        mSession = nullptr;
    }

    if (m_fullAccountName != nullptr)
    {
        delete m_fullAccountName;
        m_fullAccountName = nullptr;
    }
}

void WorldSocket::OnDisconnect()
{
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

#if VERSION_STRING != Mop
void WorldSocket::OutPacket(uint16 opcode, size_t len, const void* data)
#else
void WorldSocket::OutPacket(uint32_t opcode, size_t len, const void* data)
#endif
{
    if ((len + 10) > WORLDSOCKET_SENDBUF_SIZE)
    {
        LOG_ERROR("WARNING: Tried to send a packet of %u bytes (which is too large) to a socket. Opcode was: %u (0x%03X)", static_cast<unsigned int>(len), static_cast<unsigned int>(opcode), static_cast<unsigned int>(opcode));
        return;
    }

    OUTPACKET_RESULT res = _OutPacket(opcode, len, data);
    if (res == OUTPACKET_RESULT_SUCCESS)
        return;

    if (res == OUTPACKET_RESULT_NO_ROOM_IN_BUFFER)
    {
        /* queue the packet */
        queueLock.Acquire();
        WorldPacket* packet = new WorldPacket(opcode, len);
        if (len)
            packet->append(static_cast<const uint8_t*>(data), len);

        _queue.Push(packet);
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
    while ((pck = _queue.front()) != nullptr)
    {
        /* try to push out as many as you can */
        switch (_OutPacket(pck->GetOpcode(), pck->size(), pck->size() ? pck->contents() : nullptr))
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
        }
    }
    queueLock.Release();
}

#if VERSION_STRING != Mop
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
    sWorldPacketLog.logPacket(static_cast<uint32_t>(len), opcode, static_cast<const uint8_t*>(data), 1, (mSession ? mSession->GetAccountId() : 0));

#if VERSION_STRING >= Cata
    ServerPktHeader Header(uint32(len + 2), sOpcodeTables.getHexValueForVersionId(sOpcodeTables.getVersionIdForAEVersion(), opcode));
#else
    // Encrypt the packet
    // First, create the header.
    ServerPktHeader Header;
    Header.cmd = sOpcodeTables.getHexValueForVersionId(sOpcodeTables.getVersionIdForAEVersion(), opcode);
    Header.size = ntohs((uint16)len + 2);
#endif

#if VERSION_STRING < WotLK
    _crypt.encryptLegacySend((uint8*)&Header, sizeof(ServerPktHeader));
#elif VERSION_STRING == WotLK
    _crypt.encryptWotlkSend((uint8*)&Header, sizeof(ServerPktHeader));
#elif VERSION_STRING >= Cata
    _crypt.encryptWotlkSend(static_cast<uint8*>(Header.header), Header.getHeaderLength());
#endif

#if VERSION_STRING >= Cata
    rv = BurstSend(reinterpret_cast<const uint8*>(&Header.header), Header.getHeaderLength());
#else
    // Pass the header to our send buffer
    rv = BurstSend((const uint8*)&Header, 4);
#endif

    // Pass the rest of the packet to our send buffer (if there is any)
    if (len > 0 && rv)
    {
        rv = BurstSend(static_cast<const uint8*>(data), static_cast<uint32>(len));
    }

    if (rv) BurstPush();
    BurstEnd();
    return rv ? OUTPACKET_RESULT_SUCCESS : OUTPACKET_RESULT_SOCKET_ERROR;
}
#else
OUTPACKET_RESULT WorldSocket::_OutPacket(uint32_t opcode, size_t len, const void* data)
{
    bool rv;
    if (!IsConnected())
        return OUTPACKET_RESULT_NOT_CONNECTED;

    BurstBegin();

    if (writeBuffer.GetSpace() < (len + 4))
    {
        BurstEnd();
        return OUTPACKET_RESULT_NO_ROOM_IN_BUFFER;
    }

    switch (opcode)
    {
    //case SMSG_POWER_UPDATE:
    //case SMSG_ITEM_TIME_UPDATE:
    //case SMSG_AURA_UPDATE_ALL:
    //case SMSG_UPDATE_INSTANCE_OWNERSHIP:
    //case SMSG_SPELL_GO:
    //case SMSG_SPELL_COOLDOWN:
    //case SMSG_SPELL_START:
    //case SMSG_SPELL_FAILURE:
    //case SMSG_CAST_FAILED:
    case SMSG_MOVE_SET_CAN_FLY:
    //case SMSG_TIME_SYNC_REQ:
    //case SMSG_UPDATE_OBJECT:
    //case SMSG_UPDATE_WORLD_STATE:
    //case MSG_MOVE_SET_FLIGHT_SPEED:
    //case MSG_MOVE_SET_RUN_SPEED:
    //case SMSG_LOGIN_SETTIMESPEED:
    //case SMSG_INITIALIZE_FACTIONS:
    //case SMSG_ACTION_BUTTONS:
    //case SMSG_SEND_UNLEARN_SPELLS:
    //case SMSG_INITIAL_SPELLS:
    //case SMSG_TALENTS_INFO:
    //case SMSG_TUTORIAL_FLAGS:
    //case SMSG_SET_PROFICIENCY:
    //case SMSG_BINDPOINTUPDATE:
    //case SMSG_INSTANCE_DIFFICULTY:
    //case SMSG_MOTD:
    //case SMSG_MESSAGECHAT:
    //case MSG_SET_RAID_DIFFICULTY:
    //case MSG_SET_DUNGEON_DIFFICULTY:
    //case SMSG_CONTACT_LIST:
    //case SMSG_ACCOUNT_DATA_TIMES:
    //case SMSG_FEATURE_SYSTEM_STATUS:
    //case SMSG_LOGIN_VERIFY_WORLD:
        return OUTPACKET_RESULT_NOT_CONNECTED;
    }

    // Packet logger :)
    sWorldPacketLog.logPacket(static_cast<uint32_t>(len), opcode, static_cast<const uint8_t*>(data), 1, (mSession ? mSession->GetAccountId() : 0));

    if (_crypt.isInitialized())
    {
        AuthPktHeader authPktHeader(static_cast<uint32_t>(len), sOpcodeTables.getHexValueForVersionId(sOpcodeTables.getVersionIdForAEVersion(), opcode));
        _crypt.encryptWotlkSend(reinterpret_cast<uint8_t*>(&authPktHeader.raw), 4);
        rv = BurstSend(reinterpret_cast<const uint8_t*>(&authPktHeader.raw), 4);
    }
    else
    {
        ServerPktHeader serverPktHeader(static_cast<uint32_t>(len + 2), sOpcodeTables.getHexValueForVersionId(sOpcodeTables.getVersionIdForAEVersion(), opcode));
        rv = BurstSend(reinterpret_cast<const uint8_t*>(&serverPktHeader.header), serverPktHeader.headerLength);
    }

    // Pass the rest of the packet to our send buffer (if there is any)
    if (len > 0 && rv)
        rv = BurstSend(static_cast<const uint8_t*>(data), static_cast<uint32_t>(len));

    if (rv)
        BurstPush();

    BurstEnd();
    return rv ? OUTPACKET_RESULT_SUCCESS : OUTPACKET_RESULT_SOCKET_ERROR;
}
#endif


void WorldSocket::OnConnect()
{
    sWorld.increaseAcceptedConnections();
    _latency = Util::getMSTime();

#if VERSION_STRING <= WotLK
    SendPacket(SmsgAuthChallenge(mSeed).serialise().get());

#else
    WorldPacket packet(MSG_WOW_CONNECTION, 46);
    packet << "RLD OF WARCRAFT CONNECTION - SERVER TO CLIENT";
    SendPacket(&packet);
#endif
}

#if VERSION_STRING >= Cata
void WorldSocket::OnConnectTwo()
{
    SendPacket(SmsgAuthChallenge(mSeed).serialise().get());
}
#endif

void WorldSocket::_HandleAuthSession(WorldPacket* recvPacket)
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
        *recvPacket >> mClientBuild;
        *recvPacket >> AuthDigest[17];
        *recvPacket >> AuthDigest[7];
        *recvPacket >> AuthDigest[13];
        *recvPacket >> AuthDigest[15];
        *recvPacket >> AuthDigest[1];
        *recvPacket >> AuthDigest[10];

        *recvPacket >> addonSize;
        if (addonSize)
        {
            mAddonInfoBuffer.resize(addonSize);
            recvPacket->read(static_cast<uint8_t*>(mAddonInfoBuffer.contents()), addonSize);
        }

        recvPacket->readBit();
        const auto accountNameLength = recvPacket->readBits(11);
        account = recvPacket->ReadString(accountNameLength);
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
            recvPacket->read(static_cast<uint8_t*>(mAddonInfoBuffer.contents()), addonSize);
        }
        
        recvPacket->readBit();
        uint32_t accountNameLength = recvPacket->readBits(12);
        account = recvPacket->ReadString(accountNameLength);
    }
#else
    std::string account;
    uint32 unk2;

    _latency = Util::getMSTime() - _latency;

    try
    {
#if VERSION_STRING < WotLK
        *recvPacket >> mClientBuild;
        *recvPacket >> unk2;
        *recvPacket >> account;
        *recvPacket >> mClientSeed;
#else
        uint32 unk3;
        uint64 unk4;
        uint32 unk5, unk6, unk7;

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
        SendPacket(SmsgAuthResponse(AuthFailed, ARST_ONLY_ERROR).serialise().get());
        return;
    }

    // Extract account information from the packet.
    std::string AccountName;
    uint32 AccountID;
    std::string GMFlags;
    uint8 AccountFlags;
    std::string lang;

    recvData >> AccountID;
    recvData >> AccountName;
    recvData >> GMFlags;
    recvData >> AccountFlags;

    const std::string* forcedPermissions = sLogonCommHandler.getPermissionStringForAccountId(AccountID);
    if (forcedPermissions != nullptr)
        GMFlags.assign(*forcedPermissions);

    LOG_DEBUG(" >> got information packet from logon: `%s` ID %u (request %u)", AccountName.c_str(), AccountID, mRequestID);

    mRequestID = 0;

    // Pull the sessionkey we generated during the logon - client handshake
    uint8 K[40];
    recvData.read(K, 40);

#if VERSION_STRING < WotLK
    BigNumber BNK;
    BNK.SetBinary(K, 40);

#if VERSION_STRING == TBC
    uint8 *key = new uint8[20];
    WowCrypt::generateTbcKey(key, K);

    _crypt.setLegacyKey(key, 20);
    _crypt.initLegacyCrypt();
    delete[] key;
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
    hasher.Initialize();
    memcpy(buffer, abuf, 64);
    memcpy(&buffer[64], K, 40);
    hasher.UpdateData(buffer, 104);
    hasher.Finalize();
    memcpy(buffer, bbuf, 64);
    memcpy(&buffer[64], hasher.GetDigest(), 20);
    hasher.Initialize();
    hasher.UpdateData(buffer, 84);
    hasher.Finalize();

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
        recvData.read((uint8*)lang.data(), 4);
#endif

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
        SendPacket(SmsgAuthResponse(AuthUnknownAccount, ARST_ONLY_ERROR).serialise().get());
        return;
    }

    Sha1Hash sha;
#if VERSION_STRING < Cata
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

    sha.UpdateData(reinterpret_cast<uint8*>(&t), 4);
    sha.UpdateData(reinterpret_cast<uint8*>(&mClientSeed), 4);
    sha.UpdateData(reinterpret_cast<uint8*>(&mSeed), 4);
#if VERSION_STRING < WotLK
    sha.UpdateBigNumbers(&BNK, NULL);
#else
    sha.UpdateData(reinterpret_cast<uint8*>(&K), 40);
#endif
    sha.Finalize();

#if VERSION_STRING < Cata
    if (memcmp(sha.GetDigest(), digest, 20))
#else
    if (memcmp(sha.GetDigest(), AuthDigest, 20))
#endif
    {
        // AUTH_UNKNOWN_ACCOUNT = 21
        SendPacket(SmsgAuthResponse(AuthUnknownAccount, ARST_ONLY_ERROR).serialise().get());
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
    pSession->m_lastPing = static_cast<uint32>(UNIXTIME);
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
        QueryResult* pResult = CharacterDatabase.Query("SELECT * FROM account_data WHERE acct = %u", AccountID);
        if (pResult == nullptr)
            CharacterDatabase.Execute("INSERT INTO account_data VALUES(%u, '', '', '', '', '', '', '', '', '')", AccountID);
        else
        {
            for (uint8_t i = 0; i < 8; ++i)
            {
                const char* data = pResult->Fetch()[1 + i].GetString();
                size_t len = data ? strlen(data) : 0;
                if (len > 1)
                {
                    char* d = new char[len + 1];
                    memcpy(d, data, len + 1);
                    pSession->SetAccountData(i, d, true, static_cast<uint32>(len));
                }
            }

            delete pResult;
        }
    }

    LOG_DEBUG("%s from %s:%u [%ums]", AccountName.c_str(), GetRemoteIP().c_str(), GetRemotePort(), _latency);

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
        SendPacket(SmsgAuthResponse(AuthRejected, ARST_ONLY_ERROR).serialise().get());
        Disconnect();
    }

    // release delete mutex
    pSession->deleteMutex.Release();
}

void WorldSocket::Authenticate()
{
    ARCEMU_ASSERT(pAuthenticationPacket != NULL);
    mQueued = false;

    if (mSession == nullptr)
        return;

    SendPacket(SmsgAuthResponse(AuthOkay, ARST_ACCOUNT_DATA).serialise().get());
#if VERSION_STRING < Cata
    sAddonMgr.SendAddonInfoPacket(pAuthenticationPacket, static_cast<uint32>(pAuthenticationPacket->rpos()), mSession);
#else
    mSession->sendAddonInfo();
#endif
#if VERSION_STRING > TBC
    mSession->sendClientCacheVersion(BUILD_VERSION);
#endif
    mSession->_latency = _latency;

    delete pAuthenticationPacket;
    pAuthenticationPacket = nullptr;

    sWorld.addSession(mSession);
    sWorld.addGlobalSession(mSession);
}

void WorldSocket::UpdateQueuePosition(uint32 Position)
{
    SendPacket(SmsgAuthResponse(0, ARST_QUEUE, Position).serialise().get());
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
        mSession->m_lastPing = static_cast<uint32>(UNIXTIME);

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
            setsockopt(GetFd(), 0x6, 0x1, reinterpret_cast<const char*>(&arg), sizeof(arg));
            m_nagleEanbled = true;
        }
    }
    else
    {
        if (m_nagleEanbled)
        {
            u_long arg = 1;
            setsockopt(GetFd(), 0x6, 0x1, reinterpret_cast<const char*>(&arg), sizeof(arg));
            m_nagleEanbled = false;
        }
    }
#endif
}

void WorldSocket::OnRead()
{
    for (;;)
    {
#if VERSION_STRING != Mop
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
            readBuffer.Read(reinterpret_cast<uint8*>(&Header), 6);

            // Decrypt the header
#if VERSION_STRING < WotLK
            _crypt.decryptLegacyReceive((uint8*)&Header, sizeof(ClientPktHeader));
#else
            _crypt.decryptWotlkReceive(reinterpret_cast<uint8*>(&Header), sizeof(ClientPktHeader));
#endif

            mRemaining = mSize = ntohs(Header.size) - 4;
            mOpcode = sOpcodeTables.getInternalIdForHex(Header.cmd);
        }
#else
        if (mRemaining == 0)
        {
            if (_crypt.isInitialized())
            {
                if (readBuffer.GetSize() < 4)
                {
                    return;
                }

                AuthPktHeader authPktHeader(0, 0);
                readBuffer.Read(reinterpret_cast<uint8_t*>(&authPktHeader.raw), 4);
                _crypt.decryptWotlkReceive(reinterpret_cast<uint8_t*>(&authPktHeader.raw), 4);

                mRemaining = mSize = authPktHeader.getSize();
                mOpcode = sOpcodeTables.getInternalIdForHex(authPktHeader.getOpcode());
            }
            else
            {
                if (readBuffer.GetSize() < 6)
                {
                    return;
                }

                ClientPktHeader clientPktHeader;
                readBuffer.Read(reinterpret_cast<uint8_t*>(&clientPktHeader), 6);
                _crypt.decryptWotlkReceive(reinterpret_cast<uint8_t*>(&clientPktHeader), sizeof(ClientPktHeader));

                mRemaining = mSize = clientPktHeader.size -= 4;
                mOpcode = sOpcodeTables.getInternalIdForHex(clientPktHeader.cmd);
            }
        }
#endif

        if (mRemaining > 0)
        {
            if (readBuffer.GetSize() < mRemaining)
            {
                // We have a fragmented packet. Wait for the complete one before proceeding.
                return;
            }
        }

        WorldPacket* packet = new WorldPacket(sOpcodeTables.getHexValueForVersionId(sOpcodeTables.getVersionIdForAEVersion(), mOpcode), mSize);
        packet->resize(mSize);

        if (mRemaining > 0)
        {
            // Copy from packet buffer into our actual buffer.
            ///Read(mRemaining, (uint8*)Packet->contents());
            readBuffer.Read(static_cast<uint8*>(packet->contents()), mRemaining);
        }

        sWorldPacketLog.logPacket(mSize, mOpcode, mSize ? packet->contents() : nullptr, 0, (mSession ? mSession->GetAccountId() : 0));
        mRemaining = mSize = /*mOpcode =*/ 0;

        // Check for packets that we handle
        switch (sOpcodeTables.getInternalIdForHex(packet->GetOpcode()))
        {
            case CMSG_PING:
            {
                _HandlePing(packet);
                delete packet;
            }
            break;
#if VERSION_STRING >= Cata
            case MSG_WOW_CONNECTION:
            {
                HandleWoWConnection(packet);
            }
            break;
#endif
            case CMSG_AUTH_SESSION:
            {
                _HandleAuthSession(packet);
            }
            break;
            default:
            {
                if (mSession)
                    mSession->QueuePacket(packet);
                else
                    delete packet;
            }
            break;
        }
    }
}

#if VERSION_STRING >= Cata
void WorldSocket::HandleWoWConnection(WorldPacket* recvPacket)
{
    std::string ClientToServerMsg;
    *recvPacket >> ClientToServerMsg;

    OnConnectTwo();
}
#endif

void WorldPacketLog::logPacket(uint32_t len, uint16_t opcode, const uint8_t* data, uint8_t direction, uint32_t accountid)
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
                    sOpcodeTables.getNameForInternalId(opcode).c_str(), sOpcodeTables.getHexValueForVersionId(sOpcodeTables.getVersionIdForAEVersion(), opcode), len);
            } break;
        }
}

    if (isLogEnabled)
    {
        mPacketLogMutex.Acquire();
        unsigned int line = 1;
        unsigned int countpos = 0;
        uint16_t lenght = static_cast<uint16_t>(len);

        fprintf(mPacketLogFile, "{%s} Packet: (0x%04X) %s PacketSize = %u stamp = %u accountid = %u\n", (direction ? "SERVER" : "CLIENT"), 
            sOpcodeTables.getHexValueForVersionId(sOpcodeTables.getVersionIdForAEVersion(), opcode),
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
