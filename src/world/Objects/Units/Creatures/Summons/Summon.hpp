/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Unit.hpp"
#include "Objects/Object.hpp"
#include "SummonDefines.hpp"
#include "Spell/Definitions/SummonControlTypes.hpp"

class SERVER_DECL Summon : public Creature
{
public:
    Summon(uint64_t guid, DBC::Structures::SummonPropertiesEntry const* properties);
    ~Summon();

    virtual void load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector& position, uint32_t duration, uint32_t spellId);

    virtual void unSummon();

    // Despawn Type
    CreatureSummonDespawnType getDespawnType() const;
    void setDespawnType(CreatureSummonDespawnType type);

    // Duration
    uint32_t getTimeLeft() const;
    void setTimeLeft(uint32_t time);

    // Max Duration
    uint32_t getLifeTime() const;
    void setLifeTime(uint32_t time);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Object functions
    void OnPushToWorld() override;
    void OnPreRemoveFromWorld() override;
    bool isSummon() const override;
    void onRemoveInRangeObject(Object* object) override;
    void Update(unsigned long /*time_passed*/);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Unit functions
    void die(Unit* pAttacker, uint32 damage, uint32 spellid) override;
    Unit* getUnitOwner() override;          // override creature function
    Unit* getUnitOwnerOrSelf() override;    // override creature function
    Player* getPlayerOwner() override;      // override creature function

    //////////////////////////////////////////////////////////////////////////////////////////
    // Summoner Unit functions
    uint64_t getSummonerGuid() const { return m_summonerGuid; }

    // Summon Information from DBC, when this is nullptr its mostly an Scripted Summon
    DBC::Structures::SummonPropertiesEntry const* const m_Properties;

private:
    Unit* getSummonerUnit();

    // This determines how Despawning of our Summon is Handled
    CreatureSummonDespawnType m_despawnType = MANUAL_DESPAWN;
    uint32_t m_duration = 0;    // Duration Left
    uint32_t m_lifetime = 0;    // Max Duration
    uint64_t m_summonerGuid = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implements guardians
// Guardians are summons that follow and protect their owner
class GuardianSummon : public Summon
{
public:
    GuardianSummon(uint64_t GUID, DBC::Structures::SummonPropertiesEntry const* properties);
    ~GuardianSummon();

    void load(CreatureProperties const* properties_, Unit* unitOwner, LocationVector& position, uint32_t duration, uint32_t spellid) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class implementing companions/vanity pets/critterpets
// These are totally passive and inattackable, they only serve iCandy purposes
class CompanionSummon : public Summon
{
public:
    CompanionSummon(uint64_t GUID, DBC::Structures::SummonPropertiesEntry const* properties);
    ~CompanionSummon();

    void load(CreatureProperties const* properties_, Unit* unitOwner, LocationVector& position, uint32_t duration, uint32_t spellid) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implements summons that are possessed by the player after spawning.
// They despawn when killed or dismissed
class PossessedSummon : public Summon
{
public:
    PossessedSummon(uint64_t GUID, DBC::Structures::SummonPropertiesEntry const* properties);
    ~PossessedSummon();

    void load(CreatureProperties const* properties_, Unit* unitOwner, LocationVector& position, uint32_t duration, uint32_t spellid) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implement wild summons. Wild summonned creatures don't follow or
// protect their owner, however they can be hostile, and attack (not the owner)
class WildSummon : public Summon
{
public:
    WildSummon(uint64_t GUID, DBC::Structures::SummonPropertiesEntry const* properties);
    ~WildSummon();

    void load(CreatureProperties const* properties_, Unit* unitOwner, LocationVector& position, uint32_t duration, uint32_t spellid) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implement Totem summons.
class TotemSummon : public Summon
{
public:
    TotemSummon(uint64_t guid, DBC::Structures::SummonPropertiesEntry const* properties);
    ~TotemSummon();

    void load(CreatureProperties const* properties_, Unit* unitOwner, LocationVector& position, uint32_t duration, uint32_t spellId) override;

    void unSummon() override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Object functions
    void OnPushToWorld() override;
    bool isTotem() const override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Unit functions
    void die(Unit* /*pAttacker*/, uint32_t /*damage*/, uint32_t /*spellid*/) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    //\brief: Sets up the spells the totem will cast. This code was almost directly copied
    //        from SpellEffects.cpp, it requires further refactoring!
    //        For example totems should cast like other units..
    void setupSpells();
};
