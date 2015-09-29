/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org/>
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
 *
 */

#include "LogonStdAfx.h"
#pragma pack(push, 1)
typedef struct
{
    uint16 opcode;
    uint32 size;
} logonpacket;
#pragma pack(pop)

static void swap32(uint32* p) { *p = ((*p >> 24) & 0xff) | ((*p >> 8) & 0xff00) | ((*p << 8) & 0xff0000) | (*p << 24); }

LogonCommServerSocket::LogonCommServerSocket(SOCKET fd) : Socket(fd, 65536, 524288)
{
    // do nothing
    last_ping.SetVal((uint32)UNIXTIME);
    remaining = opcode = 0;
    removed = true;

    use_crypto = false;
    authenticated = 0;
    seed = 0;

    LOG_DETAIL("Created LogonCommServerSocket %u", m_fd);
}

LogonCommServerSocket::~LogonCommServerSocket()
{

}

void LogonCommServerSocket::OnDisconnect()
{
    LOG_DETAIL("LogonCommServerSocket::Ondisconnect event.");

    // if we're registered -> Set offline
    if (!removed)
    {
        std::set<uint32>::iterator itr = server_ids.begin();

        for (; itr != server_ids.end(); ++itr)
            sInfoCore.SetRealmOffline((*itr));

        sInfoCore.RemoveServerSocket(this);
    }
}

void LogonCommServerSocket::OnConnect()
{
    if (!IsServerAllowed(GetRemoteAddress().s_addr))
    {
        LOG_ERROR("Server connection from %s:%u DENIED, not an allowed IP.", GetRemoteIP().c_str(), GetRemotePort());
        Disconnect();
        return;
    }

    sInfoCore.AddServerSocket(this);
    removed = false;
}

void LogonCommServerSocket::OnRead()
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

            if (use_crypto)
            {
                // decrypt the packet
                recvCrypto.Process((unsigned char*)&opcode, (unsigned char*)&opcode, 2);
                recvCrypto.Process((unsigned char*)&remaining, (unsigned char*)&remaining, 4);
            }

            /* reverse byte order */
            swap32(&remaining);
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

        if (use_crypto && remaining)
            recvCrypto.Process((unsigned char*)buff.contents(), (unsigned char*)buff.contents(), remaining);

        // handle the packet
        HandlePacket(buff);

        remaining = 0;
        opcode = 0;
    }
}

void LogonCommServerSocket::HandlePacket(WorldPacket & recvData)
{
    if (authenticated == 0 && recvData.GetOpcode() != RCMSG_AUTH_CHALLENGE)
    {
        // invalid
        Disconnect();
        return;
    }

    static logonpacket_handler Handlers[RMSG_COUNT] =
    {
        NULL,                                               // RMSG_NULL
        &LogonCommServerSocket::HandleRegister,             // RCMSG_REGISTER_REALM
        NULL,                                               // RSMSG_REALM_REGISTERED
        &LogonCommServerSocket::HandleSessionRequest,       // RCMSG_REQUEST_SESSION
        NULL,                                               // RSMSG_SESSION_RESULT
        &LogonCommServerSocket::HandlePing,                 // RCMSG_PING
        NULL,                                               // RSMSG_PONG
        &LogonCommServerSocket::HandleSQLExecute,           // RCMSG_SQL_EXECUTE
        &LogonCommServerSocket::HandleReloadAccounts,       // RCMSG_RELOAD_ACCOUNTS
        &LogonCommServerSocket::HandleAuthChallenge,        // RCMSG_AUTH_CHALLENGE
        NULL,                                               // RSMSG_AUTH_RESPONSE
        NULL,                                               // RSMSG_REQUEST_ACCOUNT_CHARACTER_MAPPING
        &LogonCommServerSocket::HandleMappingReply,         // RCMSG_ACCOUNT_CHARACTER_MAPPING_REPLY
        &LogonCommServerSocket::HandleUpdateMapping,        // RCMSG_UPDATE_CHARACTER_MAPPING_COUNT
        NULL,                                               // RSMSG_DISCONNECT_ACCOUNT
        &LogonCommServerSocket::HandleTestConsoleLogin,     // RCMSG_TEST_CONSOLE_LOGIN
        NULL,                                               // RSMSG_CONSOLE_LOGIN_RESULT
        &LogonCommServerSocket::HandleDatabaseModify,       // RCMSG_MODIFY_DATABASE_REQUEST
        NULL,                                               // RSMSG_MODIFY_DATABASE_RESULT
        NULL,                                               // RSMSG_REALM_POP_REQ
        &LogonCommServerSocket::HandlePopulationRespond,    // RCMSG_REALM_POP_RES
        &LogonCommServerSocket::HandleRequestCheckAccount,  // RCMSG_CHECK_ACCOUNT_REQUEST
        NULL,                                               // RSMSG_CHECK_ACCOUNT_RESULT
    };

    if (recvData.GetOpcode() >= RMSG_COUNT || Handlers[recvData.GetOpcode()] == 0)
    {
        LOG_ERROR("Got unknwon packet from logoncomm: %u", recvData.GetOpcode());
        return;
    }

    (this->*(Handlers[recvData.GetOpcode()]))(recvData);
}

