/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ObjectUpdate.hpp"

#include "Data/WoWObject.hpp"
#include "Data/WoWUnit.hpp"
#include "version/Forever/Fields/ForeverUpdateFields.hpp"
#include "Network/ByteBuffer.hpp"

#include <string>
#include <algorithm>
#include <cstring>
#include <limits>
#include <span>

namespace AscEmu::Version::Forever::ObjectUpdate
{
    namespace
    {
        constexpr uint8_t UPDATE_TYPE_CREATE_OBJECT_2 = 2;
        constexpr uint8_t OBJECT_TYPE_UNIT = 5;
        constexpr uint8_t OBJECT_TYPE_ACTIVE_PLAYER = 7;
        constexpr uint8_t OBJECT_TYPE_GAMEOBJECT = 8;

        // 69913 minimal stationary-unit create movement profile.
        // The variable GUID and position/orientation are generated per object;
        // this suffix was observed byte-identical across multiple stationary
        // 69913 stationary retail creature samples. Its individual fields are
        // intentionally left semantically unnamed until separately proven.
        inline constexpr std::array<uint8_t, 135> STATIONARY_UNIT_MOVEMENT_SUFFIX_69913 = {
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x80,0x3F,0x00,0x00,0x00,0x00,0x20,0x40,0x00,0x00,0x00,0x41,0x00,0x00,
            0x90,0x40,0x71,0x1C,0x97,0x40,0x00,0x00,0x20,0x40,0x00,0x00,0xE0,0x40,0x00,0x00,
            0x90,0x40,0xDB,0x0F,0x49,0x40,0xDB,0x0F,0x49,0x40,0x00,0x00,0x00,0x00,0x00,0x00,
            0x80,0x3F,0x00,0x00,0x00,0x40,0x00,0x00,0x82,0x42,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x40,0x40,0x00,0x00,0x20,0x41,0x00,0x00,0xC8,0x42,0xDB,0x0F,0xC9,0x3F,0xAA,0x61,
            0x1C,0x40,0xDB,0x0F,0x49,0x40,0xDB,0x0F,0xC9,0x40,0xDB,0x0F,0xC9,0x3F,0xE4,0xCB,
            0x96,0x40,0x00,0x00,0xF0,0x41,0x00,0x00,0xA0,0x42,0x00,0x00,0x30,0x40,0x00,0x00,
            0xE0,0x40,0xCD,0xCC,0xCC,0x3E,0x00
        };
        // 69913 self-player movement defaults. The live position and movement
        // flags are patched by the serializer below; the remaining bytes are
        // build-specific protocol defaults whose semantics are not yet named.
        inline constexpr std::array<uint8_t, 181> SELF_PLAYER_MOVEMENT_DEFAULTS_69913 = {
            0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0xC5,0x1C,0x5A,0xBA,0xCD,0xD7,0x0B,0xC6,
            0x35,0x7E,0x04,0xC3,0xF9,0x0F,0xA7,0x42,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,
            0x10,0x00,0x00,0x00,0x20,0x40,0x00,0x00,0xE0,0x40,0x00,0x00,0x90,0x40,0x71,0x1C,
            0x97,0x40,0x00,0x00,0x20,0x40,0x00,0x00,0xE0,0x40,0x00,0x00,0x90,0x40,0xDB,0x0F,
            0x49,0x40,0xDB,0x0F,0x49,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x00,0x40,0x00,0x00,0x82,0x42,0x00,0x00,0x80,0x3F,0x00,0x00,0x40,0x40,0x00,0x00,
            0x20,0x41,0x00,0x00,0xC8,0x42,0xDB,0x0F,0xC9,0x3F,0xAA,0x61,0x1C,0x40,0xDB,0x0F,
            0x49,0x40,0xDB,0x0F,0xC9,0x40,0xDB,0x0F,0xC9,0x3F,0xE4,0xCB,0x96,0x40,0x00,0x00,
            0xF0,0x41,0x00,0x00,0xA0,0x42,0x00,0x00,0x30,0x40,0x00,0x00,0xE0,0x40,0xCD,0xCC,
            0xCC,0x3E,0x80,0xB5,0x16,0x6C,0x00,0xCD,0xD7,0x0B,0xC6,0x35,0x7E,0x04,0xC3,0xF9,
            0x0F,0xA7,0x42,0x00,0x00
        };

    }

    namespace
    {
        void writeModernGuid(ByteBuffer& data, WoWGuid const& guid)
        {
            const std::vector<uint8_t> packed = guid.packModern();
            data.append(packed.data(), packed.size());
        }

        constexpr uint8_t SELF_FIELD_FLAGS_69913 = 0x07U;
        constexpr uint8_t FRAGMENT_CGOBJECT_69913 = 0x03U;
        constexpr uint8_t FRAGMENT_PLAYER_HOUSE_INFO_69913 = 0x21U;
        constexpr uint8_t FRAGMENT_PLAYER_INITIATIVE_69913 = 0x26U;

        // Retail Forever 1.60.1.69913 vendor creatures carry fragment 0x12
        // in addition to CGObject + Tag_Unit. Godric Rothgar was the controlled
        // runtime proof: without 0x12 right-click produced no interaction CMSG;
        // with 0x12 the client started sending the interaction request.
        constexpr uint8_t FRAGMENT_VENDOR_69913 = 0x12U;

        constexpr uint8_t FRAGMENT_TAG_UNIT_69913 = 0xCCU;
        constexpr uint8_t FRAGMENT_TAG_PLAYER_69913 = 0xCDU;
        constexpr uint8_t FRAGMENT_TAG_GAMEOBJECT_69913 = 0xCEU;
        constexpr uint8_t FRAGMENT_END_69913 = 0xFFU;


        void writeEmptyPlayerHouseInfoComponentCreate(ByteBuffer& data)
        {
            // Forever 69913 empty 0x21 component layout verified for the 69913 wire layout.
            data << uint32_t(0); // Field_8 count (owner)
            data << uint32_t(0); // Houses count
            data << uint32_t(0); // Field_88 count (owner)
            data << uint32_t(0); // Field_C0 count (owner)
            data << uint32_t(0); // Field_F8 count (owner)
            data << uint32_t(0); // Field_130 count (owner)

            // Verified Forever 69913 empty house defaults.
            // These two scalar fields are 1 and -1 even with no house data.
            data << int32_t(1) << int32_t(-1) << uint32_t(0);
            data << uint8_t(0);

            data << uint8_t(0); // EditorMode

            // Empty NeighborhoodOwnershipTransfer.
            writeModernGuid(data, WoWGuid());
            writeModernGuid(data, WoWGuid());
            data << uint8_t(0); // empty ownership name

            writeModernGuid(data, WoWGuid()); // CurrentHouse
        }

        void writeEmptyPlayerInitiativeComponentCreate(ByteBuffer& data)
        {
            writeModernGuid(data, WoWGuid()); // NeighborhoodGUID

            // Empty PlayerInitiativeInfo.
            data << int64_t(0);
            data << int32_t(0) << int32_t(0) << int32_t(0);
            data << float(0.0f) << float(0.0f) << float(0.0f);

            data << uint32_t(0); // CompletedTasks count
            data << uint32_t(0); // CompletedInitiatives count
            data << uint32_t(0); // Houses set count (owner)
        }

        void writeSpellCastVisualCreate(ByteBuffer& data, Fields::SpellCastVisual const& fields)
        {
            data << fields.spellXSpellVisualId << fields.scriptVisualId;
        }

        void writeUnitChannelCreate(ByteBuffer& data, Fields::UnitChannel const& fields)
        {
            data << fields.spellId;
            writeSpellCastVisualCreate(data, fields.spellVisual);
            data << fields.startTimeMs << fields.duration;
        }

        void writeVisibleItemCreate(ByteBuffer& data, Fields::VisibleItem const& fields)
        {
            data << fields.itemId << fields.secondaryItemModifiedAppearanceId << fields.conditionalItemAppearanceId << fields.itemAppearanceModId << fields.itemVisual << fields.itemModifiedAppearanceId << fields.field69913 << fields.transmogSlotOption << fields.sheatheCategory;
            data.writeBit(fields.hasTransmog);
            data.writeBit(fields.hasIllusion);
            data.flushBits();
        }

        void writePassiveSpellHistoryCreate(ByteBuffer& data, Fields::PassiveSpellHistory const& fields)
        {
            data << fields.spellId << fields.auraSpellId;
        }

        void writeUnitAssistActionDataCreate(ByteBuffer& data, Fields::UnitAssistActionData const& fields)
        {
            data << fields.type << fields.virtualRealmAddress;
            data.writeBits(fields.playerName.size(), 6);
            data.flushBits();
            if (!fields.playerName.empty())
                data.append(reinterpret_cast<uint8_t const*>(fields.playerName.data()), fields.playerName.size());
        }
    }

    namespace
    {
        void writeDynamicRecord(ByteBuffer& data, Fields::DynamicRecord const& record)
        {
            if (!record.data.empty())
                data.append(record.data.data(), record.data.size());
        }

        template <typename K>
        void writeDynamicRecordMap(ByteBuffer& data, std::map<K, Fields::DynamicRecord> const& values)
        {
            data << uint32_t(values.size());
            for (auto const& [key, value] : values)
            {
                data << key;
                writeDynamicRecord(data, value);
            }
        }

        void writeCustomizationCreate(ByteBuffer& data, Fields::ChrCustomizationChoice const& value)
        {
            data << value.optionId << value.choiceId;
        }

        void writeQuestLogCreate(ByteBuffer& data, Fields::QuestLog const& value)
        {
            data << value.questId << value.stateFlags;
            for (int16_t progress : value.objectiveProgress)
                data << progress;
            data << value.endTime << value.objectiveFlags << value.enabledObjectivesMask;
        }

        void writeSkillInfoCreate(ByteBuffer& data, Fields::SkillInfo const& value)
        {
            for (std::size_t i = 0; i < value.skillLineId.size(); ++i)
                data << value.skillLineId[i] << value.skillStep[i] << value.skillRank[i] << value.skillStartingRank[i] << value.skillMaxRank[i] << value.skillTempBonus[i] << value.skillPermBonus[i];
        }

