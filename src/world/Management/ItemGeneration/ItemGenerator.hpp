/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace AscEmu::Items
{
    struct StatOverride
    {
        int32_t type{-1};
        std::optional<int32_t> fixedValue;
        std::optional<int32_t> percentEditor;
        bool remove{false};
    };

    struct GenerationOverrides
    {
        std::optional<uint32_t> itemLevel;
        std::optional<uint32_t> quality;
        std::vector<StatOverride> stats;
    };

    struct ResolvedStat
    {
        int32_t type{-1};
        int32_t value{0};
        int32_t percentEditor{0};
        float percentageOfSocket{0.0f};
        bool fixed{false};
    };

    struct ResolvedItemEffect
    {
        int32_t legacySlotIndex{0};
        uint32_t spellId{0};
        uint32_t trigger{0};
        int32_t charges{0};
        int32_t cooldown{-1};
        uint32_t category{0};
        int32_t categoryCooldown{-1};
    };

    struct GeneratedItemData
    {
        uint32_t entry{0};
        uint32_t baseItemLevel{0};
        uint32_t itemLevel{0};
        uint32_t quality{0};
        uint32_t inventoryType{0};
        float randomPropertyPoints{0.0f};
        uint32_t armor{0};
        uint32_t maxDurability{0};
        std::vector<ResolvedStat> stats;
        std::vector<ResolvedItemEffect> effects;
    };

    // Returns client-derived generated data where the active expansion provides
    // such a model. Legacy expansions keep their existing item_properties path
    // and intentionally return std::nullopt here.
    [[nodiscard]] std::optional<GeneratedItemData> generateItemData(uint32_t entry, GenerationOverrides const& overrides = {});
}
