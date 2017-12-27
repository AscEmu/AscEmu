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

//\todo move AddEmote to database
class NalorakkAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(NalorakkAI);
        NalorakkAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto brutalSwipe = addAISpell(NALORAKK_BRUTAL_SWIPE, 2.0f, TARGET_ATTACKING, 0, 35);
            brutalSwipe->setAvailableForScriptPhase({ 1 });

            auto mangle = addAISpell(NALORAKK_MANGLE, 12.0f, TARGET_ATTACKING, 0, 20);
            mangle->setAvailableForScriptPhase({ 1 });

            auto surge = addAISpell(NALORAKK_SURGE, 8.0f, TARGET_RANDOM_SINGLE, 0, 20, false, true);
            surge->setAvailableForScriptPhase({ 1 });
            surge->addEmote("I bring da pain!", CHAT_MSG_MONSTER_YELL, 12071);

            auto slash = addAISpell(NALORAKK_LACERATING_SLASH, 12.0f, TARGET_ATTACKING, 0, 20);
            slash->setAvailableForScriptPhase({ 2 });

            auto flesh = addAISpell(NALORAKK_REND_FLESH, 12.0f, TARGET_ATTACKING, 0, 12);
            flesh->setAvailableForScriptPhase({ 2 });

            auto roar = addAISpell(NALORAKK_DEAFENING_ROAR, 11.0f, TARGET_RANDOM_SINGLE, 0, 12);
            roar->setAvailableForScriptPhase({ 2 });

            mLocaleEnrageSpell = addAISpell(NALORAKK_BERSERK, 0.0f, TARGET_SELF, 0, 600);
            surge->addEmote("You had your chance, now it be too late!", CHAT_MSG_MONSTER_YELL, 12074);


            addEmoteForEvent(Event_OnCombatStart, 8855);
            addEmoteForEvent(Event_OnTargetDied, 8856);
            addEmoteForEvent(Event_OnTargetDied, 8857);
            addEmoteForEvent(Event_OnDied, 8858);

            // Bear Form
            Morph = addAISpell(42377, 0.0f, TARGET_SELF, 0, 0);
            Morph->addEmote("You call on da beast, you gonna get more dan you bargain for!", CHAT_MSG_MONSTER_YELL, 12072);

            MorphTimer = 0;
            mLocaleEnrageTimerId = 0;
        }

        void OnCombatStart(Unit* /*pTarget*/) override
        {
            MorphTimer = _addTimer(45000);
            mLocaleEnrageTimerId = _addTimer(600000);
        }

        void OnCombatStop(Unit* /*pTarget*/) override
        {
            _setDisplayId(21631); 
            _removeTimer(mLocaleEnrageTimerId);
        }

        void OnDied(Unit* /*pKiller*/) override
        {
            _setDisplayId(21631);
        }

        void AIUpdate() override
        {
            if (_isTimerFinished(mLocaleEnrageTimerId))
            {
                _castAISpell(mLocaleEnrageSpell);
                _removeTimer(mLocaleEnrageTimerId);
            }

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

        CreatureAISpells* Morph;
        int32 MorphTimer;

        CreatureAISpells* mLocaleEnrageSpell;
        uint32_t mLocaleEnrageTimerId;
};

class AkilzonAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AkilzonAI);
        AkilzonAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(AKILZON_STATIC_DISRUPTION, 2.0f, TARGET_SELF, 0, 60);
            addAISpell(AKILZON_CALL_LIGHTING, 2.0f, TARGET_ATTACKING);
            addAISpell(AKILZON_GUST_OF_WIND, 0.0f, TARGET_ATTACKING);
            addAISpell(AKILZON_ELECTRICAL_STORM, 1.0f, TARGET_SELF);

            addEmoteForEvent(Event_OnCombatStart, 8859);
            addEmoteForEvent(Event_OnTargetDied, 8860);
            addEmoteForEvent(Event_OnTargetDied, 8861);
            addEmoteForEvent(Event_OnDied, 8862);

            mSummonTime = 0;
        }

        void OnCombatStart(Unit* /*pTarget*/) override
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
                    /*CreatureAIScript* Eagle =*/ spawnCreatureAndGetAIScript(CN_SOARING_EAGLE, (getCreature()->GetPositionX() + Util::getRandomFloat(12) - 10), (getCreature()->GetPositionY() + Util::getRandomFloat(12) - 15),
                                          getCreature()->GetPositionZ(), getCreature()->GetOrientation(), getCreature()->GetFaction());
                }

                sendChatMessage(CHAT_MSG_MONSTER_YELL, 12019, "Feed, me bruddahs!");
                // Restart the timer
                _resetTimer(mSummonTime, 120000);
            }
        }

        int32 mSummonTime;
};

class SoaringEagleAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SoaringEagleAI);
        SoaringEagleAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(EAGLE_SWOOP, 5.0f, TARGET_DESTINATION, 0, 0);
            getCreature()->m_noRespawn = true;
        }
};


class HalazziAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HalazziAI);
        HalazziAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto saberLash = addAISpell(HALAZZI_SABER_LASH, 0.5f, TARGET_DESTINATION, 0, 0);
            saberLash->addEmote("Me gonna carve ya now!", CHAT_MSG_MONSTER_YELL, 12023);
            saberLash->addEmote("You gonna leave in pieces!", CHAT_MSG_MONSTER_YELL, 12024);
            saberLash->setAvailableForScriptPhase({ 1, 3 });

            auto flameShock = addAISpell(HALAZZI_FLAME_SHOCK, 12.0f, TARGET_ATTACKING, 0, 0);
            flameShock->setAvailableForScriptPhase({ 2, 3 });

            auto earthShock = addAISpell(HALAZZI_EARTH_SHOCK, 12.0f, TARGET_ATTACKING, 0, 0);
            earthShock->setAvailableForScriptPhase({ 2, 3 });

            auto enrage = addAISpell(HALAZZI_ENRAGE, 100.0f, TARGET_SELF, 0, 60);
            enrage->setAvailableForScriptPhase({ 3 });

            // Transfigure: 4k aoe damage
            Transfigure = addAISpell(44054, 0.0f, TARGET_SELF, 0, 0);
            Transfigure->addEmote("I fight wit' untamed spirit...", CHAT_MSG_MONSTER_YELL, 12021);

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

        void OnCombatStart(Unit* /*pTarget*/) override
        {
            mTotemTimer = _addTimer(5000); // Just to make the Timer ID
            SplitCount = 1;
            MaxHealth = getCreature()->getUInt32Value(UNIT_FIELD_MAXHEALTH);
            mLynx = NULL;
        }

        void OnCombatStop(Unit* /*pTarget*/) override
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
                    CreatureAIScript* Totem = spawnCreatureAndGetAIScript(CN_TOTEM, (getCreature()->GetPositionX() + Util::getRandomFloat(3) - 3), (getCreature()->GetPositionY() + Util::getRandomFloat(3) - 3), getCreature()->GetPositionZ(), 0, getCreature()->GetFaction());
                    if (Totem)
                    {
                        Totem->despawn(60000); // Despawn in 60 seconds
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
                    _castAISpell(Transfigure);
                    break;
                default:
                    break;
            }
        }

        Creature* mLynx;
        CreatureAISpells* Transfigure;
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
            addAISpell(43290, 15.0f, TARGET_SELF, 0, 8);
            // Shred Armor
            addAISpell(43243, 20.0f, TARGET_ATTACKING, 0, 0);
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