        void writeZonePlayerForcedReactionCreate(ByteBuffer& data, Fields::ZonePlayerForcedReaction const& value)
        {
            data << value.factionId << value.reaction;
        }

        void writeCtrOptionsCreate(ByteBuffer& data, Fields::CtrOptions const& value)
        {
            // Forever 1.60.1.69913 capture layout:
            //   uint32 ConditionalFlagsCount
            //   uint8  FactionGroup
            //   uint32 ChromieTimeExpansionMask
            //   uint32 ConditionalFlags[Count]
            //
            // The previous serializer wrote unknown69913 as an extra fixed
            // uint32. With Count=0 that accidentally preserved the expected
            // 13-byte size, while encoding the wrong structure.
            data << uint32_t(value.conditionalFlags.size())
                 << value.factionGroup
                 << value.chromieTimeExpansionMask;

            for (uint32_t flag : value.conditionalFlags)
                data << flag;
        }

        void writeDungeonScoreSummaryCreate(ByteBuffer& data, Fields::DungeonScoreSummary const& value)
        {
            data << value.overallScoreCurrentSeason << value.ladderScoreCurrentSeason << uint32_t(value.runs.size());
            for (Fields::DungeonScoreMapSummary const& run : value.runs)
            {
                data << run.challengeModeId << run.mapScore << run.bestRunLevel << run.bestRunDurationMs << run.unknown1110;
                data.writeBit(run.finishedSuccess);
                data.flushBits();
            }
        }

        void writeCustomTabardInfoCreate(ByteBuffer& data, Fields::CustomTabardInfo const& value)
        {
            data << value.emblemStyle << value.emblemColor << value.borderStyle << value.borderColor << value.backgroundColor;
        }

        void writeNpcAsPlayerInfoCreate(ByteBuffer& data, Fields::NpcAsPlayerInfo const& value)
        {
            data << value.field0 << value.characterLoadoutId << value.creatureId;
            data << value.locWorldSpace.x << value.locWorldSpace.y << value.locWorldSpace.z << value.facingWorldSpace;
            writeModernGuid(data, value.transportGuid);
        }

        void writeItemInstanceCreate(ByteBuffer& data, Fields::ItemInstance const& value)
        {
            data << value.itemId;
            data.writeBit(value.itemBonus.has_value());
            data.writeBits(value.modifications.size(), 7);
            data.flushBits();
            for (Fields::ItemInstanceMod const& mod : value.modifications)
                data << mod.type << mod.value;
            if (value.itemBonus)
            {
                data << value.itemBonus->context << uint32_t(value.itemBonus->bonusListIds.size());
                for (uint32_t bonus : value.itemBonus->bonusListIds)
                    data << bonus;
            }
        }

        void writeTransmogOutfitDataInfoCreate(ByteBuffer& data, Fields::TransmogOutfitDataInfo const& value)
        {
            data << value.setType << value.icon;
            data.writeBits(value.name.size(), 8);
            data.writeBit(value.situationsEnabled);
            data.flushBits();
            if (!value.name.empty())
                data.append(reinterpret_cast<uint8_t const*>(value.name.data()), value.name.size());
        }

        void writeTransmogOutfitSituationInfoCreate(ByteBuffer& data, Fields::TransmogOutfitSituationInfo const& value)
        {
            data << value.situationId << value.specId << value.loadoutId << value.equipmentSetId;
        }

        void writeTransmogOutfitSlotDataCreate(ByteBuffer& data, Fields::TransmogOutfitSlotData const& value)
        {
            data << value.slot << value.slotOption << value.sheatheCategory << value.itemModifiedAppearanceId << value.appearanceDisplayType;
            data << value.spellItemEnchantmentId << value.illusionDisplayType << value.flags;
        }

        void writeTransmogOutfitDataCreate(ByteBuffer& data, Fields::TransmogOutfitData const& value)
        {
            data << value.id;
            writeTransmogOutfitDataInfoCreate(data, value.outfitInfo);
            data << uint32_t(value.situations.size()) << uint32_t(value.slots.size()) << value.flags;
            for (Fields::TransmogOutfitSituationInfo const& situation : value.situations)
                writeTransmogOutfitSituationInfoCreate(data, situation);
            for (Fields::TransmogOutfitSlotData const& slot : value.slots)
                writeTransmogOutfitSlotDataCreate(data, slot);
        }

        void writeTransmogOutfitMetadataCreate(ByteBuffer& data, Fields::TransmogOutfitMetadata const& value)
        {
            data << value.situationTrigger << value.transmogOutfitId << value.stampedOptionMainHand << value.stampedOptionOffHand << value.costMod;
            data.writeBit(value.locked);
            data.flushBits();
        }

        bool hasRequiredPlayerOpaqueRecords(Fields::PlayerData const&)
        {
            return true;
        }

        bool hasRequiredActivePlayerOpaqueRecords(Fields::ActivePlayerData const&)
        {
            return true;
        }
    }

    void writeObjectDataCreate(ByteBuffer& data, Fields::ObjectData const& fields)
    {
        data << fields.entryId << fields.dynamicFlags << fields.scale;
    }

    void writeGameObjectDataCreate(ByteBuffer& data, Fields::GameObjectData const& fields)
    {
        // Capture-verified 1.60.1.69913 CREATE_OBJECT order. A full-mask
        // GameObject VALUES sample serializes the same 104-byte field body.
        data << fields.displayId
             << fields.spellVisualId
             << fields.stateSpellVisualId
             << fields.spawnTrackingStateAnimId
             << fields.spawnTrackingStateAnimKitId;

        data << uint32_t(fields.stateWorldEffectIds.size())
             << fields.stateWorldEffectsQuestObjectiveId;
        for (uint32_t value : fields.stateWorldEffectIds)
            data << value;

        writeModernGuid(data, fields.createdBy);
        writeModernGuid(data, fields.guildGuid);

        data << fields.flags << fields.flagsB;
        for (float value : fields.parentRotation)
            data << value;

        data << fields.factionTemplate
             << fields.level
             << fields.state
             << fields.typeId
             << fields.percentHealth
             << fields.artKit;

        data << uint32_t(fields.enableDoodadSets.size())
             << fields.customParam;
        for (int32_t value : fields.enableDoodadSets)
            data << value;

        data << uint32_t(fields.worldEffects.size());
        for (int32_t value : fields.worldEffects)
            data << value;

        data << fields.animGroupInstance
             << fields.uiWidgetItemId
             << fields.uiWidgetItemQuality
             << fields.uiWidgetItemCount
             << fields.unknownU32_26_69913
             << fields.unknownU32_27_69913;
    }

