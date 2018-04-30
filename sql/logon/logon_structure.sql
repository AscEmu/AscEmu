/*
********************************************************************
AscEmu logon structure
Last update: 09/13/2017
********************************************************************
*/

/*!40101 SET NAMES utf8 */;
/*!40101 SET SQL_MODE=''*/;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

/*Table structure for table `accounts` */
DROP TABLE IF EXISTS `accounts`;

CREATE TABLE `accounts` (
  `id` int(10) unsigned NOT NULL auto_increment COMMENT 'Unique ID',
  `acc_name` varchar(32) collate utf8_unicode_ci NOT NULL COMMENT 'Login username',
  `encrypted_password` varchar(42) collate utf8_unicode_ci NOT NULL default '',
  `banned` int(10) unsigned NOT NULL,
  `lastlogin` timestamp NOT NULL default '0000-00-00 00:00:00' COMMENT 'Last login timestamp',
  `lastip` varchar(16) collate utf8_unicode_ci NOT NULL default '' COMMENT 'Last remote address',
  `email` varchar(64) collate utf8_unicode_ci NOT NULL default '' COMMENT 'Contact e-mail address',
  `flags` tinyint(3) unsigned NOT NULL default '0' COMMENT 'Client flags',
  `forceLanguage` varchar(5) collate utf8_unicode_ci NOT NULL default 'enUS',
  `muted` int(30) NOT NULL default '0',
  `banreason` varchar(255) collate utf8_unicode_ci default NULL,
  `joindate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `a` (`acc_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Account Information';

/*Data for the table `accounts` */
LOCK TABLES `accounts` WRITE;

UNLOCK TABLES;


/*Table structure for table `ipbans` */
DROP TABLE IF EXISTS `ipbans`;

CREATE TABLE `ipbans` (
  `ip` varchar(20) collate utf8_unicode_ci NOT NULL,
  `expire` int(10) NOT NULL COMMENT 'Expiry time (s)',
  `banreason` varchar(255) collate utf8_unicode_ci default NULL,
  PRIMARY KEY (`ip`),
  UNIQUE KEY `a` (`ip`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='IPBanner';

/*Data for the table `ipbans` */
LOCK TABLES `ipbans` WRITE;

UNLOCK TABLES;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
