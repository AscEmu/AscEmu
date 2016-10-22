/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#define MAX_GUILD_BANK_TAB_TEXT_LEN 500
#define GOLD 10000
#define EMBLEM_PRICE 10 * GOLD


std::string _GetGuildEventString(GuildEvents event)
{
    switch (event)
    {
        case GE_PROMOTION:
            return "Member promotion";
        case GE_DEMOTION:
            return "Member demotion";
        case GE_MOTD:
            return "Guild MOTD";
        case GE_JOINED:
            return "Member joined";
        case GE_LEFT:
            return "Member left";
        case GE_REMOVED:
            return "Member removed";
        case GE_LEADER_IS:
            return "Leader is";
        case GE_LEADER_CHANGED:
            return "Leader changed";
        case GE_DISBANDED:
            return "Guild disbanded";
        case GE_TABARDCHANGE:
            return "Tabard change";
        case GE_RANK_UPDATED:
            return "Rank updated";
        case GE_RANK_DELETED:
            return "Rank deleted";
        case GE_SIGNED_ON:
            return "Member signed on";
        case GE_SIGNED_OFF:
            return "Member signed off";
        case GE_GUILDBANKBAGSLOTS_CHANGED:
            return "Bank bag slots changed";
        case GE_BANK_TAB_PURCHASED:
            return "Bank tab purchased";
        case GE_BANK_TAB_UPDATED:
            return "Bank tab updated";
        case GE_BANK_MONEY_SET:
            return "Bank money set";
        case GE_BANK_MONEY_CHANGED:
            return "Bank money changed";
        case GE_BANK_TEXT_CHANGED:
            return "Bank tab text changed";
        default:
            break;
        }
        return "<None>";
}

inline uint32 _GetGuildBankTabPrice(uint8 tabId)
{
    switch (tabId)
    {
        case 0: return 100;
        case 1: return 250;
        case 2: return 500;
        case 3: return 1000;
        case 4: return 2500;
        case 5: return 5000;
        default: return 0;
    }
}

void Guild::SendCommandResult(WorldSession* session, GuildCommandType type, GuildCommandError errCode, std::string const& param)
{
    WorldPacket data(SMSG_GUILD_COMMAND_RESULT, 8 + param.size() + 1);
    data << uint32(type);
    data << param;
    data << uint32(errCode);
    session->SendPacket(&data);

    Log.Debug("Guild", "SMSG_GUILD_COMMAND_RESULT [%s]: Type: %u, code: %u, param: %s", session->GetPlayer()->GetName(), type, errCode, param.c_str());
}

void Guild::SendSaveEmblemResult(WorldSession* session, GuildEmblemError errCode)
{
    WorldPacket data(MSG_SAVE_GUILD_EMBLEM, 4);
    data << uint32(errCode);
    session->SendPacket(&data);

    Log.Debug("Guild", "MSG_SAVE_GUILD_EMBLEM [%s] Code: %u", session->GetPlayer()->GetName(), errCode);
}

// GuildLogHolder
Guild::GuildLogHolder::~GuildLogHolder()
{
    // Cleanup
    for (GuildLog::iterator itr = m_log.begin(); itr != m_log.end(); ++itr)
        delete (*itr);
}


// Adds event loaded from database to collection
inline void Guild::GuildLogHolder::LoadEvent(GuildLogEntry* entry)
{
    if (m_nextGUID == uint32(GUILD_EVENT_LOG_GUID_UNDEFINED))
        m_nextGUID = entry->GetGUID();
    m_log.push_front(entry);
}

// Adds new event happened in game.
// If maximum number of events is reached, oldest event is removed from collection.
inline void Guild::GuildLogHolder::AddEvent(GuildLogEntry* entry)
{
    // Check max records limit
    if (m_log.size() >= m_maxRecords)
    {
        GuildLogEntry* oldEntry = m_log.front();
        delete oldEntry;
        m_log.pop_front();
    }
    // Add event to list
    m_log.push_back(entry);
    // Save to DB
    entry->SaveToDB();
}

// Writes information about all events into packet.
inline void Guild::GuildLogHolder::WritePacket(WorldPacket& data) const
{
    ByteBuffer buffer;
    data.writeBits(m_log.size(), 23);
    for (GuildLog::const_iterator itr = m_log.begin(); itr != m_log.end(); ++itr)
        (*itr)->WritePacket(data, buffer);

    data.flushBits();
    data.append(buffer);
}

inline uint32 Guild::GuildLogHolder::GetNextGUID()
{
    // Next guid was not initialized. It means there are no records for this holder in DB yet.
    // Start from the beginning.
    if (m_nextGUID == uint32(GUILD_EVENT_LOG_GUID_UNDEFINED))
        m_nextGUID = 0;
    else
        m_nextGUID = (m_nextGUID + 1) % m_maxRecords;
    return m_nextGUID;
}

// EventLogEntry
void Guild::GuildEventLogEntry::SaveToDB() const
{
        CharacterDatabase.Execute("DELETE FROM guild_eventlog WHERE guildId = %u AND logGuid = %u", m_guildId, m_guid);
        CharacterDatabase.Execute("INSERT INTO guild_eventlog VALUES('%u', '%u', '%u', '%u', '%u', '%u', '%llu')", 
                                  m_guildId, m_guid, uint8(m_eventType), m_playerGuid1, m_playerGuid2, (uint32)m_newRank, m_timestamp );
}

void Guild::GuildEventLogEntry::WritePacket(WorldPacket& data, ByteBuffer& content) const
{
    ObjectGuid guid1 = MAKE_NEW_GUID(m_playerGuid1, 0, HIGHGUID_TYPE_PLAYER);
    ObjectGuid guid2 = MAKE_NEW_GUID(m_playerGuid2, 0, HIGHGUID_TYPE_PLAYER);

    data.writeBit(guid1[2]);
    data.writeBit(guid1[4]);
    data.writeBit(guid2[7]);
    data.writeBit(guid2[6]);
    data.writeBit(guid1[3]);
    data.writeBit(guid2[3]);
    data.writeBit(guid2[5]);
    data.writeBit(guid1[7]);
    data.writeBit(guid1[5]);
    data.writeBit(guid1[0]);
    data.writeBit(guid2[4]);
    data.writeBit(guid2[2]);
    data.writeBit(guid2[0]);
    data.writeBit(guid2[1]);
    data.writeBit(guid1[1]);
    data.writeBit(guid1[6]);

    content.WriteByteSeq(guid2[3]);
    content.WriteByteSeq(guid2[2]);
    content.WriteByteSeq(guid2[5]);

    // New Rank
    content << uint8(m_newRank);

    content.WriteByteSeq(guid2[4]);
    content.WriteByteSeq(guid1[0]);
    content.WriteByteSeq(guid1[4]);

    // Event timestamp
    content << uint32(::time(NULL) - m_timestamp);

    content.WriteByteSeq(guid1[7]);
    content.WriteByteSeq(guid1[3]);
    content.WriteByteSeq(guid2[0]);
    content.WriteByteSeq(guid2[6]);
    content.WriteByteSeq(guid2[7]);
    content.WriteByteSeq(guid1[5]);

    // Event type
    content << uint8(m_eventType);

    content.WriteByteSeq(guid2[1]);
    content.WriteByteSeq(guid1[2]);
    content.WriteByteSeq(guid1[6]);
    content.WriteByteSeq(guid1[1]);
}


// BankEventLogEntry
void Guild::GuildBankEventLogEntry::SaveToDB() const
{
        CharacterDatabase.Execute("DELETE FROM guild_bank_eventlog WHERE guildId = %u AND logGuid = %u AND tabId = %u",
                                  m_guildId, m_guid, m_bankTabId);
   
        CharacterDatabase.Execute("INSERT INTO guild_bank_eventlog VALUES('%u', '%u', '%u', '%u', '%u', '%llu', '%u', '%u', '%llu')",
                                  m_guildId, m_guid, m_bankTabId, (uint32)m_eventType, m_playerGuid, m_itemOrMoney, (uint32)m_itemStackCount,
                                  (uint32)m_destTabId, m_timestamp);
}

void Guild::GuildBankEventLogEntry::WritePacket(WorldPacket& data, ByteBuffer& content) const
{
    ObjectGuid logGuid = MAKE_NEW_GUID(m_playerGuid, 0, HIGHGUID_TYPE_PLAYER);

    bool hasItem = m_eventType == GUILD_BANK_LOG_DEPOSIT_ITEM || m_eventType == GUILD_BANK_LOG_WITHDRAW_ITEM ||
        m_eventType == GUILD_BANK_LOG_MOVE_ITEM || m_eventType == GUILD_BANK_LOG_MOVE_ITEM2;

    bool itemMoved = (m_eventType == GUILD_BANK_LOG_MOVE_ITEM || m_eventType == GUILD_BANK_LOG_MOVE_ITEM2);

    bool hasStack = (hasItem && m_itemStackCount > 1) || itemMoved;

    data.writeBit(IsMoneyEvent());
    data.writeBit(logGuid[4]);
    data.writeBit(logGuid[1]);
    data.writeBit(hasItem);
    data.writeBit(hasStack);
    data.writeBit(logGuid[2]);
    data.writeBit(logGuid[5]);
    data.writeBit(logGuid[3]);
    data.writeBit(logGuid[6]);
    data.writeBit(logGuid[0]);
    data.writeBit(itemMoved);
    data.writeBit(logGuid[7]);

    content.WriteByteSeq(logGuid[6]);
    content.WriteByteSeq(logGuid[1]);
    content.WriteByteSeq(logGuid[5]);
    if (hasStack)
        content << uint32(m_itemStackCount);

    content << uint8(m_eventType);
    content.WriteByteSeq(logGuid[2]);
    content.WriteByteSeq(logGuid[4]);
    content.WriteByteSeq(logGuid[0]);
    content.WriteByteSeq(logGuid[7]);
    content.WriteByteSeq(logGuid[3]);
    if (hasItem)
        content << uint32(m_itemOrMoney);

    content << uint32(time(NULL) - m_timestamp);

    if (IsMoneyEvent())
        content << uint64(m_itemOrMoney);

    if (itemMoved)
        content << uint8(m_destTabId);
}

void Guild::GuildNewsLogEntry::SaveToDB() const
{
        CharacterDatabase.Execute("DELETE FROM guild_newslog WHERE guildId = %u AND logGuid = %u", m_guildId, GetGUID());

        CharacterDatabase.Execute("INSERT INTO guild_newslog VALUES('%u', '%u', '%u', '%u', '%u', '%u', '%llu')", 
                                  m_guildId, GetGUID(), (uint32)GetType(), (uint32)GetPlayerGuid(), GetFlags(), GetValue(), GetTimestamp());
}

void Guild::GuildNewsLogEntry::WritePacket(WorldPacket& data, ByteBuffer& /*content*/) const
{
    data.writeBits(0, 26); // Not yet implemented used for guild achievements
    ObjectGuid guid = GetPlayerGuid();

    data.writeBit(guid[7]);
    data.writeBit(guid[0]);
    data.writeBit(guid[6]);
    data.writeBit(guid[5]);
    data.writeBit(guid[4]);
    data.writeBit(guid[3]);
    data.writeBit(guid[1]);
    data.writeBit(guid[2]);

    data.flushBits();

    data.WriteByteSeq(guid[5]);

    data << uint32(GetFlags());   // 1 sticky
    data << uint32(GetValue());
    data << uint32(0);            // always 0

    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[1]);

    data << uint32(GetGUID());
    data << uint32(GetType());
    data.AppendPackedTime(GetTimestamp());
}

void Guild::GuildRankInfo::LoadFromDB(Field* fields)
{
    m_rankId = fields[1].GetUInt8();
    m_name = fields[2].GetString();
    m_rights = fields[3].GetUInt32();
    m_bankMoneyPerDay = fields[4].GetUInt32();
    if (m_rankId == GR_GUILDMASTER)                 // Prevent loss of leader rights
        m_rights |= GR_RIGHT_ALL;
}

void Guild::GuildRankInfo::SaveToDB(bool _delete) const
{
    if (_delete)
    {
        CharacterDatabase.Execute("DELETE FROM guild_rank WHERE guildId = %u AND rid = %u", m_guildId, (uint32)m_rankId);
    }
    else
    {
        CharacterDatabase.Execute("DELETE FROM guild_rank WHERE guildId = %u AND rid = %u", m_guildId, (uint32)m_rankId);
        CharacterDatabase.Execute("INSERT INTO guild_rank (guildId, rid, rname, rights, bankMoneyPerDay) VALUES ('%u', '%u', '%s', '%u', '0')",
            m_guildId, (uint32)m_rankId, m_name.c_str(), m_rights);
    }
}

void Guild::GuildRankInfo::CreateMissingTabsIfNeeded(uint8 tabs, bool _delete, bool logOnCreate /* = false */)
{
    for (uint8 i = 0; i < tabs; ++i)
    {
        GuildBankRightsAndSlots& rightsAndSlots = m_bankTabRightsAndSlots[i];
        if (rightsAndSlots.GetTabId() == i)
            continue;

        rightsAndSlots.SetTabId(i);
        if (m_rankId == GR_GUILDMASTER)
            rightsAndSlots.SetGuildMasterValues();

        if (logOnCreate)
            Log.Error("Guild", "Guild %u has broken Tab %u for rank %u. Created default tab.", m_guildId, i, (uint32)m_rankId);

            CharacterDatabase.Execute("DELETE FROM guild_bank_right WHERE guildId = %u AND tabId = %u", m_guildId, i);

            CharacterDatabase.Execute("INSERT INTO guild_bank_right VALUES('%u', '%u', '%u', '%u', '%u')",
                                      m_guildId, i, (uint32)m_rankId, (uint32)rightsAndSlots.GetRights(), rightsAndSlots.GetSlots());
    }
}

void Guild::GuildRankInfo::SetName(std::string const& name)
{
    if (m_name == name)
        return;

    m_name = name;

    CharacterDatabase.Execute("UPDATE guild_rank SET rname = '%s', rid = %u WHERE guildId = %u", m_name.c_str(), (uint32)m_rankId, m_guildId);
}

