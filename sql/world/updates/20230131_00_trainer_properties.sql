/*Add column temp_id to old table trainer_spells*/
ALTER TABLE `trainer_spells` ADD COLUMN `temp_id` INT DEFAULT 0 AFTER `deletespell`;
UPDATE `trainer_spells` SET `temp_id` = 62 WHERE `entry` = 4258;
UPDATE `trainer_spells` SET `temp_id` = 63 WHERE `entry` = 16583;
UPDATE `trainer_spells` SET `temp_id` = 70 WHERE `entry` = 1215;

UPDATE `trainer_spells` SET `temp_id` = 98 WHERE `entry` = 26906;
UPDATE `trainer_spells` SET `temp_id` = 99 WHERE `entry` = 1317;
UPDATE `trainer_spells` SET `temp_id` = 95 WHERE `entry` = 18753;
UPDATE `trainer_spells` SET `temp_id` = 93 WHERE `entry` = 1676;

/*Blacksmithing:
Spellset 61

 In trainer_properties NPCs with can_train_min_skill_value == 300 should use a new spellset that has spells from NPC id 4258 from old table.

 NPCs with can_train_min_skill_value == 350 OR can_train_max_skill_value == 350 should use a new spellset that has spells from NPC id 16583 from old table.
 Also, NPC id 33609 should use this new spellset.*/

REPLACE INTO `trainer_properties_spellset` (`id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell`)
SELECT `temp_id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell` FROM `trainer_spells` WHERE `entry` = 4258;

UPDATE `trainer_properties` SET `spellset_id` = 62 WHERE `spellset_id` = 61 AND `can_train_min_skill_value` = 300;

REPLACE INTO `trainer_properties_spellset` (`id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell`)
SELECT `temp_id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell` FROM `trainer_spells` WHERE `entry` = 16583;

UPDATE `trainer_properties` SET `spellset_id` = 63 WHERE `spellset_id` = 61 AND `can_train_min_skill_value` = 350;
UPDATE `trainer_properties` SET `spellset_id` = 63 WHERE `spellset_id` = 61 AND `can_train_max_skill_value` = 350;
UPDATE `trainer_properties` SET `spellset_id` = 63 WHERE `entry` = 33609;

/*Leatherworking
Spellsets 64-67

 Spellset 66 can be fully removed.

 NPC id 26996 should use spellset 67 and NPC id 33612 should use spellset 65.*/
 
DELETE FROM `trainer_properties_spellset` WHERE `id` = 66;
UPDATE `trainer_properties` SET `spellset_id` = 67 WHERE `entry` = 26996;
UPDATE `trainer_properties` SET `spellset_id` = 65 WHERE `entry` = 33612;

/*Alchemy
Spellset 69

 NPCs with can_train_max_skill_value == 325 but excluding NPC ids 16588, 18802, 19052, 27023, 27029, 33630, 33674 should use a new spellset that has spells from NPC id 1215 from old table.
*/

REPLACE INTO `trainer_properties_spellset` (`id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell`)
SELECT `temp_id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell` FROM `trainer_spells` WHERE `entry` = 1215;

UPDATE `trainer_properties` SET `spellset_id` = 70 WHERE `spellset_id` = 69 AND `can_train_max_skill_value` = 325 AND `entry` NOT IN(16588, 18802, 19052, 27023, 27029, 33630, 33674);

/*Herbalism
Spellsets 73-74

 NPC id 33616 should use spellset 73.*/

UPDATE `trainer_properties` SET `spellset_id` = 73 WHERE `entry` = 33616;

/*Tailoring
Spellsets 75-77

 NPC id 33613 should use spellset 76.*/
 
UPDATE `trainer_properties` SET `spellset_id` = 76 WHERE `entry` = 33613;

/*Cooking
Spellset 78

 NPC id 33619 should have can_train_max_skill_value at 275.*/
 
UPDATE `trainer_properties` SET `can_train_max_skill_value` = 275 WHERE `entry` = 33619;
 
/*Mining
Spellsets 81-83

 NPC id 33617 should use spellset 82.*/

