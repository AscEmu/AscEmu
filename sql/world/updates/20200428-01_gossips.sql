
-- onChooseData2
ALTER TABLE `gossip_menu_items`
ADD COLUMN `on_choose_data2` int(10) NOT NULL DEFAULT 0 AFTER `on_choose_data`;

-- creature_properties
UPDATE `creature_properties` SET `gossipId`=205 WHERE `entry`=30051;
UPDATE `creature_properties` SET `gossipId`=206 WHERE `entry`=26949;
UPDATE `creature_properties` SET `gossipId`=207 WHERE `entry`=27575;
UPDATE `creature_properties` SET `gossipId`=208 WHERE `entry`=26443;

-- gossip_menu
REPLACE INTO `gossip_menu` VALUES(205, 13022, 'Tiare');
REPLACE INTO `gossip_menu` VALUES(206, 12714, 'Torastrasza');
REPLACE INTO `gossip_menu` VALUES(207, 12887, 'LordAfrasastrasz');
REPLACE INTO `gossip_menu` VALUES(208, 12713, 'Tariolstrasz');

-- gossip_menu_items
REPLACE INTO `gossip_menu_items` VALUES(205, 1, 350, 0, 2, 50135, 0, 0, 0);

REPLACE INTO `gossip_menu_items` VALUES(206, 1, 413, 0, 3, 879, 6371, 0, 0);
REPLACE INTO `gossip_menu_items` VALUES(206, 2, 414, 0, 3, 880, 6371, 0, 0);

REPLACE INTO `gossip_menu_items` VALUES(207, 1, 415, 0, 3, 881, 6371, 0, 0);
REPLACE INTO `gossip_menu_items` VALUES(207, 2, 416, 0, 3, 882, 6371, 0, 0);

REPLACE INTO `gossip_menu_items` VALUES(208, 1, 417, 0, 3, 878, 6371, 0, 0);
REPLACE INTO `gossip_menu_items` VALUES(208, 2, 418, 0, 3, 883, 6371, 0, 0);

-- update world_db_version

INSERT INTO `world_db_version` VALUES ('54', '20200428-01_gossips');
