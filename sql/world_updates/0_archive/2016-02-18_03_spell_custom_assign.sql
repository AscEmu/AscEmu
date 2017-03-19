--
-- Add column self_cast_only and description to spell_custom_assign
--
ALTER TABLE `spell_custom_assign` ADD self_cast_only TINYINT(1) unsigned NOT NULL DEFAULT 0 AFTER from_caster_on_self_flag;
ALTER TABLE `spell_custom_assign` ADD Description VARCHAR(300) NOT NULL DEFAULT '' AFTER self_cast_only;

--
-- Add data for self_cast_only spells
--
REPLACE INTO `spell_custom_assign` VALUES
  (8690, 0, 0, 1, 'Heartstone'),
  (54318, 0, 0, 1, 'Heartstone'),
  (7355, 0, 0, 1, 'Stuck'),
  (556, 0, 0, 1, 'Astral Recall');
  
--
-- Update spell_proc with data from c++
--
UPDATE `spell_proc` SET `target_self` = 1, `description` = 'Illumination Rank 1' WHERE `spell_id` = 20210;
UPDATE `spell_proc` SET `target_self` = 1, `description` = 'Illumination Rank 2' WHERE `spell_id` = 20212;
UPDATE `spell_proc` SET `target_self` = 1, `description` = 'Illumination Rank 3' WHERE `spell_id` = 20213;
UPDATE `spell_proc` SET `target_self` = 1, `description` = 'Illumination Rank 4' WHERE `spell_id` = 20214;
UPDATE `spell_proc` SET `target_self` = 1, `description` = 'Illumination Rank 5' WHERE `spell_id` = 20215;

REPLACE INTO `spell_proc` VALUES
   (20272, 0, 0, 1, -1, -1, 0, -1, -1, -1, 'Illumination');
   
UPDATE `spell_proc` SET `proc_chance` = 30, `description` = 'Decapitate' WHERE `spell_id` = 25669;
REPLACE INTO `spell_proc` VALUES
   (24241, 0, 0, -1, 30, -1, 0, -1, -1, -1, 'Decapitate'),
   (54670, 0, 0, -1, 30, -1, 0, -1, -1, -1, 'Decapitate');
   
UPDATE `spell_proc` SET `proc_interval` = 10000, `description` = 'Aviana\'s Purpose' WHERE `spell_id` = 41260;

UPDATE `spell_proc` SET `proc_interval` = 10000, `description` = 'Infused Mushroom' WHERE `spell_id` IN(33743, 33746, 33758, 33759);
REPLACE INTO `spell_proc` VALUES
   (57686, 0, 0, 1, -1, -1, 10000, -1, -1, -1, 'Infused Mushroom Meatloaf');
   
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Infused Mushroom' WHERE `spell_id` IN(34355, 39027);

UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Earth Shield' WHERE `spell_id` = 379;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Earth Shield Rank 1' WHERE `spell_id` = 974;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Earth Shield Rank 2' WHERE `spell_id` = 32593;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Earth Shield Rank 3' WHERE `spell_id` = 32594;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Earth Shield Rank 4' WHERE `spell_id` = 49283;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Earth Shield Rank 5' WHERE `spell_id` = 49284;

UPDATE `spell_proc` SET `proc_interval` = 3000, `target_self` = 1, `description` = 'Water Shield Rank 1' WHERE `spell_id` = 52127;
UPDATE `spell_proc` SET `proc_interval` = 3000, `target_self` = 1, `description` = 'Water Shield Rank 2' WHERE `spell_id` = 52129;
UPDATE `spell_proc` SET `proc_interval` = 3000, `target_self` = 1, `description` = 'Water Shield Rank 3' WHERE `spell_id` = 52131;
UPDATE `spell_proc` SET `proc_interval` = 3000, `target_self` = 1, `description` = 'Water Shield Rank 4' WHERE `spell_id` = 52134;
UPDATE `spell_proc` SET `proc_interval` = 3000, `target_self` = 1, `description` = 'Water Shield Rank 5' WHERE `spell_id` = 52136;
UPDATE `spell_proc` SET `proc_interval` = 3000, `target_self` = 1, `description` = 'Water Shield Rank 6' WHERE `spell_id` = 52138;
UPDATE `spell_proc` SET `proc_interval` = 3000, `target_self` = 1, `description` = 'Water Shield Rank 7' WHERE `spell_id` = 24398;
UPDATE `spell_proc` SET `proc_interval` = 3000, `target_self` = 1, `description` = 'Water Shield Rank 8' WHERE `spell_id` = 33736;
UPDATE `spell_proc` SET `proc_interval` = 3000, `target_self` = 1, `description` = 'Water Shield Rank 9' WHERE `spell_id` = 57960;

--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-02-18_03_spell_custom_assign' WHERE `LastUpdate` = '2016-02-18_02_spellsystem_update';
