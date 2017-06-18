--
-- Update Dark Portal target position
--
UPDATE `areatriggers` SET `position_x` = -248.113, `position_y` = 922.9, `position_z` = 84.3497, `orientation` = 1.5708 WHERE `entry` = 4354;


UPDATE `world_db_version` SET `LastUpdate` = '2017-05-05_02_areatriggers' WHERE `LastUpdate` = '2017-05-05_01_playercreateinfo';
