# Shadowlands protocol profile

Shadowlands 9.2.7 clients log in through Battle.net with the protobuf services (the same v1 services as
Legion 7.3.5 and BfA 8.3.7) and select the Battle.net server topology.

Pinned client:

- Version: 9.2.7
- Build: 45745

`World/WorldProfile.hpp` holds the values of this client for the AES world connection
(`src/version/Shared/World/WorldSocketAes.*`): line feed terminated V2 initializers, the 16 byte header
(size and AES-GCM tag) in both directions, the build auth seed of the Windows client and
SMSG_ENTER_ENCRYPTED_MODE / CMSG_ENTER_ENCRYPTED_MODE_ACK before the AES-128-GCM encryption starts.
The seeds of the digest, the session key, the continued session and the encryption key are those of 8.3.7.
Everything after authentication goes through the central opcode table and the managed packets.

SMSG_ENTER_ENCRYPTED_MODE of 9.x carries an Ed25519 signature with a context (64 bytes) instead of the RSA
signature; the key pair and the context are in `src/version/Shared/World/WorldConnectKey.hpp`. SMSG_CONNECT_TO
keeps the RSA signature.

## Status

Verified with the 45745 client started through the Arctium WoW Launcher (2026-10-10): Battle.net login through
the v1 services, realm list, the AES world connection with the Ed25519 signed encryption start, the character
list and entering the world work.

Written against the 9.2.7 reference, not verified in detail yet:

- Character list details (customization list empty, the characters show their default look), character creation
  with the customization choices skipped, 9.x error codes.
- Login packets of 9.x (feature system status, glue screen, account data times with 13 types and 64 bit times,
  world server info, chat with 14 flag bits, emotes with visual kits, player name queries for several players).
- 9.x movement info (plain flags, third flags, inertia bit) in the codec descriptors and the create block,
  SMSG_ON_MONSTER_MOVE of 9.x, update field structures of 9.2.7 in `src/world/Objects/ObjectUpdateShadowlands.*`.
- Spell, aura, combat, query, gossip and world state packets with the 9.x differences (cast visual with two
  values, 16 bit aura flags, content tuning blocks, 400 factions, broadcast texts of sounds).

Open:

- quest giver status values of 9.x differ from 8.x, no value map yet
- SMSG_AVAILABLE_HOTFIXES, SMSG_BATTLE_PET_JOURNAL_LOCK_ACQUIRED and SMSG_SEASON_INFO are not sent
- CMSG_HOTFIX_REQUEST and CMSG_MOVE_SET_COLLISION_HEIGHT_ACK have new layouts in 9.x
- the battle pay purchase list has no 9.2.7 reference and is not sent
- only the Windows auth seed of 45745 is known

## Client

From Shadowlands on the connection patcher (`src/tools/connection`) is not used and rejects build 45745 on
purpose: the client is started through the Arctium WoW Launcher, which replaces the Battle.net keys, the portal
suffix, the versions address and the certificate bundle in memory. A 45745 binary whose bundle signing key is
replaced in the file crashes at login whatever bundle and signature it carries, the launcher avoids that.
The launcher sets the public key of the Ed25519 pair in `src/version/Shared/World/WorldConnectKey.hpp`, so the
signature of SMSG_ENTER_ENCRYPTED_MODE verifies without a client patch.
Use the original executable with `.build.info` next to it (the 8.3.7 columns incl. `KeyRing` and `Product`),
`SET portal "127.0.0.1"` in `WTF\Config.wtf` and the launcher's Battle.net server option. Redistributed
executables with their own protection (launcher-bound builds) reject other servers.
