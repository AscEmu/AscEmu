-- https://wowhead.com/npc=6491
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43967 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43969 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43982 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43983 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43984 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43985 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43986 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43987 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43988 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43989 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43990 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43991 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43992 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43993 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43994 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43995 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43996 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=43999 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44007 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44009 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44010 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44011 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44013 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44014 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44015 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44016 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44017 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44018 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44019 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44020 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44021 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44073 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44138 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44149 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44157 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44158 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44159 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44160 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=44177 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`='0' WHERE  `id`=116767 AND `min_build`=12340;
-- double spawn
DELETE FROM `creature_spawns` WHERE  `id`=134897;
DELETE FROM `creature_spawns` WHERE  `id`=310414;
DELETE FROM `creature_spawns` WHERE  `id`=310662;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('107', '20220522-00_creature_spawns');
