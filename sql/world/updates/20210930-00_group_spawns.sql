

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('92', '20210930-00_group_spawns');

-- ----------------------------
-- Table structure for `spawn_group_id`
-- ----------------------------
DROP TABLE IF EXISTS `spawn_group_id`;
CREATE TABLE `spawn_group_id` (
  `groupId` tinyint NOT NULL,
  `groupName` text NOT NULL,
  `groupFlags` tinyint NOT NULL DEFAULT '0',
  `extraFlags` tinyint DEFAULT NULL,
  `BossId` int DEFAULT NULL,
  PRIMARY KEY (`groupId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of spawn_group_id
-- ----------------------------
INSERT INTO `spawn_group_id` VALUES ('1', 'Hellfire Ramparts Vazruden Trash', '1', '3', '17537');
INSERT INTO `spawn_group_id` VALUES ('2', 'Hellfire Ramparts Bridge', '1', '0', '17537');

-- ----------------------------
-- Table structure for `creature_group_spawn`
-- ----------------------------
DROP TABLE IF EXISTS `creature_group_spawn`;
CREATE TABLE `creature_group_spawn` (
  `groupId` tinyint NOT NULL DEFAULT '0',
  `spawnId` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`groupId`,`spawnId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of creature_group_spawn
-- ----------------------------
INSERT INTO `creature_group_spawn` VALUES ('4', '160105');
INSERT INTO `creature_group_spawn` VALUES ('4', '160106');
INSERT INTO `creature_group_spawn` VALUES ('6', '160044');
INSERT INTO `creature_group_spawn` VALUES ('6', '160045');