    void writeUnitDataCreate(ByteBuffer& data, Fields::UnitData const& fields, bool ownerVisible)
    {
        data << fields.displayId << fields.npcFlags << fields.npcFlags2 << fields.stateSpellVisualId << fields.stateAnimId << fields.stateAnimKitId;
        data << uint32_t(fields.stateWorldEffectIds.size()) << fields.stateWorldEffectsQuestObjectiveId << fields.spellOverrideNameId;

        for (uint32_t value : fields.stateWorldEffectIds)
            data << value;

        writeModernGuid(data, fields.charm);
        writeModernGuid(data, fields.summon);
        if (ownerVisible)
            writeModernGuid(data, fields.critter);
        writeModernGuid(data, fields.charmedBy);
        writeModernGuid(data, fields.summonedBy);
        writeModernGuid(data, fields.createdBy);
        writeModernGuid(data, fields.demonCreator);
        writeModernGuid(data, fields.lookAtControllerTarget);
        writeModernGuid(data, fields.target);
        writeModernGuid(data, fields.battlePetCompanionGuid);

        data << fields.battlePetDbId;
        writeModernGuid(data, fields.battlePetAttachedToDecorGuid);
        writeModernGuid(data, fields.battlePetDecorHouseGuid);
        writeUnitChannelCreate(data, fields.channelData);
        data << fields.spellEmpowerStage << fields.summonedByHomeRealm << fields.race << fields.classId << fields.playerClassId << fields.sex << fields.creatureType << fields.displayPower << fields.overrideDisplayPowerId << fields.health;

        for (std::size_t i = 0; i < fields.power.size(); ++i)
            data << fields.power[i] << fields.maxPower[i];

        // 1.60.1.69913 creature creates contain both regen arrays as part of the
        // fixed UnitData create payload as well; they are not owner-only on the wire.
        for (std::size_t i = 0; i < fields.powerRegenFlatModifier.size(); ++i)
            data << fields.powerRegenFlatModifier[i] << fields.powerRegenInterruptedFlatModifier[i];

        data << fields.maxHealth << fields.level << fields.effectiveLevel << fields.contentTuningId << fields.scalingLevelMin << fields.scalingLevelMax << fields.scalingLevelDelta << fields.scalingFactionGroup << fields.factionTemplate;

        // 1.60.1.69913: VirtualItems are part of the fixed UnitData prefix and
        // are written immediately after FactionTemplate, before UnitFlags.
        for (Fields::VisibleItem const& value : fields.virtualItems)
            writeVisibleItemCreate(data, value);
        // 1.60.1.69913 verified from retail creature creates:
        // UnitFlags, UnitFlags2, UnitFlags3, Flags4, AuraState.
        data << fields.unitFlags69913 << fields.unitFlags2_69913 << fields.unitFlags3_69913 << fields.flags4_69913 << fields.auraState69913;

        for (uint32_t value : fields.attackRoundBaseTime)
            data << value;

        if (ownerVisible)
            data << fields.rangedAttackRoundBaseTime;

        data << fields.boundingRadius << fields.combatReach << fields.displayScale << fields.creatureFamily << fields.overrideCreatureType << fields.nativeDisplayId << fields.nativeXDisplayScale << fields.mountDisplayId << fields.cosmeticMountDisplayId;

        if (ownerVisible)
            data << fields.minDamage69913 << fields.maxDamage69913 << fields.minOffHandDamage69913 << fields.maxOffHandDamage69913;

        data << fields.standState << fields.petTalentPoints << fields.visFlags << fields.animTier << fields.petNumber << fields.petNameTimestamp << fields.petExperience << fields.unknownAfterPetExperience69913 << fields.petNextLevelExperience;
        data << fields.modCastingSpeed << fields.modCastingSpeedNeg << fields.modSpellHaste << fields.modHaste << fields.modRangedHaste << fields.modHasteRegen << fields.modTimeRate69913;
        data << fields.createdBySpell69913 << fields.emoteState69913;

        if (ownerVisible)
        {
            data << fields.unknownBeforeStats69913;

            for (std::size_t i = 0; i < fields.stats69913.size(); ++i)
                data << fields.stats69913[i] << fields.statPosBuff69913[i] << fields.statNegBuff69913[i] << fields.statSupportBuff69913[i];

            for (int32_t value : fields.resistances69913)
                data << value;

            for (std::size_t i = 0; i < fields.bonusResistanceMods69913.size(); ++i)
                data << fields.bonusResistanceMods69913[i] << fields.manaCostModifier69913[i];
        }

        data << fields.baseMana;
        if (ownerVisible)
            data << fields.baseHealth;

        data << fields.sheatheState << fields.pvpFlags << fields.petFlags << fields.shapeshiftForm;

        if (ownerVisible)
        {
            data << fields.attackPower69913 << fields.attackPowerModPos69913 << fields.attackPowerModNeg69913 << fields.attackPowerMultiplier69913 << fields.attackPowerModSupport69913;
            data << fields.unknownBeforeRangedAttackPower69913A << fields.unknownBeforeRangedAttackPower69913B;
            data << fields.rangedAttackPower69913 << fields.rangedAttackPowerModPos69913 << fields.rangedAttackPowerModNeg69913 << fields.rangedAttackPowerMultiplier69913 << fields.rangedAttackPowerModSupport69913;
            data << fields.mainHandWeaponAttackPower69913 << fields.offHandWeaponAttackPower69913 << fields.rangedWeaponAttackPower69913 << fields.setAttackSpeedAura69913 << fields.lifesteal69913 << fields.minRangedDamage69913 << fields.maxRangedDamage69913 << fields.manaCostMultiplier69913;
        }

        data << fields.maxHealthModifier69913 << fields.hoverHeight69913 << fields.minItemLevelCutoff69913 << fields.minItemLevel69913 << fields.maxItemLevel69913 << fields.azeriteItemLevel69913 << fields.wildBattlePetLevel69913 << fields.battlePetCompanionExperience69913 << fields.battlePetCompanionNameTimestamp69913;
        data << fields.interactSpellId69913 << fields.scaleDuration69913 << fields.looksLikeMountId69913 << fields.looksLikeCreatureId69913 << fields.lookAtControllerId69913 << fields.perksVendorItemId69913 << fields.taxiNodesId69913;
        writeModernGuid(data, fields.unknownGuid0_69913);
        data << uint32_t(fields.passiveSpells.size()) << uint32_t(fields.worldEffects.size()) << uint32_t(fields.channelObjects.size());
        data << fields.flightCapabilityId69913 << fields.glideEventSpeedDivisor69913 << fields.driveCapabilityId69913 << fields.maxHealthModifierFlatNeg69913 << fields.maxHealthModifierFlatPos69913 << fields.silencedSchoolMask69913;
        if (ownerVisible)
            data << fields.unknownBeforeCurrentAreaId69913;
        data << fields.currentAreaId << fields.nameplateDistanceMod << fields.autoAttackRangeMod;
        if (ownerVisible)
        {
            data.append(fields.ownerExtension69913.prefix.data(), fields.ownerExtension69913.prefix.size());
            writeModernGuid(data, fields.ownerExtension69913.guidA);
            writeModernGuid(data, fields.ownerExtension69913.guidB);
            data.append(fields.ownerExtension69913.suffix.data(), fields.ownerExtension69913.suffix.size());
        }
        writeModernGuid(data, fields.nameplateAttachToGuid);

        for (Fields::PassiveSpellHistory const& value : fields.passiveSpells)
            writePassiveSpellHistoryCreate(data, value);
        for (int32_t value : fields.worldEffects)
            data << value;
        for (WoWGuid const& value : fields.channelObjects)
            writeModernGuid(data, value);

        data.writeBit(fields.field314);
        data.writeBit(fields.unknownOptionalRecord0_69913.has_value());
        data.flushBits();
        if (fields.unknownOptionalRecord0_69913)
            writeUnitAssistActionDataCreate(data, *fields.unknownOptionalRecord0_69913);
    }

    bool writePlayerDataCreateImpl(ByteBuffer& data, Fields::PlayerData const& fields, bool partyMemberVisible)
    {
        if (!hasRequiredPlayerOpaqueRecords(fields))
            return false;
        writeModernGuid(data, fields.unknownGuid0_69913);
        writeModernGuid(data, fields.unknownGuid1_69913);
        writeModernGuid(data, fields.unknownGuid2_69913);
        data << fields.unknownU64_0_69913;
        writeModernGuid(data, fields.unknownGuid3_69913);
        data << fields.unknownU32_0_69913 << fields.unknownU32_1_69913 << fields.unknownU32_2_69913 << fields.unknownU32_3_69913 << fields.unknownI32_0_69913;
        data.append(fields.unknownBeforeCustomizationCounts69913.data(), fields.unknownBeforeCustomizationCounts69913.size());
        data << uint32_t(fields.customizations.size()) << uint32_t(fields.unknownCustomizationChoices0_69913.size());

        for (uint8_t value : fields.unknownBytes0_69913)
            data << value;

        data << fields.unknownU8_0_69913 << fields.unknownU8_1_69913 << fields.unknownU8_2_69913 << fields.unknownU8_3_69913 << fields.unknownU32_4_69913 << fields.unknownI32_1_69913;

        if (partyMemberVisible)
        {
            for (Fields::QuestLog const& value : fields.unknownPartyRecords0_69913)
                writeQuestLogCreate(data, value);
            data << uint32_t(fields.unknownPartyMap0_69913.size());
            for (auto const& [questId, index] : fields.unknownPartyMap0_69913)
                data << questId << index;
            data << uint32_t(fields.unknownPartyDynamicRecords0_69913.size());
        }

        for (Fields::VisibleItem const& value : fields.unknownVisibleItemRecords0_69913)
            writeVisibleItemCreate(data, value);

        data << fields.unknownI32_2_69913 << fields.unknownI32_3_69913 << fields.unknownU32_5_69913 << fields.unknownU32_6_69913 << fields.unknownI32_4_69913 << fields.unknownI32_5_69913;

        for (float value : fields.unknownFloatArray0_69913)
            data << value;

        data << fields.unknownU8_4_69913 << fields.unknownI32_6_69913 << fields.unknownI64_0_69913;
        data << uint32_t(fields.unknownDynamicRecords0_69913.size());

        for (Fields::ZonePlayerForcedReaction const& value : fields.unknownFixedRecords0_69913)
            writeZonePlayerForcedReactionCreate(data, value);

        data << fields.unknownI32_7_69913 << fields.unknownI32_8_69913 << fields.unknownI32_9_69913;
        data << uint32_t(fields.unknownDynamicRecords1_69913.size());
        writeCtrOptionsCreate(data, fields.unknownCtrOptions0_69913);
        data << fields.unknownI32_10_69913 << fields.unknownI32_11_69913;
        writeDungeonScoreSummaryCreate(data, fields.unknownDungeonScore0_69913);
        writeModernGuid(data, fields.unknownLeaverInfo0_69913.bnetAccountGuid);
        data << fields.unknownLeaverInfo0_69913.leaveScore << fields.unknownLeaverInfo0_69913.seasonId << fields.unknownLeaverInfo0_69913.totalLeaves
             << fields.unknownLeaverInfo0_69913.totalSuccesses << fields.unknownLeaverInfo0_69913.consecutiveSuccesses
             << fields.unknownLeaverInfo0_69913.lastPenaltyTime << fields.unknownLeaverInfo0_69913.leaverExpirationTime << fields.unknownLeaverInfo0_69913.flags;
        data.writeBit(fields.unknownLeaverInfo0_69913.isLeaver);
        data.flushBits();
        writeModernGuid(data, fields.unknownGuid4_69913);
        data << fields.unknownI32_12_69913;

        for (Fields::ItemInstance const& value : fields.unknownItemInstances0_69913)
            writeItemInstanceCreate(data, value);
        data << uint32_t(fields.unknownI32Vector0_69913.size());

        for (uint32_t value : fields.attackRoundBaseTime)
            data << value;

        writeCustomTabardInfoCreate(data, fields.unknownCustomTabard0_69913);
        writeNpcAsPlayerInfoCreate(data, fields.unknownNpcAsPlayer0_69913);
        data.append(fields.unknownBeforeCustomizationPayload69913.data(), fields.unknownBeforeCustomizationPayload69913.size());

        for (Fields::ChrCustomizationChoice const& value : fields.customizations)
            writeCustomizationCreate(data, value);
        for (Fields::ChrCustomizationChoice const& value : fields.unknownCustomizationChoices0_69913)
            writeCustomizationCreate(data, value);

        if (partyMemberVisible)
            for (Fields::QuestLog const& value : fields.unknownPartyDynamicRecords0_69913)
                writeQuestLogCreate(data, value);

        for (Fields::DynamicRecord const& value : fields.unknownDynamicRecords0_69913)
            writeDynamicRecord(data, value);
        for (Fields::DynamicRecord const& value : fields.unknownDynamicRecords1_69913)
            writeDynamicRecord(data, value);
        for (int32_t value : fields.unknownI32Vector0_69913)
            data << value;

        // Forever 1.60.1 build 69913 carries first and last name separately.
        //
        // The verified 69913 wire layout uses byte-aligned length encoding:
        //   Test / Hims   -> 10 08 00 + "TestHims"
        //   Schurki / Asc -> 1C 06 00 + "SchurkiAsc"
        //
        // byte0 = firstNameLength << 2  (6-bit length, byte-aligned)
        // byte1 = lastNameLength  << 1  (observed zero guard bits around a 6-bit length)
        // byte2 = currently-zero optional-name flags in the verified samples.
        //
        // Keep the third byte conservative until a sample with one of the optional
        // states set proves its individual bit assignments.
        const std::size_t firstNameLength = std::min<std::size_t>(fields.firstName.size(), 63U);
        const std::size_t lastNameLength = std::min<std::size_t>(fields.lastName.size(), 63U);

        data << static_cast<uint8_t>(firstNameLength << 2U);
        data << static_cast<uint8_t>(lastNameLength << 1U);

        uint8_t foreverNameFlags = 0;
        if (partyMemberVisible && fields.unknownNameFlag0_69913)
            foreverNameFlags |= 0x80U;
        if (fields.unknownNameFlag1_69913)
            foreverNameFlags |= 0x40U;
        if (fields.unknownOptionalNamePayload0_69913.has_value())
            foreverNameFlags |= 0x20U;
        data << foreverNameFlags;

        if (firstNameLength != 0U)
            data.append(reinterpret_cast<uint8_t const*>(fields.firstName.data()), firstNameLength);
        if (lastNameLength != 0U)
            data.append(reinterpret_cast<uint8_t const*>(fields.lastName.data()), lastNameLength);

        if (fields.unknownOptionalNamePayload0_69913)
            writeDynamicRecord(data, *fields.unknownOptionalNamePayload0_69913);
        return true;
    }

