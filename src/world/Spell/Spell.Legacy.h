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

#ifndef _SPELL_H
#define _SPELL_H
#ifndef USE_EXPERIMENTAL_SPELL_SYSTEM

#include "Spell/SpellInfo.hpp"
#include "Spell/Customization/SpellCustomizations.hpp"
#include "SpellTarget.h"
#include "SpellFailure.h"
#include "Units/Creatures/AIInterface.h"
#include "Units/Creatures/Creature.h"
#include "Units/Players/Player.h"
#include "Units/Unit.h"
#include "Units/Creatures/Pet.h"
#include "SpellEffects.h"
#include "SpellTargetConstraint.h"
#include "Spell/SpellHelpers.h"

class WorldSession;
class Unit;
class DynamicObj;
class Player;
class Item;
class Group;
class Aura;

inline bool TargetTypeCheck(Object* obj, uint32 ReqCreatureTypeMask)
{
    if (!ReqCreatureTypeMask)
        return true;

    if (obj->IsCreature())
    {
        CreatureProperties const* inf = static_cast< Creature* >(obj)->GetCreatureProperties();
        if (!(1 << (inf->Type - 1) & ReqCreatureTypeMask))
            return false;
    }
    else if (obj->IsPlayer() && !(UNIT_TYPE_HUMANOID_BIT & ReqCreatureTypeMask))
        return false;
    else
        return false;//mg, how in the hack did we cast it on a GO ? But who cares ?

    return true;
}

class SpellCastTargets
{
    public:
        void read(WorldPacket & data, uint64 caster);
        void write(WorldPacket & data);

        SpellCastTargets() : m_targetMask(0), m_targetMaskExtended(0), m_unitTarget(0), m_itemTarget(0), m_srcX(0), m_srcY(0), m_srcZ(0),
            m_destX(0), m_destY(0), m_destZ(0), unkuint64_1(0), unkuint64_2(0){}

        SpellCastTargets(uint16 TargetMask, uint64 unitTarget, uint64 itemTarget, float srcX, float srcY,
                         float srcZ, float destX, float destY, float destZ) : m_targetMask(TargetMask), m_targetMaskExtended(0), m_unitTarget(unitTarget),
            m_itemTarget(itemTarget), m_srcX(srcX), m_srcY(srcY), m_srcZ(srcZ), m_destX(destX), m_destY(destY), m_destZ(destZ), unkuint64_1(0), unkuint64_2(0){}

        SpellCastTargets(uint64 unitTarget) : m_targetMask(0x2), m_targetMaskExtended(0), m_unitTarget(unitTarget), m_itemTarget(0),
            m_srcX(0), m_srcY(0), m_srcZ(0), m_destX(0), m_destY(0), m_destZ(0), unkuint64_1(0), unkuint64_2(0) {}

        SpellCastTargets(WorldPacket & data, uint64 caster) : m_targetMask(0), m_targetMaskExtended(0), m_unitTarget(0), m_itemTarget(0), m_srcX(0), m_srcY(0), m_srcZ(0),
            m_destX(0), m_destY(0), m_destZ(0), unkuint64_1(0), unkuint64_2(0)
        {
            read(data, caster);
        }

        SpellCastTargets & operator=(const SpellCastTargets & target)
        {
            m_unitTarget = target.m_unitTarget;
            m_itemTarget = target.m_itemTarget;

            m_srcX = target.m_srcX;
            m_srcY = target.m_srcY;
            m_srcZ = target.m_srcZ;

            m_destX = target.m_destX;
            m_destY = target.m_destY;
            m_destZ = target.m_destZ;

            m_strTarget = target.m_strTarget;

            m_targetMask = target.m_targetMask;
            m_targetMaskExtended = target.m_targetMaskExtended;

            unkuint64_1 = target.unkuint64_1;
            unkuint64_2 = target.unkuint64_2;
            return *this;
        }

        ~SpellCastTargets() { m_strTarget.clear(); }
        uint16 m_targetMask;
        uint16 m_targetMaskExtended;            // this could be a 32 also
        uint64 m_unitTarget;
        uint64 m_itemTarget;

        uint64 unkuint64_1;
        float m_srcX, m_srcY, m_srcZ;
        uint64 unkuint64_2;
        float m_destX, m_destY, m_destZ;
        std::string m_strTarget;

        uint32 GetTargetMask() { return m_targetMask; }
    bool HasSrc();

