/*
********************************************************************
Updates for mailbox
********************************************************************
*/
ALTER TABLE mailbox MODIFY `sender_guid` bigint(30) unsigned NOT NULL DEFAULT '0';

UPDATE `character_db_version` SET `LastUpdate` = '2015-03-21_01_mailbox';
