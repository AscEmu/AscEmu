-- void elf demon hunter (12.x): 1 start position, 6 bar buttons; start zones of 29 rows
INSERT INTO `playercreateinfo` (`build`, `race`, `class`, `mapID`, `zoneID`, `positionX`, `positionY`, `positionZ`, `orientation`) VALUES
(70575, 29, 12, 1481, 7705, 1180.64, 3284.96, 70.3484, 4.76134);

INSERT INTO `playercreateinfo_bars` (`build`, `race`, `class`, `button`, `action`, `type`, `misc`) VALUES
(70575, 29, 12, 0, 344865, 0, 0),
(70575, 29, 12, 1, 344859, 0, 0),
(70575, 29, 12, 2, 344862, 0, 0),
(70575, 29, 12, 9, 256948, 0, 0),
(70575, 29, 12, 10, 188501, 0, 0),
(70575, 29, 12, 11, 200175, 0, 0);

UPDATE `playercreateinfo` SET `zoneID` = 7503 WHERE `race` = 28 AND `mapID` = 1220 AND `zoneID` = 0;
UPDATE `playercreateinfo` SET `zoneID` = 15342 WHERE `race` IN (84, 85) AND `mapID` = 2739 AND `zoneID` = 0;
UPDATE `playercreateinfo` SET `zoneID` = 7705 WHERE `class` = 12 AND `mapID` = 1481 AND `zoneID` = 6383;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('285', '20261027-08_playercreateinfo_new_races');
