/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Field;

class Charter
{
public:
    Charter(Field const* _field);
    Charter(uint32_t _id, uint32_t _leaderGuid, uint8_t _type);
    ~Charter();

    void saveToDB();
    void destroy();

    uint32_t getLeaderGuid() const;

    uint32_t getId() const;

    uint8_t getCharterType() const;

    std::string getGuildName();
    void setGuildName(const std::string& _guildName);

    uint64_t getItemGuid() const;
    void setItemGuid(uint64_t _itemGuid);

    uint8_t getNumberOfAvailableSlots() const;
    bool isFull() const;
    uint8_t getAvailableSlots() const;

    void addSignature(uint32_t _playerGuid);
    void removeSignature(uint32_t _playerGuid);
    uint8_t getSignatureCount() const;
    std::vector<uint32_t> getSignatures();

    uint32_t m_petitionSignerCount = 0;

private:
    uint32_t m_charterId = 0;
    uint8_t m_charterType = 0;

    uint32_t m_leaderGuid = 0;
    std::string m_guildName;
    uint64_t m_itemGuid = 0;

    uint8_t m_availableSlots = 0;

    std::vector<uint32_t> m_signatures;
};
