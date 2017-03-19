-- ----------------------------
-- Change columns size/type worldstring_tables
-- ----------------------------
ALTER TABLE `worldstring_tables` CHANGE COLUMN `entry` `entry` INT(11) NOT NULL FIRST;
ALTER TABLE `worldstring_tables` AUTO_INCREMENT=0;
ALTER TABLE `worldstring_tables` COLLATE='utf8_unicode_ci';

--
-- Update entry worldstring_tables
--
UPDATE `worldstring_tables` SET `entry`='0' WHERE `entry`=1129;

--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-03-18_01_misc' WHERE `LastUpdate` = '2016-03-14_01_creature_spawns';