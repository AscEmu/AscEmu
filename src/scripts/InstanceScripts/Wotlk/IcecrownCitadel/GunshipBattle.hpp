/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Raid_IceCrownCitadel.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Gossip: Gunship Battle Alliance
class MuradinGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override;
    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override;
};


//////////////////////////////////////////////////////////////////////////////////////////
/// Muradin: Gunship Battle Alliance
class MuradinAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit MuradinAI(Creature* pCreature);

    void AIUpdate(unsigned long time_passed) override;
    void DoAction(int32_t const action) override;

protected:
    // Common
    IceCrownCitadelScript* mInstance;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Gossip: Gunship Battle Horde
class SaurfangGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override;
    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Saurfang: Gunship Battle Horde
class SaurfangAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit SaurfangAI(Creature* pCreature);

    void AIUpdate(unsigned long time_passed) override;
    void DoAction(int32_t const action) override;

protected:
    // Common
    IceCrownCitadelScript* mInstance;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Zafod Boombos Gossip
class ZafodBoomboxGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override;
    void onSelectOption(Object* /*pObject*/, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override;

protected:
    IceCrownCitadelScript* pInstance;
};

class ZafodBoomboxAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit ZafodBoomboxAI(Creature* pCreature);

protected:
    // Common
    IceCrownCitadelScript* mInstance;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Gunship
class GunshipAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit GunshipAI(Creature* pCreature);

    void DamageTaken(Unit* /*_attacker*/, uint32_t* damage) override;
    void OnDied(Unit* /*pTarget*/) override;

private:
    uint32_t _teamInInstance;
    std::map<uint64_t, uint32_t> _shipVisits;
    bool _summonedFirstMage;
    bool _died;

protected:
    // Common
    IceCrownCitadelScript* mInstance;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Gunship Hull
class GunshipHullAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit GunshipHullAI(Creature* pCreature);

    void DoAction(int32_t const action) override;

protected:
    // Common
    IceCrownCitadelScript* mInstance;

    // Spells
    CreatureAISpells* ExplosionVictory;
    CreatureAISpells* ExplosionWipe;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Gunship Canon
class GunshipCanonAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit GunshipCanonAI(Creature* pCreature);

    void DoAction(int32_t const action) override;

protected:
    // Common
    IceCrownCitadelScript* mInstance;

    // Spells
    CreatureAISpells* EjectBelowZero;
    CreatureAISpells* EcectWipe;
};
