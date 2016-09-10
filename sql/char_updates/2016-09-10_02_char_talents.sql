--
-- Update characters table
--
ALTER TABLE `characters` ADD COLUMN `firsttalenttree` INT AFTER `talentpoints`;

--
-- Update character_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2016-09-10_02_char_talents' WHERE `LastUpdate` = '2016-09-10_01_mailbox';
