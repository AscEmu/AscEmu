/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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

#ifndef USE_EXPERIMENTAL_SPELL_SYSTEM
#include "SpellCastTargets.h"
#include "Definitions/SpellTargetMod.h"
#include "Spell/SpellInfo.hpp"
#include "SpellFailure.h"
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
typedef void(Spell::*pSpellTarget)(uint32 i, uint32 j);

#define GO_FISHING_BOBBER 35591
#define SPELL_SPELL_CHANNEL_UPDATE_INTERVAL 1000

class SERVER_DECL Spell : public EventableObject
{
    public:
        // APGL Ends
        // MIT Starts
        int32_t getFullCastTime() const { return m_castTime; }
        int32_t getCastTimeLeft() const { return m_timer; }
        // MIT Ends
        // APGL Starts
        friend class DummySpellHandler;
        Spell(Object* Caster, SpellInfo* info, bool triggered, Aura* aur);
        ~Spell();

    int32 event_GetInstanceID() override;

        bool m_overrideBasePoints;
        uint32 m_overridenBasePoints[3];

        // Fills specified targets at the area of effect
        void FillSpecifiedTargetsInArea(float srcx, float srcy, float srcz, uint32 ind, uint32 specification);
        // Fills specified targets at the area of effect. We suppose we already inited this spell and know the details
        void FillSpecifiedTargetsInArea(uint32 i, float srcx, float srcy, float srcz, float range, uint32 specification);
        // Fills the targets at the area of effect
        void FillAllTargetsInArea(uint32 i, float srcx, float srcy, float srcz, float range);
        // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
        void FillAllTargetsInArea(float srcx, float srcy, float srcz, uint32 ind);
        // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
        void FillAllTargetsInArea(LocationVector & location, uint32 ind);
        // Fills the targets at the area of effect. We suppose we already inited this spell and know the details
        void FillAllFriendlyInArea(uint32 i, float srcx, float srcy, float srcz, float range);
        //get single Enemy as target
        uint64 GetSinglePossibleEnemy(uint32 i, float prange = 0);
        //get single Enemy as target
        uint64 GetSinglePossibleFriend(uint32 i, float prange = 0);
        //generate possible target list for a spell. Use as last resort since it is not accurate
        bool GenerateTargets(SpellCastTargets* store_buff);
        // Fills the target map of the spell packet
        void FillTargetMap(uint32);

        void HandleTargetNoObject();

        // See if we hit the target or can it resist (evade/immune/resist on spellgo) (0=success)
        uint8 DidHit(uint32 effindex, Unit* target);
        // Prepares the spell that's going to cast to targets
        uint8 prepare(SpellCastTargets* targets);
        // Cancels the current spell
        void cancel();
        // Update spell state based on time difference
        void Update(unsigned long time_passed);
        // Casts the spell
        void castMe(bool);
        // Finishes the casted spell
        void finish(bool successful = true);
        // Handle the Effects of the Spell
        virtual void HandleEffects(uint64 guid, uint32 i);
        void HandleCastEffects(uint64 guid, uint32 i);

        void HandleModeratedTarget(uint64 guid);

        void HandleModeratedEffects(uint64 guid);

        // Take Power from the caster based on spell power usage
        bool TakePower();
        // Has power?
        bool HasPower();
        // Trigger Spell function that triggers triggered spells
        //void TriggerSpell();

        // Checks the caster is ready for cast
        virtual uint8 CanCast(bool);

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
        int32 CalculateEffect(uint32, Unit* target);
        // Handles Teleport function
        void HandleTeleport(float x, float y, float z, uint32 mapid, Unit* Target);
        // Determines how much skill caster going to gain
        void DetermineSkillUp();
        // Increases cast time of the spell
        void AddTime(uint32 type);
        void AddCooldown();
        void AddStartCooldown();
        //
        uint8 GetErrorAtShapeshiftedCast(SpellInfo* spellInfo, uint32 form);


        bool Reflect(Unit* refunit);

    uint32 getState() const;
    void SetUnitTarget(Unit* punit);
    void SetTargetConstraintCreature(Creature* pCreature);
    void SetTargetConstraintGameObject(GameObject* pGameobject);
    Creature* GetTargetConstraintCreature() const;
    GameObject* GetTargetConstraintGameObject() const;

        // Send Packet functions
        void SetExtraCastResult(SpellExtraError result);
        void SendCastResult(Player* caster, uint8 castCount, uint8 result, SpellExtraError extraError);
        void WriteCastResult(WorldPacket& data, Player* caster, uint32 spellInfo, uint8 castCount, uint8 result, SpellExtraError extraError);
        void SendCastResult(uint8 result);
        void SendSpellStart();
        void SendSpellGo();
        void SendLogExecute(uint32 damage, uint64 & targetGuid);
        void SendInterrupted(uint8 result);
        void SendChannelUpdate(uint32 time);
        void SendChannelStart(uint32 duration);
        void SendResurrectRequest(Player* target);
        void SendTameFailure(uint8 failure);
        static void SendHealSpellOnPlayer(Object* caster, Object* target, uint32 healed, bool critical, uint32 overhealed, uint32 spellid, uint32 absorbed = 0);
        static void SendHealManaSpellOnPlayer(Object* caster, Object* target, uint32 dmg, uint32 powertype, uint32 spellid);


        void HandleAddAura(uint64 guid);
        void writeSpellGoTargets(WorldPacket* data);
        void writeSpellMissedTargets(WorldPacket* data);
        uint32 pSpellId;
        SpellInfo* ProcedOnSpell;
        SpellCastTargets m_targets;
        SpellExtraError m_extraError;

