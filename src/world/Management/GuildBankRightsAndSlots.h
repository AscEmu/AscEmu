/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Guild/GuildDefinitions.h"

class GuildBankRightsAndSlots
{
    public:

        GuildBankRightsAndSlots();
        GuildBankRightsAndSlots(uint8_t tabId);
        GuildBankRightsAndSlots(uint8_t tabId, uint8_t rights, uint32_t slots);

        void SetGuildMasterValues();

        void setTabId(uint8_t tabId);
        uint8_t getTabId() const;

        void setRights(uint8_t rights);
        uint8_t getRights() const;

        void setSlots(uint32_t slots);
        uint32_t getSlots() const;

    private:

        uint8_t mTabId;
        uint8_t mRights;
        uint32_t mSlots;
};
