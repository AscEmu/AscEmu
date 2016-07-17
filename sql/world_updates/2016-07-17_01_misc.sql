--
-- Removed invalid npc_text defines
--
DELETE FROM `npc_gossip_textid` WHERE `creatureid` = 2075;

--
-- Removed invalid spelloverrides defines
--
DELETE FROM `spelloverride` WHERE `spellId` IN(12730,6226,11703,11704,27221,30908,6226,11703,11704,27221,30908,18265,18879,18880,18881,23439,27264,30911);


--
-- Update world_db_version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-07-17_01_misc' WHERE `LastUpdate` = '2016-07-08_01_misc';
