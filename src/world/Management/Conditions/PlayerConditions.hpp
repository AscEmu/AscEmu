/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <cstdint>
#include <set>

class Player;

namespace Conditions
{
    // a PlayerCondition row of the client data, the fields of every client version in one shape
    struct PlayerConditionData
    {
        uint32_t id = 0;
        uint32_t flags = 0;
        int32_t minLevel = 0;
        int32_t maxLevel = 0;
        uint64_t raceMask = 0;
        uint32_t classMask = 0;
        int32_t gender = -1;
        int32_t nativeGender = -1;

        std::array<uint32_t, 4> skill{};
        std::array<uint32_t, 4> minSkill{};
        std::array<uint32_t, 4> maxSkill{};
        uint32_t skillLogic = 0;

        uint32_t languageId = 0;
        int32_t minLanguage = 0;
        int32_t maxLanguage = 0;

        std::array<uint32_t, 3> minFaction{};
        std::array<uint32_t, 3> minReputation{};
        uint32_t maxFaction = 0;
        uint32_t maxReputation = 0;
        uint32_t reputationLogic = 0;

        int32_t minPvpRank = 0;
        int32_t maxPvpRank = 0;
        int32_t pvpMedal = 0;
        int32_t currentPvpFaction = 0;
        int32_t lifetimeMaxPvpRank = 0;

        std::array<uint32_t, 4> prevQuest{};
        uint32_t prevQuestLogic = 0;
        std::array<uint32_t, 4> currQuest{};
        uint32_t currQuestLogic = 0;
        std::array<uint32_t, 4> currCompletedQuest{};
        uint32_t currCompletedQuestLogic = 0;

        std::array<uint32_t, 4> spell{};
        uint32_t spellLogic = 0;

        std::array<uint32_t, 4> item{};
        std::array<uint32_t, 4> itemCount{};
        uint32_t itemFlags = 0;
        uint32_t itemLogic = 0;

        std::array<uint32_t, 2> explored{};
        std::array<uint32_t, 2> time{};

        std::array<uint32_t, 4> aura{};
        std::array<uint32_t, 4> auraStacks{};
        uint32_t auraLogic = 0;

        uint32_t worldStateExpression = 0;
        uint32_t weather = 0;
        uint32_t partyStatus = 0;

        std::array<uint32_t, 4> achievement{};
        uint32_t achievementLogic = 0;

        std::array<uint32_t, 4> lfgStatus{};
        std::array<uint32_t, 4> lfgCompare{};
        std::array<uint32_t, 4> lfgValue{};
        uint32_t lfgLogic = 0;

        std::array<uint32_t, 4> area{};
        uint32_t areaLogic = 0;

        std::array<uint32_t, 4> currency{};
        std::array<uint32_t, 4> currencyCount{};
        uint32_t currencyLogic = 0;

        uint32_t questKill = 0;
        uint32_t questKillLogic = 0;
        std::array<uint32_t, 6> questKillMonster{};

        int32_t minExpansion = -1;
        int32_t maxExpansion = -1;
        int32_t minExpansionTier = -1;
        int32_t maxExpansionTier = -1;
        int32_t minGuildLevel = 0;
        int32_t maxGuildLevel = 0;

        uint32_t phaseUseFlags = 0;
        uint32_t phaseId = 0;
        uint32_t phaseGroupId = 0;

        int32_t minAvgItemLevel = 0;
        int32_t maxAvgItemLevel = 0;
        int32_t minAvgEquippedItemLevel = 0;
        int32_t maxAvgEquippedItemLevel = 0;

        int32_t specializationIndex = -1;
        int32_t specializationRole = -1;

        int32_t powerType = -1;
        int32_t powerCompare = 0;
        int32_t powerValue = 0;

        uint32_t movementFlags = 0;
        uint32_t weaponSubclassMask = 0;
        uint32_t modifierTree = 0;
        uint32_t contentTuning = 0;
        uint32_t covenant = 0;
        std::array<int32_t, 4> traitNode{};
    };

    // a realm with its own db2 tables adds the rows, every other realm reads the dbc store
    void addPlayerCondition(PlayerConditionData data);
    void initializePlayerConditions();

    // false for a row that is missing or names something the core can not check
    bool isPlayerConditionMet(Player* player, uint32_t conditionId);

    // names the missing rows and the fields the core can not check of the conditions in use
    void reportPlayerConditions(std::set<uint32_t> const& conditionIds);
}
