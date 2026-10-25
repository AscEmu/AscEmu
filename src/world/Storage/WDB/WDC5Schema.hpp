/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <initializer_list>
#include <utility>
#include <vector>

namespace WDB
{
    // The WDC files carry the packing of every column but not which columns
    // are arrays, nor the record id position of a table whose id is not
    // stored inline. A schema supplies exactly that per table and client
    // version; the layout hash guards against a client whose columns moved.
    struct WDC5FieldSchema
    {
        uint8_t arraySize{1};
        bool isString{false};
    };

    struct WDC5TableSchema
    {
        char const* filename{nullptr};
        uint32_t layoutHash{0};
        int32_t indexField{-1};         // -1: the record ids come from the section id table
        std::vector<WDC5FieldSchema> fields;
    };

    inline WDC5TableSchema makeScalarSchema(char const* filename, uint32_t layoutHash, int32_t indexField, uint32_t fieldCount)
    {
        return { filename, layoutHash, indexField, std::vector<WDC5FieldSchema>(fieldCount, WDC5FieldSchema{ 1, false }) };
    }

    inline WDC5TableSchema makeSchemaWithArrays(char const* filename, uint32_t layoutHash, int32_t indexField,
        uint32_t fieldCount, std::initializer_list<std::pair<uint32_t, uint8_t>> arrays)
    {
        auto schema = makeScalarSchema(filename, layoutHash, indexField, fieldCount);
        for (auto const& [field, arraySize] : arrays)
            schema.fields[field].arraySize = arraySize;
        return schema;
    }

    inline WDC5TableSchema makeSparseSchema(char const* filename, uint32_t layoutHash, int32_t indexField,
        uint32_t fieldCount, std::initializer_list<std::pair<uint32_t, uint8_t>> arrays,
        std::initializer_list<uint32_t> stringFields)
    {
        auto schema = makeSchemaWithArrays(filename, layoutHash, indexField, fieldCount, arrays);
        for (uint32_t const field : stringFields)
        {
            if (field < schema.fields.size())
                schema.fields[field].isString = true;
        }
        return schema;
    }

    // Column layouts of the 12.1.5 client (build 70391). Field indices are the
    // physical columns of the file; the ids of tables with indexField -1 and the
    // parent ids of relation tables are read from the section tables instead.
    namespace Formats::MidnightClient
    {
        // Characters, classes, races, factions
        inline const WDC5TableSchema ChrClasses = makeScalarSchema("ChrClasses.db2", 0xAFC9B0C2, 29, 43);
        inline const WDC5TableSchema ChrClassesXPowerTypes = makeScalarSchema("ChrClassesXPowerTypes.db2", 0x70DA1F8C, -1, 1);
        inline const WDC5TableSchema ChrRaces = makeSchemaWithArrays("ChrRaces.db2", 0x4F44C796, -1, 51, { { 23, 3 }, { 24, 3 }, { 30, 3 }, { 32, 3 }, { 33, 3 } });
        inline const WDC5TableSchema ChrModel = makeSchemaWithArrays("ChrModel.db2", 0x03FAB755, 2, 17, { { 0, 3 }, { 1, 3 } });
        inline const WDC5TableSchema ChrRaceXChrModel = makeScalarSchema("ChrRaceXChrModel.db2", 0xA203BC29, -1, 4);
        inline const WDC5TableSchema ChrSpecialization = makeSchemaWithArrays("ChrSpecialization.db2", 0xDAB4CA4B, 3, 13, { { 12, 2 } });
        inline const WDC5TableSchema Faction = makeSchemaWithArrays("Faction.db2", 0xE7553DC7, -1, 20, { { 10, 4 }, { 11, 4 }, { 12, 4 }, { 13, 4 }, { 14, 2 }, { 15, 2 }, { 16, 2 }, { 17, 2 }, { 18, 2 }, { 19, 2 } });
        inline const WDC5TableSchema FactionTemplate = makeSchemaWithArrays("FactionTemplate.db2", 0x22B6DC22, -1, 7, { { 5, 8 }, { 6, 8 } });
        inline const WDC5TableSchema CharTitles = makeScalarSchema("CharTitles.db2", 0x2F5DFF7F, -1, 4);
        inline const WDC5TableSchema CharacterLoadout = makeSchemaWithArrays("CharacterLoadout.db2", 0xA5B16B13, -1, 4, { { 3, 2 } });
        inline const WDC5TableSchema CharacterLoadoutItem = makeScalarSchema("CharacterLoadoutItem.db2", 0x0C7A1862, -1, 2);
        inline const WDC5TableSchema NameGen = makeScalarSchema("NameGen.db2", 0x3CA45193, -1, 3);

