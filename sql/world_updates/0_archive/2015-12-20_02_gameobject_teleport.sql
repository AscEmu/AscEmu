--
-- Add two additional fields
--
ALTER TABLE `gameobject_teleports` ADD required_class tinyint(2) NOT NULL DEFAULT '0' AFTER required_level;
ALTER TABLE `gameobject_teleports` ADD required_achievement int(4) NOT NULL DEFAULT '0' AFTER required_class;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2015-12-20_02_gameobject_teleport' WHERE `LastUpdate` = '2015-12-20_01_misc_update';
