--
-- Data lfg_fungeon_rewards thx TC
--
DELETE FROM `lfg_dungeon_rewards` WHERE `dungeon_id` = 258 AND `max_level` = 20;

INSERT INTO `lfg_dungeon_rewards`
   (`dungeon_id`, `max_level`, `quest_id_1`, `money_var_1`, `xp_var_1`, `quest_id_2`, `money_var_2`, `xp_var_2`)
VALUES
   (258, 15, 24881, 0, 0, 24889, 0, 0),
   (258, 25, 24882, 0, 0, 24890, 0, 0),
   (258, 34, 24883, 0, 0, 24891, 0, 0),
   (258, 45, 24884, 0, 0, 24892, 0, 0),
   (258, 55, 24885, 0, 0, 24893, 0, 0),
   (258, 60, 24886, 0, 0, 24894, 0, 0),
   (259, 64, 24887, 0, 0, 24895, 0, 0),
   (259, 70, 24888, 0, 0, 24896, 0, 0),
   (260, 70, 24922, 0, 0, 24923, 0, 0),
   (261, 80, 24790, 0, 0, 24791, 0, 0),
   (262, 80, 24788, 0, 0, 24789, 0, 0),
   (288, 80, 25485, 0, 0, 0, 0, 0),
   (287, 80, 25483, 0, 0, 0, 0, 0),
   (286, 80, 25484, 0, 0, 0, 0, 0),
   (285, 80, 25482, 0, 0, 0, 0, 0);

--
-- Update world_db_version
--

UPDATE `world_db_version` SET `LastUpdate` = '2015-10-25_01_lfg_dungeon_rewards' WHERE `LastUpdate` = '2015-10-22_06_misc_updates';
