/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QUESTMGR_H
#define QUESTMGR_H

#include "QuestDefines.hpp"
#include "QuestProperties.hpp"
#include "CommonTypes.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <list>

class Creature;
class GameObject;
class Object;
class Player;
struct QuestProperties;
class WorldPacket;
class GossipMenu;

struct QuestRelation
{
    QuestProperties const* qst;
    uint8_t type;
};

struct QuestAssociation
{
    QuestProperties const* qst;
    uint8_t item_count;
};

struct QuestPOIPoint
{
    int32_t x;
    int32_t y;

    QuestPOIPoint() : x(0), y(0) {}

    QuestPOIPoint(int32_t px, int32_t py) :
        x(px),
        y(py) {}
};

struct QuestPOI
{
    uint32_t PoiId;
    int32_t ObjectiveIndex;
    uint32_t MapId;
    uint32_t MapAreaId;
    uint32_t FloorId;
    uint32_t Unk3;
    uint32_t Unk4;

    std::vector<QuestPOIPoint> points;

    QuestPOI() : PoiId(0), ObjectiveIndex(0), MapId(0), MapAreaId(0), FloorId(0), Unk3(0), Unk4(0) {}

    QuestPOI(uint32_t poiId, int32_t objIndex, uint32_t mapId, uint32_t mapAreaId, uint32_t floorId, uint32_t unk3, uint32_t unk4) :
    PoiId(poiId),
    ObjectiveIndex(objIndex),
    MapId(mapId),
    MapAreaId(mapAreaId),
    FloorId(floorId),
    Unk3(unk3),
    Unk4(unk4) {}
};

typedef std::vector<QuestPOI> QuestPOIVector;
typedef std::unordered_map<uint32_t, QuestPOIVector> QuestPOIMap;

class Item;
typedef std::list<std::unique_ptr<QuestRelation>> QuestRelationList;
typedef std::list<std::unique_ptr<QuestAssociation>> QuestAssociationList;

// APGL End
// MIT Start
class SERVER_DECL QuestMgr
{
private:
    QuestMgr() = default;
    ~QuestMgr() = default;

public:
    static QuestMgr& getInstance();
    void finalize();

    QuestMgr(QuestMgr&&) = delete;
    QuestMgr(QuestMgr const&) = delete;
    QuestMgr& operator=(QuestMgr&&) = delete;
    QuestMgr& operator=(QuestMgr const&) = delete;

    void onPlayerItemRemove(Player* plr, Item const* item);

    // MIT End
    // APGL Start

        uint32_t PlayerMeetsReqs(Player* plr, QuestProperties const* qst, bool skiplevelcheck);

        uint32_t CalcStatus(Object* quest_giver, Player* plr);
        uint32_t CalcQuestStatus(Object* quest_giver, Player* plr, QuestRelation* qst);
        uint32_t CalcQuestStatus(Object* quest_giver, Player* plr, QuestProperties const* qst, uint8_t type, bool skiplevelcheck);
        uint32_t CalcQuestStatus(Player* plr, uint32_t qst);
        uint32_t ActiveQuestsCount(Object* quest_giver, Player* plr);

        //Packet Forging...
        void BuildOfferReward(WorldPacket* data, QuestProperties const* qst, Object* qst_giver, uint32_t menutype, uint32_t language, Player* plr);
        void BuildQuestDetails(WorldPacket* data, QuestProperties const* qst, Object* qst_giver, uint32_t menutype, uint32_t language, Player* plr);
        void BuildRequestItems(WorldPacket* data, QuestProperties const* qst, Object* qst_giver, uint32_t status, uint32_t language);
        void BuildQuestComplete(Player*, QuestProperties const* qst);
        void BuildQuestList(WorldPacket* data, Object* qst_giver, Player* plr, uint32_t language);
        bool OnActivateQuestGiver(Object* qst_giver, Player* plr);
        bool isRepeatableQuestFinished(Player* plr, QuestProperties const* qst);

        void SendQuestUpdateAddKill(Player* plr, uint32_t questid, uint32_t entry, uint32_t count, uint32_t tcount, uint64_t guid);
        void BuildQuestUpdateAddItem(WorldPacket* data, uint32_t itemid, uint32_t count);
        void BuildQuestUpdateComplete(WorldPacket* data, QuestProperties const* qst);
        void BuildQuestPOIResponse(WorldPacket & data, uint32_t questid);
        void SendPushToPartyResponse(Player* plr, Player* pTarget, uint8_t response);

        bool OnGameObjectActivate(Player* plr, GameObject* go);
        void OnPlayerKill(Player* plr, Creature* victim, bool IsGroupKill);
        void _OnPlayerKill(Player* plr, uint32_t entry, bool IsGroupKill);
        void OnPlayerCast(Player* plr, uint32_t spellid, uint64_t & victimguid);
        void OnPlayerEmote(Player* plr, uint32_t emoteid, uint64_t & victimguid);
        void OnPlayerItemPickup(Player* plr, Item* item);
        void OnPlayerExploreArea(Player* plr, uint32_t AreaID);
        void AreaExplored(Player* plr, uint32_t QuestID);// scriptdev2

