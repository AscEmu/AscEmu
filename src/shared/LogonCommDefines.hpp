/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <cstdint>
#include <string>

//////////////////////////////////////////////////////////////////////////////////////////
/// \brief Logon Realm communication
///
/// These opcodes are used for logon/realm communication. All opcodes used by realm is marked
/// as C (client). All opcodes used by logon is marked as S (server). Our logonserver is
/// our communication server, the realm is our communication client.
///
/// \note work in progress.
//////////////////////////////////////////////////////////////////////////////////////////
enum LogonRealmOpcodes
{
    LRCMSG_REALM_REGISTER_REQUEST     = 0x001,  // try register our realm
    LRSMSG_REALM_REGISTER_RESULT      = 0x002,  // register result from logonserver
    LRCMSG_ACC_SESSION_REQUEST        = 0x003,
    LRSMSG_ACC_SESSION_RESULT         = 0x004,
    LRCMSG_LOGON_PING_STATUS          = 0x005,  // request logon online
    LRSMSG_LOGON_PING_RESULT          = 0x006,  // send result if logon is online
    LRCMSG_BUILD_REQUEST              = 0x007,  // Request Build for ip at logonserver
    LRSMSG_BUILD_RESULT               = 0x008,  // Result of client build from logonserver
    LRCMSG_AUTH_REQUEST               = 0x009,  // try authenticate our realm
    LRSMSG_AUTH_RESPONSE              = 0x00A,  // authentication result from logonserver
    LRSMSG_ACC_CHAR_MAPPING_REQUEST   = 0x00B,
    LRCMSG_ACC_CHAR_MAPPING_RESULT    = 0x00C,
    LRCMSG_ACC_CHAR_MAPPING_UPDATE    = 0x00D,
    LRSMSG_SEND_ACCOUNT_DISCONNECT    = 0x00E,  // send when account is disconnected
    LRCMSG_LOGIN_CONSOLE_REQUEST      = 0x00F,
    LRSMSG_LOGIN_CONSOLE_RESULT       = 0x010,
    LRCMSG_ACCOUNT_DB_MODIFY_REQUEST  = 0x011,  // request logon db change
    LRSMSG_ACCOUNT_DB_MODIFY_RESULT   = 0x012,
    LRSMSG_REALM_POPULATION_REQUEST   = 0x013,
    LRCMSG_REALM_POPULATION_RESULT    = 0x014,
    LRCMSG_ACCOUNT_REQUEST            = 0x015,  // request account data
    LRSMSG_ACCOUNT_RESULT             = 0x016,  // send account information to realm
    LRCMSG_ALL_ACCOUNT_REQUEST        = 0x017,  // request all account data
    LRSMSG_ALL_ACCOUNT_RESULT         = 0x018,  // send id, name, rank of all accounts to realm
    LRSMSG_PENDING_WORLD_SESSION      = 0x019,  // Battle.net session announced to the realm before its client connects
    LRCMSG_BATTLE_PET_REQUEST         = 0x01A,  // request the battle pet journal of an account
    LRSMSG_BATTLE_PET_RESULT          = 0x01B,  // the battle pets and battle slots of an account
    LRCMSG_BATTLE_PET_SAVE            = 0x01C,  // changed and released battle pets of an account
    LRCMSG_BATTLE_PET_ADD             = 0x01D,  // new battle pet, the logonserver hands out its id
    LRSMSG_BATTLE_PET_ADD_RESULT      = 0x01E,  // the new battle pet with its id or the reason it was refused

    LRMSG_MAX_OPCODES                           // max opcodes
};

