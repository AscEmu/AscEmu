/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <cstdint>

class Creature;
class Item;
class Player;
class Object;
class GameObject;

class SERVER_DECL GossipScript
{
public:
    GossipScript() = default;
    virtual ~GossipScript() = default;

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
    GossipSpiritHealer() = default;
    virtual ~GossipSpiritHealer() = default;

    void onHello(Object* object, Player* player) override;
};

class SERVER_DECL GossipVendor : public GossipScript
{
public:
    GossipVendor() = default;
    virtual ~GossipVendor() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* plr, uint32_t id, const char* enteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipTrainer : public GossipScript
{
public:
    GossipTrainer() = default;
    virtual ~GossipTrainer() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t id, const char* enteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipClassTrainer : public GossipScript
{
public:
    GossipClassTrainer() = default;
    virtual ~GossipClassTrainer() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t id, const char* enteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipPetTrainer : public GossipScript
{
public:
    GossipPetTrainer() = default;
    virtual ~GossipPetTrainer() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t id, const char* enteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipFlightMaster : public GossipScript
{
public:
    GossipFlightMaster() = default;
    virtual ~GossipFlightMaster() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipAuctioneer : public GossipScript
{
public:
    GossipAuctioneer() = default;
    virtual ~GossipAuctioneer() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipInnKeeper : public GossipScript
{
public:
    GossipInnKeeper() = default;
    virtual ~GossipInnKeeper() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipBattleMaster : public GossipScript
{
public:
    GossipBattleMaster() = default;
    virtual ~GossipBattleMaster() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipBanker : public GossipScript
{
public:
    GossipBanker() = default;
    virtual ~GossipBanker() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipCharterGiver : public GossipScript
{
public:
    GossipCharterGiver() = default;
    virtual ~GossipCharterGiver() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipTabardDesigner : public GossipScript
{
public:
    GossipTabardDesigner() = default;
    virtual ~GossipTabardDesigner() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t Id, const char* EnteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipStableMaster : public GossipScript
{
public:
    GossipStableMaster() = default;
    virtual ~GossipStableMaster() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t id, const char* enteredCode, uint32_t gossipId) override;
};

class SERVER_DECL GossipGeneric : public GossipScript
{
public:
    GossipGeneric() = default;
    virtual ~GossipGeneric() = default;

    void onHello(Object* object, Player* player) override;
    void onSelectOption(Object* object, Player* player, uint32_t id, const char* enteredCode, uint32_t gossipId) override;
};
