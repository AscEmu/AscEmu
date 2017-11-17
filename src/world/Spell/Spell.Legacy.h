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

typedef void(Spell::*pSpellEffect)(uint32 i);
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

        void SpellEffectUnused(uint32 i);

        void ApplyAreaAura(uint32 i);

        // Effect Handlers
        void SpellEffectNULL(uint32 i);
        void SpellEffectInstantKill(uint32 i);
        void SpellEffectSchoolDMG(uint32 i);
        void SpellEffectDummy(uint32 i);
        void SpellEffectTeleportUnits(uint32 i);
        void SpellEffectApplyAura(uint32 i);
        void SpellEffectEnvironmentalDamage(uint32 i);
        void SpellEffectPowerDrain(uint32 i);
        void SpellEffectHealthLeech(uint32 i);
        void SpellEffectHeal(uint32 i);
        void SpellEffectBind(uint32 i);
        void SpellEffectQuestComplete(uint32 i);
        void SpellEffectWeapondamageNoschool(uint32 i);
        void SpellEffectResurrect(uint32 i);
        void SpellEffectAddExtraAttacks(uint32 i);
        void SpellEffectDodge(uint32 i);
        void SpellEffectParry(uint32 i);
        void SpellEffectBlock(uint32 i);
        void SpellEffectCreateItem(uint32 i);
        void SpellEffectWeapon(uint32 i);
        void SpellEffectDefense(uint32 i);
        void SpellEffectPersistentAA(uint32 i);

        virtual void SpellEffectSummon(uint32 i);
        void SpellEffectSummonWild(uint32 i);
        void SpellEffectSummonGuardian(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonTemporaryPet(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonTotem(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonPossessed(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonCompanion(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectSummonVehicle(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v);
        void SpellEffectLeap(uint32 i);
        void SpellEffectEnergize(uint32 i);
        void SpellEffectWeaponDmgPerc(uint32 i);
        void SpellEffectTriggerMissile(uint32 i);
        void SpellEffectOpenLock(uint32 i);
        void SpellEffectTransformItem(uint32 i);
        void SpellEffectApplyGroupAA(uint32 i);
        void SpellEffectLearnSpell(uint32 i);
        void SpellEffectSpellDefense(uint32 i);
        void SpellEffectDispel(uint32 i);
        void SpellEffectLanguage(uint32 i);
        void SpellEffectDualWield(uint32 i);
        void SpellEffectSkillStep(uint32 i);
        void SpellEffectAddHonor(uint32 i);
        void SpellEffectSpawn(uint32 i);
        void SpellEffectSummonObject(uint32 i);
        void SpellEffectEnchantItem(uint32 i);
        void SpellEffectEnchantItemTemporary(uint32 i);
        void SpellEffectTameCreature(uint32 i);
        void SpellEffectSummonPet(uint32 i);
        void SpellEffectLearnPetSpell(uint32 i);
        void SpellEffectWeapondamage(uint32 i);
        void SpellEffectOpenLockItem(uint32 i);
        void SpellEffectProficiency(uint32 i);
        void SpellEffectSendEvent(uint32 i);
        void SpellEffectPowerBurn(uint32 i);
        void SpellEffectThreat(uint32 i);
        void SpellEffectClearQuest(uint32 i);
        void SpellEffectTriggerSpell(uint32 i);
        void SpellEffectApplyRaidAA(uint32 i);
        void SpellEffectPowerFunnel(uint32 i);
        void SpellEffectHealMaxHealth(uint32 i);
        void SpellEffectInterruptCast(uint32 i);
        void SpellEffectDistract(uint32 i);
        void SpellEffectPickpocket(uint32 i);
        void SpellEffectAddFarsight(uint32 i);
        void SpellEffectUseGlyph(uint32 i);
        void SpellEffectHealMechanical(uint32 i);
        void SpellEffectSummonObjectWild(uint32 i);
        void SpellEffectScriptEffect(uint32 i);
        void SpellEffectSanctuary(uint32 i);
        void SpellEffectAddComboPoints(uint32 i);
        void SpellEffectCreateHouse(uint32 i);
        void SpellEffectDuel(uint32 i);
        void SpellEffectStuck(uint32 i);
        void SpellEffectSummonPlayer(uint32 i);
        void SpellEffectActivateObject(uint32 i);
        void SpellEffectBuildingDamage(uint32 i);
        void SpellEffectEnchantHeldItem(uint32 i);
        void SpellEffectSetMirrorName(uint32 i);
        void SpellEffectSelfResurrect(uint32 i);
        void SpellEffectSkinning(uint32 i);
        void SpellEffectCharge(uint32 i);
        void SpellEffectKnockBack(uint32 i);
        void SpellEffectKnockBack2(uint32 i);
        void SpellEffectDisenchant(uint32 i);
        void SpellEffectInebriate(uint32 i);
        void SpellEffectFeedPet(uint32 i);
        void SpellEffectDismissPet(uint32 i);
        void SpellEffectReputation(uint32 i);
        void SpellEffectSummonObjectSlot(uint32 i);
        void SpellEffectDispelMechanic(uint32 i);
        void SpellEffectSummonDeadPet(uint32 i);
        void SpellEffectDestroyAllTotems(uint32 i);
        void SpellEffectDurabilityDamage(uint32 i);
        void SpellEffectDurabilityDamagePCT(uint32 i);
        void SpellEffectResurrectNew(uint32 i);
        void SpellEffectAttackMe(uint32 i);
        void SpellEffectSkinPlayerCorpse(uint32 i);
        void SpellEffectSkill(uint32 i);
        void SpellEffectApplyPetAA(uint32 i);
        void SpellEffectDummyMelee(uint32 i);
        void SpellEffectStartTaxi(uint32 i);
        void SpellEffectPlayerPull(uint32 i);
        void SpellEffectReduceThreatPercent(uint32 i);
        void SpellEffectSpellSteal(uint32 i);
        void SpellEffectProspecting(uint32 i);
        void SpellEffectApplyFriendAA(uint32 i);
        void SpellEffectApplyEnemyAA(uint32 i);
        void SpellEffectRedirectThreat(uint32 i);
        void SpellEffectPlayMusic(uint32 i);
        void SpellEffectForgetSpecialization(uint32 i);
        void SpellEffectKillCredit(uint32 i);
        void SpellEffectRestorePowerPct(uint32 i);
        void SpellEffectTriggerSpellWithValue(uint32 i);
        void SpellEffectApplyOwnerAA(uint32 i);
        void SpellEffectCreatePet(uint32 i);
        void SpellEffectTeachTaxiPath(uint32 i);
        void SpellEffectDualWield2H(uint32 i);
        void SpellEffectEnchantItemPrismatic(uint32 i);
        void SpellEffectCreateItem2(uint32 i);
        void SpellEffectMilling(uint32 i);
        void SpellEffectRenamePet(uint32 i);
        void SpellEffectRestoreHealthPct(uint32 i);
        void SpellEffectLearnSpec(uint32 i);
        void SpellEffectActivateSpec(uint32 i);
        void SpellEffectActivateRunes(uint32 i);
        void SpellEffectJumpTarget(uint32 i);
        void SpellEffectJumpBehindTarget(uint32 i);

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