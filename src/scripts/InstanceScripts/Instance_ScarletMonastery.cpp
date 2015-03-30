/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2009-2010 ArcEmu Team <http://www.arcemu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
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


#include "Setup.h"
#include "Instance_ScarletMonastery.h"

//////////////////////////////////////////////////////////////////////////////////////////
//Scarlet Monastery
class InstanceScarletMonasteryScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstanceScarletMonasteryScript, MoonInstanceScript);
        InstanceScarletMonasteryScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
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

// Graveyard

// Interrogator Vishas
class VishasAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(VishasAI, MoonScriptCreatureAI);
        VishasAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(SP_VISHAS_SHADOW_WORD, Target_RandomPlayer, 20, 0, 8);

            m_uiSay = 0;
        }

        void OnCombatStart(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(2110);     // Tell me... tell me everything!
        }

        void OnTargetDied(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(2113);     // Purged by pain!
        }

        void OnCombatStop(Unit* pTarget)
        {
            m_uiSay = 0;

            ParentClass::OnCombatStop(pTarget);
        }

        void AIUpdate()
        {
            if (GetHealthPercent() <= 75 && m_uiSay == 0)
            {
                _unit->SendScriptTextChatMessage(2111);     // Naughty secrets!
                m_uiSay = 1;
            }

            if (GetHealthPercent() <= 25 && m_uiSay == 1)
            {
                _unit->SendScriptTextChatMessage(2112);     // I'll rip the secrets from your flesh!
                m_uiSay = 2;
            }

            ParentClass::AIUpdate();
        }

    private:

        uint8 m_uiSay;
};

// Bloodmage Thalnos
class ThalnosAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(ThalnosAI, MoonScriptCreatureAI);
        ThalnosAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(SP_THALNOS_SHADOW_BOLT, Target_RandomPlayer, 20, 3.0f, 2);
            AddSpell(SP_THALNOS_FLAME_SPIKE, Target_RandomPlayerDestination, 20, 3.0f, 14);

            m_bEmoted = false;
        }

        void OnCombatStart(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(2107);     // We hunger for vengeance.
        }

        void OnTargetDied(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(2109);     // More... More souls!
        }

        void OnCombatStop(Unit* pTarget)
        {
            m_bEmoted = false;

            ParentClass::OnCombatStop(pTarget);
        }

        void AIUpdate()
        {
            if (GetHealthPercent() <= 50 && m_bEmoted == false)
            {
                _unit->SendScriptTextChatMessage(2108);     // No rest... for the angry dead!
                m_bEmoted = true;
            }

            ParentClass::AIUpdate();
        }

    private:

        bool m_bEmoted;
};

// Library
//Houndmaster Loksey
class LokseyAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(LokseyAI, MoonScriptCreatureAI);
        LokseyAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(SP_LOKSEY_BLOODLUST, Target_Self, 5, 0, 40);
        }

        void OnCombatStart(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(2086);     // Release the hounds!
        }
};

// Arcanist Doan
class DoanAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(DoanAI, MoonScriptCreatureAI);
        DoanAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(SP_DOAN_SILENCE, Target_Self, 25, 1.5f, 14);
            AddSpell(SP_DOAN_POLY, Target_SecondMostHated, 15, 1.5f, 10);
            AddSpell(SP_DOAN_ARCANE_EXP, Target_Self, 20, 0, 10);

            m_bShielded = false;
        }

        void OnCombatStart(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(2099);     // You will not defile these mysteries!
        }

        void OnDamageTaken(Unit* mAttacker, uint32 fAmount)
        {
            if (GetHealthPercent() <= 50 && !m_bShielded)
                Shield();
        }

        void Shield()
        {
            _unit->CastSpell(_unit, SP_DOAN_SHIELD, true);
            _unit->SendScriptTextChatMessage(2100);     // Burn in righteous fire!
            _unit->CastSpell(_unit, SP_DOAN_NOVA, false);
            m_bShielded = true;
        }

        void OnCombatStop(Unit* pTarget)
        {
            m_bShielded = false;

            ParentClass::OnCombatStop(pTarget);
        }

    private:

        bool m_bShielded;
};