    bool writePlayerDataCreate(ByteBuffer& data, Fields::PlayerData const& fields, bool partyMemberVisible)
    {
        return writePlayerDataCreateImpl(data, fields, partyMemberVisible);
    }


    namespace
    {
        inline constexpr std::array<uint8_t, 180> PostSkillDefaults69913 = {
            0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x01,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0xC9,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
            0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x80,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x80,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x80,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x80,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x80,0x3F,0x00,0x00,
        };

        inline constexpr std::array<uint8_t, 1089> PreOutfitDefaults69913 = {
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,0x80,0x3F,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x80,0x3F,0x00,0x00,0x80,0x3F,0x00,0x00,0x80,0x3F,0x00,0x00,0x80,0x3F,0x00,0x00,
            0x80,0x3F,0x00,0x00,0x80,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x04,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x99,0x99,
            0xF9,0x3F,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x15,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x3F,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x8B,0xFC,0x3A,0x00,
            0x8B,0xFC,0x3A,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0xD3,0x05,0x00,0x00,0x01,0x00,0x00,0x00,0x2D,0xFA,0xFF,0xFF,0x04,0x80,0x55,0x6E,
            0x6B,0x6E,0x6F,0x77,0x6E,0x30,0x31,0x8B,0xFC,0x3A,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x03,0x00,
            0x00,0x00,0x03,0x00,0x00,0x00,0x01,0x00,0x0D,0x02,0x00,0x08,0x80,0x4F,0x75,0x74,
            0x66,0x69,0x74,0x20,0x32,0x07,0x00,0x00,0x00,0x2D,0x00,0x00,0x00,0x01,0x00,0x00,
            0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x25,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x02,0x00,0x00,
            0x00,
        };


        void applyDefaultTransmogOutfit(Fields::TransmogOutfitData& outfit, uint32_t id, uint8_t setType, char const* name, bool withSituations, uint32_t flags)
        {
            static constexpr std::array<uint32_t, 7> SituationIds = {1u, 3u, 13u, 18u, 20u, 37u, 32u};
            static constexpr std::array<Fields::TransmogOutfitSlotData, 45> Slots = {{
            {0, 12u, 0u, 0u, 2u, 0u, 2u, 0u},
            {0, 13u, 0u, 0u, 2u, 0u, 2u, 0u},
            {0, 14u, 0u, 0u, 2u, 0u, 2u, 0u},
            {0, 15u, 0u, 0u, 2u, 0u, 2u, 0u},
            {1, 12u, 0u, 0u, 2u, 0u, 2u, 0u},
            {1, 13u, 0u, 0u, 2u, 0u, 2u, 0u},
            {1, 14u, 0u, 0u, 2u, 0u, 2u, 0u},
            {1, 15u, 0u, 0u, 2u, 0u, 2u, 0u},
            {2, 12u, 0u, 0u, 2u, 0u, 2u, 0u},
            {2, 13u, 0u, 0u, 2u, 0u, 2u, 0u},
            {2, 14u, 0u, 0u, 2u, 0u, 2u, 0u},
            {2, 15u, 0u, 0u, 2u, 0u, 2u, 0u},
            {6, 0u, 0u, 0u, 2u, 0u, 2u, 0u},
            {4, 12u, 0u, 0u, 2u, 0u, 2u, 0u},
            {4, 13u, 0u, 0u, 2u, 0u, 2u, 0u},
            {4, 14u, 0u, 0u, 2u, 0u, 2u, 0u},
            {4, 15u, 0u, 0u, 2u, 0u, 2u, 0u},
            {9, 12u, 0u, 0u, 2u, 0u, 2u, 0u},
            {9, 13u, 0u, 0u, 2u, 0u, 2u, 0u},
            {9, 14u, 0u, 0u, 2u, 0u, 2u, 0u},
            {9, 15u, 0u, 0u, 2u, 0u, 2u, 0u},
            {10, 12u, 0u, 0u, 2u, 0u, 2u, 0u},
            {10, 13u, 0u, 0u, 2u, 0u, 2u, 0u},
            {10, 14u, 0u, 0u, 2u, 0u, 2u, 0u},
            {10, 15u, 0u, 0u, 2u, 0u, 2u, 0u},
            {11, 12u, 0u, 0u, 2u, 0u, 2u, 0u},
            {11, 13u, 0u, 0u, 2u, 0u, 2u, 0u},
            {11, 14u, 0u, 0u, 2u, 0u, 2u, 0u},
            {11, 15u, 0u, 0u, 2u, 0u, 2u, 0u},
            {7, 12u, 0u, 0u, 2u, 0u, 2u, 0u},
            {7, 13u, 0u, 0u, 2u, 0u, 2u, 0u},
            {7, 14u, 0u, 0u, 2u, 0u, 2u, 0u},
            {7, 15u, 0u, 0u, 2u, 0u, 2u, 0u},
            {8, 12u, 0u, 0u, 2u, 0u, 2u, 0u},
            {8, 13u, 0u, 0u, 2u, 0u, 2u, 0u},
            {8, 14u, 0u, 0u, 2u, 0u, 2u, 0u},
            {8, 15u, 0u, 0u, 2u, 0u, 2u, 0u},
            {3, 0u, 0u, 0u, 2u, 0u, 2u, 0u},
            {5, 0u, 0u, 0u, 2u, 0u, 2u, 0u},
            {12, 1u, 0u, 0u, 2u, 0u, 2u, 0u},
            {12, 2u, 0u, 0u, 2u, 0u, 2u, 0u},
            {13, 1u, 0u, 0u, 2u, 0u, 2u, 0u},
            {13, 5u, 0u, 0u, 2u, 0u, 2u, 0u},
            {13, 4u, 0u, 0u, 2u, 0u, 2u, 0u},
            {14, 3u, 0u, 0u, 2u, 0u, 2u, 0u}
            }};

            outfit = {};
            outfit.id = id;
            outfit.outfitInfo.setType = setType;
            outfit.outfitInfo.icon = 134400u;
            outfit.outfitInfo.name = name;
            outfit.outfitInfo.situationsEnabled = true;
            outfit.flags = flags;

            if (withSituations)
            {
                outfit.situations.reserve(SituationIds.size());
                for (uint32_t situationId : SituationIds)
                    outfit.situations.push_back({situationId, 0u, 0u, 0u});
            }

            outfit.slots.assign(Slots.begin(), Slots.end());
        }

