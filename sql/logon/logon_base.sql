/*
********************************************************************
AscEmu logon structure
Last update: 10/08/2018
********************************************************************
*/

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

CREATE TABLE IF NOT EXISTS `accounts` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Unique ID',
  `acc_name` varchar(32) COLLATE utf8_unicode_ci NOT NULL COMMENT 'Login username',
  `encrypted_password` varchar(42) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `banned` int(10) unsigned NOT NULL,
  `lastlogin` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00' COMMENT 'Last login timestamp',
  `lastip` varchar(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT '' COMMENT 'Last remote address',
  `email` varchar(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '' COMMENT 'Contact e-mail address',
  `flags` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Client flags',
  `forceLanguage` varchar(5) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'enUS',
  `muted` int(30) NOT NULL DEFAULT '0',
  `banreason` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `joindate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `a` (`acc_name`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Account Information';

CREATE TABLE IF NOT EXISTS `ipbans` (
  `ip` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `expire` int(10) NOT NULL COMMENT 'Expiry time (s)',
  `banreason` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  PRIMARY KEY (`ip`),
  UNIQUE KEY `a` (`ip`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='IPBanner';

DELETE FROM `ipbans`;
/*!40000 ALTER TABLE `ipbans` DISABLE KEYS */;
/*!40000 ALTER TABLE `ipbans` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `logon_db_version` (
  `LastUpdate` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`LastUpdate`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DELETE FROM `logon_db_version`;
/*!40000 ALTER TABLE `logon_db_version` DISABLE KEYS */;
INSERT INTO `logon_db_version` (`LastUpdate`) VALUES
	('20180810-00_realms');
/*!40000 ALTER TABLE `logon_db_version` ENABLE KEYS */;

CREATE TABLE IF NOT EXISTS `realms` (
  `id` int(10) unsigned NOT NULL,
  `password` varchar(60) NOT NULL DEFAULT 'change_me_logon',
  `status` tinyint(1) unsigned NOT NULL,
  `status_change_time` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00' ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DELETE FROM `realms`;
/*!40000 ALTER TABLE `realms` DISABLE KEYS */;
INSERT INTO `realms` (`id`, `password`, `status`, `status_change_time`) VALUES
	(1, 'change_me_logon', 0, '0000-00-00 00:00:00');
/*!40000 ALTER TABLE `realms` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
