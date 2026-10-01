/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "PlayerData.hpp"

#include "ChangeMask.hpp"
#include "Definitions/PlayerData.hpp"
#include "Nested/QuestLog.hpp"
#include "Nested/VisibleItem.hpp"
#include "WireHelpers.hpp"
#include "Trace.hpp"

#include "Network/ByteBuffer.hpp"
#include "Logging/Logger.hpp"

#include <algorithm>

namespace AscEmu::Version::Forever::UpdateFields
{
    namespace
    {
        void writeDynamicRecord(ByteBuffer& data, Fields::DynamicRecord const& record)
        {
            if (!record.data.empty())
                data.append(record.data.data(), record.data.size());
        }

        void writeCustomizationCreate(ByteBuffer& data, Fields::ChrCustomizationChoice const& value)
        {
            data << value.optionId << value.choiceId;
        }

        void writeZonePlayerForcedReactionCreate(ByteBuffer& data, Fields::ZonePlayerForcedReaction const& value)
        {
            data << value.factionId << value.reaction;
        }

        void writeCtrOptionsCreate(ByteBuffer& data, Fields::CtrOptions const& value)
        {
            // Forever Forever capture layout:
            //   uint32 ConditionalFlagsCount
            //   uint8  FactionGroup
            //   uint32 ChromieTimeExpansionMask
            //   uint32 ConditionalFlags[Count]
            data << uint32_t(value.conditionalFlags.size()) << value.factionGroup << value.chromieTimeExpansionMask;
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

        bool hasRequiredPlayerOpaqueRecords(Fields::PlayerData const&)
        {
            return true;
        }
    }