    bool HasDst();
        bool HasDstOrSrc() { return (HasSrc() || HasDst()); }
};

enum SpellState
{
    SPELL_STATE_NULL      = 0,
    SPELL_STATE_PREPARING = 1,
    SPELL_STATE_CASTING   = 2,
    SPELL_STATE_FINISHED  = 3,
    SPELL_STATE_IDLE      = 4
};

enum DISPEL_TYPE
{
    DISPEL_ZGTRINKETS       = -1,
    DISPEL_NULL             = 0,
    DISPEL_MAGIC            = 1,
    DISPEL_CURSE            = 2,
    DISPEL_DISEASE          = 3,
    DISPEL_POISON           = 4,
    DISPEL_STEALTH          = 5,
    DISPEL_INVISIBILTY      = 6,
    DISPEL_ALL              = 7,
    DISPEL_SPECIAL_NPCONLY  = 8,
    DISPEL_FRENZY           = 9,
};

enum SpellMechanics
{
    MECHANIC_NONE = 0,
    MECHANIC_CHARMED,           // 1
    MECHANIC_DISORIENTED,       // 2
    MECHANIC_DISARMED,          // 3
    MECHANIC_DISTRACED,         // 4
    MECHANIC_FLEEING,           // 5
    MECHANIC_CLUMSY,            // 6
    MECHANIC_ROOTED,            // 7
    MECHANIC_PACIFIED,          // 8
    MECHANIC_SILENCED,          // 9
    MECHANIC_ASLEEP,            // 10
    MECHANIC_ENSNARED,          // 11
    MECHANIC_STUNNED,           // 12
    MECHANIC_FROZEN,            // 13
    MECHANIC_INCAPACIPATED,     // 14
    MECHANIC_BLEEDING,          // 15
    MECHANIC_HEALING,           // 16
    MECHANIC_POLYMORPHED,       // 17
    MECHANIC_BANISHED,          // 18
    MECHANIC_SHIELDED,          // 19
    MECHANIC_SHACKLED,          // 20
    MECHANIC_MOUNTED,           // 21
    MECHANIC_SEDUCED,           // 22
    MECHANIC_TURNED,            // 23
    MECHANIC_HORRIFIED,         // 24
    MECHANIC_INVULNARABLE,      // 25
    MECHANIC_INTERRUPTED,       // 26
    MECHANIC_DAZED,             // 27
    MECHANIC_DISCOVERY,         // 28
    MECHANIC_INVULNERABLE,      // 29
    MECHANIC_SAPPED,            // 30
    MECHANIC_ENRAGED,           // 31
    MECHANIC_END
};

