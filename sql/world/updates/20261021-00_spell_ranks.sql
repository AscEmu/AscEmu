DELETE FROM `spell_ranks` WHERE `first_spell` IN ('23784', '23759', '23760', '23761', '23762', '35702');
INSERT INTO `spell_ranks` VALUES
    -- Master Demonologist script effect
    ('23784', '5875', '15595', '23784', '1', 'Master Demonologist - Script'),
    ('23830', '5875', '15595', '23784', '2', 'Master Demonologist - Script'),
    ('23831', '5875', '15595', '23784', '3', 'Master Demonologist - Script'),
    ('23832', '5875', '15595', '23784', '4', 'Master Demonologist - Script'),
    ('35708', '8606', '15595', '23784', '5', 'Master Demonologist - Script'),
    -- Master Demonologist imp effect
    ('23759', '5875', '15595', '23759', '1', 'Master Demonologist - Imp'),
    ('23826', '5875', '15595', '23759', '2', 'Master Demonologist - Imp'),
    ('23827', '5875', '15595', '23759', '3', 'Master Demonologist - Imp'),
    ('23828', '5875', '15595', '23759', '4', 'Master Demonologist - Imp'),
    ('23829', '5875', '15595', '23759', '5', 'Master Demonologist - Imp'),
    -- Master Demonologist voidwalker effect
    ('23760', '5875', '15595', '23760', '1', 'Master Demonologist - Voidwalker'),
    ('23841', '5875', '15595', '23760', '2', 'Master Demonologist - Voidwalker'),
    ('23842', '5875', '15595', '23760', '3', 'Master Demonologist - Voidwalker'),
    ('23843', '5875', '15595', '23760', '4', 'Master Demonologist - Voidwalker'),
    ('23844', '5875', '15595', '23760', '5', 'Master Demonologist - Voidwalker'),
    -- Master Demonologist succubus effect
    ('23761', '5875', '15595', '23761', '1', 'Master Demonologist - Succubus'),
    ('23833', '5875', '15595', '23761', '2', 'Master Demonologist - Succubus'),
    ('23834', '5875', '15595', '23761', '3', 'Master Demonologist - Succubus'),
    ('23835', '5875', '15595', '23761', '4', 'Master Demonologist - Succubus'),
    ('23836', '5875', '15595', '23761', '5', 'Master Demonologist - Succubus'),
    -- Master Demonologist felhunter effect
    ('23762', '5875', '15595', '23762', '1', 'Master Demonologist - Felhunter'),
    ('23837', '5875', '15595', '23762', '2', 'Master Demonologist - Felhunter'),
    ('23838', '5875', '15595', '23762', '3', 'Master Demonologist - Felhunter'),
    ('23839', '5875', '15595', '23762', '4', 'Master Demonologist - Felhunter'),
    ('23840', '5875', '15595', '23762', '5', 'Master Demonologist - Felhunter'),
    -- Master Demonologist felguard effect
    ('35702', '8606', '15595', '35702', '1', 'Master Demonologist - Felguard'),
    ('35703', '8606', '15595', '35702', '2', 'Master Demonologist - Felguard'),
    ('35704', '8606', '15595', '35702', '3', 'Master Demonologist - Felguard'),
    ('35705', '8606', '15595', '35702', '4', 'Master Demonologist - Felguard'),
    ('35706', '8606', '15595', '35702', '5', 'Master Demonologist - Felguard');

-- Remove unused "assign_c_is_flag" values
UPDATE `spell_custom_override`
SET `assign_c_is_flag`= assign_c_is_flag & ~(0x200 | 0x400 | 0x800);

-- Remove empty rows
DELETE FROM `spell_custom_override` WHERE
    (assign_on_target_flag IS NULL OR assign_on_target_flag='0') AND
    (assign_self_cast_only IS NULL OR assign_self_cast_only='0') AND
    (assign_c_is_flag IS NULL OR assign_c_is_flag='0') AND
    (proc_flags IS NULL OR proc_flags='0') AND
    (proc_target_selfs IS NULL OR proc_target_selfs='0') AND
    (proc_chance IS NULL OR proc_chance='0') AND
    (proc_charges IS NULL OR proc_charges='0') AND
    (proc_interval IS NULL OR proc_interval='0') AND
    (proc_effect_trigger_spell_0 IS NULL OR proc_effect_trigger_spell_0='0') AND
    (proc_effect_trigger_spell_1 IS NULL OR proc_effect_trigger_spell_1='0') AND
    (proc_effect_trigger_spell_2 IS NULL OR proc_effect_trigger_spell_2='0');

DELETE FROM `spell_coefficient_override` WHERE `spell_id`='33763';
INSERT INTO `spell_coefficient_override` VALUES
    -- Lifebloom hot 7.42% sp tbc
    ('33763', '0', '8606', '8606', '0.0742', NULL, '0', 'Lifebloom hot'),
    -- Lifebloom final heal 34.39% sp tbc
    ('33763', '1', '8606', '8606', '0.3439', NULL, '0', 'Lifebloom final'),
    -- Lifebloom final heal 38.57% sp wotlk
    ('33763', '1', '12340', '12340', '0.3857', NULL, '0', 'Lifebloom final');

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('199', '20261021-00_spell_ranks');
