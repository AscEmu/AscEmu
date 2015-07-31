/*
********************************************************************
Updates for accounts
********************************************************************
*/

ALTER TABLE `accounts` ADD COLUMN `joindate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP AFTER `banreason`;

UPDATE `accounts` SET `joindate` = NOW();
