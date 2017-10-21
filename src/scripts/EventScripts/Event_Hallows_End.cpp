/*
 Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 This file is released under the MIT license. See README-MIT for more information.
 */

#include "Setup.h"

// Black Cat
class BlackCat : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(BlackCat, MoonScriptCreatureAI);
    BlackCat(Creature* pCreature) : MoonScriptCreatureAI(pCreature) {}

    void OnDied(Unit* pKiller)
    {
        pKiller->CastSpell(pKiller, 39477, true);
        ParentClass::OnDied(pKiller);
    }
};

// HEADLESS HORSEMAN ENCOUNTER
static Movement::LocationWithFlag WaypointGoldshire[] =
{
    { -9502.733398f, 31.395960f, 60.433193f, 1.217366f, Movement::WP_MOVE_TYPE_FLY }, // 0
    { -9493.925781f, 55.272415f, 60.433193f, 0.781469f, Movement::WP_MOVE_TYPE_FLY },
    { -9483.589844f, 63.685684f, 60.433193f, 6.224273f, Movement::WP_MOVE_TYPE_FLY },
    { -9463.258789f, 62.515587f, 60.433193f, 6.204639f, Movement::WP_MOVE_TYPE_FLY },
    { -9457.368164f, 48.343132f, 66.931587f, 4.641701f, Movement::WP_MOVE_TYPE_FLY },
    { -9458.772461f, 27.414370f, 77.199722f, 4.001603f, Movement::WP_MOVE_TYPE_FLY },
    { -9473.457031f, 29.496262f, 77.199722f, 1.394081f, Movement::WP_MOVE_TYPE_FLY },
    { -9471.234275f, 44.239151f, 75.393852f, 1.241714f, Movement::WP_MOVE_TYPE_FLY },
    { -9459.474609f, 81.118446f, 71.725540f, 1.720021f, Movement::WP_MOVE_TYPE_FLY },
    { -9467.220703f, 88.311104f, 71.786453f, 2.572178f, Movement::WP_MOVE_TYPE_FLY }, // 9
    { -9486.188477f, 83.939690f, 82.718826f, 3.569634f, Movement::WP_MOVE_TYPE_FLY }, // 10 Starting round (3 rounds left)
    { -9506.228516f, 36.876194f, 89.180916f, 6.167746f, Movement::WP_MOVE_TYPE_FLY },
    { -9437.569396f, 34.403599f, 75.426025f, 1.270783f, Movement::WP_MOVE_TYPE_FLY },
    { -9448.488281f, 85.930862f, 75.290497f, 2.909909f, Movement::WP_MOVE_TYPE_FLY },
    { -9477.427734f, 86.952667f, 70.950249f, 3.318317f, Movement::WP_MOVE_TYPE_FLY }, // 14
    { -9486.188477f, 83.939690f, 82.718826f, 3.569634f, Movement::WP_MOVE_TYPE_FLY }, // 15 Next round (2 rounds left)
    { -9506.228516f, 36.876194f, 89.180916f, 6.167746f, Movement::WP_MOVE_TYPE_FLY },
    { -9437.569396f, 34.403599f, 75.426025f, 1.270783f, Movement::WP_MOVE_TYPE_FLY },
    { -9448.488281f, 85.930862f, 75.290497f, 2.909909f, Movement::WP_MOVE_TYPE_FLY },
    { -9477.427734f, 86.952667f, 70.950249f, 3.318317f, Movement::WP_MOVE_TYPE_FLY }, // 19
    { -9486.188477f, 83.939690f, 82.718826f, 3.569634f, Movement::WP_MOVE_TYPE_FLY }, // 20 Next round (1 rounds left)
    { -9506.228516f, 36.876194f, 89.180916f, 6.167746f, Movement::WP_MOVE_TYPE_FLY },
    { -9437.569396f, 34.403599f, 75.426025f, 1.270783f, Movement::WP_MOVE_TYPE_FLY },
    { -9448.488281f, 85.930862f, 75.290497f, 2.909909f, Movement::WP_MOVE_TYPE_FLY },
    { -9477.427734f, 86.952667f, 70.950249f, 3.318317f, Movement::WP_MOVE_TYPE_FLY }, // 24
    { -9486.188477f, 83.939690f, 82.718826f, 3.569634f, Movement::WP_MOVE_TYPE_FLY }, // 25 Next round (0 rounds left)
    { -9506.228516f, 36.876194f, 89.180916f, 6.167746f, Movement::WP_MOVE_TYPE_FLY },
    { -9437.569396f, 34.403599f, 75.426025f, 1.270783f, Movement::WP_MOVE_TYPE_FLY },
    { -9448.488281f, 85.930862f, 75.290497f, 2.909909f, Movement::WP_MOVE_TYPE_FLY },
    { -9477.427734f, 86.952667f, 70.950249f, 3.318317f, Movement::WP_MOVE_TYPE_FLY }  // 29
};

