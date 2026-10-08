/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once

#include "Server/World.h"
#include "Storage/WDB/WDBStores.hpp"
#include "version/Forever/BuildProfile.hpp"
#include "version/Forever/RulesetProfile.hpp"

namespace AscEmu::Version::Forever
{
    [[nodiscard]] inline RealmRulesetState getCurrentRealmRulesetState()
    {
        return getRealmRulesetState(getSuperDistrictProfile(Build), sWorld.settings.battleNetComm.ruleset);
    }

    [[nodiscard]] inline uint32_t getRealmCfgSuperDistrictId()
    {
        return getCurrentRealmRulesetState().cfgSuperDistrictId;
    }

    [[nodiscard]] inline uint32_t getRealmAvailableSuperDistrictId()
    {
        return getCurrentRealmRulesetState().availableSuperDistrictId;
    }

#if defined(AE_FOREVER)
    [[nodiscard]] inline bool isSuperDistrictSetActiveForRealm(int32_t superDistrictSetId)
    {
        if (superDistrictSetId == 0)
            return true;
        if (superDistrictSetId < 0)
            return false;

        uint32_t const availableSuperDistrictId = getRealmAvailableSuperDistrictId();
        if (availableSuperDistrictId == 0)
            return false;

        for (auto const& [id, relation] : sSuperDistrictSetXAvailableSDStore)
        {
            (void)id;
            if (relation.superDistrictSetId == static_cast<uint32_t>(superDistrictSetId)
                && relation.availableSuperDistrictId == availableSuperDistrictId)
                return true;
        }

        return false;
    }
#endif
}
