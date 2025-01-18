/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Raid_MoltenCore.h"

#include "Setup.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Utilities/Random.hpp"

class MoltenCoreInstanceScript : public InstanceScript
{
public:
    explicit MoltenCoreInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) { }
    static InstanceScript* Create(WorldMap* pMapMgr) { return new MoltenCoreInstanceScript(pMapMgr); }
};

class CoreRagerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new CoreRagerAI(c); }
    explicit CoreRagerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_mangle = true;

        info_mangle = sSpellMgr.getSpellInfo(MoltenCore::MANGLE);
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
            //Unit* target = _unit->getAIInterface()->GetNextTarget();
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

class SulfuronAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SulfuronAI(c); }
    explicit SulfuronAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_demoralizingshout = m_inspire = m_flamespear = true;

        info_demoralizingshout = sSpellMgr.getSpellInfo(MoltenCore::DEMORALIZING_SHOUT);
        info_inspire = sSpellMgr.getSpellInfo(MoltenCore::INSPIRE);
        info_flamespear = sSpellMgr.getSpellInfo(MoltenCore::FLAME_SPEAR);
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
            //Unit* target = _unit->getAIInterface()->GetNextTarget();

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

class RagnarosAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RagnarosAI(c); }
    explicit RagnarosAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_elementalfire = m_wrath = m_hammer = m_meltweapon = m_summonsons = true;

        info_elementalfire = sSpellMgr.getSpellInfo(MoltenCore::ELEMENTAL_FIRE);
        info_wrath = sSpellMgr.getSpellInfo(MoltenCore::WRATH_OF_RAGNAROS);
        info_hammer = sSpellMgr.getSpellInfo(MoltenCore::HAMMER_OF_RAGNAROS);
        info_meltweapon = sSpellMgr.getSpellInfo(MoltenCore::MELT_WEAPON);
        info_summonsons = sSpellMgr.getSpellInfo(MoltenCore::SUMMON_SONS_OF_FLAMES);
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

class AncientCoreHoundAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AncientCoreHoundAI(c); }
    explicit AncientCoreHoundAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //Each Ancient Core Hound have only one of the following spell
        switch (Util::getRandomUInt(5))
        {
            case 0:
                addAISpell(MoltenCore::ANCIENTCOREHOUND_GROUND_STOMP, 20.0f, TARGET_SELF, 0, 15);
                break;
            case 1:
                addAISpell(MoltenCore::ANCIENTCOREHOUND_ANCIENT_DREAD, 20.0f, TARGET_SELF, 0, 15);
                break;
            case 2:
                addAISpell(MoltenCore::ANCIENTCOREHOUND_ANCIENT_DESPAIR, 20.0f, TARGET_SELF, 0, 15);
                break;
            case 3:
                addAISpell(MoltenCore::ANCIENTCOREHOUND_CAUTERIZING_FLAMES, 20.0f, TARGET_SELF, 0, 15);
                break;
            case 4:
                addAISpell(MoltenCore::ANCIENTCOREHOUND_WITHERING_HEAT, 20.0f, TARGET_SELF, 0, 15);
                break;
            case 5:
                addAISpell(MoltenCore::ANCIENTCOREHOUND_ANCIENT_HYSTERIA, 20.0f, TARGET_SELF, 0, 15);
                break;
        }
    }
};

class FireswornAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new FireswornAI(c); }
    explicit FireswornAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mGarr = nullptr;

        mSeparationAnxiety = addAISpell(MoltenCore::FIRESWORN_SEPARATION_ANXIETY, 0.0f, TARGET_SELF, 5, 5);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mGarr = getNearestCreatureAI(MoltenCore::CN_GARR);
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

class ShazzrahAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShazzrahAI(c); }
    explicit ShazzrahAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mArcaneExplosion = addAISpell(MoltenCore::SHAZZRAH_ARCANE_EXPLOSION, 8.0f, TARGET_SELF, 0, 0);
    }

    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == MoltenCore::SHAZZRAH_BLINK)
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

    pScriptMgr->register_creature_script(MoltenCore::CN_ANCIENTCOREHOUND, &AncientCoreHoundAI::Create);

    pScriptMgr->register_creature_script(MoltenCore::CN_FIRESWORN, &FireswornAI::Create);
    pScriptMgr->register_creature_script(MoltenCore::CN_CORERAGER, &CoreRagerAI::Create);

    pScriptMgr->register_creature_script(MoltenCore::CN_SULFURON_HARBRINGER, &SulfuronAI::Create);
    pScriptMgr->register_creature_script(MoltenCore::CN_RAGNAROS, &RagnarosAI::Create);
}
