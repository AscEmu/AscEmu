/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ObjectRegistry.h"

//////////////////////////////////////////////////////////////////////////////////////////
// FactoryHolder holds a factory object of a specific type
template<class T, class O, class Key = std::string>
class FactoryHolder
{
public:
    typedef ObjectRegistry<FactoryHolder<T, O, Key>, Key> FactoryHolderRegistry;

    explicit FactoryHolder(Key const& k) : _key(k) { }
    virtual ~FactoryHolder() { }

    void registerSelf() { FactoryHolderRegistry::getInstance()->insertItem(this, _key); }

    /// Abstract Factory create method
    virtual T* create(O* object = nullptr) const = 0;

private:
    Key const _key;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Permissible is a classic way of letting the object decide
// whether how good they handle things.  This is not retricted to factory selectors.
template<class T>
class Permissible
{
public:
    virtual ~Permissible() { }
    virtual int32_t permit(T const*) const = 0;
};
