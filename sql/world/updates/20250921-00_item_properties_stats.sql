-- create new table

CREATE TABLE `item_properties_stats` (
  `entry` mediumint unsigned NOT NULL DEFAULT '0',
  `build` smallint NOT NULL DEFAULT '12340',
  `type` tinyint unsigned NOT NULL DEFAULT '0',
  `value` smallint NOT NULL DEFAULT '0',
  UNIQUE KEY `item_properties_stats_entry_IDX` (`entry`,`build`,`type`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- move stats to seperated table

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, stat_type1, stat_value1
FROM item_properties
WHERE stat_type1 <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, stat_type2, stat_value2
FROM item_properties
WHERE stat_type2 <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, stat_type3, stat_value3
FROM item_properties
WHERE stat_type3 <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, stat_type4, stat_value4
FROM item_properties
WHERE stat_type4 <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, stat_type5, stat_value5
FROM item_properties
WHERE stat_type5 <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, stat_type6, stat_value6
FROM item_properties
WHERE stat_type6 <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, stat_type7, stat_value7
FROM item_properties
WHERE stat_type7 <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, stat_type8, stat_value8
FROM item_properties
WHERE stat_type8 <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, stat_type9, stat_value9
FROM item_properties
WHERE stat_type9 <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, stat_type10, stat_value10
FROM item_properties
WHERE stat_type10 <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, 53, holy_res
FROM item_properties
WHERE holy_res <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, 55, fire_res
FROM item_properties
WHERE fire_res <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, 52, nature_res
FROM item_properties
WHERE nature_res <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, 54, frost_res
FROM item_properties
WHERE frost_res <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, 56, shadow_res
FROM item_properties
WHERE shadow_res <> 0;

REPLACE INTO item_properties_stats (entry, build, type, value)
SELECT entry, build, 51, arcane_res
FROM item_properties
WHERE arcane_res <> 0;


ALTER TABLE item_properties
  DROP COLUMN itemstatscount,
  DROP COLUMN stat_type1,
  DROP COLUMN stat_value1,
  DROP COLUMN stat_type2,
  DROP COLUMN stat_value2,
  DROP COLUMN stat_type3,
  DROP COLUMN stat_value3,
  DROP COLUMN stat_type4,
  DROP COLUMN stat_value4,
  DROP COLUMN stat_type5,
  DROP COLUMN stat_value5,
  DROP COLUMN stat_type6,
  DROP COLUMN stat_value6,
  DROP COLUMN stat_type7,
  DROP COLUMN stat_value7,
  DROP COLUMN stat_type8,
  DROP COLUMN stat_value8,
  DROP COLUMN stat_type9,
  DROP COLUMN stat_value9,
  DROP COLUMN stat_type10,
  DROP COLUMN stat_value10,
  DROP COLUMN holy_res,
  DROP COLUMN fire_res,
  DROP COLUMN nature_res,
  DROP COLUMN frost_res,
  DROP COLUMN shadow_res,
  DROP COLUMN arcane_res;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('152', '20250921-00_item_properties_stats');
