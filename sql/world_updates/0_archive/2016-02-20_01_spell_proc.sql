--
-- Update spell_proc table
--
UPDATE `spell_proc` SET `effect_trigger_spell_0` = 31893, `description` = 'Seal of Blood' WHERE `spell_id` = 38008;
UPDATE `spell_proc` SET `proc_flags` = 4, `effect_trigger_spell_0` = 53719, `description` = 'Seal of the Martyr' WHERE `spell_id` = 53720;
UPDATE `spell_proc` SET `proc_flags` = 4, `effect_trigger_spell_0` = 31893, `description` = 'Seal of Blood' WHERE `spell_id` = 31892;
UPDATE `spell_proc` SET `proc_flags` = 1048576, `target_self` = 1, `proc_interval` = 6000, `effect_trigger_spell_0` = 58597, `description` = 'Sacred Shield Rank 1' WHERE `spell_id` = 53601;

UPDATE `spell_proc` SET `proc_flags` = 1048576, `target_self` = 1, `description` = 'Reckoning Rank 1' WHERE `spell_id` = 20177;
UPDATE `spell_proc` SET `proc_flags` = 1048576, `target_self` = 1, `description` = 'Reckoning Rank 2' WHERE `spell_id` = 20179;
UPDATE `spell_proc` SET `proc_flags` = 1048576, `target_self` = 1, `description` = 'Reckoning Rank 3' WHERE `spell_id` = 20180;
UPDATE `spell_proc` SET `proc_flags` = 1048576, `target_self` = 1, `description` = 'Reckoning Rank 4' WHERE `spell_id` = 20181;

UPDATE `spell_proc` SET `proc_flags` = 1048576, `target_self` = 1, `description` = 'Reckoning Effect' WHERE `spell_id` = 20178;

UPDATE `spell_proc` SET `proc_flags` = 16, `effect_trigger_spell_0` = 54180, `description` = 'Judgements of the Wise Rank 1' WHERE `spell_id` = 31876;
UPDATE `spell_proc` SET `proc_flags` = 16, `effect_trigger_spell_0` = 54180, `description` = 'Judgements of the Wise Rank 2' WHERE `spell_id` = 31877;
UPDATE `spell_proc` SET `proc_flags` = 16, `effect_trigger_spell_0` = 54180, `description` = 'Judgements of the Wise Rank 3' WHERE `spell_id` = 31878;

UPDATE `spell_proc` SET `proc_flags` = 32768, `target_self` = 1, `effect_trigger_spell_0` = 31786, `description` = 'Spiritual Attunement Rank 1' WHERE `spell_id` = 31785;
UPDATE `spell_proc` SET `proc_flags` = 32768, `target_self` = 1, `effect_trigger_spell_0` = 31786, `description` = 'Spiritual Attunement Rank 2' WHERE `spell_id` = 33776;

UPDATE `spell_proc` SET `proc_flags` = 4096, `description` = 'The Art of War Rank 1' WHERE `spell_id` = 53486;
UPDATE `spell_proc` SET `proc_flags` = 4096, `description` = 'The Art of War Rank 2' WHERE `spell_id` = 53488;

UPDATE `spell_proc` SET `proc_flags` = 1073741824, `effect_trigger_spell_1` = 54203, `description` = 'Sheath of Light Rank 1' WHERE `spell_id` = 53501;
UPDATE `spell_proc` SET `proc_flags` = 1073741824, `effect_trigger_spell_1` = 54203, `description` = 'Sheath of Light Rank 2' WHERE `spell_id` = 53502;
UPDATE `spell_proc` SET `proc_flags` = 1073741824, `effect_trigger_spell_1` = 54203, `description` = 'Sheath of Light Rank 3' WHERE `spell_id` = 53503;

UPDATE `spell_proc` SET `proc_flags` = 128, `description` = 'Expose Weakness Rank 1' WHERE `spell_id` = 34500;
UPDATE `spell_proc` SET `proc_flags` = 128, `description` = 'Expose Weakness Rank 2' WHERE `spell_id` = 34502;
UPDATE `spell_proc` SET `proc_flags` = 128, `description` = 'Expose Weakness Rank 2' WHERE `spell_id` = 34503;

UPDATE `spell_proc` SET `proc_flags` = 1048576, `description` = 'Aspect of the Pack' WHERE `spell_id` = 13159;

UPDATE `spell_proc` SET `description` = 'Blade Twisting Rank 1' WHERE `spell_id` = 31124;
UPDATE `spell_proc` SET `description` = 'Blade Twisting Rank 2' WHERE `spell_id` = 31126;

