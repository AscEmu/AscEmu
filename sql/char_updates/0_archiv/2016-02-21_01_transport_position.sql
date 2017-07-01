--
-- Add new column to save transport orientation.
--
ALTER TABLE `characters` ADD transporter_odiff FLOAT NOT NULL DEFAULT 0 AFTER transporter_zdiff;

--
-- Update char_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2016-02-21_01_transport_position';
