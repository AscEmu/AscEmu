SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `logon_db_version`
-- ----------------------------
DROP TABLE IF EXISTS `logon_db_version`;
CREATE TABLE `logon_db_version` (
  `id` smallint NOT NULL AUTO_INCREMENT,
  `LastUpdate` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='LogonDB version';

-- ----------------------------
-- Records of logon_db_version
-- ----------------------------
INSERT INTO `logon_db_version` VALUES ('1', '20180722-00_realms');
INSERT INTO `logon_db_version` VALUES ('2', '20180729-00_logon_db_version');
INSERT INTO `logon_db_version` VALUES ('3', '20180810-00_realms');
INSERT INTO `logon_db_version` VALUES ('4', '20200221-00_utf8mb4_unicode_ci');
INSERT INTO `logon_db_version` VALUES ('5', '20250119-00_logon_db_version');
