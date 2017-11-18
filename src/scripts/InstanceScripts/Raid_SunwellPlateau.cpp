/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"

/*

//Kil'Jaeden sound IDs saved for future use
//Some of them are used anytime during the raid progression in the instance (no mechanism for this yet)
//Some others are used in the actual Kil'Jaeden encounter

12527 Sunwell Plateau - Kil Jaeden "Spawning"
12495 Sunwell Plateau - Kil Jaeden - "All my plans have led to this"
12496 Sunwell Plateau - Kil Jaeden - "Stay on task, do not waste time"
12497 Sunwell Plateau - Kil Jaeden - "I've waited long enough!"
12498 Sunwell Plateau - Kil Jaeden - "Fail me, and suffer for eternity!"
12499 Sunwell Plateau - Kil Jaeden - "Drain the girl, drain her power, untill there is nothing but an ...something... shell"
12500 Sunwell Plateau - Kil Jaeden - Very long thing, basiclly he tells us that he will take control over the Burning Legion.
12501 Sunwell Plateau - Kil Jaeden - "Another step towards destruction!"

*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Sunblade Protector
const uint32 CN_SUNBLADE_PROTECTOR = 25507;
const uint32 SUNBLADE_PROTECTOR_FEL_LIGHTNING = 46480;

class SunbladeProtectorAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SunbladeProtectorAI);
        SunbladeProtectorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(SUNBLADE_PROTECTOR_FEL_LIGHTNING, Target_RandomPlayer, 100, 0, 15, 0, 60);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Shadowsword Assassin
const uint32 CN_SHADOWSWORD_ASSASSIN = 25484;
const uint32 SHADOWSWORD_ASSASSIN_ASSASSINS_MARK = 46459;
const uint32 SHADOWSWORD_ASSASSIN_AIMED_SHOT = 46460;
const uint32 SHADOWSWORD_ASSASSIN_SHADOWSTEP = 46463;
const uint32 SHADOWSWORD_ASSASSIN_GREATER_INVISIBILITY = 16380;

class ShadowswordAssassinAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ShadowswordAssassinAI);
        ShadowswordAssassinAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(SHADOWSWORD_ASSASSIN_ASSASSINS_MARK, Target_RandomPlayer, 100, 0, 15, 0, 100);
            AddSpell(SHADOWSWORD_ASSASSIN_AIMED_SHOT, Target_Current, 15, 4, 6, 5, 35, true);
            AddSpell(SHADOWSWORD_ASSASSIN_SHADOWSTEP, Target_RandomPlayer, 15, 0, 50, 0, 40);
            AddSpell(SHADOWSWORD_ASSASSIN_GREATER_INVISIBILITY, Target_Self, 5, 0, 180);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Shadowsword Commander
const uint32 CN_SHADOWSWORD_COMMANDER = 25837;
const uint32 SHADOWSWORD_COMMANDER_SHIELD_SLAM = 46762;
const uint32 SHADOWSWORD_COMMANDER_BATTLESHOUT = 46763;

class ShadowswordCommanderAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ShadowswordCommanderAI);
        ShadowswordCommanderAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(SHADOWSWORD_COMMANDER_SHIELD_SLAM, Target_Current, 10, 0, 10);
            AddSpell(SHADOWSWORD_COMMANDER_BATTLESHOUT, Target_Self, 20, 0, 25);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Kalecgos
/*const uint32 CN_KALECGOS = 24850
const uint32 KALECGOS_FROST_BREATH = 44799
const uint32 KALECGOS_SPECTRAL_BLAST = 44866
const uint32 KALECGOS_ARCANE_BUFFET = 45018

void SpellFunc_Kalecgos_WildMagic(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);

class KalecgosAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KalecgosAI);
    KalecgosAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(KALECGOS_FROST_BREATH, Target_Current, 10, 1, 12, 0, 30);
        AddSpellFunc(SpellFunc_Kalecgos_WildMagic, Target_RandomPlayer, 15, 0, 10, 0, 100);
        AddSpell(KALECGOS_SPECTRAL_BLAST, Target_Self, 25, 0, 25, 0, 50);
        AddSpell(KALECGOS_ARCANE_BUFFET, Target_Self, 100, 0, 8);

        //Emotes
        AddEmote(Event_OnCombatStart, "I need... your help... Cannot... resist him... much longer...", Text_Yell, 12428);
        AddEmote(Event_OnTargetDied, "In the name of Kil'jaeden!", Text_Yell, 12425);
        AddEmote(Event_OnTargetDied, "You were warned! ", Text_Yell, 12426);
        AddEmote(Event_OnDied, "I am forever in your debt. Once we have triumphed over Kil'jaeden, this entire world will be in your debt as well.", Text_Yell, 12431);
    }
};

void SpellFunc_Kalecgos_WildMagic(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType)
{
    KalecgosAI* Kalecgos = (pCreatureAI) ? (KalecgosAI*)pCreatureAI : NULL;
    if (Kalecgos)
    {
        \todo 

        const uint32 SP_WILD_MAGIC_1            44978
        const uint32 SP_WILD_MAGIC_2            45001
        const uint32 SP_WILD_MAGIC_3            45002
        const uint32 SP_WILD_MAGIC_4            45004
        const uint32 SP_WILD_MAGIC_5            45006
        const uint32 SP_WILD_MAGIC_6            45010
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Sathrovarr the Corruptor
const uint32 CN_SATHROVARR_THE_CORRUPTOR = 24892
const uint32 SATHROVARR_THE_CORRUPTOR_CURSE_OF_BOUNDLESS_AGONY = 45034
const uint32 SATHROVARR_THE_CORRUPTOR_SHADOW_BOLT_VOLLEY = 38840
const uint32 SATHROVARR_THE_CORRUPTOR_CORRUPTING_STRIKE = 45029

class SathrovarrTheCorruptorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SathrovarrTheCorruptorAI);
    SathrovarrTheCorruptorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SATHROVARR_THE_CORRUPTOR_CURSE_OF_BOUNDLESS_AGONY, Target_RandomPlayer, 20, 0, 12, 0, 40);
        AddSpell(SATHROVARR_THE_CORRUPTOR_SHADOW_BOLT_VOLLEY, Target_RandomPlayerApplyAura, 20, 1, 25, 0, 40);
        AddSpell(SATHROVARR_THE_CORRUPTOR_CORRUPTING_STRIKE, Target_Current, 30, 0, 5, 0, 10);

        //Emotes
        AddEmote(Event_OnCombatStart, "Gyahaha... There will be no reprieve. My work here is nearly finished.", Text_Yell, 12451);
        AddEmote(Event_OnTargetDied, "Pitious mortal!", Text_Yell, 12455);
        AddEmote(Event_OnTargetDied, "Haven't you heard? I always win!", Text_Yell, 12456);
        AddEmo(Event_OnDied, "I'm... never on... the losing... side...", Text_Yell, 12452);
    }
};
*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Brutallus
const uint32 CN_BRUTALLUS = 24882;
const uint32 BRUTALLUS_METEOR_SLASH = 45150;
const uint32 BRUTALLUS_BURN = 45141;
const uint32 BRUTALLUS_STOMP = 45185;
const uint32 BRUTALLUS_BERSERK = 26662;

class BrutallusAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(BrutallusAI);
        BrutallusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(BRUTALLUS_METEOR_SLASH, Target_Self, 100, 1, 12);
            AddSpell(BRUTALLUS_BURN, Target_RandomPlayer, 50, 0, 20);
            AddSpell(BRUTALLUS_STOMP, Target_Current, 25, 0, 30);

            //6min Enrage
            SetEnrageInfo(AddSpell(BRUTALLUS_BERSERK, Target_Self, 0, 0, 0, 0, 0, false, "So much for a real challenge... Die!", CHAT_MSG_MONSTER_YELL, 12470), 360000);

            //Emotes
            addEmoteForEvent(Event_OnCombatStart, 8834);
            addEmoteForEvent(Event_OnTargetDied, 8835);
            addEmoteForEvent(Event_OnTargetDied, 8836);
            addEmoteForEvent(Event_OnTargetDied, 8837);
            addEmoteForEvent(Event_OnDied, 8838);
            addEmoteForEvent(Event_OnTaunt, 8839);
            addEmoteForEvent(Event_OnTaunt, 8840);
            addEmoteForEvent(Event_OnTaunt, 8841);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Felmyst
const uint32 CN_FELMYST = 25038;
const uint32 FELMYST_CLEAVE = 19983;
const uint32 FELMYST_CORROSION = 45866;
const uint32 FELMYST_DEMONIC_VAPOR = 45402;
const uint32 FELMYST_GAS_NOVA = 45855;
const uint32 FELMYST_NOXIOUS_FUME = 47002;
const uint32 FELMYST_ENCAPSULATE = 45662;
const uint32 FELMYST_FOG_OF_CORRUPTION = 45717;
const uint32 FELMYST_ENRAGE = 26662;    //Using same as Brutallus for now, need to find actual spell id

class FelmystAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(FelmystAI);
        FelmystAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            //Phase 1 spells
            AddPhaseSpell(1, AddSpell(FELMYST_CLEAVE, Target_Current, 6, 0, 10, 0, 5));
            AddPhaseSpell(1, AddSpell(FELMYST_GAS_NOVA, Target_Self, 25, 1, 18));
            AddPhaseSpell(1, AddSpell(FELMYST_ENCAPSULATE, Target_RandomPlayer, 25, 7, 30, 0, 30));
            AddPhaseSpell(1, AddSpell(FELMYST_CORROSION, Target_Current, 20, 0.75f, 35, 0, 30, false, "Choke on your final breath!", CHAT_MSG_MONSTER_YELL, 12478));

            //Phase 2 spells
            AddPhaseSpell(2, AddSpell(FELMYST_DEMONIC_VAPOR, Target_RandomPlayer, 10, 0, 20));

            //Phase 3 spells
            //Fog of corruption is the actual breath Felmyst does during his second phase, probably we'll have to spawn it like a creature.
            //AddSpell(FELMYST_FOG_OF_CORRUPTION, Target_RandomPlayerApplyAura, 15, 0, 20, 0, 10); Does not support by the core.

            //10min Enrage
            SetEnrageInfo(AddSpell(FELMYST_ENRAGE, Target_Self, 0, 0, 0, 0, 0, false, "No more hesitation! Your fates are written!", CHAT_MSG_MONSTER_YELL, 12482), 600000);

            //Emotes
            addEmoteForEvent(Event_OnCombatStart, 8842);
            addEmoteForEvent(Event_OnTargetDied, 8843);
            addEmoteForEvent(Event_OnTargetDied, 8844);
            addEmoteForEvent(Event_OnDied, 8845);
            addEmoteForEvent(Event_OnTaunt, 8846);
        }

        void OnCombatStart(Unit* /*pTarget*/) override
        {
            _applyAura(FELMYST_NOXIOUS_FUME);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Lady Sacrolash
const uint32 CN_LADY_SACROLASH = 25165;
const uint32 CN_GRAND_WARLOCK_ALYTHESS = 25166;
const uint32 LADY_SACROLASH_DARK_TOUCHED = 45347;
const uint32 LADY_SACROLASH_SHADOW_BLADES = 45248;
const uint32 LADY_SACROLASH_SHADOW_NOVA = 45329;
const uint32 LADY_SACROLASH_CONFOUNDING_BLOW = 45256;
const uint32 LADY_SACROLASH_ENRAGE = 26662;    //Using same as Brutallus for now, need to find actual spell id

class LadySacrolashAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LadySacrolashAI);
        LadySacrolashAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(LADY_SACROLASH_DARK_TOUCHED, Target_RandomPlayerApplyAura, 50, 0, 10, 0, 50);
            AddSpell(LADY_SACROLASH_SHADOW_BLADES, Target_Current, 25, 1.5, 5, 0, 50);
            AddSpell(LADY_SACROLASH_SHADOW_NOVA, Target_RandomPlayer, 15, 3.5, 20, 0, 50, false, "Shadow to the aid of fire!", CHAT_MSG_MONSTER_YELL, 12485);
            AddSpell(LADY_SACROLASH_CONFOUNDING_BLOW, Target_RandomPlayer, 10, 0, 15, 0, 50);
            SetEnrageInfo(AddSpell(LADY_SACROLASH_ENRAGE, Target_Self, 0, 0, 0, 0, 0, 0, "Time is a luxury you no longer possess!", CHAT_MSG_MONSTER_YELL, 0), 360000); // Wasn't able to find sound for this text

            //Emotes
            addEmoteForEvent(Event_OnTargetDied, 8847);
            addEmoteForEvent(Event_OnDied, 8848); // Wasn't able to find sound for this text
        }

        void OnDied(Unit* /*pKiller*/) override
        {
            CreatureAIScript* mGrandWarlockAlythess = getNearestCreatureAI(CN_GRAND_WARLOCK_ALYTHESS);
            if (mGrandWarlockAlythess != nullptr && mGrandWarlockAlythess->isAlive())
            {
                mGrandWarlockAlythess->sendChatMessage(CHAT_MSG_MONSTER_YELL, 12492, "Sacrolash!");
            }
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Grand Warlock Alythess
const uint32 GRAND_WARLOCK_ALYTHESS_PYROGENICS = 45230;
const uint32 GRAND_WARLOCK_ALYTHESS_FLAME_TOUCHED = 45348;
const uint32 GRAND_WARLOCK_ALYTHESS_CONFLAGRATION = 45342;
const uint32 GRAND_WARLOCK_ALYTHESS_BLAZE = 45235;
const uint32 GRAND_WARLOCK_ALYTHESS_FLAME_SEAR = 46771;
const uint32 GRAND_WARLOCK_ALYTHESS_ENRAGE = 26662;    //Using same as Brutallus for now, need to find actual spell id

class GrandWarlockAlythessAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GrandWarlockAlythessAI);
        GrandWarlockAlythessAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(GRAND_WARLOCK_ALYTHESS_PYROGENICS, Target_Self, 100, 0, 10, 0, 50);
            AddSpell(GRAND_WARLOCK_ALYTHESS_FLAME_TOUCHED, Target_RandomPlayerApplyAura, 10, 0, 30, 0, 50);
            AddSpell(GRAND_WARLOCK_ALYTHESS_CONFLAGRATION, Target_RandomPlayer, 15, 3.5, 25, 0, 50, false, "Fire to the aid of shadow!", CHAT_MSG_MONSTER_YELL, 12489);
            AddSpell(GRAND_WARLOCK_ALYTHESS_BLAZE, Target_RandomPlayer, 30, 2.5, 0, 0, 50);
            AddSpell(GRAND_WARLOCK_ALYTHESS_FLAME_SEAR, Target_RandomPlayer, 20, 0, 0, 0, 50);
            SetEnrageInfo(AddSpell(GRAND_WARLOCK_ALYTHESS_ENRAGE, Target_Self, 0, 0, 0, 0, 0, false, "Your luck has run its course!", CHAT_MSG_MONSTER_YELL, 12493), 360000);

            //Emotes
            addEmoteForEvent(Event_OnTargetDied, 8849);
            addEmoteForEvent(Event_OnDied, 8850); // Wasn't able to find sound for this text
        }

        void OnDied(Unit* /*pKiller*/) override
        {
            CreatureAIScript* mLadySacrolash = getNearestCreatureAI(CN_LADY_SACROLASH);
            if (mLadySacrolash != nullptr && mLadySacrolash->isAlive())
            {
                mLadySacrolash->sendChatMessage(CHAT_MSG_MONSTER_YELL, 12488, "Alythess! Your fire burns within me!");
            }
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//M'uru
const uint32 CN_MURU = 25741;
const uint32 CN_SHADOWSWORD_BERSERKER = 25798;
const uint32 CN_SHADOWSWORD_FURY_MAGE = 25799;
const uint32 CN_VOID_SENTINEL = 25772;
const uint32 MURU_NEGATIVE_ENERGY = 46008;   //patch 2.4.2: this spell shouldn't cause casting pushback (to be fixed in core)
const uint32 MURU_DARKNESS = 45996;
const uint32 MURU_SUMMON_BERSERKER = 46037;
const uint32 MURU_SUMMON_FURY_MAGE = 46038;
const uint32 MURU_SUMMON_VOID_SENTINEL = 45988;

class MuruAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MuruAI);
        MuruAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(MURU_NEGATIVE_ENERGY, Target_Self, 25, 0, 0);
            AddSpell(MURU_DARKNESS, Target_Self, 20, 0, 45);

            //AddSpell(MURU_SUMMON_BERSERKER, Target_, 15, 3.5, 25, 0, 50);  Most of Databases don't the SQL for this yet. Also I am not sure what function are for summoning Spells :).
            //AddSpell(MURU_SUMMON_FURY_MAGE, Target_, 30, 2.5, 0, 0, 50);
            //AddSpell(MURU_SUMMON_VOID_SENTINEL, Target_, 20, 0, 0, 0, 50);
        }
};

class ShadowswordBerserkerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ShadowswordBerserkerAI);
        ShadowswordBerserkerAI(Creature* pCreature) : CreatureAIScript(pCreature) {}
};

class ShadowswordFuryMageAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ShadowswordFuryMageAI);
        ShadowswordFuryMageAI(Creature* pCreature) : CreatureAIScript(pCreature) {}
};

class VoidSentinelAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(VoidSentinelAI);
        VoidSentinelAI(Creature* pCreature) : CreatureAIScript(pCreature) {}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Entropius
const uint32 CN_ENTROPIUS = 25840;

class EntropiusAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(EntropiusAI);
        EntropiusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            ///\todo Entropius AI Boss Script
        }
};

void SetupSunwellPlateau(ScriptMgr* pScriptMgr)
{
    pScriptMgr->register_creature_script(CN_SUNBLADE_PROTECTOR, &SunbladeProtectorAI::Create);

    //pScriptMgr->register_creature_script(CN_KALECGOS, &SUNWELL_KALECGOS::DRAGON_KALECGOS::Create);
    //pScriptMgr->register_creature_script(CN_SATHROVAR, &SUNWELL_KALECGOS::SATHROVAR::Create);
    //pScriptMgr->register_creature_script(CN_DARK_ELF, &SUNWELL_KALECGOS::DARK_ELF::Create);
    //pScriptMgr->register_dummy_spell(KALECGOS_SPECTRAL_TELEPORT, &SUNWELL_KALECGOS::HandleSpectralTeleport);
    pScriptMgr->register_creature_script(CN_BRUTALLUS, &BrutallusAI::Create);
    pScriptMgr->register_creature_script(CN_FELMYST, &FelmystAI::Create);
    pScriptMgr->register_creature_script(CN_LADY_SACROLASH, &LadySacrolashAI::Create);
    pScriptMgr->register_creature_script(CN_GRAND_WARLOCK_ALYTHESS, &GrandWarlockAlythessAI::Create);
    pScriptMgr->register_creature_script(CN_MURU, &MuruAI::Create);
    pScriptMgr->register_creature_script(CN_SHADOWSWORD_BERSERKER, &ShadowswordBerserkerAI::Create);
    pScriptMgr->register_creature_script(CN_SHADOWSWORD_FURY_MAGE, &ShadowswordFuryMageAI::Create);
    pScriptMgr->register_creature_script(CN_VOID_SENTINEL, &VoidSentinelAI::Create);
    pScriptMgr->register_creature_script(CN_ENTROPIUS, &EntropiusAI::Create);
}
