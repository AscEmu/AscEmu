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

#include "LogonCommServer.h"

#include <mutex>
#include <sstream>

#include "LogonCommDefines.hpp"
#include "LogonCommBattlePets.hpp"
#include "LogonCommAchievements.hpp"
#include "Server/LogonServerDefines.hpp"

#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Realm/RealmManager.hpp"
#include "Server/Master.hpp"
#include "Network/Socket.hpp"
#include "Network/WorldPacket.hpp"
#include "Server/AccountMgr.h"
#include "Server/AccountService.hpp"
#include "Cryptography/Sha1.hpp"
#include "Database/Database.hpp"
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

    sLogger.info("Created LogonCommServerSocket {}", m_socket);
}

void LogonCommServerSocket::onDisconnect()
{
    sLogger.info("LogonCommServerSocket::onDisconnect event.");

    // if we're registered -> Set offline
    if (!removed)
    {
        for (auto itr : server_ids)
            sRealmManager.setRealmOffline(itr);

        sRealmManager.removeServerSocket(this);
    }
}

void LogonCommServerSocket::onConnect()
{
    if (!sMasterLogon.IsServerAllowed(getRemoteAddress().s_addr))
    {
        sLogger.failure("Server connection from {}:{} DENIED, not an allowed IP.", getRemoteIp(), getRemotePort());
        disconnect();
        return;
    }

    sRealmManager.addServerSocket(this);
    removed = false;
}

void LogonCommServerSocket::onRead()
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
    m_logAccountId = 0;
    m_logDetail.clear();

    if (authenticated == 0 && recvData.getOpcode() != LRCMSG_AUTH_REQUEST)
    {
        // invalid
        m_logDetail = "not authenticated, disconnected";
        writeCommLog(recvData.getOpcode());
        disconnect();
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
        &LogonCommServerSocket::HandleBuildRequest,         // LRCMSG_BUILD_REQUEST
        NULL,                                               // LRSMSG_BUILD_RESULT
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
        NULL,                                               // LRSMSG_PENDING_WORLD_SESSION
        &LogonCommServerSocket::HandleBattlePetRequest,     // LRCMSG_BATTLE_PET_REQUEST
        NULL,                                               // LRSMSG_BATTLE_PET_RESULT
        &LogonCommServerSocket::HandleBattlePetSave,        // LRCMSG_BATTLE_PET_SAVE
        &LogonCommServerSocket::HandleBattlePetAdd,         // LRCMSG_BATTLE_PET_ADD
        NULL,                                               // LRSMSG_BATTLE_PET_ADD_RESULT
        &LogonCommServerSocket::HandleAchievementRequest,   // LRCMSG_ACHIEVEMENT_REQUEST
        NULL,                                               // LRSMSG_ACHIEVEMENT_RESULT
        &LogonCommServerSocket::HandleAchievementSave,      // LRCMSG_ACHIEVEMENT_SAVE
        &LogonCommServerSocket::HandleAchievementReset,     // LRCMSG_ACHIEVEMENT_RESET
    };

    if (recvData.getOpcode() >= LRMSG_MAX_OPCODES || Handlers[recvData.getOpcode()] == 0)
    {
        sLogger.failure("Got unknwon packet from logoncomm: {}", recvData.getOpcode());
        m_logDetail = "unknown opcode";
        writeCommLog(recvData.getOpcode());
        return;
    }

    (this->*(Handlers[recvData.getOpcode()]))(recvData);
    writeCommLog(recvData.getOpcode());
}

void LogonCommServerSocket::writeCommLog(uint32_t logOpcode)
{
    // the heartbeat of every realm would bury the requests
    if (logOpcode == LRCMSG_LOGON_PING_STATUS || logOpcode == LRCMSG_REALM_POPULATION_RESULT)
        return;

    std::string realmIds;
    for (const uint32_t realmId : server_ids)
        realmIds += (realmIds.empty() ? "" : ",") + std::to_string(realmId);

    if (m_logDetail.size() > 255)
        m_logDetail.resize(255);

    sLogonSQL->execute("INSERT INTO logoncomm_log (time, realm_ids, remote_ip, opcode, opcode_name, account_id, detail) VALUES (FROM_UNIXTIME(%u), '%s', '%s', %u, '%s', %u, '%s')",
        static_cast<uint32_t>(UNIXTIME), realmIds.c_str(), sLogonSQL->escapeString(getRemoteIp()).c_str(), logOpcode, getLogonRealmOpcodeName(logOpcode),
        m_logAccountId, sLogonSQL->escapeString(m_logDetail).c_str());
}

