/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Instance_CullingOfStratholme.h"

class CullingOfStratholmeInstanceScript : public InstanceScript
{
public:

    explicit CullingOfStratholmeInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new CullingOfStratholmeInstanceScript(pMapMgr); }


};

class MeathookAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MeathookAI)
    explicit MeathookAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
        {
            // ConstrictingChains
            addAISpell(58823, 100.0f, TARGET_RANDOM_SINGLE, 0, 8);
            // DiseaseExpulsion
           addAISpell(58824, 50.0f, TARGET_RANDOM_SINGLE, 0, 5);
            // Frenzy
            addAISpell(58841, 20.0f, TARGET_SELF, 0, 1);
        }
        else
        {
            // ConstrictingChains
            addAISpell(52696, 100.0f, TARGET_RANDOM_SINGLE, 0, 8);
            // DiseaseExpulsion
            addAISpell(52666, 50.0f, TARGET_RANDOM_SINGLE, 0, 5);
            // Frenzy
            addAISpell(58841, 20.0f, TARGET_SELF, 0, 1);
        }

        // new
        addEmoteForEvent(Event_OnCombatStart, SAY_MEATHOOK_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_MEATHOOK_02);
        addEmoteForEvent(Event_OnTargetDied, SAY_MEATHOOK_03);
        addEmoteForEvent(Event_OnTargetDied, SAY_MEATHOOK_04);
        addEmoteForEvent(Event_OnDied, SAY_MEATHOOK_06);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->RemoveAllAuras();
    }
};

class SalramTheFleshcrafterAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SalramTheFleshcrafterAI)
    explicit SalramTheFleshcrafterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
        {
            // shadowBolt
            addAISpell(58827, 50.0f, TARGET_RANDOM_SINGLE, 0, 4);
            // curseOfTwistedFlesh
            addAISpell(58845, 50.0f, TARGET_RANDOM_SINGLE, 0, 4);
            // stealFlesh
            addAISpell(52708, 30.0f, TARGET_RANDOM_SINGLE, 0, 5);
            // summonGhouls
            addAISpell(52451, 50.0f, TARGET_SELF, 0, 4);

            auto explodeGhoul = addAISpell(58825, 80.0f, TARGET_CUSTOM, 0, 6);
            if (explodeGhoul)
                explodeGhoul->setCustomTarget(getNearestCreature(27733));
        }
        else
        {
            // shadowBolt
            addAISpell(57725, 50.0f, TARGET_RANDOM_SINGLE, 0, 4);
            // stealFlesh
            addAISpell(52708, 30.0f, TARGET_RANDOM_SINGLE, 0, 5);
            // summonGhouls
            addAISpell(52451, 50.0f, TARGET_SELF, 0, 4);
            auto explodeGhoul = addAISpell(52480, 80.0f, TARGET_CUSTOM, 0, 6);
            if (explodeGhoul)
                explodeGhoul->setCustomTarget(getNearestCreature(27733));
        }

        // new
        addEmoteForEvent(Event_OnCombatStart, SAY_SALRAM_FLESH_01);
        addEmoteForEvent(Event_OnTargetDied, SAY_SALRAM_FLESH_03);
        addEmoteForEvent(Event_OnTargetDied, SAY_SALRAM_FLESH_04);
        addEmoteForEvent(Event_OnTargetDied, SAY_SALRAM_FLESH_05);
        addEmoteForEvent(Event_OnDied, SAY_SALRAM_FLESH_06);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->RemoveAllAuras();
    }
};

class ChronoLordEpochAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ChronoLordEpochAI)
    explicit ChronoLordEpochAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
        {
            // WoundingStrike
            addAISpell(58830, 50.0f, TARGET_ATTACKING, 0, 3);
            // TimeStop
            addAISpell(58848, 20.0f, TARGET_ATTACKING, 0, 2);
        }
        else
        {
            // WoundingStrike
            addAISpell(52771, 50.0f, TARGET_ATTACKING, 0, 3);
        }

        // CurseOfExertion
        addAISpell(52772, 100.0f, TARGET_RANDOM_SINGLE, 0, 5);
        // TimeWarp
        addAISpell(52766, 50.0f, TARGET_ATTACKING, 0, 6);


        addEmoteForEvent(Event_OnCombatStart, SAY_CHRONOLORD_EPOCH_02);
        addEmoteForEvent(Event_OnTargetDied, SAY_CHRONOLORD_EPOCH_06);
        addEmoteForEvent(Event_OnTargetDied, SAY_CHRONOLORD_EPOCH_07);
        addEmoteForEvent(Event_OnTargetDied, SAY_CHRONOLORD_EPOCH_08);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->RemoveAllAuras();
    }
};

class InfiniteCorruptorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(InfiniteCorruptorAI)
    explicit InfiniteCorruptorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        // VoidStrike
        addAISpell(60590, 50.0f, TARGET_ATTACKING, 0, 1);
        // CorruptingBlight
        addAISpell(60588, 50.0f, TARGET_RANDOM_SINGLE, 0, 3);

        
        addEmoteForEvent(Event_OnCombatStart, SAY_INFINITE_CORRUP_01);
        addEmoteForEvent(Event_OnDied, SAY_INFINITE_CORRUP_02);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->RemoveAllAuras();
    }
};


class MalganisAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MalganisAI)
    explicit MalganisAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
        {
            // CarrionSwarm
            addAISpell(58852, 60.0f, TARGET_ATTACKING, 0, 10);
            // MindBlast
            addAISpell(58850, 50.0f, TARGET_RANDOM_SINGLE, 0, 5);
            // Sleep
            addAISpell(58849, 40.0f, TARGET_RANDOM_SINGLE, 0, 7);
        }
        else
        {
            // CarrionSwarm
            addAISpell(52720, 60.0f, TARGET_ATTACKING, 0, 10);
            // MindBlast
            addAISpell(52722, 50.0f, TARGET_RANDOM_SINGLE, 0, 5);
            // Sleep
            addAISpell(52721, 40.0f, TARGET_RANDOM_SINGLE, 0, 7);
        }

        // VampiricTouch
        addAISpell(52723, 90.0f, TARGET_SELF, 0, 30);


        addEmoteForEvent(Event_OnCombatStart, SAY_MALGANIS_03);
        addEmoteForEvent(Event_OnTargetDied, SAY_MALGANIS_04);
        addEmoteForEvent(Event_OnTargetDied, SAY_MALGANIS_05);
        addEmoteForEvent(Event_OnTargetDied, SAY_MALGANIS_06);
        addEmoteForEvent(Event_OnDied, SAY_MALGANIS_16);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        getCreature()->RemoveAllAuras();
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t fAmount) override
    {
        if (getCreature()->HasAura(52723))    //handling a dummy :)
        {
            getCreature()->addSimpleHealingBatchEvent(fAmount / 2, getCreature(), sSpellMgr.getSpellInfo(52723));
        }

        if (getCreature()->getHealthPct() < 2)
        {
            getCreature()->setMoveRoot(true);
            getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
            for (uint8_t i = 0; i < 7; ++i)
                getCreature()->SchoolImmunityList[i] = 1;

            RemoveAIUpdateEvent();
            sendDBChatMessage(SAY_MALGANIS_17);

            //spawn a chest and go
            GameObject* go = getCreature()->GetMapMgr()->CreateGameObject(190663);
            go->CreateFromProto(190663, getCreature()->GetMapMgr()->GetMapId(), getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f);
            go->PushToWorld(getCreature()->GetMapMgr());
            getCreature()->Despawn(1, 0);
        }
    }
};


class Quest_Dispelling_Illusions : public QuestScript
{
public:

