/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

#include "CommonTypes.hpp"
#include "ByteBuffer.h"
#include "Network/Socket.h"
#include "LogonCommDefines.h"
#include "../shared/Log.hpp"
#include <RC4Engine.h>
#include "zlib.h"

class LogonCommClientSocket : public Socket
{
    uint32 remaining;
    uint16 opcode;
    RC4Engine _sendCrypto;
    RC4Engine _recvCrypto;

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
        void UpdateAccountCount(uint32 account_id, uint8 add);
        void HandleDisconnectAccount(WorldPacket& recvData);
        void HandleConsoleAuthResult(WorldPacket& recvData);
        void HandlePopulationRequest(WorldPacket& recvData);
        void HandleModifyDatabaseResult(WorldPacket& recvData);
        void HandleResultCheckAccount(WorldPacket& recvData);
        void HandleResultAllAccount(WorldPacket& recvData);

        void OnDisconnect();
        void CompressAndSend(ByteBuffer& uncompressed);
        uint32 last_ping;
        uint32 last_pong;

        uint32 pingtime;
        uint32 latency;
        uint32 _id;
        uint32 authenticated;
        bool use_crypto;
        std::set<uint32> realm_ids;
};

typedef void (LogonCommClientSocket::*logonpacket_handler)(WorldPacket&);

#endif // LOGON_COMM_CLIENT_H
