-- 1 table: every request a realm sends to the logonserver over LogonComm
CREATE TABLE IF NOT EXISTS `logoncomm_log` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL,
  `realm_ids` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `remote_ip` varchar(45) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `opcode` smallint unsigned NOT NULL DEFAULT '0',
  `opcode_name` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `account_id` int unsigned NOT NULL DEFAULT '0',
  `detail` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `time` (`time`),
  KEY `account_id` (`account_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `logon_db_version` (`id`, `LastUpdate`) VALUES (9, '20261025-01_logoncomm_log');
