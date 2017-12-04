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

// Graveyard

class VishasAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(VishasAI);
        VishasAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(SP_VISHAS_SHADOW_WORD, 20.0f, TARGET_RANDOM_SINGLE, 0, 8);

            m_uiSay = 0;

            addEmoteForEvent(Event_OnCombatStart, 2110);    // Tell me... tell me everything!
            addEmoteForEvent(Event_OnTargetDied, 2113);     // Purged by pain!
        }

        void OnCombatStop(Unit* /*pTarget*/) override
        {
            m_uiSay = 0;
        }

        void AIUpdate() override
        {
            if (_getHealthPercent() <= 75 && m_uiSay == 0)
            {
                sendDBChatMessage(2111);     // Naughty secrets!
                m_uiSay = 1;
            }

            if (_getHealthPercent() <= 25 && m_uiSay == 1)
            {
                sendDBChatMessage(2112);     // I'll rip the secrets from your flesh!
                m_uiSay = 2;
            }
        }

    private:

        uint8 m_uiSay;
};

class ThalnosAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ThalnosAI);
        ThalnosAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(SP_THALNOS_SHADOW_BOLT, 20.0f, TARGET_RANDOM_SINGLE, 3, 2);
            addAISpell(SP_THALNOS_FLAME_SPIKE, 20.0f, TARGET_RANDOM_DESTINATION, 3, 14);

            m_bEmoted = false;

            addEmoteForEvent(Event_OnCombatStart, 2107);     // We hunger for vengeance.
            addEmoteForEvent(Event_OnTargetDied, 2109);     // More... More souls!
        }

        void OnCombatStop(Unit* /*pTarget*/) override
        {
            m_bEmoted = false;
        }

        void AIUpdate() override
        {
            if (_getHealthPercent() <= 50 && m_bEmoted == false)
            {
                sendDBChatMessage(2108);     // No rest... for the angry dead!
                m_bEmoted = true;
            } 
        }

    private:

        bool m_bEmoted;
};

// Library
class LokseyAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LokseyAI);
        LokseyAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(SP_LOKSEY_BLOODLUST, 5.0f, TARGET_SELF, 0, 40);

            addEmoteForEvent(Event_OnCombatStart, 2086);     // Release the hounds!
        }
};

class DoanAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(DoanAI);
        DoanAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(SP_DOAN_SILENCE, 25.0f, TARGET_SELF, 2, 14);
            addAISpell(SP_DOAN_POLY, 15.0f, TARGET_VARIOUS, 2, 10);
            addAISpell(SP_DOAN_ARCANE_EXP, 20.0f, TARGET_SELF, 0, 10);

            m_bShielded = false;

            addEmoteForEvent(Event_OnCombatStart, 2099);     // You will not defile these mysteries!
        }

        void OnDamageTaken(Unit* /*mAttacker*/, uint32 /*fAmount*/) override
        {
            if (_getHealthPercent() <= 50 && !m_bShielded)
                Shield();
        }

        void Shield()
        {
            getCreature()->CastSpell(getCreature(), SP_DOAN_SHIELD, true);
            sendDBChatMessage(2100);     // Burn in righteous fire!
            getCreature()->CastSpell(getCreature(), SP_DOAN_NOVA, false);
            m_bShielded = true;
        }

        void OnCombatStop(Unit* /*pTarget*/) override
        {
            m_bShielded = false;
        }

    private:

        bool m_bShielded;
};



// Armory
class HerodAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HerodAI);
        HerodAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto whirlwind = addAISpell(SP_HEROD_WHIRLWINDSPELL, 12.0f, TARGET_SELF, 0, 12);
            whirlwind->addEmote("Blades of Light!", CHAT_MSG_MONSTER_YELL, 5832);

            addAISpell(SP_HEROD_CHARGE, 6.0f, TARGET_RANDOM_SINGLE, 0, 20);

            m_bEnraged = false;

            addEmoteForEvent(Event_OnCombatStart, 2094);     // Ah - I've been waiting for a real challenge!
            addEmoteForEvent(Event_OnTargetDied, 2087);     // Is that all?
        }

        void OnCombatStop(Unit* /*pTarget*/) override
        {
            m_bEnraged = false;
            _removeAura(SP_HEROD_ENRAGESPELL); 
        }

        void AIUpdate() override
        {
            if (_getHealthPercent() <= 40 && m_bEnraged == false)
            {
                sendDBChatMessage(2090);     // Light, give me strength!
                _applyAura(SP_HEROD_ENRAGESPELL);
            }
        }

        bool m_bEnraged;
};