void Guild::GuildRankInfo::SetRights(uint32 rights)
{
    if (m_rankId == GR_GUILDMASTER)                     // Prevent loss of leader rights
        rights = GR_RIGHT_ALL;

    if (m_rights == rights)
        return;

    m_rights = rights;

    CharacterDatabase.Execute("UPDATE guild_rank SET rights = '%u', rid = '%u' WHERE guildId = %u", m_rights, (uint32)m_rankId, m_guildId);
}

void Guild::GuildRankInfo::SetBankMoneyPerDay(uint32 money)
{
    if (m_rankId == GR_GUILDMASTER)                     // Prevent loss of leader rights
        money = uint32(GUILD_WITHDRAW_MONEY_UNLIMITED);

    if (m_bankMoneyPerDay == money)
        return;

    m_bankMoneyPerDay = money;

    CharacterDatabase.Execute("UPDATE guild_rank SET BankMoneyPerDay = '%u', rid = '%u' WHERE guildId = %u", money, (uint32)m_rankId, m_guildId);
}

void Guild::GuildRankInfo::SetBankTabSlotsAndRights(GuildBankRightsAndSlots rightsAndSlots, bool saveToDB)
{
    if (m_rankId == GR_GUILDMASTER)                     // Prevent loss of leader rights
        rightsAndSlots.SetGuildMasterValues();

    GuildBankRightsAndSlots& guildBR = m_bankTabRightsAndSlots[rightsAndSlots.GetTabId()];
    guildBR = rightsAndSlots;

    if (saveToDB)
    {
        CharacterDatabase.Execute("DELETE FROM guild_bank_right WHERE guildId = %u AND tabId = %u", m_guildId, (uint32)guildBR.GetTabId());

        CharacterDatabase.Execute("INSERT INTO guild_bank_right VALUES('%u', '%u', '%u', '%u', '%u')",
            m_guildId, (uint32)guildBR.GetTabId(), (uint32)m_rankId, (uint32)guildBR.GetRights(), guildBR.GetSlots());
    }
}

// BankTab
void Guild::GuildBankTab::LoadFromDB(Field* fields)
{
    m_name = fields[2].GetString();
    m_icon = fields[3].GetString();
    m_text = fields[4].GetString();
}

bool Guild::GuildBankTab::LoadItemFromDB(Field* fields)
{
    uint8 slotId = fields[2].GetUInt8();
    uint32 itemGuid = fields[3].GetUInt32();

    Item* pItem = objmgr.LoadItem(fields[3].GetUInt32());
    if (pItem == nullptr)
    {
        CharacterDatabase.Execute("DELETE FROM guild_bankitems WHERE itemGuid = %u AND guildId = %u AND tabId = %u", 
                                  fields[3].GetUInt32(), m_guildId, (uint32)fields[1].GetUInt8());
    }

    m_items[slotId] = pItem;
    return true;
}

void Guild::GuildBankTab::Delete(bool removeItemsFromDB)
{
    for (uint8 slotId = 0; slotId < GUILD_BANK_MAX_SLOTS; ++slotId)
        if (Item* pItem = m_items[slotId])
        {
            pItem->RemoveFromWorld();
            if (removeItemsFromDB)
                pItem->DeleteFromDB();
            delete pItem;
            pItem = nullptr;
        }
}

void Guild::GuildBankTab::SetInfo(std::string const& name, std::string const& icon)
{
    if (m_name == name && m_icon == icon)
        return;

    m_name = name;
    m_icon = icon;

    CharacterDatabase.Execute("UPDATE guild_bank_tab SET tabName = '%s' , tabIcon = '%s' WHERE guildId = '%u' AND tabId = '%u' ",
        m_name.c_str(), m_icon.c_str(), m_guildId, (uint32)m_tabId);
}

void Guild::GuildBankTab::SetText(std::string const& text)
{
    if (m_text == text)
        return;

    m_text = text;

    CharacterDatabase.Execute("UPDATE guild_bank_tab SET tabText = '%s' , tabIcon = '%s' WHERE guildId = %u AND tabId = %u ",
        m_text.c_str(), m_icon.c_str(), m_guildId, (uint32)m_tabId);
}

void Guild::GuildBankTab::SendText(Guild const* guild, WorldSession* session) const
{
    WorldPacket data(SMSG_GUILD_BANK_QUERY_TEXT_RESULT, 1 + m_text.size() + 1);
    data.writeBits(m_text.length(), 14);
    data << uint32(m_tabId);
    data.WriteString(m_text);

    if (session)
    {
       Log.Debug("Guild", "SMSG_GUILD_BANK_QUERY_TEXT_RESULT [%s]: Tabid: %u, Text: %s", 
                 session->GetPlayer()->GetNameString(), (uint32)m_tabId, m_text.c_str());
       session->SendPacket(&data);
    }
    else
    {
        Log.Debug("Guild", "SMSG_GUILD_BANK_QUERY_TEXT_RESULT [Broadcast]: Tabid: %u, Text: %s", (uint32)m_tabId, m_text.c_str());
        guild->BroadcastPacket(&data);
    }
}

// Member
void Guild::GuildMember::SetStats(Player* player)
{
    m_name = player->GetName();
    m_level = player->getLevel();
    m_class = player->getClass();
    m_zoneId = player->GetZoneId();
    m_accountId = player->GetSession()->GetAccountId();
    m_achievementPoints = 0; // Achievment Points
}

void Guild::GuildMember::SetStats(std::string const& name, uint8 level, uint8 _class, uint32 zoneId, uint32 accountId, uint32 reputation)
{
    m_name = name;
    m_level = level;
    m_class = _class;
    m_zoneId = zoneId;
    m_accountId = accountId;
    m_totalReputation = reputation;
}

void Guild::GuildMember::SetPublicNote(std::string const& publicNote)
{
    if (m_publicNote == publicNote)
        return;

    m_publicNote = publicNote;

    CharacterDatabase.Execute("UPDATE guild_member SET pnote = '%s' WHERE playerGuid = %u",
        publicNote.c_str(), Arcemu::Util::GUID_LOPART(m_guid));
}

void Guild::GuildMember::SetOfficerNote(std::string const& officerNote)
{
    if (m_officerNote == officerNote)
        return;

    m_officerNote = officerNote;

    CharacterDatabase.Execute("UPDATE guild_member SET offnote = '%s' WHERE playerGuid = %u",
        officerNote.c_str(), Arcemu::Util::GUID_LOPART(m_guid));
}

void Guild::GuildMember::ChangeRank(uint8 newRank)
{
    m_rankId = newRank;

    // Update rank information in player's field, if he is online.
    if (Player* player = objmgr.GetPlayer(Arcemu::Util::GUID_LOPART(m_guid)))
        player->SetRank(newRank);

    CharacterDatabase.Execute("UPDATE guild_member SET rank = '%u' WHERE playerGuid = %u",
        (uint32)newRank, Arcemu::Util::GUID_LOPART(m_guid));
}

void Guild::GuildMember::SaveToDB(bool _delete) const
{
        CharacterDatabase.Execute("INSERT INTO guild_member (guildId, playerGuid, rank, pnote, offnote) VALUES ('%u', '%u', '%u', '%s', '%s')",
            m_guildId, Arcemu::Util::GUID_LOPART(m_guid), (uint32)m_rankId, m_publicNote.c_str(), m_officerNote.c_str());
}

// Loads member's data from database.
// If member has broken fields (level, class) returns false.
// In this case member has to be removed from guild.
bool Guild::GuildMember::LoadFromDB(Field* fields, Field* fields2)
{
    PlayerInfo* plr = objmgr.GetPlayerInfo((fields[1].GetUInt32()));
    if (plr == nullptr)
        return false;

    plr->m_guild = fields[0].GetUInt32();
    plr->guildRank = fields[2].GetUInt32();
    m_publicNote = fields[3].GetString();
    m_officerNote = fields[4].GetString();

    for (uint8 i = 0; i <= GUILD_BANK_MAX_TABS; ++i)
        m_bankWithdraw[i] = fields2[1 + i].GetUInt32();

    SetStats(plr->name, plr->lastLevel, plr->cl, plr->lastZone, plr->acct, 0);
    m_logoutTime = plr->lastOnline;      // characters.logout_time
    m_totalActivity = 0;
    m_weekActivity = 0;
    m_weekReputation = 0;

    if (!CheckStats())
        return false;

    if (!m_zoneId)
    {
        Log.Error("Guild", "Player (GUID: %u) has broken zone-data", Arcemu::Util::GUID_LOPART(m_guid));
        m_zoneId = objmgr.GetPlayer(Arcemu::Util::GUID_LOPART(m_guid))->GetZoneId();
    }
    ResetFlags();
    return true;
}

// Validate player fields. Returns false if corrupted fields are found.
bool Guild::GuildMember::CheckStats() const
{
    if (m_level < 1)
    {
        Log.Error("Guild", "Player (GUID: %u) has a broken data in field `characters`.`level`, deleting him from guild!", Arcemu::Util::GUID_LOPART(m_guid));
        return false;
    }

    if (m_class < 1 || m_class >= 12)
    {
        Log.Error("Guild", "Player (GUID: %u) has a broken data in field `characters`.`class`, deleting him from guild!", Arcemu::Util::GUID_LOPART(m_guid));
        return false;
    }
    return true;
}


void Guild::GuildMember::UpdateBankWithdrawValue(uint8 tabId, uint32 amount)
{
    m_bankWithdraw[tabId] += amount;

    CharacterDatabase.Execute("REPLACE INTO guild_member_withdraw VALUES('%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u' ,'%u', '%u' )",
        Arcemu::Util::GUID_LOPART(m_guid),
        m_bankWithdraw[0], m_bankWithdraw[1], m_bankWithdraw[2], m_bankWithdraw[3], m_bankWithdraw[4],
        m_bankWithdraw[5], m_bankWithdraw[7], m_bankWithdraw[7], 0);
}

void Guild::GuildMember::ResetValues(bool weekly /* = false*/)
{
    for (uint8 tabId = 0; tabId <= GUILD_BANK_MAX_TABS; ++tabId)
        m_bankWithdraw[tabId] = 0;

    if (weekly)
    {
        m_weekActivity = 0;
        m_weekReputation = 0;
    }
}

// Get amount of money/slots left for today.
// If (tabId == GUILD_BANK_MAX_TABS) return money amount.
// Otherwise return remaining items amount for specified tab.
int32 Guild::GuildMember::GetBankWithdrawValue(uint8 tabId) const
{
    // Guild master has unlimited amount.
    if (IsRank(GR_GUILDMASTER))
        return tabId == GUILD_BANK_MAX_TABS ? GUILD_WITHDRAW_MONEY_UNLIMITED : GUILD_WITHDRAW_SLOT_UNLIMITED;

    return m_bankWithdraw[tabId];
}

// EmblemInfo
void EmblemInfo::ReadPacket(WorldPacket& recv_data)
{
    recv_data >> m_style;
    recv_data >> m_color;
    recv_data >> m_borderStyle;
    recv_data >> m_borderColor;
    recv_data >> m_backgroundColor;
}

void EmblemInfo::LoadFromDB(Field* fields)
{
    m_style = fields[3].GetUInt8();
    m_color = fields[4].GetUInt8();
    m_borderStyle = fields[5].GetUInt8();
    m_borderColor = fields[6].GetUInt8();
    m_backgroundColor = fields[7].GetUInt8();
}

void EmblemInfo::WritePacket(WorldPacket& data) const
{
    data << uint32(m_style);
    data << uint32(m_color);
    data << uint32(m_borderStyle);
    data << uint32(m_borderColor);
    data << uint32(m_backgroundColor);
}

void EmblemInfo::SaveToDB(uint32 guildId) const
{
    CharacterDatabase.Execute("UPDATE guild SET emblemStyle = '%u', emblemColor = '%u', borderStyle = '%u', borderColor = '%u', backgroundColor = '%u' WHERE guildId=%u",
        m_style, m_color, m_borderStyle, m_borderColor, m_backgroundColor, guildId);
}

// Guild
Guild::Guild() :
    m_id(0),
    m_leaderGuid(0),
    m_createdDate(0),
    m_accountsNumber(0),
    m_bankMoney(0),
    m_eventLog(nullptr),
    m_newsLog(nullptr),
    //m_achievementMgr(this),
    _level(1),
    _experience(0),
    _todayExperience(0)
{
    memset(&m_bankEventLog, 0, (GUILD_BANK_MAX_TABS + 1) * sizeof(GuildLogHolder*));
}

Guild::~Guild()
{
    // Cleanup
    delete m_eventLog;
    m_eventLog = nullptr;
    delete m_newsLog;
    m_newsLog = nullptr;

    for (uint8 tabId = 0; tabId <= GUILD_BANK_MAX_TABS; ++tabId)
    {
        delete m_bankEventLog[tabId];
        m_bankEventLog[tabId] = nullptr;
    }

    for (Members::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        delete itr->second;
        itr->second = nullptr;
    }
}

// Creates new guild with default data and saves it to database.
bool Guild::Create(Player* pLeader, std::string const& name)
{
    // Check if guild with such name already exists
    if (sGuildMgr.GetGuildByName(name))
        return false;

    WorldSession* pLeaderSession = pLeader->GetSession();
    if (!pLeaderSession)
        return false;

    m_id = sGuildMgr.GenerateGuildId();
    m_leaderGuid = pLeader->GetGUID();
    m_name = name;
    m_info = "No message set.";
    m_motd = "No message set.";
    m_bankMoney = 0;
    m_createdDate = ::time(NULL);
    _level = 1;
    _experience = 0;
    _todayExperience = 0;
    _CreateLogHolders();

    Log.Debug("Guild", "GUILD: creating guild [%s] for leader %s (%u)",
        name.c_str(), pLeader->GetName(), Arcemu::Util::GUID_LOPART(m_leaderGuid));

    CharacterDatabase.Execute("DELETE FROM guild_member WHERE guildId = %u", m_id);

    

    CharacterDatabase.Execute("INSERT INTO guild (guildId, guildName, leaderGuid, emblemStyle, emblemColor, borderStyle, borderColor, backgroundColor,"
                              "guildInfo, motd, createdate, bankBalance, guildLevel, guildExperience, todayExperience) "
                              "VALUES('%u', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%s', '%s', '%u', '%u', '%u', '0', '0')",
                              m_id, name.c_str(), m_leaderGuid, m_emblemInfo.GetStyle(), m_emblemInfo.GetColor(), m_emblemInfo.GetBorderStyle(),
                              m_emblemInfo.GetBorderColor(),m_emblemInfo.GetBackgroundColor(), m_info.c_str(), m_motd.c_str(), uint32(m_createdDate),
                              m_bankMoney, _level);
    
    _CreateDefaultGuildRanks(/*LOCALE_enUS*/); // Create default ranks
    bool ret = AddMember(m_leaderGuid, GR_GUILDMASTER);                  // Add guildmaster

    if (ret)
    {
        _BroadcastEvent(GE_FOUNDER, 0);
        sHookInterface.OnGuildCreate(pLeader, this);
    }

    SaveToDB();

    return ret;
}

