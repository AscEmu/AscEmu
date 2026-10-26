/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "PlayerConditions.hpp"

#include "WorldStateExpressions.hpp"
#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Management/AchievementMgr.h"
#include "Management/Achievements/AchievementConditions.hpp"
#include "Management/Achievements/AchievementTrees.hpp"
#include "Management/Group.h"
#include "Management/ItemInterface.h"
#include "Management/LFG/LFGMgr.hpp"
#include "Management/QuestLogEntry.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Objects/Item.hpp"
#include "Objects/ItemDefines.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

#include <bitset>
#include <ctime>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace Conditions
{
    namespace
    {
        constexpr uint32_t FlagCheckAllCharacters  = 0x0002;
        constexpr uint32_t FlagComparePowerToMax   = 0x0004;
        constexpr uint32_t FlagInvert              = 0x0008;
        constexpr uint32_t FlagDisabled            = 0x0100;
        constexpr uint32_t FlagInvertModifierTree  = 0x0200;

        std::unordered_map<uint32_t, PlayerConditionData> conditions;

        // the parts of a row the core has no system for, a row naming one is not decided
        std::vector<std::string> unsupportedParts(PlayerConditionData const& data)
        {
            std::vector<std::string> parts;
            if (data.languageId != 0)
                parts.emplace_back("language");
            if (data.minPvpRank != 0 || data.maxPvpRank != 0 || data.pvpMedal != 0 || data.lifetimeMaxPvpRank != 0)
                parts.emplace_back("pvp rank");
            if (data.weather != 0)
                parts.emplace_back("weather");
            if (data.questKill != 0)
                parts.emplace_back("quest kill");
            if (data.minExpansionTier != -1 || data.maxExpansionTier != -1)
                parts.emplace_back("expansion tier");
            if (data.minGuildLevel != 0 || data.maxGuildLevel != 0)
                parts.emplace_back("guild level");
            if (data.phaseUseFlags != 0 || data.phaseId != 0 || data.phaseGroupId != 0)
                parts.emplace_back("phase");
            if (data.minAvgItemLevel != 0 || data.maxAvgItemLevel != 0 || data.minAvgEquippedItemLevel != 0 || data.maxAvgEquippedItemLevel != 0)
                parts.emplace_back("average item level");
            if (data.specializationRole >= 0)
                parts.emplace_back("specialization role");
            if (data.movementFlags != 0)
                parts.emplace_back("movement");
            if (data.contentTuning != 0)
                parts.emplace_back("content tuning");
            if (data.covenant != 0)
                parts.emplace_back("covenant");
            for (int32_t const node : data.traitNode)
                if (node != 0)
                {
                    parts.emplace_back("talent trait");
                    break;
                }
            for (uint32_t const status : data.lfgStatus)
                if (status > 2)
                {
                    parts.emplace_back("lfg status");
                    break;
                }
            // the other characters of the account are not loaded, their own achievements are unknown
            if (data.flags & FlagCheckAllCharacters)
                for (uint32_t const achievementId : data.achievement)
                    if (auto const* achievement = achievementId != 0 ? sAchievementStore.lookupEntry(achievementId) : nullptr)
                        if ((achievement->flags & Achievements::AchievementFlagAccount) == 0)
                        {
                            parts.emplace_back("achievements of other characters");
                            break;
                        }
            return parts;
        }

        // the results of up to 4 slots joined by the logic of the row: bit 16 and up invert, then 2 bits per slot
        template <size_t N>
        bool joinResults(uint32_t logic, std::bitset<N> results)
        {
            uint32_t const values = static_cast<uint32_t>(results.to_ulong()) ^ (logic >> 16);
            uint32_t result = values & 1;
            for (size_t i = 1; i < N; ++i)
            {
                uint32_t const value = (values >> i) & 1;
                switch ((logic >> (2 * (i - 1))) & 3)
                {
                    case 1: result &= value; break;
                    case 2: result |= value; break;
                    default: break;
                }
            }
            return result != 0;
        }

        template <size_t N, typename Check>
        bool checkSlots(std::array<uint32_t, N> const& ids, uint32_t logic, Check&& check)
        {
            std::bitset<N> results;
            for (size_t i = 0; i < N; ++i)
                results[i] = ids[i] == 0 || check(i);
            return joinResults(logic, results);
        }

        bool compareValues(int32_t comparison, int32_t left, int32_t right)
        {
            switch (comparison)
            {
                case 1: return left == right;
                case 2: return left != right;
                case 3: return left > right;
                case 4: return left >= right;
                case 5: return left < right;
                case 6: return left <= right;
                default: return false;
            }
        }

        bool isInArea(uint32_t areaId, uint32_t wanted)
        {
            for (uint8_t depth = 0; areaId != 0 && depth < 8; ++depth)
            {
                if (areaId == wanted)
                    return true;
                auto const* area = MapManagement::AreaManagement::AreaStorage::getAreaById(areaId);
                areaId = area != nullptr ? area->zone : 0;
            }
            return false;
        }

        // the packed time of the client: minute, hour, weekday, day, month, year since 2000, -1 for any
        bool matchesTime(uint32_t packed, tm const& now)
        {
            auto part = [packed](uint32_t shift, uint32_t mask) { return static_cast<int32_t>((packed >> shift) & mask); };
            auto any = [](int32_t value, int32_t mask) { return value == mask; };

            int32_t const minute = part(0, 0x3F), hour = part(6, 0x1F), weekday = part(11, 0x7), day = part(14, 0x3F), month = part(20, 0xF), year = part(24, 0x1F);
            return (any(minute, 0x3F) || minute == now.tm_min) && (any(hour, 0x1F) || hour == now.tm_hour)
                && (any(weekday, 0x7) || weekday == now.tm_wday) && (any(day, 0x3F) || day == now.tm_mday - 1)
                && (any(month, 0xF) || month == now.tm_mon) && (any(year, 0x1F) || year == now.tm_year - 100);
        }

        time_t unpackTime(uint32_t packed)
        {
            tm parts{};
            parts.tm_min = static_cast<int>(packed & 0x3F);
            parts.tm_hour = static_cast<int>((packed >> 6) & 0x1F);
            parts.tm_mday = static_cast<int>(((packed >> 14) & 0x3F) + 1);
            parts.tm_mon = static_cast<int>((packed >> 20) & 0xF);
            parts.tm_year = static_cast<int>(((packed >> 24) & 0x1F) + 100);
            parts.tm_isdst = -1;
            return std::mktime(&parts);
        }

        uint32_t reputationRankOf(Player* player, uint32_t factionId)
        {
            return static_cast<uint32_t>(player->getFactionStandingRank(factionId));
        }

        uint32_t lfgValueOf(Player* player, uint32_t status)
        {
            if (player->getGroup() == nullptr || sLfgMgr.GetState(player->getGuid()) != LFG_STATE_DUNGEON)
                return 0;

            if (status == 1)
                return 1;

            for (uint32_t const dungeonId : sLfgMgr.GetSelectedDungeons(player->getGuid()))
                if (auto const* dungeon = sLFGDungeonStore.lookupEntry(dungeonId & 0x00FFFFFF))
                    if (dungeon->type == LFG_TYPE_RANDOM)
                        return 1;
            return 0;
        }

        bool meetsParty(Player* player, uint32_t status)
        {
            Group* group = player->getGroup();
            switch (status)
            {
                case 1: return group == nullptr;
                case 2: return group != nullptr;
                case 3: return group != nullptr && !group->isRaid();
                case 4: return group != nullptr && group->isRaid();
                case 5: return group == nullptr || !group->isRaid();
                default: return false;
            }
        }

        bool isMet(Player* player, PlayerConditionData const& data)
        {
            if (data.flags & FlagDisabled)
                return true;

            if (!unsupportedParts(data).empty())
                return false;

            uint32_t const level = player->getLevel();
            if (data.minLevel != 0 && level < static_cast<uint32_t>(data.minLevel))
                return false;
            if (data.maxLevel != 0 && level > static_cast<uint32_t>(data.maxLevel))
                return false;

            if (data.raceMask != 0 && (player->getRace() == 0 || player->getRace() > 32 || (data.raceMask & (uint64_t(1) << (player->getRace() - 1))) == 0))
                return false;
            if (data.classMask != 0 && (data.classMask & (1u << (player->getClass() - 1))) == 0)
                return false;
            if (data.gender >= 0 && player->getGender() != static_cast<uint32_t>(data.gender))
                return false;
            if (data.nativeGender >= 0 && player->getGender() != static_cast<uint32_t>(data.nativeGender))
                return false;

            if (data.powerType >= 0 && data.powerCompare != 0)
            {
                auto const power = static_cast<PowerType>(data.powerType);
                int32_t const wanted = (data.flags & FlagComparePowerToMax) ? static_cast<int32_t>(player->getMaxPower(power)) : data.powerValue;
                if (!compareValues(data.powerCompare, static_cast<int32_t>(player->getPower(power)), wanted))
                    return false;
            }

            if (data.specializationIndex >= 0)
            {
                auto const* specialization = sChrSpecializationStore.lookupEntry(player->getActiveSpec().getSpecializationId());
                if (specialization == nullptr || static_cast<int32_t>(specialization->tabPage) != data.specializationIndex)
                    return false;
            }

            if (!checkSlots(data.skill, data.skillLogic, [&](size_t i)
            {
                uint32_t const value = player->getSkillLineCurrent(static_cast<uint16_t>(data.skill[i]), false);
                return value != 0 && value > data.minSkill[i] && value < data.maxSkill[i];
            }))
                return false;

            if (data.minFaction[0] != 0 || data.minFaction[1] != 0 || data.minFaction[2] != 0 || data.maxFaction != 0)
            {
                auto maxMet = [&] { return data.maxFaction == 0 || sFactionStore.lookupEntry(data.maxFaction) == nullptr || reputationRankOf(player, data.maxFaction) <= data.maxReputation; };
                if (data.minFaction[0] == 0 && data.minFaction[1] == 0 && data.minFaction[2] == 0)
                {
                    if (!maxMet())
                        return false;
                }
                else
                {
                    std::bitset<4> results;
                    for (size_t i = 0; i < 3; ++i)
                        results[i] = data.minFaction[i] == 0 || sFactionStore.lookupEntry(data.minFaction[i]) == nullptr || reputationRankOf(player, data.minFaction[i]) >= data.minReputation[i];
                    results[3] = maxMet();
                    if (!joinResults(data.reputationLogic, results))
                        return false;
                }
            }

            if (data.currentPvpFaction != 0 && data.currentPvpFaction - 1 != (player->isTeamAlliance() ? 1 : 0))
                return false;

            if (data.weaponSubclassMask != 0)
            {
                Item* mainHand = player->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                if (mainHand == nullptr || ((1u << mainHand->getItemProperties()->SubClass) & data.weaponSubclassMask) == 0)
                    return false;
            }

            if (data.partyStatus != 0 && !meetsParty(player, data.partyStatus))
                return false;

            if (data.prevQuest[0] != 0 && !checkSlots(data.prevQuest, data.prevQuestLogic, [&](size_t i) { return player->hasQuestFinished(data.prevQuest[i]); }))
                return false;
            if (data.currQuest[0] != 0 && !checkSlots(data.currQuest, data.currQuestLogic, [&](size_t i) { return player->hasQuestInQuestLog(data.currQuest[i]); }))
                return false;
            if (data.currCompletedQuest[0] != 0 && !checkSlots(data.currCompletedQuest, data.currCompletedQuestLogic, [&](size_t i)
            {
                QuestLogEntry* entry = player->getQuestLogByQuestId(data.currCompletedQuest[i]);
                return entry != nullptr && entry->canBeFinished();
            }))
                return false;

            if (data.spell[0] != 0 && !checkSlots(data.spell, data.spellLogic, [&](size_t i) { return player->hasSpell(data.spell[i]); }))
                return false;

            if (data.item[0] != 0 && !checkSlots(data.item, data.itemLogic, [&](size_t i)
            {
                // flag 1 counts the bank, flag 2 the equipment only
                bool const withBank = (data.itemFlags & 1) != 0;
                uint32_t count = 0;
                if (data.itemFlags & 2)
                {
                    for (int16_t slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
                        if (Item* equipped = player->getItemInterface()->GetInventoryItem(slot))
                            if (equipped->getEntry() == data.item[i])
                                count += equipped->getStackCount();
                }
                else
                    count = player->getItemInterface()->GetItemCount(data.item[i], withBank);
                return count >= std::max(data.itemCount[i], 1u);
            }))
                return false;

            if (data.currency[0] != 0 && !checkSlots(data.currency, data.currencyLogic, [&](size_t i) { return player->getCurrency(data.currency[i]) >= data.currencyCount[i]; }))
                return false;

            for (uint32_t const areaId : data.explored)
                if (areaId != 0)
                    if (auto const* area = MapManagement::AreaManagement::AreaStorage::getAreaById(areaId))
                        if (!player->hasAreaExplored(area))
                            return false;

            if (data.aura[0] != 0 && !checkSlots(data.aura, data.auraLogic, [&](size_t i)
            {
                return data.auraStacks[i] != 0 ? player->getAuraCountForId(data.aura[i]) >= data.auraStacks[i] : player->hasAurasWithId(data.aura[i]);
            }))
                return false;

            if (data.time[0] != 0)
            {
                time_t const now = UNIXTIME;
                tm const local = *std::localtime(&now);
                if (data.time[1] != 0)
                {
                    if (now < unpackTime(data.time[0]) || now > unpackTime(data.time[1]))
                        return false;
                }
                else if (!matchesTime(data.time[0], local))
                    return false;
            }

            if (data.worldStateExpression != 0 && !isWorldStateExpressionMet(player, data.worldStateExpression))
                return false;

            if (data.achievement[0] != 0 && !checkSlots(data.achievement, data.achievementLogic, [&](size_t i) { return player->getAchievementMgr()->hasCompleted(data.achievement[i]); }))
                return false;

            if (data.lfgStatus[0] != 0 && !checkSlots(data.lfgStatus, data.lfgLogic, [&](size_t i)
            {
                return compareValues(static_cast<int32_t>(data.lfgCompare[i]), static_cast<int32_t>(lfgValueOf(player, data.lfgStatus[i])), static_cast<int32_t>(data.lfgValue[i]));
            }))
                return false;

            if (data.area[0] != 0 && !checkSlots(data.area, data.areaLogic, [&](size_t i) { return isInArea(player->getAreaId(), data.area[i]); }))
                return false;

            int32_t const expansion = player->getSession() != nullptr ? static_cast<int32_t>(player->getSession()->getClientProtocol().expansion) : 0;
            if (data.minExpansion != -1 && expansion < data.minExpansion)
                return false;
            if (data.maxExpansion != -1 && expansion > data.maxExpansion)
                return false;

            if (data.modifierTree != 0)
            {
                bool const met = Achievements::isConditionTreeMet(sAchievementTrees.getConditionTree(data.modifierTree), player, {});
                if (met == ((data.flags & FlagInvertModifierTree) != 0))
                    return false;
            }

            return true;
        }
    }

    void addPlayerCondition(PlayerConditionData data)
    {
        uint32_t const id = data.id;
        conditions[id] = std::move(data);
    }

    void initializePlayerConditions()
    {
        if (conditions.empty())
        {
            // the columns of the 5.2 to 5.4 rows after the id, the failure text skipped
            for (auto const& [conditionId, condition] : sPlayerConditionStore)
            {
                auto const* entry = &condition;

                int32_t const* v = entry->values.data();
                auto col = [v](size_t column) { return column < 132 ? v[column - 1] : v[column - 2]; };
                auto u = [&col](size_t column) { return static_cast<uint32_t>(col(column)); };

                PlayerConditionData data;
                data.id = entry->id;
                data.flags = u(1);
                data.minLevel = col(2);
                data.maxLevel = col(3);
                data.raceMask = u(4);
                data.classMask = u(5);
                data.gender = col(6);
                data.nativeGender = col(7);
                for (size_t s = 0; s < 4; ++s)
                {
                    data.skill[s] = u(8 + s);
                    data.minSkill[s] = u(12 + s);
                    data.maxSkill[s] = u(16 + s);
                    data.prevQuest[s] = u(37 + s);
                    data.currQuest[s] = u(42 + s);
                    data.currCompletedQuest[s] = u(47 + s);
                    data.spell[s] = u(52 + s);
                    data.item[s] = u(57 + s);
                    data.itemCount[s] = u(61 + s);
                    data.aura[s] = u(71 + s);
                    data.achievement[s] = u(80 + s);
                    data.lfgStatus[s] = u(85 + s);
                    data.lfgCompare[s] = u(89 + s);
                    data.lfgValue[s] = u(93 + s);
                    data.area[s] = u(98 + s);
                    data.currency[s] = u(103 + s);
                    data.currencyCount[s] = u(107 + s);
                    data.questKillMonster[s] = u(113 + s);
                }
                data.skillLogic = u(20);
                data.languageId = u(21);
                data.minLanguage = col(22);
                data.maxLanguage = col(23);
                for (size_t s = 0; s < 3; ++s)
                {
                    data.minFaction[s] = u(24 + s);
                    data.minReputation[s] = u(28 + s);
                }
                data.maxFaction = u(27);
                data.maxReputation = u(31);
                data.reputationLogic = u(32);
                data.minPvpRank = col(33);
                data.maxPvpRank = col(34);
                data.pvpMedal = col(35);
                data.prevQuestLogic = u(36);
                data.currQuestLogic = u(41);
                data.currCompletedQuestLogic = u(46);
                data.spellLogic = u(51);
                data.itemLogic = u(56);
                data.itemFlags = u(65);
                data.explored = { u(66), u(67) };
                data.time = { u(68), u(69) };
                data.auraLogic = u(70);
                data.worldStateExpression = u(75);
                data.weather = u(76);
                data.partyStatus = u(77);
                data.lifetimeMaxPvpRank = col(78);
                data.achievementLogic = u(79);
                data.lfgLogic = u(84);
                data.areaLogic = u(97);
                data.currencyLogic = u(102);
                data.questKill = u(111);
                data.questKillLogic = u(112);
                data.minExpansion = col(117);
                data.maxExpansion = col(118);
                data.minExpansionTier = col(119);
                data.maxExpansionTier = col(120);
                data.minGuildLevel = col(121);
                data.maxGuildLevel = col(122);
                data.phaseUseFlags = u(123);
                data.phaseId = u(124);
                data.phaseGroupId = u(125);
                data.minAvgItemLevel = col(126);
                data.maxAvgItemLevel = col(127);
                data.minAvgEquippedItemLevel = col(128);
                data.maxAvgEquippedItemLevel = col(129);
                data.specializationIndex = col(130);
                data.specializationRole = col(131);
                data.powerType = col(133);
                data.powerCompare = col(134);
                data.powerValue = col(135);
                addPlayerCondition(std::move(data));
            }
        }

        sLogger.info("PlayerConditions : {} conditions", conditions.size());
    }

    bool isPlayerConditionMet(Player* player, uint32_t conditionId)
    {
        if (player == nullptr)
            return false;

        auto const itr = conditions.find(conditionId);
        if (itr == conditions.end())
            return false;

        return isMet(player, itr->second) != ((itr->second.flags & FlagInvert) != 0);
    }

    void reportPlayerConditions(std::set<uint32_t> const& conditionIds)
    {
        uint32_t missing = 0;
        std::map<std::string, uint32_t> parts;
        for (uint32_t const id : conditionIds)
        {
            auto const itr = conditions.find(id);
            if (itr == conditions.end())
            {
                ++missing;
                continue;
            }

            for (std::string const& part : unsupportedParts(itr->second))
                ++parts[part];
        }

        if (missing != 0)
            sLogger.info("PlayerConditions : {} of {} conditions in use are not part of the client data", missing, conditionIds.size());

        std::string text;
        for (auto const& [part, count] : parts)
            text += (text.empty() ? "" : ", ") + part + " (" + std::to_string(count) + ")";
        if (!text.empty())
            sLogger.info("PlayerConditions : condition parts not evaluated yet, part (conditions): {}", text);
    }
}
