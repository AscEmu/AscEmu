/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "Guild.h"
#include "Management/GuildMgr.h"
#if VERSION_STRING == Cata
#include "GameCata/Management/GuildFinderMgr.h"
#elif VERSION_STRING == Mop
#include "GameMop/Management/GuildFinderMgr.h"
#endif
#include "Chat/ChatHandler.hpp"
#include "Server/MainServerDefines.h"
#include "Objects/ObjectMgr.h"
#include "Units/Players/Player.h"
#include "Management/ItemInterface.h"
#include "Server/Packets/SmsgGuildCommandResult.h"
#include "Server/Packets/MsgSaveGuildEmblem.h"
#include "Server/Packets/SmsgGuildBankMoneyWithdrawn.h"
#include "Server/Packets/MsgGuildBankMoneyWithdrawn.h"
#include "Server/Packets/SmsgGuildInvite.h"
#include "Server/Packets/SmsgGuildEvent.h"

using namespace AscEmu::Packets;

Guild::Guild() : m_id(0), m_leaderGuid(0), m_createdDate(0), m_bankMoney(0),
    m_level(1), m_experience(0), m_todayExperience(0), mAccountsNumber(0),
    mEventLog(nullptr), mBankEventLog{ nullptr }, mNewsLog(nullptr)
{
    memset(&mBankEventLog, 0, (MAX_GUILD_BANK_TABS + 1) * sizeof(GuildLogHolder*));
}

inline uint32_t GetGuildBankTabPrice(uint8_t tabId)
{
    // these prices are in gold units, not copper
    static uint32_t const tabPrices[MAX_GUILD_BANK_TABS] = { 100, 250, 500, 1000, 2500, 5000 };
    ASSERT(tabId < MAX_GUILD_BANK_TABS);

    return tabPrices[tabId];
}

void Guild::sendGuildCommandResult(WorldSession* session, uint32_t guildCommand, std::string text, uint32_t error)
{
    session->SendPacket(SmsgGuildCommandResult(guildCommand, text, error).serialise().get());
}

void Guild::sendGuildInvitePacket(WorldSession* session, std::string invitedName)
{
    const auto invitedPlayer = objmgr.GetPlayer(invitedName.c_str(), false);
    const auto guild = session->GetPlayer()->GetGuild();

    if (invitedPlayer == nullptr)
    {
        session->SendPacket(SmsgGuildCommandResult(GC_TYPE_INVITE, invitedName, GC_ERROR_PLAYER_NOT_FOUND_S).serialise().get());
        return;
    }

    if (guild == nullptr)
    {
        session->SendPacket(SmsgGuildCommandResult(GC_TYPE_CREATE, "", GC_ERROR_PLAYER_NOT_IN_GUILD).serialise().get());
        return;
    }

    if (invitedPlayer->getGuildId())
    {
        session->SendPacket(SmsgGuildCommandResult(GC_TYPE_INVITE, invitedPlayer->getName(), GC_ERROR_ALREADY_IN_GUILD_S).serialise().get());
        return;
    }

    if (invitedPlayer->GetGuildIdInvited())
    {
        session->SendPacket(SmsgGuildCommandResult(GC_TYPE_INVITE, invitedPlayer->getName(), GC_ERROR_ALREADY_INVITED_TO_GUILD).serialise().get());
        return;
    }

    if (!session->GetPlayer()->GetGuild()->_hasRankRight(session->GetPlayer()->getGuid(), GR_RIGHT_INVITE))
    {
        session->SendPacket(SmsgGuildCommandResult(GC_TYPE_INVITE, "", GC_ERROR_PERMISSIONS).serialise().get());
        return;
    }

    if (invitedPlayer->getTeam() != session->GetPlayer()->getTeam() && session->GetPlayer()->GetSession()->GetPermissionCount() == 0 && !worldConfig.player.isInterfactionGuildEnabled)
    {
        session->SendPacket(SmsgGuildCommandResult(GC_TYPE_INVITE, "", GC_ERROR_NOT_ALLIED).serialise().get());
        return;
    }

    const auto memberCount = guild->getMembersCount();
    if (worldConfig.guild.maxMembers > 0 && memberCount >= worldConfig.guild.maxMembers)
    {
        session->SystemMessage("Your guild is full.");
        return;
    }

    session->SendPacket(SmsgGuildCommandResult(GC_TYPE_INVITE, invitedName, GC_ERROR_SUCCESS).serialise().get());

    guild->logEvent(GE_LOG_INVITE_PLAYER, session->GetPlayer()->getGuidLow(), invitedPlayer->getGuidLow());
    invitedPlayer->SetGuildIdInvited(guild->getId());

#if VERSION_STRING < Cata
    invitedPlayer->GetSession()->SendPacket(SmsgGuildInvite(session->GetPlayer()->getName(), guild->getName()).serialise().get());

#else
    invitedPlayer->GetSession()->SendPacket(SmsgGuildInvite(session->GetPlayer()->getName(), guild->getName(), guild->getLevel(),
        guild->getEmblemInfo(), guild->getId(), guild->getGUID()).serialise().get());
#endif
}