typedef enum
{
    EFF_TARGET_NONE                                                 = 0,
    EFF_TARGET_SELF                                                 = 1,
    EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS       = 3,
    EFF_TARGET_PET                                                  = 5,
    EFF_TARGET_SINGLE_ENEMY                                         = 6,
    EFF_TARGET_SCRIPTED_TARGET                                      = 7,
    EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS             = 8,
    EFF_TARGET_HEARTSTONE_LOCATION                                  = 9,
    EFF_TARGET_ALL_ENEMY_IN_AREA                                    = 15,
    EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT                            = 16,
    EFF_TARGET_TELEPORT_LOCATION                                    = 17,
    EFF_TARGET_LOCATION_TO_SUMMON                                   = 18,
    EFF_TARGET_ALL_PARTY_AROUND_CASTER                              = 20,
    EFF_TARGET_SINGLE_FRIEND                                        = 21,
    EFF_TARGET_ALL_ENEMIES_AROUND_CASTER                            = 22,
    EFF_TARGET_GAMEOBJECT                                           = 23,
    EFF_TARGET_IN_FRONT_OF_CASTER                                   = 24,
    EFF_TARGET_DUEL                                                 = 25, // Don't know the real name!!!
    EFF_TARGET_GAMEOBJECT_ITEM                                      = 26,
    EFF_TARGET_PET_MASTER                                           = 27,
    EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED                          = 28,
    EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED                          = 29,
    EFF_TARGET_ALL_FRIENDLY_IN_AREA                                 = 30,
    EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME   = 31,
    EFF_TARGET_MINION                                               = 32,
    EFF_TARGET_ALL_PARTY_IN_AREA                                    = 33,
    EFF_TARGET_SINGLE_PARTY                                         = 35,
    EFF_TARGET_PET_SUMMON_LOCATION                                  = 36,
    EFF_TARGET_ALL_PARTY                                            = 37,
    EFF_TARGET_SCRIPTED_OR_SINGLE_TARGET                            = 38,
    EFF_TARGET_SELF_FISHING                                         = 39,
    EFF_TARGET_SCRIPTED_GAMEOBJECT                                  = 40,
    EFF_TARGET_TOTEM_EARTH                                          = 41,
    EFF_TARGET_TOTEM_WATER                                          = 42,
    EFF_TARGET_TOTEM_AIR                                            = 43,
    EFF_TARGET_TOTEM_FIRE                                           = 44,
    EFF_TARGET_CHAIN                                                = 45,
    EFF_TARGET_SCIPTED_OBJECT_LOCATION                              = 46,
    EFF_TARGET_DYNAMIC_OBJECT                                       = 47, // not sure exactly where is used
    EFF_TARGET_MULTIPLE_SUMMON_LOCATION                             = 48,
    EFF_TARGET_MULTIPLE_SUMMON_PET_LOCATION                         = 49,
    EFF_TARGET_SUMMON_LOCATION                                      = 50,
    EFF_TARGET_CALIRI_EGS                                           = 51,
    EFF_TARGET_LOCATION_NEAR_CASTER                                 = 52,
    EFF_TARGET_CURRENT_SELECTION                                    = 53,
    EFF_TARGET_TARGET_AT_ORIENTATION_TO_CASTER                      = 54,
    EFF_TARGET_LOCATION_INFRONT_CASTER                              = 55,
    EFF_TARGET_ALL_RAID                                             = 56,
    EFF_TARGET_PARTY_MEMBER                                         = 57,
    EFF_TARGET_TARGET_FOR_VISUAL_EFFECT                             = 59,
    EFF_TARGET_SCRIPTED_TARGET2                                     = 60,
    EFF_TARGET_AREAEFFECT_PARTY_AND_CLASS                           = 61,
    EFF_TARGET_PRIEST_CHAMPION                                      = 62, // wtf ?
    EFF_TARGET_NATURE_SUMMON_LOCATION                               = 63,
    EFF_TARGET_BEHIND_TARGET_LOCATION                               = 65,
    EFF_TARGET_MULTIPLE_GUARDIAN_SUMMON_LOCATION                    = 72,
    EFF_TARGET_NETHETDRAKE_SUMMON_LOCATION                          = 73,
    EFF_TARGET_SCRIPTED_LOCATION                                    = 74,
    EFF_TARGET_LOCATION_INFRONT_CASTER_AT_RANGE                     = 75,
    EFF_TARGET_ENEMIES_IN_AREA_CHANNELED_WITH_EXCEPTIONS            = 76,
    EFF_TARGET_SELECTED_ENEMY_CHANNELED                             = 77,
    EFF_TARGET_SELECTED_ENEMY_DEADLY_POISON                         = 86,
    EFF_TARGET_NON_COMBAT_PET                                       = 90,
    // these are custom, feel free to move them further if targeting gets extended
    EFF_TARGET_CUSTOM_PARTY_INJURED_SINGLE                          = 99,
    EFF_TARGET_CUSTOM_PARTY_INJURED_MULTI                           = 100,
    EFF_TARGET_CONE_IN_FRONT                                        = 104,
    EFF_TARGET_LIST_LENGTH_MARKER                                   = 111,
} SpellEffectTarget;


inline bool HasTargetType(SpellInfo* sp, uint32 ttype)
{
    if (
        sp->EffectImplicitTargetA[0] == ttype ||
        sp->EffectImplicitTargetA[1] == ttype ||
        sp->EffectImplicitTargetA[2] == ttype ||
        sp->EffectImplicitTargetB[0] == ttype ||
        sp->EffectImplicitTargetB[1] == ttype ||
        sp->EffectImplicitTargetB[2] == ttype
   )
        return true;
    return false;
}

