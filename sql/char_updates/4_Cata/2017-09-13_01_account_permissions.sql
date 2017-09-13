--
-- account_permissions
--
DROP TABLE IF EXISTS `account_forced_permissions`;
CREATE TABLE `account_permissions` (
  `id` int(10) unsigned NOT NULL,
  `permissions` varchar(100) NOT NULL,
  `name` varchar(50) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Update char_db_version
--
UPDATE `character_db_version` SET `LastUpdate` = '2017-09-13_01_account_permissions' WHERE `LastUpdate` = '2017-06-30_01_guild';
