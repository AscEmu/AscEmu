
-- creature_properties
UPDATE `creature_properties` SET `gossipId`=214 WHERE `entry`=35364;
UPDATE `creature_properties` SET `gossipId`=214 WHERE `entry`=35365;


-- gossip_menu
REPLACE INTO `gossip_menu` VALUES(214, 14736, 'XPToggle');


-- gossip_menu_items
REPLACE INTO `gossip_menu_items` VALUES(214, 1, 419, 0, 6, 100000, 421, 0, 0, 0, 0, 3, 0);
REPLACE INTO `gossip_menu_items` VALUES(214, 2, 420, 0, 6, 100000, 422, 0, 0, 0, 0, 4, 0);


-- update world_db_version

INSERT INTO `world_db_version` VALUES ('56', '20200429-00_gossips');
