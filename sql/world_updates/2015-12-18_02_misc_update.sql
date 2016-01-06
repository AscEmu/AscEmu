--
-- Willard Blauvelt (Sitting on the ground)
--
UPDATE `creature_spawns` SET `bytes1`=1 WHERE `id`=130394;

--
-- Dark Iron Kidnapper
--
INSERT INTO `creature_spawns` VALUES (143791, 15692, 0, -6600.01, -1848.25, 244.266, 2.61101, 2, 7792, 54, 32768, 16777475, 0, 4097, 0, 0, 0, 0, 0, 0, 0, 0, 15215, 0, 0, 0, 1);
INSERT INTO `creature_waypoints`
VALUES
   (143791, 1, -6600.01, -1848.25, 244.266, 10000, 0, 1, 0, 1, 0, 7792, 7792),
   (143791, 2, -6616.01, -1850.34, 244.152, 10000, 0, 1, 0, 1, 0, 7792, 7792),
   (143791, 3, -6614.67, -1864.57, 244.207, 10000, 0, 1, 0, 1, 0, 7792, 7792),
   (143791, 4, -6602.01, -1863.24, 244.694, 10000, 0, 1, 0, 1, 0, 7792, 7792);

--
-- Dark Iron Geologist
--
DELETE FROM `creature_spawns` WHERE `id`= 41118;
DELETE FROM `creature_waypoints` WHERE `spawnid`= 41118;

INSERT INTO `creature_spawns` VALUES (41118, 5839, 0, -6604.43, -1864.63, 244.62, 4.62512, 2, 7793, 54, 32768, 2048, 0, 4097, 0, 0, 0, 0, 0, 0, 0, 0, 1907, 0, 0, 0, 1);
INSERT INTO `creature_waypoints`
VALUES
   (41118, 1, -6604.43, -1864.63, 244.62, 10000, 0, 1, 0, 1, 0, 7793, 7793),
   (41118, 2, -6611.68, -1870.47, 244.35, 10000, 0, 1, 0, 1, 0, 7793, 7793),
   (41118, 3, -6599.88, -1878.65, 244.91, 10000, 0, 1, 0, 1, 0, 7793, 7793),
   (41118, 4, -6594.51, -1870.84, 245.16, 10000, 0, 1, 0, 1, 0, 7793, 7793);