    void OnQuestStart(Player* mTarget, QuestLogEntry* /*qLogEntry*/) override
    {
        for (uint8_t i = 0; i < 5; i++)
        SpawnCrates(i, mTarget->GetMapMgr());
    }

    static void SpawnCrates(uint32_t id, MapMgr* pMapMgr)
    {
        uint32_t entry = 190094;
        float x = 0.0f, y = 0.0f, z = 0.0f, o = 0.0f;
        switch (id)
        {
            case 0:
            {
                x = 1570.92f;
                y = 669.933f;
                z = 102.309f;
                o = -1.64061f;
            }
            break;
            case 1:
            {
                x = 1579.42f;
                y = 621.446f;
                z = 99.7329f;
                o = 2.9845f;
            }
            break;
            case 2:
            {
                x = 1629.68f;
                y = 731.367f;
                z = 112.847f;
                o = -0.837757f;
            }
            break;
            case 3:
            {
                x = 1674.39f;
                y = 872.307f;
                z = 120.394f;
                o = -1.11701f;
            }
            break;
            case 4:
            {
                x = 1628.98f;
                y = 812.142f;
                z = 120.689f;
                o = 0.436332f;
            }
            break;
        }
        GameObject* crate = pMapMgr->GetInterface()->GetGameObjectNearestCoords(x, y, z, 190094);
        if (crate)
            crate->Despawn(0, 0);

        GameObject* go = pMapMgr->CreateGameObject(entry);
        go->CreateFromProto(entry, pMapMgr->GetMapId(), x, y, z, o, 0.0f, 0.0f, 0.0f, 0.0f);
        go->PushToWorld(pMapMgr);
    }
};


static Movement::Location walk[] =
{
    { 0, 0, 0, 0 },
    { 1811.2177f, 1276.5729f, 141.9048f, 0.098f },
    { 1884.9484f, 1284.9110f, 143.7776f, 6.2810f },
    { 1897.4763f, 1291.1870f, 143.5821f, 1.4194f }
};

class UtherAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(UtherAI)
    explicit UtherAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddWaypoint(CreateWaypoint(1, 0, Movement::WP_MOVE_TYPE_RUN, walk[1]));
        AddWaypoint(CreateWaypoint(2, 0, Movement::WP_MOVE_TYPE_RUN, walk[2]));
        AddWaypoint(CreateWaypoint(3, 90000, Movement::WP_MOVE_TYPE_RUN, walk[3]));
        check = true;
    }

    void OnReachWP(uint32_t i, bool /*usl*/) override
    {
        if (i == 3 && check)
        {
            check = false;
            Creature* Arthas = getNearestCreature(CN_ARTHAS);
            Creature* Jaina = getNearestCreature(CN_JAINA);
            if (Arthas && Jaina)  //Show must go on!
            {
                //we add 0,5s per speech
                //1 = 0s
                Arthas->SendScriptTextChatMessage(SAY_ARTHAS_01);
                //2 = 2,5s
                getCreature()->SendTimedScriptTextChatMessage(SAY_UTHER_01, 2500);
                //3 = 9s
                Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_02, 9000);
                //4 = 14,5s
                Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_03, 14500);
                //5 = 25s
                getCreature()->SendTimedScriptTextChatMessage(SAY_UTHER_02, 25000);
                //6 = 26,5s
                Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_04, 26500);
                //7 = 29s
                getCreature()->SendTimedScriptTextChatMessage(SAY_UTHER_03, 29000);
                //8 = 33,5s
                Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_05, 33500);
                //9 = 38s
                getCreature()->SendTimedScriptTextChatMessage(SAY_UTHER_04, 38000);
                //10 = 44,5s
                Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_06, 44500);
                //11 = 49s
                getCreature()->SendTimedScriptTextChatMessage(SAY_UTHER_05, 49000);
                //12 = 53,5s
                Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_07, 53500);
                //13 = 65s
                Jaina->SendTimedScriptTextChatMessage(SAY_JAINA_01, 65000);
                //14 = 67,5s
                Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_08, 67500);
                //15 = 77s
                //here few knights should leave, after speech, Uther should leave also
                getCreature()->SendTimedScriptTextChatMessage(SAY_UTHER_06, 77000);
                //16 = 80,5s
                //Jaina begins leaving
                Arthas->SendTimedScriptTextChatMessage(SAY_ARTHAS_09, 80500);
                //17 = 82s
                Jaina->SendTimedScriptTextChatMessage(SAY_JAINA_02, 82000);
                //trigger Arthas actions = 86,5s
                getCreature()->Despawn(100000, 0);
            }
        }
    }

