/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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

//////////////////////////////////////////////////////////////////////////////////////////
///\details <b>Brewfest</b>\n
/// event_names entry: 24 \n
/// event_names holiday: 372 \n

/// Boss Coren Direbrew
enum eGossipTexts
{
    DIREBREW_1 = 15858,
    DIREBREW_2 = 15859,
};

/// Gossip Options
#define DIREBREW_MENU_1 "Insult Coren Direbrew's brew."
#define DIREBREW_MENU_2 "Fight."
#define DIREBREW_MENU_3 "Apologize."

#define BOSS_DIREBREW 23872

#define NPC_ILSA 26764
#define NPC_URSULA 26822
#define NPC_MINION 26776

#define SPELL_DISARM 47310
#define SPELL_SUMMON_MINION 47375

class SCRIPT_DECL CorenDirebrewGossip : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* Plr);
    void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char* EnteredCode);
    void GossipEnd(Object* pObject, Player* Plr) { Plr->CloseGossip(); }
};

void CorenDirebrewGossip::GossipHello(Object* pObject, Player * Plr)
{
    GossipMenu* Menu;
    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), DIREBREW_1, Plr);
    Menu->AddItem(0, DIREBREW_MENU_1, 1);
    Menu->SendTo(Plr);
}

void CorenDirebrewGossip::GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char * Code)
{
    GossipMenu* Menu;
    objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), DIREBREW_2, Plr);

    if (!pObject->IsCreature())
        return;

    Creature* pCreature = TO_CREATURE(pObject);

    switch (IntId)
    {
        case 1:
        {
            Menu->AddItem(0, DIREBREW_MENU_2, 2);/// Fight.
            Menu->AddItem(0, DIREBREW_MENU_3, 3);/// Apologize.
            Menu->SendTo(Plr);
        }break;
        case 2:
        {
            pCreature->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "You'll pay for this insult, $c!");
            Plr->Gossip_Complete();
            pCreature->GetAIInterface()->setMoveType(MOVEMENTTYPE_FORWARDTHANSTOP);
            pCreature->MoveToWaypoint(1);
        }break;
    }
};

class CorenDirebrew : public CreatureAIScript
{
public:
    ADD_CREATURE_FACTORY_FUNCTION(CorenDirebrew);
    CorenDirebrew(Creature* pCreature) : CreatureAIScript(pCreature)
    { }
};

void SetupBrewfest(ScriptMgr* mgr)
{
    mgr->register_gossip_script(BOSS_DIREBREW, new CorenDirebrewGossip);
    //mgr->register_creature_script(BOSS_DIREBREW, CorenDirebrew::Create);
}

//////////////////////////////////////////////////////////////////////////////////////////
///\details <b>Brew of the Month</b>\n
/// Brew of the Month Vendors \n
/// -> Accept Quest on Brewfest (qid: 12421) \n
/// -> bringt the ticket(id: 37829) to npc \n
/// -> get every month mail with the brew of the month. \n
/// -> earn achievement "Brew of the Month" (id: 2796) \n
/// -> can buy brew of the month after finishing the quest. \n
/// f.e. Larkin Thunderbrew \n
///\todo create internal event for every month spawn and insert the sql to the db



#define BOTM_GOSSIP_TEXT "Hey there friend!  I see you've got some Brewfest tokens.  As it happens, I still have some Brewfest items for sale." // entry 12864 in npc_text table
#define BOTM_GOSSIP_ITEM "What do you have for me?"

// January
//INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
//    ('434316', 27806, 0, '-4843.8', '-861.921', '501.914', '4.87919', '0', '24979', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1'),
//    ('434317', 27806, 1, '1475.8', '-4210.23', '43.1424', '4.11898', '0', '24979', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1');

// February
//INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
//  ('434316', 27810, 0, '-4843.8', '-861.921', '501.914', '4.87919', '0', '24980', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1'),
//  ('434317', 27810, 1, '1475.8', '-4210.23', '43.1424', '4.11898', '0', '24980', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1');

// March
//INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
//    ('434316', 27811, 0, '-4843.8', '-861.921', '501.914', '4.87919', '0', '24981', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1'),
//   ('434317', 27811, 1, '1475.8', '-4210.23', '43.1424', '4.11898', '0', '24981', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1');

// April
//INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
//    ('434316', 27812, 0, '-4843.8', '-861.921', '501.914', '4.87919', '0', '24982', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1'),
//    ('434317', 27812, 1, '1475.8', '-4210.23', '43.1424', '4.11898', '0', '24982', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1');

// May
//INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
//   ('434316', 27813, 0, '-4843.8', '-861.921', '501.914', '4.87919', '0', '24983', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1'),
//    ('434317', 27813, 1, '1475.8', '-4210.23', '43.1424', '4.11898', '0', '24983', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1');

// June
//INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
//    ('434316', 27814, 0, '-4843.8', '-861.921', '501.914', '4.87919', '0', '24984', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1'),
//    ('434317', 27814, 1, '1475.8', '-4210.23', '43.1424', '4.11898', '0', '24984', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1');

// July
//INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
//    ('434316', 27815, 0, '-4843.8', '-861.921', '501.914', '4.87919', '0', '24985', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1'),
//    ('434317', 27815, 1, '1475.8', '-4210.23', '43.1424', '4.11898', '0', '24985', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1');

// August
//INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
//    ('434316', 27816, 0, '-4843.8', '-861.921', '501.914', '4.87919', '0', '24986', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1'),
//    ('434317', 27816, 1, '1475.8', '-4210.23', '43.1424', '4.11898', '0', '24986', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1');

// September
//INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
//    ('434316', 27817, 0, '-4843.8', '-861.921', '501.914', '4.87919', '0', '24987', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1'),
//    ('434317', 27817, 1, '1475.8', '-4210.23', '43.1424', '4.11898', '0', '24987', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1');


// October
//INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
//    ('434316', 27818, 0, '-4843.8', '-861.921', '501.914', '4.87919', '0', '24988', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1'),
//    ('434317', 27818, 1, '1475.8', '-4210.23', '43.1424', '4.11898', '0', '24988', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1');


// November
//INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
//    ('434316', 27819, 0, '-4843.8', '-861.921', '501.914', '4.87919', '0', '24989', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1'),
//    ('434317', 27819, 1, '1475.8', '-4210.23', '43.1424', '4.11898', '0', '24989', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1');


// December
//INSERT INTO `creature_spawns` (`id`, `entry`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `movetype`, `displayid`, `faction`, `flags`, `bytes0`, `bytes1`, `bytes2`, `emote_state`, `npc_respawn_link`, `channel_spell`, `channel_target_sqlid`, `channel_target_sqlid_creature`, `standstate`, `death_state`, `mountdisplayid`, `slot1item`, `slot2item`, `slot3item`, `CanFly`, `phase`) VALUES
//    ('434316', 27820, 0, '-4843.8', '-861.921', '501.914', '4.87919', '0', '24990', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1'),
//    ('434317', 27820, 1, '1475.8', '-4210.23', '43.1424', '4.11898', '0', '24990', 774, '768', '16843008', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '33161', '0', '0', '0', '1');
