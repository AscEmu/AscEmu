--
-- Create Table items_linked_itemsets
--
DROP TABLE IF EXISTS `items_linked_itemsets`;
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
   (898, 898),
   (853, 897),
   (242, 897),
   (261, 897),
   (897, 897),
   (852, 896),
   (241, 896),
   (260, 896),
   (896, 896),
   (851, 895),
   (240, 895),
   (259, 895),
   (895, 895),
   (258, 894),
   (239, 894),
   (850, 894),
   (894, 894),
   (257, 893),
   (238, 893),
   (849, 893),
   (893, 893),
   (256, 892),
   (237, 892),
   (848, 892),
   (892, 892),
   (255, 891),
   (236, 891),
   (847, 891),
   (891, 891),
   (254, 890),
   (235, 890),
   (846, 890),
   (890, 890);

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
UPDATE `items` SET `itemset` = 853 WHERE `entry` = 50094;
UPDATE `items` SET `itemset` = 853 WHERE `entry` = 50095;
UPDATE `items` SET `itemset` = 853 WHERE `entry` = 50096;
UPDATE `items` SET `itemset` = 853 WHERE `entry` = 50097;
UPDATE `items` SET `itemset` = 853 WHERE `entry` = 50098;
UPDATE `items` SET `itemset` = 242 WHERE `entry` = 51125;
UPDATE `items` SET `itemset` = 242 WHERE `entry` = 51126;
UPDATE `items` SET `itemset` = 242 WHERE `entry` = 51127;
UPDATE `items` SET `itemset` = 242 WHERE `entry` = 51128;
UPDATE `items` SET `itemset` = 242 WHERE `entry` = 51129;
UPDATE `items` SET `itemset` = 261 WHERE `entry` = 51310;
UPDATE `items` SET `itemset` = 261 WHERE `entry` = 51311;
UPDATE `items` SET `itemset` = 261 WHERE `entry` = 51312;
UPDATE `items` SET `itemset` = 261 WHERE `entry` = 51313;
UPDATE `items` SET `itemset` = 261 WHERE `entry` = 51314;
UPDATE `items` SET `itemset` = 852 WHERE `entry` = 50846;
UPDATE `items` SET `itemset` = 852 WHERE `entry` = 50847;
UPDATE `items` SET `itemset` = 852 WHERE `entry` = 50848;
UPDATE `items` SET `itemset` = 852 WHERE `entry` = 50849;
UPDATE `items` SET `itemset` = 852 WHERE `entry` = 50850;
UPDATE `items` SET `itemset` = 241 WHERE `entry` = 51215;
UPDATE `items` SET `itemset` = 241 WHERE `entry` = 51216;
UPDATE `items` SET `itemset` = 241 WHERE `entry` = 51217;
UPDATE `items` SET `itemset` = 241 WHERE `entry` = 51218;
UPDATE `items` SET `itemset` = 241 WHERE `entry` = 51219;
UPDATE `items` SET `itemset` = 260 WHERE `entry` = 51220;
UPDATE `items` SET `itemset` = 260 WHERE `entry` = 51221;
UPDATE `items` SET `itemset` = 260 WHERE `entry` = 51222;
UPDATE `items` SET `itemset` = 260 WHERE `entry` = 51223;
UPDATE `items` SET `itemset` = 260 WHERE `entry` = 51224;
UPDATE `items` SET `itemset` = 851 WHERE `entry` = 50078;
UPDATE `items` SET `itemset` = 851 WHERE `entry` = 50079;
UPDATE `items` SET `itemset` = 851 WHERE `entry` = 50080;
UPDATE `items` SET `itemset` = 851 WHERE `entry` = 50081;
UPDATE `items` SET `itemset` = 851 WHERE `entry` = 50082;
UPDATE `items` SET `itemset` = 240 WHERE `entry` = 51210;
UPDATE `items` SET `itemset` = 240 WHERE `entry` = 51211;
UPDATE `items` SET `itemset` = 240 WHERE `entry` = 51212;
UPDATE `items` SET `itemset` = 240 WHERE `entry` = 51213;
UPDATE `items` SET `itemset` = 240 WHERE `entry` = 51214;
UPDATE `items` SET `itemset` = 259 WHERE `entry` = 51225;
UPDATE `items` SET `itemset` = 259 WHERE `entry` = 51226;
UPDATE `items` SET `itemset` = 259 WHERE `entry` = 51227;
UPDATE `items` SET `itemset` = 259 WHERE `entry` = 51228;
UPDATE `items` SET `itemset` = 259 WHERE `entry` = 51229;
UPDATE `items` SET `itemset` = 258 WHERE `entry` = 51240;
UPDATE `items` SET `itemset` = 258 WHERE `entry` = 51241;
UPDATE `items` SET `itemset` = 258 WHERE `entry` = 51242;
UPDATE `items` SET `itemset` = 258 WHERE `entry` = 51243;
UPDATE `items` SET `itemset` = 258 WHERE `entry` = 51244;
UPDATE `items` SET `itemset` = 239 WHERE `entry` = 51195;
UPDATE `items` SET `itemset` = 239 WHERE `entry` = 51196;
UPDATE `items` SET `itemset` = 239 WHERE `entry` = 51197;
UPDATE `items` SET `itemset` = 239 WHERE `entry` = 51198;
UPDATE `items` SET `itemset` = 239 WHERE `entry` = 51199;
UPDATE `items` SET `itemset` = 850 WHERE `entry` = 50830;
UPDATE `items` SET `itemset` = 850 WHERE `entry` = 50831;
UPDATE `items` SET `itemset` = 850 WHERE `entry` = 50832;
UPDATE `items` SET `itemset` = 850 WHERE `entry` = 50833;
UPDATE `items` SET `itemset` = 850 WHERE `entry` = 50834;
UPDATE `items` SET `itemset` = 257 WHERE `entry` = 51235;
UPDATE `items` SET `itemset` = 257 WHERE `entry` = 51236;
UPDATE `items` SET `itemset` = 257 WHERE `entry` = 51237;
UPDATE `items` SET `itemset` = 257 WHERE `entry` = 51238;
UPDATE `items` SET `itemset` = 257 WHERE `entry` = 51239;
UPDATE `items` SET `itemset` = 238 WHERE `entry` = 51200;
UPDATE `items` SET `itemset` = 238 WHERE `entry` = 51201;
UPDATE `items` SET `itemset` = 238 WHERE `entry` = 51202;
UPDATE `items` SET `itemset` = 238 WHERE `entry` = 51203;
UPDATE `items` SET `itemset` = 238 WHERE `entry` = 51204;
UPDATE `items` SET `itemset` = 849 WHERE `entry` = 50841;
UPDATE `items` SET `itemset` = 849 WHERE `entry` = 50842;
UPDATE `items` SET `itemset` = 849 WHERE `entry` = 50843;
UPDATE `items` SET `itemset` = 849 WHERE `entry` = 50844;
UPDATE `items` SET `itemset` = 849 WHERE `entry` = 50845;
UPDATE `items` SET `itemset` = 256 WHERE `entry` = 51245;
UPDATE `items` SET `itemset` = 256 WHERE `entry` = 51246;
UPDATE `items` SET `itemset` = 256 WHERE `entry` = 51247;
UPDATE `items` SET `itemset` = 256 WHERE `entry` = 51248;
UPDATE `items` SET `itemset` = 256 WHERE `entry` = 51249;
UPDATE `items` SET `itemset` = 237 WHERE `entry` = 51190;
UPDATE `items` SET `itemset` = 237 WHERE `entry` = 51191;
UPDATE `items` SET `itemset` = 237 WHERE `entry` = 51192;
UPDATE `items` SET `itemset` = 237 WHERE `entry` = 51193;
UPDATE `items` SET `itemset` = 237 WHERE `entry` = 51194;
UPDATE `items` SET `itemset` = 848 WHERE `entry` = 50835;
UPDATE `items` SET `itemset` = 848 WHERE `entry` = 50836;
UPDATE `items` SET `itemset` = 848 WHERE `entry` = 50837;
UPDATE `items` SET `itemset` = 848 WHERE `entry` = 50838;
UPDATE `items` SET `itemset` = 848 WHERE `entry` = 50839;
UPDATE `items` SET `itemset` = 255 WHERE `entry` = 51285;
UPDATE `items` SET `itemset` = 255 WHERE `entry` = 51286;
UPDATE `items` SET `itemset` = 255 WHERE `entry` = 51287;
UPDATE `items` SET `itemset` = 255 WHERE `entry` = 51288;
UPDATE `items` SET `itemset` = 255 WHERE `entry` = 51289;
UPDATE `items` SET `itemset` = 236 WHERE `entry` = 51150;
UPDATE `items` SET `itemset` = 236 WHERE `entry` = 51151;
UPDATE `items` SET `itemset` = 236 WHERE `entry` = 51152;
UPDATE `items` SET `itemset` = 236 WHERE `entry` = 51153;
UPDATE `items` SET `itemset` = 236 WHERE `entry` = 51154;
UPDATE `items` SET `itemset` = 847 WHERE `entry` = 50114;
UPDATE `items` SET `itemset` = 847 WHERE `entry` = 50115;
UPDATE `items` SET `itemset` = 847 WHERE `entry` = 50116;
UPDATE `items` SET `itemset` = 847 WHERE `entry` = 50117;
UPDATE `items` SET `itemset` = 847 WHERE `entry` = 50118;
UPDATE `items` SET `itemset` = 254 WHERE `entry` = 51250;
UPDATE `items` SET `itemset` = 254 WHERE `entry` = 51251;
UPDATE `items` SET `itemset` = 254 WHERE `entry` = 51252;
UPDATE `items` SET `itemset` = 254 WHERE `entry` = 51253;
UPDATE `items` SET `itemset` = 254 WHERE `entry` = 51254;
UPDATE `items` SET `itemset` = 235 WHERE `entry` = 51185;
UPDATE `items` SET `itemset` = 235 WHERE `entry` = 51186;
UPDATE `items` SET `itemset` = 235 WHERE `entry` = 51187;
UPDATE `items` SET `itemset` = 235 WHERE `entry` = 51188;
UPDATE `items` SET `itemset` = 235 WHERE `entry` = 51189;
UPDATE `items` SET `itemset` = 846 WHERE `entry` = 50087;
UPDATE `items` SET `itemset` = 846 WHERE `entry` = 50088;
UPDATE `items` SET `itemset` = 846 WHERE `entry` = 50089;
UPDATE `items` SET `itemset` = 846 WHERE `entry` = 50090;
UPDATE `items` SET `itemset` = 846 WHERE `entry` = 50105;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2015-12-14-01_items_itemset' WHERE `LastUpdate` = '2015-12-13-02_event_call_to_arms';