void LogonCommServerSocket::HandleRegister(WorldPacket & recvData)
{
    std::string Name;
    int32 my_id;

    recvData >> Name;
    my_id = sInfoCore.GetRealmIdByName(Name);

    if (my_id == -1)
    {
        my_id = sInfoCore.GenerateRealmID();
        sLog.outString("Registering realm `%s` under ID %u.", Name.c_str(), my_id);
    }
    else
    {
        sInfoCore.RemoveRealm(my_id);
        int new_my_id = sInfoCore.GenerateRealmID(); //socket timout will DC old id after a while, make sure it's not the one we restarted
        sLog.outString("Updating realm `%s` with ID %u to new ID %u.", Name.c_str(), my_id, new_my_id);
        my_id = new_my_id;
    }

    Realm* realm = new Realm;

    realm->flags = 0;
    realm->Icon = 0;
    realm->TimeZone = 0;
    realm->Population = 0;
    realm->Lock = 0;

    realm->Name = Name;
    realm->flags = 0;
    recvData >> realm->Address >> realm->flags >> realm->Icon >> realm->TimeZone >> realm->Population >> realm->Lock;

    sLog.outString("TEST FLAGS %u", realm->flags);


    //    uint32 my_id = sInfoCore.GenerateRealmID();
    //    sLog.outString("Registering realm `%s` under ID %u.", realm->Name.c_str(), my_id);

    // Add to the main realm list
    sInfoCore.AddRealm(my_id, realm);

    // Send back response packet.
    WorldPacket data(RSMSG_REALM_REGISTERED, 4);
    data << uint32(0);      // Error
    data << my_id;          // Realm ID
    data << realm->Name;
    SendPacket(&data);
    server_ids.insert(my_id);

    /* request character mapping for this realm */
    data.Initialize(RSMSG_REQUEST_ACCOUNT_CHARACTER_MAPPING);
    data << my_id;
    SendPacket(&data);
}

void LogonCommServerSocket::HandleSessionRequest(WorldPacket & recvData)
{
    uint32 request_id;
    std::string account_name;
    recvData >> request_id;
    recvData >> account_name;

    // get sessionkey!
    uint32 error = 0;
    Account* acct = sAccountMgr.GetAccount(account_name);
    if (acct == NULL || acct->SessionKey == NULL)
        error = 1;          // Unauthorized user.

    // build response packet
    WorldPacket data(RSMSG_SESSION_RESULT, 150);
    data << request_id;
    data << error;
    if (!error)
    {
        // Append account information.
        data << acct->AccountId;
        data << acct->UsernamePtr->c_str();
        if (!acct->GMFlags)
            data << uint8(0);
        else
            data << acct->GMFlags;

        data << acct->AccountFlags;
        data.append(acct->SessionKey, 40);
        data.append(acct->Locale, 4);
        data << acct->Muted;
    }

    SendPacket(&data);
}

void LogonCommServerSocket::HandlePing(WorldPacket & recvData)
{
    WorldPacket data(RSMSG_PONG, 4);
    SendPacket(&data);
    last_ping.SetVal((uint32)time(NULL));
}

