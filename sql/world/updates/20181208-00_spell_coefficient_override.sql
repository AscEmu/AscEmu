-- Create new coefficient override table
CREATE TABLE IF NOT EXISTS `spell_coefficient_override` (
  `spell_id` int(10) unsigned NOT NULL DEFAULT '0',
  `min_build` int(6) NOT NULL DEFAULT '12340',
  `max_build` int(6) NOT NULL DEFAULT '12340',
  `direct_coefficient` float NOT NULL DEFAULT '-1',
  `overtime_coefficient` float NOT NULL DEFAULT '-1',
  `description` varchar(300) DEFAULT NULL,
  PRIMARY KEY (`spell_id`,`min_build`),
  UNIQUE KEY `unique_index` (`spell_id`,`min_build`) USING BTREE,
  KEY `spell_id` (`spell_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Spell System';

-- Delete old coefficients from override table
DELETE FROM `spell_custom_override` WHERE `rank` IS NULL AND `assign_on_target_flag` IS NULL AND `assign_from_caster_on_self_flag` IS NULL
AND `assign_self_cast_only` IS NULL AND `assign_c_is_flag` IS NULL AND `proc_on_namehash` IS NULL AND `proc_flags` IS NULL
AND `proc_target_selfs` IS NULL AND `proc_chance` IS NULL AND `proc_charges` IS NULL AND `proc_interval` IS NULL AND `proc_effect_trigger_spell_0` IS NULL
AND `proc_effect_trigger_spell_1` IS NULL AND `proc_effect_trigger_spell_2` IS NULL;

INSERT INTO `world_db_version` VALUES ('45', '20181208-00_spell_coefficient_override');
