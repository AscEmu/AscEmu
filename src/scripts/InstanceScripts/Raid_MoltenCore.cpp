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


const uint32 CN_CORERAGER = 11672;

const uint32 MANGLE = 19820; // 1 target
// put full HP if less 50% and golemagg is still alive

// Golemagg AI
class CoreRagerAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CoreRagerAI);
        CoreRagerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            m_mangle = true;

            info_mangle = sSpellCustomizations.GetSpellInfo(MANGLE);
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            uint32 val = RandomUInt(1000);
            SpellCast(val);
        }

        void SpellCast(uint32 val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())//_unit->getAttackTarget())
            {
                //Unit* target = _unit->GetAIInterface()->GetNextTarget();
                if (m_mangle)
                {
                    getCreature()->CastSpell(getCreature(), info_mangle, false);
                    m_mangle = false;
                    return;
                }

                if (val >= 100 && val <= 220)
                {
                    getCreature()->setAttackTimer(9000, false);
                    m_mangle = true;
                }
            }
        }

    protected:

        bool m_mangle;
        SpellInfo* info_mangle;
};

const uint32 CN_SULFURON_HARBRINGER = 12098;

const uint32 DEMORALIZING_SHOUT = 19778;
const uint32 INSPIRE = 19779;
const uint32 FLAME_SPEAR = 19781;
// needs a aoe knockback 19780?

// Sulfuron Harbringer AI
class SulfuronAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(SulfuronAI);
        SulfuronAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            m_demoralizingshout = m_inspire = m_flamespear = true;

            info_demoralizingshout = sSpellCustomizations.GetSpellInfo(DEMORALIZING_SHOUT);
            info_inspire = sSpellCustomizations.GetSpellInfo(INSPIRE);
            info_flamespear = sSpellCustomizations.GetSpellInfo(FLAME_SPEAR);
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            uint32 val = RandomUInt(1000);
            SpellCast(val);
        }

        void SpellCast(uint32 val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())//_unit->getAttackTarget())
            {
                //Unit* target = _unit->GetAIInterface()->GetNextTarget();

                if (m_demoralizingshout)
                {
                    getCreature()->CastSpell(getCreature(), info_demoralizingshout, false);
                    m_demoralizingshout = false;
                    return;
                }

                if (m_inspire)
                {
                    getCreature()->CastSpell(getCreature(), info_inspire, false);
                    m_inspire = false;
                    return;
                }

                if (m_flamespear)
                {
                    getCreature()->CastSpell(getCreature(), info_flamespear, false);
                    m_flamespear = false;
                    return;
                }

                if (val >= 100 && val <= 180)
                {
                    getCreature()->setAttackTimer(1000, false);
                    m_inspire = true;
                }

                if (val > 180 && val <= 260)
                {
                    getCreature()->setAttackTimer(1000, false);
                    m_demoralizingshout = true;
                }

                if (val > 260 && val <= 320)
                {
                    getCreature()->setAttackTimer(1000, false);
                    m_flamespear = true;
                }
            }
        }

    protected:

        bool m_demoralizingshout, m_inspire, m_flamespear;
        SpellInfo* info_demoralizingshout, *info_inspire, *info_flamespear;
};



// Woot DOING RAGNAROS Tha BosS

/*    * Ragnaros Summoning talk:
          o Majordomo Executus: Behold Ragnaros, the Firelord! He who was ancient when this world was young! Bow before him, mortals! Bow before your ending!
          o Ragnaros: TOO SOON! YOU HAVE AWAKENED ME TOO SOON, EXECUTUS! WHAT IS THE MEANING OF THIS INTRUSION?
          o Majordomo Executus: These mortal infidels, my lord! They have invaded your sanctum, and seek to steal your secrets!
          o Ragnaros: FOOL! YOU ALLOWED THESE INSECTS TO RUN RAMPANT THROUGH THE HALLOWED CORE, AND NOW YOU LEAD THEM TO MY VERY LAIR? YOU HAVE FAILED ME, EXECUTUS! JUSTICE SHALL BE MET, INDEED!
          o Ragnaros: NOW FOR YOU, INSECTS. BOLDLY YOU SOUGHT THE POWER OF RAGNAROS! NOW YOU SHALL SEE IT FIRSTHAND!
    * DIE, INSECT! (When he kills the player he has aggro on)
    * BY FIRE BE PURGED! (Ranged knockback)
    * TASTE THE FLAMES OF SULFURON! (Melee knockback)
    * COME FORTH, MY SERVANTS! DEFEND YOUR MASTER! (Summoning Sons of Flame) */

