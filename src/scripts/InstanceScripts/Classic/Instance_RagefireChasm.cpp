/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_RagefireChasm.h"

class RagefireChasmInstanceScript : public InstanceScript
{
public:

    explicit RagefireChasmInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new RagefireChasmInstanceScript(pMapMgr); }


};

class RagefireShamanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RagefireShamanAI)
    explicit RagefireShamanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_RF_SHAMAN_HEALIN_WAVE, 15.0f, TARGET_RANDOM_FRIEND, 3, 10);
        addAISpell(SP_RF_SHAMAN_LIGHTNING_BOLT, 20.0f, TARGET_ATTACKING, 3, 0);
    }
};

class RagefireTroggAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RagefireTroggAI)
    explicit RagefireTroggAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_RF_TROGG_STRIKE, 40.0f, TARGET_ATTACKING, 0, 0);
    }
};

class SearingBladeWarlockAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SearingBladeWarlockAI)
    explicit SearingBladeWarlockAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_SB_WARLOCK_SHADOW_BOLT, 20.0f, TARGET_ATTACKING, 3, 0);
    }
};

class SearingBladeEnforcerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SearingBladeEnforcerAI)
    explicit SearingBladeEnforcerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_SB_ENFORCERER_SHIELD_SLAM, 15.0f, TARGET_ATTACKING, 0, 0);
    }
};

class BladeCultistAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BladeCultistAI)
    explicit BladeCultistAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_SB_CULTIST_CURSE_OF_AGONY, 30.0f, TARGET_ATTACKING, 0, 15);
    }
};

class MoltenElementalAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MoltenElementalAI)
    explicit MoltenElementalAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_MOLTEN_ELEMENTAL_FIRE_SHIELD, 40.0f, TARGET_SELF, 1, 15);
    }
};

class EarthborerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EarthborerAI)
    explicit EarthborerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_EARTHBORER_ACID, 15.0f, TARGET_ATTACKING, 0, 0);
    }
};

class BloodFilledOrb : public GameObjectAIScript
{
public:

    explicit BloodFilledOrb(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new BloodFilledOrb(GO); }

    void OnActivate(Player* pPlayer) override
    {
        // Make sure player has the quest and Zelemar isn't spawned yet
        if (!pPlayer->HasQuest(9692)) // The Path of the Adept
        {
            pPlayer->GetSession()->SendNotification("Request quest `The Path of the Adept`.");
            return;
        }
        Creature* Zelemar = NULL;
        Zelemar = _gameobject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-370.133f, 162.519f, -21.1299f, CN_ZELMAR);
        if (Zelemar)
            return;

        // Spawn Zelemar the Wrathful
        Zelemar = _gameobject->GetMapMgr()->GetInterface()->SpawnCreature(17830, -370.133f, 162.519f, -21.1299f, -1.29154f, true, false, 0, 0);
        if (Zelemar)
        {
            Zelemar->m_noRespawn = true;
            Zelemar = NULL;
        }
    }
};

// BOSSES

class OggleflintAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(OggleflintAI)
    explicit OggleflintAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_OGGLEFLINT_CLEAVE, 10.0f, TARGET_ATTACKING, 0, 1);
    }
};

class TaragamanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TaragamanAI)
    explicit TaragamanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_TARAGAMAN_FIRE_NOVA, 10.0f, TARGET_SELF, 2, 0);
        addAISpell(SP_TARAGAMAN_UPPERCUT, 10.0f, TARGET_ATTACKING, 0, 0);
    }
};

class JergoshAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(JergoshAI)
    explicit JergoshAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_JERGOSH_IMMOLATE, 15.0f, TARGET_ATTACKING, 2, 0);
        addAISpell(SP_JERGOSH_CURSE_OF_WEAKNESS, 10.0f, TARGET_ATTACKING, 0, 0);
    }
};

class BazzalanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BazzalanAI)
    explicit BazzalanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SP_BAZZLAN_SINISTER_STRIKE, 15.0f, TARGET_ATTACKING, 0, 0);
        addAISpell(SP_BAZZLAN_POISON, 5.0f, TARGET_ATTACKING, 0, 0);
    }
};

void SetupRagefireChasm(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_RAGEFIRE_CHASM, &RagefireChasmInstanceScript::Create);

    //Trash Mobs
    mgr->register_creature_script(CN_SEARING_BLADE_WARLOCK, &SearingBladeWarlockAI::Create);
    mgr->register_creature_script(CN_EARTHBORER, &EarthborerAI::Create);
    mgr->register_creature_script(CN_MOLTEN_ELEMENTAL, &MoltenElementalAI::Create);
    mgr->register_creature_script(CN_RAGEFIRE_SHAMAN, &RagefireShamanAI::Create);
    mgr->register_creature_script(CN_RAGEFIRE_TROGG, &RagefireTroggAI::Create);
    mgr->register_creature_script(CN_BLADE_CULTIST, &BladeCultistAI::Create);
    mgr->register_creature_script(CN_SEARING_BLADE_ENFORCER, &SearingBladeEnforcerAI::Create);

    //Gameobjects
    mgr->register_gameobject_script(GO_BLOOD_FILLED_ORB, &BloodFilledOrb::Create);

    //Bosses
    mgr->register_creature_script(CN_OGGLEFLINT, &OggleflintAI::Create);
    mgr->register_creature_script(CN_TARAGAMAN, &TaragamanAI::Create);
    mgr->register_creature_script(CN_JERGOSH, &JergoshAI::Create);
    mgr->register_creature_script(CN_BAZZALAN, &BazzalanAI::Create);
}
