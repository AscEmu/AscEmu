DROP TABLE IF EXISTS `spell_required`;

CREATE TABLE `spell_required` (
  `spell_id` mediumint(8) NOT NULL DEFAULT '0',
  `req_spell` mediumint(8) NOT NULL DEFAULT '0',
  PRIMARY KEY (`spell_id`,`req_spell`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Spell Additinal Data';

INSERT INTO `spell_required` VALUES
(9788, 9785),
(10656, 10662),
(10658, 10662),
(10660, 10662),
(16689, 339),
(17039, 9787),
(17040, 9787),
(17041, 9787),
(20219, 12656),
(20222, 12656),
(23161, 5784),
(23161, 33391),
(23214, 13819),
(23214, 33391),
(26797, 26790),
(26798, 26790),
(26801, 26790),
(28672, 28596),
(28675, 28596),
(28677, 28596),
(34767, 33391),
(34767, 34769);

INSERT INTO `world_db_version` VALUES ('47', '20190806-00_spell_required');