const uint32 CN_RAGNAROS = 11502;

const uint32 ELEMENTAL_FIRE = 20563; // 1 target
const uint32 MAGMA_BLAST = 20565; // various targets on not attacked. -> sound -> 8048 ?
const uint32 WRATH_OF_RAGNAROS = 20566;  // Fly bitches fly! quote -> "Taste the Flames of Sulfuron!" -> sound 8047
const uint32 HAMMER_OF_RAGNAROS = 19780; // quote -> "By fire be purged!" -> sound 8046
const uint32 MELT_WEAPON = 21387; // 1 target
const uint32 SUMMON_SONS_OF_FLAMES = 21108; //\todo  DUMMY :P summon the sons of flames. entry = 12143 -> sound 8049,8050


//Ragnaros Submerge Visual -> 20567
//Ragnaros Emerge -> 20568

// Ragnaros AI
class RagnarosAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(RagnarosAI);
        RagnarosAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            m_elementalfire = m_wrath = m_hammer = m_meltweapon = m_summonsons = true;

            info_elementalfire = sSpellCustomizations.GetSpellInfo(ELEMENTAL_FIRE);
            info_wrath = sSpellCustomizations.GetSpellInfo(WRATH_OF_RAGNAROS);
            info_hammer = sSpellCustomizations.GetSpellInfo(HAMMER_OF_RAGNAROS);
            info_meltweapon = sSpellCustomizations.GetSpellInfo(MELT_WEAPON);
            info_summonsons = sSpellCustomizations.GetSpellInfo(SUMMON_SONS_OF_FLAMES);
            getCreature()->setMoveRoot(true);
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
            getCreature()->GetAIInterface()->skip_reset_hp = true;
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            sendDBChatMessage(3053);     // Die, insect!
        }

        void AIUpdate()
        {
            uint32 val = RandomUInt(1000);
            SpellCast(val);
        }

        void SpellCast(uint32 val)
        {
            if (getCreature()->GetCurrentSpell() == NULL && getCreature()->GetAIInterface()->getNextTarget())//_unit->getAttackTarget())
            {
                Unit* target = getCreature()->GetAIInterface()->getNextTarget();

                if (m_elementalfire)
                {
                    getCreature()->CastSpell(target, info_elementalfire, false);
                    m_elementalfire = false;
                    return;
                }

                if (m_wrath)
                {
                    sendDBChatMessage(3052);     // TASTE THE FLAMES OF SULFURON!
                    getCreature()->CastSpell(getCreature(), info_wrath, false);
                    m_wrath = false;
                    return;
                }

                if (m_hammer)
                {
                    sendDBChatMessage(3051);     // By fire be purged!
                    getCreature()->CastSpell(getCreature(), info_hammer, false);
                    m_hammer = false;
                    return;
                }

                if (m_meltweapon)
                {
                    getCreature()->CastSpell(target, info_meltweapon, false);
                    m_meltweapon = false;
                    return;
                }

                if (val >= 100 && val <= 160)
                {
                    getCreature()->setAttackTimer(1000, false);
                    m_elementalfire = true;
                }

                if (val > 160 && val <= 220)
                {
                    getCreature()->setAttackTimer(1000, false);
                    m_wrath = true;
                }

                if (val > 220 && val <= 280)
                {
                    getCreature()->setAttackTimer(1000, false);
                    m_hammer = true;
                }
                if (val > 340 && val <= 400)
                {
                    getCreature()->setAttackTimer(1000, false);
                    m_meltweapon = true;
                }

            }
        }

    protected:

        bool m_elementalfire, m_wrath, m_hammer, m_meltweapon, m_summonsons;
        SpellInfo* info_elementalfire, *info_wrath, *info_hammer, *info_meltweapon, *info_summonsons;
};

