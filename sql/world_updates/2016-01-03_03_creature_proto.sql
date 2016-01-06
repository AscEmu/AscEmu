--
-- Update creature_proto
--
UPDATE `creature_proto` SET `minhealth` = 181020, `maxhealth` = 181020 WHERE `entry` = 38112; -- Falric normal

--
-- INSERT proto difficulty
--
INSERT INTO `creature_proto_difficulty` (`entry`, `difficulty_type`, `minlevel`, `maxlevel`, `faction`, `minhealth`, `maxhealth`, `mana`, `scale`, `npcflags`, `attacktime`, `attacktype`, `mindamage`, `maxdamage`, `can_ranged`, `rangedattacktime`, `rangedmindamage`, `rangedmaxdamage`, `respawntime`, `armor`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `combat_reach`, `bounding_radius`, `auras`, `boss`, `money`, `invisibility_type`, `walk_speed`, `run_speed`, `fly_speed`, `extra_a9_flags`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `spell_flags`, `modImmunities`, `isTrainingDummy`, `guardtype`, `summonguard`, `spelldataid`, `vehicleid`, `rooted`) VALUES
	(38112, 1, 82, 82, 0, 303855, 303855, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, NULL, 0, 0, 0, 2.5, 8, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

--
-- Update world db version
--   
UPDATE `world_db_version` SET `LastUpdate` = '2016-01-03_03_creature_proto' WHERE `LastUpdate` = '2016-01-03_02_gossip_menu_option';

