/*
 Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 This file is released under the MIT license. See README-MIT for more information.
 */

#include "Setup.h"

// Black Cat
class BlackCat : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BlackCat(c); }
    BlackCat(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* pKiller)
    {
        pKiller->CastSpell(pKiller, 39477, true);
    }
};

// HEADLESS HORSEMAN ENCOUNTER
static Movement::Location WaypointGoldshire[] =
{
    { -9502.733398f, 31.395960f, 60.433193f, 1.217366f }, // 0
    { -9493.925781f, 55.272415f, 60.433193f, 0.781469f },
    { -9483.589844f, 63.685684f, 60.433193f, 6.224273f },
    { -9463.258789f, 62.515587f, 60.433193f, 6.204639f },
    { -9457.368164f, 48.343132f, 66.931587f, 4.641701f },
    { -9458.772461f, 27.414370f, 77.199722f, 4.001603f },
    { -9473.457031f, 29.496262f, 77.199722f, 1.394081f },
    { -9471.234275f, 44.239151f, 75.393852f, 1.241714f },
    { -9459.474609f, 81.118446f, 71.725540f, 1.720021f },
    { -9467.220703f, 88.311104f, 71.786453f, 2.572178f }, // 9
    { -9486.188477f, 83.939690f, 82.718826f, 3.569634f }, // 10 Starting round (3 rounds left)
    { -9506.228516f, 36.876194f, 89.180916f, 6.167746f },
    { -9437.569396f, 34.403599f, 75.426025f, 1.270783f },
    { -9448.488281f, 85.930862f, 75.290497f, 2.909909f },
    { -9477.427734f, 86.952667f, 70.950249f, 3.318317f }, // 14
    { -9486.188477f, 83.939690f, 82.718826f, 3.569634f }, // 15 Next round (2 rounds left)
    { -9506.228516f, 36.876194f, 89.180916f, 6.167746f },
    { -9437.569396f, 34.403599f, 75.426025f, 1.270783f },
    { -9448.488281f, 85.930862f, 75.290497f, 2.909909f },
    { -9477.427734f, 86.952667f, 70.950249f, 3.318317f }, // 19
    { -9486.188477f, 83.939690f, 82.718826f, 3.569634f }, // 20 Next round (1 rounds left)
    { -9506.228516f, 36.876194f, 89.180916f, 6.167746f },
    { -9437.569396f, 34.403599f, 75.426025f, 1.270783f },
    { -9448.488281f, 85.930862f, 75.290497f, 2.909909f },
    { -9477.427734f, 86.952667f, 70.950249f, 3.318317f }, // 24
    { -9486.188477f, 83.939690f, 82.718826f, 3.569634f }, // 25 Next round (0 rounds left)
    { -9506.228516f, 36.876194f, 89.180916f, 6.167746f },
    { -9437.569396f, 34.403599f, 75.426025f, 1.270783f },
    { -9448.488281f, 85.930862f, 75.290497f, 2.909909f },
    { -9477.427734f, 86.952667f, 70.950249f, 3.318317f }  // 29
};

// Headless HorsemanAI
const uint32 CN_HEADLESS_HORSEMAN = 23682;
const uint32 HEADLESS_HORSEMAN_CLEAVE = 42587;
const uint32 HEADLESS_HORSEMAN_CONFLAGRATION = 42380;
class HeadlessHorsemanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HeadlessHorsemanAI(c); }
    HeadlessHorsemanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //Scarlet Monastery Boss
    }
};


// Headless Horseman - Fire
const uint32 CN_HEADLESS_HORSEMAN_FIRE = 23537;
class HeadlessHorsemanFireAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HeadlessHorsemanFireAI(c); }
    HeadlessHorsemanFireAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->CastSpell(getCreature(), 42971, true);
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
class ShadeOfTheHorsemanAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ShadeOfTheHorsemanAI(c); }
    ShadeOfTheHorsemanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        setCanEnterCombat(false);
        getCreature()->SetMount(22653);
        ///Spells
        mSummon = AddSpell(SHADE_OF_THE_HORSEMAN_SUMMON, Target_Self, 0, 0, 0);

        //Emotes
        addEmoteForEvent(Event_OnDied, 8802);
        sendDBChatMessage(8803);    //On Spawn?

        WPCount = 0;

        auto area = getCreature()->GetArea();
        if (area != nullptr)
        {
            switch (area->id)
            {
                case 87: // Goldshire
                {
                    WPCount = 29;
                    for (uint8 i = 0; i <= WPCount; ++i)
                    {
                        AddWaypoint(CreateWaypoint(i, 0, Movement::WP_MOVE_TYPE_FLY, WaypointGoldshire[i]));
                    }
                } break;
                default:
                    break;
            }
        }
    }

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
    {
        auto area = getCreature()->GetArea();
        auto area_id = area ? area->id : 0;

        if (iWaypointId == uint32(WPCount))   // Reached end
        {
            StopWaypointMovement();
            if (getNearestCreature(CN_HEADLESS_HORSEMAN_FIRE) == NULL)     // CASE players win
            {
                sendDBChatMessage(8804);
                despawn(30000, 0); //Despawn after 30 secs
            }
            else // CASE players lost
            {
                sendDBChatMessage(8805);
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
                        getCreature()->CastSpell(getCreature(), 42118, true);
                    }
                } break;
                default:
                    break;
            }
        }
    }

    void OnDied(Unit* pKiller)
    {
        GameObject* Pumpkin = pKiller->GetMapMgr()->CreateAndSpawnGameObject(2883, getCreature()->GetPositionX() + RandomFloat(5.0f), getCreature()->GetPositionY() + RandomFloat(5.0f), getCreature()->GetPositionZ(), 0, 1);
        if (Pumpkin != nullptr)
            getCreature()->CastSpell(Pumpkin->GetGUID(), 42277, true);
    }

    int8 WPCount;
    SpellDesc* mSummon;
};


// Headless Horseman - Wisp Invis
const uint32 CN_HEADLESS_HORSEMAN_WISP_INVIS = 24034;    // 42394
class HeadlessHorsemanWispInvisAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new HeadlessHorsemanWispInvisAI(c); }
    HeadlessHorsemanWispInvisAI(Creature* pCreature) : CreatureAIScript(pCreature)
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
                spawnCreature(CN_SHADE_OF_THE_HORSEMAN, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
                SetAIUpdateFreq(4 * 60 * 1000);
            }
        }
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
