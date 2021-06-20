-- Rename npc_monstersay to creature_ai_texts
DROP TABLE IF EXISTS creature_ai_texts;
RENAME TABLE `npc_monstersay` TO `creature_ai_texts`;
ALTER TABLE `creature_ai_texts` COMMENT='AI System';

-- Rename npc_gossip_textid to npc_gossip_properties
DROP TABLE IF EXISTS `npc_gossip_properties`;
RENAME TABLE `npc_gossip_textid` TO `npc_gossip_properties`;

-- Rename npc_text to npc_gossip_texts
DROP TABLE IF EXISTS `npc_gossip_texts`;
RENAME TABLE `npc_text` TO `npc_gossip_texts`;

-- Remove locales_npc_monstersay table
DROP TABLE IF EXISTS `locales_npc_monstersay`;

-- Rename locales_npc_text to locales_npc_gossip_texts
DROP TABLE IF EXISTS `locales_npc_gossip_texts`;
RENAME TABLE `locales_npc_text` TO `locales_npc_gossip_texts`;

INSERT INTO `world_db_version` VALUES ('81', '20210617-02_npc_text_cleanup');
