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
#include "Raid_ZulAman.h"

///\todo move AddEmote to database

//NalorakkAI
class NalorakkAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(NalorakkAI);
        NalorakkAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddPhaseSpell(1, AddSpell(NALORAKK_BRUTAL_SWIPE, Target_Current, 2, 0, 35));
            AddPhaseSpell(1, AddSpell(NALORAKK_MANGLE, Target_Current, 12, 0, 20));
            AddPhaseSpell(1, AddSpell(NALORAKK_SURGE, Target_RandomPlayer, 8, 0, 20, 0.0f, 45.0f, true, "I bring da pain!", CHAT_MSG_MONSTER_YELL, 12071));

            AddPhaseSpell(2, AddSpell(NALORAKK_LACERATING_SLASH, Target_Current, 12, 0, 20));
            AddPhaseSpell(2, AddSpell(NALORAKK_REND_FLESH, Target_Current, 12, 0, 12));
            AddPhaseSpell(2, AddSpell(NALORAKK_DEAFENING_ROAR, Target_RandomPlayer, 11, 0, 12));

            SetEnrageInfo(AddSpell(NALORAKK_BERSERK, Target_Self, 0, 0, 600, 0, 0, false, "You had your chance, now it be too late!", CHAT_MSG_MONSTER_YELL, 12074), 600000);

            addEmoteForEvent(Event_OnCombatStart, 8855);
            addEmoteForEvent(Event_OnTargetDied, 8856);
            addEmoteForEvent(Event_OnTargetDied, 8857);
            addEmoteForEvent(Event_OnDied, 8858);

            // Bear Form
            Morph = AddSpell(42377, Target_Self, 0, 0, 0, 0, 0, false, "You call on da beast, you gonna get more dan you bargain for!", CHAT_MSG_MONSTER_YELL, 12072);
            MorphTimer = 0;
        }

        void OnCombatStart(Unit* pTarget) override
        {
            MorphTimer = _addTimer(45000);
        }

        void OnCombatStop(Unit* pTarget) override
        {
            _setDisplayId(21631); 
        }

        void OnDied(Unit* pKiller) override
        {
            _setDisplayId(21631);
        }

        void AIUpdate() override
        {
            // Bear Form
            if (_isTimerFinished(MorphTimer) && isScriptPhase(1))
            {
                setScriptPhase(2);
                // Morph into a bear since the spell doesnt work
                _setDisplayId(21635);
                // 20 Seconds until switch to Troll Form
                _resetTimer(MorphTimer, 20000);
            }

            // Troll Form
            else if (_isTimerFinished(MorphTimer) && isScriptPhase(2))
            {
                // Remove Bear Form
                _removeAura(42377);
                // Transform back into a Troll
                _setDisplayId(21631);
                setScriptPhase(1);
                // 45 Seconds until switch to Bear Form
                _resetTimer(MorphTimer, 45000);

                sendChatMessage(CHAT_MSG_MONSTER_YELL, 12073, "Make way for Nalorakk!");
            }
        }

        SpellDesc* Morph;
        int32 MorphTimer;
};

//Akil'zon <Eagle Avatar>
class AkilzonAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AkilzonAI);
        AkilzonAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(AKILZON_STATIC_DISRUPTION, Target_Self, 2, 0, 60);
            AddSpell(AKILZON_CALL_LIGHTING, Target_Current, 2, 0, 0);
            AddSpell(AKILZON_GUST_OF_WIND, Target_Current, 0, 0, 0);
            AddSpell(AKILZON_ELECTRICAL_STORM, Target_Self, 1, 0, 0);

            addEmoteForEvent(Event_OnCombatStart, 8859);
            addEmoteForEvent(Event_OnTargetDied, 8860);
            addEmoteForEvent(Event_OnTargetDied, 8861);
            addEmoteForEvent(Event_OnDied, 8862);

            mSummonTime = 0;
        }

        void OnCombatStart(Unit* pTarget) override
        {
            mSummonTime = _addTimer(120000);
        }

        void AIUpdate() override
        {
            if (_isTimerFinished(mSummonTime))
            {
                // Spawn 3 Soaring Eagles
                for (uint8 x = 0; x < 3; x++)
                {
                    CreatureAIScript* Eagle = spawnCreatureAndGetAIScript(CN_SOARING_EAGLE, (getCreature()->GetPositionX() + RandomFloat(12) - 10), (getCreature()->GetPositionY() + RandomFloat(12) - 15),
                                          getCreature()->GetPositionZ(), getCreature()->GetOrientation(), getCreature()->GetFaction());
                    if (Eagle)
                    {
                        static_cast<CreatureAIScript*>(Eagle)->AggroNearestUnit();
                        Eagle->_setDespawnWhenInactive(true);
                    }
                }

                sendChatMessage(CHAT_MSG_MONSTER_YELL, 12019, "Feed, me bruddahs!");
                // Restart the timer
                _resetTimer(mSummonTime, 120000);
            }
        }

        int32 mSummonTime;
};

//SOARING_EAGLE Summon Akil'zon
class SoaringEagleAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SoaringEagleAI);
        SoaringEagleAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(EAGLE_SWOOP, Target_Destination, 5, 0, 0);
            getCreature()->m_noRespawn = true;
        }
};


