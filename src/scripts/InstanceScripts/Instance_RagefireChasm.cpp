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

//////////////////////////////////////////////////////////////////////////////////////////
//Ragefire Chasm
class InstanceRagefireChasmScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceRagefireChasmScript, MoonInstanceScript);
        InstanceRagefireChasmScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            // Way to select bosses
            BuildEncounterMap();
            if (mEncounters.size() == 0)
                return;

            for (auto Iter = mEncounters.begin(); Iter != mEncounters.end(); ++Iter)
            {
                if ((*Iter).second.mState != State_Finished)
                    continue;
            }
        }

        void OnGameObjectPushToWorld(GameObject* pGameObject) { }

        void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = (EncounterState)pData;
        }

        uint32 GetInstanceData(uint32 pType, uint32 pIndex)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return 0;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return 0;

            return (*Iter).second.mState;
        }

        void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
        {
            EncounterMap::iterator Iter = mEncounters.find(pCreature->GetEntry());
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = State_Finished;

            return;
        }
};

// Ragefire Shaman AI
class RagefireShamanAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(RagefireShamanAI, MoonScriptCreatureAI);
    RagefireShamanAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SP_RF_SHAMAN_HEALIN_WAVE, Target_WoundedFriendly, 15, 3, 10);
        AddSpell(SP_RF_SHAMAN_LIGHTNING_BOLT, Target_Current, 20, 3, 0);
    }
};

// Ragefire Trogg AI
class RagefireTroggAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(RagefireTroggAI, MoonScriptCreatureAI);
    RagefireTroggAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SP_RF_TROGG_STRIKE, Target_Current, 40, 0, 0);
    }
};

// Searing Blade Warlock AI
class SearingBladeWarlockAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SearingBladeWarlockAI, MoonScriptCreatureAI);
    SearingBladeWarlockAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SP_SB_WARLOCK_SHADOW_BOLT, Target_Current, 20, 3, 0);
        // Summon Voidwalker -- Doesnt work (Disabled for now)
        //CastSpellNowNoScheduling(AddSpell(12746, Target_Self, 0, 0, 0));
    }
};

// SearingBladeEnforcerAI
class SearingBladeEnforcerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SearingBladeEnforcerAI, MoonScriptCreatureAI);
    SearingBladeEnforcerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SP_SB_ENFORCERER_SHIELD_SLAM, Target_Current, 15, 0, 0);
    }
};

// Blade Cultist AI
class BladeCultistAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(BladeCultistAI, MoonScriptCreatureAI);
    BladeCultistAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Curse of Agony
        AddSpell(SP_SB_CULTIST_CURSE_OF_AGONY, Target_Current, 30, 0, 15);
    }
};

// Molten Elemental AI
class MoltenElementalAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(MoltenElementalAI, MoonScriptCreatureAI);
    MoltenElementalAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Fire Shield
        AddSpell(SP_MOLTEN_ELEMENTAL_FIRE_SHIELD, Target_Self, 40, 1, 15);
    }
};

// Earthborer AI
class EarthborerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(EarthborerAI, MoonScriptCreatureAI);
    EarthborerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
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

        void OnActivate(Player* pPlayer)
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
class OggleflintAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(OggleflintAI, MoonScriptCreatureAI);
    OggleflintAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Cleave
        AddSpell(SP_OGGLEFLINT_CLEAVE, Target_Current, 10, 0, 1);
    }
};

//Taragaman the Hungerer
class TaragamanAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(TaragamanAI, MoonScriptCreatureAI);
    TaragamanAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SP_TARAGAMAN_FIRE_NOVA, Target_Self, 10, 2, 0);
        AddSpell(SP_TARAGAMAN_UPPERCUT, Target_Current, 10, 0, 0);
    }
};

//Jergosh The Invoker
class JergoshAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(JergoshAI, MoonScriptCreatureAI);
    JergoshAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SP_JERGOSH_IMMOLATE, Target_Current, 15, 2, 0);
        AddSpell(SP_JERGOSH_CURSE_OF_WEAKNESS, Target_Current, 10, 0, 0);
    }
};

//Bazzalan
class BazzalanAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(BazzalanAI, MoonScriptCreatureAI);
    BazzalanAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SP_BAZZLAN_SINISTER_STRIKE, Target_Current, 15, 0, 0);
        AddSpell(SP_BAZZLAN_POISON, Target_Current, 5, 0, 0);
    }
};

void SetupRagefireChasm(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_RAGEFIRE_CHASM, &InstanceRagefireChasmScript::Create);

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
