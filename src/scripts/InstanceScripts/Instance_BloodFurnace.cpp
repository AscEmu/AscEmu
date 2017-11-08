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
#include "Instance_BloodFurnace.h"


// Keli'dan the BreakerAI
class KelidanTheBreakerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(KelidanTheBreakerAI, MoonScriptCreatureAI);
    KelidanTheBreakerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        //spells
        if (_isHeroic())
        {
            mShadowBoltVolley = AddSpell(KELIDAN_SHADOW_BOLT_VOLLEY_H, Target_Self, 25, 0, 6);
            mFireNova = AddSpell(KELIDAN_FIRE_NOVA_H, Target_Current, 15, 0, 12);
        }
        else
        {
            mShadowBoltVolley = AddSpell(KELIDAN_SHADOW_BOLT_VOLLEY, Target_Self, 25, 0, 6);
            mFireNova = AddSpell(KELIDAN_FIRE_NOVA, Target_Self, 15, 0, 12);
        }

        mBurningNova = AddSpell(KELIDAN_BURNING_NOVA, Target_Self, 0, 0, 0);
        mBurningNova->AddEmote("Closer! Come closer... and burn!", CHAT_MSG_MONSTER_YELL);
        mVortex = AddSpell(KELIDAN_FIRE_NOVA, Target_Self, 0, 0, 0);
        AddSpell(KELIDAN_CORRUPTION, Target_Current, 15, 0, 10);

        mBurningNovaTimer = INVALIDATE_TIMER;
        SetAIUpdateFreq(800);
    }

    void OnCombatStart(Unit* pTarget)
    {
        sendDBChatMessage(4841);     // Who dares interrupt--What is this; what have you done? You'll ruin everything!

        mBurningNovaTimer = _addTimer(15000);
        ParentClass::OnCombatStart(pTarget);
    }

    void AIUpdate()
    {
        if (!_isCasting())
        {
            if (mBurningNovaTimer == INVALIDATE_TIMER || _isTimerFinished(mBurningNovaTimer))
            {
                if (_isHeroic())
                    CastSpell(mVortex);
                CastSpell(mBurningNova);

                _resetTimer(mBurningNovaTimer, 30000);

                ParentClass::AIUpdate();
            }
        }

        ParentClass::AIUpdate();
    }

    void OnTargetDied(Unit* pTarget)
    {
        switch (RandomUInt(1))
        {
            case 0:
                sendDBChatMessage(4845);     // Just as you deserve!
                break;
            case 1:
                sendDBChatMessage(4846);     // Your friends will soon be joining you!
                break;
        }
    }

    void OnDied(Unit* pTarget)
    {
        sendDBChatMessage(4848);     // Good...luck. You'll need it.
    }

    SpellDesc* mShadowBoltVolley;
    SpellDesc* mFireNova;
    SpellDesc* mBurningNova;
    SpellDesc* mVortex;
    int32 mBurningNovaTimer;
};


// Broggok
class BroggokAI : public MoonScriptCreatureAI
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(BroggokAI);
        BroggokAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(POISON_BOLT, Target_Self, 12.0f, 0, 15);
            AddSpell(POISON_CLOUD, Target_RandomPlayerDestination, 8.0f, 0, 40, 0, 40);
            AddSpell(SLIME_SPRAY, Target_Self, 10.0f, 0, 25);
        }

        void OnDied(Unit* pKiller)
        {
            GameObject* pDoor = getNearestGameObject(456.157349f, 34.248005f, 9.559463f, GO_BROGGOK);
            if (pDoor)
                pDoor->SetState(GO_STATE_OPEN);

            MoonScriptCreatureAI::OnDied(pKiller);
        }
};


// The Maker
class TheMakerAI : public MoonScriptCreatureAI
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(TheMakerAI);
        TheMakerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(DOMINATION, Target_RandomPlayer, 8.0f, 0, 30);
            AddSpell(ACID_SPRAY, Target_Self, 10.0f, 0, 20);
            AddSpell(THROW_BEAKER, Target_RandomPlayerDestination, 20.0f, 0, 0, 0, 40);
        }

        void OnCombatStart(Unit* pTarget)
        {
            switch (RandomUInt(2))
            {
                case 0:
                    sendDBChatMessage(4849);     // My work must not be interrupted!
                    break;
                case 1:
                    sendDBChatMessage(4850);     // Perhaps I can find a use for you...
                    break;
                case 2:
                    sendDBChatMessage(4851);     // Anger...hate... These are tools I can use.
                    break;
            }
        }

        void OnTargetDied(Unit* pTarget)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    sendDBChatMessage(4852);     // Let's see what I can make of you!
                    break;
                case 1:
                    sendDBChatMessage(4853);     // It is pointless to resist.
                    break;
            }
        }

        void OnDied(Unit* pKiller)
        {
            sendDBChatMessage(4854);     // Stay away from... Me!

            GameObject* pDoor = getNearestGameObject(327.155487f, 149.753418f, 9.559869f, GO_THE_MAKER);
            if (pDoor)
                pDoor->SetState(GO_STATE_OPEN);

            MoonScriptCreatureAI::OnDied(pKiller);
        }
};

void SetupBloodFurnace(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_KELIDAN_THE_BREAKER, &KelidanTheBreakerAI::Create);
    mgr->register_creature_script(CN_BROGGOK, &BroggokAI::Create);
    mgr->register_creature_script(CN_THE_MAKER, &TheMakerAI::Create);
}
