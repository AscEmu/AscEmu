--
-- Add table banned_char_log
--
ALTER TABLE `clientaddons` ADD `timestamp` TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL AFTER showinlist;
ALTER TABLE `clientaddons` ADD `version` VARCHAR(60) DEFAULT NULL AFTER timestamp;

--
-- Update char_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2017-06-25_01_clienbtaddons' WHERE `LastUpdate` = '2017-04-22_01_banned_char_log';