        void CreateItem(uint32 itemId);

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
        void SpellEffectSummonGuardian(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonTemporaryPet(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonTotem(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonPossessed(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonCompanion(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonVehicle(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
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

        void Heal(int32 amount, bool ForceCrit = false);

        GameObject*     g_caster;
        Unit*           u_caster;
        Item*           i_caster;
        Player*         p_caster;
        Object*         m_caster;

        // 15007 = resurrection sickness

        // This returns SPELL_ENTRY_Spell_Dmg_Type where 0 = SPELL_DMG_TYPE_NONE, 1 = SPELL_DMG_TYPE_MAGIC, 2 = SPELL_DMG_TYPE_MELEE, 3 = SPELL_DMG_TYPE_RANGED
        // It should NOT be used for weapon_damage_type which needs: 0 = MELEE, 1 = OFFHAND, 2 = RANGED
        uint32 GetType();

        std::map<uint64, Aura*> m_pendingAuras;
        std::vector<uint64_t> UniqueTargets;
        std::vector<SpellTargetMod> ModeratedTargets;

    Item* GetItemTarget() const;
    Unit* GetUnitTarget() const;
    Player* GetPlayerTarget() const;
    GameObject* GetGameObjectTarget() const;
    Corpse* GetCorpseTarget() const;

        uint32 chaindamage;
        // -------------------------------------------

        bool IsAspect();
        bool IsSeal();

    SpellInfo* GetSpellInfo();

    void InitProtoOverride();

    uint32 GetDuration();

    float GetRadius(uint32 i);

    static uint32 GetBaseThreat(uint32 dmg);

    static uint32 GetMechanic(SpellInfo* sp);

        bool IsStealthSpell();
        bool IsInvisibilitySpell();

        int32 damage;
        Aura* m_triggeredByAura;
        signed int  forced_basepoints[3]; //some talent inherit base points from previous caster spells

        bool m_triggeredSpell;
        bool m_AreaAura;
        //uint32 TriggerSpellId;  // used to set next spell to use
        //uint64 TriggerSpellTarget; // used to set next spell target
        bool m_requiresCP;
        float m_castPositionX;
        float m_castPositionY;
        float m_castPositionZ;
        int32 m_charges;

        int32 damageToHit;
        uint32 castedItemId;
        uint8 extra_cast_number;
        uint32 m_glyphslot;

        void SendCastSuccess(Object* target);
        void SendCastSuccess(const uint64 & guid);

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

    bool IsReflected() const;
    void SetReflected(bool reflected = true);

        /// Spell possibility's
    bool GetCanReflect() const;
    void SetCanReflect(bool reflect = true);


        Spell* m_reflectedParent;

    protected:

        /// Spell state's
        bool m_usesMana;
        bool m_Spell_Failed;         //for 5sr
        bool m_IsReflected;
        bool m_Delayed;
        uint8 m_DelayStep;            //3.0.2 - spells can only be delayed twice.

        // Spell possibility's
        bool m_CanRelect;

        bool m_IsCastedOnSelf;

        bool hadEffect;

        uint32 m_spellState;
        int32 m_castTime;
        int32 m_timer;
        int64 m_magnetTarget;

        // Current Targets to be used in effect handler
        Unit* unitTarget;
        Item* itemTarget;
        GameObject* gameObjTarget;
        Player* playerTarget;
        Corpse* corpseTarget;
        Creature* targetConstraintCreature;
        GameObject* targetConstraintGameObject;
        uint32 add_damage;

        uint8 cancastresult;
        uint32 Dur;
        bool bDurSet;
        float Rad[3];
        bool bRadSet[3];
        bool m_cancelled;
        bool m_isCasting;
        uint8 m_rune_avail_before;
        //void _DamageRangeUpdate();

    bool HasTarget(const uint64& guid, std::vector<uint64_t>* tmpMap);

        SpellTargetConstraint* m_target_constraint;

        virtual int32 DoCalculateEffect(uint32 i, Unit* target, int32 value);
    virtual void DoAfterHandleEffect(Unit* target, uint32 i);

    public:     //Modified by LUAppArc private->public

        float m_missilePitch;
        uint32 m_missileTravelTime;

        std::vector<uint64_t> m_targetUnits[3];
        void SafeAddTarget(std::vector<uint64_t>* tgt, uint64 guid);

        void SafeAddMissedTarget(uint64 guid);
        void SafeAddModeratedTarget(uint64 guid, uint16 type);

        friend class DynamicObject;
        void DetermineSkillUp(uint32 skillid, uint32 targetlevel, uint32 multiplicator = 1);
        void DetermineSkillUp(uint32 skillid);

        uint32 GetTargetType(uint32 value, uint32 i);
        bool AddTarget(uint32 i, uint32 TargetType, Object* obj);
        void AddAOETargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
        void AddPartyTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
        void AddRaidTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets, bool partylimit = false);
        void AddChainTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
        void AddConeTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);
        void AddScriptedOrSpellFocusTargets(uint32 i, uint32 TargetType, float r, uint32 maxtargets);

    public:

        SpellInfo* m_spellInfo;
        SpellInfo* m_spellInfo_override;   //used by spells that should have dynamic variables in spellentry.
        static SpellInfo* checkAndReturnSpellEntry(uint32_t spellid);
};

#endif // USE_EXPERIMENTAL_SPELL_SYSTEM