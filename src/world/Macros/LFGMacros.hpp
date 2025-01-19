/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

///
//#define MAX_LFG_QUEUE_ID 3

///
//#define LFG_MATCH_TIMEOUT 30 // in seconds

//////////////////////////////////////////////////////////////////////////////////////////
// MAX_DUNGEONS
//
// \param Max dungeons (Max entry's +1 on lfgdungeons.dbc)
//
// Vanilla = 165+1
// The Burning Crusade = 201+1
// Wrath of the Lich King = 294+1
// Cataclysm = 448+1
// Mists of Pandaria = untested+1
// Warlords of Draenor = untested+1
// Legion = untested+1
//
//////////////////////////////////////////////////////////////////////////////////////////

#if VERSION_STRING == Classic
    #define MAX_DUNGEONS 166    // Classic WoW (untested)
#elif VERSION_STRING == TBC
    #define MAX_DUNGEONS 202    // The Burning Crusade
#elif VERSION_STRING == WotLK
    #define MAX_DUNGEONS 295    // Wrath of the Lich King
#elif VERSION_STRING == Cata
    #define MAX_DUNGEONS 449    // Cataclysm
#elif VERSION_STRING == Mop
    #define MAX_DUNGEONS 449    //  Mists of Pandaria (untested)
#endif
