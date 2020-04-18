/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

#pragma once

#include "SpellCastTargets.h"
#include "Definitions/SpellTargetMod.h"
#include "Spell/SpellInfo.hpp"
#include "Definitions/SpellFailure.h"
#include "Units/Creatures/Creature.h"
#include "Units/Players/Player.h"
#include "Units/Unit.h"
#include "SpellTargetConstraint.h"

class WorldSession;
class Unit;
class DynamicObj;
class Player;
class Item;
class Group;
class Aura;
class DummySpellHandler;

typedef void(Spell::*pSpellEffect)(uint8_t effectIndex);
typedef void(Spell::*pSpellTarget)(uint32_t i, uint32_t j);

#define GO_FISHING_BOBBER 35591
#define SPELL_SPELL_CHANNEL_UPDATE_INTERVAL 1000

class SERVER_DECL Spell : public EventableObject
{
    public:
        // APGL Ends
        // MIT Starts

        //////////////////////////////////////////////////////////////////////////////////////////
        // Main control flow

        // Prepares the spell that's going to cast to targets
        SpellCastResult prepare(SpellCastTargets* targets);
        // Casts the spell
        void castMe(const bool doReCheck);
        // Handles missed targets and effects
        void handleMissedTarget(SpellTargetMod const missedTarget);
        void handleMissedEffect(const uint64_t targetGuid);

        // Update spell state based on time difference
        void Update(unsigned long timePassed);

        //////////////////////////////////////////////////////////////////////////////////////////
        // Spell cast checks

        // Second check in ::cast() should not be as strict as initial check
        virtual SpellCastResult canCast(const bool secondCheck, uint32_t* parameter1, uint32_t* parameter2);
        SpellCastResult checkPower() const;

    private:
        SpellCastResult checkItems(uint32_t* parameter1, uint32_t* parameter2) const;
        SpellCastResult checkCasterState() const;
        SpellCastResult checkRange(const bool secondCheck) const;
#if VERSION_STRING >= WotLK
        SpellCastResult checkRunes(bool takeRunes) const;
#endif
        SpellCastResult checkShapeshift(SpellInfo const* spellInfo, const uint32_t shapeshiftForm) const;

    public:
        //////////////////////////////////////////////////////////////////////////////////////////
        // Spell packets
        void sendCastResult(SpellCastResult result, uint32_t parameter1 = 0, uint32_t parameter2 = 0);
        void sendChannelUpdate(const uint32_t time);

    private:
        // Spell cast bar packet
        void sendSpellStart();
        // Spell "missile" packet
        void sendSpellGo();
        void sendChannelStart(const uint32_t duration);

        void sendCastResult(Player* caster, uint8_t castCount, SpellCastResult result, uint32_t parameter1, uint32_t parameter2);

        void writeProjectileDataToPacket(WorldPacket *data);
        void writeSpellMissedTargets(WorldPacket *data);

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
        uint32_t calculatePowerCost() const;

        uint32_t m_powerCost = 0;

    public:
        //////////////////////////////////////////////////////////////////////////////////////////
        // Caster

    private:
        float m_castPositionX = 0.0f;
        float m_castPositionY = 0.0f;
        float m_castPositionZ = 0.0f;
        float m_castPositionO = 0.0f;

    public:
        //////////////////////////////////////////////////////////////////////////////////////////
        // Targets

    private:
        // Stores unique hitted targets
        std::vector<uint64_t> uniqueHittedTargets;
        // Stores targets with hit result != SPELL_DID_HIT_SUCCESS
        std::vector<SpellTargetMod> missedTargets;
        // Stores hitted targets for each spell effect
        std::vector<uint64_t> m_effectTargets[MAX_SPELL_EFFECTS];

    public:
        //////////////////////////////////////////////////////////////////////////////////////////
        // Misc
        SpellInfo const* getSpellInfo() const;

    private:
        bool canAttackCreatureType(Creature* target) const;

        void removeReagents();
#if VERSION_STRING < Cata
        void removeAmmo();
#endif

