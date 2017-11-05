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
#include "Instance_Botanica.h"

// Bloodwarder Protector AI
class BloodProtectorAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(BloodProtectorAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        BloodProtectorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 1;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = sSpellCustomizations.GetSpellInfo(CRYSTAL_STRIKE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000; // 1sec

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
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

        uint8 nrspells;
};


// Bloodwarder Mender AI
// \todo Script me!

    //Healer
    //Casts Shadow Word: Pain and Mind Blast
    //Mind Control these for Holy Fury buff (+295 spell damage for 30 minutes, shows as DIVINE fury on the pet bar). Can be spellstolen.

// Bloodwarder Greenkeeper AI
class BloodGreenkeeperAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(BloodGreenkeeperAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        BloodGreenkeeperAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 1;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = sSpellCustomizations.GetSpellInfo(GREENKEEPER_FURY);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000; // 1sec

        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
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

        uint8 nrspells;
};


// Sunseeker Chemist AI
class SunchemistAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SunchemistAI);
        SP_AI_Spell spells[2];
        bool m_spellcheck[2];

        SunchemistAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 2;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = sSpellCustomizations.GetSpellInfo(FLAME_BREATH);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 3000; // 1sec

            spells[1].info = sSpellCustomizations.GetSpellInfo(POISON_CLOUD);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = false;
            spells[1].perctrigger = 5.0f;
            spells[1].attackstoptimer = 2000; // 1sec
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
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

        uint8 nrspells;
};


// Sunseeker Researcher AI
class SunResearcherAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(SunResearcherAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        SunResearcherAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // -- Number of spells to add --
            nrspells = 4;

            // --- Initialization ---
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }
            // ----------------------

            // Create basic info for spells here, and play with it later , fill always the info, targettype and if is instant or not!
            spells[0].info = sSpellCustomizations.GetSpellInfo(POISON_SHIELD);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = false;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000; // 1sec

            spells[1].info = sSpellCustomizations.GetSpellInfo(MIND_SHOCK);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].perctrigger = 5.0f;
            spells[1].attackstoptimer = 2000; // 1sec

            spells[2].info = sSpellCustomizations.GetSpellInfo(FROST_SHOCK);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].perctrigger = 5.0f;
            spells[2].attackstoptimer = 2000; // 1sec

            spells[3].info = sSpellCustomizations.GetSpellInfo(FLAME_SHOCK);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = false;
            spells[3].perctrigger = 10.0f;
            spells[3].attackstoptimer = 2000; // 1sec
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            _unit->CastSpell(_unit, spells[0].info, spells[0].instant);
        }

        void OnCombatStop(Unit* mTarget)
        {
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
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
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
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

        uint8 nrspells;
};


// Commander Sarannis AI
class CommanderSarannisAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(CommanderSarannisAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        CommanderSarannisAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            GuardAdds = false;
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(ARCANE_RESONANCE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 7.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(ARCANE_DEVASTATION);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 15.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SUMMON_REINFORCEMENTS);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            GuardAdds = false;
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            sendDBChatMessage(SAY_COMMANDER_SARANNIS_01);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget)
        {
            GuardAdds = false;
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        sendDBChatMessage(SAY_COMMANDER_SARANNIS_02);
                        break;
                    case 1:
                        sendDBChatMessage(SAY_COMMANDER_SARANNIS_03);
                        break;
                }
            }
        }

        void OnDied(Unit* mKiller)
        {
            GuardAdds = false;
            CastTime();
            RemoveAIUpdateEvent();
            sendDBChatMessage(SAY_COMMANDER_SARANNIS_07);
        }

        void AIUpdate()
        {
            if (_unit->GetHealthPct() <= 50 && GuardAdds == false)
            {
                GuardAdds = true;    // need to add guard spawning =/
                sendDBChatMessage(SAY_COMMANDER_SARANNIS_06);
            }
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void ArcaneSound()
        {
            switch (RandomUInt(1))
            {
                case 0:
                    sendDBChatMessage(SAY_COMMANDER_SARANNIS_04);
                    break;
                case 1:
                    sendDBChatMessage(SAY_COMMANDER_SARANNIS_05);
                    break;
            }
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }

                        ArcaneSound();

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        bool GuardAdds;
        uint8 nrspells;
};


