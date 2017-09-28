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
#include "Instance_ShadowfangKeep.h"


// Commander Springvale AI
class SpringvaleAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(SpringvaleAI, MoonScriptCreatureAI);
    SpringvaleAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Holy Light
        AddSpell(1026, Target_Self, 10, 2.5f, 0);
        // Devotion Aura (Not the right Spell ID)
        DevoAura = AddSpell(10290, Target_Self, 0, 0, 0);
        // Divine Protection (Not the right Spell ID)
        DivineProt = AddSpell(498, Target_Self, 0, 0, 0);
        // Hammer of Justice
        AddSpell(5588, Target_Current, 12, 0, 60);
    }

    void OnCombatStart(Unit* pTarget)
    {
        if (!GetUnit()->HasAura(DevoAura->mInfo->getId()))
            CastSpellNowNoScheduling(DevoAura);

        ParentClass::OnCombatStart(pTarget);
    }

    void AIUpdate()
    {
        if (GetHealthPercent() <= 20 && DivineProt->mEnabled == true)
        {
            CastSpellNowNoScheduling(DivineProt);
            DivineProt->mEnabled = false;
        }
        ParentClass::AIUpdate();
    }

    SpellDesc* DevoAura;
    SpellDesc* DivineProt;
};

// Odo the Blindwatcher AI
class BlindWatcherAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(BlindWatcherAI, MoonScriptBossAI);
    BlindWatcherAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        // Howling Rage 1
        HowlingRage1 = AddSpell(7481, Target_Self, 0, 5, 0);
        // Howling Rage 2
        HowlingRage2 = AddSpell(7483, Target_Self, 0, 1.5f, 0);
        // Howling Rage 3
        HowlingRage3 = AddSpell(7484, Target_Self, 0, 1.5f, 0);
    }

    void AIUpdate()
    {
        if (GetHealthPercent() <= 75 && GetPhase() == 1)
        {
            SetPhase(2, HowlingRage1);
        }
        else if (GetHealthPercent() <= 45 && GetPhase() == 2)
        {
            if (GetUnit()->HasAura(7481))
                RemoveAura(7481);
            SetPhase(3, HowlingRage2);
        }
        else if (GetHealthPercent() <= 20 && GetPhase() == 3)
        {
            if (GetUnit()->HasAura(7483))
                RemoveAura(7483);
            SetPhase(4, HowlingRage3);
        }
        ParentClass::AIUpdate();
    }

    SpellDesc* HowlingRage1;
    SpellDesc* HowlingRage2;
    SpellDesc* HowlingRage3;
};

// Fenrus the Devourer AI
static Movement::Location VWSpawns[] =
{
    {}, // Spawn Locations for the 4 voidwalkers
    { -154.274368f, 2177.196533f, 128.448517f, 5.760980f },
    { -142.647537f, 2181.019775f, 128.448410f, 4.178475f },
    { -139.146774f, 2168.201904f, 128.448364f, 2.650803f },
    { -150.860092f, 2165.156250f, 128.448502f, 0.999966f },
};

class FenrusAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(FenrusAI, MoonScriptCreatureAI);
    FenrusAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(7125, Target_Current, 12, 1.5f, 60);
    }

    void OnDied(Unit* pKiller)
    {
        _unit->SendScriptTextChatMessage(SAY_FENRUS_01);

        MoonScriptCreatureAI* voidwalker = NULL;
        // Spawn 4 x Arugal's Voidwalkers
        for (uint8 x = 1; x < 5; x++)
        {
            voidwalker = SpawnCreature(4627, VWSpawns[x].x, VWSpawns[x].y, VWSpawns[x].z, VWSpawns[x].o);
            if (voidwalker)
            {
                voidwalker->AggroNearestPlayer();
                voidwalker = NULL;
            }
        }

        ParentClass::OnDied(pKiller);
    }
};

//Arugals Voidwalkers
class VoidWalkerAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(VoidWalkerAI, MoonScriptCreatureAI);
    VoidWalkerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(7154, Target_WoundedFriendly, 5, 0, 7);
    }

    void OnDied(Unit* pKiller)
    {
        GameObject* pDoor = GetUnit()->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-129.034f, 2166.16f, 129.187f, GO_SORCERER_GATE);
        if (pDoor)
            pDoor->SetState(GO_STATE_OPEN);

        ParentClass::OnDied(pKiller);
    }
};

