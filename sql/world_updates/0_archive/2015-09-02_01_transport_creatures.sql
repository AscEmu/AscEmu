-- ----------------------------
-- Delete invalid transport_creaturese for non existing transporter
-- ----------------------------
DELETE FROM `transport_creatures` WHERE `transport_entry` = '195121';

UPDATE `world_db_version` SET `LastUpdate` = '2015-09-02_01_transport_creatures' WHERE `LastUpdate` = '2015-09-01_01_icc_crash';
  