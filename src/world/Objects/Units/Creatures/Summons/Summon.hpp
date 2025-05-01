/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Units/Creatures/Creature.h"

namespace WDB::Structures
{
    struct SummonPropertiesEntry;
}

class SERVER_DECL Summon : public Creature
{
public:
    Summon(uint64_t guid, WDB::Structures::SummonPropertiesEntry const* properties);
    virtual ~Summon();

    virtual void load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector const& position, uint32_t duration, uint32_t spellId);

    virtual void unSummon();
    void dieAndDisappearOnExpire();

    // Despawn Type
    CreatureSummonDespawnType getDespawnType() const;
    void setDespawnType(CreatureSummonDespawnType type);

    // Summon properties
    WDB::Structures::SummonPropertiesEntry const* getSummonProperties() const;

    bool isSummonActive() const;
    bool isPermanentSummon() const;

    // Duration
    uint32_t getTimeLeft() const;
    void setTimeLeft(uint32_t time);

    // Max Duration
    uint32_t getLifeTime() const;
    void setLifeTime(uint32_t time);

    // Resets life time and time left
    void setNewLifeTime(uint32_t time);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Object functions
    virtual void OnPushToWorld() override;
    void OnPreRemoveFromWorld() override;
    bool isSummon() const override;
    void onRemoveInRangeObject(Object* object) override;
    virtual void Update(unsigned long /*time_passed*/) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Unit functions
    void die(Unit* pAttacker, uint32_t damage, uint32_t spellid) override;
    // Returns unit owner
    Unit* getUnitOwner() override;
    // Returns unit owner
    Unit const* getUnitOwner() const override;
    // Returns player owner
    Player* getPlayerOwner() override;
    // Returns player owner
    Player const* getPlayerOwner() const override;

protected:
    void _onUnsummon();
    bool _hasExtendedDespawnDelayInDeath() const;

    Unit* m_unitOwner = nullptr;
    bool m_summonActive = false;

    // This determines how Despawning of our Summon is Handled
    CreatureSummonDespawnType m_despawnType = MANUAL_DESPAWN;

    // Summon Information from DBC, when this is nullptr its mostly an Scripted Summon
    WDB::Structures::SummonPropertiesEntry const* const m_summonProperties;

    // Duration Left
    uint32_t m_duration = 0;
    // Max Duration
    uint32_t m_lifetime = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implements guardians
// Guardians are summons that follow and protect their owner
class GuardianSummon : public Summon
{
public:
    GuardianSummon(uint64_t GUID, WDB::Structures::SummonPropertiesEntry const* properties);
    ~GuardianSummon();

    void load(CreatureProperties const* properties_, Unit* unitOwner, LocationVector const& position, uint32_t duration, uint32_t spellid) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class implementing companions/vanity pets/critterpets
// These are totally passive and inattackable, they only serve iCandy purposes
class CompanionSummon : public Summon
{
public:
    CompanionSummon(uint64_t GUID, WDB::Structures::SummonPropertiesEntry const* properties);
    ~CompanionSummon();

    void load(CreatureProperties const* properties_, Unit* unitOwner, LocationVector const& position, uint32_t duration, uint32_t spellid) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implements summons that are possessed by the player after spawning.
// They despawn when killed or dismissed
class PossessedSummon : public Summon
{
public:
    PossessedSummon(uint64_t GUID, WDB::Structures::SummonPropertiesEntry const* properties);
    ~PossessedSummon();

    void load(CreatureProperties const* properties_, Unit* unitOwner, LocationVector const& position, uint32_t duration, uint32_t spellid) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implement wild summons. Wild summonned creatures don't follow or
// protect their owner, however they can be hostile, and attack (not the owner)
class WildSummon : public Summon
{
public:
    WildSummon(uint64_t GUID, WDB::Structures::SummonPropertiesEntry const* properties);
    ~WildSummon();

    void load(CreatureProperties const* properties_, Unit* unitOwner, LocationVector const& position, uint32_t duration, uint32_t spellid) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implement Totem summons.
class TotemSummon : public Summon
{
public:
    TotemSummon(uint64_t guid, WDB::Structures::SummonPropertiesEntry const* properties);
    ~TotemSummon();

    void load(CreatureProperties const* properties_, Unit* unitOwner, LocationVector const& position, uint32_t duration, uint32_t spellId) override;

    void unSummon() override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Object functions
    void OnPushToWorld() override;
    bool isTotem() const override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    //\brief: Sets up the spells the totem will cast. This code was almost directly copied
    //        from SpellEffects.cpp, it requires further refactoring!
    //        For example totems should cast like other units..
    void setupSpells();
};
