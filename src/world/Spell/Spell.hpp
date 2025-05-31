/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Definitions/SpellFailure.hpp"
#include "Definitions/SpellState.hpp"
#include "Definitions/SpellTargetMod.hpp"
#include "SpellCastTargets.hpp"
#include "SpellTargetConstraint.hpp"
#include "Movement/Spline/MovementTypedefs.h"
#include "Objects/DamageInfo.hpp"
#include "SpellDefines.hpp"
#include "Storage/WDB/WDBDefines.hpp"

#include <map>
#include <memory>
#include <set>

namespace WDB::Structures
{
    struct SummonPropertiesEntry;
}

struct AuraEffectModifier;
struct SpellForcedBasePoints;
class Object;
class SpellInfo;
class Corpse;
class WorldSession;
class Unit;
class DynamicObj;
class Player;
class Item;
class Group;
class Aura;
class DummySpellHandler;
class Creature;
struct CreatureProperties;
class GameObject;
class Spell;

typedef void(Spell::*pSpellEffect)(uint8_t effectIndex);
typedef void(Spell::*pSpellTarget)(uint32_t i, uint32_t j);

#define GO_FISHING_BOBBER 35591

typedef std::pair<uint64_t, DamageInfo> SpellUniqueTarget;

class SERVER_DECL Spell
{
public:
    Spell(Object* _caster, SpellInfo const* _spellInfo, bool _triggered, Aura* _aura);
    ~Spell();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Main control flow

    // Prepares the spell that's going to cast to targets
    SpellCastResult prepare(SpellCastTargets* targets);
    // Casts the spell
    void castMe(const bool doReCheck);
    void handleHittedTarget(const uint64_t targetGuid, uint8_t effIndex);
    void handleHittedEffect(const uint64_t targetGuid, uint8_t effIndex, int32_t effDamage, bool reCheckTarget = false);
    // Handles missed targets and effects
    void handleMissedTarget(SpellTargetMod const missedTarget);
    void handleMissedEffect(SpellTargetMod const missedTarget, bool reCheckTarget = false);
    // Finishes the casted spell
    void finish(bool successful = true);

    // Update spell state based on time difference
    void update(unsigned long timePassed);

    void cancel();

    int32_t calculateEffect(uint8_t effIndex);
    void calculateJumpSpeeds(Unit* unitCaster, SpellInfo const* spellInfo, uint8_t i, float dist, float& speedXY, float& speedZ);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Spell cast checks

    // Second check in ::cast() should not be as strict as initial check
    virtual SpellCastResult canCast(const bool secondCheck, uint32_t* parameter1, uint32_t* parameter2);
    SpellCastResult checkPower();

private:
    SpellCastResult checkItems(uint32_t* parameter1, uint32_t* parameter2) const;
    SpellCastResult checkCasterState() const;
    SpellCastResult checkRange(const bool secondCheck);
#if VERSION_STRING >= WotLK
    SpellCastResult checkRunes(bool takeRunes);
#endif
    SpellCastResult checkShapeshift(SpellInfo const* spellInfo, const uint32_t shapeshiftForm) const;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Spell packets
    void sendCastResult(SpellCastResult result, uint32_t parameter1 = 0, uint32_t parameter2 = 0);
    // Diff is sent only when channel expires
    // This allows for last periodic tick to happen
    void sendChannelUpdate(const uint32_t time, const uint32_t diff = 0);

private:
    // Spell cast bar packet
    void sendSpellStart();
    // Spell "missile" packet
    void sendSpellGo();
    void sendChannelStart(const uint32_t duration);

    void sendCastResult(Player* caster, uint8_t castCount, SpellCastResult result, uint32_t parameter1, uint32_t parameter2);

    void writeProjectileDataToPacket(WorldPacket* data);
    void writeSpellMissedTargets(WorldPacket* data);

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Cast time
    int32_t getFullCastTime() const;
    int32_t getCastTimeLeft() const;

private:
    int32_t m_castTime = 0;
    int32_t m_timer = 0;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Power
    uint32_t getPowerCost() const;

private:
    void takePower();
    uint32_t calculatePowerCost();

    uint32_t m_powerCost = 0;

    bool m_usesMana = false;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Caster