// Cathedral
class MograineAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(MograineAI);
        MograineAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto shield = addAISpell(SP_MORGRAINE_SHIELD, 5.0f, TARGET_SELF, 0, 10, false, true);
            shield->setAttackStopTimer(1000);

            auto hammer = addAISpell(SP_MORGRAINE_HAMMER, 10.0f, TARGET_ATTACKING, 0, 10, false, true);
            hammer->setAttackStopTimer(1000);

            auto crusade = addAISpell(SP_MORGRAINE_CRUSADER, 30.0f, TARGET_ATTACKING, 0, 10, false, true);
            crusade->setAttackStopTimer(1000);

            addEmoteForEvent(Event_OnCombatStart, SAY_MORGRAINE_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_MORGRAINE_02);
            addEmoteForEvent(Event_OnDied, SAY_MORGRAINE_03);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            GameObject* pDoor = getNearestGameObject(1173.01f, 1389.91f, 31.9723f, GO_INQUISITORS_DOOR);
            if (pDoor != nullptr)
                pDoor->SetState(GO_STATE_OPEN);
        }
};

class WhitemaneAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(WhitemaneAI);
        WhitemaneAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto smite = addAISpell(SP_WHITEMANE_SMITE, 15.0f, TARGET_ATTACKING);
            smite->setAttackStopTimer(1000);

            sleep = addAISpell(SP_WHITEMANE_SLEEP, 0.0f, TARGET_ATTACKING, 0, 0, false, true);
            sleep->setAttackStopTimer(1000);

            resurrection = addAISpell(SP_WHITEMANE_RESURRECTION, 0.0f, TARGET_VARIOUS, 0, 0, false, true);
            resurrection->setAttackStopTimer(1000);
            resurrection->addEmote("Arise, my champion!", CHAT_MSG_MONSTER_YELL, SAY_SOUND_RESTALK2);

            addEmoteForEvent(Event_OnCombatStart, SAY_WHITEMANE_01);
            addEmoteForEvent(Event_OnTargetDied, SAY_WHITEMANE_02);
            addEmoteForEvent(Event_OnDied, SAY_WHITEMANE_03);
        }

        void OnDamageTaken(Unit* /*mAttacker*/, uint32 fAmount) override
        {
            if (fAmount < 5)
                return;

            if (getCreature()->GetHealthPct() <= 50 && getScriptPhase() == 1)
                ChangeToPhase1();
        }

        void ChangeToPhase1()
        {
            setScriptPhase(2);

            sendDBChatMessage(2106);
            CastSleep();
            CastRes();
        }

        void CastSleep()
        {
            getCreature()->CastSpell(getCreature(), sleep->mSpellInfo, true);
        }

        void CastRes()
        {
            auto morgrain = getNearestCreature(CN_COMMANDER_MOGRAINE);
            if (morgrain != nullptr)
                getCreature()->CastSpell(morgrain, resurrection->mSpellInfo, true);
        }

    protected:

        CreatureAISpells* sleep;
        CreatureAISpells* resurrection;
};


class FairbanksAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(FairbanksAI);
        FairbanksAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto blood = addAISpell(SP_FAIRBANKS_BLOOD, 15.0f, TARGET_ATTACKING, 0, 20, false, true);
            blood->setAttackStopTimer(1000);

            auto pws = addAISpell(SP_FAIRBANKS_PWS, 15.0f, TARGET_SELF, 0, 0, false, true);
            pws->setAttackStopTimer(1000);
        }
};

class ScarletTorch : public GameObjectAIScript
{
    public:

        ScarletTorch(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
        static GameObjectAIScript* Create(GameObject* GO) { return new ScarletTorch(GO); }

        void OnActivate(Player* pPlayer) override
        {
            GameObject* SecretDoor = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(1167.79f, 1347.26f, 31.5494f, GO_SCARLET_SECRET_DOOR);
            if (SecretDoor != NULL)
            {
                if (SecretDoor->GetState() == GO_STATE_CLOSED)
                    SecretDoor->SetState(GO_STATE_OPEN);
                else
                    SecretDoor->SetState(GO_STATE_CLOSED);
            }
        }
};

class ArmoryLever : public GameObjectAIScript
{
    public:

        ArmoryLever(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
        static GameObjectAIScript* Create(GameObject* GO) { return new ArmoryLever(GO); }

        void OnActivate(Player* pPlayer) override
        {
            GameObject* ArmoryDoor = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2886.31f, -827.261f, 160.336f, GO_ARMORY_DOOR);
            if (ArmoryDoor != NULL)
            {
                if (ArmoryDoor->GetState() == GO_STATE_CLOSED)
                    ArmoryDoor->SetState(GO_STATE_OPEN);
                else
                    ArmoryDoor->SetState(GO_STATE_CLOSED);
            }
        }
};

class CathedralLever : public GameObjectAIScript
{
    public:

        CathedralLever(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
        static GameObjectAIScript* Create(GameObject* GO) { return new CathedralLever(GO); }

        void OnActivate(Player* pPlayer) override
        {
            GameObject* CathedralDoor = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(2908.18f, -818.203f, 160.332f, GO_CATHEDRAL_DOOR);
            if (CathedralDoor != NULL)
            {
                if (CathedralDoor->GetState() == GO_STATE_CLOSED)
                    CathedralDoor->SetState(GO_STATE_OPEN);
                else
                    CathedralDoor->SetState(GO_STATE_CLOSED);
            }
        }
};

void SetupScarletMonastery(ScriptMgr* mgr)
{
    //Bosses
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
