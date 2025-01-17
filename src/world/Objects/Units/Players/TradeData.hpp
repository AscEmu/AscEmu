/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "PlayerDefines.hpp"
#include <cstdint>

class Player;

class TradeData
{
private:
    Player* m_player;
    Player* m_tradeTarget;
    bool m_accepted;
    uint64_t m_money;
    uint32_t m_spell;
    uint64_t m_spellCastItem;
    uint64_t m_items[TRADE_SLOT_COUNT];

public:
    TradeData(Player* player, Player* trader);

    Player* getTradeTarget() const;
    TradeData* getTargetTradeData() const;

    Item* getTradeItem(TradeSlots slot) const;
    bool hasTradeItem(uint64_t itemGuid) const;
    bool hasPlayerOrTraderItemInTrade(uint64_t itemGuid) const;

    uint32_t getSpell() const;
    Item* getSpellCastItem() const;
    bool hasSpellCastItem() const;

    uint64_t getTradeMoney() const;
    void setTradeMoney(uint64_t money);

    void setTradeAccepted(bool state, bool sendBoth = false);
    bool isTradeAccepted() const;

    void setTradeItem(TradeSlots slot, Item* item);
    void setTradeSpell(uint32_t spell_id, Item* cast_item = nullptr);
};