// Disbands guild and deletes all related data from database
void Guild::Disband()
{
    _BroadcastEvent(GE_DISBANDED, 0);
    // Remove all members
    while (!m_members.empty())
    {
        Members::const_iterator itr = m_members.begin();
        DeleteMember(itr->second->GetGUID(), true);
    }

    CharacterDatabase.Execute("DELETE FROM guild WHERE guildId = %u", m_id);

    CharacterDatabase.Execute("DELETE FROM guild_rank WHERE guildId = %u", m_id);

    CharacterDatabase.Execute("DELETE FROM guild_bank_tab WHERE guildId = %u", m_id);

    // Free bank tab used memory and delete items stored in them
    _DeleteBankItems(true);

    CharacterDatabase.Execute("DELETE FROM guild_bank_item WHERE guildId = %u", m_id);

    CharacterDatabase.Execute("DELETE FROM guild_bank_right WHERE guildId = %u", m_id);

    CharacterDatabase.Execute("DELETE FROM guild_eventlog WHERE guildId = %u", m_id);

    CharacterDatabase.Execute("DELETE FROM guild_bank_eventlog WHERE guildId = %u", m_id);

    sGuildFinderMgr.DeleteGuild(m_id);

    sGuildMgr.RemoveGuild(m_id);
}

void Guild::SaveToDB()
{
    CharacterDatabase.Execute("UPDATE guild SET guildLevel = '%u', guildExperience = '%llu', todayExperience = '%llu' WHERE guildId = %u",
        (uint32)GetLevel(), GetExperience(), GetTodayExperience(), GetId());
}

void Guild::UpdateMemberData(Player* player, uint8 dataid, uint32 value)
{
    if (GuildMember* member = GetMember(player->GetGUID()))
    {
        switch (dataid)
        {
        case GUILD_MEMBER_DATA_ZONEID:
            member->SetZoneId(value);
            break;
        case GUILD_MEMBER_DATA_ACHIEVEMENT_POINTS:
            member->SetAchievementPoints(value);
            break;
        case GUILD_MEMBER_DATA_LEVEL:
            member->SetLevel(value);
            break;
        default:
            Log.Error("Guild", "Guild::UpdateMemberData: Called with incorrect DATAID %u (value %u)", (uint32)dataid, value);
            return;
        }
        //HandleRoster();
    }
}

void Guild::OnPlayerStatusChange(Player* player, uint32 flag, bool state)
{
    if (GuildMember* member = GetMember(player->GetGUID()))
    {
        if (state)
            member->AddFlag(flag);
        else member->RemFlag(flag);
    }
}

void Guild::HandleRoster(WorldSession* session /*= nullptr*/)
{
    ByteBuffer memberData(100);
    // Guess size
    WorldPacket data(SMSG_GUILD_ROSTER, 100);
    data.writeBits(m_motd.length(), 11);
    data.writeBits(m_members.size(), 18);

    for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        GuildMember* member = itr->second;
        size_t pubNoteLength = member->GetPublicNote().length();
        size_t offNoteLength = member->GetOfficerNote().length();

        ObjectGuid guid = member->GetGUID();
        data.writeBit(guid[3]);
        data.writeBit(guid[4]);
        data.writeBit(0); // Has Authenticator
        data.writeBit(0); // Can Scroll of Ressurect
        data.writeBits(pubNoteLength, 8);
        data.writeBits(offNoteLength, 8);
        data.writeBit(guid[0]);
        data.writeBits(member->GetName().length(), 7);
        data.writeBit(guid[1]);
        data.writeBit(guid[2]);
        data.writeBit(guid[6]);
        data.writeBit(guid[5]);
        data.writeBit(guid[7]);

        memberData << uint8(member->GetClass());
        memberData << uint32(member->GetTotalReputation());
        memberData.WriteByteSeq(guid[0]);
        memberData << uint64(member->GetWeekActivity());
        memberData << uint32(member->GetRankId());
        memberData << uint32(member->GetAchievementPoints());

        // for (2 professions)
        memberData << uint32(0) << uint32(0) << uint32(0);
        memberData << uint32(0) << uint32(0) << uint32(0);

        memberData.WriteByteSeq(guid[2]);
        memberData << uint8(member->GetFlags());
        memberData << uint32(member->GetZoneId());
        memberData << uint64(member->GetTotalActivity());
        memberData.WriteByteSeq(guid[7]);
        memberData << uint32(sWorld.m_guild.MaxRepPerWeek - member->GetWeekReputation());

        if (pubNoteLength)
            memberData.WriteString(member->GetPublicNote());

        memberData.WriteByteSeq(guid[3]);
        memberData << uint8(member->GetLevel());
        memberData << int32(0);                                     // unk
        memberData.WriteByteSeq(guid[5]);
        memberData.WriteByteSeq(guid[4]);
        memberData << uint8(0);                                     // unk
        memberData.WriteByteSeq(guid[1]);
        memberData << float(member->IsOnline() ? 0.0f : float(::time(NULL) - member->GetLogoutTime()) / DAY);

        if (offNoteLength)
            memberData.WriteString(member->GetOfficerNote());

        memberData.WriteByteSeq(guid[6]);
        memberData.WriteString(member->GetName());
    }

    size_t infoLength = m_info.length();
    data.writeBits(infoLength, 12);

    data.flushBits();
    data.append(memberData);

    if (infoLength)
        data.WriteString(m_info);

    data.WriteString(m_motd);
    data << uint32(m_accountsNumber);
    data << uint32(sWorld.m_guild.MaxRepPerWeek);
    data.AppendPackedTime(m_createdDate);
    data << uint32(0);

    if (session)
    {
        Log.Debug("Guild", "SMSG_GUILD_ROSTER [%s]", session->GetPlayer()->GetName());
        session->SendPacket(&data);
    }
    else
    {
        Log.Debug("Guild", "SMSG_GUILD_ROSTER [Broadcast]");
        BroadcastPacket(&data);
    }
}

void Guild::HandleQuery(WorldSession* session)
{
    WorldPacket data(SMSG_GUILD_QUERY_RESPONSE, 8 * 32 + 200);      // Guess size

    data << uint64(GetGUID());
    data << m_name;

    // Rank name
    for (uint8 i = 0; i < GUILD_RANKS_MAX_COUNT; ++i)               // Always show 10 ranks
    {
        if (i < _GetRanksSize())
            data << m_ranks[i].GetName();
        else
            data << uint8(0);                                       // Empty string
    }

    // Rank order of creation
    for (uint8 i = 0; i < GUILD_RANKS_MAX_COUNT; ++i)
    {
        if (i < _GetRanksSize())
            data << uint32(i);
        else
            data << uint32(0);
    }

    // Rank order of "importance" (sorting by rights)
    for (uint8 i = 0; i < GUILD_RANKS_MAX_COUNT; ++i)
    {
        if (i < _GetRanksSize())
            data << uint32(m_ranks[i].GetId());
        else
            data << uint32(0);
    }

    m_emblemInfo.WritePacket(data);
    data << uint32(_GetRanksSize());                                // Number of ranks used

    session->SendPacket(&data);
    Log.Debug("Guild", "SMSG_GUILD_QUERY_RESPONSE [%s]", session->GetPlayer()->GetName());
}

void Guild::SendGuildRankInfo(WorldSession* session) const
{
    ByteBuffer rankData(100);
    WorldPacket data(SMSG_GUILD_RANK, 100);

    data.writeBits(_GetRanksSize(), 18);

    for (uint8 i = 0; i < _GetRanksSize(); ++i)
    {
        GuildRankInfo const* rankInfo = GetRankInfo(i);
        if (!rankInfo)
            continue;

        data.writeBits(rankInfo->GetName().length(), 7);

        rankData << uint32(rankInfo->GetId());

        for (uint8 j = 0; j < GUILD_BANK_MAX_TABS; ++j)
        {
            rankData << uint32(rankInfo->GetBankTabSlotsPerDay(j));
            rankData << uint32(rankInfo->GetBankTabRights(j));
        }

        rankData << uint32(rankInfo->GetBankMoneyPerDay());
        rankData << uint32(rankInfo->GetRights());

        if (rankInfo->GetName().length())
            rankData.WriteString(rankInfo->GetName());

        rankData << uint32(i);
    }

    data.flushBits();
    data.append(rankData);
    session->SendPacket(&data);
    Log.Debug("Guild", "SMSG_GUILD_RANK [%s]", session->GetPlayer()->GetName());
}

void Guild::HandleSetMOTD(WorldSession* session, std::string const& motd)
{
    if (m_motd == motd)
        return;

    // Player must have rights to set MOTD
    if (!_HasRankRight(session->GetPlayer()->GetGUID(), GR_RIGHT_SETMOTD))
        SendCommandResult(session, GUILD_COMMAND_EDIT_MOTD, ERR_GUILD_PERMISSIONS);
    else
    {
        m_motd = motd;

        CharacterDatabase.Execute("UPDATE guild SET motd = '%s' WHERE guildId = %u", motd.c_str(), m_id);

        _BroadcastEvent(GE_MOTD, 0, motd.c_str());
    }
}

void Guild::HandleSetInfo(WorldSession* session, std::string const& info)
{
    if (m_info == info)
        return;

    // Player must have rights to set guild's info
    if (_HasRankRight(session->GetPlayer()->GetGUID(), GR_RIGHT_MODIFY_GUILD_INFO))
    {
        m_info = info;

        CharacterDatabase.Execute("UPDATE guild SET guildInfo = '%s' WHERE guildId = %u", info.c_str(), m_id);
    }
}

void Guild::HandleSetEmblem(WorldSession* session, const EmblemInfo& emblemInfo)
{
    Player* player = session->GetPlayer();
    if (!_IsLeader(player))
        SendSaveEmblemResult(session, ERR_GUILDEMBLEM_NOTGUILDMASTER); // "Only guild leaders can create emblems."
    else if (!player->HasGold(uint64(EMBLEM_PRICE)))
        SendSaveEmblemResult(session, ERR_GUILDEMBLEM_NOTENOUGHMONEY); // "You can't afford to do that."
    else
    {
        player->ModGold(-int64(EMBLEM_PRICE));

        m_emblemInfo = emblemInfo;
        m_emblemInfo.SaveToDB(m_id);

        SendSaveEmblemResult(session, ERR_GUILDEMBLEM_SUCCESS); // "Guild Emblem saved."

        HandleQuery(session);
    }
}

void Guild::HandleSetNewGuildMaster(WorldSession* session, std::string const& name)
{
    Player* player = session->GetPlayer();
    // Only the guild master can throne a new guild master
    if (!_IsLeader(player))
        SendCommandResult(session, GUILD_COMMAND_CHANGE_LEADER, ERR_GUILD_PERMISSIONS);
    // Old GM must be a guild member
    else if (GuildMember* oldGuildMaster = GetMember(player->GetGUID()))
    {
        // Same for the new one
        if (GuildMember* newGuildMaster = GetMember(name))
        {
            _SetLeaderGUID(newGuildMaster);
            oldGuildMaster->ChangeRank(GR_INITIATE);
            _BroadcastEvent(GE_LEADER_CHANGED, 0, session->GetPlayer()->GetName(), name.c_str());
        }
    }
}

void Guild::HandleSetBankTabInfo(WorldSession* session, uint8 tabId, std::string const& name, std::string const& icon)
{
    GuildBankTab* tab = GetBankTab(tabId);
    if (!tab)
    {
        Log.Error("Guild", "Guild::HandleSetBankTabInfo: Player %s trying to change bank tab info from unexisting tab %d.",
            session->GetPlayer()->GetName(), tabId);
        return;
    }

    char aux[2];
    sprintf(aux, "%u", tabId);

    tab->SetInfo(name, icon);
    _BroadcastEvent(GE_BANK_TAB_UPDATED, 0, aux, name.c_str(), icon.c_str());
}

void Guild::HandleSetMemberNote(WorldSession* session, std::string const& note, uint64 guid, bool isPublic)
{
    // Player must have rights to set public/officer note
    if (!_HasRankRight(session->GetPlayer()->GetGUID(), isPublic ? GR_RIGHT_EPNOTE : GR_RIGHT_EOFFNOTE))
        SendCommandResult(session, GUILD_COMMAND_PUBLIC_NOTE, ERR_GUILD_PERMISSIONS);
    else if (GuildMember* member = GetMember(guid))
    {
        if (isPublic)
            member->SetPublicNote(note);
        else
            member->SetOfficerNote(note);

        HandleRoster(session); // FIXME - We should send SMSG_GUILD_MEMBER_UPDATE_NOTE
    }
}

void Guild::HandleSetRankInfo(WorldSession* session, uint8 rankId, std::string const& name, uint32 rights, uint32 moneyPerDay, GuildBankRightsAndSlotsVec rightsAndSlots)
{
    // Only leader can modify ranks
    if (!_IsLeader(session->GetPlayer()))
        SendCommandResult(session, GUILD_COMMAND_CHANGE_RANK, ERR_GUILD_PERMISSIONS);
    else if (GuildRankInfo* rankInfo = GetRankInfo(rankId))
    {
        Log.Debug("Guild", "Changed RankName to '%s', rights to 0x%08X", name.c_str(), rights);

        rankInfo->SetName(name);
        rankInfo->SetRights(rights);
        _SetRankBankMoneyPerDay(rankId, moneyPerDay);

        for (GuildBankRightsAndSlotsVec::const_iterator itr = rightsAndSlots.begin(); itr != rightsAndSlots.end(); ++itr)
            _SetRankBankTabRightsAndSlots(rankId, *itr);

        char aux[2];
        sprintf(aux, "%u", rankId);
        _BroadcastEvent(GE_RANK_UPDATED, 0, aux);
    }
}

