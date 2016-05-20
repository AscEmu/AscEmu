/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef MYSQL_DATA_LOADS_HPP
#define MYSQL_DATA_LOADS_HPP

#include "Singleton.h"

class SERVER_DECL MySQLDataStore : public Singleton < MySQLDataStore >
{
public:

    MySQLDataStore();
    ~MySQLDataStore();

    //maps
    typedef std::unordered_map<uint32, ItemPrototype> ItemPrototypeContainer;
    typedef std::unordered_map<uint32, CreatureInfo> CreatureInfoContainer;
    typedef std::unordered_map<uint32, CreatureProto> CreatureProtoContainer;
    typedef std::unordered_map<uint32, GameObjectInfo> GameObjectNamesContainer;
    typedef std::unordered_map<uint32, Quest> QuestContainer;

    //helper
    ItemPrototype const* GetItemProto(uint32 entry);
    ItemPrototypeContainer const* GetItemPrototypeStore() { return &_itemPrototypeStore; }

    CreatureInfo const* GetCreatureInfo(uint32 entry);
    CreatureInfoContainer const* GetCreatureNamesStore() { return &_creatureNamesStore; }
    CreatureProto const* GetCreatureProto(uint32 entry);
    CreatureProtoContainer const* GetCreatureProtoStore() { return &_creatureProtoStore; }

    GameObjectInfo const* GetGameObjectInfo(uint32 entry);
    GameObjectNamesContainer const* GetGameObjectNamesStore() { return &_gameobjectNamesStore; }

    Quest const* GetQuest(uint32 entry);
    QuestContainer const* GetQuestStore() { return &_questStore; }

    //Loads
    void LoadItemsTable();
    void LoadCreatureNamesTable();
    void LoadCreatureProtoTable();
    void LoadGameObjectNamesTable();
    void LoadQuestsTable();

    void LoadGameObjectQuestItemBindingTable();
    void LoadGameObjectQuestPickupBindingTable();


    ItemPrototypeContainer _itemPrototypeStore;
    CreatureInfoContainer _creatureNamesStore;
    CreatureProtoContainer _creatureProtoStore;
    GameObjectNamesContainer _gameobjectNamesStore;
    QuestContainer _questStore;

};

#define sMySQLStore MySQLDataStore::getSingleton()

#endif MYSQL_DATA_LOADS_HPP