void LogonCommServerSocket::SendPacket(WorldPacket* data)
{
    bool rv;
    BurstBegin();

    logonpacket header;
    header.opcode = data->GetOpcode();
    //header.size   = ntohl((u_long)data->size());
    header.size = (uint32)data->size();
    swap32(&header.size);

    if (use_crypto)
        sendCrypto.Process((unsigned char*)&header, (unsigned char*)&header, 6);

    rv = BurstSend((uint8*)&header, 6);

    if (data->size() > 0 && rv)
    {
        if (use_crypto)
            sendCrypto.Process((unsigned char*)data->contents(), (unsigned char*)data->contents(), (uint32)data->size());

        rv = BurstSend(data->contents(), (uint32)data->size());
    }

    if (rv) BurstPush();
    BurstEnd();
}

void LogonCommServerSocket::HandleSQLExecute(WorldPacket & recvData)
{
    /*string Query;
    recvData >> Query;
    sLogonSQL->Execute(Query.c_str());*/
    LOG_ERROR("!! WORLD SERVER IS REQUESTING US TO EXECUTE SQL. THIS IS DEPRECATED AND IS BEING IGNORED. THE SERVER WAS: %s, PLEASE UPDATE IT.", GetRemoteIP().c_str());
}

void LogonCommServerSocket::HandleReloadAccounts(WorldPacket & recvData)
{
    LOG_ERROR("!! WORLD SERVER IS REQUESTING US TO RELOAD ACCOUNTS. THIS IS DEPRECATED AND IS BEING IGNORED. THE SERVER WAS: %s, PLEASE UPDATE IT.", GetRemoteIP().c_str());
    //sAccountMgr.ReloadAccounts(true);
}

void LogonCommServerSocket::HandleAuthChallenge(WorldPacket & recvData)
{
    unsigned char key[20];
    uint32 result = 1;
    recvData.read(key, 20);

    // check if we have the correct password
    if (memcmp(key, LogonServer::getSingleton().sql_hash, 20))
        result = 0;

    sLog.outString("Authentication request from %s, result %s.", GetRemoteIP().c_str(), result ? "OK" : "FAIL");

    std::stringstream sstext;
    sstext << "Key: ";
    char buf[3];
    for (int i = 0; i < 20; ++i)
    {
        snprintf(buf, 3, "%.2X", key[i]);
        sstext << buf;
    }
    LOG_DETAIL(sstext.str().c_str());

    recvCrypto.Setup(key, 20);
    sendCrypto.Setup(key, 20);

    /* packets are encrypted from now on */
    use_crypto = true;

    /* send the response packet */
    WorldPacket data(RSMSG_AUTH_RESPONSE, 1);
    data << result;
    SendPacket(&data);

    /* set our general var */
    authenticated = result;
}

void LogonCommServerSocket::HandleMappingReply(WorldPacket & recvData)
{
    /* this packet is gzipped, whee! :D */
    uint32 real_size;
    recvData >> real_size;
    uLongf rsize = real_size;

    ByteBuffer buf(real_size);
    buf.clear();
    buf.resize(real_size);

    if (uncompress((uint8*)buf.contents(), &rsize, recvData.contents() + 4, (u_long)recvData.size() - 4) != Z_OK)
    {
        LOG_ERROR("Uncompress of mapping failed.");
        return;
    }

    uint32 account_id;
    uint8 number_of_characters;
    uint32 count;
    uint32 realm_id;
    buf >> realm_id;
    Realm* realm = sInfoCore.GetRealm(realm_id);
    if (!realm)
        return;

    sInfoCore.getRealmLock().Acquire();

    HM_NAMESPACE::hash_map<uint32, uint8>::iterator itr;
    buf >> count;
    LOG_BASIC("Got mapping packet for realm %u, total of %u entries.", (unsigned int)realm_id, (unsigned int)count);
    for (uint32 i = 0; i < count; ++i)
    {
        buf >> account_id >> number_of_characters;
        itr = realm->CharacterMap.find(account_id);
        if (itr != realm->CharacterMap.end())
            itr->second = number_of_characters;
        else
            realm->CharacterMap.insert(std::make_pair(account_id, number_of_characters));
    }

    sInfoCore.getRealmLock().Release();
}

