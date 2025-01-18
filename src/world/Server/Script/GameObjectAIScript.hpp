/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <cstdint>

struct ItemProperties;
class Player;
class GameObject;
class InstanceScript;

class SERVER_DECL GameObjectAIScript
{
public:
    GameObjectAIScript(GameObject* _goinstance);
    virtual ~GameObjectAIScript() {}

    virtual void OnCreate() {}
    virtual void OnSpawn() {}
    virtual void OnDespawn() {}
    virtual void OnLootTaken(Player* /*pLooter*/, ItemProperties const* /*pItemInfo*/) {}
    virtual void OnActivate(Player* /*pPlayer*/) {}
    virtual void OnDamaged(uint32_t /*damage*/) {}
    virtual void OnDestroyed() {}
    virtual void AIUpdate() {}
    virtual void Destroy() { delete this; }

    // Data sharing between scripts
    virtual void setGameObjectData(uint32_t /*type*/) {}
    virtual uint32_t getGameObjectData(uint32_t /*type*/) const { return 0; }
    virtual void setGuidData(uint32_t /*guidType*/, uint64_t /*guidData*/) {}
    virtual uint64_t getGuidData(uint32_t /*guidType*/) const { return 0; }

    void RegisterAIUpdateEvent(uint32_t _frequency);
    void ModifyAIUpdateEvent(uint32_t _newfrequency);
    void RemoveAIUpdateEvent();

    //////////////////////////////////////////////////////////////////////////////////////////
    // instance
    InstanceScript* getInstanceScript() const;

    bool isHeroic() const;

    unsigned getRaidModeValue(const unsigned& normal10, const unsigned& normal25, const unsigned& heroic10, const unsigned& heroic25) const;

protected:
    GameObject* _gameobject;
};
