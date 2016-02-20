--
-- Modify column proc_flags in spell_proc table to unsigned
--
ALTER TABLE `spell_proc` MODIFY `proc_flags` BIGINT(30) signed NOT NULL DEFAULT -1;

--
-- Update spell_proc table
--
UPDATE `spell_proc` SET `proc_flags` = 1073741824, `effect_trigger_spell_0` = 12654, `description` = 'Ignite Rank 1' WHERE `spell_id` = 11119;
UPDATE `spell_proc` SET `proc_flags` = 1073741824, `effect_trigger_spell_0` = 12654, `description` = 'Ignite Rank 2' WHERE `spell_id` = 11120;
UPDATE `spell_proc` SET `proc_flags` = 1073741824, `effect_trigger_spell_0` = 12654, `description` = 'Ignite Rank 3' WHERE `spell_id` = 12846;
UPDATE `spell_proc` SET `proc_flags` = 1073741824, `effect_trigger_spell_0` = 12654, `description` = 'Ignite Rank 4' WHERE `spell_id` = 12847;
UPDATE `spell_proc` SET `proc_flags` = 1073741824, `effect_trigger_spell_0` = 12654, `description` = 'Ignite Rank 5' WHERE `spell_id` = 12848;

UPDATE `spell_proc` SET `target_self` = 1, `proc_interval` = 3000, `description` = 'Improved Water Shield Rank 1' WHERE `spell_id` = 16180;
UPDATE `spell_proc` SET `target_self` = 1, `proc_interval` = 3000, `description` = 'Improved Water Shield Rank 2' WHERE `spell_id` = 16196;
UPDATE `spell_proc` SET `target_self` = 1, `proc_interval` = 3000, `description` = 'Improved Water Shield Rank 3' WHERE `spell_id` = 16198;

UPDATE `spell_proc` SET `target_self` = 1, `proc_interval` = 3000, `description` = 'Water Shield' WHERE `spell_id` = 34827;

UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Glyph of Earth Shield' WHERE `spell_id` = 63279;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Earth Shield' WHERE `spell_id` = 66063;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Earth Shield' WHERE `spell_id` = 67530;

UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Borrowed Time (generated proc_flags 81920)' WHERE `spell_id` = 59887;
UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Borrowed Time (generated proc_flags 81920)' WHERE `spell_id` = 59888;
UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Borrowed Time (generated proc_flags 81920)' WHERE `spell_id` = 59889;
UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Borrowed Time (generated proc_flags 81920)' WHERE `spell_id` = 59890;
UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Borrowed Time (generated proc_flags 81920)' WHERE `spell_id` = 59891;

UPDATE `spell_proc` SET `proc_flags` = 1073745920, `effect_trigger_spell_0` = 12721, `description` = 'Deep Wounds Rank 1' WHERE `spell_id` = 12834;
UPDATE `spell_proc` SET `proc_flags` = 1073745920, `effect_trigger_spell_0` = 12721, `description` = 'Deep Wounds Rank 2' WHERE `spell_id` = 12849;
UPDATE `spell_proc` SET `proc_flags` = 1073745920, `effect_trigger_spell_0` = 12721, `description` = 'Deep Wounds Rank 3' WHERE `spell_id` = 12867;

UPDATE `spell_proc` SET `effect_trigger_spell_1` = 23694, `description` = 'Improved Hamstring Rank 1' WHERE `spell_id` = 12289;
UPDATE `spell_proc` SET `effect_trigger_spell_1` = 23694, `description` = 'Improved Hamstring Rank 2' WHERE `spell_id` = 12668;
UPDATE `spell_proc` SET `effect_trigger_spell_1` = 23694, `description` = 'Improved Hamstring Rank 3' WHERE `spell_id` = 23695;

UPDATE `spell_proc` SET `effect_trigger_spell_0` = 22858, `proc_flags` = 512, `description` = 'Retaliation' WHERE `spell_id` = 20230;

UPDATE `spell_proc` SET `proc_interval` = 7000, `description` = 'Furious Attacks Rank 1' WHERE `spell_id` = 46910;
UPDATE `spell_proc` SET `proc_interval` = 5000, `description` = 'Furious Attacks Rank 2' WHERE `spell_id` = 46911;

