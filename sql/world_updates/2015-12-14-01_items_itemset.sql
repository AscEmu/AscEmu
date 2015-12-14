--
-- Create Table items_linked_itemsets
--
CREATE TABLE IF NOT EXISTS `items_linked_itemsets` (
  `itemset` int(5) NOT NULL COMMENT 'itemset from items table',
  `itemset_bonus` int(5) NOT NULL COMMENT 'linked itemset for itemset bonus',
  PRIMARY KEY (`itemset`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Add linked itemsets
--
INSERT INTO `items_linked_itemsets`
VALUES 
   (701, 736),    -- Data from Core Player::_ApplyItemMods
   (567, 736),
   (736, 736),
   (702, 734),
   (568, 734),
   (734, 734),
   (703, 732),
   (578, 732),
   (732, 732),
   (704, 735),
   (615, 735),
   (735, 735),
   (705, 728),
   (687, 728),
   (728, 728),
   (706, 723),
   (586, 723),
   (723, 723),
   (707, 729),
   (581, 729),
   (729, 729),
   (708, 725),
   (690, 725),
   (725, 725),
   (709, 720),
   (685, 720),
   (720, 720),
   (710, 724),
   (579, 724),
   (724, 724),
   (711, 721),
   (584, 721),
   (721, 721),
   (712, 733),
   (580, 733),
   (733, 733),
   (713, 730),
   (577, 730),
   (730, 730),
   (714, 726),
   (583, 726),
   (726, 726),
   (715, 731),
   (686, 731),
   (731, 731),
   (716, 722),
   (585, 722),
   (722, 722),    
   (856, 900),    -- Database correction
   (245, 900),
   (900, 900),
   (857, 901),
   (246, 901),
   (265, 901),
   (901, 901),
   (855, 899),
   (244, 899),
   (263, 899),
   (899, 899),
   (854, 898),
   (243, 898),
   (262, 898),
   (898, 898);

--
-- Update items to correct itemsetid
--
UPDATE `items` SET `itemset` = 856 WHERE `entry` = 50324;
UPDATE `items` SET `itemset` = 856 WHERE `entry` = 50325;
UPDATE `items` SET `itemset` = 856 WHERE `entry` = 50326;
UPDATE `items` SET `itemset` = 856 WHERE `entry` = 50327;
UPDATE `items` SET `itemset` = 856 WHERE `entry` = 50328;
UPDATE `items` SET `itemset` = 245 WHERE `entry` = 51160;
UPDATE `items` SET `itemset` = 245 WHERE `entry` = 51161;
UPDATE `items` SET `itemset` = 245 WHERE `entry` = 51162;
UPDATE `items` SET `itemset` = 245 WHERE `entry` = 51163;
UPDATE `items` SET `itemset` = 245 WHERE `entry` = 51164;
UPDATE `items` SET `itemset` = 264 WHERE `entry` = 51275;
UPDATE `items` SET `itemset` = 264 WHERE `entry` = 51276;
UPDATE `items` SET `itemset` = 264 WHERE `entry` = 51277;
UPDATE `items` SET `itemset` = 264 WHERE `entry` = 51278;
UPDATE `items` SET `itemset` = 264 WHERE `entry` = 51279;
UPDATE `items` SET `itemset` = 701 WHERE `entry` = 30486;
UPDATE `items` SET `itemset` = 701 WHERE `entry` = 30487;
UPDATE `items` SET `itemset` = 701 WHERE `entry` = 30488;
UPDATE `items` SET `itemset` = 701 WHERE `entry` = 30489;
UPDATE `items` SET `itemset` = 701 WHERE `entry` = 30490;
UPDATE `items` SET `itemset` = 736 WHERE `entry` = 33728;
UPDATE `items` SET `itemset` = 736 WHERE `entry` = 33729;
UPDATE `items` SET `itemset` = 736 WHERE `entry` = 33730;
UPDATE `items` SET `itemset` = 736 WHERE `entry` = 33731;
UPDATE `items` SET `itemset` = 736 WHERE `entry` = 33732;
UPDATE `items` SET `itemset` = 857 WHERE `entry` = 50860;
UPDATE `items` SET `itemset` = 857 WHERE `entry` = 50861;
UPDATE `items` SET `itemset` = 857 WHERE `entry` = 50862;
UPDATE `items` SET `itemset` = 857 WHERE `entry` = 50863;
UPDATE `items` SET `itemset` = 857 WHERE `entry` = 50864;
UPDATE `items` SET `itemset` = 246 WHERE `entry` = 51170;
UPDATE `items` SET `itemset` = 246 WHERE `entry` = 51171;
UPDATE `items` SET `itemset` = 246 WHERE `entry` = 51172;
UPDATE `items` SET `itemset` = 246 WHERE `entry` = 51173;
UPDATE `items` SET `itemset` = 246 WHERE `entry` = 51174;
UPDATE `items` SET `itemset` = 265 WHERE `entry` = 51265;
UPDATE `items` SET `itemset` = 265 WHERE `entry` = 51266;
UPDATE `items` SET `itemset` = 265 WHERE `entry` = 51267;
UPDATE `items` SET `itemset` = 265 WHERE `entry` = 51268;
UPDATE `items` SET `itemset` = 265 WHERE `entry` = 51269;
UPDATE `items` SET `itemset` = 855 WHERE `entry` = 50865;
UPDATE `items` SET `itemset` = 855 WHERE `entry` = 50866;
UPDATE `items` SET `itemset` = 855 WHERE `entry` = 50867;
UPDATE `items` SET `itemset` = 855 WHERE `entry` = 50868;
UPDATE `items` SET `itemset` = 855 WHERE `entry` = 50869;
UPDATE `items` SET `itemset` = 244 WHERE `entry` = 51165;
UPDATE `items` SET `itemset` = 244 WHERE `entry` = 51166;
UPDATE `items` SET `itemset` = 244 WHERE `entry` = 51167;
UPDATE `items` SET `itemset` = 244 WHERE `entry` = 51168;
UPDATE `items` SET `itemset` = 244 WHERE `entry` = 51169;
UPDATE `items` SET `itemset` = 263 WHERE `entry` = 51270;
UPDATE `items` SET `itemset` = 263 WHERE `entry` = 51271;
UPDATE `items` SET `itemset` = 263 WHERE `entry` = 51272;
UPDATE `items` SET `itemset` = 263 WHERE `entry` = 51273;
UPDATE `items` SET `itemset` = 263 WHERE `entry` = 51274;
UPDATE `items` SET `itemset` = 854 WHERE `entry` = 50853;
UPDATE `items` SET `itemset` = 854 WHERE `entry` = 50854;
UPDATE `items` SET `itemset` = 854 WHERE `entry` = 50855;
UPDATE `items` SET `itemset` = 854 WHERE `entry` = 50856;
UPDATE `items` SET `itemset` = 854 WHERE `entry` = 50857;
UPDATE `items` SET `itemset` = 243 WHERE `entry` = 51130;
UPDATE `items` SET `itemset` = 243 WHERE `entry` = 51131;
UPDATE `items` SET `itemset` = 243 WHERE `entry` = 51132;
UPDATE `items` SET `itemset` = 243 WHERE `entry` = 51133;
UPDATE `items` SET `itemset` = 243 WHERE `entry` = 51134;
UPDATE `items` SET `itemset` = 262 WHERE `entry` = 51305;
UPDATE `items` SET `itemset` = 262 WHERE `entry` = 51306;
UPDATE `items` SET `itemset` = 262 WHERE `entry` = 51307;
UPDATE `items` SET `itemset` = 262 WHERE `entry` = 51308;
UPDATE `items` SET `itemset` = 262 WHERE `entry` = 51309;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2015-12-14-01_items_itemset' WHERE `LastUpdate` = '2015-12-13-02_event_call_to_arms';
