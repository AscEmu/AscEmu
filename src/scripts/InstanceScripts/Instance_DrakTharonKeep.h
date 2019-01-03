/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

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