        Fields::ActivePlayerData makeConservativeActivePlayerData(Fields::ActivePlayerData const& source)
        {
            // Forever 69913 live profile:
            // Keep only fields whose placement/encoding has been established from
            // verified 69913 wire data. Everything else intentionally stays at the protocol's
            // minimal zero/empty state until their semantics are verified.
            //
            // Unproven modern regions are represented only by zeroed
            // opaque Forever wire spans.
            Fields::ActivePlayerData result{};

            // Proven prefix. Packed GUID length is part of the wire layout, so keep
            // the real inventory/observer GUID state instead of fixed packet bytes.
            result.invSlots = source.invSlots;
            result.farsightObject = source.farsightObject;
            result.summonedBattlePetGuid = source.summonedBattlePetGuid;
            result.knownTitles = source.knownTitles;
            result.coinage = source.coinage;
            result.accountBankCoinage = source.accountBankCoinage;
            result.xp = source.xp;
            result.nextLevelXp = source.nextLevelXp;

            // SkillInfo layout is now wire/runtime verified as the 4200-byte
            // block immediately following the core scalars.
            result.skill = source.skill;

            // The complete 104-byte post-SkillInfo scalar/combat-stat cluster
            // is now structurally identified and may be emitted from live state.
            result.characterPoints = source.characterPoints;
            result.maxTalentTiers = source.maxTalentTiers;
            result.trackCreatureMask = source.trackCreatureMask;
            result.mainhandExpertise = source.mainhandExpertise;
            result.offhandExpertise = source.offhandExpertise;
            result.rangedExpertise = source.rangedExpertise;
            result.combatRatingExpertise = source.combatRatingExpertise;
            result.blockPercentage = source.blockPercentage;
            result.dodgePercentage = source.dodgePercentage;
            result.dodgePercentageFromAttribute = source.dodgePercentageFromAttribute;
            result.parryPercentage = source.parryPercentage;
            result.parryPercentageFromAttribute = source.parryPercentageFromAttribute;
            result.critPercentage = source.critPercentage;
            result.rangedCritPercentage = source.rangedCritPercentage;
            result.offhandCritPercentage = source.offhandCritPercentage;
            result.spellCritPercentage = source.spellCritPercentage;
            result.shieldBlock = source.shieldBlock;
            result.shieldBlockCritPercentage = source.shieldBlockCritPercentage;
            result.mastery = source.mastery;
            result.speed = source.speed;
            result.avoidance = source.avoidance;
            result.sturdiness = source.sturdiness;
            result.versatility = source.versatility;
            result.versatilityBonus = source.versatilityBonus;
            result.pvpPowerDamage = source.pvpPowerDamage;
            result.pvpPowerHealing = source.pvpPowerHealing;

            // 69913 build-specific create defaults for the still-unresolved
            // post-SkillInfo spans. These bytes now flow through our field
            // model and writer instead of being spliced directly from the
            // legacy packet template. Replace individual values with live semantics
            // as their live semantics are identified.
            std::copy_n(PostSkillDefaults69913.begin(), Fields::ActivePlayerData::PostSkillHeaderSize69913, result.unknownPostSkillHeader69913.begin());
            std::size_t postSkillSeedOffset = Fields::ActivePlayerData::PostSkillHeaderSize69913;
            for (Fields::ActivePlayerData::PostSkillRecord69913& record : result.unknownPostSkillRecords69913)
            {
                std::memcpy(&record.unknown0, PostSkillDefaults69913.data() + postSkillSeedOffset + 0, sizeof(record.unknown0));
                std::memcpy(&record.unknown4, PostSkillDefaults69913.data() + postSkillSeedOffset + 4, sizeof(record.unknown4));
                std::memcpy(&record.multiplier0, PostSkillDefaults69913.data() + postSkillSeedOffset + 8, sizeof(record.multiplier0));
                std::memcpy(&record.multiplier1, PostSkillDefaults69913.data() + postSkillSeedOffset + 12, sizeof(record.multiplier1));
                postSkillSeedOffset += 16;
            }
            std::copy_n(PostSkillDefaults69913.begin() + postSkillSeedOffset, Fields::ActivePlayerData::PostSkillTailSize69913, result.unknownPostSkillTail69913.begin());

            std::copy_n(PreOutfitDefaults69913.begin(), Fields::ActivePlayerData::UnknownBeforeOutfitSize69913, result.unknownBeforeOutfit69913.begin());

            // The following 32-bit slot exists on the wire immediately after
            // NextLevelXP, but its Forever semantics are not proven. Keep
            // unknownAfterNextLevelXp69913 at its zero default until differential testing
            // demonstrates what it represents.

            // Build-specific default outfit state. These records are now
            // represented as typed protocol defaults rather than retained packet bytes.
            result.unknownOutfitScalar0_69913 = 2u;
            result.unknownOutfitScalar1_69913 = 3u;
            applyDefaultTransmogOutfit(result.viewedOutfit, 3u, 1u, "Outfit 2", true, 1u);
            result.additionalOutfits69913.resize(2);
            applyDefaultTransmogOutfit(result.additionalOutfits69913[0], 2u, 1u, "Outfit 1", true, 1u);
            applyDefaultTransmogOutfit(result.additionalOutfits69913[1], 1u, 0u, "Outfit", false, 0u);
            result.transmogMetadata.situationTrigger = 0u;
            result.transmogMetadata.transmogOutfitId = 2u;
            result.transmogMetadata.stampedOptionMainHand = 0u;
            result.transmogMetadata.stampedOptionOffHand = 0u;
            result.transmogMetadata.costMod = 1.0f;
            result.transmogMetadata.locked = false;

            return result;
        }
    }

    bool writeActivePlayerDataCreate(ByteBuffer& data, Fields::ActivePlayerData const& fields)
    {
        if (!hasRequiredActivePlayerOpaqueRecords(fields))
            return false;

        for (WoWGuid const& value : fields.invSlots)
            writeModernGuid(data, value);

        writeModernGuid(data, fields.farsightObject);
        writeModernGuid(data, fields.summonedBattlePetGuid);

        data << uint32_t(fields.knownTitles.size());
        data.append(fields.unknownInventoryExtension69913.data(), fields.unknownInventoryExtension69913.size());
        data << fields.coinage << fields.accountBankCoinage << fields.xp << fields.nextLevelXp << fields.unknownAfterNextLevelXp69913;

        writeSkillInfoCreate(data, fields.skill);

        data << fields.characterPoints
             << fields.maxTalentTiers
             << fields.trackCreatureMask
             << fields.mainhandExpertise
             << fields.offhandExpertise
             << fields.rangedExpertise
             << fields.combatRatingExpertise
             << fields.blockPercentage
             << fields.dodgePercentage
             << fields.dodgePercentageFromAttribute
             << fields.parryPercentage
             << fields.parryPercentageFromAttribute
             << fields.critPercentage
             << fields.rangedCritPercentage
             << fields.offhandCritPercentage
             << fields.spellCritPercentage
             << fields.shieldBlock
             << fields.shieldBlockCritPercentage
             << fields.mastery
             << fields.speed
             << fields.avoidance
             << fields.sturdiness
             << fields.versatility
             << fields.versatilityBonus
             << fields.pvpPowerDamage
             << fields.pvpPowerHealing;

        data.append(fields.unknownPostSkillHeader69913.data(), fields.unknownPostSkillHeader69913.size());
        for (Fields::ActivePlayerData::PostSkillRecord69913 const& record : fields.unknownPostSkillRecords69913)
            data << record.unknown0 << record.unknown4 << record.multiplier0 << record.multiplier1;
        data.append(fields.unknownPostSkillTail69913.data(), fields.unknownPostSkillTail69913.size());

        data.append(fields.unknownBeforeOutfit69913.data(), fields.unknownBeforeOutfit69913.size());
        data << fields.unknownOutfitScalar0_69913 << fields.unknownOutfitScalar1_69913;
        writeTransmogOutfitDataCreate(data, fields.viewedOutfit);
        data << uint32_t(fields.additionalOutfits69913.size());
        for (Fields::TransmogOutfitData const& outfit : fields.additionalOutfits69913)
            writeTransmogOutfitDataCreate(data, outfit);
        writeTransmogOutfitMetadataCreate(data, fields.transmogMetadata);
        data.flushBits();

        for (uint64_t value : fields.knownTitles)
            data << value;

        data.append(fields.unknownAfterTransmog69913.data(), fields.unknownAfterTransmog69913.size());
        return true;
    }



    namespace
    {
        void writeStationaryUnitMovement(ByteBuffer& data, std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, uint32_t movementTimeMs)
        {
            // Capture-proven minimal stationary creature layout:
            //   7-byte fixed prefix
            //   ModernGUID
            //   8-byte zero block
            //   movement timestamp (uint32 ms)
            //   x/y/z/orientation
            //   byte-stable stationary movement suffix
            static constexpr std::array<uint8_t, 7> prefix = { 0x84, 0, 0, 0, 0, 0, 0 };
            static constexpr std::array<uint8_t, 8> zeros = { 0, 0, 0, 0, 0, 0, 0, 0 };
            data.append(prefix.data(), prefix.size());
            data.append(packedGuid.data(), packedGuid.size());
            data.append(zeros.data(), zeros.size());
            data << movementTimeMs;
            data << x << y << z << orientation;
            data.append(STATIONARY_UNIT_MOVEMENT_SUFFIX_69913.data(), STATIONARY_UNIT_MOVEMENT_SUFFIX_69913.size());
        }
    }

    std::vector<uint8_t> buildCreatureCreateBlock(std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, uint32_t movementTimeMs, Fields::ObjectData const& objectFields, Fields::UnitData const& unitFields, uint32_t vendorDataFlags69913)
    {
        if (packedGuid.empty())
            return {};

        ByteBuffer fieldPayload;

        // Ordinary stationary creature:
        //   04 03 CC FF 01
        //
        // Capture/runtime-verified vendor creature fragment list in 69913:
        //   04 03 12 CC FF 01
        fieldPayload << uint8_t(0x04)
                     << uint8_t(FRAGMENT_CGOBJECT_69913);

        if (vendorDataFlags69913 != 0)
            fieldPayload << uint8_t(FRAGMENT_VENDOR_69913);

        fieldPayload << uint8_t(FRAGMENT_TAG_UNIT_69913)
                     << uint8_t(FRAGMENT_END_69913)
                     << uint8_t(1);

        writeObjectDataCreate(fieldPayload, objectFields);
        writeUnitDataCreate(fieldPayload, unitFields, false);

        if (vendorDataFlags69913 != 0)
            fieldPayload << int32_t(vendorDataFlags69913);

        ByteBuffer block;
        block << uint8_t(1); // CREATE_OBJECT (ordinary world unit)
        block.append(packedGuid.data(), packedGuid.size());
        block << uint8_t(OBJECT_TYPE_UNIT);
        writeStationaryUnitMovement(block, packedGuid, x, y, z, orientation, movementTimeMs);
        block << uint32_t(fieldPayload.size());
        block.append(fieldPayload);

        return std::vector<uint8_t>(block.contents(), block.contents() + block.size());
    }

    std::vector<uint8_t> buildGameObjectCreateBlock(std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, int64_t packedLocalRotation, Fields::ObjectData const& objectFields, Fields::GameObjectData const& gameObjectFields)
    {
        if (packedGuid.empty())
            return {};

        ByteBuffer fieldPayload;
        // Capture-verified ordinary GameObject fragment list:
        //   00 03 CE FF 01
        fieldPayload << uint8_t(0)
                     << uint8_t(FRAGMENT_CGOBJECT_69913)
                     << uint8_t(FRAGMENT_TAG_GAMEOBJECT_69913)
                     << uint8_t(FRAGMENT_END_69913)
                     << uint8_t(1);
        writeObjectDataCreate(fieldPayload, objectFields);
        writeGameObjectDataCreate(fieldPayload, gameObjectFields);

        ByteBuffer block;
        block << uint8_t(1); // CREATE_OBJECT
        block.append(packedGuid.data(), packedGuid.size());
        block << uint8_t(OBJECT_TYPE_GAMEOBJECT);

        // 69913 stationary GameObject movement is 31 bytes:
        // flags(3), transport/time placeholder(4), position+orientation(16),
        // packed local rotation(8). This shape is byte-stable across the
        // stationary GameObjects present in the reference captures.
        static constexpr std::array<uint8_t, 3> movementFlags = { 0x81, 0x08, 0x00 };
        block.append(movementFlags.data(), movementFlags.size());
        block << uint32_t(0);
        block << x << y << z << orientation;
        block << packedLocalRotation;

        block << uint32_t(fieldPayload.size());
        block.append(fieldPayload);
        return std::vector<uint8_t>(block.contents(), block.contents() + block.size());
    }

