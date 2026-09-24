/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string_view>

namespace AscEmu::Battlenet::Protocol
{
    namespace WoW
    {
        constexpr uint32_t TitleId = 0x00576F57u; // "WoW"
        constexpr uint8_t EuropeRegion = 2u;
        constexpr uint8_t DefaultBattlegroup = 1u;
    }

    // Battle.net RPC service hashes and method IDs used by the WoW client.
    // Keep wire identifiers here instead of scattering magic numbers through handlers.

    // Canonical Battle.net service hashes known from Blizzard BGS protobuf
    // descriptors. These entries are registry/documentation data only unless a
    // dedicated handler is wired below. Keeping them here lets the dispatcher
    // identify optional social/platform calls without pretending they are
    // implemented.
    namespace AccountServiceV1
    {
        constexpr uint32_t Hash = 0x62DA0891u;
        constexpr uint32_t ResolveAccount = 13u;
        constexpr uint32_t Subscribe = 25u;
        constexpr uint32_t Unsubscribe = 26u;
        constexpr uint32_t GetAccountState = 30u;
        constexpr uint32_t GetGameAccountState = 31u;
        constexpr uint32_t GetLicenses = 32u;
        constexpr uint32_t GetGameTimeRemainingInfo = 33u;
        constexpr uint32_t GetGameSessionInfo = 34u;
        constexpr uint32_t GetCAISInfo = 35u;
        constexpr uint32_t GetAuthorizedData = 37u;
        constexpr uint32_t GetSignedAccountState = 44u;
        constexpr uint32_t GetAccountInfo = 45u;
        constexpr uint32_t GetAccountPlatformRestrictions = 46u;
    }
    namespace AuthenticationServiceV1
    {
        constexpr uint32_t Hash = 0x0DECFC01u;
        constexpr uint32_t Logon = 1u;
        constexpr uint32_t VerifyWebCredentials = 7u;
        constexpr uint32_t GenerateWebCredentials = 8u;
    }
    namespace BlockListServiceV1
    {
        constexpr uint32_t Hash = 0x8E8F5FB0u;
        constexpr uint32_t Subscribe = 1u;
        constexpr uint32_t Unsubscribe = 2u;
        constexpr uint32_t GetState = 3u;
        constexpr uint32_t BlockPlayer = 4u;
        constexpr uint32_t UnblockPlayer = 5u;
        constexpr uint32_t BlockPlayerForSession = 6u;
    }
    namespace ClubMembershipServiceV1
    {
        constexpr uint32_t Hash = 0x94B94786u;
        constexpr uint32_t Subscribe = 1u;
        constexpr uint32_t Unsubscribe = 2u;
        constexpr uint32_t GetState = 3u;
        constexpr uint32_t UpdateClubSharedSettings = 4u;
        constexpr uint32_t GetStreamMentions = 5u;
        constexpr uint32_t RemoveStreamMentions = 6u;
        constexpr uint32_t AdvanceStreamMentionViewTime = 7u;
    }
    namespace ClubServiceV1
    {
        constexpr uint32_t Hash = 0xE273DE0Eu;
        constexpr uint32_t Subscribe = 1u;
        constexpr uint32_t Unsubscribe = 2u;
        constexpr uint32_t Create = 3u;
        constexpr uint32_t Destroy = 4u;
        constexpr uint32_t GetDescription = 5u;
        constexpr uint32_t GetClubType = 6u;
        constexpr uint32_t UpdateClubState = 7u;
        constexpr uint32_t UpdateClubSettings = 8u;
        constexpr uint32_t Join = 30u;
        constexpr uint32_t Leave = 31u;
        constexpr uint32_t Kick = 32u;
        constexpr uint32_t GetMember = 33u;
        constexpr uint32_t GetMembers = 34u;
        constexpr uint32_t UpdateMemberState = 35u;
        constexpr uint32_t UpdateSubscriberState = 36u;
        constexpr uint32_t AssignRole = 37u;
        constexpr uint32_t UnassignRole = 38u;
        constexpr uint32_t SendInvitation = 50u;
        constexpr uint32_t AcceptInvitation = 51u;
        constexpr uint32_t DeclineInvitation = 52u;
        constexpr uint32_t RevokeInvitation = 53u;
        constexpr uint32_t GetInvitation = 54u;
        constexpr uint32_t GetInvitations = 55u;
        constexpr uint32_t SendSuggestion = 60u;
        constexpr uint32_t AcceptSuggestion = 61u;
        constexpr uint32_t DeclineSuggestion = 62u;
        constexpr uint32_t GetSuggestion = 63u;
        constexpr uint32_t GetSuggestions = 64u;
        constexpr uint32_t CreateTicket = 70u;
        constexpr uint32_t DestroyTicket = 71u;
        constexpr uint32_t RedeemTicket = 72u;
        constexpr uint32_t GetTicket = 73u;
        constexpr uint32_t GetTickets = 74u;
        constexpr uint32_t AddBan = 80u;
        constexpr uint32_t RemoveBan = 81u;
        constexpr uint32_t GetBan = 82u;
        constexpr uint32_t GetBans = 83u;
        constexpr uint32_t SubscribeStream = 100u;
        constexpr uint32_t UnsubscribeStream = 101u;
        constexpr uint32_t CreateStream = 102u;
        constexpr uint32_t DestroyStream = 103u;
        constexpr uint32_t GetStream = 104u;
        constexpr uint32_t GetStreams = 105u;
        constexpr uint32_t UpdateStreamState = 106u;
        constexpr uint32_t SetStreamFocus = 107u;
        constexpr uint32_t GetStreamVoiceToken = 108u;
        constexpr uint32_t KickFromStreamVoice = 109u;
        constexpr uint32_t CreateMessage = 150u;
        constexpr uint32_t DestroyMessage = 151u;
        constexpr uint32_t EditMessage = 152u;
        constexpr uint32_t SetMessagePinned = 153u;
        constexpr uint32_t SetTypingIndicator = 154u;
        constexpr uint32_t AdvanceStreamViewTime = 155u;
        constexpr uint32_t GetStreamHistory = 156u;
        constexpr uint32_t GetStreamMessage = 157u;
    }
    namespace FriendsServiceV1
    {
        constexpr uint32_t Hash = 0xA3DDB1BDu;
        constexpr uint32_t Subscribe = 1u;
        constexpr uint32_t SendInvitation = 2u;
        constexpr uint32_t AcceptInvitation = 3u;
        constexpr uint32_t RevokeInvitation = 4u;
        constexpr uint32_t DeclineInvitation = 5u;
        constexpr uint32_t IgnoreInvitation = 6u;
        constexpr uint32_t RemoveFriend = 8u;
        constexpr uint32_t ViewFriends = 9u;
        constexpr uint32_t UpdateFriendState = 10u;
        constexpr uint32_t Unsubscribe = 11u;
        constexpr uint32_t RevokeAllInvitations = 12u;
        constexpr uint32_t GetFriendList = 13u;
        constexpr uint32_t CreateFriendship = 14u;
    }
    namespace GameUtilitiesServiceV1
    {
        constexpr uint32_t Hash = 0x3FC1274Du;
        constexpr uint32_t ProcessClientRequest = 1u;
        constexpr uint32_t PresenceChannelCreated = 2u;
        constexpr uint32_t ProcessServerRequest = 6u;
        constexpr uint32_t OnGameAccountOnline = 7u;
        constexpr uint32_t OnGameAccountOffline = 8u;
        constexpr uint32_t GetAllValuesForAttribute = 10u;
        constexpr uint32_t RegisterUtilities = 11u;
        constexpr uint32_t UnregisterUtilities = 12u;
    }
    namespace NotificationServiceV1
    {
        constexpr uint32_t Hash = 0x0CBE3C43u;
        constexpr uint32_t SendNotification = 1u;
        constexpr uint32_t Subscribe = 6u;
        constexpr uint32_t Unsubscribe = 7u;
        constexpr uint32_t Publish = 8u;
    }
    namespace NotificationServiceV2
    {
        constexpr uint32_t Hash = 0xF8E1EB98u;
        constexpr uint32_t SendNotification = 1u;
    }
    namespace PresenceServiceV1
    {
        constexpr uint32_t Hash = 0xFA0796FFu;
        constexpr uint32_t Subscribe = 1u;
        constexpr uint32_t Unsubscribe = 2u;
        constexpr uint32_t Update = 3u;
        constexpr uint32_t Query = 4u;
        constexpr uint32_t BatchSubscribe = 8u;
        constexpr uint32_t BatchUnsubscribe = 9u;
    }
    namespace ReportServiceV1
    {
        constexpr uint32_t Hash = 0x7CAF61C9u;
        constexpr uint32_t SendReport = 1u;
        constexpr uint32_t SubmitReport = 2u;
    }
    namespace ReportServiceV2
    {
        constexpr uint32_t Hash = 0x3A4218FBu;
        constexpr uint32_t SubmitReport = 1u;
    }
    namespace ResourcesServiceV1
    {
        constexpr uint32_t Hash = 0xECBE75BAu;
        constexpr uint32_t GetContentHandle = 1u;
        constexpr uint32_t GetTitleIcons = 2u;
    }
    namespace WhisperServiceV2
    {
        constexpr uint32_t Hash = 0xFEE1AA14u;
        constexpr uint32_t Subscribe = 1u;
        constexpr uint32_t Unsubscribe = 2u;
        constexpr uint32_t GetWhisperHistory = 3u;
        constexpr uint32_t SendWhisper = 4u;
        constexpr uint32_t AdvanceViewTime = 5u;
        constexpr uint32_t AdvanceClearTime = 6u;
        constexpr uint32_t SetTypingIndicator = 7u;
    }

