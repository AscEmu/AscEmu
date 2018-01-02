/*
 Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 This file is released under the MIT license. See README-MIT for more information.
 */

#include "Setup.h"

//////////////////////////////////////////////////////////////////////////////////////////
//\details <b>Edge of Madness, Gri'lek</b>\n
// event_properties entry: 27 \n
// event_properties holiday: 0 \n
// npc_id: 15082 \n
// What the hell... https://www.youtube.com/watch?v=-kQMJ2M30PA \n
//\todo Monsterscript for Grilek

//////////////////////////////////////////////////////////////////////////////////////////
//\details <b>Edge of Madness, Hazza'rah</b>\n
// event_properties entry: 28 \n
// event_properties holiday: 0 \n
//
// Hazza'rah entry: 15083 \n
// Skill "Archaeology (794) 255 required to summon the boss! \n
// Use GOB 180327 (Brazier of Madness) to summon. \n
// http://www.wowhead.com/npc=52271 \n
//\todo Monsterscript for Hazza'rah

const uint32 SPELL_MANABURN = 26046;
const uint32 SPELL_SLEEP = 24664;

const uint32 SPELL_SUMMON_NIGHTMARE_ILLUSIONS = 24728;

#define YELL_HAZZARAH_1 "Today, you'll wish you never stirred from your bed!"                                       ///on start attacking
#define YELL_HAZZARAH_2 "Slumber... another dream awaits you..."                                                    /// on casting spell xxx??
#define YELL_HAZZARAH_3 "My spirit sleeps no longer..."                                                             /// on casting spell xxx??
#define YELL_HAZZARAH_4 "Let's see what's more horrorfying... To dream onwalking, or awaken to a living nightmare"  /// by spawning nightmare illusions (npc_id: 15163)
#define YELL_HAZZARAH_5 "Let us see what horrors stir within your nightmares!"                                      /// by spawning nightmare illusions (npc_id: 15163)

//////////////////////////////////////////////////////////////////////////////////////////
//\details <b>Edge of Madness, Renataki</b>\n
// event_properties entry: 29 \n
// event_properties holiday: 0 \n
// npc_id: 15084 \n
//\todo Monsterscript for Rentaki

const uint32 SPELL_THOUSANDBLADES = 34799;
const uint32 SPELL_AMBUSH = 34794;

//////////////////////////////////////////////////////////////////////////////////////////
//\details <b>Edge of Madness, Wushoolay</b>\n
// event_properties entry: 30 \n
// event_properties holiday: 0 \n
// npc_id: 15085 \n
// First you must find four Artifacts to interact with them (Archeologie) with an Gossip MenuItem \n
// GOSSIP_MENU_ITEM "Examine the object." 
//\todo Monsterscript for Wushoolay

#define YELL_WUSHOOLAY_1 "You shall regret disturbing my homeland!"                     /// on start attacking
#define YELL_WUSHOOLAY_2 "The storm feeds my power!"                                    /// on casting spell xxx??
#define YELL_WUSHOOLAY_3 "In a flash, you'll be nothing more than dust..."              /// on casting spell xxx??
#define YELL_WUSHOOLAY_4 "Can you feel that tingling sensation? It may be your last."   /// on casting spell xxx??
#define YELL_WUSHOOLAY_5 "Shocking. I know"                                             /// If a player dies
#define YELL_WUSHOOLAY_6 "Are you quicker than lightning?"                              /// on casting spell xxx??
#define YELL_WUSHOOLAY_7 "The end of our empire... is only the beginning..."            /// on die


void SetupEdgeOfMadnes(ScriptMgr* /*mgr*/)
{ }