        // Spell reflect stuff
        bool m_canBeReflected = false;

    public:
        // MIT Ends
        // APGL Starts
        friend class DummySpellHandler;
        Spell(Object* Caster, SpellInfo* info, bool triggered, Aura* aur);
        ~Spell();

        int32_t event_GetInstanceID() override;

        bool m_overrideBasePoints;
        uint32_t m_overridenBasePoints[3];

        // Fills specified targets at the area of effect
        void FillSpecifiedTargetsInArea(float srcx, float srcy, float srcz, uint32_t ind, uint32_t specification);
        // Fills specified targets at the area of effect. We suppose we already inited this spell and know the details
        void FillSpecifiedTargetsInArea(uint32_t i, float srcx, float srcy, float srcz, float range, uint32_t specification);
        // Fills the targets at the area of effect
        void FillAllTargetsInArea(uint32_t i, float srcx, float srcy, float srcz, float range);
        // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
        void FillAllTargetsInArea(float srcx, float srcy, float srcz, uint32_t ind);
        // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
        void FillAllTargetsInArea(LocationVector & location, uint32_t ind);
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
        // Cancels the current spell
        void cancel();
        // Casts the spell
        void castMeOld();
        // Finishes the casted spell
        void finish(bool successful = true);
        // Handle the Effects of the Spell
        virtual void HandleEffects(uint64_t guid, uint32_t i);
        void HandleCastEffects(uint64_t guid, uint32_t i);

        // Trigger Spell function that triggers triggered spells
        //void TriggerSpell();

        // Checks the caster is ready for cast
        uint8_t CanCast(bool);

        bool hasAttribute(SpellAttributes attribute);
        bool hasAttributeEx(SpellAttributesEx attribute);
        bool hasAttributeExB(SpellAttributesExB attribute);
        bool hasAttributeExC(SpellAttributesExC attribute);
        bool hasAttributeExD(SpellAttributesExD attribute);
        bool hasAttributeExE(SpellAttributesExE attribute);
        bool hasAttributeExF(SpellAttributesExF attribute);
        bool hasAttributeExG(SpellAttributesExG attribute);

        // Removes reagents, ammo, and items/charges
        void RemoveItems();
        // Calculates the i'th effect value
        int32_t CalculateEffect(uint32_t, Unit* target);
        // Handles Teleport function
        void HandleTeleport(float x, float y, float z, uint32_t mapid, Unit* Target);
        // Determines how much skill caster going to gain
        void DetermineSkillUp();
        // Increases cast time of the spell
        void AddTime(uint32_t type);

        uint32_t getState() const;
        void SetUnitTarget(Unit* punit);
        void SetTargetConstraintCreature(Creature* pCreature);
        void SetTargetConstraintGameObject(GameObject* pGameobject);
        Creature* GetTargetConstraintCreature() const;
        GameObject* GetTargetConstraintGameObject() const;

        // Send Packet functions
        void SendLogExecute(uint32_t damage, uint64_t & targetGuid);
        void SendInterrupted(uint8_t result);
        void SendResurrectRequest(Player* target);
        void SendTameFailure(uint8_t failure);
        static void SendHealSpellOnPlayer(Object* caster, Object* target, uint32_t healed, bool critical, uint32_t overhealed, uint32_t spellid, uint32_t absorbed = 0);

        void HandleAddAura(uint64_t guid);
        void writeSpellGoTargets(WorldPacket* data);
        uint32_t pSpellId;
        SpellInfo const* ProcedOnSpell;
        SpellCastTargets m_targets;

        void CreateItem(uint32_t itemId);

        // Effect Handlers for effectIndex
        void SpellEffectUnused(uint8_t effectIndex);

        void ApplyAreaAura(uint8_t effectIndex);

