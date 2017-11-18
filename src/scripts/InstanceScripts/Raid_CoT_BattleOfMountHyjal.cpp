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
#include "Objects/Faction.h"
#include "Spell/SpellMgr.h"
#include <Spell/Definitions/PowerType.h>

enum HyjalEvents
{
    HYJAL_EVENT_RAGE_WINTERCHILL,
    HYJAL_EVENT_ANETHERON,
    HYJAL_EVENT_KAZROGAL,
    HYJAL_EVENT_AZGALOR,
    HYJAL_EVENT_ARCHIMONDE
};

enum HyjalPhases
{
    HYJAL_PHASE_NOT_STARTED = 0,
    HYJAL_PHASE_RAGE_WINTERCHILL_COMPLETE,
    HYJAL_PHASE_ANETHERON_COMPLETE,
    HYJAL_PHASE_KAZROGAL_COMPLETE,
    HYJAL_PHASE_AZGALOR_COMPLETE,
    HYJAL_PHASE_ARCHIMONDE_COMPLETE,
};

enum HyjalType
{
    HYJAL_TYPE_BASIC = 0,
    HYJAL_TYPE_END
};

class MountHyjalScript : public InstanceScript
{
    public:

        MountHyjalScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
        {
            InstanceData[HYJAL_TYPE_BASIC][0] = HYJAL_PHASE_NOT_STARTED;
        }

        static InstanceScript* Create(MapMgr* pMapMgr) { return new MountHyjalScript(pMapMgr); }

        void SetLocaleInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
        {
            if (pType >= HYJAL_TYPE_END || pIndex >= 10)
                return;

            InstanceData[pType][pIndex] = pData;
        }

        uint32 GetLocaleInstanceData(uint32 pType, uint32 pIndex)
        {
            if (pType >= HYJAL_TYPE_END || pIndex >= 10)
                return 0;

            return InstanceData[pType][pIndex];
        }

    private:
        uint32 InstanceData[HYJAL_TYPE_END][10]; // Expand this to fit your needs.
        // Type 0 = Basic Data;
        //   Index 0 = Current Phase;
};
//Jaina Proudmoore AI & GS
const uint32 CN_JAINA_PROUDMOORE = 17772;

class JainaProudmooreAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(JainaProudmooreAI);
        JainaProudmooreAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            getCreature()->setUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
};

class JainaProudmooreGS : public Arcemu::Gossip::Script
{
    public:
        void OnHello(Object* pObject, Player* plr) override
        {
            if (pObject->GetMapMgr()->GetMapId() != MAP_HYJALPAST)//in case someone spawned this NPC in another map
                return;

            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 2);
            switch (static_cast<MountHyjalScript*>(pObject->GetMapMgr()->GetScript())->GetLocaleInstanceData(HYJAL_TYPE_BASIC, 0))
            {
                case HYJAL_PHASE_NOT_STARTED:
                    menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(435), 1);     // We are ready to defend the Alliance base.
                    break;
                case HYJAL_PHASE_RAGE_WINTERCHILL_COMPLETE:
                    menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(435), 1);     // We are ready to defend the Alliance base.
                    break;
                case HYJAL_PHASE_ANETHERON_COMPLETE:
                    menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(436), 1);     // The defenses are holding up: we can continue.
                    break;
            }

            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* /*Plr*/, uint32 /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
        {
            if (pObject->GetMapMgr()->GetMapId() != MAP_HYJALPAST)//in case someone spawned this NPC in another map
                return;

            switch (static_cast<MountHyjalScript*>(pObject->GetMapMgr()->GetScript())->GetLocaleInstanceData(HYJAL_TYPE_BASIC, 0))
            {
                case HYJAL_PHASE_NOT_STARTED:
                case HYJAL_PHASE_RAGE_WINTERCHILL_COMPLETE:
                case HYJAL_PHASE_ANETHERON_COMPLETE:
                    break;
            }

            pObject->setUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        }
};

//Thrall AI & GS
const uint32 CN_THRALL = 17852;

class ThrallAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ThrallAI);
        ThrallAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            getCreature()->setUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
};

