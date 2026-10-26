/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <vector>

namespace Achievements
{
    // the start and fail events of the client data, the values of 5.4 and later
    enum StartTrigger : uint8_t
    {
        StartReachLevel             = 1,
        StartCompleteDailyQuest     = 2,
        StartBattleground           = 3,
        StartGainAura               = 5,
        StartGainAuraEffect         = 6,
        StartCastSpell              = 7,
        StartBeSpellTarget          = 8,
        StartAcceptQuest            = 9,
        StartKillCreature           = 10,
        StartKillPlayer             = 11,
        StartUseItem                = 12,
        StartSendEvent              = 13,
        StartUnknown                = 0xFF  // a start event the core does not know, its criteria never start
    };

    enum FailTrigger : uint8_t
    {
        FailDeath                   = 1,
        FailLeaveBattleground       = 3,
        FailLoseAura                = 5,
        FailGainAura                = 6,
        FailGainAuraEffect          = 7,
        FailCastSpell               = 8,
        FailBeSpellTarget           = 9,
        FailPartyStatus             = 10,
        FailDailyQuestsCleared      = 13,
        FailSendEvent               = 14
    };
}

class Object;
class Player;
class Unit;

namespace Achievements
{
    struct Criteria;

    enum class ProgressChange : uint8_t
    {
        Add,
        Set,
        Highest
    };

    // what the conditions of a criteria may ask about the event
    struct EventSubject
    {
        Unit* target = nullptr;
        uint32_t itemId = 0;
        uint64_t miscValue1 = 0;
        uint64_t miscValue2 = 0;
        uint64_t eventValue = 0;        // what a value requirement of the world database compares
    };

    struct ProgressUpdate
    {
        ProgressChange change = ProgressChange::Add;
        uint64_t value = 0;
        EventSubject subject;
    };

    // a game event updates the criteria type
    bool isCriteriaTypeTracked(uint16_t type);

    // the criteria types of the 5.4 and later client data a game event of the core updates
    std::vector<uint16_t> const& getCriteriaTypesOfEvent(uint16_t eventType);

    // whether the event concerns the criteria and how its progress changes
    bool resolveEvent(Player* player, Criteria const& criteria, int32_t miscValue1, int32_t miscValue2, Object* reference, ProgressUpdate& update);

    // a game event of the core sends it
    bool isStartEventHooked(uint8_t event);
    bool isFailEventHooked(uint8_t event);

    // the start event resets the progress when it comes again
    constexpr uint32_t CriteriaFlagResetOnStart = 0x02;

    // types the core derives from the player at login and after every change, never stored
    bool isDerivedType(uint16_t type);
    bool deriveProgress(Player* player, Criteria const& criteria, uint64_t& value);
}
