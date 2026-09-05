
-- Quest 28608 "The Shadow Grave" (Cata, Deathknell) requires items 64582 (Thick Embalming Fluid)
-- and 64581 (Corpse-Stitching Twine). The two gameobjects that actually give them - 207255
-- "Thick Embalming Fluid" and 207256 "Corpse-Stitching Twine" (both real, correctly type=3 Chest,
-- spawned right next to the purely decorative "Shadow Grave Table" prop, entry 207254) - already
-- reference real loot ids (36084/36085 via their loot_id parameter), but neither loot table had
-- any rows, so the chests opened empty. 100% chance, matching the convention used elsewhere in
-- this table for guaranteed single-item drops (e.g. entryid=164).
INSERT INTO `loot_gameobjects`
(`entryid`, `itemid`, `normal10percentchance`, `normal25percentchance`, `heroic10percentchance`, `heroic25percentchance`, `mincount`, `maxcount`, `comment`)
VALUES
(36084, 64582, 100.00, 100.00, 100.00, 100.00, 1, 1, 'Thick Embalming Fluid - quest 28608 The Shadow Grave'),
(36085, 64581, 100.00, 100.00, 100.00, 100.00, 1, 1, 'Corpse-Stitching Twine - quest 28608 The Shadow Grave');

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('190', '20260905-06_cata_shadow_grave_loot');
