--
-- Change go spawn
--
UPDATE `gameobject_spawns` SET `phase` = 1 WHERE `id` = 51168;
 
--
-- Add creature spawns
--
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
	(143747, 30205, 571, 6468.95, 269.38, 398.06, 3.736, 2, 25267, 2068, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2);
--
-- Add waypoints
--
INSERT INTO `creature_waypoints` (`spawnid`, `waypointid`, `position_x`, `position_y`, `position_z`, `waittime`, `flags`, `forwardemoteoneshot`, `forwardemoteid`, `backwardemoteoneshot`, `backwardemoteid`, `forwardskinid`, `backwardskinid`) VALUES
	(143747, 1, 6481.48, 278.625, 400.013, 0, 0, 1, 0, 1, 0, 25267, 25267),
	(143747, 2, 6494.1, 290.832, 398.645, 0, 0, 1, 0, 1, 0, 25267, 25267),
	(143747, 3, 6520.03, 286.988, 402.588, 0, 0, 1, 0, 1, 0, 25267, 25267),
	(143747, 4, 6545.72, 291.909, 400.06, 0, 0, 1, 0, 1, 0, 25267, 25267),
	(143747, 5, 6545.59, 306.489, 401.244, 0, 0, 1, 0, 1, 0, 25267, 25267),
	(143747, 6, 6531.71, 288.404, 400.013, 0, 0, 1, 0, 1, 0, 25267, 25267),
	(143747, 7, 6521.14, 288.006, 402.656, 0, 0, 1, 0, 1, 0, 25267, 25267),
	(143747, 8, 6494.25, 284.885, 398.642, 0, 0, 1, 0, 1, 0, 25267, 25267),
	(143747, 9, 6467.32, 267.19, 397.927, 0, 0, 1, 0, 1, 0, 25267, 25267),
	(142050, 1, 6550.05, 448.885, 414.553, 0, 0, 1, 0, 1, 0, 23883, 23883),
	(142050, 2, 6562.06, 415.889, 416.354, 0, 0, 1, 0, 1, 0, 23883, 23883),
	(142050, 3, 6597.94, 393.282, 416.214, 0, 0, 1, 0, 1, 0, 23883, 23883),
	(142050, 4, 6627.66, 390.043, 412.154, 0, 0, 1, 0, 1, 0, 23883, 23883),
	(142050, 5, 6647.09, 408.513, 404.966, 0, 0, 1, 0, 1, 0, 23883, 23883),
	(142050, 6, 6615.99, 438.385, 402.948, 0, 0, 1, 0, 1, 0, 23883, 23883),
	(142050, 7, 6591.4, 461.873, 403.622, 0, 0, 1, 0, 1, 0, 23883, 23883);
 
--
-- Update world_db_version
--

UPDATE `world_db_version` SET `LastUpdate` = '2015-10-25_03_misc_spawns' WHERE `LastUpdate` = '2015-10-25_02_misc_spawns';