// Archmage Arugal AI
class ArugalAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(ArugalAI, MoonScriptCreatureAI);
    ArugalAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Void Bolt
        AddSpell(7588, Target_Current, 25, 3, 0);
        // Thunder Shock
        AddSpell(7803, Target_Self, 10, 0, 0);
        // Arugal's Curse
        AddSpell(7621, Target_RandomPlayer, 5, 0, 0, 0, 0, false, "Release your rage!", Text_Yell, 5797);
        // Arugal spawn-in spell (Teleport)
        AddSpell(10418, Target_Self, 10, 2, 0);

        AddEmote(Event_OnCombatStart, "You, too, shall serve!", Text_Yell, 5793);
        AddEmote(Event_OnTargetDied, "Another falls!", Text_Yell, 5795);
    }
};

//Wolf Master Nandos AI
class NandosAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(NandosAI, MoonScriptCreatureAI);
    NandosAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature) {}

    void OnDied(Unit* pKiller)
    {
        GameObject* pDoor = GetUnit()->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-118.11f, 2161.86f, 155.678f, 18971);
        if (pDoor)
            pDoor->SetState(GO_STATE_OPEN);

        ParentClass::OnDied(pKiller);
    }
};

//Deathstalker Adamant
///\todo Deathstalker Adamant seems to be missing here... is it here? or is it there?... no I cant find it ;)


//RethilgoreAI
class RethilgoreAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(RethilgoreAI, MoonScriptCreatureAI);
    RethilgoreAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature) {}
    void OnDied(Unit* pKiller)
    {
        ///\todo check these texts
        _unit->SendChatMessageAlternateEntry(3849, CHAT_MSG_MONSTER_SAY, LANG_GUTTERSPEAK, "About time someone killed the wretch.");
        _unit->SendChatMessageAlternateEntry(3850, CHAT_MSG_MONSTER_SAY, LANG_COMMON, "For once I agree with you... scum.");      // dont know the allys text yet
        ParentClass::OnDied(pKiller);
    }
};

// Prison Levers
class RightLever : public GameObjectAIScript
{
    public:

        RightLever(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
        static GameObjectAIScript* Create(GameObject* GO) { return new RightLever(GO); }

        void OnActivate(Player* pPlayer)
        {
            GameObject* CellDoor = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-252.696f, 2114.22f, 82.8052f, GO_RIGHT_CELL);
            if (CellDoor != NULL)
            {
                if (CellDoor->GetState() == GO_STATE_CLOSED)
                    CellDoor->SetState(GO_STATE_OPEN);
                else
                    CellDoor->SetState(GO_STATE_CLOSED);
            }
        }
};

class MiddleLever : public GameObjectAIScript
{
    public:

        MiddleLever(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
        static GameObjectAIScript* Create(GameObject* GO) { return new MiddleLever(GO); }

        void OnActivate(Player* pPlayer)
        {
            GameObject* CellDoor = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-249.22f, 2123.1f, 82.8052f, GO_MIDDLE_CELL);
            if (CellDoor != NULL)
            {
                if (CellDoor->GetState() == GO_STATE_CLOSED)
                    CellDoor->SetState(GO_STATE_OPEN);
                else
                    CellDoor->SetState(GO_STATE_CLOSED);
            }
        }
};

class LeftLever : public GameObjectAIScript
{
    public:

        LeftLever(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
        static GameObjectAIScript* Create(GameObject* GO) { return new LeftLever(GO); }

        void OnActivate(Player* pPlayer)
        {
            GameObject* CellDoor = pPlayer->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(-245.598f, 2132.32f, 82.8052f, GO_LEFT_CELL);
            if (CellDoor != NULL)
            {
                if (CellDoor->GetState() == GO_STATE_CLOSED)
                    CellDoor->SetState(GO_STATE_OPEN);
                else
                    CellDoor->SetState(GO_STATE_CLOSED);
            }
        }
};

void SetupShadowfangKeep(ScriptMgr* mgr)
{
    //Creatures
    mgr->register_creature_script(CN_NENDOS, &NandosAI::Create);
    mgr->register_creature_script(CN_VOIDWALKER, &VoidWalkerAI::Create);
    mgr->register_creature_script(CN_RETHILGORE, &RethilgoreAI::Create);
    mgr->register_creature_script(CN_SPRINGVALE, &SpringvaleAI::Create);
    mgr->register_creature_script(CN_BLINDWATCHER, &BlindWatcherAI::Create);
    mgr->register_creature_script(CN_FENRUS, &FenrusAI::Create);
    mgr->register_creature_script(CN_ARUGAL, &ArugalAI::Create);

    //Gameobjects
    mgr->register_gameobject_script(GO_RIGHT_LEVER, &RightLever::Create);
    mgr->register_gameobject_script(GO_MIDDLE_LEVER, &MiddleLever::Create);
    mgr->register_gameobject_script(GO_LEFT_LEVER, &LeftLever::Create);
}
