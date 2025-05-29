/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SpellMgr.hpp"
#include "SpellProc.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Definitions/ProcFlags.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Objects/Units/Players/Player.hpp"

SpellProc::SpellProc()
{
    mOverrideEffectDamage = std::make_shared<SpellForcedBasePoints>();
}

SpellProc::~SpellProc()
{
}

void SpellProc::init(Object* /*obj*/) { }

bool SpellProc::canProc(Unit* /*victim*/, SpellInfo const* /*castingSpell*/)
{
    return true;
}

bool SpellProc::checkExtraProcFlags([[maybe_unused]]Unit* procOwner, DamageInfo damageInfo) const
{
    // Quick check
    if (mExtraProcFlags == EXTRA_PROC_NULL)
        return true;

    // Main hand hits should not proc off-hand enchantments/poisons
    if (damageInfo.weaponType == MELEE && mExtraProcFlags & EXTRA_PROC_ON_OFF_HAND_HIT_ONLY)
        return false;
    // Off-hand hits should not proc main hand enchantments/poisons
    if (damageInfo.weaponType == OFFHAND && mExtraProcFlags & EXTRA_PROC_ON_MAIN_HAND_HIT_ONLY)
        return false;

#if VERSION_STRING < WotLK
    // In Classic and TBC weapon enchantments never proc while in feral forms
    if (procOwner->isPlayer() && static_cast<Player*>(procOwner)->isInFeralForm() &&
        (mExtraProcFlags & EXTRA_PROC_ON_MAIN_HAND_HIT_ONLY ||
        mExtraProcFlags & EXTRA_PROC_ON_OFF_HAND_HIT_ONLY))
        return false;
#endif

    // Check on crit procs
    if (mExtraProcFlags & EXTRA_PROC_ON_CRIT_ONLY && !damageInfo.isCritical)
        return false;

    return true;
}

bool SpellProc::canDeleteProc(uint32_t spellId, uint64_t casterGuid, uint64_t /*misc*/)
{
    if (mSpell->getId() == spellId && (casterGuid == 0 || mCaster == casterGuid) && !mDeleted)
        return true;

    return false;
}

bool SpellProc::checkClassMask(SpellInfo const* castingSpell) const
{
    if (mProcClassMask[0] == 0 && mProcClassMask[1] == 0 && mProcClassMask[2] == 0)
        return true;

    if (castingSpell->getSpellFamilyFlags(0) == 0 && castingSpell->getSpellFamilyFlags(1) == 0 && castingSpell->getSpellFamilyFlags(2) == 0)
        return false;

    if (((castingSpell->getSpellFamilyFlags(0) == 0) || (mProcClassMask[0] & castingSpell->getSpellFamilyFlags(0))) &&
        ((castingSpell->getSpellFamilyFlags(1) == 0) || (mProcClassMask[1] & castingSpell->getSpellFamilyFlags(1))) &&
        ((castingSpell->getSpellFamilyFlags(2) == 0) || (mProcClassMask[2] & castingSpell->getSpellFamilyFlags(2))))
        return true;

    return false;
}

bool SpellProc::doEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32_t /*flag*/, uint32_t /*dmg*/, uint32_t /*abs*/, uint32_t /*weaponDamageType*/)
{
    return false;
}

uint32_t SpellProc::calcProcChance(Unit* /*victim*/, SpellInfo const* /*castingSpell*/)
{
    // Check if proc chance is based on combo points
    if (mOwner->isPlayer() && mOrigSpell != nullptr && mOrigSpell->getAttributesEx() & ATTRIBUTESEX_REQ_COMBO_POINTS1 && mOrigSpell->getAttributesExD() & ATTRIBUTESEXD_PROCCHANCE_COMBOBASED)
        return static_cast<uint32_t>(static_cast<Player*>(mOwner)->getComboPoints() * mOrigSpell->getEffectPointsPerComboPoint(0));
    else
        return mProcChance;
}

