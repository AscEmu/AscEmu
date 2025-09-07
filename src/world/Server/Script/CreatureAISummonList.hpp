/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include <cstdint>
#include <list>

class Creature;

class SERVER_DECL SummonList
{
public:
    typedef std::list<uint64_t> StorageType;
    typedef StorageType::iterator iterator;
    typedef StorageType::const_iterator const_iterator;
    typedef StorageType::size_type size_type;
    typedef StorageType::value_type value_type;

    explicit SummonList(Creature* creature) : _creature(creature) { }

    iterator begin()
    {
        return _storage.begin();
    }

    const_iterator begin() const
    {
        return _storage.begin();
    }

    iterator end()
    {
        return _storage.end();
    }

    const_iterator end() const
    {
        return _storage.end();
    }

    iterator erase(iterator i)
    {
        return _storage.erase(i);
    }

    bool empty() const
    {
        return _storage.empty();
    }

    size_type size() const
    {
        return _storage.size();
    }

    void clear()
    {
        _storage.clear();
    }

    void summon(Creature const* summon);
    void despawn(Creature const* summon);
    void despawnEntry(uint32_t entry);
    void despawnAll();

    template <typename T>
    void despawnIf(T const& predicate)
    {
        _storage.remove_if(predicate);
    }

    void DoActionForEntry(int32_t info, uint32_t entry);

    void removeNotExisting();
    bool hasEntry(uint32_t entry) const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // basic
private:
    void doAction(int32_t action, StorageType const& summons);

    Creature* _creature;
    StorageType _storage;
};