    Object* getCaster() const;
    Unit* getUnitCaster() const;
    Player* getPlayerCaster() const;
    GameObject* getGameObjectCaster() const;
    Item* getItemCaster() const;

    void setItemCaster(Item* itemCaster);

    bool wasCastedinDuel() const;

protected:
    float_t m_castPositionX = 0.0f;
    float_t m_castPositionY = 0.0f;
    float_t m_castPositionZ = 0.0f;
    float_t m_castPositionO = 0.0f;

    bool duelSpell = false;

    Object* m_caster = nullptr;
    Unit* u_caster = nullptr;
    Player* p_caster = nullptr;
    GameObject* g_caster = nullptr;
    Item* i_caster = nullptr;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Targets
    bool hasTarget(const uint64_t& _guid, std::vector<uint64_t>* tmpGuidMap);

    Item* getItemTarget() const;

    void setUnitTarget(Unit* _unit);
    Unit* getUnitTarget() const;

    Player* getPlayerTarget() const;
    GameObject* getGameObjectTarget() const;
    Corpse* getCorpseTarget() const;

    void unsetAllTargets();

    void setTargetConstraintCreature(Creature* _creature);
    Creature* getTargetConstraintCreature() const;

    void setTargetConstraintGameObject(GameObject* _gameobject);
    GameObject* getTargetConstraintGameObject() const;

    LocationVector getDestination() { return m_targets.getDestination(); }
    LocationVector getSource() { return m_targets.getSource(); }

private:
    // Stores unique hitted targets with DamageInfo for proc system
    std::vector<SpellUniqueTarget> m_uniqueHittedTargets;
    // Stores targets with hit result != SPELL_DID_HIT_SUCCESS
    std::vector<SpellTargetMod> m_missedTargets;
    // Stores hitted targets for each spell effect
    std::vector<uint64_t> m_effectTargets[MAX_SPELL_EFFECTS];

    SpellCastResult checkExplicitTarget(Object* target, uint32_t requiredTargetMask) const;
    void safeAddMissedTarget(uint64_t targetGuid, SpellDidHitResult hitResult, SpellDidHitResult extendedHitResult);

    Unit* m_unitTarget = nullptr;
    Item* m_itemTarget = nullptr;
    GameObject* m_gameObjTarget = nullptr;
    Player* m_playerTarget = nullptr;
    Corpse* m_corpseTarget = nullptr;

protected:
    // Current Targets to be used in effect handler
    Creature* m_targetConstraintCreature = nullptr;
    GameObject* m_targetConstraintGameObject = nullptr;

    SpellTargetConstraint const* m_targetConstraint;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // SpellInfo
    SpellInfo const* getSpellInfo() const;

    // helper
    bool hasAttribute(SpellAttributes _attribute) const;
    bool hasAttributeEx(SpellAttributesEx _attribute) const;
    bool hasAttributeExB(SpellAttributesExB _attribute) const;
    bool hasAttributeExC(SpellAttributesExC _attribute) const;
    bool hasAttributeExD(SpellAttributesExD _attribute) const;
    bool hasAttributeExE(SpellAttributesExE _attribute) const;
    bool hasAttributeExF(SpellAttributesExF _attribute) const;
    bool hasAttributeExG(SpellAttributesExG _attribute) const;

    // LuaEngine
    void resetSpellInfoOverride();

private:
    SpellInfo const* m_spellInfo = nullptr;
    // used by spells that should have dynamic variables in spellentry
    // seems to be used only by LuaEngine -Appled
    SpellInfo const* m_spellInfo_override = nullptr;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    // Some spells inherit base points from the mother spell
    std::shared_ptr<SpellForcedBasePoints> forced_basepoints;

    Aura* getTriggeredByAura() const;

    // Returns how many combo points this spell used
    int8_t getUsedComboPoints() const;

    void addUsedSpellModifier(AuraEffectModifier const* aurEff);
    void removeUsedSpellModifier(AuraEffectModifier const* aurEff);
    void takeUsedSpellModifiers();

    // If called from spell scripts, this needs to be called either in
    // doBeforeEffectHit, doCalculateEffect or beforeSpellEffect script hooks to have any effect
    void setForceCritOnTarget(Unit const* target);