        // Creatures
        inline const WDC5TableSchema CreatureDisplayInfo = makeSchemaWithArrays("CreatureDisplayInfo.db2", 0x7275F5F6, 0, 28, { { 27, 4 } });
        inline const WDC5TableSchema CreatureDisplayInfoExtra = makeScalarSchema("CreatureDisplayInfoExtra.db2", 0x4D9FE25C, 0, 7);
        inline const WDC5TableSchema CreatureModelData = makeSchemaWithArrays("CreatureModelData.db2", 0x0F5449F8, -1, 34, { { 0, 6 }, { 33, 2 } });
        inline const WDC5TableSchema CreatureFamily = makeSchemaWithArrays("CreatureFamily.db2", 0x7FE3CBA7, -1, 10, { { 9, 2 } });
        inline const WDC5TableSchema SummonProperties = makeSchemaWithArrays("SummonProperties.db2", 0xA4CA5ECF, -1, 5, { { 4, 2 } });
        inline const WDC5TableSchema Vehicle = makeSchemaWithArrays("Vehicle.db2", 0xEF5C7D41, -1, 18, { { 16, 8 }, { 17, 3 } });
        inline const WDC5TableSchema VehicleSeat = makeSchemaWithArrays("VehicleSeat.db2", 0x973F2793, -1, 61, { { 0, 3 }, { 1, 3 } });

