-- ----------------------------
-- Delete invalid gameobject spawn near icc-entrance
-- ----------------------------
DELETE FROM `gameobject_spawns` WHERE `id` = '56076';

UPDATE `world_db_version` SET `LastUpdate` = '2015-09-01_01_icc_crash' WHERE `LastUpdate` = '2015-08-20_01_transport_creatures';
  