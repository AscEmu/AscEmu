# AscEmu Battle.net protocol implementation

The Battle.net server is intentionally implemented with AscEmu-owned protocol names and documentation. Wire values are recorded from supported clients and kept in `Server/BNetProtocol.hpp`.

The Battle.net transport/service layer is shared infrastructure. The active modern game profile is selected separately by the AscEmu build configuration; at present the only supported modern profile is Forever 1.60.1 build 69893. Legacy client families continue to use the legacy logon-server path.

## Service status

| Service | Hash | Status | Purpose |
| --- | ---: | --- | --- |
| ConnectionService | `0x65446991` | implemented | RPC connection setup, keepalive and clean disconnect |
| AuthenticationServiceV2 | `0xC02F8216` | implemented | Battle.net v2 logon and WebAuth credential verification |
| AuthenticationListenerV2 | `0x9DA8116B` | implemented (server initiated) | external WebAuth challenge and logon-complete notification |
| GameUtilitiesService | `0x5DBB51C2` | partial/implemented for WoW realm flow | realm-list ticket, subregions, realm list and realm join |
| AccountServiceV2 | `0x22DC2464` | implemented subset | account/game-account info, links and restriction queries |

Unknown services and methods are logged with service hash, service name (when known), method id, method name, token, payload size and a bounded hexadecimal payload dump. Unknown methods on a known service are therefore distinguishable from completely unknown services.

Connection method 7 is handled as `RequestDisconnect`.


## Known Battle.net service registry

The registry contains every service hash available in the supplied Battle.net protobuf descriptor set, even when AscEmu does not implement the service yet. This lets logs name optional platform/social traffic instead of reporting only an unknown hexadecimal hash.

### Client-callable services

| Service | Hash | AscEmu status |
| --- | ---: | --- |
| AccountServiceV1 | `0x62DA0891` | known, no handler yet |
| AuthenticationServiceV1 | `0x0DECFC01` | known, no handler yet |
| AuthenticationServiceV2 | `0xC02F8216` | implemented for current login flow |
| BlockListServiceV1 | `0x8E8F5FB0` | known, no handler yet |
| ClubMembershipServiceV1 | `0x94B94786` | known, no handler yet |
| ClubServiceV1 | `0xE273DE0E` | known, no handler yet |
| ConnectionServiceV1 | `0x65446991` | implemented |
| FriendsServiceV1 | `0xA3DDB1BD` | known, no handler yet |
| GameUtilitiesServiceV1 | `0x3FC1274D` | known, no handler yet |
| GameUtilitiesServiceV2 | `0x5DBB51C2` | partial, realm flow implemented |
| NotificationServiceV1 | `0x0CBE3C43` | known, no handler yet |
| NotificationServiceV2 | `0xF8E1EB98` | known, no handler yet |
| PresenceServiceV1 | `0xFA0796FF` | known, no handler yet |
| ReportServiceV1 | `0x7CAF61C9` | known, no handler yet |
| ReportServiceV2 | `0x3A4218FB` | known, no handler yet |
| ResourcesServiceV1 | `0xECBE75BA` | known, no handler yet |
| WhisperServiceV2 | `0xFEE1AA14` | known, no handler yet |
| AccountServiceV2 | `0x22DC2464` | implemented subset from current BGS v2 descriptors |

### Server-to-client listeners

These hashes identify listener endpoints used for server-initiated RPCs. They are intentionally kept separate from client-callable service handlers.

| Listener | Hash |
| --- | ---: |
| AccountListenerV1 | `0x54DFDA17` |
| AuthenticationListenerV1 | `0x71240E35` |
| AuthenticationListenerV2 | `0x9DA8116B` |
| BlockListListenerV1 | `0xB5DD8A75` |
| ChallengeListenerV1 | `0xBBDA171F` |
| ClubMembershipListenerV1 | `0x2B34597B` |
| ClubListenerV1 | `0x80909D73` |
| FriendsListenerV1 | `0x6F259A13` |
| NotificationListenerV1 | `0xE1CB2EA8` |
| NotificationListenerV2 | `0x2362BECD` |
| PresenceListenerV1 | `0x890AB85F` |
| WhisperListenerV2 | `0x62615E21` |

