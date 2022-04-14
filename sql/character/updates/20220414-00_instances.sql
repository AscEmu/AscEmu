SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `character_instance`
-- ----------------------------
DROP TABLE IF EXISTS `character_instance`;
CREATE TABLE `character_instance` (
  `guid` int unsigned NOT NULL DEFAULT '0',
  `instance` int unsigned NOT NULL DEFAULT '0',
  `permanent` tinyint unsigned NOT NULL DEFAULT '0',
  `extendState` tinyint unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`guid`,`instance`),
  KEY `instance` (`instance`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

-- ----------------------------
-- Table structure for `group_instance`
-- ----------------------------
DROP TABLE IF EXISTS `group_instance`;
CREATE TABLE `group_instance` (
  `guid` int unsigned NOT NULL DEFAULT '0',
  `instance` int unsigned NOT NULL DEFAULT '0',
  `permanent` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`instance`),
  KEY `instance` (`instance`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

-- ----------------------------
-- Table structure for `instance`
-- ----------------------------
DROP TABLE IF EXISTS `instance`;
CREATE TABLE `instance` (
  `id` int unsigned NOT NULL DEFAULT '0',
  `map` smallint unsigned NOT NULL DEFAULT '0',
  `resettime` bigint unsigned NOT NULL DEFAULT '0',
  `difficulty` tinyint unsigned NOT NULL DEFAULT '0',
  `completedEncounters` int unsigned NOT NULL DEFAULT '0',
  `data` tinytext NOT NULL,
  PRIMARY KEY (`id`),
  KEY `map` (`map`),
  KEY `resettime` (`resettime`),
  KEY `difficulty` (`difficulty`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

-- ----------------------------
-- Table structure for `respawn`
-- ----------------------------
DROP TABLE IF EXISTS `respawn`;
CREATE TABLE `respawn` (
  `type` smallint unsigned NOT NULL,
  `spawnId` int unsigned NOT NULL,
  `respawnTime` bigint unsigned NOT NULL,
  `mapId` smallint unsigned NOT NULL,
  `instanceId` int unsigned NOT NULL,
  PRIMARY KEY (`type`,`spawnId`,`instanceId`),
  KEY `idx_instance` (`instanceId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3 COMMENT='Stored respawn times';

-- ----------------------------
-- Table structure for `instance_reset`
-- ----------------------------
DROP TABLE IF EXISTS `instance_reset`;
CREATE TABLE `instance_reset` (
  `mapid` smallint unsigned NOT NULL DEFAULT '0',
  `difficulty` tinyint unsigned NOT NULL DEFAULT '0',
  `resettime` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`mapid`,`difficulty`),
  KEY `difficulty` (`difficulty`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb3;

DROP TABLE IF EXISTS `instanceids`;
DROP TABLE IF EXISTS `instances`;

UPDATE `character_db_version` SET LastUpdate = '20220414-00_instances';
