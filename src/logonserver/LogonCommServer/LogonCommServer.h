/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

#ifndef __LOGON_COMM_SERVER_H
#define __LOGON_COMM_SERVER_H

#include <RC4Engine.h>
#include "zlib.h"

class LogonCommServerSocket : public Socket
{
    uint32_t remaining;
    uint16_t opcode;
    uint32_t seed;
    RC4Engine sendCrypto;
    RC4Engine recvCrypto;

    public:

        uint32_t authenticated;
        bool use_crypto;

        LogonCommServerSocket(SOCKET fd);
        ~LogonCommServerSocket();

        void OnRead();
        void OnDisconnect();
        void OnConnect();
        void SendPacket(WorldPacket* data);
        void HandlePacket(WorldPacket & recvData);

        void HandleRegister(WorldPacket& recvData);
        void HandlePing(WorldPacket& recvData);
        void HandleSessionRequest(WorldPacket& recvData);
        void HandleSQLExecute(WorldPacket& recvData);
        void HandleReloadAccounts(WorldPacket& recvData);
        void HandleAuthChallenge(WorldPacket& recvData);
        void HandleMappingReply(WorldPacket& recvData);
        void HandleUpdateMapping(WorldPacket& recvData);
        void HandleTestConsoleLogin(WorldPacket& recvData);
        void HandleDatabaseModify(WorldPacket& recvData);
        void HandlePopulationRespond(WorldPacket& recvData);
        void HandleRequestCheckAccount(WorldPacket& recvData);
        void HandleRequestAllAccounts(WorldPacket& recvData);

        void RefreshRealmsPop();

        std::atomic<unsigned long> last_ping;
        bool removed;
        std::set<uint32_t> server_ids;
};

typedef void (LogonCommServerSocket::*logonpacket_handler)(WorldPacket&);

#endif  // __LOGON_COMM_SERVER_H
