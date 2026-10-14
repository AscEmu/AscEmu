/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    struct ForeverFeatureSocialQueueConfig
    {
        bool toastsDisabled = false;
        float toastDuration = 7.0f;
        float delayDuration = 10.0f;
        float queueMultiplier = 1.0f;
        float playerMultiplier = 1.0f;
        float playerFriendValue = 5.0f;
        float playerGuildValue = 1.0f;
        float throttleInitialThreshold = 0.0f;
        float throttleDecayTime = 60.0f;
        float throttlePrioritySpike = 20.0f;
        float throttleMinThreshold = 0.0f;
        float throttlePvPPriorityNormal = 50.0f;
        float throttlePvPPriorityLow = 1.0f;
        float throttlePvPHonorThreshold = 10.0f;
        float throttleLfgListPriorityDefault = 50.0f;
        float throttleLfgListPriorityAbove = 100.0f;
        float throttleLfgListPriorityBelow = 50.0f;
        float throttleLfgListIlvlScalingAbove = 1.0f;
        float throttleLfgListIlvlScalingBelow = 1.0f;
        float throttleRfPriorityAbove = 100.0f;
        float throttleRfIlvlScalingAbove = 1.0f;
        float throttleDfMaxItemLevel = 850.0f;
        float throttleDfBestPriority = 80.0f;
    };

    struct ForeverFeaturePackedGuid128
    {
        uint64_t low = 0;
        uint64_t high = 0;
    };

    struct ForeverFeatureSquelchInfo
    {
        bool isSquelched = false;
        ForeverFeaturePackedGuid128 bnetAccountGuid = { 0x000000004250F08EULL, 0x7800000000000000ULL };
        ForeverFeaturePackedGuid128 guildGuid = {};
    };

    struct ForeverFeatureGameModeData
    {
        uint8_t gameMode = 0;
        int32_t contentSetId = 0;
        int32_t gameModeRecordId = 0;
    };

    struct ForeverFeatureGameRuleValuePair
    {
        int32_t rule = 0;
        int32_t value = 0;
        float valueF = 0.0f;
    };

    struct ForeverFeatureChatThrottleParams
    {
        int32_t maxTries = 0;
        int32_t triesRestoredPerSecond = 0;
        int32_t usedTriesPerMessage = 0;
    };

    struct ForeverFeatureSavedThrottleObjectState
    {
        uint32_t maxTries = 0;
        uint32_t perMilliseconds = 0;
        uint32_t tryCount = 0;
        uint32_t lastResetTimeBeforeNow = 0;
    };

    struct ForeverFeatureEuropaTicketConfig
    {
        bool ticketsEnabled = false;
        bool bugsEnabled = true;
        bool complaintsEnabled = false;
        bool suggestionsEnabled = true;
        ForeverFeatureSavedThrottleObjectState throttleState = { 10, 60000, 0, 23261 };
        ForeverFeatureSavedThrottleObjectState expensiveThrottleState = { 1, 60000, 0, 23261 };
    };

    enum class FeatureSystemStatusLayout : uint8_t
    {
        Legacy,
        ForeverInWorld
    };

    class SmsgFeatureSystemStatus : public ManagedPacket
    {
    public:
        uint8_t unknown1;
        uint8_t unknown2;

        // Forever 1.60.1.69913 in-world layout. The scalar fields and arrays
        // below are mapped from the packet structure used by the client. The
        // final feature-bit group is still build-specific and therefore kept
        // as one explicit bit field instead of inventing names for unproven bits.
        uint8_t complaintStatus = 2;
        uint32_t cfgRealmId = 2;
        int32_t cfgRealmRecId = 4618;
        uint32_t rafMaxRecruits = 0;
        uint32_t rafMaxRecruitMonths = 0;
        uint32_t rafMaxRecruitmentUses = 0;
        uint32_t rafDaysInCycle = 0;
        uint32_t rafRewardsVersion = 0;
        uint32_t commercePricePollTimeSeconds = 300;
        ForeverFeatureSocialQueueConfig quickJoinConfig;
        int64_t redeemForBalanceAmount = 0;
        uint32_t clubsPresenceDelay = 0;
        uint32_t clubPresenceUnsubscribeDelay = 60000;
        ForeverFeatureSquelchInfo squelch;
        int32_t contentSetId = 137;
        int32_t activeTimerunningSeasonId = 0;
        int32_t remainingTimerunningSeasonSeconds = 1;
        int16_t maxPlayerGuidLookupsPerRequest = 32;
        int16_t nameLookupTelemetryInterval = 32;
        uint32_t notFoundCacheTimeSeconds = 10;
        uint32_t realmPvpTypeOverride = 0;
        ForeverFeatureChatThrottleParams addonChatThrottle = { 10, 1, 1 };
        ForeverFeatureChatThrottleParams guildChatThrottle = { 0, 20, 1 };
        ForeverFeatureChatThrottleParams groupChatThrottle = { 0, 20, 1 };
        float addonPerformanceMsgWarning = 0.0f;
        float addonPerformanceMsgError = 0.0f;
        float addonPerformanceMsgOverall = 0.0f;

        std::vector<ForeverFeatureGameModeData> disabledGameModes =
        {
            { 3, 13, 0 },
            { 2, 5, 0 }
        };

        std::vector<ForeverFeatureGameRuleValuePair> gameRules =
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

        // Exact eight-byte feature bit group from the working 69913 packet.
        // Individual positions are intentionally not named until the Forever
        // reader layout is proven for this build.
        uint64_t foreverFeatureBits = 0xE133E46448004400ULL;
        ForeverFeatureEuropaTicketConfig europaTicketSystemStatus;

        SmsgFeatureSystemStatus() : SmsgFeatureSystemStatus(0, 0) {}

        SmsgFeatureSystemStatus(uint8_t unknown1, uint8_t unknown2) :
            ManagedPacket(SMSG_FEATURE_SYSTEM_STATUS, 0),
            unknown1(unknown1),
            unknown2(unknown2),
            layout(FeatureSystemStatusLayout::Legacy) {}

        explicit SmsgFeatureSystemStatus(FeatureSystemStatusLayout layout) :
            ManagedPacket(SMSG_FEATURE_SYSTEM_STATUS, layout == FeatureSystemStatusLayout::ForeverInWorld ? 723U : 0U),
            unknown1(0),
            unknown2(0),
            layout(layout) {}

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.isForever() && layout == FeatureSystemStatusLayout::ForeverInWorld)
                return 723U;

            if (m_protocol.expansion > WoW::Expansion::_Cata)
                return 4 + 4 + 4 + 1 + 4 + 2 + 4 + 4 + 4 + 4 + 4 + 4 + 4;
            else if (m_protocol.expansion > WoW::Expansion::_WotLK)
                return 1 + 4 + 4 + 4 + 4 + 2 + 4 + 4 + 4 + 4 + 4 + 4 + 4;
            else
                return 1 + 1;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever() && layout == FeatureSystemStatusLayout::ForeverInWorld)
                return serialiseForeverInWorld(packet);

            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            if (m_protocol.expansion == WoW::Expansion::_Cata)
            {
                bool featureBitFour = true;

                packet << unknown1 << uint32_t(1) << uint32_t(1) << uint32_t(2) << uint32_t(0);
                packet.writeBit(1);
                packet.writeBit(1);
                packet.writeBit(0);
                packet.writeBit(featureBitFour);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.flushBits();

                if (featureBitFour)
                    packet << uint32_t(1) << uint32_t(0) << uint32_t(10) << uint32_t(60);
            }
            else if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                bool isFeedbackSysEnabled = false;
                bool isExcessiveWarningEnabled = false;

                packet << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint8_t(2) << uint32_t(0);
                packet.writeBit(0);
                packet.writeBit(1);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.writeBit(1);
                packet.writeBit(0);
                packet.writeBit(isExcessiveWarningEnabled);
                packet.writeBit(0);
                packet.writeBit(isFeedbackSysEnabled);
                packet.flushBits();

                if (isExcessiveWarningEnabled)
                    packet << uint32_t(14400) << uint32_t(0) << uint32_t(0);

                if (isFeedbackSysEnabled)
                    packet << uint32_t(0) << uint32_t(1) << uint32_t(10) << uint32_t(60000);
            }
            else
            {
                packet << unknown1 << unknown2;
            }
            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }

    private:
        FeatureSystemStatusLayout layout;

        static void writePackedGuid128(WorldPacket& packet, const ForeverFeaturePackedGuid128& guid)
        {
            uint8_t bytes[16] = {};
            for (uint32_t i = 0; i < 8U; ++i)
            {
                bytes[i] = static_cast<uint8_t>((guid.low >> (i * 8U)) & 0xFFU);
                bytes[i + 8U] = static_cast<uint8_t>((guid.high >> (i * 8U)) & 0xFFU);
            }

            uint16_t mask = 0;
            for (uint32_t i = 0; i < 16U; ++i)
                if (bytes[i] != 0)
                    mask |= static_cast<uint16_t>(1U << i);

            packet << mask;
            for (uint32_t i = 0; i < 16U; ++i)
                if (bytes[i] != 0)
                    packet << bytes[i];
        }

        static void writeSavedThrottleState(WorldPacket& packet, const ForeverFeatureSavedThrottleObjectState& state)
        {
            packet << state.maxTries;
            packet << state.perMilliseconds;
            packet << state.tryCount;
            packet << state.lastResetTimeBeforeNow;
        }

        bool serialiseForeverInWorld(WorldPacket& packet)
        {
            packet << complaintStatus;
            packet << cfgRealmId;
            packet << cfgRealmRecId;
            packet << rafMaxRecruits;
            packet << rafMaxRecruitMonths;
            packet << rafMaxRecruitmentUses;
            packet << rafDaysInCycle;
            packet << rafRewardsVersion;
            packet << commercePricePollTimeSeconds;

            packet << quickJoinConfig.toastDuration;
            packet << quickJoinConfig.delayDuration;
            packet << quickJoinConfig.queueMultiplier;
            packet << quickJoinConfig.playerMultiplier;
            packet << quickJoinConfig.playerFriendValue;
            packet << quickJoinConfig.playerGuildValue;
            packet << quickJoinConfig.throttleInitialThreshold;
            packet << quickJoinConfig.throttleDecayTime;
            packet << quickJoinConfig.throttlePrioritySpike;
            packet << quickJoinConfig.throttleMinThreshold;
            packet << quickJoinConfig.throttlePvPPriorityNormal;
            packet << quickJoinConfig.throttlePvPPriorityLow;
            packet << quickJoinConfig.throttlePvPHonorThreshold;
            packet << quickJoinConfig.throttleLfgListPriorityDefault;
            packet << quickJoinConfig.throttleLfgListPriorityAbove;
            packet << quickJoinConfig.throttleLfgListPriorityBelow;
            packet << quickJoinConfig.throttleLfgListIlvlScalingAbove;
            packet << quickJoinConfig.throttleLfgListIlvlScalingBelow;
            packet << quickJoinConfig.throttleRfPriorityAbove;
            packet << quickJoinConfig.throttleRfIlvlScalingAbove;
            packet << quickJoinConfig.throttleDfMaxItemLevel;
            packet << quickJoinConfig.throttleDfBestPriority;
            packet.writeBit(quickJoinConfig.toastsDisabled);
            packet.flushBits();

            packet << redeemForBalanceAmount;
            packet << clubsPresenceDelay;
            packet << clubPresenceUnsubscribeDelay;

            packet.writeBit(squelch.isSquelched);
            packet.flushBits();
            writePackedGuid128(packet, squelch.bnetAccountGuid);
            writePackedGuid128(packet, squelch.guildGuid);

            packet << contentSetId;
            packet << static_cast<uint32_t>(disabledGameModes.size());
            packet << static_cast<uint32_t>(gameRules.size());
            packet << activeTimerunningSeasonId;
            packet << remainingTimerunningSeasonSeconds;
            packet << maxPlayerGuidLookupsPerRequest;
            packet << nameLookupTelemetryInterval;
            packet << notFoundCacheTimeSeconds;
            packet << realmPvpTypeOverride;
            packet << addonChatThrottle.maxTries;
            packet << addonChatThrottle.triesRestoredPerSecond;
            packet << addonChatThrottle.usedTriesPerMessage;
            packet << guildChatThrottle.usedTriesPerMessage;
            packet << guildChatThrottle.triesRestoredPerSecond;
            packet << groupChatThrottle.usedTriesPerMessage;
            packet << groupChatThrottle.triesRestoredPerSecond;
            packet << addonPerformanceMsgWarning;
            packet << addonPerformanceMsgError;
            packet << addonPerformanceMsgOverall;

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

            packet.writeBits(foreverFeatureBits, 64);
            packet.flushBits();

            packet.writeBit(europaTicketSystemStatus.ticketsEnabled);
            packet.writeBit(europaTicketSystemStatus.bugsEnabled);
            packet.writeBit(europaTicketSystemStatus.complaintsEnabled);
            packet.writeBit(europaTicketSystemStatus.suggestionsEnabled);
            packet.flushBits();
            writeSavedThrottleState(packet, europaTicketSystemStatus.throttleState);
            writeSavedThrottleState(packet, europaTicketSystemStatus.expensiveThrottleState);

            return true;
        }
    };
}
