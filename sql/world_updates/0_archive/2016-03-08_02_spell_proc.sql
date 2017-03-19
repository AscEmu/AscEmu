--
-- Inserts for spell_proc table
--
REPLACE INTO `spell_proc` (`spell_id`, `proc_on_name_hash`, `proc_flags`, `target_self`, `proc_chance`, `proc_charges`, `proc_interval`, `effect_trigger_spell_0`, `effect_trigger_spell_1`, `effect_trigger_spell_2`, `description`) VALUES
	(32385, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Shadow Embrace Rank 1'),
   (32387, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Shadow Embrace Rank 2'),
   (32392, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Shadow Embrace Rank 3'),
   (32393, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Shadow Embrace Rank 4'),
   (32393, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Shadow Embrace Rank 5'),
	(49005, 0, 1092, 0, -1, -1, 0, -1, -1, -1, 'Mark of Blood'),
   (49027, 0, 4, 0, -1, -1, 0, 50452, -1, -1, 'Bloodworms Rank 1'),
   (49542, 0, 4, 0, -1, -1, 0, 50452, -1, -1, 'Bloodworms Rank 2'),
   (49543, 0, 4, 0, -1, -1, 0, 50452, -1, -1, 'Bloodworms Rank 3'),
   (34139, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Gladiator\'s Libram of Justice'),
   (42368, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Merciless Gladiator\'s Libram of Justice'),
   (43726, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Vengeful Gladiator\'s Libram of Justice'),
   (46092, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Brutal Gladiator\'s Libram of Justice'),
   (34262, 0, 16, 1, -1, -1, 0, -1, -1, -1, 'Mercy Libram of Saints Departed and Libram of Zeal'),
   (34258, 0, 16, 1, -1, -1, 0, -1, -1, -1, 'Libram of Avengement'),
   (43741, 0, 16, 1, -1, -1, 0, -1, -1, -1, 'Libram of Mending'),
   (43745, 0, 16, 1, -1, -1, 0, -1, -1, -1, 'Libram of Divine Judgement'),
   (43747, 0, 0, 0, 100, -1, 0, -1, -1, -1, 'Libram of Divine Judgement'),
   (43748, 0, 16, 1, -1, -1, 0, -1, -1, -1, 'Elemental Strength'),
   (43749, 0, 0, 0, 100, -1, 0, -1, -1, -1, 'Elemental Strength'),
   (27997, 0, -1, 0, 3, -1, 30000, -1, -1, -1, 'Spellsurge Trigger');

   
--
-- Inserts for spell_custom assign table
--
REPLACE INTO `spell_custom_assign` VALUES
   (43747, 0, 0, 1, 0, 'Libram of Divine Judgement'),
   (43749, 0, 0, 1, 0, 'Elemental Strength');
--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-03-08_02_spell_proc' WHERE `LastUpdate` = '2016-03-08_01_spell_coef_flags';
