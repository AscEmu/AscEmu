--
-- Add creature spawns
--
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`)
VALUES
   (143770, 31402, 571, 5641.98, 963.093, 173.739, 0.265412, 0, 25195, 103, 0, 256, 0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 36990, 37119, 0, 0, 1),
   (143771, 31403, 571, 5644.68, 935.264, 167.694, 1.894634, 0, 25195, 103, 0, 256, 0, 1, 0, 0, 0, 0, 0, 0, 2, 0, 36990, 37119, 0, 0, 1);
--
-- Update creature_spawns
--
UPDATE `creature_spawns` SET `death_state` = 2 WHERE `id` IN(143724,143725,143716);
UPDATE `creature_spawns` SET `entry` = 31403, `position_x` = 5700.041, `position_y` = 982.128, `position_z` = 174.4812, `orientation` = 1.405 WHERE `id` = 143716;

--
-- Update world_db_version
--

UPDATE `world_db_version` SET `LastUpdate` = '2015-10-25_05_misc_spawns' WHERE `LastUpdate` = '2015-10-25_04_misc_spawns';
