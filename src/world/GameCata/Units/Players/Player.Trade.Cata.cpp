/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Units/Players/Player.h"
#include "Server/WorldSession.h"
#include "Management/ItemInterface.h"

void Player::cancelTrade(bool sendback)
{
    if (m_TradeData)
    {
        auto trade_target = m_TradeData->getTradeTarget();

        if (sendback || trade_target == nullptr)
        {
            GetSession()->sendTradeCancel();
            delete m_TradeData;
            m_TradeData = nullptr;
        }
        else
        {
            trade_target->GetSession()->sendTradeCancel();
            delete trade_target->m_TradeData;
            trade_target->m_TradeData = nullptr;
        }
    }
}

TradeData* TradeData::getTargetTradeData() const
{
    return m_tradeTarget->getTradeData();
}

Item* TradeData::getTradeItem(TradeSlots slot) const
{
    return m_items[slot] ? m_player->GetItemInterface()->GetItemByGUID(m_items[slot]) : nullptr;
}

bool TradeData::hasTradeItem(uint64 item_guid) const
{
    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (m_items[i] == item_guid)
        {
            return true;
        }
    }

    return false;
}

Item* TradeData::getSpellCastItem() const
{
    return m_spellCastItem ? m_player->GetItemInterface()->GetItemByGUID(m_spellCastItem) : nullptr;
}

void TradeData::setItem(TradeSlots slot, Item* item)
{
    ObjectGuid itemGuid;

    if (item)
    {
        itemGuid = item->GetGUID();
    }
    else
    {
        itemGuid = ObjectGuid();
    }

    if (m_items[slot] == itemGuid)
    {
        return;
    }

    m_items[slot] = itemGuid;

    setAccepted(false);
    getTargetTradeData()->setAccepted(false);

    updateTrade();
}

void TradeData::setSpell(uint32_t spell_id, Item* cast_item /*= nullptr*/)
{
    ObjectGuid itemGuid;

    if (cast_item)
    {
        itemGuid = cast_item->GetGUID();
    }
    else
    {
        itemGuid = ObjectGuid();
    }

    if (m_spell == spell_id && m_spellCastItem == itemGuid)
    {
        return;
    }

    m_spell = spell_id;
    m_spellCastItem = itemGuid;

    setAccepted(false);
    getTargetTradeData()->setAccepted(false);

    updateTrade(true); // spell info to owner
    updateTrade(false); // spell info to caster
}
