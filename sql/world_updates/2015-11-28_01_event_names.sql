--
-- Update Arena Seasons occurence and length in event_names
--
UPDATE `event_names` SET `occurence` = 5184000, `length` = 2592000 WHERE `entry` IN(55, 56, 57, 58, 59, 60);

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2015-11-28_01_event_names' WHERE `LastUpdate` = '2015-11-06_03_event_spawns';
