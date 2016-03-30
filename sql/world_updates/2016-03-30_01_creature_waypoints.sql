--
-- Update creature_waypoints for Lieutnant Sinclari
--
UPDATE `creature_waypoints` SET `waittime` = 3000, `forwardemoteoneshot` = 1, `forwardemoteid` = 69 WHERE `spawnid` = 125515 AND `waypointid` = 1;
UPDATE `creature_waypoints` SET `waittime` = 2000 WHERE `spawnid` = 125515 AND `waypointid` = 2;
UPDATE `creature_waypoints` SET `waittime` = 5000, `forwardemoteoneshot`= 1, `forwardemoteid` = 69 WHERE `spawnid` = 125515 AND `waypointid` = 4;
INSERT INTO `creature_waypoints` (`spawnid`, `waypointid`, `position_x`, `position_y`, `position_z`, `waittime`, `flags`, `forwardemoteoneshot`, `forwardemoteid`, `backwardemoteoneshot`, `backwardemoteid`, `forwardskinid`, `backwardskinid`)
VALUES
   (125515, 5, 1818.08, 804.023, 44.36, 0, 0, 0, 0, 0, 0, 0, 0),
   (125515, 6, 1814.70, 804.16, 44.36, 0, 0, 0, 0, 0, 0, 0, 0);

--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-03-30_01_creature_waypoints' WHERE `LastUpdate` = '2016-03-28_01_brew_of_the_month';
