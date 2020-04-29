
-- creature_properties
UPDATE `creature_properties` SET `gossipId`=200 WHERE `entry`=3052;
UPDATE `creature_properties` SET `gossipId`=201 WHERE `entry`=7763;
UPDATE `creature_properties` SET `gossipId`=202 WHERE `entry`=7804;
UPDATE `creature_properties` SET `gossipId`=203 WHERE `entry`=7916;
UPDATE `creature_properties` SET `gossipId`=204 WHERE `entry`=24932;


-- gossip_menu
REPLACE INTO `gossip_menu` VALUES(200, 522, 'SkornWhitecloud');
REPLACE INTO `gossip_menu` VALUES(201, 1519, 'CurgleCranklehop');
REPLACE INTO `gossip_menu` VALUES(202, 1758, 'TrentonLighthammer');
REPLACE INTO `gossip_menu` VALUES(203, 2153, 'ErelasAmbersky');
REPLACE INTO `gossip_menu` VALUES(204, 12227, 'ExarchNasuun');

-- gossip_menu_items
REPLACE INTO `gossip_menu_items` VALUES(200, 1, 392, 0, 0, 0, 523);
REPLACE INTO `gossip_menu_items` VALUES(201, 1, 401, 0, 0, 0, 1521);
REPLACE INTO `gossip_menu_items` VALUES(201, 2, 402, 0, 0, 0, 1646);
REPLACE INTO `gossip_menu_items` VALUES(202, 1, 403, 0, 0, 0, 1759);
REPLACE INTO `gossip_menu_items` VALUES(203, 1, 404, 0, 0, 0, 2154);
REPLACE INTO `gossip_menu_items` VALUES(204, 1, 394, 0, 0, 0, 12623);

-- update world_db_version

INSERT INTO `world_db_version` VALUES ('52', '20200324-00_gossips');
