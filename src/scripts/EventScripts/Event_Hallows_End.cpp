/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

enum
{
    // Headless HorsemanAI
    CN_HEADLESS_HORSEMAN = 23682,
    //HEADLESS_HORSEMAN_CLEAVE = 42587,
    //HEADLESS_HORSEMAN_CONFLAGRATION = 42380,

    // Shade of the HorsemanAI
    CN_SHADE_OF_THE_HORSEMAN = 23543,
    //SHADE_OF_THE_HORSEMAN_SUMMON = 42394,  // Don't think this one is the correct spell

    // Headless Horseman - Wisp Invis
    CN_HEADLESS_HORSEMAN_WISP_INVIS = 24034, // 42394
};

// Black Cat
class BlackCat : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BlackCat);
    explicit BlackCat(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* pKiller) override
    {
        pKiller->castSpell(pKiller, 39477, true);
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

class HeadlessHorsemanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HeadlessHorsemanAI);
    explicit HeadlessHorsemanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //Scarlet Monastery Boss
    }
};

// Headless Horseman - Fire
const uint32 CN_HEADLESS_HORSEMAN_FIRE = 23537;
class HeadlessHorsemanFireAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HeadlessHorsemanFireAI);
    explicit HeadlessHorsemanFireAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->castSpell(getCreature(), 42971, true);
    }
};

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
    ADD_CREATURE_FACTORY_FUNCTION(ShadeOfTheHorsemanAI);
    explicit ShadeOfTheHorsemanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        setCanEnterCombat(false);
        getCreature()->setMountDisplayId(22653);
        //Spells
        //SHADE_OF_THE_HORSEMAN_SUMMON

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
                        getCreature()->castSpell(getCreature(), 42118, true);
                    }
                } break;
                default:
                    break;
            }
        }
    }

    void OnDied(Unit* pKiller)
    {
        GameObject* Pumpkin = pKiller->GetMapMgr()->CreateAndSpawnGameObject(2883, getCreature()->GetPositionX() + Util::getRandomFloat(5.0f), getCreature()->GetPositionY() + Util::getRandomFloat(5.0f), getCreature()->GetPositionZ(), 0, 1);
        if (Pumpkin != nullptr)
            getCreature()->castSpell(Pumpkin->getGuid(), 42277, true);
    }

    int8 WPCount;
};

class HeadlessHorsemanWispInvisAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HeadlessHorsemanWispInvisAI);
    explicit HeadlessHorsemanWispInvisAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mHeadlessHorseman = nullptr;
    }

    void AIUpdate()
    {
        auto _now = std::chrono::system_clock::now();
        auto _time_now = std::chrono::system_clock::to_time_t(_now);
        auto tmPtr = std::localtime(&_time_now);
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

    explicit WaterBarrel(GameObject* goinstance) : GameObjectAIScript(goinstance) {}
    static GameObjectAIScript* Create(GameObject* GO) { return new WaterBarrel(GO); }

    void OnActivate(Player* pPlayer)
    {
        SlotResult slotresult;
        ItemProperties const* proto = sMySQLStore.getItemProperties(32971);
        if (!proto)
            return;

        slotresult = pPlayer->getItemInterface()->FindFreeInventorySlot(proto);

        if (!slotresult.Result)
        {
            pPlayer->getItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
            return;
        }
        else
        {
            if (pPlayer->getItemInterface()->GetItemCount(32971, false) == 0)
            {
                auto itm = objmgr.CreateItem(32971, pPlayer);
                if (itm == nullptr)
                    return;

                auto result = pPlayer->getItemInterface()->SafeAddItem(itm, slotresult.ContainerSlot, slotresult.Slot);
                if (!result)
                {
                    DLLLogDetail("Error while adding item %u to player %s", itm->getEntry(), pPlayer->getName().c_str());
                    itm->DeleteMe();
                }
            }
            else
            {
                pPlayer->getItemInterface()->BuildInventoryChangeError(NULL, NULL, INV_ERR_INVENTORY_FULL);
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
