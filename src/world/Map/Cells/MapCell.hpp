/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Map/Maps/BaseMap.hpp"

#include <list>
#include <mutex>
#include <set>

class Object;
class WorldMap;

class SERVER_DECL MapCell
{
    friend class WorldMap;

public:
    MapCell() : _x(0), _y(0), _active(false), _loaded(false), _unloadpending(false), _map(nullptr) {};
    ~MapCell();

    typedef std::set<Object*> ObjectSet;

    // Init
    void init(uint32_t x, uint32_t y, WorldMap* map);

    // Object Managing
    void addObject(Object* obj);
    void removeObject(Object* obj);
    bool hasObject(Object* obj) { return (_objects.find(obj) != _objects.end()); }
    bool hasPlayers() const { return _playerCount > 0; }
    bool hasTransporters() const { return _transportCount > 0; }
    inline size_t getObjectCount() { return _objects.size(); }
    void removeObjects();
    inline ObjectSet::iterator Begin() { return _objects.begin(); }
    inline ObjectSet::iterator End() { return _objects.end(); }

    // State Related
    void setActivity(bool state);

    inline bool isActive() { return _active; }
    inline bool isLoaded() { return _loaded; }
    inline void setLoaded(bool Loaded = true) { _loaded = Loaded; }

    // Object Loading Managing
    void loadObjects(CellSpawns* sp);
    inline uint32_t getPlayerCount() { return _playerCount; }

    bool isIdlePending() const;
    void scheduleCellIdleState();
    void cancelPendingIdle();

    inline bool isUnloadPending() { return _unloadpending; }
    inline void setUnloadPending(bool up) { _unloadpending = up; }
    void queueUnloadPending();
    void cancelPendingUnload();
    void unload();
    inline uint16_t getPositionX() { return _x; }
    inline uint16_t getPositionY() { return _y; }

    ObjectSet _respawnObjects;
    ObjectSet::iterator objects_iterator; /// required by MapCell::removeObjects() removing Creatures which will remove their guardians and corrupt itr.

    // the corpse has no more an owner (like if he resurrected) so it can be despawned and
    // the MapCell can be unloaded(if CanUnload() returns true)
    void corpseGoneIdle(Object* corpse);

private:
    uint16_t _x;
    uint16_t _y;
    ObjectSet _objects;
    bool _active;
    bool _loaded;
    bool _unloadpending;
    bool _idlepending = false;

    uint16_t _playerCount = 0;
    uint16_t _transportCount = 0;

    // checks if the MapCell can be unloaded, based on _corpses and if it's in a battleground
    bool canUnload();

    // checks if the MapCell can be unloaded and if so it queues it for unload.
    // this MUST be called when a corpse goes idle
    void checkUnload();

    /// keep track of active corpses so we don't unload a MapCell with an active corpse (otherwise players will not be able to resurrect)
    std::list<Object*> _corpses;

    WorldMap* _map;

    std::mutex m_objectlock;
};
