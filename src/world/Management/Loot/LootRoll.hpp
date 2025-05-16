/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/EventableObject.h"

class Player;
class WorldMap;

class LootRoll : public EventableObject
{
public:
    friend void LootItem::playerRolled(Player*, uint8_t);

    LootRoll(uint32_t timer, uint32_t groupcount, uint64_t guid, uint8_t slotid, uint32_t itemid, uint32_t randomsuffixid, uint32_t randompropertyid, WorldMap* mgr);
    ~LootRoll();

private:
    // player rolled on the item
    // returns true/false if roll can be deleted
    bool playerRolled(Player* player, uint8_t choice);

    // finish roll for item
    void finalize();

    std::map<uint32_t, uint8_t> m_NeedRolls;
    std::map<uint32_t, uint8_t> m_GreedRolls;
    std::set<uint32_t> m_passRolls;
    uint32_t _groupcount = 0;
    uint8_t _slotid = 0;
    uint32_t _itemid = 0;
    uint32_t _randomsuffixid = 0;
    uint32_t _randompropertyid = 0;
    uint32_t _remaining = 0;
    uint64_t _guid = 0;
    WorldMap* _mgr = nullptr;
};