UPDATE `spell_proc` SET `description` = 'Seal Fate Rank 1' WHERE `spell_id` = 14186;
UPDATE `spell_proc` SET `description` = 'Seal Fate Rank 2' WHERE `spell_id` = 14190;
UPDATE `spell_proc` SET `description` = 'Seal Fate Rank 3' WHERE `spell_id` = 14193;
UPDATE `spell_proc` SET `description` = 'Seal Fate Rank 4' WHERE `spell_id` = 14194;
UPDATE `spell_proc` SET `description` = 'Seal Fate Rank 5' WHERE `spell_id` = 14195;

UPDATE `spell_proc` SET `proc_flags` = 1048576, `description` = 'Prayer of Mending Rank 1' WHERE `spell_id` = 41635;
UPDATE `spell_proc` SET `proc_flags` = 1048576, `description` = 'Prayer of Mending Rank 2' WHERE `spell_id` = 48110;
UPDATE `spell_proc` SET `proc_flags` = 1048576, `description` = 'Prayer of Mending Rank 3' WHERE `spell_id` = 48111;

UPDATE `spell_proc` SET `proc_flags` = 2056, `effect_trigger_spell_0` = 27813, `description` = 'Blessed Recovery Rank 1' WHERE `spell_id` = 27811;
UPDATE `spell_proc` SET `proc_flags` = 2056, `effect_trigger_spell_0` = 27817, `description` = 'Blessed Recovery Rank 2' WHERE `spell_id` = 27815;
UPDATE `spell_proc` SET `proc_flags` = 2056, `effect_trigger_spell_0` = 27818, `description` = 'Blessed Recovery Rank 3' WHERE `spell_id` = 27816;

UPDATE `spell_proc` SET `proc_flags` = 1073741824, `target_self` = 1, `description` = 'Surge of Light Rank 1' WHERE `spell_id` = 33150;
UPDATE `spell_proc` SET `proc_flags` = 1073741824, `target_self` = 1, `description` = 'Surge of Light Rank 2' WHERE `spell_id` = 33154;

UPDATE `spell_proc` SET `proc_interval` = 100 WHERE `spell_id` = 47516;
UPDATE `spell_proc` SET `proc_interval` = 100 WHERE `spell_id` = 47517;

UPDATE `spell_proc` SET `proc_flags` = 131072, `description` = 'Stormstrike' WHERE `spell_id` = 17364;

UPDATE `spell_proc` SET `proc_flags` = 1024, `effect_trigger_spell_0` = 39805, `description` = 'Lightning Overload Rank 1' WHERE `spell_id` = 30675;
UPDATE `spell_proc` SET `proc_flags` = 1024, `effect_trigger_spell_0` = 39805, `description` = 'Lightning Overload Rank 2' WHERE `spell_id` = 30678;
UPDATE `spell_proc` SET `proc_flags` = 1024, `effect_trigger_spell_0` = 39805, `description` = 'Lightning Overload Rank 3' WHERE `spell_id` = 30679;

UPDATE `spell_proc` SET `proc_charges` = 0, `description` = 'Maelstrom Weapon' WHERE `spell_id` = 53817;

UPDATE `spell_proc` SET `proc_flags` = 1073741824, `description` = 'Ancestral Healing Rank 1' WHERE `spell_id` = 16176;
UPDATE `spell_proc` SET `proc_flags` = 1073741824, `description` = 'Ancestral Healing Rank 2' WHERE `spell_id` = 16235;
UPDATE `spell_proc` SET `proc_flags` = 1073741824, `description` = 'Ancestral Healing Rank 3' WHERE `spell_id` = 16240;

UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Missile Barrage Rank 1' WHERE `spell_id` = 44404;
UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Missile Barrage Rank 2' WHERE `spell_id` = 54486;
UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Missile Barrage Rank 3' WHERE `spell_id` = 54488;
UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Missile Barrage Rank 4' WHERE `spell_id` = 54489;
UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Missile Barrage Rank 5' WHERE `spell_id` = 54490;

UPDATE `spell_proc` SET `proc_flags` = 1024, `target_self` = 1, `description` = 'Arcane Concentration Rank 1' WHERE `spell_id` = 11213;
UPDATE `spell_proc` SET `proc_flags` = 1024, `target_self` = 1, `description` = 'Arcane Concentration Rank 2' WHERE `spell_id` = 12574;
UPDATE `spell_proc` SET `proc_flags` = 1024, `target_self` = 1, `description` = 'Arcane Concentration Rank 3' WHERE `spell_id` = 12575;
UPDATE `spell_proc` SET `proc_flags` = 1024, `target_self` = 1, `description` = 'Arcane Concentration Rank 4' WHERE `spell_id` = 12576;
UPDATE `spell_proc` SET `proc_flags` = 1024, `target_self` = 1, `description` = 'Arcane Concentration Rank 5' WHERE `spell_id` = 12577;
UPDATE `spell_proc` SET `proc_flags` = 16, `proc_charges` = 2, `description` = 'Clearcasting (Arcane Concentration Effect)' WHERE `spell_id` = 12536;