void Guild::HandleBuyBankTab(WorldSession* session, uint8 tabId)
{
    Player* player = session->GetPlayer();
    if (!player)
        return;

    GuildMember const* member = GetMember(player->GetGUID());
    if (!member)
        return;

    if (_GetPurchasedTabsSize() >= GUILD_BANK_MAX_TABS)
        return;

    if (tabId != _GetPurchasedTabsSize())
        return;

    uint32 tabCost = _GetGuildBankTabPrice(tabId) * GOLD;
    if (!tabCost)
        return;

    if (!player->HasGold(uint64(tabCost)))                   // Should not happen, this is checked by client
        return;

    player->ModGold(-int64(tabCost));

    _CreateNewBankTab();
    _BroadcastEvent(GE_BANK_TAB_PURCHASED, 0);
    SendPermissions(session); /// Hack to force client to update permissions
}

void Guild::HandleInviteMember(WorldSession* session, std::string const& name)
{
    PlayerInfo* info = objmgr.GetPlayerInfoByName(name.c_str());
    if (info == nullptr)
        return;

    Player* pInvitee = objmgr.GetPlayer(info->guid);
    if (!pInvitee)
    {
        SendCommandResult(session, GUILD_COMMAND_INVITE, ERR_GUILD_PLAYER_NOT_FOUND_S, name);
        return;
    }

    Player* player = session->GetPlayer();
    // Do not show invitations from ignored players
    if (pInvitee->Social_IsIgnoring(player->GetLowGUID()))
        return;

    if (!sWorld.interfaction_guild && pInvitee->GetTeam() != player->GetTeam())
    {
        SendCommandResult(session, GUILD_COMMAND_INVITE, ERR_GUILD_NOT_ALLIED, name);
        return;
    }

    // Invited player cannot be in another guild
    if (pInvitee->GetGuildId())
    {
        SendCommandResult(session, GUILD_COMMAND_INVITE, ERR_ALREADY_IN_GUILD_S, name);
        return;
    }

    // Invited player cannot be invited
    if (pInvitee->GetGuildIdInvited())
    {
        SendCommandResult(session, GUILD_COMMAND_INVITE, ERR_ALREADY_INVITED_TO_GUILD_S, name);
        return;
    }
    // Inviting player must have rights to invite
    if (!_HasRankRight(player->GetGUID(), GR_RIGHT_INVITE))
    {
        SendCommandResult(session, GUILD_COMMAND_INVITE, ERR_GUILD_PERMISSIONS);
        return;
    }

    SendCommandResult(session, GUILD_COMMAND_INVITE, ERR_GUILD_COMMAND_SUCCESS, name);

    Log.Debug("Guild", "Player %s invited %s to join his Guild", player->GetName(), name.c_str());

    pInvitee->SetGuildIdInvited(m_id);
    _LogEvent(GUILD_EVENT_LOG_INVITE_PLAYER, player->GetLowGUID(), pInvitee->GetLowGUID());

    WorldPacket data(SMSG_GUILD_INVITE, 100);
    data << uint32(GetLevel());
    data << uint32(m_emblemInfo.GetBorderStyle());
    data << uint32(m_emblemInfo.GetBorderColor());
    data << uint32(m_emblemInfo.GetStyle());
    data << uint32(m_emblemInfo.GetBackgroundColor());
    data << uint32(m_emblemInfo.GetColor());

    ObjectGuid oldGuildGuid = MAKE_NEW_GUID(pInvitee->GetGuildId(), 0, pInvitee->GetGuildId() ? uint32(HIGHGUID_TYPE_GUILD) : 0);
    ObjectGuid newGuildGuid = GetGUID();

    data.writeBit(newGuildGuid[3]);
    data.writeBit(newGuildGuid[2]);
    data.writeBits(pInvitee->GetGuildName().length(), 8);
    data.writeBit(newGuildGuid[1]);
    data.writeBit(oldGuildGuid[6]);
    data.writeBit(oldGuildGuid[4]);
    data.writeBit(oldGuildGuid[1]);
    data.writeBit(oldGuildGuid[5]);
    data.writeBit(oldGuildGuid[7]);
    data.writeBit(oldGuildGuid[2]);
    data.writeBit(newGuildGuid[7]);
    data.writeBit(newGuildGuid[0]);
    data.writeBit(newGuildGuid[6]);
    data.writeBits(m_name.length(), 8);
    data.writeBit(oldGuildGuid[3]);
    data.writeBit(oldGuildGuid[0]);
    data.writeBit(newGuildGuid[5]);
    data.writeBits(player->GetNameString()->size(), 7);
    data.writeBit(newGuildGuid[4]);

    data.flushBits();

    data.WriteByteSeq(newGuildGuid[1]);
    data.WriteByteSeq(oldGuildGuid[3]);
    data.WriteByteSeq(newGuildGuid[6]);
    data.WriteByteSeq(oldGuildGuid[2]);
    data.WriteByteSeq(oldGuildGuid[1]);
    data.WriteByteSeq(newGuildGuid[0]);

    if (!pInvitee->GetGuildName().empty())
        data.WriteString(pInvitee->GetGuildName());

    data.WriteByteSeq(newGuildGuid[7]);
    data.WriteByteSeq(newGuildGuid[2]);

    data.WriteString(player->GetName());

    data.WriteByteSeq(oldGuildGuid[7]);
    data.WriteByteSeq(oldGuildGuid[6]);
    data.WriteByteSeq(oldGuildGuid[5]);
    data.WriteByteSeq(oldGuildGuid[0]);
    data.WriteByteSeq(newGuildGuid[4]);

    data.WriteString(m_name);

    data.WriteByteSeq(newGuildGuid[5]);
    data.WriteByteSeq(newGuildGuid[3]);
    data.WriteByteSeq(oldGuildGuid[4]);
    pInvitee->GetSession()->SendPacket(&data);
    Log.Debug("Guild", "SMSG_GUILD_INVITE [%s]", pInvitee->GetNameString());
}

void Guild::HandleAcceptMember(WorldSession* session)
{
    Player* player = session->GetPlayer();
    Player* leader = objmgr.GetPlayer(Arcemu::Util::GUID_LOPART(GetLeaderGUID()));
    if (!sWorld.interfaction_guild && player->GetTeam() != leader->GetTeam())
        return;

    AddMember(player->GetGUID());
}

void Guild::HandleLeaveMember(WorldSession* session)
{
    Player* player = session->GetPlayer();
    // If leader is leaving
    if (_IsLeader(player))
    {
        if (m_members.size() > 1)
            // Leader cannot leave if he is not the last member
            SendCommandResult(session, GUILD_COMMAND_QUIT, ERR_GUILD_LEADER_LEAVE);
        else if (GetLevel() >= sWorld.m_guild.UndeletabelLevel)
            SendCommandResult(session, GUILD_COMMAND_QUIT, ERR_GUILD_UNDELETABLE_DUE_TO_LEVEL);
        else
            Disband(); // Guild is disbanded if leader leaves.
    }
    else
    {
        DeleteMember(player->GetGUID(), false, false);

        _LogEvent(GUILD_EVENT_LOG_LEAVE_GUILD, player->GetLowGUID());
        _BroadcastEvent(GE_LEFT, player->GetGUID(), player->GetName());

        SendCommandResult(session, GUILD_COMMAND_QUIT, ERR_GUILD_COMMAND_SUCCESS, m_name);
    }
}

void Guild::HandleRemoveMember(WorldSession* session, uint64 guid)
{
    Player* player = session->GetPlayer();

    // Player must have rights to remove members
    if (!_HasRankRight(player->GetGUID(), GR_RIGHT_REMOVE))
        SendCommandResult(session, GUILD_COMMAND_REMOVE, ERR_GUILD_PERMISSIONS);
    else if (GuildMember* member = GetMember(guid))
    {
        std::string name = member->GetName();

        // Guild masters cannot be removed
        if (member->IsRank(GR_GUILDMASTER))
            SendCommandResult(session, GUILD_COMMAND_REMOVE, ERR_GUILD_LEADER_LEAVE);
        // Do not allow to remove player with the same rank or higher
        else
        {
            GuildMember const* memberMe = GetMember(player->GetGUID());
            if (!memberMe || member->IsRankNotLower(memberMe->GetRankId()))
                SendCommandResult(session, GUILD_COMMAND_REMOVE, ERR_GUILD_RANK_TOO_HIGH_S, name);
            else
            {
                // After call to DeleteMember pointer to member becomes invalid
                DeleteMember(guid, false, true);
                _LogEvent(GUILD_EVENT_LOG_UNINVITE_PLAYER, player->GetLowGUID(), Arcemu::Util::GUID_LOPART(guid));
                _BroadcastEvent(GE_REMOVED, 0, name.c_str(), player->GetName());
                SendCommandResult(session, GUILD_COMMAND_REMOVE, ERR_GUILD_COMMAND_SUCCESS, name);
            }
        }
    }
}

void Guild::HandleUpdateMemberRank(WorldSession* session, uint64 guid, bool demote)
{
    Player* player = session->GetPlayer();
    GuildCommandType type = demote ? GUILD_COMMAND_DEMOTE : GUILD_COMMAND_PROMOTE;
    // Player must have rights to promote
    if (!_HasRankRight(player->GetGUID(), demote ? GR_RIGHT_DEMOTE : GR_RIGHT_PROMOTE))
        SendCommandResult(session, type, ERR_GUILD_PERMISSIONS);
    // Promoted player must be a member of guild
    else if (GuildMember* member = GetMember(guid))
    {
        std::string name = member->GetName();
        // Player cannot promote himself
        if (member->IsSamePlayer(player->GetGUID()))
        {
            SendCommandResult(session, type, ERR_GUILD_NAME_INVALID);
            return;
        }

        GuildMember const* memberMe = GetMember(player->GetGUID());
        uint8 rankId = memberMe->GetRankId();
        if (demote)
        {
            // Player can demote only lower rank members
            if (member->IsRankNotLower(rankId))
            {
                SendCommandResult(session, type, ERR_GUILD_RANK_TOO_HIGH_S, name);
                return;
            }
            // Lowest rank cannot be demoted
            if (member->GetRankId() >= _GetLowestRankId())
            {
                SendCommandResult(session, type, ERR_GUILD_RANK_TOO_LOW_S, name);
                return;
            }
        }
        else
        {
            // Allow to promote only to lower rank than member's rank
            // member->GetRankId() + 1 is the highest rank that current player can promote to
            if (member->IsRankNotLower(rankId + 1))
            {
                SendCommandResult(session, type, ERR_GUILD_RANK_TOO_HIGH_S, name);
                return;
            }
        }

        uint32 newRankId = member->GetRankId() + (demote ? 1 : -1);
        member->ChangeRank(newRankId);
        _LogEvent(demote ? GUILD_EVENT_LOG_DEMOTE_PLAYER : GUILD_EVENT_LOG_PROMOTE_PLAYER, player->GetLowGUID(), Arcemu::Util::GUID_LOPART(member->GetGUID()), newRankId);
        _BroadcastEvent(demote ? GE_DEMOTION : GE_PROMOTION, 0, player->GetName(), name.c_str(), _GetRankName(newRankId).c_str());
    }
}

void Guild::HandleSetMemberRank(WorldSession* session, uint64 targetGuid, uint64 setterGuid, uint32 rank)
{
    Player* player = session->GetPlayer();
    GuildMember* member = GetMember(targetGuid);
    GuildRankRights rights = GR_RIGHT_PROMOTE;
    GuildCommandType type = GUILD_COMMAND_PROMOTE;

    if (rank > member->GetRankId())
    {
        rights = GR_RIGHT_DEMOTE;
        type = GUILD_COMMAND_DEMOTE;
    }

    // Promoted player must be a member of guild
    if (!_HasRankRight(player->GetGUID(), rights))
    {
        SendCommandResult(session, type, ERR_GUILD_PERMISSIONS);
        return;
    }

    // Player cannot promote himself
    if (member->IsSamePlayer(player->GetGUID()))
    {
        SendCommandResult(session, type, ERR_GUILD_NAME_INVALID);
        return;
    }

    SendGuildRanksUpdate(setterGuid, targetGuid, rank);
}

void Guild::HandleAddNewRank(WorldSession* session, std::string const& name)
{
    uint8 size = _GetRanksSize();
    if (size >= GUILD_RANKS_MAX_COUNT)
        return;

    // Only leader can add new rank
    if (_IsLeader(session->GetPlayer()))
        if (_CreateRank(name, GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK))
            _BroadcastEvent(GE_RANK_CREATED, 0);
}

void Guild::HandleRemoveRank(WorldSession* session, uint8 rankId)
{
    // Cannot remove rank if total count is minimum allowed by the client or is not leader
    if (_GetRanksSize() <= GUILD_RANKS_MIN_COUNT || rankId >= _GetRanksSize() || !_IsLeader(session->GetPlayer()))
        return;

    // Delete bank rights for rank
    CharacterDatabase.Execute("DELETE FROM guild_bank_right WHERE rid = %u AND guildId = %u", rankId, m_id);

    // Delete rank
    CharacterDatabase.Execute("DELETE FROM guild_rank WHERE rid = %u AND guildId = %u", rankId, m_id);

    m_ranks.erase(m_ranks.begin() + rankId);

    _BroadcastEvent(GE_RANK_DELETED, rankId);
}

void Guild::HandleMemberDepositMoney(WorldSession* session, uint64 amount, bool cashFlow /*=false*/)
{
    Player* player = session->GetPlayer();

    _ModifyBankMoney(amount, true);
    if (!cashFlow)
    {
        player->ModGold(-int64(amount));
    }

    _LogBankEvent(cashFlow ? GUILD_BANK_LOG_CASH_FLOW_DEPOSIT : GUILD_BANK_LOG_DEPOSIT_MONEY, uint8(0), player->GetLowGUID(), amount);

    std::string aux = ByteArrayToHexStr(reinterpret_cast<uint8*>(&amount), 8, true);
    _BroadcastEvent(GE_BANK_MONEY_CHANGED, 0, aux.c_str());
}

