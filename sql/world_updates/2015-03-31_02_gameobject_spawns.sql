/*
********************************************************************
Updates for gameobject_spawns
********************************************************************
*/
DELETE FROM `gameobject_spawns` WHERE `id` = 141713;

INSERT INTO gameobject_spawns
   (`id`, `Entry`, `map`, `position_x`, `position_y`, `position_z`, `Facing`, `orientation1`, `orientation2`, `orientation3`, `orientation4`, `State`, `Flags`, `Faction`, `Scale`, `stateNpcLink`, `phase`, `overrides`)
VALUES
   (141713, 184225, 530, 2862.44, 1546.04, 250.16, 3.89, 0.00, 0.00, 0.93, -0.36, 7937, 0, 0, 3.06, 0, 1, 0);