UPDATE `trainer_properties` SET `spellset_id` = 82 WHERE `entry` = 33617;
 
/*First Aid
Spellsets 84-86

 NPC id 33621 should use spellset 85.*/
 
UPDATE `trainer_properties` SET `spellset_id` = 85 WHERE `entry` = 33621;
 
/*Engineering
Spellsets 87, 90, 92, 93

 NPCs with id 17637, 19576, 33611, 33634, 33677 should use spellset 87.

 Spellset 90 can be fully removed.

 Remove all spells from spellset 93 and replace them with spells from NPC id 1676 from old table.*/

UPDATE `trainer_properties` SET `spellset_id` = 87 WHERE `entry` IN(17637, 19576, 33611, 33634, 33677);

DELETE FROM `trainer_properties_spellset` WHERE `id` = 90;

DELETE FROM `trainer_properties_spellset` WHERE `id` = 93;
REPLACE INTO `trainer_properties_spellset` (`id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell`)
SELECT `temp_id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell` FROM `trainer_spells` WHERE `entry` = 1676;

/*Enchanting
Spellsets 97-98

 NPCs with id 18753, 18773, 19252, 19540, 33610, 33633, 33676 should use a new spellset that has spells from NPC id 18753 from old table.

 Spellset 97 can be fully removed.

 NPCs with can_train_max_skill_value == 295 or with id 19251 should use a new spellset that has spells from NPC id 1317 from old table.

 Remove all spells from spellset 98 and replace them with spells from NPC id 26906 from old table.*/

REPLACE INTO `trainer_properties_spellset` (`id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell`)
SELECT `temp_id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell` FROM `trainer_spells` WHERE `entry` = 18753;
UPDATE `trainer_properties` SET `spellset_id` = 95 WHERE `entry` IN(18753, 18773, 19252, 19540, 33610, 33633, 33676);

DELETE FROM `trainer_properties_spellset` WHERE `id` = 97;

REPLACE INTO `trainer_properties_spellset` (`id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell`)
SELECT `temp_id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell` FROM `trainer_spells` WHERE `entry` = 1317;
UPDATE `trainer_properties` SET `spellset_id` = 99 WHERE `spellset_id` IN(97,98) AND `can_train_max_skill_value` = 295;
UPDATE `trainer_properties` SET `spellset_id` = 99 WHERE `entry` = 19251;

DELETE FROM `trainer_properties_spellset` WHERE `id` = 98;
REPLACE INTO `trainer_properties_spellset` (`id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell`)
SELECT `temp_id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell` FROM `trainer_spells` WHERE `entry` = 26906;

/*Fishing
Spellset 100

 NPC id 33623 should have can_train_max_skill_value at 275.*/
 
UPDATE `trainer_properties` SET `can_train_max_skill_value` = 275 WHERE `entry` = 33623;
 
/*Skinning
Spellset 105

 NPCs with can_train_max_skill_value == 25, change to 200.

 NPCs with can_train_max_skill_value == 40 or NPC id is 33618, change to 275.*/
 
UPDATE `trainer_properties` SET `can_train_max_skill_value` = 200 WHERE `spellset_id` = 105 AND `can_train_max_skill_value` = 25;

UPDATE `trainer_properties` SET `can_train_max_skill_value` = 275 WHERE `spellset_id` = 105 AND `can_train_max_skill_value` = 40;
UPDATE `trainer_properties` SET `can_train_max_skill_value` = 275 WHERE `entry` = 33618;

/*Jewelcrafting
Spellsets 114-116

 NPCs with id 19063 and 33614 should use spellset 115.*/
 
UPDATE `trainer_properties` SET `spellset_id` = 115 WHERE `entry` IN(19063,33614);
 
/*Inscription
Spellsets 122-124

 NPC with id 33615 should use spellset 123.*/
 
UPDATE `trainer_properties` SET `spellset_id` = 123 WHERE `entry` = 33615;

ALTER TABLE `trainer_spells` DROP COLUMN `temp_id`;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('117', '20230131_00_trainer_properties');
