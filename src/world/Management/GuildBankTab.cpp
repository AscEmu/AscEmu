/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Guild.h"
#include "GuildBankTab.h"
#include "Server/MainServerDefines.h"
#include "Management/Item.h"
#include "Objects/ObjectMgr.h"
#include "Server/Packets/MsgQueryGuildBankText.h"
#include "Server/Packets/SmsgGuildBankQueryTextResult.h"

using namespace AscEmu::Packets;

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

    Item* pItem = objmgr.LoadItem(fields[3].GetUInt32());
    if (pItem == nullptr)
    {
        CharacterDatabase.Execute("DELETE FROM guild_bank_items WHERE itemGuid = %u AND guildId = %u AND tabId = %u",
            fields[3].GetUInt32(), mGuildId, static_cast<uint32_t>(fields[1].GetUInt8()));
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
                pItem->DeleteFromDB();

            delete pItem;
            pItem = nullptr;
        }
    }
}

void GuildBankTab::writeInfoPacket(WorldPacket& data) const
{
#if VERSION_STRING < Cata
    uint8_t count = 0;

    size_t pos = data.wpos();
    data << uint8_t(0);

    for (uint8_t slotId = 0; slotId < MAX_GUILD_BANK_SLOTS; ++slotId)
        if (writeSlotPacket(data, slotId))
            ++count;

    data.put<uint8_t>(pos, count);
#else

    data << mName;
    data << mIcon;
#endif
}

bool GuildBankTab::writeSlotPacket(WorldPacket& data, uint8_t slotId, bool ignoreEmpty) const
{
    Item* pItem = getItem(slotId);
    const uint32_t itemEntry = pItem ? pItem->getEntry() : 0;

    if (!itemEntry && ignoreEmpty)
        return false;

    data << uint8_t(slotId);
    data << uint32_t(itemEntry);
    if (itemEntry)
    {
        data << uint32(0);

        if (uint32_t random = pItem->getRandomPropertiesId())
        {
            data << uint32_t(random);
            data << uint32_t(pItem->GenerateRandomSuffixFactor(pItem->getItemProperties()));
        }
        else
        {
            data << uint32_t(0);
        }

        data << uint32_t(pItem->getStackCount());
        data << uint32_t(0);
        data << uint8_t(abs(pItem->getSpellCharges(0)));

        uint8_t enchCount = 0;
        size_t enchCountPos = data.wpos();

        data << uint8_t(enchCount);
        for (uint32_t i = PERM_ENCHANTMENT_SLOT; i < MAX_ENCHANTMENT_SLOT; ++i)
        {
            if (uint32_t enchId = pItem->getEnchantmentId(static_cast<uint8_t>(EnchantmentSlot(i))))
            {
                data << uint8_t(i);
                data << uint32_t(enchId);
                ++enchCount;
            }
        }
        data.put<uint8_t>(enchCountPos, enchCount);
    }
    return true;
}

void GuildBankTab::setInfo(std::string const& name, std::string const& icon)
{
    if (mName == name && mIcon == icon)
        return;

    mName = name;
    mIcon = icon;

    CharacterDatabase.Execute("UPDATE guild_bank_tabs SET tabName = '%s' , tabIcon = '%s' WHERE guildId = '%u' AND tabId = '%u' ",
        mName.c_str(), mIcon.c_str(), mGuildId, static_cast<uint32_t>(mTabId));
}

void GuildBankTab::setText(std::string const& text)
{
    if (mText == text)
        return;

    mText = text;

    CharacterDatabase.Execute("UPDATE guild_bank_tabs SET tabText = '%s' , tabIcon = '%s' WHERE guildId = %u AND tabId = %u ",
        mText.c_str(), mIcon.c_str(), mGuildId, static_cast<uint32_t>(mTabId));
}

void GuildBankTab::sendText(Guild const* guild, WorldSession* session) const
{
    if (session)
        LogDebugFlag(LF_OPCODE, "sendText %s: Tabid: %u, Text: %s", session->GetPlayer()->getName().c_str(), static_cast<uint32_t>(mTabId), mText.c_str());
    else
        LogDebugFlag(LF_OPCODE, "sendText (Broadcast): Tabid: %u, Text: %s", static_cast<uint32_t>(mTabId), mText.c_str());

#if VERSION_STRING < Cata
    if (session)
        session->SendPacket(MsgQueryGuildBankText(mTabId, mText).serialise().get());
    else
        guild->broadcastPacket(MsgQueryGuildBankText(mTabId, mText).serialise().get());
#else
    if (session)
        session->SendPacket(SmsgGuildBankQueryTextResult(mTabId, mText).serialise().get());
    else
        guild->broadcastPacket(SmsgGuildBankQueryTextResult(mTabId, mText).serialise().get());
#endif
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
        return false;

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

        CharacterDatabase.Execute("INSERT INTO guild_bank_items VALUES (%u, %u, %u, %u)", mGuildId, static_cast<uint32_t>(mTabId), slot_id, item->getGuidLow());
    }
    else
    {
        mItems[slotId] = nullptr;
        CharacterDatabase.Execute("DELETE FROM guild_bank_items WHERE guildId = %u AND tabId = %u AND slotId = %u", mGuildId, static_cast<uint32_t>(mTabId), static_cast<uint32_t>(slotId));
    }

    return true;
}