class ThrallGS : public Arcemu::Gossip::Script
{
    public:
        void OnHello(Object* pObject, Player* plr) override
        {
            if (pObject->GetMapMgr()->GetMapId() != MAP_HYJALPAST)//in case someone spawned this NPC in another map
                return;

            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 2);
            switch (static_cast<MountHyjalScript*>(pObject->GetMapMgr()->GetScript())->GetLocaleInstanceData(HYJAL_TYPE_BASIC, 0))
            {
                case HYJAL_PHASE_ANETHERON_COMPLETE:
                    menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(437), 1);     // We're here to help! The Alliance are overrun.
                    break;
                case HYJAL_PHASE_KAZROGAL_COMPLETE:
                    menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(438), 1);     // We're okay so far. Let's do this!
                    break;
            }

            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* /*Plr*/, uint32 /*Id*/, const char* /*Code*/, uint32_t /*gossipId*/) override
        {
            if (pObject->GetMapMgr()->GetMapId() != MAP_HYJALPAST)//in case someone spawned this NPC in another map
                return;

            switch (static_cast<MountHyjalScript*>(pObject->GetMapMgr()->GetScript())->GetLocaleInstanceData(HYJAL_TYPE_BASIC, 0))
            {
                case HYJAL_PHASE_ANETHERON_COMPLETE:
                case HYJAL_PHASE_KAZROGAL_COMPLETE:
                    break;
            }

            pObject->setUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        }
};


// Rage WinterchillAI
const uint32 CN_RAGE_WINTERCHILL = 17767;

const uint32 FROSTBOLT = 31249;           // it's not correct spell for sure (not sure to others too :P)
const uint32 DEATCH_AND_DECAY = 31258;
const uint32 FROST_NOVA = 31250;
const uint32 FROST_ARMOR = 31256;

class RageWinterchillAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(RageWinterchillAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        RageWinterchillAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(FROSTBOLT);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = true;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].cooldown = 10;
            spells[0].mindist2cast = 0.0f;
            spells[0].maxdist2cast = 80.0f;

            spells[1].info = sSpellCustomizations.GetSpellInfo(DEATCH_AND_DECAY);
            spells[1].targettype = TARGET_RANDOM_DESTINATION;
            spells[1].instant = false;
            spells[1].perctrigger = 3.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].cooldown = 30;
            spells[1].mindist2cast = 0.0f;
            spells[1].maxdist2cast = 30.0f;

            spells[2].info = sSpellCustomizations.GetSpellInfo(FROST_NOVA);
            spells[2].targettype = TARGET_RANDOM_SINGLE;
            spells[2].instant = true;
            spells[2].perctrigger = 5.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].cooldown = 15;
            spells[2].mindist2cast = 0.0f;
            spells[2].maxdist2cast = 45.0f;

            spells[3].info = sSpellCustomizations.GetSpellInfo(FROST_ARMOR);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = true;
            spells[3].perctrigger = 5.0f;
            spells[3].attackstoptimer = 1000;
            spells[3].cooldown = 10;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            sendDBChatMessage(1590);     // The Legion's final conquest has begun! Once again the subjugation of this world is within our grasp. Let

            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* /*mTarget*/) override
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                switch (RandomUInt(4))
                {
                    case 0:
                        sendDBChatMessage(1586);     // Crumble and rot!
                        break;
                    case 1:
                        sendDBChatMessage(1587);     // Ashes to ashes, dust to dust
                        break;
                    case 2:
                        sendDBChatMessage(1584);     // All life must perish!");
                        break;
                    case 3:
                        sendDBChatMessage(1585);     // Victory to the Legion!");
                        break;
                    default:
                        break;
                }
            }
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            sendDBChatMessage(1583);     // You have won this battle, but not... the...war
        }

        void AIUpdate() override
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt
                for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(getCreature(), (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(getCreature(), (*itr)) && (*itr) != getCreature())) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && getCreature()->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (getCreature()->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(getCreature(), RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (getCreature()->GetHealthPct() >= minhp2cast && getCreature()->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(getCreature());

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        getCreature()->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        getCreature()->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:

        uint8 nrspells;
};

// AnetheronAI

const uint32 CN_ANETHERON = 17808;

const uint32 CARRION_SWARM = 31306;
const uint32 VAMPIRIC_AURA = 38196;   // 31317
const uint32 INFERNO = 31299;    // doesn't summon infernal - core bug
const uint32 SLEEP = 31298;   // 12098
const uint32 BERSERK = 26662;

class AnetheronAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AnetheronAI);
        SP_AI_Spell spells[5];
        bool m_spellcheck[5];

        AnetheronAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(CARRION_SWARM);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = true;
            spells[0].perctrigger = 10.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].cooldown = 10;
            spells[0].mindist2cast = 0.0f;
            spells[0].maxdist2cast = 60.0f;

            spells[1].info = sSpellCustomizations.GetSpellInfo(VAMPIRIC_AURA);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].perctrigger = 8.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].cooldown = 8;

            spells[2].info = sSpellCustomizations.GetSpellInfo(INFERNO);
            spells[2].targettype = TARGET_RANDOM_DESTINATION;
            spells[2].instant = false;
            spells[2].perctrigger = 6.0f;
            spells[2].attackstoptimer = 3000;
            spells[2].cooldown = 30;
            spells[2].mindist2cast = 0.0f;
            spells[2].maxdist2cast = 60.0f;
            spells[2].speech = "Hit he, no time for a slow death";
            spells[2].soundid = 11039;

            spells[3].info = sSpellCustomizations.GetSpellInfo(SLEEP);
            spells[3].targettype = TARGET_RANDOM_SINGLE;
            spells[3].instant = true;
            spells[3].perctrigger = 5.0f;
            spells[3].attackstoptimer = 3000;
            spells[3].cooldown = 7;
            spells[3].mindist2cast = 0.0f;
            spells[3].maxdist2cast = 30.0f;

            spells[4].info = sSpellCustomizations.GetSpellInfo(BERSERK);
            spells[4].targettype = TARGET_SELF;
            spells[4].instant = true;
            spells[4].cooldown = 600;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            sendDBChatMessage(1569);     // You are defenders of a doomed world. Flee here and perhaps you will prolong your pathetic lives!

            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            uint32 t = (uint32)time(NULL);
            spells[4].casttime = t + spells[4].cooldown;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* /*mTarget*/) override
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                switch (RandomUInt(2))
                {
                    case 0:
                        sendDBChatMessage(1560);     // Your hopes are lost.
                        break;
                    case 1:
                        sendDBChatMessage(1561);     // Scream for me.
                        break;
                    case 2:
                        sendDBChatMessage(1565);     // You look tired
                        break;
                }
            }
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            sendDBChatMessage(1559);     // The clock... is still...ticking.
        }

        void AIUpdate() override
        {
            uint32 t = (uint32)time(NULL);
            if (t > spells[4].casttime)
            {
                getCreature()->CastSpell(getCreature(), spells[4].info, spells[4].instant);

                spells[4].casttime = t + spells[4].cooldown;
                return;
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        if (i == 1)
                        {
                            Aura* aura = sSpellFactoryMgr.NewAura(spells[1].info, (uint32)5000, getCreature(), getCreature());
                            getCreature()->AddAura(aura);
                        }

                        else
                        {
                            target = getCreature()->GetAIInterface()->getNextTarget();
                            switch (spells[i].targettype)
                            {
                                case TARGET_SELF:
                                case TARGET_VARIOUS:
                                    getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                    break;
                                case TARGET_ATTACKING:
                                    getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                    break;
                                case TARGET_DESTINATION:
                                    getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                    break;
                                case TARGET_RANDOM_FRIEND:
                                case TARGET_RANDOM_SINGLE:
                                case TARGET_RANDOM_DESTINATION:
                                    CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                    break;
                            }
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt
                for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(getCreature(), (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(getCreature(), (*itr)) && (*itr) != getCreature())) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && getCreature()->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (getCreature()->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(getCreature(), RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (getCreature()->GetHealthPct() >= minhp2cast && getCreature()->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(getCreature());

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        getCreature()->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        getCreature()->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:

        uint8 nrspells;
};

// KazrogalAI

const uint32 CN_KAZROGAL = 17888;

const uint32 K_CLEAVE = 31345;
const uint32 WAR_STOMP = 31480;
const uint32 MARK_OF_KAZROGAL = 31447;
const uint32 MARK_OF_KAZROGAL2 = 31463;   // should it be scripted to attack friends?

class KazrogalAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(KazrogalAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        KazrogalAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(K_CLEAVE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].cooldown = 10;

            spells[1].info = sSpellCustomizations.GetSpellInfo(WAR_STOMP);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].perctrigger = 6.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].cooldown = 15;

            spells[2].info = sSpellCustomizations.GetSpellInfo(MARK_OF_KAZROGAL);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = false;
            spells[2].perctrigger = 4.0f;
            spells[2].attackstoptimer = 2000;
            spells[2].cooldown = 25;

            spells[3].info = sSpellCustomizations.GetSpellInfo(MARK_OF_KAZROGAL2);
            spells[3].targettype = TARGET_VARIOUS;
            spells[3].instant = true;

            MarkDeto = 0;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            sendDBChatMessage(1582);     // Cry for mercy! Your meaningless lives will soon be forfeit.

            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* /*mTarget*/) override
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                switch (RandomUInt(3))
                {
                    case 0:
                        sendDBChatMessage(1580);     // Your death will be a painful one.
                        break;
                    case 1:
                        sendDBChatMessage(1581);     // You... are marked
                        break;
                    case 2:
                        sendDBChatMessage(1578);     // You... are nothing!
                        break;
                    case 3:
                        sendDBChatMessage(1579);     // Miserable nuisance!
                        break;
                }
            }
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            ///\todo move this to db
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "hahahahaa aahaah");
            getCreature()->PlaySoundToSet(11018);
        }

        void AIUpdate() override
        {
            if (MarkDeto)
                MarkCast();

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }

                        if (i == 2)
                            MarkCast();

                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void MarkCast()
        {
            for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
            {
                if (isHostile(getCreature(), (*itr)) && (*itr)->IsUnit())
                {
                    Unit* Target = NULL;
                    Target = static_cast<Unit*>(*itr);

                    if (Target->isAlive() && Target->getAuraWithId(MARK_OF_KAZROGAL) && (Target->GetPowerType() != POWER_TYPE_MANA || !Target->GetBaseMana()))
                    {
                        Target->CastSpell(Target, spells[3].info, spells[3].instant);
                    }
                }
            }

            MarkDeto++;
            if (MarkDeto == 3)
                MarkDeto = 0;
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt
                for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(getCreature(), (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(getCreature(), (*itr)) && (*itr) != getCreature())) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && getCreature()->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (getCreature()->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(getCreature(), RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (getCreature()->GetHealthPct() >= minhp2cast && getCreature()->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(getCreature());

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        getCreature()->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        getCreature()->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:

        uint32 MarkDeto;
        uint8 nrspells;
};

// AzgalorAI

const uint32 CN_AZGALOR = 17842;

const uint32 CLEAVE = 31345;
const uint32 RAIN_OF_FIRE = 31340;
const uint32 HOWL_OF_AZGALOR = 31344;
const uint32 DOOM = 31347;    // it's applied, but doesn't do anything more - should be scripted?

class AzgalorAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AzgalorAI);
        SP_AI_Spell spells[4];
        bool m_spellcheck[4];

        AzgalorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(CLEAVE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 8.0f;
            spells[0].attackstoptimer = 3000;
            spells[0].cooldown = 10;

            spells[1].info = sSpellCustomizations.GetSpellInfo(RAIN_OF_FIRE);
            spells[1].targettype = TARGET_RANDOM_DESTINATION;
            spells[1].instant = true;
            spells[1].perctrigger = 7.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].cooldown = 15;
            spells[1].mindist2cast = 0.0f;
            spells[1].maxdist2cast = 40.0f;

            spells[2].info = sSpellCustomizations.GetSpellInfo(HOWL_OF_AZGALOR);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = true;
            spells[2].perctrigger = 5.0f;
            spells[2].attackstoptimer = 2000;
            spells[2].cooldown = 15;

            spells[3].info = sSpellCustomizations.GetSpellInfo(DOOM);
            spells[3].targettype = TARGET_RANDOM_SINGLE;
            spells[3].instant = true;
            spells[3].cooldown = 45;
            spells[3].mindist2cast = 0.0f;
            spells[3].maxdist2cast = 50.0f;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            sendDBChatMessage(1576);     // Abandon all hope! The legion has returned to finish what was begun so many years ago. This time there will be no escape!

            for (uint8 i = 0; i < 3; i++)
                spells[i].casttime = 0;

            uint32 t = (uint32)time(NULL);
            spells[3].casttime = t + spells[3].cooldown;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* /*mTarget*/) override
        {
            if (getCreature()->GetHealthPct() > 0)
            {
                switch (RandomUInt(2))
                {
                    case 0:
                        sendDBChatMessage(1571);     // Reesh, hokta!
                        break;
                    case 1:
                        sendDBChatMessage(1573);     // No one is going to save you!
                        break;
                    case 2:
                        sendDBChatMessage(1572);     // Don't fight it
                        break;
                }
            }
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            sendDBChatMessage(1570);     // Your time is almost... up!
        }

        void AIUpdate() override
        {
            uint32 t = (uint32)time(NULL);
            if (t > spells[3].casttime)
            {
                CastSpellOnRandomTarget(3, spells[3].mindist2cast, spells[3].maxdist2cast, 0, 100);

                spells[3].casttime = t + spells[3].cooldown;
                return;
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime && (i != 1 || (i == 1 && getCreature()->GetHealthPct() >= 20)))
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt
                for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(getCreature(), (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(getCreature(), (*itr)) && (*itr) != getCreature())) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && getCreature()->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (getCreature()->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(getCreature(), RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (getCreature()->GetHealthPct() >= minhp2cast && getCreature()->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(getCreature());

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                if (i == 3)
                {
                    Aura* aura = sSpellFactoryMgr.NewAura(spells[3].info, (uint32)20000, getCreature(), random_target);
                    random_target->AddAura(aura);

                    TargetTable.clear();
                    return;
                }
                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        getCreature()->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        getCreature()->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:

        uint8 nrspells;
};

// Archimonde Channel TriggerAI

const uint32 CN_ARCHIMONDE_CHANNEL_TRIGGER = 30004;

// Additional
const uint32 DRAIN_WORLD_TREE_VISUAL = 39140;
const uint32 DRAIN_WORLD_TREE_VISUAL2 = 39141;

class ArchimondeTriggerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ArchimondeTriggerAI);
        ArchimondeTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            getCreature()->m_noRespawn = true;

            Unit* Archimonde = getNearestCreature(5598.629883f, -3447.719971f, 1576.650024f, 17968);
            if (Archimonde)
            {
                getCreature()->SetChannelSpellTargetGUID(Archimonde->GetGUID());
                getCreature()->SetChannelSpellId(DRAIN_WORLD_TREE_VISUAL2);
            }
        }
};

// DoomfireAI

const uint32 CN_DOOMFIRE = 18095;

class DoomfireAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(DoomfireAI);
        DoomfireAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
            getCreature()->m_noRespawn = true;

            RegisterAIUpdateEvent(1000);

            DespawnTimer = 0;
            DirChange = 0;
        }

        void AIUpdate() override
        {
            DespawnTimer++;
            if (DespawnTimer >= 27)
            {
                getCreature()->Despawn(0, 0);
                return;
            }
            // After 4 sec of last direction change, doomfire has 33% chance to change direction
            DirChange++;
            if ((DirChange == 4 && RandomUInt(3) == 1) || DirChange >= 5)
            {
                if (getCreature()->GetAIInterface()->getUnitToFollow())
                {
                    if (RandomUInt(3) == 1 || getCreature()->GetDistance2dSq(getCreature()->GetAIInterface()->getUnitToFollow()) <= 2.0f)
                    {
                        getCreature()->GetAIInterface()->ResetUnitToFollow();
                        getCreature()->GetAIInterface()->SetUnitToFollowAngle(0.0f);
                    }
                }

                if (!getCreature()->GetAIInterface()->getUnitToFollow())
                {
                    if (RandomUInt(3) == 1)
                    {
                        Unit* NewTarget = NULL;
                        NewTarget = FindTarget();
                        if (NewTarget)
                        {
                            getCreature()->GetAIInterface()->SetUnitToFollow(NewTarget);
                            getCreature()->GetAIInterface()->SetUnitToFollowAngle(2.0f);
                        }
                    }

                    if (!getCreature()->GetAIInterface()->getUnitToFollow())
                    {
                        float movedist = 10.0f;
                        float x = 0.0f;
                        float y = 0.0f;

                        float xchange = RandomFloat(movedist);
                        float ychange = sqrt(movedist * movedist - xchange * xchange);

                        if (RandomUInt(2) == 1)
                            xchange *= -1;
                        if (RandomUInt(2) == 1)
                            ychange *= -1;

                        x = getCreature()->GetPositionX() + xchange;
                        y = getCreature()->GetPositionY() + ychange;

                        getCreature()->GetAIInterface()->MoveTo(x, y, getCreature()->GetPositionZ());
                    }
                }

                DirChange = 0;
            }
        }
        // A bit rewritten FindTarget function
        Unit* FindTarget()
        {
            Unit* target = NULL;
            float distance = 15.0f;
            float z_diff;

            Unit* pUnit;
            float dist;

            for (std::set<Object*>::iterator itr = getCreature()->GetInRangeOppFactsSetBegin(); itr != getCreature()->GetInRangeOppFactsSetEnd(); ++itr)
            {
                if (!(*itr)->IsUnit())
                    continue;

                pUnit = static_cast<Unit*>((*itr));

                if (pUnit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH))
                    continue;

                z_diff = fabs(getCreature()->GetPositionZ() - pUnit->GetPositionZ());
                if (z_diff > 2.5f)
                    continue;

//            if (pUnit->m_auracount[SPELL_AURA_MOD_INVISIBILITY])
//                continue;

                if (!pUnit->isAlive() || getCreature() == pUnit)
                    continue;

                dist = getCreature()->GetDistance2dSq(pUnit);

                if (dist > distance * distance)
                    continue;

                if (dist < 3.0f)
                    continue;

                distance = dist;
                target = pUnit;
            }

            return target;
        }

    protected:

        uint32 DespawnTimer;
        uint32 DirChange;
};

