--
-- Add column orientation to playercreateinfo
--
ALTER TABLE `playercreateinfo` ADD `orientation` FLOAT(0) DEFAULT 0 AFTER `positionZ`;

UPDATE `playercreateinfo` SET `orientation` = 6.17716 WHERE `race` = 3;
UPDATE `playercreateinfo` SET `orientation` = 5.69632 WHERE `race` = 4;
UPDATE `playercreateinfo` SET `orientation` = 5.31605 WHERE `race` = 10;
UPDATE `playercreateinfo` SET `orientation` = 2.08364 WHERE `race` = 11;

UPDATE `world_db_version` SET `LastUpdate` = '2017-05-05_01_playercreateinfo' WHERE `LastUpdate` = '2017-02-25_01_gameobject_spawns';
