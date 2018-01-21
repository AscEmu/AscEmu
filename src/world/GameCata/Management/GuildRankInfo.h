/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Guild/GuildDefinitions.h"
#include "GuildBankRightsAndSlots.h"
#include "Database/Field.h"

class Guild;

class GuildRankInfo
{
    public:

        GuildRankInfo();

        GuildRankInfo(uint32_t guildId);

        GuildRankInfo(uint32_t guildId, uint8_t rankId, std::string const& name, uint32_t rights, uint32_t money);

        void loadGuildRankFromDB(Field* fields);
        void saveGuildRankToDB(bool _delete) const;

        uint8_t getId() const;

        std::string const& getName() const;
        void setName(std::string const& name);

        uint32_t getRights() const;
        void setRights(uint32_t rights);

        int32_t getBankMoneyPerDay() const;
        void setBankMoneyPerDay(uint32_t money);

        int8_t getBankTabRights(uint8_t tabId) const;

        int32_t getBankTabSlotsPerDay(uint8_t tabId) const;

        void setBankTabSlotsAndRights(GuildBankRightsAndSlots rightsAndSlots, bool saveToDB);
        void createMissingTabsIfNeeded(uint8_t ranks, bool _delete, bool logOnCreate = false);

    private:

        uint32_t mGuildId;
        uint8_t mRankId;
        std::string mName;
        uint32_t mRights;
        uint32_t mBankMoneyPerDay;

        GuildBankRightsAndSlots mBankTabRightsAndSlots[MAX_GUILD_BANK_TABS];
};