    int32_t getDuration();
    float_t getEffectRadius(uint8_t effectIndex);

private:
    struct HitSpellEffect
    {
        uint32_t travelTime = 0;
        uint8_t effIndex = 0;
        int32_t damage = 0;
    };

    struct HitAuraEffect
    {
        uint32_t travelTime = 0;
        std::unique_ptr<Aura> aur;
    };

    struct MissSpellEffect
    {
        uint32_t travelTime = 0;
        SpellTargetMod missInfo = SpellTargetMod(0, SPELL_DID_HIT_SUCCESS, SPELL_DID_HIT_SUCCESS);
    };

    bool canAttackCreatureType(Creature* target) const;

    // Removes used item and/or item charges
    void removeCastItem();
    void removeReagents();
#if VERSION_STRING < Cata
    void removeAmmo();
#endif

    void _updateCasterPointers(Object* caster);
    void _updateTargetPointers(const uint64_t targetGuid);
    // Loads initial target pointers from spell's SpellCastTargets
    // Used only in spell cast check phase, proper targets are set in spell cast phase
    void _loadInitialTargetPointers(bool reset = false);
    float_t _getSpellTravelTimeForTarget(uint64_t guid) const;

    // Spell reflect stuff
    bool m_canBeReflected = false;

    bool m_requiresCP = false;
    int8_t m_usedComboPoints = 0;

    int32_t m_duration = 0;
    bool isDurationSet = false;

    float_t m_effectRadius[MAX_SPELL_EFFECTS] = { 0.0f };
    bool m_isEffectRadiusSet[MAX_SPELL_EFFECTS] = { false };

    // Spell proc
    DamageInfo m_casterDamageInfo = DamageInfo();
    DamageInfo m_targetDamageInfo = DamageInfo();
    bool isTargetDamageInfoSet = false;
    uint32_t m_casterProcFlags = 0;
    uint32_t m_targetProcFlags = 0;
    void _prepareProcFlags();
    // Stores guids of targets who have handled procs on caster
    std::set<uint64_t> m_doneTargetProcs;

#ifdef _MSC_VER
    // Need a dummy variable to fix build on MSVC... -Appled
    std::unique_ptr<Aura> dummyUnusedVariableCompileFix_DoNotUse;
#endif
    std::map<uint64_t, HitAuraEffect> m_pendingAuras;
    std::map<uint64_t, HitSpellEffect> m_hitEffects;
    std::map<uint64_t, MissSpellEffect> m_missEffects;
    std::vector<uint64_t> m_critTargets;

    bool isForcedCrit = false;
    bool isEffectDamageStatic[MAX_SPELL_EFFECTS];
    float_t effectPctModifier[MAX_SPELL_EFFECTS];

    std::map<AuraEffectModifier const*, bool> m_usedModifiers;

    SpellState m_spellState = SPELL_STATE_NULL;
    SpellCastResult cancastresult = SPELL_CAST_SUCCESS;

    bool m_triggeredSpell = false;
    Aura* m_triggeredByAura = nullptr;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Spell effects (defined in SpellEffects.cpp)

    // Used with effects that are not implemented yet
    void spellEffectNotImplemented(uint8_t effectIndex);
    // Used with effects that are not used or are handled elsewhere
    void spellEffectNotUsed(uint8_t effectIndex);

