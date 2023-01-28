/*create new table for trainer properties and copy data/structure from old table*/
DROP TABLE IF EXISTS `trainer_properties`;
CREATE TABLE `trainer_properties` AS SELECT * FROM `trainer_defs`;

/*add new columns*/
ALTER TABLE `trainer_properties` ADD COLUMN `build` INT DEFAULT 12340 AFTER `entry`;
ALTER TABLE `trainer_properties` ADD COLUMN `spellset_id` INT DEFAULT 0 AFTER `cannot_train_gossip_textid`;
ALTER TABLE `trainer_properties` ADD COLUMN `can_train_max_level` INT DEFAULT 0 AFTER `spellset_id`;
ALTER TABLE `trainer_properties` ADD COLUMN `can_train_min_skill_value` INT DEFAULT 0 AFTER `can_train_max_level`;
ALTER TABLE `trainer_properties` ADD COLUMN `can_train_max_skill_value` INT DEFAULT 0 AFTER `can_train_min_skill_value`;
ALTER TABLE `trainer_properties` ADD COLUMN `comment` VARCHAR(250) DEFAULT '' AFTER `can_train_max_skill_value`;

/*add subname to help us*/
UPDATE `trainer_properties` t1, `creature_properties` t2
SET t1.comment = t2.subname, t1.build = t2.build
WHERE t1.entry = t2.entry;