        void OnQuestAccepted(Player* plr, QuestProperties const* qst, Object* qst_giver);
        void OnQuestFinished(Player* plr, QuestProperties const* qst, Object* qst_giver, uint32_t reward_slot);

        void GiveQuestRewardReputation(Player* plr, QuestProperties const* qst, Object* qst_giver);

        uint32_t GenerateQuestXP(Player* plr, QuestProperties const* qst);
        uint32_t GenerateRewardMoney(Player* plr, QuestProperties const* qst);

        void SendQuestInvalid(INVALID_REASON reason, Player* plyr);
        void SendQuestFailed(FAILED_REASON failed, QuestProperties const* qst, Player* plyr);
        void SendQuestUpdateFailed(QuestProperties const* pQuest, Player* plyr);
        void SendQuestUpdateFailedTimer(QuestProperties const* pQuest, Player* plyr);
        void SendQuestLogFull(Player* plyr);

        void LoadNPCQuests(Creature* qst_giver);
        void LoadGOQuests(GameObject* go);

        QuestRelationList* GetCreatureQuestList(uint32_t entryid);
        QuestRelationList* GetGOQuestList(uint32_t entryid);

        void addCreatureQuest(uint32_t _entry, const QuestProperties* _questProp, uint8_t _type);
        void addGameObjectQuest(uint32_t _entry, const QuestProperties* _questProp, uint8_t _type);

        QuestAssociationList* GetQuestAssociationListForItemId(uint32_t itemId);
        uint32_t GetGameObjectLootQuest(uint32_t GO_Entry);
        void SetGameObjectLootQuest(uint32_t GO_Entry, uint32_t Item_Entry);
        inline bool IsQuestRepeatable(QuestProperties const* qst) { return (qst->is_repeatable == 1 ? true : false); }
        inline bool IsQuestDaily(QuestProperties const* qst) { return (qst->is_repeatable == 2 ? true : false); }

        bool CanStoreReward(Player* plyr, QuestProperties const* qst, uint32_t reward_slot);

        inline int32_t QuestHasMob(QuestProperties const* qst, uint32_t mob)
        {
            for (uint8_t i = 0; i < 4; ++i)
                if (qst->required_mob_or_go[i] == (int32_t)mob)
                    return qst->required_mob_or_go_count[i];
            return -1;
        }

        inline int32_t GetOffsetForMob(QuestProperties const* qst, uint32_t mob)
        {
            for (uint8_t i = 0; i < 4; ++i)
                if (qst->required_mob_or_go[i] == (int32_t)mob)
                    return i;

            return -1;
        }

        inline int32_t GetOffsetForItem(QuestProperties const* qst, uint32_t itm)
        {
            for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
                if (qst->required_item[i] == itm)
                    return i;

            return -1;
        }
        void LoadExtraQuestStuff();

        //////////////////////////////////////////////////////////////////////////////////////////
        /// Fills the packet with the quests that the quest giver which the player qualifies for.
        ///
        /// \param Creature* quest giver
        /// \param Player*  player for whom quests are qualified
        /// \param GossipMenu& - menu to fill with quests.
        ///
        /// \returns void
        ///
        //////////////////////////////////////////////////////////////////////////////////////////
        void FillQuestMenu(Creature*, Player*, GossipMenu &);

    private:
        std::unordered_map<uint32_t, std::unique_ptr<QuestRelationList> > m_npc_quests;
        std::unordered_map<uint32_t, std::unique_ptr<QuestRelationList> > m_obj_quests;
        std::unordered_map<uint32_t, std::list<QuestRelation*>* > m_itm_quests;
        QuestPOIMap m_QuestPOIMap;

        std::unordered_map<uint32_t, std::unique_ptr<QuestAssociationList> > m_quest_associations;

        std::unordered_map<uint32_t, uint32_t> m_ObjectLootQuestList;

        /*template <class T>
        void _AddQuest(uint32_t entryid, QuestProperties const* qst, uint8_t type);*/

        /*template <class T>
        std::unordered_map<uint32_t, std::list<QuestRelation*>* >& _GetList();*/

        void AddItemQuestAssociation(uint32_t itemId, QuestProperties const* qst, uint8_t item_count);

        // Quest Loading
        void _RemoveChar(char* c, std::string* str);
        // Zyres: not used 2022/03/06
        //void _CleanLine(std::string* str);
};

//template<> inline std::unordered_map<uint32_t, std::list<QuestRelation*>* >& QuestMgr::_GetList<Creature>()
//{ return m_npc_quests; }
//template<> inline std::unordered_map<uint32_t, std::list<QuestRelation*>* >& QuestMgr::_GetList<GameObject>()
//{ return m_obj_quests; }
//template<> inline std::unordered_map<uint32_t, std::list<QuestRelation*>* >& QuestMgr::_GetList<Item>()
//{ return m_itm_quests; }


#define sQuestMgr QuestMgr::getInstance()

#endif // QUESTMGR_H
