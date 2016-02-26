--
-- Added missing gameobject_names data
--
REPLACE INTO gameobject_names
   (`entry`, `type`, `display_id`, `name`, `category_name`, `cast_bar_text`, `UnkStr`, `parameter_0`, `parameter_1`, `parameter_2`, `parameter_3`, `parameter_4`, `parameter_5`, `parameter_6`, `parameter_7`, `parameter_8`, `parameter_9`, `parameter_10`, `parameter_11`, `parameter_12`, `parameter_13`, `parameter_14`, `parameter_15`, `parameter_16`, `parameter_17`, `parameter_18`, `parameter_19`, `parameter_20`, `parameter_21`, `parameter_22`, `parameter_23`, `size`, `QuestItem1`, `QuestItem2`, `QuestItem3`, `QuestItem4`, `QuestItem5`, `QuestItem6`)
VALUES
   (193993, 8, 2770, 'Corpse of the Fallen Worg Spell Focus', '', '', '', 1592, 15, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0);

--
-- Update world db version
--
UPDATE `world_db_version` SET `LastUpdate` = '2016-02-26_01_gameobject_names' WHERE `LastUpdate` = '2016-02-22_01_new_transport_creatures';
