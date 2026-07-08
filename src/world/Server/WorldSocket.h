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
 */

#ifndef WORLDSOCKET_H
#define WORLDSOCKET_H

#include "Cryptography/WowCrypt.hpp"
#include "Network/WorldPacket.hpp"
#include "Network/Network.hpp"
#include "AEVersion.hpp"
#include "ClientProtocol.hpp"
#include "Threading/ThreadSafeQueue.hpp"

#include <string>

#define WORLDSOCKET_SENDBUF_SIZE 131078
#define WORLDSOCKET_RECVBUF_SIZE 16384

class SocketHandler;
class WorldSession;

enum OUTPACKET_RESULT
{
    OUTPACKET_RESULT_SUCCESS = 1,
    OUTPACKET_RESULT_NO_ROOM_IN_BUFFER = 2,
    OUTPACKET_RESULT_NOT_CONNECTED = 3,
    OUTPACKET_RESULT_SOCKET_ERROR = 4,
};

class SERVER_DECL WorldSocket : public Socket
{
public:
    WorldSocket(SOCKET fd);
    ~WorldSocket();

    //////////////////////////////////////////////////////////////////////////////////////////
    // virtual functions (Socket)
    void onRead() override;
    void onConnect() override;
    void onDisconnect() override;

    // helper
    inline void setClientProtocol(WoW::ClientProtocolState protocol)
    {
        m_protocol = protocol;
    }
    void setCurrentVersionAsProtocol();

    void sendClientConnectionPacket();

    // vs8 fix - send null on empty buffer
    inline void SendPacket(WorldPacket* packet) { if (!packet) return; OutPacket(packet->getOpcode(), packet->size(), (packet->size() ? (const void*)packet->contents() : NULL)); }

    void OutPacket(uint32_t opcode, size_t len, const void* data);
    OUTPACKET_RESULT _OutPacket(uint32_t opcode, size_t len, const void* data);

    inline uint32_t GetLatency() { return _latency; }

    void Authenticate(std::unique_ptr<WorldSession> sessionHolder);

    void UpdateQueuePosition(uint32_t Position);

    inline void SetSession(WorldSession* session) { mSession = session; }
    inline WorldSession* GetSession() { return mSession; }

    void UpdateQueuedPackets();

    void sendAuthChallengePacket();
    void sendVerifyConnectPacket();

    bool processHeader();

    void dispatchPacket(std::unique_ptr<WorldPacket> packet);

protected:
    void _handleAuthSession(std::unique_ptr<WorldPacket> recvPacket);
    void _handlePing(std::unique_ptr<WorldPacket> recvPacket);
    void _handleMsgVerifyConnection(std::unique_ptr<WorldPacket> recvPacket);

public:
    // used by LogonCommClient
    void informationRetreiveCallback(WorldPacket& recvData, uint32_t requestid);
    bool isAuthenticated{false};
    void setClientProtocolByBuild(uint32_t build);
    bool m_protocolSetByLogonComm{ true };

private:
    uint32_t mOpcode;
    uint32_t mRemaining;
    uint32_t mSize;
    uint32_t mSeed;
    uint32_t mClientSeed;
    
    uint32_t mRequestID;

    WorldSession* mSession;
    ThreadSafeQueue<std::unique_ptr<WorldPacket>> _queue;

    WowCrypt _crypt;
    uint32_t _latency;
    bool mQueued;
    bool m_nagleEanbled;
    std::string m_accountName;

    ByteBuffer mAddonInfoBuffer;

    WoW::ClientProtocolState m_protocol{};
    bool m_HandshakeReceived{false};

    uint8_t AuthDigest[20];
    uint32_t mClientBuild;
};

static inline void FastGUIDPack(ByteBuffer & buf, const uint64_t & oldguid)
{
    // hehe speed freaks
    uint8_t guidmask = 0;
    uint8_t guidfields[9] = {0, 0, 0, 0, 0, 0, 0, 0};

    int j = 1;
    uint8_t* test = (uint8_t*)&oldguid;

    if (*test)  //7*8
    {
        guidfields[j] = *test;
        guidmask |= 1;
        j++;
    }
    if (*(test + 1)) //6*8
    {
        guidfields[j] = *(test + 1);
        guidmask |= 2;
        j++;
    }
    if (*(test + 2)) //5*8
    {
        guidfields[j] = *(test + 2);
        guidmask |= 4;
        j++;
    }
    if (*(test + 3)) //4*8
    {
        guidfields[j] = *(test + 3);
        guidmask |= 8;
        j++;
    }
    if (*(test + 4)) //3*8
    {
        guidfields[j] = *(test + 4);
        guidmask |= 16;
        j++;
    }
    if (*(test + 5)) //2*8
    {
        guidfields[j] = *(test + 5);
        guidmask |= 32;
        j++;
    }
    if (*(test + 6)) //1*8
    {
        guidfields[j] = *(test + 6);
        guidmask |= 64;
        j++;
    }
    if (*(test + 7)) //0*8
    {
        guidfields[j] = *(test + 7);
        guidmask |= 128;
        j++;
    }
    guidfields[0] = guidmask;

    buf.append(guidfields, j);
}

//!!! warning. This presumes that all guids can be compressed at least 1 byte
//make sure you choose highguids accordingly
static inline unsigned int FastGUIDPack(const uint64_t & oldguid, unsigned char* buffer, uint32_t pos)
{
    // hehe speed freaks
    uint8_t guidmask = 0;

    int j = 1 + pos;

    uint8_t* test = (uint8_t*)&oldguid;

    if (*test)  //7*8
    {
        buffer[j] = *test;
        guidmask |= 1;
        j++;
    }
    if (*(test + 1)) //6*8
    {
        buffer[j] = *(test + 1);
        guidmask |= 2;
        j++;
    }
    if (*(test + 2)) //5*8
    {
        buffer[j] = *(test + 2);
        guidmask |= 4;
        j++;
    }
    if (*(test + 3)) //4*8
    {
        buffer[j] = *(test + 3);
        guidmask |= 8;
        j++;
    }
    if (*(test + 4)) //3*8
    {
        buffer[j] = *(test + 4);
        guidmask |= 16;
        j++;
    }
    if (*(test + 5)) //2*8
    {
        buffer[j] = *(test + 5);
        guidmask |= 32;
        j++;
    }
    if (*(test + 6)) //1*8
    {
        buffer[j] = *(test + 6);
        guidmask |= 64;
        j++;
    }
    if (*(test + 7)) //0*8
    {
        buffer[j] = *(test + 7);
        guidmask |= 128;
        j++;
    }
    buffer[pos] = guidmask;
    return (j - pos);
}

#endif      //WORLDSOCKET_H
