/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#if !defined(AE_FOREVER)
#error "ForeverUpdateFields.hpp is only valid for the Forever client profile."
#endif

#include "WoWGuid.hpp"

#include <array>
#include <bitset>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace AscEmu::Version::Forever::Fields
{
    inline constexpr uint32_t SchemaBuild = 69913;

    struct Vec2 { float x = 0.0f; float y = 0.0f; };
    struct Vec3 { float x = 0.0f; float y = 0.0f; float z = 0.0f; };
    struct Quaternion { float x = 0.0f; float y = 0.0f; float z = 0.0f; float w = 1.0f; };

    struct SpellCastVisual
    {
        int32_t spellXSpellVisualId = 0;
        int32_t scriptVisualId = 0;
    };

    struct ChrCustomizationChoice
    {
        uint32_t optionId = 0;
        uint32_t choiceId = 0;
    };

    struct ItemEnchantment
    {
        int32_t id = 0;
        uint32_t duration = 0;
        int16_t charges = 0;
        uint16_t inactive = 0;
    };

    struct ItemMod
    {
        uint8_t type = 0;
        int32_t value = 0;
    };

    struct ArtifactPower
    {
        int16_t artifactPowerId = 0;
        uint8_t purchasedRank = 0;
        uint8_t currentRankWithBonus = 0;
    };

    struct SocketedGem
    {
        int32_t itemId = 0;
        uint8_t context = 0;
        std::array<uint16_t, 16> bonusListIds{};
    };

    struct VisibleItem
    {
        bool hasTransmog = false;
        bool hasIllusion = false;
        int32_t itemId = 0;
        int32_t secondaryItemModifiedAppearanceId = 0;
        int32_t conditionalItemAppearanceId = 0;
        uint16_t itemAppearanceModId = 0;
        uint16_t itemVisual = 0;
        uint32_t itemModifiedAppearanceId = 0;
        // 1.60.1.69913 captures contain one additional uint32 in VisibleItem.
        // Its semantic meaning is not identified yet; keep it explicit instead of hiding it as padding.
        uint32_t field69913 = 0;
        uint8_t transmogSlotOption = 0;
        uint8_t sheatheCategory = 0;
    };

    struct PassiveSpellHistory
    {
        int32_t spellId = 0;
        int32_t auraSpellId = 0;
    };

    struct UnitChannel
    {
        int32_t spellId = 0;
        SpellCastVisual spellVisual{};
        uint32_t startTimeMs = 0;
        uint32_t duration = 0;
    };

    struct UnitAssistActionData
    {
        uint8_t type = 0;
        std::string playerName;
        uint32_t virtualRealmAddress = 0;
    };

    struct QuestLog
    {
        int32_t questId = 0;
        uint16_t stateFlags = 0;
        int64_t endTime = 0;
        uint32_t objectiveFlags = 0;
        uint32_t enabledObjectivesMask = 0;
        std::array<int16_t, 24> objectiveProgress{};
    };

    struct SkillInfo
    {
        std::array<uint16_t, 300> skillLineId{};
        std::array<uint16_t, 300> skillStep{};
        std::array<uint16_t, 300> skillRank{};
        std::array<uint16_t, 300> skillStartingRank{};
        std::array<uint16_t, 300> skillMaxRank{};
        std::array<int16_t, 300> skillTempBonus{};
        std::array<uint16_t, 300> skillPermBonus{};
    };

    // These nested records are intentionally represented as semantic payload records here.
    // Their exact sub-field serializers are implemented separately from the top-level field schema.
    struct DynamicRecord { std::vector<uint8_t> data; };


    struct ZonePlayerForcedReaction
    {
        int32_t factionId = 0;
        int32_t reaction = 0;
    };

    struct CtrOptions
    {
        std::vector<uint32_t> conditionalFlags;
        uint8_t factionGroup = 0;
        uint32_t chromieTimeExpansionMask = 0;
        // 1.60.1.69913 carries one additional 32-bit scalar before the
        // conditional flag payload. Semantics are not known yet.
        uint32_t unknown69913 = 0;
    };

    struct DungeonScoreMapSummary
    {
        int32_t challengeModeId = 0;
        float mapScore = 0.0f;
        int32_t bestRunLevel = 0;
        int32_t bestRunDurationMs = 0;
        bool finishedSuccess = false;
        uint8_t unknown1110 = 0;
    };

    struct DungeonScoreSummary
    {
        float overallScoreCurrentSeason = 0.0f;
        float ladderScoreCurrentSeason = 0.0f;
        std::vector<DungeonScoreMapSummary> runs;
    };

    struct LeaverInfo
    {
        bool isLeaver = false;
        WoWGuid bnetAccountGuid;
        float leaveScore = 0.0f;
        uint32_t seasonId = 0;
        uint32_t totalLeaves = 0;
        uint32_t totalSuccesses = 0;
        int32_t consecutiveSuccesses = 0;
        int64_t lastPenaltyTime = 0;
        int64_t leaverExpirationTime = 0;
        int32_t flags = 0;
    };

    struct CustomTabardInfo
    {
        int32_t emblemStyle = 0;
        int32_t emblemColor = 0;
        int32_t borderStyle = 0;
        int32_t borderColor = 0;
        int32_t backgroundColor = 0;
    };

    struct NpcAsPlayerInfo
    {
        int32_t field0 = 0;
        int32_t characterLoadoutId = 0;
        int32_t creatureId = 0;
        Vec3 locWorldSpace{};
        float facingWorldSpace = 0.0f;
        WoWGuid transportGuid;
    };

    struct ItemBonuses
    {
        uint8_t context = 0;
        std::vector<uint32_t> bonusListIds;
    };

    struct ItemInstanceMod
    {
        uint8_t type = 0;
        int32_t value = 0;
    };

    struct ItemInstance
    {
        int32_t itemId = 0;
        std::optional<ItemBonuses> itemBonus;
        std::vector<ItemInstanceMod> modifications;
    };

    struct TransmogOutfitDataInfo
    {
        bool situationsEnabled = false;
        uint8_t setType = 0;
        std::string name;
        uint32_t icon = 0;
    };

    struct TransmogOutfitSituationInfo
    {
        uint32_t situationId = 0;
        uint32_t specId = 0;
        uint32_t loadoutId = 0;
        uint32_t equipmentSetId = 0;
    };

    struct TransmogOutfitSlotData
    {
        int8_t slot = 0;
        uint8_t slotOption = 0;
        uint8_t sheatheCategory = 0;
        uint32_t itemModifiedAppearanceId = 0;
        uint8_t appearanceDisplayType = 0;
        uint32_t spellItemEnchantmentId = 0;
        uint8_t illusionDisplayType = 0;
        uint32_t flags = 0;
    };

    struct TransmogOutfitData
    {
        std::vector<TransmogOutfitSituationInfo> situations;
        std::vector<TransmogOutfitSlotData> slots;
        uint32_t id = 0;
        TransmogOutfitDataInfo outfitInfo{};
        uint32_t flags = 0;
    };

    struct TransmogOutfitMetadata
    {
        bool locked = false;
        uint8_t situationTrigger = 0;
        uint32_t transmogOutfitId = 0;
        uint8_t stampedOptionMainHand = 0;
        uint8_t stampedOptionOffHand = 0;
        float costMod = 0.0f;
    };

    struct ObjectData
    {
        static inline constexpr std::size_t ChangeMaskSize = 4;
        static inline constexpr std::size_t GroupBit = 0;
        static inline constexpr std::size_t EntryIdBit = 1;
        static inline constexpr std::size_t DynamicFlagsBit = 2;
        static inline constexpr std::size_t ScaleBit = 3;

        int32_t entryId = 0;
        uint32_t dynamicFlags = 0;
        float scale = 1.0f;
        std::bitset<ChangeMaskSize> changes{};

        void markChanged(std::size_t bit) { changes.set(GroupBit); changes.set(bit); }
        void clearChanges() { changes.reset(); }
        bool hasChanges() const { return changes.any(); }
    };

    struct ItemData
    {
        static inline constexpr std::size_t ChangeMaskSize = 41;
        static inline constexpr std::size_t ArtifactPowersBit = 1;
        static inline constexpr std::size_t GemsBit = 2;
        static inline constexpr std::size_t OwnerBit = 3;
        static inline constexpr std::size_t ContainedInBit = 4;
        static inline constexpr std::size_t CreatorBit = 5;
        static inline constexpr std::size_t GiftCreatorBit = 6;
        static inline constexpr std::size_t StackCountBit = 7;
        static inline constexpr std::size_t ExpirationBit = 8;
        static inline constexpr std::size_t DynamicFlagsBit = 9;
        static inline constexpr std::size_t DurabilityBit = 10;
        static inline constexpr std::size_t MaxDurabilityBit = 11;
        static inline constexpr std::size_t CreatePlayedTimeBit = 12;
        static inline constexpr std::size_t ContextBit = 13;
        static inline constexpr std::size_t CreateTimeBit = 14;
        static inline constexpr std::size_t ArtifactXpBit = 15;
        static inline constexpr std::size_t ItemAppearanceModIdBit = 16;
        static inline constexpr std::size_t ModifiersBit = 17;
        static inline constexpr std::size_t ZoneFlagsBit = 18;
        static inline constexpr std::size_t ItemBonusKeyBit = 19;
        static inline constexpr std::size_t DebugItemLevelBit = 20;
        static inline constexpr std::size_t SpellChargesGroupBit = 21;
        static inline constexpr std::size_t SpellChargesFirstBit = 22;
        static inline constexpr std::size_t EnchantmentGroupBit = 27;
        static inline constexpr std::size_t EnchantmentFirstBit = 28;

        std::bitset<ChangeMaskSize> changes{};
        std::vector<ArtifactPower> artifactPowers;
        std::vector<SocketedGem> gems;
        WoWGuid owner;
        WoWGuid containedIn;
        WoWGuid creator;
        WoWGuid giftCreator;
        uint32_t stackCount = 0;
        uint32_t expiration = 0;
        uint32_t dynamicFlags = 0;
        uint32_t durability = 0;
        uint32_t maxDurability = 0;
        uint32_t createPlayedTime = 0;
        uint8_t context = 0;
        int64_t createTime = 0;
        uint64_t artifactXp = 0;
        uint8_t itemAppearanceModId = 0;
        std::vector<ItemMod> modifiers;
        uint32_t zoneFlags = 0;
        DynamicRecord itemBonusKey;
        uint16_t debugItemLevel = 0;
        std::array<int32_t, 5> spellCharges{};
        std::array<ItemEnchantment, 13> enchantment{};

        void markChanged(std::size_t bit) { changes.set(bit & ~std::size_t(31)); changes.set(bit); }
        void markArrayChanged(std::size_t groupBit, std::size_t elementBit) { markChanged(groupBit); changes.set(elementBit); }
        void clearChanges() { changes.reset(); }
        bool hasChanges() const { return changes.any(); }
    };

    struct ContainerData
    {
        static inline constexpr std::size_t ChangeMaskSize = 101;
        static inline constexpr std::size_t NumSlotsBit = 1;
        static inline constexpr std::size_t SlotsGroupBit = 2;
        static inline constexpr std::size_t SlotsFirstBit = 3;

        std::bitset<ChangeMaskSize> changes{};
        uint32_t numSlots = 0;
        std::array<WoWGuid, 98> slots{};

        void markChanged(std::size_t bit) { changes.set(bit & ~std::size_t(31)); changes.set(bit); }
        void markArrayChanged(std::size_t groupBit, std::size_t elementBit) { markChanged(groupBit); changes.set(elementBit); }
        void clearChanges() { changes.reset(); }
        bool hasChanges() const { return changes.any(); }
    };

    struct UnitData
    {
        // Retail 1.60.1.69913 UnitData uses a 230-bit mask.
        // The previous AscEmu table was stale by two bits from Race onward because
        // BattlePetAttachedToDecorGUID/BattlePetDecorHouseGUID were present in the
        // create structure but missing from the dirty-bit numbering.
        static inline constexpr std::size_t ChangeMaskSize = 230;
        static inline constexpr std::size_t DisplayIdBit = 6;
        static inline constexpr std::size_t NpcFlagsBit = 7;
        static inline constexpr std::size_t NpcFlags2Bit = 8;
        static inline constexpr std::size_t CharmBit = 14;
        static inline constexpr std::size_t SummonBit = 15;
        static inline constexpr std::size_t CritterBit = 16;
        static inline constexpr std::size_t CharmedByBit = 17;
        static inline constexpr std::size_t SummonedByBit = 18;
        static inline constexpr std::size_t CreatedByBit = 19;
        static inline constexpr std::size_t DemonCreatorBit = 20;
        static inline constexpr std::size_t LookAtControllerTargetBit = 21;
        static inline constexpr std::size_t TargetBit = 22;
        static inline constexpr std::size_t BattlePetCompanionGuidBit = 23;
        static inline constexpr std::size_t BattlePetDbIdBit = 24;
        static inline constexpr std::size_t BattlePetAttachedToDecorGuidBit = 25;
        static inline constexpr std::size_t BattlePetDecorHouseGuidBit = 26;
        static inline constexpr std::size_t ChannelDataBit = 27;
        static inline constexpr std::size_t SpellEmpowerStageBit = 28;
        static inline constexpr std::size_t SummonedByHomeRealmBit = 29;
        static inline constexpr std::size_t RaceBit = 30;
        static inline constexpr std::size_t ClassIdBit = 31;
        static inline constexpr std::size_t PlayerClassIdBit = 33;
        static inline constexpr std::size_t SexBit = 34;
        static inline constexpr std::size_t CreatureTypeBit = 35;
        static inline constexpr std::size_t DisplayPowerBit = 36;
        static inline constexpr std::size_t OverrideDisplayPowerBit = 37;
        static inline constexpr std::size_t HealthBit = 38;
        static inline constexpr std::size_t MaxHealthBit = 39;
        static inline constexpr std::size_t LevelBit = 40;
        static inline constexpr std::size_t EffectiveLevelBit = 41;
        static inline constexpr std::size_t ContentTuningBit = 42;
        static inline constexpr std::size_t ScalingLevelMinBit = 43;
        static inline constexpr std::size_t ScalingLevelMaxBit = 44;
        static inline constexpr std::size_t ScalingLevelDeltaBit = 45;
        static inline constexpr std::size_t ScalingFactionGroupBit = 46;
        static inline constexpr std::size_t FactionTemplateBit = 47;
        static inline constexpr std::size_t FlagsBit = 48;
        static inline constexpr std::size_t Flags2Bit = 49;
        static inline constexpr std::size_t Flags3Bit = 50;
        static inline constexpr std::size_t Flags4Bit = 51;
        static inline constexpr std::size_t AuraStateBit = 52;
        static inline constexpr std::size_t RangedAttackRoundBaseTimeBit = 53;
        static inline constexpr std::size_t BoundingRadiusBit = 54;
        static inline constexpr std::size_t CombatReachBit = 55;
        static inline constexpr std::size_t DisplayScaleBit = 56;
        static inline constexpr std::size_t CreatureFamilyBit = 57;
        static inline constexpr std::size_t OverrideCreatureTypeBit = 58;
        static inline constexpr std::size_t NativeDisplayIdBit = 59;
        static inline constexpr std::size_t NativeXDisplayScaleBit = 60;
        static inline constexpr std::size_t MountDisplayIdBit = 61;
        static inline constexpr std::size_t CosmeticMountDisplayIdBit = 62;
        static inline constexpr std::size_t MinDamageBit = 63;
        static inline constexpr std::size_t MaxDamageBit = 65;
        static inline constexpr std::size_t MinOffHandDamageBit = 66;
        static inline constexpr std::size_t MaxOffHandDamageBit = 67;
        static inline constexpr std::size_t StandStateBit = 68;
        static inline constexpr std::size_t PetTalentPointsBit = 69;
        static inline constexpr std::size_t VisFlagsBit = 70;
        static inline constexpr std::size_t AnimTierBit = 71;
        static inline constexpr std::size_t PetNumberBit = 72;
        static inline constexpr std::size_t PetNameTimestampBit = 73;
        static inline constexpr std::size_t PetExperienceBit = 74;
        static inline constexpr std::size_t PetNextLevelExperienceBit = 75;
        static inline constexpr std::size_t ModCastingSpeedBit = 76;
        static inline constexpr std::size_t ModCastingSpeedNegBit = 77;
        static inline constexpr std::size_t ModSpellHasteBit = 78;
        static inline constexpr std::size_t ModHasteBit = 79;
        static inline constexpr std::size_t ModRangedHasteBit = 80;
        static inline constexpr std::size_t ModHasteRegenBit = 81;
        static inline constexpr std::size_t ModTimeRateBit = 82;
        static inline constexpr std::size_t CreatedBySpellBit = 83;
        static inline constexpr std::size_t EmoteStateBit = 84;
        static inline constexpr std::size_t BaseManaBit = 85;
        static inline constexpr std::size_t BaseHealthBit = 86;
        static inline constexpr std::size_t SheatheStateBit = 87;
        static inline constexpr std::size_t PvpFlagsBit = 88;
        static inline constexpr std::size_t PetFlagsBit = 89;
        static inline constexpr std::size_t ShapeshiftFormBit = 90;

        // The verified 69913 differential updates line up
        // with the modern 230-bit grouping (for example bits 48/49/52/90 and
        // the 148..150 power island). Keep the complete array groups aligned.
        static inline constexpr std::size_t PowerGroupBit = 139;
        static inline constexpr std::size_t PowerFirstBit = 140;
        static inline constexpr std::size_t MaxPowerFirstBit = 150;
        static inline constexpr std::size_t AttackRoundBaseTimeGroupBit = 184;
        static inline constexpr std::size_t AttackRoundBaseTimeFirstBit = 185;
        static inline constexpr std::size_t StatsGroupBit = 187;
        static inline constexpr std::size_t StatsFirstBit = 188;
        static inline constexpr std::size_t StatPosBuffFirstBit = 193;
        static inline constexpr std::size_t StatNegBuffFirstBit = 198;
        static inline constexpr std::size_t StatSupportBuffFirstBit = 203;
        static inline constexpr std::size_t ResistancesGroupBit = 208;
        static inline constexpr std::size_t ResistancesFirstBit = 209;
        static inline constexpr std::size_t BonusResistanceModsFirstBit = 216;
        static inline constexpr std::size_t ManaCostModifierFirstBit = 223;

        std::bitset<ChangeMaskSize> changes{};

        void markChanged(std::size_t bit) { changes.set(bit & ~std::size_t(31)); changes.set(bit); }
        void markArrayChanged(std::size_t groupBit, std::size_t elementBit) { markChanged(groupBit); changes.set(elementBit); }
        void clearChanges() { changes.reset(); }
        bool hasChanges() const { return changes.any(); }

        // Forever 1.60.1.69913 CREATE wire order.
        // Semantic names are kept only where current captures strongly support them.
        bool field314 = false; // final bit; semantic meaning not proven
        std::vector<uint32_t> stateWorldEffectIds;
        std::vector<PassiveSpellHistory> passiveSpells;
        std::vector<int32_t> worldEffects;
        std::vector<WoWGuid> channelObjects;

        int32_t displayId = 0;
        uint32_t npcFlags = 0;
        uint32_t npcFlags2 = 0;
        uint32_t stateSpellVisualId = 0;
        uint32_t stateAnimId = 0;
        uint32_t stateAnimKitId = 0;
        uint32_t stateWorldEffectsQuestObjectiveId = 0;
        int32_t spellOverrideNameId = 0;

        WoWGuid charm;
        WoWGuid summon;
        WoWGuid critter;
        WoWGuid charmedBy;
        WoWGuid summonedBy;
        WoWGuid createdBy;
        WoWGuid demonCreator;
        WoWGuid lookAtControllerTarget;
        WoWGuid target;
        WoWGuid battlePetCompanionGuid;
        uint64_t battlePetDbId = 0;
        WoWGuid battlePetAttachedToDecorGuid;
        WoWGuid battlePetDecorHouseGuid;
        UnitChannel channelData{};
        int8_t spellEmpowerStage = 0;
        uint32_t summonedByHomeRealm = 0;
        uint8_t race = 0;
        uint8_t classId = 0;
        uint8_t playerClassId = 0;
        uint8_t sex = 0;
        uint8_t creatureType = 0;
        uint8_t displayPower = 0;
        uint32_t overrideDisplayPowerId = 0;
        int64_t health = 0;

        std::array<int32_t, 10> power{};
        std::array<int32_t, 10> maxPower{};
        std::array<float, 10> powerRegenFlatModifier{};
        std::array<float, 10> powerRegenInterruptedFlatModifier{};

        int64_t maxHealth = 0;
        int32_t level = 0;
        int32_t effectiveLevel = 0;
        int32_t contentTuningId = 0;
        int32_t scalingLevelMin = 0;
        int32_t scalingLevelMax = 0;
        int32_t scalingLevelDelta = 0;
        uint8_t scalingFactionGroup = 0;
        int32_t factionTemplate = 0;

        // Verified 1.60.1.69913 creature-create wire order.
        // Example: Deputy Willem (entry 823) carries Flags=0x300, Flags2=0x800,
        // Flags3=0, unknown=0, AuraState=0x00D00000.
        std::array<VisibleItem, 3> virtualItems{};
        uint32_t unitFlags69913 = 0;
        uint32_t unitFlags2_69913 = 0;
        uint32_t unitFlags3_69913 = 0;
        uint32_t flags4_69913 = 0;
        uint32_t auraState69913 = 0;
        std::array<uint32_t, 2> attackRoundBaseTime{};
        uint32_t rangedAttackRoundBaseTime = 0;
        float boundingRadius = 0.0f;
        float combatReach = 0.0f;
        float displayScale = 1.0f;
        int32_t creatureFamily = 0;
        uint8_t overrideCreatureType = 0;
        int32_t nativeDisplayId = 0;
        float nativeXDisplayScale = 1.0f;
        int32_t mountDisplayId = 0;
        int32_t cosmeticMountDisplayId = 0;
        // Owner-visible weapon damage fields. Verified by the 69913 self-create
        // ordering and the existing Unit damage accessors: main hand min/max,
        // followed by off-hand min/max.
        float minDamage69913 = 0.0f;
        float maxDamage69913 = 0.0f;
        float minOffHandDamage69913 = 0.0f;
        float maxOffHandDamage69913 = 0.0f;
        // Verified in 73 ordinary 1.60.1.69913 creature creates (20 entries).
        // Wire order is StandState, PetTalentPoints, VisFlags, AnimTier.
        // The capture contains non-default VisFlags=5 on the generic hunter-pet
        // entry, providing an additional boundary/order check.
        uint8_t standState = 0;
        uint8_t petTalentPoints = 0;
        uint8_t visFlags = 0;
        uint8_t animTier = 0;
        uint32_t petNumber = 0;
        uint32_t petNameTimestamp = 0;
        uint32_t petExperience = 0;
        uint8_t unknownAfterPetExperience69913 = 0;
        uint32_t petNextLevelExperience = 0;
        float modCastingSpeed = 1.0f;
        float modCastingSpeedNeg = 1.0f;
        float modSpellHaste = 1.0f;
        float modHaste = 1.0f;
        float modRangedHaste = 1.0f;
        float modHasteRegen = 1.0f;
        float modTimeRate69913 = 1.0f;
        int32_t createdBySpell69913 = 0;
        int32_t emoteState69913 = 0;

        uint32_t unknownBeforeStats69913 = 0;
        // Verified against the 1.60.1.69913 self-create capture. The five
        // primary stats are Strength, Agility, Stamina, Intellect and Spirit.
        std::array<int32_t, 5> stats69913{};
        std::array<int32_t, 5> statPosBuff69913{};
        std::array<int32_t, 5> statNegBuff69913{};
        std::array<int32_t, 5> statSupportBuff69913{};
        // Resistance[0] is physical armor, followed by the six magic schools.
        std::array<int32_t, 7> resistances69913{};
        std::array<int32_t, 7> bonusResistanceMods69913{};
        std::array<int32_t, 7> manaCostModifier69913{};

        int32_t baseMana = 0;
        int32_t baseHealth = 0;
        // Verified 69913 creature-create byte quartet. Across the capture,
        // SheatheState varies 0/1, PvpFlags 0/1 and Generic Hunter Pet carries
        // PetFlags=2; ShapeshiftForm is zero in the sampled creatures.
        uint8_t sheatheState = 0;
        uint8_t pvpFlags = 0;
        uint8_t petFlags = 0;
        uint8_t shapeshiftForm = 0;

        int32_t attackPower69913 = 0;
        int32_t attackPowerModPos69913 = 0;
        int32_t attackPowerModNeg69913 = 0;
        float attackPowerMultiplier69913 = 0.0f;
        int32_t attackPowerModSupport69913 = 0;
        uint32_t unknownBeforeRangedAttackPower69913A = 0;
        uint32_t unknownBeforeRangedAttackPower69913B = 0;
        int32_t rangedAttackPower69913 = 0;
        int32_t rangedAttackPowerModPos69913 = 0;
        int32_t rangedAttackPowerModNeg69913 = 0;
        float rangedAttackPowerMultiplier69913 = 0.0f;
        int32_t rangedAttackPowerModSupport69913 = 0;
        int32_t mainHandWeaponAttackPower69913 = 0;
        int32_t offHandWeaponAttackPower69913 = 0;
        int32_t rangedWeaponAttackPower69913 = 0;
        int32_t setAttackSpeedAura69913 = 0;
        float lifesteal69913 = 0.0f;
        float minRangedDamage69913 = 0.0f;
        float maxRangedDamage69913 = 0.0f;
        float manaCostMultiplier69913 = 0.0f;

        float maxHealthModifier69913 = 1.0f;
        float hoverHeight69913 = 1.0f;
        int32_t minItemLevelCutoff69913 = 0;
        int32_t minItemLevel69913 = 0;
        int32_t maxItemLevel69913 = 0;
        int32_t azeriteItemLevel69913 = 0;
        int32_t wildBattlePetLevel69913 = 0;
        int32_t battlePetCompanionExperience69913 = 0;
        uint32_t battlePetCompanionNameTimestamp69913 = 0;
        int32_t interactSpellId69913 = 0;
        int32_t scaleDuration69913 = 0;
        int32_t looksLikeMountId69913 = 0;
        int32_t looksLikeCreatureId69913 = 0;
        int32_t lookAtControllerId69913 = 0;
        int32_t perksVendorItemId69913 = 0;
        int32_t taxiNodesId69913 = 0;
        WoWGuid unknownGuid0_69913;

        int32_t flightCapabilityId69913 = 0;
        float glideEventSpeedDivisor69913 = 0.0f;
        int32_t driveCapabilityId69913 = 0;
        int32_t maxHealthModifierFlatNeg69913 = 0;
        int32_t maxHealthModifierFlatPos69913 = 0;
        uint32_t silencedSchoolMask69913 = 0;
        uint32_t unknownBeforeCurrentAreaId69913 = 0;
        uint32_t currentAreaId = 0;
        float nameplateDistanceMod = 0.0f;
        float autoAttackRangeMod = 0.0f;

        struct OwnerExtension69913
        {
            std::array<uint8_t, 15> prefix{};
            WoWGuid guidA;
            WoWGuid guidB;
            std::array<uint8_t, 3> suffix{};
        } ownerExtension69913;

        WoWGuid nameplateAttachToGuid;
        std::optional<UnitAssistActionData> unknownOptionalRecord0_69913;
    };

    struct PlayerData
    {
        static inline constexpr std::size_t ChangeMaskSize = 326;

        // PlayerData uses the same 326-bit modern mask layout as the reference
        // 12.x structure. These names only describe dirty-mask positions; the
        // 69913 CREATE payload remains capture-driven below.
        static inline constexpr std::size_t HasQuestSessionBit = 1;
        static inline constexpr std::size_t HasLevelLinkBit = 2;
        static inline constexpr std::size_t CustomizationsBit = 3;
        static inline constexpr std::size_t RandomCustomizationsBit = 4;
        static inline constexpr std::size_t QuestSessionQuestLogBit = 5;
        static inline constexpr std::size_t ArenaCooldownsBit = 6;
        static inline constexpr std::size_t PetNamesBit = 7;
        static inline constexpr std::size_t VisualItemReplacementsBit = 8;
        static inline constexpr std::size_t DuelArbiterBit = 9;
        static inline constexpr std::size_t WowAccountBit = 10;
        static inline constexpr std::size_t BnetAccountBit = 11;
        static inline constexpr std::size_t GuildClubMemberIdBit = 12;
        static inline constexpr std::size_t LootTargetGuidBit = 13;
        static inline constexpr std::size_t PlayerFlagsBit = 14;
        static inline constexpr std::size_t PlayerFlagsExBit = 15;
        static inline constexpr std::size_t GuildRankBit = 16;
        static inline constexpr std::size_t GuildDeleteDateBit = 17;
        static inline constexpr std::size_t GuildLevelBit = 18;
        static inline constexpr std::size_t NativeSexBit = 19;
        static inline constexpr std::size_t InebriationBit = 20;
        static inline constexpr std::size_t PvpTitleBit = 21;
        static inline constexpr std::size_t ArenaFactionBit = 22;
        static inline constexpr std::size_t DuelTeamBit = 23;
        static inline constexpr std::size_t GuildTimeStampBit = 24;
        static inline constexpr std::size_t QuestLogQuestIdToIndexBit = 25;
        static inline constexpr std::size_t PlayerTitleBit = 26;
        static inline constexpr std::size_t FakeInebriationBit = 27;
        static inline constexpr std::size_t VirtualPlayerRealmBit = 28;
        static inline constexpr std::size_t CurrentSpecBit = 29;
        static inline constexpr std::size_t CombatTraitSubTreeBit = 30;
        static inline constexpr std::size_t TaxiMountAnimKitBit = 31;
        static inline constexpr std::size_t BattlePetBreedQualityBit = 33;
        static inline constexpr std::size_t HonorBit = 34;
        static inline constexpr std::size_t LogoutTimeBit = 35;
        static inline constexpr std::size_t NameBit = 36;
        static inline constexpr std::size_t OfferedAdventureQuestBit = 37;
        static inline constexpr std::size_t OfferedScriptedQuestBit = 38;
        static inline constexpr std::size_t CurrentBattlePetSpeciesBit = 39;
        static inline constexpr std::size_t CtrOptionsBit = 40;
        static inline constexpr std::size_t CovenantIdBit = 41;
        static inline constexpr std::size_t SoulbindIdBit = 42;
        static inline constexpr std::size_t DungeonScoreBit = 43;
        static inline constexpr std::size_t LeaverInfoBit = 44;
        static inline constexpr std::size_t SpectateTargetBit = 45;
        static inline constexpr std::size_t WorldLootSwapSlotBit = 46;
        static inline constexpr std::size_t DeclinedNamesBit = 47;
        static inline constexpr std::size_t PersonalTabardBit = 48;
        static inline constexpr std::size_t NpcAsPlayerInfoBit = 49;
        static inline constexpr std::size_t PartyTypeGroupBit = 50;
        static inline constexpr std::size_t PartyTypeFirstBit = 51;
        static inline constexpr std::size_t QuestLogGroupBit = 53;
        static inline constexpr std::size_t QuestLogFirstBit = 54;
        static inline constexpr std::size_t VisibleItemsGroupBit = 229;
        static inline constexpr std::size_t VisibleItemsFirstBit = 230;
        static inline constexpr std::size_t AvgItemLevelGroupBit = 249;
        static inline constexpr std::size_t AvgItemLevelFirstBit = 250;
        static inline constexpr std::size_t ForcedReactionsGroupBit = 256;
        static inline constexpr std::size_t ForcedReactionsFirstBit = 257;
        static inline constexpr std::size_t VisibleEquipableSpellsGroupBit = 289;
        static inline constexpr std::size_t VisibleEquipableSpellsFirstBit = 290;
        static inline constexpr std::size_t PlunderstormItemDisplayIdGroupBit = 306;
        static inline constexpr std::size_t PlunderstormItemDisplayIdFirstBit = 307;

        // Legacy aliases kept while existing runtime setters are migrated.
        static inline constexpr std::size_t UnknownChangeBit3_69913 = 3;
        static inline constexpr std::size_t UnknownChangeBit9_69913 = 9;
        static inline constexpr std::size_t UnknownChangeBit14_69913 = 14;
        static inline constexpr std::size_t UnknownChangeBit15_69913 = 15;
        static inline constexpr std::size_t UnknownChangeBit16_69913 = 16;
        static inline constexpr std::size_t UnknownChangeBit17_69913 = 17;
        static inline constexpr std::size_t UnknownChangeBit18_69913 = 18;
        static inline constexpr std::size_t UnknownChangeBit19_69913 = 19;
        static inline constexpr std::size_t UnknownChangeBit20_69913 = 20;
        static inline constexpr std::size_t UnknownChangeBit21_69913 = 21;
        static inline constexpr std::size_t UnknownChangeBit22_69913 = 22;
        static inline constexpr std::size_t UnknownChangeBit23_69913 = 23;
        static inline constexpr std::size_t UnknownChangeBit24_69913 = 24;
        static inline constexpr std::size_t UnknownChangeBit26_69913 = 26;
        static inline constexpr std::size_t UnknownChangeBit28_69913 = 28;
        static inline constexpr std::size_t UnknownChangeBit29_69913 = 29;
        static inline constexpr std::size_t UnknownChangeBit34_69913 = 34;
        static inline constexpr std::size_t UnknownChangeBit35_69913 = 35;
        static inline constexpr std::size_t UnknownChangeBit36_69913 = 36;
        static inline constexpr std::size_t UnknownChangeBit53_69913 = 53;
        static inline constexpr std::size_t UnknownChangeBit54_69913 = 54;
        static inline constexpr std::size_t UnknownChangeBit229_69913 = 229;
        static inline constexpr std::size_t UnknownChangeBit230_69913 = 230;

        std::bitset<ChangeMaskSize> changes{};

        void markChanged(std::size_t bit) { changes.set(bit & ~std::size_t(31)); changes.set(bit); }
        void markArrayChanged(std::size_t groupBit, std::size_t elementBit) { markChanged(groupBit); changes.set(elementBit); }
        void clearChanges() { changes.reset(); }
        bool hasChanges() const { return changes.any(); }

        // Forever 1.60.1.69913 CREATE wire order.
        // Only wire-verified semantics keep semantic names. Every other field
        // intentionally uses an unknown*69913 name, even when an older client
        // gives the slot a plausible semantic meaning.

        WoWGuid unknownGuid0_69913;
        WoWGuid unknownGuid1_69913;
        WoWGuid unknownGuid2_69913;
        uint64_t unknownU64_0_69913 = 0;
        WoWGuid unknownGuid3_69913;
        uint32_t unknownU32_0_69913 = 0;
        uint32_t unknownU32_1_69913 = 0;
        uint32_t unknownU32_2_69913 = 0;
        uint32_t unknownU32_3_69913 = 0;
        int32_t unknownI32_0_69913 = 0;

        // Wire-verified 5-byte structural block immediately before the two
        // customization-count fields. Internal semantics are unknown.
        std::array<uint8_t, 5> unknownBeforeCustomizationCounts69913{};

        // Wire-verified customization list. The second list occupies a proven
        // customization-shaped wire slot, but its semantic purpose is not yet proven.
        std::vector<ChrCustomizationChoice> customizations;
        std::vector<ChrCustomizationChoice> unknownCustomizationChoices0_69913;

        std::array<uint8_t, 2> unknownBytes0_69913{};
        uint8_t unknownU8_0_69913 = 0;
        uint8_t unknownU8_1_69913 = 0;
        uint8_t unknownU8_2_69913 = 0;
        uint8_t unknownU8_3_69913 = 0;
        uint32_t unknownU32_4_69913 = 0;
        int32_t unknownI32_1_69913 = 0;

        std::array<QuestLog, 175> unknownPartyRecords0_69913{};
        std::map<int32_t, int32_t> unknownPartyMap0_69913;
        std::vector<QuestLog> unknownPartyDynamicRecords0_69913;

        std::array<VisibleItem, 19> unknownVisibleItemRecords0_69913{};
        int32_t unknownI32_2_69913 = 0;
        int32_t unknownI32_3_69913 = 0;
        uint32_t unknownU32_5_69913 = 0;
        uint32_t unknownU32_6_69913 = 0;
        int32_t unknownI32_4_69913 = 0;
        int32_t unknownI32_5_69913 = 0;
        std::array<float, 6> unknownFloatArray0_69913{};
        uint8_t unknownU8_4_69913 = 0;
        int32_t unknownI32_6_69913 = 0;
        int64_t unknownI64_0_69913 = 0;

        std::vector<DynamicRecord> unknownDynamicRecords0_69913;
        std::array<ZonePlayerForcedReaction, 32> unknownFixedRecords0_69913{};
        int32_t unknownI32_7_69913 = 0;
        int32_t unknownI32_8_69913 = 0;
        int32_t unknownI32_9_69913 = 0;
        std::vector<DynamicRecord> unknownDynamicRecords1_69913;
        CtrOptions unknownCtrOptions0_69913{};
        int32_t unknownI32_10_69913 = 0;
        int32_t unknownI32_11_69913 = 0;
        DungeonScoreSummary unknownDungeonScore0_69913{};
        LeaverInfo unknownLeaverInfo0_69913{};
        WoWGuid unknownGuid4_69913;
        int32_t unknownI32_12_69913 = 0;
        std::array<ItemInstance, 16> unknownItemInstances0_69913{};
        std::vector<int32_t> unknownI32Vector0_69913;
        std::array<uint32_t, 19> attackRoundBaseTime{};
        CustomTabardInfo unknownCustomTabard0_69913{};
        NpcAsPlayerInfo unknownNpcAsPlayer0_69913{};

        // Wire-verified 33-byte structural block immediately before the
        // ChrCustomizationChoice payload. Internal semantics are unknown.
        std::array<uint8_t, 33> unknownBeforeCustomizationPayload69913{
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x41, 0x14, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
            0x0C, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x01
        };

        // Wire-verified 69913 name tail.
        std::string firstName;
        std::string lastName;

        // Three optional-name bits/payload positions are structurally present,
        // but their individual semantic assignments are not proven yet.
        bool unknownNameFlag0_69913 = false;
        bool unknownNameFlag1_69913 = false;
        std::optional<DynamicRecord> unknownOptionalNamePayload0_69913;
    };

    struct ActivePlayerData
    {
        static inline constexpr std::size_t ChangeMaskSize = 398;
        // Runtime candidate bit positions retained from the old mapping.
        // These bit positions remain provisional until differential VALUES testing confirms them.
        static inline constexpr std::size_t UnknownChangeBit56_69913 = 56;
        static inline constexpr std::size_t UnknownChangeBit57_69913 = 57;
        static inline constexpr std::size_t UnknownChangeBit58_69913 = 58;
        static inline constexpr std::size_t UnknownChangeBit59_69913 = 59;
        static inline constexpr std::size_t UnknownChangeBit60_69913 = 60;
        static inline constexpr std::size_t UnknownChangeBit61_69913 = 61;
        static inline constexpr std::size_t UnknownChangeBit62_69913 = 62;
        static inline constexpr std::size_t UnknownChangeBit163_69913 = 163;
        static inline constexpr std::size_t UnknownChangeBit164_69913 = 164;

        std::bitset<ChangeMaskSize> changes{};

        void markChanged(std::size_t bit) { changes.set(bit & ~std::size_t(31)); changes.set(bit); }
        void markArrayChanged(std::size_t groupBit, std::size_t elementBit) { markChanged(groupBit); changes.set(elementBit); }
        void clearChanges() { changes.reset(); }
        bool hasChanges() const { return changes.any(); }

        // -----------------------------------------------------------------
        // CREATE wire order for Forever 1.60.1.69913.
        // Keep this declaration order aligned with writeActivePlayerDataCreate.
        // Names inherited from older layouts are not automatically considered
        // proven Forever semantics; they remain until their regions are replaced
        // by wire-verified names/unknown blocks.
        // -----------------------------------------------------------------

        std::array<WoWGuid, 105> invSlots{};
        WoWGuid farsightObject;
        WoWGuid summonedBattlePetGuid;

        // The uint32 knownTitles count is emitted here on the CREATE wire.
        // The actual uint64 title payload is emitted later, after the outfit
        // cluster; keep the member itself at that later declaration position
        // so this struct mirrors wire payload order instead of count order.

        // WoW 1.60.1.69913: 80-byte zeroed prefix extension observed between
        // the known-titles count and the fixed core scalars. Its semantics are
        // intentionally left unknown until differential tests (for example
        // with the additional Forever bag/inventory state) can prove ownership.
        static inline constexpr std::size_t UnknownInventoryExtensionSize69913 = 80;
        std::array<uint8_t, UnknownInventoryExtensionSize69913> unknownInventoryExtension69913{};

        uint64_t coinage = 0;
        uint64_t accountBankCoinage = 0;
        int32_t xp = 0;
        int32_t nextLevelXp = 0;
        int32_t unknownAfterNextLevelXp69913 = 0;

        SkillInfo skill{};

        // WoW 1.60.1.69913: verified wire alignment plus the current retail
        // ActivePlayerData source layout identifies this 104-byte span exactly
        // as 26 consecutive 32-bit fields immediately following SkillInfo.
        int32_t characterPoints = 0;
        int32_t maxTalentTiers = 0;
        uint32_t trackCreatureMask = 0;
        float mainhandExpertise = 0.0f;
        float offhandExpertise = 0.0f;
        float rangedExpertise = 0.0f;
        float combatRatingExpertise = 0.0f;
        float blockPercentage = 0.0f;
        float dodgePercentage = 0.0f;
        float dodgePercentageFromAttribute = 0.0f;
        float parryPercentage = 0.0f;
        float parryPercentageFromAttribute = 0.0f;
        float critPercentage = 0.0f;
        float rangedCritPercentage = 0.0f;
        float offhandCritPercentage = 0.0f;
        float spellCritPercentage = 0.0f;
        int32_t shieldBlock = 0;
        float shieldBlockCritPercentage = 0.0f;
        float mastery = 0.0f;
        float speed = 0.0f;
        float avoidance = 0.0f;
        float sturdiness = 0.0f;
        int32_t versatility = 0;
        float versatilityBonus = 0.0f;
        float pvpPowerDamage = 0.0f;
        float pvpPowerHealing = 0.0f;

        // WoW 1.60.1.69913: structurally verified 180-byte create span.
        // The final 80 bytes form five repeated 16-byte records. Their two trailing
        // floats are 1.0f in all five entries.
        // Semantics are intentionally left neutral until differential tests
        // with different equipment can prove the meaning.
        struct PostSkillRecord69913
        {
            uint32_t unknown0 = 0;
            uint32_t unknown4 = 0;
            float multiplier0 = 0.0f;
            float multiplier1 = 0.0f;
        };

        static inline constexpr std::size_t PostSkillHeaderSize69913 = 98;
        static inline constexpr std::size_t PostSkillRecordCount69913 = 5;
        static inline constexpr std::size_t PostSkillTailSize69913 = 2;
        std::array<uint8_t, PostSkillHeaderSize69913> unknownPostSkillHeader69913{};
        std::array<PostSkillRecord69913, PostSkillRecordCount69913> unknownPostSkillRecords69913{};
        std::array<uint8_t, PostSkillTailSize69913> unknownPostSkillTail69913{};

        // WoW 1.60.1.69913: the first 922 bytes of the former 1089-byte
        // "pre-transmog" span are still semantically unresolved. They include
        // one outfit-related record whose exact schema is not yet proven. The
        // reference-specific text payload is neutralized. Keep only this prefix
        // opaque; everything after it is now emitted through typed outfit fields.
        static inline constexpr std::size_t UnknownBeforeOutfitSize69913 = 922;
        std::array<uint8_t, UnknownBeforeOutfitSize69913> unknownBeforeOutfit69913{};

        // Wire-verified Forever 69913 outfit cluster immediately following the
        // opaque prefix. The two scalar values precede the currently viewed
        // outfit in the reference create state; their semantics are not yet
        // proven, so keep neutral names while still generating them explicitly.
        uint32_t unknownOutfitScalar0_69913 = 0;
        uint32_t unknownOutfitScalar1_69913 = 0;
        TransmogOutfitData viewedOutfit{};

        // The reference create state contains two additional complete outfit
        // records after ViewedOutfit ("Outfit 1" and "Outfit"). Their container
        // semantics are still under verification, but the count + element wire
        // encoding and each TransmogOutfitData payload are wire-verified.
        std::vector<TransmogOutfitData> additionalOutfits69913;
        TransmogOutfitMetadata transmogMetadata{};

        // CREATE wire payload for the title entries. Its uint32 element count
        // is emitted in the prefix immediately after summonedBattlePetGuid.
        // Keeping the vector here makes the member declaration order follow
        // the actual payload order while preserving the split count/payload
        // encoding used by the wire format.
        std::vector<uint64_t> knownTitles;

        // Opaque minimal tail after the verified transmog metadata.
        // Current zero-state wire span is 2 bytes; semantics are not proven.
        static inline constexpr std::size_t UnknownAfterTransmogSize69913 = 2;
        std::array<uint8_t, UnknownAfterTransmogSize69913> unknownAfterTransmog69913{};
    };

    struct GameObjectData
    {
        // 1.60.1.69913 uses a 28-bit GameObject change mask. The complete
        // create payload and an all-fields VALUES sample from retail both
        // serialize 104 bytes after ObjectData. Bits 0..25 are structurally
        // identified; the final two uint32 values are still semantically
        // unknown and intentionally kept neutral until a sniff exercises them.
        static inline constexpr std::size_t ChangeMaskSize = 28;
        static inline constexpr std::size_t GroupBit = 0;
        static inline constexpr std::size_t StateWorldEffectIdsBit = 1;
        static inline constexpr std::size_t EnableDoodadSetsBit = 2;
        static inline constexpr std::size_t WorldEffectsBit = 3;
        static inline constexpr std::size_t DisplayIdBit = 4;
        static inline constexpr std::size_t SpellVisualIdBit = 5;
        static inline constexpr std::size_t StateSpellVisualIdBit = 6;
        static inline constexpr std::size_t SpawnTrackingStateAnimIdBit = 7;
        static inline constexpr std::size_t SpawnTrackingStateAnimKitIdBit = 8;
        static inline constexpr std::size_t StateWorldEffectsQuestObjectiveIdBit = 9;
        static inline constexpr std::size_t CreatedByBit = 10;
        static inline constexpr std::size_t GuildGuidBit = 11;
        static inline constexpr std::size_t FlagsBit = 12;
        static inline constexpr std::size_t FlagsBBit = 13;
        static inline constexpr std::size_t ParentRotationBit = 14;
        static inline constexpr std::size_t FactionTemplateBit = 15;
        static inline constexpr std::size_t StateBit = 16;
        static inline constexpr std::size_t TypeIdBit = 17;
        static inline constexpr std::size_t PercentHealthBit = 18;
        static inline constexpr std::size_t ArtKitBit = 19;
        static inline constexpr std::size_t CustomParamBit = 20;
        static inline constexpr std::size_t LevelBit = 21;
        static inline constexpr std::size_t AnimGroupInstanceBit = 22;
        static inline constexpr std::size_t UiWidgetItemIdBit = 23;
        static inline constexpr std::size_t UiWidgetItemQualityBit = 24;
        static inline constexpr std::size_t UiWidgetItemCountBit = 25;
        static inline constexpr std::size_t UnknownU32Bit26_69913 = 26;
        static inline constexpr std::size_t UnknownU32Bit27_69913 = 27;

        std::bitset<ChangeMaskSize> changes{};

        // Wire order for CREATE_OBJECT in build 69913.
        int32_t displayId = 0;
        uint32_t spellVisualId = 0;
        uint32_t stateSpellVisualId = 0;
        uint32_t spawnTrackingStateAnimId = 0;
        uint32_t spawnTrackingStateAnimKitId = 0;
        std::vector<uint32_t> stateWorldEffectIds;
        uint32_t stateWorldEffectsQuestObjectiveId = 0;
        WoWGuid createdBy;
        WoWGuid guildGuid;
        uint32_t flags = 0;
        uint32_t flagsB = 0;
        std::array<float, 4> parentRotation{0.0f, 0.0f, 0.0f, 1.0f};
        int32_t factionTemplate = 0;
        int32_t level = 0;
        int8_t state = 0;
        int8_t typeId = 0;
        uint8_t percentHealth = 255;
        uint8_t artKit = 0;
        std::vector<int32_t> enableDoodadSets;
        uint32_t customParam = 0;
        std::vector<int32_t> worldEffects;
        uint32_t animGroupInstance = 0;
        uint32_t uiWidgetItemId = 0;
        uint32_t uiWidgetItemQuality = 0;
        uint32_t uiWidgetItemCount = 0;
        uint32_t unknownU32_26_69913 = 0;
        uint32_t unknownU32_27_69913 = 0;

        void markChanged(std::size_t bit) { changes.set(GroupBit); changes.set(bit); }
        void clearChanges() { changes.reset(); }
        bool hasChanges() const { return changes.any(); }
    };

    struct DynamicObjectData
    {
        static inline constexpr std::size_t ChangeMaskSize = 7;
        static inline constexpr std::size_t CasterBit = 1;
        static inline constexpr std::size_t TypeBit = 2;
        static inline constexpr std::size_t SpellVisualBit = 3;
        static inline constexpr std::size_t SpellIdBit = 4;
        static inline constexpr std::size_t RadiusBit = 5;
        static inline constexpr std::size_t CastTimeBit = 6;

        std::bitset<ChangeMaskSize> changes{};
        WoWGuid caster;
        uint8_t type = 0;
        SpellCastVisual spellVisual{};
        int32_t spellId = 0;
        float radius = 0.0f;
        uint32_t castTime = 0;

        void markChanged(std::size_t bit) { changes.set(bit & ~std::size_t(31)); changes.set(bit); }
        void clearChanges() { changes.reset(); }
        bool hasChanges() const { return changes.any(); }
    };

    struct CorpseData
    {
        static inline constexpr std::size_t ChangeMaskSize = 33;
        static inline constexpr std::size_t CustomizationsBit = 1;
        static inline constexpr std::size_t DynamicFlagsBit = 2;
        static inline constexpr std::size_t OwnerBit = 3;
        static inline constexpr std::size_t PartyGuidBit = 4;
        static inline constexpr std::size_t GuildGuidBit = 5;
        static inline constexpr std::size_t DisplayIdBit = 6;
        static inline constexpr std::size_t RaceIdBit = 7;
        static inline constexpr std::size_t SexBit = 8;
        static inline constexpr std::size_t ClassBit = 9;
        static inline constexpr std::size_t FlagsBit = 10;
        static inline constexpr std::size_t FactionTemplateBit = 11;
        static inline constexpr std::size_t StateSpellVisualKitIdBit = 12;
        static inline constexpr std::size_t ItemsGroupBit = 13;
        static inline constexpr std::size_t ItemsFirstBit = 14;

        std::bitset<ChangeMaskSize> changes{};
        std::vector<ChrCustomizationChoice> customizations;
        uint32_t dynamicFlags = 0;
        WoWGuid owner;
        WoWGuid partyGuid;
        WoWGuid guildGuid;
        uint32_t displayId = 0;
        uint8_t raceId = 0;
        uint8_t sex = 0;
        uint8_t classId = 0;
        uint32_t flags = 0;
        int32_t factionTemplate = 0;
        uint32_t stateSpellVisualKitId = 0;
        std::array<uint32_t, 19> items{};

        void markChanged(std::size_t bit) { changes.set(bit & ~std::size_t(31)); changes.set(bit); }
        void markArrayChanged(std::size_t groupBit, std::size_t elementBit) { markChanged(groupBit); changes.set(elementBit); }
        void clearChanges() { changes.reset(); }
        bool hasChanges() const { return changes.any(); }
    };

    struct ScaleCurve
    {
        bool overrideActive = false;
        uint32_t startTimeOffset = 0;
        uint32_t parameterCurve = 0;
        std::array<Vec2, 2> points{};
    };

    struct VisualAnim
    {
        bool isDecay = false;
        std::optional<int16_t> animationDataId;
        uint32_t animKitId = 0;
        uint32_t animProgress = 0;
    };

    struct ForceSetAreaTriggerPositionAndRotation
    {
        WoWGuid triggerGuid;
        Vec3 pos{};
        Quaternion rotation{};
    };

    struct AreaTriggerActionSetPeriodModifier
    {
        int32_t field0 = 0;
        float field4 = 0.0f;
    };

    struct AreaTriggerData
    {
        static inline constexpr std::size_t ChangeMaskSize = 39;
        static inline constexpr std::size_t CasterBit = 7;
        static inline constexpr std::size_t DurationBit = 8;
        static inline constexpr std::size_t TimeToTargetBit = 9;
        static inline constexpr std::size_t SpellIdBit = 14;
        static inline constexpr std::size_t SpellForVisualsBit = 15;
        static inline constexpr std::size_t SpellVisualBit = 16;
        static inline constexpr std::size_t BoundsRadius2DBit = 17;
        static inline constexpr std::size_t CreatingEffectGuidBit = 19;
        static inline constexpr std::size_t OrbitPathTargetBit = 20;
        static inline constexpr std::size_t FlagsBit = 27;
        static inline constexpr std::size_t FacingBit = 34;
        static inline constexpr std::size_t PathTypeBit = 36;
        static inline constexpr std::size_t ShapeTypeBit = 37;

        std::bitset<ChangeMaskSize> changes{};
        ScaleCurve overrideScaleCurve{};
        ScaleCurve extraScaleCurve{};
        ScaleCurve overrideMoveCurveX{};
        ScaleCurve overrideMoveCurveY{};
        ScaleCurve overrideMoveCurveZ{};
        ScaleCurve unk1205Curve{};
        WoWGuid caster;
        uint32_t duration = 0;
        uint32_t timeToTarget = 0;
        uint32_t timeToTargetScale = 0;
        uint32_t timeToTargetExtraScale = 0;
        uint32_t timeToTargetPos = 0;
        uint32_t timeToTargetUnk1205Curve = 0;
        int32_t spellId = 0;
        int32_t spellForVisuals = 0;
        SpellCastVisual spellVisual{};
        float boundsRadius2D = 0.0f;
        uint32_t decalPropertiesId = 0;
        WoWGuid creatingEffectGuid;
        WoWGuid orbitPathTarget;
        Vec3 rollPitchYaw{};
        int32_t positionalSoundKitId = 0;
        uint32_t movementStartTime = 0;
        uint32_t creationTime = 0;
        float zOffset = 0.0f;
        std::optional<Vec3> targetRollPitchYaw;
        uint32_t flags = 0;
        VisualAnim visualAnim{};
        uint32_t scaleCurveId = 0;
        uint32_t facingCurveId = 0;
        uint32_t morphCurveId = 0;
        uint32_t moveCurveId = 0;
        float facing = 0.0f;
        std::optional<ForceSetAreaTriggerPositionAndRotation> forcedPositionAndRotation;
        int32_t pathType = 0;
        uint8_t shapeType = 0;
        AreaTriggerActionSetPeriodModifier periodModifier{};
        DynamicRecord pathData;
        DynamicRecord shapeData;

        void markChanged(std::size_t bit) { changes.set(bit & ~std::size_t(31)); changes.set(bit); }
        void clearChanges() { changes.reset(); }
        bool hasChanges() const { return changes.any(); }
    };

    struct PlayerObjectData
    {
        ObjectData object;
        UnitData unit;
        PlayerData player;
        ActivePlayerData active;
    };
}
