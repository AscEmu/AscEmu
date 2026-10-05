
SET FOREIGN_KEY_CHECKS = 0;

UPDATE `quest_poi`
SET `build` = 12340
WHERE `build` = 0;

UPDATE `quest_poi_points`
SET `build` = 12340
WHERE `build` = 0;

INSERT INTO `world_db_version` (`LastUpdate`)
SELECT '20261005-03_quest_poi_legacy_build'
WHERE NOT EXISTS (
    SELECT 1
    FROM `world_db_version`
    WHERE `LastUpdate` = '20261005-03_quest_poi_legacy_build'
);

SET FOREIGN_KEY_CHECKS = 1;