Additional service versions are added only when their hashes and method contracts are verified. Unknown hashes are deliberately not guessed.

## Known RPC method IDs

The method names below are taken from the supplied Battle.net protobuf descriptor/generated source set. They are registry metadata only unless the service status above says that AscEmu implements the method. This distinction is intentional: a known name must not silently become a fake successful implementation.

| Service | Methods |
| --- | --- |
| ConnectionServiceV1 | `1 Connect`, `2 Bind`, `3 Echo`, `4 ForceDisconnect`, `5 KeepAlive`, `6 Encrypt`, `7 RequestDisconnect` |
| AccountServiceV1 | `13 ResolveAccount`, `25 Subscribe`, `26 Unsubscribe`, `30 GetAccountState`, `31 GetGameAccountState`, `32 GetLicenses`, `33 GetGameTimeRemainingInfo`, `34 GetGameSessionInfo`, `35 GetCAISInfo`, `37 GetAuthorizedData`, `44 GetSignedAccountState`, `45 GetAccountInfo`, `46 GetAccountPlatformRestrictions` |
| AuthenticationServiceV1 | `1 Logon`, `7 VerifyWebCredentials`, `8 GenerateWebCredentials` |
| BlockListServiceV1 | `1 Subscribe`, `2 Unsubscribe`, `3 GetState`, `4 BlockPlayer`, `5 UnblockPlayer`, `6 BlockPlayerForSession` |
| ClubMembershipServiceV1 | `1 Subscribe`, `2 Unsubscribe`, `3 GetState`, `4 UpdateClubSharedSettings`, `5 GetStreamMentions`, `6 RemoveStreamMentions`, `7 AdvanceStreamMentionViewTime` |
| ClubServiceV1 | `1 Subscribe`, `2 Unsubscribe`, `3 Create`, `4 Destroy`, `5 GetDescription`, `6 GetClubType`, `7 UpdateClubState`, `8 UpdateClubSettings`, `30-38 membership`, `50-55 invitations`, `60-64 suggestions`, `70-74 tickets`, `80-83 bans`, `100-109 streams`, `150-157 messages` |
| FriendsServiceV1 | `1 Subscribe`, `2 SendInvitation`, `3 AcceptInvitation`, `4 RevokeInvitation`, `5 DeclineInvitation`, `6 IgnoreInvitation`, `8 RemoveFriend`, `9 ViewFriends`, `10 UpdateFriendState`, `11 Unsubscribe`, `12 RevokeAllInvitations`, `13 GetFriendList`, `14 CreateFriendship` |
| GameUtilitiesServiceV1 | `1 ProcessClientRequest`, `2 PresenceChannelCreated`, `6 ProcessServerRequest`, `7 OnGameAccountOnline`, `8 OnGameAccountOffline`, `10 GetAllValuesForAttribute`, `11 RegisterUtilities`, `12 UnregisterUtilities` |
| NotificationServiceV1 | `1 SendNotification`, `6 Subscribe`, `7 Unsubscribe`, `8 Publish` |
| NotificationServiceV2 | `1 SendNotification` |
| PresenceServiceV1 | `1 Subscribe`, `2 Unsubscribe`, `3 Update`, `4 Query`, `8 BatchSubscribe`, `9 BatchUnsubscribe` |
| ReportServiceV1 | `1 SendReport`, `2 SubmitReport` |
| ReportServiceV2 | `1 SubmitReport` |
| ResourcesServiceV1 | `1 GetContentHandle`, `2 GetTitleIcons` |
| WhisperServiceV2 | `1 Subscribe`, `2 Unsubscribe`, `3 GetWhisperHistory`, `4 SendWhisper`, `5 AdvanceViewTime`, `6 AdvanceClearTime`, `7 SetTypingIndicator` |

Service versions keep independent method tables. For example, `AuthenticationServiceV2::VerifyWebCredentials` is method `2`, while V1 uses method `7`. Do not merge method tables solely because operation names match.

## GameUtilities commands

The WoW client places command names inside `ProcessClientRequest` envelopes. Realm dispatch uses stable semantic prefixes instead of client-family suffixes:

