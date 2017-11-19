/*
 Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 This file is released under the MIT license. See README-MIT for more information.
 */

#include "Setup.h"

//////////////////////////////////////////////////////////////////////////////////////////
//\details <b>Love is in the Air</b>\n
// event_properties entry: 8
// event_properties holiday: 423
//\todo BossScript for Humme, Baxter and Frye \n

// Hummel (36296)
#define SAY_HUMMEL_1 "Did they bother to tell you who I am and why I am doing this?"
#define SAY_HUMMEL_2 "...or are they just using you like they do everybody else?"
#define SAY_HUMMEL_3 "But what does it matter. It is time for this to end."         // start fighting
#define SAY_HUMMEL_4 "Baxter! Get in there and help! NOW!"                          // by 75% life
#define SAY_HUMMEL_5 "It is time, Frye! Attack!"                                    // by 25% life
#define SAY_HUMMEL_6 "...please don't think less of me."                            // On die

// Baster (36565) Summon by 75% life
#define SAY_BEXTOR "It has been the greatest honor of my life to serve with you, Hummel."   // On die

// Frye (36272) Summon by 25% life
#define SAY_FRYE "Great. We're not gutless, we're incompetent."                             // On die

void SetupLoveIsInTheAir(ScriptMgr* /*mgr*/)
{ }
