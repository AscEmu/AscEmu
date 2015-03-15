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

#ifndef _INSTANCE_DRAK_THARON_KEEP_H
#define _INSTANCE_DRAK_THARON_KEEP_H

enum CreatureEntry
{
    //TrollgoreAI
    CN_TROLLGORE            = 26630,
    CN_DRAKKARI_INVADER     = 27709,

    // NovosTheSummonerAI
    CN_NOVOS_THE_SUMMONER   = 26631,

    // CrystalHandlerAI
    CN_CRYSTAL_HANDLER      = 26627,

    // KingDreadAI
    CN_KING_DRED            = 27483,

    // TheProphetTaronjaAI
    CN_THE_PROPHET_THARONJA = 26632,

};

enum CreatureSpells
{
    // NovosTheSummonerAI
    SPELL_ARCANE_FIELD      = 47346,

};

enum CreatureSay
{
    // NovosTheSummonerAI
    SAY_NOVOS_SUMMONER_01   = 4020,     // The chill you feel is the herald of your doom!
    SAY_NOVOS_SUMMONER_02   = 4021,     // Such is the fate of all who oppose the Lich King!
    SAY_NOVOS_SUMMONER_03   = 4022,     // Your efforts... are in vain.
    SAY_NOVOS_SUMMONER_04   = 4023,     // Bolster my defenses!Hurry, curse you!
    SAY_NOVOS_SUMMONER_05   = 4024,     // Surely you can see the futility of it all!
    SAY_NOVOS_SUMMONER_06   = 4025,     // Just give up and die already!

    BROADCAST_NOVOS_SUMMONER_01     = 6924      // \todo not used % s calls for assistance!


};

enum GameObjectEntry
{
    // NovosTheSummonerAI
    GO_RITUAL_CRYSTAL_ENTRY_1   = 189299,
    GO_RITUAL_CRYSTAL_ENTRY_2   = 189300,
    GO_RITUAL_CRYSTAL_ENTRY_3   = 189301,
    GO_RITUAL_CRYSTAL_ENTRY_4   = 189302,   //make sure that you doesn't have these on the map
};

#endif // _INSTANCE_DRAK_THARON_KEEP_H
