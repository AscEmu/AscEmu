/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Event_Nights.h"

#include "Setup.h"
#include "Server/Script/EventScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Nights
// Hande the nights
// event_properties entry: 25

class HandleNights : public EventScript
{

};

void SetupHandleNights(ScriptMgr* mgr)
{
    mgr->register_event_script(25, new HandleNights);
}