void SpellProc::castSpell(Unit* victim, SpellInfo const* castingSpell)
{
    if (isCastedOnProcOwner())
        victim = getProcOwner();

    if (victim == nullptr)
        return;

    Unit* caster = getProcOwner();
    if (isCastedByProcCreator())
    {
        if (getCasterGuid() == getProcOwner()->getGuid())
            caster = getProcOwner();
        else
            caster = getProcOwner()->getWorldMapUnit(getCasterGuid());
    }

    if (caster == nullptr)
        return;

    SpellCastTargets targets(victim->getGuid());
    Spell* spell = sSpellMgr.newSpell(caster, mSpell, true, nullptr);

    spell->forced_basepoints = mOverrideEffectDamage;

    spell->ProcedOnSpell = castingSpell;
    if (mOrigSpell != nullptr)
        spell->pSpellId = mOrigSpell->getId();

    // Final script hook before casting the spell
    const auto scriptResult = sScriptMgr.callScriptedSpellProcCastSpell(this, caster, victim, spell);
    if (scriptResult == SpellScriptExecuteState::EXECUTE_PREVENT)
    {
        // Script prevents this spell cast => delete spell and do nothing
        caster->addGarbageSpell(spell);
        return;
    }

    spell->prepare(&targets);
}

SpellInfo const* SpellProc::getSpell() const { return mSpell; }

SpellInfo const* SpellProc::getOriginalSpell() const { return mOrigSpell; }

Unit* SpellProc::getProcOwner() const { return mOwner; }

uint64_t SpellProc::getCasterGuid() const { return mCaster; }

uint32_t SpellProc::getProcChance() const { return mProcChance; }

float_t SpellProc::getProcsPerMinute() const { return mProcsPerMinute; }

SpellProcFlags SpellProc::getProcFlags() const { return mProcFlags; }

SpellExtraProcFlags SpellProc::getExtraProcFlags() const { return mExtraProcFlags; }

uint32_t SpellProc::getProcClassMask(uint8_t i) const { return mProcClassMask[i]; }

void SpellProc::setProcClassMask(uint8_t i, uint32_t mask) { mProcClassMask[i] = mask; }

void SpellProc::setProcFlags(SpellProcFlags procFlags) { mProcFlags = procFlags; }

void SpellProc::setExtraProcFlags(SpellExtraProcFlags extraProcFlags) { mExtraProcFlags = extraProcFlags; }

void SpellProc::setProcChance(uint32_t procChance) { mProcChance = procChance; }

void SpellProc::setProcsPerMinute(float_t ppm) { mProcsPerMinute = ppm; }

uint32_t SpellProc::getProcInterval() const { return mProcInterval; }

void SpellProc::setProcInterval(uint32_t time) { mProcInterval = time; }

uint32_t SpellProc::getLastTriggerTime() const { return mLastTrigger; }

void SpellProc::setLastTriggerTime(uint32_t time) { mLastTrigger = time; }

bool SpellProc::isCastedByProcCreator() const { return m_castedByProcCreator; }
void SpellProc::setCastedByProcCreator(bool enable) { m_castedByProcCreator = enable; }

bool SpellProc::isCastedOnProcOwner() const { return m_castOnProcOwner; }
void SpellProc::setCastedOnProcOwner(bool enable) { m_castOnProcOwner = enable; }

int32_t SpellProc::getOverrideEffectDamage(uint8_t effIndex) const
{
    int32_t overrideValue = 0;
    mOverrideEffectDamage->get(effIndex, &overrideValue);
    return overrideValue;
}

void SpellProc::setOverrideEffectDamage(uint8_t effIndex, int32_t damage)
{
    mOverrideEffectDamage->set(effIndex, damage);
}

Aura* SpellProc::getCreatedByAura() const { return m_createdByAura; }
void SpellProc::setCreatedByAura(Aura* aur) { m_createdByAura = aur; }

void SpellProc::skipOnNextHandleProc(bool skip) { mSkipNextProcUpdate = skip; }
bool SpellProc::isSkippingHandleProc() const { return mSkipNextProcUpdate; }

