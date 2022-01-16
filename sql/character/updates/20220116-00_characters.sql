ALTER TABLE `characters` ADD COLUMN `bindpositionO` FLOAT NOT NULL DEFAULT 0 AFTER `bindpositionZ`;

UPDATE `character_db_version` SET LastUpdate = '20220116-00_characters';
