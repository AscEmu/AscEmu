
-- npc_gossip_texts had 349 rows (across all 8 pages, not just page 0) whose language column was
-- set to 7 (LANG_COMMON - the Alliance human language) for text that is clearly meant to be
-- understood by every player (bank tellers, innkeepers, ship captains, generic trainer/vendor
-- lines, etc). Any Horde player - or Alliance player without the Common skill for some other
-- reason - sees this text rendered as unreadable gibberish by the client, since the client
-- garbles text tagged with a language the reader doesn't know. Fixed to Universal (0), which
-- every player always understands, matching the language used by the overwhelming majority of
-- the table's other rows.
UPDATE `npc_gossip_texts` SET `lang0` = 0 WHERE `lang0` = 7;
UPDATE `npc_gossip_texts` SET `lang1` = 0 WHERE `lang1` = 7;
UPDATE `npc_gossip_texts` SET `lang2` = 0 WHERE `lang2` = 7;
UPDATE `npc_gossip_texts` SET `lang3` = 0 WHERE `lang3` = 7;
UPDATE `npc_gossip_texts` SET `lang4` = 0 WHERE `lang4` = 7;
UPDATE `npc_gossip_texts` SET `lang5` = 0 WHERE `lang5` = 7;
UPDATE `npc_gossip_texts` SET `lang6` = 0 WHERE `lang6` = 7;
UPDATE `npc_gossip_texts` SET `lang7` = 0 WHERE `lang7` = 7;

-- Warrior trainers' "you're not a warrior" rejection text (entry 5721) has 4 unused
-- race-flavored sibling variants sitting right next to it in the same table (5722-5725, real
-- Blizzard text, previously referenced by zero trainers) that trainer_properties never wired up -
-- every one of the 37 Warrior trainers pointed at the same generic (Human-flavored) 5721
-- regardless of the trainer's own race. Reassigning the three variants with an unambiguous
-- thematic/dialect match to their matching trainers (Undead "darkness"-themed, Dwarven "ye/yer"
-- dialect, Tauren spiritual "child" address). The remaining trainers (Human, Orc, Night Elf,
-- Draenei) keep the generic 5721, now Universal per the fix above - there's no equally
-- unambiguous variant for them among 5722-5725, so no reassignment for those races.

-- Undead trainers -> 5722 ("The darkness does not embrace you, $c...")
UPDATE `trainer_properties` SET `cannot_train_gossip_textid` = 5722
    WHERE `entry` IN (2119, 2131, 4593, 4594, 4595) AND `cannot_train_gossip_textid` = 5721;

-- Dwarf trainers -> 5724 ("Yer no warrior... Why ye wouldn't even last a day...")
UPDATE `trainer_properties` SET `cannot_train_gossip_textid` = 5724
    WHERE `entry` IN (912, 1229, 1901, 5113) AND `cannot_train_gossip_textid` = 5721;

-- Tauren trainers -> 5725 ("I do not see the warrior's spirit within you, child...")
UPDATE `trainer_properties` SET `cannot_train_gossip_textid` = 5725
    WHERE `entry` IN (3041, 3042, 3043, 3059, 3063) AND `cannot_train_gossip_textid` = 5721;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('189', '20260905-05_fix_npc_gossip_text_language');
