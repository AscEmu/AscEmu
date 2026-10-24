CREATE TABLE `character_customizations` (
  `guid` int unsigned NOT NULL COMMENT 'character guid',
  `option_id` int unsigned NOT NULL COMMENT 'ChrCustomizationOption id',
  `choice_id` int unsigned NOT NULL DEFAULT 0 COMMENT 'ChrCustomizationChoice id',
  PRIMARY KEY (`guid`,`option_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Player System';

INSERT INTO `character_db_version` (`id`, `LastUpdate`) VALUES ('20', '20261024-00_character_customizations');
