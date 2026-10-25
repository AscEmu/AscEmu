-- trainer_properties: primary key (entry, build) for the per build load, the keyed copy drops the duplicate rows
CREATE TABLE `trainer_properties_keyed` LIKE `trainer_properties`;
ALTER TABLE `trainer_properties_keyed` MODIFY `build` int NOT NULL DEFAULT '12340', ADD PRIMARY KEY (`entry`, `build`);
INSERT IGNORE INTO `trainer_properties_keyed` SELECT * FROM `trainer_properties`;
DROP TABLE `trainer_properties`;
RENAME TABLE `trainer_properties_keyed` TO `trainer_properties`;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('267', '20261025-00_trainer_properties_key');
