/*
Add worldserver specific password.
*/

ALTER TABLE `realms`
ADD COLUMN `password` VARCHAR(60) NOT NULL DEFAULT 'change_me_logon' AFTER `id`;

UPDATE `logon_db_version` SET LastUpdate = '20180810-00_realms';