void LogonCommServerSocket::HandleUpdateMapping(WorldPacket & recvData)
{
    uint32 realm_id;
    uint32 account_id;
    uint8 chars_to_add;
    recvData >> realm_id;

    Realm* realm = sInfoCore.GetRealm(realm_id);
    if (!realm)
        return;

    sInfoCore.getRealmLock().Acquire();
    recvData >> account_id >> chars_to_add;

    HM_NAMESPACE::hash_map<uint32, uint8>::iterator itr = realm->CharacterMap.find(account_id);
    if (itr != realm->CharacterMap.end())
        itr->second += chars_to_add;
    else
        realm->CharacterMap.insert(std::make_pair(account_id, chars_to_add));

    sInfoCore.getRealmLock().Release();
}

void LogonCommServerSocket::HandleTestConsoleLogin(WorldPacket & recvData)
{
    WorldPacket data(RSMSG_CONSOLE_LOGIN_RESULT, 8);
    uint32 request;
    std::string accountname;
    uint8 key[20];

    recvData >> request;
    recvData >> accountname;
    recvData.read(key, 20);

    data << request;

    Account* pAccount = sAccountMgr.GetAccount(accountname);
    if (pAccount == NULL)
    {
        data << uint32(0);
        SendPacket(&data);
        return;
    }

    if (pAccount->GMFlags == NULL || strchr(pAccount->GMFlags, 'z') == NULL)
    {
        data << uint32(0);
        SendPacket(&data);
        return;
    }

    if (memcmp(pAccount->SrpHash, key, 20) != 0)
    {
        data << uint32(0);
        SendPacket(&data);
        return;
    }

    data << uint32(1);
    SendPacket(&data);
}

void LogonCommServerSocket::HandleDatabaseModify(WorldPacket & recvData)
{
    uint32 method;
    recvData >> method;

    switch (method)
    {
        case 1:            // set account ban
        {
            std::string account;
            std::string banreason;
            uint32 duration;
            recvData >> account >> duration >> banreason;

            // remember we expect this in uppercase
            arcemu_TOUPPER(account);

            Account* pAccount = sAccountMgr.GetAccount(account);
            if (pAccount == NULL)
                return;

            pAccount->Banned = duration;

            // update it in the sql (duh)
            sLogonSQL->Execute("UPDATE accounts SET banned = %u, banreason = '%s' WHERE login = \"%s\"", duration, sLogonSQL->EscapeString(banreason).c_str(), sLogonSQL->EscapeString(account).c_str());

        }
        break;

        case 2:        // set gm
        {
            std::string account;
            std::string gm;
            recvData >> account >> gm;

            // remember we expect this in uppercase
            arcemu_TOUPPER(account);

            Account* pAccount = sAccountMgr.GetAccount(account);
            if (pAccount == NULL)
                return;

            pAccount->SetGMFlags(account.c_str());

            // update it in the sql (duh)
            sLogonSQL->Execute("UPDATE accounts SET gm = \"%s\" WHERE login = \"%s\"", sLogonSQL->EscapeString(gm).c_str(), sLogonSQL->EscapeString(account).c_str());

        }
        break;

        case 3:        // set mute
        {
            std::string account;
            uint32 duration;
            recvData >> account >> duration;

            // remember we expect this in uppercase
            arcemu_TOUPPER(account);

            Account* pAccount = sAccountMgr.GetAccount(account);
            if (pAccount == NULL)
                return;

            pAccount->Muted = duration;

            // update it in the sql (duh)
            sLogonSQL->Execute("UPDATE accounts SET muted = %u WHERE login = \"%s\"", duration, sLogonSQL->EscapeString(account).c_str());
        }
        break;

        case 4:        // ip ban add
        {
            std::string ip;
            std::string banreason;
            uint32 duration;

            recvData >> ip >> duration >> banreason;

            if (sIPBanner.Add(ip.c_str(), duration))
                sLogonSQL->Execute("INSERT INTO ipbans VALUES(\"%s\", %u, \"%s\")", sLogonSQL->EscapeString(ip).c_str(), duration, sLogonSQL->EscapeString(banreason).c_str());

        }
        break;

        case 5:        // ip ban remove
        {
            std::string ip;
            recvData >> ip;

            if (sIPBanner.Remove(ip.c_str()))
                sLogonSQL->Execute("DELETE FROM ipbans WHERE ip = \"%s\"", sLogonSQL->EscapeString(ip).c_str());

        }
        break;

        case 6:        // account change password
        {
            // Prepare our "send-back" packet
            WorldPacket data(RSMSG_MODIFY_DATABASE_RESULT, 200);

            std::string old_password;
            std::string new_password;
            std::string account_name;

            recvData >> old_password;
            recvData >> new_password;
            recvData >> account_name;

            std::string pass;
            pass.assign(account_name);
            pass.push_back(':');
            pass.append(old_password);
            auto check_oldpass_query = sLogonSQL->Query("SELECT login, encrypted_password FROM accounts WHERE encrypted_password=SHA(UPPER('%s')) AND login='%s'", pass.c_str(), account_name.c_str());

            if (!check_oldpass_query)
            {
                // Send packet back... Your current password matches not your input!
                data << uint32(method);     // method_id
                data << uint8(1);           // result_id
                data << account_name;       // account_name
                SendPacket(&data);
            }
            else
            {
                std::string new_pass;
                new_pass.assign(account_name);
                new_pass.push_back(':');
                new_pass.append(new_password);

                auto new_pass_query = sLogonSQL->Query("UPDATE accounts SET encrypted_password=SHA(UPPER('%s')) WHERE login='%s'", new_pass.c_str(), account_name.c_str());

                /*The query is already done, don't know why we are here. \todo check sLogonSQL query handling.
                if (!new_pass_query)
                {
                    // Send packet back... Somehting went wrong!
                    data << uint32(method);     // method_id
                    data << uint8(2);           // result_id
                    data << account_name;       // account_name
                    SendPacket(&data);
                }
                else
                {*/
                    // Send packet back... Everything is fine!
                    data << uint32(method);     // method_id
                    data << uint8(3);           // result_id
                    data << account_name;       // account_name
                    SendPacket(&data);
                //}

                sAccountMgr.ReloadAccounts(false);
            }

        }
        break;

    }
}

