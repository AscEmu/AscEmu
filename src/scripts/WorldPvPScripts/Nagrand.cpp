/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Script/ScriptMgr.hpp"
#include "Server/Script/HookInterfaceDefines.hpp"
#include "Server/Script/GameObjectAIScript.hpp"

#include "Objects/Units/Players/Player.hpp"
#include "Objects/GameObject.h"

#include <iostream>
#include <cstdint>

////////////////////////////////////////////////////////////////////////////////
// zone
constexpr uint32_t NAGRAND_ZONE_ID = 3519;

// gameobject
constexpr uint32_t NAGRAND_GO_BANNER = 182210;

////////////////////////////////////////////////////////////////////////////////
// click
class NagrandBannerGO : public GameObjectAIScript
{
public:
    explicit NagrandBannerGO(GameObject* go) : GameObjectAIScript(go) {}
    static GameObjectAIScript* Create(GameObject* go) { return new NagrandBannerGO(go); }

    void OnActivate(Player* player) override
    {
        if (!player)
            return;

        if (player->getZoneId() != NAGRAND_ZONE_ID)
            return;

        // DEBUG
        std::cout << "[NAGRAND DEBUG] Nagrand banner clicked by player GUID="
                  << player->getGuid() << std::endl;
        // DEBUG
        
        player->sendAreaTriggerMessage("Nagrand banner clicked!");
    }
};

////////////////////////////////////////////////////////////////////////////////
// hooks
void Nagrand_OnEnterWorld(Player* /*player*/)
{
}

void Nagrand_OnZoneChange(Player* /*player*/, uint32_t /*newZone*/, uint32_t /*oldZone*/)
{
}

////////////////////////////////////////////////////////////////////////////////
// setup
void SetupNagrand(ScriptMgr* mgr)
{
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ENTER_WORLD, (void*)&Nagrand_OnEnterWorld);
    mgr->register_hook(SERVER_HOOK_EVENT_ON_ZONE, (void*)&Nagrand_OnZoneChange);
    mgr->register_gameobject_script(NAGRAND_GO_BANNER, &NagrandBannerGO::Create);
}

