SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `character_db_version`
-- ----------------------------
DROP TABLE IF EXISTS `character_db_version`;
CREATE TABLE `character_db_version` (
  `id` smallint NOT NULL AUTO_INCREMENT,
  `LastUpdate` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=13 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='CharacterDB version';

-- ----------------------------
-- Records of character_db_version
-- ----------------------------
INSERT INTO `character_db_version` VALUES ('1', '20180427-00_character_db_version');
INSERT INTO `character_db_version` VALUES ('2', '20180709-00_guilds');
INSERT INTO `character_db_version` VALUES ('3', '20180714-00_guild_tables');
INSERT INTO `character_db_version` VALUES ('4', '20180916-00_guild_tables');
INSERT INTO `character_db_version` VALUES ('5', '20200221-00_utf8mb4_unicode_ci');
INSERT INTO `character_db_version` VALUES ('6', '20201216-00_rename_event_saves');
INSERT INTO `character_db_version` VALUES ('7', '20211123-00_character_void_storage');
INSERT INTO `character_db_version` VALUES ('8', '20220116-00_characters');
INSERT INTO `character_db_version` VALUES ('9', '20220414-00_instances');
INSERT INTO `character_db_version` VALUES ('10', '20220415-00_account_instance_times');
INSERT INTO `character_db_version` VALUES ('11', '20230710-00_characters_taxi');
INSERT INTO `character_db_version` VALUES ('12', '20241206-00_playerpets');
INSERT INTO `character_db_version` VALUES ('13', '20250119-00_character_db_version');
