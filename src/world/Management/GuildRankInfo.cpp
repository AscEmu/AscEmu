/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GuildRankInfo.h"
#include "GuildBankRightsAndSlots.h"
#include "Log.hpp"
#include "Database/Database.h"
#include "Server/MainServerDefines.h"


GuildRankInfo::GuildRankInfo() : mGuildId(0), mRankId(GUILD_RANK_NONE), mRights(GR_RIGHT_EMPTY), mBankMoneyPerDay(0)
{
}

GuildRankInfo::GuildRankInfo(uint32_t guildId) : mGuildId(guildId), mRankId(GUILD_RANK_NONE), mRights(GR_RIGHT_EMPTY), mBankMoneyPerDay(0)
{
}

GuildRankInfo::GuildRankInfo(uint32_t guildId, uint8_t rankId, std::string const& name, uint32_t rights, uint32_t money) :
    mGuildId(guildId), mRankId(rankId), mName(name), mRights(rights), mBankMoneyPerDay(money)
{
}

void GuildRankInfo::loadGuildRankFromDB(Field* fields)
{
    mRankId = fields[1].GetUInt8();
    mName = fields[2].GetString();
    mRights = fields[3].GetUInt32();
    mBankMoneyPerDay = fields[4].GetUInt32();

    if (mRankId == GR_GUILDMASTER)
    {
        mRights |= GR_RIGHT_ALL;
    }
}

void GuildRankInfo::saveGuildRankToDB(bool _delete) const
{
    if (_delete)
    {
        CharacterDatabase.Execute("DELETE FROM guild_ranks WHERE guildId = %u AND rankId = %u", mGuildId, (uint32_t)mRankId);
    }
    else
    {
        CharacterDatabase.Execute("DELETE FROM guild_ranks WHERE guildId = %u AND rankId = %u", mGuildId, (uint32_t)mRankId);
        CharacterDatabase.Execute("INSERT INTO guild_ranks (guildId, rankId, rankName, rankRights, goldLimitPerDay) VALUES ('%u', '%u', '%s', '%u', '0')",
            mGuildId, (uint32_t)mRankId, mName.c_str(), mRights);
    }
}

uint8_t GuildRankInfo::getId() const
{
    return mRankId;
}

std::string const& GuildRankInfo::getName() const
{
    return mName;
}

void GuildRankInfo::setName(std::string const& name)
{
    if (mName == name)
        return;

    mName = name;

    CharacterDatabase.Execute("UPDATE guild_ranks SET rankName = '%s', rankId = %u WHERE guildId = %u", mName.c_str(), static_cast<uint32_t>(mRankId), mGuildId);
}

uint32_t GuildRankInfo::getRights() const
{
    return mRights;
}

void GuildRankInfo::setRights(uint32_t rights)
{
    if (mRankId == GR_GUILDMASTER)
        rights = GR_RIGHT_ALL;

    if (mRights == rights)
        return;

    mRights = rights;

    CharacterDatabase.Execute("UPDATE guild_ranks SET rankRights = %u WHERE guildId = %u AND rankId = %u", mRights, mGuildId, static_cast<uint32_t>(mRankId));
}

int32_t GuildRankInfo::getBankMoneyPerDay() const
{
    return mBankMoneyPerDay;
}

void GuildRankInfo::setBankMoneyPerDay(uint32_t money)
{
    if (mRankId == GR_GUILDMASTER)
        money = uint32_t(UNLIMITED_WITHDRAW_MONEY);

    if (mBankMoneyPerDay == money)
        return;

    mBankMoneyPerDay = money;

    CharacterDatabase.Execute("UPDATE guild_ranks SET goldLimitPerDay = '%u', rankId = '%u' WHERE guildId = %u", money, static_cast<uint32_t>(mRankId), mGuildId);
}

int8_t GuildRankInfo::getBankTabRights(uint8_t tabId) const
{
    return tabId < MAX_GUILD_BANK_TABS ? mBankTabRightsAndSlots[tabId].getRights() : 0;
}

int32_t GuildRankInfo::getBankTabSlotsPerDay(uint8_t tabId) const
{
    return tabId < MAX_GUILD_BANK_TABS ? mBankTabRightsAndSlots[tabId].getSlots() : 0;
}

void GuildRankInfo::createMissingTabsIfNeeded(uint8_t tabs, bool /*_delete*/, bool logOnCreate)
{
    for (uint8_t i = 0; i < tabs; ++i)
    {
        GuildBankRightsAndSlots& rightsAndSlots = mBankTabRightsAndSlots[i];
        if (rightsAndSlots.getTabId() == i)
            continue;

        rightsAndSlots.setTabId(i);
        if (mRankId == GR_GUILDMASTER)
            rightsAndSlots.SetGuildMasterValues();

        if (logOnCreate)
            LogError("Guild %u has broken Tab %u for rank %u. Created default tab.", mGuildId, i, static_cast<uint32_t>(mRankId));

        CharacterDatabase.Execute("REPLACE INTO guild_bank_rights VALUES(%u, %u, %u, %u, %u);",
            mGuildId, i, static_cast<uint32_t>(mRankId), static_cast<uint32_t>(rightsAndSlots.getRights()), rightsAndSlots.getSlots());
    }
}

void GuildRankInfo::setBankTabSlotsAndRights(GuildBankRightsAndSlots rightsAndSlots, bool saveToDB)
{
    if (mRankId == GR_GUILDMASTER)
        rightsAndSlots.SetGuildMasterValues();

    GuildBankRightsAndSlots& guildBR = mBankTabRightsAndSlots[rightsAndSlots.getTabId()];
    guildBR = rightsAndSlots;

    if (saveToDB)
    {
        CharacterDatabase.Execute("REPLACE INTO guild_bank_rights VALUES(%u, %u, %u, %u, %u)",
            mGuildId, static_cast<uint32_t>(guildBR.getTabId()), static_cast<uint32_t>(mRankId), static_cast<uint32_t>(guildBR.getRights()), guildBR.getSlots());
    }
}