inline int GetAiTargetType(SpellInfo* sp)
{
    /*  this is not good as one spell effect can target self and other one an enemy,
        maybe we should make it for each spell effect or use as flags */
    if (
        HasTargetType(sp, EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS) ||
        HasTargetType(sp, EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS) ||
        HasTargetType(sp, EFF_TARGET_ALL_ENEMY_IN_AREA) ||
        HasTargetType(sp, EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT) ||
        HasTargetType(sp, EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED) ||
        HasTargetType(sp, EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME)
   )
        return TTYPE_DESTINATION;
    if (
        HasTargetType(sp, EFF_TARGET_LOCATION_TO_SUMMON) ||
        HasTargetType(sp, EFF_TARGET_IN_FRONT_OF_CASTER) ||
        HasTargetType(sp, EFF_TARGET_ALL_FRIENDLY_IN_AREA) ||
        HasTargetType(sp, EFF_TARGET_PET_SUMMON_LOCATION) ||
        HasTargetType(sp, EFF_TARGET_LOCATION_INFRONT_CASTER) ||
        HasTargetType(sp, EFF_TARGET_CONE_IN_FRONT)
   )
        return TTYPE_SOURCE;
    if (
        HasTargetType(sp, EFF_TARGET_SINGLE_ENEMY) ||
        HasTargetType(sp, EFF_TARGET_ALL_ENEMIES_AROUND_CASTER) ||
        HasTargetType(sp, EFF_TARGET_DUEL) ||
        HasTargetType(sp, EFF_TARGET_SCRIPTED_OR_SINGLE_TARGET) ||
        HasTargetType(sp, EFF_TARGET_CHAIN) ||
        HasTargetType(sp, EFF_TARGET_CURRENT_SELECTION) ||
        HasTargetType(sp, EFF_TARGET_TARGET_AT_ORIENTATION_TO_CASTER) ||
        HasTargetType(sp, EFF_TARGET_MULTIPLE_GUARDIAN_SUMMON_LOCATION) ||
        HasTargetType(sp, EFF_TARGET_SELECTED_ENEMY_CHANNELED)
   )
        return TTYPE_SINGLETARGET;
    if (
        HasTargetType(sp, EFF_TARGET_ALL_PARTY_AROUND_CASTER) ||
        HasTargetType(sp, EFF_TARGET_SINGLE_FRIEND) ||
        HasTargetType(sp, EFF_TARGET_PET_MASTER) ||
        HasTargetType(sp, EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED) ||
        HasTargetType(sp, EFF_TARGET_ALL_PARTY_IN_AREA) ||
        HasTargetType(sp, EFF_TARGET_SINGLE_PARTY) ||
        HasTargetType(sp, EFF_TARGET_ALL_PARTY) ||
        HasTargetType(sp, EFF_TARGET_ALL_RAID) ||
        HasTargetType(sp, EFF_TARGET_PARTY_MEMBER) ||
        HasTargetType(sp, EFF_TARGET_AREAEFFECT_PARTY_AND_CLASS)
   )
        return TTYPE_OWNER;
    if (
        HasTargetType(sp, EFF_TARGET_SELF) ||
        HasTargetType(sp, 4) ||
        HasTargetType(sp, EFF_TARGET_PET) ||
        HasTargetType(sp, EFF_TARGET_MINION)
   )
        return TTYPE_CASTER;
    return TTYPE_NULL;
}

inline bool IsTargetingStealthed(SpellInfo* sp)
{
    if (HasTargetType(sp, EFF_TARGET_INVISIBLE_OR_HIDDEN_ENEMIES_AT_LOCATION_RADIUS) ||
        HasTargetType(sp, EFF_TARGET_ALL_ENEMIES_AROUND_CASTER) ||
        HasTargetType(sp, EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED) ||
        HasTargetType(sp, EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT))
        return 1;

    switch (sp->Id)
    {
        // SPELL_HASH_MAGMA_TOTEM
        case 8187:     // Magma Totem Rank 1
        case 8190:     // Magma Totem Rank 1
        case 10579:    // Magma Totem Rank 2
        case 10580:    // Magma Totem Rank 3
        case 10581:    // Magma Totem Rank 4
        case 10585:    // Magma Totem Rank 2
        case 10586:    // Magma Totem Rank 3
        case 10587:    // Magma Totem Rank 4
        case 25550:    // Magma Totem Rank 5
        case 25552:    // Magma Totem Rank 5
        case 58731:    // Magma Totem Rank 6
        case 58732:    // Magma Totem Rank 6
        case 58734:    // Magma Totem Rank 7
        case 58735:    // Magma Totem Rank 7
        {
            return 1;
        }
        default:
            break;
    }

    return 0;
}