    std::vector<uint8_t> buildSelfFieldPayload(Fields::ObjectData const& objectFields, Fields::UnitData const& unitFields, Fields::PlayerData const& playerFields, Fields::ActivePlayerData const& activePlayerFields)
    {
        ByteBuffer payload;

        payload << SELF_FIELD_FLAGS_69913;
        payload << FRAGMENT_CGOBJECT_69913;
        payload << FRAGMENT_PLAYER_HOUSE_INFO_69913;
        payload << FRAGMENT_PLAYER_INITIATIVE_69913;
        payload << FRAGMENT_TAG_UNIT_69913;
        payload << FRAGMENT_TAG_PLAYER_69913;
        payload << FRAGMENT_END_69913;

        payload << uint8_t(1); // CGObject indirect fragment activation

        writeObjectDataCreate(payload, objectFields);
        writeUnitDataCreate(payload, unitFields, true);
        if (!writePlayerDataCreate(payload, playerFields, true))
            return {};

        const Fields::ActivePlayerData activePlayer = makeConservativeActivePlayerData(activePlayerFields);

        ByteBuffer activePlayerPayload;
        if (!writeActivePlayerDataCreate(activePlayerPayload, activePlayer))
            return {};

        payload.append(activePlayerPayload.contents(), activePlayerPayload.size());

        // Remaining 69913 ActivePlayer fields that are structurally required
        // but not semantically identified yet. Keep them isolated from the
        // typed portion so they can be replaced field-by-field later.
        static constexpr std::size_t TypedReferenceEnd69913 = 8422;
        static constexpr std::size_t ZeroDefaultsEnd69913 = 23573;
        static constexpr std::size_t WriterOwnedTailBytes69913 =
            Fields::ActivePlayerData::UnknownAfterTransmogSize69913;
        static constexpr std::size_t ZeroDefaultsSize69913 =
            (ZeroDefaultsEnd69913 - TypedReferenceEnd69913) - WriterOwnedTailBytes69913;
        static const std::array<uint8_t, ZeroDefaultsSize69913> ZeroDefaults69913{};
        payload.append(ZeroDefaults69913.data(), ZeroDefaults69913.size());

        static constexpr std::size_t SparseDefaultsSize69913 = 38749 - 23573;
        static const std::array<uint8_t, SparseDefaultsSize69913> SparseDefaults69913 = []
        {
            std::array<uint8_t, SparseDefaultsSize69913> data{};
            data[0] = 0x80;
            data[1] = 0x03;
            data[733] = 0x0C;
            data[860] = 0x80;
            data[2124] = 0x08;
            data[3408] = 0x1C;
            data[9837] = 0x80;
            data[9838] = 0x38;
            data[9929] = 0x40;

            constexpr std::array<uint8_t, 15> finalDefaults = {
                0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00,
                0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x10
            };
            std::copy(finalDefaults.begin(), finalDefaults.end(), data.begin() + 15159);
            return data;
        }();
        payload.append(SparseDefaults69913.data(), SparseDefaults69913.size());

        payload << uint8_t(1);
        writeEmptyPlayerHouseInfoComponentCreate(payload);

        payload << uint8_t(1);
        writeEmptyPlayerInitiativeComponentCreate(payload);

        return std::vector<uint8_t>(payload.contents(), payload.contents() + payload.size());
    }


    namespace
    {
    }



    namespace
    {
        template <std::size_t N>
        uint32_t getChangeBlock(std::bitset<N> const& changes, std::size_t blockIndex)
        {
            uint32_t value = 0;
            const std::size_t firstBit = blockIndex * 32U;
            const std::size_t lastBit = std::min<std::size_t>(firstBit + 32U, N);
            for (std::size_t bit = firstBit; bit < lastBit; ++bit)
                if (changes.test(bit))
                    value |= uint32_t(1) << (bit - firstBit);
            return value;
        }

        template <std::size_t N>
        void writeStructuredChangeMask(ByteBuffer& data, std::bitset<N> const& changes)
        {
            constexpr std::size_t BlockCount = (N + 31U) / 32U;
            if constexpr (BlockCount == 1)
            {
                data.writeBits(getChangeBlock(changes, 0), N);
            }
            else
            {
                uint32_t blocksMask = 0;
                for (std::size_t block = 0; block < BlockCount; ++block)
                    if (getChangeBlock(changes, block) != 0)
                        blocksMask |= uint32_t(1) << block;

                data.writeBits(blocksMask, BlockCount);
                for (std::size_t block = 0; block < BlockCount; ++block)
                    if (blocksMask & (uint32_t(1) << block))
                        data.writeBits(getChangeBlock(changes, block), 32);
            }
        }

        void writeObjectDataUpdate(ByteBuffer& data, Fields::ObjectData const& fields)
        {
            writeStructuredChangeMask(data, fields.changes);
            if (fields.changes.test(Fields::ObjectData::EntryIdBit))
                data << fields.entryId;
            if (fields.changes.test(Fields::ObjectData::DynamicFlagsBit))
                data << fields.dynamicFlags;
            if (fields.changes.test(Fields::ObjectData::ScaleBit))
                data << fields.scale;
            data.flushBits();
        }

        void writeItemDataUpdate(ByteBuffer& data, Fields::ItemData const& fields)
        {
            writeStructuredChangeMask(data, fields.changes);
            auto changed = [&](std::size_t bit) { return fields.changes.test(bit); };
            if (changed(Fields::ItemData::OwnerBit)) writeModernGuid(data, fields.owner);
            if (changed(Fields::ItemData::ContainedInBit)) writeModernGuid(data, fields.containedIn);
            if (changed(Fields::ItemData::CreatorBit)) writeModernGuid(data, fields.creator);
            if (changed(Fields::ItemData::GiftCreatorBit)) writeModernGuid(data, fields.giftCreator);
            if (changed(Fields::ItemData::StackCountBit)) data << fields.stackCount;
            if (changed(Fields::ItemData::ExpirationBit)) data << fields.expiration;
            if (changed(Fields::ItemData::DynamicFlagsBit)) data << fields.dynamicFlags;
            if (changed(Fields::ItemData::DurabilityBit)) data << fields.durability;
            if (changed(Fields::ItemData::MaxDurabilityBit)) data << fields.maxDurability;
            if (changed(Fields::ItemData::CreatePlayedTimeBit)) data << fields.createPlayedTime;
            if (changed(Fields::ItemData::SpellChargesGroupBit))
                for (std::size_t i = 0; i < fields.spellCharges.size(); ++i)
                    if (changed(Fields::ItemData::SpellChargesFirstBit + i))
                        data << fields.spellCharges[i];
            data.flushBits();
        }

        void writeContainerDataUpdate(ByteBuffer& data, Fields::ContainerData const& fields)
        {
            writeStructuredChangeMask(data, fields.changes);
            auto changed = [&](std::size_t bit) { return fields.changes.test(bit); };
            if (changed(Fields::ContainerData::NumSlotsBit)) data << fields.numSlots;
            if (changed(Fields::ContainerData::SlotsGroupBit))
                for (std::size_t i = 0; i < fields.slots.size(); ++i)
                    if (changed(Fields::ContainerData::SlotsFirstBit + i))
                        writeModernGuid(data, fields.slots[i]);
            data.flushBits();
        }

