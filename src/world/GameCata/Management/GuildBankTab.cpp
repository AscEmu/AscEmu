/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Guild.h"
#include "GuildBankTab.h"
#include "Server/MainServerDefines.h"
#include "Management/Item.h"
#include "Objects/ObjectMgr.h"


GuildBankTab::GuildBankTab(uint32_t guildId, uint8_t tabId) : mGuildId(guildId), mTabId(tabId)
{
    memset(mItems, 0, MAX_GUILD_BANK_SLOTS * sizeof(Item*));
}

void GuildBankTab::loadGuildBankTabFromDB(Field* fields)
{
    mName = fields[2].GetString();
    mIcon = fields[3].GetString();
    mText = fields[4].GetString();
}

bool GuildBankTab::loadGuildBankTabItemFromDB(Field* fields)
{
    uint8_t slotId = fields[2].GetUInt8();
    // uint32_t itemGuid = fields[3].GetUInt32();

    Item* pItem = objmgr.LoadItem(fields[3].GetUInt32());
    if (pItem == nullptr)
    {
        CharacterDatabase.Execute("DELETE FROM guild_bankitems WHERE itemGuid = %u AND guildId = %u AND tabId = %u",
            fields[3].GetUInt32(), mGuildId, (uint32_t)fields[1].GetUInt8());
    }

    mItems[slotId] = pItem;
    return true;
}

void GuildBankTab::removeBankTabItemFromDB(bool removeItemsFromDB)
{
    for (uint8_t slotId = 0; slotId < MAX_GUILD_BANK_SLOTS; ++slotId)
    {
        if (Item* pItem = mItems[slotId])
        {
            pItem->RemoveFromWorld();
            if (removeItemsFromDB)
            {
                pItem->DeleteFromDB();
            }

            delete pItem;
            pItem = nullptr;
        }
    }
}

void GuildBankTab::writeInfoPacket(WorldPacket& data) const
{
    data << mName;
    data << mIcon;
}

void GuildBankTab::setInfo(std::string const& name, std::string const& icon)
{
    if (mName == name && mIcon == icon)
    {
        return;
    }

    mName = name;
    mIcon = icon;

    CharacterDatabase.Execute("UPDATE guild_bank_tab SET tabName = '%s' , tabIcon = '%s' WHERE guildId = '%u' AND tabId = '%u' ",
        mName.c_str(), mIcon.c_str(), mGuildId, (uint32_t)mTabId);
}

void GuildBankTab::setText(std::string const& text)
{
    if (mText == text)
    {
        return;
    }

    mText = text;

    CharacterDatabase.Execute("UPDATE guild_bank_tab SET tabText = '%s' , tabIcon = '%s' WHERE guildId = %u AND tabId = %u ",
        mText.c_str(), mIcon.c_str(), mGuildId, (uint32_t)mTabId);
}

void GuildBankTab::sendText(Guild const* guild, WorldSession* session) const
{
    WorldPacket data(SMSG_GUILD_BANK_QUERY_TEXT_RESULT, 1 + mText.size() + 1);
    data.writeBits(mText.length(), 14);
    data << uint32_t(mTabId);
    data.WriteString(mText);

    if (session)
    {
        LogDebugFlag(LF_OPCODE, "SMSG_GUILD_BANK_QUERY_TEXT_RESULT %s: Tabid: %u, Text: %s",
            session->GetPlayer()->GetName(), (uint32_t)mTabId, mText.c_str());
        session->SendPacket(&data);
    }
    else
    {
        LogDebugFlag(LF_OPCODE, "SMSG_GUILD_BANK_QUERY_TEXT_RESULT [Broadcast]: Tabid: %u, Text: %s", (uint32_t)mTabId, mText.c_str());
        guild->broadcastPacket(&data);
    }
}

std::string const& GuildBankTab::getName() const
{
    return mName;
}

std::string const& GuildBankTab::getIcon() const
{
    return mIcon;
}

std::string const& GuildBankTab::getText() const
{
    return mText;
}

Item* GuildBankTab::getItem(uint8_t slotId) const
{
    return slotId < MAX_GUILD_BANK_SLOTS ? mItems[slotId] : nullptr;
}

bool GuildBankTab::setItem(uint8_t slotId, Item* item)
{
    if (slotId >= MAX_GUILD_BANK_SLOTS && slotId != UNDEFINED_TAB_SLOT)
    {
        return false;
    }

    if (item != nullptr)
    {
        uint32_t slot_id = 0;
        if (slotId == 0 || slotId == UNDEFINED_TAB_SLOT)
        {
            for (int i = 0; i < MAX_GUILD_BANK_SLOTS; ++i)
            {
                if (mItems[i] == nullptr)
                {
                    slot_id = i;
                    mItems[i] = item;
                    break;
                }
            }
        }

        CharacterDatabase.Execute("INSERT INTO guild_bank_item VALUES (%u, %u, %u, %u)", mGuildId, (uint32_t)mTabId, slot_id, item->GetLowGUID());
    }
    else
    {
        CharacterDatabase.Execute("DELETE FROM guild_bank_item WHERE guildId = %u AND tabId = %u AND slotId = %u", mGuildId, (uint32_t)mTabId, (uint32_t)slotId);
    }

    return true;
}
