-- Generate new action bars on next summon
UPDATE `playerpets` SET `actionbar`='';
-- Pet::addSpell will set this correct on next summon
UPDATE `playerpetspells` SET `flags`=0x81;

INSERT INTO `character_db_version` (`id`, `LastUpdate`) VALUES ('15', '20250921-00_playerpets');
