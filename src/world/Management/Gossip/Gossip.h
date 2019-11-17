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

        enum ws
        {
            SURE_TO_PURCHASE_DTS,               // ""
            PURCHASE_DTS,                       // ""
            NOT_ENOUGH_MONEY_DTS,               // ""
        };

        enum GossipText
        {
            TXTID_TALENTRESET = 5674,           // NT_  ? ""
            TXTID_PETUNTRAIN = 7722,            // NT_  ? ""
            TXTID_DUALSPECPURCHASE = 14136      // NT_  ? this is not correct.
        };
        

        class Menu;
        class Item
        {
            public:

                uint16 id_;
                bool coded_;
                uint8 icon_;
                uint32 boxmoney_;
                std::string boxmessage_;
                std::string text_;
                Item(size_t, uint8);
                Item(size_t, uint8, const char*, bool = false, size_t = 0, const char* = nullptr);

                friend class Menu;
        };

        typedef std::vector<Gossip::Item> ItemList;
        typedef std::map<QuestProperties const*, uint8> QuestList;

        class SERVER_DECL Menu
        {
            public:

                Menu(uint64, uint32, uint32 = 0, uint32 = 0);
                Menu(Object*, uint32, uint32 = 0, uint32 = 0);

                //////////////////////////////////////////////////////////////////////////////////////////
                // Adds a menu item.
                // \param uint8 - the icon
                // \param const char * - the text of the item.
                // \param uint32 - the id of the item, limit is 2^16 or 0xFFFF;
                // \param bool - whether or not to retrieve text input.
                //////////////////////////////////////////////////////////////////////////////////////////
                void AddItem(uint8, const char*, uint32, bool = false);

                //////////////////////////////////////////////////////////////////////////////////////////
                // Adds a menu item.
                // \param uint8 - the icon
                // \param const char * - the text of the item.
                // \param uint32 - the id of the item, limit is 2^16 or 0xFFFF;
                // \param uint32 - box money
                // \param const char* - box text
                // \param bool - whether or not to retrieve text input.
                //////////////////////////////////////////////////////////////////////////////////////////
                void AddItem(uint8, const char*, uint32, uint32, const char*, bool = false);

                //////////////////////////////////////////////////////////////////////////////////////////
                // Removes the item with the specified id.
                // \param uint32 - the id of the item.
                //////////////////////////////////////////////////////////////////////////////////////////
                void RemoveItem(uint32);

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
                void RemoveQuest(QuestProperties const*);

                //////////////////////////////////////////////////////////////////////////////////////////
                // Fills the packet with the menu data.
                // \param WorldPacket & - the packet to fill
                //////////////////////////////////////////////////////////////////////////////////////////
                void BuildPacket(WorldPacket &) const;
                void BuildPacket(WorldPacket*) const;

                template<uint32 size>
                void BuildPacket(StackBuffer<size> &) const;

                uint32 getTextID() const { return textid_; }
                uint32 getLanguage() const { return language_; }

                inline void setTextID(uint32 textid) { textid_ = textid; }
                inline void setLanguage(uint32 language) { language_ = language; }

                //////////////////////////////////////////////////////////////////////////////////////////
                // Creates an worldpacket SMSG_GOSSIP_MESSAGE packet, fills it and sends it to the specified player.
                // \param Player*  - player to send to.
                // \returns  void
                //////////////////////////////////////////////////////////////////////////////////////////
                void Send(Player*) const;

                //////////////////////////////////////////////////////////////////////////////////////////
                // Creates a stackworldpacket SMSG_GOSSIP_MESSAGE, fills it and sends it to the specified player.
                // \param Player*  - player to send to.
                // \returns  void
                //////////////////////////////////////////////////////////////////////////////////////////
                template<uint32>
                void StackSend(Player*) const;

                //////////////////////////////////////////////////////////////////////////////////////////
                // Sends a menu with just the text id and no options.
                // \param uint64 - the creature guid.
                // \param size_t - the text id.
                // \param Player*  - the player to send to.
                //////////////////////////////////////////////////////////////////////////////////////////
                static void SendSimpleMenu(uint64, size_t, Player*);

                //////////////////////////////////////////////////////////////////////////////////////////
                // Sends a menu with a given text id and menu item.
                // \param uint32 - creature guid
                // \param size_t - txt id
                // \param Player* - player to send to.
                // \param size_t - item id
                // \param uint8 - icon
                // \param const string * - item text
                // \param size_t - required money
                // \param const string * - money text
                // \param uint8 extra
                //////////////////////////////////////////////////////////////////////////////////////////
                static void SendQuickMenu(uint64, size_t, Player*, size_t, uint8, const char*, size_t = 0, const char * = nullptr, uint8 = 0);

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
