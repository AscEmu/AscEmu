--
-- Change column size rbg_daily
--
ALTER TABLE `characters` MODIFY `rbg_daily` TINYINT(1) NOT NULL DEFAULT '0' COMMENT 'Boolean already done a daily rbg?';

UPDATE `character_db_version` SET `LastUpdate` = '2015-09-17_01_characters' WHERE `LastUpdate` = '2015-09-14_01_rbg_daily';
