--
-- Modify characters
--
ALTER TABLE `characters` ADD COLUMN `firsttalenttree` INT AFTER `talentpoints`;

--
-- Update char_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2017-03-21_01_characters' WHERE `LastUpdate` = '2017-03-20_01_mailbox';
