/*
 * AscScripts for AscEmu Framework
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
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

#ifndef _INSTANCE_BLACK_MORASS_H
#define _INSTANCE_BLACK_MORASS_H

enum CreatureEntry
{
    // ChronoLordAI
    CN_CHRONO_LORD_DEJA = 17879,

    // TemporusAI
    CN_TEMPORUS         = 17880,

    //AenusAI
    CN_AEONUS           = 17881,

};

enum CreatureSpells
{
    // ChronoLordAI
    ARCANE_BLAST    = 24857,
    TIME_LAPSE      = 31467,
    //MAGNETIC_PULL     = 31705, // Only in Heroics - Correct ID?

    // TemporusAI
    HASTEN          = 31458,
    MORTAL_WOUND    = 28467,
    //SPELL_REFLECTION  = 31705, // Only in Heroics - Correct ID?

    //AenusAI
    SAND_BREATH     = 31478,
    TIME_STOP       = 31422,
    FRENZY          = 28371     //ID according to wowwiki
};

enum CreatureSay
{
    // ChronoLordAI
    SAY_CHRONOLORD_01   = 3563,     // If you will not cease this foolish quest, then you will die!
    SAY_CHRONOLORD_02   = 3565,     // I told you it was a fool's quest!
    SAY_CHRONOLORD_03   = 3566,     // Leaving so soon?
    SAY_CHRONOLORD_04   = 3567,     // Time... is on our side.
                                    // ID: 3564 "You have outstayed your welcome, Keeper. Be gone!"
                                    // ID: 3562 "Why do you aid the Magus? Just think of how many lives could be saved if the portal is never opened, if the resulting wars could be erased...."
                                    // ID: 3564 "You have outstayed your welcome, Keeper. Be gone!"

    // TemporusAI
    SAY_TEMPORUS_01     = 3585,     // So be it... you have been warned.
    SAY_TEMPORUS_02     = 3587,     // You should have left when you had the chance.
    SAY_TEMPORUS_03     = 3588,     // Your days are done.
    SAY_TEMPORUS_04     = 3589,     // My death means... little.
                                    // ID: 3584 "Why do you persist? Surely you can see the futility of it all. It is not too late! You may still leave with your lives..."
                                    // ID: 3586 "Keeper! The sands of time have run out for you."

    //AenusAI
    SAY_AENUS_01        = 3569,     // Let us see what fate has in store....
    SAY_AENUS_02        = 3572,     // No one will stop us! old:No one can stop us! No one!
    SAY_AENUS_03        = 3571,     // One less obstacle in our way!
    SAY_AENUS_04        = 3573,     // It is only a matter...of time.
                                    // ID: 3568 "The time has come to shatter this clockwork universe forever! Let us no longer be slaves of the hourglass! I warn you: those who do not embrace this greater path shall become victims of its passing!"
                                    // ID: 3569 "Let us see what fate has in store...."
                                    // BROADCAST ID: 3574 "%s goes into a frenzy!"
};

#endif // _INSTANCE_BLACK_MORASS_H
