--
-- Replace into spell_proc
--
REPLACE INTO `spell_proc` VALUES (33757, 0, 4, 0, 20, 0, 3000, 33750, -1, -1);

--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-02-12_01_spell_proc' WHERE `LastUpdate` = '2016-02-11_02_areatriggers';
