-- Frost Armor is no longer a mage starting spell in cata, it is learnt at level 56
UPDATE `playercreateinfo_spell_learn` SET `max_build`='12340' WHERE `spellid`='168';
-- Seal of Righteousness is no longer a paladin starting spell in cata, it is learnt at level 3
UPDATE `playercreateinfo_spell_learn` SET `max_build`='12340' WHERE `spellid`='21084';
-- Rogues no longer have passive threat reduction in cata
UPDATE `playercreateinfo_spell_learn` SET `max_build`='12340' WHERE `spellid`='21184';

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('97', '20211112-00_playercreateinfo_spell_learn');
