/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "WoWGuid.hpp"

class Field;

class Charter
{
public:
    Charter(Field const* _field);
    Charter(uint32_t _id, const WoWGuid& _leaderGuid, uint8_t _type);
    ~Charter();

    void saveToDB();
    void destroy();

    const WoWGuid& getLeaderGuid() const;

    uint32_t getId() const;

    uint8_t getCharterType() const;

    std::string getGuildName();
    void setGuildName(const std::string& _guildName);

    const WoWGuid& getItemGuid() const;
    void setItemGuid(const WoWGuid& _itemGuid);

    uint8_t getNumberOfAvailableSlots() const;
    bool isFull() const;
    uint8_t getAvailableSlots() const;

    void addSignature(const WoWGuid& _playerGuid);
    void removeSignature(const WoWGuid& _playerGuid);
    uint8_t getSignatureCount() const;
    const std::vector<WoWGuid>& getSignatures() const;

    uint32_t m_petitionSignerCount = 0;

private:
    uint32_t m_charterId = 0;
    uint8_t m_charterType = 0;

    WoWGuid m_leaderGuid;
    std::string m_guildName;
    WoWGuid m_itemGuid;

    uint8_t m_availableSlots = 0;

    std::vector<WoWGuid> m_signatures;
};
