/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#ifndef MAILMGR_H
#define MAILMGR_H

#include "Server/EventableObject.h"
#include <vector>

#define MAIL_MAX_ITEM_SLOT 12
#define MAIL_DEFAULT_EXPIRATION_TIME 30

class QueryResult;
class Player;
/// \todo refactoring
struct AuctionEntry;
struct CalendarEvent;

enum MailCMD
{
    MAIL_RES_MAIL_SENT          = 0,
    MAIL_RES_MONEY_TAKEN        = 1,
    MAIL_RES_ITEM_TAKEN         = 2,
    MAIL_RES_RETURNED_TO_SENDER = 3,
    MAIL_RES_DELETED            = 4,
    MAIL_RES_MADE_PERMANENT     = 5
};

enum MailFlags
{
    MAIL_FLAG_NO_COST_FOR_GM                    = 1,
    MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION      = 2,
    MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION_GM   = 4,
    MAIL_FLAG_DISABLE_POSTAGE_COSTS             = 8,
    MAIL_FLAG_DISABLE_HOUR_DELAY_FOR_ITEMS      = 16,
    MAIL_FLAG_NO_EXPIRY                         = 32
};

enum MailTypes
{
    MAIL_TYPE_NORMAL      = 0,
    MAIL_TYPE_COD         = 1,
    MAIL_TYPE_AUCTION     = 2,
    MAIL_TYPE_CREATURE    = 3,
    MAIL_TYPE_GAMEOBJECT  = 4,
    MAIL_TYPE_ITEM        = 5
};

enum MailError
{
    MAIL_OK                             = 0,
    MAIL_ERR_BAG_FULL                   = 1,
    MAIL_ERR_CANNOT_SEND_TO_SELF        = 2,
    MAIL_ERR_NOT_ENOUGH_MONEY           = 3,
    MAIL_ERR_RECIPIENT_NOT_FOUND        = 4,
    MAIL_ERR_NOT_YOUR_ALLIANCE          = 5,
    MAIL_ERR_INTERNAL_ERROR             = 6,
    MAIL_ERR_DISABLED_FOR_TRIAL_ACC     = 14,
    MAIL_ERR_RECIPIENT_CAP_REACHED      = 15,
    MAIL_ERR_CANT_SEND_WRAPPED_COD      = 16,
    MAIL_ERR_MAIL_AND_CHAT_SUSPENDED    = 17,
    MAIL_ERR_TOO_MANY_ATTACHMENTS       = 18,
    MAIL_ERR_MAIL_ATTACHMENT_INVALID    = 19,
    MAIL_ERR_ITEM_HAS_EXPIRED           = 21
};

// Values based on Stationery.dbc
enum MailStationery
{
    // item:
    MAIL_STATIONERY_TEST1               = 1,     // 8164
    MAIL_STATIONERY_TEST2               = 41,    // 9311
    MAIL_STATIONERY_GM                  = 61,    // 18154
    MAIL_STATIONERY_AUCTION             = 62,    // 21140
    MAIL_STATIONERY_VAL                 = 64,    // 22058, Valentines day
    // WOTLK
    MAIL_STATIONERY_CHR                 = 65,    // 34171, Winter
    // Cataclysm
    MAIL_STATIONERY_STATIONERY_ORP      = 67,    // 46830
};

enum MailCheckMask
{
    MAIL_CHECK_MASK_NONE                = 0x00,
    MAIL_CHECK_MASK_READ                = 0x01,
    MAIL_CHECK_MASK_RETURNED            = 0x02,
    MAIL_CHECK_MASK_COPIED              = 0x04,
    MAIL_CHECK_MASK_COD_PAYMENT         = 0x08,
    MAIL_CHECK_MASK_HAS_BODY            = 0x10,
};

struct MailMessage
{
    uint32_t message_id;
    uint32_t message_type;
    uint64_t player_guid;
    uint64_t sender_guid;
    std::string subject;
    std::string body;
    uint32_t money;
    std::vector<uint32_t> items;
    uint32_t cod;
    uint32_t stationery;
    uint32_t expire_time;
    uint32_t delivery_time;
    uint32_t checked_flag;
    bool deleted_flag;
};

typedef std::map<uint32_t, MailMessage> MessageMap;

class Mailbox
{
protected:
    uint64_t owner;

public:
    MessageMap Messages;

    Mailbox(uint64_t owner_) : owner(owner_) {}

    void AddMessage(MailMessage* Message);
    void DeleteMessage(uint32_t MessageId, bool sql);
    MailMessage* GetMessageById(uint32_t message_id)
    {
        MessageMap::iterator iter = Messages.find(message_id);
        if (iter == Messages.end())
            return NULL;
        return &(iter->second);
    }

    void CleanupExpiredMessages();
    inline size_t MessageCount() { return Messages.size(); }
    inline uint64_t GetOwner() { return owner; }
    void Load(QueryResult* result);
};


class SERVER_DECL MailSystem : public EventableObject
{
private:
    MailSystem() = default;
    ~MailSystem() = default;

public:
    static MailSystem& getInstance();

    MailSystem(MailSystem&&) = delete;
    MailSystem(MailSystem const&) = delete;
    MailSystem& operator=(MailSystem&&) = delete;
    MailSystem& operator=(MailSystem const&) = delete;

    void StartMailSystem();
    MailError DeliverMessage(uint64_t recipent, MailMessage* message);
    void RemoveMessageIfDeleted(uint32_t message_id, Player* plr);
    void SaveMessageToSQL(MailMessage* message);
    void SendAutomatedMessage(uint32_t type, uint64_t sender, uint64_t receiver, std::string subject, std::string body, uint32_t money,
                              uint32_t cod, std::vector<uint64_t> &item_guids, uint32_t stationery, MailCheckMask checked = MAIL_CHECK_MASK_HAS_BODY, uint32_t deliverdelay = 0);

    /// overload to keep backward compatibility (passing just 1 item guid instead of a vector)
    void SendAutomatedMessage(uint32_t type, uint64_t sender, uint64_t receiver, std::string subject, std::string body, uint32_t money,
                              uint32_t cod, uint64_t item_guid, uint32_t stationery, MailCheckMask checked = MAIL_CHECK_MASK_HAS_BODY, uint32_t deliverdelay = 0);

    void SendCreatureGameobjectMail(uint32_t type, uint32_t sender, uint64_t receiver, std::string subject, std::string body, uint32_t money,
                              uint32_t cod, uint64_t item_guid, uint32_t stationery, MailCheckMask checked = MAIL_CHECK_MASK_HAS_BODY, uint32_t deliverdelay = 0);

    inline bool MailOption(uint32_t flag)
    {
        return (config_flags & flag) ? true : false;
    }
    uint32_t config_flags = 0;
};

#define sMailSystem MailSystem::getInstance()

#endif // MAILMGR_H