// Armory
// Herod
class HerodAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(HerodAI, MoonScriptCreatureAI);
        HerodAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(SP_HEROD_WHIRLWINDSPELL, Target_Self, 12, 0, 12)->AddEmote("Blades of Light!", Text_Yell, 5832);
            AddSpell(SP_HEROD_CHARGE, Target_RandomPlayer, 6, 0, 20);

            m_bEnraged = false;
        }

        void OnCombatStart(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(2094);     // Ah - I've been waiting for a real challenge!
        }

        void OnTargetDied(Unit* pTarget)
        {
            _unit->SendScriptTextChatMessage(2087);     // Is that all?
        }

        void OnCombatStop(Unit* pTarget)
        {
            m_bEnraged = false;
            RemoveAura(SP_HEROD_ENRAGESPELL);

            ParentClass::OnCombatStop(pTarget);
        }

        void AIUpdate()
        {
            if (GetHealthPercent() <= 40 && m_bEnraged == false)
            {
                _unit->SendScriptTextChatMessage(2090);     // Light, give me strength!
                ApplyAura(SP_HEROD_ENRAGESPELL);
            }

            ParentClass::AIUpdate();
        }

        bool    m_bEnraged;
};


// Cathedral
// Scarlet Commander Mograine
class MograineAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(MograineAI);

        SP_AI_Spell spells[3];

        bool m_spellcheck[3];

        MograineAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            mPhase = 0;

            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_MORGRAINE_SHIELD);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].perctrigger = 5.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_MORGRAINE_HAMMER);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_MORGRAINE_CRUSADER);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].perctrigger = 30.0f;
            spells[2].attackstoptimer = 1000;
            Timer = 0;

        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(SAY_MORGRAINE_01);
            RegisterAIUpdateEvent(_unit->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME));
        }

        void OnTargetDied(Unit* mTarget)
        {

            if (_unit->GetHealthPct() > 0)
            {
                int RandomSpeach;
                RandomFloat(1000);
                RandomSpeach = rand() % 2;
                switch (RandomSpeach)
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_MORGRAINE_02);
                        break;

                }
            }
        }


        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_MORGRAINE_03);

            GameObject* pDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(1173.01f, 1389.91f, 31.9723f, GO_INQUISITORS_DOOR);
            if (pDoor == 0)
                return;

            // Open the door
            pDoor->SetState(GAMEOBJECT_STATE_OPEN);

            RemoveAIUpdateEvent();
        }


        void AIUpdate()
        {
            Timer = Timer + 1;

            /*if (Timer == 30000000000)
            {
            _unit->CastSpell(_unit, spells[1].info, spells[1].instant);
            }

            else
            {*/
            float val = RandomFloat(100.0f);
            SpellCast(val);
            //}
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (int i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:

                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;

                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;

                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }

                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint32 mPhase;
        int nrspells;
        int Timer;
};

// High Inquisitor Whitemane
class WhitemaneAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(WhitemaneAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        WhitemaneAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            mPhase = 0;

            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_WHITEMANE_SMITE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].perctrigger = 15.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_WHITEMANE_SLEEP);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_WHITEMANE_RESURRECTION);
            spells[2].targettype = TARGET_VARIOUS; //Can't seem to get her to cast it on Mograine...
            spells[2].instant = false;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].soundid = SAY_SOUND_RESTALK2;
            spells[2].speech = "Arise, my champion!";
            Timer = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(SAY_WHITEMANE_01);
            RegisterAIUpdateEvent(_unit->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME));
        }

        void OnTargetDied(Unit* mTarget)
        {

            if (_unit->GetHealthPct() > 0)
            {
                int RandomSpeach;
                RandomFloat(1000);
                RandomSpeach = rand() % 2;
                switch (RandomSpeach)
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(SAY_WHITEMANE_02);
                        break;

                }
            }
        }

        void OnDamageTaken(Unit* mAttacker, uint32 fAmount)
        {
            if (fAmount < 5) return;
            // <50% hp -> We go to phase 1
            if (_unit->GetHealthPct() <= 50 && mPhase == 0)
            {
                ChangeToPhase1();
            }
        }

        void ChangeToPhase1()
        {
            // Set phase var
            mPhase = 1;

            // Play sound, and send text.
            /*_unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Light, give me strength!");
                   ^^ Notes for myself */
            _unit->SendScriptTextChatMessage(2106);
            //MoveToRes();
            CastSleep();
            CastRes();
        }

        /*void MoveToRes()
        {
        _unit->GetAIInterface ()->MoveTo(1154.859009,1403.924683,32.250183,3.466254);
        }*/

        void CastSleep()
        {
            _unit->CastSpell(_unit, spells[1].info, spells[1].instant);
        }

        void CastRes()
        {
            _unit->CastSpell(_unit, spells[2].info, spells[2].instant);
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(SAY_WHITEMANE_03);
            RemoveAIUpdateEvent();
        }


        void AIUpdate()
        {
            Timer = Timer + 1;

            /*if (Timer == 30000000000)
            {
            //_unit->CastSpell(_unit, spells[1].info, spells[1].instant);
            }

            else
            {*/
            float val = RandomFloat(100.0f);
            SpellCast(val);
            //}
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (int i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint32 mPhase;  // NPC has 2 phases
        int nrspells;
        int Timer;
};


// High Inquisitor Fairbanks
class FairbanksAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(FairbanksAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];
        FairbanksAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            mPhase = 0;

            for (int i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_FAIRBANKS_BLOOD);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 15.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_FAIRBANKS_PWS);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].perctrigger = 15.0f;
            spells[1].attackstoptimer = 1000;
            Timer = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetUInt32Value(UNIT_FIELD_BASEATTACKTIME));
        }

        void OnTargetDied(Unit* mTarget)
        {

            if (_unit->GetHealthPct() > 0)
            {
                int RandomSpeach;
                RandomFloat(1000);
                RandomSpeach = rand() % 2;
                switch (RandomSpeach)
                {
                    case 0:
                        _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Ha! Had enough?");       /// \todo can anybody verify this?
                        _unit->PlaySoundToSet(0000);
                        break;

                }
            }
        }


        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {

            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            Timer = Timer + 1;

            if (Timer == 20000)
            {
                _unit->CastSpell(_unit, spells[1].info, spells[1].instant);
            }

            else
            {
                float val = RandomFloat(100.0f);
                SpellCast(val);
            }
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (int i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                _unit->CastSpell(_unit, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                _unit->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger))
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }

                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint32 mPhase;  // NPC has 2 phases
        int nrspells;
        int Timer;
};

