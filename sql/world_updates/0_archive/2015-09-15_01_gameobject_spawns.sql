--
-- Delete invalid spawn at Elwyn Forest.
--
DELETE FROM gameobject_spawns WHERE entry = '180184';

UPDATE `world_db_version` SET `LastUpdate` = '2015-09-15_01_gameobject_spawns' WHERE `LastUpdate` = '2015-09-07_01_gameobject_names';