    // Client/listener hashes are kept separately because they are normally
    // targets for server-initiated RPCs rather than services the client calls.
    namespace AccountListenerV1 { constexpr uint32_t Hash = 0x54DFDA17u; }
    namespace AuthenticationListenerV1 { constexpr uint32_t Hash = 0x71240E35u; }
    namespace BlockListListenerV1 { constexpr uint32_t Hash = 0xB5DD8A75u; }
    namespace ClubMembershipListenerV1 { constexpr uint32_t Hash = 0x2B34597Bu; }
    namespace ClubListenerV1 { constexpr uint32_t Hash = 0x80909D73u; }
    namespace FriendsListenerV1 { constexpr uint32_t Hash = 0x6F259A13u; }
    namespace NotificationListenerV1 { constexpr uint32_t Hash = 0xE1CB2EA8u; }
    namespace NotificationListenerV2 { constexpr uint32_t Hash = 0x2362BECDu; }
    namespace PresenceListenerV1 { constexpr uint32_t Hash = 0x890AB85Fu; }
    namespace WhisperListenerV2 { constexpr uint32_t Hash = 0x62615E21u; }
    namespace ChallengeListenerV1 { constexpr uint32_t Hash = 0xBBDA171Fu; }

    namespace ConnectionService
    {
        constexpr uint32_t Hash = 0x65446991u;
        constexpr uint32_t Connect = 1u;
        constexpr uint32_t Bind = 2u;
        constexpr uint32_t Echo = 3u;
        constexpr uint32_t ForceDisconnect = 4u;
        constexpr uint32_t KeepAlive = 5u;
        constexpr uint32_t Encrypt = 6u;
        constexpr uint32_t RequestDisconnect = 7u;
    }

