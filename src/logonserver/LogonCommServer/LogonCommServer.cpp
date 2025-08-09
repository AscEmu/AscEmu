/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "LogonCommServer.h"

#include <sstream>

#include "Cryptography/LogonCommDefines.h"

#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Realm/RealmManager.hpp"
#include "Server/Master.hpp"
#include "Network/Socket.h"
#include "WorldPacket.h"
#include "Server/AccountMgr.h"
#include "Cryptography/Sha1.hpp"
#include "Database/Database.h"
#include "Utilities/Strings.hpp"
#include "Server/IpBanMgr.h"

LogonCommServerSocket::LogonCommServerSocket(SOCKET fd) : Socket(fd, 65536, 524288)
{
    // do nothing
    last_ping = (uint32_t)UNIXTIME;
    remaining = opcode = 0;
    removed = true;

    use_crypto = false;
    authenticated = 0;
    seed = 0;

    sLogger.info("Created LogonCommServerSocket {}", m_fd);
}

void LogonCommServerSocket::OnDisconnect()
{
    sLogger.info("LogonCommServerSocket::Ondisconnect event.");

    // if we're registered -> Set offline
    if (!removed)
    {
        for (auto itr : server_ids)
            sRealmManager.setRealmOffline(itr);

        sRealmManager.removeServerSocket(this);
    }
}

void LogonCommServerSocket::OnConnect()
{
    if (!sMasterLogon.IsServerAllowed(GetRemoteAddress().s_addr))
    {
        sLogger.failure("Server connection from {}:{} DENIED, not an allowed IP.", GetRemoteIP(), GetRemotePort());
        Disconnect();
        return;
    }

    sRealmManager.addServerSocket(this);
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
            readBuffer.Read((uint8_t*)&opcode, 2);
            readBuffer.Read((uint8_t*)&remaining, 4);

            if (use_crypto)
            {
                // decrypt the packet
                _rwCrypto.process((unsigned char*)&opcode, (unsigned char*)&opcode, 2);
                _rwCrypto.process((unsigned char*)&remaining, (unsigned char*)&remaining, 4);
            }

            /* reverse byte order */
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
            //Read(remaining, (uint8_t*)buff.contents());
            readBuffer.Read((uint8_t*)buff.contents(), remaining);
        }

        if (use_crypto && remaining)
            _rwCrypto.process((unsigned char*)buff.contents(), (unsigned char*)buff.contents(), remaining);

        // handle the packet
        HandlePacket(buff);

        remaining = 0;
        opcode = 0;
    }
}