void SpellProc::deleteProc() { mDeleted = true; }
bool SpellProc::isDeleted() const { return mDeleted; }

SpellProcMgr& SpellProcMgr::getInstance()
{
    static SpellProcMgr mInstance;
    return mInstance;
}

void SpellProcMgr::initialize()
{
    // Setup legacy scripts
    SetupItems();
    SetupSpellProcClassScripts();
}

void SpellProcMgr::addById(uint32_t spellId, spell_proc_factory_function spellProc)
{
    mSpellProc.insert(std::make_pair(spellId, spellProc));
}

void SpellProcMgr::addByIds(uint32_t* spellIds, spell_proc_factory_function spellProc)
{
    for (uint32_t i = 0; spellIds[i] != 0; ++i)
    {
        mSpellProc.insert(std::make_pair(spellIds[i], spellProc));
    }
}

std::unique_ptr<SpellProc> SpellProcMgr::newSpellProc(Unit* owner, uint32_t spellId, uint32_t origSpellId, uint64_t casterGuid, uint32_t procChance, SpellProcFlags procFlags, SpellExtraProcFlags exProcFlags, uint32_t const* spellFamilyMask, uint32_t const* procClassMask, Aura* createdByAura, Object* obj)
{
    return newSpellProc(owner, sSpellMgr.getSpellInfo(spellId), sSpellMgr.getSpellInfo(origSpellId), casterGuid, procChance, procFlags, exProcFlags, spellFamilyMask, procClassMask, createdByAura, obj);
}

std::unique_ptr<SpellProc> SpellProcMgr::newSpellProc(Unit* owner, SpellInfo const* spellInfo, SpellInfo const* origSpellInfo, uint64_t casterGuid, uint32_t procChance, SpellProcFlags procFlags, SpellExtraProcFlags exProcFlags, uint32_t const* spellFamilyMask, uint32_t const* procClassMask, Aura* createdByAura, Object* obj)
{
    if (spellInfo == nullptr)
        return nullptr;

    spell_proc_factory_function ptr = nullptr;

    // Search for SpellProc script
    const auto itr = mSpellProc.find(spellInfo->getId());
    if (itr != mSpellProc.end())
        ptr = itr->second;

    std::unique_ptr<SpellProc> result = nullptr;
    if (ptr != nullptr)
        result = (*ptr)();                      // Found, create a new object of this specific class
    else
        result = std::make_unique<SpellProc>(); // Not found, create a new object of generic SpellProc

    result->mSpell = spellInfo;
    result->mOrigSpell = origSpellInfo;
    result->mOwner = owner;
    result->mCaster = casterGuid;
    result->mProcChance = procChance;
    result->mProcFlags = procFlags;
    result->mExtraProcFlags = exProcFlags;
    result->mLastTrigger = 0;
    result->m_createdByAura = createdByAura;
    result->mDeleted = false;

    if (spellFamilyMask != nullptr)
    {
        result->mGroupRelation[0] = spellFamilyMask[0];
        result->mGroupRelation[1] = spellFamilyMask[1];
        result->mGroupRelation[2] = spellFamilyMask[2];
    }
    else
    {
        result->mGroupRelation[0] = 0;
        result->mGroupRelation[1] = 0;
        result->mGroupRelation[2] = 0;
    }

    if (procClassMask != nullptr)
    {
        result->mProcClassMask[0] = procClassMask[0];
        result->mProcClassMask[1] = procClassMask[1];
        result->mProcClassMask[2] = procClassMask[2];
    }
    else
    {
        result->mProcClassMask[0] = 0;
        result->mProcClassMask[1] = 0;
        result->mProcClassMask[2] = 0;
    }

    if (sScriptMgr.getSpellScript(spellInfo->getId()) != nullptr)
        sScriptMgr.callScriptedSpellProcCreate(result.get(), obj);
    else
        result->init(obj);

    return result;
}
