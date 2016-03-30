--
-- Set Violet Hold portals visible
--
UPDATE `creature_proto` SET `invisibility_type` = 0 WHERE `entry` = 31011;

--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-03-30_02_creature_proto' WHERE `LastUpdate` = '2016-03-30_01_creature_waypoints';