void LogonCommServerSocket::HandlePacket(WorldPacket & recvData)
{
    if (authenticated == 0 && recvData.GetOpcode() != LRCMSG_AUTH_REQUEST)
    {
        // invalid
        Disconnect();
        return;
    }

    static logonpacket_handler Handlers[LRMSG_MAX_OPCODES] =
    {
        NULL,
        &LogonCommServerSocket::HandleRegister,             // LRCMSG_REALM_REGISTER_REQUEST
        NULL,                                               // LRSMSG_REALM_REGISTER_RESULT
        &LogonCommServerSocket::HandleSessionRequest,       // LRCMSG_ACC_SESSION_REQUEST
        NULL,                                               // LRSMSG_ACC_SESSION_RESULT
        &LogonCommServerSocket::HandlePing,                 // LRCMSG_LOGON_PING_STATUS
        NULL,                                               // LRSMSG_LOGON_PING_RESULT
        NULL,                                               // LRCMSG_FREE_01
        NULL,                                               // LRCMSG_FREE_02
        &LogonCommServerSocket::HandleAuthChallenge,        // LRCMSG_AUTH_REQUEST
        NULL,                                               // LRSMSG_AUTH_RESPONSE
        NULL,                                               // LRSMSG_ACC_CHAR_MAPPING_REQUEST
        &LogonCommServerSocket::HandleMappingReply,         // LRCMSG_ACC_CHAR_MAPPING_RESULT
        &LogonCommServerSocket::HandleUpdateMapping,        // LRCMSG_ACC_CHAR_MAPPING_UPDATE
        NULL,                                               // LRSMSG_SEND_ACCOUNT_DISCONNECT
        &LogonCommServerSocket::HandleTestConsoleLogin,     // LRCMSG_LOGIN_CONSOLE_REQUEST
        NULL,                                               // LRSMSG_LOGIN_CONSOLE_RESULT
        &LogonCommServerSocket::HandleDatabaseModify,       // LRCMSG_ACCOUNT_DB_MODIFY_REQUEST
        NULL,                                               // LRSMSG_ACCOUNT_DB_MODIFY_RESULT
        NULL,                                               // LRSMSG_REALM_POPULATION_REQUEST
        &LogonCommServerSocket::HandlePopulationRespond,    // LRCMSG_REALM_POPULATION_RESULT
        &LogonCommServerSocket::HandleRequestCheckAccount,  // LRCMSG_ACCOUNT_REQUEST
        NULL,                                               // LRSMSG_ACCOUNT_RESULT
        &LogonCommServerSocket::HandleRequestAllAccounts,   // LRCMSG_ALL_ACCOUNT_REQUEST
        NULL,                                               // LRSMSG_ALL_ACCOUNT_RESULT
    };

    if (recvData.GetOpcode() >= LRMSG_MAX_OPCODES || Handlers[recvData.GetOpcode()] == 0)
    {
        sLogger.failure("Got unknwon packet from logoncomm: {}", recvData.GetOpcode());
        return;
    }

    (this->*(Handlers[recvData.GetOpcode()]))(recvData);
}

void LogonCommServerSocket::HandleRegister(WorldPacket & recvData)
{
    std::string realmName;
    uint32_t realmId;

    recvData >> realmId;
    recvData >> realmName;

    sLogger.info("Registering realm `{}` with ID {}.", realmName, realmId);

    // check Realms if realmId is valid! Otherwise send back error.
    auto realm = sRealmManager.getRealmById(realmId);
    if (realm == nullptr)
    {
        WorldPacket data(LRSMSG_REALM_REGISTER_RESULT, 4);
        data << uint32_t(1);        // 1 = realm not known by logonserver - failed
        data << realmId;
        data << realmName;
        SendPacket(&data);

        return;
    }

    realm->name = realmName;

    recvData >> realm->address;
    recvData >> realm->flags;
    recvData >> realm->icon;
    recvData >> realm->timeZone;
    recvData >> realm->population;
    recvData >> realm->lock;
    recvData >> realm->gameBuild;

    sRealmManager.setStatusForRealm(realmId, 1);

    WorldPacket data(LRSMSG_REALM_REGISTER_RESULT, 4);
    data << uint32_t(0);              // 0 = everything ok - success
    data << realmId;
    data << realm->name;
    SendPacket(&data);

    server_ids.insert(realmId);

    // request character mapping for this realm
    data.Initialize(LRSMSG_ACC_CHAR_MAPPING_REQUEST);
    data << realmId;
    SendPacket(&data);
}

void LogonCommServerSocket::HandleSessionRequest(WorldPacket & recvData)
{
    uint32_t request_id;
    std::string account_name;

    recvData >> request_id;
    recvData >> account_name;

    // get sessionkey!
    uint32_t error = 0;
    const auto* acct = sAccountMgr.getAccountByName(account_name);
    if (acct == nullptr || acct->SessionKey == NULL)
        error = 1;          // Unauthorized user.

    // build response packet
    WorldPacket data(LRSMSG_ACC_SESSION_RESULT, 150);
    data << request_id;
    data << error;

    if (!error)
    {
        // Append account information.
        data << acct->AccountId;
        data << acct->UsernamePtr->c_str();
        if (!acct->GMFlags)
            data << uint8_t(0);
        else
            data << acct->GMFlags.get();

        data << acct->AccountFlags;
        data.append(acct->SessionKey.get(), 40);
        data << acct->forcedLanguage;
        data << acct->Muted;
    }

    SendPacket(&data);
}

