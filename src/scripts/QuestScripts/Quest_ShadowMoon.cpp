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

class InfiltratingDragonmawFortressQAI : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(InfiltratingDragonmawFortressQAI);
    InfiltratingDragonmawFortressQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller)
    {
        if (mKiller->IsPlayer())
        {
            static_cast<Player*>(mKiller)->AddQuestKill(10836, 0, 0);
        }
    }
};

class KneepadsQAI : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(KneepadsQAI);
    KneepadsQAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* mKiller)
    {
        if (mKiller->IsPlayer())
        {
            static_cast<Player*>(mKiller)->AddQuestKill(10703, 0, 0);
            static_cast<Player*>(mKiller)->AddQuestKill(10702, 0, 0);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// Deathbringer Jovaan
const uint32 CN_DEATHBRINGER_JOVAAN = 21633;

//WP Coords Wait Times
struct WPWaitTimes
{
    Movement::Location mCoords;
    uint32 WaitTime;
};

const WPWaitTimes DeathbringerJovaanWP[] =
{
    { { }, 0},
    { { -3310.743896f, 2951.929199f, 171.132538f, 5.054039f }, 0 },
    { { -3308.501221f, 2940.098389f, 171.025772f, 5.061895f }, 0 },
    { { -3306.261203f, 2933.843210f, 170.934145f, 5.474234f }, 44000 },
    { { -3310.743896f, 2951.929199f, 171.132538f, 1.743588f }, 0 }
};

class DeathbringerJovaanAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(DeathbringerJovaanAI, MoonScriptCreatureAI);
    DeathbringerJovaanAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        mJovaanTimer = INVALIDATE_TIMER;
        mJovaanPhase = -1;

        for (int i = 1; i < 5; ++i)
            AddWaypoint(CreateWaypoint(i, DeathbringerJovaanWP[i].WaitTime, Movement::WP_MOVE_TYPE_WALK, DeathbringerJovaanWP[i].mCoords));
    }

    void AIUpdate()
    {
        if (_isTimerFinished(mJovaanTimer))
        {
            switch (mJovaanPhase)
            {
                case 0:
                {
                    CreatureAIScript* pRazuunAI = spawnCreatureAndGetAIScript(21502, -3300.47f, 2927.22f, 173.870f, 2.42924f);    // Spawn Razuun
                    if (pRazuunAI != nullptr)
                    {
                        pRazuunAI->getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
                        pRazuunAI->setCanEnterCombat(false);
                        pRazuunAI->SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                        pRazuunAI->setRooted(true);
                    }
                    getCreature()->SetStandState(STANDSTATE_KNEEL);
                    getCreature()->Emote(EMOTE_ONESHOT_TALK);
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Everything is in readiness, warbringer.");
                    mJovaanPhase = 1;
                    _resetTimer(mJovaanTimer, 6000);
                }
                break;
                case 1:
                {
                    getCreature()->Emote(EMOTE_ONESHOT_TALK);
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Warbringer, that will require the use of all the hold's infernals. It may leave us vulnerable to a counterattack.");
                    mJovaanPhase = 2;
                    _resetTimer(mJovaanTimer, 11000);
                }
                break;
                case 2:
                {
                    getCreature()->SetStandState(STANDSTATE_STAND);
                    mJovaanPhase = 3;
                    _resetTimer(mJovaanTimer, 1000);
                }
                break;
                case 3:
                {
                    getCreature()->Emote(EMOTE_ONESHOT_SALUTE);
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "It shall be as you say, warbringer. One last question, if I may...");
                    mJovaanPhase = 4;
                    _resetTimer(mJovaanTimer, 10000);
                }
                break;
                case 4:
                {
                    getCreature()->Emote(EMOTE_ONESHOT_QUESTION);
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "What's in the crate?");
                    mJovaanPhase = 5;
                    _resetTimer(mJovaanTimer, 10000);
                }
                break;
                case 5:
                {
                    getCreature()->Emote(EMOTE_ONESHOT_SALUTE);
                    mJovaanPhase = -1;
                    _removeTimer(mJovaanTimer);
                }
                break;
            }
        }
        ParentClass::AIUpdate();
    }

    void OnReachWP(uint32 iWaypointId, bool bForwards)
    {
        switch (iWaypointId)
        {
            case 3:
            {
                RegisterAIUpdateEvent(1000);
                getCreature()->Emote(EMOTE_ONESHOT_POINT);
                mJovaanPhase = 0;
                mJovaanTimer = _addTimer(1500);
            }
            break;
            case 4:
            {
                despawn(1, 0);
            }
            break;
        }
    }

    uint32    mJovaanTimer;
    int32    mJovaanPhase;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// Warbringer Razuun