inline bool IsRequireCooldownSpell(SpellInfo* sp)
{
    if ((sp->Attributes & ATTRIBUTES_TRIGGER_COOLDOWN && sp->AttributesEx & ATTRIBUTESEX_NOT_BREAK_STEALTH)     //rogue cold blood
        || (sp->Attributes & ATTRIBUTES_TRIGGER_COOLDOWN && (!sp->AttributesEx || sp->AttributesEx & ATTRIBUTESEX_REMAIN_OOC)))
        return true;

    return false;
}

// slow
struct SpellTargetMod
{
    SpellTargetMod(uint64 _TargetGuid, uint8 _TargetModType) : TargetGuid(_TargetGuid), TargetModType(_TargetModType)
    {

    }
    uint64 TargetGuid;
    uint8  TargetModType;
};

typedef std::vector<uint64> TargetsList;
typedef std::vector<SpellTargetMod> SpellTargetsList;

typedef void(Spell::*pSpellEffect)(uint32 i);
typedef void(Spell::*pSpellTarget)(uint32 i, uint32 j);

#define POWER_TYPE_HEALTH -2

enum PowerType
{
    POWER_TYPE_MANA         = 0,
    POWER_TYPE_RAGE         = 1,
    POWER_TYPE_FOCUS        = 2,
    POWER_TYPE_ENERGY       = 3,
    POWER_TYPE_HAPPINESS    = 4,
    POWER_TYPE_RUNES        = 5,
    POWER_TYPE_RUNIC_POWER  = 6,
    POWER_TYPE_STEAM        = 61,
    POWER_TYPE_PYRITE       = 41,
    POWER_TYPE_HEAT         = 101,
    POWER_TYPE_OOZE         = 121,
    POWER_TYPE_BLOOD        = 141,
    POWER_TYPE_WRATH        = 142,
};
// we have power type 15 and 31 :S

#define SPEC_PRIMARY 0
#define SPEC_SECONDARY 1

#define GO_FISHING_BOBBER 35591

#define SPELL_SPELL_CHANNEL_UPDATE_INTERVAL 1000
class DummySpellHandler;

enum SpellDidHitResult
{
    SPELL_DID_HIT_SUCCESS                   = 0,
    SPELL_DID_HIT_MISS                      = 1,
    SPELL_DID_HIT_RESIST                    = 2,
    SPELL_DID_HIT_DODGE                     = 3,
    SPELL_DID_HIT_PARRY                     = 4,
    SPELL_DID_HIT_BLOCK                     = 5,
    SPELL_DID_HIT_EVADE                     = 6,
    SPELL_DID_HIT_IMMUNE                    = 7,
    SPELL_DID_HIT_IMMUNE2                   = 8,
    SPELL_DID_HIT_DEFLECT                   = 9,  // See - http://www.wowwiki.com/Deflect
    SPELL_DID_HIT_ABSORB                    = 10, // See - http://www.wowwiki.com/Absorb
    SPELL_DID_HIT_REFLECT                   = 11, // See - http://www.wowwiki.com/Reflect
    NUM_SPELL_DID_HIT_RESULTS,
};

// Spell instance
class SERVER_DECL Spell : public EventableObject
{
    public:

        friend class DummySpellHandler;
        Spell(Object* Caster, SpellInfo* info, bool triggered, Aura* aur);
        ~Spell();

        int32 event_GetInstanceID() { return m_caster->GetInstanceID(); }

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

        bool HasCustomFlag(uint32 flag)
        {
            if ((GetSpellInfo()->CustomFlags & flag) != 0)
                return true;
            else
                return false;
        }

        inline bool hasAttribute(SpellAttributes attribute) { return (GetSpellInfo()->Attributes & attribute) != 0; }
        inline bool hasAttributeEx(SpellAttributesEx attribute) { return (GetSpellInfo()->AttributesEx & attribute) != 0; }
        inline bool hasAttributeExB(SpellAttributesExB attribute) { return (GetSpellInfo()->AttributesExB & attribute) != 0; }
        inline bool hasAttributeExC(SpellAttributesExC attribute) { return (GetSpellInfo()->AttributesExC & attribute) != 0; }
        inline bool hasAttributeExD(SpellAttributesExD attribute) { return (GetSpellInfo()->AttributesExD & attribute) != 0; }
        inline bool hasAttributeExE(SpellAttributesExE attribute) { return (GetSpellInfo()->AttributesExE & attribute) != 0; }
        inline bool hasAttributeExF(SpellAttributesExF attribute) { return (GetSpellInfo()->AttributesExF & attribute) != 0; }
        inline bool hasAttributeExG(SpellAttributesExG attribute) { return (GetSpellInfo()->AttributesExG & attribute) != 0; }

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

