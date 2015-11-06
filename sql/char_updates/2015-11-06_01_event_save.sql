--
-- Update event_tables columns
--
ALTER TABLE `event_save` CHANGE `eventEntry` `event_entry` INT(3) unsigned NOT NULL;

--
-- Update character_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2015-11-06_01_event_save' WHERE `LastUpdate` = '2015-09-17_01_characters';
