# Midnight protocol profile

Midnight 12.1.5 clients log in through Battle.net with the protobuf services and select the Battle.net server
topology like The War Within 11.2.7.

Pinned client:

- Version: 12.1.5
- Build: 70391

`World/WorldProfile.hpp` holds the values of this client for the AES world connection
(`src/version/Shared/World/WorldSocketAes.*`): line feed terminated V2 initializers, the wide transport of 11.x
(32 bit opcodes, 32 byte challenges, SHA512 digests, AES-256-GCM) and the build auth keys of the four client
variants.

## Status

Written against the 12.1.5 reference on 2026-10-15, NOT compiled and NOT tested with a client yet. The target
starts as the 11.2.7 profile: every 11.x branch (`isTheWarWithinOrLater()`, the `TheWarWithin || Midnight`
conditions) serves the 12.x client until a 12.1.5 difference is ported on top.

Live on 2026-10-15 with the Burralis launcher: Battle.net login, realm list, character list and the world
login handshake work. The client hung on the loading screen until the 12.1.5 object values were ported (see
below); that port is NOT compiled and NOT tested yet.

Ported 12.1.5 differences: the Battle.net v2 services with the newer Variant layout, the client instance id in
every header and the realm list JSON (`cfgContentSetID`, `useBleepChance`); SMSG_AUTH_RESPONSE (game time info
and virtual realms before the time and the availabilities); the character list (visual item order, realm queue,
the rpe reason, class unlock lists of the race unlocks); SMSG_CONNECT_TO (no RSA signature, connect payloads);
SMSG_MESSAGECHAT (32 bit flags, the encounter event bit); the movement info (one 64 bit flag field, the gravity
modifier behind the move index, the optional blocks in the order standing-on game object, transport, fall,
inertia, advanced flying, drive status; `movementFlags64()` maps the 16 bit extra flags onto bit 32 and up); the
monster move spline (id, spline, then the bits and the mover position); the create block of SMSG_UPDATE_OBJECT
(the pause times count follows the flag bits, the position on a transport precedes the stationary position, the
pause times list closes the block); the spell packets (SMSG_SPELL_START and SMSG_SPELL_GO carry the target data
behind the cast time, `SpellCastDataMidnight.h`; SMSG_AURA_UPDATE carries the unit before the auras;
SMSG_LEARNED_SPELLS carries the first action bar slot and the trait bit; SMSG_SPELL_FAILURE, SMSG_SPELL_FAILED_OTHER
and SMSG_CAST_FAILED end with the guid of the unit that failed the cast); the 16 bit difficulties of
SMSG_WORLD_SERVER_INFO and SMSG_SET_DUNGEON_DIFFICULTY; SMSG_FEATURE_SYSTEM_STATUS and its glue screen variant (the
bits of 12.x, the quick join and squelch values before the bits); the combat logs (reflecting spell of the non melee
damage log, the log data bit of the periodic aura log behind the effects); SMSG_GOSSIP_MESSAGE (options and quests
before the text bits, treasure behind the language); SMSG_ITEM_PUSH_RESULT (item instance behind the slots);
SMSG_MOVE_TELEPORT (preload world as bit); SMSG_QUERY_ITEM_TEXT_RESPONSE (item before the text); the object values
(`src/world/Objects/ObjectUpdateMidnight.cpp`, generated from the 11.2.7 writer by the `midnight_updatefields.py`
tooling): entity fragment 3 for the object values, UnitData 230 bits (two battle pet decor guids, five stats,
nameplate distance and auto attack range modifiers), PlayerData 326 bits (random customizations, the quest log
index map, the dungeon score and leaver info in the byte section, the npc as player info, plunderstorm item
displays), ActivePlayerData 398 bits (300 skill slots, house types, unlocked transmog outfits, pvp spell
modifiers by label, discord info, level link info, transmog cost min scaling level, the delve data and transmog
outfit maps, the viewed outfit and the transmog metadata), VisibleItem with modified appearance, transmog slot
option, sheathe category and the transmog and illusion bits, ItemData with the modifier list (seven bit size)
before the artifact powers, GameObjectData with the second flag field, AreaTriggerData with the six curves in
front and the period modifier.

Known 12.1.5 differences still to port (from a diff of every `Write()` body of the reference between 11.2.7 and
12.1.5; the pattern of 12.x is that nested structures are written at their field position instead of behind the
bit block):

- loot, quest giver, vendor and player choice packets: our writers have no 7.x+ branch at all, the 12.x layouts
  (items and texts before the bits, the quest objectives before the strings, the race mask as three words,
  `RewardFavor`) are documented in the reference `LootPackets.cpp`, `QuestPackets.cpp`, `NPCPackets.cpp`
- SMSG_PHASE_SHIFT_CHANGE (8 bit phase flags), SMSG_RAID_INSTANCE_MESSAGE and the instance locks (16 bit
  difficulty, map before the instance id), SMSG_UPDATE_ACCOUNT_DATA, SMSG_DISPLAY_TOAST (32 bit method), the
  channel, calendar, guild, inspect, lfg, mail, party, auction, black market, battle pet, scenario, talent and
  trait packets
- SMSG_NEW_WORLD carries a 64 bit instance id (the packet has no 6.x+ branch at all)
- the vendor entity fragment (18, `FVendor_C`) of vendors
- the opcodes of 12.1.5 that have no 11.2.7 name (1021 names, see the opcode report of the tooling)

## Client

The Burralis Game Launcher starts the original 70391 executable with `SET portal "<host>"` in `WTF\Config.wtf`.
It expects a certificate of the Battle.net server that is valid for that host name, so the Battle.net server has
to run with a certificate the client machine trusts (an own CA in the trust store of the machine or a public
certificate for a real host name). The launcher's own readme lists 12.0.5 as the newest supported mainline
version.
