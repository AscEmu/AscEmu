
-- Rename poi column and add on choose actions

ALTER TABLE `gossip_menu_items`
CHANGE COLUMN `point_of_interest` `on_choose_data` INT(10) NOT NULL DEFAULT 0;

ALTER TABLE `gossip_menu_items`
ADD COLUMN `on_choose_action` int(6) NOT NULL DEFAULT 0 AFTER `icon`;

-- Update existing poi with type 1

UPDATE `gossip_menu_items` SET `on_choose_action`=1 WHERE `on_choose_data`<>0;

-- update world_db_version

INSERT INTO `world_db_version` VALUES ('53', '20200428-00_gossips_actions');