    bool writePlayerDataCreate(ByteBuffer& data, Fields::PlayerData const& fields, bool partyMemberVisible)
    {
        // Verification source of truth: Definitions::PlayerDataCreateFields. Neutral canonical
        // names describe unknown slots; modern semantic names survive only as reference metadata.
        if (!hasRequiredPlayerOpaqueRecords(fields))
            return false;

        writeModernGuid(data, fields.unknownGuid0);
        writeModernGuid(data, fields.unknownGuid1);
        writeModernGuid(data, fields.unknownGuid2);
        data << fields.unknownU64_0;
        writeModernGuid(data, fields.unknownGuid3);
        data << fields.unknownU32_0 << fields.unknownU32_1 << fields.unknownU32_2 << fields.unknownU32_3 << fields.unknownI32_0;
        data.append(fields.unknownBeforeCustomizationCounts.data(), fields.unknownBeforeCustomizationCounts.size());
        data << uint32_t(fields.customizations.size()) << uint32_t(fields.unknownCustomizationChoices0.size());

        for (uint8_t value : fields.unknownBytes0)
            data << value;

        data << fields.unknownU8_0 << fields.unknownU8_1 << fields.unknownU8_2 << fields.unknownU8_3 << fields.unknownU32_4 << fields.unknownI32_1;

        if (partyMemberVisible)
        {
            for (Fields::QuestLog const& value : fields.unknownPartyRecords0)
                Nested::writeQuestLogCreate(data, value);
            data << uint32_t(fields.unknownPartyMap0.size());
            for (auto const& [questId, index] : fields.unknownPartyMap0)
                data << questId << index;
            data << uint32_t(fields.unknownPartyDynamicRecords0.size());
        }

        for (Fields::VisibleItem const& value : fields.unknownVisibleItemRecords0)
            Nested::writeVisibleItemCreate(data, value);

        data << fields.unknownI32_2 << fields.unknownI32_3 << fields.unknownU32_5 << fields.unknownU32_6 << fields.unknownI32_4 << fields.unknownI32_5;

        for (float value : fields.unknownFloatArray0)
            data << value;

        data << fields.unknownU8_4 << fields.unknownI32_6 << fields.unknownI64_0;
        data << uint32_t(fields.unknownDynamicRecords0.size());

        for (Fields::ZonePlayerForcedReaction const& value : fields.unknownFixedRecords0)
            writeZonePlayerForcedReactionCreate(data, value);

        data << fields.unknownI32_7 << fields.unknownI32_8 << fields.unknownI32_9;
        data << uint32_t(fields.unknownDynamicRecords1.size());
        writeCtrOptionsCreate(data, fields.unknownCtrOptions0);
        data << fields.unknownI32_10 << fields.unknownI32_11;
        writeDungeonScoreSummaryCreate(data, fields.unknownDungeonScore0);
        writeModernGuid(data, fields.unknownLeaverInfo0.bnetAccountGuid);
        data << fields.unknownLeaverInfo0.leaveScore << fields.unknownLeaverInfo0.seasonId << fields.unknownLeaverInfo0.totalLeaves
             << fields.unknownLeaverInfo0.totalSuccesses << fields.unknownLeaverInfo0.consecutiveSuccesses
             << fields.unknownLeaverInfo0.lastPenaltyTime << fields.unknownLeaverInfo0.leaverExpirationTime << fields.unknownLeaverInfo0.flags;
        data.writeBit(fields.unknownLeaverInfo0.isLeaver);
        data.flushBits();
        writeModernGuid(data, fields.unknownGuid4);
        data << fields.unknownI32_12;

        for (Fields::ItemInstance const& value : fields.unknownItemInstances0)
            writeItemInstanceCreate(data, value);
        data << uint32_t(fields.unknownI32Vector0.size());

        for (uint32_t value : fields.attackRoundBaseTime)
            data << value;

        writeCustomTabardInfoCreate(data, fields.unknownCustomTabard0);
        writeNpcAsPlayerInfoCreate(data, fields.unknownNpcAsPlayer0);
        data.append(fields.unknownBeforeCustomizationPayload.data(), fields.unknownBeforeCustomizationPayload.size());

        for (Fields::ChrCustomizationChoice const& value : fields.customizations)
            writeCustomizationCreate(data, value);
        for (Fields::ChrCustomizationChoice const& value : fields.unknownCustomizationChoices0)
            writeCustomizationCreate(data, value);

        if (partyMemberVisible)
            for (Fields::QuestLog const& value : fields.unknownPartyDynamicRecords0)
                Nested::writeQuestLogCreate(data, value);

        for (Fields::DynamicRecord const& value : fields.unknownDynamicRecords0)
            writeDynamicRecord(data, value);
        for (Fields::DynamicRecord const& value : fields.unknownDynamicRecords1)
            writeDynamicRecord(data, value);
        for (int32_t value : fields.unknownI32Vector0)
            data << value;

        // Forever 1.60.1 Forever carries first and last name separately.
        // The verified wire layout uses byte-aligned 6-bit lengths.
        const std::size_t firstNameLength = std::min<std::size_t>(fields.firstName.size(), 63U);
        const std::size_t lastNameLength = std::min<std::size_t>(fields.lastName.size(), 63U);
        data << static_cast<uint8_t>(firstNameLength << 2U) << static_cast<uint8_t>(lastNameLength << 1U);

        uint8_t foreverNameFlags = 0;
        if (partyMemberVisible && fields.unknownNameFlag0)
            foreverNameFlags |= 0x80U;
        if (fields.unknownNameFlag1)
            foreverNameFlags |= 0x40U;
        if (fields.unknownOptionalNamePayload0.has_value())
            foreverNameFlags |= 0x20U;
        data << foreverNameFlags;

        if (firstNameLength != 0U)
            data.append(reinterpret_cast<uint8_t const*>(fields.firstName.data()), firstNameLength);
        if (lastNameLength != 0U)
            data.append(reinterpret_cast<uint8_t const*>(fields.lastName.data()), lastNameLength);

        if (fields.unknownOptionalNamePayload0)
            writeDynamicRecord(data, *fields.unknownOptionalNamePayload0);
        return true;
    }