        inline uint32 getState() { return m_spellState; }
        inline void SetUnitTarget(Unit* punit) { unitTarget = punit; }
        inline void SetTargetConstraintCreature(Creature* pCreature) { targetConstraintCreature = pCreature; }
        inline void SetTargetConstraintGameObject(GameObject* pGameobject) { targetConstraintGameObject = pGameobject; }
        inline Creature* GetTargetConstraintCreature() { return targetConstraintCreature; }
        inline GameObject* GetTargetConstraintGameObject() { return targetConstraintGameObject; }

        // Send Packet functions
        void SetExtraCastResult(SpellExtraError result);
        void SendCastResult(Player* caster, uint8 castCount, uint8 result, SpellExtraError extraError);
        void WriteCastResult(WorldPacket& data, Player* caster, uint32 spellInfo, uint8 castCount, uint8 result, SpellExtraError extraError);
        void SendCastResult(uint8 result);
        void SetCustomCastResultMessage(SpellExtraError result);
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
        // Zyres: Not called.
        //void writeAmmoToPacket(WorldPacket* data);
        uint32 pSpellId;
        SpellInfo* ProcedOnSpell; //some spells need to know the origins of the proc too
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

        // Spell Targets Handlers
        void SpellTargetNULL(uint32 i, uint32 j);
        void SpellTargetDefault(uint32 i, uint32 j);
        void SpellTargetSelf(uint32 i, uint32 j);
        void SpellTargetInvisibleAOE(uint32 i, uint32 j);
        void SpellTargetFriendly(uint32 i, uint32 j);
        void SpellTargetPet(uint32 i, uint32 j);
        void SpellTargetSingleTargetEnemy(uint32 i, uint32 j);
        void SpellTargetCustomAreaOfEffect(uint32 i, uint32 j);
        void SpellTargetAreaOfEffect(uint32 i, uint32 j);
        void SpellTargetLandUnderCaster(uint32 i, uint32 j);            /// I don't think this is the correct name for this one
        void SpellTargetAllPartyMembersRangeNR(uint32 i, uint32 j);
        void SpellTargetSingleTargetFriend(uint32 i, uint32 j);
        void SpellTargetAoE(uint32 i, uint32 j);                        // something special
        void SpellTargetSingleGameobjectTarget(uint32 i, uint32 j);
        void SpellTargetInFrontOfCaster(uint32 i, uint32 j);
        void SpellTargetSingleFriend(uint32 i, uint32 j);
        void SpellTargetGameobject_itemTarget(uint32 i, uint32 j);
        void SpellTargetPetOwner(uint32 i, uint32 j);
        void SpellTargetEnemysAreaOfEffect(uint32 i, uint32 j);
        void SpellTargetTypeTAOE(uint32 i, uint32 j);
        void SpellTargetAllyBasedAreaEffect(uint32 i, uint32 j);
        void SpellTargetScriptedEffects(uint32 i, uint32 j);
        void SpellTargetSummon(uint32 i, uint32 j);
        void SpellTargetNearbyPartyMembers(uint32 i, uint32 j);
        void SpellTargetSingleTargetPartyMember(uint32 i, uint32 j);
        void SpellTargetScriptedEffects2(uint32 i, uint32 j);
        void SpellTargetPartyMember(uint32 i, uint32 j);
        void SpellTargetDummyTarget(uint32 i, uint32 j);
        void SpellTargetFishing(uint32 i, uint32 j);
        void SpellTargetType40(uint32 i, uint32 j);
        void SpellTargetTotem(uint32 i, uint32 j);
        void SpellTargetChainTargeting(uint32 i, uint32 j);
        void SpellTargetSimpleTargetAdd(uint32 i, uint32 j);
        void SpellTargetAllRaid(uint32 i, uint32 j);
        void SpellTargetTargetAreaSelectedUnit(uint32 i, uint32 j);
        void SpellTargetInFrontOfCaster2(uint32 i, uint32 j);
        void SpellTargetTargetPartyMember(uint32 i, uint32 j);
        void SpellTargetSameGroupSameClass(uint32 i, uint32 j);
        //these are custom
        void SpellTargetSinglePartyInjured(uint32 i, uint32 j);
        void SpellTargetMultiplePartyInjured(uint32 i, uint32 j);
        void SpellTargetNonCombatPet(uint32 i, uint32 j);

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
        TargetsList UniqueTargets;
        SpellTargetsList    ModeratedTargets;