UPDATE `spell_proc` SET `proc_flags` = 1073741824, `target_self` = 1, `effect_trigger_spell_0` = 29077, `description` = 'Master of Elements Rank 1' WHERE `spell_id` = 29074;
UPDATE `spell_proc` SET `proc_flags` = 1073741824, `target_self` = 1, `effect_trigger_spell_0` = 29077, `description` = 'Master of Elements Rank 2' WHERE `spell_id` = 29075;
UPDATE `spell_proc` SET `proc_flags` = 1073741824, `target_self` = 1, `effect_trigger_spell_0` = 29077, `description` = 'Master of Elements Rank 3' WHERE `spell_id` = 29076;

UPDATE `spell_proc` SET `effect_trigger_spell_0` = 31643, `description` = 'Blazing Speed Rank 1' WHERE `spell_id` = 31641;
UPDATE `spell_proc` SET `effect_trigger_spell_0` = 31643, `description` = 'Blazing Speed Rank 2' WHERE `spell_id` = 31641;

UPDATE `spell_proc` SET `proc_flags` = 67239936, `effect_trigger_spell_1` = 27285, `description` = 'Seed of Corruption Rank 1' WHERE `spell_id` = 27243;

UPDATE `spell_proc` SET `proc_flags` = 1, `target_self` = 1, `effect_trigger_spell_0` = 17941, `description` = 'Nightfall Rank 1' WHERE `spell_id` = 18094;
UPDATE `spell_proc` SET `proc_flags` = 1, `target_self` = 1, `effect_trigger_spell_0` = 17941, `description` = 'Nightfall Rank 2' WHERE `spell_id` = 18095;


UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Light\'s Grace Rank 1' WHERE `spell_id` = 31878;
UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Light\'s Grace Rank 2' WHERE `spell_id` = 31835;
UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Light\'s Grace Rank 3' WHERE `spell_id` = 31836;

--
-- Update for spell_custom_assign (delete Clearcasting)
--
DELETE FROM `spell_custom_assign` WHERE `spell_id` = 12536;

