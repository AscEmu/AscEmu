DROP TABLE IF EXISTS `creature_script_waypoints`;
CREATE TABLE `creature_script_waypoints` (
  `id` int unsigned NOT NULL DEFAULT '0' COMMENT 'Creature GUID',
  `point` mediumint unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `delay` int unsigned NOT NULL DEFAULT '0',
  `move_type` int NOT NULL DEFAULT '0',
  `action` int NOT NULL DEFAULT '0',
  `action_chance` smallint NOT NULL DEFAULT '100',
  `description` char(100) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`,`point`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of creature_script_waypoints
-- ----------------------------
INSERT INTO `creature_script_waypoints` VALUES ('1', '1', '-250.923', '2116.26', '81.179', '0', '0', '0', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '2', '-255.049', '2119.39', '81.179', '0', '0', '0', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '3', '-254.129', '2123.45', '81.179', '0', '0', '0', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '4', '-253.898', '2130.87', '81.179', '0', '0', '0', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '5', '-249.889', '2142.31', '86.972', '0', '0', '0', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '6', '-248.205', '2144.02', '87.013', '0', '0', '0', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '7', '-240.553', '2140.55', '87.012', '0', '0', '0', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '8', '-237.514', '2142.07', '87.012', '0', '0', '0', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '9', '-235.638', '2149.23', '90.587', '0', '0', '0', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '10', '-237.188', '2151.95', '90.624', '0', '0', '0', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '11', '-239.075', '155.25', '90.624', '0', '0', '0', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '12', '-208.764', '2141.6', '90.6257', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '13', '-206.441', '2143.51', '90.4287', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '14', '-203.715', '2145.85', '88.7052', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '15', '-199.199', '2144.88', '86.501', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '16', '-195.798', '2143.58', '86.501', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '17', '-190.029', '2141.38', '83.2712', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '18', '-189.353', '2138.65', '83.1102', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '19', '-190.304', '2135.73', '81.5288', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '20', '-207.325', '2112.43', '81.0548', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '21', '-208.754', '2109.9', '81.0527', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '22', '-206.248', '2108.62', '81.0555', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '23', '-202.017', '2106.64', '78.6836', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '24', '-200.928', '2104.49', '78.5569', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '25', '-201.845', '2101.17', '76.9256', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '26', '-202.844', '2100.11', '76.8911', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '27', '-213.326', '2105.83', '76.8925', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '28', '-226.993', '2111.47', '76.8925', '0', '0', '1', '1', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '29', '-227.955', '2112.34', '76.8925', '0', '0', '1', '0', '100', 'Adamant Shadowfang');
INSERT INTO `creature_script_waypoints` VALUES ('1', '30', '-229.159', '2109.52', '76.8895', '0', '0', '1', '0', '100', 'Adamant Shadowfang');

INSERT INTO `world_db_version` VALUES ('75', '20210522-00_creature_script_waypoint');
