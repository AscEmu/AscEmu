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


// Partially by Plexor (I used a spell before, but changed to his method)
class Berthold : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* Plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 4037, Plr);

            Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(428), 1);     // What is this place?
            Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(429), 2);     // Where is Medivh?
            Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(430), 3);     // How do you navigate the tower?

            //Killing the Shade of Aran makes a teleport to medivh's available from Berthold the Doorman.
            Unit* soa = pObject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-11165.2f, -1912.13f, 232.009f, 16524);
            if (!soa || !soa->isAlive())
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(431), 4); // Please teleport me to the Guardian's Library.

            Menu->SendTo(Plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            switch (IntId)
            {
                case 0:
                    GossipHello(pObject, Plr);
                    break;

                case 4:
                    Plr->SafeTeleport(Plr->GetMapId(), Plr->GetInstanceID(), -11165.123f, -1911.13f, 232.009f, 2.3255f);
                    break;
            }
        }

};

/////////////////////////////////////////////////////////////////////
//Attumen the Huntsman (and Midnight)
#define CN_MIDNIGHT                    16151
#define CN_ATTUMEN                    15550
#define ATTUMEN_SHADOW_CLEAVE        29832
#define ATTUMEN_BERSERKER_CHARGE    22886
#define ATTUMEN_INTANGIBLE_PRESENCE    29833

class AttumenTheHuntsmanAI : public MoonScriptBossAI
{
        MOONSCRIPT_FACTORY_FUNCTION(AttumenTheHuntsmanAI, MoonScriptBossAI);
        AttumenTheHuntsmanAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            //All phase spells
            AddSpell(ATTUMEN_SHADOW_CLEAVE, Target_Current, 15, 0, 6, 0, 5, true);
            AddSpell(ATTUMEN_INTANGIBLE_PRESENCE, Target_Current, 15, 0, 12, 0, 5, true);

            //Phase 2 spells
            AddPhaseSpell(2, AddSpell(ATTUMEN_BERSERKER_CHARGE, Target_RandomPlayerNotCurrent, 15, 0, 6, 15, 40, true));

            //Emotes
            AddEmote(Event_OnCombatStart, "Cowards! Wretches!", Text_Yell, 9167);
            AddEmote(Event_OnCombatStart, "Who dares attack the steed of the Huntsman?", Text_Yell, 9298);
            AddEmote(Event_OnCombatStart, "Perhaps you would rather test yourselves against a more formidable opponent!", Text_Yell, 9299);
            AddEmote(Event_OnTargetDied, "It was... inevitable.", Text_Yell, 9169);
            AddEmote(Event_OnTargetDied, "Another trophy to add to my collection!", Text_Yell, 9300);
            AddEmote(Event_OnDied, "Always knew... someday I would become... the hunted.", Text_Yell, 9165);
            AddEmote(Event_OnTaunt, "Such easy sport.", Text_Yell, 9170);
            AddEmote(Event_OnTaunt, "Amatures! Do not think you can best me! I kill for a living.", Text_Yell, 9304);
        }

        void OnLoad()
        {
            AggroNearestUnit(); //Aggro on spawn
        }

        void OnCombatStop(Unit* pTarget)
        {
            Despawn(10000);
            ParentClass::OnCombatStop(pTarget);
        }

        void AIUpdate()
        {
            if (GetPhase() == 1)
            {
                if (GetLinkedCreature() && GetLinkedCreature()->IsAlive() && GetHealthPercent() <= 25 && !IsCasting())
                {
                    SetPhase(2);
                    SetAllowMelee(false);
                    SetAllowSpell(false);
                    Emote("Come Midnight, let's disperse this petty rabble!", Text_Yell, 9168);
                    MoonScriptBossAI* midnight = static_cast< MoonScriptBossAI* >(GetLinkedCreature());
                    midnight->SetPhase(2);
                    midnight->MoveTo(this);
                    midnight->SetAllowMelee(false);
                }
            }
            ParentClass::AIUpdate();
        }
};

class MidnightAI : public MoonScriptBossAI
{
        MOONSCRIPT_FACTORY_FUNCTION(MidnightAI, MoonScriptBossAI);
        MidnightAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
        }

        void OnCombatStop(Unit* pTarget)
        {
            SetAllowMelee(true);
            SetAllowSpell(true);
            ParentClass::OnCombatStop(pTarget);
        }

        void OnTargetDied(Unit* pTarget)
        {
            if (GetLinkedCreature() && GetLinkedCreature()->IsAlive())
            {
                static_cast< MoonScriptCreatureAI* >(GetLinkedCreature())->Emote("Well done Midnight!", Text_Yell, 9173);
            }
            ParentClass::OnTargetDied(pTarget);
        }

        void AIUpdate()
        {
            if (GetPhase() == 1)
            {
                if (GetLinkedCreature() == NULL && GetHealthPercent() <= 95 && !IsCasting())
                {
                    Emote("Midnight calls for her master!", Text_Emote);
                    CreatureAIScript* attumen = SpawnCreature(CN_ATTUMEN);
                    if (attumen != NULL)
                    {
                        SetLinkedCreature(attumen);
                        attumen->SetLinkedCreature(this);
                    }
                }
                else if (GetLinkedCreature() && GetLinkedCreature()->IsAlive() && GetHealthPercent() <= 25 && !IsCasting())
                {
                    SetPhase(2);
                    MoonScriptBossAI* attumen = static_cast< MoonScriptBossAI* >(GetLinkedCreature());
                    MoveTo(attumen);
                    SetAllowMelee(false);
                    attumen->SetPhase(2);
                    attumen->SetAllowMelee(false);
                    attumen->SetAllowSpell(false);
                    attumen->Emote("Come Midnight, let's disperse this petty rabble!", Text_Yell, 9168);
                }
            }
            else if (GetPhase() == 2)
            {
                if (GetLinkedCreature() && GetLinkedCreature()->IsAlive())
                {
                    MoonScriptBossAI* attumen = static_cast< MoonScriptBossAI* >(GetLinkedCreature());
                    if (GetRange(attumen) <= 15)
                    {
                        attumen->Regenerate();
                        attumen->SetDisplayId(16040);
                        attumen->ClearHateList();
                        attumen->SetAllowMelee(true);
                        attumen->SetAllowSpell(true);
                        Despawn();
                        return;
                    }
                    else MoveTo(attumen);
                }
            }
            ParentClass::AIUpdate();
        }
};

/////////////////////////////////////////////////////////////////////
//Moroes
#define CN_MOROES        15687
#define MOROES_GOUGE    28456
#define MOROES_VANISH    29448
#define MOROES_BLIND    34654
#define MOROES_ENRAGE    37023
#define MOROES_GARROTE    37066

class MoroesAI : public MoonScriptBossAI
{
        MOONSCRIPT_FACTORY_FUNCTION(MoroesAI, MoonScriptBossAI);
        MoroesAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            //Initialize timers
            mVanishTimer = mGarroteTimer = INVALIDATE_TIMER;

            //Phase 1 spells
            AddPhaseSpell(1, AddSpell(MOROES_GOUGE, Target_Current, 20, 0, 10, 0, 5));
            AddPhaseSpell(1, AddSpell(MOROES_BLIND, Target_ClosestPlayerNotCurrent, 20, 0, 10, 0, 10, true));
            mVanish = AddSpell(MOROES_VANISH, Target_Self, 0, 12, 0);
            mVanish->AddEmote("Now, where was I? Oh yes...", Text_Yell, 9215);
            mVanish->AddEmote("You rang?", Text_Yell, 9316);
            mEnrage = AddSpell(MOROES_ENRAGE, Target_Self, 0, 0, 0);

            //Phase 2 spells
            mGarrote = AddSpell(MOROES_GARROTE, Target_RandomPlayer, 0, 0, 0);

            //Emotes
            AddEmote(Event_OnCombatStart, "Hm, unannounced visitors. Preparations must be made...", Text_Yell, 9211);
            AddEmote(Event_OnDied, "How terribly clumsy of me...", Text_Yell, 9213);
            AddEmote(Event_OnTargetDied, "One more for dinner this evening.", Text_Yell, 9214);
            AddEmote(Event_OnTargetDied, "Time... Never enough time.", Text_Yell, 9314);
            AddEmote(Event_OnTargetDied, "I've gone and made a mess.", Text_Yell, 9315);
        }

        void OnCombatStart(Unit* pTarget)
        {
            mEnrage->mEnabled = true;
            mVanishTimer = AddTimer(35000);    //First vanish after 35sec
            ParentClass::OnCombatStart(pTarget);
        }

        void OnDied(Unit* pKiller)
        {
            RemoveAuraOnPlayers(MOROES_GARROTE);
            ParentClass::OnDied(pKiller);
        }

        void AIUpdate()
        {
            if (GetPhase() == 1)
            {
                if (mEnrage->mEnabled && GetHealthPercent() <= 30 && !IsCasting())
                {
                    CastSpell(mEnrage);
                    mEnrage->mEnabled = false;
                }
                else if (IsTimerFinished(mVanishTimer) && !IsCasting())
                {
                    SetPhase(2, mVanish);
                    mGarroteTimer = AddTimer(12000);
                    ResetTimer(mVanishTimer, 35000);
                }
            }
            else if (GetPhase() == 2)
            {
                if (IsTimerFinished(mGarroteTimer) && !IsCasting())
                {
                    SetPhase(1, mGarrote);
                    RemoveAura(MOROES_VANISH);
                    RemoveTimer(mGarroteTimer);
                }
            }
            ParentClass::AIUpdate();
        }

        SpellDesc* mVanish;
        SpellDesc* mGarrote;
        SpellDesc* mEnrage;
        int32 mVanishTimer;
        int32 mGarroteTimer;
};

/////////////////////////////////////////////////////////////////////
//Maiden of Virtue
#define CN_MAIDENOFVIRTUE            16457
#define MAIDENOFVIRTUE_REPENTANCE    29511
#define MAIDENOFVIRTUE_HOLY_GROUND    29512
#define MAIDENOFVIRTUE_HOLY_FIRE    29522
#define MAIDENOFVIRTUE_HOLY_WRATH    32445

class MaidenOfVirtueAI : public MoonScriptBossAI
{
        MOONSCRIPT_FACTORY_FUNCTION(MaidenOfVirtueAI, MoonScriptBossAI);
        MaidenOfVirtueAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            //Spells
            AddSpell(MAIDENOFVIRTUE_HOLY_GROUND, Target_Self, 100, 0, 3);
            AddSpell(MAIDENOFVIRTUE_HOLY_FIRE, Target_RandomPlayer, 25, 1, 15, 0, 50);
            AddSpell(MAIDENOFVIRTUE_HOLY_WRATH, Target_RandomPlayer, 25, 0, 20, 0, 100);
            mRepentance = AddSpell(MAIDENOFVIRTUE_REPENTANCE, Target_Self, 25, 0.6f, 30);
            mRepentance->AddEmote("Cast out your corrupt thoughts.", Text_Yell, 9313);
            mRepentance->AddEmote("Your impurity must be cleansed.", Text_Yell, 9208);

            //Emotes
            AddEmote(Event_OnCombatStart, "Your behavior will not be tolerated.", Text_Yell, 9204);
            AddEmote(Event_OnTargetDied, "Ah ah ah...", Text_Yell, 9207);
            AddEmote(Event_OnTargetDied, "This is for the best.", Text_Yell, 9312);
            AddEmote(Event_OnTargetDied, "Impure thoughts lead to profane actions.", Text_Yell, 9311);
            AddEmote(Event_OnDied, "Death comes. Will your conscience be clear?", Text_Yell, 9206);
        }

        void OnCombatStart(Unit* pTarget)
        {
            mRepentance->TriggerCooldown();    //No repentance at the beginning of the fight
            ParentClass::OnCombatStart(pTarget);
        }

        SpellDesc* mRepentance;
};

/////////////////////////////////////////////////////////////////////
//The Big Bad Wolf
#define CN_BIGBADWOLF                    17521

#define TERRIFYING_HOWL                    30752
#define WIDE_SWIPE                        6749
// Combines display visual + buff
#define REDRIDINGHOOD_DEBUFF            30768
#define PBS_TAUNT                        30755    // Picnic Basket Smell (taunt)

class BigBadWolfAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(BigBadWolfAI);
        bool m_spellcheck[4];
        SP_AI_Spell spells[4];

        BigBadWolfAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
                m_spellcheck[i] = false;

            spells[0].info = dbcSpell.LookupEntry(TERRIFYING_HOWL);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].perctrigger = 50.0f;
            spells[0].cooldown = 30;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(WIDE_SWIPE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].perctrigger = 50.0f;
            spells[1].instant = true;
            spells[1].cooldown = 20;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(REDRIDINGHOOD_DEBUFF);
            spells[2].targettype = TARGET_RANDOM_SINGLE;
            spells[2].perctrigger = 50.0f;
            spells[2].instant = true;
            spells[2].cooldown = 45;
            spells[2].attackstoptimer = 1000;
            spells[2].mindist2cast = 0.0f;
            spells[2].maxdist2cast = 60.0f;
            spells[2].soundid = 9278;
            spells[2].speech = "Run away little girl, run away!";

            spells[3].info = dbcSpell.LookupEntry(PBS_TAUNT);
            spells[3].targettype = TARGET_RANDOM_SINGLE;
            spells[3].perctrigger = 0.0f;
            spells[3].instant = true;
            spells[3].cooldown = 0;
            spells[3].attackstoptimer = 1000;

            m_threattimer = 0;
            ThreatAdd = false;
            RTarget = NULL;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(1993);     // The better to own you with!

            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            ThreatAdd = false;
            m_threattimer = 20;

            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            ///\todo not in db
            _unit->PlaySoundToSet(9275);
            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Aarrhhh.");
            RemoveAIUpdateEvent();

            GameObject* DoorLeftO = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10917.1445f, -1774.05f, 90.478f, 184279);
            GameObject* DoorRightO = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10872.195f, -1779.42f, 90.45f, 184278);

            if (DoorLeftO)
                DoorLeftO->SetState(GO_STATE_OPEN);

            if (DoorRightO)
                DoorRightO->SetState(GO_STATE_OPEN);
        }

        void OnTargetDied(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(1994);     // Mmmm... delicious.
        }

        void AIUpdate()
        {
            if (ThreatAdd == true)
            {
                m_threattimer--;
            }
            else if (!m_threattimer)
            {
                _unit->GetAIInterface()->taunt(RTarget, false);
                ThreatAdd = false;
                m_threattimer = 19;
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
                            case TARGET_RANDOM_SINGLE:
                                {
                                    _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Red Riding Hood cast");
                                    std::vector<Player* > TargetTable;
                                    for (std::set< Object* >::iterator itr = _unit->GetInRangePlayerSetBegin();
                                            itr != _unit->GetInRangePlayerSetEnd(); ++itr)
                                    {
                                        Player* RandomTarget = NULL;
                                        RandomTarget = static_cast< Player* >(*itr);
                                        if (RandomTarget && RandomTarget->isAlive())
                                            TargetTable.push_back(RandomTarget);
                                        RandomTarget = NULL;
                                    }

                                    if (!TargetTable.size())
                                        return;

                                    auto random_index = RandomUInt(0, TargetTable.size() - 1);
                                    auto random_target = TargetTable[random_index];

                                    if (!RTarget)
                                        return;

                                    _unit->CastSpell(RTarget, spells[2].info, spells[2].instant);
                                    _unit->GetAIInterface()->taunt(RTarget, true);
                                    ThreatAdd = true;
                                    m_threattimer = 19;
                                }
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

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint8 nrspells;
        int m_threattimer;
        bool ThreatAdd;
        Unit* RTarget;
};


#define TERRIFYING_HOWL 30752
#define MORPH_LITTLE_RED_RIDING_HOOD 30768
#define DEBUFF_LITTLE_RED_RIDING_HOOD 30756

class THEBIGBADWOLFAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(THEBIGBADWOLFAI);
        bool m_spellcheck[3];
        SP_AI_Spell spells[3];

        THEBIGBADWOLFAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            //SpellEntry *infoImmunity;
            nrspells = 3;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(TERRIFYING_HOWL);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = true;
            spells[0].cooldown = 10;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(MORPH_LITTLE_RED_RIDING_HOOD);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].cooldown = 30;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(DEBUFF_LITTLE_RED_RIDING_HOOD);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = true;
            spells[2].cooldown = 30;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;
            spells[2].soundid = 9278;
            spells[2].speech = "Run away little girl, run away!";
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            _unit->SendScriptTextChatMessage(1993);     // The better to own you with!
            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            GameObject* DoorLeft = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10917.1445f, -1774.05f, 90.478f, 184279);
            GameObject* DoorRight = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10872.195f, -1779.42f, 90.45f, 184278);
            GameObject* Curtain = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10894.17f, -1774.218f, 90.477f, 183932);

            if (DoorLeft)
                DoorLeft->SetState(GO_STATE_CLOSED);

            if (DoorRight)
                DoorRight->SetState(GO_STATE_OPEN);

            if (Curtain)
                Curtain->SetState(GO_STATE_CLOSED);

            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            GameObject* DoorLeft = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10917.1445f, -1774.05f, 90.478f, 184279);
            GameObject* DoorRight = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10872.195f, -1779.42f, 90.45f, 184278);
            GameObject* Curtain = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10894.17f, -1774.218f, 90.477f, 183932);

            if (DoorLeft)
                DoorLeft->SetState(GO_STATE_OPEN);

            if (DoorRight)
                DoorRight->SetState(GO_STATE_OPEN);

            // Make sure the curtain stays up
            if (Curtain)
                Curtain->SetState(GO_STATE_OPEN);

            CastTime();
            _unit->PlaySoundToSet(9275);
            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "AArrhhh.");
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            _unit->PlaySoundToSet(9277);
            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Mmmm... delicious.");
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
        uint8 nrspells;
};


