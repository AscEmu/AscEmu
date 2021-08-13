-- ----------------------------
-- Table structure for `creature_ai_scripts`
-- ----------------------------
DROP TABLE IF EXISTS `creature_ai_scripts`;
CREATE TABLE `creature_ai_scripts` (
  `entry` int unsigned NOT NULL,
  `difficulty` tinyint unsigned NOT NULL DEFAULT '0',
  `phase` tinyint unsigned NOT NULL DEFAULT '0',
  `event` tinyint unsigned NOT NULL DEFAULT '0',
  `action` tinyint unsigned NOT NULL DEFAULT '0',
  `maxCount` tinyint unsigned NOT NULL DEFAULT '0',
  `chance` float unsigned NOT NULL DEFAULT '1',
  `spell` int unsigned NOT NULL DEFAULT '0',
  `spell_type` int NOT NULL,
  `triggered` tinyint(1) NOT NULL DEFAULT '0',
  `target` tinyint NOT NULL DEFAULT '0',
  `cooldownMin` int NOT NULL DEFAULT '0',
  `cooldownMax` int unsigned NOT NULL DEFAULT '0',
  `minHealth` float NOT NULL DEFAULT '0',
  `maxHealth` float NOT NULL DEFAULT '100',
  `textId` int unsigned NOT NULL DEFAULT '0',
  `misc1` int NOT NULL DEFAULT '0',
  `comments` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  UNIQUE KEY `entry` (`entry`,`difficulty`,`phase`,`spell`,`event`,`action`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='AI System';

INSERT INTO `world_db_version` VALUES ('83', '20210914-00_creature_ai_scripts');
