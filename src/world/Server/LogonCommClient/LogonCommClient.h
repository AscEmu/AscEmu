/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#ifndef LOGON_COMM_CLIENT_H
#define LOGON_COMM_CLIENT_H

#include "Network/Socket.h"
#include <Cryptography/RC4.hpp>

class ByteBuffer;

class LogonCommClientSocket : public Socket
{
    uint32_t remaining;
    uint16_t opcode;

    AscEmu::RC4Engine _rwCrypto;
    AscEmu::RC4Engine _sendCrypto;

    public:
        LogonCommClientSocket(SOCKET fd);
        ~LogonCommClientSocket();

        void OnRead();
        void SendPacket(WorldPacket* data, bool no_crypto);
        void HandlePacket(WorldPacket& recvData);
        void SendPing();
        void SendChallenge();
        void HandleAuthResponse(WorldPacket& recvData);

        void HandleRegister(WorldPacket& recvData);
        void HandlePong(WorldPacket& recvData);
        void HandleSessionInfo(WorldPacket& recvData);
        void HandleRequestAccountMapping(WorldPacket& recvData);
        void UpdateAccountCount(uint32_t account_id, uint8_t add);
        void HandleDisconnectAccount(WorldPacket& recvData);
        void HandleConsoleAuthResult(WorldPacket& recvData);
        void HandlePopulationRequest(WorldPacket& recvData);
        void HandleModifyDatabaseResult(WorldPacket& recvData);
        void HandleResultCheckAccount(WorldPacket& recvData);
        void HandleResultAllAccount(WorldPacket& recvData);

        void OnDisconnect();
        void CompressAndSend(ByteBuffer& uncompressed);
        uint32_t last_ping;
        uint32_t last_pong;

        uint32_t pingtime;
        uint32_t latency;
        uint32_t _id;
        uint32_t authenticated;
        bool use_crypto;
        std::set<uint32_t> realm_ids;
};

typedef void (LogonCommClientSocket::*logonpacket_handler)(WorldPacket&);

#endif // LOGON_COMM_CLIENT_H
