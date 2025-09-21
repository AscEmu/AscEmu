-- create new table

CREATE TABLE `item_properties_spells` (
  `entry` mediumint unsigned NOT NULL DEFAULT '0',
  `build` smallint NOT NULL DEFAULT '12340',
  `spellid` mediumint NOT NULL DEFAULT '0',
  `spelltrigger` tinyint unsigned NOT NULL DEFAULT '0',
  `spellcharges` smallint NOT NULL DEFAULT '0',
  `spellcooldown` int NOT NULL DEFAULT '-1',
  `spellcategory` smallint unsigned NOT NULL DEFAULT '0',
  `spellcategorycooldown` int NOT NULL DEFAULT '-1',
  UNIQUE KEY `item_properties_spells_entry_IDX` (`entry`,`build`,`spellid`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- move stats to seperated table

REPLACE INTO item_properties_spells (entry, build, spellid, spelltrigger, spellcharges, spellcooldown, spellcategory, spellcategorycooldown)
SELECT entry, build, spellid_1, spelltrigger_1, spellcharges_1, spellcooldown_1, spellcategory_1, spellcategorycooldown_1
FROM item_properties
WHERE spellid_1 <> 0;

REPLACE INTO item_properties_spells (entry, build, spellid, spelltrigger, spellcharges, spellcooldown, spellcategory, spellcategorycooldown)
SELECT entry, build, spellid_2, spelltrigger_2, spellcharges_2, spellcooldown_2, spellcategory_2, spellcategorycooldown_2
FROM item_properties
WHERE spellid_2 <> 0;

REPLACE INTO item_properties_spells (entry, build, spellid, spelltrigger, spellcharges, spellcooldown, spellcategory, spellcategorycooldown)
SELECT entry, build, spellid_3, spelltrigger_3, spellcharges_3, spellcooldown_3, spellcategory_3, spellcategorycooldown_3
FROM item_properties
WHERE spellid_3 <> 0;

REPLACE INTO item_properties_spells (entry, build, spellid, spelltrigger, spellcharges, spellcooldown, spellcategory, spellcategorycooldown)
SELECT entry, build, spellid_4, spelltrigger_4, spellcharges_4, spellcooldown_4, spellcategory_4, spellcategorycooldown_4
FROM item_properties
WHERE spellid_4 <> 0;

REPLACE INTO item_properties_spells (entry, build, spellid, spelltrigger, spellcharges, spellcooldown, spellcategory, spellcategorycooldown)
SELECT entry, build, spellid_5, spelltrigger_5, spellcharges_5, spellcooldown_5, spellcategory_5, spellcategorycooldown_5
FROM item_properties
WHERE spellid_5 <> 0;


ALTER TABLE item_properties
  DROP COLUMN spellid_1,
  DROP COLUMN spelltrigger_1,
  DROP COLUMN spellcharges_1,
  DROP COLUMN spellcooldown_1,
  DROP COLUMN spellcategory_1,
  DROP COLUMN spellcategorycooldown_1,
  DROP COLUMN spellid_2,
  DROP COLUMN spelltrigger_2,
  DROP COLUMN spellcharges_2,
  DROP COLUMN spellcooldown_2,
  DROP COLUMN spellcategory_2,
  DROP COLUMN spellcategorycooldown_2,
  DROP COLUMN spellid_3,
  DROP COLUMN spelltrigger_3,
  DROP COLUMN spellcharges_3,
  DROP COLUMN spellcooldown_3,
  DROP COLUMN spellcategory_3,
  DROP COLUMN spellcategorycooldown_3,
  DROP COLUMN spellid_4,
  DROP COLUMN spelltrigger_4,
  DROP COLUMN spellcharges_4,
  DROP COLUMN spellcooldown_4,
  DROP COLUMN spellcategory_4,
  DROP COLUMN spellcategorycooldown_4,
  DROP COLUMN spellid_5,
  DROP COLUMN spelltrigger_5,
  DROP COLUMN spellcharges_5,
  DROP COLUMN spellcooldown_5,
  DROP COLUMN spellcategory_5,
  DROP COLUMN spellcategorycooldown_5;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('153', '20250921-01_item_properties_spells');
