
UPDATE `gameobject_spawns` SET `state` = '1' WHERE (`map` = '608');
UPDATE `gameobject_spawns` SET `state`= '0' WHERE (`id`='18517');
INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `probability`, `emote`, `broadcast_id`) VALUES ('10760','I\'m locking the door. Good luck, and thank you for doing this.', '30658', '2', '12', '100', '396', '31475');
INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `probability`, `emote`, `broadcast_id`) VALUES ('10761','You did it! You held the Blue Dragonflight back and defeated their commander. Amazing work!', '30658', '3', '12', '100', '396', '31694');
INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `probability`, `emote`, `broadcast_id`) VALUES ('10762','An elite Blue Dragonflight squad appears from the portal!', '32204', '1', '12', '100', '0', '32994');
INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `probability`, `emote`, `broadcast_id`) VALUES ('10763','A Portal Guardian defends the new portal!', '32204', '2', '12', '100', '0', '32995');
INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `probability`, `emote`, `broadcast_id`) VALUES ('10764','A Portal Keeper emerges from the portal!', '32204', '3', '12', '100', '0', '32996');
INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `probability`, `emote`, `broadcast_id`) VALUES ('10765','Adventurers, the door is beginning to weaken!', '32204', '4', '12', '100', '0', '32557');
INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `probability`, `emote`, `broadcast_id`) VALUES ('10766','Only half of the door seal\'s strength remains! You must fight on!', '32204', '5', '12', '100', '0', '32558');
INSERT INTO `npc_script_text` (`entry`, `text`, `creature_entry`, `id`, `type`, `probability`, `emote`, `broadcast_id`) VALUES ('10767','The door seal is about to collapse! All is lost if the Blue Dragonflight breaks through the door!', '32204', '6', '12', '100', '0', '32559');

UPDATE `creature_properties` SET `minlevel`='75', `maxlevel`='75', `faction`='1720' WHERE (`entry`='30664') AND (`build`='9056');
UPDATE `creature_properties` SET `minlevel`='77', `maxlevel`='77', `faction`='16' WHERE (`entry`='32234') AND (`build`='9056');
UPDATE `creature_properties` SET `minlevel`='77', `maxlevel`='77', `faction`='16' WHERE (`entry`='32231') AND (`build`='9056');
UPDATE `creature_properties` SET `minlevel`='77', `maxlevel`='77', `faction`='16' WHERE (`entry`='32237') AND (`build`='9056');
UPDATE `creature_properties` SET `minlevel`='77', `maxlevel`='77', `faction`='16' WHERE (`entry`='32230') AND (`build`='9056');
UPDATE `creature_properties` SET `minlevel`='77', `maxlevel`='77', `faction`='16' WHERE (`entry`='32226') AND (`build`='9056');
UPDATE `creature_properties` SET `minlevel`='77', `maxlevel`='77', `faction`='16' WHERE (`entry`='32235') AND (`build`='9056');
UPDATE `creature_properties` SET `minlevel`='76', `maxlevel`='76', `faction`='16' WHERE (`entry`='32228') AND (`build`='9056');

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('114', '20221023_00_violet_hold');