UPDATE `spell_proc` SET `proc_flags` = 264200, `description` = 'Blood Craze Rank 1' WHERE `spell_id` = 16487;
UPDATE `spell_proc` SET `proc_flags` = 264200, `description` = 'Blood Craze Rank 2' WHERE `spell_id` = 16489;
UPDATE `spell_proc` SET `proc_flags` = 264200, `description` = 'Blood Craze Rank 3' WHERE `spell_id` = 16492;

UPDATE `spell_proc` SET `proc_flags` = 4, `target_self` = 1, `description` = 'Unbridled Wrath Rank 1' WHERE `spell_id` = 12322;
UPDATE `spell_proc` SET `proc_flags` = 4, `target_self` = 1, `description` = 'Unbridled Wrath Rank 2' WHERE `spell_id` = 12999;
UPDATE `spell_proc` SET `proc_flags` = 4, `target_self` = 1, `description` = 'Unbridled Wrath Rank 3' WHERE `spell_id` = 13000;
UPDATE `spell_proc` SET `proc_flags` = 4, `target_self` = 1, `description` = 'Unbridled Wrath Rank 4' WHERE `spell_id` = 13001;
UPDATE `spell_proc` SET `proc_flags` = 4, `target_self` = 1, `description` = 'Unbridled Wrath Rank 5' WHERE `spell_id` = 13002;

UPDATE `spell_proc` SET `proc_flags` = 65536, `effect_trigger_spell_1` = 12798, `description` = 'Improved Revenge Rank 1' WHERE `spell_id` = 12797;
UPDATE `spell_proc` SET `proc_flags` = 65536, `effect_trigger_spell_1` = 12798, `description` = 'Improved Revenge Rank 2' WHERE `spell_id` = 12799;

UPDATE `spell_proc` SET `proc_flags` = 16, `effect_trigger_spell_1` = 18498, `description` = 'Gag Order Rank 1' WHERE `spell_id` = 12311;
UPDATE `spell_proc` SET `proc_flags` = 16, `effect_trigger_spell_1` = 18498, `description` = 'Gag Order Rank 2' WHERE `spell_id` = 12958;

UPDATE `spell_proc` SET `proc_chance` = 50, `description` = 'Judgement of Light Rank 1' WHERE `spell_id` = 20185;
UPDATE `spell_proc` SET `proc_chance` = 50, `description` = 'Judgement of Wisdom Rank 1' WHERE `spell_id` = 20186;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Seal of Corruption' WHERE `spell_id` = 53736;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Seal of Light' WHERE `spell_id` = 20165;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Seal of Justice' WHERE `spell_id` = 20164;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Seal of Righteousness' WHERE `spell_id` = 21084;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Seal of Vengeance' WHERE `spell_id` = 31801;
UPDATE `spell_proc` SET `proc_interval` = 3000, `description` = 'Seal of Wisdom' WHERE `spell_id` = 20166;

UPDATE `spell_proc` SET `description` = 'Vengeance Rank 2' WHERE `spell_id` = 20056;
UPDATE `spell_proc` SET `description` = 'Vengeance Rank 3' WHERE `spell_id` = 20057;

