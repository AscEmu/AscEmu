/*Add column temp_id to old table trainer_spells*/
ALTER TABLE `trainer_spells` ADD COLUMN `temp_id` INT DEFAULT 0 AFTER `deletespell`;
UPDATE `trainer_spells` SET `temp_id` = 79 WHERE `entry` = 26905;

/*Blacksmithing
Spellsets 61-63

 All NPCs in these spellsets should have can_train_min_skill_value and can_train_max_skill_value at 0.*/

UPDATE `trainer_properties` SET `can_train_min_skill_value` = 0, `can_train_max_skill_value` = 0 WHERE `spellset_id` IN(61,62,63);

/*Alchemy
Spellsets 68-70

 NPC with id 33608 should use spellset 69.
 Spellset 68 can be fully removed.*/

UPDATE `trainer_properties` SET `spellset_id` = 69 WHERE `entry` = 33608;
DELETE FROM `trainer_properties_spellset` WHERE `id` = 68;

/*Cooking
Spellset 78

 NPCs with can_train_max_skill_value == 0 should use a new spellset that has spells from NPC id 26905 from old table.
*/

REPLACE INTO `trainer_properties_spellset` (`id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell`)
SELECT `temp_id`, `cast_spell`, `learn_spell`, `spellcost`, `reqspell`, `reqskill`, `reqskillvalue`, `reqlevel`, `deletespell` FROM `trainer_spells` WHERE `entry` = 26905;

UPDATE `trainer_properties` SET `spellset_id` = 79 WHERE `spellset_id` = 78 AND `can_train_max_skill_value` = 0;

ALTER TABLE `trainer_spells` DROP COLUMN `temp_id`;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('118', '20230211_00_trainer_properties');