void LogonCommServerSocket::HandlePing(WorldPacket & recvData)
{
    uint8_t realmId;
    recvData >> realmId;

    WorldPacket data(LRSMSG_LOGON_PING_RESULT, 4);
    SendPacket(&data);
    last_ping = static_cast<uint32_t>(time(nullptr));

    sRealmManager.setLastPing(realmId);
}

void LogonCommServerSocket::SendPacket(WorldPacket* data)
{
    bool rv;
    BurstBegin();

    LogonWorldPacket header;
    header.opcode = data->GetOpcode();
    //header.size   = ntohl((u_long)data->size());
    header.size = (uint32_t)data->size();

    byteSwapUInt32(&header.size);

    if (use_crypto)
        _sendCrypto.process((unsigned char*)&header, (unsigned char*)&header, 6);

    rv = BurstSend((uint8_t*)&header, 6);

    if (data->size() > 0 && rv)
    {
        if (use_crypto)
            _sendCrypto.process((unsigned char*)data->contents(), (unsigned char*)data->contents(), (uint32_t)data->size());

        rv = BurstSend(data->contents(), (uint32_t)data->size());
    }

    if (rv) BurstPush();
    BurstEnd();
}

void LogonCommServerSocket::HandleAuthChallenge(WorldPacket & recvData)
{
    unsigned char key[20];
    uint8_t realmId = 0;

    recvData.read(key, 20);
    recvData >> realmId;

    const auto realm = sRealmManager.getRealmById(realmId);
    if (realm == nullptr)
    {
        sLogger.failure("Realm {} is missing in  table realms. Please add the server to your realms table.", static_cast<uint32_t>(realmId));
        return;
    }

    Sha1Hash hash;
    hash.updateData(realm->password);
    hash.finalize();

    // check if we have the correct password
    uint32_t result = 1;

    if (memcmp(key, hash.getDigest(), 20) != 0)
        result = 0;

    sLogger.info("Authentication request from {}, id {} - result {}.", GetRemoteIP(), uint32_t(realmId), result ? "OK" : "FAIL");

    std::stringstream sstext;
    sstext << "Key: ";
    char buf[3];
    for (int i = 0; i < 20; ++i)
    {
        snprintf(buf, 3, "%.2X", key[i]);
        sstext << buf;
    }
    //sLogger.info(sstext); FIX fmt

    _rwCrypto.setup(key, 20);
    _sendCrypto.setup(key, 20);

    // packets are encrypted from now on
    use_crypto = true;

    // send the response packet
    WorldPacket data(LRSMSG_AUTH_RESPONSE, 1);
    data << result;
    SendPacket(&data);

    // set our general var
    authenticated = result;
}

void LogonCommServerSocket::HandleMappingReply(WorldPacket & recvData)
{
    // this packet is gzipped, whee! :D
    uint32_t real_size;
    recvData >> real_size;
    uLongf rsize = real_size;

    ByteBuffer buf(real_size);
    buf.clear();
    buf.resize(real_size);

    if (uncompress((uint8_t*)buf.contents(), &rsize, recvData.contents() + 4, (u_long)recvData.size() - 4) != Z_OK)
    {
        sLogger.failure("Uncompress of mapping failed.");
        return;
    }

    uint32_t account_id;
    uint8_t number_of_characters;
    uint32_t count;
    uint32_t realm_id;
    buf >> realm_id;
    auto realm = sRealmManager.getRealmById(realm_id);
    if (!realm)
        return;

    sRealmManager.getRealmLock().lock();

    std::unordered_map<uint32_t, uint8_t>::iterator itr;
    buf >> count;
    sLogger.info("Got mapping packet for realm {}, total of {} entries.", (unsigned int)realm_id, (unsigned int)count);
    for (uint32_t i = 0; i < count; ++i)
    {
        buf >> account_id >> number_of_characters;
        itr = realm->_characterMap.find(account_id);
        if (itr != realm->_characterMap.end())
            itr->second = number_of_characters;
        else
            realm->_characterMap.insert(std::make_pair(account_id, number_of_characters));
    }

    sRealmManager.getRealmLock().unlock();
}