--
-- Inserts for spell_proc table
--
REPLACE INTO `spell_proc` (`spell_id`, `proc_on_name_hash`, `proc_flags`, `target_self`, `proc_chance`, `proc_charges`, `proc_interval`, `effect_trigger_spell_0`, `effect_trigger_spell_1`, `effect_trigger_spell_2`, `description`) VALUES
	(26016, 0, 4, 0, 30, -1, 0, -1, -1, -1, 'Vindication Rank 2'),
	(9452, 0, 4, 0, 30, -1, 0, -1, -1, -1, 'Vindication Rank 1'),
	(20182, 0, 1048576, 1, -1, -1, 0, -1, -1, -1, 'Reckoning Rank 5'),
	(54180, 0, -1, 0, -1, -1, 4000, -1, -1, -1, 'Judgements of the Wise'),
	(31828, 0, 1048576, 0, -1, -1, 0, -1, -1, -1, 'Blessed Life Rank 1'),
	(31829, 0, 1048576, 0, -1, -1, 0, -1, -1, -1, 'Blessed Life Rank 2'),
	(31830, 0, 1048576, 0, -1, -1, 0, -1, -1, -1, 'Blessed Life Rank 3'),
	(20234, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Improved Lay on Hands Rank 1'),
	(20235, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Improved Lay on Hands Rank 2'),
	(53569, 0, 1073741824, 0, -1, -1, 0, -1, -1, -1, 'Infusion of Light Rank 1'),
	(53576, 0, 1073741824, 0, -1, -1, 0, -1, -1, -1, 'Infusion of Light Rank 2'),
	(53551, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Sacred Cleansing Rank 1'),
	(53552, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Sacred Cleansing Rank 2'),
	(53553, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Sacred Cleansing Rank 3'),
	(53671, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Judgements of the Pure Rank 1'),
	(53673, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Judgements of the Pure Rank 2'),
	(54151, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Judgements of the Pure Rank 3'),
	(54154, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Judgements of the Pure Rank 4'),
	(54155, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Judgements of the Pure Rank 5'),
	(20335, 0, 16, 0, 100, -1, 0, 21183, -1, -1, 'Heart of the Crusader Rank 1'),
	(20336, 0, 16, 0, 100, -1, 0, 54498, -1, -1, 'Heart of the Crusader Rank 2'),
	(20337, 0, 16, 0, 100, -1, 0, 54499, -1, -1, 'Heart of the Crusader Rank 3'),
	(53215, 0, 64, 0, -1, -1, 0, -1, -1, -1, 'Wild Quiver Rank 1'),
	(53216, 0, 64, 0, -1, -1, 0, -1, -1, -1, 'Wild Quiver Rank 2'),
	(53217, 0, 64, 0, -1, -1, 0, -1, -1, -1, 'Wild Quiver Rank 3'),
	(56342, 0, 2097152, 0, -1, -1, 30000, -1, -1, -1, 'Lock and Load Rank 1'),
	(56343, 0, 2097152, 0, -1, -1, 30000, -1, -1, -1, 'Lock and Load Rank 2'),
	(56344, 0, 2097152, 0, -1, -1, 30000, -1, -1, -1, 'Lock and Load Rank 3'),
	(34692, 0, 16, 0, 100, -1, 0, 34471, -1, -1, 'The Beast Within'),
	(34950, 0, 128, 0, -1, -1, 0, -1, -1, -1, 'Go for the Throat Rank 1'),
	(34954, 0, 128, 0, -1, -1, 0, -1, -1, -1, 'Go for the Throat Rank 2'),
	(34506, 0, 64, 1, -1, -1, 0, -1, -1, -1, 'Master Tactician Rank 1'),
	(34507, 0, 64, 1, -1, -1, 0, -1, -1, -1, 'Master Tactician Rank 2'),
	(34508, 0, 64, 1, -1, -1, 0, -1, -1, -1, 'Master Tactician Rank 3'),
	(34838, 0, 64, 1, -1, -1, 0, -1, -1, -1, 'Master Tactician Rank 4'),
	(34839, 0, 64, 1, -1, -1, 0, -1, -1, -1, 'Master Tactician Rank 5'),
	(5118, 0, 1048576, 0, -1, -1, 0, -1, -1, -1, 'Aspect of the Cheetah'),
	(15273, 0, -1, 0, 20, -1, 0, -1, -1, -1, 'Improved Mind Blast Rank 1'),
	(15312, 0, -1, 0, 40, -1, 0, -1, -1, -1, 'Improved Mind Blast Rank 2'),
	(15313, 0, -1, 0, 60, -1, 0, -1, -1, -1, 'Improved Mind Blast Rank 3'),
	(15314, 0, -1, 0, 80, -1, 0, -1, -1, -1, 'Improved Mind Blast Rank 4'),
	(15316, 0, -1, 0, 100, -1, 0, -1, -1, -1, 'Improved Mind Blast Rank 5'),
	(33142, 0, 264200, 0, -1, -1, 0, -1, -1, -1, 'Blessed Resilience Rank 1'),
	(33145, 0, 264200, 0, -1, -1, 0, -1, -1, -1, 'Blessed Resilience Rank 2'),
	(33146, 0, 264200, 0, -1, -1, 0, -1, -1, -1, 'Blessed Resilience Rank 3'),
	(45234, 0, 264200, 0, -1, -1, 0, -1, -1, -1, 'Focused Will Rank 1'),
	(45243, 0, 264200, 0, -1, -1, 0, -1, -1, -1, 'Focused Will Rank 2'),
	(45244, 0, 264200, 0, -1, -1, 0, -1, -1, -1, 'Focused Will Rank 3'),
	(16164, 0, 1073741824, 0, -1, -1, 0, -1, -1, -1, 'Elemental Focus'),
	(29062, 0, 8, 0, -1, -1, 0, -1, -1, -1, 'Eye of the Storm Rank 1'),
	(29064, 0, 8, 0, -1, -1, 0, -1, -1, -1, 'Eye of the Storm Rank 2'),
	(29065, 0, 8, 0, -1, -1, 0, -1, -1, -1, 'Eye of the Storm Rank 3'),
	(46098, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Brutal Totem of Third WInd'),
	(34138, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Totem of the Third Wind'),
	(42370, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Merciless Totem of the Third WInd'),
	(43728, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Vengeful Totem of Third WInd'),
	(51562, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Tidal Waves Rank 1'),
	(51563, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Tidal Waves Rank 2'),
	(51564, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Tidal Waves Rank 3'),
	(51565, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Tidal Waves Rank 4'),
	(51566, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Tidal Waves Rank 5'),
	(51940, 0, 16, 0, 20, -1, 0, -1, -1, -1, 'Earthliving Weapon (Passive) Rank 1'),
	(51989, 0, 16, 0, 20, -1, 0, -1, -1, -1, 'Earthliving Weapon (Passive) Rank 2'),
	(52004, 0, 16, 0, 20, -1, 0, -1, -1, -1, 'Earthliving Weapon (Passive) Rank 3'),
	(52005, 0, 16, 0, 20, -1, 0, -1, -1, -1, 'Earthliving Weapon (Passive) Rank 4'),
	(52007, 0, 16, 0, 20, -1, 0, -1, -1, -1, 'Earthliving Weapon (Passive) Rank 5'),
	(52008, 0, 16, 0, 20, -1, 0, -1, -1, -1, 'Earthliving Weapon (Passive) Rank 6'),
	(51528, 0, 16, 0, -1, -1, 24000, -1, -1, -1, 'Maelstrom Weapon Rank 1'),
	(51529, 0, 16, 0, -1, -1, 12000, -1, -1, -1, 'Maelstrom Weapon Rank 2'),
	(51530, 0, 16, 0, -1, -1, 8000, -1, -1, -1, 'Maelstrom Weapon Rank 3'),
	(51531, 0, 16, 0, -1, -1, 6000, -1, -1, -1, 'Maelstrom Weapon Rank 4'),
	(51532, 0, 16, 0, -1, -1, 5000, -1, -1, -1, 'Maelstrom Weapon Rank 5'),
	(29179, 0, 1073741824, 0, -1, -1, 0, -1, -1, -1, 'Elemental Devastation Rank 2'),
	(29180, 0, 1073741824, 0, -1, -1, 0, -1, -1, -1, 'Elemental Devastation Rank 3'),
	(30160, 0, 1073741824, 0, -1, -1, 0, -1, -1, -1, 'Elemental Devastation Rank 1'),
	(31569, 0, 65536, 0, -1, -1, 0, -1, -1, -1, 'Improved Blink Rank 1'),
	(31570, 0, 65536, 0, -1, -1, 0, -1, -1, -1, 'Improved Blink Rank 2'),
	(11185, 0, 16, 0, -1, -1, 0, 12484, -1, -1, 'Improved Blizzard Rank 1'),
	(12487, 0, 16, 0, -1, -1, 0, 12485, -1, -1, 'Improved Blizzard Rank 2'),
	(12488, 0, 16, 0, -1, -1, 0, 12486, -1, -1, 'Improved Blizzard Rank 3'),
	(44544, 0, 1024, 0, -1, 2, 0, -1, -1, -1, 'Fingers of Frost'),
	(57761, 0, 16, 0, -1, 1, 0, -1, -1, -1, 'Fireball!'),
	(11095, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Improved Scorch Rank 1'),
	(12872, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Improved Scorch Rank 2'),
	(12873, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Improved Scorch Rank 3'),
	(11189, 0, -1, 0, 10, -1, 0, -1, -1, -1, 'Frost Warding Rank 1'),
	(28332, 0, -1, 0, 20, -1, 0, -1, -1, -1, 'Frost Warding Rank 2'),
	(47195, 0, 1, 0, -1, -1, 0, -1, -1, -1, 'Eradication Rank 1'),
	(47196, 0, 1, 0, -1, -1, 0, -1, -1, -1, 'Eradication Rank 2'),
	(47197, 0, 1, 0, -1, -1, 0, -1, -1, -1, 'Eradication Rank 3'),
	(47245, 0, 1, 0, -1, -1, 0, -1, -1, -1, 'Molten Core Rank 1'),
	(47246, 0, 1, 0, -1, -1, 0, -1, -1, -1, 'Molten Core Rank 2'),
	(47247, 0, 1, 0, -1, -1, 0, -1, -1, -1, 'Molten Core Rank 3'),
	(30299, 0, -1, 0, -1, -1, 13000, -1, -1, -1, 'Nether Protection Rank 1'),
	(30301, 0, -1, 0, -1, -1, 13000, -1, -1, -1, 'Nether Protection Rank 2'),
	(30302, 0, -1, 0, -1, -1, 13000, -1, -1, -1, 'Nether Protection Rank 3'),
	(34935, 0, 512, 1, -1, -1, 8000, -1, -1, -1, 'Backlash Rank 1'),
	(34938, 0, 512, 1, -1, -1, 8000, -1, -1, -1, 'Backlash Rank 2'),
	(34939, 0, 512, 1, -1, -1, 8000, -1, -1, -1, 'Backlash Rank 3');

--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-02-20_01_spell_proc' WHERE `LastUpdate` = '2016-02-19_02_spell_proc';