// Headless HorsemanAI
const uint32 CN_HEADLESS_HORSEMAN = 23682;
const uint32 HEADLESS_HORSEMAN_CLEAVE = 42587;
const uint32 HEADLESS_HORSEMAN_CONFLAGRATION = 42380;
class HeadlessHorsemanAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(HeadlessHorsemanAI, MoonScriptCreatureAI);
    HeadlessHorsemanAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        //Scarlet Monastery Boss
    }
};


// Headless Horseman - Fire
const uint32 CN_HEADLESS_HORSEMAN_FIRE = 23537;
class HeadlessHorsemanFireAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(HeadlessHorsemanFireAI, MoonScriptCreatureAI);
    HeadlessHorsemanFireAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        _unit->CastSpell(_unit, 42971, true);
    }
};


// Shade of the HorsemanAI
const uint32 CN_SHADE_OF_THE_HORSEMAN = 23543;
const uint32 SHADE_OF_THE_HORSEMAN_SUMMON = 42394;  //Don't think this one is the correct spell
/*
* Research
* NPC:
* http://www.wowhead.com/?npc=23537 Headless Horseman - Fire (DND) should be invisible? created by ->
* http://www.wowhead.com/?spell=42118
* I guess this is the target of the water spells
* \todo Need to check all visual auras for these http://www.wowhead.com/?search=horseman#uncategorized-spells
*/
class ShadeOfTheHorsemanAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(ShadeOfTheHorsemanAI, MoonScriptCreatureAI);
    ShadeOfTheHorsemanAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        setCanEnterCombat(false);
        _unit->SetMount(22653);
        ///Spells
        mSummon = AddSpell(SHADE_OF_THE_HORSEMAN_SUMMON, Target_Self, 0, 0, 0);

        //Emotes
        AddEmote(Event_OnDied, "So eager you are, for my blood to spill. Yet to vanquish me, 'tis my head you must kill!", Text_Yell, 11969);
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 11966, "Prepare yourselves, the bells have tolled! Shelter your weak, your young, and your old! Each of you shall pay the final sum. Cry for mercy, the reckoning has come!");    //On Spawn?

        WPCount = 0;
        WayPoints = nullptr;

        auto area = _unit->GetArea();
        if (area != nullptr)
        {
            switch (area->id)
            {
                case 87: // Goldshire
                {
                    WPCount = 29;
                    WayPoints = WaypointGoldshire;
                    for (uint8 i = 0; i <= WPCount; ++i)
                    {
                        AddWaypoint(CreateWaypoint(i, 0, WayPoints[i]));
                    }
                } break;
                default:
                    break;
            }
        }
    }

    void OnReachWP(uint32 iWaypointId, bool bForwards)
    {
        auto area = _unit->GetArea();
        auto area_id = area ? area->id : 0;

        if (iWaypointId == uint32(WPCount))   // Reached end
        {
            StopWaypointMovement();
            if (getNearestCreature(CN_HEADLESS_HORSEMAN_FIRE) == NULL)     // CASE players win
            {
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 11968, "My flames have died, left not a spark! I shall send you now to the lifeless dark!");
                despawn(30000, 0); //Despawn after 30 secs
            }
            else // CASE players lost
            {
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 11967, "Fire consumes! You've tried and failed. Let there be no doubt, justice prevailed!");
                despawn(12000, 0); //Despawn after 12 secs
            }
        }
        else
        {
            switch (area_id)
            {
                case 87: // Goldshire
                {
                    if (iWaypointId == 6)
                    {
                        _unit->CastSpell(_unit, 42118, true);
                    }
                } break;
                default:
                    break;
            }
        }
        ParentClass::OnReachWP(iWaypointId, bForwards);
    }

    void OnDied(Unit* pKiller)
    {
        GameObject* Pumpkin = pKiller->GetMapMgr()->CreateAndSpawnGameObject(2883, _unit->GetPositionX() + RandomFloat(5.0f), _unit->GetPositionY() + RandomFloat(5.0f), _unit->GetPositionZ(), 0, 1);
        if (Pumpkin != nullptr)
            _unit->CastSpell(Pumpkin->GetGUID(), 42277, true);

        ParentClass::OnDied(pKiller);
    }

    int8 WPCount;
    Movement::LocationWithFlag* WayPoints;
    SpellDesc* mSummon;
};


