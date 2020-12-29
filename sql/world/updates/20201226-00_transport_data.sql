SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `transport_data`
-- ----------------------------
DROP TABLE IF EXISTS `transport_data`;
DROP TABLE IF EXISTS `transport_creatures`;
CREATE TABLE `transport_data` (
  `entry` mediumint unsigned NOT NULL DEFAULT '0',
  `min_build` smallint NOT NULL DEFAULT '12340',
  `max_build` smallint NOT NULL DEFAULT '12340',
  `name` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  PRIMARY KEY (`entry`,`min_build`),
  UNIQUE KEY `unique_index` (`entry`,`min_build`) USING BTREE
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci ROW_FORMAT=FIXED COMMENT='Transports';

-- ----------------------------
-- Records of transport_data
-- ----------------------------
INSERT INTO `transport_data` VALUES ('176310', '12340', '18414', 'Stormwind Harbor and Auberdine, Darkshore (\"Ship (The Bravery)\")');
INSERT INTO `transport_data` VALUES ('176244', '5875', '12340', 'Rut\'theran Village, Teldrassil and Auberdine, Darkshore (\"The Moonspray\")');
INSERT INTO `transport_data` VALUES ('176231', '5875', '18414', 'Menethil Harbor, Wetlands and Theramore Isle, Dustwallow Marsh (\"The Lady Mehley\")');
INSERT INTO `transport_data` VALUES ('175080', '5875', '18414', 'Orgrimmar, Durotar and Grom\'gol Base Camp, Stranglethorn Vale (\"The Iron Eagle\")');
INSERT INTO `transport_data` VALUES ('164871', '5875', '18414', 'Orgrimmar, Durotar and Undercity, Tirisfal Glades (\"The Thundercaller\")');
INSERT INTO `transport_data` VALUES ('20808', '5875', '18414', 'Steamwheedle Cartel ports, Ratchet and Booty Bay (\"The Maiden\'s Fancy\")');
INSERT INTO `transport_data` VALUES ('190536', '12340', '18414', 'Stormwing Harbor and Valiance Keep, Borean Tundra (\"The Kraken\")');
INSERT INTO `transport_data` VALUES ('176495', '5875', '18414', 'Undercity, Tirisfal Glades and Grom\'gol Base Camp, Stranglethorn Vale (\"The Purple Princess\")');
INSERT INTO `transport_data` VALUES ('177233', '5875', '12340', 'The Forgotten Coast, Feralas and Feathermoon Stronghold, Sardor Isle, Feralas (\"Feathermoon Ferry\")');
INSERT INTO `transport_data` VALUES ('181646', '8606', '18414', 'Valaar\'s Berth, Azuremyst Isle and Auberdine, Darkshore (\"Elune\'s Blessing\")');
INSERT INTO `transport_data` VALUES ('181688', '12340', '18414', 'Menethil Harbor, Wetlands and Valgarde, Howling Fjord (\"Northspear\")');
INSERT INTO `transport_data` VALUES ('181689', '12340', '18414', 'Undercity, Tirisfal Glades and Vengeance Landing, Howling Fjord (\"Zeppelin, Horde (Cloudkisser)\")');
INSERT INTO `transport_data` VALUES ('186238', '12340', '18414', 'Orgrimmar, Durotar and Warsong Hold, Borean Tundra (\"Zeppelin, Horde (The Mighty Wind)\")');
INSERT INTO `transport_data` VALUES ('186371', '12340', '18414', 'Westguard Keep in Howling Fjord to bombard pirate (\"Zeppelin\")');
INSERT INTO `transport_data` VALUES ('187038', '12340', '18414', 'Not Boardable - Cyrcling in Howling Fjord (\"Sister Mercy\")');
INSERT INTO `transport_data` VALUES ('187568', '12340', '18414', 'Unu\'pe, Borean Tundra and Moa\'ki Harbor, Dragonblight (\"Turtle (Walker of Waves)\")');
INSERT INTO `transport_data` VALUES ('188511', '12340', '18414', 'Moa\'ki Harbor and Kamagua (\"Turtle (Green Island)\")');
INSERT INTO `transport_data` VALUES ('192241', '12340', '18414', 'Horde gunship patrolling above Icecrown (\"Orgrim\'s Hammer\")');
INSERT INTO `transport_data` VALUES ('192242', '12340', '18414', 'Alliance gunship patrolling above Icecrown (\"The Skybreaker\")');
INSERT INTO `transport_data` VALUES ('190549', '12340', '18414', 'Orgrimmar and Thunder Bluff (\"The Zephyr\")');



INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316813', '3681', '582', '29.5013', '0.000602', '24.4455', '0.0349066', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316814', '24998', '582', '4.9897', '-1.72901', '5.41924', '3.26161', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316815', '24456', '582', '29.5627', '0.150031', '16.6147', '3.18002', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316816', '24993', '582', '13.1874', '7.71381', '6.07001', '3.09834', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316817', '24995', '582', '13.3456', '-7.63689', '6.09325', '3.06064', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316818', '24996', '582', '-0.258897', '-7.62734', '4.80823', '5.66423', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316819', '24997', '582', '-0.416482', '4.39825', '4.79739', '5.10346', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316820', '25007', '582', '21.2462', '1.87803', '11.7334', '3.19572', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316821', '24837', '584', '21.2882', '-6.49847', '6.34678', '3.66717', '0', '22662');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316822', '24839', '584', '9.59588', '-1.21492', '11.801', '1.53995', '0', '22664');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316823', '24842', '584', '20.1177', '-6.31861', '6.38887', '3.3096', '0', '22666');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316824', '24841', '584', '-9.323', '-1.66992', '6.09808', '0.0174532', '0', '1858');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316825', '24840', '584', '-12.1809', '5.48897', '6.14024', '1.08723', '0', '22665');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316826', '24843', '584', '6.35056', '8.71821', '6.18084', '5.42797', '0', '22667');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316827', '24838', '584', '-27.217', '-0.0012207', '21.583', '2.4052', '0', '22663');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316828', '24836', '584', '-2.2334', '2.55383', '6.09902', '1.57667', '0', '262');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316829', '24833', '584', '37.0303', '4.62236', '6.17641', '1', '0', '22659');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316830', '24834', '584', '17.7362', '-7.96364', '6.18101', '1.65806', '0', '22660');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316831', '24835', '584', '24.6154', '5.46865', '16.124', '3.1765', '0', '22661');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316832', '25050', '586', '29.8662', '-0.374622', '16.6206', '3.09382', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316833', '25051', '586', '13.4899', '-7.71638', '6.11067', '3.11424', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316834', '25052', '586', '13.5093', '7.8287', '6.11039', '3.12838', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316835', '25054', '586', '-21.8863', '-2.61623', '4.33103', '6.14509', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316836', '25055', '586', '-21.4646', '2.87105', '4.31055', '6.26055', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316837', '25056', '586', '20.3045', '1.2971', '11.7088', '3.06319', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316838', '25053', '586', '-36.7911', '-0.04812', '5.97636', '2.77507', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316839', '3681', '586', '-48.4659', '0.112139', '8.7589', '3.45575', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316840', '3681', '587', '-38.6477', '-0.071194', '6.08577', '0.0698132', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316841', '3681', '587', '29.5623', '0.119925', '24.4539', '0.0872665', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316842', '25019', '587', '12.805', '-7.60196', '6.10507', '2.9147', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316843', '25020', '587', '12.9539', '7.33394', '6.13112', '3.28122', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316844', '25021', '587', '-21.4174', '-2.8336', '4.39169', '6.24828', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316845', '25022', '587', '20.8633', '-1.28591', '11.809', '2.94961', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316846', '25023', '587', '-36.7691', '0.169367', '5.97592', '3.1765', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316847', '25024', '587', '-21.6595', '3.22012', '4.40273', '0.139626', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316848', '25025', '587', '31.131', '-0.454317', '16.7328', '2.96706', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316849', '25009', '588', '25.2165', '6.9111', '16.1459', '3.94958', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316850', '25010', '588', '13.2057', '-2.817', '6.09989', '3.88733', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316851', '25011', '588', '19.2178', '-8.20848', '12.1102', '4.81518', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316852', '25012', '588', '18.1475', '-7.41572', '6.09809', '1.88535', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316853', '25013', '588', '0.194107', '9.84585', '6.09941', '3.08731', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316854', '25014', '588', '-0.532552', '-8.68575', '6.09815', '3.19019', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316855', '25015', '588', '34.0669', '0.119702', '18.287', '3.17832', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316856', '25016', '588', '-11.1276', '6.60326', '6.09852', '3.05167', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316857', '25017', '588', '6.22581', '9.13103', '11.4836', '1.53614', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316858', '25018', '588', '10.2474', '2.78122', '11.803', '3.46823', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316859', '24924', '589', '-1.2076', '-9.94886', '-23.6749', '0.15708', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316860', '24926', '589', '-10.3057', '-12.1052', '-16.9691', '5.92724', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316861', '24927', '589', '-1.87417', '-7.84711', '-23.6872', '3.38594', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316862', '24929', '589', '9.08301', '-4.96411', '-23.5921', '1.59406', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316863', '24930', '589', '-17.0083', '-7.87488', '-15.1878', '3.14159', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316864', '24931', '589', '7.88492', '-11.1513', '-17.7623', '4.90483', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316865', '24934', '589', '9.20919', '-3.50392', '-23.5121', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316866', '24935', '589', '9.55492', '-12.0229', '-23.5059', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316867', '25075', '589', '-2.70556', '-7.84588', '-23.6967', '0.0349066', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316868', '25080', '589', '-4.91002', '-4.74236', '-17.6153', '1.6057', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316869', '25081', '589', '-4.93939', '-10.8049', '-17.6109', '4.71239', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316870', '24934', '590', '9.63549', '-3.67192', '-23.588', '6.25904', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316871', '24935', '590', '9.79838', '-11.8681', '-23.5848', '6.25118', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316872', '25075', '590', '-3.59133', '-7.84061', '-23.7802', '6.1969', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316873', '25100', '590', '-0.411733', '-5.7239', '-23.7457', '3.70414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316874', '25101', '590', '7.56919', '-4.02088', '-17.7543', '4.77071', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316875', '25102', '590', '-11.0882', '-3.47446', '-16.7988', '2.00924', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316876', '25103', '590', '7.59398', '-11.5166', '-17.7745', '1.62204', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316877', '25104', '590', '-0.898213', '-7.76559', '-23.7548', '3.11116', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316878', '25105', '590', '-9.54405', '-7.94072', '-17.2053', '3.17242', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316879', '25106', '590', '-4.93938', '-10.9634', '-17.6988', '4.76285', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316880', '25107', '590', '-4.98215', '-4.67863', '-17.696', '1.59455', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316881', '15214', '591', '7.0053', '-7.64791', '-16.1126', '2.89725', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316882', '24934', '591', '-4.5165', '-13.1125', '-22.5947', '1.53589', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316883', '24935', '591', '-6.37827', '-13.1838', '-22.5939', '4.71239', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316884', '25070', '591', '-9.40787', '-8.02398', '-17.1578', '3.1765', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316885', '25071', '591', '7.24887', '-5.48033', '-17.6859', '4.81711', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316886', '25072', '591', '8.00807', '-10.7134', '-17.6737', '1.16937', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316887', '25074', '591', '5.02375', '-7.69781', '-17.7888', '5.98648', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316888', '25075', '591', '-4.16189', '-7.68752', '-23.6975', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316889', '25075', '591', '4.36215', '-2.25417', '-23.59', '4.71239', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316890', '25075', '591', '-3.31418', '-6.12881', '-23.6984', '4.67748', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316891', '25075', '591', '4.48208', '-13.4008', '-23.59', '1.62316', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316892', '25075', '591', '-3.30777', '-9.47416', '-23.6959', '1.55334', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316893', '25075', '591', '10.7034', '-3.50542', '-23.49', '3.24631', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316894', '25075', '591', '-8.87581', '-11.4028', '-22.5883', '6.24828', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316895', '25075', '591', '10.8261', '-12.1854', '-23.4895', '3.1765', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316896', '25075', '591', '11.7436', '-10.4452', '-24.2189', '6.16101', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316897', '25076', '591', '-2.72723', '-7.77286', '-23.6968', '1.55334', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316898', '25077', '591', '-19.6886', '-8.17058', '-14.3765', '3.1765', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316899', '25079', '591', '-5.1094', '-11.1466', '-17.606', '4.4855', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316900', '25083', '591', '-5.2125', '-4.92702', '-17.5966', '1.43117', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316901', '25171', '591', '-8.70329', '-11.4079', '-22.5887', '0.0349066', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316902', '25078', '593', '34.095', '3.54049', '17.8892', '5.50987', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316903', '25082', '593', '15.6121', '1.09944', '6.09764', '2.52482', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316904', '25089', '593', '17.8437', '-7.84575', '6.09877', '1.64493', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316905', '25093', '593', '15.8067', '-5.80051', '11.9732', '1.86484', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316906', '25094', '593', '34.0585', '-0.04162', '18.2865', '3.17017', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316907', '25095', '593', '9.39981', '9.17899', '11.5941', '1.52083', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316908', '25097', '593', '-11.4014', '6.67999', '6.09785', '2.93715', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316909', '25111', '593', '6.20811', '0.005208', '14.0554', '2.54813', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316910', '24910', '594', '34.6962', '-0.27625', '20.9157', '3.44936', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316911', '24911', '594', '-3.08712', '11.1947', '8.6042', '1.59543', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316912', '24911', '594', '-3.00336', '-1.39497', '8.72655', '0.455023', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316913', '24911', '594', '29.079', '6.02911', '19.504', '1.29931', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316914', '24911', '594', '-11.92', '6.82298', '8.72743', '2.64628', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316915', '24911', '594', '19.1465', '-9.70741', '14.7601', '4.79434', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316916', '24911', '594', '5.55254', '10.6903', '14.0795', '1.41713', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316917', '24911', '594', '5.53691', '-10.9158', '14.0808', '4.59956', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316918', '24911', '594', '19.1591', '9.74589', '14.7625', '1.55457', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316919', '24911', '594', '-10.8992', '6.36276', '20.589', '1.29146', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316920', '24911', '594', '-1.47544', '9.97225', '8.72811', '1.47291', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316921', '24911', '594', '-15.0531', '6.78103', '21.0344', '1.54279', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316922', '24911', '594', '0.568386', '10.818', '8.68709', '1.24593', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316923', '24911', '594', '-16.544', '7.01147', '21.3668', '1.52708', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316924', '24992', '594', '-13.669', '5.23144', '19.2894', '1.4721', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316925', '25026', '594', '5.50506', '5.17797', '1.39596', '1.53711', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316926', '25026', '594', '-3.34169', '-4.92735', '1.39596', '4.70933', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316927', '25026', '594', '-2.64281', '5.46732', '1.39596', '0.520802', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316928', '25026', '594', '6.52141', '0.490373', '1.39693', '6.00288', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316929', '25026', '594', '23.0708', '-2.7187', '1.39595', '5.37378', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316930', '25026', '594', '15.3122', '6.40496', '1.39596', '4.24673', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316931', '25026', '594', '24.9381', '4.10155', '1.44058', '1.07922', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316932', '25026', '594', '14.4097', '-6.40611', '1.39596', '1.86383', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316933', '25026', '594', '5.51407', '-5.26758', '1.39596', '4.38889', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316934', '25075', '610', '4.36215', '-2.25417', '-23.59', '4.71239', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316935', '31704', '610', '5.21605', '-2.36685', '-17.8223', '1.04622', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316936', '31705', '610', '5.07824', '-13.1188', '-17.8135', '5.24182', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316937', '31706', '610', '-16.8964', '-10.8497', '-15.9745', '4.64346', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316938', '31708', '610', '-2.74581', '-1.47146', '-17.7765', '4.67712', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316939', '31715', '610', '-3.54276', '-13.8752', '-17.684', '1.53946', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316940', '31716', '610', '11.5731', '-7.65137', '-16.6839', '3.20372', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316941', '31759', '612', '-9.17065', '-9.22241', '9.44523', '4.33964', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316942', '31760', '612', '-24.342', '-1.4956', '11.7907', '4.53119', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316943', '31761', '612', '17.25', '3.98267', '9.8274', '1.12707', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316944', '31762', '612', '34.0835', '-0.002845', '19.7971', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316945', '31763', '612', '30.1151', '-5.08848', '19.3282', '3.08923', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316946', '31764', '612', '26.0707', '2.05775', '19.328', '3.00197', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316947', '25075', '613', '9.44542', '-7.84947', '-16.6006', '0.0523599', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316948', '31720', '613', '-16.5685', '-5.08333', '-15.9421', '1.98968', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316949', '31723', '613', '-10.7552', '-12.8129', '-16.7745', '4.53786', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316950', '31724', '613', '5.77627', '-2.00469', '-17.7218', '1.64061', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316951', '31725', '613', '10.6984', '-7.82192', '-16.6006', '3.28122', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316952', '31726', '613', '-3.45307', '-13.7896', '-17.6111', '1.16964', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316953', '31727', '613', '-3.38308', '-1.91393', '-17.6198', '5.47991', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316954', '31788', '614', '27.5951', '-2.34424', '19.3281', '3.22886', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316955', '31789', '614', '28.0998', '5.9939', '19.328', '3.64774', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316956', '31790', '614', '34.2236', '0.067648', '19.7627', '3.07178', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316957', '31791', '614', '0.919363', '8.75723', '9.43661', '1.25664', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316958', '31792', '614', '-4.76611', '0.0998535', '9.36669', '4.83456', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316959', '31793', '614', '17.146', '-3.92139', '9.81305', '5.49708', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316960', '31807', '620', '-19.8763', '-0.049722', '17.6106', '2.86234', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316961', '31805', '620', '8.10938', '-1.96228', '15.8348', '3.22434', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316962', '31804', '621', '14.0833', '1.95972', '18.8097', '3.735', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316963', '30649', '622', '-32.5343', '24.3023', '33.9708', '3.21141', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316964', '30649', '622', '4.10968', '19.5269', '34.7477', '3.75246', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316965', '30690', '622', '-11.2231', '32.912', '10.5587', '1.58825', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316966', '30690', '622', '15.2472', '32.3771', '10.6319', '1.55334', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316967', '29795', '622', '45.6197', '7.29317', '30.0955', '4.67642', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316968', '30752', '622', '15.4321', '28.6642', '9.92277', '1.54012', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316969', '30752', '622', '2.01988', '28.7211', '9.33565', '1.58332', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316970', '30752', '622', '-11.241', '28.5576', '9.91826', '1.5519', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316971', '30752', '622', '8.25547', '-21.6199', '34.8875', '1.73515', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316972', '30752', '622', '7.30473', '24.2619', '34.9491', '4.43536', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316973', '30753', '622', '-26.8391', '-10.402', '35.5991', '1.29366', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316974', '30753', '622', '2.15579', '17.0338', '9.16353', '1.54405', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316975', '30753', '622', '-7.63407', '0.007234', '86.0904', '6.2728', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316976', '30753', '622', '46.382', '7.89944', '10.4129', '3.96271', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316977', '30753', '622', '45.981', '-6.55312', '10.3636', '2.23091', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316978', '30753', '622', '15.254', '-0.009458', '86.0904', '3.14299', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316979', '30753', '622', '-34.939', '-11.6484', '11.4697', '5.50858', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316980', '30753', '622', '-34.785', '11.9312', '11.4869', '0.836243', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316981', '30753', '622', '-26.9812', '10.5208', '35.5936', '4.74156', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316982', '30753', '622', '1.98831', '-18.0873', '9.16057', '4.66993', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316983', '30754', '622', '-19.0109', '27.0177', '89.9667', '6.22411', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316984', '30754', '622', '-54.639', '-15.3254', '34.3972', '3.83128', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316985', '30754', '622', '-54.7182', '15.5861', '34.3897', '2.50086', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316986', '30754', '622', '-18.9215', '-26.8371', '89.9664', '0.087792', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316987', '30755', '622', '-3.46042', '28.0231', '34.2784', '4.66548', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316988', '30755', '622', '7.21494', '-6.31021', '34.4191', '3.11118', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316989', '30755', '622', '24.5852', '6.86575', '7.06382', '3.11055', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316990', '30755', '622', '8.85995', '18.8224', '8.7027', '4.69035', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316991', '30755', '622', '24.3302', '-6.97827', '7.08356', '3.34184', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316992', '30755', '622', '-4.43012', '18.742', '8.62646', '4.66286', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316993', '30755', '622', '-5.59682', '-28.2501', '34.1226', '1.55058', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316994', '30755', '622', '-4.46811', '-18.5567', '8.62604', '1.53698', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316995', '30755', '622', '29.8693', '-29.5825', '89.7663', '3.05267', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316996', '30755', '622', '8.83021', '-4.6978', '84.7137', '2.34582', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316997', '30755', '622', '37.6428', '-9.00797', '30.0954', '0.004917', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316998', '30755', '622', '-26.188', '-6.1712', '9.33333', '3.14217', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('316999', '30755', '622', '9.06884', '-18.7113', '8.70787', '1.56054', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317000', '30755', '622', '8.67689', '4.88796', '84.7137', '3.90012', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317001', '30755', '622', '7.49121', '6.05275', '34.4239', '3.16615', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317002', '30755', '622', '37.6401', '8.9586', '30.0954', '0.012771', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317003', '30755', '622', '-26.1511', '6.90449', '9.405', '3.12647', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317004', '30755', '622', '29.4588', '29.8761', '89.7684', '3.15321', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317005', '30824', '622', '55.5028', '0.080449', '30.5268', '3.15669', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317006', '30825', '622', '38.4745', '0.038424', '10.1868', '3.15788', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317007', '30826', '622', '55.0542', '-3.74557', '30.0955', '2.77577', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317008', '30827', '622', '45.884', '-8.99976', '30.0955', '1.52942', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317009', '30866', '622', '-36.528', '23.9373', '33.9184', '1.89617', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317010', '30866', '622', '15.9225', '26.2539', '35.4586', '1.60085', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317011', '30866', '622', '-36.1494', '-23.2606', '33.9568', '4.2232', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317012', '30866', '622', '17.7216', '-26.2695', '35.5686', '5.06367', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317013', '31243', '622', '-11.2408', '33.2155', '10.5949', '1.58668', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317014', '31243', '622', '1.99373', '33.0756', '10.0105', '1.53853', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317015', '31243', '622', '15.239', '32.5832', '10.5826', '1.52441', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317016', '31243', '622', '15.2483', '-33.3678', '10.5837', '4.72649', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317017', '31243', '622', '1.91814', '-32.9286', '10.0097', '4.6891', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317018', '31243', '622', '-11.0915', '-32.8872', '10.5819', '4.68507', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317019', '31261', '622', '-24.084', '-22.2178', '24.3778', '1.43738', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317020', '32301', '622', '-3.37706', '0.007499', '34.0151', '4.65055', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317021', '30649', '622', '50.9957', '46.9566', '23.4137', '2.58309', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317022', '30649', '622', '2.00674', '15.7385', '9.25007', '3.36848', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317023', '30699', '622', '1.85384', '32.8888', '10.0236', '1.58825', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317024', '30700', '622', '-35.6663', '29.4333', '1.87925', '1.74533', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317025', '30700', '622', '7.41708', '32.8267', '38.356', '1.55334', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317026', '30700', '622', '-55.9708', '28.4419', '18.025', '2.26893', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317027', '30700', '622', '38.7626', '30.0934', '2.30818', '1.13446', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317028', '30707', '622', '19.4709', '27.5296', '10.6453', '1.39626', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317029', '30707', '622', '-15.3085', '30.5928', '11.1161', '2.63545', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317030', '31353', '622', '-7.99985', '17.8519', '35.0486', '2.46091', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317031', '30588', '622', '-18.1028', '-0.042108', '45.3172', '1.76278', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317032', '30589', '622', '-11.832', '-0.019289', '43.1147', '4.15388', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317033', '30342', '622', '-3.57706', '0.507499', '34.0151', '6.17572', '0', '17200');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317034', '30343', '623', '6.08656', '-0.107499', '20.5756', '0.588749', '0', '17200');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317035', '29799', '623', '34.5332', '-38.5618', '25.0323', '3.15234', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317036', '30344', '623', '43.6738', '0.121325', '25.1341', '3.10227', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317037', '30345', '623', '-48.0654', '-0.185737', '-4.98898', '3.11436', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317038', '30346', '623', '25.0778', '-0.047958', '9.59893', '3.13291', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317039', '30347', '623', '28.6378', '-7.55243', '23.2873', '0.157027', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317040', '30350', '623', '16.4056', '-2.2827', '20.4235', '3.11453', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317041', '30351', '623', '2.42088', '-23.0053', '22.5625', '0.046087', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317042', '30351', '623', '1.29162', '-9.37181', '20.458', '3.17295', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317043', '30351', '623', '0.773476', '22.5004', '22.5503', '3.2248', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317044', '30351', '623', '39.976', '44.3876', '25.0331', '3.16019', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317045', '30351', '623', '-36.4471', '6.81573', '20.4485', '4.73177', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317046', '30351', '623', '-36.1811', '-6.90251', '20.4501', '1.6483', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317047', '30351', '623', '1.44228', '9.63379', '20.4566', '3.15096', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317048', '30352', '623', '48.8649', '-8.72834', '40.0818', '3.12642', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317049', '30352', '623', '-17.337', '3.98796', '20.7652', '3.1541', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317050', '30352', '623', '16.5684', '2.46962', '20.4252', '3.1329', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317051', '30352', '623', '43.7981', '13.0009', '-2.07474', '4.61776', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317052', '30352', '623', '-67.4841', '3.50927', '9.60209', '5.83577', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317053', '30352', '623', '-17.2379', '-3.94242', '20.7667', '3.1541', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317054', '30352', '623', '-67.1723', '-3.73439', '9.60211', '0.318344', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317055', '30352', '623', '36.4909', '6.11523', '9.60666', '3.01117', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317056', '30352', '623', '-49.1048', '0.044213', '20.6694', '0.029782', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317057', '30352', '623', '36.436', '-6.06257', '9.60687', '3.09364', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317058', '30352', '623', '48.8813', '8.78624', '40.0817', '3.1426', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317059', '30352', '623', '-60.5592', '0.055898', '-5.27774', '0.004184', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317060', '30380', '623', '-6.13984', '21.6533', '9.991', '1.48436', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317061', '30380', '623', '3.99105', '-21.2539', '9.67311', '4.9577', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317062', '30380', '623', '-41.0289', '25.7685', '1.20385', '1.49696', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317063', '30380', '623', '-17.4619', '22.2092', '9.60018', '1.58254', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317064', '30380', '623', '-8.79147', '30.0334', '-0.157799', '0.471494', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317065', '30380', '623', '-26.5726', '29.6008', '-0.15773', '0.856857', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317066', '30380', '623', '45.5093', '6.67955', '30.1788', '5.44543', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317067', '30380', '623', '1.92073', '28.7498', '0.101361', '0.232732', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317068', '30380', '623', '-37.3836', '19.9617', '9.59771', '1.87549', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317069', '30380', '623', '-37.1975', '-20.2765', '9.65711', '4.32467', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317070', '30380', '623', '-7.54172', '-30.0747', '0.101348', '4.17752', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317071', '30380', '623', '-30.1731', '-21.9358', '9.59686', '4.72914', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317072', '30380', '623', '3.45962', '-28.1289', '0.101388', '4.29376', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317073', '30380', '623', '-18.0212', '-22.0926', '9.60068', '4.66788', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317074', '30380', '623', '-24.7068', '-29.9771', '0.101334', '3.66623', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317075', '30380', '623', '3.92454', '20.827', '9.67354', '1.30372', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317076', '30380', '623', '-6.33308', '-21.7722', '9.99575', '4.85167', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317077', '30380', '623', '-39.6946', '-26.8419', '0.82802', '2.93659', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317078', '30392', '623', '28.7566', '7.6217', '23.2872', '6.08285', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317079', '30394', '623', '-14.1505', '23.0373', '-5.24869', '0.027745', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317080', '30394', '623', '-11.7295', '-24.7904', '9.58663', '1.63703', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317081', '30394', '623', '-11.9688', '25.5424', '9.58513', '4.66945', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317082', '30394', '623', '-57.9337', '6.01148', '23.5029', '1.54855', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317083', '30394', '623', '-14.3898', '-23.2398', '-5.25039', '6.16013', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317084', '30394', '623', '-48.1449', '-3.10366', '-5.21617', '3.11436', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317085', '30833', '623', '6.18656', '-0.008156', '20.5756', '6.28313', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317086', '30867', '623', '-11.0475', '-22.7053', '22.5096', '4.51265', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317087', '30867', '623', '-32.9158', '-22.1469', '22.5861', '4.59982', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317088', '30867', '623', '-10.0824', '23.2226', '22.5129', '1.54405', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317089', '30867', '623', '36.8757', '45.226', '25.0331', '2.16509', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317090', '30867', '623', '-33.4747', '22.2096', '22.5895', '1.56211', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317091', '30867', '623', '36.9277', '-44.9241', '25.0318', '4.11052', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317092', '31259', '623', '16.8761', '-17.8635', '20.4597', '2.02864', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317093', '32193', '623', '-21.6978', '0.127903', '-18.1897', '3.12341', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317094', '32302', '623', '54.6648', '-6.9431', '40.0874', '3.0302', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317095', '32566', '623', '34.6465', '-41.7087', '25.0325', '3.20731', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317096', '32777', '623', '42.9517', '4.20903', '25.1088', '3.47298', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317097', '30640', '623', '-27.1637', '2.98126', '20.5409', '0.122173', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317098', '30640', '623', '6.90969', '9.52932', '20.5401', '2.30383', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317099', '30640', '623', '35.0385', '36.0634', '25.1171', '5.28835', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317100', '30646', '623', '-30.2557', '31.8003', '12.3542', '1.6057', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317101', '30640', '623', '-56.3119', '12.3922', '31.0047', '3.28122', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317102', '31353', '623', '-21.7234', '19.3375', '9.6872', '1.64061', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317103', '30476', '623', '31.4181', '0.126893', '41.6982', '0.0523599', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317104', '30559', '623', '38.1615', '-0.040522', '40.168', '4.2237', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317105', '30655', '623', '-43.5396', '18.6637', '9.69258', '3.24631', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317106', '30655', '623', '6.66292', '19.239', '10.0516', '0.506145', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317107', '30651', '623', '5.88316', '30.5042', '12.3476', '1.32645', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317108', '30651', '623', '-17.8134', '32.0788', '12.3449', '1.55334', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317109', '30651', '623', '-40.6824', '29.2156', '12.335', '1.91986', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317110', '30646', '623', '-5.32528', '31.625', '12.34', '1.50098', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317111', '20213', '641', '11.6996', '0.034146', '20.6208', '3.21141', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317112', '36166', '641', '-53.5545', '2.9776', '23.4432', '0.331613', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317113', '36166', '641', '17.0807', '-7.10945', '20.5052', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317114', '36166', '641', '49.9331', '-7.4586', '40.1678', '3.97935', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317115', '36166', '641', '50.207', '8.00353', '40.1665', '2.53073', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317116', '36166', '641', '39.7879', '-38.5873', '25.1146', '3.97935', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317117', '36166', '641', '-17.5794', '4.24393', '20.8454', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317118', '36166', '641', '16.8488', '6.61854', '20.5117', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317119', '36166', '641', '-61.941', '0.089971', '23.569', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317120', '36166', '641', '-17.5936', '-4.38373', '20.8457', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317121', '36166', '641', '-53.6908', '-3.68207', '23.4431', '6.05629', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317122', '36165', '641', '-35.8827', '3.1627', '20.5351', '1.65806', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317123', '36165', '641', '-36.385', '-7.32538', '20.4489', '1.65378', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317124', '34929', '641', '-12.4734', '25.7265', '21.6781', '1.74289', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317125', '34929', '641', '-41.7123', '22.9081', '22.601', '1.90241', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317126', '34929', '641', '-2.8113', '25.7812', '21.6915', '1.65806', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317127', '34929', '641', '-21.4492', '25.3989', '21.6696', '1.65806', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317128', '34929', '641', '-31.0354', '24.862', '21.7027', '1.65806', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317129', '36164', '642', '-31.1289', '-19.4868', '34.2492', '4.15388', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317130', '36164', '642', '-30.18', '17.9483', '34.3424', '2.23402', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317131', '34935', '642', '-21.4009', '-31.343', '34.2563', '4.67748', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317132', '34935', '642', '-12.1064', '-31.9696', '34.4639', '4.62512', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317133', '34935', '642', '-2.48761', '-31.9885', '34.9217', '4.83456', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317134', '34935', '642', '19.4637', '-30.794', '36.3084', '4.85202', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317135', '34935', '642', '10.2664', '-32.0713', '35.819', '4.85202', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317136', '36164', '642', '16.9491', '14.9328', '35.0811', '3.07178', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317137', '36164', '642', '6.64578', '-8.05272', '34.4752', '3.07178', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317138', '36164', '642', '17.5203', '-14.0262', '35.072', '3.07178', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317139', '36164', '642', '28.4088', '-19.3047', '35.9826', '4.15388', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317140', '36164', '642', '37.4704', '8.73106', '30.1788', '6.00393', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317141', '36164', '642', '6.56619', '7.98873', '34.4726', '3.07178', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317142', '36164', '642', '37.4039', '-8.93743', '30.1788', '6.26573', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317143', '36164', '642', '28.4441', '18.9595', '35.9598', '2.23402', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317144', '36162', '642', '-18.3838', '-11.1172', '34.9192', '3.08923', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317145', '36162', '642', '-22.1626', '11.1943', '35.0962', '3.9619', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317146', '20212', '642', '7.30561', '-0.095246', '34.5102', '3.15905', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317147', '34717', '647', '-13.6702', '-12.4018', '-15.9876', '4.59022', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317148', '24935', '647', '10.2871', '-12.0272', '-23.4942', '3.10669', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317149', '24934', '647', '10.321', '-3.57351', '-23.4941', '3.1765', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317150', '3084', '647', '-5.20674', '-11.3432', '-17.6101', '4.71239', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317151', '3084', '647', '-4.83257', '-4.31233', '-17.6322', '1.62316', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317152', '34730', '647', '-2.16687', '-7.85422', '-23.6919', '3.38594', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317153', '34723', '647', '-17.4797', '-5.60698', '-14.9281', '3.22886', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317154', '34721', '647', '-0.919197', '-6.16422', '-23.6729', '3.57792', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317155', '34719', '647', '7.62309', '-5.02532', '-17.6702', '4.92183', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317156', '34718', '647', '13.7451', '-5.12846', '-24.0452', '0.139626', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317157', '25075', '647', '-3.3964', '-7.90545', '-23.6967', '6.24828', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317158', '34715', '647', '-9.18316', '-7.77573', '-17.217', '3.28122', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317159', '37184', '672', '18.8226', '9.7001', '20.4184', '3.10669', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317160', '37182', '672', '42.789', '-0.010491', '25.2405', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317161', '36970', '672', '42.8015', '25.0622', '31.8407', '4.75675', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317162', '36970', '672', '35.651', '20.2921', '25.1161', '1.58825', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317163', '36970', '672', '29.2506', '-6.92039', '23.3714', '3.45575', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317164', '36970', '672', '35.5713', '-20.1836', '25.1162', '4.71239', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317165', '36970', '672', '9.26111', '-22.7322', '21.8455', '5.86431', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317166', '36970', '672', '-36.3806', '2.92895', '20.5322', '1.5708', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317167', '36970', '672', '-36.2222', '-2.96029', '20.5331', '4.69494', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317168', '36970', '672', '-64.7097', '4.57594', '23.5233', '2.09439', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317169', '36948', '672', '13.5155', '-0.160213', '20.8725', '3.10669', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317170', '36838', '672', '-6.15582', '-25.2387', '21.705', '4.71239', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317171', '36838', '672', '-33.6443', '-24.0658', '21.6801', '4.71239', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317172', '36838', '672', '-24.6625', '-24.5267', '21.6443', '4.71239', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317173', '36838', '672', '-15.3503', '-24.9037', '21.6201', '4.71239', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317174', '37547', '672', '-50.1652', '9.71624', '23.5871', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317175', '37540', '672', '-13.1429', '-0.36969', '12.8909', '0', '0', '31043');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317176', '37519', '672', '39.4475', '0.136515', '25.2321', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317177', '37547', '672', '38.9434', '-33.808', '25.3962', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317178', '37547', '672', '11.4584', '16.3662', '20.5419', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317179', '37547', '672', '8.5994', '-28.5585', '24.7992', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317180', '37547', '672', '19.7229', '-2.19379', '33.0698', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317181', '37547', '672', '0.554884', '-1.2329', '20.5371', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317182', '32780', '672', '1.29247', '-0.006242', '20.8767', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317183', '36948', '672', '1.34481', '-0.077413', '20.8492', '3.15905', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317184', '37547', '672', '-19.8822', '-6.57876', '20.5744', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317185', '37547', '672', '-11.6446', '-19.8518', '20.8843', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317186', '37519', '672', '-28.275', '15.5946', '20.5379', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317187', '37547', '672', '-41.4456', '-7.6475', '20.4975', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317188', '37547', '672', '-34.2702', '-26.1897', '21.3748', '0', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317189', '37833', '673', '60.4547', '0.021568', '38.7034', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317190', '37184', '673', '38.5985', '18.0196', '36.6939', '3.94444', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317191', '36939', '673', '36.4055', '0.184604', '36.7153', '3.10669', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317192', '37547', '673', '27.6276', '27.103', '36.8003', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317193', '37547', '673', '9.46182', '16.1523', '35.1091', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317194', '37547', '673', '-15.0316', '12.0216', '33.8629', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317195', '37547', '673', '-27.097', '27.9929', '34.3631', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317196', '37547', '673', '-39.4953', '16.6872', '34.3943', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317197', '37547', '673', '-58.1547', '0.748094', '41.8766', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317198', '37547', '673', '53.1563', '29.0877', '44.7302', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317199', '37547', '673', '4.7803', '-29.0523', '35.0963', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317200', '37547', '673', '23.4778', '-7.53715', '35.8162', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317201', '37547', '673', '-5.60755', '-6.35065', '34.0036', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317202', '37519', '673', '-19.9011', '-11.1976', '33.4849', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317203', '37519', '673', '22.1763', '-11.4125', '34.9973', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317204', '36971', '673', '-56.4357', '12.2929', '34.6332', '2.51327', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317205', '36971', '673', '30.8803', '22.7656', '36.3547', '1.69297', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317206', '36971', '673', '-26.8348', '13.4803', '34.6954', '5.3058', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317207', '36971', '673', '-29.3313', '-23.2348', '33.9633', '2.80988', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317208', '36971', '673', '-26.1657', '-13.3904', '34.679', '0.890118', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317209', '36971', '673', '60.0911', '-6.35005', '38.9569', '2.54818', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317210', '36971', '673', '59.6708', '6.21392', '39.0067', '3.735', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317211', '36971', '673', '30.9602', '-22.9078', '36.363', '4.46804', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317212', '36839', '673', '-15.6908', '31.1423', '34.391', '1.5708', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317213', '37215', '673', '3.8386', '0.183334', '24.1005', '0', '0', '31044');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317214', '36839', '673', '18.1923', '29.8694', '36.3265', '1.55334', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317215', '36971', '673', '-54.3389', '-14.5897', '34.4998', '3.9619', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317216', '36839', '673', '11.1788', '30.7344', '35.9594', '1.55334', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317217', '36839', '673', '-8.20063', '31.4933', '34.524', '1.55334', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317218', '30351', '712', '40.8536', '44.6598', '25.1171', '2.61799', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317219', '30867', '712', '-32.5883', '22.112', '21.7854', '1.76278', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317220', '30867', '712', '-11.1192', '23.0231', '21.7103', '1.72788', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317221', '30867', '712', '36.8039', '45.6098', '25.1163', '1.43117', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317222', '30351', '712', '1.43283', '22.4552', '21.7537', '3.19395', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317223', '30394', '712', '-57.6612', '-6.00481', '23.5631', '4.95674', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317224', '30351', '712', '1.0329', '9.63597', '20.5398', '3.21141', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317225', '30351', '712', '-36.2749', '-6.71154', '20.5328', '1.53589', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317226', '30351', '712', '-36.2637', '6.61251', '20.5329', '4.64258', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317227', '30352', '712', '-16.9331', '2.49734', '20.8759', '3.10669', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317228', '30352', '712', '-49.0054', '0.003014', '20.7507', '0.0174533', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317229', '30352', '712', '16.7383', '2.37812', '20.5012', '3.15905', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317230', '30352', '712', '48.8141', '8.76864', '40.1645', '1.67552', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317231', '30352', '712', '-16.8541', '-2.51852', '20.8759', '3.26377', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317232', '30351', '712', '4.01017', '-22.4291', '21.7794', '0.261799', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317233', '30351', '712', '0.778628', '-9.48492', '20.5411', '3.03687', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317234', '30867', '712', '-9.59931', '-23.155', '21.7158', '4.7822', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317235', '30352', '712', '48.8267', '-8.80392', '40.1644', '4.71239', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317236', '30867', '712', '-32.9935', '-22.1739', '21.7879', '4.50295', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317237', '30867', '712', '37.1434', '-45.9459', '25.1164', '4.01426', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317238', '30344', '712', '-2.70074', '12.2316', '20.5294', '1.72788', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317239', '22515', '712', '-27.094', '38.8533', '1.36691', '1.25664', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317240', '22515', '712', '-6.39693', '39.8011', '1.4704', '1.25664', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317241', '22515', '712', '4.01781', '38.32', '1.53394', '1.25664', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317242', '30392', '712', '28.1948', '7.5426', '23.3718', '5.8294', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317243', '30833', '712', '6.51805', '0.003965', '20.6643', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317244', '30350', '712', '16.3658', '-2.32358', '20.492', '3.14159', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317245', '30347', '712', '28.2482', '-7.66799', '23.3718', '0.436332', '0', '0');


INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317246', '30755', '713', '-18.9615', '27.5222', '90.0499', '6.21337', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317247', '30754', '713', '-54.6848', '15.0154', '34.4928', '2.33874', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317248', '30752', '713', '-10.9423', '32.1228', '10.6522', '1.51844', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317249', '30754', '713', '-8.50488', '-0.019059', '86.1737', '3.15905', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317250', '30866', '713', '-36.0366', '23.9163', '34.004', '1.93731', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317251', '30754', '713', '29.9708', '29.3299', '89.8491', '0.122173', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317252', '30753', '713', '15.1924', '-0.108369', '86.1737', '3.07178', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317253', '30752', '713', '7.70243', '25.2304', '35.0808', '4.76475', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317254', '30755', '713', '8.55542', '5.15577', '84.7971', '3.56047', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317255', '30755', '713', '-4.58862', '27.9195', '34.2793', '4.72984', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317256', '30754', '713', '-19.1954', '-27.024', '90.0507', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317257', '30754', '713', '-54.6367', '-15.2983', '34.4815', '3.71755', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317258', '30753', '713', '-26.199', '-10.3783', '35.6305', '1.64061', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317259', '30755', '713', '-56.7952', '-3.87048', '13.3164', '0.10472', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317260', '30755', '713', '-56.8364', '3.55791', '13.3138', '6.17846', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317261', '30753', '713', '-34.9544', '11.8472', '11.5961', '1.0821', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317262', '30752', '713', '1.99471', '31.9634', '10.0931', '1.44862', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317263', '30753', '713', '-26.0622', '10.5776', '35.5886', '4.71239', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317264', '30752', '713', '15.2307', '31.7683', '10.6651', '1.46608', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317265', '30753', '713', '1.99477', '17.0682', '9.24621', '1.50098', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317266', '30755', '713', '-4.48487', '18.0011', '8.70937', '4.72984', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317267', '30755', '713', '8.52195', '17.9289', '8.77778', '4.62512', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317268', '30755', '713', '-26.0374', '6.53311', '9.42994', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317269', '30755', '713', '-19.3032', '6.17474', '6.87912', '0', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317270', '30755', '713', '8.78642', '-5.03791', '84.7971', '2.74017', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317271', '30755', '713', '8.5575', '5.72448', '34.5215', '3.10669', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317272', '30755', '713', '8.45057', '-5.60921', '34.5206', '2.79253', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317273', '30866', '713', '-36.3256', '-23.2157', '34.0423', '4.39823', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317274', '30753', '713', '-35.4844', '-11.9256', '11.7141', '5.98648', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317275', '30752', '713', '8.47234', '-22.3667', '34.9983', '1.78024', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317276', '30755', '713', '-4.73831', '-28.1738', '34.2683', '1.46608', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317277', '30755', '713', '31.3718', '-29.8437', '89.8417', '2.96706', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317278', '30755', '713', '37.5961', '-9.01009', '30.1788', '0.0523599', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317279', '30755', '713', '37.5268', '8.71897', '30.1788', '5.96903', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317280', '30755', '713', '-25.4266', '-6.5772', '9.33257', '2.93215', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317281', '30755', '713', '23.2418', '6.30286', '7.0329', '3.1765', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317282', '30755', '713', '-19.4575', '-6.23543', '6.89157', '6.26573', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317283', '30752', '713', '-11.3651', '-29.1423', '10.0125', '4.64258', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317284', '30753', '713', '46.4166', '7.69621', '10.4885', '4.10152', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317285', '30752', '713', '15.4448', '-29.7788', '9.97704', '4.69494', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317286', '30754', '713', '38.019', '-12.8361', '30.1745', '0.610865', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317287', '30866', '713', '17.2224', '-26.6399', '35.6419', '4.81711', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317288', '30752', '713', '2.06997', '-29.6696', '9.39572', '4.67748', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317289', '30753', '713', '2.03651', '-17.1882', '9.24591', '4.76475', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317290', '30753', '713', '46.3396', '-7.35728', '10.476', '2.3911', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317291', '30755', '713', '8.59252', '-17.9413', '8.77952', '1.55334', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317292', '30755', '713', '-4.40636', '-17.971', '8.7093', '1.48353', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317293', '30755', '713', '22.9069', '-6.74543', '7.11532', '3.12414', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317294', '30824', '713', '17.2827', '21.7332', '35.3774', '1.62316', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317295', '30827', '713', '45.7689', '-8.96441', '30.1788', '1.39626', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317296', '30825', '713', '38.5575', '-0.025193', '10.2721', '3.10669', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317297', '30826', '713', '55.0852', '-3.34473', '30.1788', '2.68781', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317298', '37593', '713', '2.0159', '34.4453', '10.0931', '1.64061', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317299', '37593', '713', '14.8561', '33.8016', '9.84978', '1.64061', '0', '0');
INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `faction`, `displayid`) VALUES ('317300', '37593', '713', '-10.8423', '34.345', '10.6434', '1.64061', '0', '0');

-- update world_db_version
REPLACE INTO `world_db_version` VALUES ('66', '20201226-00_transport_data');

