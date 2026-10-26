/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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

#include "Cryptography/RC4.hpp"
#include "LogonCommDefines.hpp"
#include "Network/Socket.hpp"
#include "zlib.h"

class LogonCommServerSocket : public Socket
{
public:
    LogonCommServerSocket(SOCKET fd);
    ~LogonCommServerSocket() = default;

    //////////////////////////////////////////////////////////////////////////////////////////
    // virtual functions (Socket)
    void onRead() override;
    void onDisconnect() override;
    void onConnect() override;

    void SendPacket(WorldPacket* data);
    // announces a Battle.net session to the realm (LRSMSG_PENDING_WORLD_SESSION)
    void SendPendingSession(const PendingWorldSession& session);
    void HandlePacket(WorldPacket& recvData);

    void HandleRegister(WorldPacket& recvData);
    void HandlePing(WorldPacket& recvData);
    void HandleBuildRequest(WorldPacket& recvData);
    void HandleSessionRequest(WorldPacket& recvData);
    void HandleSQLExecute(WorldPacket& recvData);
    void HandleReloadAccounts(WorldPacket& recvData);
    void HandleAuthChallenge(WorldPacket& recvData);
    void HandleMappingReply(WorldPacket& recvData);
    void HandleUpdateMapping(WorldPacket& recvData);
    void HandleTestConsoleLogin(WorldPacket& recvData);
    void HandleDatabaseModify(WorldPacket& recvData);
    void HandleBattlePetRequest(WorldPacket& recvData);
    void HandleBattlePetSave(WorldPacket& recvData);
    void HandleBattlePetAdd(WorldPacket& recvData);
    void HandleAchievementRequest(WorldPacket& recvData);
    void HandleAchievementSave(WorldPacket& recvData);
    void HandleAchievementReset(WorldPacket& recvData);
    void HandlePopulationRespond(WorldPacket& recvData);
    void HandleRequestCheckAccount(WorldPacket& recvData);
    void HandleRequestAllAccounts(WorldPacket& recvData);

    void RefreshRealmsPop();

    uint32_t authenticated;
    bool use_crypto;

    std::atomic<unsigned long> last_ping;
    bool removed;
    std::set<uint32_t> server_ids;

private:
    // one row in logoncomm_log for every request of a realm, the handler adds account and detail
    void writeCommLog(uint32_t logOpcode);

    uint32_t m_logAccountId = 0;
    std::string m_logDetail;

    uint32_t remaining;
    uint16_t opcode;
    uint32_t seed;

    AscEmu::RC4Engine _sendCrypto;
    AscEmu::RC4Engine _rwCrypto;
};

typedef void (LogonCommServerSocket::*logonpacket_handler)(WorldPacket&);

#endif  // __LOGON_COMM_SERVER_H