uint32 WayStartBBW[1000000];

#define SendQuickMenu(textid) objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), textid, Plr); \
    Menu->SendTo(Plr);

class BarnesGS : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* Plr)
        {
            GossipMenu* Menu;
            if (WayStartBBW[pObject->GetInstanceID()] == 5)
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 8975, Plr);         // Splendid! Marvelous! An excellent performance!    (when opera event is over)
            }
            else
            {
                objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 8970, Plr);         // Finally, everything is in place. Are you ready for your big stage debut?
                Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(432), 1);     // I'm not an actor.

                Menu->SendTo(Plr);

            }
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            switch (IntId)
            {
                case 0:
                    GossipHello(pObject, Plr);
                    break;

                case 1:
                    {
                        GossipMenu* Menu;
                        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 8971, Plr);         // Don't worry, you'll be fine. You look like a natural!
                        Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(433), 2);     // Ok, I'll give it a try, then.
                        Menu->SendTo(Plr);
                    }
                    break;
                case 2:
                    {
                        Creature* pCreature = static_cast<Creature*>(pObject);
                        pCreature->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Splendid. I'm going to get the audience ready. Break a leg!");
                        pCreature->CastSpell(pCreature, 32616, false);
                        pCreature->GetAIInterface()->StopMovement(0);
                        pCreature->GetAIInterface()->SetAIState(STATE_SCRIPTMOVE);
                        pCreature->GetAIInterface()->setMoveType(11);
                        pCreature->GetAIInterface()->setWaypointToMove(0);
                        pCreature->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
                        pCreature->PlaySoundToSet(9357);
                        WayStartBBW[pCreature->GetInstanceID()] = 2;
                    }
                    break;
            }
        }

};

class GrandMother : public GossipScript
{
    public:
        void GossipHello(Object* pObject, Player* Plr)
        {
            GossipMenu* Menu;
            objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 7245, Plr);             // Don't get too close, $N. I'm liable to fumble and bash your brains open with the face of my hammer.

            Menu->AddItem(GOSSIP_ICON_CHAT, Plr->GetSession()->LocalizedGossipOption(434), 1);         // What phat lewts you have Grandmother!

            Menu->SendTo(Plr);
        }

        void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* Code)
        {
            switch (IntId)
            {
                case 0:
                    GossipHello(pObject, Plr);
                    break;

                case 1:
                    {
                        static_cast<Creature*>(pObject)->Despawn(100, 0);
                        Creature* pop = pObject->GetMapMgr()->GetInterface()->SpawnCreature(17521, pObject->GetPositionX(), pObject->GetPositionY(),
                                        pObject->GetPositionZ(), 0, true, true, 0, 0);
                        pop->GetAIInterface()->AttackReaction(Plr, 1, 0);
                        break;
                    }
            }
        }

};

static Location Barnes[] =
{
    { },
    { -10873.91f, -1780.177f, 90.50f, 3.3f },
    { -10895.299805f, -1783.349976f, 90.50f, 4.5f },
    { -10873.91f, -1780.177f, 90.50f, 3.3f },
    { -10868.4f, -1781.63f, 90.50f, 1.24f }
};

class BarnesAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(BarnesAI);

        BarnesAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(1, 0, 0));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(2, 43000, 0));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(3, 0, 0));
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(4, 0, 0));

            _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_DONTMOVEWP);
            _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);

            WayStartBBW[_unit->GetInstanceID()] = 1;

            eventRand = 0;
            switch (RandomUInt(2))
            {
                case 0:
                    eventRand = 0;
                    break;
                case 1:
                    eventRand = 1;
                    break;
                case 2:
                    eventRand = 2;
                    break;
            }
        }

        void OnReachWP(uint32 iWaypointId, bool bForwards)
        {
            switch (iWaypointId)
            {
                case 0:
                    _unit->GetAIInterface()->setWaypointToMove(1);
                    WayStartBBW[_unit->GetInstanceID()] = 3;
                    break;
                case 1:
                    break;
                case 2:
                    cleanStage();
                    WayStartBBW[_unit->GetInstanceID()] = 4;
                    _unit->GetMapMgr()->GetInterface()->SpawnCreature(19525, Barnes[2].x, Barnes[2].y, Barnes[2].z, Barnes[2].o, true, false, 0, 0)->Despawn(43000, 0);
                    _unit->SetFacing(4.5f);
                    switch (eventRand)
                    {
                        case 0:
                            BarnesSpeakRed();
                            break;
                        case 1:
                            BarnesSpeakRJ();
                            break;
                        case 2:
                            BarnesSpeakWOZ();
                            break;
                    }
                    break;
                case 3:
                    switch (eventRand)
                    {
                        case 0:
                            EventRed();
                            break;
                        case 1:
                            EventRJ();
                            break;
                        case 2:
                            EventWOZ();
                            break;
                    }
                    _unit->SetUInt32Value(UNIT_FIELD_FLAGS, 1);
                    WayStartBBW[_unit->GetInstanceID()] = 5;
                    break;
            }
        }

        void cleanStage()
        {
            GameObject* DoorLeft = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10917.1445f, -1774.05f, 90.478f, 184279);
            GameObject* DoorRight = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10872.195f, -1779.42f, 90.45f, 184278);
            GameObject* Curtain = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10894.17f, -1774.218f, 90.477f, 183932);

            if (DoorLeft)
                DoorLeft->SetState(GO_STATE_CLOSED);

            if (DoorRight)
                DoorRight->SetState(GO_STATE_OPEN);

            if (Curtain)
                Curtain->SetState(GO_STATE_CLOSED);

            Creature* Julianne      = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10883.0f, -1751.81f, 90.4765f, 17534);
            Creature* Romulo        = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10883.0f, -1751.81f, 90.4765f, 17533);
            Creature* BigBadWolf    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10883.0f, -1751.81f, 90.4765f, 17521);
            Creature* Grandma       = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10883.0f, -1751.81f, 90.4765f, 17603);

            Creature* Dorothee      = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10897.650f, -1755.8311f, 90.476f, 17535); //Dorothee
            Creature* Strawman      = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10904.088f, -1754.8988f, 90.476f, 17543); //Strawman
            Creature* Roar          = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10891.115f, -1756.4898f, 90.476f, 17546);//Roar
            Creature* Tinman        = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10884.501f, -1757.3249f, 90.476f, 17547); //Tinman

            GameObject* House = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10883.0f, -1751.81f, 90.4765f, 183493);
            GameObject* Tree  = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10877.7f, -1763.18f, 90.4771f, 183492);
            GameObject* Tree2 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10906.7f, -1750.01f, 90.4765f, 183492);
            GameObject* Tree3 = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10909.5f, -1761.79f, 90.4773f, 183492);
            //GameObject* BackDrop = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10890.9f, -1744.06f, 90.4765f, 183491);

            if (Julianne)
                Julianne->Despawn(0, 0);
            if (Romulo)
                Romulo->Despawn(0, 0);

            if (BigBadWolf)
                BigBadWolf->Despawn(0, 0);
            if (Grandma)
                Grandma->Despawn(0, 0);

            if (Dorothee)
                Dorothee->Despawn(0, 0);
            if (Strawman)
                Strawman->Despawn(0, 0);
            if (Roar)
                Roar->Despawn(0, 0);
            if (Tinman)
                Tinman->Despawn(0, 0);

            if (House)
                House->Despawn(0, 0);
            if (Tree)
                Tree->Despawn(0, 0);
            if (Tree2)
                Tree2->Despawn(0, 0);
            if (Tree3)
                Tree3->Despawn(0, 0);
            //if (BackDrop)
            //    BackDrop->GetMapMgr()->GetInterface()->DeleteGameObject(BackDrop);
        }

        void BarnesSpeakWOZ()
        {
            // Start text
            _unit->SendScriptTextChatMessage(2011);     // Good evening, ladies and gentleman. Welcome to this evening's presentation!
            // Timed text 1
            _unit->SendTimedScriptTextChatMessage(2008, 7000);  // Tonight, we plumb the depths of the human soul as we join a lost, lonely girl trying desperately, with the help of her loyal companions, to find her way home.
            // Timed text 2
            _unit->SendTimedScriptTextChatMessage(2009, 23000); // But she is pursued by a wicked, malevolent crone!", 23000);
            // Timed text 3
            _unit->SendTimedScriptTextChatMessage(2010, 32000); // Will she survive? Will she prevail? Only time will tell. And now: On with the show!", 32000);
            // Applause
            sEventMgr.AddEvent(static_cast<Object*>(_unit), &Object::PlaySoundToSet, (uint32)9332, EVENT_UNK, 41000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }

        void EventWOZ()
        {
            GameObject* DoorLeft = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10917.1445f, -1774.05f, 90.478f, 184279);
            GameObject* DoorRight = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10872.195f, -1779.42f, 90.45f, 184278);
            GameObject* Curtain = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10894.17f, -1774.218f, 90.477f, 183932);

            if (DoorLeft)
                DoorLeft->SetState(GO_STATE_CLOSED);

            if (DoorRight)
                DoorRight->SetState(GO_STATE_CLOSED);

            if (Curtain)
                Curtain->SetState(GO_STATE_OPEN);

            _unit->SetDisplayId(16616);

            _unit->GetMapMgr()->GetInterface()->SpawnCreature(17535, -10897.650f, -1755.8311f, 90.476f, 4.61f, true, true, 0, 0); //Dorothee
            _unit->GetMapMgr()->GetInterface()->SpawnCreature(17543, -10904.088f, -1754.8988f, 90.476f, 4.61f, true, true, 0, 0); //Strawman
            _unit->GetMapMgr()->GetInterface()->SpawnCreature(17546, -10891.115f, -1756.4898f, 90.476f, 4.61f, true, true, 0, 0);//Roar
            _unit->GetMapMgr()->GetInterface()->SpawnCreature(17547, -10884.501f, -1757.3249f, 90.476f, 4.61f, true, true, 0, 0); //Tinman
        }

        void BarnesSpeakRJ()
        {
            // Start text
            _unit->SendScriptTextChatMessage(2011);                 // Good evening, ladies and gentleman. Welcome to this evening's presentation!
            // Timed text 1
            _unit->SendTimedScriptTextChatMessage(2016, 6000);      // Tonight we explore a tale of forbidden love!
            // Timed text 2
            _unit->SendTimedScriptTextChatMessage(2016, 19300);     // But beware, for not all love stories end happily. As you may find out, sometimes love pricks like a thorn.
            // Timed text 3
            _unit->SendTimedScriptTextChatMessage(2018, 32000);     // But don't take it from me. See for yourself what tragedy lies ahead when the paths of star-crossed lovers meet. And now: On with the show!
            // Applause
            sEventMgr.AddEvent(static_cast<Object*>(_unit), &Object::PlaySoundToSet, (uint32)9332, EVENT_UNK, 41000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }

        void EventRJ()
        {
            GameObject* DoorLeft = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10917.1445f, -1774.05f, 90.478f, 184279);
            GameObject* DoorRight = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10872.195f, -1779.42f, 90.45f, 184278);
            GameObject* Curtain = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10894.17f, -1774.218f, 90.477f, 183932);

            if (DoorLeft)
                DoorLeft->SetState(GO_STATE_CLOSED);

            if (DoorRight)
                DoorRight->SetState(GO_STATE_CLOSED);

            if (Curtain)
                Curtain->SetState(GO_STATE_OPEN);

            _unit->SetDisplayId(16616);
            _unit->GetMapMgr()->GetInterface()->SpawnCreature(17534, -10891.582f, -1755.5177f, 90.476f, 4.61f, true, true, 0, 0); //Spawn Julianne
        }

        void BarnesSpeakRed()
        {
            // Start text
            _unit->SendScriptTextChatMessage(2011);                 // Good evening, ladies and gentleman. Welcome to this evening's presentation!
            // Timed text 1
            _unit->SendTimedScriptTextChatMessage(2012, 7000);      // Tonight things are not what they seems for tonight your eyes may not be trusted.
            // Timed text 2
            _unit->SendTimedScriptTextChatMessage(2013, 17000);     // Take for instance this quiet elderly woman waiting for a visit from her granddaughter. Surely there is nothing to fear from this sweet gray-haired old lady.
            // Timed text 3
            _unit->SendTimedScriptTextChatMessage(2014, 32000);     // But don't let me pull the wool over your eyes. See for yourself what lies beneath those covers. And now: On with the show!
            // Applause
            sEventMgr.AddEvent(static_cast<Object*>(_unit), &Object::PlaySoundToSet, (uint32)9332, EVENT_UNK, 41000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }

        void EventRed()
        {
            GameObject* DoorLeft = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10917.1445f, -1774.05f, 90.478f, 184279);
            GameObject* DoorRight = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10872.195f, -1779.42f, 90.45f, 184278);
            GameObject* Curtain = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10894.17f, -1774.218f, 90.477f, 183932);
            /*GameObject* House = _unit->GetMapMgr()->GetInterface()->SpawnGameObject(183493, -10883.0f, -1751.81f, 90.4765f, -1.72788f, false, 0, 0);
            GameObject* Tree = _unit->GetMapMgr()->GetInterface()->SpawnGameObject(183492, -10877.7f, -1763.18f, 90.4771f, -1.6297f, false, 0, 0);
            GameObject* Tree2 = _unit->GetMapMgr()->GetInterface()->SpawnGameObject(183492, -10906.7f, -1750.01f, 90.4765f, -1.69297f, false, 0, 0);
            GameObject* Tree3 = _unit->GetMapMgr()->GetInterface()->SpawnGameObject(183492, -10909.5f, -1761.79f, 90.4773f, -1.65806f, false, 0, 0);
            GameObject* BackDrop = _unit->GetMapMgr()->GetInterface()->SpawnGameObject(183491, -10890.9f, -1744.06f, 90.4765f, -1.67552f, false, 0, 0);*/

            if (DoorLeft)
                DoorLeft->SetState(GO_STATE_CLOSED);

            if (DoorRight)
                DoorRight->SetState(GO_STATE_CLOSED);

            if (Curtain)
                Curtain->SetState(GO_STATE_OPEN);

            // Float Value does not work for go rotation any more, as of lastest patches
            /*// Back Right - House
            if (House)
            {
                House->SetFloatValue(GAMEOBJECT_ROTATION_02, 0.760406f);
                House->SetFloatValue(GAMEOBJECT_ROTATION_03, -0.649448f);
                House->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.0f);
            }

            // Front Right - Tree
            if (Tree)
            {
                Tree->SetFloatValue(GAMEOBJECT_ROTATION_02, 0.748956f);
                Tree->SetFloatValue(GAMEOBJECT_ROTATION_03, -0.66262f);
            }

            // Back Left - Tree 2
            if (Tree2)
            {
                Tree2->SetFloatValue(GAMEOBJECT_ROTATION_02, 0.648956f);
                Tree2->SetFloatValue(GAMEOBJECT_ROTATION_03, -0.66262f);
            }

            // Front Left - Tree 3
            if (Tree3)
            {
                Tree3->SetFloatValue(GAMEOBJECT_ROTATION_02, 0.737277f);
                Tree3->SetFloatValue(GAMEOBJECT_ROTATION_03, -0.67559f);
            }

            // Back - Back Drop
            if (BackDrop)
            {
                BackDrop->SetFloatValue(GAMEOBJECT_ROTATION_02, 0.743145f);
                BackDrop->SetFloatValue(GAMEOBJECT_ROTATION_03, -0.669131f);
            }*/

            _unit->SetDisplayId(16616);
            _unit->GetMapMgr()->GetInterface()->SpawnCreature(17603, -10891.582f, -1755.5177f, 90.476f, 4.61f, true, true, 0, 0);

        }

        inline WayPoint* CreateWaypoint(int id, uint32 waittime, uint32 flags)
        {
            WayPoint* wp = _unit->CreateWaypointStruct();
            wp->id = id;
            wp->x = Barnes[id].x;
            wp->y = Barnes[id].y;
            wp->z = Barnes[id].z;
            wp->o = Barnes[id].o;
            wp->waittime = waittime;
            wp->flags = flags;
            wp->forwardemoteoneshot = false;
            wp->forwardemoteid = 0;
            wp->backwardemoteoneshot = false;
            wp->backwardemoteid = 0;
            wp->forwardskinid = 0;
            wp->backwardskinid = 0;
            return wp;
        }

    protected:
        int eventRand;
};


