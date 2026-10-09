# BfA protocol profile

BfA 8.3.7 clients log in through Battle.net with the protobuf services (the same v1 services as Legion 7.3.5)
and select the Battle.net server topology.

Pinned client:

- Version: 8.3.7
- Build: 35662

`World/WorldProfile.hpp` holds the values of this client for the AES world connection
(`src/version/Shared/World/WorldSocketAes.*`): line feed terminated V2 initializers, the 16 byte header
(size and AES-GCM tag) in both directions, the build auth seeds of the two client platforms and
SMSG_ENTER_ENCRYPTED_MODE / CMSG_ENTER_ENCRYPTED_MODE_ACK before the AES-128-GCM encryption starts.
Everything after authentication goes through the central opcode table and the managed packets.

## Client preparation

Redistributed 8.3.7 clients expect the CASC build information in `.fzbuildnfo` next to the executable (the
8.3.7 columns, including `KeyRing` and `Product` = `wow`); a `.flavor.info` next to the executable switches
the client to the Battle.net layout with `../.fzbuildnfo` and `../Data` and must not exist in the flat layout.
`SET portal "127.0.0.1"` in `WTF/Config.wtf` names the bnetserver, the host suffix is only appended to a
portal value without a dot.

The connection patcher (`src/tools/connection`) accepts build 35662. A client that already carries the known
SMSG_CONNECT_TO key is treated as prepared: only the versions address and the bundle address are changed,
the bundle signing key and the launcher parameters stay untouched because such clients do not verify the
bundle and close or crash when these are changed; their own portal suffix stays, a dotted portal value
never gets it appended. The bundle signing key is kept as
`web_cert_bundle.key.pem` next to the server certificate, so 7.3.5 and 8.3.7 clients accept the same bundle
in the shared Battle.net cache (`%ProgramData%\Blizzard Entertainment\Battle.net\Cache\web_cert_bundle`).
A client whose bundle refresh fails removes that cached file; run the patcher again before starting a
7.3.5 client afterwards.

## Status

Verified with the pinned client: Battle.net login (v1 services and web login), realm list and realm join,
AES-GCM world connection with the second instance connection, character list, world entry with the 8.x
update field structures, movement, camera and interface.

Open:

- handlers that are not enabled for 6.x, 7.x and 8.x clients because their packet layouts changed
  (quest query, raid info, cemetery list, calendar, guild bank, battle pets, conquest constants)
- hotfix item records are still written with the 7.3.5 writers
- 185 renamed 8.3.7 opcodes without an internal name (battle pay, tokens, artifacts, calendar)
- SpellCastResult and InventoryResult values of 8.x are not renumbered
- SMSG_QUERY_PLAYER_NAME_RESPONSE repeats the name as declined names
- the certificate bundle in the shared Battle.net cache is removed by an 8.3.7 client whose refresh fails
