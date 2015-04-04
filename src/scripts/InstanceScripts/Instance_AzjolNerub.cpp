/*
* ArcScripts for ArcEmu MMORPG Server
* Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
* Copyright (C) 2008-2009 Sun++ Team <http://www.sunplusplus.info/>
* Copyright (C) 2005-2007 Ascent Team
* Copyright (C) 2007-2008 Moon++ Team <http://www.moonplusplus.info/>
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

/// \ todo Finish Kritkhir Encounter, needs more blizzlike, may need InstanceScript | Anuburak | Add's AI and trash

#include "Setup.h"
#include "Instance_AzjolNerub.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Azjol-Nerub
class InstanceAzjolNerubScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceAzjolNerubScript, MoonInstanceScript);
        InstanceAzjolNerubScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            // Way to select bosses
            BuildEncounterMap();
            if (mEncounters.size() == 0)
                return;

            for (EncounterMap::iterator Iter = mEncounters.begin(); Iter != mEncounters.end(); ++Iter)
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

//Krikthir The Gatewatcher
class KrikthirAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(KrikthirAI, MoonScriptCreatureAI);
    KrikthirAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        if (!IsHeroic())
        {
            AddSpell(KRIKTHIR_CURSEOFFATIGUE, Target_Self, 100, 0, 10);
            AddSpell(KRIKTHIR_MINDFLAY, Target_RandomPlayer, 100, 0, 7, 0, 30);
        }
        else
        {
            AddSpell(KRIKTHIR_CURSEOFFATIGUE_HC, Target_Self, 100, 0, 10);
            AddSpell(KRIKTHIR_MINDFLAY_HC, Target_RandomPlayer, 100, 0, 7, 0, 30);
        }

        mEnraged = false;
    }

    void AIUpdate()
    {
        if (_unit->GetHealthPct() <= 10 && mEnraged == false)
        {
            ApplyAura(KRIKTHIR_ENRAGE);
            mEnraged = true;
        }

        ParentClass::AIUpdate();
    }

    void OnCombatStart(Unit* pTarget)
    {
        _unit->SendScriptTextChatMessage(3908);     // This kingdom belongs to the Scourge. Only the dead may enter!
    }

    void OnTargetDied(Unit* mKiller)
    {
        switch (rand() % 2)
        {
            case 0:
                _unit->SendScriptTextChatMessage(3910);     // As Anub'arak commands!
                break;
            case 1:
                _unit->SendScriptTextChatMessage(3909);     // You were foolish to come.
                break;
        }
    }

    void OnDied(Unit* pKiller)
    {
        _unit->SendScriptTextChatMessage(3911);         // I should be grateful... but I long ago lost the capacity....

        GameObject* Doors = GetNearestGameObject(192395);
        if (Doors != NULL)
            Doors->Despawn(0, 0);

        ParentClass::OnDied(pKiller);
    }

    bool mEnraged;
};

//Hadronox
class HadronoxAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(HadronoxAI, MoonScriptCreatureAI);
    HadronoxAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        if (!IsHeroic())
        {
            AddSpell(HADRONOX_WEBGRAB, Target_RandomPlayer, 22, 0, 14, 0, 0);
            AddSpell(HADRONOX_LEECHPOISON, Target_Self, 14, 0, 25, 0, 20);
            AddSpell(HADRONOX_ACIDCLOUD, Target_RandomPlayer, 18, 0, 20, 0, 60);
        }
        else
        {
            AddSpell(HADRONOX_WEBGRAB_HC, Target_RandomPlayer, 22, 0, 14, 0, 0);
            AddSpell(HADRONOX_LEECHPOISON_HC, Target_Self, 14, 0, 25, 0, 20);
            AddSpell(HADRONOX_ACIDCLOUD_HC, Target_RandomPlayer, 18, 0, 20, 0, 60);
        };

        AddSpell(HADRONOX_PIERCEARMOR, Target_ClosestPlayer, 20, 0, 5, 0, 0);
    };

};

//Watcher Gashra
class GashraAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(GashraAI, MoonScriptCreatureAI);
    GashraAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(GASHRA_WEBWRAP, Target_RandomPlayer, 22, 0, 35, 0, 0);
        AddSpell(GASHRA_INFECTEDBITE, Target_ClosestPlayer, 35, 0, 12, 0, 0);
    }

};

//Watcher Narjil
class NarjilAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(NarjilAI, MoonScriptCreatureAI);
    NarjilAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(NARJIL_WEBWRAP, Target_RandomPlayer, 22, 0, 35, 0, 0);
        AddSpell(NARJIL_INFECTEDBITE, Target_ClosestPlayer, 35, 0, 12, 0, 0);
        AddSpell(NARJIL_BLINDINGWEBS, Target_ClosestPlayer, 16, 0, 9, 0, 0);
    }

};

//Watcher Silthik
class SilthikAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SilthikAI, MoonScriptCreatureAI);
    SilthikAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(NARJIL_WEBWRAP, Target_RandomPlayer, 22, 0, 35, 0, 0);
        AddSpell(NARJIL_INFECTEDBITE, Target_ClosestPlayer, 35, 0, 12, 0, 0);
        AddSpell(SILTHIK_POISONSPRAY, Target_RandomPlayer, 30, 0, 15, 0, 0);
    }

};

//Anub'ar Shadowcaster (anub shadowcaster)
class AnubShadowcasterAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(AnubShadowcasterAI, MoonScriptCreatureAI);
    AnubShadowcasterAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SHADOWCASTER_SHADOWBOLT, Target_RandomPlayer, 36, 0, 8);
        AddSpell(SHADOWCASTER_SHADOW_NOVA, Target_Self, 22, 0, 15);
    }

};

void SetupAzjolNerub(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_AZJOL_NERUB, &InstanceAzjolNerubScript::Create);

    //Bosses
    mgr->register_creature_script(BOSS_KRIKTHIR, &KrikthirAI::Create);
    mgr->register_creature_script(BOSS_HADRONOX, &HadronoxAI::Create);

    // watchers
    mgr->register_creature_script(CN_GASHRA, &GashraAI::Create);
    mgr->register_creature_script(CN_NARJIL, &NarjilAI::Create);
    mgr->register_creature_script(CN_SILTHIK, &SilthikAI::Create);
}
