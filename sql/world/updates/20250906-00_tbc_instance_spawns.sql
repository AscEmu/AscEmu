-- combine unique spawns for all versions

UPDATE creature_spawns SET min_build = 8606, max_build = 15595 WHERE `map` IN (540, 542, 545, 546, 548);

UPDATE gameobject_spawns SET min_build = 8606, max_build = 15595 WHERE `map` IN (540, 542, 545, 546, 548);

-- set missing displayids from creature_properties

UPDATE creature_spawns AS cs JOIN creature_properties AS cp ON cp.entry = cs.entry SET cs.displayid = cp.male_displayid WHERE cs.displayid = 0 and `map` IN (540, 542, 545, 546, 548);


INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('145', '20250906-00_tbc_instance_spawns');
