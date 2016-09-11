--
-- Copy wp table and remove most data in our table
--
CREATE TABLE `creature_waypoints_copy` LIKE `creature_waypoints`; 
INSERT `creature_waypoints_copy` SELECT * FROM `creature_waypoints`;

DELETE FROM `creature_waypoints` WHERE `spawnid` <> 24253;

--
-- Records of spell_required
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-09-11_01_waypoint_copy' WHERE `LastUpdate` = '2016-09-09_01_trainer';