    namespace AuthenticationServiceV2
    {
        constexpr uint32_t Hash = 0xC02F8216u;
        constexpr uint32_t Logon = 1u;
        constexpr uint32_t VerifyWebCredentials = 2u;
    }

    namespace AuthenticationListenerV2
    {
        constexpr uint32_t Hash = 0x9DA8116Bu;
        constexpr uint32_t OnLogonComplete = 1u;
        constexpr uint32_t OnExternalChallenge = 4u;
    }

    namespace GameUtilitiesServiceV2
    {
        constexpr uint32_t Hash = 0x5DBB51C2u;
        constexpr uint32_t ProcessClientRequest = 1u;
        constexpr uint32_t GetAllValuesForAttribute = 2u;
    }

    // Service hash 0x22DC2464 is observed directly after authentication.
    // Its exact descriptor name is not confirmed, so AscEmu keeps a neutral name.
    namespace GameUtilitiesService
    {
        constexpr uint32_t Hash = GameUtilitiesServiceV2::Hash;
        constexpr uint32_t ProcessClientRequest = GameUtilitiesServiceV2::ProcessClientRequest;
        constexpr uint32_t GetAllValuesForAttribute = GameUtilitiesServiceV2::GetAllValuesForAttribute;
    }

    namespace AccountServiceV2
    {
        // bgs.protocol.account.v2.client.AccountService original hash.
        constexpr uint32_t Hash = 0x22DC2464u;
        constexpr uint32_t GetAccountInfo = 101u;
        constexpr uint32_t GetRestriction = 104u;
        constexpr uint32_t GetGameAccountLinks = 108u;
        constexpr uint32_t GetGameAccountInfo = 201u;
        constexpr uint32_t GetGameAccountRestriction = 203u;
    }