    void spellEffectInstantKill(uint8_t effectIndex);
    void spellEffectSchoolDMG(uint8_t effectIndex);
    void spellEffectDummy(uint8_t effectIndex);
    void spellEffectTeleportUnits(uint8_t effectIndex);
    void spellEffectApplyAura(uint8_t effectIndex);
    void spellEffectEnvironmentalDamage(uint8_t effectIndex);
    void spellEffectPowerDrain(uint8_t effectIndex);
    void spellEffectHealthLeech(uint8_t effectIndex);
    void spellEffectHeal(uint8_t effectIndex);
    void spellEffectBind(uint8_t effectIndex);
    void spellEffectQuestComplete(uint8_t effectIndex);
    void spellEffectWeapondamageNoschool(uint8_t effectIndex);
    void spellEffectResurrect(uint8_t effectIndex);
    void spellEffectAddExtraAttacks(uint8_t effectIndex);
    void spellEffectDodge(uint8_t effectIndex);
    void spellEffectParry(uint8_t effectIndex);
    void spellEffectBlock(uint8_t effectIndex);
    void spellEffectCreateItem(uint8_t effectIndex);
    void spellEffectWeapon(uint8_t effectIndex);
    void spellEffectDefense(uint8_t effectIndex);
    void spellEffectPersistentAA(uint8_t effectIndex);
    void spellEffectSummon(uint8_t effectIndex);
    void spellEffectSummonWild(uint8_t effectIndex);
    void spellEffectSummonGuardian(uint8_t effectIndex, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector& v);
    void spellEffectSummonTemporaryPet(uint8_t effectIndex, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector& v);
    void spellEffectSummonTotem(uint8_t effectIndex, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties, LocationVector& v);
    void spellEffectSummonPossessed(uint8_t effectIndex, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector& v);
    void spellEffectSummonCompanion(uint8_t effectIndex, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector& v);
    void spellEffectSummonVehicle(uint8_t effectIndex, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector& v);
    void spellEffectLeap(uint8_t effectIndex);
    void spellEffectEnergize(uint8_t effectIndex);
    void spellEffectWeaponDmgPerc(uint8_t effectIndex);
    void spellEffectTriggerMissile(uint8_t effectIndex);
    void spellEffectOpenLock(uint8_t effectIndex);
    void spellEffectTransformItem(uint8_t effectIndex);
    void spellEffectApplyGroupAA(uint8_t effectIndex);
    void spellEffectLearnSpell(uint8_t effectIndex);
    void spellEffectSpellDefense(uint8_t effectIndex);
    void spellEffectDispel(uint8_t effectIndex);
    void spellEffectDualWield(uint8_t effectIndex);
    void spellEffectSkillStep(uint8_t effectIndex);
    void spellEffectAddHonor(uint8_t effectIndex);
    void spellEffectSpawn(uint8_t effectIndex);
    void spellEffectSummonObject(uint8_t effectIndex);
    void spellEffectEnchantItem(uint8_t effectIndex);
    void spellEffectEnchantItemTemporary(uint8_t effectIndex);
    void spellEffectTameCreature(uint8_t effectIndex);
    void spellEffectSummonPet(uint8_t effectIndex);
    void spellEffectLearnPetSpell(uint8_t effectIndex);
    void spellEffectWeapondamage(uint8_t effectIndex);
    void spellEffectOpenLockItem(uint8_t effectIndex);
    void spellEffectProficiency(uint8_t effectIndex);
    void spellEffectSendEvent(uint8_t effectIndex);
    void spellEffectPowerBurn(uint8_t effectIndex);
    void spellEffectThreat(uint8_t effectIndex);
    void spellEffectClearQuest(uint8_t effectIndex);
    void spellEffectForceCast(uint8_t effectIndex);
    void spellEffectTriggerSpell(uint8_t effectIndex);
    void spellEffectApplyRaidAA(uint8_t effectIndex);
    void spellEffectPowerFunnel(uint8_t effectIndex);
    void spellEffectHealMaxHealth(uint8_t effectIndex);
    void spellEffectInterruptCast(uint8_t effectIndex);
    void spellEffectDistract(uint8_t effectIndex);
    void spellEffectPickpocket(uint8_t effectIndex);
    void spellEffectAddFarsight(uint8_t effectIndex);
    void spellEffectUseGlyph(uint8_t effectIndex);
    void spellEffectHealMechanical(uint8_t effectIndex);
    void spellEffectSummonObjectWild(uint8_t effectIndex);
    void spellEffectScriptEffect(uint8_t effectIndex);
    void spellEffectSanctuary(uint8_t effectIndex);
    void spellEffectAddComboPoints(uint8_t effectIndex);
    void spellEffectCreateHouse(uint8_t effectIndex);
    void spellEffectDuel(uint8_t effectIndex);
    void spellEffectStuck(uint8_t effectIndex);
    void spellEffectSummonPlayer(uint8_t effectIndex);
    void spellEffectActivateObject(uint8_t effectIndex);
    void spellEffectBuildingDamage(uint8_t effectIndex);
    void spellEffectEnchantHeldItem(uint8_t effectIndex);
    void spellEffectSetMirrorName(uint8_t effectIndex);
    void spellEffectSelfResurrect(uint8_t effectIndex);
    void spellEffectSkinning(uint8_t effectIndex);
    void spellEffectCharge(uint8_t effectIndex);
    void spellEffectKnockBack(uint8_t effectIndex);
    void spellEffectKnockBack2(uint8_t effectIndex);
    void spellEffectDisenchant(uint8_t effectIndex);
    void spellEffectInebriate(uint8_t effectIndex);
    void spellEffectFeedPet(uint8_t effectIndex);
    void spellEffectDismissPet(uint8_t effectIndex);
    void spellEffectReputation(uint8_t effectIndex);
    void spellEffectSummonObjectSlot(uint8_t effectIndex);
    void spellEffectDispelMechanic(uint8_t effectIndex);
    void spellEffectSummonDeadPet(uint8_t effectIndex);
    void spellEffectDestroyAllTotems(uint8_t effectIndex);
    void spellEffectDurabilityDamage(uint8_t effectIndex);
    void spellEffectDurabilityDamagePCT(uint8_t effectIndex);
    void spellEffectResurrectNew(uint8_t effectIndex);
    void spellEffectAttackMe(uint8_t effectIndex);
    void spellEffectSkinPlayerCorpse(uint8_t effectIndex);
    void spellEffectSkill(uint8_t effectIndex);
    void spellEffectApplyPetAA(uint8_t effectIndex);
    void spellEffectDummyMelee(uint8_t effectIndex);
    void spellEffectStartTaxi(uint8_t effectIndex);
    void spellEffectPlayerPull(uint8_t effectIndex);
    void spellEffectReduceThreatPercent(uint8_t effectIndex);
    void spellEffectSpellSteal(uint8_t effectIndex);
    void spellEffectProspecting(uint8_t effectIndex);
    void spellEffectApplyFriendAA(uint8_t effectIndex);
    void spellEffectApplyEnemyAA(uint8_t effectIndex);
    void spellEffectRedirectThreat(uint8_t effectIndex);
    void spellEffectPlayMusic(uint8_t effectIndex);
    void spellEffectForgetSpecialization(uint8_t effectIndex);
    void spellEffectKillCredit(uint8_t effectIndex);
    void spellEffectRestorePowerPct(uint8_t effectIndex);
    void spellEffectTriggerSpellWithValue(uint8_t effectIndex);
    void spellEffectApplyOwnerAA(uint8_t effectIndex);
    void spellEffectCreatePet(uint8_t effectIndex);
    void spellEffectTeachTaxiPath(uint8_t effectIndex);
    void spellEffectDualWield2H(uint8_t effectIndex);
    void spellEffectEnchantItemPrismatic(uint8_t effectIndex);
    void spellEffectCreateItem2(uint8_t effectIndex);
    void spellEffectMilling(uint8_t effectIndex);
    void spellEffectRenamePet(uint8_t effectIndex);
    void spellEffectRestoreHealthPct(uint8_t effectIndex);
    void spellEffectLearnSpec(uint8_t effectIndex);
    void spellEffectActivateSpec(uint8_t effectIndex);
    void spellEffectActivateRunes(uint8_t effectIndex);
    void spellEffectJumpTarget(uint8_t effectIndex);
    void spellEffectJumpBehindTarget(uint8_t effectIndex);

public:
    friend class DummySpellHandler;

