-- Rename table event_properties to gameevent_properties

RENAME TABLE event_properties TO gameevent_properties;

-- update world_db_version

REPLACE INTO `world_db_version` VALUES ('63', '20201216-00_rename_event_properties');
