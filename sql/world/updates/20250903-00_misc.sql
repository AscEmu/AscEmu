-- resolving issues

UPDATE `creature_properties` SET `isTrainingDummy`='1', `range_attack_mod`='1', `rangedattacktime`='0', `bounding_radius`='0.3', `extra_a9_flags`='0' WHERE `entry` 
IN(17578,24792,30527,31143,31144,31146,32541,32542,32543,32545,32546,32666,32667,48304,44548,44794,44171,44389,44614,44820,44848,46647);


INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('144', '20250903-00_misc');
