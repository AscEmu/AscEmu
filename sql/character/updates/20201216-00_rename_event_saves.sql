-- Rename table event_properties to gameevent_properties

RENAME TABLE event_save TO gameevent_save;

-- update character_db_version

UPDATE `character_db_version` SET LastUpdate = '20201216-00_rename_event_properties';
