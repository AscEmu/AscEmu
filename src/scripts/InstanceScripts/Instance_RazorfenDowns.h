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

#ifndef _INSTANCE_RAZORFEN_DOWNS_H
#define _INSTANCE_RAZORFEN_DOWNS_H

enum CreatureEntry
{
    CN_AMNENNAR_GOLDBRINGER     = 7358,     // AmnennarTheColdbringerAI
    CN_GLUTTON                  = 8567,     // GluttonAI
    CN_MORDRESH_FIRE_EYE        = 7357,     // MordreshFireEyeAI
    CN_PLAGUEMAW_THE_ROTTING    = 7356,     // PlaguemawTheRottingAI
    CN_RAGGLESNOUT              = 7354,     // RagglesnoutAI
    CN_TUTEN_KASH               = 7355      // TutenKashAI
};

enum CreatureSpells
{
    // GluttonAI
    SP_GLUTTON_DISEASE_CLOUD    = 12627,
    SP_GLUTTON_FRENZY           = 12795,

    // MordreshFireEyeAI
    SP_MORDRESH_FIRE_NOVA       = 12470,
    SP_MORDRESH_FIREBALL        = 12466
};

enum CreatureSay
{
};

#endif // _INSTANCE_RAZORFEN_DOWNS_H