    // Fills specified targets at the area of effect
    void FillSpecifiedTargetsInArea(float srcx, float srcy, float srcz, uint32_t ind, uint32_t specification);
    // Fills specified targets at the area of effect. We suppose we already inited this spell and know the details
    void FillSpecifiedTargetsInArea(uint32_t i, float srcx, float srcy, float srcz, float range, uint32_t specification);
    // Fills the targets at the area of effect
    void FillAllTargetsInArea(uint32_t i, float srcx, float srcy, float srcz, float range);
    // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
    void FillAllTargetsInArea(float srcx, float srcy, float srcz, uint32_t ind);
    // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
    void FillAllTargetsInArea(LocationVector& location, uint32_t ind);
    // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
    void FillAllFriendlyInArea(uint32_t i, float srcx, float srcy, float srcz, float range);
    //get single Enemy as target
    uint64_t GetSinglePossibleEnemy(uint32_t i, float prange = 0);
    //get single Enemy as target
    uint64_t GetSinglePossibleFriend(uint32_t i, float prange = 0);
    //generate possible target list for a spell. Use as last resort since it is not accurate
    bool GenerateTargets(SpellCastTargets* store_buff);
    // Fills the target map of the spell packet
    void FillTargetMap(uint32_t);

    void HandleTargetNoObject();