/* * Doomfire - Leaves a trail of fire on the ground, which does 2400 fire
        damage per second (occasionally feared people run into these and die) */

// ArchimondeAI

const uint32 CN_ARCHIMONDE = 17968;

const uint32 FEAR = 33547;
const uint32 AIR_BURST = 32014;
const uint32 GRIP_OF_THE_LEGION = 31972;
const uint32 DOOMFIRE_STRIKE = 31903;
const uint32 FINGER_OF_DEATH = 31984;   // should be casted only when no one in melee range
const uint32 HAND_OF_DEATH = 35354;   // used if too close to Well of Eternity or if after 10 min caster has more than 10% hp
const uint32 SOUL_CHARGER = 32053;   // If player dies whole raid gets one of those 3 buffs
const uint32 SOUL_CHARGEO = 32054;
const uint32 SOUL_CHARGEG = 32057;

class ArchimondeAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ArchimondeAI);
        SP_AI_Spell spells[7];
        bool m_spellcheck[7];

        ArchimondeAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = sSpellCustomizations.GetSpellInfo(AIR_BURST);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].perctrigger = 5.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].cooldown = 25;

            spells[1].info = sSpellCustomizations.GetSpellInfo(GRIP_OF_THE_LEGION);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = true;
            spells[1].perctrigger = 5.0f;
            spells[1].attackstoptimer = 1000;
            spells[1].cooldown = 25;
            spells[1].mindist2cast = 0.0f;
            spells[1].maxdist2cast = 60.0f;

            spells[2].info = sSpellCustomizations.GetSpellInfo(DOOMFIRE_STRIKE);
            spells[2].targettype = TARGET_VARIOUS;
            spells[2].instant = true;
            spells[2].perctrigger = 7.0f;
            spells[2].attackstoptimer = 2000;

            spells[3].info = sSpellCustomizations.GetSpellInfo(FEAR);
            spells[3].targettype = TARGET_VARIOUS;
            spells[3].instant = true;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;
            spells[3].cooldown = 40;

            spells[4].info = sSpellCustomizations.GetSpellInfo(FINGER_OF_DEATH);
            spells[4].targettype = TARGET_RANDOM_SINGLE;
            spells[4].instant = false;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;
            spells[4].mindist2cast = 30.0f;
            spells[4].maxdist2cast = 80.0f;

            spells[5].info = sSpellCustomizations.GetSpellInfo(HAND_OF_DEATH);
            spells[5].targettype = TARGET_VARIOUS;
            spells[5].instant = true;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 1000;
            spells[5].cooldown = 600;

            spells[6].instant = false;
            spells[6].cooldown = 10;

            Trigger = spawnCreature(CN_ARCHIMONDE_CHANNEL_TRIGGER, 5501.476563f, -3524.868408f, 1604.188965f, 0.393633f);

            if (Trigger && Trigger->IsInWorld())
            {
                Trigger->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                Trigger->GetAIInterface()->SetAllowedToEnterCombat(false);
                Trigger->m_noRespawn = true;

                Trigger->SetChannelSpellTargetGUID(getCreature()->GetGUID());
                Trigger->SetChannelSpellId(DRAIN_WORLD_TREE_VISUAL2);

                getCreature()->SetChannelSpellTargetGUID(Trigger->GetGUID());
                getCreature()->SetChannelSpellId(DRAIN_WORLD_TREE_VISUAL);
            }
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            sendDBChatMessage(1591);     // Your resistance is insignificant.

            getCreature()->SetChannelSpellTargetGUID(0);
            getCreature()->SetChannelSpellId(0);

            if (Trigger && Trigger->IsInWorld())
            {
                Trigger->SetChannelSpellTargetGUID(0);
                Trigger->SetChannelSpellId(0);
            }

            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            uint32 t = (uint32)time(NULL);
            spells[3].casttime = t + spells[3].cooldown;
            spells[5].casttime = t + spells[5].cooldown;
            spells[6].casttime = 0;

            RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
        }

        void OnTargetDied(Unit* mTarget) override
        {
            if (getCreature()->GetHealthPct() > 0)    // Hack to prevent double yelling (OnDied and OnTargetDied when creature is dying)
            {
                switch (RandomUInt(2))
                {
                    case 0:
                        sendDBChatMessage(1597);     // Your soul will languish for eternity.
                        break;
                    case 1:
                        sendDBChatMessage(1596);     // All creation will be devoured!
                        break;
                    case 2:
                        sendDBChatMessage(1598);     // I am the coming of the end!
                        break;
                }

                uint32 t = (uint32)time(NULL);
                if (mTarget->IsPlayer() && isAlive() && !getCreature()->isCastingNonMeleeSpell() && t > spells[6].casttime)
                {
                    uint32 SpellID = 0;
                    if (mTarget->getClass() == WARRIOR || mTarget->getClass() == ROGUE || mTarget->getClass() == MAGE)
                        SpellID = SOUL_CHARGEO;
                    else if (mTarget->getClass() == PRIEST || mTarget->getClass() == PALADIN || mTarget->getClass() == WARLOCK)
                        SpellID = SOUL_CHARGER;
                    else
                        SpellID = SOUL_CHARGEG;

                    getCreature()->CastSpell(getCreature(), sSpellCustomizations.GetSpellInfo(SpellID), spells[6].instant);

                    spells[6].casttime = t + spells[6].cooldown;
                }
            }
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setAIAgent(AGENT_NULL);
            getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

            if (Trigger && Trigger->IsInWorld() && isAlive())
            {
                Trigger->SetChannelSpellTargetGUID(getCreature()->GetGUID());
                Trigger->SetChannelSpellId(DRAIN_WORLD_TREE_VISUAL2);

                getCreature()->SetChannelSpellTargetGUID(Trigger->GetGUID());
                getCreature()->SetChannelSpellId(DRAIN_WORLD_TREE_VISUAL);
            }

            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            sendDBChatMessage(1600);     // No, it cannot be! Nooo!
        }

        void AIUpdate() override
        {
            //setAIAgent(AGENT_NULL);

            uint32 t = (uint32)time(NULL);
            if (t > spells[3].casttime && !getCreature()->isCastingNonMeleeSpell())
            {
                getCreature()->CastSpell(getCreature(), spells[3].info, spells[3].instant);

                spells[3].casttime = t + spells[3].cooldown;
            }

            else if (t > spells[5].casttime && !getCreature()->isCastingNonMeleeSpell())
            {
                getCreature()->CastSpell(getCreature(), spells[5].info, spells[5].instant);

                spells[5].casttime = t + spells[5].cooldown;
            }
            /*    Crashes server if Archimonde kills player o_O (even with TARGET_ATTACKING, without StopMovement, without setting current agent and so on
                    else if (_unit->GetAIInterface()->GetNextTarget())
                    {
                        if (FingerOfDeath())
                        {
                            setAIAgent(AGENT_SPELL);
                            _unit->GetAIInterface()->StopMovement(2000);

                            if (_unit->GetCurrentSpell() == NULL)
                            {
                                CastSpellOnRandomTarget(4, spells[4].mindist2cast, spells[4].maxdist2cast, 0, 100);
                                return;
                            }
                        }
                    }
            */
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void SpellCast(float val)
        {
            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (uint8 i = 0; i < nrspells; i++)
                {
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = getCreature()->GetAIInterface()->getNextTarget();
                        switch (spells[i].targettype)
                        {
                            case TARGET_SELF:
                            case TARGET_VARIOUS:
                                getCreature()->CastSpell(getCreature(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_ATTACKING:
                                getCreature()->CastSpell(target, spells[i].info, spells[i].instant);
                                break;
                            case TARGET_DESTINATION:
                                getCreature()->CastSpellAoF(target->GetPosition(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
                                break;
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        getCreature()->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (!getCreature()->isCastingNonMeleeSpell() && getCreature()->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt
                for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(getCreature(), (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(getCreature(), (*itr)) && (*itr) != getCreature())) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && getCreature()->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && getCreature()->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (getCreature()->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(getCreature(), RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (getCreature()->GetHealthPct() >= minhp2cast && getCreature()->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(getCreature());

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, uint32(TargetTable.size() - 1));
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_FRIEND:
                    case TARGET_RANDOM_SINGLE:
                        getCreature()->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        getCreature()->CastSpellAoF(random_target->GetPosition(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

        bool FingerOfDeath()
        {
            Unit* NextTarget = NULL;

            for (std::set<Object*>::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
            {
                if (isHostile(getCreature(), (*itr)) && (*itr)->IsUnit() && getCreature()->GetDistance2dSq((*itr)) <= spells[4].mindist2cast * spells[4].mindist2cast)
                {
                    NextTarget = static_cast<Unit*>(*itr);
                    if (NextTarget->isAlive() && getCreature()->GetAIInterface()->getThreatByPtr(NextTarget) > 0)
                    {
                        getCreature()->GetAIInterface()->WipeTargetList();
                        getCreature()->GetAIInterface()->WipeHateList();
                        getCreature()->GetAIInterface()->AttackReaction(NextTarget, 1, 0);
                        return false;
                    }
                }
            }

            return true;
        }

    protected:
        Creature* Trigger;
        uint8 nrspells;
};

void SetupBattleOfMountHyjal(ScriptMgr* mgr)
{
    /*mgr->register_instance_script(MAP_HYJALPAST, &MountHyjalScript::Create);*/

    mgr->register_creature_gossip(CN_JAINA_PROUDMOORE, new JainaProudmooreGS());
    mgr->register_creature_script(CN_JAINA_PROUDMOORE, &JainaProudmooreAI::Create);

    mgr->register_creature_gossip(CN_THRALL, new ThrallGS());
    mgr->register_creature_script(CN_THRALL, &ThrallAI::Create);

    mgr->register_creature_script(CN_RAGE_WINTERCHILL, &RageWinterchillAI::Create);
    mgr->register_creature_script(CN_ANETHERON, &AnetheronAI::Create);
    mgr->register_creature_script(CN_KAZROGAL, &KazrogalAI::Create);
    mgr->register_creature_script(CN_AZGALOR, &AzgalorAI::Create);
    mgr->register_creature_script(CN_ARCHIMONDE_CHANNEL_TRIGGER, &ArchimondeTriggerAI::Create);
    mgr->register_creature_script(CN_DOOMFIRE, &DoomfireAI::Create);
    mgr->register_creature_script(CN_ARCHIMONDE, &ArchimondeAI::Create);
}
