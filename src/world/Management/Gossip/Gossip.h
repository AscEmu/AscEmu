/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

#ifndef GOSSIP_H
#define GOSSIP_H

#include "Management/Gossip/GossipDefines.hpp"
#include "WorldPacket.h"
#include "StackBuffer.h"
#include <map>

struct QuestProperties;
class Creature;
class Item;
class Player;
class Object;
class GameObject;


namespace Arcemu
{
    namespace Gossip
    {
        using namespace Arcemu;

        typedef std::map<uint32_t, GossipItem> ItemList;
        typedef std::map<uint32_t, GossipQuestItem> QuestList;

        class SERVER_DECL Menu
        {
            public:

                Menu(uint64, uint32, uint32 = 0, uint32 = 0);

                //MIT starts
                void addItem(uint8_t icon, uint32_t textId, uint32_t id, std::string text = "", uint32_t boxMoney = 0, std::string boxMessage = "", bool isCoded = false);

                void removeItem(uint32_t id);
                //MIT ends


                //////////////////////////////////////////////////////////////////////////////////////////
                // Adds a quest item to the menu
                // \param Quest * - the quest
                // \param uint8 - the icon
                //////////////////////////////////////////////////////////////////////////////////////////
                void AddQuest(QuestProperties const*, uint8);

                //////////////////////////////////////////////////////////////////////////////////////////
                // Removes a quest.
                // \param Quest * - quest to remove.
                // \returns  void
                //////////////////////////////////////////////////////////////////////////////////////////
                void RemoveQuest(uint32_t id);


                // MIT starts
                uint32_t getTextID() const { return textid_; }
                uint32_t getLanguage() const { return language_; }

                void setTextID(uint32_t textid) { textid_ = textid; }
                void setLanguage(uint32_t language) { language_ = language; }

                void sendGossipPacket(Player* player) const;
                static void sendSimpleMenu(uint64_t guid, uint32_t txt_id, Player* plr);
                static void sendQuickMenu(uint64_t guid, uint32_t textid, Player* Plr, uint32_t itemid, uint8_t itemicon, std::string itemtext, uint32_t requiredmoney = 0, std::string moneytext = "" , bool extra = false);

                // MIT ends

                //////////////////////////////////////////////////////////////////////////////////////////
                // Simply sends out an SMSG_GOSSIP_COMPLETE packet.
                // \param Plr *
                // \returns  void
                //////////////////////////////////////////////////////////////////////////////////////////
                static void Complete(Player*);

                uint32 textid_;
                uint32 language_;                   /// For localized quest texts.
                uint32 gossipId;
                uint64 guid_;
                Gossip::ItemList itemlist_;         /// Contains non quest items.
                Gossip::QuestList questlist_;       /// Contains the quests, it's filled up then added to the packet when it comes to send.
        };

        class SERVER_DECL Script
        {
            public:

                Script() {}
                virtual ~Script() {}

                virtual void OnHello(Object* pObject, Player* plr) = 0;
                virtual void OnSelectOption(Object* /*pObject*/, Player* /*plr*/, uint32 /*id*/, const char* /*enteredCode*/, uint32 /*gossipId*/ = 0) {}
                virtual void OnEnd(Object* /*pObject*/, Player* /*plr*/) {}
                virtual void Destroy();

                static Script* GetInterface(Creature*);
                static Script* GetInterface(::Item*);
                static Script* GetInterface(GameObject*);
        };
        /// Spirit Healer Dialog.
        class SERVER_DECL SpiritHealer : public Script
        {
            public:

                SpiritHealer() {}
                virtual ~SpiritHealer() {}
                void OnHello(Object* pObject, Player* plr) override;
        };

        class SERVER_DECL Vendor : public Script
        {
            public:

                Vendor() {}
                virtual ~Vendor() {}
                void OnHello(Object* pObject, Player* plr) override;
                void OnSelectOption(Object* pObject, Player* plr, uint32 id, const char* enteredCode, uint32_t gossipId) override;
        };
        class SERVER_DECL Trainer : public Script
        {
            public:

                Trainer() {}
                virtual ~Trainer() {}
                void OnHello(Object* pObject, Player* plr) override;
                void OnSelectOption(Object* pObject, Player* plr, uint32 id, const char* enteredCode, uint32_t gossipId) override;
        };
        class SERVER_DECL ClassTrainer : public Script
        {
            public:

                ClassTrainer() {}
                virtual ~ClassTrainer() {}
                void OnHello(Object* pObject, Player* plr) override;
                void OnSelectOption(Object* pObject, Player* plr, uint32 id, const char* enteredCode, uint32_t gossipId) override;
        };
        class SERVER_DECL PetTrainer : public Script
        {
            public:

                PetTrainer() {}
                virtual ~PetTrainer() {}
                void OnHello(Object* pObject, Player* Plr) override;
                void OnSelectOption(Object* pObject, Player* plr, uint32 id, const char* enteredCode, uint32_t gossipId) override;
        };
        class SERVER_DECL FlightMaster : public Script
        {
            public:

                FlightMaster() {}
                virtual ~FlightMaster() {}
                void OnHello(Object* pObject, Player* Plr) override;
                void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId) override;
        };
        class SERVER_DECL Auctioneer: public Script
        {
            public:

                Auctioneer() {}
                virtual ~Auctioneer() {}
                void OnHello(Object* pObject, Player* Plr) override;
                void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId) override;
        };
        class SERVER_DECL InnKeeper : public Script
        {
            public:

                InnKeeper() {}
                virtual ~InnKeeper() {}
                void OnHello(Object* pObject, Player* Plr) override;
                void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId) override;
        };
        class SERVER_DECL BattleMaster : public Script
        {
            public:

                BattleMaster() {}
                virtual ~BattleMaster() {}
                void OnHello(Object* pObject, Player* Plr) override;
                void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId) override;
        };
        class SERVER_DECL Banker : public Script
        {
            public:

                Banker() {}
                virtual ~Banker() {}
                void OnHello(Object* pObject, Player* Plr) override;
                void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId) override;
        };
        class SERVER_DECL CharterGiver : public Script
        {
            public:

                CharterGiver() {}
                virtual ~CharterGiver() {}
                void OnHello(Object* pObject, Player* Plr) override;
                void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId) override;
        };
        class SERVER_DECL TabardDesigner : public Script
        {
            public:

                TabardDesigner() {}
                virtual ~TabardDesigner() {}
                void OnHello(Object* pObject, Player* Plr) override;
                void OnSelectOption(Object* pObject, Player* Plr, uint32 Id, const char* EnteredCode, uint32_t gossipId) override;
        };
        class SERVER_DECL StableMaster : public Script
        {
            public:

                StableMaster() {}
                virtual ~StableMaster() {}
                void OnHello(Object* pObject, Player* plr) override;
                void OnSelectOption(Object* pObject, Player* plr, uint32 id, const char* enteredCode, uint32_t gossipId) override;
        };
        class SERVER_DECL Generic : public Script
        {
            public:

                Generic() {}
                virtual ~Generic() {}
                void OnHello(Object* pObject, Player* plr) override;
                void OnSelectOption(Object* pObject, Player* plr, uint32 id, const char* enteredCode, uint32_t gossipId) override;
        };
    }
}

#endif      //GOSSIP_H
