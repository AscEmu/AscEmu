--
-- Disable spell 46598 until it is handled.
--
REPLACE INTO spell_disable (spellid, replacement_spellid) VALUES (46598, 0);
   
--
-- Update world db version
--   
UPDATE `world_db_version` SET `LastUpdate` = '2016-01-12_03_spell_disable' WHERE `LastUpdate` = '2016-01-12_02_creature_initial_equip';

