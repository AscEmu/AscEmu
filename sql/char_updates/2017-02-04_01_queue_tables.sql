--
-- Drop unused queue tables
--
DROP TABLE IF EXISTS `mailbox_insert_queue`;
DROP TABLE IF EXISTS `playeritems_insert_queue`;
DROP TABLE IF EXISTS `characters_insert_queue`;

--
-- Update char_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2017-02-04_01_queue_tables' WHERE `LastUpdate` = '2016-03-28_01_gm_survey';
