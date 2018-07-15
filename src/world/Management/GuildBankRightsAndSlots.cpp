/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GuildBankRightsAndSlots.h"


GuildBankRightsAndSlots::GuildBankRightsAndSlots() : mTabId(UNDEFINED_GUILD_TAB), mRights(0), mSlots(0)
{
}

GuildBankRightsAndSlots::GuildBankRightsAndSlots(uint8_t tabId) : mTabId(tabId), mRights(0), mSlots(0)
{
}

GuildBankRightsAndSlots::GuildBankRightsAndSlots(uint8_t tabId, uint8_t rights, uint32_t slots) : mTabId(tabId), mRights(rights), mSlots(slots)
{
}

void GuildBankRightsAndSlots::SetGuildMasterValues()
{
    mRights = GB_RIGHT_FULL;
    mSlots = uint32_t(UNLIMITED_WITHDRAW_SLOTS);
}

void GuildBankRightsAndSlots::setTabId(uint8_t tabId)
{
    mTabId = tabId;
}

uint8_t GuildBankRightsAndSlots::getTabId() const
{
    return mTabId;
}

void GuildBankRightsAndSlots::setRights(uint8_t rights)
{
    mRights = rights;
}

uint8_t GuildBankRightsAndSlots::getRights() const
{
    return mRights;
}

void GuildBankRightsAndSlots::setSlots(uint32_t slots)
{
    mSlots = slots;
}

uint32_t GuildBankRightsAndSlots::getSlots() const
{
    return mSlots;
}