        void writeUnitDataUpdate(ByteBuffer& data, Fields::UnitData const& fields)
        {
            writeStructuredChangeMask(data, fields.changes);

            auto changed = [&](std::size_t bit) { return fields.changes.test(bit); };
            if (changed(Fields::UnitData::DisplayIdBit)) data << fields.displayId;
            if (changed(Fields::UnitData::NpcFlagsBit)) data << fields.npcFlags;
            if (changed(Fields::UnitData::NpcFlags2Bit)) data << fields.npcFlags2;
            if (changed(Fields::UnitData::CharmBit)) writeModernGuid(data, fields.charm);
            if (changed(Fields::UnitData::SummonBit)) writeModernGuid(data, fields.summon);
            if (changed(Fields::UnitData::CritterBit)) writeModernGuid(data, fields.critter);
            if (changed(Fields::UnitData::CharmedByBit)) writeModernGuid(data, fields.charmedBy);
            if (changed(Fields::UnitData::SummonedByBit)) writeModernGuid(data, fields.summonedBy);
            if (changed(Fields::UnitData::CreatedByBit)) writeModernGuid(data, fields.createdBy);
            if (changed(Fields::UnitData::TargetBit)) writeModernGuid(data, fields.target);
            if (changed(Fields::UnitData::RaceBit)) data << fields.race;
            if (changed(Fields::UnitData::ClassIdBit)) data << fields.classId;
            if (changed(Fields::UnitData::PlayerClassIdBit)) data << fields.playerClassId;
            if (changed(Fields::UnitData::SexBit)) data << fields.sex;
            if (changed(Fields::UnitData::DisplayPowerBit)) data << fields.displayPower;
            if (changed(Fields::UnitData::HealthBit)) data << fields.health;
            if (changed(Fields::UnitData::MaxHealthBit)) data << fields.maxHealth;
            if (changed(Fields::UnitData::LevelBit)) data << fields.level;
            if (changed(Fields::UnitData::EffectiveLevelBit)) data << fields.effectiveLevel;
            if (changed(Fields::UnitData::FactionTemplateBit)) data << fields.factionTemplate;
            if (changed(Fields::UnitData::FlagsBit)) data << fields.unitFlags69913;
            if (changed(Fields::UnitData::Flags2Bit)) data << fields.unitFlags2_69913;
            if (changed(Fields::UnitData::AuraStateBit)) data << fields.auraState69913;
            if (changed(Fields::UnitData::RangedAttackRoundBaseTimeBit)) data << fields.rangedAttackRoundBaseTime;
            if (changed(Fields::UnitData::BoundingRadiusBit)) data << fields.boundingRadius;
            if (changed(Fields::UnitData::CombatReachBit)) data << fields.combatReach;
            if (changed(Fields::UnitData::NativeDisplayIdBit)) data << fields.nativeDisplayId;
            if (changed(Fields::UnitData::MountDisplayIdBit)) data << fields.mountDisplayId;
            if (changed(Fields::UnitData::MinDamageBit)) data << fields.minDamage69913;
            if (changed(Fields::UnitData::MaxDamageBit)) data << fields.maxDamage69913;
            if (changed(Fields::UnitData::MinOffHandDamageBit)) data << fields.minOffHandDamage69913;
            if (changed(Fields::UnitData::MaxOffHandDamageBit)) data << fields.maxOffHandDamage69913;
            if (changed(Fields::UnitData::BaseManaBit)) data << fields.baseMana;
            if (changed(Fields::UnitData::BaseHealthBit)) data << fields.baseHealth;

            if (changed(Fields::UnitData::PowerGroupBit))
            {
                for (std::size_t i = 0; i < fields.power.size(); ++i)
                    if (changed(Fields::UnitData::PowerFirstBit + i))
                        data << fields.power[i];
                for (std::size_t i = 0; i < fields.maxPower.size(); ++i)
                    if (changed(Fields::UnitData::MaxPowerFirstBit + i))
                        data << fields.maxPower[i];
            }

            if (changed(Fields::UnitData::AttackRoundBaseTimeGroupBit))
                for (std::size_t i = 0; i < fields.attackRoundBaseTime.size(); ++i)
                    if (changed(Fields::UnitData::AttackRoundBaseTimeFirstBit + i))
                        data << fields.attackRoundBaseTime[i];

            if (changed(Fields::UnitData::StatsGroupBit))
            {
                for (std::size_t i = 0; i < fields.stats69913.size(); ++i)
                    if (changed(Fields::UnitData::StatsFirstBit + i)) data << fields.stats69913[i];
                for (std::size_t i = 0; i < fields.statPosBuff69913.size(); ++i)
                    if (changed(Fields::UnitData::StatPosBuffFirstBit + i)) data << fields.statPosBuff69913[i];
                for (std::size_t i = 0; i < fields.statNegBuff69913.size(); ++i)
                    if (changed(Fields::UnitData::StatNegBuffFirstBit + i)) data << fields.statNegBuff69913[i];
                for (std::size_t i = 0; i < fields.statSupportBuff69913.size(); ++i)
                    if (changed(Fields::UnitData::StatSupportBuffFirstBit + i)) data << fields.statSupportBuff69913[i];
            }

            if (changed(Fields::UnitData::ResistancesGroupBit))
            {
                for (std::size_t i = 0; i < fields.resistances69913.size(); ++i)
                    if (changed(Fields::UnitData::ResistancesFirstBit + i)) data << fields.resistances69913[i];
                for (std::size_t i = 0; i < fields.bonusResistanceMods69913.size(); ++i)
                    if (changed(Fields::UnitData::BonusResistanceModsFirstBit + i)) data << fields.bonusResistanceMods69913[i];
                for (std::size_t i = 0; i < fields.manaCostModifier69913.size(); ++i)
                    if (changed(Fields::UnitData::ManaCostModifierFirstBit + i)) data << fields.manaCostModifier69913[i];
            }
            data.flushBits();
        }

        void writePlayerDataUpdate(ByteBuffer& data, Fields::PlayerData const& fields)
        {
            writeStructuredChangeMask(data, fields.changes);
            auto changed = [&](std::size_t bit) { return fields.changes.test(bit); };
            if (changed(Fields::PlayerData::DuelArbiterBit)) writeModernGuid(data, fields.unknownGuid0_69913);
            if (changed(Fields::PlayerData::PlayerFlagsBit)) data << fields.unknownU32_0_69913;
            if (changed(Fields::PlayerData::CurrentSpecBit)) data << fields.unknownU32_6_69913;
            if (changed(Fields::PlayerData::NameBit))
            {
                const std::size_t firstNameLength = std::min<std::size_t>(fields.firstName.size(), 63U);
                const std::size_t lastNameLength = std::min<std::size_t>(fields.lastName.size(), 63U);
                data << static_cast<uint8_t>(firstNameLength << 2U)
                     << static_cast<uint8_t>(lastNameLength << 1U)
                     << uint8_t(0);
                if (firstNameLength) data.append(reinterpret_cast<uint8_t const*>(fields.firstName.data()), firstNameLength);
                if (lastNameLength) data.append(reinterpret_cast<uint8_t const*>(fields.lastName.data()), lastNameLength);
            }
            data.flushBits();
        }

        void writeGameObjectDataUpdate(ByteBuffer& data, Fields::GameObjectData const& fields)
        {
            writeStructuredChangeMask(data, fields.changes);
            auto changed = [&](std::size_t bit) { return fields.changes.test(bit); };

            // The three collection fields are currently only populated by
            // full replacements. No core setter marks them yet; this keeps
            // their wire representation explicit instead of dropping a dirty
            // bit silently when support is added later.
            if (changed(Fields::GameObjectData::StateWorldEffectIdsBit))
            {
                data << uint32_t(fields.stateWorldEffectIds.size());
                for (uint32_t value : fields.stateWorldEffectIds)
                    data << value;
            }
            if (changed(Fields::GameObjectData::EnableDoodadSetsBit))
            {
                data << uint32_t(fields.enableDoodadSets.size());
                for (int32_t value : fields.enableDoodadSets)
                    data << value;
            }
            if (changed(Fields::GameObjectData::WorldEffectsBit))
            {
                data << uint32_t(fields.worldEffects.size());
                for (int32_t value : fields.worldEffects)
                    data << value;
            }

            if (changed(Fields::GameObjectData::DisplayIdBit)) data << fields.displayId;
            if (changed(Fields::GameObjectData::SpellVisualIdBit)) data << fields.spellVisualId;
            if (changed(Fields::GameObjectData::StateSpellVisualIdBit)) data << fields.stateSpellVisualId;
            if (changed(Fields::GameObjectData::SpawnTrackingStateAnimIdBit)) data << fields.spawnTrackingStateAnimId;
            if (changed(Fields::GameObjectData::SpawnTrackingStateAnimKitIdBit)) data << fields.spawnTrackingStateAnimKitId;
            if (changed(Fields::GameObjectData::StateWorldEffectsQuestObjectiveIdBit)) data << fields.stateWorldEffectsQuestObjectiveId;
            if (changed(Fields::GameObjectData::CreatedByBit)) writeModernGuid(data, fields.createdBy);
            if (changed(Fields::GameObjectData::GuildGuidBit)) writeModernGuid(data, fields.guildGuid);
            if (changed(Fields::GameObjectData::FlagsBit)) data << fields.flags;
            if (changed(Fields::GameObjectData::FlagsBBit)) data << fields.flagsB;
            if (changed(Fields::GameObjectData::ParentRotationBit))
                for (float value : fields.parentRotation) data << value;
            if (changed(Fields::GameObjectData::FactionTemplateBit)) data << fields.factionTemplate;
            if (changed(Fields::GameObjectData::StateBit)) data << fields.state;
            if (changed(Fields::GameObjectData::TypeIdBit)) data << fields.typeId;
            if (changed(Fields::GameObjectData::PercentHealthBit)) data << fields.percentHealth;
            if (changed(Fields::GameObjectData::ArtKitBit)) data << fields.artKit;
            if (changed(Fields::GameObjectData::CustomParamBit)) data << fields.customParam;
            if (changed(Fields::GameObjectData::LevelBit)) data << fields.level;
            if (changed(Fields::GameObjectData::AnimGroupInstanceBit)) data << fields.animGroupInstance;
            if (changed(Fields::GameObjectData::UiWidgetItemIdBit)) data << fields.uiWidgetItemId;
            if (changed(Fields::GameObjectData::UiWidgetItemQualityBit)) data << fields.uiWidgetItemQuality;
            if (changed(Fields::GameObjectData::UiWidgetItemCountBit)) data << fields.uiWidgetItemCount;
            if (changed(Fields::GameObjectData::UnknownU32Bit26_69913)) data << fields.unknownU32_26_69913;
            if (changed(Fields::GameObjectData::UnknownU32Bit27_69913)) data << fields.unknownU32_27_69913;
            data.flushBits();
        }

        void writeDynamicObjectDataUpdate(ByteBuffer& data, Fields::DynamicObjectData const& fields)
        {
            writeStructuredChangeMask(data, fields.changes);
            auto changed = [&](std::size_t bit) { return fields.changes.test(bit); };
            if (changed(Fields::DynamicObjectData::CasterBit)) writeModernGuid(data, fields.caster);
            if (changed(Fields::DynamicObjectData::TypeBit)) data << fields.type;
            if (changed(Fields::DynamicObjectData::SpellVisualBit)) writeSpellCastVisualCreate(data, fields.spellVisual);
            if (changed(Fields::DynamicObjectData::SpellIdBit)) data << fields.spellId;
            if (changed(Fields::DynamicObjectData::RadiusBit)) data << fields.radius;
            if (changed(Fields::DynamicObjectData::CastTimeBit)) data << fields.castTime;
            data.flushBits();
        }

