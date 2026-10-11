/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"

#include <cstdint>
#include <vector>

// 9.x and later clients describe the look of a character as a list of customization choices (option id, choice
// id) instead of the skin, face, hair style, hair color and facial feature bytes the server stores. The conversion
// table maps the bytes of every race, sex and class to the choices of the 9.0 character migration.
namespace CharacterCustomizations
{
    struct ChrCustomizationChoice
    {
        uint32_t optionId;
        uint32_t choiceId;
    };

    // the legacy look bytes
    struct LegacyLook
    {
        uint8_t skin = 0;
        uint8_t face = 0;
        uint8_t hairStyle = 0;
        uint8_t hairColor = 0;
        uint8_t facialStyle = 0;
        uint8_t customDisplay[3] = { 0, 0, 0 };
    };

    enum LegacyOption : uint8_t
    {
        LegacyOptionSkin = 1,
        LegacyOptionFace = 2,
        LegacyOptionHairStyle = 3,
        LegacyOptionHairColor = 4,
        LegacyOptionFacialStyle = 5,
        LegacyOptionCustomDisplay1 = 6,
        LegacyOptionCustomDisplay2 = 7,
        LegacyOptionCustomDisplay3 = 8
    };

    // the choices of a look, sorted by option id; empty when the race, sex or class is unknown
    SERVER_DECL std::vector<ChrCustomizationChoice> convertLegacyLook(uint8_t race, uint8_t playerClass, uint8_t sex, LegacyLook const& look);
}
