-- ----------------------------
-- Change columns size/names gameobject_names
-- ----------------------------
ALTER TABLE `gameobject_names` CHANGE `Type` `type` TINYINT(3) unsigned NOT NULL COMMENT 'Type of this go. Fill in all necessary parameters';
ALTER TABLE `gameobject_names` CHANGE `DisplayID` `display_id` MEDIUMINT(8) unsigned NOT NULL COMMENT 'Go visible display id';
ALTER TABLE `gameobject_names` CHANGE `Name` `name` VARCHAR(100) NOT NULL DEFAULT '';
ALTER TABLE `gameobject_names` CHANGE `Category` `category_name` VARCHAR(100) NOT NULL DEFAULT '';
ALTER TABLE `gameobject_names` CHANGE `CastBarText` `cast_bar_text` VARCHAR(100) NOT NULL DEFAULT '';
ALTER TABLE `gameobject_names` CHANGE `spellfocus` `parameter_0` INT(10) unsigned NOT NULL DEFAULT '0';
ALTER TABLE `gameobject_names` CHANGE `sound1` `parameter_1` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `sound2` `parameter_2` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `sound3` `parameter_3` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `sound4` `parameter_4` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `sound5` `parameter_5` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `sound6` `parameter_6` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `sound7` `parameter_7` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `sound8` `parameter_8` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `sound9` `parameter_9` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown1` `parameter_10` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown2` `parameter_11` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown3` `parameter_12` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown4` `parameter_13` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown5` `parameter_14` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown6` `parameter_15` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown7` `parameter_16` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown8` `parameter_17` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown9` `parameter_18` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown10` `parameter_19` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown11` `parameter_20` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown12` `parameter_21` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown13` `parameter_22` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `unknown14` `parameter_23` INT(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Parameter for type';
ALTER TABLE `gameobject_names` CHANGE `Size` `size` FLOAT(0) unsigned NOT NULL DEFAULT '1' COMMENT 'Default size for this gameobject';


UPDATE `world_db_version` SET `LastUpdate` = '2015-09-04_01_gameobject_names' WHERE `LastUpdate` = '2015-09-02_01_transport_creatures';
  