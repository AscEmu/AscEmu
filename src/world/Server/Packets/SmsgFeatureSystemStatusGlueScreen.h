/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <array>
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    struct ForeverGlueCopySourceRegionRecord
    {
        uint8_t unknown0 = 0x50;
        std::array<int32_t, 9> values = { 10, 60000, 0, 1893, 1, 60000, 0, 1893, 1 };
    };

    struct ForeverGlueGameModeData
    {
        uint8_t gameMode = 0;
        int32_t contentSetId = 0;
        int32_t gameModeRecordId = 0;
    };

    struct ForeverGlueGameRuleValuePair
    {
        int32_t rule = 0;
        int32_t value = 0;
        float valueF = 0.0f;
    };

    class SmsgFeatureSystemStatusGlueScreen : public ManagedPacket
    {
    public:
        // First eight Forever 1.60.1.69893 feature bits are proven by the
        // working capture. Their ordering also matches the modern packet.
        bool bpayStoreAvailable = true;
        bool charUndeleteEnabled = true;
        bool commerceServerEnabled = false;
        bool paidCharacterTransfersBetweenBnetAccountsEnabled = false;
        bool veteranTokenRedeemWillKick = true;
        bool worldTokenRedeemWillKick = false;
        bool expansionPreorderInStore = false;
        bool kioskModeEnabled = true;

        // Remaining 40 feature bits are capture-proven but not yet named for
        // Forever. Keep them as an explicit bit group instead of inventing
        // semantics or embedding the complete packet as a raw payload.
        std::array<uint8_t, 5> foreverUnmappedFeatureFlagBytes = { 0x02, 0x40, 0x00, 0x0A, 0x00 };

        uint32_t commercePricePollTimeSeconds = 300;
        int64_t redeemForBalanceAmount = 0;
        int32_t maxCharactersOnThisRealm = 70;
        int32_t activeBoostType = 9;
        int32_t trialBoostType = 0;
        int32_t minimumExpansionLevel = 0;
        int32_t maximumExpansionLevel = 0;
        int32_t contentSetId = 137;
        int32_t activeTimerunningSeasonId = 0;

        // This 32-bit field occupies the position previously labelled
        // remainingTimerunningSeasonSeconds. Its captured value is not a
        // plausible duration, so leave the semantic name unproven for 69893.
        int32_t foreverUnknownAfterActiveTimerunningSeasonId = 0x2F616369;

        int32_t timerunningConversionMinCharacterAge = 86400;
        int32_t timerunningConversionMaxSeasonId = -1;
        int16_t maxPlayerGuidLookupsPerRequest = 32;
        int16_t nameLookupTelemetryInterval = 32;
        uint32_t notFoundCacheTimeSeconds = 10;
        int32_t mostRecentTimeEventId = 0;
        uint32_t eventRealmQueues = 0;

        std::vector<ForeverGlueCopySourceRegionRecord> liveRegionCharacterCopySourceRegions =
        {
            {}
        };

        std::vector<ForeverGlueGameModeData> disabledGameModes =
        {
            { 3, 13, 0 },
            { 2, 5, 0 }
        };

        std::vector<ForeverGlueGameRuleValuePair> gameRules =
        {
            { 216, 1, 0.0f },
            { 174, 1, 0.0f },
            { 206, 1, 0.0f },
            { 109, 1, 0.0f },
            { 114, 1, 0.0f },
            { 211, 1, 0.0f },
            { 112, 1, 0.0f },
            { 173, 1, 0.0f },
            { 154, 0, 0.0f },
            { 180, 0, 0.0f },
            { 181, 1, 0.0f },
            { 182, 1, 0.0f },
            { 195, 1, 0.0f },
            { 200, 0, 0.5f },
            { 23, 1, 0.0f },
            { 98, 0, 0.0f },
            { 93, 2, 0.0f },
            { 102, 1, 0.0f },
            { 42, 1, 0.0f },
            { 40, 0, 0.0f },
            { 157, 1, 0.0f },
            { 190, 0, 0.0f },
            { 162, 1, 0.0f },
            { 209, 1, 0.0f },
            { 89, 1, 0.0f },
            { 177, 1, 0.0f },
            { 197, 1, 0.0f },
            { 199, -5, 0.0f },
            { 63, 1, 0.0f },
            { 90, 1, 0.0f },
            { 3, 10, 0.0f },
            { 33, 20, 0.0f },
            { 129, 0, 0.0f },
            { 5, 1787684400, 0.0f },
            { 228, 1, 0.0f },
            { 101, 1, 0.0f },
            { 237, 0, 0.0f }
        };

        std::vector<int32_t> availableGameModeIds = { 8, 14, 15, 16, 4 };

        SmsgFeatureSystemStatusGlueScreen() : ManagedPacket(SMSG_FEATURE_SYSTEM_STATUS_GLUE_SCREEN, 613) {}

    protected:
        size_t expectedSize() const override
        {
            if (!m_protocol.isForever())
                return 0U;

            constexpr size_t fixedSize = 94U;
            constexpr size_t copySourceRegionRecordSize = 37U;
            constexpr size_t gameModeDataSize = 9U;
            constexpr size_t gameRuleValuePairSize = 12U;

            return fixedSize
                + liveRegionCharacterCopySourceRegions.size() * copySourceRegionRecordSize
                + disabledGameModes.size() * gameModeDataSize
                + gameRules.size() * gameRuleValuePairSize
                + availableGameModeIds.size() * sizeof(int32_t);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet.writeBit(bpayStoreAvailable);
            packet.writeBit(charUndeleteEnabled);
            packet.writeBit(commerceServerEnabled);
            packet.writeBit(paidCharacterTransfersBetweenBnetAccountsEnabled);
            packet.writeBit(veteranTokenRedeemWillKick);
            packet.writeBit(worldTokenRedeemWillKick);
            packet.writeBit(expansionPreorderInStore);
            packet.writeBit(kioskModeEnabled);

            // ByteBuffer writes bits MSB-first. Re-emit the five unproven
            // feature-flag bytes as 40 individual bits so the field remains a
            // bit group and the captured C9 02 40 00 0A 00 prefix is reproduced.
            for (const uint8_t flagByte : foreverUnmappedFeatureFlagBytes)
                for (int32_t bit = 7; bit >= 0; --bit)
                    packet.writeBit((flagByte >> bit) & 1U);
            packet.flushBits();

            packet << commercePricePollTimeSeconds;
            packet << redeemForBalanceAmount;
            packet << maxCharactersOnThisRealm;
            packet << static_cast<uint32_t>(liveRegionCharacterCopySourceRegions.size());
            packet << activeBoostType;
            packet << trialBoostType;
            packet << minimumExpansionLevel;
            packet << maximumExpansionLevel;
            packet << contentSetId;
            packet << static_cast<uint32_t>(disabledGameModes.size());
            packet << static_cast<uint32_t>(gameRules.size());
            packet << static_cast<uint32_t>(availableGameModeIds.size());
            packet << activeTimerunningSeasonId;
            packet << foreverUnknownAfterActiveTimerunningSeasonId;
            packet << timerunningConversionMinCharacterAge;
            packet << timerunningConversionMaxSeasonId;
            packet << maxPlayerGuidLookupsPerRequest;
            packet << nameLookupTelemetryInterval;
            packet << notFoundCacheTimeSeconds;
            packet << uint32_t(0); // DebugTimeEvents count in the captured 69893 packet.
            packet << mostRecentTimeEventId;
            packet << eventRealmQueues;

            for (const auto& record : liveRegionCharacterCopySourceRegions)
            {
                packet << record.unknown0;
                for (const int32_t value : record.values)
                    packet << value;
            }

            for (const auto& gameMode : disabledGameModes)
            {
                packet << gameMode.gameMode;
                packet << gameMode.contentSetId;
                packet << gameMode.gameModeRecordId;
            }

            for (const auto& rule : gameRules)
            {
                packet << rule.rule;
                packet << rule.value;
                packet << rule.valueF;
            }

            for (const int32_t gameModeId : availableGameModeIds)
                packet << gameModeId;

            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
