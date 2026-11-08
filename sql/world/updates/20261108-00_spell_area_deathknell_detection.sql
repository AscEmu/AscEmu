-- 1 spell area: new undead in the Deathknell graves detect the invisible Val'kyr (65051) until The Shadow Grave is done
REPLACE INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_start_active`, `quest_end`, `aura_spell`, `racemask`, `gender`, `autocast`) VALUES
(65051, 5692, 0, 0, 28608, 0, 16, 2, 1);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('288', '20261108-00_spell_area_deathknell_detection');