class StageLight : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(StageLight);
        StageLight(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            _unit->GetAIInterface()->disable_melee = true;
            _unit->GetAIInterface()->m_canMove = false;
            _unit->m_noRespawn = true;
            _unit->CastSpell(_unit, 34126, true);
        }

};

// The Curator + Astral Flare
#define CN_CURATOR            15691
#define CN_ASTRALFLARE        17096

#define HATEFUL_BOLT        30383
#define EVOCATION            30254
#define C_ENRAGE            28747
#define BERSERK                26662

class CuratorAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CuratorAI);
        bool m_spellcheck[4];
        SP_AI_Spell spells[4];

        CuratorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(HATEFUL_BOLT);
            spells[0].instant = true;
            spells[0].cooldown = 12;

            spells[1].info = dbcSpell.LookupEntry(BERSERK);
            spells[1].instant = true;
            spells[1].cooldown = 720;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(EVOCATION);
            spells[2].instant = false;
            spells[2].cooldown = 0;
            spells[2].attackstoptimer = 19000;

            spells[3].info = dbcSpell.LookupEntry(C_ENRAGE);
            spells[3].instant = true;
            spells[3].cooldown = 0;
            spells[3].attackstoptimer = 1000;

            evocation = false;
            enrage = false;
            berserk = false;
            Timer = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(2062);     // The Menagerie is for guests only.

            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;

            evocation = false;
            enrage = false;
            uint32 t = (uint32)time(NULL);
            Timer = t + 10;

            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(2069);     // This Curator is no longer op... er... ation... al.
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)
            {
                switch (RandomUInt(1))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(2067);     // Do not touch the displays.
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(2068);     // You are not a guest.
                        break;
                }
            }
        }

        void AIUpdate()
        {
            if (!evocation)
            {
                if (_unit->GetManaPct() <= 10)
                {
                    _unit->Root();
                    _unit->setAttackTimer(spells[1].attackstoptimer, false);
                    _unit->SendScriptTextChatMessage(2065);     // Your request cannot be processed.
                    _unit->CastSpell(_unit, spells[2].info, spells[2].instant);
                    evocation = true;
                }
                else if (!enrage && _unit->GetHealthPct() <= 16)
                {
                    _unit->CastSpell(_unit, spells[3].info, spells[3].instant);
                    _unit->SendScriptTextChatMessage(2066);     // Failure to comply will result in offensive action.
                    enrage = true;
                }
                else
                    Trigger();
            }
            else if (_unit->GetManaPct() > 94)
            {
                _unit->Unroot();
                evocation = false;
            }
        }

        void Trigger()
        {
            uint32 t = (uint32)time(NULL);
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget() && t > spells[0].casttime)
            {
                Unit* target = _unit->GetAIInterface()->GetSecondHated();
                _unit->CastSpell(target, spells[0].info, spells[0].instant);
                target = NULL;
                spells[0].casttime = t + spells[0].cooldown;
            }
            else if (_unit->GetAIInterface()->getNextTarget() && !enrage && !evocation && t > Timer &&
                    _unit->GetMaxPower(POWER_TYPE_MANA) != 0)
            {
                AstralSpawn();
                Timer = t + 10;
            }
        }

        void AstralSpawn()
        {
            std::vector<Player*> Target_List;
            for (std::set< Object* >::iterator itr = _unit->GetInRangePlayerSetBegin();
                    itr != _unit->GetInRangePlayerSetEnd(); ++itr)
            {
                Player* RandomTarget = NULL;
                RandomTarget = static_cast< Player* >(*itr);
                if (RandomTarget && RandomTarget->isAlive() && isHostile(_unit, (*itr)))
                    Target_List.push_back(RandomTarget);
                RandomTarget = NULL;
            }
            if (!Target_List.size())
                return;

            auto random_index = RandomUInt(0, Target_List.size() - 1);
            Unit* random_target = Target_List[random_index];

            if (random_target == nullptr)
                return;

            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(2063);     // Gallery rules will be strictly enforced.
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(2064);     // This curator is equipped for gallery protection.
                    break;
            }

            _unit->SetUInt32Value(UNIT_FIELD_POWER1, _unit->GetPower(POWER_TYPE_MANA) - (_unit->GetMaxPower(POWER_TYPE_MANA) / 10));
            float dX = _unit->GetPositionX();
            float dY = _unit->GetPositionY();
            Creature* AstralFlare = NULL;
            switch (RandomUInt(3))
            {
                case 0:
                    {
                        AstralFlare = _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_ASTRALFLARE, dX + 3,
                                      dY + 3, _unit->GetPositionZ(), 0, true, false, 0, 0);
                        AstralFlare->GetAIInterface()->AttackReaction(random_target, 1, 0);
                        AstralFlare = NULL;
                    }
                    break;
                case 1:
                    {
                        AstralFlare = _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_ASTRALFLARE, dX + 3,
                                      dY - 3, _unit->GetPositionZ(), 0, true, false, 0, 0);
                        AstralFlare->GetAIInterface()->AttackReaction(random_target, 1, 0);
                        AstralFlare = NULL;
                    }
                    break;
                case 2:
                    {
                        AstralFlare = _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_ASTRALFLARE, dX - 3,
                                      dY - 3, _unit->GetPositionZ(), 0, true, false, 0, 0);
                        AstralFlare->GetAIInterface()->AttackReaction(random_target, 1, 0);
                        AstralFlare = NULL;
                    }
                    break;
                case 3:
                    {
                        AstralFlare = _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_ASTRALFLARE, dX - 3,
                                      dY + 3, _unit->GetPositionZ(), 0, true, false, 0, 0);
                        AstralFlare->GetAIInterface()->AttackReaction(random_target, 1, 0);
                        AstralFlare = NULL;
                    }
                    break;
            }
            Target_List.clear();
        }

    protected:
        bool evocation;
        bool enrage;
        bool berserk;
        uint8 nrspells;
        uint32 Timer;
};

// Astral Flare
#define ASTRAL_FLARE_PASSIVE    30234
#define ASTRAL_FLARE_VISUAL        30237
#define ARCING_SEAR                30235

class AstralFlareAI : public MoonScriptCreatureAI
{
        MOONSCRIPT_FACTORY_FUNCTION(AstralFlareAI, MoonScriptCreatureAI);
        AstralFlareAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            AddSpell(ASTRAL_FLARE_PASSIVE, Target_Self, 100, 0, 3);
            AddSpell(ASTRAL_FLARE_VISUAL, Target_Self, 100, 0, 6);
            AddSpell(30235, Target_Current, 20, 0, 0);
            SetDespawnWhenInactive(true);
            AggroNearestPlayer();
        }
};

// Shade of Aran
#define SHADEOFARAN            16524

#define FROSTBOLT            29954
#define FIREBALL            29953
#define ARCMISSLE            29955
#define CHAINSOFICE            29991
#define DRAGONSBREATH        29964
#define AOE_COUNTERSPELL    29961

#define BLIZZARD            29952
#define FLAME_WREATH        29946 // detonate -> 30004

#define BLINK_CENTER        29967
#define MAGNETIC_PULL        38540
#define MASSSLOW            30035
#define AEXPLOSION            29973

#define MASS_POLYMORPH        29963
#define CONJURE                29975
#define DRINK                30024
#define AOE_PYROBLAST        29978

#define SUMMON_ELEMENTAL_1    37051
#define SUMMON_ELEMENTAL_2    37052
#define SUMMON_ELEMENTAL_3    37053
#define WATERELE            17167
#define WATERBOLT            31012

#define SHADOWOFARAN        18254
#define SHADOWPYRO            29978

enum SUPERSPELL
{
    SUPER_FLAME = 0,
    SUPER_BLIZZARD = 1,
    SUPER_AOE = 2,
};

class ShadeofAranAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(ShadeofAranAI);
        bool m_spellcheck[6];
        SP_AI_Spell spells[6];

        ShadeofAranAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 6;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(FROSTBOLT);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = false;
            spells[0].cooldown = 5;
            spells[0].casttime = 5;
            spells[0].attackstoptimer = 2000;

            spells[1].info = dbcSpell.LookupEntry(FIREBALL);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = false;
            spells[1].cooldown = 5;
            spells[1].casttime = 5;
            spells[1].attackstoptimer = 2000;

            spells[2].info = dbcSpell.LookupEntry(ARCMISSLE);
            spells[2].targettype = TARGET_RANDOM_SINGLE;
            spells[2].instant = false;
            spells[2].cooldown = 10;
            spells[2].casttime = 10;
            spells[2].attackstoptimer = 6000;

            spells[3].info = dbcSpell.LookupEntry(CHAINSOFICE);
            spells[3].targettype = TARGET_RANDOM_SINGLE;
            spells[3].instant = true;
            spells[3].cooldown = RandomUInt(5) + 14;
            spells[3].casttime = RandomUInt(5) + 14;
            spells[3].attackstoptimer = 1000;

            spells[4].info = dbcSpell.LookupEntry(DRAGONSBREATH);
            spells[4].targettype = TARGET_RANDOM_SINGLE;
            spells[4].instant = true;
            spells[4].cooldown = RandomUInt(5) + 16;
            spells[4].casttime = RandomUInt(5) + 16;
            spells[4].attackstoptimer = 1000;
            spells[4].maxdist2cast = 15;

            spells[5].info = dbcSpell.LookupEntry(AOE_COUNTERSPELL);
            spells[5].targettype = TARGET_SELF;
            spells[5].instant = true;
            spells[5].cooldown = 13;
            spells[5].casttime = 13;
            spells[5].attackstoptimer = 1000;

            info_flame_wreath = dbcSpell.LookupEntry(FLAME_WREATH);
            info_blink_center = dbcSpell.LookupEntry(BLINK_CENTER);
            info_massslow = dbcSpell.LookupEntry(MASSSLOW);
            info_magnetic_pull = dbcSpell.LookupEntry(MAGNETIC_PULL);
            info_aexplosion = dbcSpell.LookupEntry(AEXPLOSION);
            info_blizzard = dbcSpell.LookupEntry(BLIZZARD);
            info_summon_elemental_1 = dbcSpell.LookupEntry(SUMMON_ELEMENTAL_1);
            info_summon_elemental_2 = dbcSpell.LookupEntry(SUMMON_ELEMENTAL_2);
            info_summon_elemental_3 = dbcSpell.LookupEntry(SUMMON_ELEMENTAL_3);
            info_mass_polymorph = dbcSpell.LookupEntry(MASS_POLYMORPH);
            info_conjure = dbcSpell.LookupEntry(CONJURE);
            info_drink = dbcSpell.LookupEntry(DRINK);
            info_pyroblast = dbcSpell.LookupEntry(AOE_PYROBLAST);

            drinking = false;
            enraged = false;
            summoned = false;
            explode = false;
            slow = false;
            LastSuperSpell = 0;
            m_time_enrage = 0;
            m_time_special = 0;
            m_time_pyroblast = 0;
            m_time_conjure = 0;
            FlameWreathTimer = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            //Atiesh check
            bool HasAtiesh = false;
            if (mTarget->IsPlayer())
            {
                for (std::set< Object* >::iterator itr = _unit->GetInRangePlayerSetBegin(); itr != _unit->GetInRangePlayerSetEnd(); ++itr)
                {
                    if (*itr)
                    {
                        Player* plr = static_cast< Player* >(*itr);
                        if (plr->GetItemInterface()->GetItemCount(22589) > 0 ||
                                plr->GetItemInterface()->GetItemCount(22630) > 0 ||
                                plr->GetItemInterface()->GetItemCount(22631) > 0 ||
                                plr->GetItemInterface()->GetItemCount(22632) > 0)
                        {
                            HasAtiesh = true;
                            break;
                        }
                    }
                }
            }

            if (HasAtiesh)
            {
                _unit->SendScriptTextChatMessage(2046);     // Where did you get that?! Did HE send you?!
            }
            else
            {
                switch (RandomUInt(2))
                {
                    case 0:
                        _unit->SendScriptTextChatMessage(2031);     // Please, no more. My son... he's gone mad!
                        break;
                    case 1:
                        _unit->SendScriptTextChatMessage(2032);     // I'll not be tortured again!
                        break;
                    case 2:
                        _unit->SendScriptTextChatMessage(2033);     // Who are you? What do you want? Stay away from me!
                        break;
                }
            }

            _unit->GetAIInterface()->setCurrentAgent(AGENT_SPELL);
            RegisterAIUpdateEvent(1000);
            m_time_enrage = 900;
            m_time_special = (uint32) RandomUInt(5) + 25;
            m_time_pyroblast = 0;
            CastTime();
            drinking = false;
            enraged = false;
            summoned = false;
            explode = false;
            slow = false;
            LastSuperSpell = RandomUInt(100) % 3;
            // Door closing
            GameObject* SDoor = NULL;
            SDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-11190.012f, -1881.016f, 231.95f, 184517);
            if (SDoor)
            {
                SDoor->SetState(GO_STATE_CLOSED);
                SDoor->SetFlags(33);
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
            _unit->SetUInt32Value(UNIT_FIELD_POWER1, _unit->GetMaxPower(POWER_TYPE_MANA));
            // Door opening
            GameObject* SDoor = NULL;
            SDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-11190.012f, -1881.016f, 231.95f, 184517);
            if (SDoor)
                SDoor->SetFlags(34);
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
            _unit->SendScriptTextChatMessage(2045);     // At last... The nightmare is.. over...

            RemoveAIUpdateEvent();
            // Door opening
            GameObject* SDoor = NULL;
            SDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-11190.012f, -1881.016f, 231.95f, 184517);
            if (SDoor)
                SDoor->SetFlags(34);
        }

        void OnTargetDied(Unit* mTarget)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(2042);     // I want this nightmare to be over!
                    break;

                case 1:
                    _unit->SendScriptTextChatMessage(2043);     // Torment me no more!
                    break;
            }
        }

        void AIUpdate()
        {
            if (FlameWreathTimer)
            {
                FlameWreathTimer--;
                for (uint8 i = 0; i < 3; i++)
                {
                    if (!FlameWreathTarget[i])
                        continue;

                    Unit* pTarget = _unit->GetMapMgr()->GetUnit(FlameWreathTarget[i]);
                    if (pTarget && pTarget->GetDistanceSq(FWTargPosX[i], FWTargPosY[i], _unit->GetPositionZ()) > 3)
                    {
                        pTarget->CastSpell(pTarget, 20476, true);
                        FlameWreathTarget[i] = 0;
                    }
                }
            }

            if (!drinking)
            {
                if (explode)
                {
                    if (slow)
                    {
                        _unit->CastSpell(_unit, info_massslow, true);
                        slow = false;
                    }
                    else
                    {
                        _unit->CastSpell(_unit, info_aexplosion, false);
                        explode = false;
                    }
                }
                else if (!summoned && _unit->GetHealthPct() <= 40)
                {
                    _unit->CastSpell(_unit, info_summon_elemental_1, true);
                    _unit->CastSpell(_unit, info_summon_elemental_2, true);
                    _unit->CastSpell(_unit, info_summon_elemental_3, true);

                    _unit->SendScriptTextChatMessage(2041);     // I'm not finished yet! No, I have a few more tricks up me sleeve.
                    summoned = true;
                }
                else if (_unit->GetManaPct() <= 20 && _unit->GetCurrentSpell() == NULL)
                {
                    if (!m_time_pyroblast)
                    {
                        _unit->GetAIInterface()->WipeHateList();
                        _unit->SendScriptTextChatMessage(2040);     // Surely you would not deny an old man a replenishing drink? No, no I thought not.
                        m_time_pyroblast = 10;
                        _unit->CastSpell(_unit, info_mass_polymorph, true);
                        _unit->setAttackTimer(2000, false);
                    }
                    else if (_unit->GetStandState() != STANDSTATE_SIT)
                    {
                        _unit->setAttackTimer(3000, false);
                        _unit->CastSpell(_unit, info_conjure, false);
                        _unit->SetStandState(1);
                        _unit->setEmoteState(EMOTE_ONESHOT_EAT);
                    }
                    else
                        _unit->CastSpell(_unit, info_drink, true);
                }
                else
                    SpellTrigger();
            }
            else if (_unit->GetCurrentSpell() == NULL)
            {
                m_time_pyroblast--;
                if (!m_time_pyroblast)
                {
                    _unit->SetStandState(0);
                    _unit->setEmoteState(0);
                    _unit->CastSpell(_unit, info_pyroblast, false);
                    drinking = false;
                }

            }
        }

        void SpellTrigger()
        {
            m_time_enrage--;
            m_time_special--;
            if (!enraged && !m_time_enrage)
            {
                _unit->SendScriptTextChatMessage(2044);     // You've wasted enough of my time. Let these games be finished!

                float ERX = 5 * cos(RandomFloat(6.28f)) + (_unit->GetPositionX());
                float ERY = 5 * sin(RandomFloat(6.28f)) + (_unit->GetPositionY());
                float ERZ = _unit->GetPositionZ();

                for (uint8 i = 0; i < 4; i++)
                {
                    _unit->GetMapMgr()->GetInterface()->SpawnCreature(SHADOWOFARAN, ERX, ERY, ERZ, 0, true, false, 0, 0);
                }
                ERX = 0;
                ERY = 0;
                ERZ = 0;
                enraged = true;
                return;
            }
            else if (!m_time_special)
            {
                CastSpecial();
                m_time_special = (uint32) RandomUInt(5) + 25;
                return;
            }
            else
            {
                float val = RandomFloat(100.0f);
                SpellCast(val);
            }
        }

        void FlameWreath()
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(2034);     // I'll show you this beaten dog still has some teeth!
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(2035);     // Burn you hellish fiends!
                    break;
            }

            FlameWreathTimer = 20;
            FlameWreathTarget[0] = 0;
            FlameWreathTarget[1] = 0;
            FlameWreathTarget[2] = 0;

            std::vector<Player*> Targets;
            std::set< Object* >::iterator hostileItr = _unit->GetInRangePlayerSetBegin();
            for (; hostileItr != _unit->GetInRangePlayerSetEnd(); ++hostileItr)
            {
                Player* RandomTarget = NULL;
                RandomTarget = static_cast< Player* >(*hostileItr);

                if (RandomTarget && RandomTarget->isAlive() && _unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0)
                    Targets.push_back(RandomTarget);
            }

            while(Targets.size() > 3)
                Targets.erase(Targets.begin() + RandomUInt(Targets.size()));

            uint32 i = 0;
            for (std::vector<Player*>::iterator itr = Targets.begin(); itr != Targets.end(); ++itr)
            {
                if (*itr)
                {
                    FlameWreathTarget[i] = (*itr)->GetGUID();
                    FWTargPosX[i] = (*itr)->GetPositionX();
                    FWTargPosY[i] = (*itr)->GetPositionY();
                    _unit->CastSpell((*itr), FLAME_WREATH, true);
                }
            }

        }

        void Blizzard()
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(2036);     // I'll freeze you all!
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(2037);     // Back to the cold dark with you!
                    break;
            }

            _unit->CastSpell(_unit, info_blizzard, true);
        }

        void AoEExplosion()
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(2038);     // Yes, yes, my son is quite powerful... but I have powers of my own!
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(2039);     // I am not some simple jester! I am Nielas Aran!
                    break;
            }

            _unit->CastSpell(_unit, info_blink_center, true);
            _unit->CastSpell(_unit, info_magnetic_pull, true);
            explode = true;
            slow = true;
        }

        void CastSpecial()
        {
            int Available[2];

            switch (LastSuperSpell)
            {
                case SUPER_BLIZZARD:
                    Available[0] = SUPER_FLAME;
                    Available[1] = SUPER_AOE;
                    break;

                case SUPER_FLAME:
                    Available[0] = SUPER_BLIZZARD;
                    Available[1] = SUPER_AOE;
                    break;

                case SUPER_AOE:
                    Available[0] = SUPER_BLIZZARD;
                    Available[1] = SUPER_FLAME;
                    break;

                default:
                    return;
            }

            LastSuperSpell = Available[RandomUInt(1)];

            switch (LastSuperSpell)
            {
                case SUPER_BLIZZARD:
                    Blizzard();
                    break;

                case SUPER_FLAME:
                    FlameWreath();
                    break;

                case SUPER_AOE:
                    AoEExplosion();
                    break;
            }
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
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

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Player* > TargetTable;
                for (std::set< Object* >::iterator itr = _unit->GetInRangePlayerSetBegin(); itr != _unit->GetInRangePlayerSetEnd(); ++itr)
                {
                    Player* RandomTarget = NULL;
                    RandomTarget = static_cast< Player* >(*itr);

                    if ((RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast) || (_unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(_unit, RandomTarget)))
                        TargetTable.push_back(RandomTarget);
                }

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, TargetTable.size() - 1);
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPositionX(), random_target->GetPositionY(), random_target->GetPositionZ(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:
        bool drinking;
        bool enraged;
        bool summoned;
        bool explode;
        bool slow;
        float FWTargPosX[3];
        float FWTargPosY[3];
        uint8 nrspells;
        int LastSuperSpell;
        uint32 m_time_enrage;
        uint32 m_time_special;
        uint32 m_time_pyroblast;
        uint32 m_time_conjure;
        uint32 FlameWreathTimer;
        uint64 FlameWreathTarget[3];
        SpellEntry* info_flame_wreath;
        SpellEntry* info_aexplosion;
        SpellEntry* info_blizzard;
        SpellEntry* info_magnetic_pull;
        SpellEntry* info_blink_center;
        SpellEntry* info_massslow;
        SpellEntry* info_mass_polymorph;
        SpellEntry* info_conjure;
        SpellEntry* info_drink;
        SpellEntry* info_pyroblast;
        SpellEntry* info_summon_elemental_1;
        SpellEntry* info_summon_elemental_2;
        SpellEntry* info_summon_elemental_3;
};

class WaterEleAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(WaterEleAI);

        WaterEleAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            WaterBolt = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            WaterBolt = (RandomUInt(3) + 5);
            RegisterAIUpdateEvent(1250);
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
            _unit->Despawn(20000, 0);
        }

        void AIUpdate()
        {
            WaterBolt--;
            if (!WaterBolt)
            {
                _unit->setAttackTimer(2000, false);
                Unit* target = _unit->GetAIInterface()->getNextTarget();
                if (target)
                    _unit->CastSpell(target, WATERBOLT, true);
            }
        }

    protected:
        int WaterBolt;
};

class ShadowofAranAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(ShadowofAranAI);

        ShadowofAranAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            ShadowPyro = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            ShadowPyro = (RandomUInt(2) + 4);
            RegisterAIUpdateEvent(1250);
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            _unit->Despawn(10000, 0);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
            _unit->Despawn(5000, 0);
        }

        void AIUpdate()
        {
            ShadowPyro--;
            if (!ShadowPyro)
            {
                Unit* target = _unit->GetAIInterface()->getNextTarget();
                if (target != NULL)
                    _unit->CastSpell(target, SHADOWPYRO, true);
            }
        }

    protected:
        int ShadowPyro;
};

// Terestian Illhoof
#define CN_ILLHOOF                    15688

#define SHADOW_BOLT                    19729
// #define S_DEMONCHAINS                30120
#define S_KILREK                        30066
// #define F_PORTAL1                    30179
// #define F_PORTAL2                    30171
#define I_ENRAGE                    32964
#define SACRIFICE                    30115

// Kil'Rek
#define CN_KILREK                    17229
#define AMPLIFY_FLAMES                30053
#define BROKEN_PACT                    30065

// Imps
#define CN_FIENDISH_IMP                17267
#define FIREBALL_IMP                31620

// Demon Chains
#define    CN_DEMONCHAINS                17248
#define    CHAINS_VISUAL                30206

#define CN_FPORTAL                    17265

class IllhoofAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(IllhoofAI);
        bool m_spellcheck[2];
        SP_AI_Spell spells[2];

        IllhoofAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
                m_spellcheck[i] = false;

            spells[0].info = dbcSpell.LookupEntry(SHADOW_BOLT);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 5;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(I_ENRAGE);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].cooldown = 600;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;
            /*
                    spells[2].info = dbcSpell.LookupEntry(S_DEMONCHAINS);
                    spells[2].targettype = TARGET_VARIOUS;
                    spells[2].instant = true;
                    spells[2].cooldown = 45;
                    spells[2].perctrigger = 0.0f;
                    spells[2].attackstoptimer = 1000;
            */

            ReSummon = false;
            ImpTimer = 0;
            ReKilrek = 0;
            DemonChain = 0;

        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(2050);     // Ah, you're just in time. The rituals are about to begin.

            uint32 t = (uint32)time(NULL);
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;

            DemonChain = t + 45;
            ImpTimer = t + 10;
            ReSummon = false;

            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            clean();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            clean();
            _unit->SendScriptTextChatMessage(2049);     // My life, is yours. Oh great one.
            RemoveAIUpdateEvent();
        }

        void clean()
        {
            Creature* portal = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-11249.51f, -1702.182f, 179.237f, CN_FPORTAL);
            Creature* portal2 = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-11239.534f, -1715.338f, 179.237f, CN_FPORTAL);

            if (portal != NULL)
                portal->Despawn(0, 0);
            if (portal2 != NULL)
                portal2->Despawn(0, 0);
        }

        void OnTargetDied(Unit* mTarget)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(2047);     // Your blood will anoint my circle.
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(2048);     // The great one will be pleased.
                    break;
            }
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);

            uint32 t = (uint32)time(NULL);
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                if (t > ImpTimer)
                {
                    spawnSummoningPortals();
                    ImpTimer = -1;
                }
                if (t > DemonChain)
                {
                    DemonChain = t + 45;
                    PlrSacrifice();
                }
                if (_unit->HasAura(BROKEN_PACT) && !ReSummon)
                {
                    ReSummon = true;
                    ReKilrek = t + 45;
                }
                else if (ReSummon && t > ReKilrek)
                {
                    ReSummon = false;
                    _unit->CastSpell(_unit, dbcSpell.LookupEntry(S_KILREK), true);
                    _unit->RemoveAura(BROKEN_PACT);
                }
            }
        }

        void spawnSummoningPortals()
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(2053);     // Come, you dwellers in the dark. Rally to my call!
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(2054);     // Gather, my pets. There is plenty for all.
                    break;
            }

            _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_FPORTAL, -11249.51f, -1702.182f, 179.237f, 0, true, false, 0, 0);
            _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_FPORTAL, -11239.534f, -1715.338f, 179.237f, 0, true, false, 0, 0);
        }

        void PlrSacrifice()
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(2051);     // Please, accept this humble offering, oh great one.
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(2052);     // Let the sacrifice serve his testament to my fealty.
                    break;
            }

            std::vector<Player* > TargetTable;
            std::set< Object* >::iterator itr = _unit->GetInRangePlayerSetBegin();

            for (; itr != _unit->GetInRangePlayerSetEnd(); ++itr)
            {
                if (isHostile(_unit, (*itr)))
                {
                    Player* RandomTarget = NULL;
                    RandomTarget = static_cast< Player* >(*itr);
                    if (RandomTarget && RandomTarget->isAlive() && isHostile(_unit, RandomTarget))
                        TargetTable.push_back(RandomTarget);
                }
            }
            if (!TargetTable.size())
                return;

            auto random_index = RandomUInt(0, TargetTable.size() - 1);
            auto random_target = TargetTable[random_index];

            if (random_target == nullptr)
                return;

            _unit->CastSpell(random_target, dbcSpell.LookupEntry(SACRIFICE), false);

            TargetTable.clear();

            float dcX = -11234.7f;
            float dcY = -1698.73f;
            float dcZ = 179.24f;
            _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_DEMONCHAINS, dcX, dcY, dcZ, 0, true, false, 0, 0);
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                        }

                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:

        uint8 nrspells;
        bool ReSummon;
        uint32 ImpTimer;
        uint32 ReKilrek;
        uint32 DemonChain;
};