private:

    bool check;
};

static Movement::Location ArthasWalk[] =
{
    { 0, 0, 0, 0 },
    { 1908.9722f, 1312.8898f, 149.9889f, 0.6858f },
    { 1884.9483f, 1284.9110f, 143.7776f, -2.2802f },
    { 1991.4326f, 1286.5925f, 145.4636f, 1.2591f },
    { 2007.2526f, 1327.5848f, 142.9831f, 5.5553f },
    { 2024.4555f, 1308.2036f, 143.4564f, 4.8492f },
    { 2028.9012f, 1285.9205f, 143.6552f, 0.0661f },
    { 2078.9479f, 1287.9812f, 141.4830f, 0.0308f }
};


class ArthasAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ArthasAI)
    explicit ArthasAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        AddWaypoint(CreateWaypoint(1, 10500, Movement::WP_MOVE_TYPE_RUN, ArthasWalk[1]));
        AddWaypoint(CreateWaypoint(2, 0, Movement::WP_MOVE_TYPE_RUN, ArthasWalk[2]));
        AddWaypoint(CreateWaypoint(3, 0, Movement::WP_MOVE_TYPE_RUN, ArthasWalk[3]));
        AddWaypoint(CreateWaypoint(4, 0, Movement::WP_MOVE_TYPE_RUN, ArthasWalk[4]));
        AddWaypoint(CreateWaypoint(5, 0, Movement::WP_MOVE_TYPE_RUN, ArthasWalk[5]));
        AddWaypoint(CreateWaypoint(6, 0, Movement::WP_MOVE_TYPE_RUN, ArthasWalk[6]));
        AddWaypoint(CreateWaypoint(7, 0, Movement::WP_MOVE_TYPE_RUN, ArthasWalk[7]));

        setAIAgent(AGENT_NULL);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
        phase = 0;
    }

    void OnReachWP(uint32_t i, bool /*usl*/) override
    {
        switch (i)
        {
            case 1:
            {
                sendDBChatMessage(SAY_ARTHAS_10);
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(2);
            }
            break;
            case 7:
            {
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTIDLE);
                getCreature()->GetAIInterface()->m_canMove = false;
                getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
            }
            break;
            case 1000://haxxed ;)
            {
                sendDBChatMessage(SAY_ARTHAS_11);
                phase++;
            }
            break;
            default:
            {
                if (i > 1 && i < 7)
                {
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(i + 1);
                }
            }
            break;
        }
    }

    void AIUpdate() override
    {
        switch (phase)
        {
            case 0:
            {
                getCreature()->GetAIInterface()->StopMovement(0);
                getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
                getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                getCreature()->GetAIInterface()->setWayPointToMove(1);
            }
            break;
            case 1:
            {
                getCreature()->SendTimedScriptTextChatMessage(SAY_ARTHAS_12, 300);
                Creature* citizen = getNearestCreature(28167);
                if (citizen)
                {
                    getCreature()->GetAIInterface()->MoveTo(citizen->GetPositionX(), citizen->GetPositionY(), citizen->GetPositionZ());
                    getCreature()->dealDamage(citizen, citizen->getHealth(), 0);
                }
                citizen = getNearestCreature(28169);
                if (citizen)
                {
                    getCreature()->GetAIInterface()->MoveTo(citizen->GetPositionX(), citizen->GetPositionY(), citizen->GetPositionZ());
                    getCreature()->dealDamage(citizen, citizen->getHealth(), 0);
                }
                getCreature()->SendTimedScriptTextChatMessage(SAY_ARTHAS_13, 1000);
                phase++;
            }
            break;
            case 2:
            {
                //we need that tricky animation here
                //spawn Mal'Ganis
                CreatureProperties const* cp = sMySQLStore.getCreatureProperties(26533);
                Creature* c = nullptr;
                if (cp)
                {
                    c = getCreature()->GetMapMgr()->CreateCreature(26533);
                    if (c)
                    {
                        //position is guessed
                        c->Load(cp, 2113.52f, 1288.01f, 136.382f, 2.30383f);
                        c->PushToWorld(getCreature()->GetMapMgr());
                    }
                }
                if (c)
                {
                    c->bInvincible = true;
                    c->GetAIInterface()->m_canMove = false;
                    c->GetAIInterface()->SetAllowedToEnterCombat(false);
                    for (uint8_t i = 0; i < 7; i++)
                        c->SchoolImmunityList[i] = 1;
                    c->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                    //1 = 0s
                    c->SendScriptTextChatMessage(SAY_MALGANIS_01);
                    //2 = 13s
                    //change all citizens to undeads...
                    c->SendTimedScriptTextChatMessage(SAY_MALGANIS_02, 13000);
                    //2 = 32s
                    getCreature()->SendTimedScriptTextChatMessage(SAY_ARTHAS_14, 32000);
                    c->Despawn(38500, 0);
                    //3 = 37s
                }
            }
            break;
        }
    }

