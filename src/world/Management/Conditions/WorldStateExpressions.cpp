/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WorldStateExpressions.hpp"

#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Management/GameEvent.hpp"
#include "Management/GameEventMgr.hpp"
#include "Management/WorldStatesHandler.hpp"
#include "Map/Maps/BaseMap.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Random.hpp"

#include <ctime>
#include <map>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>

namespace Conditions
{
    namespace
    {
        enum class ValueKind : uint8_t
        {
            Constant    = 1,
            WorldState  = 2,
            Function    = 3
        };

        enum class Arithmetic : uint8_t
        {
            None        = 0,
            Add         = 1,
            Subtract    = 2,
            Multiply    = 3,
            Divide      = 4,
            Modulo      = 5
        };

        enum class Comparison : uint8_t
        {
            None            = 0,
            Equal           = 1,
            NotEqual        = 2,
            Less            = 3,
            LessOrEqual     = 4,
            Greater         = 5,
            GreaterOrEqual  = 6
        };

        enum class Combination : uint8_t
        {
            End     = 0,
            And     = 1,
            Or      = 2,
            Xor     = 3
        };

        // the functions of an expression the core can answer, the numbers of the client data
        enum Function : uint32_t
        {
            FunctionNone            = 0,
            FunctionRandom          = 1,
            FunctionMonth           = 2,
            FunctionDay             = 3,
            FunctionTimeOfDay       = 4,
            FunctionRegion          = 5,
            FunctionClockHour       = 6,
            FunctionOldDifficulty   = 7,
            FunctionHolidayActive   = 10,
            FunctionCurrentTime     = 11,
            FunctionWeekNumber      = 12,
            FunctionDifficulty      = 15,
            FunctionExpression      = 22
        };

        // the first raid reset the week numbers count from
        constexpr int64_t RaidOrigin = 1135695600;

        std::unordered_map<uint32_t, std::vector<uint8_t>> expressions;
        std::map<uint32_t, uint32_t> unknownFunctions;

        std::vector<uint8_t> fromHex(std::string_view text)
        {
            std::vector<uint8_t> bytes;
            bytes.reserve(text.size() / 2);
            auto nibble = [](char c) -> int
            {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
            };

            for (size_t i = 0; i + 1 < text.size(); i += 2)
            {
                int const high = nibble(text[i]);
                int const low = nibble(text[i + 1]);
                if (high < 0 || low < 0)
                    return {};
                bytes.push_back(static_cast<uint8_t>(high << 4 | low));
            }
            return bytes;
        }

        // reads one expression, an unknown function or a short buffer stops it
        class Reader
        {
        public:
            Reader(std::vector<uint8_t> const& data, Player* player, uint8_t depth) : m_data(data), m_player(player), m_depth(depth) {}

            bool failed() const { return m_failed; }
            bool atEnd() const { return m_position >= m_data.size(); }

            uint8_t byte()
            {
                if (m_position + 1 > m_data.size())
                {
                    m_failed = true;
                    return 0;
                }
                return m_data[m_position++];
            }

            int32_t int32()
            {
                if (m_position + 4 > m_data.size())
                {
                    m_failed = true;
                    return 0;
                }

                uint32_t const value = static_cast<uint32_t>(m_data[m_position]) | static_cast<uint32_t>(m_data[m_position + 1]) << 8
                    | static_cast<uint32_t>(m_data[m_position + 2]) << 16 | static_cast<uint32_t>(m_data[m_position + 3]) << 24;
                m_position += 4;
                return static_cast<int32_t>(value);
            }

            int32_t single()
            {
                switch (static_cast<ValueKind>(byte()))
                {
                    case ValueKind::Constant:
                        return int32();
                    case ValueKind::WorldState:
                        return worldState(static_cast<uint32_t>(int32()));
                    case ValueKind::Function:
                    {
                        uint32_t const function = static_cast<uint32_t>(int32());
                        int32_t const first = single();
                        int32_t const second = single();
                        return call(function, first, second);
                    }
                    default:
                        m_failed = true;
                        return 0;
                }
            }

            int32_t value()
            {
                int32_t const left = single();
                Arithmetic const operation = static_cast<Arithmetic>(byte());
                if (operation == Arithmetic::None)
                    return left;

                int32_t const right = single();
                switch (operation)
                {
                    case Arithmetic::Add:       return left + right;
                    case Arithmetic::Subtract:  return left - right;
                    case Arithmetic::Multiply:  return left * right;
                    case Arithmetic::Divide:    return right != 0 ? left / right : 0;
                    case Arithmetic::Modulo:    return right != 0 ? left % right : 0;
                    default:
                        m_failed = true;
                        return 0;
                }
            }

            bool relation()
            {
                int32_t const left = value();
                Comparison const comparison = static_cast<Comparison>(byte());
                if (comparison == Comparison::None)
                    return left != 0;

                int32_t const right = value();
                switch (comparison)
                {
                    case Comparison::Equal:             return left == right;
                    case Comparison::NotEqual:          return left != right;
                    case Comparison::Less:              return left < right;
                    case Comparison::LessOrEqual:       return left <= right;
                    case Comparison::Greater:           return left > right;
                    case Comparison::GreaterOrEqual:    return left >= right;
                    default:
                        m_failed = true;
                        return false;
                }
            }

        private:
            int32_t worldState(uint32_t worldStateId) const
            {
                WorldMap* map = m_player != nullptr ? m_player->getWorldMap() : nullptr;
                if (map == nullptr)
                    return 0;
                return static_cast<int32_t>(map->getWorldStatesHandler().GetWorldStateForZone(m_player->getZoneId(), m_player->getAreaId(), worldStateId));
            }

