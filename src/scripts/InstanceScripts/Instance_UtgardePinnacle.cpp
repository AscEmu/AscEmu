/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Instance_UtgardePinnacle.h"

//////////////////////////////////////////////////////////////////////////////////////////
//UtgardePinnacle Instance
class InstanceUtgardePinnacle : public MoonInstanceScript
{
    public:

    MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceUtgardePinnacle, MoonInstanceScript);
    InstanceUtgardePinnacle(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
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

    void OnGameObjectPushToWorld(GameObject* pGameObject) {}

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


//SvalaSorrowgrave
class SvalaSorrowgraveAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(SvalaSorrowgraveAI, MoonScriptBossAI);
        SvalaSorrowgraveAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
        }
};

//GortokPalehoof
class GortokPalehoofAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(GortokPalehoofAI, MoonScriptBossAI);
        GortokPalehoofAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(SAY_GROTOK_PALEHOOF_01);
        }

        void OnTargetDied(Unit* mKiller)
        {
            switch (RandomUInt(2))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_GROTOK_PALEHOOF_02);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_GROTOK_PALEHOOF_03);
                    break;
                default:
                    break;
            }
        }
};

//SkadiTheRuthless
class SkadiTheRuthlessAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(SkadiTheRuthlessAI, MoonScriptBossAI);
        SkadiTheRuthlessAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(SAY_SKADI_RUTHLESS_START);
        }

        void OnTargetDied(Unit* mKiller)
        {
            switch (RandomUInt(2))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_SKADI_RUTHLESS_KILL_01);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_SKADI_RUTHLESS_KILL_02);
                    break;
                default:
                    break;
            }
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_SKADI_RUTHLESS_DIE);
        }
};

//KingYmiron
class KingYmironAI : public MoonScriptBossAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(KingYmironAI, MoonScriptBossAI);
        KingYmironAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(SAY_KING_YMIRON_START);
        }

        void OnTargetDied(Unit* mKiller)
        {
            switch (RandomUInt(5))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(SAY_KING_YMIRON_KILL_01);
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(SAY_KING_YMIRON_KILL_02);
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(SAY_KING_YMIRON_KILL_03);
                    break;
                case 3:
                    _unit->SendScriptTextChatMessage(SAY_KING_YMIRON_KILL_04);
                    break;
                default:
                    break;
            }
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_KING_YMIRON_DIE);
        }
};


void SetupUtgardePinnacle(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_UTGARDE_PINNACLE, &InstanceUtgardePinnacle::Create);

    //Bosses
    mgr->register_creature_script(CN_SVALA_SORROWGRAVE, &SvalaSorrowgraveAI::Create);
    mgr->register_creature_script(CN_GORTOK_PALEHOOF, &GortokPalehoofAI::Create);
    mgr->register_creature_script(CN_SKADI_THE_RUTHLESS, &SkadiTheRuthlessAI::Create);
    mgr->register_creature_script(CN_KING_YMIRON, &KingYmironAI::Create);
}
