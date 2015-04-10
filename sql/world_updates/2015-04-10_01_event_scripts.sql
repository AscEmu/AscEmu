
-- ----------------------------
-- Table structure for `event_scripts`
-- ----------------------------
DROP TABLE IF EXISTS `event_scripts`;
CREATE TABLE `event_scripts` (
  `event_id` int(11) NOT NULL,
  `function` int(11) NOT NULL,
  `script_type` mediumint(8) NOT NULL,
  `data_1` mediumint(8) DEFAULT NULL,
  `data_2` mediumint(8) DEFAULT NULL,
  `data_3` mediumint(8) NOT NULL DEFAULT '0',
  `data_4` mediumint(8) DEFAULT NULL,
  `data_5` mediumint(8) DEFAULT NULL,
  `x` float DEFAULT NULL,
  `y` float DEFAULT NULL,
  `z` float DEFAULT NULL,
  `o` float DEFAULT NULL,
  `delay` int(11) DEFAULT NULL,
  `next_event` tinyint(11) DEFAULT NULL,
  PRIMARY KEY (`event_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Records of event_scripts
-- ----------------------------
INSERT INTO `event_scripts` VALUES ('1', '8', '1', '46574', '11913', '1', '0', '0', '0', '0', '0', '0', '1000', '2');
INSERT INTO `event_scripts` VALUES ('2', '9', '5', '300184', '30000', '0', '0', '0', '0', '0', '0', '0', '0', '3');
INSERT INTO `event_scripts` VALUES ('3', '9', '5', '188112', '30000', '0', '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `event_scripts` VALUES ('4', '13', '2', '193019', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0');

-- This is a Lever in Dalaran , to open the Door when you  exit the instance.
UPDATE `gameobject_names` SET `sound9`='4' WHERE (`entry`='193020');

UPDATE `world_db_version` SET `LastUpdate` = '2015-04-10_01_event_scripts';
