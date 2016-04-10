/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
* Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
* Copyright (C) 2008-2009 Sun++ Team <http://www.sunplusplus.info/>
* Copyright (C) 2008 WEmu Team
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Setup.h"
#include "../Common/EasyFunctions.h"


///////////////////////////////////////////////////////
//Quest: Rite of Vision
//

// Plain Visions Encounter
static Movement::LocationWithFlag WaypointPlainVision[] =
{
    {},
    { -2240.521729f, -407.114532f, -9.424648f, 5.753043f, Movement::WP_MOVE_TYPE_RUN }, //1
    { -2225.764404f, -419.251404f, -9.360950f, 5.575544f, Movement::WP_MOVE_TYPE_RUN },
    { -2200.883789f, -440.999634f, -5.606637f, 5.599899f, Movement::WP_MOVE_TYPE_RUN },
    { -2143.712646f, -468.068665f, -9.401191f, 0.797177f, Movement::WP_MOVE_TYPE_RUN },
    { -2100.811279f, -420.980194f, -5.322747f, 0.918914f, Movement::WP_MOVE_TYPE_RUN },
    { -2079.469482f, -392.465576f, -10.262321f, 0.930689f, Movement::WP_MOVE_TYPE_RUN },
    { -2043.699707f, -343.802155f, -6.971242f, 0.934694f, Movement::WP_MOVE_TYPE_RUN },
    { -2001.858521f, -242.533966f, -10.763200f, 0.952265f, Movement::WP_MOVE_TYPE_RUN },
    { -1924.751343f, -119.969292f, -11.770116f, 0.754775f, Movement::WP_MOVE_TYPE_RUN },
    { -1794.804565f, -7.919222f, -9.326016f, 0.711578f, Movement::WP_MOVE_TYPE_RUN },
    { -1755.206421f, 72.430847f, 1.121445f, 1.103484f, Movement::WP_MOVE_TYPE_RUN },
    { -1734.550049f, 116.842003f, -4.337420f, 3.350100f, Movement::WP_MOVE_TYPE_RUN },
    { -1720.041870f, 125.933235f, -2.325089f, 0.903206f, Movement::WP_MOVE_TYPE_RUN },
    { -1704.406860f, 183.596405f, 12.065074f, 0.691148f, Movement::WP_MOVE_TYPE_RUN },
    { -1674.317261f, 201.597565f, 11.235887f, 0.475164f, Movement::WP_MOVE_TYPE_RUN },
    { -1624.068726f, 223.555389f, 2.074259f, 0.416259f, Movement::WP_MOVE_TYPE_RUN },
    { -1572.863403f, 234.714813f, 2.305119f, 0.781471f, Movement::WP_MOVE_TYPE_RUN },
    { -1542.866943f, 277.896759f, 20.543165f, 1.256637f, Movement::WP_MOVE_TYPE_RUN },
    { -1541.813232f, 316.415649f, 49.910889f, 1.248783f, Movement::WP_MOVE_TYPE_RUN },
    { -1526.979126f, 329.664001f, 61.835552f, 1.182024f, Movement::WP_MOVE_TYPE_RUN },
    { -1524.173584f, 335.237610f, 63.325703f, 1.173092f, Movement::WP_MOVE_TYPE_RUN },
    { -1513.968506f, 355.759338f, 63.064487f, 1.119193f, Movement::WP_MOVE_TYPE_RUN } //22
};

class The_Plains_Vision : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(The_Plains_Vision, MoonScriptCreatureAI);
        The_Plains_Vision(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            WPCount = 22;
            WayPoints = WaypointPlainVision;
            _unit->GetAIInterface()->SetAllowedToEnterCombat(false);


            for (uint8 i = 1; i < WPCount; ++i)
            {
                AddWaypoint(CreateWaypoint(i, 0, WayPoints[i]));
            }
        }

        void OnReachWP(uint32 iWaypointId, bool bForwards)
        {
            if(iWaypointId == 1)
                _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You follow me.");
            if(iWaypointId == 22)
            {

                sEAS.DeleteWaypoints(_unit);
                _unit->Despawn(500, 0);
            }
        }

        uint8 WPCount;
        Movement::LocationWithFlag* WayPoints;
};

void SetupMulgore(ScriptMgr* mgr)
{
    mgr->register_creature_script(2983, &The_Plains_Vision::Create);
}