Guild::~Guild()
{
    delete mEventLog;
    mEventLog = nullptr;
    delete mNewsLog;
    mNewsLog = nullptr;

    for (uint8_t tabId = 0; tabId <= MAX_GUILD_BANK_TABS; ++tabId)
    {
        delete mBankEventLog[tabId];
        mBankEventLog[tabId] = nullptr;
    }

    for (auto itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
    {
        delete itr->second;
        itr->second = nullptr;
    }
}

bool Guild::create(Player* pLeader, std::string const& name)
{
    if (sGuildMgr.getGuildByName(name))
        return false;

    WorldSession* pLeaderSession = pLeader->GetSession();
    if (pLeaderSession == nullptr)
        return false;

    m_id = sGuildMgr.getNextGuildId();
    m_leaderGuid = pLeader->getGuid();
    m_name = name;
    m_info = "No message set.";
    m_motd = "No message set.";
    m_bankMoney = 0;
    m_createdDate = ::time(nullptr);
    m_level = 1;
    m_experience = 0;
    m_todayExperience = 0;
    createLogHolders();

    LogDebug("GUILD: creating guild %s for leader %s (%u)", name.c_str(), pLeader->getName().c_str(), WoWGuid::getGuidLowPartFromUInt64(m_leaderGuid));

    CharacterDatabase.Execute("DELETE FROM guild_members WHERE guildId = %u", m_id);

    CharacterDatabase.Execute("INSERT INTO guilds (guildId, guildName, leaderGuid, emblemStyle, emblemColor, borderStyle, borderColor, backgroundColor,"
        "guildInfo, motd, createdate, bankBalance, guildLevel, guildExperience, todayExperience) "
        "VALUES('%u', '%s', '%u', '%u', '%u', '%u', '%u', '%u', '%s', '%s', '%u', '%u', '%u', '0', '0')",
        m_id, name.c_str(), m_leaderGuid, m_emblemInfo.getStyle(), m_emblemInfo.getColor(), m_emblemInfo.getBorderStyle(),
        m_emblemInfo.getBorderColor(), m_emblemInfo.getBackgroundColor(), m_info.c_str(), m_motd.c_str(), uint32_t(m_createdDate),
        m_bankMoney, m_level);

    createDefaultGuildRanks();
    bool ret = addMember(m_leaderGuid, GR_GUILDMASTER);

    if (ret)
    {
#if VERSION_STRING >= Cata
        broadcastEvent(GE_FOUNDER, 0, {});
#endif
        sHookInterface.OnGuildCreate(pLeader, this);
    }

    saveGuildToDB();

    return ret;
}

void Guild::disband()
{
    broadcastEvent(GE_DISBANDED, 0, {});
    while (!_guildMembersStore.empty())
    {
        GuildMembersStore::const_iterator itr = _guildMembersStore.begin();
        deleteMember(itr->second->getGUID(), true);
    }

    CharacterDatabase.Execute("DELETE FROM guilds WHERE guildId = %u", m_id);

    CharacterDatabase.Execute("DELETE FROM guildranks WHERE guildId = %u", m_id);

    CharacterDatabase.Execute("DELETE FROM guild_bank_tabs WHERE guildId = %u", m_id);

    deleteBankItems(true);

    CharacterDatabase.Execute("DELETE FROM guild_bank_items WHERE guildId = %u", m_id);

    CharacterDatabase.Execute("DELETE FROM guild_bank_rights WHERE guildId = %u", m_id);

    CharacterDatabase.Execute("DELETE FROM guild_logs WHERE guildId = %u", m_id);

    CharacterDatabase.Execute("DELETE FROM guild_bank_logs WHERE guildId = %u", m_id);
#if VERSION_STRING >= Cata
    sGuildFinderMgr.deleteGuild(m_id);
#endif

    sGuildMgr.removeGuild(m_id);
}

void Guild::saveGuildToDB()
{
    CharacterDatabase.Execute("UPDATE guilds SET guildLevel = '%u', guildExperience = '%llu', todayExperience = '%llu' WHERE guildId = %u",
        static_cast<uint32_t>(getLevel()), getExperience(), getTodayExperience(), getId());
}

void Guild::updateMemberData(Player* player, uint8_t dataid, uint32_t value)
{
    if (GuildMember* member = getMember(player->getGuid()))
    {
        switch (dataid)
        {
            case GM_DATA_ZONEID:
            {
                member->setZoneId(value);
            } break;
            case GM_DATA_ACHIEVEMENT_POINTS:
            {
                member->setAchievementPoints(value);
            } break;
            case GM_DATA_LEVEL:
            {
                member->setLevel(static_cast<uint8_t>(value));
            } break;
            default:
            {
                LogError("Called with incorrect ID %u (val %u)", static_cast<uint32_t>(dataid), value);
                return;
            }
        }

        handleRoster();
    }
}

void Guild::onPlayerStatusChange(Player* player, uint32_t flag, bool state)
{
    if (GuildMember* member = getMember(player->getGuid()))
    {
        if (state)
            member->addFlag(static_cast<uint8_t>(flag));
        else
            member->removeFlag(static_cast<uint8_t>(flag));
    }
}

void Guild::handleRoster(WorldSession* session)
{
#if VERSION_STRING < Cata
    WorldPacket data(SMSG_GUILD_ROSTER, (4 + m_motd.length() + 1 + m_info.length() + 1 + 4 + _getRanksSize() * (4 + 4 + MAX_GUILD_BANK_TABS * (4 + 4)) + _guildMembersStore.size() * 50));
    data << uint32_t(_guildMembersStore.size());
    data << m_motd;
    data << m_info;

    data << uint32_t(_getRanksSize());
    for (auto ritr = _guildRankInfoStore.begin(); ritr != _guildRankInfoStore.end(); ++ritr)
    {
        data << uint32_t(ritr->getRights());
        if (ritr->getBankMoneyPerDay() == sizeof(uint32_t))
            data << uint32_t(sizeof(uint32_t));
        else
            data << uint32_t(ritr->getBankMoneyPerDay());

        for (uint8_t i = 0; i < MAX_GUILD_BANK_TABS; ++i)
        {
            data << uint32_t(ritr->getBankTabRights(i));
            data << uint32_t(ritr->getBankTabSlotsPerDay(i));
        }
    }

    for (auto itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
    {
        data << uint64_t(itr->second->getGUID())
            << uint8_t(itr->second->getFlags())
            << itr->second->getName()
            << uint32_t(itr->second->getRankId())
            << uint8_t(itr->second->getLevel())
            << uint8_t(itr->second->getClass())
            << uint8_t(0)
            << uint32_t(itr->second->getZoneId());

        if (!itr->second->getFlags())
            data << float(float(::time(nullptr) - itr->second->getLogoutTime()) / DAY);

        data << itr->second->getPublicNote();

        if (_hasRankRight(session->GetPlayer()->getGuid(), GR_RIGHT_VIEWOFFNOTE))
            data << itr->second->getOfficerNote();
        else
            data << "";
    }

    session->SendPacket(&data);
#else
    ByteBuffer memberData(100);
    WorldPacket data(SMSG_GUILD_ROSTER, 100);
    data.writeBits(m_motd.length(), 11);
    data.writeBits(_guildMembersStore.size(), 18);

    for (GuildMembersStore::const_iterator itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
    {
        GuildMember* member = itr->second;
        size_t pubNoteLength = member->getPublicNote().length();
        size_t offNoteLength = member->getOfficerNote().length();

        ObjectGuid guid = member->getGUID();
        data.writeBit(guid[3]);
        data.writeBit(guid[4]);
        data.writeBit(0);
        data.writeBit(0);
        data.writeBits(pubNoteLength, 8);
        data.writeBits(offNoteLength, 8);
        data.writeBit(guid[0]);
        data.writeBits(member->getName().length(), 7);
        data.writeBit(guid[1]);
        data.writeBit(guid[2]);
        data.writeBit(guid[6]);
        data.writeBit(guid[5]);
        data.writeBit(guid[7]);

        memberData << uint8_t(member->getClass());
        memberData << uint32_t(member->getTotalReputation());
        memberData.WriteByteSeq(guid[0]);
        memberData << uint64_t(member->getWeekActivity());
        memberData << uint32_t(member->getRankId());
        memberData << uint32_t(member->getAchievementPoints());

        memberData << uint32_t(0);
        memberData << uint32_t(0);
        memberData << uint32_t(0);
        memberData << uint32_t(0);
        memberData << uint32_t(0);
        memberData << uint32_t(0);

        memberData.WriteByteSeq(guid[2]);
        memberData << uint8_t(member->getFlags());
        memberData << uint32_t(member->getZoneId());
        memberData << uint64_t(member->getTotalActivity());
        memberData.WriteByteSeq(guid[7]);
        memberData << uint32_t(worldConfig.guild.maxRepPerWeek - member->getWeekReputation());

        if (pubNoteLength)
        {
            memberData.WriteString(member->getPublicNote());
        }

        memberData.WriteByteSeq(guid[3]);
        memberData << uint8_t(member->getLevel());
        memberData << int32_t(0);
        memberData.WriteByteSeq(guid[5]);
        memberData.WriteByteSeq(guid[4]);
        memberData << uint8_t(0);
        memberData.WriteByteSeq(guid[1]);
        memberData << float(member->isOnline() ? 0.0f : float(::time(nullptr) - member->getLogoutTime()) / DAY);

        if (offNoteLength)
        {
            memberData.WriteString(member->getOfficerNote());
        }

        memberData.WriteByteSeq(guid[6]);
        memberData.WriteString(member->getName());
    }

    size_t infoLength = m_info.length();
    data.writeBits(infoLength, 12);

    data.flushBits();
    data.append(memberData);

    if (infoLength)
    {
        data.WriteString(m_info);
    }

    data.WriteString(m_motd);
    data << uint32_t(mAccountsNumber);
    data << uint32_t(worldConfig.guild.maxRepPerWeek);
    data.appendPackedTime(m_createdDate);
    data << uint32_t(0);

    if (session)
    {
        LogDebugFlag(LF_OPCODE, "SMSG_GUILD_ROSTER %s", session->GetPlayer()->getName().c_str());
        session->SendPacket(&data);
    }
    else
    {
        LogDebugFlag(LF_OPCODE, "SMSG_GUILD_ROSTER [Broadcast]");
        broadcastPacket(&data);
    }
#endif
}

void Guild::handleQuery(WorldSession* session)
{
    WorldPacket data(SMSG_GUILD_QUERY_RESPONSE, 8 * 32 + 200);

#if VERSION_STRING >= Cata
    data << uint64_t(getGUID());
#else
    data << uint32_t(m_id);
#endif
    data << m_name;

    for (uint8_t i = 0; i < MAX_GUILD_RANKS; ++i)
    {
        if (i < _getRanksSize())
            data << _guildRankInfoStore[i].getName();
        else
            data << uint8_t(0);
    }
#if VERSION_STRING >= Cata
    for (uint8_t i = 0; i < MAX_GUILD_RANKS; ++i)
    {
        if (i < _getRanksSize())
            data << uint32_t(i);
        else
            data << uint32_t(0);
    }

    for (uint8_t i = 0; i < MAX_GUILD_RANKS; ++i)
    {
        if (i < _getRanksSize())
            data << uint32_t(_guildRankInfoStore[i].getId());
        else
            data << uint32_t(0);
    }
#endif

    m_emblemInfo.writeEmblemInfoToPacket(data);
    data << uint32_t(_getRanksSize());

    session->SendPacket(&data);

    //LogDebugFlag(LF_OPCODE, "SMSG_GUILD_QUERY_RESPONSE %s", session->GetPlayer()->getName().c_str());
}

void Guild::sendGuildRankInfo(WorldSession* session) const
{
#if VERSION_STRING >= Cata
    ByteBuffer rankData(100);
    WorldPacket data(SMSG_GUILD_RANK, 100);

    data.writeBits(_getRanksSize(), 18);

    for (uint8_t i = 0; i < _getRanksSize(); ++i)
    {
        GuildRankInfo const* rankInfo = getRankInfo(i);
        if (rankInfo == nullptr)
        {
            continue;
        }

        data.writeBits(rankInfo->getName().length(), 7);

        rankData << uint32_t(rankInfo->getId());

        for (uint8_t j = 0; j < MAX_GUILD_BANK_TABS; ++j)
        {
            rankData << uint32_t(rankInfo->getBankTabSlotsPerDay(j));
            rankData << uint32_t(rankInfo->getBankTabRights(j));
        }

        rankData << uint32_t(rankInfo->getBankMoneyPerDay());
        rankData << uint32_t(rankInfo->getRights());

        if (rankInfo->getName().length())
        {
            rankData.WriteString(rankInfo->getName());
        }

        rankData << uint32_t(i);
    }

    data.flushBits();
    data.append(rankData);
    session->SendPacket(&data);

    LogDebugFlag(LF_OPCODE, "SMSG_GUILD_RANK %s", session->GetPlayer()->getName().c_str());
#endif
}

void Guild::handleSetMOTD(WorldSession* session, std::string const& motd)
{
    if (m_motd == motd)
        return;

    if (!_hasRankRight(session->GetPlayer()->getGuid(), GR_RIGHT_SETMOTD))
    {
        session->SendPacket(SmsgGuildCommandResult(GC_TYPE_EDIT_MOTD, "", GC_ERROR_PERMISSIONS).serialise().get());
    }
    else
    {
        m_motd = motd;
        CharacterDatabase.Execute("UPDATE guilds SET motd = '%s' WHERE guildId = %u", CharacterDatabase.EscapeString(motd).c_str(), m_id);
        broadcastEvent(GE_MOTD, 0, { motd });
    }
}

void Guild::handleSetInfo(WorldSession* session, std::string const& info)
{
    if (m_info == info)
        return;

    if (_hasRankRight(session->GetPlayer()->getGuid(), GR_RIGHT_MODIFY_GUILD_INFO))
    {
        m_info = info;
        CharacterDatabase.Execute("UPDATE guilds SET guildInfo = '%s' WHERE guildId = %u", info.c_str(), m_id);
    }
}

void Guild::handleSetEmblem(WorldSession* session, const EmblemInfo& emblemInfo)
{
    Player* player = session->GetPlayer();
    if (!isLeader(player))
    {
        session->SendPacket(MsgSaveGuildEmblem(GEM_ERROR_NOTGUILDMASTER).serialise().get());
    }
    else if (!player->hasEnoughCoinage(uint64_t(EMBLEM_PRICE)))
    {
        session->SendPacket(MsgSaveGuildEmblem(GEM_ERROR_NOTENOUGHMONEY).serialise().get());
    }
    else
    {
        player->modCoinage(-int64_t(EMBLEM_PRICE));
        m_emblemInfo = emblemInfo;
        m_emblemInfo.saveEmblemInfoToDB(m_id);

        session->SendPacket(MsgSaveGuildEmblem(GEM_ERROR_SUCCESS).serialise().get());
        handleQuery(session);
    }
}

void Guild::handleSetNewGuildMaster(WorldSession* session, std::string const& name)
{
    Player* player = session->GetPlayer();
    if (!isLeader(player))
    {
        session->SendPacket(SmsgGuildCommandResult(GC_TYPE_CHANGE_LEADER, "", GC_ERROR_PERMISSIONS).serialise().get());
    }
    else if (GuildMember* oldGuildMaster = getMember(player->getGuid()))
    {
        if (GuildMember* newGuildMaster = getMember(name))
        {
            setLeaderGuid(newGuildMaster);
            oldGuildMaster->changeRank(GR_INITIATE);
            broadcastEvent(GE_LEADER_CHANGED, 0, { session->GetPlayer()->getName(), name });
        }
    }
}

void Guild::handleSetBankTabInfo(WorldSession* session, uint8_t tabId, std::string const& name, std::string const& icon)
{
    GuildBankTab* tab = getBankTab(tabId);
    if (tab == nullptr)
    {
        LOG_ERROR("Player %s trying to change bank tab info from unexisting tab %d.", session->GetPlayer()->getName().c_str(), tabId);
        return;
    }

    tab->setInfo(name, icon);
    broadcastEvent(GE_BANK_TAB_UPDATED, 0, { std::to_string(tabId), name, icon });
}

void Guild::handleSetMemberNote(WorldSession* session, std::string const& note, uint64_t guid, bool isPublic)
{
    if (!_hasRankRight(session->GetPlayer()->getGuid(), isPublic ? GR_RIGHT_EPNOTE : GR_RIGHT_EOFFNOTE))
    {
        session->SendPacket(SmsgGuildCommandResult(GC_TYPE_PUBLIC_NOTE, "", GC_ERROR_PERMISSIONS).serialise().get());
    }
    else if (GuildMember* member = getMember(guid))
    {
        if (isPublic)
            member->setPublicNote(note);
        else
            member->setOfficerNote(note);

        handleRoster(session);
    }
}

void Guild::handleSetRankInfo(WorldSession* session, uint8_t rankId, std::string const& name, uint32_t rights, uint32_t moneyPerDay, GuildBankRightsAndSlotsVec rightsAndSlots)
{
    if (!isLeader(session->GetPlayer()))
    {
        session->SendPacket(SmsgGuildCommandResult(GC_TYPE_CHANGE_RANK, "", GC_ERROR_PERMISSIONS).serialise().get());
    }
    else if (GuildRankInfo* rankInfo = getRankInfo(rankId))
    {
        LogDebug("Changed RankName to '%s', rights to 0x%08X", name.c_str(), rights);

        rankInfo->setName(name);
        rankInfo->setRights(rights);
        setRankBankMoneyPerDay(rankId, moneyPerDay);

        for (GuildBankRightsAndSlotsVec::const_iterator itr = rightsAndSlots.begin(); itr != rightsAndSlots.end(); ++itr)
            setRankBankTabRightsAndSlots(rankId, *itr);

        broadcastEvent(GE_RANK_UPDATED, 0, { std::to_string(rankId), name });
    }
}

void Guild::handleBuyBankTab(WorldSession* session, uint8_t tabId)
{
    Player* player = session->GetPlayer();
    if (player == nullptr)
        return;

    GuildMember const* member = getMember(player->getGuid());
    if (member == nullptr)
        return;

    if (_getPurchasedTabsSize() >= MAX_GUILD_BANK_TABS)
        return;

    if (tabId != _getPurchasedTabsSize())
        return;

    if (tabId < MAX_GUILD_BANK_TABS - 2)
    {
        uint32_t tabCost = _GetGuildBankTabPrice(tabId) * GOLD;
        if (tabCost == 0)
            return;

        if (!player->hasEnoughCoinage(tabCost))
            return;

        player->modCoinage(-int64_t(tabCost));
    }

    createNewBankTab();
    broadcastEvent(GE_BANK_TAB_PURCHASED, 0, {});
    sendPermissions(session);
}

void Guild::handleAcceptMember(WorldSession* session)
{
    Player* player = session->GetPlayer();
    Player* leader = objmgr.GetPlayer(WoWGuid::getGuidLowPartFromUInt64(getLeaderGUID()));
    if (worldConfig.player.isInterfactionGuildEnabled == false && player->getTeam() != leader->getTeam())
        return;

    addMember(player->getGuid());
}

void Guild::handleLeaveMember(WorldSession* session)
{
    Player* player = session->GetPlayer();
    if (isLeader(player))
    {
        if (_guildMembersStore.size() > 1)
            session->SendPacket(SmsgGuildCommandResult(GC_TYPE_QUIT, "", GC_ERROR_LEADER_LEAVE).serialise().get());
#if VERSION_STRING >= Cata
        else if (getLevel() >= worldConfig.guild.undeletableLevel)
            session->SendPacket(SmsgGuildCommandResult(GC_TYPE_QUIT, "", GC_ERROR_UNDELETABLE_DUE_TO_LEVEL).serialise().get());
#endif
        else
            disband();
    }
    else
    {
        deleteMember(player->getGuid(), false, false);

        logEvent(GE_LOG_LEAVE_GUILD, player->getGuidLow());
        broadcastEvent(GE_LEFT, player->getGuid(), { player->getName() });

        session->SendPacket(SmsgGuildCommandResult(GC_TYPE_QUIT, m_name, GC_ERROR_SUCCESS).serialise().get());
    }
}

void Guild::handleRemoveMember(WorldSession* session, uint64_t guid)
{
    Player* player = session->GetPlayer();

    if (!_hasRankRight(player->getGuid(), GR_RIGHT_REMOVE))
    {
        session->SendPacket(SmsgGuildCommandResult(GC_TYPE_REMOVE, "", GC_ERROR_PERMISSIONS).serialise().get());
    }
    else if (GuildMember* member = getMember(guid))
    {
        std::string name = member->getName();

        if (member->isRank(GR_GUILDMASTER))
        {
            session->SendPacket(SmsgGuildCommandResult(GC_TYPE_REMOVE, "", GC_ERROR_LEADER_LEAVE).serialise().get());
        }
        else
        {
            GuildMember const* memberMe = getMember(player->getGuid());
            if (memberMe == nullptr || member->isRankNotLower(memberMe->getRankId()))
            {
                session->SendPacket(SmsgGuildCommandResult(GC_TYPE_REMOVE, name, GC_ERROR_RANK_TOO_HIGH_S).serialise().get());
            }
            else
            {
                deleteMember(guid, false, true);
                logEvent(GE_LOG_UNINVITE_PLAYER, player->getGuidLow(), WoWGuid::getGuidLowPartFromUInt64(guid));
                broadcastEvent(GE_REMOVED, 0, { name, player->getName() });
                session->SendPacket(SmsgGuildCommandResult(GC_TYPE_REMOVE, name, GC_ERROR_SUCCESS).serialise().get());
            }
        }
    }
}

void Guild::handleUpdateMemberRank(WorldSession* session, uint64_t guid, bool demote)
{
    Player* player = session->GetPlayer();
    const GuildCommandType type = demote ? GC_TYPE_DEMOTE : GC_TYPE_PROMOTE;

    if (!_hasRankRight(player->getGuid(), demote ? GR_RIGHT_DEMOTE : GR_RIGHT_PROMOTE))
    {
        session->SendPacket(SmsgGuildCommandResult(type, "", GC_ERROR_PERMISSIONS).serialise().get());
    }
    else if (GuildMember* member = getMember(guid))
    {
        std::string name = member->getName();

        if (member->isSamePlayer(player->getGuid()))
        {
            session->SendPacket(SmsgGuildCommandResult(type, "", GC_ERROR_NAME_INVALID).serialise().get());
            return;
        }

        GuildMember const* memberMe = getMember(player->getGuid());
        uint8_t rankId = memberMe->getRankId();
        if (demote)
        {
            if (member->isRankNotLower(rankId))
            {
                session->SendPacket(SmsgGuildCommandResult(type, name, GC_ERROR_RANK_TOO_HIGH_S).serialise().get());
                return;
            }

            if (member->getRankId() >= _getLowestRankId())
            {
                session->SendPacket(SmsgGuildCommandResult(type, name, GC_ERROR_RANK_TOO_LOW_S).serialise().get());
                return;
            }
        }
        else
        {
            if (member->isRankNotLower(rankId + 1))
            {
                session->SendPacket(SmsgGuildCommandResult(type, name, GC_ERROR_RANK_TOO_HIGH_S).serialise().get());
                return;
            }
        }

        const uint32_t newRankId = member->getRankId() + (demote ? 1 : -1);
        member->changeRank(static_cast<uint8_t>(newRankId));

        logEvent(demote ? GE_LOG_DEMOTE_PLAYER : GE_LOG_PROMOTE_PLAYER, player->getGuidLow(), WoWGuid::getGuidLowPartFromUInt64(member->getGUID()), static_cast<uint8_t>(newRankId));
        broadcastEvent(demote ? GE_DEMOTION : GE_PROMOTION, 0, { player->getName(), name, getRankName(static_cast<uint8_t>(newRankId)) });
    }
}

void Guild::handleSetMemberRank(WorldSession* session, uint64_t targetGuid, uint64_t setterGuid, uint32_t rank)
{
    Player* player = session->GetPlayer();
    GuildMember* member = getMember(targetGuid);
    GuildRankRights rights = GR_RIGHT_PROMOTE;
    GuildCommandType type = GC_TYPE_PROMOTE;

    if (rank > member->getRankId())
    {
        rights = GR_RIGHT_DEMOTE;
        type = GC_TYPE_DEMOTE;
    }

    if (!_hasRankRight(player->getGuid(), rights))
    {
        session->SendPacket(SmsgGuildCommandResult(type, "", GC_ERROR_PERMISSIONS).serialise().get());
        return;
    }

    if (member->isSamePlayer(player->getGuid()))
    {
        session->SendPacket(SmsgGuildCommandResult(type, "", GC_ERROR_NAME_INVALID).serialise().get());
        return;
    }

    sendGuildRanksUpdate(setterGuid, targetGuid, rank);
}

void Guild::handleAddNewRank(WorldSession* session, std::string const& name)
{
    const uint8_t size = _getRanksSize();
    if (size >= MAX_GUILD_RANKS)
        return;

    if (isLeader(session->GetPlayer()))
    {
        if (createRank(name, GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK))
        {
#if VERSION_STRING >= Cata
            broadcastEvent(GE_RANK_CREATED, 0, {});
#else
            broadcastEvent(GE_RANK_UPDATED, 0, {});
#endif
        }
    }
}

void Guild::handleRemoveLowestRank(WorldSession* session)
{
    handleRemoveRank(session, _getLowestRankId());
}

void Guild::handleRemoveRank(WorldSession* session, uint8_t rankId)
{
    if (_getRanksSize() <= MIN_GUILD_RANKS || rankId >= _getRanksSize() || !isLeader(session->GetPlayer()))
        return;

    CharacterDatabase.Execute("DELETE FROM guild_bank_rights WHERE rankId = %u AND guildId = %u", rankId, m_id);

    CharacterDatabase.Execute("DELETE FROM guild_ranks WHERE rankId = %u AND guildId = %u", rankId, m_id);

    _guildRankInfoStore.erase(_guildRankInfoStore.begin() + rankId);

    broadcastEvent(GE_RANK_DELETED, rankId, {});
}

void Guild::handleMemberDepositMoney(WorldSession* session, uint64_t amount, bool cashFlow)
{
    Player* player = session->GetPlayer();

    modifyBankMoney(amount, true);
    if (!cashFlow)
        player->modCoinage(-int32_t(amount));

    logBankEvent(cashFlow ? GB_LOG_CASH_FLOW_DEPOSIT : GB_LOG_DEPOSIT_MONEY, uint8_t(0), player->getGuidLow(), static_cast<uint32_t>(amount));

    std::string aux = Util::ByteArrayToHexString(reinterpret_cast<uint8_t*>(&amount), 8, true);
    broadcastEvent(GE_BANK_TAB_AND_MONEY_UPDATED, 0, { aux });
}

bool Guild::handleMemberWithdrawMoney(WorldSession* session, uint64_t amount, bool repair)
{
    if (m_bankMoney < amount)
        return false;

    Player* player = session->GetPlayer();

    GuildMember* member = getMember(player->getGuid());
    if (member == nullptr)
        return false;

    if (uint64_t(getMemberRemainingMoney(member)) < amount)
        return false;

    if (!repair)
        player->modCoinage(int32_t(amount));

    member->updateBankWithdrawValue(MAX_GUILD_BANK_TABS, static_cast<uint32_t>(amount));
    modifyBankMoney(amount, false);

    logBankEvent(repair ? GB_LOG_REPAIR_MONEY : GB_LOG_WITHDRAW_MONEY, uint8_t(0), player->getGuidLow(), static_cast<uint32_t>(amount));

    std::string aux = Util::ByteArrayToHexString(reinterpret_cast<uint8_t*>(&amount), 8, true);
    broadcastEvent(GE_BANK_TAB_AND_MONEY_UPDATED, 0, { aux });

    return true;
}

void Guild::handleMemberLogout(WorldSession* session)
{
    Player* player = session->GetPlayer();
    if (GuildMember* member = getMember(player->getGuid()))
    {
        member->setStats(player);
        member->updateLogoutTime();
        member->resetFlags();
    }
    broadcastEvent(GE_SIGNED_OFF, player->getGuid(), { player->getName() });

    saveGuildToDB();
}

void Guild::handleDisband(WorldSession* session)
{
    if (isLeader(session->GetPlayer()))
    {
        disband();
        LogDebug("Successfully Disbanded");
    }
}

void Guild::handleGuildPartyRequest(WorldSession* session)
{
    Player* player = session->GetPlayer();
    Group* group = player->GetGroup();

    if (!isMember(player->getGuid()) || !group)
        return;

    LogDebugFlag(LF_OPCODE, "SMSG_GUILD_PARTY_STATE_RESPONSE %s", session->GetPlayer()->getName().c_str());
}

void Guild::sendEventLog(WorldSession* session) const
{
#if VERSION_STRING >= Cata
    WorldPacket data(SMSG_GUILD_EVENT_LOG_QUERY_RESULT, 1 + mEventLog->getSize() * (1 + 8 + 4));
#else
    WorldPacket data(MSG_GUILD_EVENT_LOG_QUERY, 1 + mEventLog->getSize() * (1 + 8 + 4));
#endif
    mEventLog->writeLogHolderPacket(data);
    session->SendPacket(&data);

    LogDebugFlag(LF_OPCODE, "SMSG_GUILD_EVENT_LOG_QUERY_RESULT %s", session->GetPlayer()->getName().c_str());
}

void Guild::sendNewsUpdate(WorldSession* session)
{
#if VERSION_STRING >= Cata
    uint32_t size = mNewsLog->getSize();
    GuildLog* logs = mNewsLog->getGuildLog();

    if (logs == nullptr)
    {
        return;
    }

    WorldPacket data(SMSG_GUILD_NEWS_UPDATE, (21 + size * (26 + 8)) / 8 + (8 + 6 * 4) * size);
    data.writeBits(size, 21);

    for (GuildLog::const_iterator itr = logs->begin(); itr != logs->end(); ++itr)
    {
        data.writeBits(0, 26);
        ObjectGuid guid = static_cast<GuildNewsLogEntry*>(*itr)->getPlayerGuid();

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
        GuildNewsLogEntry* news = static_cast<GuildNewsLogEntry*>(*itr);
        ObjectGuid guid = news->getPlayerGuid();
        data.WriteByteSeq(guid[5]);

        data << uint32_t(news->getFlags());
        data << uint32_t(news->getValue());
        data << uint32_t(0);

        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[3]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[1]);

        data << uint32_t(news->getGUID());
        data << uint32_t(news->getType());
        data.appendPackedTime(news->getTimestamp());
    }

    session->SendPacket(&data);

    LogDebugFlag(LF_OPCODE, "SMSG_GUILD_NEWS_UPDATE %s", session->GetPlayer()->getName().c_str());
#endif
}

void Guild::sendBankLog(WorldSession* session, uint8_t tabId) const
{
    if (tabId < _getPurchasedTabsSize() || tabId == MAX_GUILD_BANK_TABS)
    {
        GuildLogHolder const* log = mBankEventLog[tabId];

#if VERSION_STRING >= Cata
        WorldPacket data(SMSG_GUILD_BANK_LOG_QUERY_RESULT, log->getSize() * (4 * 4 + 1) + 1 + 1);
        data.writeBit(getLevel() >= 5 && tabId == MAX_GUILD_BANK_TABS);
        log->writeLogHolderPacket(data);
        data << uint32_t(tabId);
#else
        WorldPacket data(MSG_GUILD_BANK_LOG_QUERY, log->getSize() * (4 * 4 + 1) + 1 + 1);
        data << uint8_t(tabId);
        log->writeLogHolderPacket(data);
#endif
        session->SendPacket(&data);

        LogDebugFlag(LF_OPCODE, "SMSG_GUILD_BANK_LOG_QUERY_RESULT %s TabId: %u", session->GetPlayer()->getName().c_str(), tabId);
    }
}

void Guild::sendBankTabText(WorldSession* session, uint8_t tabId) const
{
    if (GuildBankTab const* tab = getBankTab(tabId))
        tab->sendText(this, session);
}

void Guild::sendPermissions(WorldSession* session) const
{
    GuildMember const* member = getMember(session->GetPlayer()->getGuid());
    if (member == nullptr)
        return;

    uint8_t rankId = member->getRankId();

#if VERSION_STRING >= Cata
    WorldPacket data(SMSG_GUILD_PERMISSIONS_QUERY_RESULTS, 4 * 15 + 1);
    data << uint32_t(rankId);
    data << uint32_t(_getPurchasedTabsSize());
    data << uint32_t(getRankRights(rankId));
    data << uint32_t(getMemberRemainingMoney(member));
    data.writeBits(MAX_GUILD_BANK_TABS, 23);
#else
    WorldPacket data(MSG_GUILD_PERMISSIONS, 4 * 15 + 1);
    data << uint32_t(rankId);
    data << uint32_t(getRankRights(rankId));
    data << uint32_t(getMemberRemainingMoney(member));
    data << uint8_t(_getPurchasedTabsSize());
#endif

    for (uint8_t tabId = 0; tabId < MAX_GUILD_BANK_TABS; ++tabId)
    {
        data << uint32_t(getRankBankTabRights(rankId, tabId));
        data << uint32_t(getMemberRemainingSlots(member, tabId));
    }

    session->SendPacket(&data);

    LogDebugFlag(LF_OPCODE, "SMSG_GUILD_PERMISSIONS_QUERY_RESULTS %s Rank: %u", session->GetPlayer()->getName().c_str(), rankId);
}

void Guild::sendMoneyInfo(WorldSession* session) const
{
    GuildMember const* member = getMember(session->GetPlayer()->getGuid());
    if (member == nullptr)
        return;

    const int32_t amount = getMemberRemainingMoney(member);

#if VERSION_STRING >= Cata
    session->SendPacket(SmsgGuildBankMoneyWithdrawn(amount).serialise().get());
#else
    session->SendPacket(MsgGuildBankMoneyWithdrawn(amount).serialise().get());
#endif
}

void Guild::sendLoginInfo(WorldSession* session)
{
    session->SendPacket(SmsgGuildEvent(GE_MOTD, { m_motd }, 0).serialise().get());

    LogDebugFlag(LF_OPCODE, "SMSG_GUILD_EVENT %s MOTD", session->GetPlayer()->getName().c_str());

    Player* player = session->GetPlayer();

#if VERSION_STRING < Cata
    sendBankList(session, 0, false, true);

    handleRoster(session);
    broadcastEvent(GE_SIGNED_ON, player->getGuid(), { player->getName() });

    if (GuildMember* member = getMember(player->getGuid()))
    {
        member->setStats(player);
        member->addFlag(GEM_STATUS_ONLINE);
    }
#else
    sendGuildRankInfo(session);
    broadcastEvent(GE_SIGNED_ON, player->getGuid(), { player->getName() });

    WorldPacket data;
    data.Initialize(SMSG_GUILD_MEMBER_DAILY_RESET, 0);
    session->SendPacket(&data);

    if (worldConfig.guild.levelingEnabled == false)
        return;

    for (uint32_t i = 0; i < sGuildPerkSpellsStore.GetNumRows(); ++i)
    {
        if (DBC::Structures::GuildPerkSpellsEntry const* entry = sGuildPerkSpellsStore.LookupEntry(i))
        {
            if (entry->Level <= getLevel())
                player->addSpell(entry->SpellId);
        }
    }

    if (GuildMember* member = getMember(player->getGuid()))
    {
        sendGuildReputationWeeklyCap(session, member->getWeekReputation());

        member->setStats(player);
        member->addFlag(GEM_STATUS_ONLINE);
    }
#endif
}

bool Guild::loadGuildFromDB(Field* fields)
{
    m_id = fields[0].GetUInt32();
    m_name = fields[1].GetString();
    m_leaderGuid = MAKE_NEW_GUID(fields[2].GetUInt32(), 0, HIGHGUID_TYPE_PLAYER);

    m_emblemInfo.loadEmblemInfoFromDB(fields);

    m_info = fields[8].GetString();
    m_motd = fields[9].GetString();
    m_createdDate = time_t(fields[10].GetUInt32());
    m_bankMoney = fields[11].GetUInt64();
#if VERSION_STRING >= Cata
    m_level = static_cast<uint8_t>(fields[12].GetUInt32());
    m_experience = fields[13].GetUInt64();
    m_todayExperience = fields[14].GetUInt64();

    uint8_t purchasedTabs = uint8_t(fields[15].GetUInt64());
#else
    uint8_t purchasedTabs = uint8_t(fields[12].GetUInt64());
#endif
    if (purchasedTabs > MAX_GUILD_BANK_TABS)
    {
        purchasedTabs = MAX_GUILD_BANK_TABS;
    }

    _guildBankTabsStore.resize(purchasedTabs);
    for (uint8_t i = 0; i < purchasedTabs; ++i)
    {
        _guildBankTabsStore[i] = new GuildBankTab(m_id, i);
    }

    createLogHolders();

    return true;
}

void Guild::loadRankFromDB(Field* fields)
{
    GuildRankInfo rankInfo(m_id);

    rankInfo.loadGuildRankFromDB(fields);

    _guildRankInfoStore.push_back(rankInfo);
}

bool Guild::loadMemberFromDB(Field* fields, Field* fields2)
{
    uint32_t lowguid = fields[1].GetUInt32();
    auto member = new GuildMember(m_id, MAKE_NEW_GUID(lowguid, 0, HIGHGUID_TYPE_PLAYER), fields[2].GetUInt8());
    if (!member->loadGuildMembersFromDB(fields, fields2))
    {
        CharacterDatabase.Execute("DELETE FROM guild_members WHERE guildId = %u", lowguid);
        delete member;
        return false;
    }

    _guildMembersStore[lowguid] = member;

    return true;
}

void Guild::loadBankRightFromDB(Field* fields)
{
    GuildBankRightsAndSlots rightsAndSlots(fields[1].GetUInt8(), fields[3].GetUInt8(), fields[4].GetUInt32());
    setRankBankTabRightsAndSlots(fields[2].GetUInt8(), rightsAndSlots, false);
}

bool Guild::loadEventLogFromDB(Field* fields)
{
    if (mEventLog->canInsert())
    {
        mEventLog->loadEvent(new GuildEventLogEntry(m_id, fields[1].GetUInt32(), time_t(fields[6].GetUInt32()),
                                                    GuildEventLogTypes(fields[2].GetUInt8()), fields[3].GetUInt32(),
                                                    fields[4].GetUInt32(), fields[5].GetUInt8()));
        return true;
    }

    return false;
}

bool Guild::loadBankEventLogFromDB(Field* fields)
{
    uint8_t dbTabId = fields[1].GetUInt8();
    bool isMoneyTab = dbTabId == GUILD_BANK_MONEY_TAB;
    if (dbTabId < _getPurchasedTabsSize() || isMoneyTab)
    {
        uint8_t tabId = isMoneyTab ? uint8_t(MAX_GUILD_BANK_TABS) : dbTabId;
        GuildLogHolder* pLog = mBankEventLog[tabId];
        if (pLog->canInsert())
        {
            uint32_t guid = fields[2].GetUInt32();
            const auto eventType = GuildBankEventLogTypes(fields[3].GetUInt8());
            if (GuildBankEventLogEntry::isMoneyEvent(eventType))
            {
                if (!isMoneyTab)
                {
                    LogError("GuildBankEventLog ERROR: MoneyEvent(LogGuid: %u, Guild: %u) does not belong to money tab (%u), ignoring...", guid, m_id, dbTabId);
                    return false;
                }
            }
            else if (isMoneyTab)
            {
                LogError("GuildBankEventLog ERROR: non-money event (LogGuid: %u, Guild: %u) belongs to money tab, ignoring...", guid, m_id);
                return false;
            }

            pLog->loadEvent(new GuildBankEventLogEntry(m_id, guid, time_t(fields[8].GetUInt32()), dbTabId, eventType, fields[4].GetUInt32(),
                                                        fields[5].GetUInt32(), fields[6].GetUInt16(), fields[7].GetUInt8()));
        }
    }

    return true;
}

void Guild::loadGuildNewsLogFromDB(Field* fields)
{
    if (!mNewsLog->canInsert())
        return;

    mNewsLog->loadEvent(new GuildNewsLogEntry(m_id, fields[1].GetUInt32(), fields[6].GetUInt32(), GuildNews(fields[2].GetUInt8()),
                                                fields[3].GetUInt32(), fields[4].GetUInt32(), fields[5].GetUInt32())); 
}

void Guild::loadBankTabFromDB(Field* fields)
{
    uint8_t tabId = fields[1].GetUInt8();
    if (tabId >= _getPurchasedTabsSize())
        LogError("Invalid tab (tabId: %u) in guild bank, skipped.", tabId);
    else
        _guildBankTabsStore[tabId]->loadGuildBankTabFromDB(fields);
}

bool Guild::loadBankItemFromDB(Field* fields)
{
    uint8_t tabId = fields[1].GetUInt8();
    if (tabId >= _getPurchasedTabsSize())
    {
        LogError("Invalid tab for item (GUID: %u) in guild bank, skipped.", fields[3].GetUInt32());
        return false;
    }

    return _guildBankTabsStore[tabId]->loadGuildBankTabItemFromDB(fields);
}

bool Guild::validate()
{
    bool broken_ranks = false;
    uint8_t ranks = _getRanksSize();

    if (ranks < MIN_GUILD_RANKS || ranks > MAX_GUILD_RANKS)
    {
        LogError("Guild %u has invalid number of ranks, creating...", m_id);
        broken_ranks = true;
    }
    else
    {
        for (uint8_t rankId = 0; rankId < ranks; ++rankId)
        {
            GuildRankInfo* rankInfo = getRankInfo(rankId);
            if (rankInfo->getId() != rankId)
            {
                LogError("Guild %u has invalid rank id %u, creating default set of ranks...", m_id, rankId);
                broken_ranks = true;
            }
            else
            {
                rankInfo->createMissingTabsIfNeeded(_getPurchasedTabsSize(), false, true);
            }
        }
    }

    if (broken_ranks)
    {
        _guildRankInfoStore.clear();
        createDefaultGuildRanks();
    }

    for (auto itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
    {
        if (itr->second->getRankId() > _getRanksSize())
            itr->second->changeRank(_getLowestRankId());
    }

    GuildMember* pLeader = getMember(m_leaderGuid);
    if (pLeader == nullptr)
    {
        deleteMember(m_leaderGuid);
        if (_guildMembersStore.empty())
        {
            disband();
            return false;
        }
    }
    else if (!pLeader->isRank(GR_GUILDMASTER))
    {
        setLeaderGuid(pLeader);
    }

    for (auto itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
    {
        if (itr->second->getRankId() == GR_GUILDMASTER && !itr->second->isSamePlayer(m_leaderGuid))
            itr->second->changeRank(GR_OFFICER);
    }

    updateAccountsNumber();

    return true;
}

void Guild::broadcastToGuild(WorldSession* session, bool officerOnly, std::string const& msg, uint32_t language) const
{
    if (session && session->GetPlayer() && _hasRankRight(session->GetPlayer()->getGuid(), officerOnly ? GR_RIGHT_OFFCHATSPEAK : GR_RIGHT_GCHATSPEAK))
    {
        WorldPacket* data = sChatHandler.FillMessageData(officerOnly ? CHAT_MSG_OFFICER : CHAT_MSG_GUILD, language, msg.c_str(), NULL, 0);

        for (auto itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
        {
            if (Player* player = itr->second->getPlayerByGuid(session->GetPlayer()->getGuid()))
            {
                if (player->GetSession() && _hasRankRight(player->getGuid(), officerOnly ? GR_RIGHT_OFFCHATLISTEN : GR_RIGHT_GCHATLISTEN) &&
                    !player->Social_IsIgnoring(session->GetPlayer()->getGuidLow()))
                {
                    player->GetSession()->SendPacket(data);
                }
            }
        }
    }
}

void Guild::broadcastAddonToGuild(WorldSession* session, bool officerOnly, std::string const& msg, std::string const& /*prefix*/) const
{
    if (session && session->GetPlayer() && _hasRankRight(session->GetPlayer()->getGuid(), officerOnly ? GR_RIGHT_OFFCHATSPEAK : GR_RIGHT_GCHATSPEAK))
    {
        WorldPacket* data = sChatHandler.FillMessageData(officerOnly ? CHAT_MSG_OFFICER : CHAT_MSG_GUILD, uint32_t(CHAT_MSG_ADDON), msg.c_str(), NULL, 0);

        for (auto itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
        {
            if (Player* player = itr->second->getPlayerByGuid(session->GetPlayer()->getGuid()))
            {
                if (player->GetSession() && _hasRankRight(player->getGuid(), officerOnly ? GR_RIGHT_OFFCHATLISTEN : GR_RIGHT_GCHATLISTEN) &&
                    !player->Social_IsIgnoring(session->GetPlayer()->getGuidLow()))
                {
                    player->GetSession()->SendPacket(data);
                }
            }
        }
    }
}

void Guild::broadcastPacketToRank(WorldPacket* packet, uint8_t rankId) const
{
    for (auto itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
    {
        if (itr->second->isRank(rankId))
        {
            if (Player* player = itr->second->getPlayerByGuid(itr->second->getGUID()))
            {
                player->GetSession()->SendPacket(packet);
            }
        }
    }
}

void Guild::broadcastPacket(WorldPacket* packet) const
{
    for (auto itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
    {
        if (Player* player = itr->second->getPlayerByGuid(itr->second->getGUID()))
        {
            player->GetSession()->SendPacket(packet);
        }
    }
}

void Guild::massInviteToEvent(WorldSession* session, uint32_t minLevel, uint32_t maxLevel, uint32_t minRank)
{
#if VERSION_STRING > TBC
    uint32_t count = 0;

    WorldPacket data(SMSG_CALENDAR_FILTER_GUILD);
    data << uint32_t(count);

    for (auto itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
    {
        GuildMember* member = itr->second;
        uint32_t level = member->getLevel();

        if (member->getGUID() != session->GetPlayer()->getGuid() && level >= minLevel && level <= maxLevel && member->isRankNotLower(static_cast<uint8_t>(minRank)))
        {
            data.appendPackGUID(member->getGUID());
            data << uint8_t(0);
            ++count;
        }
    }

    data.put<uint32_t>(0, count);

    session->SendPacket(&data);
#endif
}

bool Guild::addMember(uint64_t guid, uint8_t rankId)
{
    WoWGuid wGuid;
    wGuid.Init(guid);

    Player* player = objmgr.GetPlayer(wGuid.getGuidLowPart());
    if (player)
    {
        if (player->getGuildId() != 0)
            return false;
    }
    else if (Player::GetGuildIdFromDB(guid) != 0)
    {
        return false;
    }

    uint32_t lowguid = wGuid.getGuidLow();

    if (rankId == GUILD_RANK_NONE)
        rankId = _getLowestRankId();

    auto member = new GuildMember(m_id, guid, rankId);
    std::string name;
    if (player)
    {
        _guildMembersStore[lowguid] = member;
        player->setGuildId(m_id);
        player->SetGuildIdInvited(0);
        player->setGuildRank(rankId);
#if VERSION_STRING >= Cata
        player->setGuildLevel(getLevel());
#endif
        sendLoginInfo(player->GetSession());
        name = player->getName().c_str();
    }
    else
    {
        member->resetFlags();

        bool ok = false;
        if (objmgr.GetPlayerInfo(lowguid))
        {
            PlayerInfo* info = objmgr.GetPlayerInfo(lowguid);
            name = info->name;
            member->setStats(name, static_cast<uint8_t>(info->lastLevel), info->cl, info->lastZone, info->acct, 0);

            ok = member->checkStats();
        }

        if (!ok)
        {
            delete member;
            return false;
        }

        _guildMembersStore[lowguid] = member;
    }

    member->saveGuildMembersToDB(false);

    CharacterDatabase.Execute("INSERT INTO guild_members_withdraw VALUES(%u, 0, 0, 0, 0, 0, 0, 0, 0, 0)", m_id);

    updateAccountsNumber();

    logEvent(GE_LOG_JOIN_GUILD, lowguid);
    broadcastEvent(GE_JOINED, guid, { name });
#if VERSION_STRING >= Cata
    sGuildFinderMgr.removeAllMembershipRequestsFromPlayer(lowguid);
#endif
    sHookInterface.OnGuildJoin(player, this);

    return true;
}

void Guild::deleteMember(uint64_t guid, bool isDisbanding, bool /*isKicked*/)
{
    uint32_t lowguid = WoWGuid::getGuidLowPartFromUInt64(guid);
    Player* player = objmgr.GetPlayer(lowguid);

    if (m_leaderGuid == guid && !isDisbanding)
    {
        GuildMember* oldLeader = nullptr;
        GuildMember* newLeader = nullptr;
        for (auto i = _guildMembersStore.begin(); i != _guildMembersStore.end(); ++i)
        {
            if (i->first == lowguid)
                oldLeader = i->second;
            else if (newLeader == nullptr || newLeader->getRankId() > i->second->getRankId())
                newLeader = i->second;
        }

        if (newLeader == nullptr)
        {
            disband();
            return;
        }

        setLeaderGuid(newLeader);

        if (Player* newLeaderPlayer = newLeader->getPlayerByGuid(newLeader->getGUID()))
            newLeaderPlayer->setGuildRank(GR_GUILDMASTER);

        if (oldLeader)
        {
            broadcastEvent(GE_LEADER_CHANGED, 0, { oldLeader->getName(), newLeader->getName() });
            broadcastEvent(GE_LEFT, guid, { oldLeader->getName() });
        }
    }

    if (GuildMember* member = getMember(guid))
        delete member;

    _guildMembersStore.erase(lowguid);

    if (player)
    {
        player->setGuildId(0);
        player->setGuildRank(0);
#if VERSION_STRING >= Cata
        player->setGuildLevel(0);

        for (uint32_t i = 0; i < sGuildPerkSpellsStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::GuildPerkSpellsEntry const* entry = sGuildPerkSpellsStore.LookupEntry(i))
            {
                if (entry->Level <= getLevel())
                {
                    player->removeSpell(entry->SpellId, false, false, 0);
                }
            }
        }
#endif
    }

    CharacterDatabase.Execute("DELETE FROM guild_members WHERE guildId = %u", lowguid);

    if (!isDisbanding)
        updateAccountsNumber();
}

bool Guild::changeMemberRank(uint64_t guid, uint8_t newRank)
{
    if (newRank <= _getLowestRankId())
    {
        if (GuildMember* member = getMember(guid))
        {
            member->changeRank(newRank);
            return true;
        }
    }

    return false;
}

bool Guild::isMember(uint64_t guid) const
{
    auto itr = _guildMembersStore.find(WoWGuid::getGuidLowPartFromUInt64(guid));
    return itr != _guildMembersStore.end();
}

void Guild::setBankTabText(uint8_t tabId, std::string const& text)
{
    if (GuildBankTab* pTab = getBankTab(tabId))
    {
        pTab->setText(text);
        pTab->sendText(this, nullptr);
    }
}

void Guild::createLogHolders()
{
    mEventLog = new GuildLogHolder(m_id, worldConfig.guild.eventLogCount);
    mNewsLog = new GuildLogHolder(m_id, worldConfig.guild.newsLogCount);

    for (uint8_t tabId = 0; tabId <= MAX_GUILD_BANK_TABS; ++tabId)
        mBankEventLog[tabId] = new GuildLogHolder(m_id, worldConfig.guild.bankLogCount);
}

void Guild::createNewBankTab()
{
    uint8_t tabId = _getPurchasedTabsSize();
    _guildBankTabsStore.push_back(new GuildBankTab(m_id, tabId));

    CharacterDatabase.Execute("DELETE FROM guild_bank_tabs WHERE guildId = %u AND tabId = %u", m_id, static_cast<uint32_t>(tabId));

    CharacterDatabase.Execute("INSERT INTO guild_bank_tabs VALUES(%u, %u, '', '', '')", m_id, static_cast<uint32_t>(tabId));

    ++tabId;
    for (auto itr = _guildRankInfoStore.begin(); itr != _guildRankInfoStore.end(); ++itr)
        (*itr).createMissingTabsIfNeeded(tabId, false, false);
}

void Guild::createDefaultGuildRanks()
{
    CharacterDatabase.Execute("DELETE FROM guild_ranks WHERE guildId = %u", m_id);
    CharacterDatabase.Execute("DELETE FROM guild_bank_rights WHERE guildId = %u", m_id);

    createRank("GuildMaster", GR_RIGHT_ALL);
    createRank("Officer", GR_RIGHT_ALL);
    createRank("Veteran", GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
    createRank("Member", GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
    createRank("Initiante", GR_RIGHT_GCHATLISTEN | GR_RIGHT_GCHATSPEAK);
}

bool Guild::createRank(std::string const& name, uint32_t rights)
{
    uint8_t newRankId = _getRanksSize();
    if (newRankId >= MAX_GUILD_RANKS)
        return false;

    GuildRankInfo info(m_id, newRankId, name, rights, 0);
    _guildRankInfoStore.push_back(info);

    info.createMissingTabsIfNeeded(_getPurchasedTabsSize(), false, false);
    info.saveGuildRankToDB(false);

    return true;
}

void Guild::updateAccountsNumber()
{
    std::set<uint32_t> accountsIdSet;
    for (auto itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
        accountsIdSet.insert(itr->second->getAccountId());

    mAccountsNumber = static_cast<uint32_t>(accountsIdSet.size());
}

bool Guild::isLeader(Player* player) const
{
    if (player->getGuid() == m_leaderGuid)
        return true;

    if (const GuildMember* member = getMember(player->getGuid()))
        return member->isRank(GR_GUILDMASTER);

    return false;
}

void Guild::deleteBankItems(bool removeItemsFromDB)
{
    for (uint8_t tabId = 0; tabId < _getPurchasedTabsSize(); ++tabId)
    {
        _guildBankTabsStore[tabId]->removeBankTabItemFromDB(removeItemsFromDB);
        delete _guildBankTabsStore[tabId];
        _guildBankTabsStore[tabId] = nullptr;
    }
    _guildBankTabsStore.clear();
}

bool Guild::modifyBankMoney(uint64_t amount, bool add)
{
    if (add)
    {
        m_bankMoney += amount;
    }
    else
    {
        if (m_bankMoney < amount)
            return false;

        m_bankMoney -= amount;
    }

    CharacterDatabase.Execute("UPDATE guild SET bankBalance = %llu WHERE guildId = %u", m_bankMoney, m_id);

    return true;
}

void Guild::setLeaderGuid(GuildMember* pLeader)
{
    if (pLeader == nullptr)
        return;

    m_leaderGuid = pLeader->getGUID();
    pLeader->changeRank(GR_GUILDMASTER);

    CharacterDatabase.Execute("UPDATE guild SET leaderGuid = '%u' WHERE guildId = %u", WoWGuid::getGuidLowPartFromUInt64(m_leaderGuid), m_id);
}

void Guild::setRankBankMoneyPerDay(uint8_t rankId, uint32_t moneyPerDay)
{
    if (GuildRankInfo* rankInfo = getRankInfo(rankId))
        rankInfo->setBankMoneyPerDay(moneyPerDay);
}

void Guild::setRankBankTabRightsAndSlots(uint8_t rankId, GuildBankRightsAndSlots rightsAndSlots, bool saveToDB)
{
    if (rightsAndSlots.getTabId() >= _getPurchasedTabsSize())
        return;

    if (GuildRankInfo* rankInfo = getRankInfo(rankId))
        rankInfo->setBankTabSlotsAndRights(rightsAndSlots, saveToDB);
}

inline std::string Guild::getRankName(uint8_t rankId) const
{
    if (const GuildRankInfo* rankInfo = getRankInfo(rankId))
        return rankInfo->getName();

    return "unk";
}

inline uint32_t Guild::getRankRights(uint8_t rankId) const
{
    if (const GuildRankInfo* rankInfo = getRankInfo(rankId))
        return rankInfo->getRights();

    return 0;
}

inline int32_t Guild::getRankBankMoneyPerDay(uint8_t rankId) const
{
    if (const GuildRankInfo* rankInfo = getRankInfo(rankId))
        return rankInfo->getBankMoneyPerDay();

    return 0;
}

inline int32_t Guild::getRankBankTabSlotsPerDay(uint8_t rankId, uint8_t tabId) const
{
    if (tabId < _getPurchasedTabsSize())
    {
        if (const GuildRankInfo* rankInfo = getRankInfo(rankId))
            return rankInfo->getBankTabSlotsPerDay(tabId);
    }

    return 0;
}

inline int8_t Guild::getRankBankTabRights(uint8_t rankId, uint8_t tabId) const
{
    if (const GuildRankInfo* rankInfo = getRankInfo(rankId))
        return rankInfo->getBankTabRights(tabId);

    return 0;
}

inline int32_t Guild::getMemberRemainingSlots(GuildMember const* member, uint8_t tabId) const
{
    if (member)
    {
        uint8_t rankId = member->getRankId();
        if (rankId == GR_GUILDMASTER)
            return UNLIMITED_WITHDRAW_SLOTS;

        if ((getRankBankTabRights(rankId, tabId) & GB_RIGHT_VIEW_TAB) != 0)
        {
            int32_t remaining = getRankBankTabSlotsPerDay(rankId, tabId) - member->getBankWithdrawValue(tabId);
            if (remaining > 0)
                return remaining;
        }
    }

    return 0;
}

inline int32_t Guild::getMemberRemainingMoney(GuildMember const* member) const
{
    if (member)
    {
        uint8_t rankId = member->getRankId();
        if (rankId == GR_GUILDMASTER)
            return UNLIMITED_WITHDRAW_MONEY;

        if ((getRankRights(rankId) & (GR_RIGHT_WITHDRAW_REPAIR | GR_RIGHT_WITHDRAW_GOLD)) != 0)
        {
            int32_t remaining = getRankBankMoneyPerDay(rankId) - member->getBankWithdrawValue(MAX_GUILD_BANK_TABS);
            if (remaining > 0)
                return remaining;
        }
    }

    return 0;
}

inline void Guild::updateMemberWithdrawSlots(uint64_t guid, uint8_t tabId)
{
    if (GuildMember* member = getMember(guid))
    {
        uint8_t rankId = member->getRankId();
        if (rankId != GR_GUILDMASTER && member->getBankWithdrawValue(tabId) < getRankBankTabSlotsPerDay(rankId, tabId))
            member->updateBankWithdrawValue(tabId, 1);
    }
}

inline bool Guild::memberHasTabRights(uint64_t guid, uint8_t tabId, uint32_t rights) const
{
    if (const GuildMember* member = getMember(guid))
    {
        if (member->isRank(GR_GUILDMASTER) || m_leaderGuid == guid)
            return true;
        return (getRankBankTabRights(member->getRankId(), tabId) & rights) == rights;
    }

    return false;
}

inline void Guild::logEvent(GuildEventLogTypes eventType, uint32_t playerGuid1, uint32_t playerGuid2, uint8_t newRank)
{
    mEventLog->addEvent(new GuildEventLogEntry(m_id, mEventLog->getNextGUID(), eventType, playerGuid1, playerGuid2, newRank));
}

void Guild::logBankEvent(GuildBankEventLogTypes eventType, uint8_t tabId, uint32_t lowguid, uint32_t itemOrMoney, uint16_t itemStackCount, uint8_t destTabId)
{
    if (tabId > MAX_GUILD_BANK_TABS)
        return;

    if (eventType == GB_LOG_MOVE_ITEM && tabId == destTabId)
        return;

    uint8_t dbTabId = tabId;
    if (GuildBankEventLogEntry::isMoneyEvent(eventType))
    {
        tabId = MAX_GUILD_BANK_TABS;
        dbTabId = GUILD_BANK_MONEY_TAB;
    }

    GuildLogHolder* pLog = mBankEventLog[tabId];
    pLog->addEvent(new GuildBankEventLogEntry(m_id, pLog->getNextGUID(), eventType, dbTabId, lowguid, itemOrMoney, itemStackCount, destTabId));
}

void Guild::broadcastEvent(GuildEvents guildEvent, uint64_t guid, std::vector<std::string> vars) const
{
    broadcastPacket(SmsgGuildEvent(guildEvent, vars, guid).serialise().get());

    LogDebugFlag(LF_OPCODE, "SMSG_GUILD_EVENT: %s (%u)", _GetGuildEventString(guildEvent).c_str(), guildEvent);
}

void Guild::sendBankList(WorldSession* session, uint8_t tabId, bool withContent, bool withTabInfo) const
{
#if VERSION_STRING < Cata
    bool sendAllSlots = true;

    WorldPacket data(SMSG_GUILD_BANK_LIST, 500);
    data << uint64_t(m_bankMoney);
    data << uint8_t(tabId);
    size_t rempos = data.wpos();
    data << uint32_t(0);
    data << uint8_t(sendAllSlots);

    if (!tabId)
    {
        data << uint8_t(_getPurchasedTabsSize());                  // Number of tabs
        for (uint8_t i = 0; i < _getPurchasedTabsSize(); ++i)
        {
            data << _guildBankTabsStore[i]->getName();
            data << _guildBankTabsStore[i]->getIcon();
        }
    }

    const auto tab = getBankTab(tabId);
    if (!tab)
        data << uint8_t(0);
    else if (sendAllSlots)
        tab->writeInfoPacket(data);
    else
        data << uint8_t(0);

    if (session)
    {
        int32 numSlots = 0;
        if (GuildMember const* member = getMember(session->GetPlayer()->getGuid()))
            numSlots = getRankBankTabSlotsPerDay(member->getRankId(), tabId);
        data.put<uint32_t>(rempos, numSlots);
        session->SendPacket(&data);
    }
    else
    {
        for (auto itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
        {
            if (!memberHasTabRights(itr->second->getGUID(), tabId, GB_RIGHT_VIEW_TAB))
                continue;

            Player* player =  objmgr.GetPlayer(itr->second->getName().c_str(), true);
            if (!player)
                continue;

            uint32_t numSlots = getRankBankTabSlotsPerDay(itr->second->getRankId(), tabId);
            data.put<uint32_t>(rempos, numSlots);
            player->GetSession()->SendPacket(&data);
        }
    }
#else
    GuildMember const* member = getMember(session->GetPlayer()->getGuid());
    if (member == nullptr)
        return;

    ByteBuffer tabData;

    WorldPacket data(SMSG_GUILD_BANK_LIST, 500);
    data.writeBit(0);

    uint32_t itemCount = 0;
    if (withContent && memberHasTabRights(session->GetPlayer()->getGuid(), tabId, GB_RIGHT_VIEW_TAB))
    {
        if (GuildBankTab const* tab = getBankTab(tabId))
        {
            for (uint8_t slotId = 0; slotId < MAX_GUILD_BANK_SLOTS; ++slotId)
            {
                if (tab->getItem(slotId))
                {
                    ++itemCount;
                }
            }
        }
    }

    data.writeBits(itemCount, 20);
    data.writeBits(withTabInfo ? _getPurchasedTabsSize() : 0, 22);

    if (withContent && memberHasTabRights(session->GetPlayer()->getGuid(), tabId, GB_RIGHT_VIEW_TAB))
    {
        if (GuildBankTab const* tab = getBankTab(tabId))
        {
            for (uint8_t slotId = 0; slotId < MAX_GUILD_BANK_SLOTS; ++slotId)
            {
                if (Item* tabItem = tab->getItem(slotId))
                {
                    data.writeBit(0);

                    uint32_t enchants = 0;
                    for (uint32_t ench = 0; ench < MAX_ENCHANTMENT_SLOT; ++ench)
                    {
                        if (uint32_t enchantId = tabItem->getEnchantmentId(static_cast<uint8_t>(EnchantmentSlot(ench))))
                        {
                            tabData << uint32_t(enchantId);
                            tabData << uint32_t(ench);
                            ++enchants;
                        }
                    }

                    data.writeBits(enchants, 23);

                    tabData << uint32_t(0);
                    tabData << uint32_t(0);
                    tabData << uint32_t(0);
                    tabData << uint32_t(tabItem->getStackCount());
                    tabData << uint32_t(slotId);
                    tabData << uint32_t(0);
                    tabData << uint32_t(tabItem->getEntry());
                    tabData << uint32_t(tabItem->getRandomPropertiesId());
                    tabData << uint32_t(abs(0));
                    tabData << uint32_t(tabItem->getPropertySeed());
                }
            }
        }
    }

    if (withTabInfo)
    {
        for (uint8_t i = 0; i < _getPurchasedTabsSize(); ++i)
        {
            data.writeBits(_guildBankTabsStore[i]->getIcon().length(), 9);
            data.writeBits(_guildBankTabsStore[i]->getName().length(), 7);
        }
    }

    data.flushBits();

    if (withTabInfo)
    {
        for (uint8_t i = 0; i < _getPurchasedTabsSize(); ++i)
        {
            data.WriteString(_guildBankTabsStore[i]->getIcon());
            data << uint32_t(i);
            data.WriteString(_guildBankTabsStore[i]->getName());
        }
    }

    data << uint64_t(m_bankMoney);
    if (tabData.size())
    {
        data.append(tabData);
    }

    data << uint32_t(tabId);
    data << uint32_t(getMemberRemainingSlots(member, tabId));

    session->SendPacket(&data);
#endif
}

void Guild::sendGuildRanksUpdate(uint64_t setterGuid, uint64_t targetGuid, uint32_t rank)
{
#if VERSION_STRING >= Cata
    ObjectGuid tarGuid = targetGuid;
    ObjectGuid setGuid = setterGuid;

    GuildMember* member = getMember(targetGuid);
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

    data.writeBit(rank < member->getRankId());

    data.writeBit(setGuid[5]);
    data.writeBit(setGuid[0]);

    data.writeBit(tarGuid[6]);

    data.writeBit(setGuid[3]);
    data.writeBit(setGuid[6]);

    data.writeBit(tarGuid[3]);

    data.writeBit(setGuid[4]);

    data.flushBits();

    data << uint32_t(rank);

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

    broadcastPacket(&data);

    member->changeRank(static_cast<uint8_t>(rank));

    LogDebugFlag(LF_OPCODE, "SMSG_GUILD_RANKS_UPDATE target: %u, issuer: %u, rankId: %u",
        WoWGuid::getGuidLowPartFromUInt64(targetGuid), WoWGuid::getGuidLowPartFromUInt64(setterGuid), rank);
#endif
}

void Guild::giveXP(uint32_t xp, Player* source)
{
#if VERSION_STRING >= Cata
    if (worldConfig.guild.levelingEnabled == false)
        return;

    if (getLevel() >= worldConfig.guild.maxLevel)
        xp = 0;

    if (getLevel() >= UNCAPPED_GUILD_LEVEL)
        xp = std::min(xp, worldConfig.guild.maxXpPerDay - uint32_t(m_todayExperience));

    WorldPacket data(SMSG_GUILD_XP_GAIN, 8);
    data << uint64_t(xp);
    source->GetSession()->SendPacket(&data);

    m_experience += xp;
    m_todayExperience += xp;

    if (xp == 0)
        return;

    uint32_t oldLevel = getLevel();

    while (getExperience() >= sGuildMgr.getXPForGuildLevel(getLevel()) && getLevel() < 25)
    {
        m_experience -= sGuildMgr.getXPForGuildLevel(getLevel());
        ++m_level;

        std::vector<uint32_t> perksToLearn;
        for (uint32_t i = 0; i < sGuildPerkSpellsStore.GetNumRows(); ++i)
        {
            if (DBC::Structures::GuildPerkSpellsEntry const* entry = sGuildPerkSpellsStore.LookupEntry(i))
            {
                if (entry->Level > oldLevel && entry->Level <= getLevel())
                {
                    perksToLearn.push_back(entry->SpellId);
                }
            }
        }

        for (GuildMembersStore::const_iterator itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
        {
            if (Player* player = itr->second->getPlayerByGuid(itr->second->getGUID()))
            {
                player->setGuildLevel(getLevel());
                for (size_t i = 0; i < perksToLearn.size(); ++i)
                {
                    player->addSpell(perksToLearn[i]);
                }
            }
        }

        addGuildNews(GN_LEVEL_UP, 0, 0, m_level);
        ++oldLevel;
    }
#endif
}

void Guild::sendGuildXP(WorldSession* session) const
{
#if VERSION_STRING >= Cata
    WorldPacket data(SMSG_GUILD_XP, 40);
    data << uint64_t(/*member ? member->getTotalActivity() :*/ 0);
    data << uint64_t(sGuildMgr.getXPForGuildLevel(getLevel()) - getExperience());
    data << uint64_t(getTodayExperience());
    data << uint64_t(/*member ? member->getWeeklyActivity() :*/ 0);
    data << uint64_t(getExperience());

    session->SendPacket(&data);
#endif
}

void Guild::sendGuildReputationWeeklyCap(WorldSession* session, uint32_t reputation) const
{
#if VERSION_STRING >= Cata
    uint32_t cap = worldConfig.guild.maxRepPerWeek - reputation;

    WorldPacket data(SMSG_GUILD_REPUTATION_WEEKLY_CAP, 4);
    data << uint32_t(cap);
    session->SendPacket(&data);

    LogDebugFlag(LF_OPCODE, "SMSG_GUILD_REPUTATION_WEEKLY_CAP %s: Left: %u", session->GetPlayer()->getName().c_str(), cap);
#endif
}

void Guild::resetTimes(bool weekly)
{
#if VERSION_STRING >= Cata
    m_todayExperience = 0;
    for (GuildMembersStore::const_iterator itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
    {
        itr->second->resetValues(weekly);
        if (Player* player = itr->second->getPlayerByGuid(itr->second->getGUID()))
        {
            WorldPacket data(SMSG_GUILD_MEMBER_DAILY_RESET, 0);
            player->GetSession()->SendPacket(&data);
        }
    }
#endif
}

void Guild::addGuildNews(uint8_t type, uint64_t guid, uint32_t flags, uint32_t value)
{
#if VERSION_STRING >= Cata
    uint32_t lowGuid = WoWGuid::getGuidLowPartFromUInt64(guid);
    GuildNewsLogEntry* news = new GuildNewsLogEntry(m_id, mNewsLog->getNextGUID(), GuildNews(type), lowGuid, flags, value);

    mNewsLog->addEvent(news);

    WorldPacket data(SMSG_GUILD_NEWS_UPDATE, 7 + 32);
    data.writeBits(1, 21);
    ByteBuffer buffer;
    news->writeGuildLogPacket(data, buffer);

    broadcastPacket(&data);
#endif
}

bool Guild::hasAchieved(uint32_t /*achievementId*/) const
{
    return false;
}

void Guild::handleNewsSetSticky(WorldSession* session, uint32_t newsId, bool sticky)
{
#if VERSION_STRING >= Cata
    GuildLog* logs = mNewsLog->getGuildLog();
    GuildLog::iterator itr = logs->begin();
    while (itr != logs->end() && (*itr)->getGUID() != newsId)
        ++itr;

    if (itr == logs->end())
    {
        LogDebug("HandleNewsSetSticky: %s requested unknown newsId %u - Sticky: %u", session->GetPlayer()->getName().c_str(), newsId, sticky);
        return;
    }

    GuildNewsLogEntry* news = static_cast<GuildNewsLogEntry*>(*itr);
    news->setSticky(sticky);

    LogDebug("HandleNewsSetSticky: %s chenged newsId %u sticky to %u", session->GetPlayer()->getName().c_str(), newsId, sticky);

    WorldPacket data(SMSG_GUILD_NEWS_UPDATE, 7 + 32);
    data.writeBits(1, 21);
    ByteBuffer buffer;
    news->writeGuildLogPacket(data, buffer);

    session->SendPacket(&data);
#endif
}

void Guild::handleGuildRequestChallengeUpdate(WorldSession* session)
{
#if VERSION_STRING >= Cata
    WorldPacket data(SMSG_GUILD_CHALLENGE_UPDATED, 4 * 4 * 5);

    for (int i = 0; i < 4; ++i)
        data << uint32_t(guildChallengeXPReward[i]);

    for (int i = 0; i < 4; ++i)
        data << uint32_t(guildChallengeGoldReward[i]);

    for (int i = 0; i < 4; ++i)
        data << uint32_t(guildChallengesPerWeek[i]);

    for (int i = 0; i < 4; ++i)
        data << uint32_t(guildChallengeMaxLevelGoldReward[i]);

    for (int i = 0; i < 4; ++i)
        data << uint32_t(0);

    session->SendPacket(&data);
#endif
}

void Guild::sendTurnInPetitionResult(WorldSession* pClient, uint32_t result)
{
    if (pClient == nullptr)
        return;

    WorldPacket data(SMSG_TURN_IN_PETITION_RESULTS, 4);
    data << result;
    pClient->SendPacket(&data);
}

void Guild::swapItems(Player* player, uint8_t tabId, uint8_t slotId, uint8_t destTabId, uint8_t destSlotId, uint32_t splitedAmount)
{
    if (tabId >= _getPurchasedTabsSize() || slotId >= MAX_GUILD_BANK_SLOTS ||
        destTabId >= _getPurchasedTabsSize() || destSlotId >= MAX_GUILD_BANK_SLOTS)
        return;

    if (tabId == destTabId && slotId == destSlotId)
        return;

    Item* pItem = getBankTab(tabId)->getItem(slotId);
    Item* pItem2;

    if (memberHasTabRights(player->getGuid(), tabId, GB_RIGHT_DEPOSIT_ITEM) == false)
        return;

    if (pItem != nullptr && splitedAmount > 0 && pItem->getStackCount() > splitedAmount)
    {
        pItem2 = pItem;

        pItem2->modStackCount(-static_cast<int32_t>(splitedAmount));
        pItem2->setCreatorGuid(0);
        pItem2->SaveToDB(0, 0, true, nullptr);

        pItem = objmgr.CreateItem(pItem2->getEntry(), player);
        if (pItem == nullptr)
            return;

        pItem->setStackCount(splitedAmount);
        pItem->setCreatorGuid(0);
        pItem->SaveToDB(0, 0, true, nullptr);
    }
    else
    {
        pItem2 = pItem;
        pItem = nullptr;
    }

    getBankTab(destTabId)->setItem(slotId, pItem);
    getBankTab(destTabId)->setItem(destSlotId, pItem2);

    SlotIds slots;
    slots.insert(slotId);
    slots.insert(destSlotId);

    _sendBankContentUpdate(tabId, slots);
}

void Guild::swapItemsWithInventory(Player* player, bool toChar, uint8_t tabId, uint8_t slotId, uint8_t playerBag, uint8_t playerSlotId, uint32_t splitedAmount)
{
    if ((slotId >= MAX_GUILD_BANK_SLOTS && slotId != UNDEFINED_TAB_SLOT) || tabId >= _getPurchasedTabsSize())
        return;

    Item* pSourceItem = player->getItemInterface()->GetInventoryItem(playerBag, playerSlotId);
    Item* pDestItem = getBankTab(tabId)->getItem(slotId);
    Item* pSourceItem2 = pSourceItem;

    if (pSourceItem != nullptr)
    {
        if (pSourceItem->isSoulbound() || pSourceItem->getItemProperties()->Class == ITEM_CLASS_QUEST)
        {
            player->getItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_CANT_DROP_SOULBOUND);
            return;
        }
    }

    if (!toChar)
    {
        if (!memberHasTabRights(player->getGuid(), tabId, GB_RIGHT_DEPOSIT_ITEM))
            return;

        if (splitedAmount && pSourceItem->getStackCount() > splitedAmount)
        {
            pSourceItem = objmgr.CreateItem(pSourceItem2->getEntry(), player);
            if (pSourceItem == nullptr)
                return;

            pSourceItem->setStackCount(splitedAmount);
            pSourceItem->setCreatorGuid(pSourceItem2->getCreatorGuid());
            pSourceItem2->modStackCount(-static_cast<int32_t>(splitedAmount));
            pSourceItem2->m_isDirty = true;
        }
        else
        {
            if (player->getItemInterface()->SafeRemoveAndRetreiveItemFromSlot(playerBag, playerSlotId, false) == nullptr)
                return;
            if(pSourceItem)
                pSourceItem->RemoveFromWorld();
        }

        if (pSourceItem == nullptr)
        {
            if (pDestItem != nullptr && splitedAmount > 0 && pDestItem->getStackCount() > splitedAmount)
            {
                pSourceItem2 = pDestItem;

                pSourceItem2->modStackCount(-static_cast<int32_t>(splitedAmount));
                pSourceItem2->SaveToDB(0, 0, true, nullptr);

                pDestItem = objmgr.CreateItem(pSourceItem2->getEntry(), player);
                if (pDestItem == nullptr)
                {
                    return;
                }

                pDestItem->setStackCount(splitedAmount);
                pDestItem->setCreatorGuid(pSourceItem2->getCreatorGuid());
            }
            else
            {
                getBankTab(tabId)->setItem(slotId, nullptr);
            }
        }
        else
        {
            getBankTab(tabId)->setItem(slotId, pSourceItem);

            pSourceItem->setOwner(nullptr);
            pSourceItem->SaveToDB(0, 0, true, nullptr);
        }
    }
    else
    {
        if (pDestItem)
        {
            if (getMemberRemainingSlots(getMember(player->getGuid()), tabId) == 0)
                return;

            pDestItem->setOwner(player);
            pDestItem->SaveToDB(playerBag, playerSlotId, true, nullptr);

            if (!player->getItemInterface()->SafeAddItem(pDestItem, 0, 0))
            {
                if (!player->getItemInterface()->AddItemToFreeSlot(pDestItem))
                    pDestItem->DeleteMe();
            }

            logBankEvent(GB_LOG_WITHDRAW_ITEM, tabId, player->getGuidLow(),
                getBankTab(tabId)->getItem(slotId)->getEntry(), static_cast<uint16_t>(getBankTab(tabId)->getItem(slotId)->getStackCount()));

            getBankTab(tabId)->setItem(slotId, nullptr);
        }
    }

    SlotIds slots;

    slots.insert(slotId);

    _sendBankContentUpdate(tabId, slots, true);
}

void Guild::_sendBankContentUpdate(uint8_t tabId, SlotIds slots, bool sendAllSlots) const
{
#if VERSION_STRING < Cata
    WorldPacket data(SMSG_GUILD_BANK_LIST, 500);
    data << uint64_t(m_bankMoney);
    data << uint8_t(tabId);
    size_t rempos = data.wpos();
    data << uint32_t(0);
    data << uint8_t(sendAllSlots);

    if (!tabId && sendAllSlots)
    {
        data << uint8_t(_getPurchasedTabsSize());                  // Number of tabs
        for (uint8_t i = 0; i < _getPurchasedTabsSize(); ++i)
        {
            data << _guildBankTabsStore[i]->getName();
            data << _guildBankTabsStore[i]->getIcon();
        }
    }

    const auto tab = getBankTab(tabId);
    if (!tab)
        data << uint8_t(0);
    else if (sendAllSlots)
        tab->writeInfoPacket(data);
    else if (!slots.empty())
    {
        data << uint8(slots.size());
        for (auto itr = slots.begin(); itr != slots.end(); ++itr)
            tab->writeSlotPacket(data, *itr, true);
    }
    else
        data << uint8_t(0);

    for (auto itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
    {
        if (!memberHasTabRights(itr->second->getGUID(), tabId, GB_RIGHT_VIEW_TAB))
            continue;

        Player* player = objmgr.GetPlayer(itr->second->getName().c_str(), true);
        if (!player)
            continue;

        uint32_t numSlots = getRankBankTabSlotsPerDay(itr->second->getRankId(), tabId);
        data.put<uint32_t>(rempos, numSlots);
        player->GetSession()->SendPacket(&data);
    }
#else
    if (GuildBankTab const* guildBankTab = getBankTab(tabId))
    {
        ByteBuffer tabData;
        WorldPacket data(SMSG_GUILD_BANK_LIST, 1200);
        data.writeBit(0);

        if (sendAllSlots)
            data.writeBits(MAX_GUILD_BANK_SLOTS, 20);
        else
            data.writeBits(slots.size(), 20);

        data.writeBits(0, 22);

        if (sendAllSlots)
        {
            for (uint8_t slotId = 0; slotId < MAX_GUILD_BANK_SLOTS; ++slotId)
            {
                if (const auto tab = getBankTab(tabId))
                {
                    Item* tabItem = tab->getItem(slotId);

                    data.writeBit(0);

                    uint32_t enchantCount = 0;
                    if (tabItem)
                    {
                        for (uint32_t enchSlot = 0; enchSlot < MAX_ENCHANTMENT_SLOT; ++enchSlot)
                        {
                            if (uint32_t enchantId = tabItem->getEnchantmentId(static_cast<uint8_t>(EnchantmentSlot(enchSlot))))
                            {
                                tabData << uint32_t(enchantId);
                                tabData << uint32_t(enchSlot);
                                ++enchantCount;
                            }
                        }
                    }

                    data.writeBits(enchantCount, 23);

                    tabData << uint32_t(0);
                    tabData << uint32_t(0);
                    tabData << uint32_t(0);
                    tabData << uint32_t(tabItem ? tabItem->getStackCount() : 0);
                    tabData << uint32_t(slotId);
                    tabData << uint32_t(0);
                    tabData << uint32_t(tabItem ? tabItem->getEntry() : 0);
                    tabData << uint32_t(tabItem ? tabItem->getRandomPropertiesId() : 0);
                    tabData << uint32_t(tabItem ? 0 : 0); // @todo add Spell Charges
                    tabData << uint32_t(tabItem ? tabItem->getPropertySeed() : 0);
                }
            }
        }
        else
        {
            for (auto itr = slots.begin(); itr != slots.end(); ++itr)
            {
                data.writeBit(0);

                Item* tabItem = guildBankTab->getItem(*itr);
                uint32_t enchantCount = 0;
                if (tabItem)
                {
                    for (uint32_t enchSlot = 0; enchSlot < MAX_ENCHANTMENT_SLOT; ++enchSlot)
                    {
                        if (uint32_t enchantId = tabItem->getEnchantmentId(static_cast<uint8_t>(EnchantmentSlot(enchSlot))))
                        {
                            tabData << uint32_t(enchantId);
                            tabData << uint32_t(enchSlot);
                            ++enchantCount;
                        }
                    }
                }

                data.writeBits(enchantCount, 23);

                tabData << uint32_t(0);
                tabData << uint32_t(0);
                tabData << uint32_t(0);
                tabData << uint32_t(tabItem ? tabItem->getStackCount() : 0);
                tabData << uint32_t(*itr);
                tabData << uint32_t(0);
                tabData << uint32_t(tabItem ? tabItem->getEntry() : 0);
                tabData << uint32_t(tabItem ? tabItem->getRandomPropertiesId() : 0);
                tabData << uint32_t(tabItem ? 0 : 0); // @todo add Spell Charges
                tabData << uint32_t(tabItem ? tabItem->getPropertySeed() : 0);
            }
        }

        data.flushBits();

        data << uint64_t(m_bankMoney);
        if (tabData.size())
        {
            data.append(tabData);
        }

        data << uint32_t(tabId);

        size_t rempos = data.wpos();
        data << uint32_t(0);

        for (GuildMembersStore::const_iterator itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
        {
            if (memberHasTabRights(itr->second->getGUID(), tabId, GB_RIGHT_VIEW_TAB))
            {
                if (Player* player = itr->second->getPlayerByGuid(itr->second->getGUID()))
                {
                    data.put<uint32_t>(rempos, uint32_t(getMemberRemainingSlots(itr->second, tabId)));
                    player->GetSession()->SendPacket(&data);
                }
            }
        }

        LogDebugFlag(LF_OPCODE, "SMSG_GUILD_BANK_LIST");
    }
#endif
}

Guild::GuildMember::GuildMember(uint32_t guildId, uint64_t guid, uint8_t rankId) : mGuildId(guildId), mGuid(guid), mZoneId(0), mLevel(0), mClass(0),
mFlags(GEM_STATUS_NONE), mLogoutTime(::time(nullptr)), mAccountId(0), mRankId(rankId), mAchievementPoints(0),
mTotalActivity(0), mWeekActivity(0), mTotalReputation(0), mBankWithdraw{ 0 }, mWeekReputation(0)
{
    memset(mBankWithdraw, 0, (MAX_GUILD_BANK_TABS + 1) * sizeof(int32_t));
}

void Guild::GuildMember::setStats(Player* player)
{
    mName = player->getName();
    mLevel = static_cast<uint8_t>(player->getLevel());
    mClass = player->getClass();
    mZoneId = player->GetZoneId();
    mAccountId = player->GetSession()->GetAccountId();
    mAchievementPoints = 0;
}

void Guild::GuildMember::setStats(std::string const& name, uint8_t level, uint8_t _class, uint32_t zoneId, uint32_t accountId, uint32_t reputation)
{
    mName = name;
    mLevel = level;
    mClass = _class;
    mZoneId = zoneId;
    mAccountId = accountId;
    mTotalReputation = reputation;
}

bool Guild::GuildMember::checkStats() const
{
    if (mLevel < 1)
    {
        LogError("Player (GUID: %u) has a broken data in field `characters`.`level`, deleting him from guild!", WoWGuid::getGuidLowPartFromUInt64(mGuid));
        return false;
    }

    if (mClass < 1 || mClass >= 12)
    {
        LogError("Player (GUID: %u) has a broken data in field `characters`.`class`, deleting him from guild!", WoWGuid::getGuidLowPartFromUInt64(mGuid));
        return false;
    }

    return true;
}

void Guild::GuildMember::setPublicNote(std::string const& publicNote)
{
    if (mPublicNote == publicNote)
        return;

    mPublicNote = publicNote;

    CharacterDatabase.Execute("UPDATE guild_members SET publicNote = '%s' WHERE playerid = %u", publicNote.c_str(), WoWGuid::getGuidLowPartFromUInt64(mGuid));
}

void Guild::GuildMember::setOfficerNote(std::string const& officerNote)
{
    if (mOfficerNote == officerNote)
        return;

    mOfficerNote = officerNote;

    CharacterDatabase.Execute("UPDATE guild_members SET officerNote = '%s' WHERE playerid = %u", officerNote.c_str(), WoWGuid::getGuidLowPartFromUInt64(mGuid));
}

void Guild::GuildMember::setZoneId(uint32_t id)
{
    mZoneId = id;
}

void Guild::GuildMember::setAchievementPoints(uint32_t val)
{
    mAchievementPoints = val;
}

void Guild::GuildMember::setLevel(uint8_t var)
{
    mLevel = var;
}

void Guild::GuildMember::addFlag(uint8_t var)
{
    mFlags |= var;
}

void Guild::GuildMember::removeFlag(uint8_t var)
{
    mFlags &= ~var;
}

void Guild::GuildMember::resetFlags()
{
    mFlags = GEM_STATUS_NONE;
}

bool Guild::GuildMember::loadGuildMembersFromDB(Field* fields, Field* fields2)
{
    PlayerInfo* plr = objmgr.GetPlayerInfo((fields[1].GetUInt32()));
    if (plr == nullptr)
        return false;

    plr->m_guild = fields[0].GetUInt32();
    plr->guildRank = fields[2].GetUInt32();
    mPublicNote = fields[3].GetString();
    mOfficerNote = fields[4].GetString();

    for (uint8_t i = 0; i <= MAX_GUILD_BANK_TABS; ++i)
        mBankWithdraw[i] = fields2[1 + i].GetUInt32();

    setStats(plr->name, static_cast<uint8_t>(plr->lastLevel), plr->cl, plr->lastZone, plr->acct, 0);
    mLogoutTime = plr->lastOnline;
    mTotalActivity = 0;
    mWeekActivity = 0;
    mWeekReputation = 0;

    if (!checkStats())
        return false;

    if (!mZoneId)
    {
        LogError("Player (GUID: %u) has broken zone-data", WoWGuid::getGuidLowPartFromUInt64(mGuid));
        mZoneId = objmgr.GetPlayer(WoWGuid::getGuidLowPartFromUInt64(mGuid))->GetZoneId();
    }

    resetFlags();

    return true;
}

void Guild::GuildMember::saveGuildMembersToDB(bool /*_delete*/) const
{
    CharacterDatabase.Execute("REPLACE INTO guild_members VALUES (%u, %u, %u, '%s', '%s')",
        mGuildId, WoWGuid::getGuidLowPartFromUInt64(mGuid), static_cast<uint32_t>(mRankId), mPublicNote.c_str(), mOfficerNote.c_str());
}

uint64_t Guild::GuildMember::getGUID() const
{
    return mGuid;
}

std::string const& Guild::GuildMember::getName() const
{
    return mName;
}

const char* Guild::GuildMember::getNameChar()
{
    return mName.c_str();
}

uint32_t Guild::GuildMember::getAccountId() const
{
    return mAccountId;
}

uint8_t Guild::GuildMember::getRankId() const
{
    return mRankId;
}

uint64_t Guild::GuildMember::getLogoutTime() const
{
    return mLogoutTime;
}

std::string Guild::GuildMember::getPublicNote() const
{
    return mPublicNote;
}

std::string Guild::GuildMember::getOfficerNote() const
{
    return mOfficerNote;
}

uint8_t Guild::GuildMember::getClass() const
{
    return mClass;
}

uint8_t Guild::GuildMember::getLevel() const
{
    return mLevel;
}

uint8_t Guild::GuildMember::getFlags() const
{
    return mFlags;
}

uint32_t Guild::GuildMember::getZoneId() const
{
    return mZoneId;
}

uint32_t Guild::GuildMember::getAchievementPoints() const
{
    return mAchievementPoints;
}

uint64_t Guild::GuildMember::getTotalActivity() const
{
    return mTotalActivity;
}

uint64_t Guild::GuildMember::getWeekActivity() const
{
    return mWeekActivity;
}

uint32_t Guild::GuildMember::getTotalReputation() const
{
    return mTotalReputation;
}

uint32_t Guild::GuildMember::getWeekReputation() const
{
    return mWeekReputation;
}

bool Guild::GuildMember::isOnline()
{
    return (mFlags & GEM_STATUS_ONLINE);
}

void Guild::GuildMember::changeRank(uint8_t newRank)
{
    mRankId = newRank;

    if (Player* player = objmgr.GetPlayer(WoWGuid::getGuidLowPartFromUInt64(mGuid)))
        player->setGuildRank(newRank);

    CharacterDatabase.Execute("UPDATE guild_members SET guildRank = '%u' WHERE playerid = %u", static_cast<uint32_t>(newRank), WoWGuid::getGuidLowPartFromUInt64(mGuid));
}

void Guild::GuildMember::updateLogoutTime()
{
    mLogoutTime = ::time(nullptr);
}

bool Guild::GuildMember::isRank(uint8_t rankId) const
{
    return mRankId == rankId;
}

bool Guild::GuildMember::isRankNotLower(uint8_t rankId) const
{
    return mRankId <= rankId;
}

bool Guild::GuildMember::isSamePlayer(uint64_t guid) const
{
    return mGuid == guid;
}

void Guild::GuildMember::updateBankWithdrawValue(uint8_t tabId, uint32_t amount)
{
    mBankWithdraw[tabId] += amount;

    CharacterDatabase.Execute("REPLACE INTO guild_members_withdraw VALUES('%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u', '%u')",
        WoWGuid::getGuidLowPartFromUInt64(mGuid),
        mBankWithdraw[0], mBankWithdraw[1], mBankWithdraw[2], mBankWithdraw[3], mBankWithdraw[4],
        mBankWithdraw[5], mBankWithdraw[6], 0, 0);
}

void Guild::GuildMember::resetValues(bool weekly)
{
    for (uint8_t tabId = 0; tabId <= MAX_GUILD_BANK_TABS; ++tabId)
        mBankWithdraw[tabId] = 0;

    if (weekly)
    {
        mWeekActivity = 0;
        mWeekReputation = 0;
    }
}

int32_t Guild::GuildMember::getBankWithdrawValue(uint8_t tabId) const
{
    if (isRank(GR_GUILDMASTER))
        return tabId == MAX_GUILD_BANK_TABS ? UNLIMITED_WITHDRAW_MONEY : UNLIMITED_WITHDRAW_SLOTS;

    return mBankWithdraw[tabId];
}

Player* Guild::GuildMember::getPlayerByGuid(uint64_t m_guid)
{
    return objmgr.GetPlayer(WoWGuid::getGuidLowPartFromUInt64(m_guid));
}