        // World
        inline const WDC5TableSchema Map = makeSchemaWithArrays("Map.db2", 0x75863E23, -1, 25, { { 6, 2 }, { 24, 3 } });
        inline const WDC5TableSchema MapDifficulty = makeScalarSchema("MapDifficulty.db2", 0x24A16AD3, 1, 11);
        inline const WDC5TableSchema UiMapAssignment = makeSchemaWithArrays("UiMapAssignment.db2", 0xC9CC8DFB, 3, 11, { { 0, 2 }, { 1, 2 }, { 2, 6 } });
        inline const WDC5TableSchema WorldMapOverlay = makeSchemaWithArrays("WorldMapOverlay.db2", 0x9A196494, 0, 13, { { 12, 4 } });
        inline const WDC5TableSchema AreaTable = makeSchemaWithArrays("AreaTable.db2", 0x5DB1A3FF, -1, 23, { { 21, 2 }, { 22, 4 } });
        inline const WDC5TableSchema AreaTrigger = makeSchemaWithArrays("AreaTrigger.db2", 0xB97D2FC5, 1, 15, { { 0, 3 } });
        inline const WDC5TableSchema AreaGroupMember = makeScalarSchema("AreaGroupMember.db2", 0x6746D145, -1, 1);
        inline const WDC5TableSchema LiquidType = makeSchemaWithArrays("LiquidType.db2", 0xD1ECEEC9, -1, 21, { { 1, 6 }, { 16, 6 }, { 17, 3 }, { 18, 38 }, { 19, 4 }, { 20, 4 } });
        inline const WDC5TableSchema WMOAreaTable = makeScalarSchema("WMOAreaTable.db2", 0xC5A7B977, 1, 15);
        inline const WDC5TableSchema DungeonEncounter = makeScalarSchema("DungeonEncounter.db2", 0x35863DC4, 1, 11);
        inline const WDC5TableSchema LFGDungeons = makeSchemaWithArrays("LFGDungeons.db2", 0x34B02DE8, -1, 33, { { 32, 2 } });
        inline const WDC5TableSchema ContentTuning = makeScalarSchema("ContentTuning.db2", 0xA3E13004, 0, 19);
        inline const WDC5TableSchema Phase = makeScalarSchema("Phase.db2", 0x6D5EB3BD, -1, 1);
        inline const WDC5TableSchema TransportAnimation = makeSchemaWithArrays("TransportAnimation.db2", 0xE622B105, -1, 3, { { 0, 3 } });
        inline const WDC5TableSchema TransportRotation = makeSchemaWithArrays("TransportRotation.db2", 0xD92B0C43, -1, 2, { { 0, 4 } });
        inline const WDC5TableSchema TaxiNodes = makeSchemaWithArrays("TaxiNodes.db2", 0xE7B597F0, 4, 15, { { 1, 3 }, { 2, 2 }, { 3, 2 }, { 14, 2 } });
        inline const WDC5TableSchema TaxiPath = makeScalarSchema("TaxiPath.db2", 0xA303DE51, 0, 4);
        inline const WDC5TableSchema TaxiPathNode = makeSchemaWithArrays("TaxiPathNode.db2", 0xFE362E70, 1, 9, { { 0, 3 } });
        inline const WDC5TableSchema Holidays = makeSchemaWithArrays("Holidays.db2", 0x432931C0, 0, 12, { { 8, 10 }, { 9, 26 }, { 10, 10 }, { 11, 3 } });
        inline const WDC5TableSchema ChatChannels = makeScalarSchema("ChatChannels.db2", 0xB8ED4F4C, -1, 5);
        inline const WDC5TableSchema AuctionHouse = makeScalarSchema("AuctionHouse.db2", 0x9D007DB2, -1, 4);
        inline const WDC5TableSchema BankBagSlotPrices = makeScalarSchema("BankBagSlotPrices.db2", 0xDBCA3459, -1, 1);
        inline const WDC5TableSchema MailTemplate = makeScalarSchema("MailTemplate.db2", 0xB436C1C8, -1, 1);
        inline const WDC5TableSchema Emotes = makeSchemaWithArrays("Emotes.db2", 0x0A598B68, -1, 9, { { 8, 2 } });
        inline const WDC5TableSchema EmotesText = makeScalarSchema("EmotesText.db2", 0xCCDBFBAF, -1, 2);
        inline const WDC5TableSchema QuestXP = makeSchemaWithArrays("QuestXP.db2", 0x05F1DB7A, -1, 1, { { 0, 10 } });
        inline const WDC5TableSchema QuestSort = makeScalarSchema("QuestSort.db2", 0x9B92BE63, -1, 3);
        inline const WDC5TableSchema BannedAddons = makeScalarSchema("BannedAddons.db2", 0x56583F69, -1, 3);
        inline const WDC5TableSchema Lock = makeSchemaWithArrays("Lock.db2", 0xDA418152, -1, 5, { { 1, 8 }, { 2, 8 }, { 3, 8 }, { 4, 8 } });
        inline const WDC5TableSchema GameObjectDisplayInfo = makeSchemaWithArrays("GameObjectDisplayInfo.db2", 0x16D1DD61, -1, 9, { { 0, 6 } });
        inline const WDC5TableSchema TotemCategory = makeScalarSchema("TotemCategory.db2", 0x86C8149C, -1, 3);
        inline const WDC5TableSchema GemProperties = makeScalarSchema("GemProperties.db2", 0x86487AD2, -1, 2);
        inline const WDC5TableSchema MountCapability = makeScalarSchema("MountCapability.db2", 0xA0297DBC, -1, 10);
        inline const WDC5TableSchema MountTypeXCapability = makeScalarSchema("MountTypeXCapability.db2", 0x218BBE7F, -1, 3);
        inline const WDC5TableSchema CurrencyTypes = makeSchemaWithArrays("CurrencyTypes.db2", 0xFA3226AB, -1, 22, { { 21, 2 } });
        inline const WDC5TableSchema GuildPerkSpells = makeScalarSchema("GuildPerkSpells.db2", 0xC933095D, -1, 1);
        inline const WDC5TableSchema GlyphProperties = makeScalarSchema("GlyphProperties.db2", 0x2601475B, -1, 4);
        inline const WDC5TableSchema Achievement = makeScalarSchema("Achievement.db2", 0x6FC5281B, 3, 19);
        inline const WDC5TableSchema Criteria = makeScalarSchema("Criteria.db2", 0x53D03513, 0, 12);
        inline const WDC5TableSchema CriteriaTree = makeScalarSchema("CriteriaTree.db2", 0xFF97AE12, -1, 7);

