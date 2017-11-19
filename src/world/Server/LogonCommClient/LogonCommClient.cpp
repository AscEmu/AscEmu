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
#include "StdAfx.h"
#include "CommonTypes.hpp"
#include "LogonCommClient.h"
#include "LogonCommHandler.h"
#include "WorldPacket.h"
#include "Database/Database.h"
#include "Server/MainServerDefines.h"
#include <set>
#include <map>
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "LogonCommDefines.h"


LogonCommClientSocket::LogonCommClientSocket(SOCKET fd) : Socket(fd, 724288, 262444)
{
    // do nothing
    last_ping = last_pong = (uint32)UNIXTIME;
    remaining = opcode = 0;
    _id = 0;
    latency = 0;
    use_crypto = false;
    authenticated = 0;
    pingtime = 0;

    LOG_DEBUG("Create new LogonCommClientSocket %u", m_fd);
}

void LogonCommClientSocket::OnRead()
{
    while (true)
    {
        if (!remaining)
        {
            if (readBuffer.GetSize() < 6)
                return;     // no header

            // read header
            readBuffer.Read((uint8*)&opcode, 2);
            readBuffer.Read((uint8*)&remaining, 4);

            // decrypt the first two bytes
            if (use_crypto)
            {
                _recvCrypto.Process((uint8*)&opcode, (uint8*)&opcode, 2);
                _recvCrypto.Process((uint8*)&remaining, (uint8*)&remaining, 4);
            }

            // convert network byte order
            byteSwapUInt32(&remaining);
        }

        // do we have a full packet?
        if (readBuffer.GetSize() < remaining)
            return;

        // create the buffer
        WorldPacket buff(opcode, remaining);
        if (remaining)
        {
            buff.resize(remaining);
            //Read(remaining, (uint8*)buff.contents());
            readBuffer.Read((uint8*)buff.contents(), remaining);
        }

        // decrypt the rest of the packet
        if (use_crypto && remaining)
            _recvCrypto.Process((unsigned char*)buff.contents(), (unsigned char*)buff.contents(), remaining);

        // handle the packet
        HandlePacket(buff);

        remaining = 0;
        opcode = 0;
    }
}

void LogonCommClientSocket::HandlePacket(WorldPacket& recvData)
{
    static logonpacket_handler Handlers[LRMSG_MAX_OPCODES] =
    {
        NULL,
        NULL,                                                   // LRCMSG_REALM_REGISTER_REQUEST
        &LogonCommClientSocket::HandleRegister,                 // LRSMSG_REALM_REGISTER_RESULT
        NULL,                                                   // LRCMSG_ACC_SESSION_REQUEST
        &LogonCommClientSocket::HandleSessionInfo,              // LRSMSG_ACC_SESSION_RESULT
        NULL,                                                   // LRCMSG_LOGON_PING_STATUS
        &LogonCommClientSocket::HandlePong,                     // LRSMSG_LOGON_PING_RESULT
        NULL,                                                   // LRCMSG_FREE_01
        NULL,                                                   // LRSMSG_FREE_02
        NULL,                                                   // LRCMSG_AUTH_REQUEST
        &LogonCommClientSocket::HandleAuthResponse,             // LRSMSG_AUTH_RESPONSE
        &LogonCommClientSocket::HandleRequestAccountMapping,    // LRSMSG_ACC_CHAR_MAPPING_REQUEST
        NULL,                                                   // LRCMSG_ACC_CHAR_MAPPING_RESULT
        NULL,                                                   // LRCMSG_ACC_CHAR_MAPPING_UPDATE
        &LogonCommClientSocket::HandleDisconnectAccount,        // LRSMSG_SEND_ACCOUNT_DISCONNECT
        NULL,                                                   // LRCMSG_LOGIN_CONSOLE_REQUEST
        &LogonCommClientSocket::HandleConsoleAuthResult,        // LRSMSG_LOGIN_CONSOLE_RESULT
        NULL,                                                   // LRCMSG_ACCOUNT_DB_MODIFY_REQUEST
        &LogonCommClientSocket::HandleModifyDatabaseResult,     // LRSMSG_ACCOUNT_DB_MODIFY_RESULT
        &LogonCommClientSocket::HandlePopulationRequest,        // LRSMSG_REALM_POPULATION_REQUEST
        NULL,                                                   // LRCMSG_REALM_POPULATION_RESULT
        NULL,                                                   // LRCMSG_ACCOUNT_REQUEST
        &LogonCommClientSocket::HandleResultCheckAccount,       // LRSMSG_ACCOUNT_RESULT
        NULL,                                                   // LRCMSG_ALL_ACCOUNT_REQUEST
        &LogonCommClientSocket::HandleResultAllAccount,         // LRSMSG_ALL_ACCOUNT_RESULT
    };

    if (recvData.GetOpcode() >= LRMSG_MAX_OPCODES || Handlers[recvData.GetOpcode()] == 0)
    {
        LOG_ERROR("Got unknown packet from logoncomm: %u", recvData.GetOpcode());
        return;
    }

    (this->*(Handlers[recvData.GetOpcode()]))(recvData);
}

