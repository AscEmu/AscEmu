--
-- Modify stationary
--
ALTER TABLE `mailbox` MODIFY `stationary` BIGINT(20) unsigned DEFAULT '0';

--
-- Update char_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2017-03-20_01_mailbox' WHERE `LastUpdate` = '2017-02-04_01_queue_tables';
