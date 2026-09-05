
-- creature_properties row for the Track Switch NPC (entry 60283, used by both interactive
-- lever creatures) was missing entirely - spawnCreature() silently failed for both track
-- switches ("tried to push a invalid creature with entry 60283"). Real displayid/faction/
-- speed values, confirmed against creature_template entry 60283 ("Track Switch").
INSERT INTO `creature_properties`
(`entry`, `build`, `male_displayid`, `female_displayid`, `name`, `subname`, `type_flags`, `type`, `family`, `rank`,
 `base_attack_mod`, `range_attack_mod`, `minlevel`, `maxlevel`, `faction`, `minhealth`, `maxhealth`, `mana`, `scale`,
 `attacktime`, `mindamage`, `maxdamage`, `respawntime`, `combat_reach`, `bounding_radius`, `auras`, `boss`, `walk_speed`, `run_speed`, `fly_speed`, `armor`)
VALUES
(60283, 18414, 42102, 0, 'Track Switch', '', 0, 10, 0, 0, 1, 1, 1, 1, 35, 500000, 500000, 0, 1, 2000, 0, 0, 0, 1, 1, '', 0, 1.0, 1.14286, 14, 0);

-- The Mine Cart's (60140) level was set to a placeholder 90/90 in the previous migration -
-- its real creature_template row (entry 60140) has minlevel/maxlevel = 1, same as the Track
-- Switch above (both are non-combat mechanic props, not level-90 creatures).
UPDATE `creature_properties` SET `minlevel` = 1, `maxlevel` = 1 WHERE `entry` = 60140 AND `build` = 18414;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('187', '20260905-03_mop_silvershard_track_switch');
