UPDATE item_properties SET randomprop = 0 WHERE entry IN (5744, 7109);
UPDATE item_properties SET spellid_1 = 0 WHERE spellid_1 = -1;
UPDATE item_properties SET spellid_2 = 0 WHERE spellid_2 = -1;
UPDATE item_properties SET spellid_3 = 0 WHERE spellid_3 = -1;

UPDATE reputation_creature_onkill SET rep_limit = 21000 WHERE creature_id = 23051;

-- Update world db version
INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('140', '20240904-00_misc');