- `Command_RealmListTicketRequest_v1_`
- `Command_RealmListRequest_v1_`
- `Command_RealmJoinRequest_v1_`

Unknown commands are logged by name and answered only when their contract is understood.

## Naming policy

Do not copy service/class names from other emulator projects unless they are confirmed protocol descriptor names. Unknown wire contracts use neutral AscEmu names (`UnknownService`, `UnknownMethod`) and are documented with the build on which they were observed.

Build-specific behavior should be isolated behind explicit build checks and comments. Capture replay code is diagnostic only and must not become part of the normal Battle.net request path.

## Logging policy

Normal Battle.net RPC logging is intentionally concise. A handled request/response pair is logged as one semantic RX line and one semantic TX line, for example:

```text
BNet RX AuthenticationServiceV2::Logon size=128 token=4
BNet TX AuthenticationServiceV2::LogonResponse size=0 token=4
```

Server-initiated RPCs use the same format, for example:

```text
BNet TX AuthenticationListenerV2::OnExternalChallenge size=73 token=1
```

Raw protobuf headers, decrypted TLS payloads, frame bytes, command parameters and other protocol diagnostics are emitted only at debug level. Unknown services or methods remain visible at normal info level as `BNet UNHANDLED ...`, including the numeric service hash and method id so a new client build can be catalogued without enabling packet spam.

Malformed RPC headers, invalid payloads and authentication/transport failures remain failure-level messages.


## Supported World V2 target

The currently supported modern World V2 client profile is **World of Warcraft Forever 1.60.1 build 69893**.

Forever is intentionally maintained as its own protocol profile under `src/version/Forever`. Battle.net authentication and realm discovery are shared infrastructure, while World opcodes, packet layouts, authentication material and bootstrap data remain version-specific.

The verified World V2 framing/authentication opcodes for build 69893 are:

- `SMSG_AUTH_CHALLENGE = 0x004D0000`
- `CMSG_AUTH_SESSION = 0x00450001`
- `CMSG_AUTH_CONTINUED_SESSION = 0x00450003`
- `SMSG_ENTER_ENCRYPTED_MODE = 0x004D0004`
- `CMSG_ENTER_ENCRYPTED_MODE_ACK = 0x00450005`
- `CMSG_PING = 0x00450006`
- `SMSG_RESUME_COMMS = 0x004D0006`
- `SMSG_CONNECT_TO = 0x004D0008`
- `SMSG_PONG = 0x004D0009`

The first verified encrypted bootstrap packets include:

- `SMSG_POST_AUTH_650007 = 0x00650007`
- `SMSG_POST_AUTH_4602CE = 0x004602CE`
- `SMSG_AUTH_RESPONSE = 0x00460001`
- `SMSG_SET_TIME_ZONE_INFORMATION = 0x00460123`
- `SMSG_FEATURE_SYSTEM_STATUS_GLUE_SCREEN = 0x00460064`
- `SMSG_POST_AUTH_CONFIG = 0x00460371`
- `SMSG_GLUE_BOOTSTRAP_EMPTY = 0x00460003`
- `SMSG_GLUE_BOOTSTRAP_STATE = 0x004602CB`
- `SMSG_TUTORIAL_FLAGS = 0x00460268`
- `SMSG_ACCOUNT_DATA_TIMES = 0x004601B5`

The World V2 initializer strings are:

```text
WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2
WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2
```

The observed authentication challenge payload is 65 bytes: a 32-byte DoS challenge, a 32-byte server challenge and a one-byte zero-bit field. The client auth-session fixed portion is 77 bytes before the variable ticket data. World V2 encrypted packet authentication uses a 12-byte authentication tag.

`SMSG_ENTER_ENCRYPTED_MODE` uses the Forever-specific verified signing/authentication path. Build-specific authentication keys and packet constants belong in the Forever profile and must not be shared with another client family.

Only **build 69893** is currently supported. Captures from other Forever builds may be useful for protocol comparison, but matching packet layouts do not make those builds supported automatically. Unsupported builds must be rejected instead of silently falling back to the 69893 profile.
