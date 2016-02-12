--
-- Drop column difficulty and add column dungeon_difficulty and raid_difficulty to table characters
--
ALTER TABLE `characters` DROP difficulty;
ALTER TABLE `characters` ADD dungeon_difficulty SMALLINT(1) unsigned NOT NULL DEFAULT 0 AFTER rbg_daily;
ALTER TABLE `characters` ADD raid_difficulty SMALLINT(1) unsigned NOT NULL DEFAULT 0 AFTER dungeon_difficulty;

--
-- Update char_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2016-02-11_01_difficulty';