#define CN_WARBRINGER_RAZUUN    21502

class WarbringerRazuunAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(WarbringerRazuunAI, MoonScriptCreatureAI);
    WarbringerRazuunAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        RegisterAIUpdateEvent(1000);
        mRazuunTimer = _addTimer(800);
        mRazuunPhase = 0;
    }

    void AIUpdate()
    {
        if (_isTimerFinished(mRazuunTimer))
        {
            switch (mRazuunPhase)
            {
                case 0:
                {
                    getCreature()->Emote(EMOTE_ONESHOT_TALK);
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Doom Lord Kazzak will be pleased. You are to increase the pace of your attacks. Destroy the orcish and dwarven strongholds with all haste.");
                    mRazuunPhase = 1;
                    _resetTimer(mRazuunTimer, 9000);
                }
                break;
                case 1:
                {
                    getCreature()->Emote(EMOTE_ONESHOT_TALK);
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Don't worry about that. I've increased production at the Deathforge. You'll have all the infernals you need to carry out your orders. Don't fail, Jovaan.");
                    mRazuunPhase = 2;
                    _resetTimer(mRazuunTimer, 15000);
                }
                break;
                case 2:
                {
                    getCreature()->Emote(EMOTE_ONESHOT_QUESTION);
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Yes?");
                    mRazuunPhase = 3;
                    _resetTimer(mRazuunTimer, 8000);
                }
                break;
                case 3:
                {
                    getCreature()->Emote(EMOTE_ONESHOT_QUESTION);
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Crate? I didn't send you a crate, Jovaan. Don't you have more important things to worry about? Go see to them!");
                    mRazuunPhase = 4;
                    _resetTimer(mRazuunTimer, 5000);
                }
                break;
                case 4:
                {
                    mRazuunPhase = -1;
                    _removeTimer(mRazuunTimer);
                    despawn(0, 0);
                    return;
                }
                break;
            }
        }
        ParentClass::AIUpdate();
    }

    uint32 mRazuunTimer;
    int32 mRazuunPhase;
};


class NeltharakusTale_Gossip : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* plr)
    {
        if (plr->HasQuest(10814))
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), 10613, plr->GetSession()->language);
            if (plr->HasQuest(10583))
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(471), 1);     // I am listening, Dragon

            menu.Send(plr);
        }
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* Code, uint32 gossipId)
    {
        switch (Id)
        {
            case 1:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 10614, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(472), 2);     // But you are Dragons! How could orcs do this to you?
                menu.Send(plr);
            } break;
            case 2:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 10615, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(473), 3);     // Your mate?
                menu.Send(plr);
            } break;
            case 3:
            {
                Arcemu::Gossip::Menu menu(pObject->GetGUID(), 10616, plr->GetSession()->language);
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(474), 4);     // I have battled many beasts, Dragon. I will help you.
                menu.Send(plr);
            } break;
            case 4:
            {
                plr->AddQuestKill(10814, 0, 0);
            } break;
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// Warbringer Razuun
const uint32 CN_ENSLAVED_NETHERWING_DRAKE = 21722;

class EnslavedNetherwingDrakeAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(EnslavedNetherwingDrakeAI, MoonScriptCreatureAI);
    EnslavedNetherwingDrakeAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        Movement::LocationWithFlag WayPoint = { getCreature()->GetPositionX(), getCreature()->GetPositionY() + 30, getCreature()->GetPositionZ() + 100, getCreature()->GetOrientation(), Movement::WP_MOVE_TYPE_FLY };
        setRooted(true);
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH | UNIT_FLAG_NOT_ATTACKABLE_2);
        AddWaypoint(CreateWaypoint(1, 0, WayPoint.wp_flag, WayPoint.wp_location));
    }

    void OnReachWP(uint32 iWaypointId, bool bForwards)
    {
        if (iWaypointId == 1)
        {
            despawn(0, 3 * 60 * 1000);
        }
    }
};


class KarynakuChains : public GameObjectAIScript
{
public:
    KarynakuChains(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new KarynakuChains(GO); }

    void OnActivate(Player* pPlayer)
    {
        QuestLogEntry* pQuest = pPlayer->GetQuestLogForEntry(10872);

        if (pQuest == NULL)
            return;

        pQuest->SetMobCount(0, pQuest->GetMobCount(0) + 1);
        pQuest->SendUpdateAddKill(0);
        pQuest->UpdatePlayerFields();
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// Flanis Swiftwing
class FlanisSwiftwing_Gossip : public Arcemu::Gossip::Script
{
public:
    void OnHello(Object* pObject, Player* Plr);
    void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* Code, uint32 gossipId);
};

void FlanisSwiftwing_Gossip::OnHello(Object* pObject, Player* plr)
{
    Arcemu::Gossip::Menu menu(pObject->GetGUID(), 40002, plr->GetSession()->language);
    if (plr->HasQuest(10583))
        menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(475), 1);     // Examine the corpse

    menu.Send(plr);
};

void FlanisSwiftwing_Gossip::OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* Code, uint32 gossipId)
{
    Creature* pCreature = static_cast<Creature*>(pObject);

    Item* item = objmgr.CreateItem(30658, Plr);
    if (item == nullptr)
        return;

    item->SetStackCount(1);
    if (!Plr->GetItemInterface()->AddItemToFreeSlot(item))
    {
        Plr->GetSession()->SendNotification("No free slots were found in your inventory!");
        item->DeleteMe();
    }
    else
    {
        Plr->SendItemPushResult(false, true, false, true, Plr->GetItemInterface()->LastSearchResult()->ContainerSlot,
            Plr->GetItemInterface()->LastSearchResult()->Slot, 1, item->GetEntry(), item->GetItemRandomSuffixFactor(),
            item->GetItemRandomPropertyId(), item->GetStackCount());
    }
};


void SetupShadowmoon(ScriptMgr* mgr)
{
    mgr->register_creature_script(11980, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(21718, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(21719, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(21720, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(22253, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(22274, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(22331, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(23188, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(21717, &InfiltratingDragonmawFortressQAI::Create);
    mgr->register_creature_script(21878, &KneepadsQAI::Create);
    mgr->register_creature_script(21879, &KneepadsQAI::Create);
    mgr->register_creature_script(21864, &KneepadsQAI::Create);
    mgr->register_creature_script(CN_DEATHBRINGER_JOVAAN, &DeathbringerJovaanAI::Create);
    mgr->register_creature_script(CN_WARBRINGER_RAZUUN, &WarbringerRazuunAI::Create);
    mgr->register_creature_script(CN_ENSLAVED_NETHERWING_DRAKE, &EnslavedNetherwingDrakeAI::Create);

    mgr->register_gameobject_script(185156, &KarynakuChains::Create);

    Arcemu::Gossip::Script* NeltharakusTaleGossip = new NeltharakusTale_Gossip();
    mgr->register_creature_gossip(21657, NeltharakusTaleGossip);

    Arcemu::Gossip::Script* FlanisSwiftwingGossip = new FlanisSwiftwing_Gossip();
    mgr->register_creature_gossip(21727, FlanisSwiftwingGossip); //Add Flanis' Pack
}