    // See if we hit the target or can it resist (evade/immune/resist on spellgo) (0=success)
    uint8_t DidHit(uint32_t effindex, Unit* target);
    // Casts the spell
    void castMeOld();

    // Trigger Spell function that triggers triggered spells
    //void TriggerSpell();

    // Checks the caster is ready for cast
    uint8_t CanCast(bool);

    // Handles Teleport function
    void HandleTeleport(LocationVector position, uint32_t mapid, Unit* Target);
    // Determines how much skill caster going to gain
    void DetermineSkillUp();
    // Increases cast time of the spell
    void AddTime(uint32_t type);

    uint32_t getState() const;

    // Send Packet functions
    void SendLogExecute(uint32_t damage, uint64_t& targetGuid);
    void SendInterrupted(uint8_t result);
    void SendResurrectRequest(Player* target);
    void SendTameFailure(uint8_t failure);

    void HandleAddAura(uint64_t guid);
    uint32_t pSpellId;
    SpellInfo const* ProcedOnSpell;
    SpellCastTargets m_targets;

    void CreateItem(uint32_t itemId);

    // Effect Handlers for effectIndex
    void ApplyAreaAura(uint8_t effectIndex);

    void SpellEffectInstantKill(uint8_t effectIndex);
    void SpellEffectSchoolDMG(uint8_t effectIndex);
    void SpellEffectTeleportUnits(uint8_t effectIndex);
    void SpellEffectApplyAura(uint8_t effectIndex);
    void SpellEffectEnvironmentalDamage(uint8_t effectIndex);
    void SpellEffectPowerDrain(uint8_t effectIndex);
    void SpellEffectHeal(uint8_t effectIndex);
    void SpellEffectBind(uint8_t effectIndex);
    void SpellEffectQuestComplete(uint8_t effectIndex);
    void SpellEffectWeapondamageNoschool(uint8_t effectIndex);
    void SpellEffectResurrect(uint8_t effectIndex);
    void SpellEffectAddExtraAttacks(uint8_t effectIndex);
    void SpellEffectDodge(uint8_t effectIndex);
    void SpellEffectParry(uint8_t effectIndex);
    void SpellEffectBlock(uint8_t effectIndex);
    void SpellEffectCreateItem(uint8_t effectIndex);
    void SpellEffectPersistentAA(uint8_t effectIndex);

