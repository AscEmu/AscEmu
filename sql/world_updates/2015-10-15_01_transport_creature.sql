--
-- Delete old table 'transport_creatures'
--

DROP TABLE IF EXISTS `transport_creatures`;

UPDATE `world_db_version` SET `LastUpdate` = '2015-10-15_01_transport_creature' WHERE `LastUpdate` = '2015-09-15_01_gameobject_spawns';
