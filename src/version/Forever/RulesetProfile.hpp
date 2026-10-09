/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once

#include "BattleNetCommDefines.hpp"
#include "version/Forever/BuildProfile.hpp"

#include <cstdint>

namespace AscEmu::Version::Forever
{
    enum class RulesetProfile : uint8_t
    {
        None,
        Legacy69893,
        Modern70009Plus,
    };


    // Realm-wide Forever ruleset state. Cfg_SuperDistrictID is the value exposed
    // to realm/character/player wire state; AvailableSuperDistrictID is the DB2
    // relation key used to resolve SuperDistrictSet membership.
    struct RealmRulesetState
    {
        AscEmu::BattlenetComm::RealmRuleset ruleset = AscEmu::BattlenetComm::RealmRuleset::PvE;
        uint32_t cfgSuperDistrictId = 0;
        uint32_t availableSuperDistrictId = 0;
    };

    struct SuperDistrictProfile
    {
        RulesetProfile rulesetProfile = RulesetProfile::None;
        uint32_t build = 0;
        uint32_t collectionId = 0;
        uint32_t superDistrictSetId = 0;

        uint32_t pveAvailableSuperDistrictId = 0;
        uint32_t pvpAvailableSuperDistrictId = 0;
        uint32_t roleplayAvailableSuperDistrictId = 0;
        uint32_t hardcoreAvailableSuperDistrictId = 0;

        uint32_t currentCfgContentSetId = 0;
        bool contentSetIdKnown = false;
    };

    [[nodiscard]] inline constexpr SuperDistrictProfile getSuperDistrictProfile(uint32_t clientBuild)
    {
        if (!supportsBuild(clientBuild))
            return {};

        if (clientBuild == 69893u)
        {
            return {
                .rulesetProfile = RulesetProfile::Legacy69893,
                .build = clientBuild,
                .superDistrictSetId = 60u,
            };
        }

        // FOREVER-VERIFIED: builds using the modern 1.60 ruleset selector use collection 1 / set 36.
        // AvailableSuperDistrict 3 = PvE/Normal and 2 = PvP. Roleplay/Hardcore remain unverified.
        return {
            .rulesetProfile = RulesetProfile::Modern70009Plus,
            .build = clientBuild,
            .collectionId = 1u,
            .superDistrictSetId = 36u,
            .pveAvailableSuperDistrictId = 3u,
            .pvpAvailableSuperDistrictId = 2u,
            .roleplayAvailableSuperDistrictId = 0u,
            .hardcoreAvailableSuperDistrictId = 0u,
            .currentCfgContentSetId = 0u,
            .contentSetIdKnown = false,
        };
    }

    [[nodiscard]] inline constexpr uint32_t getCfgSuperDistrictId(AscEmu::BattlenetComm::RealmRuleset ruleset)
    {
        // FOREVER-VERIFIED: Cfg_SuperDistrictID carried by realm, character and player state.
        switch (ruleset)
        {
            case AscEmu::BattlenetComm::RealmRuleset::PvP: return 1u;
            case AscEmu::BattlenetComm::RealmRuleset::PvE: return 2u;
            case AscEmu::BattlenetComm::RealmRuleset::Roleplay: return 3u;
            case AscEmu::BattlenetComm::RealmRuleset::Hardcore: return 4u;
        }

        return 0u;
    }

    [[nodiscard]] inline constexpr uint32_t getAvailableSuperDistrictId(SuperDistrictProfile const& profile, AscEmu::BattlenetComm::RealmRuleset ruleset)
    {
        switch (ruleset)
        {
            case AscEmu::BattlenetComm::RealmRuleset::PvE: return profile.pveAvailableSuperDistrictId;
            case AscEmu::BattlenetComm::RealmRuleset::PvP: return profile.pvpAvailableSuperDistrictId;
            case AscEmu::BattlenetComm::RealmRuleset::Roleplay: return profile.roleplayAvailableSuperDistrictId;
            case AscEmu::BattlenetComm::RealmRuleset::Hardcore: return profile.hardcoreAvailableSuperDistrictId;
        }

        return 0u;
    }

    [[nodiscard]] inline constexpr RealmRulesetState getRealmRulesetState(SuperDistrictProfile const& profile, AscEmu::BattlenetComm::RealmRuleset ruleset)
    {
        return {
            .ruleset = ruleset,
            .cfgSuperDistrictId = getCfgSuperDistrictId(ruleset),
            .availableSuperDistrictId = getAvailableSuperDistrictId(profile, ruleset),
        };
    }
}
