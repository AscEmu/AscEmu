-- Invisibility_type column will be removed or changed in future because almost every invisibility type has its own passive aura

-- Shaman's totem quest npcs (invisibility type 1)
-- These npcs have already correct aura set
UPDATE `creature_properties` SET `invisibility_type`='0' WHERE `invisibility_type`='1';

-- Shade of Jin'do npc (invisibility_type 2)
-- Only npc with this type, add invisibility aura to the npc
-- Spell id 24307, Shade of Jin'do Passive
UPDATE `creature_properties` SET `invisibility_type`='0',`auras`='24307' WHERE `invisibility_type`='2';

-- Quest npc invisibility (invisibility_type 4)
UPDATE `creature_properties` SET `invisibility_type`='0' WHERE `invisibility_type`='4';
-- Multiple trigger npcs had incorrect invisibility type
UPDATE `creature_properties` SET `invisibility_type`='15' WHERE `entry`
IN ('17794', '17795', '18263', '18264','18782','20767','20809','22515');

-- Dungeon Set 2 npcs (invisibility_type 5)
-- These npcs have already correct aura set
UPDATE `creature_properties` SET `invisibility_type`='0' WHERE `invisibility_type`='5';

-- "Pink"/drunk npcs, related to Brewfest (invisibility_type 6)
-- These npcs have already correct aura set
UPDATE `creature_properties` SET `invisibility_type`='0' WHERE `invisibility_type`='6';

-- Quest npc invisibility (invisibility_type 7)
-- These npcs have already correct aura set
UPDATE `creature_properties` SET `invisibility_type`='0' WHERE `invisibility_type`='7';

-- Quest npc invisibility (invisibility_type 8)
-- These npcs have already correct aura set
UPDATE `creature_properties` SET `invisibility_type`='0' WHERE `invisibility_type`='8';

-- Quest npc invisibility (invisibility_type 9)
-- These npcs have already correct aura set
UPDATE `creature_properties` SET `invisibility_type`='0' WHERE `invisibility_type`='9';

-- Quest npc invisibility (invisibility_type 10)
-- These npcs have already correct aura set
UPDATE `creature_properties` SET `invisibility_type`='0' WHERE `invisibility_type`='10';
