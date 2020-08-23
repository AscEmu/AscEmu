/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../world/Server/WorldSession.h"

void WorldSession::loadSpecificHandlers()
{
    WorldPacketHandlers[CMSG_OBJECT_UPDATE_FAILED].handler = &WorldSession::handleObjectUpdateFailedOpcode;
    WorldPacketHandlers[CMSG_OBJECT_UPDATE_FAILED].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_LOAD_SCREEN].handler = &WorldSession::handleLoadScreenOpcode;
    WorldPacketHandlers[CMSG_LOAD_SCREEN].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::handleReadyForAccountDataTimes;
    WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_UI_TIME_REQUEST].handler = &WorldSession::handleUITimeRequestOpcode;
    WorldPacketHandlers[CMSG_UI_TIME_REQUEST].status = STATUS_AUTHED;

    WorldPacketHandlers[CMSG_TIME_SYNC_RESP].handler = &WorldSession::handleTimeSyncRespOpcode;
    WorldPacketHandlers[CMSG_TIME_SYNC_RESP].status = STATUS_AUTHED;

    // Taxi / NPC Interaction
    WorldPacketHandlers[CMSG_ENABLETAXI].handler = &WorldSession::handleEnabletaxiOpcode;
    WorldPacketHandlers[CMSG_TAXINODE_STATUS_QUERY].handler = &WorldSession::handleTaxiNodeStatusQueryOpcode;
    WorldPacketHandlers[CMSG_TAXIQUERYAVAILABLENODES].handler = &WorldSession::handleTaxiQueryAvaibleNodesOpcode;
    WorldPacketHandlers[CMSG_ACTIVATETAXI].handler = &WorldSession::handleActivateTaxiOpcode;
    WorldPacketHandlers[MSG_TABARDVENDOR_ACTIVATE].handler = &WorldSession::handleTabardVendorActivateOpcode;
    WorldPacketHandlers[CMSG_BANKER_ACTIVATE].handler = &WorldSession::handleBankerActivateOpcode;
    WorldPacketHandlers[CMSG_BUY_BANK_SLOT].handler = &WorldSession::handleBuyBankSlotOpcode;
    WorldPacketHandlers[CMSG_TRAINER_LIST].handler = &WorldSession::handleTrainerListOpcode;
    WorldPacketHandlers[CMSG_TRAINER_BUY_SPELL].handler = &WorldSession::handleTrainerBuySpellOpcode;
    WorldPacketHandlers[CMSG_PETITION_SHOWLIST].handler = &WorldSession::handleCharterShowListOpcode;
    WorldPacketHandlers[MSG_AUCTION_HELLO].handler = &WorldSession::handleAuctionHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_HELLO].handler = &WorldSession::handleGossipHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_SELECT_OPTION].handler = &WorldSession::handleGossipSelectOptionOpcode;
    WorldPacketHandlers[CMSG_SPIRIT_HEALER_ACTIVATE].handler = &WorldSession::handleSpiritHealerActivateOpcode;
    WorldPacketHandlers[CMSG_NPC_TEXT_QUERY].handler = &WorldSession::handleNpcTextQueryOpcode;
    WorldPacketHandlers[CMSG_BINDER_ACTIVATE].handler = &WorldSession::handleBinderActivateOpcode;
    WorldPacketHandlers[CMSG_ACTIVATETAXIEXPRESS].handler = &WorldSession::handleMultipleActivateTaxiOpcode;

    // Item / Vendors
    WorldPacketHandlers[CMSG_SWAP_INV_ITEM].handler = &WorldSession::handleSwapInvItemOpcode;
    WorldPacketHandlers[CMSG_SWAP_ITEM].handler = &WorldSession::handleSwapItemOpcode;
    WorldPacketHandlers[CMSG_DESTROYITEM].handler = &WorldSession::handleDestroyItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM].handler = &WorldSession::handleAutoEquipItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM_SLOT].handler = &WorldSession::handleAutoEquipItemSlotOpcode;
    //WorldPacketHandlers[CMSG_ITEM_QUERY_SINGLE].handler = &WorldSession::handleItemQuerySingleOpcode;
    WorldPacketHandlers[CMSG_SELL_ITEM].handler = &WorldSession::handleSellItemOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM_IN_SLOT].handler = &WorldSession::handleBuyItemInSlotOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM].handler = &WorldSession::handleBuyItemOpcode;
    WorldPacketHandlers[CMSG_LIST_INVENTORY].handler = &WorldSession::handleListInventoryOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BAG_ITEM].handler = &WorldSession::handleAutoStoreBagItemOpcode;
    //WorldPacketHandlers[CMSG_SET_AMMO].handler = &WorldSession::handleAmmoSetOpcode;
    WorldPacketHandlers[CMSG_BUYBACK_ITEM].handler = &WorldSession::handleBuyBackOpcode;
    WorldPacketHandlers[CMSG_SPLIT_ITEM].handler = &WorldSession::handleSplitOpcode;
    //WorldPacketHandlers[CMSG_READ_ITEM].handler = &WorldSession::handleReadItemOpcode;
    WorldPacketHandlers[CMSG_REPAIR_ITEM].handler = &WorldSession::handleRepairItemOpcode;
    WorldPacketHandlers[CMSG_AUTOBANK_ITEM].handler = &WorldSession::handleAutoBankItemOpcode;
    WorldPacketHandlers[CMSG_AUTOSTORE_BANK_ITEM].handler = &WorldSession::handleAutoStoreBankItemOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_TEMP_ENCHANTMENT].handler = &WorldSession::handleCancelTemporaryEnchantmentOpcode;
    WorldPacketHandlers[CMSG_SOCKET_GEMS].handler = &WorldSession::handleInsertGemOpcode;
    //WorldPacketHandlers[CMSG_WRAP_ITEM].handler = &WorldSession::handleWrapItemOpcode;
    WorldPacketHandlers[CMSG_ITEMREFUNDINFO].handler = &WorldSession::handleItemRefundInfoOpcode;
    //WorldPacketHandlers[CMSG_ITEMREFUNDREQUEST].handler = &WorldSession::handleItemRefundRequestOpcode;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_SAVE].handler = &WorldSession::handleEquipmentSetSave;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_USE].handler = &WorldSession::handleEquipmentSetUse;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_DELETE].handler = &WorldSession::handleEquipmentSetDelete;

    // Spell System
    WorldPacketHandlers[CMSG_USE_ITEM].handler = &WorldSession::handleUseItemOpcode;
    WorldPacketHandlers[CMSG_CAST_SPELL].handler = &WorldSession::handleCastSpellOpcode;
    WorldPacketHandlers[CMSG_SPELLCLICK].handler = &WorldSession::handleSpellClick;
    WorldPacketHandlers[CMSG_CANCEL_CAST].handler = &WorldSession::handleCancelCastOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AURA].handler = &WorldSession::handleCancelAuraOpcode;
    WorldPacketHandlers[CMSG_CANCEL_CHANNELLING].handler = &WorldSession::handleCancelChannellingOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AUTO_REPEAT_SPELL].handler = &WorldSession::handleCancelAutoRepeatSpellOpcode;
    WorldPacketHandlers[CMSG_TOTEM_DESTROYED].handler = &WorldSession::handleCancelTotem;
    WorldPacketHandlers[CMSG_UPDATE_PROJECTILE_POSITION].handler = &WorldSession::handleUpdateProjectilePosition;
    
    // Talent System
    WorldPacketHandlers[CMSG_LEARN_TALENT].handler = &WorldSession::handleLearnTalentOpcode;
    WorldPacketHandlers[CMSG_LEARN_PREVIEW_TALENTS].handler = &WorldSession::handleLearnPreviewTalentsOpcode;
    //WorldPacketHandlers[CMSG_LEARN_TALENTS_MULTIPLE].handler = &WorldSession::handleLearnMultipleTalentsOpcode;
    //WorldPacketHandlers[CMSG_UNLEARN_TALENTS].handler = &WorldSession::handleUnlearnTalents;
    WorldPacketHandlers[MSG_TALENT_WIPE_CONFIRM].handler = &WorldSession::handleUnlearnTalents;

    // Combat / Duel
    WorldPacketHandlers[CMSG_ATTACKSWING].handler = &WorldSession::handleAttackSwingOpcode;
    WorldPacketHandlers[CMSG_ATTACKSTOP].handler = &WorldSession::handleAttackStopOpcode;
    WorldPacketHandlers[CMSG_DUEL_ACCEPTED].handler = &WorldSession::handleDuelAccepted;
    WorldPacketHandlers[CMSG_DUEL_CANCELLED].handler = &WorldSession::handleDuelCancelled;

    // Trade
    WorldPacketHandlers[CMSG_INITIATE_TRADE].handler = &WorldSession::handleInitiateTradeOpcode;
    WorldPacketHandlers[CMSG_BEGIN_TRADE].handler = &WorldSession::handleBeginTradeOpcode;
    //WorldPacketHandlers[CMSG_BUSY_TRADE].handler = &WorldSession::handleBusyTrade;
    //WorldPacketHandlers[CMSG_IGNORE_TRADE].handler = &WorldSession::handleIgnoreTrade;
    WorldPacketHandlers[CMSG_ACCEPT_TRADE].handler = &WorldSession::handleAcceptTrade;
    //WorldPacketHandlers[CMSG_UNACCEPT_TRADE].handler = &WorldSession::handleUnacceptTrade;
    WorldPacketHandlers[CMSG_CANCEL_TRADE].handler = &WorldSession::handleCancelTrade;
    WorldPacketHandlers[CMSG_SET_TRADE_ITEM].handler = &WorldSession::handleSetTradeItem;
    WorldPacketHandlers[CMSG_CLEAR_TRADE_ITEM].handler = &WorldSession::handleClearTradeItem;
    WorldPacketHandlers[CMSG_SET_TRADE_GOLD].handler = &WorldSession::handleSetTradeGold;
    // Quest System
    WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
    WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_QUERY].handler = &WorldSession::handleQuestgiverStatusQueryOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_HELLO].handler = &WorldSession::handleQuestgiverHelloOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_ACCEPT_QUEST].handler = &WorldSession::handleQuestgiverAcceptQuestOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_CANCEL].handler = &WorldSession::handleQuestgiverCancelOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_CHOOSE_REWARD].handler = &WorldSession::handleQuestgiverChooseRewardOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_REQUEST_REWARD].handler = &WorldSession::handleQuestgiverRequestRewardOpcode;
    WorldPacketHandlers[CMSG_QUEST_QUERY].handler = &WorldSession::handleQuestQueryOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_QUERY_QUEST].handler = &WorldSession::handleQuestGiverQueryQuestOpcode;
    WorldPacketHandlers[CMSG_QUESTGIVER_COMPLETE_QUEST].handler = &WorldSession::handleQuestgiverCompleteQuestOpcode;
    WorldPacketHandlers[CMSG_QUESTLOG_REMOVE_QUEST].handler = &WorldSession::handleQuestlogRemoveQuestOpcode;
    WorldPacketHandlers[CMSG_RECLAIM_CORPSE].handler = &WorldSession::handleCorpseReclaimOpcode;
    //WorldPacketHandlers[CMSG_RESURRECT_RESPONSE].handler = &WorldSession::HandleResurrectResponseOpcode;
    WorldPacketHandlers[CMSG_PUSHQUESTTOPARTY].handler = &WorldSession::handlePushQuestToPartyOpcode;
    //WorldPacketHandlers[MSG_QUEST_PUSH_RESULT].handler = &WorldSession::handleQuestPushResultOpcode;
    WorldPacketHandlers[CMSG_QUEST_POI_QUERY].handler = &WorldSession::handleQuestPOIQueryOpcode;

    // Battlegrounds
    //WorldPacketHandlers[CMSG_BATTLEFIELD_PORT].handler = &WorldSession::HandleBattlefieldPortOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_STATUS].handler = &WorldSession::handleBattlefieldStatusOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_LIST].handler = &WorldSession::handleBattlefieldListOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_HELLO].handler = &WorldSession::handleBattleMasterHelloOpcode;
    //WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN_ARENA].handler = &WorldSession::handleArenaJoinOpcode;
    WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN].handler = &WorldSession::handleBattleMasterJoinOpcode;
    //WorldPacketHandlers[CMSG_LEAVE_BATTLEFIELD].handler = &WorldSession::HandleLeaveBattlefieldOpcode;
    WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUERY].handler = &WorldSession::handleAreaSpiritHealerQueryOpcode;
    WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUEUE].handler = &WorldSession::handleAreaSpiritHealerQueueOpcode;
    //WorldPacketHandlers[MSG_BATTLEGROUND_PLAYER_POSITIONS].handler = &WorldSession::HandleBattlegroundPlayerPositionsOpcode;
    //WorldPacketHandlers[MSG_PVP_LOG_DATA].handler = &WorldSession::handlePVPLogDataOpcode;
    WorldPacketHandlers[MSG_INSPECT_HONOR_STATS].handler = &WorldSession::handleInspectHonorStatsOpcode;
    WorldPacketHandlers[CMSG_SET_ACTIONBAR_TOGGLES].handler = &WorldSession::handleSetActionBarTogglesOpcode;
    //WorldPacketHandlers[CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE].handler = &WorldSession::HandleBgInviteResponse;
    WorldPacketHandlers[CMSG_REQUEST_RATED_BG_INFO].handler = &WorldSession::handleRequestRatedBgInfoOpcode;
    WorldPacketHandlers[CMSG_REQUEST_RATED_BG_STATS].handler = &WorldSession::handleRequestRatedBgStatsOpcode;
    WorldPacketHandlers[CMSG_REQUEST_PVP_REWARDS].handler = &WorldSession::handleRequestPvPRewardsOpcode;
    WorldPacketHandlers[CMSG_REQUEST_PVP_OPTIONS_ENABLED].handler = &WorldSession::handleRequestPvpOptionsOpcode;

    // Meeting Stone / Instances
    //WorldPacketHandlers[CMSG_SUMMON_RESPONSE].handler = &WorldSession::handleSummonResponseOpcode;
    WorldPacketHandlers[CMSG_RESET_INSTANCES].handler = &WorldSession::handleResetInstanceOpcode;
    //WorldPacketHandlers[CMSG_SELF_RES].handler = &WorldSession::HandleSelfResurrectOpcode;
    //WorldPacketHandlers[MSG_RANDOM_ROLL].handler = &WorldSession::HandleRandomRollOpcode;
    WorldPacketHandlers[MSG_SET_DUNGEON_DIFFICULTY].handler = &WorldSession::handleDungeonDifficultyOpcode;
    WorldPacketHandlers[MSG_SET_RAID_DIFFICULTY].handler = &WorldSession::handleRaidDifficultyOpcode;

    // Misc
    WorldPacketHandlers[CMSG_OPEN_ITEM].handler = &WorldSession::handleOpenItemOpcode;
    WorldPacketHandlers[CMSG_COMPLETE_CINEMATIC].handler = &WorldSession::handleCompleteCinematic;
    WorldPacketHandlers[CMSG_NEXT_CINEMATIC_CAMERA].handler = &WorldSession::handleNextCinematic;
    //WorldPacketHandlers[CMSG_MOUNTSPECIAL_ANIM].handler = &WorldSession::handleMountSpecialAnimOpcode;
    //WorldPacketHandlers[CMSG_TOGGLE_CLOAK].handler = &WorldSession::HandleToggleCloakOpcode;
    //WorldPacketHandlers[CMSG_TOGGLE_HELM].handler = &WorldSession::HandleToggleHelmOpcode;
    //WorldPacketHandlers[CMSG_SET_TITLE].handler = &WorldSession::HandleSetVisibleRankOpcode;
    WorldPacketHandlers[CMSG_GAMEOBJ_REPORT_USE].handler = &WorldSession::handleGameobjReportUseOpCode;
    WorldPacketHandlers[CMSG_PET_CAST_SPELL].handler = &WorldSession::handlePetCastSpell;
    WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].handler = &WorldSession::handleWorldStateUITimerUpdate;
    WorldPacketHandlers[CMSG_SET_TAXI_BENCHMARK_MODE].handler = &WorldSession::handleSetTaxiBenchmarkOpcode;
    WorldPacketHandlers[CMSG_UNLEARN_SKILL].handler = &WorldSession::handleUnlearnSkillOpcode;
    WorldPacketHandlers[CMSG_REQUEST_CEMETERY_LIST].handler = &WorldSession::handleRequestCemeteryListOpcode;
    WorldPacketHandlers[CMSG_REQUEST_HOTFIX].handler = &WorldSession::handleRequestHotfix;
    WorldPacketHandlers[CMSG_RETURN_TO_GRAVEYARD].handler = &WorldSession::handleReturnToGraveyardOpcode;
    WorldPacketHandlers[CMSG_BUG].handler = &WorldSession::handleBugOpcode;
    WorldPacketHandlers[CMSG_SUGGESTION].handler = &WorldSession::handleSuggestionOpcode;
    WorldPacketHandlers[CMSG_LOG_DISCONNECT].handler = &WorldSession::handleLogDisconnectOpcode;

    // Chat
    WorldPacketHandlers[CMSG_CHAT_IGNORED].handler = &WorldSession::handleChatIgnoredOpcode;
    WorldPacketHandlers[CMSG_SET_CHANNEL_WATCH].handler = &WorldSession::handleChatChannelWatchOpcode;

    // cheat/gm commands?
    //WorldPacketHandlers[CMSG_WORLD_TELEPORT].handler = &WorldSession::handleWorldTeleportOpcode;

    // Unsorted
    //WorldPacketHandlers[CMSG_TIME_SYNC_RESP].handler = &WorldSession::HandleTimeSyncResOp;       //MiscHandler.cpp
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::HandleReadyForAccountDataTimes;
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_OPT_OUT_OF_LOOT].handler = &WorldSession::HandleSetAutoLootPassOpcode;
    //WorldPacketHandlers[CMSG_QUESTGIVER_STATUS_MULTIPLE_QUERY].handler = &WorldSession::handleInrangeQuestgiverQuery;
    //WorldPacketHandlers[CMSG_REMOVE_GLYPH].handler = &WorldSession::handleRemoveGlyph;
    WorldPacketHandlers[CMSG_ALTER_APPEARANCE].handler = &WorldSession::handleBarberShopResult;
    //WorldPacketHandlers[CMSG_GET_MIRRORIMAGE_DATA].handler = &WorldSession::HandleMirrorImageOpcode;

    //Misc - wanted to be unhandled
    WorldPacketHandlers[CMSG_FAR_SIGHT].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_LFG_GET_STATUS].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_VOICE_SESSION_ENABLE].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_PET_LEVEL_CHEAT].handler = &WorldSession::Unhandled;
    WorldPacketHandlers[CMSG_QUERY_BATTLEFIELD_STATE].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_SET_ACTIVE_VOICE_CHANNEL].handler = &WorldSession::Unhandled;

    WorldPacketHandlers[CMSG_ADDON_REGISTERED_PREFIXES].handler = &WorldSession::handleAddonRegisteredPrefixesOpcode;
    WorldPacketHandlers[CMSG_UNREGISTER_ALL_ADDON_PREFIXES].handler = &WorldSession::handleUnregisterAddonPrefixesOpcode;
}