class ScarletTorch : public GameObjectAIScript
{
    public:

        ScarletTorch(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
        static GameObjectAIScript* Create(GameObject* GO) { return new ScarletTorch(GO); }

        void OnActivate(Player* pPlayer)
        {
            GameObject* SecretDoor = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(1167.79f, 1347.26f, 31.5494f, GO_SCARLET_SECRET_DOOR);
            if (SecretDoor != NULL)
            {
                if (SecretDoor->GetState() == GAMEOBJECT_STATE_CLOSED)
                    SecretDoor->SetState(GAMEOBJECT_STATE_OPEN);
                else
                    SecretDoor->SetState(GAMEOBJECT_STATE_CLOSED);
            }
        }
};

class ArmoryLever : public GameObjectAIScript
{
    public:

        ArmoryLever(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
        static GameObjectAIScript* Create(GameObject* GO) { return new ArmoryLever(GO); }

        void OnActivate(Player* pPlayer)
        {
            GameObject* ArmoryDoor = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2886.31f, -827.261f, 160.336f, GO_ARMORY_DOOR);
            if (ArmoryDoor != NULL)
            {
                if (ArmoryDoor->GetState() == GAMEOBJECT_STATE_CLOSED)
                    ArmoryDoor->SetState(GAMEOBJECT_STATE_OPEN);
                else
                    ArmoryDoor->SetState(GAMEOBJECT_STATE_CLOSED);
            }
        }
};

class CathedralLever : public GameObjectAIScript
{
    public:

        CathedralLever(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
        static GameObjectAIScript* Create(GameObject* GO) { return new CathedralLever(GO); }

        void OnActivate(Player* pPlayer)
        {
            GameObject* CathedralDoor = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2908.18f, -818.203f, 160.332f, GO_CATHEDRAL_DOOR);
            if (CathedralDoor != NULL)
            {
                if (CathedralDoor->GetState() == GAMEOBJECT_STATE_CLOSED)
                    CathedralDoor->SetState(GAMEOBJECT_STATE_OPEN);
                else
                    CathedralDoor->SetState(GAMEOBJECT_STATE_CLOSED);
            }
        }
};

void SetupScarletMonastery(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_SCARLET_MONASTERY, &InstanceScarletMonasteryScript::Create);

    //Bosses?
    mgr->register_creature_script(CN_LOKSEY, &LokseyAI::Create);
    mgr->register_creature_script(CN_VISHAS, &VishasAI::Create);
    mgr->register_creature_script(CN_THALNOS, &ThalnosAI::Create);
    mgr->register_creature_script(CN_COMMANDER_MOGRAINE, &MograineAI::Create);
    mgr->register_creature_script(CN_WHITEMANE, &WhitemaneAI::Create);
    mgr->register_creature_script(CN_FAIRBANKS, &FairbanksAI::Create);
    mgr->register_creature_script(CN_HEROD, &HerodAI::Create);
    mgr->register_creature_script(CN_DOAN, &DoanAI::Create);

    //Gameobjects
    mgr->register_gameobject_script(GO_SCARLET_TORCH, &ScarletTorch::Create);
    mgr->register_gameobject_script(GO_CATHEDRAL_LEVER, &CathedralLever::Create);
    mgr->register_gameobject_script(GO_ARMORY_LEVER, &ArmoryLever::Create);
}
