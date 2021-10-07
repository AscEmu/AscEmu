UPDATE `creature_quest_starter` SET min_build = 13202, max_build = 18414 WHERE id IN(34668,34693,34830,34835,34865,34872,34874,34876,34877,34878,34957,34958,34959,35053,35054,35120,35175,35200,35222,35486,37106,37114,37203,37598);

UPDATE `creature_quest_finisher` SET min_build = 13202, max_build = 18414 WHERE id IN(34668,34693,34872,34874,35053,35054,35120,35222,37106,37602);

UPDATE `creature_spawns` SET min_build = 12984, max_build = 18414 WHERE entry = 40441;
UPDATE `creature_spawns` SET min_build = 13329, max_build = 18414 WHERE entry = 15760;

INSERT INTO `world_db_version` VALUES ('88', '20210825-00_multiversion_corrections');
