/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Units/Creatures/AIInterface.h"
#include "Management/Item.h"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include <Management/QuestLogEntry.hpp>
#include "Map/MapScriptInterface.h"
#include <Spell/Customization/SpellCustomizations.hpp>


///////////////////////////////////////////////////////
//Quest: Rite of Vision
//

// Plain Visions Encounter
static Movement::Location WaypointPlainVision[] =
{
    {},
    { -2240.521729f, -407.114532f, -9.424648f, 5.753043f }, //1
    { -2225.764404f, -419.251404f, -9.360950f, 5.575544f },
    { -2200.883789f, -440.999634f, -5.606637f, 5.599899f },
    { -2143.712646f, -468.068665f, -9.401191f, 0.797177f },
    { -2100.811279f, -420.980194f, -5.322747f, 0.918914f },
    { -2079.469482f, -392.465576f, -10.262321f, 0.930689f },
    { -2043.699707f, -343.802155f, -6.971242f, 0.934694f },
    { -2001.858521f, -242.533966f, -10.763200f, 0.952265f },
    { -1924.751343f, -119.969292f, -11.770116f, 0.754775f },
    { -1794.804565f, -7.919222f, -9.326016f, 0.711578f },
    { -1755.206421f, 72.430847f, 1.121445f, 1.103484f },
    { -1734.550049f, 116.842003f, -4.337420f, 3.350100f },
    { -1720.041870f, 125.933235f, -2.325089f, 0.903206f },
    { -1704.406860f, 183.596405f, 12.065074f, 0.691148f },
    { -1674.317261f, 201.597565f, 11.235887f, 0.475164f },
    { -1624.068726f, 223.555389f, 2.074259f, 0.416259f },
    { -1572.863403f, 234.714813f, 2.305119f, 0.781471f },
    { -1542.866943f, 277.896759f, 20.543165f, 1.256637f },
    { -1541.813232f, 316.415649f, 49.910889f, 1.248783f },
    { -1526.979126f, 329.664001f, 61.835552f, 1.182024f },
    { -1524.173584f, 335.237610f, 63.325703f, 1.173092f },
    { -1513.968506f, 355.759338f, 63.064487f, 1.119193f } //22
};

class The_Plains_Vision : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(The_Plains_Vision);
    The_Plains_Vision(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        WPCount = 22;
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);

        for (uint8 i = 1; i < WPCount; ++i)
            AddWaypoint(CreateWaypoint(i, 0, Movement::WP_MOVE_TYPE_RUN, WaypointPlainVision[i]));
    }

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        if (iWaypointId == 1)
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You follow me.");
        if (iWaypointId == 22)
        {

            getCreature()->DeleteWaypoints();
            getCreature()->Despawn(500, 0);
        }
    }

    uint8 WPCount;
};


void SetupMulgore(ScriptMgr* mgr)
{
    mgr->register_creature_script(2983, &The_Plains_Vision::Create);
}