void LogonCommServerSocket::HandleRegister(WorldPacket & recvData)
{
    std::string realmName;
    uint32_t realmId;

    recvData >> realmId;
    recvData >> realmName;

    sLogger.info("Registering realm `{}` with ID {}.", realmName, realmId);

    // check Realms if realmId is valid! Otherwise, send back error.
    m_logDetail = fmt::format("realm {} '{}'", realmId, realmName);

    auto realm = sRealmManager.getRealmById(realmId);
    if (realm == nullptr)
    {
        m_logDetail += " unknown to the logonserver";
        WorldPacket data(LRSMSG_REALM_REGISTER_RESULT, 4);
        data << uint32_t(1); // 1 = realm not known by logonserver - failed
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

    sRealmManager.setStatusForRealm(static_cast<uint8_t>(realmId), 1);

    WorldPacket data(LRSMSG_REALM_REGISTER_RESULT, 4);
    data << uint32_t(0); // 0 = everything ok - success
    data << realmId;
    data << realm->name;
    SendPacket(&data);

    server_ids.insert(realmId);

    // request character mapping for this realm
    data.initialize(LRSMSG_ACC_CHAR_MAPPING_REQUEST);
    data << realmId;
    SendPacket(&data);
}

void LogonCommServerSocket::HandleSessionRequest(WorldPacket& recvData)
{
    uint32_t request_id;
    std::string account_name;

    recvData >> request_id;
    recvData >> account_name;

    // get sessionkey!
    uint32_t error = 0;
    const auto* acct = sAccountMgr.getAccountByName(account_name);
    if (acct == nullptr || acct->SessionKey == NULL)
        error = 1; // Unauthorized user.

    m_logAccountId = acct != nullptr ? acct->AccountId : 0;
    m_logDetail = fmt::format("account '{}'{}", account_name, error ? " without session" : "");

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

void LogonCommServerSocket::HandleBuildRequest(WorldPacket& recvData)
{
    uint32_t request_id;
    std::string ip;

    recvData >> request_id;
    recvData >> ip;

    const auto build = sRealmManager.getBuildForIP(ip);
    m_logDetail = fmt::format("ip {} build {}", ip, build);

    WorldPacket data(LRSMSG_BUILD_RESULT, 50);
    data << request_id;
    data << build;

    SendPacket(&data);
}

void LogonCommServerSocket::SendPacket(WorldPacket* data)
{
    if (!data)
        return;

    burstBegin();

    LogonWorldPacket header;
    header.opcode = static_cast<uint16_t>(data->getOpcode());

    uint32_t sizeValue = static_cast<uint32_t>(data->size());
    byteSwapUInt32(&sizeValue);
    header.size = sizeValue;

    if (use_crypto)
    {
        auto* headerPtr = reinterpret_cast<unsigned char*>(&header);
        _sendCrypto.process(headerPtr, headerPtr, 6);
    }

    bool isSuccess = burstSend(reinterpret_cast<const uint8_t*>(&header), 6);

    if (data->size() > 0 && isSuccess)
    {
        if (use_crypto)
        {
            auto* contentsPtr = data->contents();
            _sendCrypto.process(contentsPtr, contentsPtr, static_cast<unsigned int>(data->size()));
        }

        isSuccess = burstSend(data->contents(), static_cast<uint32_t>(data->size()));
    }

    if (isSuccess)
        burstPush();

    burstEnd();
}

void LogonCommServerSocket::SendPendingSession(const PendingWorldSession& session)
{
    WorldPacket data(LRSMSG_PENDING_WORLD_SESSION, 300);
    data << session.accountId;
    data << session.gameAccountId;
    data << session.realmId;
    data << session.clientBuild;
    data << session.region;
    data << session.expiresAt;
    data << session.gameAccountName;
    data << session.realmJoinTicket;
    data.append(session.worldAuthKeyData.data(), session.worldAuthKeyData.size());
    data.append(session.joinSecret.data(), session.joinSecret.size());
    data << session.gmFlags;
    data << session.accountFlags;
    data << session.forcedLanguage;
    data << session.muted;
    SendPacket(&data);
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

    sLogger.info("Authentication request from {}, id {} - result {}.", getRemoteIp(), uint32_t(realmId), result ? "OK" : "FAIL");

    std::stringstream sstext;
    sstext << "Key: ";
    for (int i = 0; i < 20; ++i)
        sstext << fmt::format("{:02X}", key[i]);

    sLogger.info("{}", sstext.str());

    _rwCrypto.setup(key, 20);
    _sendCrypto.setup(key, 20);

    // packets are encrypted from now on
    use_crypto = true;

    // send the response packet
    WorldPacket data(LRSMSG_AUTH_RESPONSE, 1);
    data << result;
    SendPacket(&data);

    m_logDetail = fmt::format("realm {} {}", uint32_t(realmId), result ? "ok" : "wrong password");

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
    m_logDetail = fmt::format("realm {}, {} accounts", realm_id, count);
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

    m_logAccountId = account_id;
    m_logDetail = fmt::format("realm {}, characters {:+}", realm_id, static_cast<int8_t>(chars_to_add));

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

    // never the password
    m_logDetail = fmt::format("account '{}' failed", accountname);

    const auto* pAccount = sAccountMgr.getAccountByName(accountname);
    if (pAccount == nullptr)
    {
        data << uint32_t(0);
        SendPacket(&data);
        return;
    }

    m_logAccountId = pAccount->AccountId;

    std::string pass;
    pass.assign(accountname);
    pass.push_back(':');
    pass.append(password);
    auto checkPassQuery = sLogonSQL->query("SELECT acc_name, encrypted_password FROM accounts WHERE encrypted_password = SHA(UPPER('%s')) AND acc_name = '%s'", pass.c_str(), accountname.c_str());
    if (!checkPassQuery)
    {
        data << uint32_t(0);
        SendPacket(&data);
        return;
    }

    m_logDetail = fmt::format("account '{}' ok", accountname);
    data << uint32_t(1);
    SendPacket(&data);
}

void LogonCommServerSocket::HandleDatabaseModify(WorldPacket & recvData)
{
    uint32_t method;
    recvData >> method;

    // only realms listed in AllowedModIPs change accounts, bans and passwords
    if (!sMasterLogon.IsServerAllowedMod(getRemoteAddress().s_addr))
    {
        sLogger.failure("LogonComm: database change {} from {} DENIED, not an allowed mod IP.", method, getRemoteIp());
        m_logDetail = fmt::format("method {} denied, not in AllowedModIPs", method);

        // password change: old, new, account; account creation: name, password, account
        if (method == Method_Account_Change_PW || method == Method_Account_Create)
        {
            std::string first;
            std::string second;
            std::string accountName;
            recvData >> first >> second >> accountName;

            WorldPacket data(LRSMSG_ACCOUNT_DB_MODIFY_RESULT, 100);
            data << uint32_t(method) << uint8_t(Result_Account_Not_Allowed) << accountName;
            if (method == Method_Account_Create)
                data << first;
            SendPacket(&data);
        }
        return;
    }

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

            m_logDetail = fmt::format("ban account '{}' {} reason '{}'", account, duration, banreason);

            auto* pAccount = sAccountMgr.getAccountByName(account);
            if (pAccount == nullptr)
                return;

            m_logAccountId = pAccount->AccountId;
            pAccount->Banned = duration;

            // update it in the sql (duh)
            sLogonSQL->execute("UPDATE accounts SET banned = %u, banreason = '%s' WHERE acc_name = \"%s\"", duration, sLogonSQL->escapeString(banreason).c_str(), sLogonSQL->escapeString(account).c_str());

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

            m_logDetail = fmt::format("mute account '{}' {}", account, duration);

            auto* pAccount = sAccountMgr.getAccountByName(account);
            if (pAccount == nullptr)
                return;

            m_logAccountId = pAccount->AccountId;
            pAccount->Muted = duration;

            // update it in the sql (duh)
            sLogonSQL->execute("UPDATE accounts SET muted = %u WHERE acc_name = \"%s\"", duration, sLogonSQL->escapeString(account).c_str());
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

            m_logDetail = fmt::format("ip ban {} {} reason '{}'", ip, duration, banreason);

            if (sIpBanMgr.add(ip, duration))
                sLogonSQL->execute("INSERT INTO ipbans VALUES(\"%s\", %u, \"%s\")", sLogonSQL->escapeString(ip).c_str(), duration, sLogonSQL->escapeString(banreason).c_str());

        }
        break;

        case Method_IP_Unban:
        {
            std::string ip;
            recvData >> ip;

            m_logDetail = fmt::format("ip unban {}", ip);

            if (sIpBanMgr.remove(ip))
                sLogonSQL->execute("DELETE FROM ipbans WHERE ip = \"%s\"", sLogonSQL->escapeString(ip).c_str());

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

            const auto changed = AscEmu::Accounts::changePassword(account_name, old_password, new_password);
            result = static_cast<uint8_t>(changed == AscEmu::Accounts::Result::Ok ? Result_Account_Finished : Result_Account_PW_wrong);
            m_logDetail = fmt::format("change password of account '{}': {}", account_name, AscEmu::Accounts::describe(changed));
            if (changed != AscEmu::Accounts::Result::Ok)
                sLogger.info("LogonComm: password change of account '{}' failed: {}", account_name, AscEmu::Accounts::describe(changed));

            data << uint32_t(method);     // method_id
            data << uint8_t(result);
            data << account_name;       // account_name
            SendPacket(&data);

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

            const auto created = AscEmu::Accounts::createAccount(name, password, "");
            m_logDetail = fmt::format("create account '{}' by '{}': {}", name, account_name, AscEmu::Accounts::describe(created));
            if (created == AscEmu::Accounts::Result::Exists)
                result = static_cast<uint8_t>(Result_Account_Exists);
            else if (created == AscEmu::Accounts::Result::Ok)
                result = static_cast<uint8_t>(Result_Account_Finished);
            else
            {
                sLogger.failure("LogonComm: account creation of '{}' failed: {}", name, AscEmu::Accounts::describe(created));
                result = static_cast<uint8_t>(Result_Account_SQL_error);
            }

            data << uint32_t(method);     // method_id
            data << uint8_t(result);
            data << account_name;       // account_name
            data << name;               // created account name
            SendPacket(&data);
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
            m_logDetail = fmt::format("check account '{}' for '{}'", account_name, request_name);

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
            m_logDetail = fmt::format("id of account '{}' for '{}'", account_name, request_name);

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

    m_logDetail = fmt::format("{} accounts", accountMap.size());

    // remove last ; from string
    if (!accountsArray.empty())
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

namespace
{
    // every realm of the account reaches the same rows, one change at a time
    std::mutex battlePetLock;
}

void LogonCommServerSocket::HandleBattlePetRequest(WorldPacket& recvData)
{
    uint32_t accountId = 0;
    recvData >> accountId;

    std::lock_guard lock(battlePetLock);

    std::vector<LogonComm::BattlePetRecord> pets;
    if (auto result = sLogonSQL->query("SELECT id, species, breed, display_id, level, experience, health, quality, flags, name, name_timestamp "
        "FROM battle_pets WHERE account_id = %u", accountId))
    {
        do
        {
            Field* fields = result->fetch();

            LogonComm::BattlePetRecord pet;
            pet.id = fields[0].asUint32();
            pet.species = fields[1].asUint32();
            pet.breed = fields[2].asUint16();
            pet.displayId = fields[3].asUint32();
            pet.level = fields[4].asUint16();
            pet.experience = fields[5].asUint16();
            pet.health = fields[6].asUint32();
            pet.quality = fields[7].asUint8();
            pet.flags = fields[8].asUint16();
            pet.name = fields[9].asCString();
            pet.nameTimestamp = fields[10].asUint64();
            pets.push_back(std::move(pet));
        }
        while (result->nextRow());
    }

    std::vector<LogonComm::BattlePetSlotRecord> slots;
    if (auto result = sLogonSQL->query("SELECT slot, pet_id, locked FROM battle_pet_slots WHERE account_id = %u ORDER BY slot", accountId))
    {
        do
        {
            Field* fields = result->fetch();

            LogonComm::BattlePetSlotRecord slot;
            slot.index = fields[0].asUint8();
            slot.petId = fields[1].asUint32();
            slot.locked = fields[2].asUint8() != 0;
            slots.push_back(slot);
        }
        while (result->nextRow());
    }

    m_logAccountId = accountId;
    m_logDetail = fmt::format("load {} pets, {} slots", pets.size(), slots.size());

    WorldPacket data(LRSMSG_BATTLE_PET_RESULT, 9 + pets.size() * 48);
    data << accountId << uint32_t(pets.size());
    for (const auto& pet : pets)
        LogonComm::writeBattlePet(data, pet);
    LogonComm::writeBattlePetSlots(data, slots);
    SendPacket(&data);
}

void LogonCommServerSocket::HandleBattlePetSave(WorldPacket& recvData)
{
    uint32_t accountId = 0;
    uint32_t savedCount = 0;
    recvData >> accountId >> savedCount;

    std::lock_guard lock(battlePetLock);

    m_logAccountId = accountId;
    m_logDetail = "save";

    // a pet another realm released stays released
    for (uint32_t i = 0; i < savedCount; ++i)
    {
        const auto pet = LogonComm::readBattlePet(recvData);
        sLogonSQL->waitExecute("UPDATE battle_pets SET species = %u, breed = %u, display_id = %u, level = %u, experience = %u, health = %u, quality = %u, flags = %u, "
            "name = '%s', name_timestamp = %llu WHERE account_id = %u AND id = %u", pet.species, uint32_t(pet.breed), pet.displayId, uint32_t(pet.level),
            uint32_t(pet.experience), pet.health, uint32_t(pet.quality), uint32_t(pet.flags), sLogonSQL->escapeString(pet.name).c_str(),
            static_cast<unsigned long long>(pet.nameTimestamp), accountId, pet.id);
        m_logDetail += fmt::format(" pet {}", pet.id);
    }

    uint32_t removedCount = 0;
    recvData >> removedCount;
    for (uint32_t i = 0; i < removedCount; ++i)
    {
        uint32_t petId = 0;
        recvData >> petId;
        sLogonSQL->waitExecute("DELETE FROM battle_pets WHERE account_id = %u AND id = %u", accountId, petId);
        m_logDetail += fmt::format(" released {}", petId);
    }

    const auto slots = LogonComm::readBattlePetSlots(recvData);
    for (const auto& slot : slots)
    {
        sLogonSQL->waitExecute("REPLACE INTO battle_pet_slots (account_id, slot, pet_id, locked) VALUES (%u, %u, %u, %u)",
            accountId, uint32_t(slot.index), slot.petId, slot.locked ? 1u : 0u);
    }
    m_logDetail += fmt::format(", {} slots", slots.size());
}

void LogonCommServerSocket::HandleBattlePetAdd(WorldPacket& recvData)
{
    uint32_t accountId = 0;
    uint8_t maxPerSpecies = 0;
    uint32_t maxPets = 0;
    recvData >> accountId >> maxPerSpecies >> maxPets;
    auto pet = LogonComm::readBattlePet(recvData);

    std::lock_guard lock(battlePetLock);

    // the limits count the pets of every realm
    uint32_t total = 0;
    uint32_t ofSpecies = 0;
    uint32_t highestId = 0;
    if (auto result = sLogonSQL->query("SELECT COUNT(*), COALESCE(SUM(species = %u), 0), COALESCE(MAX(id), 0) FROM battle_pets WHERE account_id = %u", pet.species, accountId))
    {
        Field* fields = result->fetch();
        total = fields[0].asUint32();
        ofSpecies = fields[1].asUint32();
        highestId = fields[2].asUint32();
    }

    uint8_t result = 0;
    if (total >= maxPets)
        result = LogonComm::BattlePetAddTooManyPets;
    else if (ofSpecies >= maxPerSpecies)
        result = LogonComm::BattlePetAddTooManyOfSpecies;

    if (result == 0)
    {
        pet.id = highestId + 1;
        if (!sLogonSQL->waitExecute("INSERT INTO battle_pets (account_id, id, species, breed, display_id, level, experience, health, quality, flags, name, name_timestamp) "
            "VALUES (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, '%s', %llu)", accountId, pet.id, pet.species, uint32_t(pet.breed), pet.displayId,
            uint32_t(pet.level), uint32_t(pet.experience), pet.health, uint32_t(pet.quality), uint32_t(pet.flags),
            sLogonSQL->escapeString(pet.name).c_str(), static_cast<unsigned long long>(pet.nameTimestamp)))
        {
            result = LogonComm::BattlePetAddFailed;
        }
    }

    m_logAccountId = accountId;
    m_logDetail = result == 0 ? fmt::format("species {} as pet {}", pet.species, pet.id) : fmt::format("species {} refused ({})", pet.species, result);

    WorldPacket data(LRSMSG_BATTLE_PET_ADD_RESULT, 64);
    data << accountId << result;
    LogonComm::writeBattlePet(data, pet);
    SendPacket(&data);
}

void LogonCommServerSocket::HandleAchievementRequest(WorldPacket& recvData)
{
    uint32_t accountId = 0;
    recvData >> accountId;

    std::vector<LogonComm::AccountAchievementRecord> achievements;
    if (auto result = sLogonSQL->query("SELECT achievement, date FROM account_achievement WHERE account_id = %u", accountId))
    {
        do
        {
            Field* fields = result->fetch();
            achievements.push_back({ fields[0].asUint32(), fields[1].asUint64() });
        }
        while (result->nextRow());
    }

    std::vector<LogonComm::AccountCriteriaRecord> criteria;
    if (auto result = sLogonSQL->query("SELECT criteria, counter, date FROM account_achievement_progress WHERE account_id = %u", accountId))
    {
        do
        {
            Field* fields = result->fetch();
            criteria.push_back({ fields[0].asUint32(), fields[1].asUint32(), fields[2].asUint64() });
        }
        while (result->nextRow());
    }

    m_logAccountId = accountId;
    m_logDetail = fmt::format("load {} achievements, {} criteria", achievements.size(), criteria.size());

    WorldPacket data(LRSMSG_ACHIEVEMENT_RESULT, 12 + achievements.size() * 12 + criteria.size() * 16);
    data << accountId;
    LogonComm::writeAccountAchievements(data, achievements, criteria);
    SendPacket(&data);
}

void LogonCommServerSocket::HandleAchievementSave(WorldPacket& recvData)
{
    uint32_t accountId = 0;
    recvData >> accountId;

    std::vector<LogonComm::AccountAchievementRecord> achievements;
    std::vector<LogonComm::AccountCriteriaRecord> criteria;
    LogonComm::readAccountAchievements(recvData, achievements, criteria);

    // the earliest completion and the highest progress of every realm stay
    for (auto const& achievement : achievements)
    {
        sLogonSQL->waitExecute("INSERT INTO account_achievement (account_id, achievement, date) VALUES (%u, %u, %llu) ON DUPLICATE KEY UPDATE date = LEAST(date, VALUES(date))",
            accountId, achievement.achievementId, static_cast<unsigned long long>(achievement.date));
    }

    for (auto const& progress : criteria)
    {
        sLogonSQL->waitExecute("INSERT INTO account_achievement_progress (account_id, criteria, counter, date) VALUES (%u, %u, %u, %llu) "
            "ON DUPLICATE KEY UPDATE date = IF(VALUES(counter) > counter, VALUES(date), date), counter = GREATEST(counter, VALUES(counter))",
            accountId, progress.criteriaId, progress.counter, static_cast<unsigned long long>(progress.date));
    }

    m_logAccountId = accountId;
    m_logDetail = fmt::format("save {} achievements, {} criteria", achievements.size(), criteria.size());
    for (auto const& achievement : achievements)
        m_logDetail += fmt::format(" {}", achievement.achievementId);
}

void LogonCommServerSocket::HandleAchievementReset(WorldPacket& recvData)
{
    uint32_t accountId = 0;
    uint32_t achievementId = 0;
    uint32_t criteriaId = 0;
    recvData >> accountId >> achievementId >> criteriaId;

    m_logAccountId = accountId;
    if (achievementId != 0)
    {
        sLogonSQL->waitExecute("DELETE FROM account_achievement WHERE account_id = %u AND achievement = %u", accountId, achievementId);
        m_logDetail = fmt::format("reset achievement {}", achievementId);
    }

    if (criteriaId != 0)
    {
        sLogonSQL->waitExecute("DELETE FROM account_achievement_progress WHERE account_id = %u AND criteria = %u", accountId, criteriaId);
        m_logDetail = fmt::format("reset criteria {}", criteriaId);
    }
}