void LogonCommClientSocket::HandleRegister(WorldPacket& recvData)
{
    uint32 realmlid;
    uint32 error;
    std::string realmname;

    recvData >> error;
    recvData >> realmlid;
    recvData >> realmname;

    LogDefault("Realm `%s` registered as realm %u.", realmname.c_str(), realmlid);

    LogonCommHandler::getSingleton().addRealmToRealmlistResult(_id, realmlid);
    realm_ids.insert(realmlid);
}

void LogonCommClientSocket::HandleSessionInfo(WorldPacket& recvData)
{
    uint32 request_id;
    recvData >> request_id;

    Mutex & m = sLogonCommHandler.getPendingLock();
    m.Acquire();

    // find the socket with this request
    WorldSocket* sock = sLogonCommHandler.getWorldSocketForClientRequestId(request_id);
    if (sock == nullptr || sock->Authed || !sock->IsConnected())       // Expired/Client disconnected
    {
        m.Release();
        return;
    }

    // extract sessionkey / account information (done by WS)
    sock->Authed = true;
    sLogonCommHandler.removeUnauthedClientSocket(request_id);
    sock->InformationRetreiveCallback(recvData, request_id);
    m.Release();
}

void LogonCommClientSocket::HandlePong(WorldPacket& /*recvData*/)
{
    latency = Util::getMSTime() - pingtime;
    last_pong = (uint32)UNIXTIME;
}

void LogonCommClientSocket::SendPing()
{
    pingtime = Util::getMSTime();
    WorldPacket data(LRCMSG_LOGON_PING_STATUS, 4);
    SendPacket(&data, false);

    last_ping = (uint32)UNIXTIME;
}

void LogonCommClientSocket::SendPacket(WorldPacket* data, bool no_crypto)
{
    LogonWorldPacket header;
    bool rv;
    if (!IsConnected() || IsDeleted())
        return;

    BurstBegin();

    header.opcode = data->GetOpcode();
    //header.size   = ntohl((u_long)data->size());
    header.size = (uint32)data->size();
    byteSwapUInt32(&header.size);


    if (use_crypto && !no_crypto)
        _sendCrypto.Process((unsigned char*)&header, (unsigned char*)&header, 6);

    rv = BurstSend((const uint8*)&header, 6);

    if (data->size() > 0 && rv)
    {
        if (use_crypto && !no_crypto)
            _sendCrypto.Process((unsigned char*)data->contents(), (unsigned char*)data->contents(), (unsigned int)data->size());

        rv = BurstSend((const uint8*)data->contents(), (uint32)data->size());
    }

    if (rv) BurstPush();
    BurstEnd();
}

void LogonCommClientSocket::OnDisconnect()
{
    if (_id != 0)
    {
        LOG_DETAIL("Calling ConnectionDropped() due to OnDisconnect().");
        sLogonCommHandler.dropLogonServerConnection(_id);
    }
}

LogonCommClientSocket::~LogonCommClientSocket()
{}

void LogonCommClientSocket::SendChallenge()
{
    uint8* key = sLogonCommHandler.sql_passhash;

    _recvCrypto.Setup(key, 20);
    _sendCrypto.Setup(key, 20);

    // packets are encrypted from now on
    use_crypto = true;

    WorldPacket data(LRCMSG_AUTH_REQUEST, 20);
    data.append(key, 20);
    SendPacket(&data, true);
}

void LogonCommClientSocket::HandleAuthResponse(WorldPacket& recvData)
{
    uint8 result;
    recvData >> result;
    if (result != 1)
    {
        authenticated = 0xFFFFFFFF;
    }
    else
    {
        authenticated = 1;
    }
}

void LogonCommClientSocket::UpdateAccountCount(uint32 account_id, uint8 add)
{
    WorldPacket data(LRCMSG_ACC_CHAR_MAPPING_UPDATE, 9);
    std::set<uint32>::iterator itr = realm_ids.begin();

    for (; itr != realm_ids.end(); ++itr)
    {
        data.clear();
        data << (*itr);
        data << account_id;
        data << add;
        SendPacket(&data, false);
    }
}

