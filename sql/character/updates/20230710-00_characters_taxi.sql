ALTER TABLE characters
MODIFY COLUMN taxi_path longtext;

ALTER TABLE characters
DROP COLUMN taxi_lastnode;

UPDATE `character_db_version` SET LastUpdate = '20230710-00_characters_taxi';
