/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MirrorVars.hpp"

namespace AscEmu
{
    namespace
    {
        const std::vector<MirrorVar> camelotMirrorVars =
        {
            { "housingServiceEnabled", "1" },
            { "housingEnableBuyHouse", "1" },
            { "externalEventURL", "" },
            { "housingMarketViewBundleTelemThrottle", "10" },
            { "legacyFriendSystemEnabledClient", "0" },
            { "housingCatalog_CartSizeLimit", "20" },
            { "shop2SSOMaxTries", "15" },
            { "limitedLuaResourcesGlobalCapacityTimer", "500" },
            { "recentAlliesRequestDataThrottle", "180" },
            { "imageSharingPageFieldId", "" },
            { "shop2DynamicBundleMaxTries", "10" },
            { "enableEndgameEditRestrictionsForLFGText", "1" },
            { "housingEnableDeleteHouse", "1" },
            { "battleNetTitleFriendsEnabledClient", "0" },
            { "bnetDelegationURL", "https://identity.battle.net/bridge/v1/delegation" },
            { "shop2PendingOrderPollCheckoutEnabled", "1" },
            { "luaScriptBucketThrottleEnabled", "1" },
            { "housingBlueprintImportEnabled", "1" },
            { "housingMarketClearCartTelemThrottle", "5" },
            { "luaScriptBucketThrottleMaxMsBurstRestricted", "1000" },
            { "limitedLuaResourcesGlobalCapacityAnim", "50000" },
            { "limitedLuaResourcesEnabled", "0" },
            { "pvpTrainingGroundsEnabledClient", "1" },
            { "shop2OrderStatusMaxTries", "3" },
            { "shop2VCSegmentStr", "T2_WOW_PTR" },
            { "kioskModeEnabled", "0" },
            { "damageMeterProcessingEnabled", "1" },
            { "shop2TelemetryAllowed", "1" },
            { "addonChatRestrictionsEnabled", "1" },
            { "limitedLuaResourcesGlobalCapacityFontString", "50000" },
            { "limitedLuaResourcesAddonCapacityTimer", "500" },
            { "imageSharingScreenshotQuality", "70" },
            { "kioskSessionDurationMinutes1", "30" },
            { "housingExteriorLightsRadiusMultiplier", "0.500000" },
            { "housingExpertDecor_Scale_Outdoor_Max", "2.000000" },
            { "shop2HostUrlAuth", "https://oauth.integration.battle.net" },
            { "housingBlueprintExportEnabled", "1" },
            { "shop2VirtualCurrencyBalanceMaxTries", "3" },
            { "housingExteriorLightsAllowed", "1" },
            { "limitedLuaResourcesAddonCapacityFontString", "5000" },
            { "luaScriptBucketThrottleMaxMsBurstNormal", "20000" },
            { "battleNetTitleFriendTagsEnabledClient", "0" },
            { "housingMarketEnabled", "1" },
            { "housingMarketThrottleTimePeriodMs", "10000" },
            { "housingExpertDecor_Scale_Outdoor_Min", "0.200000" },
            { "raidLockoutExtendEnabled", "0" },
            { "hardcoreScriptThrottlingEnabled", "1" },
            { "shop2UseConnectedRealmGameServiceRegionId", "0" },
            { "shop2ClientErrorTelemetryAllowed", "1" },
            { "kioskSessionDurationMinutes2", "45" },
            { "bpayStorePurchaseTimeout", "180" },
            { "limitedLuaResourcesGlobalCapacityFont", "3000" },
            { "housingExpertDecor_Scale_Indoor_Min", "0.200000" },
            { "socialUIRaidListEnabledClient", "1" },
            { "shop2DefaultCurrencyMaxTries", "5" },
            { "shop2BlockedPlacements", "" },
            { "housingEnableMoveHouse", "1" },
            { "housingDecorReportScreenshotDistanceThreshold", "150.000000" },
            { "housingMarketViewInStoreTelemThrottle", "5" },
            { "limitedLuaResourcesAddonCapacityAnim", "5000" },
            { "shop2DefaultCurrencyPerMilliseconds", "1000" },
            { "shop2PMTMaxTries", "15" },
            { "shop2VCPlacementStr", "9aad42ec-a68c-487b-a300-b67518267a85" },
            { "shop2POPStr", "ffe137fd-85a7-4b46-a8c7-7543a314739e" },
            { "housingEntryRoomMoveEnabled", "1" },
            { "shop2ClientIdStr", "763293d2857941c89d0ef89d67aa825e" },
            { "performHousingExpansionCheckClient", "0" },
            { "damageMeterCacheEnabled", "1" },
            { "socialUIEnabledClient", "0" },
            { "shop2VirtualCurrencyBalancePerMilliseconds", "1000" },
            { "imageSharingPageSuffix", "" },
            { "limitedLuaResourcesAddonCapacityFrame", "10000" },
            { "shop2PMTPerMilliseconds", "1000" },
            { "sellAllJunkEnabled", "1" },
            { "housingBlueprintsEnabled", "1" },
            { "imageSharingEnabled", "0" },
            { "shop2DynamicBundlePerMilliseconds", "1000" },
            { "luaScriptBucketThrottleMaxMsPerSecondNormal", "800" },
            { "bpayStoreEnable", "0" },
            { "limitedLuaResourcesGlobalCapacityAnimGroup", "20000" },
            { "limitedLuaResourcesAddonCapacityAnimGroup", "2000" },
            { "housingMarketAddToCartTelemThrottle", "15" },
            { "housingMarketRemoveFromCartTelemThrottle", "20" },
            { "shop2Enabled", "0" },
            { "disabledGamemodes", "" },
            { "housingMaxPetBedsInterior", "100" },
            { "limitedLuaResourcesAddonCapacityFont", "300" },
            { "housingEnableCreateCharterNeighborhood", "1" },
            { "housingMaxPetBedsExterior", "25" },
            { "housingExteriorTypeByNeighborhoodFactionRestriction", "1" },
            { "shop2ClientRetriesEnabled", "1" },
            { "imageSharingPostUrl", "" },
            { "competitiveModeEnabled", "0" },
            { "shop2HostUrlRequests", "https://us.apidev.blizzard.net" },
            { "discordEnabled", "0" },
            { "shop2PendingOrderPollSeconds", "5" },
            { "housingDecorReportScreenshotFacingDotThreshold", "0.500000" },
            { "imageSharingApiEndpoint", "" },
            { "luaScriptBucketThrottleMaxMsPerSecondRestricted", "500" },
            { "limitedLuaResourcesGlobalCapacityFrame", "100000" },
            { "battleNetTitleFriendCustomNamesEnabledClient", "0" },
            { "imageSharingPostSuffix", "" },
            { "socialUIFriendsListEnabledClient", "1" },
            { "limitedLuaResourcesAddonCapacityTexture", "40000" },
            { "addonChatRestrictionsEnabledForOutgoingAddonMessages", "1" },
            { "shop2SSOPerMilliseconds", "1000" },
            { "shop2SFMPerMilliseconds", "1000" },
            { "housingEnableCreateGuildNeighborhood", "1" },
            { "browserEnabled", "1" },
            { "imageSharingOAuthUrl", "" },
            { "socialUISocialQueueEnabledClient", "1" },
            { "recentAlliesEnabledClient", "1" },
            { "shop2SFMMaxTries", "15" },
            { "housingMarketShopEnabled", "1" },
            { "shop2PendingOrderPollFileEnabled", "1" },
            { "shop2OrderStatusPerMilliseconds", "1000" },
            { "housingBasicDecor_MaxPreviewLimit", "100" },
            { "housingMarketCartFullRemoveEnabled", "1" },
            { "shop2AdditionalScopesStr", "commerce.catalog.availability:override" },
            { "housingExpertDecor_Scale_Indoor_Max", "2.000000" },
            { "limitedLuaResourcesGlobalCapacityTexture", "400000" },
            { "masterLooterEnabled", "1" },
        };

        const std::vector<MirrorVar> emptyMirrorVars;
    }

    const std::vector<MirrorVar>& getMirrorVars(const WoW::ClientProtocol& protocol)
    {
        if (protocol.isForever())
            return camelotMirrorVars;

        switch (protocol.getExpansion())
        {
            // Add expansion-specific MirrorVars here as additional modern
            // client protocols are implemented, for example Expansion::MN.
            default:
                return emptyMirrorVars;
        }
    }
}
