/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
            uint32_t required_level = gt->req_level;
            uint8_t req_class = gt->req_class;
            uint32_t req_achievement = gt->req_achievement;

            if (required_level > pPlayer->getLevel())
            {
                pPlayer->broadcastMessage("You must be at least level %u to use this portal", required_level);
                return;
            }
            else if (req_class != 0 && req_class != pPlayer->getClass())
            {
                pPlayer->broadcastMessage("You do not have the required class to use this Portal");
                return;
            }
#if VERSION_STRING > TBC
            else if (req_achievement != 0 && pPlayer->getAchievementMgr()->hasCompleted(req_achievement))
            {
                pPlayer->broadcastMessage("You do not have the required achievement to use this Portal");
                return;
            }
#endif
            else
            {
                uint32_t mapid = gt->mapid;
                LocationVector location;
                location.x = gt->x;
                location.y = gt->y;
                location.z = gt->z;
                location.o = gt->o;

                pPlayer->safeTeleport(mapid, 0, location);
            }
        }
    }
    static GameObjectAIScript* Create(GameObject* GO) { return new CustomTeleport(GO); }
};

void InitializeGameObjectTeleportTable(ScriptMgr* mgr)
{
    auto result = WorldDatabase.Query("SELECT entry, mapid, x_pos, y_pos, z_pos, orientation, required_level, required_class, required_achievement FROM gameobject_teleports");
    if (result != NULL)
    {
        // Check if the SQL table is setup correctly
        if (result->GetFieldCount() < 9)
        {
            DLLLogDetail("Error: Custom portals disabled, invalid 'gameobject_teleports' table.");
            return;
        }
        do
        {
            GameobjectTeleport* gt = new GameobjectTeleport;
            Field* fields = result->Fetch();
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
        } while (result->NextRow());
    }
}
