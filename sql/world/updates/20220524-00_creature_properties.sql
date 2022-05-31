-- Remove chain aura from Unworthy Initiates
UPDATE `creature_properties` SET `auras`='' WHERE `auras`='54612';
INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('108', '20220524-00_creature_properties');