inline const char* getLogonRealmOpcodeName(uint32_t opcode)
{
    switch (opcode)
    {
        case LRCMSG_REALM_REGISTER_REQUEST:     return "LRCMSG_REALM_REGISTER_REQUEST";
        case LRSMSG_REALM_REGISTER_RESULT:      return "LRSMSG_REALM_REGISTER_RESULT";
        case LRCMSG_ACC_SESSION_REQUEST:        return "LRCMSG_ACC_SESSION_REQUEST";
        case LRSMSG_ACC_SESSION_RESULT:         return "LRSMSG_ACC_SESSION_RESULT";
        case LRCMSG_LOGON_PING_STATUS:          return "LRCMSG_LOGON_PING_STATUS";
        case LRSMSG_LOGON_PING_RESULT:          return "LRSMSG_LOGON_PING_RESULT";
        case LRCMSG_BUILD_REQUEST:              return "LRCMSG_BUILD_REQUEST";
        case LRSMSG_BUILD_RESULT:               return "LRSMSG_BUILD_RESULT";
        case LRCMSG_AUTH_REQUEST:               return "LRCMSG_AUTH_REQUEST";
        case LRSMSG_AUTH_RESPONSE:              return "LRSMSG_AUTH_RESPONSE";
        case LRSMSG_ACC_CHAR_MAPPING_REQUEST:   return "LRSMSG_ACC_CHAR_MAPPING_REQUEST";
        case LRCMSG_ACC_CHAR_MAPPING_RESULT:    return "LRCMSG_ACC_CHAR_MAPPING_RESULT";
        case LRCMSG_ACC_CHAR_MAPPING_UPDATE:    return "LRCMSG_ACC_CHAR_MAPPING_UPDATE";
        case LRSMSG_SEND_ACCOUNT_DISCONNECT:    return "LRSMSG_SEND_ACCOUNT_DISCONNECT";
        case LRCMSG_LOGIN_CONSOLE_REQUEST:      return "LRCMSG_LOGIN_CONSOLE_REQUEST";
        case LRSMSG_LOGIN_CONSOLE_RESULT:       return "LRSMSG_LOGIN_CONSOLE_RESULT";
        case LRCMSG_ACCOUNT_DB_MODIFY_REQUEST:  return "LRCMSG_ACCOUNT_DB_MODIFY_REQUEST";
        case LRSMSG_ACCOUNT_DB_MODIFY_RESULT:   return "LRSMSG_ACCOUNT_DB_MODIFY_RESULT";
        case LRSMSG_REALM_POPULATION_REQUEST:   return "LRSMSG_REALM_POPULATION_REQUEST";
        case LRCMSG_REALM_POPULATION_RESULT:    return "LRCMSG_REALM_POPULATION_RESULT";
        case LRCMSG_ACCOUNT_REQUEST:            return "LRCMSG_ACCOUNT_REQUEST";
        case LRSMSG_ACCOUNT_RESULT:             return "LRSMSG_ACCOUNT_RESULT";
        case LRCMSG_ALL_ACCOUNT_REQUEST:        return "LRCMSG_ALL_ACCOUNT_REQUEST";
        case LRSMSG_ALL_ACCOUNT_RESULT:         return "LRSMSG_ALL_ACCOUNT_RESULT";
        case LRSMSG_PENDING_WORLD_SESSION:      return "LRSMSG_PENDING_WORLD_SESSION";
        case LRCMSG_BATTLE_PET_REQUEST:         return "LRCMSG_BATTLE_PET_REQUEST";
        case LRSMSG_BATTLE_PET_RESULT:          return "LRSMSG_BATTLE_PET_RESULT";
        case LRCMSG_BATTLE_PET_SAVE:            return "LRCMSG_BATTLE_PET_SAVE";
        case LRCMSG_BATTLE_PET_ADD:             return "LRCMSG_BATTLE_PET_ADD";
        case LRSMSG_BATTLE_PET_ADD_RESULT:      return "LRSMSG_BATTLE_PET_ADD_RESULT";
        default:                                return "UNKNOWN";
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// A Battle.net session (6.x and later clients) the logonserver announces to the realm with
/// LRSMSG_PENDING_WORLD_SESSION when the client joins it. The realm keeps it until the client
/// presents the realm join ticket in its auth session or the announcement expires.
struct PendingWorldSession
{
    uint32_t accountId = 0;             // accounts.id, the Battle.net login of the client
    uint32_t gameAccountId = 0;         // accounts.id, the game account (the same account)
    uint32_t realmId = 0;
    uint32_t clientBuild = 0;
    uint32_t region = 0;
    uint64_t expiresAt = 0;             // unix time the announcement expires at
    std::string gameAccountName;
    std::string realmJoinTicket;
    std::array<uint8_t, 64> worldAuthKeyData{};
    std::array<uint8_t, 32> joinSecret{};

    // account data of the game account, as the session result of the classic login carries it
    std::string gmFlags;
    uint8_t accountFlags = 0;
    std::string forcedLanguage;
    uint32_t muted = 0;
};

enum AccountDatabaseMethod
{
    Method_Account_Ban = 1,
    Method_Account_Set_GM,
    Method_Account_Set_Mute,
    Method_IP_Ban,
    Method_IP_Unban,
    Method_Account_Change_PW,
    Method_Account_Create
};

enum AccountDatabaseResult
{
    Result_Account_PW_wrong = 1,
    Result_Account_SQL_error,
    Result_Account_Finished,
    Result_Account_Exists,
    Result_Account_Not_Allowed                  // the realm is not listed in AllowedModIPs
};

#pragma pack(push, 1)
struct LogonWorldPacket
{
    uint16_t opcode;
    uint32_t size;
};
#pragma pack(pop)


inline void byteSwapUInt16(uint16_t* byte16)
{
    *byte16 = ((*byte16 >> 8) & 0xFF00) | ((*byte16 << 8) & 0xFF0000);
}

inline void byteSwapUInt32(uint32_t* byte32)
{
    *byte32 = ((*byte32 >> 24) & 0xFF) | ((*byte32 >> 8) & 0xFF00) | ((*byte32 << 8) & 0xFF0000) | (*byte32 << 24);
}

