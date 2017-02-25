--
-- update go rotation
--
UPDATE `gameobject_spawns` SET `facing` = 1.55194, `orientation3` = -0.700409, `orientation4` = -0.713742 WHERE `id` = 65926;
UPDATE `gameobject_spawns` SET `facing` = 1.18752, `orientation3` = -0.55948, `orientation4` = -0.828844 WHERE `id` = 65927;
UPDATE `gameobject_spawns` SET `facing` = 0.794816, `orientation3` = -0.38703, `orientation4` = -0.922067 WHERE `id` = 65928;
UPDATE `gameobject_spawns` SET `facing` = 0.388765, `orientation3` = -0.193161, `orientation4` = -0.981167 WHERE `id` = 65929;
UPDATE `gameobject_spawns` SET `facing` = 0.0361213, `orientation3` = -0.01806, `orientation4` = -0.999837 WHERE `id` = 65930;
UPDATE `gameobject_spawns` SET `facing` = 5.90147, `orientation3` = -0.189699, `orientation4` = 0.981842 WHERE `id` = 65931;
UPDATE `gameobject_spawns` SET `facing` = 4.70846, `orientation3` = -0.708496, `orientation4` = 0.705715 WHERE `id` = 65915;
UPDATE `gameobject_spawns` SET `facing` = 4.31619, `orientation3` = -0.832442, `orientation4` = 0.554113 WHERE `id` = 65916;
UPDATE `gameobject_spawns` SET `facing` = 3.93144, `orientation3` = -0.923025, `orientation4` = 0.384739 WHERE `id` = 65920;
UPDATE `gameobject_spawns` SET `facing` = 3.58273, `orientation3` = -0.975774, `orientation4` = 0.218782 WHERE `id` = 65921;
UPDATE `gameobject_spawns` SET `facing` = 3.15675, `orientation3` = -0.999971, `orientation4` = 0.00757775 WHERE `id` = 65923;
UPDATE `gameobject_spawns` SET `facing` = 2.76169, `orientation3` = -0.982014, `orientation4` = -0.18881 WHERE `id` = 65925;

UPDATE `gameobject_spawns` SET `facing` = 2.13416, `orientation3` = -0.875795, `orientation4` = -0.482683 WHERE `id` = 63177;
UPDATE `gameobject_spawns` SET `facing` = 2.13416, `orientation3` = -0.875795, `orientation4` = -0.482683 WHERE `id` = 65865;
UPDATE `gameobject_spawns` SET `facing` = 2.13416, `orientation3` = -0.875795, `orientation4` = -0.482683 WHERE `id` = 66234;
UPDATE `gameobject_spawns` SET `facing` = 2.13416, `orientation3` = -0.875795, `orientation4` = -0.482683 WHERE `id` = 66385;
UPDATE `gameobject_spawns` SET `facing` = 2.13416, `orientation3` = -0.875795, `orientation4` = -0.482683 WHERE `id` = 66386;

UPDATE `gameobject_spawns` SET `facing` = 5.28887, `orientation3` = -0.47693, `orientation4` = 0.878941 WHERE `id` = 63178;
UPDATE `gameobject_spawns` SET `facing` = 5.28887, `orientation3` = -0.47693, `orientation4` = 0.878941 WHERE `id` = 65866;
UPDATE `gameobject_spawns` SET `facing` = 5.28887, `orientation3` = -0.47693, `orientation4` = 0.878941 WHERE `id` = 66235;
UPDATE `gameobject_spawns` SET `facing` = 5.28887, `orientation3` = -0.47693, `orientation4` = 0.878941 WHERE `id` = 66387;
UPDATE `gameobject_spawns` SET `facing` = 5.28887, `orientation3` = -0.47693, `orientation4` = 0.878941 WHERE `id` = 66388;

UPDATE `gameobject_spawns` SET `facing` = 3.71336, `orientation3` = -0.959412, `orientation4` = 0.282007 WHERE `id` = 63179;
UPDATE `gameobject_spawns` SET `facing` = 3.71336, `orientation3` = -0.959412, `orientation4` = 0.282007 WHERE `id` = 65867;
UPDATE `gameobject_spawns` SET `facing` = 3.71336, `orientation3` = -0.959412, `orientation4` = 0.282007 WHERE `id` = 66236;
UPDATE `gameobject_spawns` SET `facing` = 3.71336, `orientation3` = -0.959412, `orientation4` = 0.282007 WHERE `id` = 66389;
UPDATE `gameobject_spawns` SET `facing` = 3.71336, `orientation3` = -0.959412, `orientation4` = 0.282007 WHERE `id` = 66390;

--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate`='2017-02-25_01_gameobject_spawns' WHERE  `LastUpdate`='2017-02-06_01_creature_spawns';