//Halazzi <Lynx Avatar>
class HalazziAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HalazziAI);
        HalazziAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddPhaseSpell(1, AddSpell(HALAZZI_SABER_LASH, Target_Destination, 0.5, 0, 0, 0, 0, false, "Me gonna carve ya now!", CHAT_MSG_MONSTER_YELL, 12023));

            AddPhaseSpell(2, AddSpell(HALAZZI_FLAME_SHOCK, Target_Current, 12, 0, 0));
            AddPhaseSpell(2, AddSpell(HALAZZI_EARTH_SHOCK, Target_Current, 12, 0, 0));

            AddPhaseSpell(3, AddSpell(HALAZZI_SABER_LASH, Target_Destination, 0.5, 0, 0, 0, 0, false, "You gonna leave in pieces!", CHAT_MSG_MONSTER_YELL, 12024));
            AddPhaseSpell(3, AddSpell(HALAZZI_FLAME_SHOCK, Target_Current, 18, 0, 0));
            AddPhaseSpell(3, AddSpell(HALAZZI_EARTH_SHOCK, Target_Current, 18, 0, 0));
            AddPhaseSpell(3, AddSpell(HALAZZI_ENRAGE, Target_Self, 100, 0, 60));

            // Transfigure: 4k aoe damage
            Transfigure = AddSpell(44054, Target_Self, 0, 0, 0, 0, 0, false, "I fight wit' untamed spirit...", CHAT_MSG_MONSTER_YELL, 12021);

            addEmoteForEvent(Event_OnCombatStart, 8863);
            addEmoteForEvent(Event_OnTargetDied, 8864);
            addEmoteForEvent(Event_OnTargetDied, 8865);
            addEmoteForEvent(Event_OnDied, 8866);
            mLynx = NULL;

            mTotemTimer = 0;
            CurrentHealth = 0;
            MaxHealth = 0;
            SplitCount = 0;
        }

        void OnCombatStart(Unit* pTarget) override
        {
            mTotemTimer = _addTimer(5000); // Just to make the Timer ID
            SplitCount = 1;
            MaxHealth = getCreature()->getUInt32Value(UNIT_FIELD_MAXHEALTH);
            mLynx = NULL;
        }

        void OnCombatStop(Unit* pTarget) override
        {
            Merge();
        }

        void AIUpdate() override
        {
            // Every 25% Halazzi calls on the lynx
            if (!mLynx && _getHealthPercent() <= (100 - SplitCount * 25))
                Split();

            // Lynx OR Halazzi is at 20% HP Merge them together again
            if (mLynx && (mLynx->GetHealthPct() <= 20 || _getHealthPercent() <= 20))
                Merge();

            // At <25% Phase 3 begins
            if (_getHealthPercent() < 25 && isScriptPhase(1))
            {
                _resetTimer(mTotemTimer, 30000);
                setScriptPhase(3);
            }

            if (isScriptPhase(2) || isScriptPhase(3))
            {
                if (_isTimerFinished(mTotemTimer))
                {
                    CreatureAIScript* Totem = spawnCreatureAndGetAIScript(CN_TOTEM, (getCreature()->GetPositionX() + RandomFloat(3) - 3), (getCreature()->GetPositionY() + RandomFloat(3) - 3), getCreature()->GetPositionZ(), 0, getCreature()->GetFaction());
                    if (Totem)
                    {
                        Totem->despawn(60000); // Despawn in 60 seconds
                        static_cast<CreatureAIScript*>(Totem)->AggroNearestPlayer();
                    }

                    switch (getScriptPhase())
                    {
                        case 2:
                            _resetTimer(mTotemTimer, 60000);
                            break;
                        case 3:
                            _resetTimer(mTotemTimer, 30000);
                            break; // Spawn them faster then phase 2
                    }
                }
            }
        }

        void Split()
        {
            CurrentHealth = getCreature()->getUInt32Value(UNIT_FIELD_HEALTH);
            _setDisplayId(24144);
            getCreature()->SetHealth(240000);
            getCreature()->setUInt32Value(UNIT_FIELD_MAXHEALTH, 240000);

            mLynx = spawnCreature(CN_LYNX_SPIRIT, getCreature()->GetPosition());
            if (mLynx)
            {
                mLynx->GetAIInterface()->AttackReaction(getCreature()->GetAIInterface()->getNextTarget(), 1);
                mLynx->m_noRespawn = true;
            }

            setScriptPhase(2);
        }

        void Merge()
        {
            if (mLynx)
            {
                mLynx->Despawn(0, 0);
                mLynx = NULL;
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 12022, "Spirit, come back to me!");
            }

            if (CurrentHealth)
                getCreature()->SetHealth(CurrentHealth);
            if (MaxHealth)
                getCreature()->setUInt32Value(UNIT_FIELD_MAXHEALTH, MaxHealth);
            _setDisplayId(21632);

            SplitCount++;
            setScriptPhase(1);
        }

        void OnScriptPhaseChange(uint32_t phaseId) override
        {
            switch (phaseId)
            {
                case 2:
                    CastSpellNowNoScheduling(Transfigure);
                    break;
                default:
                    break;
            }
        }

        Creature* mLynx;
        SpellDesc* Transfigure;
        int32 mTotemTimer;
        int32 CurrentHealth;
        int32 MaxHealth;
        int SplitCount;
};

class LynxSpiritAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LynxSpiritAI);
        LynxSpiritAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Lynx Flurry
            AddSpell(43290, Target_Self, 15, 0, 8);
            // Shred Armor
            AddSpell(43243, Target_Current, 20, 0, 0);
        }
};

void SetupZulAman(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_NALORAKK, &NalorakkAI::Create);

    mgr->register_creature_script(CN_AKILZON, &AkilzonAI::Create);
    mgr->register_creature_script(CN_SOARING_EAGLE, &SoaringEagleAI::Create);

    mgr->register_creature_script(CN_HALAZZI, &HalazziAI::Create);
    mgr->register_creature_script(CN_LYNX_SPIRIT, &LynxSpiritAI::Create);
}