bool Guild::HandleMemberWithdrawMoney(WorldSession* session, uint64 amount, bool repair)
{
    if (m_bankMoney < amount)                               // Not enough money in bank
        return false;

    Player* player = session->GetPlayer();

    GuildMember* member = GetMember(player->GetGUID());
    if (!member)
        return false;

    if (uint64(_GetMemberRemainingMoney(member)) < amount)   // Check if we have enough slot/money today
        return false;


    // Add money to player (if required)
    if (!repair)
    {
        player->ModGold(int32(amount));
        //if (!player->ModGold(int32(amount))) todo
            //return false;
    }

    // Update remaining money amount
    member->UpdateBankWithdrawValue(GUILD_BANK_MAX_TABS, amount);
    // Remove money from bank
    _ModifyBankMoney(amount, false);

    // Log guild bank event
    _LogBankEvent(repair ? GUILD_BANK_LOG_REPAIR_MONEY : GUILD_BANK_LOG_WITHDRAW_MONEY, uint8(0), player->GetLowGUID(), amount);

    std::string aux = ByteArrayToHexStr(reinterpret_cast<uint8*>(&amount), 8, true);
    _BroadcastEvent(GE_BANK_MONEY_CHANGED, 0, aux.c_str());
    return true;
}

void Guild::HandleMemberLogout(WorldSession* session)
{
    Player* player = session->GetPlayer();
    if (GuildMember* member = GetMember(player->GetGUID()))
    {
        member->SetStats(player);
        member->UpdateLogoutTime();
        member->ResetFlags();
    }
    _BroadcastEvent(GE_SIGNED_OFF, player->GetGUID(), player->GetName());

    SaveToDB();
}

void Guild::HandleDisband(WorldSession* session)
{
    // Only leader can disband guild
    if (_IsLeader(session->GetPlayer()))
    {
        Disband();
        Log.Debug("Guild", "Guild Successfully Disbanded");
    }
}

void Guild::HandleGuildPartyRequest(WorldSession* session)
{
    Player* player = session->GetPlayer();
    Group* group = player->GetGroup();

    // Make sure player is a member of the guild and that he is in a group.
    if (!IsMember(player->GetGUID()) || !group)
        return;
    // TODO
    Log.Debug("Guild", "SMSG_GUILD_PARTY_STATE_RESPONSE [%s]", session->GetPlayer()->GetName());
}

void Guild::SendEventLog(WorldSession* session) const
{
    WorldPacket data(SMSG_GUILD_EVENT_LOG_QUERY_RESULT, 1 + m_eventLog->GetSize() * (1 + 8 + 4));
    m_eventLog->WritePacket(data);
    session->SendPacket(&data);
    Log.Debug("Guild", "SMSG_GUILD_EVENT_LOG_QUERY_RESULT [%s]", session->GetPlayer()->GetName());
}

void Guild::SendNewsUpdate(WorldSession* session)
{
    uint32 size = m_newsLog->GetSize();
    GuildLog* logs = m_newsLog->GetGuildLog();

    if (!logs)
        return;

    WorldPacket data(SMSG_GUILD_NEWS_UPDATE, (21 + size * (26 + 8)) / 8 + (8 + 6 * 4) * size);
    data.writeBits(size, 21);

    for (GuildLog::const_iterator itr = logs->begin(); itr != logs->end(); ++itr)
    {
        data.writeBits(0, 26); // Not yet implemented used for guild achievements
        ObjectGuid guid = ((GuildNewsLogEntry*)(*itr))->GetPlayerGuid();

        data.writeBit(guid[7]);
        data.writeBit(guid[0]);
        data.writeBit(guid[6]);
        data.writeBit(guid[5]);
        data.writeBit(guid[4]);
        data.writeBit(guid[3]);
        data.writeBit(guid[1]);
        data.writeBit(guid[2]);
    }

    data.flushBits();

    for (GuildLog::const_iterator itr = logs->begin(); itr != logs->end(); ++itr)
    {
        GuildNewsLogEntry* news = (GuildNewsLogEntry*)(*itr);
        ObjectGuid guid = news->GetPlayerGuid();
        data.WriteByteSeq(guid[5]);

        data << uint32(news->GetFlags());   // 1 sticky
        data << uint32(news->GetValue());
        data << uint32(0);

        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[1]);

        data << uint32(news->GetGUID());
        data << uint32(news->GetType());
        data.AppendPackedTime(news->GetTimestamp());
    }

    session->SendPacket(&data);
    Log.Debug("Guild", "SMSG_GUILD_NEWS_UPDATE [%s]", session->GetPlayer()->GetNameString());
}

void Guild::SendBankLog(WorldSession* session, uint8 tabId) const
{
    // GUILD_BANK_MAX_TABS send by client for money log
    if (tabId < _GetPurchasedTabsSize() || tabId == GUILD_BANK_MAX_TABS)
    {
        GuildLogHolder const* log = m_bankEventLog[tabId];
        WorldPacket data(SMSG_GUILD_BANK_LOG_QUERY_RESULT, log->GetSize() * (4 * 4 + 1) + 1 + 1);
        data.writeBit(GetLevel() >= 5 && tabId == GUILD_BANK_MAX_TABS);     // has Cash Flow perk
        log->WritePacket(data);
        data << uint32(tabId);
        session->SendPacket(&data);
        Log.Debug("Guild", "SMSG_GUILD_BANK_LOG_QUERY_RESULT [%s] TabId: %u", session->GetPlayer()->GetName(), tabId);
    }
}

void Guild::SendBankTabText(WorldSession* session, uint8 tabId) const
{
    if (GuildBankTab const* tab = GetBankTab(tabId))
        tab->SendText(this, session);
}

void Guild::SendPermissions(WorldSession* session) const
{
    GuildMember const* member = GetMember(session->GetPlayer()->GetGUID());
    if (!member)
        return;

    uint8 rankId = member->GetRankId();

    WorldPacket data(SMSG_GUILD_PERMISSIONS_QUERY_RESULTS, 4 * 15 + 1);
    data << uint32(rankId);
    data << uint32(_GetPurchasedTabsSize());
    data << uint32(_GetRankRights(rankId));
    data << uint32(_GetMemberRemainingMoney(member));
    data.writeBits(GUILD_BANK_MAX_TABS, 23);
    for (uint8 tabId = 0; tabId < GUILD_BANK_MAX_TABS; ++tabId)
    {
        data << uint32(_GetRankBankTabRights(rankId, tabId));
        data << uint32(_GetMemberRemainingSlots(member, tabId));
    }

    session->SendPacket(&data);
    Log.Debug("Guild", "SMSG_GUILD_PERMISSIONS_QUERY_RESULTS [%s] Rank: %u", session->GetPlayer()->GetName(), rankId);
}

void Guild::SendMoneyInfo(WorldSession* session) const
{
    GuildMember const* member = GetMember(session->GetPlayer()->GetGUID());
    if (!member)
        return;

    int32 amount = _GetMemberRemainingMoney(member);
    WorldPacket data(SMSG_GUILD_BANK_MONEY_WITHDRAWN, 8);
    data << uint64(amount);
    session->SendPacket(&data);
    Log.Debug("Guild", "SMSG_GUILD_BANK_MONEY_WITHDRAWN [%s] Money: %u", session->GetPlayer()->GetName(), amount);
}

void Guild::SendLoginInfo(WorldSession* session)
{
    Player* player = session->GetPlayer();
    GuildMember* member = GetMember(player->GetGUID());
    if (!member)
        return;

    WorldPacket data(SMSG_GUILD_EVENT, 1 + 1 + m_motd.size() + 1);
    data << uint8(GE_MOTD);
    data << uint8(1);
    data << m_motd;
    session->SendPacket(&data);

    Log.Debug("Guild", "SMSG_GUILD_EVENT [%s] MOTD", session->GetPlayer()->GetName());

    SendGuildRankInfo(session);
    _BroadcastEvent(GE_SIGNED_ON, player->GetGUID(), player->GetName());

    // Send to self separately, player is not in world yet and is not found by _BroadcastEvent
    data.Initialize(SMSG_GUILD_EVENT, 1 + 1 + player->GetNameString()->size() + 8);
    data << uint8(GE_SIGNED_ON);
    data << uint8(1);
    data << player->GetName();
    data << uint64(player->GetGUID());
    session->SendPacket(&data);

    data.Initialize(SMSG_GUILD_MEMBER_DAILY_RESET, 0);  // tells the client to request bank withdrawal limit
    session->SendPacket(&data);

    if (!sWorld.m_guild.LevlingEnabled)
        return; 

    for (uint32 i = 0; i < sGuildPerkSpellsStore.GetNumRows(); ++i)
        if (DBC::Structures::GuildPerkSpellsEntry const* entry = sGuildPerkSpellsStore.LookupEntry(i))
           if (entry->Level <= GetLevel())
               player->addSpell(entry->SpellId);

    SendGuildReputationWeeklyCap(session, member->GetWeekReputation());

    //m_achievementMgr.SendAllAchievementData(player);

    member->SetStats(player);
    member->AddFlag(GUILDMEMBER_STATUS_ONLINE);
}

// Loading methods
bool Guild::LoadFromDB(Field* fields)
{
    m_id = fields[0].GetUInt32();
    m_name = fields[1].GetString();
    m_leaderGuid = MAKE_NEW_GUID(fields[2].GetUInt32(), 0, HIGHGUID_TYPE_PLAYER);//Check these
    m_emblemInfo.LoadFromDB(fields);
    m_info = fields[8].GetString();
    m_motd = fields[9].GetString();
    m_createdDate = time_t(fields[10].GetUInt32());
    m_bankMoney = fields[11].GetUInt64();
    _level = fields[12].GetUInt32();
    _experience = fields[13].GetUInt64();
    _todayExperience = fields[14].GetUInt64();

    uint8 purchasedTabs = uint8(fields[15].GetUInt64());
    if (purchasedTabs > GUILD_BANK_MAX_TABS)
        purchasedTabs = GUILD_BANK_MAX_TABS;

    m_bankTabs.resize(purchasedTabs);
    for (uint8 i = 0; i < purchasedTabs; ++i)
        m_bankTabs[i] = new GuildBankTab(m_id, i);

    _CreateLogHolders();
    return true;
}

void Guild::LoadRankFromDB(Field* fields)
{
    GuildRankInfo rankInfo(m_id);

    rankInfo.LoadFromDB(fields);

    m_ranks.push_back(rankInfo);
}

bool Guild::LoadMemberFromDB(Field* fields, Field* fields2)
{
    uint32 lowguid = fields[1].GetUInt32();
    GuildMember* member = new GuildMember(m_id, MAKE_NEW_GUID(lowguid, 0, HIGHGUID_TYPE_PLAYER), fields[2].GetUInt8()); // Check these
    if (!member->LoadFromDB(fields, fields2))
    {
        CharacterDatabase.Execute("DELETE FROM guild_member WHERE guildId = %u", lowguid);
        delete member;
        return false;
    }
    m_members[lowguid] = member;
    return true;
}

void Guild::LoadBankRightFromDB(Field* fields)
{
                                                // tabId              rights                slots
    GuildBankRightsAndSlots rightsAndSlots(fields[1].GetUInt8(), fields[3].GetUInt8(), fields[4].GetUInt32());
                                     // rankId
    _SetRankBankTabRightsAndSlots(fields[2].GetUInt8(), rightsAndSlots, false);
}

bool Guild::LoadEventLogFromDB(Field* fields)
{
    if (m_eventLog->CanInsert())
    {
        m_eventLog->LoadEvent(new GuildEventLogEntry(
            m_id,                                       // guild id
            fields[1].GetUInt32(),                      // guid
            time_t(fields[6].GetUInt32()),              // timestamp
            GuildEventLogTypes(fields[2].GetUInt8()),   // event type
            fields[3].GetUInt32(),                      // player guid 1
            fields[4].GetUInt32(),                      // player guid 2
            fields[5].GetUInt8()));                     // rank
        return true;
    }
    return false;
}

bool Guild::LoadBankEventLogFromDB(Field* fields)
{
    uint8 dbTabId = fields[1].GetUInt8();
    bool isMoneyTab = (dbTabId == GUILD_BANK_MONEY_LOGS_TAB);
    if (dbTabId < _GetPurchasedTabsSize() || isMoneyTab)
    {
        uint8 tabId = isMoneyTab ? uint8(GUILD_BANK_MAX_TABS) : dbTabId;
        GuildLogHolder* pLog = m_bankEventLog[tabId];
        if (pLog->CanInsert())
        {
            uint32 guid = fields[2].GetUInt32();
            GuildBankEventLogTypes eventType = GuildBankEventLogTypes(fields[3].GetUInt8());
            if (GuildBankEventLogEntry::IsMoneyEvent(eventType))
            {
                if (!isMoneyTab)
                {
                    Log.Error("Guild", "GuildBankEventLog ERROR: MoneyEvent(LogGuid: %u, Guild: %u) does not belong to money tab (%u), ignoring...", guid, m_id, dbTabId);
                    return false;
                }
            }
            else if (isMoneyTab)
            {
                Log.Error("Guild", "GuildBankEventLog ERROR: non-money event (LogGuid: %u, Guild: %u) belongs to money tab, ignoring...", guid, m_id);
                return false;
            }
            pLog->LoadEvent(new GuildBankEventLogEntry(
                m_id,                                   // guild id
                guid,                                   // guid
                time_t(fields[8].GetUInt32()),          // timestamp
                dbTabId,                                // tab id
                eventType,                              // event type
                fields[4].GetUInt32(),                  // player guid
                fields[5].GetUInt32(),                  // item or money
                fields[6].GetUInt16(),                  // itam stack count
                fields[7].GetUInt8()));                 // dest tab id
        }
    }
    return true;
}

void Guild::LoadGuildNewsLogFromDB(Field* fields)
{
    if (!m_newsLog->CanInsert())
        return;

    m_newsLog->LoadEvent(new GuildNewsLogEntry(
        m_id,                                       // guild id
        fields[1].GetUInt32(),                      // guid
        fields[6].GetUInt32(),                      // timestamp //64 bits?
        GuildNews(fields[2].GetUInt8()),            // type
        fields[3].GetUInt32(),                      // player guid
        fields[4].GetUInt32(),                      // Flags
        fields[5].GetUInt32()));                    // value
}

void Guild::LoadBankTabFromDB(Field* fields)
{
    uint8 tabId = fields[1].GetUInt8();
    if (tabId >= _GetPurchasedTabsSize())
        Log.Error("Guild", "Invalid tab (tabId: %u) in guild bank, skipped.", tabId);
    else
        m_bankTabs[tabId]->LoadFromDB(fields);
}