    namespace GameUtilitiesCommands
    {
        // Command suffixes can change between client families/builds. Dispatch
        // therefore keys off the stable semantic prefix.
        constexpr std::string_view RealmListTicketPrefix = "Command_RealmListTicketRequest_v1_";
        constexpr std::string_view RealmListPrefix = "Command_RealmListRequest_v1_";
        constexpr std::string_view RealmJoinPrefix = "Command_RealmJoinRequest_v1_";
        constexpr std::string_view LastCharPlayedPrefix = "Command_LastCharPlayedRequest_v1_";
        constexpr std::string_view FetchBleepProxiesPrefix = "Command_FetchBleepProxiesRequest_v1_";
        constexpr std::string_view SuperDistrictListPrefix = "Command_SuperDistrictListRequest_v1_";

    }

    constexpr uint32_t ResponseServiceId = 0xFEu;
    constexpr uint32_t BindlessServiceId = 0u;

    struct ServiceDescription
    {
        uint32_t hash;
        std::string_view name;
        std::string_view purpose;
    };

    struct MethodDescription
    {
        uint32_t serviceHash;
        uint32_t methodId;
        std::string_view name;
        std::string_view purpose;
    };

    constexpr ServiceDescription Services[] = {
        { ConnectionService::Hash, "ConnectionService", "Creates and maintains the Battle.net RPC connection." },
        { AuthenticationServiceV2::Hash, "AuthenticationServiceV2", "Authenticates the Battle.net account and verifies WebAuth credentials." },
        { AuthenticationListenerV2::Hash, "AuthenticationListenerV2", "Client listener used for server-initiated authentication notifications and challenges." },
        { GameUtilitiesServiceV2::Hash, "GameUtilitiesServiceV2", "WoW command-envelope service used by current clients for realm-list, realm-ticket and realm-join operations." },
        { AccountServiceV1::Hash, "AccountServiceV1", "Battle.net account and game-account state queries." },
        { AuthenticationServiceV1::Hash, "AuthenticationServiceV1", "Legacy/earlier Battle.net authentication RPC service." },
        { BlockListServiceV1::Hash, "BlockListServiceV1", "Blocked-player list subscription and management." },
        { ClubMembershipServiceV1::Hash, "ClubMembershipServiceV1", "Membership operations for Battle.net clubs/communities." },
        { ClubServiceV1::Hash, "ClubServiceV1", "Battle.net club/community operations." },
        { FriendsServiceV1::Hash, "FriendsServiceV1", "Friends list, invitations and friendship state." },
        { GameUtilitiesServiceV1::Hash, "GameUtilitiesServiceV1", "Earlier GameUtilities command-envelope service." },
        { NotificationServiceV1::Hash, "NotificationServiceV1", "Generic Battle.net notification subscription/service." },
        { NotificationServiceV2::Hash, "NotificationServiceV2", "Version 2 Battle.net notification service." },
        { PresenceServiceV1::Hash, "PresenceServiceV1", "Presence subscription and online-state queries." },
        { ReportServiceV1::Hash, "ReportServiceV1", "Version 1 Battle.net reporting service." },
        { ReportServiceV2::Hash, "ReportServiceV2", "Version 2 Battle.net reporting service." },
        { ResourcesServiceV1::Hash, "ResourcesServiceV1", "Battle.net resource/content-handle lookup service." },
        { WhisperServiceV2::Hash, "WhisperServiceV2", "Version 2 Battle.net whisper/private-message service." },
        { AccountServiceV2::Hash, "AccountServiceV2", "Battle.net v2 account and WoW game-account information service." }
    };

