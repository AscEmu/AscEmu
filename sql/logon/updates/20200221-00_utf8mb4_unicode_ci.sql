ALTER TABLE `accounts` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `ipbans` COLLATE='utf8mb4_unicode_ci';
ALTER TABLE `accounts` COLLATE='utf8mb4_unicode_ci';

-- TABLE `accounts`
ALTER TABLE `accounts`
    CHANGE COLUMN `acc_name` `acc_name` VARCHAR(32) NOT NULL COMMENT 'Login username' COLLATE 'utf8mb4_unicode_ci' AFTER `id`,
    CHANGE COLUMN `encrypted_password` `encrypted_password` VARCHAR(42) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' AFTER `acc_name`,
    CHANGE COLUMN `lastip` `lastip` VARCHAR(16) NOT NULL DEFAULT '' COMMENT 'Last remote address' COLLATE 'utf8mb4_unicode_ci' AFTER `lastlogin`,
    CHANGE COLUMN `email` `email` VARCHAR(64) NOT NULL DEFAULT '' COMMENT 'Contact e-mail address' COLLATE 'utf8mb4_unicode_ci' AFTER `lastip`,
    CHANGE COLUMN `forceLanguage` `forceLanguage` VARCHAR(5) NOT NULL DEFAULT 'enUS' COLLATE 'utf8mb4_unicode_ci' AFTER `flags`,
    CHANGE COLUMN `banreason` `banreason` VARCHAR(100) NULL DEFAULT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `muted`;

-- TABLE `ipbans`
ALTER TABLE `ipbans`
    CHANGE COLUMN `ip` `ip` VARCHAR(20) NOT NULL COLLATE 'utf8mb4_unicode_ci' FIRST,
    CHANGE COLUMN `banreason` `banreason` VARCHAR(100) NULL DEFAULT NULL COLLATE 'utf8mb4_unicode_ci' AFTER `expire`;

-- TABLE `logon_db_version`
ALTER TABLE `logon_db_version`
    CHANGE COLUMN `LastUpdate` `LastUpdate` VARCHAR(100) NOT NULL DEFAULT '' COLLATE 'utf8mb4_unicode_ci' FIRST;

-- TABLE `realms`
ALTER TABLE `realms`
    CHANGE COLUMN `password` `password` VARCHAR(60) NOT NULL DEFAULT 'change_me_logon' COLLATE 'utf8mb4_unicode_ci' AFTER `id`;

UPDATE `logon_db_version` SET LastUpdate = '20200221-00_utf8mb4_unicode_ci';