--
-- Insert new data into spell_proc
--
REPLACE INTO `spell_proc` (`spell_id`, `proc_on_name_hash`, `proc_flags`, `target_self`, `proc_chance`, `proc_charges`, `proc_interval`, `effect_trigger_spell_0`, `effect_trigger_spell_1`, `effect_trigger_spell_2`, `description`) VALUES
	(20375, 0, -1, 0, 25, -1, 3000, -1, -1, -1, 'Seal of Command'),
	(20424, 0, -1, 0, 25, -1, 0, -1, -1, -1, 'Seal of Command'),
	(29385, 0, -1, 0, 25, -1, 0, -1, -1, -1, 'Seal of Command'),
	(33127, 0, -1, 0, 25, -1, 0, -1, -1, -1, 'Seal of Command'),
	(41469, 0, -1, 0, 25, -1, 0, -1, -1, -1, 'Seal of Command'),
	(42058, 0, -1, 0, 25, -1, 0, -1, -1, -1, 'Seal of Command'),
	(57769, 0, -1, 0, 25, -1, 0, -1, -1, -1, 'Seal of Command'),
	(57770, 0, -1, 0, 25, -1, 0, -1, -1, -1, 'Seal of Command'),
	(66004, 0, -1, 0, 25, -1, 0, -1, -1, -1, 'Seal of Command'),
	(68020, 0, -1, 0, 25, -1, 0, -1, -1, -1, 'Seal of Command'),
	(68021, 0, -1, 0, 25, -1, 0, -1, -1, -1, 'Seal of Command'),
	(68022, 0, -1, 0, 25, -1, 0, -1, -1, -1, 'Seal of Command'),
	(69403, 0, -1, 0, 25, -1, 0, -1, -1, -1, 'Seal of Command'),
	(12292, 0, -1, 0, 100, -1, 0, -1, -1, -1, 'Death Wish'),
	(56636, 0, 17, 0, -1, -1, 6000, -1, -1, -1, 'Taste for Blood Rank 1'),
	(56637, 0, 17, 0, -1, -1, 6000, -1, -1, -1, 'Taste for Blood Rank 2'),
	(56638, 0, 17, 0, -1, -1, 6000, -1, -1, -1, 'Taste for Blood Rank 3'),
	(46867, 0, 4096, 0, -1, -1, 0, -1, -1, -1, 'Wrecking Crew Rank 1'),
	(56611, 0, 4096, 0, -1, -1, 0, -1, -1, -1, 'Wrecking Crew Rank 2'),
	(56612, 0, 4096, 0, -1, -1, 0, -1, -1, -1, 'Wrecking Crew Rank 3'),
	(56613, 0, 4096, 0, -1, -1, 0, -1, -1, -1, 'Wrecking Crew Rank 4'),
	(56614, 0, 4096, 0, -1, -1, 0, -1, -1, -1, 'Wrecking Crew Rank 5'),
	(23575, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield'),
	(33737, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield'),
	(34318, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Improved Water Shield'),
	(34828, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield'),
	(36816, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield'),
	(36817, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Channel Water Shield'),
	(37209, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Improved Water Shield'),
	(37432, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield'),
	(38105, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Channel Water Shield'),
	(38106, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Channel Water Shield'),
	(52128, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield'),
	(52130, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield'),
	(52132, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield'),
	(52133, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield'),
	(52135, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield'),
	(52137, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield'),
	(55535, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Glyph of Water Shield'),
	(57961, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield'),
	(58063, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Glyph of Water Shield'),
	(58266, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Glyph of Water Shield'),
	(58332, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Glyph of Water Shield'),
	(60166, 0, -1, 1, -1, -1, 3000, -1, -1, -1, 'Water Shield Boost'),
	(32734, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(37204, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Channel Earth Shield'),
	(38101, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Channel Earth Shield'),
	(38102, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Channel Earth Shield'),
	(38590, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(51560, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Improved Earth Shield Rank 1'),
	(51561, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Improved Earth Shield Rank 2'),
	(54479, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(54480, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(55599, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(55600, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(56451, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(57802, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield (with EffectTriggerSpell_0 and procFlags in dbc)'),
	(57803, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(58981, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield (with EffectTriggerSpell_0 and procFlags in dbc)'),
	(58982, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(59471, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield (with EffectTriggerSpell_0 and procFlags in dbc)'),
	(59472, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(60013, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield (with EffectTriggerSpell_0 and procFlags in dbc)'),
	(60014, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(63925, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Glyph of Earth Shield'),
	(64261, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Glyph of Earth Shield'),
	(66064, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(67537, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(68320, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(68592, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(68593, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(68594, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(69568, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(69569, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield (with EffectTriggerSpell_0 and procFlags in dbc)'),
	(69925, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield'),
	(69926, 0, -1, 0, -1, -1, 3000, -1, -1, -1, 'Earth Shield (with EffectTriggerSpell_0 and procFlags in dbc)'),
	(52795, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Borrowed Time Rank 1'),
	(52797, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Borrowed Time Rank 2'),
	(52798, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Borrowed Time Rank 3'),
	(52799, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Borrowed Time Rank 4'),
	(52800, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Borrowed Time Rank 5'),
	(47516, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Grace Rank 1'),
	(47517, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Grace Rank 2'),
	(30881, 0, 1188352, 0, -1, -1, 5000, 31616, -1, -1, 'Nature\'s Guardian Rank 1'),
	(30883, 0, 1188352, 0, -1, -1, 5000, 31616, -1, -1, 'Nature\'s Guardian Rank 2'),
	(30884, 0, 1188352, 0, -1, -1, 5000, 31616, -1, -1, 'Nature\'s Guardian Rank 3'),
	(30885, 0, 1188352, 0, -1, -1, 5000, 31616, -1, -1, 'Nature\'s Guardian Rank 4'),
	(30886, 0, 1188352, 0, -1, -1, 5000, 31616, -1, -1, 'Nature\'s Guardian Rank 5'),
	(31616, 0, 1188352, 0, -1, -1, 5000, 31616, -1, -1, 'Nature\'s Guardian'),
	(29841, 0, -1, 1, -1, -1, 0, -1, -1, -1, 'Second Wind Rank 1'),
	(29842, 0, -1, 1, -1, -1, 0, -1, -1, -1, 'Second Wind Rank 2'),
	(29723, 0, 4, 0, -1, -1, 0, -1, -1, -1, 'Sudden Death Rank 1'),
	(29725, 0, 4, 0, -1, -1, 0, -1, -1, -1, 'Sudden Death Rank 2'),
	(29724, 0, 4, 0, -1, -1, 0, -1, -1, -1, 'Sudden Death Rank 3'),
	(46913, 0, 20, 0, -1, -1, 0, -1, -1, -1, 'Bloodsurge Rank 1'),
	(46914, 0, 20, 0, -1, -1, 0, -1, -1, -1, 'Bloodsurge Rank 2'),
	(46915, 0, 20, 0, -1, -1, 0, -1, -1, -1, 'Bloodsurge Rank 3'),
	(12317, 0, 139776, 0, -1, -1, 0, -1, -1, -1, 'Enrage Rank 1'),
	(13045, 0, 139776, 0, -1, -1, 0, -1, -1, -1, 'Enrage Rank 2'),
	(13046, 0, 139776, 0, -1, -1, 0, -1, -1, -1, 'Enrage Rank 3'),
	(13047, 0, 139776, 0, -1, -1, 0, -1, -1, -1, 'Enrage Rank 4'),
	(13048, 0, 139776, 0, -1, -1, 0, -1, -1, -1, 'Enrage Rank 5'),
	(12880, 0, 4, 0, -1, -1, 0, -1, -1, -1, 'Enrage Rank 1 Remove the charges only on melee attacks'),
	(14201, 0, 4, 0, -1, -1, 0, -1, -1, -1, 'Enrage Rank 2 Remove the charges only on melee attacks'),
	(14202, 0, 4, 0, -1, -1, 0, -1, -1, -1, 'Enrage Rank 3 Remove the charges only on melee attacks'),
	(14203, 0, 4, 0, -1, -1, 0, -1, -1, -1, 'Enrage Rank 4 Remove the charges only on melee attacks'),
	(14204, 0, 4, 0, -1, -1, 0, -1, -1, -1, 'Enrage Rank 5 Remove the charges only on melee attacks'),
	(20500, 0, 16, 1, -1, -1, 0, -1, -1, -1, 'Improved Berserker Rage Rank 1'),
	(20501, 0, 16, 1, -1, -1, 0, -1, -1, -1, 'Improved Berserker Rage Rank 2'),
	(46951, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Sword and Board Rank 1'),
	(46952, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Sword and Board Rank 2'),
	(46953, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Sword and Board Rank 3'),
	(46945, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Safeguard Rank 1'),
	(46949, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Safeguard Rank 2'),
	(29593, 0, 570425344, 0, -1, -1, 0, -1, -1, -1, 'Improved Defensive Stance Rank 1'),
	(29594, 0, 570425344, 0, -1, -1, 0, -1, -1, -1, 'Improved Defensive Stance Rank 2'),
	(20467, 0, -1, 0, 50, -1, 0, -1, -1, -1, 'Judgement of Command Rank 1'),
	(53733, 0, -1, 0, 50, -1, 0, -1, -1, -1, 'Judgement of Corruption Rank 1'),
	(20184, 0, -1, 0, 50, -1, 0, -1, -1, -1, 'Judgement of Justice'),
	(20187, 0, -1, 0, 50, -1, 0, -1, -1, -1, 'Judgement of Righteousness Rank 1'),
	(31804, 0, -1, 0, 50, -1, 0, -1, -1, -1, 'Judgement of Vengeance Rank 1');
   
--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-02-19_02_spell_proc' WHERE `LastUpdate` = '2016-02-19_01_spell_custom_assign';