void LogonCommServerSocket::HandleUpdateMapping(WorldPacket & recvData)
{
    uint32_t realm_id;
    uint32_t account_id;
    uint8_t chars_to_add;
    recvData >> realm_id;

    auto realm = sRealmManager.getRealmById(realm_id);
    if (!realm)
        return;

    sRealmManager.getRealmLock().lock();
    recvData >> account_id;
    recvData >> chars_to_add;

    auto itr = realm->_characterMap.find(account_id);
    if (itr != realm->_characterMap.end())
        itr->second += chars_to_add;
    else
        realm->_characterMap.insert(std::make_pair(account_id, chars_to_add));

    sRealmManager.getRealmLock().unlock();
}

void LogonCommServerSocket::HandleTestConsoleLogin(WorldPacket & recvData)
{
    WorldPacket data(LRSMSG_LOGIN_CONSOLE_RESULT, 8);
    uint32_t request;
    std::string accountname;
    std::string password;

    recvData >> request;
    recvData >> accountname;
    recvData >> password;

    data << request;

    const auto* pAccount = sAccountMgr.getAccountByName(accountname);
    if (pAccount == nullptr)
    {
        data << uint32_t(0);
        SendPacket(&data);
        return;
    }

    std::string pass;
    pass.assign(accountname);
    pass.push_back(':');
    pass.append(password);
    auto checkPassQuery = sLogonSQL->Query("SELECT acc_name, encrypted_password FROM accounts WHERE encrypted_password = SHA(UPPER('%s')) AND acc_name = '%s'", pass.c_str(), accountname.c_str());
    if (!checkPassQuery)
    {
        data << uint32_t(0);
        SendPacket(&data);
        return;
    }

    data << uint32_t(1);
    SendPacket(&data);
}