void LogonCommClientSocket::HandleRequestAccountMapping(WorldPacket& recvData)
{
    auto startTime = Util::TimeNow();
    uint32 realm_id;
    uint32 account_id;
    QueryResult* result;
    std::map<uint32, uint8> mapping_to_send;
    std::map<uint32, uint8>::iterator itr;

    // grab the realm id
    recvData >> realm_id;

    // fetch the character mapping
    result = CharacterDatabase.Query("SELECT acct FROM characters");

    if (result)
    {
        do
        {
            account_id = result->Fetch()[0].GetUInt32();
            itr = mapping_to_send.find(account_id);
            if (itr != mapping_to_send.end())
                itr->second++;
            else
                mapping_to_send.insert(std::make_pair(account_id, 1));
        }
        while (result->NextRow());
        delete result;
    }

    if (!mapping_to_send.size())
    {
        // no point sending empty shit
        return;
    }

    ByteBuffer uncompressed(40000 * 5 + 8);
    //uint32 Count = 0;
    uint32 Remaining = (uint32)mapping_to_send.size();
    itr = mapping_to_send.begin();
    for (;;)
    {
        // Send no more than 40000 characters at once.
        uncompressed << realm_id;

        if (Remaining > 40000)
            uncompressed << uint32(40000);
        else
            uncompressed << Remaining;

        for (uint32 i = 0; i < 40000; ++i, ++itr)
        {
            uncompressed << uint32(itr->first) << uint8(itr->second);
            if (!--Remaining)
                break;
        }

        CompressAndSend(uncompressed);
        if (!Remaining)
            break;

        uncompressed.clear();
    }
    LogNotice("LogonCommClient : Build character mapping in %u ms. (%u)", Util::GetTimeDifferenceToNow(startTime), mapping_to_send.size());
}

void LogonCommClientSocket::CompressAndSend(ByteBuffer& uncompressed)
{
    // I still got no idea where this came from :p
    size_t destsize = uncompressed.size() + uncompressed.size() / 10 + 16;

    // w000t w000t kat000t for gzipped packets
    WorldPacket data(LRCMSG_ACC_CHAR_MAPPING_RESULT, destsize + 4);
    data.resize(destsize + 4);

    z_stream stream;
    stream.zalloc = 0;
    stream.zfree = 0;
    stream.opaque = 0;

    if (deflateInit(&stream, 1) != Z_OK)
    {
        LOG_ERROR("deflateInit failed.");
        return;
    }

    // set up stream pointers
    stream.next_out = (Bytef*)((uint8*)data.contents()) + 4;
    stream.avail_out = (uInt)destsize;
    stream.next_in = (Bytef*)uncompressed.contents();
    stream.avail_in = (uInt)uncompressed.size();

    // call the actual process
    if (deflate(&stream, Z_NO_FLUSH) != Z_OK ||
        stream.avail_in != 0)
    {
        LOG_ERROR("deflate failed.");
        return;
    }

    // finish the deflate
    if (deflate(&stream, Z_FINISH) != Z_STREAM_END)
    {
        LOG_ERROR("deflate failed: did not end stream");
        return;
    }

    // finish up
    if (deflateEnd(&stream) != Z_OK)
    {
        LOG_ERROR("deflateEnd failed.");
        return;
    }

    *(uint32*)data.contents() = (uint32)uncompressed.size();

    data.resize(stream.total_out + 4);
    SendPacket(&data, false);
}

void LogonCommClientSocket::HandleDisconnectAccount(WorldPacket& recvData)
{
    uint32 id;
    recvData >> id;

    WorldSession* sess = sWorld.getSessionByAccountId(id);
    if (sess != NULL)
        sess->Disconnect();
}

void ConsoleAuthCallback(uint32_t request, uint32_t result);
void LogonCommClientSocket::HandleConsoleAuthResult(WorldPacket& recvData)
{
    uint32 requestid;
    uint32 result;

    recvData >> requestid;
    recvData >> result;

    ConsoleAuthCallback(requestid, result);
}

void LogonCommClientSocket::HandlePopulationRequest(WorldPacket& recvData)
{
    uint32 realmId;
    // Grab the realm id
    recvData >> realmId;

    // Send the result
    WorldPacket data(LRCMSG_REALM_POPULATION_RESULT, 16);
    data << realmId;
    data << LogonCommHandler::getSingleton().getRealmPopulation();
    SendPacket(&data, false);
}

