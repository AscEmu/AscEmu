--
-- column rename, drop gm level
--
ALTER TABLE `accounts` CHANGE `acct` `id` int(10) unsigned NOT NULL auto_increment COMMENT 'Unique ID';
ALTER TABLE `accounts` CHANGE `login` `acc_name` varchar(32);
ALTER TABLE `accounts` DROP COLUMN gm;
