--
-- Update spell_proc table
--
UPDATE `spell_proc` SET `proc_flags` = 16, `target_self` = 1, `effect_trigger_spell_0` = 30294, `description` = 'Soul Leech Rank 1' WHERE `spell_id` = 30293;
UPDATE `spell_proc` SET `proc_flags` = 16, `target_self` = 1, `effect_trigger_spell_0` = 30294, `description` = 'Soul Leech Rank 2' WHERE `spell_id` = 30295;
UPDATE `spell_proc` SET `proc_flags` = 16, `target_self` = 1, `effect_trigger_spell_0` = 30294, `description` = 'Soul Leech Rank 3' WHERE `spell_id` = 30296;

UPDATE `spell_proc` SET `description` = 'Nature\'s Grasp Rank 1' WHERE `spell_id` = 16689;
UPDATE `spell_proc` SET `description` = 'Nature\'s Grasp Rank 2' WHERE `spell_id` = 16810;
UPDATE `spell_proc` SET `description` = 'Nature\'s Grasp Rank 3' WHERE `spell_id` = 16811;
UPDATE `spell_proc` SET `description` = 'Nature\'s Grasp Rank 4' WHERE `spell_id` = 16812;
UPDATE `spell_proc` SET `description` = 'Nature\'s Grasp Rank 5' WHERE `spell_id` = 16813;
UPDATE `spell_proc` SET `description` = 'Nature\'s Grasp Rank 6' WHERE `spell_id` = 17329;
UPDATE `spell_proc` SET `description` = 'Nature\'s Grasp Rank 7' WHERE `spell_id` = 27009;
UPDATE `spell_proc` SET `description` = 'Nature\'s Grasp Rank 8' WHERE `spell_id` = 53312;

UPDATE `spell_proc` SET `description` = 'Blood Frenzy Rank 2 (Effect)' WHERE `spell_id` = 16954;
UPDATE `spell_proc` SET `description` = 'Blood Frenzy Rank 1 (Effect)' WHERE `spell_id` = 16952;

UPDATE `spell_proc` SET `proc_interval` = 45000, `description` = 'Vestige of Haldor' WHERE `spell_id` = 60306;

UPDATE `spell_proc` SET `proc_flags` = 1073741824, `proc_interval` = 45000, `description` = 'Soul of the Dead' WHERE `spell_id` = 60537;

UPDATE `spell_proc` SET `proc_flags` = 68, `proc_interval` = 45000, `description` = 'Grim Toll' WHERE `spell_id` = 60436;

UPDATE `spell_proc` SET `proc_interval` = 45000, `description` = 'Bandit\'s Insignia' WHERE `spell_id` = 60442;

UPDATE `spell_proc` SET `proc_interval` = 45000, `description` = 'Meteorite Whetstone' WHERE `spell_id` = 60301;

UPDATE `spell_proc` SET `proc_flags` = 16, `description` = 'Band of the Eternal Sage' WHERE `spell_id` = 35083;

UPDATE `spell_proc` SET `description` = 'Mage Tier 6 Trinket' WHERE `spell_id` = 40482;

UPDATE `spell_proc` SET `proc_chance` = 6, `proc_interval` = 30000, `description` = 'Magtheridon Melee Trinket' WHERE `spell_id` = 34774;

UPDATE `spell_proc` SET `proc_flags` = 68, `proc_interval` = 60000, `description` = 'Band of the Eternal Champion' WHERE `spell_id` = 35080;

UPDATE `spell_proc` SET `proc_flags` = 139776, `proc_interval` = 60000, `description` = 'Band of the Eternal Sage' WHERE `spell_id` = 35077;

UPDATE `spell_proc` SET `description` = 'Bear Rage' WHERE `spell_id` = 37306;
UPDATE `spell_proc` SET `description` = 'Cat Energy' WHERE `spell_id` = 37311;
UPDATE `spell_proc` SET `description` = 'Crit Bonus Damage' WHERE `spell_id` = 37443;

UPDATE `spell_proc` SET `proc_flags` = 16, `proc_chance` = 20, `effect_trigger_spell_0` = 37379, `description` = 'Shadowflame' WHERE `spell_id` = 37377;
UPDATE `spell_proc` SET `proc_flags` = 16, `proc_chance` = 20, `effect_trigger_spell_0` = 37378, `description` = 'Shadowflame Hellfire and RoF' WHERE `spell_id` = 39437;