            int32_t legacyDifficulty() const
            {
                WorldMap* map = m_player != nullptr ? m_player->getWorldMap() : nullptr;
                if (map == nullptr || (!map->getBaseMap()->isDungeon() && !map->getBaseMap()->isRaid()))
                    return -1;
                return map->getSpawnMode();
            }

            int32_t difficultyId() const
            {
                WorldMap* map = m_player != nullptr ? m_player->getWorldMap() : nullptr;
                if (map == nullptr)
                    return 0;
                if (map->getBaseMap()->isRaid())
                    return 3 + map->getSpawnMode();
                if (map->getBaseMap()->isDungeon())
                    return 1 + map->getSpawnMode();
                return 0;
            }

            bool holidayActive(uint32_t holidayId) const
            {
                if (m_player == nullptr)
                    return false;

                for (uint16_t const eventId : sGameEventMgr.GetActiveEventList())
                    if (GameEvent* gameEvent = sGameEventMgr.GetEventById(eventId))
                        if (static_cast<uint32_t>(gameEvent->holiday_id) == holidayId)
                            return true;
                return false;
            }

            int32_t call(uint32_t function, int32_t first, int32_t second)
            {
                time_t const now = UNIXTIME;
                tm const local = *std::localtime(&now);
                switch (function)
                {
                    case FunctionNone:
                        return 0;
                    case FunctionRandom:
                        return static_cast<int32_t>(Util::getRandomUInt(static_cast<uint32_t>(std::min(first, second)), static_cast<uint32_t>(std::max(first, second))));
                    case FunctionMonth:
                        return local.tm_mon + 1;
                    case FunctionDay:
                        return local.tm_mday + 1;
                    case FunctionTimeOfDay:
                        return local.tm_hour * 60 + local.tm_min;
                    case FunctionRegion:
                    {
                        // clients before 6.x name no region
                        uint32_t const region = m_player != nullptr && m_player->getSession() != nullptr ? m_player->getSession()->getClientProtocol().regionId : 0;
                        if (region == 0 && m_player != nullptr)
                            m_failed = true;
                        return static_cast<int32_t>(region);
                    }
                    case FunctionClockHour:
                    {
                        int32_t const hour = local.tm_hour + 1;
                        return hour <= 12 ? (hour != 0 ? hour : 12) : hour - 12;
                    }
                    case FunctionOldDifficulty:
                        return legacyDifficulty();
                    case FunctionHolidayActive:
                        return holidayActive(static_cast<uint32_t>(first)) ? 1 : 0;
                    case FunctionCurrentTime:
                        return static_cast<int32_t>(now);
                    case FunctionWeekNumber:
                        return static_cast<int32_t>((now - RaidOrigin) / (7 * 24 * 3600));
                    case FunctionDifficulty:
                        return difficultyId();
                    case FunctionExpression:
                        if (m_player == nullptr)
                            return 0;
                        return m_depth < 8 && evaluate(m_player, static_cast<uint32_t>(first), static_cast<uint8_t>(m_depth + 1)).value_or(false) ? 1 : 0;
                    default:
                        // a function without known meaning: the expression can not be decided
                        if (m_player == nullptr)
                            ++unknownFunctions[function];
                        else
                            m_failed = true;
                        return 0;
                }
            }

        public:
            static std::optional<bool> evaluate(Player* player, uint32_t expressionId, uint8_t depth)
            {
                auto const itr = expressions.find(expressionId);
                if (itr == expressions.end() || itr->second.empty())
                    return std::nullopt;

                Reader reader(itr->second, player, depth);
                if (reader.byte() == 0)
                    return false;

                bool result = reader.relation();
                while (!reader.failed() && !reader.atEnd())
                {
                    Combination const combination = static_cast<Combination>(reader.byte());
                    if (combination == Combination::End)
                        break;

                    bool const next = reader.relation();
                    switch (combination)
                    {
                        case Combination::And:  result = result && next; break;
                        case Combination::Or:   result = result || next; break;
                        case Combination::Xor:  result = result != next; break;
                        default:
                            return std::nullopt;
                    }
                }

                if (reader.failed())
                    return std::nullopt;
                return result;
            }

        private:
            std::vector<uint8_t> const& m_data;
            Player* m_player;
            uint8_t m_depth;
            size_t m_position = 0;
            bool m_failed = false;
        };
    }

    void addWorldStateExpression(uint32_t id, std::string_view expression)
    {
        expressions[id] = fromHex(expression);
    }

    void initializeWorldStateExpressions()
    {
        if (expressions.empty())
        {
            for (auto const& [expressionId, entry] : sWorldStateExpressionStore)
                if (!entry.expression.empty())
                    addWorldStateExpression(entry.id, entry.expression);
        }

        // every expression once without a player names the functions the core does not know
        for (auto const& [id, bytes] : expressions)
            Reader::evaluate(nullptr, id, 0);

        std::string unknown;
        for (auto const& [function, count] : unknownFunctions)
            unknown += (unknown.empty() ? "" : ", ") + std::to_string(function) + " (" + std::to_string(count) + ")";

        sLogger.info("WorldStateExpressions : {} expressions", expressions.size());
        if (!unknown.empty())
            sLogger.info("WorldStateExpressions : functions not evaluated yet, function (uses): {}", unknown);
    }

    bool isWorldStateExpressionMet(Player* player, uint32_t expressionId)
    {
        if (player == nullptr)
            return false;


        return Reader::evaluate(player, expressionId, 0).value_or(false);
    }
}
