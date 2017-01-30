--
-- Remove spwns for map 543
--
INSERT INTO worldstring_tables (entry, text) VALUES ('81', 'You must have the quest, \'%s\' completed to pass through here.');

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate`='2017-01-30_01_worldstring_tables' WHERE  `LastUpdate`='2017-01-28_01_creature_spawns';