    virtual void SpellEffectSummon(uint8_t effectIndex);
    void SpellEffectSummonWild(uint8_t effectIndex);
    void SpellEffectSummonGuardian(uint32_t i, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector& v);
    void SpellEffectSummonTemporaryPet(uint32_t i, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector& v);
    void SpellEffectSummonPossessed(uint32_t i, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector& v);
    void SpellEffectSummonCompanion(uint32_t i, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector& v);
    void SpellEffectSummonVehicle(uint32_t i, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector& v);
    void SpellEffectLeap(uint8_t effectIndex);
    void SpellEffectEnergize(uint8_t effectIndex);
    void SpellEffectWeaponDmgPerc(uint8_t effectIndex);
    void SpellEffectTriggerMissile(uint8_t effectIndex);
    void SpellEffectOpenLock(uint8_t effectIndex);
    void SpellEffectTransformItem(uint8_t effectIndex);
    void SpellEffectApplyGroupAA(uint8_t effectIndex);
    void SpellEffectLearnSpell(uint8_t effectIndex);
    void SpellEffectSpellDefense(uint8_t effectIndex);
    void SpellEffectDispel(uint8_t effectIndex);
    void SpellEffectAddHonor(uint8_t effectIndex);
    void SpellEffectSpawn(uint8_t effectIndex);
    void SpellEffectSummonObject(uint8_t effectIndex);
    void SpellEffectEnchantItem(uint8_t effectIndex);
    void SpellEffectEnchantItemTemporary(uint8_t effectIndex);
    void SpellEffectTameCreature(uint8_t effectIndex);
    void SpellEffectSummonPet(uint8_t effectIndex);
    void SpellEffectLearnPetSpell(uint8_t effectIndex);
    void SpellEffectWeapondamage(uint8_t effectIndex);
    void SpellEffectOpenLockItem(uint8_t effectIndex);
    void SpellEffectSendEvent(uint8_t effectIndex);
    void SpellEffectPowerBurn(uint8_t effectIndex);
    void SpellEffectThreat(uint8_t effectIndex);
    void SpellEffectClearQuest(uint8_t effectIndex);
    void SpellEffectApplyRaidAA(uint8_t effectIndex);
    void SpellEffectPowerFunnel(uint8_t effectIndex);
    void SpellEffectHealMaxHealth(uint8_t effectIndex);
    void SpellEffectInterruptCast(uint8_t effectIndex);
    void SpellEffectDistract(uint8_t effectIndex);
    void SpellEffectPickpocket(uint8_t effectIndex);
    void SpellEffectAddFarsight(uint8_t effectIndex);
    void SpellEffectUseGlyph(uint8_t effectIndex);
    void SpellEffectHealMechanical(uint8_t effectIndex);
    void SpellEffectSummonObjectWild(uint8_t effectIndex);
    void SpellEffectSanctuary(uint8_t effectIndex);
    void SpellEffectAddComboPoints(uint8_t effectIndex);
    void SpellEffectCreateHouse(uint8_t effectIndex);
    void SpellEffectDuel(uint8_t effectIndex);
    void SpellEffectStuck(uint8_t effectIndex);
    void SpellEffectSummonPlayer(uint8_t effectIndex);
    void SpellEffectActivateObject(uint8_t effectIndex);
    void SpellEffectBuildingDamage(uint8_t effectIndex);
    void SpellEffectEnchantHeldItem(uint8_t effectIndex);
    void SpellEffectForceDeselect(uint8_t effectIndex);
    void SpellEffectSelfResurrect(uint8_t effectIndex);
    void SpellEffectSkinning(uint8_t effectIndex);
    void SpellEffectCharge(uint8_t effectIndex);
    void SpellEffectKnockBack(uint8_t effectIndex);
    void SpellEffectKnockBack2(uint8_t effectIndex);
    void SpellEffectPullTowardsDest(uint8_t effectIndex);
    void SpellEffectDisenchant(uint8_t effectIndex);
    void SpellEffectInebriate(uint8_t effectIndex);
    void SpellEffectFeedPet(uint8_t effectIndex);
    void SpellEffectDismissPet(uint8_t effectIndex);
    void SpellEffectReputation(uint8_t effectIndex);
    void SpellEffectSummonObjectSlot(uint8_t effectIndex);
    void SpellEffectDispelMechanic(uint8_t effectIndex);
    void SpellEffectSummonDeadPet(uint8_t effectIndex);
    void SpellEffectDestroyAllTotems(uint8_t effectIndex);
    void SpellEffectDurabilityDamage(uint8_t effectIndex);
    void SpellEffectDurabilityDamagePCT(uint8_t effectIndex);
    void SpellEffectResurrectNew(uint8_t effectIndex);
    void SpellEffectAttackMe(uint8_t effectIndex);
    void SpellEffectSkinPlayerCorpse(uint8_t effectIndex);
    void SpellEffectApplyPetAA(uint8_t effectIndex);
    void SpellEffectDummyMelee(uint8_t effectIndex);
    void SpellEffectStartTaxi(uint8_t effectIndex);
    void SpellEffectPlayerPull(uint8_t effectIndex);
    void SpellEffectReduceThreatPercent(uint8_t effectIndex);
    void SpellEffectSpellSteal(uint8_t effectIndex);
    void SpellEffectProspecting(uint8_t effectIndex);
    void SpellEffectApplyFriendAA(uint8_t effectIndex);
    void SpellEffectApplyEnemyAA(uint8_t effectIndex);
    void SpellEffectRedirectThreat(uint8_t effectIndex);
    void SpellEffectPlayMusic(uint8_t effectIndex);
    void SpellEffectForgetSpecialization(uint8_t effectIndex);
    void SpellEffectKillCredit(uint8_t effectIndex);
    void SpellEffectRestorePowerPct(uint8_t effectIndex);
    void SpellEffectTriggerSpellWithValue(uint8_t effectIndex);
    void SpellEffectApplyOwnerAA(uint8_t effectIndex);
    void SpellEffectCreatePet(uint8_t effectIndex);
    void SpellEffectTeachTaxiPath(uint8_t effectIndex);
    void SpellEffectDualWield2H(uint8_t effectIndex);
    void SpellEffectEnchantItemPrismatic(uint8_t effectIndex);
    void SpellEffectCreateItem2(uint8_t effectIndex);
    void SpellEffectMilling(uint8_t effectIndex);
    void SpellEffectRenamePet(uint8_t effectIndex);
    void SpellEffectRestoreHealthPct(uint8_t effectIndex);
    void SpellEffectLearnSpec(uint8_t effectIndex);
    void SpellEffectActivateSpec(uint8_t effectIndex);
    void SpellEffectActivateRunes(uint8_t effectIndex);
    void SpellEffectJumpTarget(uint8_t effectIndex);
    void SpellEffectJumpBehindTarget(uint8_t effectIndex);

