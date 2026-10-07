CREATE TABLE IF NOT EXISTS `character_trait_config` (
  `guid` int unsigned NOT NULL,
  `config_id` int NOT NULL,
  `config_type` int NOT NULL,
  `specialization_id` int NOT NULL DEFAULT 0,
  `combat_config_flags` int NOT NULL DEFAULT 0,
  `local_identifier` int NOT NULL DEFAULT 0,
  `skill_line_id` int NOT NULL DEFAULT 0,
  `trait_system_id` int NOT NULL DEFAULT 0,
  `variation_id` int NOT NULL DEFAULT 0,
  `name` varchar(127) NOT NULL DEFAULT '',
  `saved_config_id` int NOT NULL DEFAULT 0,
  `saved_local_identifier` int NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`,`config_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `character_trait_config_subtree` (
  `guid` int unsigned NOT NULL,
  `config_id` int NOT NULL,
  `subtree_id` int NOT NULL,
  `active` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`,`config_id`,`subtree_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `character_trait_config_entry` (
  `guid` int unsigned NOT NULL,
  `config_id` int NOT NULL,
  `subtree_id` int NOT NULL DEFAULT 0,
  `trait_node_id` int NOT NULL,
  `trait_node_entry_id` int NOT NULL,
  `rank` int NOT NULL DEFAULT 0,
  `granted_ranks` int NOT NULL DEFAULT 0,
  `bonus_ranks` int NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`,`config_id`,`subtree_id`,`trait_node_id`,`trait_node_entry_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
