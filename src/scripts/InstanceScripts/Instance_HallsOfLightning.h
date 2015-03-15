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

#ifndef _INSTANCE_HALLS_OF_LIGHTNING_H
#define _INSTANCE_HALLS_OF_LIGHTNING_H

enum CreatureEntry
{
    //GeneralBjarnimAI
    CN_GENERAL_BJARNGRIM    = 28586,

    //Volkhan
    CN_VOLKHAN              = 28587,
    CN_MOLTEN_GOLEM         = 28695,
    CN_BRITTLE_GOLEM        = 28681,
    CN_VOLKHANS_ANVIL       = 28823,

    //Loken
    CN_LOKEN                = 28923,

    //IlonarAI
    CN_IONAR                = 28546,
    CN_SPARK                = 28926
};

enum CreatureSpells
{
    //GeneralBjarnimAI
    SPELL_ARC_WELD              = 59085,
    SPELL_BATTLE_AURA           = 41106,
    SPELL_BATTLE_STANCE         = 53792,
    SPELL_BERSERKER_AURA        = 41107,
    SPELL_BERSERKER_STANCE      = 53791,
    SPELL_CHARGE_UP             = 52098,
    SPELL_CLEAVE                = 15284,
    SPELL_DEFENSIVE_AURA        = 41105,
    SPELL_DEFENSIVE_STANCE      = 53790,
    SPELL_INTERCEPT             = 58769,
    SPELL_IRONFORM              = 52022,
    SPELL_KNOCK_AWAY            = 52029,
    SPELL_MORTAL_STRIKE         = 16856,
    SPELL_PUMMEL                = 12555,
    SPELL_SLAM                  = 52026,
    SPELL_SPELL_REFLECTION      = 36096,
    SPELL_WHIRLWIND             = 52027

};

enum CreatureSay
{

};

enum GameObjectEntry
{
    GO_GENERAL_DOORS        = 191416,
    GO_VOLKHAN_DOORS        = 191325,
    GO_LOKEN_DOORS          = 191324,
    GO_IONAR_DOORS1         = 191326,
    GO_IONAR_DOORS2         = 191328
};

#endif // _INSTANCE_HALLS_OF_LIGHTNING_H
