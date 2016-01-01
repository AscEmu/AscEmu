/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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

//////////////////////////////////////////////////////////////////////////////////////////
///\details <b>Edge of Madness, Gri'lek</b>\n
/// event_names entry: 27 \n
/// event_names holiday: 0 \n
/// npc_id: 15082 \n
/// What the hell... https://www.youtube.com/watch?v=-kQMJ2M30PA \n
///\todo Monsterscript for Grilek

//////////////////////////////////////////////////////////////////////////////////////////
///\details <b>Edge of Madness, Hazza'rah</b>\n
/// event_names entry: 28 \n
/// event_names holiday: 0 \n
///
/// Hazza'rah entry: 15083 \n
/// Skill "Archaeology (794) 255 required to summon the boss! \n
/// Use GOB 180327 (Brazier of Madness) to summon. \n
/// http://www.wowhead.com/npc=52271 \n
///\todo Monsterscript for Hazza'rah

#define SPELL_MANABURN 26046
#define SPELL_SLEEP 24664

#define SPELL_SUMMON_NIGHTMARE_ILLUSIONS 24728

#define YELL_HAZZARAH_1 "Today, you'll wish you never stirred from your bed!"                                       ///on start attacking
#define YELL_HAZZARAH_2 "Slumber... another dream awaits you..."                                                    /// on casting spell xxx??
#define YELL_HAZZARAH_3 "My spirit sleeps no longer..."                                                             /// on casting spell xxx??
#define YELL_HAZZARAH_4 "Let's see what's more horrorfying... To dream onwalking, or awaken to a living nightmare"  /// by spawning nightmare illusions (npc_id: 15163)
#define YELL_HAZZARAH_5 "Let us see what horrors stir within your nightmares!"                                      /// by spawning nightmare illusions (npc_id: 15163)

//////////////////////////////////////////////////////////////////////////////////////////
///\details <b>Edge of Madness, Renataki</b>\n
/// event_names entry: 29 \n
/// event_names holiday: 0 \n
/// npc_id: 15084 \n
///\todo Monsterscript for Rentaki

#define SPELL_THOUSANDBLADES 34799
#define SPELL_AMBUSH 34794
#define SPELL_THOUSANDBLADES 34799

//////////////////////////////////////////////////////////////////////////////////////////
///\details <b>Edge of Madness, Wushoolay</b>\n
/// event_names entry: 30 \n
/// event_names holiday: 0 \n
/// npc_id: 15085 \n
/// First you must find four Artifacts to interact with them (Archeologie) with an Gossip MenuItem \n
/// #define GOSSIP_MENU_ITEM "Examine the object." 
///\todo Monsterscript for Wushoolay

#define YELL_WUSHOOLAY_1 "You shall regret disturbing my homeland!"                     /// on start attacking
#define YELL_WUSHOOLAY_2 "The storm feeds my power!"                                    /// on casting spell xxx??
#define YELL_WUSHOOLAY_3 "In a flash, you'll be nothing more than dust..."              /// on casting spell xxx??
#define YELL_WUSHOOLAY_4 "Can you feel that tingling sensation? It may be your last."   /// on casting spell xxx??
#define YELL_WUSHOOLAY_5 "Shocking. I know"                                             /// If a player dies
#define YELL_WUSHOOLAY_6 "Are you quicker than lightning?"                              /// on casting spell xxx??
#define YELL_WUSHOOLAY_7 "The end of our empire... is only the beginning..."            /// on die


void SetupEdgeOfMadnes(ScriptMgr* mgr)
{ }