// Kil'Rek
class KilrekAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(KilrekAI);
        bool m_spellcheck[2];
        SP_AI_Spell spells[2];

        KilrekAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(AMPLIFY_FLAMES);
            spells[0].targettype = TARGET_RANDOM_SINGLE;
            spells[0].instant = true;
            spells[0].cooldown = 5;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(BROKEN_PACT);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
        }

        void OnCombatStart(Unit* mTarget)
        {
            spells[0].casttime = (uint32)time(NULL) + spells[0].cooldown;

            RegisterAIUpdateEvent(1000);
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

            Unit* Illhoof = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), 15688);
            if (Illhoof != NULL && Illhoof->isAlive())
                Illhoof->CastSpell(Illhoof, spells[1].info, spells[1].instant);
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
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
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
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

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit* > TargetTable;
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if ((*itr) != _unit && isHostile(_unit, (*itr)) && (*itr)->IsUnit())
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && _unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0)
                            TargetTable.push_back(RandomTarget);
                    }
                }

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, TargetTable.size() - 1);
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPositionX(), random_target->GetPositionY(), random_target->GetPositionZ(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:
        uint8 nrspells;
};

// Fiendish Imp
class FiendishImpAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(FiendishImpAI);
        SP_AI_Spell spells[1];
        bool m_spellcheck[1];

        FiendishImpAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(FIREBALL_IMP);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].cooldown = 0;
            spells[0].perctrigger = 100.0f;
            spells[0].attackstoptimer = 1000;
            spells[0].casttime = 0;

            Unit* target = NULL;
            target = FindTargetForSpell();
            if (target != NULL)
            {
                _unit->GetAIInterface()->AttackReaction(target, 2500, 0);
            }
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SetUInt64Value(UNIT_FIELD_FLAGS, 0);
            _unit->GetAIInterface()->SetAllowedToEnterCombat(true);

            if (_unit->GetDistance2dSq(mTarget) <= 1225.0f)
            {
                _unit->GetAIInterface()->setCurrentAgent(AGENT_SPELL);
            }

            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();

            _unit->Despawn(1, 0);
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
            _unit->Despawn(1, 0);
        }

        void AIUpdate()
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            if (_unit->GetAIInterface()->getNextTarget() && _unit->GetDistance2dSq(_unit->GetAIInterface()->getNextTarget()) <= 1225.0f)
            {
                _unit->GetAIInterface()->setCurrentAgent(AGENT_SPELL);
                if (_unit->GetCurrentSpell() == NULL && RandomUInt(10) > 2)
                {
                    _unit->setAttackTimer(spells[0].attackstoptimer, false);

                    Unit* target = NULL;
                    target = _unit->GetAIInterface()->getNextTarget();

                    _unit->CastSpell(target, spells[0].info, spells[0].instant);
                    return;
                }
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
                    if (!spells[i].perctrigger) continue;

                    if (m_spellcheck[i])
                    {
                        target = _unit->GetAIInterface()->getNextTarget();
                        if (!spells[i].instant)
                            _unit->GetAIInterface()->StopMovement(1);

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
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

        Unit* FindTargetForSpell()
        {
            Unit* target = NULL;
            float distance = 90.0f;

            Unit* pUnit;
            float dist;

            for (std::set<Object*>::iterator itr = _unit->GetInRangeOppFactsSetBegin(); itr != _unit->GetInRangeOppFactsSetEnd(); ++itr)
            {
                if (!(*itr)->IsUnit())
                    continue;

                pUnit = static_cast<Unit*>((*itr));

                if (pUnit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH))
                    continue;

//            if (pUnit->m_auracount[SPELL_AURA_MOD_INVISIBILITY])
//                continue;

                if (!pUnit->isAlive() || _unit == pUnit)
                    continue;

                dist = _unit->GetDistance2dSq(pUnit);

                if (dist > distance * distance)
                    continue;

                target = pUnit;
                break;
            }

            return target;
        }

    protected:

        uint8 nrspells;
};

class DemonChains : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(DemonChains);
        DemonChains(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->CastSpell(_unit, dbcSpell.LookupEntry(CHAINS_VISUAL), true);
            _unit->Root();
            _unit->DisableAI();
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            if (_unit->GetHealthPct() > 0)
                _unit->Despawn(10000, 0);
        }

        void OnDied(Unit* mKiller)
        {
            Unit* uIllhoof = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(),
                             _unit->GetPositionZ(), CN_ILLHOOF);
            if (uIllhoof != NULL && uIllhoof->isAlive())
                uIllhoof->RemoveAura(SACRIFICE);

            _unit->Despawn(10000, 0);
        }
};

class FiendPortal : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(FiendPortal);
        FiendPortal(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->Root();

            _unit->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
            _unit->GetAIInterface()->disable_melee = true;
            _unit->GetAIInterface()->m_canMove = false;
            _unit->m_noRespawn = true;

            RegisterAIUpdateEvent(10000);
        }

        void AIUpdate()
        {
            _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_FIENDISH_IMP, _unit->GetPositionX(), _unit->GetPositionY(),
                    _unit->GetPositionZ(), 0, true, false, 0, 0);

        }

};

// Prince Malchezaar
#define CN_MALCHEZAAR            15690
#define CN_INFERNAL                17646
#define CN_AXES                    17650
#define CN_DUMMY                17644

#define ENFEEBLE                30843
#define SHADOWNOVA                30852
#define SW_PAIN                    30854
#define THRASH_AURA                12787
#define SUNDER_ARMOR            25225
#define AMPLIFY_DMG                39095 // old 12738
#define SUMMON_AXES                30891
#define WIELD_AXES                30857

// Extra creature info
#define INF_RAIN                33814
#define HELLFIRE                39131
#define DEMONIC_FRENZY            32851

// Item model info
#define AXE_ITEM_MODEL            40066
#define AXE_ITEM_INFO            33488898
#define AXE_ITEM_SLOT            768
/* Emotes:
SPECIAL? - 9223 - 9320
AXETOSS2? - 9317
*/

class MalchezaarAI : public MoonScriptCreatureAI
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(MalchezaarAI);
        bool m_spellcheck[9];
        SP_AI_Spell spells[9];

        MalchezaarAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            m_phase = 1;
            nrspells = 5;

            memset(Enfeeble_Targets, 0, sizeof(Enfeeble_Targets));
            memset(Enfeeble_Health, 0, sizeof(Enfeeble_Health));

            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SW_PAIN);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 15;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(ENFEEBLE);
            spells[1].targettype = TARGET_VARIOUS;
            spells[1].instant = true;
            spells[1].cooldown = 25;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(INF_RAIN);
            spells[2].targettype = TARGET_DESTINATION;
            spells[2].instant = true;
            spells[2].cooldown = 43;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SUNDER_ARMOR);
            spells[3].targettype = TARGET_ATTACKING;
            spells[3].instant = true;
            spells[3].cooldown = 15;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = dbcSpell.LookupEntry(AMPLIFY_DMG);
            spells[4].targettype = TARGET_RANDOM_SINGLE;
            spells[4].instant = true;
            spells[4].cooldown = 20;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;
            spells[4].mindist2cast = 0.0f;

            spells[5].info = dbcSpell.LookupEntry(SHADOWNOVA);
            spells[5].targettype = TARGET_VARIOUS;
            spells[5].instant = false;
            spells[5].cooldown = 4;
            spells[5].perctrigger = 0.0f;
            spells[5].attackstoptimer = 2000;

            spells[6].info = dbcSpell.LookupEntry(THRASH_AURA);
            spells[6].targettype = TARGET_SELF;
            spells[6].instant = true;
            spells[6].cooldown = 1;
            spells[6].perctrigger = 0.0f;
            spells[6].attackstoptimer = 1000;

            spells[7].info = dbcSpell.LookupEntry(WIELD_AXES);
            spells[7].targettype = TARGET_SELF;
            spells[7].instant = true;
            spells[7].attackstoptimer = 1000;

            spells[8].info = dbcSpell.LookupEntry(SUMMON_AXES);
            spells[8].targettype = TARGET_SELF;
            spells[8].instant = true;
            spells[8].attackstoptimer = 1000;

            // Dummy initialization
            float dumX = -10938.56f;
            float dumY = -2041.26f;
            float dumZ = 305.132f;
            CreatureAIScript* infernalDummy = SpawnCreature(CN_DUMMY, dumX, dumY, dumZ);
            if (infernalDummy != NULL)
            {
                SetLinkedCreature(infernalDummy);
                infernalDummy->SetLinkedCreature(this);
            }

            ranX = 0;
            ranY = 0;
            m_infernal = false;
            m_enfeebleoff = -1;
            m_spawn_infernal = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(2019);     // Madness has brought you here to me. I shall be your undoing.

            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = 0;

            uint32 t = (uint32)time(NULL);
            spells[0].casttime = t + spells[0].cooldown;
            spells[1].casttime = t + spells[1].cooldown;
            spells[2].casttime = t + spells[2].cooldown;

            m_spawn_infernal = 0;
            m_infernal = false;

            RegisterAIUpdateEvent(1000);

            GameObject* MDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-11018.5f, -1967.92f, 276.652f, 185134);
            if (MDoor != NULL)
            {
                MDoor->SetState(GO_STATE_CLOSED);
                MDoor->SetFlags(33);
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();

            // Reset weapon
            _unit->SetEquippedItem(MELEE, 0);

            // Off hand weapon
            _unit->SetEquippedItem(OFFHAND, 0);

            CreatureProto* cp = CreatureProtoStorage.LookupEntry(CN_MALCHEZAAR);
            if (!cp)
                return;

            _unit->SetMinDamage(cp->MinDamage);
            _unit->SetMaxDamage(cp->MaxDamage);

            for (uint8 i = 0; i < 5; ++i)
                Enfeeble_Targets[i] = 0;

            if (GetLinkedCreature() != NULL)
                GetLinkedCreature()->GetUnit()->Despawn(10000, 0);

            GameObject* MDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-11018.5f, -1967.92f, 276.652f, 185134);
            // Open door
            if (MDoor != NULL)
                MDoor->SetState(GO_STATE_OPEN);

            Creature* MAxes = NULL;
            MAxes = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), CN_AXES);
            if (MAxes != NULL)
                MAxes->Despawn(1000, 0);
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(2030);     // I refuse to concede defeat. I am a prince of the Eredar! I am...

            RemoveAIUpdateEvent();

            if (GetLinkedCreature() != NULL)
                GetLinkedCreature()->GetUnit()->Despawn(10000, 0);

            Creature* MAxes = NULL;
            MAxes = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_unit->GetPositionX(), _unit->GetPositionY(),
                    _unit->GetPositionZ(), CN_AXES);
            if (MAxes)
                MAxes->Despawn(1000, 0);

            GameObject* MDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-11018.5f, -1967.92f, 276.652f, 185134);
            // Open door
            if (MDoor)
                MDoor->SetState(GO_STATE_OPEN);
        }

        void OnTargetDied(Unit* mTarget)
        {
            switch (RandomUInt(2))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(2027);     // You are, but a plaything, unfit even to amuse.
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(2026);     // Your greed, your foolishness has brought you to this end.
                    break;
                case 2:
                    _unit->SendScriptTextChatMessage(2025);     // Surely you did not think you could win.
                    break;
            }
        }

        void AIUpdate()
        {
            switch (m_phase)
            {
                case 1:
                    if (_unit->GetHealthPct() <= 60)
                        PhaseTwo();
                    break;
                case 2:
                    PhaseTwo();
                    if (_unit->GetHealthPct() <= 30)
                        PhaseThree();
                    break;
                case 3:
                    PhaseThree();
                    break;
                default:
                    m_phase = 1;
                    break;
            }
            uint32 t = (uint32)time(NULL);
            if (t > spells[1].casttime && _unit->GetAIInterface()->getNextTarget() && _unit->GetCurrentSpell() == NULL)
            {
                Enfeebler();
                spells[1].casttime = t + spells[1].cooldown;
                spells[5].casttime = t + spells[5].cooldown;
            }
            else if (t > m_spawn_infernal && m_infernal == true && _unit->GetAIInterface()->getNextTarget())
            {
                _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_INFERNAL, ranX, ranY, 276.0f, 0, true, false, 0, 0);
                m_spawn_infernal = 0;
                m_infernal = false;
            }
            else if (t > spells[5].casttime && _unit->GetAIInterface()->getNextTarget() && _unit->GetCurrentSpell() == NULL)
            {
                spells[5].casttime = -1;
                _unit->CastSpell(_unit, spells[5].info, spells[5].instant);
                m_enfeebleoff = t + 3;
            }
            else if (t > m_enfeebleoff)
            {
                EnfeebleOff();
                m_enfeebleoff = -1;
            }
            else if (t > spells[2].casttime)
            {
                SummonInfernal();
                if (m_phase == 3)
                    spells[2].casttime = t + 20;
                else
                    spells[2].casttime = t + spells[2].cooldown;
            }
            else
            {
                float val = RandomFloat(100.0f);
                SpellCast(val);
            }
        }

        void PhaseTwo()
        {
            if (_unit->GetHealthPct() <= 60 && m_phase == 1)
            {
                _unit->SendScriptTextChatMessage(2020);     // Time is the fire in which you'll burn!");

                uint32 t = (uint32)time(NULL);
                spells[0].casttime = -1;
                spells[3].casttime = t + spells[3].cooldown;
                spells[3].perctrigger = 50.0f;

                _unit->CastSpell(_unit, spells[6].info, spells[6].instant);
                _unit->CastSpell(_unit, spells[7].info, spells[6].instant);

                // Main hand weapon
                _unit->SetEquippedItem(MELEE, AXE_ITEM_MODEL);

                //Off Hand
                _unit->SetEquippedItem(OFFHAND, AXE_ITEM_MODEL);

                CreatureProto* cp = CreatureProtoStorage.LookupEntry(CN_MALCHEZAAR);
                if (!cp)
                    return;

                _unit->SetMinDamage(1.5f * cp->MinDamage);
                _unit->SetMaxDamage(1.5f * cp->MaxDamage);

                m_phase = 2;
            }
            else
            {
                float val = RandomFloat(100.0f);
                SpellCast(val);
            }
        }

        void PhaseThree()
        {
            if (_unit->GetHealthPct() <= 30 && m_phase == 2)
            {
                _unit->SendScriptTextChatMessage(2024);     // How can you hope to withstand against such overwhelming power?

                uint32 t = (uint32)time(NULL);

                spells[0].targettype = TARGET_RANDOM_SINGLE;
                spells[0].casttime = t + spells[0].cooldown;

                spells[1].casttime = -1;
                spells[1].perctrigger = 0.0f;

                spells[4].casttime = t + spells[4].cooldown;
                spells[4].perctrigger = 50.0f;

                _unit->CastSpell(_unit, spells[8].info, spells[8].instant);

                _unit->RemoveAura(THRASH_AURA);
                _unit->RemoveAura(WIELD_AXES);

                // Main hand weapon
                _unit->SetEquippedItem(MELEE, 0);

                //Off Hand
                _unit->SetEquippedItem(OFFHAND, 0);

                CreatureProto* cp = CreatureProtoStorage.LookupEntry(CN_MALCHEZAAR);
                if (!cp)
                    return;

                _unit->SetMinDamage(cp->MinDamage);
                _unit->SetMaxDamage(cp->MaxDamage);
                m_phase = 3;
            }
            else
            {
                float val = RandomFloat(100.0f);
                SpellCast(val);
            }
        }

        void SummonInfernal()
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(2029);     // You face not Malchezaar alone, but the legions I command!
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(2028);     // All realities, all dimensions are open to me!
                    break;
            }

            ranX = RandomFloat(113.47f) - 11019.37f;
            ranY = RandomFloat(36.951f) - 2011.549f;
            if (GetLinkedCreature() != NULL)
            {
                GetLinkedCreature()->GetUnit()->CastSpellAoF(ranX, ranY, 275.0f, spells[2].info, spells[2].instant); // Shoots the missile
                float dist = GetLinkedCreature()->GetUnit()->CalcDistance(ranX, ranY, 275.0f);
                uint32 dtime = (uint32)(dist / spells[2].info->speed);
                m_spawn_infernal = (uint32)time(NULL) + dtime + 1;
                m_infernal = true;
            }
        }

        void Enfeebler()
        {
            std::vector<Player*> Targets;
            std::set< Object* >::iterator Itr = _unit->GetInRangePlayerSetBegin();

            for (; Itr != _unit->GetInRangePlayerSetEnd(); ++Itr)
            {
                if (isHostile(_unit, (*Itr)))
                {
                    Player* RandomTarget = static_cast<Player*>(*Itr);

                    if (RandomTarget->isAlive())
                        Targets.push_back(RandomTarget);
                }
            }

            while(Targets.size() > 5)
                Targets.erase(Targets.begin() + RandomUInt(Targets.size()));

            int i = 0;
            for (std::vector<Player*>::iterator E_Itr = Targets.begin(); E_Itr != Targets.end(); ++E_Itr)
            {
                if ((*E_Itr)->GetGUID() != _unit->GetAIInterface()->GetMostHated()->GetGUID())
                {
                    Enfeeble_Targets[i] = (*E_Itr)->GetGUID();
                    Enfeeble_Health[i] = (*E_Itr)->GetUInt32Value(UNIT_FIELD_HEALTH);

                    _unit->CastSpell((*E_Itr), spells[1].info, spells[1].instant);
                    (*E_Itr)->SetHealth(1);
                    i++;
                }
            }
        }

        void EnfeebleOff()
        {
            for (uint8 i = 0; i < 5; ++i)
            {
                Unit* ETarget = _unit->GetMapMgr()->GetUnit(Enfeeble_Targets[i]);
                if (ETarget && ETarget->isAlive())
                    ETarget->SetUInt64Value(UNIT_FIELD_HEALTH, Enfeeble_Health[i]);
                Enfeeble_Targets[i] = 0;
                Enfeeble_Health[i] = 0;
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
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
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

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Player* > TargetTable;
                for (std::set< Object* >::iterator itr = _unit->GetInRangePlayerSetBegin(); itr != _unit->GetInRangePlayerSetEnd(); ++itr)
                {
                    Player* RandomTarget = NULL;
                    RandomTarget = static_cast< Player* >(*itr);

                    if (RandomTarget && RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast)
                        TargetTable.push_back(RandomTarget);
                }

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, TargetTable.size() - 1);
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPositionX(), random_target->GetPositionY(), random_target->GetPositionZ(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:
        float ranX;
        float ranY;
        uint8 nrspells;
        int m_phase;
        bool m_infernal;
        uint32 m_enfeebleoff;
        uint32 m_spawn_infernal;
        uint64 Enfeeble_Targets[5];
        uint64 Enfeeble_Health[5];
};

class NetherInfernalAI : public MoonScriptBossAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(NetherInfernalAI, MoonScriptBossAI);
        NetherInfernalAI(Creature* pCreature) : MoonScriptBossAI(pCreature) {};

        void OnLoad()
        {
            SetCanMove(false);
            SetCanEnterCombat(false);
            _unit->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            _unit->m_noRespawn = true;
            RegisterAIUpdateEvent(6000);
            Despawn(175000, 0);
            _unit->CastSpell(_unit, dbcSpell.LookupEntry(HELLFIRE), true);
            ParentClass::OnLoad();
        };

        void AIUpdate()
        {
            _unit->CastSpell(_unit, dbcSpell.LookupEntry(HELLFIRE), true);
            ParentClass::AIUpdate();
        };

};

class InfernalDummyAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(InfernalDummyAI);
        InfernalDummyAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->GetAIInterface()->addWayPoint(CreateWaypoint(1, 0, 768));
        }

        inline WayPoint* CreateWaypoint(int id, uint32 waittime, uint32 flags)
        {
            WayPoint* wp = _unit->CreateWaypointStruct();
            wp->id = id;
            wp->x = -10938.56f;
            wp->y = -2041.26f;
            wp->z = 305.132f;
            wp->o = 0;
            wp->waittime = waittime;
            wp->flags = flags;
            wp->forwardemoteoneshot = false;
            wp->forwardemoteid = 0;
            wp->backwardemoteoneshot = false;
            wp->backwardemoteid = 0;
            wp->forwardskinid = 0;
            wp->backwardskinid = 0;
            return wp;
        }

};

class MAxesAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(MAxesAI);
        SP_AI_Spell spells[1];

        MAxesAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);

            spells[0].info = dbcSpell.LookupEntry(DEMONIC_FRENZY);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].cooldown = 1;
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(6000);

            spells[0].casttime = (uint32)time(NULL) + spells[0].cooldown;

            std::vector<Unit* > TargetTable;
            for (std::set< Object* >::iterator itr = _unit->GetInRangePlayerSetBegin(); itr != _unit->GetInRangePlayerSetEnd(); ++itr)
            {
                if (isHostile(_unit, (*itr)) && (static_cast< Player* >(*itr))->isAlive())
                {
                    Player* RandomTarget = NULL;
                    RandomTarget = static_cast<Player*>(*itr);

                    if (RandomTarget && RandomTarget->isAlive() && isHostile(_unit, RandomTarget))
                        TargetTable.push_back(RandomTarget);
                }
            }

            if (!TargetTable.size())
                return;

            auto random_index = RandomUInt(0, TargetTable.size() - 1);
            auto random_target = TargetTable[random_index];

            if (random_target == nullptr)
                return;

            _unit->GetAIInterface()->taunt(random_target, true);
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
            uint32 t = (uint32)time(NULL);
            if (t > spells[0].casttime)
            {
                _unit->CastSpell(_unit, spells[0].info, spells[0].instant);
                spells[0].casttime = t + spells[0].cooldown;
            }
        }

};

// Netherspite
#define CN_NETHERSPITE        15689
#define CN_VOIDZONE            17470

// #define NETHERBURN        30523 not aura
// #define VOIDZONE            28863
#define CONSUMPTION            32251 // used by void zone
#define NETHERBREATH        38524 // old 36631
#define N_BERSERK            38688
#define NETHERBURN            30522

class NetherspiteAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(NetherspiteAI);
        bool m_spellcheck[3];
        SP_AI_Spell spells[3];

        NetherspiteAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(NETHERBREATH);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].cooldown = RandomUInt(5) + 30;
            spells[0].perctrigger = 50.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(N_BERSERK);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].cooldown = 540;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(NETHERBURN);
            spells[2].targettype = TARGET_SELF;
            spells[2].instant = true;

            VoidTimer = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;

            uint32 t = (uint32)time(NULL);
            VoidTimer = t + 25;
            _unit->CastSpell(_unit, spells[2].info, spells[2].instant);

            RegisterAIUpdateEvent(1000);

            GameObject* NDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-11186.2f, -1665.14f, 281.398f, 185521);
            if (NDoor)
            {
                NDoor->SetState(GO_STATE_CLOSED);
                NDoor->SetFlags(33);
            }
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->RemoveAura(NETHERBURN);

            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();

            GameObject* NDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-11186.2f, -1665.14f, 281.398f, 185521);
            if (NDoor)
                NDoor->SetState(GO_STATE_OPEN);
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();

            GameObject* NDoor = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-11186.2f, -1665.14f, 281.398f, 185521);
            if (NDoor)
                NDoor->SetState(GO_STATE_OPEN);
        }

        void AIUpdate()
        {
            uint32 t = (uint32)time(NULL);
            if (t > VoidTimer && _unit->GetAIInterface()->getNextTarget())
            {
                VoidTimer = t + 20;
                std::vector<Unit* > TargetTable;
                for (std::set< Object* >::iterator itr = _unit->GetInRangePlayerSetBegin(); itr != _unit->GetInRangePlayerSetEnd(); ++itr)
                {
                    Unit* RandomTarget = NULL;
                    RandomTarget = static_cast< Unit* >(*itr);

                    if (RandomTarget && RandomTarget->isAlive() && isHostile(_unit, (*itr)))
                        TargetTable.push_back(RandomTarget);
                }

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, TargetTable.size() - 1);
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                float vzX = 5 * cos(RandomFloat(6.28f)) + random_target->GetPositionX();
                float vzY = 5 * cos(RandomFloat(6.28f)) + random_target->GetPositionY();
                float vzZ = random_target->GetPositionZ();
                _unit->GetMapMgr()->GetInterface()->SpawnCreature(CN_VOIDZONE, vzX, vzY, vzZ, 0, true, false, 0, 0);
                TargetTable.clear();
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
                        m_spellcheck[i] = false;
                        return;
                    }

                    uint32 t = (uint32)time(NULL);
                    if (val > comulativeperc && val <= (comulativeperc + spells[i].perctrigger) && t > spells[i].casttime)
                    {
                        _unit->setAttackTimer(spells[i].attackstoptimer, false);
                        spells[i].casttime = t + spells[i].cooldown;
                        m_spellcheck[i] = true;
                    }
                    comulativeperc += spells[i].perctrigger;
                }
            }
        }

    protected:
        uint8 nrspells;
        uint32 VoidTimer;
};

class VoidZoneAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(VoidZoneAI);
        SP_AI_Spell spells[1];

        VoidZoneAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->Root();
            _unit->DisableAI();
            _unit->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
            _unit->GetAIInterface()->disable_melee = true;
            _unit->GetAIInterface()->m_canMove = false;
            _unit->m_noRespawn = true;
            _unit->Despawn(30000, 0);

            RegisterAIUpdateEvent(2000);

            spells[0].info = dbcSpell.LookupEntry(CONSUMPTION);
            spells[0].instant = true;
            spells[0].cooldown = 2;
            spells[0].casttime = (uint32)time(NULL) + spells[0].cooldown;

            _unit->CastSpell(_unit, spells[0].info, spells[0].instant);
        }

        void AIUpdate()
        {
            uint32 t = (uint32)time(NULL);
            if (t > spells[0].casttime)
            {
                _unit->CastSpell(_unit, spells[0].casttime, spells[0].instant);
                spells[0].casttime = t + spells[0].cooldown;
            }
        }

};

//------------------------------------
//    -= Nightbane =-
//------------------------------------

/* \todo 
 - Rain of Bones on one random player/pet
 - Summons five Restless Skeletons.
*/

#define CN_NIGHTBANE 17225
#define CN_RESTLESS_SKELETON 17261 // not needed if spell works


// ground spells
#define BELLOWING_ROAR 36922
#define CHARRED_EARTH 30129 //Also 30209 (Target Charred Earth) triggers this
#undef CLEAVE
#define CLEAVE 31043 // fixme: correct spell?!
#define SMOLDERING_BREATH 39385
#define TAIL_SWEEP 25653 ///\todo  how to use this spell???
#define DISTRACTING_ASH 30280

// air spells
#define DISTRACTING_ASH_FLY 30130 // all guides say ground spell but animation is flying?!
#define RAIN_OF_BONES 37091 // Spell bugged: should debuff with 37098
#define SMOKING_BLAST 37057
#define FIREBALL_BARRAGE 30282
#define SUMMON_BONE_SKELETONS 30170

static Location coords[] =
{
    { 0, 0, 0, 0 },
    { -11173.719727f, -1863.993164f, 130.390396f, 5.343079f }, // casting point
    { -11125.542969f, -1926.884644f, 139.349365f, 3.982360f },
    { -11166.404297f, -1950.729736f, 114.714726f, 1.537812f },
    { -11167.497070f, -1922.315918f, 91.473755f, 1.390549f } // landing point
};

class NightbaneAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(NightbaneAI);
        bool m_spellcheck[5];
        SP_AI_Spell spells[5];

        NightbaneAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 5;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            //ground phase spells
            spells[0].info =  dbcSpell.LookupEntry(BELLOWING_ROAR);
            spells[0].targettype = TARGET_VARIOUS;
            spells[0].instant = false;
            spells[0].cooldown = 30; //confirmed
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1500;

            spells[1].info = dbcSpell.LookupEntry(CHARRED_EARTH);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = false;
            spells[1].cooldown = 15;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(CLEAVE);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].cooldown = 7;
            spells[2].perctrigger = 0.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(DISTRACTING_ASH);
            spells[3].targettype = TARGET_RANDOM_SINGLE;
            spells[3].instant = false;
            spells[3].cooldown = 60;
            spells[3].perctrigger = 0.0f;
            spells[3].attackstoptimer = 1000;

            spells[4].info = dbcSpell.LookupEntry(SMOLDERING_BREATH);
            spells[4].targettype = TARGET_ATTACKING;
            spells[4].instant = false;
            spells[4].cooldown = 20;
            spells[4].perctrigger = 0.0f;
            spells[4].attackstoptimer = 1000;

            _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_DONTMOVEWP);

            for (uint8 i = 1; i < 5; i++)
            {
                _unit->GetAIInterface()->addWayPoint(CreateWaypoint(i, 0, Flag_Fly));
            }

            m_phase = 0;
            m_currentWP = 0;
            mTailSweepTimer = 0;
            m_FlyPhaseTimer = 0;
        }

        void OnCombatStart(Unit* mTarget)
        {
            ResetCastTime();
            m_phase = 0;
            m_currentWP = 4;
            mTailSweepTimer = 25;
            //not sure about this: _unit->PlaySoundToSet(9456);
            ///\todo  "An ancient being awakens in the distance..."
            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_DONTMOVEWP);
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
            _unit->GetAIInterface()->StopFlying();
            _unit->GetAIInterface()->m_canMove = true;
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            switch (m_phase)
            {
                case 1:
                case 3:
                case 5:
                    FlyPhase();
                    break;

                case 0:
                case 2:
                case 4:
                    GroundPhase();
                    break;
            }
        }

        void OnReachWP(uint32 iWaypointId, bool bForwards)
        {
            switch (iWaypointId)
            {
                case 1: //casting point
                    {
                        _unit->GetAIInterface()->m_canMove = false;
                        _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
                        _unit->GetAIInterface()->SetAIState(STATE_SCRIPTIDLE);
                        m_currentWP = 1;
                    }
                    break;
                case 4: //ground point
                    {
                        _unit->GetAIInterface()->SetAllowedToEnterCombat(true);
                        _unit->GetAIInterface()->SetAIState(STATE_SCRIPTIDLE);
                        Land();
                        m_currentWP = 4;
                    }
                    break;
                default:
                    {
                        //move to the next waypoint
                        _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_WANTEDWP);
                        _unit->GetAIInterface()->setWaypointToMove(iWaypointId + 1);
                    }
                    break;
            };
        }

        void FlyPhase()
        {
            if (m_currentWP != 1)
                return;

            m_FlyPhaseTimer--;
            if (!m_FlyPhaseTimer)
            {
                if (_unit->GetCurrentSpell() != NULL)
                    _unit->GetCurrentSpell()->cancel();

                _unit->GetAIInterface()->m_canMove = true;
                _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
                _unit->GetAIInterface()->StopMovement(0);
                _unit->GetAIInterface()->SetAIState(STATE_SCRIPTMOVE);
                _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_WANTEDWP);
                _unit->GetAIInterface()->setWaypointToMove(2);
                m_phase++;
                return;
            }

            if (m_FlyPhaseTimer > 15)
                return;

            Unit* target = NULL;

            //first cast
            if (m_FlyPhaseTimer == 15)
            {
                //Casts Rain of Bones on one random player/pet
                //CastSpellOnRandomTarget(5, 0, 40);
                //summon 3 skeletons
                //_unit->CastSpellAoF(target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(), dbcSpell.LookupEntry(SUMMON_BONE_SKELETONS), true);
                return;
            }

            //Shoots powerful Smoking Blast every second for approximately 15 seconds.
            if (_unit->GetAIInterface()->getNextTarget() != NULL)
            {
                target = _unit->GetAIInterface()->getNextTarget();
                _unit->CastSpell(target, dbcSpell.LookupEntry(SMOKING_BLAST), true);
            }

            target = NULL;
            //fireball barrage check
            for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
            {
                if ((*itr)->IsPlayer())
                {
                    target = static_cast<Unit*>(*itr);

                    if (_unit->GetDistance2dSq(target) > 2025) //45 yards
                    {
                        _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), dbcSpell.LookupEntry(FIREBALL_BARRAGE), true);
                        break; //stop
                    }
                }
            }
        }

        void GroundPhase()
        {
            if (m_currentWP != 4)
                return;

            //Switch if needed
            if ((m_phase == 0 && _unit->GetHealthPct() <= 75)
                    || (m_phase == 2 && _unit->GetHealthPct() <= 50)
                    || (m_phase == 4 && _unit->GetHealthPct() <= 25))
            {
                if (_unit->GetCurrentSpell() != NULL)
                    _unit->GetCurrentSpell()->cancel();

                _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
                _unit->GetAIInterface()->StopMovement(0);
                _unit->GetAIInterface()->SetAIState(STATE_SCRIPTMOVE);
                _unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_WANTEDWP);
                _unit->GetAIInterface()->setWaypointToMove(1);
                Fly();
                m_FlyPhaseTimer = 17;
                m_phase++;
                return;
            }

            //Tail Sweep
            mTailSweepTimer--;
            if (!mTailSweepTimer)
            {
                Unit* target = NULL;
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if ((*itr)->IsPlayer())
                    {
                        target = static_cast<Unit*>(*itr);

                        //cone behind the boss
                        if (target->isAlive() && target->isInBack(_unit))
                            _unit->CastSpell(target, dbcSpell.LookupEntry(TAIL_SWEEP), true);
                    }
                }
                mTailSweepTimer = 25;
            }

            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        inline WayPoint* CreateWaypoint(int id, uint32 waittime, uint32 flags)
        {
            WayPoint* wp = _unit->CreateWaypointStruct();
            wp->id = id;
            wp->x = coords[id].x;
            wp->y = coords[id].y;
            wp->z = coords[id].z;
            wp->o = coords[id].o;
            wp->waittime = waittime;
            wp->flags = flags;
            wp->forwardemoteoneshot = false;
            wp->forwardemoteid = 0;
            wp->backwardemoteoneshot = false;
            wp->backwardemoteid = 0;
            wp->forwardskinid = 0;
            wp->backwardskinid = 0;
            return wp;
        }

        void Fly()
        {
            _unit->Emote(EMOTE_ONESHOT_LIFTOFF);

            WorldPacket data(SMSG_MOVE_SET_HOVER, 13);
            data << _unit->GetNewGUID();
            data << uint32(0);
            _unit->SendMessageToSet(&data, false);

            _unit->GetAIInterface()->SetFly();
        }

        void Land()
        {
            _unit->Emote(EMOTE_ONESHOT_LAND);

            WorldPacket data(SMSG_MOVE_UNSET_HOVER, 13);
            data << _unit->GetNewGUID();
            data << uint32(0);
            _unit->SendMessageToSet(&data, false);

            _unit->GetAIInterface()->StopFlying();
        }

        void ResetCastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_SINGLE:
                                CastSpellOnRandomTarget(i, 0, 0);
                                break;
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

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;        // From M4ksiu - Big THX to Capt
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if ((*itr)->IsUnit())
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && _unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(_unit, RandomTarget))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, TargetTable.size() - 1);
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                switch (spells[i].targettype)
                {
                    case TARGET_RANDOM_SINGLE:
                        _unit->CastSpell(random_target, spells[i].info, spells[i].instant);
                        break;
                    case TARGET_RANDOM_DESTINATION:
                        _unit->CastSpellAoF(random_target->GetPositionX(), random_target->GetPositionY(), random_target->GetPositionZ(), spells[i].info, spells[i].instant);
                        break;
                }

                TargetTable.clear();
            }
        }

    protected:
        uint8 nrspells;
        uint32 m_phase;
        uint32 m_FlyPhaseTimer;
        uint32 m_currentWP;
        uint32 mTailSweepTimer;
};

//Opera Event
//Wizzard of Oz
#define CN_DOROTHEE 17535

#define SP_AOE_FEAR 29321
#define SP_WATER_BOLT 31012
#define SP_SUMMON_TITO 31014

class DorotheeAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(DorotheeAI);
        bool m_spellcheck[2];
        SP_AI_Spell spells[2];
        uint32 summontito;


        DorotheeAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_AOE_FEAR);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].cooldown = 30;  //correct cooldown?
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_WATER_BOLT);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = false;
            spells[1].perctrigger = 100.0f;
            spells[1].attackstoptimer = 1000;

            summontito = 0;
            tito = NULL;
            titoSpawned = false;
            titoDeadSpeech = false;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(1978);     // Oh Tito, we simply must find a way home! The old wizard could be our only hope! Strawman, Roar, Tinhead, will you - wait... oh golly, look we have visitors!
            
            CastTime();
            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();

            _unit->Despawn(1, 0);
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(1975);     // Oh at last, at last I can go home!

            //Check to see if we can spawn The Crone now
            Creature* Dorothee    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10897.650f, -1755.8311f, 90.476f, 17535); //Dorothee
            Creature* Strawman    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10904.088f, -1754.8988f, 90.476f, 17543); //Strawman
            Creature* Roar        = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10891.115f, -1756.4898f, 90.476f, 17546);//Roar
            Creature* Tinman    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10884.501f, -1757.3249f, 90.476f, 17547); //Tinman

            if ((Dorothee == NULL || Dorothee->IsDead()) && (Strawman == NULL || Strawman->IsDead()) && (Roar == NULL || Roar->IsDead()) && (Tinman == NULL || Tinman->IsDead()))
            {
                _unit->GetMapMgr()->GetInterface()->SpawnCreature(18168, -10884.501f, -1757.3249f, 90.476f, 0.0f, true, true, 0, 0);
            }

            CastTime();
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        { }

        void SpawnTito()    // Lacking in collision checks!
        {
            float xchange  = RandomFloat(15.0f);
            float distance = 15.0f;

            float ychange = sqrt(distance * distance - xchange * xchange);

            if (RandomUInt(1) == 1)
                xchange *= -1;
            if (RandomUInt(1) == 1)
                ychange *= -1;

            float newposx = _unit->GetPositionX() + xchange;
            float newposy = _unit->GetPositionY() + ychange;

            tito = _unit->GetMapMgr()->GetInterface()->SpawnCreature(17548, newposx, newposy, _unit->GetPositionZ() + 0.5f, 2.177125f, true, false, 0, 0);
        }
        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);

            if (titoSpawned && !tito && titoDeadSpeech)
            {
                _unit->SendScriptTextChatMessage(1977);     // Tito! Oh Tito, no!

                titoDeadSpeech = false;
            }

            if (summontito > 20 && !titoSpawned)
            {
                _unit->SendScriptTextChatMessage(1976);     // Don't let them hurt us Tito! Oh, you won't, will you?

                SpawnTito();
                titoSpawned = true;
                titoDeadSpeech = true;
                return;
            }
            summontito++;
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
        }

        void CastSpellOnRandomTarget(uint32 i, float mindist2cast, float maxdist2cast, int minhp2cast, int maxhp2cast)
        {
            if (!maxdist2cast) maxdist2cast = 100.0f;
            if (!maxhp2cast) maxhp2cast = 100;

            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                std::vector<Unit*> TargetTable;
                for (std::set<Object*>::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
                {
                    if (((spells[i].targettype == TARGET_RANDOM_FRIEND && isFriendly(_unit, (*itr))) || (spells[i].targettype != TARGET_RANDOM_FRIEND && isHostile(_unit, (*itr)) && (*itr) != _unit)) && (*itr)->IsUnit())  // isAttackable(_unit, (*itr)) &&
                    {
                        Unit* RandomTarget = NULL;
                        RandomTarget = static_cast<Unit*>(*itr);

                        if (RandomTarget == _unit->GetAIInterface()->GetMostHated())
                            continue;

                        if (RandomTarget->isAlive() && _unit->GetDistance2dSq(RandomTarget) >= mindist2cast * mindist2cast && _unit->GetDistance2dSq(RandomTarget) <= maxdist2cast * maxdist2cast && ((RandomTarget->GetHealthPct() >= minhp2cast && RandomTarget->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND) || (_unit->GetAIInterface()->getThreatByPtr(RandomTarget) > 0 && isHostile(_unit, RandomTarget))))
                        {
                            TargetTable.push_back(RandomTarget);
                        }
                    }
                }

                if (_unit->GetHealthPct() >= minhp2cast && _unit->GetHealthPct() <= maxhp2cast && spells[i].targettype == TARGET_RANDOM_FRIEND)
                    TargetTable.push_back(_unit);

                if (!TargetTable.size())
                    return;

                auto random_index = RandomUInt(0, TargetTable.size() - 1);
                auto random_target = TargetTable[random_index];

                if (random_target == nullptr)
                    return;

                TargetTable.clear();
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
                                _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), spells[i].info, spells[i].instant);
                                break;
                            case TARGET_RANDOM_FRIEND:
                            case TARGET_RANDOM_SINGLE:
                            case TARGET_RANDOM_DESTINATION:
                                CastSpellOnRandomTarget(i, spells[i].mindist2cast, spells[i].maxdist2cast, spells[i].minhp2cast, spells[i].maxhp2cast);
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
        uint8 nrspells;
        Unit* tito;
        bool titoSpawned;
        bool titoDeadSpeech;
};

#define CN_TITO    17548

#define SP_ANNOYING_YIPPING    31015

//No kill sound

class TitoAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(TitoAI);
        bool m_spellcheck[1];
        SP_AI_Spell spells[1];


        TitoAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_ANNOYING_YIPPING);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 15.0f;
            spells[0].attackstoptimer = 1000;

            _unit->m_noRespawn = true;
        }

        void OnCombatStart(Unit* mTarget)
        {
            CastTime();
            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();

            _unit->Despawn(1, 0);
        }

        void OnDied(Unit* mKiller)
        {
            CastTime();
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
        uint8 nrspells;
};

#define CN_STRAWMAN    17543

#define SP_BURNING_STRAW    31075
#define SP_BRAIN_BASH    31046

class StrawmanAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(StrawmanAI);
        bool m_spellcheck[2];
        SP_AI_Spell spells[2];

        StrawmanAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_BURNING_STRAW);//  NEEDS TO BE SO IT ONLY AFFECTS HIM WHEN HE IS HIT BY FIRE DMG!
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = true;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_BRAIN_BASH);
            spells[1].targettype = TARGET_RANDOM_SINGLE;
            spells[1].instant = true;
            spells[1].cooldown = 8; //not sure about this
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;

        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(1982);     // Now what should I do with you? I simply can't make up my mind.

            CastTime();
            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();

            _unit->Despawn(1, 0);
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(1983);     // Don't let them make... a mattress outta' me.

            //Check to see if we can spawn The Crone now
            Creature* Dorothee    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10897.650f, -1755.8311f, 90.476f, 17535);    //Dorothee
            Creature* Strawman    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10904.088f, -1754.8988f, 90.476f, 17543);    //Strawman
            Creature* Roar        = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10891.115f, -1756.4898f, 90.476f, 17546);    //Roar
            Creature* Tinman    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10884.501f, -1757.3249f, 90.476f, 17547);    //Tinman

            if ((Dorothee == NULL || Dorothee->IsDead()) && (Strawman == NULL || Strawman->IsDead()) && (Roar == NULL || Roar->IsDead()) && (Tinman == NULL || Tinman->IsDead()))
            {
                _unit->GetMapMgr()->GetInterface()->SpawnCreature(18168, -10884.501f, -1757.3249f, 90.476f, 0.0f, true, true, 0, 0);
            }

            CastTime();
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(1984);     // I guess I'm not a failure after all!
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
        uint8 nrspells;
};

#define CN_TINHEAD    17547

#define SP_CLEAVE        15284
#define SP_RUST        31086
//dont bother.. spell does not work.. needs fix

class TinheadAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(TinheadAI);
        bool m_spellcheck[2];
        SP_AI_Spell spells[2];

        TinheadAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 2;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_CLEAVE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = true;
            spells[0].perctrigger = 0.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_RUST);
            spells[1].targettype = TARGET_SELF;
            spells[1].instant = true;
            spells[1].cooldown = 60;
            spells[1].perctrigger = 0.0f;
            spells[1].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(1985);     // I could really use a heart. Say, can I have yours?

            CastTime();
            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();

            _unit->Despawn(1, 0);
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(1986);     // Back to being an old rust bucket.

            //Check to see if we can spawn The Crone now
            Creature* Dorothee    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10897.650f, -1755.8311f, 90.476f, 17535);    //Dorothee
            Creature* Strawman    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10904.088f, -1754.8988f, 90.476f, 17543);    //Strawman
            Creature* Roar        = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10891.115f, -1756.4898f, 90.476f, 17546);    //Roar
            Creature* Tinman    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10884.501f, -1757.3249f, 90.476f, 17547);    //Tinman

            if ((Dorothee == NULL || Dorothee->IsDead()) && (Strawman == NULL || Strawman->IsDead()) && (Roar == NULL || Roar->IsDead()) && (Tinman == NULL || Tinman->IsDead()))
            {
                _unit->GetMapMgr()->GetInterface()->SpawnCreature(18168, -10884.501f, -1757.3249f, 90.476f, 0.0f, true, true, 0, 0);
            }

            CastTime();
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(1987);     // Guess I'm not so rusty after all.
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
        uint8 nrspells;
};

#define CN_ROAR    17546

class RoarAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(RoarAI);

        RoarAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(1979);     // I'm not afraid a' you! Do you wanna' fight? Huh, do ya'? C'mon! I'll fight ya' with both paws behind my back!
        }

        void OnCombatStop(Unit* mTarget)
        {
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            _unit->Despawn(1, 0);
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(1980);     // You didn't have to go and do that!

            //Check to see if we can spawn The Crone now
            Creature* Dorothee    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10897.650f, -1755.8311f, 90.476f, 17535); //Dorothee
            Creature* Strawman    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10904.088f, -1754.8988f, 90.476f, 17543); //Strawman
            Creature* Roar        = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10891.115f, -1756.4898f, 90.476f, 17546);//Roar
            Creature* Tinman    = _unit->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(-10884.501f, -1757.3249f, 90.476f, 17547); //Tinman

            if ((Dorothee == NULL || Dorothee->IsDead()) && (Strawman == NULL || Strawman->IsDead()) && (Roar == NULL || Roar->IsDead()) && (Tinman == NULL || Tinman->IsDead()))
            {
                _unit->GetMapMgr()->GetInterface()->SpawnCreature(18168, -10884.501f, -1757.3249f, 90.476f, 0.0f, true, true, 0, 0);
            }
        }

};

#define CN_CRONE 18168

#define SP_SUMMON_CYCLONE 38337
#define SP_CHAIN_LIGHTNING 32337

class CroneAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CroneAI);
        bool m_spellcheck[2];
        SP_AI_Spell spells[2];

        CroneAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 1;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_SUMMON_CYCLONE);
            spells[0].targettype = TARGET_DESTINATION;
            spells[0].instant = true;
            spells[0].perctrigger = 5.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_CHAIN_LIGHTNING);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = true;
            spells[1].perctrigger = 10.0f;
            spells[1].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(1989);     // Woe to each and every one of you, my pretties!

            CastTime();
            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)
            {
                GameObject* DoorLeft = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10917.1445f, -1774.05f, 90.478f, 184279);
                GameObject* DoorRight = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10872.195f, -1779.42f, 90.45f, 184278);
                GameObject* Curtain = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10894.17f, -1774.218f, 90.477f, 183932);

                if (DoorLeft)
                    DoorLeft->SetState(GO_STATE_CLOSED);

                if (DoorRight)
                    DoorRight->SetState(GO_STATE_OPEN);

                if (Curtain)
                    Curtain->SetState(GO_STATE_CLOSED);
            }

            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();

            _unit->Despawn(1, 0);
        }

        void OnDied(Unit* mKiller)
        {
            _unit->SendScriptTextChatMessage(1991);     // How could you? What a cruel, cruel world...

            GameObject* DoorLeft = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10917.1445f, -1774.05f, 90.478f, 184279);
            GameObject* DoorRight = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10872.195f, -1779.42f, 90.45f, 184278);
            GameObject* Curtain = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10894.17f, -1774.218f, 90.477f, 183932);

            if (DoorLeft)
                DoorLeft->SetState(GO_STATE_OPEN);

            if (DoorRight)
                DoorRight->SetState(GO_STATE_OPEN);

            // Make sure the curtain stays up
            if (Curtain)
                Curtain->SetState(GO_STATE_OPEN);

            CastTime();
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(1992);     // Fixed you, didn't I?
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
        uint8 nrspells;
};

#define CN_CYCLONEOZ            22104
#define CYCLONE_VISUAL            32332
#define CYCLONE_KNOCK            38517

class CycloneOZ : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(CycloneOZ);
        CycloneOZ(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _unit->CastSpell(_unit, dbcSpell.LookupEntry(CYCLONE_VISUAL), true);
            _unit->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
            _unit->GetAIInterface()->disable_melee = true;
            _unit->GetAIInterface()->m_canMove = false;
            _unit->m_noRespawn = true;
        }

        void OnCombatStart(Unit* mTarget)
        {
            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            _unit->CastSpell(_unit, dbcSpell.LookupEntry(CYCLONE_KNOCK), true);
        }
};

//Romulo & Julianne
#define CN_ROMULO 17533

#define SP_BACKWARD_LUNGE 30815
#define SP_DEADLY_SWATHE 30817
#define SP_POISONED_THRUST 30822
#define SP_DARING 30841

///\todo play sound on resurection
//_unit->SendScriptTextChatMessage(2005);     // Thou detestable maw, thou womb of death; I enforce thy rotten jaws to open!

class RomuloAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(RomuloAI);
        bool m_spellcheck[4];
        SP_AI_Spell spells[4];

        RomuloAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_BACKWARD_LUNGE);
            spells[0].targettype = TARGET_ATTACKING;
            spells[0].instant = false;
            spells[0].cooldown = 12;
            spells[0].perctrigger = 20.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_DEADLY_SWATHE);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = 0;
            spells[1].perctrigger = 20.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_POISONED_THRUST);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].cooldown = 0;
            spells[2].perctrigger = 20.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_DARING);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = false;
            spells[3].cooldown = 0;
            spells[3].perctrigger = 20.0f;
            spells[3].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(2002);     // Wilt thou provoke me? Then have at thee, boy!

            CastTime();
            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)
            {
                GameObject* DoorLeft = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10917.1445f, -1774.05f, 90.478f, 184279);
                GameObject* DoorRight = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10872.195f, -1779.42f, 90.45f, 184278);
                GameObject* Curtain = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10894.17f, -1774.218f, 90.477f, 183932);

                if (DoorLeft)
                    DoorLeft->SetState(GO_STATE_CLOSED);

                if (DoorRight)
                    DoorRight->SetState(GO_STATE_OPEN);

                if (Curtain)
                    Curtain->SetState(GO_STATE_CLOSED);
            }

            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
            _unit->Despawn(1, 0);
        }

        void OnDied(Unit* mKiller)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(2003);     // Thou smilest... upon the stroke that... murders me.
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(2004);     // This day's black fate on more days doth depend. This but begins the woe. Others must end.
                    break;
            }

            GameObject* DoorLeft = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10917.1445f, -1774.05f, 90.478f, 184279);
            GameObject* DoorRight = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10872.195f, -1779.42f, 90.45f, 184278);
            GameObject* Curtain = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10894.17f, -1774.218f, 90.477f, 183932);

            if (DoorLeft)
                DoorLeft->SetState(GO_STATE_OPEN);

            if (DoorRight)
                DoorRight->SetState(GO_STATE_OPEN);

            // Make sure the curtain stays up
            if (Curtain)
                Curtain->SetState(GO_STATE_OPEN);

            CastTime();
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(2006);     // How well my comfort is revived by this!
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
        uint8 nrspells;
};

#define CN_JULIANNE 17534

#define SP_ETERNAL_AFFECTION 30878
#define SP_POWERFUL_ATTRACTION 30889
#define SP_BINDING_PASSION 30890
#define SP_DEVOTION 30887

///\todo play sound on resurrection
//_unit->SendScriptTextChatMessage(2000);     // Come, gentle night; and give me back my Romulo!

class JulianneAI : public CreatureAIScript
{
    public:
        ADD_CREATURE_FACTORY_FUNCTION(JulianneAI);
        bool m_spellcheck[4];
        SP_AI_Spell spells[4];

        JulianneAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            nrspells = 4;
            for (uint8 i = 0; i < nrspells; i++)
            {
                m_spellcheck[i] = false;
            }

            spells[0].info = dbcSpell.LookupEntry(SP_ETERNAL_AFFECTION);
            spells[0].targettype = TARGET_SELF;
            spells[0].instant = false;
            spells[0].cooldown = 12;
            spells[0].perctrigger = 5.0f;
            spells[0].attackstoptimer = 1000;

            spells[1].info = dbcSpell.LookupEntry(SP_POWERFUL_ATTRACTION);
            spells[1].targettype = TARGET_ATTACKING;
            spells[1].instant = false;
            spells[1].cooldown = 0;
            spells[1].perctrigger = 5.0f;
            spells[1].attackstoptimer = 1000;

            spells[2].info = dbcSpell.LookupEntry(SP_BINDING_PASSION);
            spells[2].targettype = TARGET_ATTACKING;
            spells[2].instant = false;
            spells[2].cooldown = 0;
            spells[2].perctrigger = 5.0f;
            spells[2].attackstoptimer = 1000;

            spells[3].info = dbcSpell.LookupEntry(SP_DEVOTION);
            spells[3].targettype = TARGET_SELF;
            spells[3].instant = false;
            spells[3].cooldown = 0;
            spells[3].perctrigger = 5.0f;
            spells[3].attackstoptimer = 1000;
        }

        void OnCombatStart(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(1996);     // What devil art thou, that dost torment me thus?

            CastTime();
            RegisterAIUpdateEvent(1000);
        }

        void OnCombatStop(Unit* mTarget)
        {
            if (_unit->GetHealthPct() > 0)
            {
                GameObject* DoorLeft = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10917.1445f, -1774.05f, 90.478f, 184279);
                GameObject* DoorRight = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10872.195f, -1779.42f, 90.45f, 184278);
                GameObject* Curtain = _unit->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-10894.17f, -1774.218f, 90.477f, 183932);

                if (DoorLeft)
                    DoorLeft->SetState(GO_STATE_CLOSED);

                if (DoorRight)
                    DoorRight->SetState(GO_STATE_OPEN);

                if (Curtain)
                    Curtain->SetState(GO_STATE_CLOSED);
            }

            CastTime();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
            _unit->Despawn(1, 0);
        }

        void OnDied(Unit* mKiller)
        {
            switch (RandomUInt(1))
            {
                case 0:
                    _unit->SendScriptTextChatMessage(1998);     // Romulo, I come! Oh... this do I drink to thee!
                    break;
                case 1:
                    _unit->SendScriptTextChatMessage(1997);     // Where is my lord? Where is my Romulo?
                    break;
            }

            //_unit->RemoveAllAuras();
            //_unit->setEmoteState(EMOTE_ONESHOT_EAT);
            //_unit->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            _unit->GetMapMgr()->GetInterface()->SpawnCreature(17533, -10891.582f, -1755.5177f, 90.476f, 4.61f, true, true, 0, 0);
            //_unit->setEmoteState(EMOTE_STATE_DEAD);

            CastTime();
            RemoveAIUpdateEvent();
        }

        void OnTargetDied(Unit* mTarget)
        {
            _unit->SendScriptTextChatMessage(2001);     // Parting is such sweet sorrow.
        }

        void AIUpdate()
        {
            float val = RandomFloat(100.0f);
            SpellCast(val);
        }

        void CastTime()
        {
            for (uint8 i = 0; i < nrspells; i++)
                spells[i].casttime = spells[i].cooldown;
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
        uint8 nrspells;
};

void SetupKarazhan(ScriptMgr* mgr)
{
    GossipScript* KBerthold = new Berthold();
    mgr->register_gossip_script(16153, KBerthold);

    mgr->register_creature_script(CN_ATTUMEN, &AttumenTheHuntsmanAI::Create);
    mgr->register_creature_script(CN_MIDNIGHT, &MidnightAI::Create);
    mgr->register_creature_script(CN_MOROES, &MoroesAI::Create);
    mgr->register_creature_script(CN_MAIDENOFVIRTUE, &MaidenOfVirtueAI::Create);

    // Opera event related
    mgr->register_creature_script(CN_BIGBADWOLF, &BigBadWolfAI::Create);
    mgr->register_creature_script(CN_ROMULO, &RomuloAI::Create);
    mgr->register_creature_script(CN_JULIANNE, &JulianneAI::Create);
    mgr->register_creature_script(19525, &StageLight::Create);
    GossipScript* KGrandMother = new GrandMother;
    GossipScript* KBarnes = new BarnesGS;
    mgr->register_gossip_script(16812, KBarnes);
    mgr->register_gossip_script(17603, KGrandMother);
    mgr->register_creature_script(16812, &BarnesAI::Create);

    //WoOz here... commented yet to be implemented - kamyn
    mgr->register_creature_script(CN_DOROTHEE, &DorotheeAI::Create);
    mgr->register_creature_script(CN_STRAWMAN, &StrawmanAI::Create);
    mgr->register_creature_script(CN_TINHEAD, &TinheadAI::Create);
    mgr->register_creature_script(CN_ROAR, &RoarAI::Create);
    mgr->register_creature_script(CN_TITO, &TitoAI::Create);
    mgr->register_creature_script(CN_CRONE, &CroneAI::Create);
    mgr->register_creature_script(CN_CYCLONEOZ, &CycloneOZ::Create);

    mgr->register_creature_script(CN_CURATOR, &CuratorAI::Create);
    mgr->register_creature_script(CN_ASTRALFLARE, &AstralFlareAI::Create);

    mgr->register_creature_script(CN_ILLHOOF, &IllhoofAI::Create);
    mgr->register_creature_script(CN_KILREK, &KilrekAI::Create);
    mgr->register_creature_script(CN_FIENDISH_IMP, &FiendishImpAI::Create);
    mgr->register_creature_script(CN_DEMONCHAINS, &DemonChains::Create);
    mgr->register_creature_script(CN_FPORTAL, &FiendPortal::Create);

    mgr->register_creature_script(SHADEOFARAN, &ShadeofAranAI::Create);
    mgr->register_creature_script(WATERELE, &WaterEleAI::Create);
    mgr->register_creature_script(SHADOWOFARAN, &ShadowofAranAI::Create);

    mgr->register_creature_script(CN_NETHERSPITE, &NetherspiteAI::Create);
    mgr->register_creature_script(CN_VOIDZONE, &VoidZoneAI::Create);

    mgr->register_creature_script(CN_MALCHEZAAR, &MalchezaarAI::Create);
    mgr->register_creature_script(CN_INFERNAL, &NetherInfernalAI::Create);
    mgr->register_creature_script(CN_DUMMY, &InfernalDummyAI::Create);
    mgr->register_creature_script(CN_AXES, &MAxesAI::Create);

    mgr->register_creature_script(CN_NIGHTBANE, &NightbaneAI::Create);
}
