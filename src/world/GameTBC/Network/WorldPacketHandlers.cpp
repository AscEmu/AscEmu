/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "../world/Server/WorldSession.h"

void WorldSession::loadSpecificHandlers()
{
    // Faction / Reputation
    //WorldPacketHandlers[CMSG_SET_FACTION_ATWAR].handler = &WorldSession::HandleSetAtWarOpcode;
    //WorldPacketHandlers[CMSG_SET_WATCHED_FACTION].handler = &WorldSession::HandleSetWatchedFactionIndexOpcode;
    //WorldPacketHandlers[CMSG_SET_FACTION_INACTIVE].handler = &WorldSession::HandleSetFactionInactiveOpcode;

    // Channels
    WorldPacketHandlers[CMSG_JOIN_CHANNEL].handler = &WorldSession::handleChannelJoin;
    //WorldPacketHandlers[CMSG_LEAVE_CHANNEL].handler = &WorldSession::HandleChannelLeave;
    //WorldPacketHandlers[CMSG_CHANNEL_LIST].handler = &WorldSession::HandleChannelList;
    //WorldPacketHandlers[CMSG_CHANNEL_PASSWORD].handler = &WorldSession::HandleChannelPassword;
    //WorldPacketHandlers[CMSG_CHANNEL_SET_OWNER].handler = &WorldSession::HandleChannelSetOwner;
    //WorldPacketHandlers[CMSG_CHANNEL_OWNER].handler = &WorldSession::HandleChannelOwner;
    //WorldPacketHandlers[CMSG_CHANNEL_MODERATOR].handler = &WorldSession::HandleChannelModerator;
    //WorldPacketHandlers[CMSG_CHANNEL_UNMODERATOR].handler = &WorldSession::HandleChannelUnmoderator;
    //WorldPacketHandlers[CMSG_CHANNEL_MUTE].handler = &WorldSession::HandleChannelMute;
    //WorldPacketHandlers[CMSG_CHANNEL_UNMUTE].handler = &WorldSession::HandleChannelUnmute;
    //WorldPacketHandlers[CMSG_CHANNEL_INVITE].handler = &WorldSession::HandleChannelInvite;
    //WorldPacketHandlers[CMSG_CHANNEL_KICK].handler = &WorldSession::HandleChannelKick;
    //WorldPacketHandlers[CMSG_CHANNEL_BAN].handler = &WorldSession::HandleChannelBan;
    //WorldPacketHandlers[CMSG_CHANNEL_UNBAN].handler = &WorldSession::HandleChannelUnban;
    //WorldPacketHandlers[CMSG_CHANNEL_ANNOUNCEMENTS].handler = &WorldSession::HandleChannelAnnounce;
    //WorldPacketHandlers[CMSG_CHANNEL_MODERATE].handler = &WorldSession::HandleChannelModerate;
    WorldPacketHandlers[CMSG_GET_CHANNEL_MEMBER_COUNT].handler = &WorldSession::handleGetChannelMemberCount;
    //WorldPacketHandlers[CMSG_CHANNEL_DISPLAY_LIST].handler = &WorldSession::HandleChannelRosterQuery;

    // Groups / Raids
    WorldPacketHandlers[CMSG_GROUP_INVITE].handler = &WorldSession::handleGroupInviteOpcode;
    //WorldPacketHandlers[CMSG_GROUP_CANCEL].handler = &WorldSession::handleGroupCancelOpcode;
    WorldPacketHandlers[CMSG_GROUP_ACCEPT].handler = &WorldSession::handleGroupAcceptOpcode;
    WorldPacketHandlers[CMSG_GROUP_DECLINE].handler = &WorldSession::handleGroupDeclineOpcode;
    //WorldPacketHandlers[CMSG_GROUP_UNINVITE].handler = &WorldSession::HandleGroupUninviteOpcode;
    //WorldPacketHandlers[CMSG_GROUP_UNINVITE_GUID].handler = &WorldSession::HandleGroupUninviteGuidOpcode;
    //WorldPacketHandlers[CMSG_GROUP_SET_LEADER].handler = &WorldSession::HandleGroupSetLeaderOpcode;
    //WorldPacketHandlers[CMSG_GROUP_DISBAND].handler = &WorldSession::HandleGroupDisbandOpcode;
    //WorldPacketHandlers[CMSG_LOOT_METHOD].handler = &WorldSession::HandleLootMethodOpcode;
    //WorldPacketHandlers[MSG_MINIMAP_PING].handler = &WorldSession::HandleMinimapPingOpcode;
    //WorldPacketHandlers[CMSG_GROUP_RAID_CONVERT].handler = &WorldSession::HandleConvertGroupToRaidOpcode;
    //WorldPacketHandlers[CMSG_GROUP_CHANGE_SUB_GROUP].handler = &WorldSession::HandleGroupChangeSubGroup;
    //WorldPacketHandlers[CMSG_GROUP_ASSISTANT_LEADER].handler = &WorldSession::HandleGroupAssistantLeader;
    WorldPacketHandlers[CMSG_REQUEST_RAID_INFO].handler = &WorldSession::handleRequestRaidInfoOpcode;
    //WorldPacketHandlers[MSG_RAID_READY_CHECK].handler = &WorldSession::HandleReadyCheckOpcode;
    //WorldPacketHandlers[MSG_RAID_TARGET_UPDATE].handler = &WorldSession::HandleSetPlayerIconOpcode;
    //WorldPacketHandlers[CMSG_REQUEST_PARTY_MEMBER_STATS].handler = &WorldSession::HandlePartyMemberStatsOpcode;
    //WorldPacketHandlers[MSG_PARTY_ASSIGNMENT].handler = &WorldSession::HandleGroupPromote;

    // LFG System
    //WorldPacketHandlers[CMSG_SET_LFG_COMMENT].handler = &WorldSession::handleLfgSetCommentOpcode;
    //WorldPacketHandlers[CMSG_LFG_JOIN].handler = &WorldSession::handleLfgJoinOpcode;
    //WorldPacketHandlers[CMSG_LFG_LEAVE].handler = &WorldSession::handleLfgLeaveOpcode;
    //WorldPacketHandlers[CMSG_SEARCH_LFG_JOIN].handler = &WorldSession::handleLfgSearchOpcode;
    //WorldPacketHandlers[CMSG_SEARCH_LFG_LEAVE].handler = &WorldSession::handleLfgSearchLeaveOpcode;
    //WorldPacketHandlers[CMSG_LFG_PROPOSAL_RESULT].handler = &WorldSession::handleLfgProposalResultOpcode;
    //WorldPacketHandlers[CMSG_LFG_SET_ROLES].handler = &WorldSession::handleLfgSetRolesOpcode;
    //WorldPacketHandlers[CMSG_LFG_SET_BOOT_VOTE].handler = &WorldSession::handleLfgSetBootVoteOpcode;
    //WorldPacketHandlers[CMSG_LFD_PLAYER_LOCK_INFO_REQUEST].handler = &WorldSession::handleLfgPlayerLockInfoRequestOpcode;
    //WorldPacketHandlers[CMSG_LFG_TELEPORT].handler = &WorldSession::handleLfgTeleportOpcode;
    //WorldPacketHandlers[CMSG_LFD_PARTY_LOCK_INFO_REQUEST].handler = &WorldSession::handleLfgPartyLockInfoRequestOpcode;

    // Taxi / NPC Interaction
    //WorldPacketHandlers[CMSG_ENABLETAXI].handler = &WorldSession::handleEnabletaxiOpcode;
    //WorldPacketHandlers[CMSG_TAXINODE_STATUS_QUERY].handler = &WorldSession::handleTaxiNodeStatusQueryOpcode;
    //WorldPacketHandlers[CMSG_TAXIQUERYAVAILABLENODES].handler = &WorldSession::handleTaxiQueryAvaibleNodesOpcode;
    //WorldPacketHandlers[CMSG_ACTIVATETAXI].handler = &WorldSession::handleActivateTaxiOpcode;
    //WorldPacketHandlers[MSG_TABARDVENDOR_ACTIVATE].handler = &WorldSession::HandleTabardVendorActivateOpcode;
    //WorldPacketHandlers[CMSG_BANKER_ACTIVATE].handler = &WorldSession::HandleBankerActivateOpcode;
    //WorldPacketHandlers[CMSG_BUY_BANK_SLOT].handler = &WorldSession::HandleBuyBankSlotOpcode;
    //WorldPacketHandlers[CMSG_TRAINER_LIST].handler = &WorldSession::HandleTrainerListOpcode;
    //WorldPacketHandlers[CMSG_TRAINER_BUY_SPELL].handler = &WorldSession::HandleTrainerBuySpellOpcode;
    //WorldPacketHandlers[CMSG_PETITION_SHOWLIST].handler = &WorldSession::HandleCharterShowListOpcode;
    //WorldPacketHandlers[MSG_AUCTION_HELLO].handler = &WorldSession::HandleAuctionHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_HELLO].handler = &WorldSession::handleGossipHelloOpcode;
    WorldPacketHandlers[CMSG_GOSSIP_SELECT_OPTION].handler = &WorldSession::handleGossipSelectOptionOpcode;
    WorldPacketHandlers[CMSG_SPIRIT_HEALER_ACTIVATE].handler = &WorldSession::handleSpiritHealerActivateOpcode;
    WorldPacketHandlers[CMSG_NPC_TEXT_QUERY].handler = &WorldSession::handleNpcTextQueryOpcode;
    //WorldPacketHandlers[CMSG_BINDER_ACTIVATE].handler = &WorldSession::HandleBinderActivateOpcode;
    //WorldPacketHandlers[CMSG_ACTIVATETAXIEXPRESS].handler = &WorldSession::handleMultipleActivateTaxiOpcode;
    // Item / Vendors
    WorldPacketHandlers[CMSG_SWAP_INV_ITEM].handler = &WorldSession::handleSwapInvItemOpcode;
    WorldPacketHandlers[CMSG_SWAP_ITEM].handler = &WorldSession::handleSwapItemOpcode;
    WorldPacketHandlers[CMSG_DESTROYITEM].handler = &WorldSession::handleDestroyItemOpcode;
    WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM].handler = &WorldSession::handleAutoEquipItemOpcode;
    //WorldPacketHandlers[CMSG_AUTOEQUIP_ITEM_SLOT].handler = &WorldSession::handleAutoEquipItemSlotOpcode;
    WorldPacketHandlers[CMSG_ITEM_QUERY_SINGLE].handler = &WorldSession::handleItemQuerySingleOpcode;
    WorldPacketHandlers[CMSG_SELL_ITEM].handler = &WorldSession::handleSellItemOpcode;
    //WorldPacketHandlers[CMSG_BUY_ITEM_IN_SLOT].handler = &WorldSession::handleBuyItemInSlotOpcode;
    WorldPacketHandlers[CMSG_BUY_ITEM].handler = &WorldSession::handleBuyItemOpcode;
    WorldPacketHandlers[CMSG_LIST_INVENTORY].handler = &WorldSession::handleListInventoryOpcode;
    //WorldPacketHandlers[CMSG_AUTOSTORE_BAG_ITEM].handler = &WorldSession::handleAutoStoreBagItemOpcode;
    //WorldPacketHandlers[CMSG_SET_AMMO].handler = &WorldSession::handleAmmoSetOpcode;
    //WorldPacketHandlers[CMSG_BUYBACK_ITEM].handler = &WorldSession::handleBuyBackOpcode;
    //WorldPacketHandlers[CMSG_SPLIT_ITEM].handler = &WorldSession::handleSplitOpcode;
    //WorldPacketHandlers[CMSG_READ_ITEM].handler = &WorldSession::handleReadItemOpcode;
    //WorldPacketHandlers[CMSG_REPAIR_ITEM].handler = &WorldSession::handleRepairItemOpcode;
    //WorldPacketHandlers[CMSG_AUTOBANK_ITEM].handler = &WorldSession::handleAutoBankItemOpcode;
    //WorldPacketHandlers[CMSG_AUTOSTORE_BANK_ITEM].handler = &WorldSession::handleAutoStoreBankItemOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_TEMP_ENCHANTMENT].handler = &WorldSession::handleCancelTemporaryEnchantmentOpcode;
    //WorldPacketHandlers[CMSG_SOCKET_GEMS].handler = &WorldSession::handleInsertGemOpcode;
    //WorldPacketHandlers[CMSG_WRAP_ITEM].handler = &WorldSession::handleWrapItemOpcode;
    //WorldPacketHandlers[CMSG_ITEMREFUNDINFO].handler = &WorldSession::handleItemRefundInfoOpcode;
    //WorldPacketHandlers[CMSG_ITEMREFUNDREQUEST].handler = &WorldSession::HandleItemRefundRequestOpcode;

    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_SAVE].handler = &WorldSession::handleEquipmentSetSave;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_USE].handler = &WorldSession::handleEquipmentSetUse;
    //WorldPacketHandlers[CMSG_EQUIPMENT_SET_DELETE].handler = &WorldSession::handleEquipmentSetDelete;

    // Spell System / Talent System
    WorldPacketHandlers[CMSG_USE_ITEM].handler = &WorldSession::handleUseItemOpcode;
    WorldPacketHandlers[CMSG_CAST_SPELL].handler = &WorldSession::handleCastSpellOpcode;
    //WorldPacketHandlers[CMSG_SPELLCLICK].handler = &WorldSession::handleSpellClick;
    WorldPacketHandlers[CMSG_CANCEL_CAST].handler = &WorldSession::handleCancelCastOpcode;
    WorldPacketHandlers[CMSG_CANCEL_AURA].handler = &WorldSession::handleCancelAuraOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_CHANNELLING].handler = &WorldSession::handleCancelChannellingOpcode;
    //WorldPacketHandlers[CMSG_CANCEL_AUTO_REPEAT_SPELL].handler = &WorldSession::handleCancelAutoRepeatSpellOpcode;
    //WorldPacketHandlers[CMSG_TOTEM_DESTROYED].handler = &WorldSession::handleCancelTotem;
    //WorldPacketHandlers[CMSG_LEARN_TALENT].handler = &WorldSession::handleLearnTalentOpcode;
    //WorldPacketHandlers[CMSG_LEARN_TALENTS_MULTIPLE].handler = &WorldSession::handleLearnMultipleTalentsOpcode;
    //WorldPacketHandlers[CMSG_UNLEARN_TALENTS].handler = &WorldSession::handleUnlearnTalents;
    //WorldPacketHandlers[MSG_TALENT_WIPE_CONFIRM].handler = &WorldSession::handleUnlearnTalents;
    //WorldPacketHandlers[CMSG_UPDATE_PROJECTILE_POSITION].handler = &WorldSession::handleUpdateProjectilePosition;

    // Combat / Duel
    WorldPacketHandlers[CMSG_ATTACKSWING].handler = &WorldSession::handleAttackSwingOpcode;
    WorldPacketHandlers[CMSG_ATTACKSTOP].handler = &WorldSession::handleAttackStopOpcode;
    //WorldPacketHandlers[CMSG_DUEL_ACCEPTED].handler = &WorldSession::HandleDuelAccepted;
    //WorldPacketHandlers[CMSG_DUEL_CANCELLED].handler = &WorldSession::HandleDuelCancelled;

    // Trade
    //WorldPacketHandlers[CMSG_INITIATE_TRADE].handler = &WorldSession::handleInitiateTradeOpcode;
    //WorldPacketHandlers[CMSG_BEGIN_TRADE].handler = &WorldSession::handleBeginTradeOpcode;
    //WorldPacketHandlers[CMSG_BUSY_TRADE].handler = &WorldSession::handleBusyTrade;
    //WorldPacketHandlers[CMSG_IGNORE_TRADE].handler = &WorldSession::handleIgnoreTrade;
    //WorldPacketHandlers[CMSG_ACCEPT_TRADE].handler = &WorldSession::handleAcceptTrade;
    //WorldPacketHandlers[CMSG_UNACCEPT_TRADE].handler = &WorldSession::handleUnacceptTrade;
    WorldPacketHandlers[CMSG_CANCEL_TRADE].handler = &WorldSession::handleCancelTrade;
    //WorldPacketHandlers[CMSG_SET_TRADE_ITEM].handler = &WorldSession::handleSetTradeItem;
    //WorldPacketHandlers[CMSG_CLEAR_TRADE_ITEM].handler = &WorldSession::handleClearTradeItem;
    //WorldPacketHandlers[CMSG_SET_TRADE_GOLD].handler = &WorldSession::handleSetTradeGold;

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
    WorldPacketHandlers[CMSG_RESURRECT_RESPONSE].handler = &WorldSession::handleResurrectResponse;
    //WorldPacketHandlers[CMSG_PUSHQUESTTOPARTY].handler = &WorldSession::handlePushQuestToPartyOpcode;
    WorldPacketHandlers[MSG_QUEST_PUSH_RESULT].handler = &WorldSession::handleQuestPushResultOpcode;
    //WorldPacketHandlers[CMSG_QUEST_POI_QUERY].handler = &WorldSession::handleQuestPOIQueryOpcode;

    // Auction System
    //WorldPacketHandlers[CMSG_AUCTION_LIST_ITEMS].handler = &WorldSession::HandleAuctionListItems;
    //WorldPacketHandlers[CMSG_AUCTION_LIST_BIDDER_ITEMS].handler = &WorldSession::HandleAuctionListBidderItems;
    //WorldPacketHandlers[CMSG_AUCTION_SELL_ITEM].handler = &WorldSession::HandleAuctionSellItem;
    //WorldPacketHandlers[CMSG_AUCTION_LIST_OWNER_ITEMS].handler = &WorldSession::HandleAuctionListOwnerItems;
    //WorldPacketHandlers[CMSG_AUCTION_PLACE_BID].handler = &WorldSession::HandleAuctionPlaceBid;
    //WorldPacketHandlers[CMSG_AUCTION_REMOVE_ITEM].handler = &WorldSession::HandleCancelAuction;
    //WorldPacketHandlers[CMSG_AUCTION_LIST_PENDING_SALES].handler = &WorldSession::HandleAuctionListPendingSales;

    // Mail System
    //WorldPacketHandlers[CMSG_GET_MAIL_LIST].handler = &WorldSession::handleGetMailOpcode;
    //WorldPacketHandlers[CMSG_ITEM_TEXT_QUERY].handler = &WorldSession::handleItemTextQueryOpcode;
    //WorldPacketHandlers[CMSG_SEND_MAIL].handler = &WorldSession::handleSendMailOpcode;
    //WorldPacketHandlers[CMSG_MAIL_TAKE_MONEY].handler = &WorldSession::handleTakeMoneyOpcode;
    //WorldPacketHandlers[CMSG_MAIL_TAKE_ITEM].handler = &WorldSession::handleTakeItemOpcode;
    //WorldPacketHandlers[CMSG_MAIL_MARK_AS_READ].handler = &WorldSession::handleMarkAsReadOpcode;
    //WorldPacketHandlers[CMSG_MAIL_RETURN_TO_SENDER].handler = &WorldSession::handleReturnToSenderOpcode;
    //WorldPacketHandlers[CMSG_MAIL_DELETE].handler = &WorldSession::handleMailDeleteOpcode;
    WorldPacketHandlers[MSG_QUERY_NEXT_MAIL_TIME].handler = &WorldSession::handleMailTimeOpcode;
    //WorldPacketHandlers[CMSG_MAIL_CREATE_TEXT_ITEM].handler = &WorldSession::handleMailCreateTextItemOpcode;

    // Guild Query (called when not logged in sometimes)
    //WorldPacketHandlers[CMSG_GUILD_QUERY].handler = &WorldSession::HandleGuildQuery;
    //WorldPacketHandlers[CMSG_GUILD_QUERY].status = STATUS_AUTHED;

    // Guild System
    //WorldPacketHandlers[CMSG_GUILD_CREATE].handler = &WorldSession::HandleCreateGuild;
    //WorldPacketHandlers[CMSG_GUILD_INVITE].handler = &WorldSession::HandleInviteToGuild;
    //WorldPacketHandlers[CMSG_GUILD_ACCEPT].handler = &WorldSession::HandleGuildAccept;
    //WorldPacketHandlers[CMSG_GUILD_DECLINE].handler = &WorldSession::HandleGuildDecline;
    //WorldPacketHandlers[CMSG_GUILD_INFO].handler = &WorldSession::HandleGuildInfo;
    //WorldPacketHandlers[CMSG_GUILD_ROSTER].handler = &WorldSession::HandleGuildRoster;
    //WorldPacketHandlers[CMSG_GUILD_PROMOTE].handler = &WorldSession::HandleGuildPromote;
    //WorldPacketHandlers[CMSG_GUILD_DEMOTE].handler = &WorldSession::HandleGuildDemote;
    //WorldPacketHandlers[CMSG_GUILD_LEAVE].handler = &WorldSession::HandleGuildLeave;
    //WorldPacketHandlers[CMSG_GUILD_REMOVE].handler = &WorldSession::HandleGuildRemove;
    //WorldPacketHandlers[CMSG_GUILD_DISBAND].handler = &WorldSession::HandleGuildDisband;
    //WorldPacketHandlers[CMSG_GUILD_LEADER].handler = &WorldSession::HandleGuildLeader;
    //WorldPacketHandlers[CMSG_GUILD_MOTD].handler = &WorldSession::HandleGuildMotd;
    //WorldPacketHandlers[CMSG_GUILD_RANK].handler = &WorldSession::HandleGuildRank;
    //WorldPacketHandlers[CMSG_GUILD_ADD_RANK].handler = &WorldSession::HandleGuildAddRank;
    //WorldPacketHandlers[CMSG_GUILD_DEL_RANK].handler = &WorldSession::HandleGuildDelRank;
    //WorldPacketHandlers[CMSG_GUILD_SET_PUBLIC_NOTE].handler = &WorldSession::HandleGuildSetPublicNote;
    //WorldPacketHandlers[CMSG_GUILD_SET_OFFICER_NOTE].handler = &WorldSession::HandleGuildSetOfficerNote;
    //WorldPacketHandlers[CMSG_PETITION_BUY].handler = &WorldSession::HandleCharterBuy;
    //WorldPacketHandlers[CMSG_PETITION_SHOW_SIGNATURES].handler = &WorldSession::HandleCharterShowSignatures;
    //WorldPacketHandlers[CMSG_TURN_IN_PETITION].handler = &WorldSession::HandleCharterTurnInCharter;
    //WorldPacketHandlers[CMSG_PETITION_QUERY].handler = &WorldSession::HandleCharterQuery;
    //WorldPacketHandlers[CMSG_OFFER_PETITION].handler = &WorldSession::HandleCharterOffer;
    //WorldPacketHandlers[CMSG_PETITION_SIGN].handler = &WorldSession::HandleCharterSign;
    //WorldPacketHandlers[MSG_PETITION_DECLINE].handler = &WorldSession::HandleCharterDecline;
    //WorldPacketHandlers[MSG_PETITION_RENAME].handler = &WorldSession::HandleCharterRename;
    //WorldPacketHandlers[MSG_SAVE_GUILD_EMBLEM].handler = &WorldSession::HandleSaveGuildEmblem;
    //WorldPacketHandlers[CMSG_GUILD_INFO_TEXT].handler = &WorldSession::HandleSetGuildInformation;
    //WorldPacketHandlers[MSG_QUERY_GUILD_BANK_TEXT].handler = &WorldSession::HandleGuildBankQueryText;
    //WorldPacketHandlers[CMSG_SET_GUILD_BANK_TEXT].handler = &WorldSession::HandleSetGuildBankText;
    //WorldPacketHandlers[MSG_GUILD_EVENT_LOG_QUERY].handler = &WorldSession::HandleGuildLog;
    //WorldPacketHandlers[CMSG_GUILD_BANKER_ACTIVATE].handler = &WorldSession::HandleGuildBankOpenVault;
    //WorldPacketHandlers[CMSG_GUILD_BANK_BUY_TAB].handler = &WorldSession::HandleGuildBankBuyTab;
    WorldPacketHandlers[MSG_GUILD_BANK_MONEY_WITHDRAWN].handler = &WorldSession::handleGuildBankMoneyWithdrawn;
    //WorldPacketHandlers[CMSG_GUILD_BANK_UPDATE_TAB].handler = &WorldSession::HandleGuildBankModifyTab;
    //WorldPacketHandlers[CMSG_GUILD_BANK_SWAP_ITEMS].handler = &WorldSession::HandleGuildBankDepositItem;
    //WorldPacketHandlers[CMSG_GUILD_BANK_WITHDRAW_MONEY].handler = &WorldSession::HandleGuildBankWithdrawMoney;
    //WorldPacketHandlers[CMSG_GUILD_BANK_DEPOSIT_MONEY].handler = &WorldSession::HandleGuildBankDepositMoney;
    //WorldPacketHandlers[CMSG_GUILD_BANK_QUERY_TAB].handler = &WorldSession::HandleGuildBankViewTab;
    //WorldPacketHandlers[MSG_GUILD_BANK_LOG_QUERY].handler = &WorldSession::HandleGuildBankViewLog;
    //WorldPacketHandlers[MSG_GUILD_PERMISSIONS].handler = &WorldSession::HandleGuildGetFullPermissions;

    // Pets
    //WorldPacketHandlers[CMSG_PET_ACTION].handler = &WorldSession::HandlePetAction;
    //WorldPacketHandlers[CMSG_REQUEST_PET_INFO].handler = &WorldSession::HandlePetInfo;
    //WorldPacketHandlers[CMSG_PET_NAME_QUERY].handler = &WorldSession::HandlePetNameQuery;
    //WorldPacketHandlers[CMSG_BUY_STABLE_SLOT].handler = &WorldSession::HandleBuyStableSlot;
    //WorldPacketHandlers[CMSG_STABLE_PET].handler = &WorldSession::HandleStablePet;
    //WorldPacketHandlers[CMSG_UNSTABLE_PET].handler = &WorldSession::HandleUnstablePet;
    //WorldPacketHandlers[CMSG_STABLE_SWAP_PET].handler = &WorldSession::HandleStableSwapPet;
    //WorldPacketHandlers[MSG_LIST_STABLED_PETS].handler = &WorldSession::HandleStabledPetList;
    //WorldPacketHandlers[CMSG_PET_SET_ACTION].handler = &WorldSession::HandlePetSetActionOpcode;
    //WorldPacketHandlers[CMSG_PET_RENAME].handler = &WorldSession::HandlePetRename;
    //WorldPacketHandlers[CMSG_PET_ABANDON].handler = &WorldSession::HandlePetAbandon;
    //WorldPacketHandlers[CMSG_PET_UNLEARN].handler = &WorldSession::HandlePetUnlearn;
    //WorldPacketHandlers[CMSG_PET_SPELL_AUTOCAST].handler = &WorldSession::HandlePetSpellAutocast;
    //WorldPacketHandlers[CMSG_PET_CANCEL_AURA].handler = &WorldSession::HandlePetCancelAura;
    //WorldPacketHandlers[CMSG_PET_LEARN_TALENT].handler = &WorldSession::HandlePetLearnTalent;
    //WorldPacketHandlers[CMSG_DISMISS_CRITTER].handler = &WorldSession::HandleDismissCritter;

    // Battlegrounds
    //WorldPacketHandlers[CMSG_BATTLEFIELD_PORT].handler = &WorldSession::HandleBattlefieldPortOpcode;
    WorldPacketHandlers[CMSG_BATTLEFIELD_STATUS].handler = &WorldSession::handleBattlefieldStatusOpcode;
    //WorldPacketHandlers[CMSG_BATTLEFIELD_LIST].handler = &WorldSession::HandleBattlefieldListOpcode;
    //WorldPacketHandlers[CMSG_BATTLEMASTER_HELLO].handler = &WorldSession::HandleBattleMasterHelloOpcode;
    //WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN_ARENA].handler = &WorldSession::HandleArenaJoinOpcode;
    //WorldPacketHandlers[CMSG_BATTLEMASTER_JOIN].handler = &WorldSession::HandleBattleMasterJoinOpcode;
    //WorldPacketHandlers[CMSG_LEAVE_BATTLEFIELD].handler = &WorldSession::HandleLeaveBattlefieldOpcode;
    //WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUERY].handler = &WorldSession::HandleAreaSpiritHealerQueryOpcode;
    //WorldPacketHandlers[CMSG_AREA_SPIRIT_HEALER_QUEUE].handler = &WorldSession::HandleAreaSpiritHealerQueueOpcode;
    //WorldPacketHandlers[MSG_BATTLEGROUND_PLAYER_POSITIONS].handler = &WorldSession::HandleBattlegroundPlayerPositionsOpcode;
    //WorldPacketHandlers[MSG_PVP_LOG_DATA].handler = &WorldSession::HandlePVPLogDataOpcode;
    //WorldPacketHandlers[MSG_INSPECT_HONOR_STATS].handler = &WorldSession::HandleInspectHonorStatsOpcode;
    //WorldPacketHandlers[CMSG_SET_ACTIONBAR_TOGGLES].handler = &WorldSession::HandleSetActionBarTogglesOpcode;
    //WorldPacketHandlers[CMSG_BATTLEFIELD_MGR_ENTRY_INVITE_RESPONSE].handler = &WorldSession::HandleBgInviteResponse;

    // GM Ticket System
    WorldPacketHandlers[CMSG_GMTICKET_CREATE].handler = &WorldSession::handleGMTicketCreateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_UPDATETEXT].handler = &WorldSession::handleGMTicketUpdateOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_DELETETICKET].handler = &WorldSession::handleGMTicketDeleteOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_GETTICKET].handler = &WorldSession::handleGMTicketGetTicketOpcode;
    WorldPacketHandlers[CMSG_GMTICKET_SYSTEMSTATUS].handler = &WorldSession::handleGMTicketSystemStatusOpcode;
    WorldPacketHandlers[CMSG_GMTICKETSYSTEM_TOGGLE].handler = &WorldSession::handleGMTicketToggleSystemStatusOpcode;

    // Lag report
    //WorldPacketHandlers[CMSG_GM_REPORT_LAG].handler = &WorldSession::handleReportLag;
    //WorldPacketHandlers[CMSG_GM_REPORT_LAG].status = STATUS_LOGGEDIN;
    //WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].handler = &WorldSession::handleGMSurveySubmitOpcode;
    //WorldPacketHandlers[CMSG_GMSURVEY_SUBMIT].status = STATUS_LOGGEDIN;

    // Meeting Stone / Instances
    //WorldPacketHandlers[CMSG_SUMMON_RESPONSE].handler = &WorldSession::handleSummonResponseOpcode;
    //WorldPacketHandlers[CMSG_RESET_INSTANCES].handler = &WorldSession::HandleResetInstanceOpcode;
    //WorldPacketHandlers[CMSG_SELF_RES].handler = &WorldSession::HandleSelfResurrectOpcode;
    //WorldPacketHandlers[MSG_RANDOM_ROLL].handler = &WorldSession::HandleRandomRollOpcode;
    //WorldPacketHandlers[MSG_SET_DUNGEON_DIFFICULTY].handler = &WorldSession::HandleDungeonDifficultyOpcode;
    //WorldPacketHandlers[MSG_SET_RAID_DIFFICULTY].handler = &WorldSession::HandleRaidDifficultyOpcode;

    // Misc
    //WorldPacketHandlers[CMSG_OPEN_ITEM].handler = &WorldSession::HandleOpenItemOpcode;
    //WorldPacketHandlers[CMSG_COMPLETE_CINEMATIC].handler = &WorldSession::handleCompleteCinematic;
    //WorldPacketHandlers[CMSG_NEXT_CINEMATIC_CAMERA].handler = &WorldSession::handleNextCinematic;
    //WorldPacketHandlers[CMSG_MOUNTSPECIAL_ANIM].handler = &WorldSession::handleMountSpecialAnimOpcode;
    //WorldPacketHandlers[CMSG_TOGGLE_CLOAK].handler = &WorldSession::HandleToggleCloakOpcode;
    //WorldPacketHandlers[CMSG_TOGGLE_HELM].handler = &WorldSession::HandleToggleHelmOpcode;
    //WorldPacketHandlers[CMSG_SET_TITLE].handler = &WorldSession::HandleSetVisibleRankOpcode;
    //WorldPacketHandlers[CMSG_COMPLAIN].handler = &WorldSession::HandleReportSpamOpcode;
    //WorldPacketHandlers[CMSG_GAMEOBJ_REPORT_USE].handler = &WorldSession::HandleGameobjReportUseOpCode;
    //WorldPacketHandlers[CMSG_PET_CAST_SPELL].handler = &WorldSession::handlePetCastSpell;
    //WorldPacketHandlers[CMSG_WORLD_STATE_UI_TIMER_UPDATE].handler = &WorldSession::HandleWorldStateUITimerUpdate;
    //WorldPacketHandlers[CMSG_SET_TAXI_BENCHMARK_MODE].handler = &WorldSession::HandleSetTaxiBenchmarkOpcode;
    //WorldPacketHandlers[CMSG_UNLEARN_SKILL].handler = &WorldSession::handleUnlearnSkillOpcode;

    // Chat
    //WorldPacketHandlers[CMSG_CHAT_IGNORED].handler = &WorldSession::HandleChatIgnoredOpcode;
    //WorldPacketHandlers[CMSG_SET_CHANNEL_WATCH].handler = &WorldSession::HandleChatChannelWatchOpcode;

    // cheat/gm commands?
    WorldPacketHandlers[CMSG_WORLD_TELEPORT].handler = &WorldSession::handleWorldTeleportOpcode;

    // Unsorted
    //WorldPacketHandlers[CMSG_TIME_SYNC_RESP].handler = &WorldSession::HandleTimeSyncResp;       //MiscHandler.cpp
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].handler = &WorldSession::HandleReadyForAccountDataTimes;
    //WorldPacketHandlers[CMSG_READY_FOR_ACCOUNT_DATA_TIMES].status = STATUS_AUTHED;

    //WorldPacketHandlers[CMSG_OPT_OUT_OF_LOOT].handler = &WorldSession::HandleSetAutoLootPassOpcode;
    //WorldPacketHandlers[CMSG_REMOVE_GLYPH].handler = &WorldSession::handleRemoveGlyph;
    //WorldPacketHandlers[CMSG_ALTER_APPEARANCE].handler = &WorldSession::handleBarberShopResult;
    //WorldPacketHandlers[CMSG_GET_MIRRORIMAGE_DATA].handler = &WorldSession::HandleMirrorImageOpcode;

    //Misc - Unhandled
    //WorldPacketHandlers[CMSG_FAR_SIGHT].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_LFG_GET_STATUS].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_VOICE_SESSION_ENABLE].handler = &WorldSession::Unhandled;
    //WorldPacketHandlers[CMSG_SET_ACTIVE_VOICE_CHANNEL].handler = &WorldSession::Unhandled;
}
