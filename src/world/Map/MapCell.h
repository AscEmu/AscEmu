/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Map/Map.h"

class Object;
class Map;

class SERVER_DECL MapCell
{
friend class MapMgr;

public:
    MapCell() : _x(0), _y(0), _active(false), _loaded(false), _unloadpending(false), _playerCount(0), _mapmgr(nullptr) {};
    ~MapCell();

    typedef std::set<Object*> ObjectSet;

    // Init
    void Init(uint32_t x, uint32_t y, MapMgr* mapmgr);

    // Object Managing
    void AddObject(Object* obj);
    void RemoveObject(Object* obj);
    bool HasObject(Object* obj) { return (_objects.find(obj) != _objects.end()); }
    bool HasPlayers() { return ((_playerCount > 0) ? true : false); }
    inline size_t GetObjectCount() { return _objects.size(); }
    void RemoveObjects();
    inline ObjectSet::iterator Begin() { return _objects.begin(); }
    inline ObjectSet::iterator End() { return _objects.end(); }

    // State Related
    void SetActivity(bool state);

    inline bool IsActive() { return _active; }
    inline bool IsLoaded() { return _loaded; }
    inline void SetLoaded(bool Loaded = true) { _loaded = Loaded; }

    // Object Loading Managing
    void LoadObjects(CellSpawns* sp);
    inline uint32_t GetPlayerCount() { return _playerCount; }

    inline bool IsUnloadPending() { return _unloadpending; }
    inline void SetUnloadPending(bool up) { _unloadpending = up; }
    void QueueUnloadPending();
    void CancelPendingUnload();
    void Unload();
    /// xyz 
    inline uint16_t GetPositionX() { return _x; }
    inline uint16_t GetPositionY() { return _y; }
    // inline uint16_t GetPositionZ() { return _z; }

    ObjectSet _respawnObjects;
    ObjectSet::iterator objects_iterator; /// required by MapCell::RemoveObjects() removing Creatures which will remove their guardians and corrupt itr.

    /// the corpse has no more an owner (like if he resurrected) so it can be despawned and
    /// the MapCell can be unloaded(if CanUnload() returns true)
    void CorpseGoneIdle(Object* corpse);

private:
    uint16_t _x;
    uint16_t _y;
    // uint16_t _z;
    ObjectSet _objects;
    bool _active;
    bool _loaded;
    bool _unloadpending;

    uint16_t _playerCount;

    /// checks if the MapCell can be unloaded, based on _corpses and if it's in a battleground
    bool CanUnload();
    /// checks if the MapCell can be unloaded and if so it queues it for unload.
    /// this MUST be called when a corpse goes idle
    void CheckUnload();

    /// keep track of active corpses so we don't unload a MapCell with an active corpse (otherwise players will not be able to resurrect)
    std::list<Object*> _corpses;

    MapMgr* _mapmgr;

    Mutex m_objectlock;
};