        inline Item* GetItemTarget() { return itemTarget; }
        inline Unit* GetUnitTarget() { return unitTarget; }
        inline Player* GetPlayerTarget() { return playerTarget; }
        inline GameObject* GetGameObjectTarget() { return gameObjTarget; }
        Corpse* GetCorpseTarget() { return corpseTarget; }

        uint32 chaindamage;
        // -------------------------------------------

        bool IsAspect();
        bool IsSeal();

        inline SpellInfo* GetSpellInfo() { return (m_spellInfo_override == NULL) ? m_spellInfo : m_spellInfo_override; }
        void InitProtoOverride()
        {
            if (m_spellInfo_override != NULL)
                return;
            m_spellInfo_override = sSpellCustomizations.GetSpellInfo(m_spellInfo->Id);
        }
        uint32 GetDuration()
        {
            if (bDurSet)return Dur;
            bDurSet = true;
            int32 c_dur = 0;

            if (GetSpellInfo()->DurationIndex)
            {
                auto spell_duration = sSpellDurationStore.LookupEntry(GetSpellInfo()->DurationIndex);
                if (spell_duration)
                {
                    //check for negative and 0 durations.
                    //duration affected by level
                    if ((int32)spell_duration->Duration1 < 0 && spell_duration->Duration2 && u_caster)
                    {
                        this->Dur = uint32(((int32)spell_duration->Duration1 + (spell_duration->Duration2 * u_caster->getLevel())));
                        if ((int32)this->Dur > 0 && spell_duration->Duration3 > 0 && (int32)this->Dur > (int32)spell_duration->Duration3)
                        {
                            this->Dur = spell_duration->Duration3;
                        }

                        if ((int32)this->Dur < 0)
                            this->Dur = 0;
                        c_dur = this->Dur;
                    }
                    if (!c_dur)
                    {
                        this->Dur = spell_duration->Duration1;
                    }
                    //combo point lolerCopter? ;P
                    if (p_caster)
                    {
                        uint32 cp = p_caster->m_comboPoints;
                        if (cp)
                        {
                            uint32 bonus = (cp * (spell_duration->Duration3 - spell_duration->Duration1)) / 5;
                            if (bonus)
                            {
                                this->Dur += bonus;
                                m_requiresCP = true;
                            }
                        }
                    }

                    if (u_caster != nullptr)
                    {
                        ascemu::World::Spell::Helpers::spellModFlatIntValue(u_caster->SM_FDur, (int32*)&this->Dur, GetSpellInfo()->SpellGroupType);
                        ascemu::World::Spell::Helpers::spellModPercentageIntValue(u_caster->SM_PDur, (int32*)&this->Dur, GetSpellInfo()->SpellGroupType);
    #ifdef COLLECTION_OF_UNTESTED_STUFF_AND_TESTERS
                        int spell_flat_modifers = 0;
                        int spell_pct_modifers = 0;
                        spellModFlatIntValue(u_caster->SM_FDur, &spell_flat_modifers, GetProto()->SpellGroupType);
                        spellModFlatIntValue(u_caster->SM_PDur, &spell_pct_modifers, GetProto()->SpellGroupType);
                        if (spell_flat_modifers != 0 || spell_pct_modifers != 0)
                            LOG_DEBUG("!!!!!spell duration mod flat %d , spell duration mod pct %d , spell duration %d, spell group %u", spell_flat_modifers, spell_pct_modifers, Dur, GetProto()->SpellGroupType);
    #endif
                    }
                }
                else
                {
                    this->Dur = (uint32)-1;
                }
            }
            else
            {
                this->Dur = (uint32)-1;
            }

            return this->Dur;
        }

