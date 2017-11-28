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

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"

// High Priestess Jeklik AI
const uint32 CN_JEKLIK = 14517;

const uint32 TRANSFORM_BAT = 23966; //\todo  MISSING CREATURE_NAME TO TRANSFORM
const uint32 CRUSHING_BLOW = 24257;
const uint32 CRUSHING_AOE_SILENCE = 24687;

const uint32 MIND_FLAY = 23953;
const uint32 SUMMON_BATS = 23974; //\todo  EFFECT :P
const uint32 SHADOW_WORD_PAIN = 24212;
const uint32 GREAT_HEAL = 29564;

// agro/transform sound -> 8417

class JeklikAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(JeklikAI);
        JeklikAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            //spells[0].info = sSpellCustomizations.GetSpellInfo(TRANSFORM_BAT);
            //spells[0].targettype = TARGET_SELF;
            //spells[0].instant = true;
            //spells[0].perctrigger = 0.0f;
            //spells[0].attackstoptimer = 1000; // 1sec
            //m_spellcheck[0] = true;

            //spells[1].info = sSpellCustomizations.GetSpellInfo(CRUSHING_BLOW);
            //spells[1].targettype = TARGET_ATTACKING;
            //spells[1].instant = false;
            //spells[1].perctrigger = 10.0f;
            //spells[1].attackstoptimer = 2000; // 1sec

            //spells[2].info = sSpellCustomizations.GetSpellInfo(CRUSHING_AOE_SILENCE);
            //spells[2].targettype = TARGET_VARIOUS;
            //spells[2].instant = false;
            //spells[2].perctrigger = 10.0f;
            //spells[2].attackstoptimer = 2000; // 1sec

            //// 2 phase spells
            //spells[3].info = sSpellCustomizations.GetSpellInfo(MIND_FLAY);
            //spells[3].targettype = TARGET_ATTACKING;
            //spells[3].instant = false;
            //spells[3].perctrigger = 10.0f;
            //spells[3].attackstoptimer = 6000; // 1sec

            //spells[4].info = sSpellCustomizations.GetSpellInfo(SHADOW_WORD_PAIN);
            //spells[4].targettype = TARGET_ATTACKING;
            //spells[4].instant = false;
            //spells[4].perctrigger = 10.0f;
            //spells[4].attackstoptimer = 2000; // 1sec

            //spells[5].info = sSpellCustomizations.GetSpellInfo(GREAT_HEAL);
            //spells[5].targettype = TARGET_SELF;
            //spells[5].instant = false;
            //spells[5].perctrigger = 10.0f;
            //spells[5].attackstoptimer = 10000; // 1sec
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            sendDBChatMessage(3201);     // Lord Hir'eek, grant me wings of vengeance!

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
            // bat transform
            //getCreature()->CastSpell(getCreature(), spells[0].info, spells[0].instant);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
            getCreature()->RemoveAura(TRANSFORM_BAT);
        }

        void AIUpdate() override
        {
            /*if (getCreature()->GetHealthPct() <= 50 && m_spellcheck[0])
            {
                m_spellcheck[0] = false;
                getCreature()->RemoveAura(TRANSFORM_BAT);
            }*/
        }
};

/*
Jeklik begins the encounter in bat form. In this form she has an AoE silence that also damages.
She randomly charges people in the group, and summons 6-8 bloodseeker bats once per minute.
When she drops below 50% HP, she reverts to priest form. Here she casts Shadow Word: Pain, Mind Flay, Chain Mind Flay and Greater Heal.
She also summons bomb bats which drop fire bombs on the ground which AOE DoT those inside.
¨*/

// High Priestess Venoxis AI

const uint32 CN_VENOXIS = 14507; //\todo  MISSING CREATURE_NAME TO TRANSFORM

const uint32 HOLY_NOVA = 23858; // various targets
const uint32 HOLY_FIRE = 23860; // various targets

const uint32 TRANSFORM_SNAKE = 23849;
const uint32 SPIT_POISON = 24688; // various targets

class VenoxisAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(VenoxisAI);
        VenoxisAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            //spells[0].info = sSpellCustomizations.GetSpellInfo(TRANSFORM_SNAKE);
            //spells[0].targettype = TARGET_SELF;
            //spells[0].instant = false;
            //spells[0].perctrigger = 0.0f;
            //spells[0].attackstoptimer = 1000; // 1sec
            //m_spellcheck[0] = true;

            //spells[1].info = sSpellCustomizations.GetSpellInfo(HOLY_NOVA);
            //spells[1].targettype = TARGET_VARIOUS;
            //spells[1].instant = false;
            //spells[1].perctrigger = 10.0f;
            //spells[1].attackstoptimer = 2000; // 2sec

            //spells[2].info = sSpellCustomizations.GetSpellInfo(HOLY_FIRE);
            //spells[2].targettype = TARGET_VARIOUS;
            //spells[2].instant = false;
            //spells[2].perctrigger = 10.0f;
            //spells[2].attackstoptimer = 2000; // 2sec

            //spells[3].info = sSpellCustomizations.GetSpellInfo(SPIT_POISON);
            //spells[3].targettype = TARGET_VARIOUS;
            //spells[3].instant = false;
            //spells[3].perctrigger = 10.0f;
            //spells[3].attackstoptimer = 2000; // 2sec
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
            RemoveAIUpdateEvent();
            getCreature()->RemoveAura(TRANSFORM_SNAKE);
        }

        void AIUpdate() override
        {
            //if (getCreature()->GetHealthPct() <= 50 && m_spellcheck[0])
            //{
            //    //cast snake transform
            //    getCreature()->CastSpell(getCreature(), spells[0].info, spells[0].instant);
            //    m_spellcheck[0] = false;
            //}
        }
};

/*
Venoxis comes with 4 snake adds. He stays in priest form initially, where he casts Holy Nova, Holy Fire, and Renew.
He can also cast Holy Wrath, a spell which jumps from person to person with damage increasing exponentially as it hits people (9000dmg per hit isn't uncommon).
He later shifts to snake form, where his melee damage goes up dramatically and he releases a poison cloud AoE (500dmg/tick).
*/


#define CN_MARLI

const uint32 SUMMON_SPIDERS = 24081; //\todo  SUMMON WILDS
const uint32 SPIDER_TRANSFORM = 24084;

/*
Mar'li has two main forms, like the other bosses in Zul'Gurub.
She starts off in her troll form where she can spawn adds and cast 30 yard AoE poison.
These spider adds quickly gain strength and size if not killed quickly.
When she transforms into her spider form she will web everyone standing near her in place and charge.
As soon as she webs everyone she will attack the person with the highest aggro that has not been webbed (usually a healer if they are out of range.)
*/

void SetupZulGurub(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_JEKLIK, &JeklikAI::Create);
    mgr->register_creature_script(CN_VENOXIS, &VenoxisAI::Create);
}
