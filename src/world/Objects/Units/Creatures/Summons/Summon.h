/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Units/Creatures/Creature.h"

class Summon : public Creature
{
public:
    Summon(uint64_t guid, uint32_t duration);
    ~Summon();

    virtual void Load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector& position, uint32_t spellId, int32_t summonSlot);

    virtual void unSummon();

    uint32_t getTimeLeft() const;
    void setTimeLeft(uint32_t time);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Object functions
    void OnPushToWorld() override;
    void OnPreRemoveFromWorld() override;
    bool isSummon() const override;
    void onRemoveInRangeObject(Object* object) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Unit functions
    void Die(Unit* pAttacker, uint32 damage, uint32 spellid) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
protected:
    int32_t m_summonSlot = -1;
    Unit* m_unitOwner = nullptr;

    uint32_t m_duration = 0;

public:
    bool isSummonedToSlot() const;

    Unit* getUnitOwner() const { return m_unitOwner; }
    Player* getPlayerOwner() override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implements guardians
// Guardians are summons that follow and protect their owner
class GuardianSummon : public Summon
{
public:
    GuardianSummon(uint64_t GUID, uint32_t duration);
    ~GuardianSummon();

    void Load(CreatureProperties const* properties_, Unit* owner, LocationVector& position, uint32_t spellid, int32_t summonslot) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class implementing companions/vanity pets/critterpets
// These are totally passive and inattackable, they only serve iCandy purposes
class CompanionSummon : public Summon
{
public:
    CompanionSummon(uint64_t GUID, uint32_t duration);
    ~CompanionSummon();

    void Load(CreatureProperties const* properties_, Unit* owner, LocationVector& position, uint32_t spellid, int32_t summonslot) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implements summons that are possessed by the player after spawning.
// They despawn when killed or dismissed
class PossessedSummon : public Summon
{
public:
    PossessedSummon(uint64_t GUID, uint32_t duration);
    ~PossessedSummon();

    void Load(CreatureProperties const* properties_, Unit* owner, LocationVector& position, uint32_t spellid, int32_t summonslot) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implement wild summons. Wild summonned creatures don't follow or
// protect their owner, however they can be hostile, and attack (not the owner)
class WildSummon : public Summon
{
public:
    WildSummon(uint64_t GUID, uint32_t duration);
    ~WildSummon();

    void Load(CreatureProperties const* properties_, Unit* owner, LocationVector& position, uint32_t spellid, int32_t summonslot) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class that implement Totem summons.
class TotemSummon : public Summon
{
public:
    TotemSummon(uint64_t guid, uint32_t duration);
    ~TotemSummon();

    void Load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector& position, uint32_t spellId, int32_t summonSlot) override;

    void unSummon() override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Object functions
    void OnPushToWorld() override;
    void OnPreRemoveFromWorld() override;
    bool isTotem() const override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Override Unit functions
    void Die(Unit* /*pAttacker*/, uint32_t /*damage*/, uint32_t /*spellid*/) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    //\brief: Sets up the spells the totem will cast. This code was almost directly copied
    //        from SpellEffects.cpp, it requires further refactoring!
    //        For example totems should cast like other units..
    void SetupSpells();
};
