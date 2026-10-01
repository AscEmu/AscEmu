/*
Copyright (c) 2014-2026 AscEmu Team <http://ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "UnitData.hpp"
#include "Definitions/UnitData.hpp"
#include "ChangeMask.hpp"
#include "WireHelpers.hpp"
#include "Trace.hpp"
#include "Nested/VisibleItem.hpp"
#include "Logging/Logger.hpp"

#include <array>

namespace AscEmu::Version::Forever::UpdateFields
{
    namespace
    {
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

    void writeUnitDataCreate(ByteBuffer& data, Fields::UnitData const& fields, bool ownerVisible)
    {
        // Verification source of truth: Definitions::UnitDataCreateFields. Storage member
        // names copied from modern Retail are not Forever semantics unless metadata says VERIFIED.
        data << fields.displayId << fields.npcFlags << fields.npcFlags2 << fields.unknownU32Create3 << fields.unknownU32Create4 << fields.unknownU32Create5;
        data << uint32_t(fields.unknownU32Vector0.size()) << fields.unknownU32Create7 << fields.unknownI32Create8;

        for (uint32_t value : fields.unknownU32Vector0)
            data << value;

        writeModernGuid(data, fields.charm);
        writeModernGuid(data, fields.summon);
        if (ownerVisible)
            writeModernGuid(data, fields.critter);
        writeModernGuid(data, fields.charmedBy);
        writeModernGuid(data, fields.summonedBy);
        writeModernGuid(data, fields.createdBy);
        writeModernGuid(data, fields.unknownGuidCreate16);
        writeModernGuid(data, fields.unknownGuidCreate17);
        writeModernGuid(data, fields.target);
        writeModernGuid(data, fields.unknownGuidCreate19);

        data << fields.unknownU64Create20;
        writeModernGuid(data, fields.unknownGuidCreate21);
        writeModernGuid(data, fields.unknownGuidCreate22);
        writeUnitChannelCreate(data, fields.unknownRecordCreate23);
        data << fields.spellEmpowerStage << fields.unknownU32Create25 << fields.race << fields.classId << fields.playerClassId << fields.sex << fields.creatureType << fields.displayPower << fields.unknownU32Create32 << fields.health;

        for (std::size_t i = 0; i < fields.power.size(); ++i)
            data << fields.power[i] << fields.maxPower[i];

        // Forever creature creates contain both regen arrays as part of the
        // fixed UnitData create payload as well; they are not owner-only on the wire.
        for (std::size_t i = 0; i < fields.powerRegenFlatModifier.size(); ++i)
            data << fields.powerRegenFlatModifier[i] << fields.powerRegenInterruptedFlatModifier[i];

        data << fields.maxHealth << fields.level << fields.effectiveLevel << fields.contentTuningId << fields.scalingLevelMin << fields.scalingLevelMax << fields.scalingLevelDelta << fields.scalingFactionGroup << fields.factionTemplate;

        // Forever: VirtualItems are part of the fixed UnitData prefix and
        // are written immediately after FactionTemplate, before UnitFlags.
        for (Fields::VisibleItem const& value : fields.virtualItems)
            UpdateFields::Nested::writeVisibleItemCreate(data, value);
        // Forever verified from retail creature creates:
        // UnitFlags, UnitFlags2, UnitFlags3, Flags4, AuraState.
        data << fields.unitFlags << fields.unitFlags2 << fields.unitFlags3 << fields.flags4 << fields.auraState;

        for (uint32_t value : fields.attackRoundBaseTime)
            data << value;

        if (ownerVisible)
            data << fields.rangedAttackRoundBaseTime;

        data << fields.boundingRadius << fields.combatReach << fields.displayScale << fields.creatureFamily << fields.overrideCreatureType << fields.nativeDisplayId << fields.nativeXDisplayScale << fields.mountDisplayId << fields.cosmeticMountDisplayId;

        if (ownerVisible)
            data << fields.minDamage << fields.maxDamage << fields.minOffHandDamage << fields.maxOffHandDamage;

        data << fields.standState << fields.petTalentPoints << fields.visFlags << fields.animTier << fields.petNumber << fields.petNameTimestamp << fields.petExperience << fields.unknownAfterPetExperience << fields.petNextLevelExperience;
        data << fields.modCastingSpeed << fields.modCastingSpeedNeg << fields.modSpellHaste << fields.modHaste << fields.modRangedHaste << fields.modHasteRegen << fields.modTimeRate;
        data << fields.createdBySpell << fields.emoteState;

        if (ownerVisible)
        {
            data << fields.unknownBeforeStats;

            for (std::size_t i = 0; i < fields.stats.size(); ++i)
                data << fields.stats[i] << fields.statPosBuff[i] << fields.statNegBuff[i] << fields.statSupportBuff[i];

            for (int32_t value : fields.resistances)
                data << value;

            for (std::size_t i = 0; i < fields.bonusResistanceMods.size(); ++i)
                data << fields.bonusResistanceMods[i] << fields.manaCostModifier[i];
        }

        data << fields.baseMana;
        if (ownerVisible)
            data << fields.baseHealth;

        data << fields.sheatheState << fields.pvpFlags << fields.petFlags << fields.shapeshiftForm;

        if (ownerVisible)
        {
            data << fields.attackPower << fields.attackPowerModPos << fields.attackPowerModNeg << fields.attackPowerMultiplier << fields.attackPowerModSupport;
            data << fields.unknownBeforeRangedAttackPowerA << fields.unknownBeforeRangedAttackPowerB;
            data << fields.rangedAttackPower << fields.rangedAttackPowerModPos << fields.rangedAttackPowerModNeg << fields.rangedAttackPowerMultiplier << fields.rangedAttackPowerModSupport;
            data << fields.mainHandWeaponAttackPower << fields.offHandWeaponAttackPower << fields.rangedWeaponAttackPower << fields.setAttackSpeedAura << fields.lifesteal << fields.minRangedDamage << fields.maxRangedDamage << fields.manaCostMultiplier;
        }

        data << fields.maxHealthModifier << fields.hoverHeight << fields.minItemLevelCutoff << fields.minItemLevel << fields.maxItemLevel << fields.azeriteItemLevel << fields.wildBattlePetLevel << fields.battlePetCompanionExperience << fields.battlePetCompanionNameTimestamp;
        data << fields.interactSpellId << fields.scaleDuration << fields.looksLikeMountId << fields.looksLikeCreatureId << fields.lookAtControllerId << fields.perksVendorItemId << fields.taxiNodesId;
        writeModernGuid(data, fields.unknownGuid0);
        data << uint32_t(fields.passiveSpells.size()) << uint32_t(fields.worldEffects.size()) << uint32_t(fields.channelObjects.size());
        data << fields.flightCapabilityId << fields.glideEventSpeedDivisor << fields.driveCapabilityId << fields.maxHealthModifierFlatNeg << fields.maxHealthModifierFlatPos << fields.silencedSchoolMask;
        if (ownerVisible)
            data << fields.unknownBeforeCurrentAreaId;
        data << fields.currentAreaId << fields.nameplateDistanceMod << fields.autoAttackRangeMod;
        if (ownerVisible)
        {
            data.append(fields.ownerExtension.prefix.data(), fields.ownerExtension.prefix.size());
            writeModernGuid(data, fields.ownerExtension.guidA);
            writeModernGuid(data, fields.ownerExtension.guidB);
            data.append(fields.ownerExtension.suffix.data(), fields.ownerExtension.suffix.size());
        }
        writeModernGuid(data, fields.nameplateAttachToGuid);

        for (Fields::PassiveSpellHistory const& value : fields.passiveSpells)
            writePassiveSpellHistoryCreate(data, value);
        for (int32_t value : fields.worldEffects)
            data << value;
        for (WoWGuid const& value : fields.channelObjects)
            writeModernGuid(data, value);

        data.writeBit(fields.field314);
        data.writeBit(fields.unknownOptionalRecord0.has_value());
        data.flushBits();
        if (fields.unknownOptionalRecord0)
            writeUnitAssistActionDataCreate(data, *fields.unknownOptionalRecord0);
    }

    void writeUnitDataUpdate(ByteBuffer& data, Fields::UnitData const& fields)
    {
        const auto filtered = Definitions::UnitDataUpdate::filterChanges(fields);
        if (fields.changes.test(Fields::UnitData::HealthBit) || fields.changes.test(Fields::UnitData::FlagsBit) || fields.changes.test(Fields::UnitData::Flags2Bit) || fields.changes.test(Fields::UnitData::AuraStateBit))
        {
            uint32_t rawBlocksMask = 0; uint32_t filteredBlocksMask = 0; std::array<uint32_t, 8> rawBlocks{}; std::array<uint32_t, 8> filteredBlocks{};
            for (std::size_t block = 0; block < rawBlocks.size(); ++block) { rawBlocks[block] = getChangeBlock(fields.changes, block); filteredBlocks[block] = getChangeBlock(filtered, block); if (rawBlocks[block] != 0) rawBlocksMask |= uint32_t(1) << block; if (filteredBlocks[block] != 0) filteredBlocksMask |= uint32_t(1) << block; }
            sLogger.info("[ForeverDebug][UF][UnitData] rawBlocksMask=0x{:02X} raw=[{:08X},{:08X},{:08X},{:08X},{:08X},{:08X},{:08X},{:08X}] filteredBlocksMask=0x{:02X} filtered=[{:08X},{:08X},{:08X},{:08X},{:08X},{:08X},{:08X},{:08X}] health={} maxHealth={} flags=0x{:08X} flags2=0x{:08X} auraState=0x{:08X}", rawBlocksMask, rawBlocks[0], rawBlocks[1], rawBlocks[2], rawBlocks[3], rawBlocks[4], rawBlocks[5], rawBlocks[6], rawBlocks[7], filteredBlocksMask, filteredBlocks[0], filteredBlocks[1], filteredBlocks[2], filteredBlocks[3], filteredBlocks[4], filteredBlocks[5], filteredBlocks[6], filteredBlocks[7], fields.health, fields.maxHealth, fields.unitFlags, fields.unitFlags2, fields.auraState);
        }
        traceChangedFields<Definitions::UnitDataUpdate>("UnitData", fields);
        Definitions::UnitDataUpdate::writeUpdate(data, fields);
    }
}