protected:

    uint32_t phase;
};


class ArthasGossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* Plr) override
    {
        GossipMenu menu(pObject->getGuid(), 1, 0);

        menu.addItem(0, 0, 1, "We're ready to go!");  //find correct txt

        menu.sendGossipPacket(Plr);
    }

    void onSelectOption(Object* pObject, Player* Plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        switch (Id)
        {
            case 1:
            {
                static_cast<Creature*>(pObject)->setNpcFlags(UNIT_NPC_FLAG_NONE);
                static_cast<Creature*>(pObject)->GetScript()->OnReachWP(1000, 0);
            }
            break;
        }
        GossipMenu::senGossipComplete(Plr);
    }
};

// \todo check the unregistered scripts (broeken? outdated?)
void SetupCullingOfStratholme(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_GNOMEREGAN, &CullingOfStratholmeInstanceScript::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // TRASH MOBS

    //////////////////////////////////////////////////////////////////////////////////////////
    // BOSSES

    //mgr->register_creature_script(CN_MEATHOOK, &MeathookAI::Create);
    //mgr->register_creature_script(CN_SALRAMM_THE_FLESHCRAFTER, &SalramTheFleshcrafterAI::Create);
    //mgr->register_creature_script(CN_CHRONO_LORD_EPOCH, &ChronoLordEpochAI::Create);
    //mgr->register_creature_script(CN_INFINITE_CORRUPTOR, &InfiniteCorruptorAI::Create);
    //mgr->register_creature_script(CN_MALGANIS, &MalganisAI::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // QUESTS & STUFF

    //UPDATE `quests` SET `ReqKillMobOrGOCount1`='5' WHERE (`entry`='13149');
    QuestScript* Dispelling_Illusions = new Quest_Dispelling_Illusions();
    mgr->register_quest_script(13149, Dispelling_Illusions);
    //mgr->register_creature_script(CN_UTHER, &UtherAI::Create);
    //mgr->register_creature_script(CN_ARTHAS, &ArthasAI::Create);
    GossipScript* gs = new ArthasGossip();
    mgr->register_creature_gossip(CN_ARTHAS, gs);
}
