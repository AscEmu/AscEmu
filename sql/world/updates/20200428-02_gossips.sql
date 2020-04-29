
-- onChooseData3/4
ALTER TABLE `gossip_menu_items`
ADD COLUMN `on_choose_data3` int(10) NOT NULL DEFAULT 0 AFTER `on_choose_data2`,
ADD COLUMN `on_choose_data4` int(10) NOT NULL DEFAULT 0 AFTER `on_choose_data3`;

-- requirements
ALTER TABLE `gossip_menu_items`
ADD COLUMN `requirement_type` int(10) NOT NULL DEFAULT 0 AFTER `next_gossip_text`,
ADD COLUMN `requirement_data` int(10) NOT NULL DEFAULT 0 AFTER `requirement_type`;

-- creature_properties
UPDATE `creature_properties` SET `gossipId`=209 WHERE `entry`=25967;
UPDATE `creature_properties` SET `gossipId`=210 WHERE `entry`=29154;
UPDATE `creature_properties` SET `gossipId`=211 WHERE `entry`=2708;
UPDATE `creature_properties` SET `gossipId`=212 WHERE `entry`=23704;
UPDATE `creature_properties` SET `gossipId`=213 WHERE `entry`=4944;


-- gossip_menu
REPLACE INTO `gossip_menu` VALUES(209, 1, 'Zephyr');
REPLACE INTO `gossip_menu` VALUES(210, 13454, 'ThargoldIronwing');
REPLACE INTO `gossip_menu` VALUES(211, 11469, 'ArchmageMalin');
REPLACE INTO `gossip_menu` VALUES(212, 11224, 'CassaCrimsonwing');
REPLACE INTO `gossip_menu` VALUES(213, 1, 'CaptainGarranVimes');


-- gossip_menu_items
REPLACE INTO `gossip_menu_items` VALUES(209, 1, 396, 0, 4, 989, 21000, 37778, 397, 0, 0, 0, 0);
REPLACE INTO `gossip_menu_items` VALUES(210, 1, 399, 0, 3, 1041, 25679, 0, 0, 0, 0, 0, 0);
REPLACE INTO `gossip_menu_items` VALUES(210, 1, 400, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0);
REPLACE INTO `gossip_menu_items` VALUES(211, 1, 398, 0, 2, 42711, 0, 0, 0, 0, 0, 1, 11223);
REPLACE INTO `gossip_menu_items` VALUES(212, 1, 405, 0, 3, 724, 1147, 0, 0, 0, 0, 1, 11142);
REPLACE INTO `gossip_menu_items` VALUES(213, 1, 406, 0, 0, 0, 0, 0, 0, 0, 1794, 1, 11123);


-- update world_db_version

INSERT INTO `world_db_version` VALUES ('55', '20200428-02_gossips');
