-- Drop unused columns from spell_custom_override
ALTER TABLE `spell_custom_override` DROP `assign_from_caster_on_self_flag`, DROP `coef_flags`, DROP `coef_Dspell`, DROP `coef_Otspell`, DROP `proc_on_namehash`;

INSERT INTO `world_db_version` VALUES ('46', '20190114-00_spell_custom_override');