UPDATE `spell_proc` SET `description` = 'Fast Lesser Healing Wave' WHERE `spell_id` = 37239;
UPDATE `spell_proc` SET `description` = 'Mana Cost Reduction' WHERE `spell_id` = 37213;
UPDATE `spell_proc` SET `description` = 'Mana Regen Proc' WHERE `spell_id` = 38427;

UPDATE `spell_proc` SET `proc_chance` = 5, `description` = 'Black Temple Melee Trinket' WHERE `spell_id` = 40475;

--
-- Inserts for spell_proc table
--
INSERT INTO `spell_proc` (`spell_id`, `proc_on_name_hash`, `proc_flags`, `target_self`, `proc_chance`, `proc_charges`, `proc_interval`, `effect_trigger_spell_0`, `effect_trigger_spell_1`, `effect_trigger_spell_2`, `description`) VALUES
	(18073, 0, 1, 0, 13, -1, 0, 18093, -1, -1, 'Pyroclasm Rank 2'),
	(18096, 0, 1, 0, 26, -1, 0, 18093, -1, -1, 'Pyroclasm Rank 1'),
	(47258, 0, 1, 1, -1, -1, 0, -1, -1, -1, 'Backdraft Rank 1'),
	(47259, 0, 1, 1, -1, -1, 0, -1, -1, -1, 'Backdraft Rank 2'),
	(47260, 0, 1, 1, -1, -1, 0, -1, -1, -1, 'Backdraft Rank 3'),
	(16880, 0, 1073741824, 0, -1, -1, 0, -1, -1, -1, 'Nature\'s Grace'),
	(61345, 0, 1073741824, 0, -1, -1, 0, -1, -1, -1, 'Nature\'s Grace'),
	(61346, 0, 1073741824, 0, -1, -1, 0, -1, -1, -1, 'Nature\'s Grace'),
	(48506, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Earth and Moon Rank 1'),
	(48510, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Earth and Moon Rank 2'),
	(48511, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Earth and Moon Rank 3'),
	(57878, 0, 33554432, 0, -1, -1, 0, -1, -1, -1, 'Natural Reaction Rank 1'),
	(57880, 0, 33554432, 0, -1, -1, 0, -1, -1, -1, 'Natural Reaction Rank 2'),
	(57881, 0, 33554432, 0, -1, -1, 0, -1, -1, -1, 'Natural Reaction Rank 3'),
	(34297, 0, -1, 0, -1, -1, 6000, -1, -1, -1, 'Improved Leader of the Pack Rank 1'),
	(34300, 0, -1, 0, -1, -1, 6000, -1, -1, -1, 'Improved Leader of the Pack Rank 2'),
	(16961, 0, 4096, 0, -1, -1, 0, -1, -1, -1, 'Primal Fury Rank 2 (Effect)'),
	(16958, 0, 4096, 0, -1, -1, 0, -1, -1, -1, 'Primal Fury Rank 1 (Effect)'),
	(16864, 0, 20, 0, 6, -1, 0, -1, -1, -1, 'Omen of Clarity'),
	(33881, 0, 264200, 0, -1, -1, 0, -1, -1, -1, 'Natural Perfection Rank 1'),
	(33882, 0, 264200, 0, -1, -1, 0, -1, -1, -1, 'Natural Perfection Rank 2'),
	(33883, 0, 264200, 0, -1, -1, 0, -1, -1, -1, 'Natural Perfection Rank 3'),
	(17106, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Intensity Rank 1'),
	(17107, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Intensity Rank 2'),
	(17108, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Intensity Rank 3'),
	(43737, 0, 16, 1, -1, -1, 10000, -1, -1, -1, 'Primal Instinct'),
	(37197, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Spell Damage'),
	(43750, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Energized'),
	(33297, 0, 16, 1, -1, -1, 0, -1, -1, -1, 'Spell Haste Trinket'),
	(46662, 0, 4, 0, -1, -1, 0, -1, -1, -1, 'Deathfrost'),
	(60473, 0, 16, 0, -1, -1, 45000, -1, -1, -1, 'Forge Ember'),
	(33648, 0, 4096, 0, -1, -1, 45000, -1, -1, -1, 'Reflection of Torment'),
	(60524, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Majestic Dragon Figurine'),
	(62114, 0, 16, 0, -1, -1, 45000, -1, -1, -1, 'Flow of Knowledge'),
	(60490, 0, 16, 0, -1, -1, 45000, -1, -1, -1, 'Embrace of the Spider'),
	(62115, 0, 68, 0, -1, -1, 45000, -1, -1, -1, 'Strength of the Titans'),
	(60485, 0, 16, 0, -1, -1, 100, -1, -1, -1, 'Illustration of the Dragon Soul'),
	(60313, 0, 68, 0, -1, -1, 0, -1, -1, -1, 'Fury of the Five Flights'),
	(54707, 0, 4, 0, -1, -1, 60000, -1, -1, -1, 'Sonic Awareness (DND)'),
	(46832, 0, 16, 1, -1, -1, 0, -1, -1, -1, 'Moonkin Starfire Bonus'),
	(35086, 0, 16, 0, -1, -1, 60000, -1, -1, -1, 'Band of the Eternal Restorer'),
	(40478, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Warlock Tier 6 Trinket'),
	(40485, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Hunter Tier 6 Trinket'),
	(40458, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Warrior Tier 6 Trinket'),
	(37655, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Bonus Mana Regen'),
	(37447, 0, 16, 0, 100, -1, 0, 37445, -1, -1, 'Improved Mana Gems'),
	(37170, 0, 4, 0, 4, -1, 0, -1, -1, -1, 'Free Finisher Chance'),
	(37600, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Offensive Discount'),
	(37568, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Greater Heal Discount'),
	(37227, 0, 1073741824, 0, -1, -1, 60000, -1, -1, -1, 'Improved Healing Wave'),
	(37228, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Lesser Healing Wave Discount'),
	(37237, 0, 1073741824, 0, -1, -1, 0, -1, -1, -1, 'Lightning Bolt Discount'),
	(37195, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Judgement Group Heal'),
	(37189, 0, 1073741824, 0, -1, -1, 60000, -1, -1, -1, 'Recuced Holy Light Cast Time'),
	(37188, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Improved Judgement'),
	(37525, 0, 512, 0, -1, -1, 0, -1, -1, -1, 'Battle Rush'),
	(37516, 0, -1, 0, 100, -1, 0, -1, -1, -1, 'Revenge Bonus'),
	(27787, 0, 512, 0, 7, -1, 0, -1, -1, -1, 'Rogue Armor Energize'),
	(60172, 0, 16, 0, 100, -1, 0, -1, -1, -1, 'Life Tap Bonus Spirit'),
	(29501, 0, 64, 0, 30, -1, 0, -1, -1, -1, 'Frost Arrow'),
	(55776, 0, 4, 0, -1, -1, 60000, -1, -1, -1, 'Swordguard Embroidery'),
	(55768, 0, 16, 0, -1, -1, 60000, -1, -1, -1, 'Darkglow Embroidery'),
	(32748, 0, 70, 0, -1, -1, 0, -1, -1, -1, 'Deadly Throw Interrupt Rank 5'),
	(60063, 0, 1024, 0, -1, -1, 45000, -1, -1, -1, 'Now is the Time!'),
	(49622, 0, 16, 0, -1, -1, 45000, -1, -1, -1, 'Bonus Mana Regen'),
	(58901, 0, 4224, 0, -1, -1, 60000, -1, -1, -1, 'Tears of Anguish'),
	(60493, 0, 16, 0, -1, -1, 45000, -1, -1, -1, 'Dying Curse'),
	(45059, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Vessel of the Naaru'),
	(29601, 0, 16, 1, -1, -1, 0, -1, -1, -1, 'Enlightenment'),
	(35095, 0, -1, 0, 100, -1, 0, -1, -1, -1, 'Enlightenment'),
	(38332, 0, 16, 0, -1, -1, 0, -1, -1, -1, 'Blessing of Life'),
	(32642, 0, 536870912, 0, -1, -1, 0, -1, -1, -1, 'Spore Cloud'),
	(40484, 0, 4, 0, -1, -1, 0, -1, -1, -1, 'Acidic Wound'),
	(44599, 0, 4, 0, -1, -1, 0, -1, -1, -1, 'Inject Poison'),
	(46046, 0, 4, 0, -1, -1, 0, -1, -1, -1, 'Inject Poison');

--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-02-20_02_spell_proc' WHERE `LastUpdate` = '2016-02-20_01_spell_proc';