bool Guild::LoadBankItemFromDB(Field* fields)
{
    uint8 tabId = fields[1].GetUInt8();
    if (tabId >= _GetPurchasedTabsSize())
    {
        Log.Error("Guild", "Invalid tab for item (GUID: %u) in guild bank, skipped.",
            fields[3].GetUInt32());
        return false;
    }
    return m_bankTabs[tabId]->LoadItemFromDB(fields);
}

// Validates guild data loaded from database. Returns false if guild should be deleted.
bool Guild::Validate()
{
    // Validate ranks data
    // GUILD RANKS represent a sequence starting from 0 = GUILD_MASTER (ALL PRIVILEGES) to max 9 (lowest privileges).
    // The lower rank id is considered higher rank - so promotion does rank-- and demotion does rank++
    // Between ranks in sequence cannot be gaps - so 0, 1, 2, 4 is impossible
    // Min ranks count is 2 and max is 10.
    bool broken_ranks = false;
    uint8 ranks = _GetRanksSize();
    if (ranks < GUILD_RANKS_MIN_COUNT || ranks > GUILD_RANKS_MAX_COUNT)
    {
        Log.Error("Guild", "Guild %u has invalid number of ranks, creating new...", m_id);
        broken_ranks = true;
    }
    else
    {
        for (uint8 rankId = 0; rankId < ranks; ++rankId)
        {
            GuildRankInfo* rankInfo = GetRankInfo(rankId);
            if (rankInfo->GetId() != rankId)
            {
                Log.Error("Guild", "Guild %u has broken rank id %u, creating default set of ranks...", m_id, rankId);
                broken_ranks = true;
            }
            else
            {
                rankInfo->CreateMissingTabsIfNeeded(_GetPurchasedTabsSize(), false, true);
            }
        }
    }

    if (broken_ranks)
    {
        m_ranks.clear();
        _CreateDefaultGuildRanks(/*LOCALE_enUS*/);
    }

    // Validate members' data
    for (Members::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        if (itr->second->GetRankId() > _GetRanksSize())
            itr->second->ChangeRank(_GetLowestRankId());

    // Repair the structure of the guild.
    // If the guildmaster doesn't exist or isn't member of the guild
    // attempt to promote another member.
    GuildMember* pLeader = GetMember(m_leaderGuid);
    if (!pLeader)
    {
        DeleteMember(m_leaderGuid);
        // If no more members left, disband guild
        if (m_members.empty())
        {
            Disband();
            return false;
        }
    }
    else if (!pLeader->IsRank(GR_GUILDMASTER))
        _SetLeaderGUID(pLeader);

    for (Members::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        if (itr->second->GetRankId() == GR_GUILDMASTER && !itr->second->IsSamePlayer(m_leaderGuid))
            itr->second->ChangeRank(GR_OFFICER);

    _UpdateAccountsNumber();
    return true;
}

// Broadcasts
void Guild::BroadcastToGuild(WorldSession* session, bool officerOnly, std::string const& msg, uint32 language) const
{
    if (session && session->GetPlayer() && _HasRankRight(session->GetPlayer()->GetGUID(), officerOnly ? GR_RIGHT_OFFCHATSPEAK : GR_RIGHT_GCHATSPEAK))
    {
        WorldPacket* data = sChatHandler.FillMessageData(officerOnly ? CHAT_MSG_OFFICER : CHAT_MSG_GUILD, language, msg.c_str(), NULL, 0 );

        for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
            if (Player* player = itr->second->FindPlayer(session->GetPlayer()->GetGUID()))
                if (player->GetSession() && _HasRankRight(player->GetGUID(), officerOnly ? GR_RIGHT_OFFCHATLISTEN : GR_RIGHT_GCHATLISTEN) &&
                    !player->Social_IsIgnoring(session->GetPlayer()->GetLowGUID()))
                    player->GetSession()->SendPacket(data);
        //TODO
    }
}

void Guild::BroadcastAddonToGuild(WorldSession* session, bool officerOnly, std::string const& msg, std::string const& prefix) const
{
    if (session && session->GetPlayer() && _HasRankRight(session->GetPlayer()->GetGUID(), officerOnly ? GR_RIGHT_OFFCHATSPEAK : GR_RIGHT_GCHATSPEAK))
    {
        WorldPacket* data = sChatHandler.FillMessageData(officerOnly ? CHAT_MSG_OFFICER : CHAT_MSG_GUILD, uint32(CHAT_MSG_ADDON), msg.c_str(), NULL, 0);

        for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
            if (Player* player = itr->second->FindPlayer(session->GetPlayer()->GetGUID()))
                if (player->GetSession() && _HasRankRight(player->GetGUID(), officerOnly ? GR_RIGHT_OFFCHATLISTEN : GR_RIGHT_GCHATLISTEN) &&
                    !player->Social_IsIgnoring(session->GetPlayer()->GetLowGUID()))
                    player->GetSession()->SendPacket(data);
        //TODO
    }
}

void Guild::BroadcastPacketToRank(WorldPacket* packet, uint8 rankId) const
{
    for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        if (itr->second->IsRank(rankId))
            if (Player* player = itr->second->FindPlayer(itr->second->GetGUID()))
                player->GetSession()->SendPacket(packet);
}

void Guild::BroadcastPacket(WorldPacket* packet) const
{
    for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        if (Player* player = itr->second->FindPlayer(itr->second->GetGUID()))
            player->GetSession()->SendPacket(packet);
}

void Guild::MassInviteToEvent(WorldSession* session, uint32 minLevel, uint32 maxLevel, uint32 minRank)
{
    uint32 count = 0;

    WorldPacket data(SMSG_CALENDAR_FILTER_GUILD);
    data << uint32(count); // count placeholder

    for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        GuildMember* member = itr->second;
        uint32 level = member->GetLevel();

        if (member->GetGUID() != session->GetPlayer()->GetGUID() && level >= minLevel && level <= maxLevel && member->IsRankNotLower(minRank))
        {
            data.appendPackGUID(member->GetGUID());
            data << uint8(0); // unk
            ++count;
        }
    }

    data.put<uint32>(0, count);

    session->SendPacket(&data);
}

// Members handling
bool Guild::AddMember(uint64 guid, uint8 rankId)
{
    Player* player = objmgr.GetPlayer(Arcemu::Util::GUID_LOPART(guid));
    // Player cannot be in guild
    if (player)
    {
        if (player->GetGuildId() != 0)
            return false;
    }
    else if (Player::GetGuildIdFromDB(guid) != 0)
        return false;


    uint32 lowguid = player->GetLowGUID();

    // If rank was not passed, assign lowest possible rank
    if (rankId == GUILD_RANK_NONE)
        rankId = _GetLowestRankId();

    GuildMember* member = new GuildMember(m_id, guid, rankId);
    std::string name;
    if (player)
    {
        m_members[lowguid] = member;
        player->SetInGuild(m_id);
        player->SetGuildIdInvited(0);
        player->SetRank(rankId);
        player->SetGuildLevel(GetLevel());
        SendLoginInfo(player->GetSession());
        name = player->GetName();
    }
    else
    {
        member->ResetFlags();

        bool ok = false;
        // Player must exist
        if (objmgr.GetPlayerInfo(lowguid))
        {
            PlayerInfo* info = objmgr.GetPlayerInfo(lowguid);
            name = info->name;
            member->SetStats(
                name,
                info->lastLevel,
                info->cl,
                info->lastZone,
                info->acct,
                0);

            ok = member->CheckStats();
        }

        if (!ok)
        {
            delete member;
            return false;
        }
        m_members[lowguid] = member;
    }

    member->SaveToDB(false);

    CharacterDatabase.Execute("INSERT INTO guild_member_withdraw VALUES('%u', 0, 0, 0, 0, 0, 0, 0 , 0, 0 )", Arcemu::Util::GUID_LOPART(m_id));

    _UpdateAccountsNumber();
    _LogEvent(GUILD_EVENT_LOG_JOIN_GUILD, lowguid);
    _BroadcastEvent(GE_JOINED, guid, name.c_str());
    sGuildFinderMgr.RemoveAllMembershipRequestsFromPlayer(lowguid);

    // Call scripts if member was succesfully added (and stored to database)
    sHookInterface.OnGuildJoin(player, this);

    return true;
}

void Guild::DeleteMember(uint64 guid, bool isDisbanding, bool isKicked)
{
    uint32 lowguid = Arcemu::Util::GUID_LOPART(guid);
    Player* player = objmgr.GetPlayer(lowguid);

    // Guild master can be deleted when loading guild and guid doesn't exist in characters table
    // or when he is removed from guild by gm command
    if (m_leaderGuid == guid && !isDisbanding)
    {
        GuildMember* oldLeader = nullptr;
        GuildMember* newLeader = nullptr;
        for (Guild::Members::iterator i = m_members.begin(); i != m_members.end(); ++i)
        {
            if (i->first == lowguid)
                oldLeader = i->second;
            else if (!newLeader || newLeader->GetRankId() > i->second->GetRankId())
                newLeader = i->second;
        }

        if (!newLeader)
        {
            Disband();
            return;
        }

        _SetLeaderGUID(newLeader);

        // If player not online data in data field will be loaded from guild tabs no need to update it !!
        if (Player* newLeaderPlayer = newLeader->FindPlayer(newLeader->GetGUID()))
            newLeaderPlayer->SetRank(GR_GUILDMASTER);

        // If leader does not exist (at guild loading with deleted leader) do not send broadcasts
        if (oldLeader)
        {
            _BroadcastEvent(GE_LEADER_CHANGED, 0, oldLeader->GetName().c_str(), newLeader->GetName().c_str());
            _BroadcastEvent(GE_LEFT, guid, oldLeader->GetName().c_str());
        }
    }

    if (GuildMember* member = GetMember(guid))
        delete member;
    m_members.erase(lowguid);

    // If player not online data in data field will be loaded from guild tabs no need to update it !!
    if (player)
    {
        player->SetInGuild(0);
        player->SetRank(0);
        player->SetGuildLevel(0);

        for (uint32 i = 0; i < sGuildPerkSpellsStore.GetNumRows(); ++i)
            if (DBC::Structures::GuildPerkSpellsEntry const* entry = sGuildPerkSpellsStore.LookupEntry(i))
                if (entry->Level <= GetLevel())
                    player->removeSpell(entry->SpellId, false, false, 0);
    }

    CharacterDatabase.Execute("DELETE FROM guild_member WHERE guildId = %u", lowguid);

    if (!isDisbanding)
        _UpdateAccountsNumber();
}

bool Guild::ChangeMemberRank(uint64 guid, uint8 newRank)
{
    if (newRank <= _GetLowestRankId())                    // Validate rank (allow only existing ranks)
        if (GuildMember* member = GetMember(guid))
        {
            member->ChangeRank(newRank);
            return true;
        }
    return false;
}

bool Guild::IsMember(uint64 guid) const
{
    Members::const_iterator itr = m_members.find(Arcemu::Util::GUID_LOPART(guid));
    return itr != m_members.end();
}

// Bank tabs
void Guild::SetBankTabText(uint8 tabId, std::string const& text)
{
    if (GuildBankTab* pTab = GetBankTab(tabId))
    {
        pTab->SetText(text);
        pTab->SendText(this, nullptr);
    }
}

// Private methods
void Guild::_CreateLogHolders()
{
    m_eventLog = new GuildLogHolder(m_id, sWorld.m_guild.EventLogCount);
    m_newsLog = new GuildLogHolder(m_id, sWorld.m_guild.NewsLogCount);
    for (uint8 tabId = 0; tabId <= GUILD_BANK_MAX_TABS; ++tabId)
        m_bankEventLog[tabId] = new GuildLogHolder(m_id, sWorld.m_guild.BankLogCount);
}

void Guild::_CreateNewBankTab()
{
    uint8 tabId = _GetPurchasedTabsSize();                      // Next free id
    m_bankTabs.push_back(new GuildBankTab(m_id, tabId));

    CharacterDatabase.Execute("DELETE FROM guild_bank_tab WHERE guildId = %u AND tabId = %u", m_id, (uint32)tabId);

    CharacterDatabase.Execute("INSERT INTO guild_bank_tab VALUES('%u', '%u', '', '', '')", m_id, (uint32)tabId);

    ++tabId;
    for (Ranks::iterator itr = m_ranks.begin(); itr != m_ranks.end(); ++itr)
        (*itr).CreateMissingTabsIfNeeded(tabId, false, false);
}

//\todo danko implement locales
void Guild::_CreateDefaultGuildRanks(/*LocaleConstant loc*/)
{
    CharacterDatabase.Execute("DELETE FROM guild_rank WHERE guildId = %u", m_id);
    CharacterDatabase.Execute("DELETE FROM guild_bank_right WHERE guildId = %u", m_id);

    _CreateRank("GuildMaster", GR_RIGHT_ALL);
    _CreateRank("Officer", GR_RIGHT_ALL);
    _CreateRank("Veteran", GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
    _CreateRank("Member", GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
    _CreateRank("Initiante", GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
}

bool Guild::_CreateRank(std::string const& name, uint32 rights)
{
    uint8 newRankId = _GetRanksSize();
    if (newRankId >= GUILD_RANKS_MAX_COUNT)
        return false;

    // Ranks represent sequence 0, 1, 2, ... where 0 means guildmaster
    GuildRankInfo info(m_id, newRankId, name, rights, 0);
    m_ranks.push_back(info);

    info.CreateMissingTabsIfNeeded(_GetPurchasedTabsSize(), false, false);
    info.SaveToDB(false);

    return true;
}

// Updates the number of accounts that are in the guild
// Player may have many characters in the guild, but with the same account
void Guild::_UpdateAccountsNumber()
{
    // We use a set to be sure each element will be unique
    std::set<uint32> accountsIdSet;
    for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        accountsIdSet.insert(itr->second->GetAccountId());

    m_accountsNumber = accountsIdSet.size();
}

// Detects if player is the guild master.
// Check both leader guid and player's rank (otherwise multiple feature with
// multiple guild masters won't work)
bool Guild::_IsLeader(Player* player) const
{
    if (player->GetGUID() == m_leaderGuid)
        return true;
    if (const GuildMember* member = GetMember(player->GetGUID()))
        return member->IsRank(GR_GUILDMASTER);
    return false;
}

void Guild::_DeleteBankItems(bool removeItemsFromDB)
{
    for (uint8 tabId = 0; tabId < _GetPurchasedTabsSize(); ++tabId)
    {
        m_bankTabs[tabId]->Delete(removeItemsFromDB);
        delete m_bankTabs[tabId];
        m_bankTabs[tabId] = nullptr;
    }
    m_bankTabs.clear();
}

bool Guild::_ModifyBankMoney(uint64 amount, bool add)
{
    if (add)
        m_bankMoney += amount;
    else
    {
        // Check if there is enough money in bank.
        if (m_bankMoney < amount)
            return false;
        m_bankMoney -= amount;
    }

    CharacterDatabase.Execute("UPDATE guild SET bankBalance = %llu WHERE guildId = %u", m_bankMoney, m_id);

    return true;
}

void Guild::_SetLeaderGUID(GuildMember* pLeader)
{
    if (!pLeader)
        return;

    m_leaderGuid = pLeader->GetGUID();
    pLeader->ChangeRank(GR_GUILDMASTER);

    CharacterDatabase.Execute("UPDATE guild SET leaderGuid = '%u' WHERE guildId = %u", Arcemu::Util::GUID_LOPART(m_leaderGuid), m_id);
}

void Guild::_SetRankBankMoneyPerDay(uint8 rankId, uint32 moneyPerDay)
{
    if (GuildRankInfo* rankInfo = GetRankInfo(rankId))
        rankInfo->SetBankMoneyPerDay(moneyPerDay);
}

void Guild::_SetRankBankTabRightsAndSlots(uint8 rankId, GuildBankRightsAndSlots rightsAndSlots, bool saveToDB)
{
    if (rightsAndSlots.GetTabId() >= _GetPurchasedTabsSize())
        return;

    if (GuildRankInfo* rankInfo = GetRankInfo(rankId))
        rankInfo->SetBankTabSlotsAndRights(rightsAndSlots, saveToDB);
}

inline std::string Guild::_GetRankName(uint8 rankId) const
{
    if (const GuildRankInfo* rankInfo = GetRankInfo(rankId))
        return rankInfo->GetName();
    return "<unknown>";
}

inline uint32 Guild::_GetRankRights(uint8 rankId) const
{
    if (const GuildRankInfo* rankInfo = GetRankInfo(rankId))
        return rankInfo->GetRights();
    return 0;
}

inline int32 Guild::_GetRankBankMoneyPerDay(uint8 rankId) const
{
    if (const GuildRankInfo* rankInfo = GetRankInfo(rankId))
        return rankInfo->GetBankMoneyPerDay();
    return 0;
}

inline int32 Guild::_GetRankBankTabSlotsPerDay(uint8 rankId, uint8 tabId) const
{
    if (tabId < _GetPurchasedTabsSize())
        if (const GuildRankInfo* rankInfo = GetRankInfo(rankId))
            return rankInfo->GetBankTabSlotsPerDay(tabId);
    return 0;
}

inline int8 Guild::_GetRankBankTabRights(uint8 rankId, uint8 tabId) const
{
    if (const GuildRankInfo* rankInfo = GetRankInfo(rankId))
        return rankInfo->GetBankTabRights(tabId);
    return 0;
}

inline int32 Guild::_GetMemberRemainingSlots(GuildMember const* member, uint8 tabId) const
{
    if (member)
    {
        uint8 rankId = member->GetRankId();
        if (rankId == GR_GUILDMASTER)
            return GUILD_WITHDRAW_SLOT_UNLIMITED;
        if ((_GetRankBankTabRights(rankId, tabId) & GUILD_BANK_RIGHT_VIEW_TAB) != GR_RIGHT_EMPTY)
        {
            int32 remaining = _GetRankBankTabSlotsPerDay(rankId, tabId) - member->GetBankWithdrawValue(tabId);
            if (remaining > 0)
                return remaining;
        }
    }
    return 0;
}

inline int32 Guild::_GetMemberRemainingMoney(GuildMember const* member) const
{
    if (member)
    {
        uint8 rankId = member->GetRankId();
        if (rankId == GR_GUILDMASTER)
            return GUILD_WITHDRAW_MONEY_UNLIMITED;

        if ((_GetRankRights(rankId) & (GR_RIGHT_WITHDRAW_REPAIR | GR_RIGHT_WITHDRAW_GOLD)) != GR_RIGHT_EMPTY)
        {
            int32 remaining = _GetRankBankMoneyPerDay(rankId) - member->GetBankWithdrawValue(GUILD_BANK_MAX_TABS);
            if (remaining > 0)
                return remaining;
        }
    }
    return 0;
}

inline void Guild::_UpdateMemberWithdrawSlots(uint64 guid, uint8 tabId)
{
    if (GuildMember* member = GetMember(guid))
    {
        uint8 rankId = member->GetRankId();
        if (rankId != GR_GUILDMASTER
            && member->GetBankWithdrawValue(tabId) < _GetRankBankTabSlotsPerDay(rankId, tabId))
            member->UpdateBankWithdrawValue(tabId, 1);
    }
}

inline bool Guild::_MemberHasTabRights(uint64 guid, uint8 tabId, uint32 rights) const
{
    if (const GuildMember* member = GetMember(guid))
    {
        // Leader always has full rights
        if (member->IsRank(GR_GUILDMASTER) || m_leaderGuid == guid)
            return true;
        return (_GetRankBankTabRights(member->GetRankId(), tabId) & rights) == rights;
    }
    return false;
}

// Add new event log record
inline void Guild::_LogEvent(GuildEventLogTypes eventType, uint32 playerGuid1, uint32 playerGuid2, uint8 newRank)
{
    m_eventLog->AddEvent(new GuildEventLogEntry(m_id, m_eventLog->GetNextGUID(), eventType, playerGuid1, playerGuid2, newRank));;
}

// Add new bank event log record
void Guild::_LogBankEvent(GuildBankEventLogTypes eventType, uint8 tabId, uint32 lowguid, uint32 itemOrMoney, uint16 itemStackCount, uint8 destTabId)
{
    if (tabId > GUILD_BANK_MAX_TABS)
        return;

    // not logging moves within the same tab
    if (eventType == GUILD_BANK_LOG_MOVE_ITEM && tabId == destTabId)
        return;

    uint8 dbTabId = tabId;
    if (GuildBankEventLogEntry::IsMoneyEvent(eventType))
    {
        tabId = GUILD_BANK_MAX_TABS;
        dbTabId = GUILD_BANK_MONEY_LOGS_TAB;
    }
    GuildLogHolder* pLog = m_bankEventLog[tabId];
    pLog->AddEvent(new GuildBankEventLogEntry(m_id, pLog->GetNextGUID(), eventType, dbTabId, lowguid, itemOrMoney, itemStackCount, destTabId));
}

void Guild::_BroadcastEvent(GuildEvents guildEvent, uint64 guid, const char* param1, const char* param2, const char* param3) const
{
    uint8 count = !param3 ? (!param2 ? (!param1 ? 0 : 1) : 2) : 3;

    WorldPacket data(SMSG_GUILD_EVENT, 1 + 1 + count + (guid ? 8 : 0));
    data << uint8(guildEvent);
    data << uint8(count);

    if (param3)
        data << param1 << param2 << param3;
    else if (param2)
        data << param1 << param2;
    else if (param1)
        data << param1;

    if (guid)
        data << uint64(guid);

    BroadcastPacket(&data);

    Log.Debug("Guild", "SMSG_GUILD_EVENT [Broadcast] Event: %s (%u)", _GetGuildEventString(guildEvent).c_str(), guildEvent);
}

void Guild::SendBankList(WorldSession* session, uint8 tabId, bool withContent, bool withTabInfo) const
{
    GuildMember const* member = GetMember(session->GetPlayer()->GetGUID());
    if (!member) // Shouldn't happen, just in case
        return;

    ByteBuffer tabData;
    WorldPacket data(SMSG_GUILD_BANK_LIST, 500);
    data.writeBit(0);
    uint32 itemCount = 0;
    if (withContent && _MemberHasTabRights(session->GetPlayer()->GetGUID(), tabId, GUILD_BANK_RIGHT_VIEW_TAB))
        if (GuildBankTab const* tab = GetBankTab(tabId))
            for (uint8 slotId = 0; slotId < GUILD_BANK_MAX_SLOTS; ++slotId)
                if (tab->GetItem(slotId))
                    ++itemCount;

    data.writeBits(itemCount, 20);
    data.writeBits(withTabInfo ? _GetPurchasedTabsSize() : 0, 22);
    if (withContent && _MemberHasTabRights(session->GetPlayer()->GetGUID(), tabId, GUILD_BANK_RIGHT_VIEW_TAB))
    {
        if (GuildBankTab const* tab = GetBankTab(tabId))
        {
            for (uint8 slotId = 0; slotId < GUILD_BANK_MAX_SLOTS; ++slotId)
            {
                if (Item* tabItem = tab->GetItem(slotId))
                {
                    data.writeBit(0);

                    uint32 enchants = 0;
                    for (uint32 ench = 0; ench < MAX_ENCHANTMENT_SLOT; ++ench)
                    {
                        if (uint32 enchantId = tabItem->GetEnchantmentId(EnchantmentSlot(ench)))
                        {
                            tabData << uint32(enchantId);
                            tabData << uint32(ench);
                            ++enchants;
                        }
                    }

                    data.writeBits(enchants, 23);

                    tabData << uint32(0);
                    tabData << uint32(0);
                    tabData << uint32(0);
                    tabData << uint32(tabItem->GetStackCount());                 // ITEM_FIELD_STACK_COUNT
                    tabData << uint32(slotId);
                    tabData << uint32(0);
                    tabData << uint32(tabItem->GetEntry());
                    tabData << uint32(tabItem->GetItemRandomPropertyId());
                    tabData << uint32(abs(0));     // Spell charges
                    tabData << uint32(tabItem->GetItemRandomSuffixFactor());      // SuffixFactor
                }
            }
        }
    }

    if (withTabInfo)
    {
        for (uint8 i = 0; i < _GetPurchasedTabsSize(); ++i)
        {
            data.writeBits(m_bankTabs[i]->GetIcon().length(), 9);
            data.writeBits(m_bankTabs[i]->GetName().length(), 7);
        }
    }

    data.flushBits();

    if (withTabInfo)
    {
        for (uint8 i = 0; i < _GetPurchasedTabsSize(); ++i)
        {
            data.WriteString(m_bankTabs[i]->GetIcon());
            data << uint32(i);
            data.WriteString(m_bankTabs[i]->GetName());
        }
    }

    data << uint64(m_bankMoney);
    if (tabData.size())
        data.append(tabData);

    data << uint32(tabId);
    data << uint32(_GetMemberRemainingSlots(member, tabId));

    session->SendPacket(&data);

    Log.Debug("Guild", "WORLD: Sent (SMSG_GUILD_BANK_LIST)");
}

void Guild::SendGuildRanksUpdate(uint64 setterGuid, uint64 targetGuid, uint32 rank)
{
    ObjectGuid tarGuid = targetGuid;
    ObjectGuid setGuid = setterGuid;

    GuildMember* member = GetMember(targetGuid);
    ASSERT(member);

    WorldPacket data(SMSG_GUILD_RANKS_UPDATE, 100);
    data.writeBit(setGuid[7]);
    data.writeBit(setGuid[2]);
    data.writeBit(tarGuid[2]);
    data.writeBit(setGuid[1]);
    data.writeBit(tarGuid[1]);
    data.writeBit(tarGuid[7]);
    data.writeBit(tarGuid[0]);
    data.writeBit(tarGuid[5]);
    data.writeBit(tarGuid[4]);
    data.writeBit(rank < member->GetRankId());
    data.writeBit(setGuid[5]);
    data.writeBit(setGuid[0]);
    data.writeBit(tarGuid[6]);
    data.writeBit(setGuid[3]);
    data.writeBit(setGuid[6]);
    data.writeBit(tarGuid[3]);
    data.writeBit(setGuid[4]);

    data.flushBits();

    data << uint32(rank);
    data.WriteByteSeq(setGuid[3]);
    data.WriteByteSeq(tarGuid[7]);
    data.WriteByteSeq(setGuid[6]);
    data.WriteByteSeq(setGuid[2]);
    data.WriteByteSeq(tarGuid[5]);
    data.WriteByteSeq(tarGuid[0]);
    data.WriteByteSeq(setGuid[7]);
    data.WriteByteSeq(setGuid[5]);
    data.WriteByteSeq(tarGuid[2]);
    data.WriteByteSeq(tarGuid[1]);
    data.WriteByteSeq(setGuid[0]);
    data.WriteByteSeq(setGuid[4]);
    data.WriteByteSeq(setGuid[1]);
    data.WriteByteSeq(tarGuid[3]);
    data.WriteByteSeq(tarGuid[6]);
    data.WriteByteSeq(tarGuid[4]);
    BroadcastPacket(&data);

    member->ChangeRank(rank);

    Log.Debug("Guild", "SMSG_GUILD_RANKS_UPDATE [Broadcast] Target: %u, Issuer: %u, RankId: %u",
              Arcemu::Util::GUID_LOPART(targetGuid), Arcemu::Util::GUID_LOPART(setterGuid), rank);
}

void Guild::GiveXP(uint32 xp, Player* source)
{
    if (!sWorld.m_guild.LevlingEnabled)
        return;

    /// @TODO: Award reputation and count activity for player

    if (GetLevel() >= sWorld.m_guild.MaxLevel)
        xp = 0; // SMSG_GUILD_XP_GAIN is always sent, even for no gains

    if (GetLevel() >= GUILD_EXPERIENCE_UNCAPPED_LEVEL)
        xp = std::min(xp, sWorld.m_guild.MaxXpPerDay - uint32(_todayExperience));

    WorldPacket data(SMSG_GUILD_XP_GAIN, 8);
    data << uint64(xp);
    source->GetSession()->SendPacket(&data);

    _experience += xp;
    _todayExperience += xp;

    if (!xp)
        return;

    uint32 oldLevel = GetLevel();

    while (GetExperience() >= sGuildMgr.GetXPForGuildLevel(GetLevel()) && GetLevel() < 25)
    {
        _experience -= sGuildMgr.GetXPForGuildLevel(GetLevel());
        ++_level;

        // Find all guild perks to learn
        std::vector<uint32> perksToLearn;
        for (uint32 i = 0; i < sGuildPerkSpellsStore.GetNumRows(); ++i)
            if (DBC::Structures::GuildPerkSpellsEntry const* entry = sGuildPerkSpellsStore.LookupEntry(i))
                if (entry->Level > oldLevel && entry->Level <= GetLevel())
                    perksToLearn.push_back(entry->SpellId);

        // Notify all online players that guild level changed and learn perks
        for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
        {
            if (Player* player = itr->second->FindPlayer(itr->second->GetGUID()))
            {
                player->SetGuildLevel(GetLevel());
                for (size_t i = 0; i < perksToLearn.size(); ++i)
                    player->addSpell(perksToLearn[i]);
            }
        }

        AddGuildNews(GUILD_NEWS_LEVEL_UP, 0, 0, _level);
        //UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_REACH_GUILD_LEVEL, GetLevel(), 0, 0, nullptr, source);

        ++oldLevel;
    }
}

void Guild::SendGuildXP(WorldSession* session /* = nullptr */) const
{
    //Member const* member = GetMember(session->GetGuidLow());

    WorldPacket data(SMSG_GUILD_XP, 40);
    data << uint64(/*member ? member->GetTotalActivity() :*/ 0);
    data << uint64(sGuildMgr.GetXPForGuildLevel(GetLevel()) - GetExperience());    // XP missing for next level
    data << uint64(GetTodayExperience());
    data << uint64(/*member ? member->GetWeeklyActivity() :*/ 0);
    data << uint64(GetExperience());
    session->SendPacket(&data);
}

void Guild::SendGuildReputationWeeklyCap(WorldSession* session, uint32 reputation) const
{
    uint32 cap = sWorld.m_guild.MaxRepPerWeek - reputation;
    WorldPacket data(SMSG_GUILD_REPUTATION_WEEKLY_CAP, 4);
    data << uint32(cap);
    session->SendPacket(&data);
    Log.Debug("Guild", "SMSG_GUILD_REPUTATION_WEEKLY_CAP [%s]: Left: %u", session->GetPlayer()->GetName(), cap);
}

void Guild::ResetTimes(bool weekly)
{
    _todayExperience = 0;
    for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
    {
        itr->second->ResetValues(weekly);
        if (Player* player = itr->second->FindPlayer(itr->second->GetGUID()))
        {
            //SendGuildXP(player->GetSession());
            WorldPacket data(SMSG_GUILD_MEMBER_DAILY_RESET, 0);  // tells the client to request bank withdrawal limit
            player->GetSession()->SendPacket(&data);
        }
    }
}

void Guild::AddGuildNews(uint8 type, uint64 guid, uint32 flags, uint32 value)
{
    uint32 lowGuid = Arcemu::Util::GUID_LOPART(guid);
    GuildNewsLogEntry* news = new GuildNewsLogEntry(m_id, m_newsLog->GetNextGUID(), GuildNews(type), lowGuid, flags, value);

    m_newsLog->AddEvent(news);

    WorldPacket data(SMSG_GUILD_NEWS_UPDATE, 7 + 32);
    data.writeBits(1, 21); // size, we are only sending 1 news here
    ByteBuffer buffer;
    news->WritePacket(data, buffer);

    BroadcastPacket(&data);
}

bool Guild::HasAchieved(uint32 achievementId) const
{
    //return m_achievementMgr.HasAchieved(achievementId);
    return false;
}

void Guild::HandleNewsSetSticky(WorldSession* session, uint32 newsId, bool sticky)
{
    GuildLog* logs = m_newsLog->GetGuildLog();
    GuildLog::iterator itr = logs->begin();
    while (itr != logs->end() && (*itr)->GetGUID() != newsId)
        ++itr;

    if (itr == logs->end())
    {
        Log.Debug("Guild", "HandleNewsSetSticky: [%s] requested unknown newsId %u - Sticky: %u", session->GetPlayer()->GetName(), newsId, sticky);
        return;
    }

    GuildNewsLogEntry* news = (GuildNewsLogEntry*)(*itr);
    news->SetSticky(sticky);

    Log.Debug("Guild", "HandleNewsSetSticky: [%s] chenged newsId %u sticky to %u", session->GetPlayer()->GetName(), newsId, sticky);

    WorldPacket data(SMSG_GUILD_NEWS_UPDATE, 7 + 32);
    data.writeBits(1, 21);
    ByteBuffer buffer;
    news->WritePacket(data, buffer);
    session->SendPacket(&data);
}

Player* Guild::GuildMember::FindPlayer(uint64 m_guid)
{
    return objmgr.GetPlayer(Arcemu::Util::GUID_LOPART(m_guid));
}

void Guild::SendTurnInPetitionResult(WorldSession * pClient, uint32 result)
{
    if (pClient == nullptr)
        return;

    WorldPacket data(SMSG_TURN_IN_PETITION_RESULTS, 4);
    data << result;
    pClient->SendPacket(&data);
}

// Bank (items move)
void Guild::SwapItems(Player* player, uint8 tabId, uint8 slotId, uint8 destTabId, uint8 destSlotId, uint32 splitedAmount)
{
    if (tabId >= _GetPurchasedTabsSize() || slotId >= GUILD_BANK_MAX_SLOTS ||
        destTabId >= _GetPurchasedTabsSize() || destSlotId >= GUILD_BANK_MAX_SLOTS)
        return;

    if (tabId == destTabId && slotId == destSlotId)
        return;

    Item* pItem = GetBankTab(tabId)->GetItem(slotId);
    Item* pItem2 = nullptr;

    if (!_MemberHasTabRights(player->GetGUID(), tabId, GUILD_BANK_RIGHT_DEPOSIT_ITEM))
        return; // Player has no rights to store item in destination

    /// splitting
    if (pItem != nullptr && splitedAmount > 0 && pItem->GetStackCount() > splitedAmount)
    {
        pItem2 = pItem;

        pItem2->ModStackCount(-(int32)splitedAmount);
        pItem2->SetCreatorGUID(0);
        pItem2->SaveToDB(0, 0, true, nullptr);

        pItem = objmgr.CreateItem(pItem2->GetEntry(), player);
        if (pItem == nullptr)
            return;

        pItem->SetStackCount(splitedAmount);
        pItem->SetCreatorGUID(0);
        pItem->SaveToDB(0, 0, true, nullptr);
    }
    else
    {
        // We are actualy moving whole Stack or single Item
        pItem2 = pItem;
        pItem = nullptr;
    }

    // Set New Informations
    GetBankTab(destTabId)->SetItem(slotId, pItem);
    GetBankTab(destTabId)->SetItem(destSlotId, pItem2);
    
    // Update Slots
    SlotIds slots;
    slots.insert(slotId);
    slots.insert(destSlotId);

    // Send Interface Update
    _SendBankContentUpdate(tabId, slots);
}

void Guild::SwapItemsWithInventory(Player* player, bool toChar, uint8 tabId, uint8 slotId, uint8 playerBag, uint8 playerSlotId, uint32 splitedAmount)
{
    if ((slotId >= GUILD_BANK_MAX_SLOTS && slotId != 255) || tabId >= _GetPurchasedTabsSize())
        return;


    Item* pSourceItem = player->GetItemInterface()->GetInventoryItem(playerBag, playerSlotId);
    Item* pDestItem = GetBankTab(tabId)->GetItem(slotId);
    Item* pSourceItem2 = nullptr;

    if (pSourceItem != nullptr)
    {
        // make sure its not a soulbound item
        if (pSourceItem->IsSoulbound() || pSourceItem->GetItemProperties()->Class == ITEM_CLASS_QUEST)
        {
            player->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_CANT_DROP_SOULBOUND);
            return;
        }
    }

    if (!toChar)
    {
        if (!_MemberHasTabRights(player->GetGUID(), tabId, GUILD_BANK_RIGHT_DEPOSIT_ITEM))
            return; // Player has no rights to store item in destination

        // pull the item from the slot
        if (splitedAmount && pSourceItem->GetStackCount() > splitedAmount)
        {
            pSourceItem2 = pSourceItem;
            pSourceItem = objmgr.CreateItem(pSourceItem2->GetEntry(), player);
            if (pSourceItem == nullptr)
                return;

            pSourceItem->SetStackCount(splitedAmount);
            pSourceItem->SetCreatorGUID(pSourceItem2->GetCreatorGUID());
            pSourceItem2->ModStackCount(-(int32)splitedAmount);
            pSourceItem2->m_isDirty = true;
        }
        else
        {
            if (!player->GetItemInterface()->SafeRemoveAndRetreiveItemFromSlot(playerBag, playerSlotId, false))
                return;

            pSourceItem->RemoveFromWorld();
        }

        // perform the swap.
        // pSourceItem = Source item from players backpack coming into guild bank
        if (pSourceItem == nullptr)
        {
            /// splitting
            if (pDestItem != nullptr && splitedAmount > 0 && pDestItem->GetStackCount() > splitedAmount)
            {
                pSourceItem2 = pDestItem;

                pSourceItem2->ModStackCount(-(int32)splitedAmount);
                pSourceItem2->SaveToDB(0, 0, true, nullptr);

                pDestItem = objmgr.CreateItem(pSourceItem2->GetEntry(), player);
                if (pDestItem == nullptr)
                    return;

                pDestItem->SetStackCount(splitedAmount);
                pDestItem->SetCreatorGUID(pSourceItem2->GetCreatorGUID());
            }
            else
            {
                GetBankTab(tabId)->SetItem(slotId, nullptr);
            }
        }
        else
        {
            /// there is a new item in that slot.
            GetBankTab(tabId)->SetItem(slotId, pSourceItem);

            /// remove the item's association with the player
            pSourceItem->SetOwner(nullptr);
            pSourceItem->SetOwnerGUID(0);
            pSourceItem->SaveToDB(0, 0, true, nullptr);
        }
    }
    else
    {
        /// pDestItem = Item from bank coming into players backpack
        if (pDestItem)
        {
            //Check source withdraw rights
            if (!_GetMemberRemainingSlots(GetMember(player->GetGUID()), tabId))
                return; // Player has no rights to withdraw items from source

            /// the guild was robbed by some n00b! :O
            pDestItem->SetOwner(player);
            pDestItem->SetOwnerGUID(player->GetGUID());
            pDestItem->SaveToDB(playerBag, playerSlotId, true, nullptr);
            GetBankTab(tabId)->SetItem(slotId, nullptr);

            /// add it to him in game
            if (!player->GetItemInterface()->SafeAddItem(pDestItem, playerBag, playerSlotId))
            {
                /// this *really* shouldn't happen.
                if (!player->GetItemInterface()->AddItemToFreeSlot(pDestItem))
                {
                    //pDestItem->DeleteFromDB();
                    pDestItem->DeleteMe();
                }
            }

           /// log it
           _LogBankEvent(GUILD_BANK_LOG_WITHDRAW_ITEM, tabId, player->GetLowGUID(),
                GetBankTab(tabId)->GetItem(slotId)->GetEntry(), GetBankTab(tabId)->GetItem(slotId)->GetStackCount());
        }
    }

    SlotIds slots;

    slots.insert(slotId);

    _SendBankContentUpdate(tabId, slots);
}

void Guild::_SendBankContentUpdate(uint8 tabId, SlotIds slots) const
{
    if (GuildBankTab const* tab = GetBankTab(tabId))
    {
        ByteBuffer tabData;
        WorldPacket data(SMSG_GUILD_BANK_LIST, 1200);
        data.writeBit(0);
        data.writeBits(slots.size(), 20);                                           // Item count
        data.writeBits(0, 22);                                                      // Tab count

        for (SlotIds::const_iterator itr = slots.begin(); itr != slots.end(); ++itr)
        {
            data.writeBit(0);

            Item * tabItem = tab->GetItem(*itr);
            uint32 enchantCount = 0;
            if (tabItem)
            {
                for (uint32 enchSlot = 0; enchSlot < MAX_ENCHANTMENT_SLOT; ++enchSlot)
                {
                    if (uint32 enchantId = tabItem->GetEnchantmentId(EnchantmentSlot(enchSlot)))
                    {
                        tabData << uint32(enchantId);
                        tabData << uint32(enchSlot);
                        ++enchantCount;
                    }
                }
            }

            data.writeBits(enchantCount, 23);                                       // enchantment count

            tabData << uint32(0);
            tabData << uint32(0);
            tabData << uint32(0);
            tabData << uint32(tabItem ? tabItem->GetStackCount() : 0);                   // ITEM_FIELD_STACK_COUNT
            tabData << uint32(*itr);
            tabData << uint32(0);
            tabData << uint32(tabItem ? tabItem->GetEntry() : 0);
            tabData << uint32(tabItem ? tabItem->GetItemRandomPropertyId() : 0);
            tabData << uint32(tabItem ? 0 : 0);       // Spell charges
            tabData << uint32(tabItem ? tabItem->GetItemRandomSuffixFactor() : 0);        // SuffixFactor
        }

        data.flushBits();

        data << uint64(m_bankMoney);
        if (tabData.size())
            data.append(tabData);

        data << uint32(tabId);

        size_t rempos = data.wpos();
        data << uint32(0);                                      // Item withdraw amount, will be filled later

        for (Members::const_iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
            if (_MemberHasTabRights(itr->second->GetGUID(), tabId, GUILD_BANK_RIGHT_VIEW_TAB))
                if (Player* player = itr->second->FindPlayer(itr->second->GetGUID()))
                {
                    data.put<uint32>(rempos, uint32(_GetMemberRemainingSlots(itr->second, tabId)));
                    player->GetSession()->SendPacket(&data);
                }

        Log.Debug("Guild", "Sent (SMSG_GUILD_BANK_LIST)");
    }
}

bool Guild::GuildBankTab::SetItem(uint8 slotId, Item* item)
{
    if (slotId >= GUILD_BANK_MAX_SLOTS)
        return false;

    m_items[slotId] = item;

    CharacterDatabase.Execute("DELETE FROM guild_bank_item WHERE guildId = %u AND tabId = %u AND slotId = %u",
                              m_guildId, (uint32)m_tabId, (uint32)slotId);

    if (item)
    {
        CharacterDatabase.Execute("INSERT INTO guild_bank_item VALUES ('%u','%u','%u','%u')",
                                  m_guildId, (uint32)m_tabId, (uint32)slotId, item->GetLowGUID());
    }
    return true;
}