    constexpr MethodDescription Methods[] = {
        { ConnectionService::Hash, ConnectionService::Connect, "Connect", "Negotiates RPC connection identity, time and bindless-RPC capability." },
        { ConnectionService::Hash, ConnectionService::KeepAlive, "KeepAlive", "Keeps an established RPC connection alive." },
        { ConnectionService::Hash, ConnectionService::RequestDisconnect, "RequestDisconnect", "Client request to close the Battle.net RPC connection cleanly." },
        { AuthenticationServiceV2::Hash, AuthenticationServiceV2::Logon, "Logon", "Starts Battle.net v2 authentication and requests WebAuth when needed." },
        { AuthenticationServiceV2::Hash, AuthenticationServiceV2::VerifyWebCredentials, "VerifyWebCredentials", "Verifies the WebAuth login ticket and completes account authentication." },
        { AuthenticationListenerV2::Hash, AuthenticationListenerV2::OnLogonComplete, "OnLogonComplete", "Server notification that authentication completed successfully." },
        { AuthenticationListenerV2::Hash, AuthenticationListenerV2::OnExternalChallenge, "OnExternalChallenge", "Server request asking the client to complete an external WebAuth challenge." },
        { GameUtilitiesService::Hash, GameUtilitiesService::ProcessClientRequest, "ProcessClientRequest", "Processes WoW command envelopes such as RealmListTicket, RealmList and RealmJoin." },
        { GameUtilitiesService::Hash, GameUtilitiesService::GetAllValuesForAttribute, "GetAllValuesForAttribute", "Returns available values for a requested WoW game-utility attribute, including realm subregions." },
        { ConnectionService::Hash, ConnectionService::Bind, "Bind", "Known ConnectionService RPC method; currently not part of the normal AscEmu login path." },
        { ConnectionService::Hash, ConnectionService::Echo, "Echo", "Known ConnectionService RPC method; currently not part of the normal AscEmu login path." },
        { ConnectionService::Hash, ConnectionService::ForceDisconnect, "ForceDisconnect", "Known ConnectionService RPC method; currently not part of the normal AscEmu login path." },
        { ConnectionService::Hash, ConnectionService::Encrypt, "Encrypt", "Known ConnectionService RPC method; currently not part of the normal AscEmu login path." },
        { AccountServiceV1::Hash, AccountServiceV1::ResolveAccount, "ResolveAccount", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV1::Hash, AccountServiceV1::Subscribe, "Subscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV1::Hash, AccountServiceV1::Unsubscribe, "Unsubscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV1::Hash, AccountServiceV1::GetAccountState, "GetAccountState", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV1::Hash, AccountServiceV1::GetGameAccountState, "GetGameAccountState", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV1::Hash, AccountServiceV1::GetLicenses, "GetLicenses", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV1::Hash, AccountServiceV1::GetGameTimeRemainingInfo, "GetGameTimeRemainingInfo", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV1::Hash, AccountServiceV1::GetGameSessionInfo, "GetGameSessionInfo", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV1::Hash, AccountServiceV1::GetCAISInfo, "GetCAISInfo", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV1::Hash, AccountServiceV1::GetAuthorizedData, "GetAuthorizedData", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV1::Hash, AccountServiceV1::GetSignedAccountState, "GetSignedAccountState", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV1::Hash, AccountServiceV1::GetAccountInfo, "GetAccountInfo", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV1::Hash, AccountServiceV1::GetAccountPlatformRestrictions, "GetAccountPlatformRestrictions", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AuthenticationServiceV1::Hash, AuthenticationServiceV1::Logon, "Logon", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AuthenticationServiceV1::Hash, AuthenticationServiceV1::VerifyWebCredentials, "VerifyWebCredentials", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AuthenticationServiceV1::Hash, AuthenticationServiceV1::GenerateWebCredentials, "GenerateWebCredentials", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { BlockListServiceV1::Hash, BlockListServiceV1::Subscribe, "Subscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { BlockListServiceV1::Hash, BlockListServiceV1::Unsubscribe, "Unsubscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { BlockListServiceV1::Hash, BlockListServiceV1::GetState, "GetState", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { BlockListServiceV1::Hash, BlockListServiceV1::BlockPlayer, "BlockPlayer", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { BlockListServiceV1::Hash, BlockListServiceV1::UnblockPlayer, "UnblockPlayer", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { BlockListServiceV1::Hash, BlockListServiceV1::BlockPlayerForSession, "BlockPlayerForSession", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubMembershipServiceV1::Hash, ClubMembershipServiceV1::Subscribe, "Subscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubMembershipServiceV1::Hash, ClubMembershipServiceV1::Unsubscribe, "Unsubscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubMembershipServiceV1::Hash, ClubMembershipServiceV1::GetState, "GetState", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubMembershipServiceV1::Hash, ClubMembershipServiceV1::UpdateClubSharedSettings, "UpdateClubSharedSettings", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubMembershipServiceV1::Hash, ClubMembershipServiceV1::GetStreamMentions, "GetStreamMentions", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubMembershipServiceV1::Hash, ClubMembershipServiceV1::RemoveStreamMentions, "RemoveStreamMentions", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubMembershipServiceV1::Hash, ClubMembershipServiceV1::AdvanceStreamMentionViewTime, "AdvanceStreamMentionViewTime", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::Subscribe, "Subscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::Unsubscribe, "Unsubscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::Create, "Create", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::Destroy, "Destroy", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetDescription, "GetDescription", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetClubType, "GetClubType", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::UpdateClubState, "UpdateClubState", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::UpdateClubSettings, "UpdateClubSettings", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::Join, "Join", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::Leave, "Leave", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::Kick, "Kick", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetMember, "GetMember", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetMembers, "GetMembers", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::UpdateMemberState, "UpdateMemberState", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::UpdateSubscriberState, "UpdateSubscriberState", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::AssignRole, "AssignRole", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::UnassignRole, "UnassignRole", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::SendInvitation, "SendInvitation", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::AcceptInvitation, "AcceptInvitation", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::DeclineInvitation, "DeclineInvitation", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::RevokeInvitation, "RevokeInvitation", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetInvitation, "GetInvitation", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetInvitations, "GetInvitations", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::SendSuggestion, "SendSuggestion", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::AcceptSuggestion, "AcceptSuggestion", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::DeclineSuggestion, "DeclineSuggestion", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetSuggestion, "GetSuggestion", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetSuggestions, "GetSuggestions", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::CreateTicket, "CreateTicket", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::DestroyTicket, "DestroyTicket", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::RedeemTicket, "RedeemTicket", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetTicket, "GetTicket", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetTickets, "GetTickets", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::AddBan, "AddBan", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::RemoveBan, "RemoveBan", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetBan, "GetBan", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetBans, "GetBans", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::SubscribeStream, "SubscribeStream", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::UnsubscribeStream, "UnsubscribeStream", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::CreateStream, "CreateStream", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::DestroyStream, "DestroyStream", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetStream, "GetStream", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetStreams, "GetStreams", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::UpdateStreamState, "UpdateStreamState", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::SetStreamFocus, "SetStreamFocus", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetStreamVoiceToken, "GetStreamVoiceToken", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::KickFromStreamVoice, "KickFromStreamVoice", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::CreateMessage, "CreateMessage", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::DestroyMessage, "DestroyMessage", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::EditMessage, "EditMessage", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::SetMessagePinned, "SetMessagePinned", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::SetTypingIndicator, "SetTypingIndicator", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::AdvanceStreamViewTime, "AdvanceStreamViewTime", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetStreamHistory, "GetStreamHistory", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ClubServiceV1::Hash, ClubServiceV1::GetStreamMessage, "GetStreamMessage", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::Subscribe, "Subscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::SendInvitation, "SendInvitation", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::AcceptInvitation, "AcceptInvitation", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::RevokeInvitation, "RevokeInvitation", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::DeclineInvitation, "DeclineInvitation", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::IgnoreInvitation, "IgnoreInvitation", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::RemoveFriend, "RemoveFriend", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::ViewFriends, "ViewFriends", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::UpdateFriendState, "UpdateFriendState", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::Unsubscribe, "Unsubscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::RevokeAllInvitations, "RevokeAllInvitations", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::GetFriendList, "GetFriendList", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { FriendsServiceV1::Hash, FriendsServiceV1::CreateFriendship, "CreateFriendship", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { GameUtilitiesServiceV1::Hash, GameUtilitiesServiceV1::ProcessClientRequest, "ProcessClientRequest", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { GameUtilitiesServiceV1::Hash, GameUtilitiesServiceV1::PresenceChannelCreated, "PresenceChannelCreated", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { GameUtilitiesServiceV1::Hash, GameUtilitiesServiceV1::ProcessServerRequest, "ProcessServerRequest", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { GameUtilitiesServiceV1::Hash, GameUtilitiesServiceV1::OnGameAccountOnline, "OnGameAccountOnline", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { GameUtilitiesServiceV1::Hash, GameUtilitiesServiceV1::OnGameAccountOffline, "OnGameAccountOffline", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { GameUtilitiesServiceV1::Hash, GameUtilitiesServiceV1::GetAllValuesForAttribute, "GetAllValuesForAttribute", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { GameUtilitiesServiceV1::Hash, GameUtilitiesServiceV1::RegisterUtilities, "RegisterUtilities", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { GameUtilitiesServiceV1::Hash, GameUtilitiesServiceV1::UnregisterUtilities, "UnregisterUtilities", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { NotificationServiceV1::Hash, NotificationServiceV1::SendNotification, "SendNotification", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { NotificationServiceV1::Hash, NotificationServiceV1::Subscribe, "Subscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { NotificationServiceV1::Hash, NotificationServiceV1::Unsubscribe, "Unsubscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { NotificationServiceV1::Hash, NotificationServiceV1::Publish, "Publish", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { NotificationServiceV2::Hash, NotificationServiceV2::SendNotification, "SendNotification", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { PresenceServiceV1::Hash, PresenceServiceV1::Subscribe, "Subscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { PresenceServiceV1::Hash, PresenceServiceV1::Unsubscribe, "Unsubscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { PresenceServiceV1::Hash, PresenceServiceV1::Update, "Update", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { PresenceServiceV1::Hash, PresenceServiceV1::Query, "Query", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { PresenceServiceV1::Hash, PresenceServiceV1::BatchSubscribe, "BatchSubscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { PresenceServiceV1::Hash, PresenceServiceV1::BatchUnsubscribe, "BatchUnsubscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ReportServiceV1::Hash, ReportServiceV1::SendReport, "SendReport", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ReportServiceV1::Hash, ReportServiceV1::SubmitReport, "SubmitReport", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ReportServiceV2::Hash, ReportServiceV2::SubmitReport, "SubmitReport", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ResourcesServiceV1::Hash, ResourcesServiceV1::GetContentHandle, "GetContentHandle", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { ResourcesServiceV1::Hash, ResourcesServiceV1::GetTitleIcons, "GetTitleIcons", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { WhisperServiceV2::Hash, WhisperServiceV2::Subscribe, "Subscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { WhisperServiceV2::Hash, WhisperServiceV2::Unsubscribe, "Unsubscribe", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { WhisperServiceV2::Hash, WhisperServiceV2::GetWhisperHistory, "GetWhisperHistory", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { WhisperServiceV2::Hash, WhisperServiceV2::SendWhisper, "SendWhisper", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { WhisperServiceV2::Hash, WhisperServiceV2::AdvanceViewTime, "AdvanceViewTime", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { WhisperServiceV2::Hash, WhisperServiceV2::AdvanceClearTime, "AdvanceClearTime", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { WhisperServiceV2::Hash, WhisperServiceV2::SetTypingIndicator, "SetTypingIndicator", "Known Battle.net RPC method; service behavior is not implemented in AscEmu yet." },
        { AccountServiceV2::Hash, AccountServiceV2::GetAccountInfo, "GetAccountInfo", "Returns Battle.net account identity information." },
        { AccountServiceV2::Hash, AccountServiceV2::GetRestriction, "GetRestriction", "Returns Battle.net account restrictions." },
        { AccountServiceV2::Hash, AccountServiceV2::GetGameAccountLinks, "GetGameAccountLinks", "Returns WoW game accounts linked to the authenticated Battle.net account." },
        { AccountServiceV2::Hash, AccountServiceV2::GetGameAccountInfo, "GetGameAccountInfo", "Returns display information for a linked WoW game account." },
        { AccountServiceV2::Hash, AccountServiceV2::GetGameAccountRestriction, "GetGameAccountRestriction", "Returns restrictions for a linked WoW game account." }
    };

    constexpr std::string_view getServiceName(uint32_t hash)
    {
        for (ServiceDescription const& service : Services)
            if (service.hash == hash)
                return service.name;
        return "UnknownService";
    }

    constexpr std::string_view getMethodName(uint32_t serviceHash, uint32_t methodId)
    {
        for (MethodDescription const& method : Methods)
            if (method.serviceHash == serviceHash && method.methodId == methodId)
                return method.name;
        return "UnknownMethod";
    }

    constexpr bool isKnownService(uint32_t hash)
    {
        return getServiceName(hash) != "UnknownService";
    }
}