        // Battle pets: the breed and species states take their breed and species from the relation data
        inline const WDC5TableSchema BattlePetSpecies = makeScalarSchema("BattlePetSpecies.db2", 0x589BE282, 2, 12);
        inline const WDC5TableSchema BattlePetBreedState = makeScalarSchema("BattlePetBreedState.db2", 0x030182F5, -1, 2);
        inline const WDC5TableSchema BattlePetSpeciesState = makeScalarSchema("BattlePetSpeciesState.db2", 0x2ABE26A5, -1, 2);
        inline const WDC5TableSchema BattlePetBreedQuality = makeScalarSchema("BattlePetBreedQuality.db2", 0xF3E3FDFC, -1, 3);

        // Items
        inline const WDC5TableSchema Item = makeScalarSchema("Item.db2", 0x996192AA, -1, 15);
        inline const WDC5TableSchema ItemAppearance = makeScalarSchema("ItemAppearance.db2", 0x481C4281, -1, 5);
        inline const WDC5TableSchema ItemModifiedAppearance = makeScalarSchema("ItemModifiedAppearance.db2", 0x03A6C979, 0, 7);
        inline const WDC5TableSchema ItemEffect = makeScalarSchema("ItemEffect.db2", 0x4CA77678, -1, 9);
        inline const WDC5TableSchema ItemXItemEffect = makeScalarSchema("ItemXItemEffect.db2", 0x96F083AD, -1, 1);
        inline const WDC5TableSchema ItemExtendedCost = makeSchemaWithArrays("ItemExtendedCost.db2", 0x22331DAB, -1, 11, { { 7, 5 }, { 8, 5 }, { 9, 5 }, { 10, 5 } });
        inline const WDC5TableSchema ItemSet = makeSchemaWithArrays("ItemSet.db2", 0xF79068A4, -1, 5, { { 4, 17 } });
        inline const WDC5TableSchema ItemSetSpell = makeScalarSchema("ItemSetSpell.db2", 0x2666A73F, -1, 4);
        inline const WDC5TableSchema ItemCurrencyCost = makeScalarSchema("ItemCurrencyCost.db2", 0x8950022B, -1, 1);
        inline const WDC5TableSchema ItemLimitCategory = makeScalarSchema("ItemLimitCategory.db2", 0xB0A784BD, -1, 3);
        inline const WDC5TableSchema DurabilityCosts = makeSchemaWithArrays("DurabilityCosts.db2", 0x917714A1, -1, 2, { { 0, 21 }, { 1, 8 } });
        inline const WDC5TableSchema DurabilityQuality = makeScalarSchema("DurabilityQuality.db2", 0x08763C54, -1, 1);

