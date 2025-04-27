/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Macros/GuildMacros.hpp"

#include <array>

class Item;
class WorldSession;

class GuildBankTab
{
public:
    GuildBankTab(uint32_t guildId, uint8_t tabId);

    void loadGuildBankTabFromDB(Field* fields);
    bool loadGuildBankTabItemFromDB(Field* fields);
    void removeBankTabItemFromDB(bool removeItemsFromDB = false);

    void writeInfoPacket(WorldPacket& data) const;
    bool writeSlotPacket(WorldPacket& data, uint8_t slotId, bool ignoreEmpty = true) const;

    void setInfo(std::string const& name, std::string const& icon);
    void setText(std::string const& text);
    void sendText(Guild const* guild, WorldSession* session) const;

    std::string const& getName() const;
    std::string const& getIcon() const;
    std::string const& getText() const;

    Item* getItem(uint8_t slotId) const;
    std::unique_ptr<Item> getItemHolder(uint8_t slotId);
    bool setItem(uint8_t slotId, std::unique_ptr<Item> item);

private:
    uint32_t mGuildId;
    uint8_t mTabId;

    std::array<std::unique_ptr<Item>, MAX_GUILD_BANK_SLOTS> mItems;
    std::string mName;
    std::string mIcon;
    std::string mText;
};
