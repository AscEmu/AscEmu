/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Db2Registry.hpp"

#include <array>

// The tables the 1.60 line (build 70959) has on top of the 12.x list, both lines share the file data ids.
namespace MapExtractor::ForeverDefinition
{
    inline constexpr std::array<DB2::Db2Entry, 30> DB2_FILES =
    {
        DB2::Db2Entry{8426136, 0x891BE7B6u, "AuctionHouseRedirect.db2"},
        DB2::Db2Entry{8208798, 0x45952E6Eu, "AvailableSuperDistrict.db2"},
        DB2::Db2Entry{7871289, 0x2CC4E8A5u, "BattlePetTypeModifier.db2"},
        DB2::Db2Entry{7877962, 0xDA814D0Fu, "BattlePetXP.db2"},
        DB2::Db2Entry{7929271, 0xFFD9D676u, "Cfg_SuperDistrict.db2"},
        DB2::Db2Entry{8068383, 0x8E636A9Bu, "ChrModelAltVariant.db2"},
        DB2::Db2Entry{8068384, 0xF4A6DC16u, "ChrModelAltVariantChoice.db2"},
        DB2::Db2Entry{8068385, 0x65FD1ED8u, "ChrModelAltVariantOption.db2"},
        DB2::Db2Entry{7489860, 0x43B03B3Cu, "FftTile.db2"},
        DB2::Db2Entry{6156256, 0x30E32F7Au, "ItemDamageRanged.db2"},
        DB2::Db2Entry{6156257, 0x72FD7ADBu, "ItemDamageThrown.db2"},
        DB2::Db2Entry{6156258, 0xC2C866AFu, "ItemDamageWand.db2"},
        DB2::Db2Entry{7880197, 0xD80405C3u, "LevelExperience.db2"},
        DB2::Db2Entry{7492879, 0x7E319A3Du, "LightDataGlobalVolumeFog.db2"},
        DB2::Db2Entry{7489861, 0x34E07616u, "LiquidTypeXFftTile.db2"},
        DB2::Db2Entry{2060639, 0x1A6B0C40u, "ModelProperties.db2"},
        DB2::Db2Entry{5393273, 0x3734C5A2u, "ModelSound.db2"},
        DB2::Db2Entry{5393276, 0x66D2AD5Du, "ModelSoundOverride.db2"},
        DB2::Db2Entry{5682517, 0xBB58C4DAu, "ModelSoundSettings.db2"},
        DB2::Db2Entry{5393277, 0x30B4EA1Cu, "ModelSoundTagEntry.db2"},
        DB2::Db2Entry{2179609, 0x29A92016u, "PetPersonality.db2"},
        DB2::Db2Entry{7667204, 0xC012F724u, "PlayerExpectedStat.db2"},
        DB2::Db2Entry{7722633, 0x4D07E2FCu, "RaceStat.db2"},
        DB2::Db2Entry{2179613, 0x12F89E8Du, "StableSlotPrices.db2"},
        DB2::Db2Entry{8279681, 0xE4CDB5CFu, "SuperDistrictSet.db2"},
        DB2::Db2Entry{8279682, 0x90204B0Au, "SuperDistrictSetCollection.db2"},
        DB2::Db2Entry{8281618, 0x08014697u, "SuperDistrictSetXAvailableSD.db2"},
        DB2::Db2Entry{8279874, 0x94F0D5CCu, "SuperDistrictTransfer.db2"},
        DB2::Db2Entry{7935808, 0x34B4B384u, "TraitNodeGroupDisplayInfo.db2"},
        DB2::Db2Entry{7939471, 0x946A268Du, "UiTextureAtlasSet.db2"},
    };
}
