-- ----------------------------
-- Delete transport_creatures for invalid transporter
-- ----------------------------
DELETE FROM `transport_creatures` WHERE `transport_entry` = '195276';

UPDATE `world_db_version` SET `LastUpdate` = '2015-08-20_01_transport_creatures' WHERE `LastUpdate` = '2015-08-11_02_column_size';
  