void LogonCommClientSocket::HandleModifyDatabaseResult(WorldPacket& recvData)
{
    uint32 method_id;
    uint8 result_id;
    //Get the result/method id for further processing
    recvData >> method_id;
    recvData >> result_id;

    switch (method_id)
    {
        case Method_Account_Change_PW:
        {
            std::string account_name;
            recvData >> account_name;
            const char* account_string = account_name.c_str();

            WorldSession* pSession = sWorld.getSessionByAccountName(account_string);
            if (pSession == nullptr)
            {
                LOG_ERROR("No session found!");
                return;
            }

            if (result_id == Result_Account_PW_wrong)
            {
                pSession->SystemMessage("Your entered old password did not match database password!");
            }
            else if (result_id == Result_Account_SQL_error)
            {
                pSession->SystemMessage("Something went wrong by updating mysql data!");
            }
            else if (result_id == Result_Account_Finished)
            {
                pSession->SystemMessage("Your password is now updated");
            }
            else
            {
                LOG_ERROR("HandleModifyDatabaseResult: Unknown logon result in Method_Account_Change_PW");
            }

        }break;
        case Method_Account_Create:
        {
            std::string account_name;
            std::string created_name;

            recvData >> account_name;
            recvData >> created_name;

            const char* account_string = account_name.c_str();
            const char* created_string = created_name.c_str();

            if (account_name.compare("none") != 0)
            {
                WorldSession* pSession = sWorld.getSessionByAccountName(account_string);
                if (pSession == nullptr)
                {
                    LOG_ERROR("No session found!");
                    return;
                }

                if (result_id == Result_Account_Exists)
                {
                    pSession->SystemMessage("Account name: '%s' already in use!", created_string);
                }
                else if (result_id == Result_Account_Finished)
                {
                    pSession->SystemMessage("Account: '%s' created", created_string);
                }
                else
                {
                    LOG_ERROR("HandleModifyDatabaseResult: Unknown logon result in Method_Account_Create");
                }
            }

        }break;
    }
}

void LogonCommClientSocket::HandleResultCheckAccount(WorldPacket& recvData)
{
    uint8 result_id;
    std::string account_name;
    std::string request_name;

    recvData >> result_id;      //Get the result id for further processing
    recvData >> account_name;
    recvData >> request_name;

    //transform std::string to const char
    const char* request_string = request_name.c_str();
    const char* account_string = account_name.c_str();

    WorldSession* session_name = sWorld.getSessionByAccountName(request_string);
    if (session_name == nullptr)
    {
        if (request_name.compare("none") != 0)
        {
            LOG_ERROR("Receiver %s not found!", request_string);
            return;
        }
    }

    switch (result_id)
    {
        case 1:     // Account not available
        {
            if (request_name.compare("none") != 0)
                session_name->SystemMessage("Account: %s not found in database!", account_string);
        } break;
        case 2:     // No additional data set
        {
            if (request_name.compare("none") != 0)
                session_name->SystemMessage("No gmlevel set for account: %s !", account_string);
        } break;
        case 3:     // Everything is okay
        {
            std::string gmlevel;
            recvData >> gmlevel;

            uint32 accountId;
            recvData >> accountId;

            if (gmlevel.compare("0") != 0)
            {
                //Update account_permissions
                CharacterDatabase.Execute("REPLACE INTO account_permissions (`id`, `permissions`, `name`) VALUES (%u, '%s', '%s')", accountId, gmlevel.c_str(), account_string);
                if (request_name.compare("none") != 0)
                    session_name->SystemMessage("Account permissions has been updated to '%s' for account '%s' (%u). The change will be effective immediately.", gmlevel.c_str(), account_string, accountId);

                //Update forcedpermission map
                sLogonCommHandler.setAccountPermission(accountId, gmlevel.c_str());

                //Write info to gmlog
                if (request_name.compare("none") != 0)
                    sGMLog.writefromsession(session_name, "set account %s (%u) permissions to %s", account_string, accountId, gmlevel.c_str());

                //Send information to updated account
                WorldSession* updated_account_session = sWorld.getSessionByAccountName(account_string);
                if (updated_account_session != nullptr)
                {
                    updated_account_session->SystemMessage("Your permissions has been updated! Please reconnect your account.");
                }
            }
            else
            {
                //Update account_permissions
                CharacterDatabase.Execute("DELETE FROM account_permissions WHERE id = %u", accountId);
                if (request_name.compare("none") != 0)
                    session_name->SystemMessage("Account permissions removed for account '%s' (%u). The change will be effective immediately.", account_string, accountId);

                //Update forcedpermission map
                sLogonCommHandler.removeAccountPermission(accountId);

                //Write info to gmlog
                if (request_name.compare("none") != 0)
                    sGMLog.writefromsession(session_name, "removed permissions for account %s (%u)", account_string, accountId);

                //Send information to updated account
                WorldSession* updated_account_session = sWorld.getSessionByAccountName(account_string);
                if (updated_account_session != nullptr)
                {
                    sWorld.disconnectSessionByAccountName(account_string, session_name);
                }
            }

        } break;
        case 4:     // Account ID
        {
            uint32 account_id;
            recvData >> account_id;

            if (request_name.compare("none") != 0)
                session_name->SystemMessage("Account '%s' has account ID %u.", account_string, account_id);
        }
    }
}

void LogonCommClientSocket::HandleResultAllAccount(WorldPacket& recvData)
{
    std::string accountResult;
    recvData >> accountResult;

    sLogonCommHandler.accountResult = accountResult;
}
