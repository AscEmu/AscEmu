DELETE FROM gameobject_spawns WHERE entry = 182210 AND map = 530;

INSERT INTO gameobject_spawns(id, entry, map, position_x, position_y, position_z, orientation) 
VALUES (636620, 182210, 530, -1572.62, 7945.14, -22.2746, 2.74889);

-- updating the Capture Point
DELETE FROM `gameobject_properties`
WHERE `entry` = 182210 AND `build` = 8606;

INSERT INTO `gameobject_properties` (
    `entry`,
    `build`,
    `type`,
    `display_id`,
    `name`,
    `category_name`,
    `cast_bar_text`,
    `UnkStr`,

    `parameter_0`,
    `parameter_1`,
    `parameter_2`,
    `parameter_3`,
    `parameter_4`,
    `parameter_5`,
    `parameter_6`,
    `parameter_7`,
    `parameter_8`,
    `parameter_9`,
    `parameter_10`,
    `parameter_11`,
    `parameter_12`,
    `parameter_13`,
    `parameter_14`,
    `parameter_15`,
    `parameter_16`,
    `parameter_17`,
    `parameter_18`,
    `parameter_19`,
    `parameter_20`,
    `parameter_21`,
    `parameter_22`,
    `parameter_23`,

    `size`,
    `QuestItem1`,
    `QuestItem2`,
    `QuestItem3`,
    `QuestItem4`,
    `QuestItem5`,
    `QuestItem6`
) VALUES (
    182210,
    8606,
    29,                     -- Capture Point
    6834,                   -- Halaa flag model
    'Halaa Banner',
    'PVP',
    '',
    '',

    20,                     -- parameter_0: radius
    0,                      -- parameter_1: dummy
    2673,                   -- parameter_2: Alliance owner
    2672,                   -- parameter_3: Horde owner
    0,                      -- parameter_4: winEventID1
    0,                      -- parameter_5: winEventID2
    2676,                   -- parameter_6: Alliance assaulted
    2677,                   -- parameter_7: Horde assaulted
    0,                      -- parameter_8
    0,                      -- parameter_9
    2671,                   -- parameter_10: Neutral
    0,                      -- parameter_11
    50,                     -- parameter_12: neutralPercent
    0,                      -- parameter_13
    0,                      -- parameter_14
    0,                      -- parameter_15
    0,                      -- parameter_16
    0,                      -- parameter_17
    1,                      -- parameter_18: large
    1,                      -- parameter_19: highlight
    0,
    0,
    0,
    0,

    1.0,                    -- size
    0, 0, 0, 0, 0, 0
);

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('159', '20260101-00_gameobject_spawns');

