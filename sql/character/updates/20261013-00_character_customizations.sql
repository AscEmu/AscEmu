CREATE TABLE IF NOT EXISTS `character_customizations` (
    `guid` BIGINT UNSIGNED NOT NULL,
    `chrCustomizationOptionID` INT UNSIGNED NOT NULL,
    `chrCustomizationChoiceID` INT UNSIGNED NOT NULL,
    PRIMARY KEY (`guid`, `chrCustomizationOptionID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO `character_db_version` (`LastUpdate`) VALUES ('20261013-00_character_customizations');
