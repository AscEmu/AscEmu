/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

class CoreRagerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CoreRagerAI);
    explicit CoreRagerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_mangle = true;

        info_mangle = sSpellMgr.getSpellInfo(MANGLE);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(getCreature()->getBaseAttackTime(MELEE));
    }

    void AIUpdate() override
    {
        uint32 val = Util::getRandomUInt(1000);
        SpellCast(val);
    }

    void SpellCast(uint32 val)
    {
        if (!getCreature()->isCastingSpell() && getCreature()->GetAIInterface()->getNextTarget())//_unit->getAttackTarget())
        {
            //Unit* target = _unit->GetAIInterface()->GetNextTarget();
            if (m_mangle)
            {
                getCreature()->castSpell(getCreature(), info_mangle, false);
                m_mangle = false;
                return;
            }

            if (val >= 100 && val <= 220)
            {
                getCreature()->setAttackTimer(MELEE, 9000);
                m_mangle = true;
            }
        }
    }

protected:

    bool m_mangle;
    SpellInfo const* info_mangle;
};

const uint32 CN_SULFURON_HARBRINGER = 12098;

const uint32 DEMORALIZING_SHOUT = 19778;
const uint32 INSPIRE = 19779;
const uint32 FLAME_SPEAR = 19781;
// needs a aoe knockback 19780?

class SulfuronAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SulfuronAI);
    explicit SulfuronAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_demoralizingshout = m_inspire = m_flamespear = true;

        info_demoralizingshout = sSpellMgr.getSpellInfo(DEMORALIZING_SHOUT);
        info_inspire = sSpellMgr.getSpellInfo(INSPIRE);
        info_flamespear = sSpellMgr.getSpellInfo(FLAME_SPEAR);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(getCreature()->getBaseAttackTime(MELEE));
    }

    void AIUpdate() override
    {
        uint32 val = Util::getRandomUInt(1000);
        SpellCast(val);
    }

    void SpellCast(uint32 val)
    {
        if (!getCreature()->isCastingSpell() && getCreature()->GetAIInterface()->getNextTarget())//_unit->getAttackTarget())
        {
            //Unit* target = _unit->GetAIInterface()->GetNextTarget();

            if (m_demoralizingshout)
            {
                getCreature()->castSpell(getCreature(), info_demoralizingshout, false);
                m_demoralizingshout = false;
                return;
            }

            if (m_inspire)
            {
                getCreature()->castSpell(getCreature(), info_inspire, false);
                m_inspire = false;
                return;
            }

            if (m_flamespear)
            {
                getCreature()->castSpell(getCreature(), info_flamespear, false);
                m_flamespear = false;
                return;
            }

            if (val >= 100 && val <= 180)
            {
                getCreature()->setAttackTimer(MELEE, 1000);
                m_inspire = true;
            }

            if (val > 180 && val <= 260)
            {
                getCreature()->setAttackTimer(MELEE, 1000);
                m_demoralizingshout = true;
            }

            if (val > 260 && val <= 320)
            {
                getCreature()->setAttackTimer(MELEE, 1000);
                m_flamespear = true;
            }
        }
    }

protected:

    bool m_demoralizingshout, m_inspire, m_flamespear;
    SpellInfo const* info_demoralizingshout, *info_inspire, *info_flamespear;
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

class RagnarosAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RagnarosAI);
    explicit RagnarosAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_elementalfire = m_wrath = m_hammer = m_meltweapon = m_summonsons = true;

        info_elementalfire = sSpellMgr.getSpellInfo(ELEMENTAL_FIRE);
        info_wrath = sSpellMgr.getSpellInfo(WRATH_OF_RAGNAROS);
        info_hammer = sSpellMgr.getSpellInfo(HAMMER_OF_RAGNAROS);
        info_meltweapon = sSpellMgr.getSpellInfo(MELT_WEAPON);
        info_summonsons = sSpellMgr.getSpellInfo(SUMMON_SONS_OF_FLAMES);
        getCreature()->setMoveRoot(true);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(getCreature()->getBaseAttackTime(MELEE));
        getCreature()->GetAIInterface()->skip_reset_hp = true;
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(3053);     // Die, insect!
    }

    void AIUpdate() override
    {
        uint32 val = Util::getRandomUInt(1000);
        SpellCast(val);
    }

    void SpellCast(uint32 val)
    {
        if (!getCreature()->isCastingSpell() && getCreature()->GetAIInterface()->getNextTarget())//_unit->getAttackTarget())
        {
            Unit* target = getCreature()->GetAIInterface()->getNextTarget();

            if (m_elementalfire)
            {
                getCreature()->castSpell(target, info_elementalfire, false);
                m_elementalfire = false;
                return;
            }

            if (m_wrath)
            {
                sendDBChatMessage(3052);     // TASTE THE FLAMES OF SULFURON!
                getCreature()->castSpell(getCreature(), info_wrath, false);
                m_wrath = false;
                return;
            }

            if (m_hammer)
            {
                sendDBChatMessage(3051);     // By fire be purged!
                getCreature()->castSpell(getCreature(), info_hammer, false);
                m_hammer = false;
                return;
            }

            if (m_meltweapon)
            {
                getCreature()->castSpell(target, info_meltweapon, false);
                m_meltweapon = false;
                return;
            }

            if (val >= 100 && val <= 160)
            {
                getCreature()->setAttackTimer(MELEE, 1000);
                m_elementalfire = true;
            }

            if (val > 160 && val <= 220)
            {
                getCreature()->setAttackTimer(MELEE, 1000);
                m_wrath = true;
            }

            if (val > 220 && val <= 280)
            {
                getCreature()->setAttackTimer(MELEE, 1000);
                m_hammer = true;
            }
            if (val > 340 && val <= 400)
            {
                getCreature()->setAttackTimer(MELEE, 1000);
                m_meltweapon = true;
            }

        }
    }

protected:

    bool m_elementalfire, m_wrath, m_hammer, m_meltweapon, m_summonsons;
    SpellInfo const* info_elementalfire, *info_wrath, *info_hammer, *info_meltweapon, *info_summonsons;
};

/*
\todo
 - Fix spells for all mob/boss (spell id, % chance, cooldowns, range, etc.)
 - Lava Spawn doesn't split
 - Core Hound packs aren't in pack, so they don't rez each other
*/

const uint32 CN_MOLTENGIANT = 11658;
const uint32 MOLTENGIANT_STOMP = 31900;   //to verify
const uint32 MOLTENGIANT_KNOCKBACK = 30056;   //to verify

class MoltenGiantAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MoltenGiantAI);
    explicit MoltenGiantAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(MOLTENGIANT_STOMP, 10.0f, TARGET_ATTACKING, 0, 5);
        addAISpell(MOLTENGIANT_KNOCKBACK, 10.0f, TARGET_SELF, 0, 5);
    }
};

const uint32 CN_MOLTENDESTROYER = 11659;
const uint32 MOLTENDESTROYER_MASSIVE_TREMOR = 19129;    //to verify
//const uint32 MOLTENDESTROYER_SMASH_ATTACK    ?
const uint32 MOLTENDESTROYER_KNOCKDOWN = 13360;    //wrong, fixme!

class MoltenDestroyerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MoltenDestroyerAI);
    explicit MoltenDestroyerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(MOLTENDESTROYER_MASSIVE_TREMOR, 12.5f, TARGET_SELF, 0, 0);
        addAISpell(MOLTENDESTROYER_KNOCKDOWN, 12.5f, TARGET_ATTACKING, 0, 0);
    }
};

const uint32 CN_FIRELORD = 11668;
const uint32 FIRELORD_SUMMON_LAVA_SPAWN = 19392;
const uint32 FIRELORD_SOUL_BURN = 19393;

class FirelordAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FirelordAI);
    explicit FirelordAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(FIRELORD_SUMMON_LAVA_SPAWN, 20.0f, TARGET_SELF, 0, 10);
        addAISpell(FIRELORD_SOUL_BURN, 20.0f, TARGET_RANDOM_SINGLE, 0, 5);
    }
};


const uint32 CN_ANCIENTCOREHOUND = 11673;
const uint32 ANCIENTCOREHOUND_LAVA_BREATH = 19272;
const uint32 ANCIENTCOREHOUND_VICIOUS_BITE = 19319;
const uint32 ANCIENTCOREHOUND_GROUND_STOMP = 19364;
const uint32 ANCIENTCOREHOUND_ANCIENT_DREAD = 19365;
const uint32 ANCIENTCOREHOUND_ANCIENT_DESPAIR = 19369;
const uint32 ANCIENTCOREHOUND_CAUTERIZING_FLAMES = 19366;
const uint32 ANCIENTCOREHOUND_WITHERING_HEAT = 19367;
const uint32 ANCIENTCOREHOUND_ANCIENT_HYSTERIA = 19372;

class AncientCoreHoundAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AncientCoreHoundAI);
    explicit AncientCoreHoundAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ANCIENTCOREHOUND_LAVA_BREATH, 20.0f, TARGET_SELF, 0, 3);
        addAISpell(ANCIENTCOREHOUND_VICIOUS_BITE, 20.0f, TARGET_SELF, 0, 0);

        //Each Ancient Core Hound have only one of the following spell
        switch (Util::getRandomUInt(5))
        {
            case 0:
                addAISpell(ANCIENTCOREHOUND_GROUND_STOMP, 20.0f, TARGET_SELF, 0, 15);
                break;
            case 1:
                addAISpell(ANCIENTCOREHOUND_ANCIENT_DREAD, 20.0f, TARGET_SELF, 0, 15);
                break;
            case 2:
                addAISpell(ANCIENTCOREHOUND_ANCIENT_DESPAIR, 20.0f, TARGET_SELF, 0, 15);
                break;
            case 3:
                addAISpell(ANCIENTCOREHOUND_CAUTERIZING_FLAMES, 20.0f, TARGET_SELF, 0, 15);
                break;
            case 4:
                addAISpell(ANCIENTCOREHOUND_WITHERING_HEAT, 20.0f, TARGET_SELF, 0, 15);
                break;
            case 5:
                addAISpell(ANCIENTCOREHOUND_ANCIENT_HYSTERIA, 20.0f, TARGET_SELF, 0, 15);
                break;
        }
    }
};

const uint32 CN_LAVASURGER = 12101;
const uint32 LAVASURGER_SURGE = 25787;

class LavaSurgerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LavaSurgerAI);
    explicit LavaSurgerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(LAVASURGER_SURGE, 20.0f, TARGET_RANDOM_SINGLE, 0, 5);
    }
};

const uint32 CN_FLAMEIMP = 11669;
const uint32 FLAMEIMP_FIRE_NOVA = 20602;   //corrected http://www.wowhead.com/?npc=11669#abilities

class FlameImpAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FlameImpAI);
    explicit FlameImpAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(FLAMEIMP_FIRE_NOVA, 25.0f, TARGET_ATTACKING, 0, 0);
    }
};

const uint32 CN_COREHOUND = 11671;
const uint32 COREHOUND_SERRATED_BITE = 19771;

class CoreHoundAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CoreHoundAI);
    explicit CoreHoundAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(COREHOUND_SERRATED_BITE, 10.0f, TARGET_RANDOM_SINGLE, 0, 0);
    }
};

const uint32 CN_LAVAREAVER = 12100;
const uint32 LAVAREAVER_CLEAVE = 20691;

class LavaReaverAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LavaReaverAI);
    explicit LavaReaverAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(LAVAREAVER_CLEAVE, 20.0f, TARGET_ATTACKING, 0, 0);
    }
};

const uint32 CN_LAVAELEMENTAL = 12076;
const uint32 LAVAELEMENTAL_PYROCLAST_BARRAGE = 19641;

class LavaElementalAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LavaElementalAI);
    explicit LavaElementalAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(LAVAELEMENTAL_PYROCLAST_BARRAGE, 10.0f, TARGET_SELF, 0, 10);
    }
};

const uint32 CN_FLAMEGUARD = 11667;
const uint32 FLAMEGUARD_FIRE_SHIELD = 19627;
const uint32 FLAMEGUARD_FLAMES = 19628;

class FlameguardAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FlameguardAI);
    explicit FlameguardAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(FLAMEGUARD_FIRE_SHIELD, 100.0f, TARGET_SELF, 0, 0);
        mFlames = addAISpell(FLAMEGUARD_FLAMES, 0.0f, TARGET_SELF, 0, 0);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        _castAISpell(mFlames);
    }

    CreatureAISpells* mFlames;
};

const uint32 CN_FIREWALKER = 11666;
const uint32 FIREWALKER_MELT_ARMOR = 19631;
const uint32 FIREWALKER_INCITE_FLAMES = 19635;
const uint32 FIREWALKER_FIRE_BLOSSOM = 19636; //Added

class FirewalkerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FirewalkerAI);
    explicit FirewalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(FIREWALKER_MELT_ARMOR, 10.0f, TARGET_SELF, 0, 0);
        addAISpell(FIREWALKER_INCITE_FLAMES, 10.0f, TARGET_SELF, 0, 0);
        addAISpell(FIREWALKER_FIRE_BLOSSOM, 10.0f, TARGET_SELF, 0, 0);
    }
};

const uint32 CN_LUCIFRON = 12118;
const uint32 LUCIFRON_IMPEDING_DOOM = 19702;
const uint32 LUCIFRON_LUCIFRONS_CURSE = 19703;
const uint32 LUCIFRON_SHADOW_SHOCK = 20603;

class LucifronAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LucifronAI);
    explicit LucifronAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(LUCIFRON_IMPEDING_DOOM, 8.0f, TARGET_SELF, 0, 0);
        addAISpell(LUCIFRON_LUCIFRONS_CURSE, 8.0f, TARGET_SELF, 0, 0);
        addAISpell(LUCIFRON_SHADOW_SHOCK, 8.0f, TARGET_SELF, 0, 0);
    }
};

const uint32 CN_FLAMEWAKERPROTECTOR = 12119;
const uint32 FLAMEWAKERPROTECTOR_CLEAVE = 20691;
const uint32 FLAMEWAKERPROTECTOR_DOMINATE_MIND = 20740;    //to verify

class FlamewakerProtectorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FlamewakerProtectorAI);
    explicit FlamewakerProtectorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(FLAMEWAKERPROTECTOR_CLEAVE, 8.0f, TARGET_ATTACKING, 0, 0);
        addAISpell(FLAMEWAKERPROTECTOR_DOMINATE_MIND, 4.0f, TARGET_RANDOM_SINGLE, 0, 0);
    }
};

const uint32 CN_MAGMADAR = 11982;
const uint32 MAGMADAR_MAGMA_SPIT = 19450;   //aura doesnt work
const uint32 MAGMADAR_LAVA_BREATH = 19272;   //to verify
const uint32 MAGMADAR_PANIC = 19408;
const uint32 MAGMADAR_LAVA_BOMB = 19411;    //need dummy spell

class MagmadarAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MagmadarAI);
    explicit MagmadarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(MAGMADAR_MAGMA_SPIT, 8.0f, TARGET_SELF, 0, 0);
        addAISpell(MAGMADAR_LAVA_BREATH, 8.0f, TARGET_SELF, 0, 0);
        addAISpell(MAGMADAR_PANIC, 8.0f, TARGET_SELF, 0, 0);
        addAISpell(MAGMADAR_LAVA_BOMB, 8.0f, TARGET_RANDOM_SINGLE, 0, 0);
    }
};

const uint32 CN_GEHENNAS = 12259;
const uint32 GEHENNAS_SHADOW_BOLT = 29317;   //to verify
const uint32 GEHENNAS_GEHENNAS_CURSE = 19716;
const uint32 GEHENNAS_RAIN_OF_FIRE = 19717;

class GehennasAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GehennasAI);
    explicit GehennasAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(GEHENNAS_SHADOW_BOLT, 8.0f, TARGET_RANDOM_SINGLE, 0, 0);
        addAISpell(GEHENNAS_GEHENNAS_CURSE, 8.0f, TARGET_SELF, 0, 0);
        addAISpell(GEHENNAS_RAIN_OF_FIRE, 4.0f, TARGET_RANDOM_DESTINATION, 0, 0);
    }
};

const uint32 CN_FLAMEWAKER = 11661;
const uint32 FLAMEWAKER_SUNDER_ARMOR = 25051;
const uint32 FLAMEWAKER_FIST_OF_RAGNAROS = 20277;
const uint32 FLAMEWAKER_STRIKE = 11998;

class FlamewakerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FlamewakerAI);
    explicit FlamewakerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(FLAMEWAKER_SUNDER_ARMOR, 8.0f, TARGET_ATTACKING, 0, 0);
        addAISpell(FLAMEWAKER_FIST_OF_RAGNAROS, 8.0f, TARGET_SELF, 0, 0);
        addAISpell(FLAMEWAKER_STRIKE, 14.0f, TARGET_ATTACKING, 0, 0);
    }
};

const uint32 CN_GARR = 12057;
const uint32 GARR_ANTIMAGIC_PULSE = 19492;
const uint32 GARR_MAGMA_SHACKES = 19496;

class GarrAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GarrAI);
    explicit GarrAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(GARR_ANTIMAGIC_PULSE, 10.0f, TARGET_SELF, 0, 0);
        addAISpell(GARR_MAGMA_SHACKES, 10.0f, TARGET_SELF, 0, 0);
    }
};


const uint32 CN_FIRESWORN = 12099;
const uint32 FIRESWORN_IMMOLATE = 20294;
const uint32 FIRESWORN_ERUPTION = 19497;
const uint32 FIRESWORN_SEPARATION_ANXIETY = 23492;

class FireswornAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FireswornAI);
    explicit FireswornAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mGarr = nullptr;

        //Spells
        addAISpell(FIRESWORN_IMMOLATE, 10.0f, TARGET_ATTACKING, 0, 0);
        mEruption = addAISpell(FIRESWORN_ERUPTION, 0.0f, TARGET_SELF, 0, 0);
        mSeparationAnxiety = addAISpell(FIRESWORN_SEPARATION_ANXIETY, 0.0f, TARGET_SELF, 5, 5);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mGarr = getNearestCreatureAI(CN_GARR);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        _castAISpell(mEruption);
    }

    void AIUpdate() override
    {
        if (mGarr && mGarr->isAlive() && getRangeToObject(mGarr->getCreature()) > 100)
        {
            _castAISpell(mSeparationAnxiety);
        }
    }

    CreatureAISpells* mEruption;
    CreatureAISpells* mSeparationAnxiety;
    CreatureAIScript* mGarr;
};

const uint32 CN_BARONGEDDON = 12056;
const uint32 BARONGEDDON_INFERNO = 19698;    //35268
const uint32 BARONGEDDON_IGNITE_MANA = 19659;
const uint32 BARONGEDDON_LIVING_BOMB = 20475;

class BaronGeddonAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BaronGeddonAI);
    explicit BaronGeddonAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BARONGEDDON_INFERNO, 8.0f, TARGET_SELF, 0, 0);
        addAISpell(BARONGEDDON_IGNITE_MANA, 8.0f, TARGET_SELF, 0, 0);
        addAISpell(BARONGEDDON_LIVING_BOMB, 8.0f, TARGET_RANDOM_SINGLE, 0, 0);
    }
};

const uint32 CN_SHAZZRAH = 12264;
const uint32 SHAZZRAH_ARCANE_EXPLOSION = 19712;
const uint32 SHAZZRAH_SHAZZRAHS_CURSE = 19713;
const uint32 SHAZZRAH_MAGIC_GROUNDING = 19714;
const uint32 SHAZZRAH_COUNTERSPELL = 19715;
const uint32 SHAZZRAH_BLINK = 29883;    //dummy spell, need to be coded in core


class ShazzrahAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShazzrahAI);
    explicit ShazzrahAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHAZZRAH_SHAZZRAHS_CURSE, 8.0f, TARGET_SELF, 0, 0);
        addAISpell(SHAZZRAH_MAGIC_GROUNDING, 6.0f, TARGET_SELF, 0, 0);
        addAISpell(SHAZZRAH_COUNTERSPELL, 6.0f, TARGET_SELF, 0, 0);

        mBlink = addAISpell(SHAZZRAH_BLINK, 5.0f, TARGET_RANDOM_SINGLE, 0, 15);
        mArcaneExplosion = addAISpell(SHAZZRAH_ARCANE_EXPLOSION, 8.0f, TARGET_SELF, 0, 0);
    }

    void OnCastSpell(uint32 spellId) override
    {
        if (spellId == SHAZZRAH_BLINK)
        {
            for (uint8 Iter = 0; Iter < 4; Iter++)
                _castAISpell(mArcaneExplosion);
        }
    }

    CreatureAISpells* mBlink;
    CreatureAISpells* mArcaneExplosion;
};


const uint32 CN_GOLEMAGG = 11988;
const uint32 GOLEMAGG_GOLEMAGGS_TRUST = 20553;
const uint32 GOLEMAGG_MAGMA_SPLASH = 13880;
const uint32 GOLEMAGG_PYROBLAST = 20228;
const uint32 GOLEMAGG_EARTHQUAKE = 19798;

class GolemaggAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GolemaggAI);
    explicit GolemaggAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(GOLEMAGG_GOLEMAGGS_TRUST, 8.0f, TARGET_SELF, 0, 0);
        addAISpell(GOLEMAGG_MAGMA_SPLASH, 8.0f, TARGET_SELF, 0, 0);
        addAISpell(GOLEMAGG_PYROBLAST, 8.0f, TARGET_RANDOM_SINGLE, 0, 0);
        addAISpell(GOLEMAGG_EARTHQUAKE, 8.0f, TARGET_SELF, 0, 0);
    }
};

void SetupMoltenCore(ScriptMgr* pScriptMgr)
{
    pScriptMgr->register_creature_script(CN_MOLTENGIANT, &MoltenGiantAI::Create);
    pScriptMgr->register_creature_script(CN_MOLTENDESTROYER, &MoltenDestroyerAI::Create);
    pScriptMgr->register_creature_script(CN_FIRELORD, &FirelordAI::Create);
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
