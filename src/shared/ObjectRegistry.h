/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

//////////////////////////////////////////////////////////////////////////////////////////
// ObjectRegistry holds all registry item of the same type
template<class T, class Key = std::string>
class ObjectRegistry final
{
private:
    ObjectRegistry() = default;
    ~ObjectRegistry() = default;

public:
    typedef std::map<Key, std::unique_ptr<T>> RegistryMapType;

    ObjectRegistry(ObjectRegistry&&) = delete;
    ObjectRegistry(ObjectRegistry const&) = delete;
    ObjectRegistry& operator=(ObjectRegistry&&) = delete;
    ObjectRegistry& operator=(ObjectRegistry const&) = delete;

    static ObjectRegistry<T, Key>* getInstance()
    {
        static ObjectRegistry<T, Key> instance;
        return &instance;
    }

    /// Returns a registry item
    T const* getRegistryItem(Key const& key) const
    {
        auto itr = _registeredObjects.find(key);
        if (itr == _registeredObjects.end())
            return nullptr;
        return itr->second.get();
    }

    /// Inserts a registry item
    bool insertItem(T* obj, Key const& key, bool force = false)
    {
        auto itr = _registeredObjects.find(key);
        if (itr != _registeredObjects.end())
        {
            if (!force)
                return false;
            _registeredObjects.erase(itr);
        }

        _registeredObjects.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(obj));
        return true;
    }

    /// Returns true if registry contains an item
    bool hasItem(Key const& key) const
    {
        return (_registeredObjects.count(key) > 0);
    }

    /// Return the map of registered items
    RegistryMapType const& getRegisteredItems() const
    {
        return _registeredObjects;
    }

private:
    RegistryMapType _registeredObjects;
};