/*
\todo
 - Fix spells for all mob/boss (spell id, % chance, cooldowns, range, etc.)
 - Lava Spawn doesn't split
 - Core Hound packs aren't in pack, so they don't rez each other
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Molten Giant AI Script
const uint32 CN_MOLTENGIANT = 11658;
const uint32 MOLTENGIANT_STOMP = 31900;   //to verify
const uint32 MOLTENGIANT_KNOCKBACK = 30056;   //to verify

class MoltenGiantAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(MoltenGiantAI, MoonScriptCreatureAI);
        MoltenGiantAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(MOLTENGIANT_STOMP, Target_Current, 10, 0, 5);
            AddSpell(MOLTENGIANT_KNOCKBACK, Target_Self, 10, 0, 5);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Molten Destroyer AI Script
const uint32 CN_MOLTENDESTROYER = 11659;
const uint32 MOLTENDESTROYER_MASSIVE_TREMOR = 19129;    //to verify
//const uint32 MOLTENDESTROYER_SMASH_ATTACK    ?
const uint32 MOLTENDESTROYER_KNOCKDOWN = 13360;    //wrong, fixme!

class MoltenDestroyerAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(MoltenDestroyerAI, MoonScriptCreatureAI);
        MoltenDestroyerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(MOLTENDESTROYER_MASSIVE_TREMOR, Target_Self, 12.5f, 0, 0);
//        AddSpell(MOLTENDESTROYER_SMASH_ATTACK, Target_Self, 10, 0, 0);
            AddSpell(MOLTENDESTROYER_KNOCKDOWN, Target_Current, 12.5f, 0, 0);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Firelord AI Script
const uint32 CN_FIRELORD = 11668;
const uint32 FIRELORD_SUMMON_LAVA_SPAWN = 19392;
const uint32 FIRELORD_SOUL_BURN = 19393;

class FirelordAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(FirelordAI, MoonScriptCreatureAI);
        FirelordAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(FIRELORD_SUMMON_LAVA_SPAWN, Target_Self, 20, 0, 10);
            AddSpell(FIRELORD_SOUL_BURN, Target_RandomPlayer, 20, 0, 5);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Lava Annihilator AI Script
const uint32 CN_LAVAANNIHILATOR = 11665;

class LavaAnnihilatorAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(LavaAnnihilatorAI, MoonScriptCreatureAI);
        LavaAnnihilatorAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpellFunc(&SpellFunc_ClearHateList, Target_Self, 20, 0, 0);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Ancient Core Hound AI Script
const uint32 CN_ANCIENTCOREHOUND = 11673;
const uint32 ANCIENTCOREHOUND_LAVA_BREATH = 19272;
const uint32 ANCIENTCOREHOUND_VICIOUS_BITE = 19319;
const uint32 ANCIENTCOREHOUND_GROUND_STOMP = 19364;
const uint32 ANCIENTCOREHOUND_ANCIENT_DREAD = 19365;
const uint32 ANCIENTCOREHOUND_ANCIENT_DESPAIR = 19369;
const uint32 ANCIENTCOREHOUND_CAUTERIZING_FLAMES = 19366;
const uint32 ANCIENTCOREHOUND_WITHERING_HEAT = 19367;
const uint32 ANCIENTCOREHOUND_ANCIENT_HYSTERIA = 19372;

class AncientCoreHoundAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(AncientCoreHoundAI, MoonScriptCreatureAI);
        AncientCoreHoundAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(ANCIENTCOREHOUND_LAVA_BREATH, Target_Self, 20, 0, 3);
            AddSpell(ANCIENTCOREHOUND_VICIOUS_BITE, Target_Self, 20, 0, 0);

            //Each Ancient Core Hound have only one of the following spell
            switch (RandomUInt(5))
            {
                case 0:
                    AddSpell(ANCIENTCOREHOUND_GROUND_STOMP, Target_Self, 20, 0, 15);
                    break;
                case 1:
                    AddSpell(ANCIENTCOREHOUND_ANCIENT_DREAD, Target_Self, 20, 0, 15);
                    break;
                case 2:
                    AddSpell(ANCIENTCOREHOUND_ANCIENT_DESPAIR, Target_Self, 20, 0, 15);
                    break;
                case 3:
                    AddSpell(ANCIENTCOREHOUND_CAUTERIZING_FLAMES, Target_Self, 20, 0, 15);
                    break;
                case 4:
                    AddSpell(ANCIENTCOREHOUND_WITHERING_HEAT, Target_Self, 20, 0, 15);
                    break;
                case 5:
                    AddSpell(ANCIENTCOREHOUND_ANCIENT_HYSTERIA, Target_Self, 20, 0, 15);
                    break;
            }
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Lava Surger AI Script
const uint32 CN_LAVASURGER = 12101;
const uint32 LAVASURGER_SURGE = 25787;

class LavaSurgerAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(LavaSurgerAI, MoonScriptCreatureAI);
        LavaSurgerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(LAVASURGER_SURGE, Target_RandomUnit, 20, 0, 5, 0, 40);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Flame Imp AI Script
const uint32 CN_FLAMEIMP = 11669;
const uint32 FLAMEIMP_FIRE_NOVA = 20602;   //corrected http://www.wowhead.com/?npc=11669#abilities

class FlameImpAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(FlameImpAI, MoonScriptCreatureAI);
        FlameImpAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(FLAMEIMP_FIRE_NOVA, Target_Current, 25, 0, 0);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Core Hound AI Script
const uint32 CN_COREHOUND = 11671;
const uint32 COREHOUND_SERRATED_BITE = 19771;

class CoreHoundAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(CoreHoundAI, MoonScriptCreatureAI);
        CoreHoundAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(COREHOUND_SERRATED_BITE, Target_RandomPlayer, 10, 0, 0, 0, 10);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Lava Reaver AI Script
const uint32 CN_LAVAREAVER = 12100;
const uint32 LAVAREAVER_CLEAVE = 20691;

class LavaReaverAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(LavaReaverAI, MoonScriptCreatureAI);
        LavaReaverAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(LAVAREAVER_CLEAVE, Target_Current, 20, 0, 0, 0, 15);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Lava Elemental AI Script
const uint32 CN_LAVAELEMENTAL = 12076;
const uint32 LAVAELEMENTAL_PYROCLAST_BARRAGE = 19641;

class LavaElementalAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(LavaElementalAI, MoonScriptCreatureAI);
        LavaElementalAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(LAVAELEMENTAL_PYROCLAST_BARRAGE, Target_Self, 10, 0, 10);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Flameguard AI Script
const uint32 CN_FLAMEGUARD = 11667;
const uint32 FLAMEGUARD_FIRE_SHIELD = 19627;
const uint32 FLAMEGUARD_FLAMES = 19628;

class FlameguardAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(FlameguardAI, MoonScriptCreatureAI);
        FlameguardAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(FLAMEGUARD_FIRE_SHIELD, Target_Self, 100, 0, 0);
            mFlames = AddSpell(FLAMEGUARD_FLAMES, Target_Self, 0, 0, 0);
        }

        void OnDied(Unit* pKiller)
        {
            CastSpellNowNoScheduling(mFlames);
            ParentClass::OnDied(pKiller);
        }

        SpellDesc* mFlames;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Firewalker AI Script
const uint32 CN_FIREWALKER = 11666;
const uint32 FIREWALKER_MELT_ARMOR = 19631;
const uint32 FIREWALKER_INCITE_FLAMES = 19635;
const uint32 FIREWALKER_FIRE_BLOSSOM = 19636; //Added

class FirewalkerAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(FirewalkerAI, MoonScriptCreatureAI);
        FirewalkerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(FIREWALKER_MELT_ARMOR, Target_Self, 10, 0, 0);
            AddSpell(FIREWALKER_INCITE_FLAMES, Target_Self, 10, 0, 0);
            AddSpell(FIREWALKER_FIRE_BLOSSOM, Target_Self, 10, 0, 0);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Lucifron AI Script
const uint32 CN_LUCIFRON = 12118;
const uint32 LUCIFRON_IMPEDING_DOOM = 19702;
const uint32 LUCIFRON_LUCIFRONS_CURSE = 19703;
const uint32 LUCIFRON_SHADOW_SHOCK = 20603;

class LucifronAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(LucifronAI, MoonScriptCreatureAI);
        LucifronAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(LUCIFRON_IMPEDING_DOOM, Target_Self, 8, 0, 0);
            AddSpell(LUCIFRON_LUCIFRONS_CURSE, Target_Self, 8, 0, 0);
            AddSpell(LUCIFRON_SHADOW_SHOCK, Target_Self, 8, 0, 0);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Flamewaker Protector AI Script
const uint32 CN_FLAMEWAKERPROTECTOR = 12119;
const uint32 FLAMEWAKERPROTECTOR_CLEAVE = 20691;
const uint32 FLAMEWAKERPROTECTOR_DOMINATE_MIND = 20740;    //to verify

class FlamewakerProtectorAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(FlamewakerProtectorAI, MoonScriptCreatureAI);
        FlamewakerProtectorAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(FLAMEWAKERPROTECTOR_CLEAVE, Target_Current, 8, 0, 0, 0, 15);
            AddSpell(FLAMEWAKERPROTECTOR_DOMINATE_MIND, Target_RandomPlayer, 4, 0, 0, 0, 20);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Magmadar AI Script
const uint32 CN_MAGMADAR = 11982;
const uint32 MAGMADAR_MAGMA_SPIT = 19450;   //aura doesnt work
const uint32 MAGMADAR_LAVA_BREATH = 19272;   //to verify
const uint32 MAGMADAR_PANIC = 19408;
const uint32 MAGMADAR_LAVA_BOMB = 19411;    //need dummy spell

class MagmadarAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(MagmadarAI, MoonScriptCreatureAI);
        MagmadarAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(MAGMADAR_MAGMA_SPIT, Target_Self, 8, 0, 0);
            AddSpell(MAGMADAR_LAVA_BREATH, Target_Self, 8, 0, 0);
            AddSpell(MAGMADAR_PANIC, Target_Self, 8, 0, 0);
            AddSpell(MAGMADAR_LAVA_BOMB, Target_RandomPlayer, 8, 0, 0, 0, 100);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Gehennas AI Script
const uint32 CN_GEHENNAS = 12259;
const uint32 GEHENNAS_SHADOW_BOLT = 29317;   //to verify
const uint32 GEHENNAS_GEHENNAS_CURSE = 19716;
const uint32 GEHENNAS_RAIN_OF_FIRE = 19717;

class GehennasAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(GehennasAI, MoonScriptCreatureAI);
        GehennasAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(GEHENNAS_SHADOW_BOLT, Target_RandomPlayer, 8, 0, 0, 0, 45);
            AddSpell(GEHENNAS_GEHENNAS_CURSE, Target_Self, 8, 0, 0);
            AddSpell(GEHENNAS_RAIN_OF_FIRE, Target_RandomPlayerDestination, 4, 0, 0, 0, 40);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Flamewaker AI Script
const uint32 CN_FLAMEWAKER = 11661;
const uint32 FLAMEWAKER_SUNDER_ARMOR = 25051;
const uint32 FLAMEWAKER_FIST_OF_RAGNAROS = 20277;
const uint32 FLAMEWAKER_STRIKE = 11998;

class FlamewakerAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(FlamewakerAI, MoonScriptCreatureAI);
        FlamewakerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(FLAMEWAKER_SUNDER_ARMOR, Target_Current, 8, 0, 0);
            AddSpell(FLAMEWAKER_FIST_OF_RAGNAROS, Target_Self, 8, 0, 0);
            AddSpell(FLAMEWAKER_STRIKE, Target_Current, 14, 0, 0);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Garr AI Script
const uint32 CN_GARR = 12057;
const uint32 GARR_ANTIMAGIC_PULSE = 19492;
const uint32 GARR_MAGMA_SHACKES = 19496;

class GarrAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(GarrAI, MoonScriptCreatureAI);
        GarrAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(GARR_ANTIMAGIC_PULSE, Target_Self, 10, 0, 0);
            AddSpell(GARR_MAGMA_SHACKES, Target_Self, 10, 0, 0);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Firesworn AI Script
const uint32 CN_FIRESWORN = 12099;
const uint32 FIRESWORN_IMMOLATE = 20294;
const uint32 FIRESWORN_ERUPTION = 19497;
const uint32 FIRESWORN_SEPARATION_ANXIETY = 23492;

class FireswornAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(FireswornAI, MoonScriptCreatureAI);
        FireswornAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            mGarr = NULL;

            //Spells
            AddSpell(FIRESWORN_IMMOLATE, Target_Current, 10, 0, 0, 0, 40);
            mEruption = AddSpell(FIRESWORN_ERUPTION, Target_Self, 0, 0, 0);
            mSeparationAnxiety = AddSpell(FIRESWORN_SEPARATION_ANXIETY, Target_Self, 0, 5, 5);
        }

        void OnCombatStart(Unit* pTarget)
        {
            mGarr = static_cast< MoonScriptCreatureAI* >(GetNearestCreature(CN_GARR));
            ParentClass::OnCombatStart(pTarget);
        }

        void OnDied(Unit* pKiller)
        {
            CastSpellNowNoScheduling(mEruption);
            ParentClass::OnDied(pKiller);
        }

        void AIUpdate()
        {
            if (mGarr && mGarr->isAlive() && getRangeToObject(mGarr->getCreature()) > 100)
            {
                CastSpell(mSeparationAnxiety);
            }
            ParentClass::AIUpdate();
        }

        SpellDesc* mEruption;
        SpellDesc* mSeparationAnxiety;
        MoonScriptCreatureAI* mGarr;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Baron Geddon AI Script
const uint32 CN_BARONGEDDON = 12056;
const uint32 BARONGEDDON_INFERNO = 19698;    //35268
const uint32 BARONGEDDON_IGNITE_MANA = 19659;
const uint32 BARONGEDDON_LIVING_BOMB = 20475;

class BaronGeddonAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(BaronGeddonAI, MoonScriptCreatureAI);
        BaronGeddonAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(BARONGEDDON_INFERNO, Target_Self, 8, 0, 0);
            AddSpell(BARONGEDDON_IGNITE_MANA, Target_Self, 8, 0, 0);
            AddSpell(BARONGEDDON_LIVING_BOMB, Target_RandomPlayer, 8, 0, 0, 0, 45);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Shazzrah AI Script
const uint32 CN_SHAZZRAH = 12264;
const uint32 SHAZZRAH_ARCANE_EXPLOSION = 19712;
const uint32 SHAZZRAH_SHAZZRAHS_CURSE = 19713;
const uint32 SHAZZRAH_MAGIC_GROUNDING = 19714;
const uint32 SHAZZRAH_COUNTERSPELL = 19715;
const uint32 SHAZZRAH_BLINK = 29883;    //dummy spell, need to be coded in core

void SpellFunc_ShazzrahBlinkArcaneExplosions(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType);

class ShazzrahAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(ShazzrahAI, MoonScriptCreatureAI);
        ShazzrahAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(SHAZZRAH_SHAZZRAHS_CURSE, Target_Self, 8, 0, 0);
            AddSpell(SHAZZRAH_MAGIC_GROUNDING, Target_Self, 6, 0, 0);
            AddSpell(SHAZZRAH_COUNTERSPELL, Target_Self, 6, 0, 0);

            mBlink = AddSpell(SHAZZRAH_BLINK, Target_RandomPlayer, 0, 0, 0);
            mArcaneExplosion = AddSpell(SHAZZRAH_ARCANE_EXPLOSION, Target_Self, 0, 0, 0);
            AddSpellFunc(&SpellFunc_ShazzrahBlinkArcaneExplosions, Target_Self, 8, -1, 15);
        }

        SpellDesc* mBlink;
        SpellDesc* mArcaneExplosion;
};

void SpellFunc_ShazzrahBlinkArcaneExplosions(SpellDesc* pThis, MoonScriptCreatureAI* pCreatureAI, Unit* pTarget, TargetType pType)
{
    ShazzrahAI* Shazzrah = (pCreatureAI) ? static_cast< ShazzrahAI* >(pCreatureAI) : NULL;
    if (Shazzrah)
    {
        //Teleport blink, then cast 4 arcane explosions
        Shazzrah->CastSpell(Shazzrah->mBlink);
        for (uint8 Iter = 0; Iter < 4; Iter++)
            Shazzrah->CastSpell(Shazzrah->mArcaneExplosion);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Golemagg AI Script
const uint32 CN_GOLEMAGG = 11988;
const uint32 GOLEMAGG_GOLEMAGGS_TRUST = 20553;
const uint32 GOLEMAGG_MAGMA_SPLASH = 13880;
const uint32 GOLEMAGG_PYROBLAST = 20228;
const uint32 GOLEMAGG_EARTHQUAKE = 19798;

class GolemaggAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(GolemaggAI, MoonScriptCreatureAI);
        GolemaggAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(GOLEMAGG_GOLEMAGGS_TRUST, Target_Self, 8, 0, 0);
            AddSpell(GOLEMAGG_MAGMA_SPLASH, Target_Self, 8, 0, 0);
            AddSpell(GOLEMAGG_PYROBLAST, Target_RandomPlayer, 8, 0, 0, 0, 40);
            AddSpell(GOLEMAGG_EARTHQUAKE, Target_Self, 8, 0, 0);
        }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Register
void SetupMoltenCore(ScriptMgr* pScriptMgr)
{
    pScriptMgr->register_creature_script(CN_MOLTENGIANT, &MoltenGiantAI::Create);
    pScriptMgr->register_creature_script(CN_MOLTENDESTROYER, &MoltenDestroyerAI::Create);
    pScriptMgr->register_creature_script(CN_FIRELORD, &FirelordAI::Create);
    pScriptMgr->register_creature_script(CN_LAVAANNIHILATOR, &LavaAnnihilatorAI::Create);
    pScriptMgr->register_creature_script(CN_ANCIENTCOREHOUND, &AncientCoreHoundAI::Create);
    pScriptMgr->register_creature_script(CN_LAVASURGER, &LavaSurgerAI::Create);
    pScriptMgr->register_creature_script(CN_FLAMEIMP, &FlameImpAI::Create);
    pScriptMgr->register_creature_script(CN_COREHOUND, &CoreHoundAI::Create);
    pScriptMgr->register_creature_script(CN_LAVAREAVER, &LavaReaverAI::Create);
    pScriptMgr->register_creature_script(CN_LAVAELEMENTAL, &LavaElementalAI::Create);
    pScriptMgr->register_creature_script(CN_FLAMEGUARD, &FlameguardAI::Create);
    pScriptMgr->register_creature_script(CN_FIREWALKER, &FirewalkerAI::Create);
    pScriptMgr->register_creature_script(CN_LUCIFRON, &LucifronAI::Create);
    pScriptMgr->register_creature_script(CN_FLAMEWAKERPROTECTOR, &FlamewakerProtectorAI::Create);
    pScriptMgr->register_creature_script(CN_MAGMADAR, &MagmadarAI::Create);
    pScriptMgr->register_creature_script(CN_GEHENNAS, &GehennasAI::Create);
    pScriptMgr->register_creature_script(CN_FLAMEWAKER, &FlamewakerAI::Create);
    pScriptMgr->register_creature_script(CN_GARR, &GarrAI::Create);
    pScriptMgr->register_creature_script(CN_FIRESWORN, &FireswornAI::Create);
    pScriptMgr->register_creature_script(CN_BARONGEDDON, &BaronGeddonAI::Create);
    pScriptMgr->register_creature_script(CN_SHAZZRAH, &ShazzrahAI::Create);

    pScriptMgr->register_creature_script(CN_GOLEMAGG, &GolemaggAI::Create);
    pScriptMgr->register_creature_script(CN_CORERAGER, &CoreRagerAI::Create);

    pScriptMgr->register_creature_script(CN_SULFURON_HARBRINGER, &SulfuronAI::Create);
    pScriptMgr->register_creature_script(CN_RAGNAROS, &RagnarosAI::Create);
}