        inline float GetRadius(uint32 i)
        {
            if (bRadSet[i])
                return Rad[i];
            bRadSet[i] = true;
            Rad[i] = ::GetRadius(sSpellRadiusStore.LookupEntry(GetSpellInfo()->EffectRadiusIndex[i]));
            if (u_caster != nullptr)
            {
                ascemu::World::Spell::Helpers::spellModFlatFloatValue(u_caster->SM_FRadius, &Rad[i], GetSpellInfo()->SpellGroupType);
                ascemu::World::Spell::Helpers::spellModPercentageFloatValue(u_caster->SM_PRadius, &Rad[i], GetSpellInfo()->SpellGroupType);
    #ifdef COLLECTION_OF_UNTESTED_STUFF_AND_TESTERS
                float spell_flat_modifers = 0;
                float spell_pct_modifers = 1;
                spellModFlatFloatValue(u_caster->SM_FRadius, &spell_flat_modifers, GetProto()->SpellGroupType);
                spellModPercentageFloatValue(u_caster->SM_PRadius, &spell_pct_modifers, GetProto()->SpellGroupType);
                if (spell_flat_modifers != 0 || spell_pct_modifers != 1)
                    LOG_DEBUG("!!!!!spell radius mod flat %f , spell radius mod pct %f , spell radius %f, spell group %u", spell_flat_modifers, spell_pct_modifers, Rad[i], GetProto()->SpellGroupType);
    #endif
            }

            return Rad[i];
        }

        inline static uint32 GetBaseThreat(uint32 dmg)
        {
            //there should be a formula to determine what spell cause threat and which don't
            /*        switch(GetProto()->custom_NameHash)
                    {
                    //hunter's mark
                    case 4287212498:
                    {
                    return 0;
                    }break;
                    }*/
            return dmg;
        }

        inline static uint32 GetMechanic(SpellInfo* sp)
        {
            if (sp->MechanicsType)
                return sp->MechanicsType;
            if (sp->EffectMechanic[2])
                return sp->EffectMechanic[2];
            if (sp->EffectMechanic[1])
                return sp->EffectMechanic[1];
            if (sp->EffectMechanic[0])
                return sp->EffectMechanic[0];

            return 0;
        }

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
        bool DuelSpellNoMoreValid()
        {
            if (duelSpell && (
                (p_caster != NULL && p_caster->GetDuelState() != DUEL_STATE_STARTED) ||
                (u_caster != NULL && u_caster->IsPet() && static_cast< Pet* >(u_caster)->GetPetOwner() && static_cast< Pet* >(u_caster)->GetPetOwner()->GetDuelState() != DUEL_STATE_STARTED)))
                return true;
            else
                return false;
        }

        inline void safe_cancel()
        {
            m_cancelled = true;
        }

        /// Spell state's
        /// Spell failed
        inline bool GetSpellFailed() { return m_Spell_Failed; }
        inline void SetSpellFailed(bool failed = true) { m_Spell_Failed = failed; }

        inline bool IsReflected() { return m_IsReflected; }
        inline void SetReflected(bool reflected = true) { m_IsReflected = reflected; }

        /// Spell possibility's
        inline bool GetCanReflect() { return m_CanRelect; }
        inline void SetCanReflect(bool reflect = true) { m_CanRelect = reflect; }


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

        inline bool HasTarget(const uint64 & guid, TargetsList* tmpMap)
        {
            for (TargetsList::iterator itr = tmpMap->begin(); itr != tmpMap->end(); ++itr)
            {
                if (*itr == guid)
                    return true;
            }

            for (SpellTargetsList::iterator itr = ModeratedTargets.begin(); itr != ModeratedTargets.end(); ++itr)
                if ((*itr).TargetGuid == guid)
                    return true;

            return false;
        }

        SpellTargetConstraint* m_target_constraint;

        virtual int32 DoCalculateEffect(uint32 i, Unit* target, int32 value);
        virtual void DoAfterHandleEffect(Unit* target, uint32 i) {}

    public:     //Modified by LUAppArc private->public

        float m_missilePitch;
        uint32 m_missileTravelTime;

        TargetsList m_targetUnits[3];
        void SafeAddTarget(TargetsList* tgt, uint64 guid);

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

};

void ApplyDiminishingReturnTimer(uint32* Duration, Unit* Target, SpellInfo* spell);
void UnapplyDiminishingReturnTimer(Unit* Target, SpellInfo* spell);

uint32 GetDiminishingGroup(uint32 NameHash);
uint32 GetSpellDuration(SpellInfo* sp, Unit* caster = NULL);

//Logs if the spell doesn't exist, using Debug loglevel.
SpellInfo* CheckAndReturnSpellEntry(uint32 spellid);

#endif // USE_EXPERIMENTAL_SPELL_SYSTEM
#endif // _SPELL_H