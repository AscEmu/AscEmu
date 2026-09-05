-- is_currency=1: itemid column holds a CurrencyTypes.dbc id instead of an item entry.
-- No AFTER clause - always the last column, read positionally via getFieldCount()-1.
ALTER TABLE `loot_creatures` ADD COLUMN `is_currency` tinyint unsigned NOT NULL DEFAULT '0';
ALTER TABLE `loot_gameobjects` ADD COLUMN `is_currency` tinyint unsigned NOT NULL DEFAULT '0';
ALTER TABLE `loot_items` ADD COLUMN `is_currency` tinyint unsigned NOT NULL DEFAULT '0';
ALTER TABLE `loot_skinning` ADD COLUMN `is_currency` tinyint unsigned NOT NULL DEFAULT '0';
ALTER TABLE `loot_fishing` ADD COLUMN `is_currency` tinyint unsigned NOT NULL DEFAULT '0';
ALTER TABLE `loot_pickpocketing` ADD COLUMN `is_currency` tinyint unsigned NOT NULL DEFAULT '0';

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('192', '20260905-08_loot_currency_column');
