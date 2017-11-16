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


#include "Setup.h"
#include "Instance_RagefireChasm.h"


// Ragefire Shaman AI
class RagefireShamanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RagefireShamanAI);
    RagefireShamanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SP_RF_SHAMAN_HEALIN_WAVE, Target_WoundedFriendly, 15, 3, 10);
        AddSpell(SP_RF_SHAMAN_LIGHTNING_BOLT, Target_Current, 20, 3, 0);
    }
};

// Ragefire Trogg AI
class RagefireTroggAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(RagefireTroggAI);
    RagefireTroggAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SP_RF_TROGG_STRIKE, Target_Current, 40, 0, 0);
    }
};

// Searing Blade Warlock AI
class SearingBladeWarlockAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SearingBladeWarlockAI);
    SearingBladeWarlockAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SP_SB_WARLOCK_SHADOW_BOLT, Target_Current, 20, 3, 0);
        // Summon Voidwalker -- Doesnt work (Disabled for now)
        //CastSpellNowNoScheduling(AddSpell(12746, Target_Self, 0, 0, 0));
    }
};

// SearingBladeEnforcerAI
class SearingBladeEnforcerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SearingBladeEnforcerAI);
    SearingBladeEnforcerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SP_SB_ENFORCERER_SHIELD_SLAM, Target_Current, 15, 0, 0);
    }
};

// Blade Cultist AI
class BladeCultistAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BladeCultistAI);
    BladeCultistAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Curse of Agony
        AddSpell(SP_SB_CULTIST_CURSE_OF_AGONY, Target_Current, 30, 0, 15);
    }
};

// Molten Elemental AI
class MoltenElementalAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MoltenElementalAI);
    MoltenElementalAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Fire Shield
        AddSpell(SP_MOLTEN_ELEMENTAL_FIRE_SHIELD, Target_Self, 40, 1, 15);
    }
};

// Earthborer AI
class EarthborerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EarthborerAI);
    EarthborerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Earthborer Acid
        AddSpell(SP_EARTHBORER_ACID, Target_Current, 15, 0, 0);
    }
};

/* GAMEOBJECTS */
class BloodFilledOrb : public GameObjectAIScript
{
    public:

        BloodFilledOrb(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
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

/* BOSSES */

// Oggleflint
class OggleflintAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(OggleflintAI);
    OggleflintAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // Cleave
        AddSpell(SP_OGGLEFLINT_CLEAVE, Target_Current, 10, 0, 1);
    }
};

//Taragaman the Hungerer
class TaragamanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TaragamanAI);
    TaragamanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SP_TARAGAMAN_FIRE_NOVA, Target_Self, 10, 2, 0);
        AddSpell(SP_TARAGAMAN_UPPERCUT, Target_Current, 10, 0, 0);
    }
};

//Jergosh The Invoker
class JergoshAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(JergoshAI);
    JergoshAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SP_JERGOSH_IMMOLATE, Target_Current, 15, 2, 0);
        AddSpell(SP_JERGOSH_CURSE_OF_WEAKNESS, Target_Current, 10, 0, 0);
    }
};

//Bazzalan
class BazzalanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BazzalanAI);
    BazzalanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SP_BAZZLAN_SINISTER_STRIKE, Target_Current, 15, 0, 0);
        AddSpell(SP_BAZZLAN_POISON, Target_Current, 5, 0, 0);
    }
};

void SetupRagefireChasm(ScriptMgr* mgr)
{
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