void LogonCommServerSocket::HandleDatabaseModify(WorldPacket & recvData)
{
    uint32_t method;
    recvData >> method;

    switch (method)
    {
        case Method_Account_Ban:
        {
            std::string account;
            std::string banreason;
            uint32_t duration;
            recvData >> account;
            recvData >> duration;
            recvData >> banreason;

            // remember we expect this in uppercase
            AscEmu::Util::Strings::toUpperCase(account);

            auto* pAccount = sAccountMgr.getAccountByName(account);
            if (pAccount == nullptr)
                return;

            pAccount->Banned = duration;

            // update it in the sql (duh)
            sLogonSQL->Execute("UPDATE accounts SET banned = %u, banreason = '%s' WHERE acc_name = \"%s\"", duration, sLogonSQL->EscapeString(banreason).c_str(), sLogonSQL->EscapeString(account).c_str());

        }
        break;

        case Method_Account_Set_GM: //unused!
        {
            //std::string account;
            //std::string gm;
            //recvData >> account;
            //recvData >> gm;

            //// remember we expect this in uppercase
            //AscEmu::Util::Strings::toUpperCase(account);

            //Account* pAccount = sAccountMgr.getAccountByName(account);
            //if (pAccount == NULL)
            //    return;

            //pAccount->SetGMFlags(account.c_str());

            //// update it in the sql (duh)
            //sLogonSQL->Execute("UPDATE accounts SET gm = \"%s\" WHERE acc_name = \"%s\"", sLogonSQL->EscapeString(gm).c_str(), sLogonSQL->EscapeString(account).c_str());

        }
        break;

        case Method_Account_Set_Mute:
        {
            std::string account;
            uint32_t duration;
            recvData >> account;
            recvData >> duration;

            // remember we expect this in uppercase
            AscEmu::Util::Strings::toUpperCase(account);

            auto* pAccount = sAccountMgr.getAccountByName(account);
            if (pAccount == nullptr)
                return;

            pAccount->Muted = duration;

            // update it in the sql (duh)
            sLogonSQL->Execute("UPDATE accounts SET muted = %u WHERE acc_name = \"%s\"", duration, sLogonSQL->EscapeString(account).c_str());
        }
        break;

        case Method_IP_Ban:
        {
            std::string ip;
            std::string banreason;
            uint32_t duration;

            recvData >> ip;
            recvData >> duration;
            recvData >> banreason;

            if (sIpBanMgr.add(ip, duration))
                sLogonSQL->Execute("INSERT INTO ipbans VALUES(\"%s\", %u, \"%s\")", sLogonSQL->EscapeString(ip).c_str(), duration, sLogonSQL->EscapeString(banreason).c_str());

        }
        break;

        case Method_IP_Unban:
        {
            std::string ip;
            recvData >> ip;

            if (sIpBanMgr.remove(ip))
                sLogonSQL->Execute("DELETE FROM ipbans WHERE ip = \"%s\"", sLogonSQL->EscapeString(ip).c_str());

        }
        break;
        case Method_Account_Change_PW:
        {
            // Prepare our "send-back" packet
            WorldPacket data(LRSMSG_ACCOUNT_DB_MODIFY_RESULT, 200);

            std::string old_password;
            std::string new_password;
            std::string account_name;
            uint8_t result = 0;

            recvData >> old_password;
            recvData >> new_password;
            recvData >> account_name;

            std::string pass;
            pass.assign(account_name);
            pass.push_back(':');
            pass.append(old_password);
            auto check_oldpass_query = sLogonSQL->Query("SELECT acc_name, encrypted_password FROM accounts WHERE encrypted_password = SHA(UPPER('%s')) AND acc_name = '%s'", pass.c_str(), account_name.c_str());

            if (!check_oldpass_query)
            {
                // Send packet back... Your current password matches not your input!
                result = Result_Account_PW_wrong;

                data << uint32_t(method);     // method_id
                data << uint8_t(result);
                data << account_name;       // account_name
                SendPacket(&data);
            }
            else
            {
                std::string new_pass;
                new_pass.assign(account_name);
                new_pass.push_back(':');
                new_pass.append(new_password);

                // Send packet back... Everything is fine!
                result = Result_Account_Finished;

                data << uint32_t(method);     // method_id
                data << uint8_t(result);
                data << account_name;       // account_name
                SendPacket(&data);
                //}

                sAccountMgr.reloadAccounts(false);
            }

        }
        break;
        case Method_Account_Create:
        {
            // Prepare our "send-back" packet
            WorldPacket data(LRSMSG_ACCOUNT_DB_MODIFY_RESULT, 300);

            std::string name;
            std::string password;
            std::string account_name;
            uint8_t result = 0;

            recvData >> name;
            recvData >> password;
            recvData >> account_name;

            std::string name_save = name;  // save original name to check

            // remember we expect this in uppercase
            AscEmu::Util::Strings::toUpperCase(name);

            auto account_check = sAccountMgr.getAccountByName(name);

            if (account_check != nullptr)
            {
                result = Result_Account_Exists;

                data << uint32_t(method);     // method_id
                data << uint8_t(result);
                data << account_name;       // account_name
                data << name_save;          // created account name
                SendPacket(&data);
            }
            else
            {
                std::string pass;
                pass.assign(name);
                pass.push_back(':');
                pass.append(password);

                sLogonSQL->Query("INSERT INTO `accounts`(`acc_name`,`encrypted_password`,`banned`,`email`,`flags`,`banreason`) VALUES ('%s', SHA(UPPER('%s')),'0','','24','')", name_save.c_str(), pass.c_str());

                result = Result_Account_Finished;

                data << uint32_t(method);     // method_id
                data << uint8_t(result);
                data << account_name;       // account_name
                data << name_save;          // created account name
                SendPacket(&data);
            }

            sAccountMgr.reloadAccounts(false);
        }
        break;
    }
}

