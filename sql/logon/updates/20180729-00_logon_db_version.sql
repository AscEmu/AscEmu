SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `logon_db_version`
-- ----------------------------
DROP TABLE IF EXISTS `logon_db_version`;
CREATE TABLE `logon_db_version` (
  `LastUpdate` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`LastUpdate`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of logon_db_version
-- ----------------------------
INSERT INTO `logon_db_version` VALUES ('20180729-00_logon_db_version');
