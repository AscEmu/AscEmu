ALTER TABLE `characters` ADD COLUMN `talentspecialization` VARCHAR(50) NOT NULL DEFAULT '0 0';

INSERT INTO `character_db_version` (`id`, `LastUpdate`) VALUES ('16', '20260820-00_characters_talent_specialization');