    // This returns SPELL_ENTRY_Spell_Dmg_Type where 0 = SPELL_DMG_TYPE_NONE, 1 = SPELL_DMG_TYPE_MAGIC, 2 = SPELL_DMG_TYPE_MELEE, 3 = SPELL_DMG_TYPE_RANGED
    // It should NOT be used for weapon_damage_type which needs: 0 = MELEE, 1 = OFFHAND, 2 = RANGED
    uint32_t GetType();

    uint32_t chaindamage;

    bool IsAspect();
    bool IsSeal();

    void InitProtoOverride();

    static uint32_t GetBaseThreat(uint32_t dmg);

    static uint32_t GetMechanic(SpellInfo const* sp);

    bool IsStealthSpell();
    bool IsInvisibilitySpell();

    int32_t damage;
    bool m_AreaAura;
    int32_t m_charges;

    int32_t damageToHit;
    uint32_t castedItemId;
    uint8_t extra_cast_number;
    uint32_t m_glyphslot;

    //////////////////////////////////////////////////////////////////////////////////////////
    ///bool DuelSpellNoMoreValid()
    /// Tells if the Spell was being casted while dueling but now the duel is over
    ///
    /// \return true if Spell is now invalid because the duel is over false if Spell is valid.
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    bool DuelSpellNoMoreValid() const;

    /// Spell state's
    /// Spell failed
    bool GetSpellFailed() const;
    void SetSpellFailed(bool failed = true);

protected:
    /// Spell state's
    bool m_Spell_Failed;            // for 5sr
    bool m_Delayed;
    uint8_t m_DelayStep;            // 3.0.2 - spells can only be delayed twice.

    bool m_IsCastedOnSelf;

    uint64_t m_magnetTarget;

    uint32_t add_damage;

    uint8_t m_rune_avail_before;
    //void _DamageRangeUpdate();

    virtual int32_t DoCalculateEffect(uint32_t i, Unit* target, int32_t value);
    virtual void DoAfterHandleEffect(Unit* target, uint32_t i);

public: // Modified by LUAppArc private->public
    float m_missilePitch;
    uint32_t m_missileTravelTime;

    void SafeAddTarget(std::vector<uint64_t>* tgt, uint64_t guid);

    friend class DynamicObject;
    void DetermineSkillUp(uint16_t skillid, uint32_t targetlevel, uint32_t multiplicator = 1);
    void DetermineSkillUp(uint16_t skillid);

    bool AddTarget(uint32_t i, uint32_t TargetType, Object* obj);
    void AddAOETargets(uint32_t i, uint32_t TargetType, float r, uint32_t maxtargets);
    void AddPartyTargets(uint32_t i, uint32_t TargetType, float r, uint32_t maxtargets);
    void AddRaidTargets(uint32_t i, uint32_t TargetType, float r, uint32_t maxtargets, bool partylimit = false);
    void AddChainTargets(uint32_t i, uint32_t TargetType, float r, uint32_t maxtargets);
    void AddConeTargets(uint32_t i, uint32_t TargetType, float r, uint32_t maxtargets);
    void AddScriptedOrSpellFocusTargets(uint32_t i, uint32_t TargetType, float r, uint32_t maxtargets);
};