/*Add spellset_id id (template) to the trainer_properties (based on old trainer_spells table*/
UPDATE `trainer_properties` SET `spellset_id` = 99 WHERE `entry` IN(1317,3011,3345,3606,4213,4616,5157,5695,7949,11072,11073,11074,16160,16633,16725);
UPDATE `trainer_properties` SET `spellset_id` = 98 WHERE `entry` IN(18753,18773,19251,19252,19540,33633,33676);
UPDATE `trainer_properties` SET `spellset_id` = 97 WHERE `entry` IN(33610);
UPDATE `trainer_properties` SET `spellset_id` = 96 WHERE `entry` IN(2818);
UPDATE `trainer_properties` SET `spellset_id` = 95 WHERE `entry` IN(1676,1702,3290,3494,5174,5518,8736,10993,11017,11025,11031,11037,16667,16726,17222);
UPDATE `trainer_properties` SET `spellset_id` = 94 WHERE `entry` IN(33634);
UPDATE `trainer_properties` SET `spellset_id` = 93 WHERE `entry` IN(33677);
UPDATE `trainer_properties` SET `spellset_id` = 92 WHERE `entry` IN(25277,26907,26955,26991,28697,33586,33611);
UPDATE `trainer_properties` SET `spellset_id` = 91 WHERE `entry` IN(17637);
UPDATE `trainer_properties` SET `spellset_id` = 90 WHERE `entry` IN(19576);
UPDATE `trainer_properties` SET `spellset_id` = 9 WHERE `entry` IN(925,926,15280,16501);
UPDATE `trainer_properties` SET `spellset_id` = 89 WHERE `entry` IN(18775);
UPDATE `trainer_properties` SET `spellset_id` = 88 WHERE `entry` IN(18752);
UPDATE `trainer_properties` SET `spellset_id` = 87 WHERE `entry` IN(17634);
UPDATE `trainer_properties` SET `spellset_id` = 86 WHERE `entry` IN(2326,2327,2329,2798,3181,3373,4211,4591,5150,5759,5939,5943,6094,16272,16662,16731,17214,17424,19184,19478,22477);
UPDATE `trainer_properties` SET `spellset_id` = 85 WHERE `entry` IN(18990,18991);
UPDATE `trainer_properties` SET `spellset_id` = 84 WHERE `entry` IN(23734,26956,26992,28706,29233,33589,33621);
UPDATE `trainer_properties` SET `spellset_id` = 83 WHERE `entry` IN(1681,1701,3001,3137,3175,3357,3555,4254,4598,5392,5513,6297,8128,16663,16752,17488,18804);
UPDATE `trainer_properties` SET `spellset_id` = 82 WHERE `entry` IN(18747,18779,33640,33682);
UPDATE `trainer_properties` SET `spellset_id` = 81 WHERE `entry` IN(26912,26962,26976,26999,28698,33617);
UPDATE `trainer_properties` SET `spellset_id` = 80 WHERE `entry` IN(1355,1382,1430,1699,3026,3067,3087,3399,4210,4552,5159,5482,6286,8306,16253,16277,16676,16719,17246,19185,19369,34708,34710,34711,34712,34713,34714,34785,34786);
UPDATE `trainer_properties` SET `spellset_id` = 79 WHERE `entry` IN(18987,18988,18993);
UPDATE `trainer_properties` SET `spellset_id` = 78 WHERE `entry` IN(26905,26953,26972,26989,28705,29631,33587,33619);
UPDATE `trainer_properties` SET `spellset_id` = 77 WHERE `entry` IN(1103,1346,2399,2627,3004,3363,3484,3523,3704,4159,4193,4576,5153,11052,11557,16366,16640,16729,17487);
UPDATE `trainer_properties` SET `spellset_id` = 76 WHERE `entry` IN(18749,18772,33636,33684);
UPDATE `trainer_properties` SET `spellset_id` = 75 WHERE `entry` IN(26914,26964,26969,27001,28699,33580,33613);
UPDATE `trainer_properties` SET `spellset_id` = 74 WHERE `entry` IN(26910,26958,26974,26994,28704,33616);
UPDATE `trainer_properties` SET `spellset_id` = 73 WHERE `entry` IN(18748,18776,33639,33678);
UPDATE `trainer_properties` SET `spellset_id` = 72 WHERE `entry` IN(812,908,1218,1458,1473,2114,2390,2856,3013,3185,3404,3604,3965,4204,4614,4898,5137,5502,5566,8146,12025,16367,16644,16736,17434,17983,33996);
UPDATE `trainer_properties` SET `spellset_id` = 71 WHERE `entry` IN(17215);
UPDATE `trainer_properties` SET `spellset_id` = 70 WHERE `entry` IN(1215,1386,1470,2132,2391,2837,3009,3184,3347,3603,3964,4160,4611,4900,5177,5499,7948,16161,16642,16723);
UPDATE `trainer_properties` SET `spellset_id` = 7 WHERE `entry` IN(16275,16679,16680,16681,20406,23128);
UPDATE `trainer_properties` SET `spellset_id` = 69 WHERE `entry` IN(16588,18802,19052,27023,27029,33630,33674);
UPDATE `trainer_properties` SET `spellset_id` = 68 WHERE `entry` IN(33608);
UPDATE `trainer_properties` SET `spellset_id` = 67 WHERE `entry` IN(26911,26961,26998,28700,33581);
UPDATE `trainer_properties` SET `spellset_id` = 66 WHERE `entry` IN(26996,33612);
UPDATE `trainer_properties` SET `spellset_id` = 65 WHERE `entry` IN(18754,18771,19187,21087,33635,33681);
UPDATE `trainer_properties` SET `spellset_id` = 64 WHERE `entry` IN(1385,1632,3007,3069,3365,3549,3605,3703,3967,4212,4588,5127,5564,5784,8153,11097,11098,16278,16688,16728,17442);
UPDATE `trainer_properties` SET `spellset_id` = 63 WHERE `entry` IN(514,1241,2836,2998,3136,3174,3355,3478,3557,4258,4596,5511,6299,15400,16669,16724,17245);
UPDATE `trainer_properties` SET `spellset_id` = 62 WHERE `entry` IN(26564,26904,26952,26981,26988,27034,28694,29924,33591,33609);
UPDATE `trainer_properties` SET `spellset_id` = 61 WHERE `entry` IN(16583,16823,19341,33631,33675);
UPDATE `trainer_properties` SET `spellset_id` = 60 WHERE `entry` IN(13084);
UPDATE `trainer_properties` SET `spellset_id` = 6 WHERE `entry` IN(927,928,1232,5147,5148,5149,5491,5492,8140,16761,17121,17483,17509,17844,35281);
UPDATE `trainer_properties` SET `spellset_id` = 59 WHERE `entry` IN(11866);
UPDATE `trainer_properties` SET `spellset_id` = 58 WHERE `entry` IN(11870);
UPDATE `trainer_properties` SET `spellset_id` = 57 WHERE `entry` IN(16621,17005);
UPDATE `trainer_properties` SET `spellset_id` = 56 WHERE `entry` IN(11867);
UPDATE `trainer_properties` SET `spellset_id` = 55 WHERE `entry` IN(11869);
UPDATE `trainer_properties` SET `spellset_id` = 54 WHERE `entry` IN(16773);
UPDATE `trainer_properties` SET `spellset_id` = 52 WHERE `entry` IN(11868);
UPDATE `trainer_properties` SET `spellset_id` = 51 WHERE `entry` IN(2704);
UPDATE `trainer_properties` SET `spellset_id` = 50 WHERE `entry` IN(11865);
UPDATE `trainer_properties` SET `spellset_id` = 5 WHERE `entry` IN(911,912,2119,3059,3153,3593,16503);
UPDATE `trainer_properties` SET `spellset_id` = 49 WHERE `entry` IN(20914);
UPDATE `trainer_properties` SET `spellset_id` = 48 WHERE `entry` IN(16280);
UPDATE `trainer_properties` SET `spellset_id` = 47 WHERE `entry` IN(7953);
UPDATE `trainer_properties` SET `spellset_id` = 46 WHERE `entry` IN(7954);
UPDATE `trainer_properties` SET `spellset_id` = 45 WHERE `entry` IN(3690);
UPDATE `trainer_properties` SET `spellset_id` = 44 WHERE `entry` IN(4773);
UPDATE `trainer_properties` SET `spellset_id` = 43 WHERE `entry` IN(4753);
UPDATE `trainer_properties` SET `spellset_id` = 42 WHERE `entry` IN(4772);
UPDATE `trainer_properties` SET `spellset_id` = 41 WHERE `entry` IN(4752);
UPDATE `trainer_properties` SET `spellset_id` = 40 WHERE `entry` IN(4732);
UPDATE `trainer_properties` SET `spellset_id` = 4 WHERE `entry` IN(913,914,985,1229,1901,2131,3041,3042,3043,3063,3169,3353,3354,3408,3598,4087,4089,4593,4594,4595,5113,5114,5479,5480,7315,8141,16771,17120,17480,17504);
UPDATE `trainer_properties` SET `spellset_id` = 39 WHERE `entry` IN(28746,31238,31247);
UPDATE `trainer_properties` SET `spellset_id` = 38 WHERE `entry` IN(20500,20511,35093,35100,35133,35135);
UPDATE `trainer_properties` SET `spellset_id` = 37 WHERE `entry` IN(3060,3597);
UPDATE `trainer_properties` SET `spellset_id` = 36 WHERE `entry` IN(3033,3034,3036,3064,3602,4217,4218,4219,5504,5505,5506,8142,9465,12042,16655,16721);
UPDATE `trainer_properties` SET `spellset_id` = 35 WHERE `entry` IN(459,460,2126,3156,15283);
UPDATE `trainer_properties` SET `spellset_id` = 34 WHERE `entry` IN(461,906,988,2127,3172,3324,3325,3326,4563,4564,4565,5171,5172,5173,5495,5496,5612,16266,16646,16647,16648,23534);
UPDATE `trainer_properties` SET `spellset_id` = 33 WHERE `entry` IN(29156);
UPDATE `trainer_properties` SET `spellset_id` = 32 WHERE `entry` IN(27703);
UPDATE `trainer_properties` SET `spellset_id` = 31 WHERE `entry` IN(27705);
UPDATE `trainer_properties` SET `spellset_id` = 30 WHERE `entry` IN(19340);
UPDATE `trainer_properties` SET `spellset_id` = 29 WHERE `entry` IN(20791);
UPDATE `trainer_properties` SET `spellset_id` = 28 WHERE `entry` IN(16654);
UPDATE `trainer_properties` SET `spellset_id` = 27 WHERE `entry` IN(16755);
UPDATE `trainer_properties` SET `spellset_id` = 26 WHERE `entry` IN(5958);
UPDATE `trainer_properties` SET `spellset_id` = 25 WHERE `entry` IN(5957);
UPDATE `trainer_properties` SET `spellset_id` = 24 WHERE `entry` IN(4165);
UPDATE `trainer_properties` SET `spellset_id` = 23 WHERE `entry` IN(2492);
UPDATE `trainer_properties` SET `spellset_id` = 22 WHERE `entry` IN(2489);
UPDATE `trainer_properties` SET `spellset_id` = 21 WHERE `entry` IN(2485);
UPDATE `trainer_properties` SET `spellset_id` = 20 WHERE `entry` IN(198,944,2124,15279,16500);
UPDATE `trainer_properties` SET `spellset_id` = 2 WHERE `entry` IN(6251);
UPDATE `trainer_properties` SET `spellset_id` = 19 WHERE `entry` IN(328,331,1228,2128,3047,3048,3049,4566,4567,4568,5144,5145,5146,5497,5498,5880,5882,5883,5884,5885,7311,7312,16269,16651,16652,16653,16749,17105,17481,17513,17514,23103,27704,28956,28958);
UPDATE `trainer_properties` SET `spellset_id` = 18 WHERE `entry` IN(3062,3157,17089);
UPDATE `trainer_properties` SET `spellset_id` = 17 WHERE `entry` IN(986,3030,3031,3032,3066,3173,3344,3403,13417,17204,17212,17219,17519,17520,20407,23127);
UPDATE `trainer_properties` SET `spellset_id` = 16 WHERE `entry` IN(28471,28472,28474,29194,29195,29196,31084);
UPDATE `trainer_properties` SET `spellset_id` = 15 WHERE `entry` IN(375,837,2123,3707,15284,16502,17482);
UPDATE `trainer_properties` SET `spellset_id` = 14 WHERE `entry` IN(376,377,1226,2129,3044,3045,3046,3595,3600,3706,4090,4091,4092,4606,4607,4608,5141,5142,5143,5484,5489,5994,6014,6018,11397,11401,11406,16276,16658,16659,16660,16756,17510,17511);
UPDATE `trainer_properties` SET `spellset_id` = 13 WHERE `entry` IN(915,916,2122,3155,3594,15285);
UPDATE `trainer_properties` SET `spellset_id` = 127 WHERE `entry` IN(5164,7230,11177,20125);
UPDATE `trainer_properties` SET `spellset_id` = 126 WHERE `entry` IN(7231,7232,11146,11178,20124,29505);
UPDATE `trainer_properties` SET `spellset_id` = 125 WHERE `entry` IN(26903,26951,26975,26987,28703,33588);
UPDATE `trainer_properties` SET `spellset_id` = 124 WHERE `entry` IN(30706,30709,30710,30711,30713,30715,30716,30717);
UPDATE `trainer_properties` SET `spellset_id` = 123 WHERE `entry` IN(30721,30722,33638,33679);
UPDATE `trainer_properties` SET `spellset_id` = 122 WHERE `entry` IN(26916,26959,26977,26995,28702,33603,33615);
UPDATE `trainer_properties` SET `spellset_id` = 121 WHERE `entry` IN(24868,25099);
UPDATE `trainer_properties` SET `spellset_id` = 120 WHERE `entry` IN(19186);
UPDATE `trainer_properties` SET `spellset_id` = 12 WHERE `entry` IN(917,918,1234,1411,2130,3170,3327,3328,3401,3599,4163,4214,4215,4582,4583,4584,5165,5166,5167,6707,13283,16279,16684,16685,16686);
UPDATE `trainer_properties` SET `spellset_id` = 117 WHERE `entry` IN(26906,26954,26980,26990,28693,33583);
UPDATE `trainer_properties` SET `spellset_id` = 116 WHERE `entry` IN(15501,19775,19778);
UPDATE `trainer_properties` SET `spellset_id` = 115 WHERE `entry` IN(18751,18774,19539,33637,33680);
UPDATE `trainer_properties` SET `spellset_id` = 114 WHERE `entry` IN(19063,26915,26960,26982,26997,28701,33590,33614);
UPDATE `trainer_properties` SET `spellset_id` = 112 WHERE `entry` IN(7406,7944,29514);
UPDATE `trainer_properties` SET `spellset_id` = 111 WHERE `entry` IN(4578,9584);
UPDATE `trainer_properties` SET `spellset_id` = 110 WHERE `entry` IN(7868,7869,29507);
UPDATE `trainer_properties` SET `spellset_id` = 11 WHERE `entry` IN(895,3061,3154,3596,15513,16499);
UPDATE `trainer_properties` SET `spellset_id` = 109 WHERE `entry` IN(7870,7871,29509);
UPDATE `trainer_properties` SET `spellset_id` = 108 WHERE `entry` IN(7866,7867,29508);
UPDATE `trainer_properties` SET `spellset_id` = 107 WHERE `entry` IN(29506);
UPDATE `trainer_properties` SET `spellset_id` = 106 WHERE `entry` IN(8126,8738,29513);
UPDATE `trainer_properties` SET `spellset_id` = 105 WHERE `entry` IN(26913,26963,26986,27000,28696,33618);
UPDATE `trainer_properties` SET `spellset_id` = 104 WHERE `entry` IN(18755,18777,19180,33641,33683);
UPDATE `trainer_properties` SET `spellset_id` = 103 WHERE `entry` IN(1292,6287,6288,6289,6290,6291,6292,6295,6306,6387,7087,7088,7089,8144,12030,16273,16692,16763,17441);
UPDATE `trainer_properties` SET `spellset_id` = 102 WHERE `entry` IN(18911);
UPDATE `trainer_properties` SET `spellset_id` = 101 WHERE `entry` IN(1651,1680,1683,1700,2367,2834,3028,3179,3332,3607,4156,4573,5161,5493,5690,5938,5941,7946,12032,12961,14740,16774,16780,17101,18018);
UPDATE `trainer_properties` SET `spellset_id` = 100 WHERE `entry` IN(26909,26957,26993,28742,32474,33623);
UPDATE `trainer_properties` SET `spellset_id` = 10 WHERE `entry` IN(987,1231,1404,3038,3039,3040,3065,3171,3352,3406,3407,3601,3963,4138,4146,4205,5115,5116,5117,5501,5515,5516,5517,8308,10930,16270,16672,16673,16674,16738,17110,17122,17505);

