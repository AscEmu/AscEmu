--
-- Add column rbg_daily to table characters
--

ALTER TABLE characters ADD COLUMN rbg_daily INT DEFAULT 0;

UPDATE `character_db_version` SET `LastUpdate` = '2015-09-14_01_rbg_daily';
