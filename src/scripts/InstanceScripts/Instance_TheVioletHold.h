/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _INSTANCE_THE_VIOLET_HOLD_H
#define _INSTANCE_THE_VIOLET_HOLD_H

enum CreatureEntry
{
    //Main event
    CN_LIEUTNANT_SINCLARI = 30658,
    CN_VIOLET_HOLD_GUARD = 30659,
    CN_PORTAL_GUARDIAN = 30660,     //enemies
    CN_PORTAL_INTRO = 31011,        //portals, not a go its a creature ;)
    CN_CRYSTAL_SYSTEM = 30837,      // NPC with spell arcane spher

    //Portal Guardians (Normal)
    CN_AZURE_INVADER = 30661,
    CN_AZURE_SPELLBREAKER = 30662,
    CN_AZURE_BINDER = 30663,
    CN_AZURE_MAGE_SLAYER = 30664,
    CN_AZURE_CAPTAIN = 30666,
    CN_AZURE_SORCEROR = 30667,
    CN_AZURE_RAIDER = 30668,
    CN_AZURE_STALKER = 32191,

    //Bosses
    CN_EREKEM = 29315,
    CN_MORAGG = 29316,
    CN_ICHORON = 29313,
    CN_XEVOZZ = 29266,
    CN_LAVANTHOR = 29312,
    CN_TURAMAT_THE_OBLITERATOR = 29314,
    CN_CYANIGOSA = 31134
};

enum CreatureSpells
{
    //Spell Crytals
    SPELL_ARCANE_LIGHTNING = 57930
};

enum GameObjects
{
    //Crystals
    GO_INTRO_ACTIVATION_CRYSTAL = 193615,
    GO_ACTIVATION_CRYSTAL = 193611,

    //Door
    GO_PRISON_SEAL = 191723,
    GO_XEVOZZ_DOOR = 191556,
    GO_LAVANTHOR_DOOR = 191566,
    GO_ICHORON_DOOR = 191722,
    GO_ZURAMAT_THE_OBLITERATOR_DOOR = 191565,
    GO_EREKEM_DOOR = 191564,
    GO_MORAGG_DOOR = 191606
};

enum CreatureSay
{

};

#endif // _INSTANCE_THE_VIOLET_HOLD_H