        // Skills, talents, spells
        inline const WDC5TableSchema SkillLine = makeScalarSchema("SkillLine.db2", 0x6763217C, 5, 15);
        inline const WDC5TableSchema SkillLineAbility = makeSchemaWithArrays("SkillLineAbility.db2", 0xF98AA48E, 2, 17, { { 16, 2 } });
        inline const WDC5TableSchema Talent = makeSchemaWithArrays("Talent.db2", 0x147B0045, -1, 14, { { 10, 2 }, { 11, 9 }, { 12, 3 }, { 13, 3 } });
        inline const WDC5TableSchema SpellName = makeScalarSchema("SpellName.db2", 0x782EE721, -1, 1);
        inline const WDC5TableSchema SpellAuraOptions = makeSchemaWithArrays("SpellAuraOptions.db2", 0x95F4D5E1, -1, 7, { { 6, 2 } });
        inline const WDC5TableSchema SpellAuraRestrictions = makeScalarSchema("SpellAuraRestrictions.db2", 0xE7F2E213, -1, 13);
        inline const WDC5TableSchema SpellCastTimes = makeScalarSchema("SpellCastTimes.db2", 0x75B6BD3A, -1, 2);
        inline const WDC5TableSchema SpellCastingRequirements = makeScalarSchema("SpellCastingRequirements.db2", 0x02074290, -1, 7);
        inline const WDC5TableSchema SpellCategories = makeScalarSchema("SpellCategories.db2", 0x679EF94C, -1, 9);
        inline const WDC5TableSchema SpellClassOptions = makeSchemaWithArrays("SpellClassOptions.db2", 0x1F8E4FD7, -1, 4, { { 3, 4 } });
        inline const WDC5TableSchema SpellCooldowns = makeScalarSchema("SpellCooldowns.db2", 0xDC945B8C, -1, 5);
        inline const WDC5TableSchema SpellDuration = makeScalarSchema("SpellDuration.db2", 0xA931BD2B, -1, 3);
        inline const WDC5TableSchema SpellEffect = makeSchemaWithArrays("SpellEffect.db2", 0x5362E3D4, -1, 29, { { 25, 2 }, { 26, 2 }, { 27, 4 }, { 28, 2 } });
        inline const WDC5TableSchema SpellEquippedItems = makeScalarSchema("SpellEquippedItems.db2", 0xD9177916, -1, 4);
        inline const WDC5TableSchema SpellInterrupts = makeSchemaWithArrays("SpellInterrupts.db2", 0x6FFC0306, -1, 4, { { 2, 2 }, { 3, 2 } });
        inline const WDC5TableSchema SpellLevels = makeScalarSchema("SpellLevels.db2", 0x7EB86FDC, -1, 5);
        inline const WDC5TableSchema SpellMisc = makeSchemaWithArrays("SpellMisc.db2", 0x434B3607, -1, 16, { { 0, 17 } });
        inline const WDC5TableSchema SpellPower = makeScalarSchema("SpellPower.db2", 0x61AD223F, 0, 14);
        inline const WDC5TableSchema SpellRadius = makeScalarSchema("SpellRadius.db2", 0xF9A913EE, -1, 4);
        inline const WDC5TableSchema SpellRange = makeSchemaWithArrays("SpellRange.db2", 0xADA13705, -1, 5, { { 3, 2 }, { 4, 2 } });
        inline const WDC5TableSchema SpellReagents = makeSchemaWithArrays("SpellReagents.db2", 0xFF40D202, -1, 5, { { 1, 8 }, { 2, 8 }, { 3, 8 }, { 4, 8 } });
        inline const WDC5TableSchema SpellScaling = makeScalarSchema("SpellScaling.db2", 0x9FC07797, -1, 3);
        inline const WDC5TableSchema SpellShapeshift = makeSchemaWithArrays("SpellShapeshift.db2", 0xE9111399, -1, 4, { { 2, 2 }, { 3, 2 } });
        inline const WDC5TableSchema SpellShapeshiftForm = makeSchemaWithArrays("SpellShapeshiftForm.db2", 0xEE25A6A2, -1, 10, { { 9, 8 } });
        inline const WDC5TableSchema SpellTargetRestrictions = makeScalarSchema("SpellTargetRestrictions.db2", 0x1EAB753E, -1, 7);
        inline const WDC5TableSchema SpellTotems = makeSchemaWithArrays("SpellTotems.db2", 0x03B23619, -1, 3, { { 1, 2 }, { 2, 2 } });
        inline const WDC5TableSchema SpellItemEnchantment = makeSchemaWithArrays("SpellItemEnchantment.db2", 0x952B72B2, -1, 24, { { 4, 3 }, { 5, 3 }, { 6, 3 }, { 8, 3 } });
        inline const WDC5TableSchema SpellXSpellVisual = makeScalarSchema("SpellXSpellVisual.db2", 0x7994A890, 0, 12);
    }
}
