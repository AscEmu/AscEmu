-- Merge two similiar mod immunities in modImmunities column
-- Add missing 1024 to creatures that had 8192 but not 1024 (only 2 rows affected)
UPDATE creature_properties SET modImmunities=(modImmunities | 1024) WHERE modImmunities & 8192 AND NOT modImmunities & 1024;
-- Remove 8192 from creatures that had it set
UPDATE creature_properties SET modImmunities=(modImmunities & ~8192) WHERE modImmunities & 8192;

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('101', '20220116-02_creature_properties');
