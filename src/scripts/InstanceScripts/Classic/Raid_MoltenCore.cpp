/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/Script/CreatureAIScript.h"
#include "Macros/ScriptMacros.hpp"

class MoltenCoreInstanceScript : public InstanceScript
{
public:

    explicit MoltenCoreInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new MoltenCoreInstanceScript(pMapMgr); }
};


const uint32_t CN_CORERAGER = 11672;

const uint32_t MANGLE = 19820; // 1 target
// put full HP if less 50% and golemagg is still alive

class CoreRagerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CoreRagerAI)
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
        uint32_t val = Util::getRandomUInt(1000);
        SpellCast(val);
    }

    void SpellCast(uint32_t val)
    {
        if (!getCreature()->isCastingSpell() && getCreature()->getThreatManager().getCurrentVictim())//_unit->getAttackTarget())
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

const uint32_t CN_SULFURON_HARBRINGER = 12098;

const uint32_t DEMORALIZING_SHOUT = 19778;
const uint32_t INSPIRE = 19779;
const uint32_t FLAME_SPEAR = 19781;
// needs a aoe knockback 19780?

class SulfuronAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SulfuronAI)
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
        uint32_t val = Util::getRandomUInt(1000);
        SpellCast(val);
    }

    void SpellCast(uint32_t val)
    {
        if (!getCreature()->isCastingSpell() && getCreature()->getThreatManager().getCurrentVictim())//_unit->getAttackTarget())
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

const uint32_t CN_RAGNAROS = 11502;

const uint32_t ELEMENTAL_FIRE = 20563; // 1 target
const uint32_t MAGMA_BLAST = 20565; // various targets on not attacked. -> sound -> 8048 ?
const uint32_t WRATH_OF_RAGNAROS = 20566;  // Fly bitches fly! quote -> "Taste the Flames of Sulfuron!" -> sound 8047
const uint32_t HAMMER_OF_RAGNAROS = 19780; // quote -> "By fire be purged!" -> sound 8046
const uint32_t MELT_WEAPON = 21387; // 1 target
const uint32_t SUMMON_SONS_OF_FLAMES = 21108; //\todo  DUMMY :P summon the sons of flames. entry = 12143 -> sound 8049,8050


//Ragnaros Submerge Visual -> 20567
//Ragnaros Emerge -> 20568

class RagnarosAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RagnarosAI)
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
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        sendDBChatMessage(3053);     // Die, insect!
    }

    void AIUpdate() override
    {
        uint32_t val = Util::getRandomUInt(1000);
        SpellCast(val);
    }

    void SpellCast(uint32_t val)
    {
        if (!getCreature()->isCastingSpell() && getCreature()->getThreatManager().getCurrentVictim())//_unit->getAttackTarget())
        {
            Unit* target = getCreature()->getThreatManager().getCurrentVictim();

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


const uint32_t CN_ANCIENTCOREHOUND = 11673;
const uint32_t ANCIENTCOREHOUND_GROUND_STOMP = 19364;
const uint32_t ANCIENTCOREHOUND_ANCIENT_DREAD = 19365;
const uint32_t ANCIENTCOREHOUND_ANCIENT_DESPAIR = 19369;
const uint32_t ANCIENTCOREHOUND_CAUTERIZING_FLAMES = 19366;
const uint32_t ANCIENTCOREHOUND_WITHERING_HEAT = 19367;
const uint32_t ANCIENTCOREHOUND_ANCIENT_HYSTERIA = 19372;

class AncientCoreHoundAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AncientCoreHoundAI)
    explicit AncientCoreHoundAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
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

const uint32_t CN_GARR = 12057;
const uint32_t CN_FIRESWORN = 12099;
const uint32_t FIRESWORN_SEPARATION_ANXIETY = 23492;

class FireswornAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FireswornAI)
    explicit FireswornAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mGarr = nullptr;

        mSeparationAnxiety = addAISpell(FIRESWORN_SEPARATION_ANXIETY, 0.0f, TARGET_SELF, 5, 5);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mGarr = getNearestCreatureAI(CN_GARR);
    }

    void AIUpdate() override
    {
        if (mGarr && mGarr->isAlive() && getRangeToObject(mGarr->getCreature()) > 100)
        {
            _castAISpell(mSeparationAnxiety);
        }
    }

    CreatureAISpells* mSeparationAnxiety;
    CreatureAIScript* mGarr;
};

const uint32_t SHAZZRAH_ARCANE_EXPLOSION = 19712;
const uint32_t SHAZZRAH_BLINK = 29883;    //dummy spell, need to be coded in core

class ShazzrahAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShazzrahAI)
    explicit ShazzrahAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mArcaneExplosion = addAISpell(SHAZZRAH_ARCANE_EXPLOSION, 8.0f, TARGET_SELF, 0, 0);
    }

    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == SHAZZRAH_BLINK)
        {
            for (uint8_t Iter = 0; Iter < 4; Iter++)
                _castAISpell(mArcaneExplosion);
        }
    }

    CreatureAISpells* mArcaneExplosion;
};


void SetupMoltenCore(ScriptMgr* pScriptMgr)
{
    pScriptMgr->register_instance_script(MAP_MOLTEN_CORE, &MoltenCoreInstanceScript::Create);

    pScriptMgr->register_creature_script(CN_ANCIENTCOREHOUND, &AncientCoreHoundAI::Create);

    pScriptMgr->register_creature_script(CN_FIRESWORN, &FireswornAI::Create);
    pScriptMgr->register_creature_script(CN_CORERAGER, &CoreRagerAI::Create);

    pScriptMgr->register_creature_script(CN_SULFURON_HARBRINGER, &SulfuronAI::Create);
    pScriptMgr->register_creature_script(CN_RAGNAROS, &RagnarosAI::Create);
}
