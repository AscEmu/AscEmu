/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

class Creature;
class Item;
class Player;
class Object;
class GameObject;

class SERVER_DECL GossipScript
{
public:

    GossipScript() {}
    virtual ~GossipScript() {}

    virtual void onHello(Object* object, Player* player) = 0;
    virtual void onSelectOption(Object* /*object*/, Player* /*player*/, uint32_t /*id*/, const char* /*enteredCode*/, uint32_t /*gossipId*/ = 0) {}
    virtual void onEnd(Object* /*object*/, Player* /*player*/) {}
    virtual void destroy();

    static GossipScript* getInterface(Creature* creature);
    static GossipScript* getInterface(Item* item);
    static GossipScript* getInterface(GameObject* gameObject);
};

class SERVER_DECL GossipSpiritHealer : public GossipScript
{
public:

    GossipSpiritHealer() {}
    virtual ~GossipSpiritHealer() {}

    void onHello(Object* object, Player* player) override;
};

class SERVER_DECL GossipVendor : public GossipScript
{
public:

    GossipVendor() {}
    virtual ~GossipVendor() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* plr, uint32_t id, const char* enteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipTrainer : public GossipScript
{
public:

    GossipTrainer() {}
    virtual ~GossipTrainer() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t id, const char* enteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipClassTrainer : public GossipScript
{
public:

    GossipClassTrainer() {}
    virtual ~GossipClassTrainer() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t id, const char* enteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipPetTrainer : public GossipScript
{
public:

    GossipPetTrainer() {}
    virtual ~GossipPetTrainer() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t id, const char* enteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipFlightMaster : public GossipScript
{
public:

    GossipFlightMaster() {}
    virtual ~GossipFlightMaster() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipAuctioneer : public GossipScript
{
public:

    GossipAuctioneer() {}
    virtual ~GossipAuctioneer() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipInnKeeper : public GossipScript
{
public:

    GossipInnKeeper() {}
    virtual ~GossipInnKeeper() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipBattleMaster : public GossipScript
{
public:

    GossipBattleMaster() {}
    virtual ~GossipBattleMaster() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipBanker : public GossipScript
{
public:

    GossipBanker() {}
    virtual ~GossipBanker() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipCharterGiver : public GossipScript
{
public:

    GossipCharterGiver() {}
    virtual ~GossipCharterGiver() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipTabardDesigner : public GossipScript
{
public:

    GossipTabardDesigner() {}
    virtual ~GossipTabardDesigner() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipStableMaster : public GossipScript
{
public:

    GossipStableMaster() {}
    virtual ~GossipStableMaster() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t id, const char* enteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipGeneric : public GossipScript
{
public:

    GossipGeneric() {}
    virtual ~GossipGeneric() {}

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t id, const char* enteredCode, uint32_t gossipId) override;
};