        void writeCorpseDataUpdate(ByteBuffer& data, Fields::CorpseData const& fields)
        {
            writeStructuredChangeMask(data, fields.changes);
            auto changed = [&](std::size_t bit) { return fields.changes.test(bit); };
            if (changed(Fields::CorpseData::DynamicFlagsBit)) data << fields.dynamicFlags;
            if (changed(Fields::CorpseData::OwnerBit)) writeModernGuid(data, fields.owner);
            if (changed(Fields::CorpseData::PartyGuidBit)) writeModernGuid(data, fields.partyGuid);
            if (changed(Fields::CorpseData::GuildGuidBit)) writeModernGuid(data, fields.guildGuid);
            if (changed(Fields::CorpseData::DisplayIdBit)) data << fields.displayId;
            if (changed(Fields::CorpseData::RaceIdBit)) data << fields.raceId;
            if (changed(Fields::CorpseData::SexBit)) data << fields.sex;
            if (changed(Fields::CorpseData::ClassBit)) data << fields.classId;
            if (changed(Fields::CorpseData::FlagsBit)) data << fields.flags;
            if (changed(Fields::CorpseData::FactionTemplateBit)) data << fields.factionTemplate;
            if (changed(Fields::CorpseData::StateSpellVisualKitIdBit)) data << fields.stateSpellVisualKitId;
            if (changed(Fields::CorpseData::ItemsGroupBit))
                for (std::size_t i = 0; i < fields.items.size(); ++i)
                    if (changed(Fields::CorpseData::ItemsFirstBit + i))
                        data << fields.items[i];
            data.flushBits();
        }

        void writeActivePlayerDataUpdate(ByteBuffer& data, Fields::ActivePlayerData const& fields)
        {
            writeStructuredChangeMask(data, fields.changes);
            auto changed = [&](std::size_t bit) { return fields.changes.test(bit); };
            if (changed(Fields::ActivePlayerData::UnknownChangeBit56_69913)) writeModernGuid(data, fields.farsightObject);
            if (changed(Fields::ActivePlayerData::UnknownChangeBit58_69913)) data << fields.coinage;
            if (changed(Fields::ActivePlayerData::UnknownChangeBit60_69913)) data << fields.xp;
            if (changed(Fields::ActivePlayerData::UnknownChangeBit61_69913)) data << fields.nextLevelXp;
            if (changed(Fields::ActivePlayerData::UnknownChangeBit163_69913))
            {
                for (std::size_t i = 0; i < fields.invSlots.size(); ++i)
                    if (changed(Fields::ActivePlayerData::UnknownChangeBit164_69913 + i))
                        writeModernGuid(data, fields.invSlots[i]);
            }
            data.flushBits();
        }
    }

    std::vector<uint8_t> buildValuesUpdateBlock(std::span<const uint8_t> packedGuid, bool ownerVisible, Fields::ObjectData const& objectFields, Fields::ItemData const* itemFields, Fields::ContainerData const* containerFields, Fields::UnitData const* unitFields, Fields::PlayerData const* playerFields, Fields::ActivePlayerData const* activePlayerFields, Fields::GameObjectData const* gameObjectFields, Fields::DynamicObjectData const* dynamicObjectFields, Fields::CorpseData const* corpseFields)
    {
        if (packedGuid.empty())
            return {};

        uint32_t changedObjectTypeMask = 0;
        if (objectFields.hasChanges()) changedObjectTypeMask |= uint32_t(1) << 0; // Object
        if (itemFields && itemFields->hasChanges()) changedObjectTypeMask |= uint32_t(1) << 1; // Item
        if (containerFields && containerFields->hasChanges()) changedObjectTypeMask |= uint32_t(1) << 2; // Container
        if (unitFields && unitFields->hasChanges()) changedObjectTypeMask |= uint32_t(1) << 5; // Unit
        if (playerFields && playerFields->hasChanges()) changedObjectTypeMask |= uint32_t(1) << 6; // Player
        if (ownerVisible && activePlayerFields && activePlayerFields->hasChanges()) changedObjectTypeMask |= uint32_t(1) << 7; // ActivePlayer
        if (gameObjectFields && gameObjectFields->hasChanges()) changedObjectTypeMask |= uint32_t(1) << 8; // GameObject
        if (dynamicObjectFields && dynamicObjectFields->hasChanges()) changedObjectTypeMask |= uint32_t(1) << 9; // DynamicObject
        if (corpseFields && corpseFields->hasChanges()) changedObjectTypeMask |= uint32_t(1) << 10; // Corpse
        if (changedObjectTypeMask == 0)
            return {};

        ByteBuffer payload;
        payload << uint8_t(ownerVisible ? 1 : 0);
        payload << uint8_t(0); // fragment IDs did not change
        payload << uint8_t(1); // CGObject fragment contents changed
        payload << changedObjectTypeMask;

        if (changedObjectTypeMask & (uint32_t(1) << 0)) writeObjectDataUpdate(payload, objectFields);
        if (changedObjectTypeMask & (uint32_t(1) << 1)) writeItemDataUpdate(payload, *itemFields);
        if (changedObjectTypeMask & (uint32_t(1) << 2)) writeContainerDataUpdate(payload, *containerFields);
        if (changedObjectTypeMask & (uint32_t(1) << 5)) writeUnitDataUpdate(payload, *unitFields);
        if (changedObjectTypeMask & (uint32_t(1) << 6)) writePlayerDataUpdate(payload, *playerFields);
        if (changedObjectTypeMask & (uint32_t(1) << 7)) writeActivePlayerDataUpdate(payload, *activePlayerFields);
        if (changedObjectTypeMask & (uint32_t(1) << 8)) writeGameObjectDataUpdate(payload, *gameObjectFields);
        if (changedObjectTypeMask & (uint32_t(1) << 9)) writeDynamicObjectDataUpdate(payload, *dynamicObjectFields);
        if (changedObjectTypeMask & (uint32_t(1) << 10)) writeCorpseDataUpdate(payload, *corpseFields);

        ByteBuffer block;
        block << uint8_t(0); // VALUES
        block.append(packedGuid.data(), packedGuid.size());
        block << uint32_t(payload.size());
        block.append(payload);
        return std::vector<uint8_t>(block.contents(), block.contents() + block.size());
    }

    std::vector<uint8_t> buildUpdateObjectPacket(uint16_t mapId, uint32_t updateCount, std::span<const uint8_t> updateBlocks, uint32_t destroyCount, std::span<const uint8_t> destroyGuids, uint32_t outOfRangeCount, std::span<const uint8_t> outOfRangeGuids)
    {
        const uint64_t totalRemovalCount64 = static_cast<uint64_t>(destroyCount) + outOfRangeCount;
        if (totalRemovalCount64 > std::numeric_limits<uint32_t>::max())
            return {};

        const uint32_t totalRemovalCount = static_cast<uint32_t>(totalRemovalCount64);
        if (updateCount == 0 && totalRemovalCount == 0)
            return {};
        if (updateCount != 0 && updateBlocks.empty())
            return {};
        if (destroyCount != 0 && destroyGuids.empty())
            return {};
        if (outOfRangeCount != 0 && outOfRangeGuids.empty())
            return {};
        if (destroyCount > std::numeric_limits<uint16_t>::max())
            return {};

        ByteBuffer packet;
        packet << mapId << updateCount;
        packet.writeBit(1); // UpdateData header flag
        packet.writeBit(totalRemovalCount != 0);
        packet.flushBits();

        if (totalRemovalCount != 0)
        {
            // Modern retail uses one removal list. The first DestroyCount GUIDs
            // are hard destroys; the remaining GUIDs are normal out-of-range
            // removals. Keep the two queues separate internally and concatenate
            // them only on the wire.
            packet << static_cast<uint16_t>(destroyCount);
            packet << totalRemovalCount;
            if (!destroyGuids.empty())
                packet.append(destroyGuids.data(), destroyGuids.size());
            if (!outOfRangeGuids.empty())
                packet.append(outOfRangeGuids.data(), outOfRangeGuids.size());
        }

        packet << uint32_t(updateBlocks.size());
        if (!updateBlocks.empty())
            packet.append(updateBlocks.data(), updateBlocks.size());

        return std::vector<uint8_t>(packet.contents(), packet.contents() + packet.size());
    }

    std::vector<uint8_t> buildSelfCreatePacket(uint16_t mapId, std::span<const uint8_t> packedGuid, float x, float y, float z, float orientation, std::span<const uint8_t> fieldPayload)
    {
        if (packedGuid.empty() || fieldPayload.empty())
            return {};

        std::array<uint8_t, SELF_PLAYER_MOVEMENT_DEFAULTS_69913.size()> movementTail = SELF_PLAYER_MOVEMENT_DEFAULTS_69913;

        // The protocol default state was rooted. Do not inherit that fixed
        // movement-control state for our generated player.
        uint32_t movementFlags = 0;
        std::memcpy(&movementFlags, movementTail.data(), sizeof(movementFlags));
        movementFlags &= ~uint32_t(0x00000400);
        std::memcpy(movementTail.data(), &movementFlags, sizeof(movementFlags));

        // 69913 carries the self position twice in the CreateObject2
        // movement block. Keep MovementInfo and EntityPosition synchronized.
        constexpr size_t movementPositionOffset = 12;
        constexpr size_t entityPositionOffset = 167;
        std::memcpy(movementTail.data() + movementPositionOffset + 0, &x, sizeof(float));
        std::memcpy(movementTail.data() + movementPositionOffset + 4, &y, sizeof(float));
        std::memcpy(movementTail.data() + movementPositionOffset + 8, &z, sizeof(float));
        std::memcpy(movementTail.data() + movementPositionOffset + 12, &orientation, sizeof(float));
        std::memcpy(movementTail.data() + entityPositionOffset + 0, &x, sizeof(float));
        std::memcpy(movementTail.data() + entityPositionOffset + 4, &y, sizeof(float));
        std::memcpy(movementTail.data() + entityPositionOffset + 8, &z, sizeof(float));

        ByteBuffer block;
        block << uint8_t(UPDATE_TYPE_CREATE_OBJECT_2);
        block.append(packedGuid.data(), packedGuid.size());
        block << uint8_t(OBJECT_TYPE_ACTIVE_PLAYER);
        block << uint8_t(0x8C) << uint8_t(0x00) << uint8_t(0x80);
        block << uint32_t(0);
        block.append(packedGuid.data(), packedGuid.size());
        block.append(movementTail.data(), movementTail.size());
        block << uint32_t(fieldPayload.size());
        block.append(fieldPayload.data(), fieldPayload.size());

        return buildUpdateObjectPacket(mapId, 1, std::span<const uint8_t>(block.contents(), block.size()));
    }


}
