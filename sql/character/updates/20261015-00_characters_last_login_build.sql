ALTER TABLE `characters` ADD COLUMN `last_login_build` int unsigned NOT NULL DEFAULT '0' COMMENT 'client build of the last login, 0 = unknown (before the level squish)';

INSERT INTO `character_db_version` (`id`, `LastUpdate`) VALUES ('19', '20261015-00_characters_last_login_build');
