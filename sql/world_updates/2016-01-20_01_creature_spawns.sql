-- Delete spawns since nexus scripts handle it issue #220.
DELETE FROM 'creature_spawns' WHERE 'entry' in
(27949, 26796, 27947, 26798, 26805, 26802, 26800, 26803, 26801, 26799);