void LogonCommServerSocket::HandleRequestCheckAccount(WorldPacket & recvData)
{
    uint32_t method;
    recvData >> method;


    std::string account_name;           // account to check
    std::string request_name;           // account request the check

    WorldPacket data(LRSMSG_ACCOUNT_RESULT, 300);

    switch (method)
    {
        case 1:            // account exist?
        {
            std::string additional;             // additional data

            recvData >> account_name;
            recvData >> request_name;
            recvData >> additional;

            const char* additional_data = additional.c_str();

            std::string account_name_save = account_name;  // save original account_name to check

            // remember we expect this in uppercase
            AscEmu::Util::Strings::toUpperCase(account_name);

            const auto* account_check = sAccountMgr.getAccountByName(account_name);
            if (account_check == nullptr)
            {
                // Send packet "account not available"
                data << uint8_t(1);
                data << account_name_save;  // requested account
                data << request_name;       // account_name for receive the session
            }
            else if (!additional_data)
            {
                // Send packet "No additional data set"
                data << uint8_t(2);
                data << account_name_save;  // requested account
                data << request_name;       // account_name for receive the session
            }
            else
            {
                // Send packet "everything okay"
                data << uint8_t(3);
                data << account_name_save;  // requested account
                data << request_name;       // account_name for receive the session
                data << additional;         // additional data
                data << uint32_t(account_check->AccountId);
            }
        } break;
        case 2:            // get account id
        {
            recvData >> account_name;
            recvData >> request_name;

            std::string account_name_save = account_name;  // save original account_name to check

            // remember we expect this in uppercase
            AscEmu::Util::Strings::toUpperCase(account_name);

            const auto* account_check = sAccountMgr.getAccountByName(account_name);
            if (account_check == nullptr)
            {
                // Send packet "account not available"
                data << uint8_t(1);
                data << account_name_save;  // requested account
                data << request_name;       // account_name for receive the session
            }
            else
            {
                // Send packet "account id"
                data << uint8_t(4);
                data << account_name_save;  // requested account
                data << request_name;       // account_name for receive the session
                data << uint32_t(account_check->AccountId);
            }
        } break;
    }

    SendPacket(&data);
}

void LogonCommServerSocket::HandleRequestAllAccounts(WorldPacket& /*recvData*/)
{
    std::string accountsArray;
    const auto& accountMap = sAccountMgr.getAccountMap();

    for (auto const& map_itr : accountMap)
    {
        std::string gm_flags;

        if (map_itr.second->GMFlags)
            gm_flags = map_itr.second->GMFlags.get();
        else
            gm_flags = "0";

        accountsArray += std::to_string(map_itr.second->AccountId) + "," + map_itr.first + "," + (gm_flags.empty() ? "0" : gm_flags) + ";";
    }

    // remove last ; from string
    accountsArray.pop_back();

    WorldPacket data(LRSMSG_ALL_ACCOUNT_RESULT, 3000);
    data << accountsArray;
    SendPacket(&data);
}

void LogonCommServerSocket::HandlePopulationRespond(WorldPacket & recvData)
{
    float population;
    uint32_t realmId;
    recvData >> realmId >> population;
    sRealmManager.setRealmPopulation(realmId, population);
}

void LogonCommServerSocket::RefreshRealmsPop()
{
    if (server_ids.empty())
        return;

    WorldPacket data(LRSMSG_REALM_POPULATION_REQUEST, 4);
    std::set<uint32_t>::iterator itr = server_ids.begin();
    for (; itr != server_ids.end(); ++itr)
    {
        data.clear();
        data << (*itr);
        SendPacket(&data);
    }
}
