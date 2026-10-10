# Dragonflight protocol profile

Dragonflight 10.2.7 clients log in through Battle.net with the protobuf services (the same v1 services as
Legion 7.3.5, BfA 8.3.7 and Shadowlands 9.2.7) and select the Battle.net server topology.

Pinned client:

- Version: 10.2.7
- Build: 55664

`World/WorldProfile.hpp` holds the values of this client for the AES world connection
(`src/version/Shared/World/WorldSocketAes.*`): line feed terminated V2 initializers, the 16 byte header
(size and AES-GCM tag) in both directions, the build auth seed of the Windows client and
SMSG_ENTER_ENCRYPTED_MODE / CMSG_ENTER_ENCRYPTED_MODE_ACK before the AES-128-GCM encryption starts.
The seeds of the digest, the session key, the continued session and the encryption key, the Ed25519 pair and
the signature context are those of 9.2.7. Everything after authentication goes through the central opcode
table and the managed packets.

## Status

Written against the 10.2.7 reference on 2026-10-10, NOT tested with a client yet:

- Battle.net login through the v1 services, AES world connection with the Ed25519 signed encryption start.
- Character list, character creation and login with the 10.x layouts and error codes (the personal tabard,
  the timerunning season and the hardcore flag are read or sent empty).
- Login packets of 10.x: auth response (minimum class expansion, build keys), feature system status and glue
  screen (addon chat throttle, new feature bits), account data times with 15 types, the glue screen sequence
  of the reference (time zones with three names, available hotfixes, tutorials, undelete cooldown, server
  time offset and social contract answers), chat with 11 bit texts and the flags as
  field, player name query with the timerunning season, creature query with quest currencies, learned and
  superceded spells as lists, 180 action buttons, 1000 factions with 16 bit flags, currencies with recharge
  bits, criteria updates, quest giver status as 64 bit value, gossip with option ids and order, item push
  result with toasts, inventory failures with a 32 bit result, xp log without the refer a friend bonus.
- 10.x movement info (the game object the mover stands on, advanced flying, inertia with an id) in the codec
  descriptors and the create block with its advanced flying parameters, SMSG_ON_MONSTER_MOVE without the
  destination in the spline header.
- Spell cast data with 28 target flag bits and byte sized miss results, combat logs with supporter lists and
  the 10.x content tuning block.
- Update field structures of 10.2.7 in `src/world/Objects/ObjectUpdateDragonflight.*` (spell empower stage,
  support modifiers, flight capability, player name and tabard in the player data, 175 quest log slots,
  the explored zones as data flags, the inventory slots in the 10.x order, item bonus key, game object world
  effects, the area trigger additions).

A character without a chosen specialization logs in with the initial specialization of its class
(ChrSpecialization order index 4, ids 1444 to 1456 and 1465), the client needs one for its talent and
tutorial checks.

Open:

- the client data records (CMSG_DB_QUERY_BULK, 0x35E4) are answered as not available for every table; the
  5.x item record layout of the older targets passes the size check of the 10.x client and crashes it, the
  10.2.7 Item and ItemSparse record layouts are not written yet
- SMSG_MOTD does not exist anymore, the message of the day is not shown
- the trait configurations are sent empty, the talent window has no configuration yet
- the pvp brackets of the active player are sent as empty list
- the 9.x open items apply as well (quest giver status values, SMSG_AVAILABLE_HOTFIXES, CMSG_HOTFIX_REQUEST
  layout, battle pay, only the Windows auth seed of 55664 is known)

## Client

As with Shadowlands the connection patcher is not used: the client is started through the Arctium WoW
Launcher in a version that supports 10.2.7, with the original 55664 executable, `.build.info` next to it and
`SET portal "127.0.0.1"` in `WTF\Config.wtf`. The launcher replaces the Battle.net keys, the portal suffix,
the versions address and the certificate bundle in memory and sets the public key of the Ed25519 pair in
`src/version/Shared/World/WorldConnectKey.hpp`. Redistributed executables with their own protection
(launcher-bound builds) reject other servers.

The 10.x client verifies the certificate of the web login against the system store and closes the connection
right after the TLS handshake with the self signed server certificate (the Battle.net connection itself uses
the key bundle the launcher replaces). The Battle.net server therefore sends 10.2.7 clients the login url with
the `http://` scheme and answers them on the web auth port without TLS, decided per connection by the first
byte received. The client disconnects after a login form without the SRP url, so the Dragonflight target sends
the form with `srp_url` and logs the account in with the SRP challenge and proof instead of the plain password
form of the 6.2.4 to 9.x targets (`AE_BNET_PASSWORD_WEB_LOGIN` in `src/battlenet/Server/BNetProtocol.hpp`).