        void SpellEffectNULL(uint8_t effectIndex);
        void SpellEffectInstantKill(uint8_t effectIndex);
        void SpellEffectSchoolDMG(uint8_t effectIndex);
        void SpellEffectDummy(uint8_t effectIndex);
        void SpellEffectTeleportUnits(uint8_t effectIndex);
        void SpellEffectApplyAura(uint8_t effectIndex);
        void SpellEffectEnvironmentalDamage(uint8_t effectIndex);
        void SpellEffectPowerDrain(uint8_t effectIndex);
        void SpellEffectHealthLeech(uint8_t effectIndex);
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
        void SpellEffectWeapon(uint8_t effectIndex);
        void SpellEffectDefense(uint8_t effectIndex);
        void SpellEffectPersistentAA(uint8_t effectIndex);

        virtual void SpellEffectSummon(uint8_t effectIndex);
        void SpellEffectSummonWild(uint8_t effectIndex);
        void SpellEffectSummonGuardian(uint32_t i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonTemporaryPet(uint32_t i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonTotem(uint32_t i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonPossessed(uint32_t i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonCompanion(uint32_t i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonVehicle(uint32_t i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
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
        void SpellEffectLanguage(uint8_t effectIndex);
        void SpellEffectDualWield(uint8_t effectIndex);
        void SpellEffectSkillStep(uint8_t effectIndex);
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
        void SpellEffectProficiency(uint8_t effectIndex);
        void SpellEffectSendEvent(uint8_t effectIndex);
        void SpellEffectPowerBurn(uint8_t effectIndex);
        void SpellEffectThreat(uint8_t effectIndex);
        void SpellEffectClearQuest(uint8_t effectIndex);
        void SpellEffectTriggerSpell(uint8_t effectIndex);
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
        void SpellEffectScriptEffect(uint8_t effectIndex);
        void SpellEffectSanctuary(uint8_t effectIndex);
        void SpellEffectAddComboPoints(uint8_t effectIndex);
        void SpellEffectCreateHouse(uint8_t effectIndex);
        void SpellEffectDuel(uint8_t effectIndex);
        void SpellEffectStuck(uint8_t effectIndex);
        void SpellEffectSummonPlayer(uint8_t effectIndex);
        void SpellEffectActivateObject(uint8_t effectIndex);
        void SpellEffectBuildingDamage(uint8_t effectIndex);
        void SpellEffectEnchantHeldItem(uint8_t effectIndex);
        void SpellEffectSetMirrorName(uint8_t effectIndex);
        void SpellEffectSelfResurrect(uint8_t effectIndex);
        void SpellEffectSkinning(uint8_t effectIndex);
        void SpellEffectCharge(uint8_t effectIndex);
        void SpellEffectKnockBack(uint8_t effectIndex);
        void SpellEffectKnockBack2(uint8_t effectIndex);
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
        void SpellEffectSkill(uint8_t effectIndex);
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

        void Heal(int32_t amount, bool ForceCrit = false);

        GameObject*     g_caster;
        Unit*           u_caster;
        Item*           i_caster;
        Player*         p_caster;
        Object*         m_caster;

        // 15007 = resurrection sickness

        // This returns SPELL_ENTRY_Spell_Dmg_Type where 0 = SPELL_DMG_TYPE_NONE, 1 = SPELL_DMG_TYPE_MAGIC, 2 = SPELL_DMG_TYPE_MELEE, 3 = SPELL_DMG_TYPE_RANGED
        // It should NOT be used for weapon_damage_type which needs: 0 = MELEE, 1 = OFFHAND, 2 = RANGED
        uint32_t GetType();

        std::map<uint64_t, Aura*> m_pendingAuras;

        Item* GetItemTarget() const;
        Unit* GetUnitTarget() const;
        Player* GetPlayerTarget() const;
        GameObject* GetGameObjectTarget() const;
        Corpse* GetCorpseTarget() const;

        uint32_t chaindamage;
        // -------------------------------------------

        bool IsAspect();
        bool IsSeal();

        void InitProtoOverride();

        uint32_t GetDuration();

        float GetRadius(uint32_t i);

        static uint32_t GetBaseThreat(uint32_t dmg);

        static uint32_t GetMechanic(SpellInfo* sp);

        bool IsStealthSpell();
        bool IsInvisibilitySpell();

        int32_t damage;
        Aura* m_triggeredByAura;
        signed int  forced_basepoints[3]; //some talent inherit base points from previous caster spells

        bool m_triggeredSpell;
        bool m_AreaAura;
        //uint32_t TriggerSpellId;  // used to set next spell to use
        //uint64_t TriggerSpellTarget; // used to set next spell target
        bool m_requiresCP;
        int32_t m_charges;

        int32_t damageToHit;
        uint32_t castedItemId;
        uint8_t extra_cast_number;
        uint32_t m_glyphslot;

        bool duelSpell;

        ////////////////////////////////////////////////////////////////////////////////
        ///bool DuelSpellNoMoreValid()
        /// Tells if the Spell was being casted while dueling but now the duel is over
        ///
        /// \return true if Spell is now invalid because the duel is over false if Spell is valid.
        ///
        ///////////////////////////////////////////////////////////////////////////////
        bool DuelSpellNoMoreValid() const;

        void safe_cancel();

        /// Spell state's
        /// Spell failed
        bool GetSpellFailed() const;
        void SetSpellFailed(bool failed = true);

    protected:

        /// Spell state's
        bool m_usesMana;
        bool m_Spell_Failed;         //for 5sr
        bool m_Delayed;
        uint8_t m_DelayStep;            //3.0.2 - spells can only be delayed twice.

        bool m_IsCastedOnSelf;

        bool hadEffect;

        uint32_t m_spellState;
        int64_t m_magnetTarget;

        // Current Targets to be used in effect handler
        Unit* unitTarget;
        Item* itemTarget;
        GameObject* gameObjTarget;
        Player* playerTarget;
        Corpse* corpseTarget;
        Creature* targetConstraintCreature;
        GameObject* targetConstraintGameObject;
        uint32_t add_damage;

        SpellCastResult cancastresult;
        uint32_t Dur;
        bool bDurSet;
        float Rad[3];
        bool bRadSet[3];
        bool m_cancelled;
        bool m_isCasting;
        uint8_t m_rune_avail_before;
        //void _DamageRangeUpdate();

        bool HasTarget(const uint64_t& guid, std::vector<uint64_t>* tmpMap);

        SpellTargetConstraint* m_target_constraint;

        virtual int32_t DoCalculateEffect(uint32_t i, Unit* target, int32_t value);
        virtual void DoAfterHandleEffect(Unit* target, uint32_t i);

    public:     //Modified by LUAppArc private->public

        float m_missilePitch;
        uint32_t m_missileTravelTime;

        void SafeAddTarget(std::vector<uint64_t>* tgt, uint64_t guid);

        void SafeAddMissedTarget(uint64_t guid);
        void SafeAddModeratedTarget(uint64_t guid, uint16_t type);

        friend class DynamicObject;
        void DetermineSkillUp(uint32_t skillid, uint32_t targetlevel, uint32_t multiplicator = 1);
        void DetermineSkillUp(uint32_t skillid);

        uint32_t GetTargetType(uint32_t value, uint32_t i);
        bool AddTarget(uint32_t i, uint32_t TargetType, Object* obj);
        void AddAOETargets(uint32_t i, uint32_t TargetType, float r, uint32_t maxtargets);
        void AddPartyTargets(uint32_t i, uint32_t TargetType, float r, uint32_t maxtargets);
        void AddRaidTargets(uint32_t i, uint32_t TargetType, float r, uint32_t maxtargets, bool partylimit = false);
        void AddChainTargets(uint32_t i, uint32_t TargetType, float r, uint32_t maxtargets);
        void AddConeTargets(uint32_t i, uint32_t TargetType, float r, uint32_t maxtargets);
        void AddScriptedOrSpellFocusTargets(uint32_t i, uint32_t TargetType, float r, uint32_t maxtargets);

    public:

        SpellInfo const* m_spellInfo;
        SpellInfo const* m_spellInfo_override;   //used by spells that should have dynamic variables in spellentry.
};

