-- Update right cell lever location
UPDATE `gameobject_spawns` SET `position_x` = '-252.696', `position_y` = '2114.22', `position_z` = '82.8052', `facing` = '-0.190831', `orientation1` = '-0.131892', 
`orientation2` = '-0.694696', orientation3 = '-0.131892', `orientation4` = '0.694698', `state` = '1' WHERE `entry` = '18900';

-- Update middle cell lever location
UPDATE `gameobject_spawns` SET `position_x` = '-249.22', `position_y` = '2123.1', `position_z` = '82.8052', `facing` = '-0.190831', `orientation1` = '-0.131892', 
`orientation2` = '-0.694696', `orientation3` = '-0.131892', `orientation4` = '0.694698' WHERE `entry` = '18901';

-- Update left cell lever location
UPDATE `gameobject_spawns` SET `position_x` = '-245.598', `position_y` = '2132.32', `position_z` = '82.8052', `facing` = '-0.190831', `orientation1` = '-0.131892',
`orientation2` = '-0.694696', `orientation3` = '-0.131892', `orientation4` = '0.694698' WHERE `entry` = '101811';

-- Update all prisoners script texts type from "Yelling" to "Saying"
UPDATE `npc_script_text` SET `type` = '12' WHERE `creature_entry` in ('3849', '3850');

-- update ashrombe orientation
UPDATE `creature_spawns` SET `orientation` = '2.706567' WHERE `entry` = '3850';

-- Added missing prisoners script texts
DELETE FROM `npc_script_text` WHERE `entry` IN ('8787', '8788');
INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`) VALUES
('8787', 'Deathstalker Adamant fumbles with the rusty lock on the courtyard door.', '3849', '0', '16', '1', '100', '0', '0', '0', '0'),
('8788', 'Sorcerer Ashcrombe vanishes.', '3850', '0', '16', '0', '100', '0', '0', '0', '0');

-- Added Arugal's intro event texts
DELETE FROM npc_script_text WHERE entry IN ('8789', '8790', '8791', '8792', '8793');
INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`) VALUES
('8789', 'I have changed my mind loyal servants, you do not need to bring the prisoner all the way to my study, I will deal with him here and now.', '10000', '0', '12', '0', '100', '0', '0', '0', '0'),
('8790', 'Vincent! You and your pathetic ilk will find no more success in routing my sons and I than those beggardly remnants of the Kirin Tor.', '10000', '1', '12', '0', '100', '0', '0', '0', '0'),
('8791', 'If you will not serve my Master with your sword and knowledge of his enemies...', '10000', '2', '12', '0', '100', '0', '0', '0', '0'),
('8792', 'Your moldering remains will serve ME as a testament to what happens when one is foolish enough to trespass in my domain!', '10000', '3', '12', '0', '100', '0', '0', '0', '0'),
('8793', 'Arrrgh!', '4444', '0', '12', '0', '100', '0', '0', '0', '0');

-- Prisoners gossip option
DELETE FROM `gossip_menu_option` WHERE `entry` = '606';
INSERT INTO `gossip_menu_option` (`entry`, `option_text`) VALUES
('606', 'Please unlock the courtyard door.');

-- Update recall location
-- Current recall location ports you to not instanced empty building
DELETE FROM `recall` WHERE id = '526';
INSERT INTO `recall` (`id`, `name`, `MapId`, `positionX`, `positionY`, `positionZ`, `Orientation`) VALUES
('526','ShadowfangKeep', '0', '-238.328', '1556.3', '76.8921', '1.01552');

-- Deleted not needed monstersay texts
DELETE FROM `npc_monstersay` WHERE `entry` IN ('3849', '3850');

-- Boss Arugal texts
DELETE FROM `npc_script_text` WHERE `entry` IN ('8794', '8795', '8796');
INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `broadcast_id`) VALUES
('8794', 'You, too, shall serve!', '4275', '0', '14', '0', '100', '0', '0', '5793', '0'),
('8795', 'Another Falls!', '4275', '0', '14', '0', '100', '0', '0', '5795', '0'),
('8796', 'Release your rage!', '4275', '0', '14', '0', '100', '0', '0', '5797', '0');

-- Update arugal unit flags
UPDATE `creature_spawns` SET `flags` = '64' WHERE `entry`= '4275';

-- Updated prisoners texts to use EMOTE_ONESHOT_TALK
UPDATE npc_script_text SET duration = '1000', `emote` = '1' WHERE `creature_entry` IN ('3849', '3850') AND `entry` NOT IN ('8787', '8788', '2117', '2122');

-- Add auras to creatures
UPDATE creature_properties SET `auras` = '12544' WHERE `entry` = '3851';
UPDATE creature_properties SET `auras` = '7165' WHERE `entry` = '3872';

-- Set last world sql update
UPDATE `world_db_version` SET `LastUpdate` = '2017-10-14_01_shadowfang_keep' WHERE `LastUpdate` = '2017-09-20_01_gossip_menu_tables';