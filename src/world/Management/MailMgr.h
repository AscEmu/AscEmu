/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

// see Stationery.dbc
enum MailStationery
{
    // item:
    MAIL_STATIONERY_TEST1       = 1,     // 8164
    MAIL_STATIONERY_TEST2       = 41,    // 9311
    MAIL_STATIONERY_GM          = 61,    // 18154
    MAIL_STATIONERY_AUCTION     = 62,    // 21140
    MAIL_STATIONERY_VAL         = 64,    // 22058, Valentines day
    MAIL_STATIONERY_CHR         = 65     // 34171, Winter
};

enum MailCheckMask
{
    MAIL_CHECK_MASK_NONE        = 0x00,
    MAIL_CHECK_MASK_READ        = 0x01,
    MAIL_CHECK_MASK_RETURNED    = 0x02,
    MAIL_CHECK_MASK_COPIED      = 0x04,
    MAIL_CHECK_MASK_COD_PAYMENT = 0x08,
    MAIL_CHECK_MASK_HAS_BODY    = 0x10,
};

#define MAIL_MAX_ITEM_SLOT 12

struct MailMessage
{
    uint32 message_id;
    uint32 message_type;
    uint64 player_guid;
    uint64 sender_guid;
    std::string subject;
    std::string body;
    uint32 money;
    std::vector<uint32> items;
    uint32 cod;
    uint32 stationery;
    uint32 expire_time;
    uint32 delivery_time;
    uint32 checked_flag;
    bool deleted_flag;

    bool AddMessageDataToPacket(WorldPacket & data);
};

typedef std::map<uint32, MailMessage> MessageMap;

class Mailbox
{
    protected:

        uint64 owner;
        MessageMap Messages;

    public:

        Mailbox(uint64 owner_) : owner(owner_) {}

        void AddMessage(MailMessage* Message);
        void DeleteMessage(uint32 MessageId, bool sql);
        MailMessage* GetMessage(uint32 message_id)
        {
            MessageMap::iterator iter = Messages.find(message_id);
            if (iter == Messages.end())
                return NULL;
            return &(iter->second);
        }

        WorldPacket* BuildMailboxListingPacket();
        void CleanupExpiredMessages();
        inline size_t MessageCount() { return Messages.size(); }
        void FillTimePacket(WorldPacket & data);
        inline uint64 GetOwner() { return owner; }
        void Load(QueryResult* result);
};


class SERVER_DECL MailSystem : public Singleton<MailSystem>, public EventableObject
{
    public:

        void StartMailSystem();
        MailError DeliverMessage(uint64 recipent, MailMessage* message);
        void RemoveMessageIfDeleted(uint32 message_id, Player* plr);
        void SaveMessageToSQL(MailMessage* message);
        void SendAutomatedMessage(uint32 type, uint64 sender, uint64 receiver, std::string subject, std::string body, uint32 money,
                                  uint32 cod, std::vector<uint64> &item_guids, uint32 stationery, MailCheckMask checked = MAIL_CHECK_MASK_HAS_BODY, uint32 deliverdelay = 0);

        /// overload to keep backward compatibility (passing just 1 item guid instead of a vector)
        void SendAutomatedMessage(uint32 type, uint64 sender, uint64 receiver, std::string subject, std::string body, uint32 money,
                                  uint32 cod, uint64 item_guid, uint32 stationery, MailCheckMask checked = MAIL_CHECK_MASK_HAS_BODY, uint32 deliverdelay = 0);

        void SendCreatureGameobjectMail(uint32 type, uint32 sender, uint64 receiver, std::string subject, std::string body, uint32 money,
                                  uint32 cod, uint64 item_guid, uint32 stationery, MailCheckMask checked = MAIL_CHECK_MASK_HAS_BODY, uint32 deliverdelay = 0);

        inline bool MailOption(uint32 flag)
        {
            return (config_flags & flag) ? true : false;
        }
        uint32 config_flags;

};

#define sMailSystem MailSystem::getSingleton()

#define MAIL_DEFAULT_EXPIRATION_TIME 30

#endif // MAILMGR_H
