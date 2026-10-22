-- playercreateinfo_skills: 11 rows removed, the clients since 6.x grant the riding skill through the riding spells only
DELETE FROM `playercreateinfo_skills` WHERE `skillid` = 762 AND `min_build` >= 21742;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('259', '20261022-59_playercreateinfo_riding');