    void writePlayerDataUpdate(ByteBuffer& data, Fields::PlayerData const& fields)
    {
        const auto changes = Definitions::getPlayerDataAllowedChanges(fields);

        // Retail 70124 quest-accept delta has IsQuestLogChangesMaskSkipped = 0.
        writeChangeMask(data, changes);
        data.writeBit(0);
        data.flushBits();

        const auto changed = [&changes](std::size_t bit) { return changes.test(bit); };

        const auto traceMetadata = [](Definitions::PlayerFieldMetadata const& metadata, int32_t index, int64_t value)
        {
            traceManualField("PlayerData", metadata.bit, index, metadata.name, metadata.verification, metadata.referenceName, metadata.wireType, value);
        };

        if (changed(Fields::PlayerData::DuelArbiterBit)) traceMetadata(Definitions::PlayerDuelArbiter, -1, static_cast<int64_t>(fields.unknownGuid0.getRawGuid()));
        if (changed(Fields::PlayerData::PlayerFlagsBit)) traceMetadata(Definitions::PlayerFlags, -1, static_cast<int64_t>(fields.unknownU32_0));
        if (changed(Fields::PlayerData::QuestLogQuestIdToIndexBit)) traceMetadata(Definitions::PlayerQuestLogQuestIdToIndex, -1, static_cast<int64_t>(fields.questLogQuestIdToIndexChanges.size()));
        if (changed(Fields::PlayerData::CurrentSpecBit)) traceMetadata(Definitions::PlayerCurrentSpec, -1, static_cast<int64_t>(fields.unknownU32_6));
        if (changed(Fields::PlayerData::NameBit)) traceMetadata(Definitions::PlayerName, -1, static_cast<int64_t>(fields.firstName.size() + fields.lastName.size()));
        if (changed(Fields::PlayerData::QuestLogGroupBit)) traceMetadata(Definitions::PlayerQuestLogGroup, -1, 0);
        if (changed(Fields::PlayerData::VisibleItemsGroupBit)) traceMetadata(Definitions::PlayerVisibleItemsGroup, -1, 0);

        if (changed(Fields::PlayerData::DuelArbiterBit)) writeModernGuid(data, fields.unknownGuid0);
        if (changed(Fields::PlayerData::PlayerFlagsBit)) data << fields.unknownU32_0;
        if (changed(Fields::PlayerData::QuestLogQuestIdToIndexBit))
        {
            Nested::writeQuestLogQuestIdToIndexUpdate(data, fields);
            sLogger.info("[ForeverDebug][QuestLog] serialize questIdToIndex changes={}", fields.questLogQuestIdToIndexChanges.size());
        }
        if (changed(Fields::PlayerData::CurrentSpecBit)) data << fields.unknownU32_6;
        if (changed(Fields::PlayerData::NameBit))
        {
            const std::size_t firstNameLength = std::min<std::size_t>(fields.firstName.size(), 63U);
            const std::size_t lastNameLength = std::min<std::size_t>(fields.lastName.size(), 63U);
            data << static_cast<uint8_t>(firstNameLength << 2U) << static_cast<uint8_t>(lastNameLength << 1U) << uint8_t(0);
            if (firstNameLength) data.append(reinterpret_cast<uint8_t const*>(fields.firstName.data()), firstNameLength);
            if (lastNameLength) data.append(reinterpret_cast<uint8_t const*>(fields.lastName.data()), lastNameLength);
        }

        if (changed(Fields::PlayerData::QuestLogGroupBit))
        {
            for (std::size_t i = 0; i < fields.unknownPartyRecords0.size(); ++i)
            {
                if (!changed(Fields::PlayerData::QuestLogFirstBit + i))
                    continue;

                if (fields.questLogQuestIdChanged.test(i))
                {
                    Nested::writeQuestLogQuestIdUpdate(data, fields.unknownPartyRecords0[i]);
                    sLogger.info("[ForeverDebug][QuestLog] serialize sniff-exact QuestID slot={} questId={}", i, fields.unknownPartyRecords0[i].questId);
                }
                else
                {
                    Nested::writeQuestLogCreate(data, fields.unknownPartyRecords0[i]);
                }
            }
        }

        if (changed(Fields::PlayerData::VisibleItemsGroupBit))
        {
            for (std::size_t i = 0; i < fields.unknownVisibleItemRecords0.size(); ++i)
                if (changed(Fields::PlayerData::VisibleItemsFirstBit + i))
                    Nested::writeVisibleItemUpdate(data, fields.unknownVisibleItemRecords0[i]);
        }

        data.flushBits();
    }
}