// Headless Horseman - Wisp Invis
const uint32 CN_HEADLESS_HORSEMAN_WISP_INVIS = 24034;    // 42394
class HeadlessHorsemanWispInvisAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(HeadlessHorsemanWispInvisAI, MoonScriptCreatureAI);
    HeadlessHorsemanWispInvisAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        mHeadlessHorseman = nullptr;
    }

    void AIUpdate()
    {
        time_t tiempo;
        struct tm* tmPtr;
        tiempo = UNIXTIME;
        tmPtr = localtime(&tiempo);
        if (tmPtr->tm_min == 0 && (tmPtr->tm_hour % 4) == 0)   // All check for the time
        {
            mHeadlessHorseman = getNearestCreature(CN_SHADE_OF_THE_HORSEMAN);
            if (mHeadlessHorseman == nullptr)
            {
                SpawnCreature(CN_SHADE_OF_THE_HORSEMAN, _unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), _unit->GetOrientation());
                SetAIUpdateFreq(4 * 60 * 1000);
            }
        }
        ParentClass::AIUpdate();
    }

    Creature* mHeadlessHorseman;
};


class WaterBarrel : public GameObjectAIScript
{
    public:

        WaterBarrel(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
        static GameObjectAIScript* Create(GameObject* GO) { return new WaterBarrel(GO); }

        void OnActivate(Player* pPlayer)
        {
            SlotResult slotresult;
            ItemProperties const* proto = sMySQLStore.getItemProperties(32971);
            if (!proto)
                return;

            slotresult = pPlayer->GetItemInterface()->FindFreeInventorySlot(proto);

            if (!slotresult.Result)
            {
                pPlayer->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                return;
            }
            else
            {
                if (pPlayer->GetItemInterface()->GetItemCount(32971, false) == 0)
                {
                    auto itm = objmgr.CreateItem(32971, pPlayer);
                    if (itm == nullptr)
                        return;

                    auto result = pPlayer->GetItemInterface()->SafeAddItem(itm, slotresult.ContainerSlot, slotresult.Slot);
                    if (!result)
                    {
                        LOG_ERROR("Error while adding item %u to player %s", itm->GetEntry(), pPlayer->GetNameString());
                        itm->DeleteMe();
                    }
                }
                else
                {
                    pPlayer->GetItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
                    return;
                }
            }
        }
};

void SetupHallowsEnd(ScriptMgr* mgr)
{
    mgr->register_creature_script(22816, &BlackCat::Create);
    mgr->register_creature_script(CN_HEADLESS_HORSEMAN, &HeadlessHorsemanAI::Create);
    mgr->register_creature_script(CN_HEADLESS_HORSEMAN_WISP_INVIS, &HeadlessHorsemanWispInvisAI::Create);
    mgr->register_creature_script(CN_HEADLESS_HORSEMAN_FIRE, &HeadlessHorsemanFireAI::Create);
    mgr->register_creature_script(CN_SHADE_OF_THE_HORSEMAN, &ShadeOfTheHorsemanAI::Create);

    mgr->register_gameobject_script(186234, &WaterBarrel::Create);
}
