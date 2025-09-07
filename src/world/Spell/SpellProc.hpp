/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "Definitions/ProcFlags.hpp"
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <math.h>

class Aura;
class Object;
class SpellInfo;
class SpellProc;
class Unit;

struct DamageInfo;
struct SpellForcedBasePoints;

typedef std::unique_ptr<SpellProc> (*spell_proc_factory_function)();
typedef std::unordered_map<uint32_t, spell_proc_factory_function> SpellProcMap;

class SERVER_DECL SpellProc
{
    friend class SpellProcMgr;

public:
    SpellProc();
    ~SpellProc();

    // NOTE these virtual methods are legacy script functions
    // please use SpellScript class

    // Called just after this object is created
    // Useful for initialize object members
    virtual void init(Object* obj);

    // Returns true if this spell can proc, false otherwise
    virtual bool canProc(Unit* victim, SpellInfo const* castingSpell);

    // Returns true on success, false otherwise
    bool checkExtraProcFlags(Unit* procOwner, DamageInfo damageInfo) const;

    // Check if this object is identified by method arguments, so it can be deleted
    virtual bool canDeleteProc(uint32_t spellId, uint64_t casterGuid = 0, uint64_t misc = 0);

    // Called when proccing from castingSpell
    // It checks proc class mask with spell group type
    // Return true allow proc, false otherwise
    bool checkClassMask(SpellInfo const* castingSpell) const;

    // Called after proc chance is rolled
    // Return false so Unit::HandleProc execute subsequent statements
    // Return true if this handle everything, so Unit::HandleProc skips to next iteration
    virtual bool doEffect(Unit* victim, SpellInfo const* castingSpell, uint32_t procFlag, uint32_t dmg, uint32_t abs, uint32_t weapon_damage_type);

    // Calculate proc chance
    virtual uint32_t calcProcChance(Unit* victim, SpellInfo const* castingSpell);

    // Cast proc spell
    virtual void castSpell(Unit* victim, SpellInfo const* CastingSpell);

    // Get the spell to proc
    SpellInfo const* getSpell() const;

    // Get the spell that created this proc
    SpellInfo const* getOriginalSpell() const;

    // Get owner of this proc
    Unit* getProcOwner() const;

    uint64_t getCasterGuid() const;
    uint32_t getProcChance() const;
    float_t getProcsPerMinute() const;
    SpellProcFlags getProcFlags() const;
    SpellExtraProcFlags getExtraProcFlags() const;

    uint32_t getProcClassMask(uint8_t i) const;
    void setProcClassMask(uint8_t i, uint32_t mask);

    void setProcFlags(SpellProcFlags procFlags);
    void setExtraProcFlags(SpellExtraProcFlags extraProcFlags);

    void setProcChance(uint32_t procChance);
    void setProcsPerMinute(float_t ppm);

    uint32_t getProcInterval() const;
    void setProcInterval(uint32_t time);

    uint32_t getLastTriggerTime() const;
    void setLastTriggerTime(uint32_t time);

    bool isCastedByProcCreator() const;
    void setCastedByProcCreator(bool);

    bool isCastedOnProcOwner() const;
    void setCastedOnProcOwner(bool);

    // Returns 0 if effect index has not been overridden
    int32_t getOverrideEffectDamage(uint8_t effIndex) const;
    void setOverrideEffectDamage(uint8_t effIndex, int32_t damage);

    Aura* getCreatedByAura() const;
    void setCreatedByAura(Aura* aur);

    // Indicates that this proc will be skipped on next ::handleProc call
    void skipOnNextHandleProc(bool);
    bool isSkippingHandleProc() const;

    // Indicates that this object is deleted and should be removed on next iteration
    void deleteProc();
    bool isDeleted() const;

private:
    // Spell to proc
    SpellInfo const* mSpell = nullptr;

    // Spell that created this proc
    SpellInfo const* mOrigSpell = nullptr;

    // Unit 'owner' of this proc
    Unit* mOwner = nullptr;

    // GUID of the caster of this proc
    uint64_t mCaster = 0;

    uint32_t mProcChance = 0;
    float_t mProcsPerMinute = 0.0f;
    SpellProcFlags mProcFlags = PROC_NULL;
    SpellExtraProcFlags mExtraProcFlags = EXTRA_PROC_NULL;
    // In milliseconds
    uint32_t mProcInterval = 0;

    // Time of last time of proc
    uint32_t mLastTrigger = 0;

    // If set true, the proc spell is casted by the unit who created the proc, not by the unit who owns the proc
    // By default, the proc owner casts the spell
    // Example: Shaman Earth Shield; the heal spell should be casted by the shaman (proc creator),
    // not by the unit (proc owner) who has Earth Shield
    bool m_castedByProcCreator = false;

    // If set true, the proc spell is casted on the proc owner, not on victim
    // By default, the spell is casted on victim
    bool m_castOnProcOwner = false;

    // Mask used to compare with casting spell's family mask
    uint32_t mProcClassMask[3] = { 0, 0, 0 };

    // Mask used on spell effect
    uint32_t mGroupRelation[3] = { 0, 0, 0 };

    std::shared_ptr<SpellForcedBasePoints> mOverrideEffectDamage;

    // Indicates that this proc will be skipped on next ::handleProc call
    // used to avoid some spell procs from procing themselves
    bool mSkipNextProcUpdate = false;

    // Indicate that this object is deleted, and should be remove on next iteration
    bool mDeleted = false;

    Aura* m_createdByAura = nullptr;
};

class SpellProcMgr
{
private:
    SpellProcMgr() = default;
    ~SpellProcMgr() = default;

public:
    static SpellProcMgr& getInstance();
    void initialize();

    SpellProcMgr(SpellProcMgr&&) = delete;
    SpellProcMgr(SpellProcMgr const&) = delete;
    SpellProcMgr& operator=(SpellProcMgr&&) = delete;
    SpellProcMgr& operator=(SpellProcMgr const&) = delete;

    // Registers new spell proc script
    void addById(uint32_t spellId, spell_proc_factory_function spellProc);
    // Registers multiple spell ids to same spell proc script
    void addByIds(uint32_t* spellIds, spell_proc_factory_function spellProc);

    std::unique_ptr<SpellProc> newSpellProc(Unit* owner, uint32_t spellId, uint32_t origSpellId, uint64_t casterGuid, uint32_t procChance, SpellProcFlags procFlags, SpellExtraProcFlags exProcFlags, uint32_t const* spellFamilyMask, uint32_t const* procClassMask, Aura* createdByAura, Object* obj);
    std::unique_ptr<SpellProc> newSpellProc(Unit* owner, SpellInfo const* spellInfo, SpellInfo const* origSpellInfo, uint64_t casterGuid, uint32_t procChance, SpellProcFlags procFlags, SpellExtraProcFlags exProcFlags, uint32_t const* spellFamilyMask, uint32_t const* procClassMask, Aura* createdByAura, Object* obj);

private:
    SpellProcMap mSpellProc;

    // Legacy scripts
    void SetupItems();
    void SetupSpellProcClassScripts();
};

#define sSpellProcMgr SpellProcMgr::getInstance()
