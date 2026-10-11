# The War Within protocol profile

The War Within 11.2.7 clients log in through Battle.net with the protobuf services (the same v1 services as
Legion 7.3.5, BfA 8.3.7, Shadowlands 9.2.7 and Dragonflight 10.2.7) and select the Battle.net server topology.

Pinned client:

- Version: 11.2.7
- Build: 65299

`World/WorldProfile.hpp` holds the values of this client for the AES world connection
(`src/version/Shared/World/WorldSocketAes.*`): line feed terminated V2 initializers, the 16 byte header
(size and AES-GCM tag) in both directions, the build auth seeds of the four client platforms and
SMSG_ENTER_ENCRYPTED_MODE / CMSG_ENTER_ENCRYPTED_MODE_ACK before the encryption starts.

11.x widened the transport (`AesWorld::WideTransport` in `WorldSocketAes.cpp`): the opcodes are 32 bit values
on the wire and in the opcode table, the server and client challenges are 32 bytes, the five transport seeds are
32 bytes, the digest key, the session key stream and every HMAC use SHA512, the packet key is the first 32
bytes of the HMAC and the packets are encrypted with AES-256-GCM. SMSG_ENTER_ENCRYPTED_MODE starts with
the region group, CMSG_AUTH_CONTINUED_SESSION carries the key behind the digest followed by the native realm
address and a third key, SMSG_CONNECT_TO ends with both. The Ed25519 pair and the signature context are
those of 9.2.7, the client packet size limit is 0x10000.

## Status

Tested on 2026-10-11 with the 65299 client started through the Arctium WoW Launcher: Battle.net login, character
list, login and world entry work.

- Battle.net login through the v1 services (the realm join ticket and realm list JSON are unchanged), the
  widened AES world connection described above, Ed25519 signed encryption start.
- Opcode table `src/world/Version/Opcodes/OpcodeTable_TheWarWithin.cpp` (1415 rows: 1413 through the 10.2.7 names,
  SMSG_MOTD which exists again and the second whisper opcode CMSG_MESSAGE_WHISPER); 23 opcodes of 10.2.7 do not
  exist anymore (forced reactions, void storage, reagent bank, client reports, spell history refresh, area trigger
  re-path/re-shape, guild roster update).
- Every packet class with a 10.x branch serves 11.x as well; own 11.x branches where the layout changed: player
  login bit, account data times (20 types), auth response wait info, character create/delete results (32 bit),
  character list (11.2.7 CharacterInfoBasic with warband, restriction and mail data), feature system status and glue
  screen, world server info (housing guids), MOTD, currencies, factions as list, talent data glyph counts, item push,
  power update, stand state, mirror timer, channel joined, contact list / friend status (8 bit class), creature,
  game object and gossip queries, player names, quest giver status bits, gossip POI, trainer type, aura update
  (16 bit slot, destination, cast item; `AuraDataTheWarWithin.h`), spell start/go (`SpellCastDataTheWarWithin.h`),
  energize log, non melee damage log, attacker state content tuning, undelete cooldown.
- 11.x movement info (drive status bit and block), create block flags (entity position, room, decor, mesh object,
  no area trigger bit), monster spline (face byte, taxi smoothing and turn data bits, stop use face direction).
- Update field structures of 11.2.7 in `src/world/Objects/ObjectUpdateTheWarWithin.*` (unit flags 4, creature type
  byte, assist action data, player forced reactions and leaver info, quest log 31 bits, active player bit vectors
  instead of data flags, 105 inventory slots, bank tab settings, housing lists, area trigger path and shape data).
- 11.x object updates: SMSG_UPDATE_OBJECT starts with the map, the block count and a set bit; the create block
  header carries the "this is you" bit in front of the movement update; the values of an object are a list of
  entity fragments (the object data fragment 2 and the type tags 200 to 209, closed by 255) followed by the update
  field structures, value updates carry the owner flag, the unchanged fragment list and the contents mask in front
  of the changed type mask.
- Glue screen of 11.x: the warband scene collection (SMSG_ACCOUNT_ITEM_COLLECTION_DATA, type 7) behind the
  character list, SMSG_BATTLE_NET_CONNECTION_STATUS behind the tutorials, CMSG_REALM_NAME_QUERY is answered before
  the login (virtual realm address), CMSG_BATTLENET_REQUEST (resource service) is answered as not implemented; the
  client does not send CMSG_PLAYER_LOGIN while one of these is unanswered.

Open:

- the client data records (CMSG_DB_QUERY_BULK) are answered as not available for every table; the 11.2.7 Item and
  ItemSparse record layouts are not written yet
- the trait configurations, pvp brackets, warband groups, bit vectors (completed quests) and housing data are sent
  empty
- CMSG_HOTFIX_REQUEST above the 0x10000 packet limit is rejected (the client sends it only with announced hotfixes)
- the vendor fragment (17, vendor data) of vendor creatures is not written
- unhandled 11.x opcodes of the login (CMSG_SETUP_WARBAND_GROUPS, CMSG_GET_ACCOUNT_CHARACTER_LIST,
  CMSG_REORDER_CHARACTERS, CMSG_GET_GARRISON_INFO, CMSG_VIOLENCE_LEVEL, CMSG_QUEUED_MESSAGES_END, ...) are ignored
- the 10.x open items apply as well

## Client

The connection patcher is not used: the client is started through the Arctium WoW Launcher in a version that
supports 11.2.7, with the original 65299 executable, `.build.info` next to it and `SET portal "127.0.0.1"` in
`WTF\Config.wtf`. The launcher replaces the Battle.net keys, the portal suffix, the versions address and the
certificate bundle in memory and sets the public key of the Ed25519 pair in
`src/version/Shared/World/WorldConnectKey.hpp`.

Like the 10.x client the 11.x client verifies the certificate of the web login against the system store, so
the Battle.net server sends it the login url with the `http://` scheme, answers on the web auth port without
TLS and logs the account in with the SRP challenge and proof (`AE_BNET_PASSWORD_WEB_LOGIN` in
`src/battlenet/Server/BNetProtocol.hpp`).
