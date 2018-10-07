-- Several spells had their coefficient values set too high

-- These spells share same over-time coeff value
UPDATE `spell_custom_override` SET `coef_Otspell`='0.43132' WHERE `spell_id` IN ('980','1014','6217','11711','11712','11713','27218');

UPDATE `spell_custom_override` SET `coef_Dspell`='0.43313' WHERE `spell_id`='31117';
UPDATE `spell_custom_override` SET `coef_Dspell`='0.42005' WHERE `spell_id` IN ('6353','11366');
UPDATE `spell_custom_override` SET `coef_Dspell`='0.24108' WHERE `spell_id`='635';
UPDATE `spell_custom_override` SET `coef_Dspell`='0.16106' WHERE `spell_id`='331';
UPDATE `spell_custom_override` SET `coef_Dspell`='0.16104' WHERE `spell_id`='5185';

INSERT INTO `world_db_version` VALUES ('38', '20181006-00_spell_custom_override');
