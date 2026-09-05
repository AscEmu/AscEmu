
-- creature_properties/gameobject_properties rows for the Silvershard Mines mine cart and the
-- Silvershard Mines/Temple of Kotmogu gameobjects (gates, orbs, door) were missing entirely -
-- spawnCreature()/spawnGameObject() both require a properties row to exist before they will spawn
-- anything, so without this migration neither battleground's mechanic could spawn a single object,
-- silently failing every time. Real field values (entries confirmed real and present on the
-- correct maps earlier this session).
INSERT INTO `gameobject_properties`
(`entry`, `build`, `type`, `display_id`, `name`, `parameter_1`, `size`)
VALUES
-- Silvershard Mines gates (type=0 Door, real display ids)
(212939, 18414, 0, 851, 'Gate', 5743776, 1),
(212940, 18414, 0, 849, 'Gate', 5743776, 1),
(212941, 18414, 0, 851, 'Gate', 5743776, 1),
(212942, 18414, 0, 849, 'Gate', 5743776, 1),
-- Temple of Kotmogu orbs (type=24 Flagstand, matching WSG's own flag type) and door
(212091, 18414, 24, 11533, 'Orb of Power', 121164, 4),
(212092, 18414, 24, 11533, 'Orb of Power', 121175, 4),
(212093, 18414, 24, 11533, 'Orb of Power', 121176, 4),
(212094, 18414, 24, 11533, 'Orb of Power', 121177, 4),
(213172, 18414, 0, 11920, 'Great Door', 5743776, 0.698848);

-- Mine Cart creature (real displayid/faction/speeds).
-- minhealth/maxhealth are a non-combat placeholder - modern retail computes this dynamically via a
-- content-tuning system with no flat value to copy, and the cart isn't meant to be attacked.
INSERT INTO `creature_properties`
(`entry`, `build`, `male_displayid`, `female_displayid`, `name`, `subname`, `type_flags`, `type`, `family`, `rank`,
 `base_attack_mod`, `range_attack_mod`, `minlevel`, `maxlevel`, `faction`, `minhealth`, `maxhealth`, `mana`, `scale`,
 `attacktime`, `mindamage`, `maxdamage`, `respawntime`, `combat_reach`, `bounding_radius`, `auras`, `boss`, `walk_speed`, `run_speed`, `fly_speed`, `armor`)
VALUES
(60140, 18414, 41243, 0, 'Mine Cart', '', 0, 10, 0, 0, 1, 1, 90, 90, 35, 500000, 500000, 0, 1, 2000, 0, 0, 0, 1, 1, '', 0, 2.0, 2.0, 14, 0);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('186', '20260905-02_mop_silvershard_kotmogu_properties');
