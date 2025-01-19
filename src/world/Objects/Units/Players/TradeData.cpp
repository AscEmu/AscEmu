/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Player.hpp"
#include "TradeData.hpp"
#include "Management/ItemInterface.h"
#include "Objects/Item.hpp"
#include "Server/WorldSession.h"

TradeData::TradeData(Player* player, Player* trader)
{
    m_player = player;
    m_tradeTarget = trader;

    m_accepted = false;

    m_money = 0;
    m_spell = 0;
    m_spellCastItem = 0;
    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
        m_items[i] = 0;
}

Player* TradeData::getTradeTarget() const
{
    return m_tradeTarget;
}

TradeData* TradeData::getTargetTradeData() const
{
    return m_tradeTarget->getTradeData();
}

Item* TradeData::getTradeItem(TradeSlots slot) const
{
    return m_items[slot] != 0 ? m_player->getItemInterface()->GetItemByGUID(m_items[slot]) : nullptr;
}

bool TradeData::hasTradeItem(uint64_t itemGuid) const
{
    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (m_items[i] == itemGuid)
            return true;
    }

    return false;
}

bool TradeData::hasPlayerOrTraderItemInTrade(uint64_t itemGuid) const
{
    for (uint8_t i = 0; i < TRADE_SLOT_COUNT; ++i)
    {
        if (m_items[i] == itemGuid)
            return true;
        if (getTargetTradeData()->m_items[i] == itemGuid)
            return true;
    }

    return false;
}

uint32_t TradeData::getSpell() const
{
    return m_spell;
}

Item* TradeData::getSpellCastItem() const
{
    return hasSpellCastItem() ? m_player->getItemInterface()->GetItemByGUID(m_spellCastItem) : nullptr;
}

bool TradeData::hasSpellCastItem() const
{
    return m_spellCastItem != 0;
}

uint64_t TradeData::getTradeMoney() const
{
    return m_money;
}

void TradeData::setTradeMoney(uint64_t money)
{
    if (m_money == money)
        return;

    if (money > m_player->getCoinage())
        return;

    m_money = money;

    setTradeAccepted(false);
    getTargetTradeData()->setTradeAccepted(false);

    // Send update packet to trader
    m_tradeTarget->getSession()->sendTradeUpdate(true);
}

void TradeData::setTradeAccepted(bool state, bool sendBoth/* = false*/)
{
    m_accepted = state;

    if (!state)
    {
        if (sendBoth)
            m_tradeTarget->getSession()->sendTradeResult(TRADE_STATUS_STATE_CHANGED);
        else
            m_player->getSession()->sendTradeResult(TRADE_STATUS_STATE_CHANGED);
    }
}

bool TradeData::isTradeAccepted() const
{
    return m_accepted;
}

void TradeData::setTradeItem(TradeSlots slot, Item* item)
{
    const auto itemGuid = item != nullptr ? item->getGuid() : 0;
    if (m_items[slot] == itemGuid)
        return;

    m_items[slot] = itemGuid;

    setTradeAccepted(false);
    getTargetTradeData()->setTradeAccepted(false);

    // Send update packet to trader
    m_tradeTarget->getSession()->sendTradeUpdate(true);
}

void TradeData::setTradeSpell(uint32_t spell_id, Item* castItem /*= nullptr*/)
{
    const auto itemGuid = castItem != nullptr ? castItem->getGuid() : 0;
    if (m_spell == spell_id && m_spellCastItem == itemGuid)
        return;

    m_spell = spell_id;
    m_spellCastItem = itemGuid;

    setTradeAccepted(false);
    getTargetTradeData()->setTradeAccepted(false);

    // Send update packet to both parties
    m_player->getSession()->sendTradeUpdate(false);
    m_tradeTarget->getSession()->sendTradeUpdate(true);
}