void LogonCommServerSocket::HandleRequestCheckAccount(WorldPacket & recvData)
{
    uint32 method;
    recvData >> method;

    switch (method)
    {
        case 1:            // account exist?
        {
            // Prepare our "send-back" packet
            WorldPacket data(RSMSG_CHECK_ACCOUNT_RESULT, 300);

            std::string account_name;           // account to check
            std::string request_name;           // account request the check
            std::string additional;             // additional data

            recvData >> account_name;
            recvData >> request_name;
            recvData >> additional;

            const char* additional_data = additional.c_str();
            
            std::string account_name_save = account_name;  // save original account_name to check

            // remember we expect this in uppercase
            arcemu_TOUPPER(account_name);

            auto account_check = sAccountMgr.GetAccount(account_name);
            if (account_check == nullptr)
            {
                // Send packet "account not available"
                data << uint8(1);
                data << account_name_save;  // requested account
                data << request_name;       // account_name for receive the session
                SendPacket(&data);
            }
            else if (!additional_data)
            {
                // Send packet "No additional data set"
                data << uint8(2);
                data << account_name_save;  // requested account
                data << request_name;       // account_name for receive the session
                SendPacket(&data);
            }
            else
            {
                // Send packet "everything okay"
                data << uint8(3);
                data << account_name_save;  // requested account
                data << request_name;       // account_name for receive the session
                data << additional;         // additional data
                SendPacket(&data);
            }
        }
        break;
    }
}

void LogonCommServerSocket::HandlePopulationRespond(WorldPacket & recvData)
{
    float population;
    uint32 realmId;
    recvData >> realmId >> population;
    sInfoCore.UpdateRealmPop(realmId, population);
}

void LogonCommServerSocket::RefreshRealmsPop()
{
    if (server_ids.empty())
        return;

    WorldPacket data(RSMSG_REALM_POP_REQ, 4);
    std::set<uint32>::iterator itr = server_ids.begin();
    for (; itr != server_ids.end(); itr++)
    {
        data.clear();
        data << (*itr);
        SendPacket(&data);
    }
}