// High Botanist Freywinn AI
class HighBotanistFreywinnAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(HighBotanistFreywinnAI);
        SP_AI_Spell spells[7];
        bool m_spellcheck[7];

        HighBotanistFreywinnAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            PlantTimer = 10;
            nrspells = 7;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(PLANT_RED_SEEDLING);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(PLANT_GREEN_SEEDLING);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(PLANT_WHITE_SEEDLING);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(PLANT_BLUE_SEEDLING);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = -1;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = sSpellCustomizations.GetSpellInfo(SUMMON_FRAYER_PROTECTOR);
            spells[4].targettype = TARGET_SELF;
            spells[4].instant = true;
            spells[4].cooldown = -1;
            spells[4].perctrigger = 5.0f;
            spells[4].attackstoptimer = 1000;

            spells[5].info = sSpellCustomizations.GetSpellInfo(TREE_FORM);
            spells[5].targettype = TARGET_SELF;
            spells[5].instant = true;
            spells[5].cooldown = 40;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 1000;

            spells[6].info = sSpellCustomizations.GetSpellInfo(TRANQUILITY);
            spells[6].targettype = TARGET_VARIOUS;
            spells[6].instant = false;
            spells[6].cooldown = -1;
            spells[6].perctrigger = 0.0f;
            spells[6].attackstoptimer = 1000;


        }

        void OnCombatStart(Unit* mTarget)
        {
            PlantTimer = 10;
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            sendDBChatMessage(SAY_HIGH_BOTANIS_FREYWIN_04);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget)
        {
            PlantTimer = 10;
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        sendDBChatMessage(SAY_HIGH_BOTANIS_FREYWIN_03);
                        break;
                    case 1:
                        sendDBChatMessage(SAY_HIGH_BOTANIS_FREYWIN_02);
                        break;
                }
            }
        }

        void OnDied(Unit* mKiller)
        {
            PlantTimer = 10;
            CastTime();
            RemoveAIUpdateEvent();
            sendDBChatMessage(SAY_HIGH_BOTANIS_FREYWIN_06);
        }

        void AIUpdate()
        {
            PlantTimer--;
            if (!PlantTimer)
            {
                PlantColorSeedling();
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void PlantColorSeedling()
        {
            PlantTimer = RandomUInt(5, 10);    //5-10 sec (as in my DB attack time is 1000)

            switch (RandomUInt(3))
            {
                case 0:
                {
                    _unit->CastSpell(_unit, spells[0].info, spells[0].instant);
                }
                break;
                case 1:
                {
                    _unit->CastSpell(_unit, spells[1].info, spells[1].instant);
                }
                break;
                case 2:
                {
                    _unit->CastSpell(_unit, spells[2].info, spells[2].instant);
                }
                break;
                case 3:
                {
                    _unit->CastSpell(_unit, spells[3].info, spells[3].instant);
                }
                break;
            }
        }

        void TreeSound()
        {
            switch (RandomUInt(1))
            {
                case 0:
                {
                    sendDBChatMessage(SAY_HIGH_BOTANIS_FREYWIN_01);
                }
                break;
                case 1:
                {
                    sendDBChatMessage(SAY_HIGH_BOTANIS_FREYWIN_05);
                }
                break;
            }
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }

                        if (m_spellcheck[5] == true)
                        {
                            TreeSound();
                            m_spellcheck[6] = true;
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint32 PlantTimer;
        uint8 nrspells;
};


// Thorngrin the Tender AI
class ThorngrinTheTenderAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(ThorngrinTheTenderAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        ThorngrinTheTenderAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            Enraged = false;
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(HELLFIRE);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 9.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SACRIFICE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 6.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(ENRAGE);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            Enraged = false;
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            sendDBChatMessage(SAY_THORNIN_01);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget)
        {
            Enraged = false;
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        sendDBChatMessage(SAY_THORNIN_02);
                        break;
                    case 1:
                        sendDBChatMessage(SAY_THORNIN_03);
                        break;
                }
            }
        }

        void OnDied(Unit* mKiller)
        {
            Enraged = false;
            CastTime();
            RemoveAIUpdateEvent();
            sendDBChatMessage(SAY_THORNIN_08);
        }

        void AIUpdate()
        {
            if (_unit->GetHealthPct() <= 20 && Enraged == false)
            {
                Enraged = true;
                _unit->CastSpell(_unit, spells[2].info, spells[2].instant);
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void HellfireSound()
        {
            switch (RandomUInt(1))
            {
                case 0:
                    sendDBChatMessage(SAY_THORNIN_06);
                    break;
                case 1:
                    sendDBChatMessage(SAY_THORNIN_07);
                    break;
            }
        }

        void SacrificeSound()
        {
            switch (RandomUInt(1))
            {
                case 0:
                    sendDBChatMessage(SAY_THORNIN_04);
                    break;
                case 1:
                    sendDBChatMessage(SAY_THORNIN_05);
                    break;
            }
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                        }

                        if (m_spellcheck[1] == true)
                        {
                            SacrificeSound();
                        }

                        if (spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, spells[i].speech.c_str());
                            _unit->PlaySoundToSet(spells[i].soundid);
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;

                        if (m_spellcheck[0] == true)    // Hellfire
                        {
                            HellfireSound();
                        }
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        bool Enraged;
        uint8 nrspells;
};


// Laj AI
class LajAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(LajAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        LajAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            TeleportTimer = 20;    // It's sth about that
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(ALERGIC_REACTION);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SUMMON_THORN_LASHER);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 6.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(SUMMON_THORN_FLAYER);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 6.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(TELEPORT_SELF);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].cooldown = -1; // will take this spell separately as it needs additional coding for changing position
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            TeleportTimer = 20;
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget)
        {
            TeleportTimer = 20;
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnDied(Unit* mKiller)
        {
            TeleportTimer = 20;
            CastTime();
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            TeleportTimer--;

            if (!TeleportTimer)
            {
                _unit->SetPosition(-204.125000f, 391.248993f, -11.194300f, 0.017453f);    // \todo hmm doesn't work :S
                _unit->CastSpell(_unit, spells[3].info, spells[3].instant);
                TeleportTimer = 20;
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint32 TeleportTimer;
        uint8 nrspells;
};


// Warp Splinter AI
class WarpSplinterAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(WarpSplinterAI);
        SP_AI_Spell spells[3];
        bool m_spellcheck[3];

        WarpSplinterAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {

            SummonTimer = 20;    // It's sth about that
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(STOMP);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = -1;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = sSpellCustomizations.GetSpellInfo(SUMMON_SAPLINGS);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].cooldown = -1;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = sSpellCustomizations.GetSpellInfo(ARCANE_VOLLEY);
            spells[2].targettype = TARGET_VARIOUS;    // VARIOUS
            spells[2].instant = false;
            spells[2].cooldown = -1;
            spells[2].perctrigger = 12.0f;
            spells[2].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            SummonTimer = 20;
            CastTime();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
            sendDBChatMessage(SAY_WARP_SPLINTER_01);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void OnCombatStop(Unit* mTarget)
        {
            SummonTimer = 20;
            CastTime();
            setAIAgent(AGENT_NULL);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        sendDBChatMessage(SAY_WARP_SPLINTER_02);
                        break;
                    case 1:
                        sendDBChatMessage(SAY_WARP_SPLINTER_03);
                        break;
                }
            }
        }

        void OnDied(Unit* mKiller)
        {
            SummonTimer = 20;
            CastTime();
            RemoveAIUpdateEvent();
            sendDBChatMessage(SAY_WARP_SPLINTER_06);
        }

        void AIUpdate()
        {
            SummonTimer--;

            if (!SummonTimer)    // it will need more work on this spell in future (when this kind of spell will work)
            {
                /*for (uint8 i=0;i<5;i++)
                {
                _unit->CastSpell(_unit, spells[1].info, spells[1].instant);
                }*/
                _unit->CastSpell(_unit, spells[1].info, spells[1].instant);
                SummonTimer = 20;
                SummonSound();
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SummonSound()
        {
            switch (RandomUInt(1))
            {
                case 0:
                    sendDBChatMessage(SAY_WARP_SPLINTER_04);
                    break;
                case 1:
                    sendDBChatMessage(SAY_WARP_SPLINTER_05);
                    break;
            }
        }

        void SpellCast(float val)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    spells[i].casttime--;

                    if (m_spellcheck[i])
                    {
                        spells[i].casttime = spells[i].cooldown;
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
                                _unit->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
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

                    if ((val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger)) || !spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint32 SummonTimer;
        uint8 nrspells;
};

void SetupBotanica(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_BLOOD_PROTECTOR, &BloodProtectorAI::Create);
    mgr->register_creature_script(CN_BLOOD_GREENKEEPER, &BloodGreenkeeperAI::Create);
    mgr->register_creature_script(CN_SUN_CHEMIST, &SunchemistAI::Create);
    mgr->register_creature_script(CN_SUN_RESEARCHER, &SunResearcherAI::Create);
    mgr->register_creature_script(CN_COMMANDER_SARANNIS, &CommanderSarannisAI::Create);
    mgr->register_creature_script(CN_HIGH_BOTANIST_FREYWINN, &HighBotanistFreywinnAI::Create);
    mgr->register_creature_script(CN_THORNGRIN_THE_TENDER, &ThorngrinTheTenderAI::Create);
    mgr->register_creature_script(CN_LAJ, &LajAI::Create);
    mgr->register_creature_script(CN_WARP_SPLINTER, &WarpSplinterAI::Create);
}