--
-- Tarren Mill Deathguard waypoints and spawn update
--
UPDATE `creature_spawns` SET `movetype`=2 WHERE `id`=18929;
UPDATE `creature_waypoints` SET `position_x`=-25.61, `position_y`=-900.25, `position_z`=55.91 WHERE `spawnid`=18929 AND `waypointid`=2;
UPDATE `creature_waypoints` SET `position_x`=-35.92, `position_y`=-910.73, `position_z`=55.69 WHERE `spawnid`=18929 AND `waypointid`=3;
UPDATE `creature_waypoints` SET `position_x`=-38.04, `position_y`=-891.13, `position_z`=56.23 WHERE `spawnid`=18929 AND `waypointid`=4;
UPDATE `creature_spawns` SET `movetype`=2 WHERE `id`=18933;
UPDATE `creature_waypoints` SET `position_x`=-20.33, `position_y`=-916.82 WHERE `spawnid`=18933 AND `waypointid`=2;
UPDATE `creature_waypoints` SET `position_x`=-18.55, `position_y`=-907.51, `position_z`=55.92 WHERE `spawnid`=18933 AND `waypointid`=3;
UPDATE `creature_waypoints` SET `position_x`=-33.47, `position_y`=-887.31, `position_z`=56.09 WHERE `spawnid`=18933 AND `waypointid`=4;
UPDATE `creature_waypoints` SET `position_x`=14.71, `position_y`=-937.35, `position_z`=57.67 WHERE `spawnid`=18928 AND `waypointid`=3;
UPDATE `creature_spawns` SET `position_x`=-26.8365, `position_y`=-940.098, `position_z`=55.0952, `orientation`=1.76872, `movetype`=0 WHERE `id`=18915;
UPDATE `creature_waypoints` SET `position_x`=-26.83, `position_y`=-940.09, `position_z`=55.09 WHERE `spawnid`=18915 AND `waypointid`=1;
UPDATE `creature_waypoints` SET `position_x`=-22.17, `position_y`=-955.51, `position_z`=55.61 WHERE `spawnid`=18915 AND `waypointid`=2;
UPDATE `creature_waypoints` SET `position_x`=5.59, `position_y`=-957.18, `position_z`=56.52 WHERE `spawnid`=18915 AND `waypointid`=3;
UPDATE `creature_waypoints` SET `position_x`=10.04, `position_y`=-938.71, `position_z`=57.28 WHERE `spawnid`=18915 AND `waypointid`=4;
UPDATE `creature_spawns` SET `position_x`=-10.1538, `position_y`=-938.692, `position_z`=61.9336, `orientation`=5.86144, `movetype`=0 WHERE `id`=18916;
UPDATE `creature_waypoints` SET `position_x`=-10.15, `position_y`=-938.69, `position_z`=61.93 WHERE `spawnid`=18916 AND `waypointid`=1;
UPDATE `creature_waypoints` SET `position_x`=-14.91, `position_y`=-930.65, `position_z`=61.91 WHERE `spawnid`=18916 AND `waypointid`=2;
UPDATE `creature_waypoints` SET `position_x`=-10.28, `position_y`=-922.75, `position_z`=57.17 WHERE `spawnid`=18916 AND `waypointid`=3;
UPDATE `creature_waypoints` SET `position_x`=-7.99, `position_y`=-924.35, `position_z`=57.17 WHERE `spawnid`=18916 AND `waypointid`=4;
UPDATE `creature_spawns` SET `position_x`=-45.6004, `position_y`=-966.147, `position_z`=56.1964, `orientation`=1.24743, `movetype`=0 WHERE `id`=69338;
UPDATE `creature_waypoints` SET `position_x`=-45.61, `position_y`=-966.14, `position_z`=56.19 WHERE `spawnid`=69338 AND `waypointid`=1;
UPDATE `creature_waypoints` SET `position_x`=-37.61, `position_y`=-943.51, `position_z`=56.22 WHERE `spawnid`=69338 AND `waypointid`=2;
UPDATE `creature_waypoints` SET `position_x`=-34.41, `position_y`=-933.28, `position_z`=54.31 WHERE `spawnid`=69338 AND `waypointid`=3;
UPDATE `creature_waypoints` SET `position_x`=-51.31 WHERE `spawnid`=69338 AND `waypointid`=4;
UPDATE `creature_waypoints` SET `position_x`=-58.73, `position_y`=-897.78, `position_z`=55.77 WHERE `spawnid`=18932 AND `waypointid`=1;
UPDATE `creature_waypoints` SET `position_y`=-877.31 WHERE `spawnid`=18932 AND `waypointid`=3;
UPDATE `creature_waypoints` SET `position_y`=-983.21, `position_z`=56.51 WHERE `spawnid`=18944 AND `waypointid`= 1;
UPDATE `creature_waypoints` SET `position_x`=-70.31, `position_y`=-994.61, `position_z`=56.04 WHERE `spawnid`=18944 AND `waypointid`= 2;
UPDATE `creature_spawns` SET `position_x`=-18.1684, `position_y`=-922.168, `position_z`=55.6725, `orientation`=2.64483 WHERE `id`=19111;

--
-- Delete event_creature_spawns for event Winter Veil (these spawns are not event related)
--
DELETE FROM `event_creature_spawns` WHERE `id`IN(156767, 156770, 156766, 156768, 156769, 156764);

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2015-12-18_02_misc_update' WHERE `LastUpdate` = '2015-12-18_01_transport_creatures';