/*Add column temp_id to old table trainer_spells*/
ALTER TABLE `trainer_spells` ADD COLUMN `temp_id` INT DEFAULT 0 AFTER `deletespell`;

/*Add temp_id values to old table*/
UPDATE `trainer_spells` t1, `trainer_properties` t2 
SET t1.temp_id = t2.spellset_id
WHERE t1.entry = t2.entry;

/*create new table fo trainer spell definitions and copy data/structure from old table*/
DROP TABLE IF EXISTS `trainer_properties_spellset`;
CREATE TABLE `trainer_properties_spellset` (
  `id` int unsigned NOT NULL DEFAULT '0',
  `min_build` int unsigned NOT NULL DEFAULT '4044',
  `max_build` int unsigned NOT NULL DEFAULT '15595',
  `cast_spell` int unsigned NOT NULL DEFAULT '0',
  `learn_spell` int unsigned NOT NULL,
  `spellcost` int unsigned NOT NULL DEFAULT '0',
  `reqspell` int unsigned NOT NULL DEFAULT '0',
  `reqskill` int unsigned NOT NULL DEFAULT '0',
  `reqskillvalue` int unsigned NOT NULL DEFAULT '0',
  `reqlevel` int unsigned NOT NULL DEFAULT '0',
  `deletespell` int unsigned NOT NULL DEFAULT '0',
  `static` int unsigned NOT NULL DEFAULT '0',
  UNIQUE KEY (`id`,`min_build`,`cast_spell`,`learn_spell`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Trainer System';

/*Add data from our old table to the new one. we replace all data with key id, cast_spell, learn_spell so we end up with unique spells for each id*/
REPLACE INTO `trainer_properties_spellset` (`id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell`)
SELECT `temp_id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell` FROM `trainer_spells`;

/*update trainer_properties class trainers with highest reqlevel*/
UPDATE trainer_properties SET spellset_id =34, can_train_max_level = 6 WHERE spellset_id = 35;
DELETE FROM trainer_properties_spellset WHERE id = 35;

UPDATE trainer_properties SET spellset_id = 4, can_train_max_level = 6 WHERE spellset_id = 5;
DELETE FROM trainer_properties_spellset WHERE id = 5;

UPDATE trainer_properties SET spellset_id = 17, can_train_max_level = 6 WHERE spellset_id = 18;
DELETE FROM trainer_properties_spellset WHERE id = 18;

UPDATE trainer_properties SET spellset_id = 12, can_train_max_level = 6 WHERE spellset_id = 13;
DELETE FROM trainer_properties_spellset WHERE id = 13;

UPDATE trainer_properties SET spellset_id = 14 WHERE spellset_id = 15;
DELETE FROM trainer_properties_spellset WHERE id = 15;

UPDATE trainer_properties SET spellset_id = 6 WHERE spellset_id = 7;
UPDATE trainer_properties SET spellset_id = 6, can_train_max_level = 6 WHERE spellset_id = 9;
UPDATE trainer_properties_spellset SET id = 6 WHERE cast_spell IN(34768,34766);
UPDATE trainer_properties_spellset SET id = 6 WHERE learn_spell = 53736;
DELETE FROM trainer_properties_spellset WHERE id IN(7,9);

UPDATE trainer_properties SET spellset_id = 19, can_train_max_level = 6 WHERE spellset_id = 20;
DELETE FROM trainer_properties_spellset WHERE id = 20;

UPDATE trainer_properties SET spellset_id = 10, can_train_max_level = 6 WHERE spellset_id = 11;
UPDATE trainer_properties_spellset SET reqlevel = 1 WHERE id = 10 AND learn_spell = 1494;
UPDATE trainer_properties_spellset SET reqlevel = 4 WHERE id = 10 AND learn_spell = 1978;
UPDATE trainer_properties_spellset SET reqlevel = 6 WHERE id = 10 AND learn_spell = 1130;
DELETE FROM trainer_properties_spellset WHERE id = 11;

UPDATE trainer_properties SET spellset_id = 36, can_train_max_level = 6 WHERE spellset_id = 37;
DELETE FROM trainer_properties_spellset WHERE id = 37;

UPDATE trainer_properties SET spellset_id = 78, can_train_max_skill_value =275 WHERE spellset_id = 79;
DELETE FROM trainer_properties_spellset WHERE id = 79;
UPDATE trainer_properties SET spellset_id = 78, can_train_max_skill_value =200 WHERE spellset_id = 80;
DELETE FROM trainer_properties_spellset WHERE id = 80;

UPDATE trainer_properties SET spellset_id = 100, can_train_max_skill_value =200 WHERE spellset_id = 101;
DELETE FROM trainer_properties_spellset WHERE id = 101;
UPDATE trainer_properties SET spellset_id = 100, can_train_max_skill_value =275 WHERE spellset_id = 102;
DELETE FROM trainer_properties_spellset WHERE id = 102;

UPDATE trainer_properties SET spellset_id = 98, can_train_max_skill_value =295 WHERE spellset_id = 99;
DELETE FROM trainer_properties_spellset WHERE id = 99;
UPDATE trainer_properties SET spellset_id = 98, can_train_min_skill_value =350 WHERE spellset_id = 117;
UPDATE trainer_properties_spellset SET id = 98 WHERE id = 117;

UPDATE trainer_properties SET spellset_id = 107 WHERE spellset_id = 127;
DELETE FROM trainer_properties_spellset WHERE id = 127;

UPDATE trainer_properties SET can_train_max_skill_value = 350 WHERE spellset_id = 61;
UPDATE trainer_properties SET spellset_id = 61, can_train_min_skill_value = 300 WHERE spellset_id = 63;
DELETE FROM trainer_properties_spellset WHERE id = 63;
UPDATE trainer_properties SET spellset_id = 61 WHERE spellset_id = 62;
UPDATE trainer_properties_spellset SET id = 61 WHERE learn_spell IN(52568,52569,59405,55834,52572,54550,55835,52571,52567,52570,54917,55201,54918,55200,55203,54944,54941,55202,55174,54557,55013,55204,54945,59436,55055,55177,54551,54554,54946,54947,59438,55056,55179,54552,54553,54949,54948,55628,55641,55057,59440,55206,55181,54555,54556,56280,55182,55014,59442,55017,55058,59441,55656,55015,61009,61010,55301,56549,56553,56550,56551,56552,55300,55305,55309,55306,55307,55308,55298,56357,55839,55732,61008,55303,55302,56555,56554,56556,55304,55311,55310,55312,59406,55374,55377,55372,55375,55373,55376,55370,55369,55371,56234,56400,63182);
UPDATE trainer_properties_spellset SET id = 61 WHERE cast_spell = 51298;
DELETE FROM trainer_properties_spellset WHERE id = 62;

UPDATE trainer_properties SET spellset_id = 69, can_train_max_skill_value = 55 WHERE spellset_id = 71;
DELETE FROM trainer_properties_spellset WHERE id = 71;
UPDATE trainer_properties SET spellset_id = 69, can_train_max_skill_value = 285 WHERE spellset_id = 70;
DELETE FROM trainer_properties_spellset WHERE id = 70;

UPDATE trainer_properties SET can_train_max_skill_value = 325 WHERE spellset_id = 69;
UPDATE trainer_properties SET spellset_id = 69, can_train_min_skill_value =350 WHERE spellset_id = 125;
DELETE FROM trainer_properties_spellset WHERE id = 125 AND learn_spell = 53042;
UPDATE trainer_properties_spellset SET static = 1 WHERE id = 69 AND learn_spell = 53042;
UPDATE trainer_properties_spellset SET id = 69 WHERE id = 125;

UPDATE trainer_properties SET spellset_id = 105, can_train_max_skill_value = 25 WHERE spellset_id = 103;
DELETE FROM trainer_properties_spellset WHERE id = 103;
UPDATE trainer_properties SET spellset_id = 105, can_train_max_skill_value =40 WHERE spellset_id = 104;
DELETE FROM trainer_properties_spellset WHERE id = 104;

UPDATE trainer_properties SET spellset_id = 38, can_train_max_skill_value = 75 WHERE spellset_id IN(40,41,42,43,44,45,46,47,48,49);
DELETE FROM trainer_properties_spellset WHERE id IN(40,41,42,43,44,45,46,47,48,49);

UPDATE trainer_properties SET spellset_id = 87, can_train_max_skill_value = 375 WHERE spellset_id IN(87,88,89);
DELETE FROM trainer_properties_spellset WHERE id IN(88,89);
UPDATE trainer_properties SET spellset_id = 87, can_train_max_skill_value = 375 WHERE spellset_id IN(87,88,89);
DELETE FROM trainer_properties_spellset WHERE id IN(88,89);
UPDATE trainer_properties SET spellset_id = 90, can_train_max_skill_value = 375 WHERE spellset_id IN(91);
UPDATE trainer_properties_spellset SET id = 90 WHERE id = 91 AND learn_spell IN(41315,41316);
DELETE FROM trainer_properties_spellset WHERE id IN(91);
UPDATE trainer_properties SET spellset_id = 93, can_train_max_skill_value = 375 WHERE spellset_id IN(94);
DELETE FROM trainer_properties_spellset WHERE id IN(94);
UPDATE trainer_properties SET spellset_id = 93, can_train_max_skill_value = 290 WHERE spellset_id IN(95);
DELETE FROM trainer_properties_spellset WHERE id IN(95);

UPDATE trainer_properties SET spellset_id = 29 WHERE spellset_id = 30;
DELETE FROM trainer_properties_spellset WHERE id = 30;

UPDATE trainer_properties SET spellset_id = 73, can_train_max_skill_value = 200 WHERE spellset_id = 72;
DELETE FROM trainer_properties_spellset WHERE id = 72;

ALTER TABLE `trainer_spells` DROP COLUMN `temp_id`;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('116', '20230128_00_trainer_properties');
