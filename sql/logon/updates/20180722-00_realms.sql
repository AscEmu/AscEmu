SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `realms`
-- ----------------------------
DROP TABLE IF EXISTS `realms`;
CREATE TABLE `realms` (
  `id` int(10) unsigned NOT NULL,
  `status` tinyint(1) unsigned NOT NULL,
  `status_change_time` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00' ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
