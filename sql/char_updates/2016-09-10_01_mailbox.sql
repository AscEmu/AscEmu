--
-- Create table gm_survey
--
ALTER TABLE `mailbox` MODIFY `stationary` BIGINT(20) unsigned DEFAULT '0';

--
-- Update char_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2016-09-10_01_mailbox' WHERE `LastUpdate` = '2016-03-28_01_gm_survey';