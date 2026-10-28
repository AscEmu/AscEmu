/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Management/AchievementMgr.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/Master.h"
#include "Server/Script/GameObjectAIScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
//\brief This script will add support for an SQL table called gameobject_teleports.
// This table can be used to teleport players when they use a game object such
// as a door or portal. Any object used in this table should exist in the
// gameobject_properties table, and be of type 10. Custom portal can use the generic
// display id of 6831. Portals also have the parameter_1 field set to '1'.
//////////////////////////////////////////////////////////////////////////////////////////

struct GameobjectTeleport
{
    uint32_t mapid;
    float x, y, z, o;
    uint32_t req_level;
    uint8_t req_class;
    uint32_t req_achievement;
};

std::map<uint32_t, GameobjectTeleport*> m_teleStorage;

class CustomTeleport : public GameObjectAIScript // Custom Portals
{
public:
    explicit CustomTeleport(GameObject* goinstance) : GameObjectAIScript(goinstance) {}

    ~CustomTeleport() override
    {}

    void OnActivate(Player* pPlayer) override
    {
        std::map<uint32_t, GameobjectTeleport*>::iterator itr = m_teleStorage.find(this->_gameobject->getEntry());
        if (itr != m_teleStorage.end())
        {
            GameobjectTeleport* gt = itr->second;
            uint32_t requiredLevel = gt->req_level;
            uint8_t requiredClass = gt->req_class;
            uint32_t requiredAchievement = gt->req_achievement;

            if (requiredLevel > pPlayer->getLevel())
            {
                pPlayer->broadcastMessage("You must be at least level {} to use this portal", requiredLevel);
                return;
            }

            if (requiredClass != 0 && requiredClass != pPlayer->getClass())
            {
                pPlayer->broadcastMessage("You do not have the required class to use this Portal");
                return;
            }

            // Achievements since WotLK
            if (WoW::isServerExpansionAtLeast(WoW::Expansion::_WotLK) && requiredAchievement != 0)
            {
                auto* achiMgr = pPlayer->getAchievementMgr();
                if (achiMgr == nullptr || !achiMgr->hasCompleted(requiredAchievement))
                {
                    pPlayer->broadcastMessage("You do not have the required achievement to use this Portal");
                    return;
                }
            }

            pPlayer->safeTeleport(gt->mapid, 0, LocationVector(gt->x, gt->y, gt->z, gt->o));
        }
    }

    static GameObjectAIScript* Create(GameObject* GO) { return new CustomTeleport(GO); }
};

void InitializeGameObjectTeleportTable(ScriptMgr* mgr)
{
    auto result = WorldDatabase.query("SELECT entry, mapid, x_pos, y_pos, z_pos, orientation, required_level, required_class, required_achievement FROM gameobject_teleports");
    if (result != nullptr)
    {
        // Check if the SQL table is set up correctly
        if (result->getFieldCount() < 9)
        {
            DLLLogDetail("Error: Custom portals disabled, invalid 'gameobject_teleports' table.");
            return;
        }
        do
        {
            GameobjectTeleport* gt = new GameobjectTeleport;
            Field* fields = result->fetch();
            uint32_t entry = fields[0].asUint32();
            gt->mapid = fields[1].asUint32();
            gt->x = fields[2].asFloat();
            gt->y = fields[3].asFloat();
            gt->z = fields[4].asFloat();
            gt->o = fields[5].asFloat();
            gt->req_level = fields[6].asUint32();
            gt->req_class = fields[7].asUint8();
            gt->req_achievement = fields[8].asUint32();
            m_teleStorage[entry] = gt;
            mgr->register_gameobject_script(entry, &CustomTeleport::Create);
        } while (result->nextRow());
    }
}
