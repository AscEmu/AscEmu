
SET FOREIGN_KEY_CHECKS = 0;

-- Remove points belonging to ambiguous Naxxramas POIs first.
DELETE points
FROM `quest_poi_points` AS points
INNER JOIN `quest_poi` AS poi
    ON poi.`build` = points.`build`
   AND poi.`questId` = points.`questId`
   AND poi.`poiId` = points.`poiId`
WHERE poi.`build` = 69893
  AND poi.`mapAreaId` = 535;

DELETE FROM `quest_poi`
WHERE `build` = 69893
  AND `mapAreaId` = 535;

UPDATE `quest_poi`
SET `mapAreaId` = CASE `mapAreaId`
        WHEN 4 THEN 1411
        WHEN 9 THEN 1412
        WHEN 11 THEN 1413
        WHEN 15 THEN 1416
        WHEN 16 THEN 1417
        WHEN 17 THEN 1418
        WHEN 19 THEN 1419
        WHEN 20 THEN 1420
        WHEN 21 THEN 1421
        WHEN 22 THEN 1422
        WHEN 23 THEN 1423
        WHEN 24 THEN 1424
        WHEN 26 THEN 1425
        WHEN 27 THEN 1426
        WHEN 28 THEN 1427
        WHEN 29 THEN 1428
        WHEN 30 THEN 1429
        WHEN 32 THEN 1430
        WHEN 34 THEN 1431
        WHEN 35 THEN 1432
        WHEN 36 THEN 1433
        WHEN 37 THEN 1434
        WHEN 38 THEN 1435
        WHEN 39 THEN 1436
        WHEN 40 THEN 1437
        WHEN 41 THEN 1438
        WHEN 42 THEN 1439
        WHEN 43 THEN 1440
        WHEN 61 THEN 1441
        WHEN 81 THEN 1442
        WHEN 101 THEN 1443
        WHEN 121 THEN 1444
        WHEN 141 THEN 1445
        WHEN 161 THEN 1446
        WHEN 181 THEN 1447
        WHEN 182 THEN 1448
        WHEN 201 THEN 1449
        WHEN 241 THEN 1450
        WHEN 261 THEN 1451
        WHEN 281 THEN 1452
        WHEN 301 THEN 1453
        WHEN 321 THEN 1454
        WHEN 341 THEN 1455
        WHEN 362 THEN 1456
        WHEN 381 THEN 1457
        WHEN 382 THEN 1458
        WHEN 401 THEN 1459
        WHEN 461 THEN 1461
        WHEN 462 THEN 1941
        WHEN 464 THEN 1943
        WHEN 476 THEN 1950
        WHEN 478 THEN 1952
        WHEN 491 THEN 117
        WHEN 501 THEN 123
        WHEN 510 THEN 127
        ELSE `mapAreaId`
    END
WHERE `build` = 69893
  AND `mapAreaId` IN (4, 9, 11, 15, 16, 17, 19, 20, 21, 22, 23, 24, 26, 27, 28, 29, 30, 32, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 61, 81, 101, 121, 141, 161, 181, 182, 201, 241, 261, 281, 301, 321, 341, 362, 381, 382, 401, 461, 462, 464, 476, 478, 491, 501, 510);

INSERT INTO `world_db_version` (`LastUpdate`)
SELECT '20261005-04_quest_poi_uimap_ids'
WHERE NOT EXISTS (
    SELECT 1
    FROM `world_db_version`
    WHERE `LastUpdate` = '20261005-04_quest_poi_uimap_ids'
);

SET FOREIGN_KEY_CHECKS = 1;
