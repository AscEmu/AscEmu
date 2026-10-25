/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ObjectUpdateMidnight.hpp"

#include "Storage/CharacterCustomizations.hpp"

#include "Data/Flags.hpp"
#include "Management/Group.h"
#include "Management/ItemInterface.h"
#include "Management/QuestDefines.hpp"
#include "Management/QuestLogEntry.hpp"
#include "Management/QuestMgr.h"
#include "Network/ByteBuffer.hpp"
#include "Objects/Container.hpp"
#include "Objects/DynamicObject.hpp"
#include "Objects/GameObject.h"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Item.hpp"
#include "Objects/Object.hpp"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/CreatureDefines.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Unit.hpp"
#include "Server/UpdateMask.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Server/WorldConfig.h"
#include "Storage/MySQLStructures.h"
#include "Version/ObjectLayout.hpp"
#include "WoWGuid.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <string>

using Version::AreaTriggerField;
using Version::ContainerField;
using Version::CorpseField;
using Version::DynamicObjectField;
using Version::GameObjectField;
using Version::ItemField;
using Version::ObjectField;
using Version::PlayerField;
using Version::UnitField;

namespace
{
    // object types of 12.x
    enum WireTypeId : uint8_t
    {
        WireTypeObject = 0,
        WireTypeItem = 1,
        WireTypeContainer = 2,
        WireTypeAzeriteEmpoweredItem = 3,
        WireTypeAzeriteItem = 4,
        WireTypeUnit = 5,
        WireTypePlayer = 6,
        WireTypeActivePlayer = 7,
        WireTypeGameObject = 8,
        WireTypeDynamicObject = 9,
        WireTypeCorpse = 10,
        WireTypeAreaTrigger = 11,
        WireTypeSceneObject = 12,
        WireTypeConversation = 13,
        WireTypeMeshObject = 14,
        WireTypeAiGroup = 15,
        WireTypeScenario = 16,
        WireTypeLootObject = 17
    };

    // visibility of the fields
    enum FieldFlag : uint8_t
    {
        FlagNone = 0,
        FlagOwner = 0x01,
        FlagPartyMember = 0x02,
        FlagUnitAll = 0x04,
        FlagEmpath = 0x08
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    // change mask of a structure: one bit per field, 32 bit blocks, one mask bit per non empty block
    template <size_t Bits>
    class ChangeMask
    {
    public:
        static constexpr size_t BlockCount = (Bits + 31) / 32;

        void set(size_t bit)
        {
            if (bit < Bits)
                m_blocks[bit / 32] |= 1u << (bit % 32);
        }

        void setRange(size_t first, size_t count)
        {
            for (size_t i = 0; i < count; ++i)
                set(first + i);
        }

        void setAll()
        {
            for (size_t bit = 0; bit < Bits; ++bit)
                set(bit);
        }

        bool operator[](size_t bit) const
        {
            return bit < Bits && ((m_blocks[bit / 32] >> (bit % 32)) & 1u) != 0;
        }

        bool any() const
        {
            for (const uint32_t block : m_blocks)
                if (block != 0)
                    return true;
            return false;
        }

        uint32_t block(size_t index) const { return index < BlockCount ? m_blocks[index] : 0; }

        uint32_t blocksMask(size_t word) const
        {
            uint32_t mask = 0;
            for (size_t i = word * 32; i < BlockCount && i < word * 32 + 32; ++i)
                if (m_blocks[i] != 0)
                    mask |= 1u << (i - word * 32);
            return mask;
        }

        void filter(const uint32_t* allowed)
        {
            for (size_t i = 0; i < BlockCount; ++i)
                m_blocks[i] &= allowed[i];
        }

        // the small header: the first block as bits
        void writeSmall(ByteBuffer& data) const
        {
            data.writeBits(m_blocks[0], static_cast<uint32_t>(Bits));
        }

        // the blocks header: one bit per block, then every non empty block
        void writeBlocks(ByteBuffer& data) const
        {
            data.writeBits(blocksMask(0), static_cast<uint32_t>(BlockCount));
            for (size_t i = 0; i < BlockCount; ++i)
                if (m_blocks[i] != 0)
                    data.writeBits(m_blocks[i], 32);
        }

        // the large header of more than 32 blocks: every complete mask word as value, the remaining mask bits as bits
        void writeLarge(ByteBuffer& data) const
        {
            constexpr size_t words = BlockCount / 32;
            for (size_t word = 0; word < words; ++word)
                data << uint32_t(blocksMask(word));
            if constexpr (BlockCount % 32 != 0)
                data.writeBits(blocksMask(words), static_cast<uint32_t>(BlockCount % 32));
            for (size_t i = 0; i < BlockCount; ++i)
                if (m_blocks[i] != 0)
                    data.writeBits(m_blocks[i], 32);
        }

    private:
        std::array<uint32_t, BlockCount> m_blocks{};
    };

    // the size of a dynamic field with every element changed, the size takes 32 bits unless the field says otherwise
    void writeDynamicMask(ByteBuffer& data, uint32_t size, uint32_t sizeBits = 32)
    {
        data.writeBits(size, sizeBits);
        if (size == 32)
        {
            data.writeBits(0xFFFFFFFFu, 32);
            return;
        }

        if (size > 32)
        {
            // every complete word as bits, the stream is never byte aligned here
            for (uint32_t block = 0; block < size / 32; ++block)
                data.writeBits(0xFFFFFFFFu, 32);
        }

        if (size % 32)
            data.writeBits(0xFFFFFFFFu, size % 32);
    }

    // a map field: the entries, none in the server layout
    void writeMapFieldCreate(ByteBuffer& data)
    {
        data << uint32_t(0);
    }

    // a map field of an update: the changes are not ignored, no entry changed
    void writeMapFieldUpdate(ByteBuffer& data)
    {
        data << uint8_t(0);
        data << uint16_t(0);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // the values of the server layout that changed
    class ChangedFields
    {
    public:
        ChangedFields(UpdateMask const* mask) : m_mask(mask) {}

        template <typename FieldId>
        bool changed(FieldId id, uint32_t arrayIndex = 0) const
        {
            if (m_mask == nullptr)
                return true;

            const Version::FieldDesc& desc = Version::layoutFor<FieldId>().get(id);
            if (desc.offset == Version::kNoField)
                return false;

            const uint32_t first = (desc.offset + arrayIndex * desc.stride) / sizeof(uint32_t);
            const uint32_t words = std::max<uint32_t>(1, desc.size / sizeof(uint32_t));
            for (uint32_t word = 0; word < words; ++word)
                if (m_mask->GetBit(first + word))
                    return true;

            return false;
        }

        bool all() const { return m_mask == nullptr; }

    private:
        UpdateMask const* m_mask;
    };

    struct Context
    {
        ByteBuffer& data;
        Player* target;
        uint32_t realmId;
        uint32_t mapId;
        uint8_t flags;

        void guid(uint64_t raw) const { data << WoWGuid(raw).toGuid128(realmId, mapId); }
        void guildGuid(uint32_t guildId) const
        {
            if (guildId != 0)
                data << WoWGuid128::realmSpecific(HighGuid128::Guild, realmId, guildId);
            else
                data << WoWGuid128();
        }
        bool owner() const { return (flags & FlagOwner) != 0; }
        bool partyMember() const { return (flags & FlagPartyMember) != 0; }
        bool ownerOrEmpath() const { return (flags & (FlagOwner | FlagEmpath)) != 0; }
        bool ownerOrUnitAll() const { return (flags & (FlagOwner | FlagUnitAll)) != 0; }
    };

    uint8_t fieldFlags(Object* object, Player* target)
    {
        uint8_t flags = FlagNone;
        if (target == nullptr)
            return flags;

        if (object == target)
            return FlagOwner | FlagPartyMember;

        if (Unit* unit = object->ToUnit())
        {
            if (unit->getSummonedByGuid() == target->getGuid() || unit->getCreatedByGuid() == target->getGuid() || unit->getCharmedByGuid() == target->getGuid())
                flags |= FlagOwner;

            if (Player* player = object->ToPlayer())
                if (player->getGroup() != nullptr && player->getGroup() == target->getGroup())
                    flags |= FlagPartyMember;
        }
        else if (object->isItem() || object->isContainer())
        {
            if (static_cast<Item*>(object)->getOwnerGuid() == target->getGuid())
                flags |= FlagOwner;
        }

        return flags;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // ObjectData: entry, dynamic flags, scale
    using ObjectMask = ChangeMask<4>;

    void objectCreate(Object* object, Context const& ctx)
    {
        ctx.data << int32_t(object->getEntry());
        ctx.data << uint32_t(object->getViewerDynamicFlags(ctx.target));
        ctx.data << float(object->getScale());
    }

    void objectCollect(ChangedFields const& changed, ObjectMask& mask, Object* object)
    {
        if (changed.changed(ObjectField::Entry))
            mask.set(1);
        if (changed.changed(ObjectField::DynamicField) || (object->isCreatureOrPlayer() && changed.changed(UnitField::DynamicFlags))
            || (object->isCorpse() && changed.changed(CorpseField::DynamicFlags)) || (object->isGameObject() && changed.changed(GameObjectField::Dynamic)))
            mask.set(2);
        if (changed.changed(ObjectField::ScaleX))
            mask.set(3);
        if (mask.any())
            mask.set(0);
    }

    void objectUpdate(Object* object, Context const& ctx, ObjectMask const& mask)
    {
        mask.writeSmall(ctx.data);
        ctx.data.flushBits();
        if (mask[0])
        {
            if (mask[1])
                ctx.data << int32_t(object->getEntry());
            if (mask[2])
                ctx.data << uint32_t(object->getViewerDynamicFlags(ctx.target));
            if (mask[3])
                ctx.data << float(object->getScale());
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // UnitData
    using UnitMask = ChangeMask<230>;

    // 12.x carries ten power slots, the server layout six: the remaining slots are always empty
    constexpr uint8_t PowerCount = 10;
    constexpr uint8_t LayoutPowerCount = 6;
    constexpr UnitField PowerFields[LayoutPowerCount] = { UnitField::Power1, UnitField::Power2, UnitField::Power3, UnitField::Power4, UnitField::Power5, UnitField::Power6 };
    constexpr UnitField MaxPowerFields[LayoutPowerCount] = { UnitField::MaxPower1, UnitField::MaxPower2, UnitField::MaxPower3, UnitField::MaxPower4, UnitField::MaxPower5, UnitField::MaxPower6 };

    // the empower stage of a channeled spell: none
    constexpr int8_t NoSpellEmpowerStage = -1;

    int32_t unitPower(Unit* unit, uint8_t index)
    {
        return index < LayoutPowerCount ? int32_t(unit->getField<uint32_t>(PowerFields[index])) : 0;
    }

    int32_t unitMaxPower(Unit* unit, uint8_t index)
    {
        return index < LayoutPowerCount ? int32_t(unit->getField<uint32_t>(MaxPowerFields[index])) : 0;
    }

    float unitPowerRegen(Unit* unit, UnitField field, uint8_t index)
    {
        return index < LayoutPowerCount ? unit->getField<float>(field, index) : 0.0f;
    }

    // 12.x carries five stats, the server layout four: the fifth stat is always empty
    constexpr uint8_t StatCount = 5;
    constexpr uint8_t LayoutStatCount = 4;

    int32_t unitStat(Unit* unit, uint8_t index)
    {
        return index < LayoutStatCount ? int32_t(unit->getStat(index)) : 0;
    }

    int32_t unitPosStat(Unit* unit, uint8_t index)
    {
        return index < LayoutStatCount ? int32_t(unit->getPosStat(index)) : 0;
    }

    int32_t unitNegStat(Unit* unit, uint8_t index)
    {
        return index < LayoutStatCount ? int32_t(unit->getNegStat(index)) : 0;
    }

    // the visible item of a unit: entry, appearance modifier and visual from the slot display and its info
    void unitVisibleItem(Unit* unit, uint8_t slot, int32_t& itemId, uint16_t& appearanceModId, uint16_t& itemVisual)
    {
        itemId = static_cast<int32_t>(unit->getField<uint32_t>(UnitField::VirtualItemSlotDisplay, slot * 2));
        const uint32_t info = unit->getField<uint32_t>(UnitField::VirtualItemSlotDisplay, slot * 2 + 1);
        appearanceModId = static_cast<uint16_t>(info & 0xFFFF);
        itemVisual = static_cast<uint16_t>(info >> 16);
    }

    // the visible item structure: item, secondary modified appearance, conditional appearance, appearance modifier,
    // visual, modified appearance, transmog slot option, sheathe category; the transmog and illusion bits follow
    void unitVisibleItemFields(Unit* unit, uint8_t slot, ByteBuffer& data)
    {
        int32_t itemId;
        uint16_t appearanceModId;
        uint16_t itemVisual;
        unitVisibleItem(unit, slot, itemId, appearanceModId, itemVisual);
        data << itemId;
        data << int32_t(0);                                                 // secondary item modified appearance
        data << int32_t(0);                                                 // conditional item appearance
        data << appearanceModId << itemVisual;
        data << uint32_t(0);                                                // item modified appearance
        data << uint8_t(0);                                                 // transmog slot option
        data << uint8_t(0);                                                 // sheathe category
    }

    void unitVisibleItemCreate(Unit* unit, uint8_t slot, ByteBuffer& data)
    {
        unitVisibleItemFields(unit, slot, data);
        data.writeBit(false);                                               // has transmog
        data.writeBit(false);                                               // has illusion
        data.flushBits();
    }

    // the visible item with its own mask: every field, the bits precede the fields
    void unitVisibleItemUpdate(Unit* unit, uint8_t slot, ByteBuffer& data)
    {
        data.writeBits(0x7FF, 11);
        data.writeBit(false);                                               // has transmog
        data.writeBit(false);                                               // has illusion
        data.flushBits();
        unitVisibleItemFields(unit, slot, data);
    }

    // the channel: spell, spell visual (x spell visual, script visual), start time, duration
    void unitChannelCreate(Unit* unit, ByteBuffer& data)
    {
        const auto spellInfo = sSpellMgr.getSpellInfo(unit->getChannelSpellId());
        data << int32_t(unit->getChannelSpellId());
        data << int32_t(spellInfo != nullptr ? spellInfo->getSpellVisual(0) : 0);   // SpellXSpellVisual
        data << int32_t(0);                                                 // script visual
        data << uint32_t(0);                                                // start time, no server value
        data << uint32_t(0);                                                // duration, no server value
    }

    // family and type of a creature from its properties, none for players
    void unitCreatureInfo(Unit* unit, int32_t& family, uint8_t& type)
    {
        family = 0;
        type = 0;
        if (Creature* creature = unit->ToCreature())
        {
            family = int32_t(creature->GetCreatureProperties()->Family);
            type = uint8_t(creature->GetCreatureProperties()->Type);
        }
    }

    uint32_t unitGuildId(Unit* unit)
    {
        if (Player* player = unit->ToPlayer())
            return player->getGuildId();
        return 0;
    }

    void unitCreate(Unit* unit, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;
        const uint64_t channelObject = unit->getChannelObjectGuid();
        int32_t creatureFamily;
        uint8_t creatureType;
        unitCreatureInfo(unit, creatureFamily, creatureType);

        data << int32_t(unit->getViewerDisplayId(ctx.target));
        data << uint32_t(unit->getField<uint32_t>(UnitField::NpcFlags, 0, 0));
        data << uint32_t(unit->getField<uint32_t>(UnitField::NpcFlags, 0, 4));
        data << uint32_t(0);                                                // state spell visual
        data << uint32_t(0);                                                // state anim
        data << uint32_t(0);                                                // state anim kit
        data << uint32_t(0);                                                // state world effects
        data << uint32_t(0);                                                // state world effects quest objective
        data << int32_t(0);                                                 // spell override name
        ctx.guid(unit->getCharmGuid());
        ctx.guid(unit->getSummonGuid());
        if (ctx.owner())
            ctx.guid(unit->getCritterGuid());
        ctx.guid(unit->getCharmedByGuid());
        ctx.guid(unit->getSummonedByGuid());
        ctx.guid(unit->getCreatedByGuid());
        ctx.guid(unit->getField<uint64_t>(UnitField::DemonCreatorGuid));
        data << WoWGuid128();                                               // look at controller target
        ctx.guid(unit->getTargetGuid());
        data << WoWGuid128();                                               // battle pet companion
        data << uint64_t(0);                                                // battle pet db id
        data << WoWGuid128();                                               // battle pet attached to decor
        data << WoWGuid128();                                               // battle pet decor house
        unitChannelCreate(unit, data);
        data << int8_t(NoSpellEmpowerStage);                                // spell empower stage
        data << uint32_t(0);                                                // summoned by home realm
        data << uint8_t(unit->getRace());
        data << uint8_t(unit->getClass());
        data << uint8_t(unit->getClass());                                  // player class
        data << uint8_t(unit->getGender());
        data << creatureType;
        data << uint8_t(unit->getField<uint8_t>(UnitField::FieldBytes0PowerType));
        data << uint32_t(unit->getField<uint32_t>(UnitField::OverrideDisplayPowerId));
        data << int64_t(unit->getHealth());

        for (uint8_t i = 0; i < PowerCount; ++i)
        {
            data << unitPower(unit, i);
            data << unitMaxPower(unit, i);
        }

        if (ctx.ownerOrUnitAll())
        {
            for (uint8_t i = 0; i < PowerCount; ++i)
            {
                data << unitPowerRegen(unit, UnitField::PowerRegenFlatModifier, i);
                data << unitPowerRegen(unit, UnitField::PowerRegenInterruptedFlatModifier, i);
            }
        }

        data << int64_t(unit->getMaxHealth());
        data << int32_t(unit->getViewerLevel(ctx.target));
        data << int32_t(unit->getField<uint32_t>(UnitField::EffectiveLevel));
        data << int32_t(0);                                                 // content tuning
        data << int32_t(0);                                                 // scaling level min
        data << int32_t(0);                                                 // scaling level max
        data << int32_t(0);                                                 // scaling level delta
        data << uint8_t(0);                                                 // scaling faction group
        data << int32_t(unit->getFactionTemplate());

        for (uint8_t i = 0; i < 3; ++i)
            unitVisibleItemCreate(unit, i, data);

        data << uint32_t(unit->getViewerUnitFlags(ctx.target));
        data << uint32_t(unit->getUnitFlags2());
        data << uint32_t(0);                                                // flags 3
        data << uint32_t(0);                                                // flags 4
        data << uint32_t(unit->getAuraState());
        data << uint32_t(unit->getBaseAttackTime(0));
        data << uint32_t(unit->getBaseAttackTime(1));
        if (ctx.owner())
            data << uint32_t(2000);                                         // ranged attack round base time
        data << float(unit->getBoundingRadius());
        data << float(unit->getCombatReach());
        data << float(1.0f);                                                // display scale
        data << creatureFamily;
        data << uint8_t(0);                                                 // override creature type
        data << int32_t(unit->getNativeDisplayId());
        data << float(1.0f);                                                // native display scale
        data << int32_t(unit->getMountDisplayId());
        data << int32_t(0);                                                 // cosmetic mount display
        if (ctx.ownerOrEmpath())
        {
            data << float(unit->getMinDamage());
            data << float(unit->getMaxDamage());
            data << float(unit->getMinOffhandDamage());
            data << float(unit->getMaxOffhandDamage());
        }
        data << uint8_t(unit->getStandState());
        data << uint8_t(unit->getPetTalentPoints());
        data << uint8_t(unit->getStandStateFlags());
        data << uint8_t(unit->getAnimationFlags());
        data << uint32_t(unit->getPetNumber());
        data << uint32_t(unit->getField<uint32_t>(UnitField::PetNameTimestamp));
        data << uint32_t(unit->getField<uint32_t>(UnitField::PetExperience));
        data << uint32_t(unit->getField<uint32_t>(UnitField::PetNextLevelExperience));
        data << float(unit->getModCastSpeed());
        data << float(0.0f);                                                // mod casting speed neg, no server value
        data << unit->getField<float>(UnitField::ModCastHaste);             // mod spell haste
        data << unit->getField<float>(UnitField::ModHaste);
        data << unit->getField<float>(UnitField::ModRangedHaste);
        data << unit->getField<float>(UnitField::ModHasteRegen);
        data << float(1.0f);                                                // mod time rate
        data << int32_t(unit->getCreatedBySpellId());
        data << int32_t(unit->getEmoteState());

        if (ctx.owner())
        {
            // the fifth stat and the support buff of a stat have no server value
            for (uint8_t i = 0; i < StatCount; ++i)
            {
                data << unitStat(unit, i);
                data << unitPosStat(unit, i);
                data << unitNegStat(unit, i);
                data << int32_t(0);                                         // stat support buff
            }
        }

        if (ctx.ownerOrEmpath())
        {
            for (uint8_t i = 0; i < 7; ++i)
                data << int32_t(unit->getResistance(i));
        }

        if (ctx.owner())
        {
            // bonus resistance and mana cost modifier per school, the power cost multiplier is one value in 12.x
            for (uint8_t i = 0; i < 7; ++i)
            {
                data << int32_t(unit->getField<uint32_t>(UnitField::ResistanceBuffModPositive, i)) - int32_t(unit->getField<uint32_t>(UnitField::ResistanceBuffModNegative, i));
                data << int32_t(unit->getPowerCostModifier(i));
            }
        }

        data << int32_t(unit->getBaseMana());
        if (ctx.owner())
            data << int32_t(unit->getBaseHealth());
        data << uint8_t(unit->getSheathType());
        data << uint8_t(unit->getPvpFlags());
        data << uint8_t(unit->getPetFlags());
        data << uint8_t(unit->getShapeShiftForm());

        if (ctx.owner())
        {
            data << int32_t(unit->getAttackPower());
            data << int32_t(unit->getField<uint32_t>(UnitField::AttackPowerModPos));
            data << int32_t(unit->getField<uint32_t>(UnitField::AttackPowerModNeg));
            data << float(unit->getAttackPowerMultiplier());
            data << int32_t(0);                                             // attack power mod support
            data << int32_t(unit->getRangedAttackPower());
            data << int32_t(unit->getField<uint32_t>(UnitField::RangedAttackPowerModsPos));
            data << int32_t(unit->getField<uint32_t>(UnitField::RangedAttackPowerModsNeg));
            data << float(unit->getRangedAttackPowerMultiplier());
            data << int32_t(0);                                             // ranged attack power mod support
            data << int32_t(0);                                             // main hand weapon attack power
            data << int32_t(0);                                             // off hand weapon attack power
            data << int32_t(0);                                             // ranged weapon attack power
            data << int32_t(0);                                             // set attack speed aura
            data << float(0.0f);                                            // lifesteal
            data << float(unit->getMinRangedDamage());
            data << float(unit->getMaxRangedDamage());
            data << float(0.0f);                                            // mana cost multiplier, no server value
        }

        data << unit->getField<float>(UnitField::MaxHealthModifier);
        data << float(unit->getHoverHeight());
        data << int32_t(0);                                                 // min item level cutoff
        data << int32_t(unit->getField<uint32_t>(UnitField::MinItemLevel));
        data << int32_t(unit->getField<uint32_t>(UnitField::MaxItemLevel));
        data << int32_t(0);                                                 // azerite item level
        data << int32_t(unit->getField<uint32_t>(UnitField::WildBattlePetLevel));
        data << int32_t(0);                                                 // battle pet companion experience
        data << uint32_t(unit->getField<uint32_t>(UnitField::BattlePetCompanionNameTimestamp));
        data << int32_t(unit->getField<uint32_t>(UnitField::InteractSpellId));
        data << int32_t(0);                                                 // scale duration
        data << int32_t(0);                                                 // looks like mount
        data << int32_t(0);                                                 // looks like creature
        data << int32_t(0);                                                 // look at controller
        data << int32_t(0);                                                 // perks vendor item
        data << int32_t(0);                                                 // taxi nodes
        ctx.guildGuid(unitGuildId(unit));
        data << uint32_t(0);                                                // passive spells
        data << uint32_t(0);                                                // world effects
        data << uint32_t(channelObject != 0 ? 1 : 0);                       // channel objects
        data << int32_t(0);                                                 // flight capability
        data << float(0.0f);                                                // glide event speed divisor
        data << int32_t(0);                                                 // drive capability
        data << int32_t(0);                                                 // max health modifier flat neg
        data << int32_t(0);                                                 // max health modifier flat pos
        data << uint32_t(0);                                                // silenced school mask
        data << uint32_t(0);                                                // current area
        data << float(0.0f);                                                // nameplate distance mod
        data << float(0.0f);                                                // auto attack range mod
        data << WoWGuid128();                                               // nameplate attach to
        if (channelObject != 0)
            ctx.guid(channelObject);
        data.writeBit(false);                                               // field 314
        data.writeBit(false);                                               // has assist action data
        data.flushBits();
    }

    // bits of the fields that changed in the server layout
    void unitCollect(ChangedFields const& changed, UnitMask& mask)
    {
        struct Single { uint8_t bit; UnitField field; };
        static constexpr Single singles[] =
        {
            { 6, UnitField::DisplayId }, { 14, UnitField::CharmGuid }, { 15, UnitField::SummonGuid }, { 16, UnitField::CritterGuid },
            { 17, UnitField::CharmedByGuid }, { 18, UnitField::SummonedByGuid }, { 19, UnitField::CreatedByGuid }, { 20, UnitField::DemonCreatorGuid },
            { 22, UnitField::TargetGuid }, { 23, UnitField::BattlePetCompanionGuid }, { 27, UnitField::ChannelSpell }, { 29, UnitField::SummonedByHomeRealm },
            { 36, UnitField::DisplayPower }, { 37, UnitField::OverrideDisplayPowerId }, { 38, UnitField::Health }, { 39, UnitField::MaxHealth },
            { 40, UnitField::Level }, { 41, UnitField::EffectiveLevel }, { 47, UnitField::FactionTemplate }, { 48, UnitField::UnitFlags },
            { 49, UnitField::UnitFlags2 }, { 52, UnitField::AuraState }, { 54, UnitField::BoundingRadius }, { 55, UnitField::CombatReach },
            { 59, UnitField::NativeDisplayId }, { 61, UnitField::MountDisplayId }, { 63, UnitField::MinimumDamage }, { 65, UnitField::MaximumDamage },
            { 66, UnitField::MinimumOffhandDamage }, { 67, UnitField::MaximumOffhandDamage }, { 72, UnitField::PetNumber }, { 73, UnitField::PetNameTimestamp },
            { 74, UnitField::PetExperience }, { 75, UnitField::PetNextLevelExperience }, { 76, UnitField::ModCastSpeed }, { 78, UnitField::ModCastHaste },
            { 79, UnitField::ModHaste }, { 80, UnitField::ModRangedHaste }, { 81, UnitField::ModHasteRegen }, { 83, UnitField::CreatedBySpellId },
            { 84, UnitField::NpcEmoteState }, { 85, UnitField::BaseMana }, { 86, UnitField::BaseHealth }, { 91, UnitField::AttackPower },
            { 92, UnitField::AttackPowerModPos }, { 93, UnitField::AttackPowerModNeg }, { 94, UnitField::AttackPowerMultiplier }, { 97, UnitField::RangedAttackPower },
            { 98, UnitField::RangedAttackPowerModsPos }, { 99, UnitField::RangedAttackPowerModsNeg }, { 100, UnitField::RangedAttackPowerMultiplier },
            { 107, UnitField::MinimumRangedDamage }, { 108, UnitField::MaximumRangedDamage }, { 110, UnitField::MaxHealthModifier }, { 111, UnitField::HoverHeight },
            { 113, UnitField::MinItemLevel }, { 114, UnitField::MaxItemLevel }, { 116, UnitField::WildBattlePetLevel }, { 118, UnitField::BattlePetCompanionNameTimestamp },
            { 119, UnitField::InteractSpellId }
        };

        for (const Single& single : singles)
            if (changed.changed(single.field))
                mask.set(single.bit);

        if (changed.changed(UnitField::ChannelObjectGuid))
            mask.set(5);
        if (changed.changed(UnitField::NpcFlags))
            mask.setRange(7, 2);
        if (changed.changed(UnitField::FieldBytes0))
        {
            // race and class end the first block, player class and gender start the second
            mask.setRange(30, 2);
            mask.setRange(33, 2);
        }
        if (changed.changed(UnitField::FieldBytes1))
            mask.setRange(68, 4);                                           // stand state, pet talent points, vis flags, anim tier
        if (changed.changed(UnitField::FieldBytes2))
            mask.setRange(87, 4);
        if (changed.changed(PlayerField::GuildId))
            mask.set(126);

        for (uint8_t i = 0; i < LayoutPowerCount; ++i)
        {
            if (changed.changed(PowerFields[i]))
                mask.set(140 + i);
            if (changed.changed(MaxPowerFields[i]))
                mask.set(150 + i);
            if (changed.changed(UnitField::PowerRegenFlatModifier, i))
                mask.set(160 + i);
            if (changed.changed(UnitField::PowerRegenInterruptedFlatModifier, i))
                mask.set(170 + i);
        }

        for (uint8_t i = 0; i < 3; ++i)
            if (changed.changed(UnitField::VirtualItemSlotDisplay, i * 2) || changed.changed(UnitField::VirtualItemSlotDisplay, i * 2 + 1))
                mask.set(181 + i);

        for (uint8_t i = 0; i < 2; ++i)
            if (changed.changed(UnitField::BaseAttackTime, i))
                mask.set(185 + i);

        for (uint8_t i = 0; i < LayoutStatCount; ++i)
        {
            if (changed.changed(UnitField::Stat, i))
                mask.set(188 + i);
            if (changed.changed(UnitField::PositiveStat, i))
                mask.set(193 + i);
            if (changed.changed(UnitField::NegativeStat, i))
                mask.set(198 + i);
        }

        for (uint8_t i = 0; i < 7; ++i)
        {
            if (changed.changed(UnitField::Resistance, i))
                mask.set(209 + i);
            if (changed.changed(UnitField::ResistanceBuffModPositive, i) || changed.changed(UnitField::ResistanceBuffModNegative, i))
                mask.set(216 + i);
            if (changed.changed(UnitField::PowerCostModifier, i))
                mask.set(223 + i);
            if (changed.changed(UnitField::PowerCostMultiplier, i))
                mask.set(109);
        }

        // the group bits: the blocks of the plain fields, then the arrays
        struct Group { uint8_t parent; uint8_t first; uint8_t count; };
        static constexpr Group groups[] =
        {
            { 0, 1, 31 }, { 32, 33, 31 }, { 64, 65, 31 }, { 96, 97, 31 }, { 128, 129, 10 },
            { 139, 140, 40 }, { 180, 181, 3 }, { 184, 185, 2 }, { 187, 188, 20 }, { 208, 209, 21 }
        };
        for (const Group& group : groups)
            for (uint8_t i = 0; i < group.count; ++i)
                if (mask[group.first + i])
                    mask.set(group.parent);
    }

    // the fields a receiver may see
    void unitFilter(UnitMask& mask, uint8_t flags)
    {
        uint32_t allowed[8] = { 0xFFFEFFFF, 0x7FDFFFFF, 0x07BFFFF1, 0xFFFFC001, 0xFFFFFFFF, 0x07F00000, 0x00000000, 0x00000000 };
        if (flags & FlagOwner)
        {
            const uint32_t owner[8] = { 0x00010000, 0x80200000, 0xF840000E, 0x00003FFE, 0x00000800, 0xF80FFFFF, 0xFFFFFFFF, 0x0000003F };
            for (size_t i = 0; i < 8; ++i)
                allowed[i] |= owner[i];
        }
        if (flags & FlagUnitAll)
        {
            allowed[4] |= 0x00000800;
            allowed[5] |= 0x000FFFFF;
        }
        if (flags & FlagEmpath)
        {
            allowed[1] |= 0x80000000;
            allowed[2] |= 0x0000000E;
            allowed[6] |= 0x00FF0000;
        }
        mask.filter(allowed);
    }

    void unitUpdate(Unit* unit, Context const& ctx, UnitMask const& mask)
    {
        ByteBuffer& data = ctx.data;
        const uint64_t channelObject = unit->getChannelObjectGuid();
        int32_t creatureFamily;
        uint8_t creatureType;
        unitCreatureInfo(unit, creatureFamily, creatureType);

        mask.writeBlocks(data);
        if (mask[0])
        {
            if (mask[1])
                data.writeBit(false);                                       // field 314
            if (mask[2])
                data.writeBits(0, 32);                                      // state world effects
        }
        data.flushBits();

        if (mask[0])
        {
            // the dynamic fields: passive spells, world effects, channel objects
            if (mask[3])
                writeDynamicMask(data, 0);
            if (mask[4])
                writeDynamicMask(data, 0);
            if (mask[5])
                writeDynamicMask(data, channelObject != 0 ? 1 : 0);
        }
        data.flushBits();

        if (mask[0])
        {
            if (mask[5] && channelObject != 0)
                ctx.guid(channelObject);
            if (mask[6])
                data << int32_t(unit->getViewerDisplayId(ctx.target));
            for (uint8_t i = 0; i < 2; ++i)
                if (mask[7 + i])
                    data << uint32_t(unit->getField<uint32_t>(UnitField::NpcFlags, 0, i * 4));
            for (uint8_t bit = 9; bit <= 12; ++bit)
                if (mask[bit])
                    data << uint32_t(0);
            if (mask[13])
                data << int32_t(0);
            if (mask[14])
                ctx.guid(unit->getCharmGuid());
            if (mask[15])
                ctx.guid(unit->getSummonGuid());
            if (mask[16])
                ctx.guid(unit->getCritterGuid());
            if (mask[17])
                ctx.guid(unit->getCharmedByGuid());
            if (mask[18])
                ctx.guid(unit->getSummonedByGuid());
            if (mask[19])
                ctx.guid(unit->getCreatedByGuid());
            if (mask[20])
                ctx.guid(unit->getField<uint64_t>(UnitField::DemonCreatorGuid));
            if (mask[21])
                data << WoWGuid128();
            if (mask[22])
                ctx.guid(unit->getTargetGuid());
            if (mask[23])
                data << WoWGuid128();
            if (mask[24])
                data << uint64_t(0);
            if (mask[25])
                data << WoWGuid128();                                       // battle pet attached to decor
            if (mask[26])
                data << WoWGuid128();                                       // battle pet decor house
            if (mask[27])
                unitChannelCreate(unit, data);
            if (mask[28])
                data << int8_t(NoSpellEmpowerStage);
            if (mask[29])
                data << uint32_t(0);
            if (mask[30])
                data << uint8_t(unit->getRace());
            if (mask[31])
                data << uint8_t(unit->getClass());
        }

        if (mask[32])
        {
            if (mask[33])
                data << uint8_t(unit->getClass());
            if (mask[34])
                data << uint8_t(unit->getGender());
            if (mask[35])
                data << creatureType;
            if (mask[36])
                data << uint8_t(unit->getField<uint8_t>(UnitField::FieldBytes0PowerType));
            if (mask[37])
                data << uint32_t(unit->getField<uint32_t>(UnitField::OverrideDisplayPowerId));
            if (mask[38])
                data << int64_t(unit->getHealth());
            if (mask[39])
                data << int64_t(unit->getMaxHealth());
            if (mask[40])
                data << int32_t(unit->getViewerLevel(ctx.target));
            if (mask[41])
                data << int32_t(unit->getField<uint32_t>(UnitField::EffectiveLevel));
            for (uint8_t bit = 42; bit <= 45; ++bit)
                if (mask[bit])
                    data << int32_t(0);
            if (mask[46])
                data << uint8_t(0);
            if (mask[47])
                data << int32_t(unit->getFactionTemplate());
            if (mask[48])
                data << uint32_t(unit->getViewerUnitFlags(ctx.target));
            if (mask[49])
                data << uint32_t(unit->getUnitFlags2());
            if (mask[50])
                data << uint32_t(0);
            if (mask[51])
                data << uint32_t(0);
            if (mask[52])
                data << uint32_t(unit->getAuraState());
            if (mask[53])
                data << uint32_t(2000);
            if (mask[54])
                data << float(unit->getBoundingRadius());
            if (mask[55])
                data << float(unit->getCombatReach());
            if (mask[56])
                data << float(1.0f);
            if (mask[57])
                data << creatureFamily;
            if (mask[58])
                data << uint8_t(0);
            if (mask[59])
                data << int32_t(unit->getNativeDisplayId());
            if (mask[60])
                data << float(1.0f);
            if (mask[61])
                data << int32_t(unit->getMountDisplayId());
            if (mask[62])
                data << int32_t(0);
            if (mask[63])
                data << float(unit->getMinDamage());
        }

        if (mask[64])
        {
            if (mask[65])
                data << float(unit->getMaxDamage());
            if (mask[66])
                data << float(unit->getMinOffhandDamage());
            if (mask[67])
                data << float(unit->getMaxOffhandDamage());
            if (mask[68])
                data << uint8_t(unit->getStandState());
            if (mask[69])
                data << uint8_t(unit->getPetTalentPoints());
            if (mask[70])
                data << uint8_t(unit->getStandStateFlags());
            if (mask[71])
                data << uint8_t(unit->getAnimationFlags());
            if (mask[72])
                data << uint32_t(unit->getPetNumber());
            if (mask[73])
                data << uint32_t(unit->getField<uint32_t>(UnitField::PetNameTimestamp));
            if (mask[74])
                data << uint32_t(unit->getField<uint32_t>(UnitField::PetExperience));
            if (mask[75])
                data << uint32_t(unit->getField<uint32_t>(UnitField::PetNextLevelExperience));
            if (mask[76])
                data << float(unit->getModCastSpeed());
            if (mask[77])
                data << float(0.0f);
            if (mask[78])
                data << unit->getField<float>(UnitField::ModCastHaste);
            if (mask[79])
                data << unit->getField<float>(UnitField::ModHaste);
            if (mask[80])
                data << unit->getField<float>(UnitField::ModRangedHaste);
            if (mask[81])
                data << unit->getField<float>(UnitField::ModHasteRegen);
            if (mask[82])
                data << float(1.0f);
            if (mask[83])
                data << int32_t(unit->getCreatedBySpellId());
            if (mask[84])
                data << int32_t(unit->getEmoteState());
            if (mask[85])
                data << int32_t(unit->getBaseMana());
            if (mask[86])
                data << int32_t(unit->getBaseHealth());
            if (mask[87])
                data << uint8_t(unit->getSheathType());
            if (mask[88])
                data << uint8_t(unit->getPvpFlags());
            if (mask[89])
                data << uint8_t(unit->getPetFlags());
            if (mask[90])
                data << uint8_t(unit->getShapeShiftForm());
            if (mask[91])
                data << int32_t(unit->getAttackPower());
            if (mask[92])
                data << int32_t(unit->getField<uint32_t>(UnitField::AttackPowerModPos));
            if (mask[93])
                data << int32_t(unit->getField<uint32_t>(UnitField::AttackPowerModNeg));
            if (mask[94])
                data << float(unit->getAttackPowerMultiplier());
            if (mask[95])
                data << int32_t(0);
        }

        if (mask[96])
        {
            if (mask[97])
                data << int32_t(unit->getRangedAttackPower());
            if (mask[98])
                data << int32_t(unit->getField<uint32_t>(UnitField::RangedAttackPowerModsPos));
            if (mask[99])
                data << int32_t(unit->getField<uint32_t>(UnitField::RangedAttackPowerModsNeg));
            if (mask[100])
                data << float(unit->getRangedAttackPowerMultiplier());
            for (uint8_t bit = 101; bit <= 105; ++bit)
                if (mask[bit])
                    data << int32_t(0);
            if (mask[106])
                data << float(0.0f);
            if (mask[107])
                data << float(unit->getMinRangedDamage());
            if (mask[108])
                data << float(unit->getMaxRangedDamage());
            if (mask[109])
                data << float(0.0f);
            if (mask[110])
                data << unit->getField<float>(UnitField::MaxHealthModifier);
            if (mask[111])
                data << float(unit->getHoverHeight());
            if (mask[112])
                data << int32_t(0);
            if (mask[113])
                data << int32_t(unit->getField<uint32_t>(UnitField::MinItemLevel));
            if (mask[114])
                data << int32_t(unit->getField<uint32_t>(UnitField::MaxItemLevel));
            if (mask[115])
                data << int32_t(0);
            if (mask[116])
                data << int32_t(unit->getField<uint32_t>(UnitField::WildBattlePetLevel));
            if (mask[117])
                data << int32_t(0);
            if (mask[118])
                data << uint32_t(unit->getField<uint32_t>(UnitField::BattlePetCompanionNameTimestamp));
            if (mask[119])
                data << int32_t(unit->getField<uint32_t>(UnitField::InteractSpellId));
            for (uint8_t bit = 120; bit <= 125; ++bit)
                if (mask[bit])
                    data << int32_t(0);
            if (mask[126])
                ctx.guildGuid(unitGuildId(unit));
            if (mask[127])
                data << int32_t(0);
        }

        if (mask[128])
        {
            if (mask[129])
                data << float(0.0f);
            if (mask[130])
                data << int32_t(0);
            if (mask[131])
                data << int32_t(0);
            if (mask[132])
                data << int32_t(0);
            if (mask[133])
                data << uint32_t(0);
            if (mask[134])
                data << uint32_t(0);
            if (mask[135])
                data << float(0.0f);
            if (mask[136])
                data << float(0.0f);
            if (mask[137])
                data << WoWGuid128();
            data.writeBits(0, 1);                                           // has assist action data
        }
        data.flushBits();

        if (mask[139])
        {
            for (uint8_t i = 0; i < PowerCount; ++i)
            {
                if (mask[140 + i])
                    data << unitPower(unit, i);
                if (mask[150 + i])
                    data << unitMaxPower(unit, i);
                if (mask[160 + i])
                    data << unitPowerRegen(unit, UnitField::PowerRegenFlatModifier, i);
                if (mask[170 + i])
                    data << unitPowerRegen(unit, UnitField::PowerRegenInterruptedFlatModifier, i);
            }
        }

        if (mask[180])
        {
            for (uint8_t i = 0; i < 3; ++i)
                if (mask[181 + i])
                    unitVisibleItemUpdate(unit, i, data);
        }

        if (mask[184])
        {
            for (uint8_t i = 0; i < 2; ++i)
                if (mask[185 + i])
                    data << uint32_t(unit->getBaseAttackTime(i));
        }

        if (mask[187])
        {
            for (uint8_t i = 0; i < StatCount; ++i)
            {
                if (mask[188 + i])
                    data << unitStat(unit, i);
                if (mask[193 + i])
                    data << unitPosStat(unit, i);
                if (mask[198 + i])
                    data << unitNegStat(unit, i);
                if (mask[203 + i])
                    data << int32_t(0);
            }
        }

        if (mask[208])
        {
            for (uint8_t i = 0; i < 7; ++i)
            {
                if (mask[209 + i])
                    data << int32_t(unit->getResistance(i));
                if (mask[216 + i])
                    data << int32_t(unit->getField<uint32_t>(UnitField::ResistanceBuffModPositive, i)) - int32_t(unit->getField<uint32_t>(UnitField::ResistanceBuffModNegative, i));
                if (mask[223 + i])
                    data << int32_t(unit->getPowerCostModifier(i));
            }
        }
        data.flushBits();
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // PlayerData
    using PlayerMask = ChangeMask<326>;

    constexpr uint8_t QuestLogCount = 175;
    constexpr uint8_t VisibleItemCount = 19;
    constexpr uint8_t AvgItemLevelCount = 6;
    constexpr uint8_t ForcedReactionCount = 32;
    constexpr uint8_t VisibleEquipableSpellCount = 16;
    constexpr uint8_t PlunderstormItemDisplayCount = 19;

    // the quest of a log slot: id, state flags, objective progress, end time, objective flags, enabled objectives
    void playerQuestLogCreate(Player* player, uint8_t slot, ByteBuffer& data)
    {
        const QuestLogEntry* questLog = slot < MAX_QUEST_LOG_SIZE ? player->getQuestLogBySlotId(slot) : nullptr;
        data << int32_t(questLog != nullptr ? questLog->getQuestProperties()->id : 0);
        data << uint16_t(0);                                                // state flags
        for (uint8_t i = 0; i < 24; ++i)
            data << int16_t(questLog != nullptr && i < 4 ? questLog->getMobCountByIndex(i) : 0);
        data << int64_t(0);                                                 // end time
        data << uint32_t(0);                                                // objective flags
        data << uint32_t(0);                                                // enabled objectives mask
    }

    // the quest log entry with its own mask: every field, the objective progress follows the plain fields
    void playerQuestLogUpdate(Player* player, uint8_t slot, ByteBuffer& data)
    {
        const QuestLogEntry* questLog = slot < MAX_QUEST_LOG_SIZE ? player->getQuestLogBySlotId(slot) : nullptr;
        data.writeBits(1, 1);
        data.writeBits(0x7FFFFFFF, 32);
        data.flushBits();
        data << int32_t(questLog != nullptr ? questLog->getQuestProperties()->id : 0);
        data << uint16_t(0);                                                // state flags
        data << int64_t(0);                                                 // end time
        data << uint32_t(0);                                                // objective flags
        data << uint32_t(0);                                                // enabled objectives mask
        for (uint8_t i = 0; i < 24; ++i)
            data << int16_t(questLog != nullptr && i < 4 ? questLog->getMobCountByIndex(i) : 0);
    }

    // quest id to log slot, the client finds its entries through it
    void playerQuestIndexCreate(Player* player, ByteBuffer& data)
    {
        std::vector<std::pair<int32_t, int32_t>> entries;
        for (uint8_t slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
            if (const QuestLogEntry* questLog = player->getQuestLogBySlotId(slot))
                entries.emplace_back(static_cast<int32_t>(questLog->getQuestProperties()->id), static_cast<int32_t>(slot));

        data << uint32_t(entries.size());
        for (const auto& [questId, slot] : entries)
            data << questId << slot;
    }

    // update: the whole map instead of changes
    void playerQuestIndexUpdate(Player* player, ByteBuffer& data)
    {
        data << uint8_t(1);
        playerQuestIndexCreate(player, data);
    }

    void playerVisibleItemFields(Player* player, uint8_t slot, ByteBuffer& data)
    {
        data << int32_t(player->getVisibleItemEntry(slot));
        data << int32_t(0);                                                 // secondary item modified appearance
        data << int32_t(0);                                                 // conditional item appearance
        data << uint16_t(0);                                                // appearance modifier
        data << uint16_t(player->getVisibleItemEnchantment(slot, 0));       // visual: the permanent enchantment
        data << uint32_t(0);                                                // item modified appearance
        data << uint8_t(0);                                                 // transmog slot option
        data << uint8_t(0);                                                 // sheathe category
    }

    void playerVisibleItemCreate(Player* player, uint8_t slot, ByteBuffer& data)
    {
        playerVisibleItemFields(player, slot, data);
        data.writeBit(false);                                               // has transmog
        data.writeBit(false);                                               // has illusion
        data.flushBits();
    }

    // the visible item with its own mask: every field, the bits precede the fields
    void playerVisibleItemUpdate(Player* player, uint8_t slot, ByteBuffer& data)
    {
        data.writeBits(0x7FF, 11);
        data.writeBit(false);                                               // has transmog
        data.writeBit(false);                                               // has illusion
        data.flushBits();
        playerVisibleItemFields(player, slot, data);
    }

    // the content tuning options: conditional flags, faction group, chromie time expansion mask; an update carries
    // the whole structure
    void playerCtrOptionsCreate(ByteBuffer& data)
    {
        data << uint32_t(0) << uint8_t(0) << uint32_t(0);
    }

    // the dungeon score summary: two scores, no runs
    void playerDungeonScoreSummaryCreate(ByteBuffer& data)
    {
        data << float(0.0f) << float(0.0f) << uint32_t(0);
    }

    // a forced reaction of the zone: faction, reaction
    void playerForcedReactionCreate(ByteBuffer& data)
    {
        data << int32_t(0) << int32_t(0);
    }

    // the leaver info: battle net account, leave score, season, leaves, successes, consecutive successes,
    // last penalty time, expiration time, flags, the leaver bit; an update carries the whole structure
    void playerLeaverInfoCreate(ByteBuffer& data)
    {
        data << WoWGuid128();
        data << float(0.0f);
        data << uint32_t(0) << uint32_t(0) << uint32_t(0);
        data << int32_t(0);
        data << int64_t(0) << int64_t(0);
        data << int32_t(0);
        data.writeBits(0, 1);
        data.flushBits();
    }

    // the personal tabard: emblem style and color, border style and color, background color
    void playerPersonalTabardCreate(ByteBuffer& data)
    {
        for (uint8_t i = 0; i < 5; ++i)
            data << int32_t(0);
    }

    // an item instance of the equipable spells: no item, no modifiers (the size takes seven bits), no bonus
    void playerItemInstanceCreate(ByteBuffer& data)
    {
        data << int32_t(0);                                                 // item
        data.writeBits(0, 7);                                               // modifiers
        data.flushBits();
        data.writeBit(false);                                               // has an item bonus
        data.flushBits();
    }

    // the npc as player info: unknown value, character loadout, creature, position, facing, transport; an update
    // carries the whole structure
    void playerNpcAsPlayerInfoCreate(ByteBuffer& data)
    {
        data << int32_t(0) << int32_t(0) << int32_t(0);
        data << float(0.0f) << float(0.0f) << float(0.0f);
        data << float(0.0f);
        data << WoWGuid128();
    }

    // the look of a player as customization choices, converted from the stored bytes
    static std::vector<CharacterCustomizations::ChrCustomizationChoice> playerCustomizations(Player* player)
    {
        if (!player->getCustomizations().empty())
            return player->getCustomizations();

        CharacterCustomizations::LegacyLook look;
        look.skin = player->getSkinColor();
        look.face = player->getFace();
        look.hairStyle = player->getHairStyle();
        look.hairColor = player->getHairColor();
        look.facialStyle = player->getFacialFeatures();
        return CharacterCustomizations::convertLegacyLook(static_cast<uint8_t>(player->getRace()), static_cast<uint8_t>(player->getClass()), player->getPlayerGender(), look);
    }

    void playerCreate(Player* player, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;
        const std::string name = player->getName();
        const uint32_t accountId = player->getSession() != nullptr ? player->getSession()->GetAccountId() : 0;

        ctx.guid(player->getDuelArbiter());
        data << WoWGuid128::global(HighGuid128::WowAccount, accountId);
        data << WoWGuid128::global(HighGuid128::BNetAccount, accountId);
        data << uint64_t(0);                                                // guild club member
        data << WoWGuid128();                                               // loot target
        data << uint32_t(player->getPlayerFlags());
        data << uint32_t(0);                                                // player flags ex
        data << uint32_t(player->getGuildRank());
        data << uint32_t(player->getField<uint32_t>(PlayerField::GuildDeleteDate));
        data << int32_t(player->getField<uint32_t>(PlayerField::GuildLevel));
        const auto customizations = playerCustomizations(player);
        data << uint32_t(customizations.size());
        data << uint32_t(0);                                                // random customizations
        data << uint8_t(0) << uint8_t(0);                                   // party type
        data << uint8_t(player->getPlayerGender());                         // native sex
        data << uint8_t(player->getDrunkValue());                           // inebriation
        data << uint8_t(player->getPvpRank());                              // pvp title
        data << uint8_t(player->getArenaFaction());
        data << uint32_t(player->getDuelTeam());
        data << int32_t(player->getGuildTimestamp());

        if (ctx.partyMember())
        {
            for (uint8_t i = 0; i < QuestLogCount; ++i)
                playerQuestLogCreate(player, i, data);
            playerQuestIndexCreate(player, data);
            data << uint32_t(0);                                            // quest session quest log
        }

        for (uint8_t i = 0; i < VisibleItemCount; ++i)
            playerVisibleItemCreate(player, i, data);

        data << int32_t(player->getChosenTitle());
        data << int32_t(player->getField<uint32_t>(PlayerField::Inebriation)); // fake inebriation
        data << uint32_t(player->getField<uint32_t>(PlayerField::VirtualPlayerRealm));
        data << uint32_t(player->getCurrentSpecId());
        data << int32_t(0);                                                 // current combat trait config sub tree
        data << int32_t(player->getField<uint32_t>(PlayerField::TaxiMountAnimKitId));
        for (uint8_t i = 0; i < AvgItemLevelCount; ++i)
            data << float(0.0f);                                            // average item level
        data << uint8_t(player->getField<uint8_t>(PlayerField::CurrentBattlePetBreedQuality));
        data << int32_t(0);                                                 // honor level
        data << int64_t(0);                                                 // logout time
        data << uint32_t(0);                                                // arena cooldowns
        for (uint8_t i = 0; i < ForcedReactionCount; ++i)
            playerForcedReactionCreate(data);
        data << int32_t(0);                                                 // offered adventure journal quest
        data << int32_t(0);                                                 // offered script quest
        data << int32_t(0);                                                 // current battle pet species
        data << uint32_t(0);                                                // pet names
        playerCtrOptionsCreate(data);
        data << int32_t(0);                                                 // covenant
        data << int32_t(0);                                                 // soulbind
        playerDungeonScoreSummaryCreate(data);
        playerLeaverInfoCreate(data);
        data << WoWGuid128();                                               // spectate target
        data << int32_t(0);                                                 // world loot object inventory swap slot
        for (uint8_t i = 0; i < VisibleEquipableSpellCount; ++i)
            playerItemInstanceCreate(data);
        data << uint32_t(0);                                                // visual item replacements
        for (uint8_t i = 0; i < PlunderstormItemDisplayCount; ++i)
            data << uint32_t(0);                                            // plunderstorm item display
        playerPersonalTabardCreate(data);
        playerNpcAsPlayerInfoCreate(data);
        for (const auto& customization : customizations)
            data << customization.optionId << customization.choiceId;

        data.writeBits(static_cast<uint32_t>(name.size()), 6);
        if (ctx.partyMember())
            data.writeBit(false);                                           // has a quest session
        data.writeBit(false);                                               // has a level link
        data.writeBit(false);                                               // has declined names
        data.flushBits();
        data.writeString(name);
    }

    void playerCollect(ChangedFields const& changed, PlayerMask& mask)
    {
        struct Single { uint8_t bit; PlayerField field; };
        static constexpr Single singles[] =
        {
            { 9, PlayerField::DuelArbiter }, { 14, PlayerField::PlayerFlags }, { 16, PlayerField::GuildRank }, { 17, PlayerField::GuildDeleteDate },
            { 18, PlayerField::GuildLevel }, { 23, PlayerField::DuelTeam }, { 24, PlayerField::GuildTimestamp }, { 26, PlayerField::ChosenTitle },
            { 27, PlayerField::Inebriation }, { 28, PlayerField::VirtualPlayerRealm }, { 29, PlayerField::CurrentSpecId }, { 31, PlayerField::TaxiMountAnimKitId },
            { 33, PlayerField::CurrentBattlePetBreedQuality }
        };

        for (const Single& single : singles)
            if (changed.changed(single.field))
                mask.set(single.bit);

        // the look bytes become the customization list
        if (changed.changed(PlayerField::PlayerBytes) || changed.changed(PlayerField::PlayerBytes2))
            mask.set(3);
        if (changed.changed(PlayerField::PlayerBytes3))
            mask.setRange(19, 4);

        for (uint8_t i = 0; i < VisibleItemCount; ++i)
            if (changed.changed(PlayerField::VisibleItemsEntry, i) || changed.changed(PlayerField::VisibleItemsEnchantment, i))
                mask.set(230 + i);

        // quest log slots, a changed id renews the index map
        for (uint8_t i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
        {
            if (changed.changed(PlayerField::QuestsQuestId, i))
                mask.set(25);
            if (changed.changed(PlayerField::QuestsQuestId, i) || changed.changed(PlayerField::QuestsState, i) ||
                changed.changed(PlayerField::QuestsRequiredCountState, i) || changed.changed(PlayerField::QuestsRequiredMobOrGo, i) ||
                changed.changed(PlayerField::QuestsExpireTime, i))
                mask.set(54 + i);
        }

        // group bits: plain fields, quest log, visible items
        struct Group { uint16_t parent; uint16_t first; uint16_t count; };
        static constexpr Group groups[] = { { 0, 1, 31 }, { 32, 33, 17 }, { 53, 54, QuestLogCount }, { 229, 230, VisibleItemCount } };
        for (const Group& group : groups)
            for (uint16_t i = 0; i < group.count; ++i)
                if (mask[group.first + i])
                    mask.set(group.parent);
    }

    void playerFilter(PlayerMask& mask, uint8_t flags)
    {
        uint32_t allowed[11] = { 0xFDFFFFDD, 0x001FFFFF, 0, 0, 0, 0, 0, 0xFFFFFFE0, 0xFFFFFFFF, 0xFFFFFFFF, 0x0000003F };
        if (flags & FlagPartyMember)
        {
            const uint32_t party[11] = { 0x02000022, 0xFFE00000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0000001F, 0, 0, 0 };
            for (size_t i = 0; i < 11; ++i)
                allowed[i] |= party[i];
        }
        mask.filter(allowed);
    }

    void playerUpdate(Player* player, Context const& ctx, PlayerMask const& mask)
    {
        ByteBuffer& data = ctx.data;
        const std::string name = player->getName();
        const uint32_t accountId = player->getSession() != nullptr ? player->getSession()->GetAccountId() : 0;

        const auto customizations = mask[3] ? playerCustomizations(player) : std::vector<CharacterCustomizations::ChrCustomizationChoice>();

        mask.writeBlocks(data);
        data.writeBit(false);                                               // the party member fields keep their change masks
        if (mask[0])
        {
            if (mask[1])
                data.writeBit(false);                                       // has a quest session
            if (mask[2])
                data.writeBit(false);                                       // has a level link
            // the dynamic fields: customizations, random customizations, quest session quest log, arena cooldowns,
            // pet names, visual item replacements
            if (mask[3])
                writeDynamicMask(data, static_cast<uint32_t>(customizations.size()));
            for (uint8_t bit = 4; bit <= 8; ++bit)
                if (mask[bit])
                    writeDynamicMask(data, 0);
        }
        data.flushBits();

        if (mask[0])
        {
            if (mask[3])
                for (const auto& customization : customizations)
                    data << customization.optionId << customization.choiceId;
            if (mask[9])
                ctx.guid(player->getDuelArbiter());
            if (mask[10])
                data << WoWGuid128::global(HighGuid128::WowAccount, accountId);
            if (mask[11])
                data << WoWGuid128::global(HighGuid128::BNetAccount, accountId);
            if (mask[12])
                data << uint64_t(0);
            if (mask[13])
                data << WoWGuid128();
            if (mask[14])
                data << uint32_t(player->getPlayerFlags());
            if (mask[15])
                data << uint32_t(0);
            if (mask[16])
                data << uint32_t(player->getGuildRank());
            if (mask[17])
                data << uint32_t(player->getField<uint32_t>(PlayerField::GuildDeleteDate));
            if (mask[18])
                data << int32_t(player->getField<uint32_t>(PlayerField::GuildLevel));
            if (mask[19])
                data << uint8_t(player->getPlayerGender());
            if (mask[20])
                data << uint8_t(player->getDrunkValue());
            if (mask[21])
                data << uint8_t(player->getPvpRank());
            if (mask[22])
                data << uint8_t(player->getArenaFaction());
            if (mask[23])
                data << uint32_t(player->getDuelTeam());
            if (mask[24])
                data << int32_t(player->getGuildTimestamp());
            if (mask[25])
                playerQuestIndexUpdate(player, data);
            if (mask[26])
                data << int32_t(player->getChosenTitle());
            if (mask[27])
                data << int32_t(player->getField<uint32_t>(PlayerField::Inebriation));
            if (mask[28])
                data << uint32_t(player->getField<uint32_t>(PlayerField::VirtualPlayerRealm));
            if (mask[29])
                data << uint32_t(player->getCurrentSpecId());
            if (mask[30])
                data << int32_t(0);
            if (mask[31])
                data << int32_t(player->getField<uint32_t>(PlayerField::TaxiMountAnimKitId));
        }

        if (mask[32])
        {
            if (mask[33])
                data << uint8_t(player->getField<uint8_t>(PlayerField::CurrentBattlePetBreedQuality));
            if (mask[34])
                data << int32_t(0);
            if (mask[35])
                data << int64_t(0);
            if (mask[37])
                data << int32_t(0);
            if (mask[38])
                data << int32_t(0);
            if (mask[39])
                data << int32_t(0);
            if (mask[40])
                playerCtrOptionsCreate(data);
            if (mask[41])
                data << int32_t(0);
            if (mask[42])
                data << int32_t(0);
            if (mask[43])
                playerDungeonScoreSummaryCreate(data);
            if (mask[44])
                playerLeaverInfoCreate(data);
            if (mask[45])
                data << WoWGuid128();
            if (mask[46])
                data << int32_t(0);
            if (mask[48])
            {
                // the personal tabard with its own mask: every field
                data.writeBits(0x3F, 6);
                data.flushBits();
                playerPersonalTabardCreate(data);
            }
            if (mask[49])
                playerNpcAsPlayerInfoCreate(data);
            if (mask[36])
                data.writeBits(static_cast<uint32_t>(name.size()), 6);
            data.writeBits(0, 1);                                           // has declined names
        }
        data.flushBits();
        if (mask[32])
        {
            if (mask[36])
                data.writeString(name);
        }

        if (mask[50])
        {
            for (uint8_t i = 0; i < 2; ++i)
                if (mask[51 + i])
                    data << uint8_t(0);
        }

        if (mask[53])
        {
            for (uint8_t i = 0; i < QuestLogCount; ++i)
                if (mask[54 + i])
                    playerQuestLogUpdate(player, i, data);
        }

        if (mask[229])
        {
            for (uint8_t i = 0; i < VisibleItemCount; ++i)
                if (mask[230 + i])
                    playerVisibleItemUpdate(player, i, data);
        }

        if (mask[249])
        {
            for (uint8_t i = 0; i < AvgItemLevelCount; ++i)
                if (mask[250 + i])
                    data << float(0.0f);
        }

        if (mask[256])
        {
            for (uint8_t i = 0; i < ForcedReactionCount; ++i)
            {
                if (!mask[257 + i])
                    continue;

                // the forced reaction with its own mask: every field
                data.writeBits(0x7, 3);
                data.flushBits();
                playerForcedReactionCreate(data);
            }
        }

        if (mask[289])
        {
            for (uint8_t i = 0; i < VisibleEquipableSpellCount; ++i)
                if (mask[290 + i])
                    playerItemInstanceCreate(data);
        }

        if (mask[306])
        {
            for (uint8_t i = 0; i < PlunderstormItemDisplayCount; ++i)
                if (mask[307 + i])
                    data << uint32_t(0);
        }

        data.flushBits();
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // ActivePlayerData: the player itself
    using ActivePlayerMask = ChangeMask<398>;

    constexpr uint8_t InvSlotCount = 105;
    constexpr uint16_t ExploredZoneCount = 240;
    constexpr uint16_t SkillCount = 300;
    constexpr uint16_t LegacySkillCount = 300;      // the Midnight layout stores every slot
    constexpr uint8_t KnownTitleCount = 6;
    constexpr uint8_t BitVectorCount = 14;
    constexpr uint8_t ExploredZoneBitVector = 1;
    constexpr uint8_t BagSlotFlagCount = 5;
    constexpr uint8_t ItemUpgradeWatermarkCount = 17;

    // the inventory of the server layout in the order of the client slots: equipment, profession slots, bags,
    // reagent bag, backpack, bank bags, buyback, child equipment, equipable spells, account bank bags
    struct InvSlotRange { PlayerField field; uint16_t first; uint16_t count; bool mapped; };
    constexpr InvSlotRange InvSlotRanges[] =
    {
        { PlayerField::InventorySlot, 0, 19, true }, { PlayerField::ProfessionSlot, 0, 11, true }, { PlayerField::InventorySlot, 19, 4, true },
        { PlayerField::InventorySlot, 0, 1, false }, { PlayerField::PackSlot, 0, 20, true }, { PlayerField::PackSlot, 0, 8, false },
        { PlayerField::BankBagSlot, 0, 6, true }, { PlayerField::VendorBuyBackSlot, 0, 12, true }
    };

    uint64_t activeInvSlot(Player* player, uint16_t slot)
    {
        for (const InvSlotRange& range : InvSlotRanges)
        {
            if (slot < range.count)
                return range.mapped ? player->getField<uint64_t>(range.field, range.first + slot) : 0;
            slot -= range.count;
        }

        return 0;
    }

    bool activeInvSlotChanged(ChangedFields const& changed, uint16_t slot)
    {
        for (const InvSlotRange& range : InvSlotRanges)
        {
            if (slot < range.count)
                return range.mapped && changed.changed(range.field, range.first + slot);
            slot -= range.count;
        }

        return false;
    }

    // the explored zones as 64 bit words, the server layout ends after 160 of the 240 words
    uint64_t activeExploredZone(Player* player, uint16_t index)
    {
        return uint64_t(player->getExploredZone(index * 2)) | (uint64_t(player->getExploredZone(index * 2 + 1)) << 32);
    }

    // the bit vectors: the explored zones in the second vector, the others stay empty
    uint32_t activeBitVectorCount(uint8_t vector)
    {
        return vector == ExploredZoneBitVector ? ExploredZoneCount : 0;
    }

    void activeBitVectorValues(Player* player, uint8_t vector, ByteBuffer& data)
    {
        if (vector != ExploredZoneBitVector)
            return;

        for (uint16_t i = 0; i < ExploredZoneCount; ++i)
            data << activeExploredZone(player, i);
    }

    void activeBitVectorsCreate(Player* player, ByteBuffer& data)
    {
        for (uint8_t i = 0; i < BitVectorCount; ++i)
        {
            data << uint32_t(activeBitVectorCount(i));
            activeBitVectorValues(player, i, data);
        }
    }

    // the bit vectors with their own masks: every vector, every value
    void activeBitVectorsUpdate(Player* player, ByteBuffer& data)
    {
        data.writeBits(1, 1);
        data.writeBits(0x7FFF, 32);
        data.flushBits();
        for (uint8_t i = 0; i < BitVectorCount; ++i)
        {
            data.writeBits(0x3, 2);
            writeDynamicMask(data, activeBitVectorCount(i));
            data.flushBits();
            activeBitVectorValues(player, i, data);
        }
    }

    uint16_t activeSkill(Player* player, PlayerField field, uint16_t index)
    {
        if (index >= LegacySkillCount)
            return 0;

        const uint32_t value = player->getField<uint32_t>(field, index / 2);
        return static_cast<uint16_t>(index % 2 == 0 ? value & 0xFFFF : value >> 16);
    }

    void activeSkillCreate(Player* player, ByteBuffer& data)
    {
        for (uint16_t i = 0; i < SkillCount; ++i)
        {
            data << activeSkill(player, PlayerField::FieldSkillInfoSkillLine, i);
            data << activeSkill(player, PlayerField::FieldSkillInfoSkillStep, i);
            data << activeSkill(player, PlayerField::FieldSkillInfoSkillRank, i);
            data << activeSkill(player, PlayerField::FieldSkillInfoSkillStartingRank, i);
            data << activeSkill(player, PlayerField::FieldSkillInfoSkillMaxRank, i);
            data << int16_t(activeSkill(player, PlayerField::FieldSkillInfoSkillMod, i));
            data << activeSkill(player, PlayerField::FieldSkillInfoSkillTalent, i);
        }
    }

    // the skills with their own mask: the whole structure, 300 slots of seven fields and the group bit
    void activeSkillUpdate(Player* player, ByteBuffer& data)
    {
        ChangeMask<2101> mask;
        mask.setAll();
        mask.writeLarge(data);
        data.flushBits();
        activeSkillCreate(player, data);
    }

    void activeStatsCreate(Player* player, ByteBuffer& data)
    {
        data << player->getField<float>(PlayerField::Expertise);
        data << player->getField<float>(PlayerField::OffhandExpertise);
        data << player->getField<float>(PlayerField::RangedExpertise);
        data << player->getField<float>(PlayerField::CombatRatingExpertise);
        data << float(player->getBlockPercentage());
        data << float(player->getDodgePercentage());
        data << float(0.0f);                                                // dodge from attribute
        data << float(player->getParryPercentage());
        data << float(0.0f);                                                // parry from attribute
        data << player->getField<float>(PlayerField::CritPct);
        data << float(player->getRangedCritPercentage());
        data << player->getField<float>(PlayerField::OffhandCritPct);
        data << player->getField<float>(PlayerField::SpellCritPct);
        data << int32_t(player->getShieldBlock());
        data << float(player->getShieldBlockCritPercentage());
        data << player->getField<float>(PlayerField::Mastery);
        data << float(0.0f);                                                // speed
        data << float(0.0f);                                                // avoidance
        data << float(0.0f);                                                // sturdiness
        data << int32_t(0);                                                 // versatility
        data << float(0.0f);                                                // versatility bonus
        data << player->getField<float>(PlayerField::PvpPowerDamage);
        data << player->getField<float>(PlayerField::PvpPowerHealing);
    }

    // the rest info: threshold and state
    void activeRestInfoCreate(Player* player, uint8_t index, ByteBuffer& data)
    {
        if (index == 0)
            data << uint32_t(player->getRestStateXp()) << uint8_t(player->getRestState());
        else
            data << uint32_t(0) << uint8_t(0);                              // honor rest
    }

    // the dungeon score: no seasons, no runs
    void activeDungeonScoreCreate(ByteBuffer& data)
    {
        data << uint32_t(0) << int32_t(0);
    }

    // the frozen perks vendor item: available until, vendor item, mount, battle pet species, transmog set,
    // modified appearance, transmog illusion, toy, price, original price, warband scene, the disabled and
    // does not expire bits
    void activePerksVendorItemCreate(ByteBuffer& data)
    {
        data << int64_t(0);
        for (uint8_t i = 0; i < 10; ++i)
            data << int32_t(0);
        data.writeBit(false);
        data.writeBit(false);
        data.flushBits();
    }

    // the item upgrade watermarks: one per equipment slot and the one hand weapon, finger and trinket values
    void activeItemUpgradeCreate(ByteBuffer& data)
    {
        for (uint8_t i = 0; i < ItemUpgradeWatermarkCount; ++i)
            data << float(0.0f);
        data << int32_t(0);                                                 // one hand weapon item
        data << int32_t(0);                                                 // finger item
        data << float(0.0f);                                                // finger watermark
        data << int32_t(0);                                                 // trinket item
        data << float(0.0f);                                                // trinket watermark
    }

    // the discord info: the access token length with its terminator, discord user, account type, guild lobby, guild
    // settings, display name type; the empty token has no characters. An update carries the whole structure
    void activeDiscordInfoCreate(ByteBuffer& data)
    {
        data.writeBits(1, 24);
        data.flushBits();
        data << uint64_t(0) << uint8_t(0) << uint64_t(0) << uint8_t(0) << uint8_t(0);
    }

    // the level link info: target and level
    void activeLevelLinkInfoCreate(ByteBuffer& data)
    {
        data << WoWGuid128() << int32_t(0);
    }

    // the level link info with its own mask: nothing changed
    void activeLevelLinkInfoUpdate(ByteBuffer& data)
    {
        data.writeBits(0, 3);
        data.flushBits();
    }

    // a transmog outfit: id, the info (set type, icon, name, the situations bit), situations, slots, flags
    void activeTransmogOutfitCreate(ByteBuffer& data)
    {
        data << uint32_t(0);                                                // id
        data << uint8_t(0);                                                 // set type
        data << uint32_t(0);                                                // icon
        data.writeBits(0, 8);                                               // name
        data.writeBit(false);                                               // situations enabled
        data.flushBits();
        data << uint32_t(0);                                                // situations
        data << uint32_t(0);                                                // slots
        data << uint32_t(0);                                                // flags
    }

    // a transmog outfit with its own mask: nothing changed
    void activeTransmogOutfitUpdate(ByteBuffer& data)
    {
        data.writeBits(0, 5);
        data.flushBits();
    }

    // the transmog metadata: situation trigger, outfit, stamped options of the main and off hand, cost modifier, the
    // locked bit; an update carries the whole structure
    void activeTransmogMetadataCreate(ByteBuffer& data)
    {
        data << uint8_t(0) << uint32_t(0) << uint8_t(0) << uint8_t(0) << float(0.0f);
        data.writeBit(false);
        data.flushBits();
    }

    void activePlayerCreate(Player* player, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;

        for (uint8_t i = 0; i < InvSlotCount; ++i)
            ctx.guid(activeInvSlot(player, i));
        ctx.guid(player->getFarsightGuid());
        data << WoWGuid128();                                               // summoned battle pet
        data << uint32_t(KnownTitleCount);
        data << uint64_t(player->getCoinage());
        data << uint64_t(0);                                                // account bank coinage
        data << int32_t(player->getXp());
        data << int32_t(player->getNextLevelXp());
        data << int32_t(0);                                                 // trial xp
        activeSkillCreate(player, data);
        data << int32_t(player->getField<uint32_t>(PlayerField::CharacterPoints1));
        data << int32_t(player->getField<uint32_t>(PlayerField::MaxTalentTiers));
        data << uint32_t(player->getTrackCreature());
        activeStatsCreate(player, data);
        activeBitVectorsCreate(player, data);
        data << uint32_t(0);                                                // character data elements
        data << uint32_t(0);                                                // account data elements

        for (uint8_t i = 0; i < 2; ++i)
            activeRestInfoCreate(player, i, data);

        // the healing done percent is one value for every school in the server layout
        for (uint8_t i = 0; i < 7; ++i)
        {
            data << int32_t(player->getModDamageDonePositive(i));
            data << int32_t(player->getModDamageDoneNegative(i));
            data << float(player->getModDamageDonePct(i));
            data << player->getField<float>(PlayerField::FieldModHealingDonePct);
        }

        data << int32_t(player->getModHealingDone());
        data << player->getField<float>(PlayerField::FieldModHealingPct);
        data << player->getField<float>(PlayerField::FieldModPeriodicHealingDonePct);

        for (uint8_t i = 0; i < 3; ++i)
        {
            data << player->getField<float>(PlayerField::WeaponDmgMultiplier, i);
            data << float(1.0f);                                            // weapon attack speed multiplier
        }

        data << player->getField<float>(PlayerField::ModSpellPowerPct);
        data << player->getField<float>(PlayerField::ModResiliencePct);
        data << player->getField<float>(PlayerField::OverrideSpellPowerByApPct);
        data << player->getField<float>(PlayerField::OverrideApBySpellPowerPct);
        data << int32_t(player->getField<uint32_t>(PlayerField::FieldModTargetResistance));
        data << int32_t(player->getField<uint32_t>(PlayerField::FieldModTargetPhysicalResistance));
        data << uint32_t(PLAYER_LOCAL_FLAG_ACCOUNT_SECURED);                // local flags
        data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytesRafLevel));
        data << uint8_t(player->getEnabledActionBars());
        data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytesMaxPvpRank));
        data << uint8_t(0);                                                 // respecs
        data << uint32_t(player->getField<uint32_t>(PlayerField::FieldPvpMedals));

        for (uint8_t i = 0; i < 12; ++i)
        {
            data << player->getField<uint32_t>(PlayerField::FieldBuyBackPrice, i);
            data << int64_t(player->getField<uint32_t>(PlayerField::FieldBuyBackTimestamp, i));
        }

        data << player->getField<uint16_t>(PlayerField::FieldKillsKillsToday);
        data << player->getField<uint16_t>(PlayerField::FieldKillsKillsYesterday);
        data << player->getField<uint32_t>(PlayerField::FieldLifetimeHonorableKills);
        data << int32_t(player->getWatchedFaction());

        for (uint8_t i = 0; i < 32; ++i)
            data << int32_t(player->getCombatRating(i));

        data << uint32_t(0);                                                // pvp info
        data << int32_t(player->getMaxLevel());
        data << int32_t(0);                                                 // scaling player level delta
        data << int32_t(0);                                                 // max creature scaling level
        data << uint8_t(0);                                                 // transmog cost min scaling level

        for (uint8_t i = 0; i < 4; ++i)
            data << player->getField<uint32_t>(PlayerField::NoReagentCost, i);

        data << int32_t(player->getField<uint32_t>(PlayerField::PetSpellPower));

        for (uint8_t i = 0; i < 2; ++i)
            data << int32_t(player->getProfessionSkillLine(i));

        data << player->getField<float>(PlayerField::UiHitMod);
        data << player->getField<float>(PlayerField::UiHitSpellMod);
        data << int32_t(player->getField<uint32_t>(PlayerField::UiHomeRealmTimeOffset));
        data << player->getField<float>(PlayerField::ModPetHaste);
        data << int8_t(0);                                                  // jailers tower level max
        data << int8_t(0);                                                  // jailers tower level
        data << uint8_t(0);                                                 // local regen flags
        data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytes2AuraVision));
        data << uint8_t(20);                                                // backpack slots, 16 and 4 of a secured account
        data << int32_t(player->getField<uint32_t>(PlayerField::OverrideSpellId));
        data << uint16_t(player->getField<uint16_t>(PlayerField::LootSpecId));
        data << uint32_t(player->getField<uint32_t>(PlayerField::OverrideZonePvpType));

        for (uint8_t i = 0; i < BagSlotFlagCount; ++i)
            data << uint32_t(0);                                            // bag slot flags

        data << int32_t(0);                                                 // honor
        data << int32_t(0);                                                 // honor next level
        data << int32_t(0);                                                 // perks program currency
        data << uint8_t(player->getBankSlots());
        data << uint8_t(0);                                                 // character bank tabs
        data << uint8_t(0);                                                 // account bank tabs

        // the research lists: sites, site progress, research
        data << uint32_t(0) << uint32_t(0) << uint32_t(0);

        // the dynamic fields: daily quests, field 1328, quest lines, heirlooms, heirloom flags, toys, toy flags,
        // transmog, conditional transmog, self res spells, runeforge powers, transmog illusions, warband scenes,
        // house rooms, house exterior components, house themes, house room component textures, house types,
        // unlocked transmog outfits, character restrictions, spell modifiers by label (percent, flat), pvp spell
        // modifiers by label (percent, flat)
        for (uint8_t i = 0; i < 24; ++i)
            data << uint32_t(0);
        data << uint32_t(0);                                                // research history: completed projects
        data << uint32_t(0);                                                // maw powers
        data << uint32_t(0);                                                // multi floor exploration
        data << uint32_t(0);                                                // recipe progression
        activePerksVendorItemCreate(data);
        activeDiscordInfoCreate(data);
        data << uint32_t(0);                                                // replayed quests
        data << uint32_t(0);                                                // task quests
        activeLevelLinkInfoCreate(data);
        data << uint32_t(0);                                                // disabled spells
        data << int32_t(0);                                                 // ui chromie time expansion
        data << int32_t(0);                                                 // timerunning season
        data << int32_t(0);                                                 // transport server time
        data << uint32_t(0);                                                // weekly rewards period since origin
        data << int16_t(0);                                                 // soulbind conduit rank
        activeDungeonScoreCreate(data);
        writeMapFieldCreate(data);                                          // trait configs
        data << uint32_t(0);                                                // active combat trait config
        data << uint32_t(0);                                                // crafting orders
        data << uint32_t(0);                                                // personal crafting order counts
        data << uint32_t(0);                                                // npc crafting orders
        data << uint32_t(0);                                                // category cooldown mods
        data << uint32_t(0);                                                // weekly spell uses
        activeItemUpgradeCreate(data);
        data << uint64_t(0);                                                // loot history instance
        data << uint32_t(0);                                                // tracked collectable sources
        data << uint8_t(0);                                                 // required mount capability flags
        writeMapFieldCreate(data);                                          // delve data
        writeMapFieldCreate(data);                                          // transmog outfits
        activeTransmogOutfitCreate(data);                                   // viewed outfit
        activeTransmogMetadataCreate(data);
        data.flushBits();

        for (uint8_t i = 0; i < KnownTitleCount; ++i)
            data << uint64_t(player->getKnownTitles(i));

        data.writeBit(false);                                               // backpack auto sort disabled
        data.writeBit(false);                                               // backpack sell junk disabled
        data.writeBit(false);                                               // bank auto sort disabled
        data.writeBit(false);                                               // sort bags right to left
        data.writeBit(false);                                               // insert items left to right
        data.writeBit(false);                                               // has a perks program pending reward
        data.writeBits(0, 1);                                               // has a quest session
        data.writeBits(0, 1);                                               // has a pet stable
        data.writeBits(0, 3);                                               // character bank tab settings
        data.writeBits(0, 3);                                               // account bank tab settings
        data.writeBits(0, 1);                                               // has walk in data
        data.writeBits(0, 1);                                               // has challenge mode data
        data.flushBits();
    }

    void activePlayerCollect(ChangedFields const& changed, ActivePlayerMask& mask)
    {
        struct Single { uint16_t bit; PlayerField field; };
        static constexpr Single singles[] =
        {
            { 7, PlayerField::FieldKnownTitles }, { 56, PlayerField::FarsightGuid }, { 57, PlayerField::SummonedBattlePetGuid }, { 58, PlayerField::FieldCoinage },
            { 60, PlayerField::Xp }, { 61, PlayerField::NextLevelXp }, { 63, PlayerField::FieldSkillInfo }, { 64, PlayerField::CharacterPoints1 },
            { 65, PlayerField::MaxTalentTiers }, { 66, PlayerField::TrackCreatures }, { 67, PlayerField::Expertise }, { 68, PlayerField::OffhandExpertise },
            { 69, PlayerField::RangedExpertise }, { 71, PlayerField::CombatRatingExpertise }, { 72, PlayerField::BlockPct }, { 73, PlayerField::DodgePct },
            { 75, PlayerField::ParryPct }, { 77, PlayerField::CritPct }, { 78, PlayerField::RangedCritPct }, { 79, PlayerField::OffhandCritPct },
            { 80, PlayerField::SpellCritPct }, { 81, PlayerField::ShieldBlock }, { 82, PlayerField::ShieldBlockCritPct }, { 83, PlayerField::Mastery },
            { 89, PlayerField::PvpPowerDamage }, { 90, PlayerField::PvpPowerHealing }, { 92, PlayerField::FieldModHealingDone }, { 93, PlayerField::FieldModHealingPct },
            { 94, PlayerField::FieldModPeriodicHealingDonePct }, { 95, PlayerField::ModSpellPowerPct }, { 96, PlayerField::ModResiliencePct },
            { 97, PlayerField::OverrideSpellPowerByApPct }, { 98, PlayerField::OverrideApBySpellPowerPct }, { 99, PlayerField::FieldModTargetResistance },
            { 100, PlayerField::FieldModTargetPhysicalResistance }, { 107, PlayerField::FieldPvpMedals }, { 110, PlayerField::FieldLifetimeHonorableKills },
            { 111, PlayerField::FieldWatchedFactionIdx }, { 112, PlayerField::FieldMaxLevel }, { 116, PlayerField::PetSpellPower }, { 117, PlayerField::UiHitMod },
            { 118, PlayerField::UiHitSpellMod }, { 119, PlayerField::UiHomeRealmTimeOffset }, { 120, PlayerField::ModPetHaste }, { 124, PlayerField::PlayerFieldBytes2 },
            { 126, PlayerField::OverrideSpellId }, { 127, PlayerField::LootSpecId }, { 128, PlayerField::OverrideZonePvpType }, { 132, PlayerField::PlayerBytes2 }
        };

        for (const Single& single : singles)
            if (changed.changed(single.field))
                mask.set(single.bit);

        if (changed.changed(PlayerField::PlayerFieldBytes))
            mask.setRange(103, 3);
        if (changed.changed(PlayerField::FieldKills))
            mask.setRange(108, 2);

        for (uint8_t i = 0; i < InvSlotCount; ++i)
            if (activeInvSlotChanged(changed, i))
                mask.set(164 + i);
        // the explored zones are the second bit vector, every vector shares one bit
        for (uint16_t i = 0; i < ExploredZoneCount; ++i)
            if (changed.changed(PlayerField::ExploredZones, i * 2) || changed.changed(PlayerField::ExploredZones, i * 2 + 1))
                mask.set(91);
        if (changed.changed(PlayerField::RestStateXp) || changed.changed(PlayerField::PlayerBytes2))
            mask.set(270);
        for (uint8_t i = 0; i < 7; ++i)
        {
            if (changed.changed(PlayerField::FieldModDamageDonePositive, i))
                mask.set(273 + i);
            if (changed.changed(PlayerField::FieldModDamageDoneNegative, i))
                mask.set(280 + i);
            if (changed.changed(PlayerField::FieldModDamageDonePct, i))
                mask.set(287 + i);
            if (changed.changed(PlayerField::FieldModHealingDonePct))
                mask.set(294 + i);
        }
        for (uint8_t i = 0; i < 3; ++i)
            if (changed.changed(PlayerField::WeaponDmgMultiplier, i))
                mask.set(302 + i);
        for (uint8_t i = 0; i < 12; ++i)
        {
            if (changed.changed(PlayerField::FieldBuyBackPrice, i))
                mask.set(309 + i);
            if (changed.changed(PlayerField::FieldBuyBackTimestamp, i))
                mask.set(321 + i);
        }
        for (uint8_t i = 0; i < 32; ++i)
            if (changed.changed(PlayerField::FieldCombatRating, i))
                mask.set(334 + i);
        for (uint8_t i = 0; i < 4; ++i)
            if (changed.changed(PlayerField::NoReagentCost, i))
                mask.set(367 + i);
        for (uint8_t i = 0; i < 2; ++i)
            if (changed.changed(PlayerField::ProfessionSkillLine, i))
                mask.set(372 + i);

        // the group bits: the blocks of the plain fields (the research lists of the second block have their own
        // group bits), then the arrays
        struct Group { uint16_t parent; uint16_t first; uint16_t count; };
        static constexpr Group groups[] =
        {
            { 0, 1, 31 }, { 32, 33, 17 }, { 32, 56, 14 }, { 70, 71, 31 }, { 102, 103, 31 }, { 134, 135, 28 },
            { 50, 51, 1 }, { 52, 53, 1 }, { 54, 55, 1 }, { 163, 164, InvSlotCount }, { 269, 270, 2 }, { 272, 273, 28 },
            { 301, 302, 6 }, { 308, 309, 24 }, { 333, 334, 32 }, { 366, 367, 4 }, { 371, 372, 2 }, { 374, 375, BagSlotFlagCount },
            { 380, 381, ItemUpgradeWatermarkCount }
        };
        for (const Group& group : groups)
            for (uint16_t i = 0; i < group.count; ++i)
                if (mask[group.first + i])
                    mask.set(group.parent);
    }

    void activePlayerUpdate(Player* player, Context const& ctx, ActivePlayerMask const& mask)
    {
        ByteBuffer& data = ctx.data;

        mask.writeBlocks(data);
        if (mask[0])
        {
            for (uint8_t bit = 1; bit <= 6; ++bit)
                if (mask[bit])
                    data.writeBit(false);
            if (mask[7])
                writeDynamicMask(data, KnownTitleCount);
            // the dynamic fields: character data elements, account data elements, pvp info
            for (uint8_t bit = 8; bit <= 10; ++bit)
                if (mask[bit])
                    writeDynamicMask(data, 0);
        }
        if (mask[50] && mask[51])
            writeDynamicMask(data, 0);                                      // research sites
        if (mask[52] && mask[53])
            writeDynamicMask(data, 0);                                      // research site progress
        if (mask[54] && mask[55])
            writeDynamicMask(data, 0);                                      // research
        data.flushBits();
        if (mask[0])
        {
            for (uint8_t bit = 11; bit <= 31; ++bit)
                if (mask[bit])
                    writeDynamicMask(data, 0);
        }
        if (mask[32])
        {
            for (uint8_t bit = 33; bit <= 47; ++bit)
                if (mask[bit])
                    writeDynamicMask(data, 0);
        }
        data.flushBits();

        if (mask[0])
        {
            if (mask[7])
                for (uint8_t i = 0; i < KnownTitleCount; ++i)
                    data << uint64_t(player->getKnownTitles(i));
        }
        data.flushBits();
        if (mask[32])
        {
            // the bank tab settings: the size takes three bits
            if (mask[48])
                writeDynamicMask(data, 0, 3);
            if (mask[49])
                writeDynamicMask(data, 0, 3);
        }
        data.flushBits();

        // every stat of the two blocks: the create order is the bit order
        ByteBuffer stats;
        activeStatsCreate(player, stats);
        size_t position = 0;

        if (mask[32])
        {
            if (mask[56])
                ctx.guid(player->getFarsightGuid());
            if (mask[57])
                data << WoWGuid128();
            if (mask[58])
                data << uint64_t(player->getCoinage());
            if (mask[59])
                data << uint64_t(0);
            if (mask[60])
                data << int32_t(player->getXp());
            if (mask[61])
                data << int32_t(player->getNextLevelXp());
            if (mask[62])
                data << int32_t(0);
            if (mask[63])
                activeSkillUpdate(player, data);
            if (mask[64])
                data << int32_t(player->getField<uint32_t>(PlayerField::CharacterPoints1));
            if (mask[65])
                data << int32_t(player->getField<uint32_t>(PlayerField::MaxTalentTiers));
            if (mask[66])
                data << uint32_t(player->getTrackCreature());
            for (uint16_t bit = 67; bit <= 69; ++bit)
            {
                if (mask[bit])
                    data.append(stats.contents() + position, 4);
                position += 4;
            }
        }
        else
        {
            position += 3 * 4;
        }

        if (mask[70])
        {
            for (uint16_t bit = 71; bit <= 90; ++bit)
            {
                if (mask[bit])
                    data.append(stats.contents() + position, 4);
                position += 4;
            }
            if (mask[91])
                activeBitVectorsUpdate(player, data);
            if (mask[92])
                data << int32_t(player->getModHealingDone());
            if (mask[93])
                data << player->getField<float>(PlayerField::FieldModHealingPct);
            if (mask[94])
                data << player->getField<float>(PlayerField::FieldModPeriodicHealingDonePct);
            if (mask[95])
                data << player->getField<float>(PlayerField::ModSpellPowerPct);
            if (mask[96])
                data << player->getField<float>(PlayerField::ModResiliencePct);
            if (mask[97])
                data << player->getField<float>(PlayerField::OverrideSpellPowerByApPct);
            if (mask[98])
                data << player->getField<float>(PlayerField::OverrideApBySpellPowerPct);
            if (mask[99])
                data << int32_t(player->getField<uint32_t>(PlayerField::FieldModTargetResistance));
            if (mask[100])
                data << int32_t(player->getField<uint32_t>(PlayerField::FieldModTargetPhysicalResistance));
            if (mask[101])
                data << uint32_t(0);
        }

        if (mask[102])
        {
            if (mask[103])
                data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytesRafLevel));
            if (mask[104])
                data << uint8_t(player->getEnabledActionBars());
            if (mask[105])
                data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytesMaxPvpRank));
            if (mask[106])
                data << uint8_t(0);
            if (mask[107])
                data << uint32_t(player->getField<uint32_t>(PlayerField::FieldPvpMedals));
            if (mask[108])
                data << player->getField<uint16_t>(PlayerField::FieldKillsKillsToday);
            if (mask[109])
                data << player->getField<uint16_t>(PlayerField::FieldKillsKillsYesterday);
            if (mask[110])
                data << player->getField<uint32_t>(PlayerField::FieldLifetimeHonorableKills);
            if (mask[111])
                data << int32_t(player->getWatchedFaction());
            if (mask[112])
                data << int32_t(player->getMaxLevel());
            if (mask[113])
                data << int32_t(0);
            if (mask[114])
                data << int32_t(0);
            if (mask[115])
                data << uint8_t(0);                                         // transmog cost min scaling level
            if (mask[116])
                data << int32_t(player->getField<uint32_t>(PlayerField::PetSpellPower));
            if (mask[117])
                data << player->getField<float>(PlayerField::UiHitMod);
            if (mask[118])
                data << player->getField<float>(PlayerField::UiHitSpellMod);
            if (mask[119])
                data << int32_t(player->getField<uint32_t>(PlayerField::UiHomeRealmTimeOffset));
            if (mask[120])
                data << player->getField<float>(PlayerField::ModPetHaste);
            if (mask[121])
                data << int8_t(0);
            if (mask[122])
                data << int8_t(0);
            if (mask[123])
                data << uint8_t(0);
            if (mask[124])
                data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytes2AuraVision));
            if (mask[125])
                data << uint8_t(20);
            if (mask[126])
                data << int32_t(player->getField<uint32_t>(PlayerField::OverrideSpellId));
            if (mask[127])
                data << uint16_t(player->getField<uint16_t>(PlayerField::LootSpecId));
            if (mask[128])
                data << uint32_t(player->getField<uint32_t>(PlayerField::OverrideZonePvpType));
            if (mask[129])
                data << int32_t(0);
            if (mask[130])
                data << int32_t(0);
            if (mask[131])
                data << int32_t(0);
            if (mask[132])
                data << uint8_t(player->getBankSlots());
            if (mask[133])
                data << uint8_t(0);
        }

        if (mask[134])
        {
            if (mask[135])
                data << uint8_t(0);                                         // account bank tabs
            if (mask[136])
            {
                // the research history with its own mask: nothing changed
                data.writeBits(0, 2);
                data.flushBits();
            }
            if (mask[137])
                activePerksVendorItemCreate(data);
            if (mask[138])
                activeDiscordInfoCreate(data);
            if (mask[140])
                activeLevelLinkInfoUpdate(data);
            if (mask[141])
                data << int32_t(0);
            if (mask[142])
                data << int32_t(0);
            if (mask[143])
                data << int32_t(0);
            if (mask[144])
                data << uint32_t(0);
            if (mask[145])
                data << int16_t(0);
            if (mask[146])
                activeDungeonScoreCreate(data);
            if (mask[147])
                writeMapFieldUpdate(data);                                  // trait configs
            if (mask[148])
                data << uint32_t(0);
            if (mask[149])
                data << int32_t(0);
            if (mask[150])
                data << int32_t(0);
            if (mask[151])
                data << float(0.0f);
            if (mask[152])
                data << int32_t(0);
            if (mask[153])
                data << float(0.0f);
            if (mask[154])
                data << uint64_t(0);
            if (mask[156])
                data << uint8_t(0);                                         // required mount capability flags
            if (mask[158])
                writeMapFieldUpdate(data);                                  // delve data
            if (mask[159])
                writeMapFieldUpdate(data);                                  // transmog outfits
            if (mask[161])
                activeTransmogOutfitUpdate(data);
            if (mask[162])
                activeTransmogMetadataCreate(data);
        }
        if (mask[134])
        {
            data.writeBits(0, 1);                                           // has a quest session
            data.writeBits(0, 1);                                           // has a pet stable
            data.writeBits(0, 1);                                           // has walk in data
            data.writeBits(0, 1);                                           // has challenge mode data
            data.flushBits();
        }

        if (mask[163])
        {
            for (uint8_t i = 0; i < InvSlotCount; ++i)
                if (mask[164 + i])
                    ctx.guid(activeInvSlot(player, i));
        }
        if (mask[269])
        {
            for (uint8_t i = 0; i < 2; ++i)
            {
                if (!mask[270 + i])
                    continue;

                data.writeBits(0x7, 3);
                data.flushBits();
                activeRestInfoCreate(player, i, data);
            }
        }
        if (mask[272])
        {
            for (uint8_t i = 0; i < 7; ++i)
            {
                if (mask[273 + i])
                    data << int32_t(player->getModDamageDonePositive(i));
                if (mask[280 + i])
                    data << int32_t(player->getModDamageDoneNegative(i));
                if (mask[287 + i])
                    data << float(player->getModDamageDonePct(i));
                if (mask[294 + i])
                    data << player->getField<float>(PlayerField::FieldModHealingDonePct);
            }
        }
        if (mask[301])
        {
            for (uint8_t i = 0; i < 3; ++i)
            {
                if (mask[302 + i])
                    data << player->getField<float>(PlayerField::WeaponDmgMultiplier, i);
                if (mask[305 + i])
                    data << float(1.0f);
            }
        }
        if (mask[308])
        {
            for (uint8_t i = 0; i < 12; ++i)
            {
                if (mask[309 + i])
                    data << player->getField<uint32_t>(PlayerField::FieldBuyBackPrice, i);
                if (mask[321 + i])
                    data << int64_t(player->getField<uint32_t>(PlayerField::FieldBuyBackTimestamp, i));
            }
        }
        if (mask[333])
        {
            for (uint8_t i = 0; i < 32; ++i)
                if (mask[334 + i])
                    data << int32_t(player->getCombatRating(i));
        }
        if (mask[366])
        {
            for (uint8_t i = 0; i < 4; ++i)
                if (mask[367 + i])
                    data << player->getField<uint32_t>(PlayerField::NoReagentCost, i);
        }
        if (mask[371])
        {
            for (uint8_t i = 0; i < 2; ++i)
                if (mask[372 + i])
                    data << int32_t(player->getProfessionSkillLine(i));
        }
        if (mask[374])
        {
            for (uint8_t i = 0; i < BagSlotFlagCount; ++i)
                if (mask[375 + i])
                    data << uint32_t(0);
        }
        if (mask[380])
        {
            for (uint8_t i = 0; i < ItemUpgradeWatermarkCount; ++i)
                if (mask[381 + i])
                    data << float(0.0f);
        }
        data.flushBits();
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // ItemData
    using ItemMask = ChangeMask<41>;

    constexpr uint8_t EnchantmentCount = 13;

    void itemEnchantmentCreate(Item* item, uint8_t index, ByteBuffer& data)
    {
        data << int32_t(item->getEnchantmentId(index));
        data << uint32_t(item->getEnchantmentDuration(index));
        data << int16_t(item->getEnchantmentCharges(index));
        data << uint16_t(0);                                                // inactive
    }

    // the modifier list: no entries, the size takes seven bits
    void itemModifiersCreate(ByteBuffer& data)
    {
        data.writeBits(0, 7);
        data.flushBits();
    }

    void itemModifiersUpdate(ByteBuffer& data)
    {
        data.writeBits(1, 1);
        data.writeBits(0, 7);
        data.flushBits();
    }

    // the bonus key: the item, no bonus lists, no modifications
    void itemBonusKeyCreate(Item* item, ByteBuffer& data)
    {
        data << int32_t(item->getEntry());
        data << uint32_t(0) << uint32_t(0);
    }

    void itemCreate(Item* item, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;

        ctx.guid(item->getOwnerGuid());
        ctx.guid(item->getContainerGuid());
        ctx.guid(item->getCreatorGuid());
        ctx.guid(item->getGiftCreatorGuid());
        if (ctx.owner())
        {
            data << uint32_t(item->getStackCount());
            data << uint32_t(item->getDuration());
            for (uint8_t i = 0; i < 5; ++i)
                data << int32_t(item->getSpellCharges(i));
        }
        data << uint32_t(item->getFlags());
        for (uint8_t i = 0; i < EnchantmentCount; ++i)
            itemEnchantmentCreate(item, i, data);
        if (ctx.owner())
        {
            data << uint32_t(item->getDurability());
            data << uint32_t(item->getMaxDurability());
        }
        data << uint32_t(item->getCreatePlayedTime());
        data << uint8_t(0);                                                 // context
        data << int64_t(0);                                                 // create time, no server value
        if (ctx.owner())
        {
            data << uint64_t(0);                                            // artifact xp
            data << uint8_t(0);                                             // item appearance modifier
        }
        itemModifiersCreate(data);
        data << uint32_t(0);                                                // artifact powers
        data << uint32_t(0);                                                // gems
        if (ctx.owner())
            data << uint32_t(0);                                            // zone flags
        itemBonusKeyCreate(item, data);
        if (ctx.owner())
            data << uint16_t(0);                                            // debug item level
    }

    void itemCollect(ChangedFields const& changed, ItemMask& mask)
    {
        struct Single { uint8_t bit; ItemField field; };
        static constexpr Single singles[] =
        {
            { 3, ItemField::OwnerGuid }, { 4, ItemField::ContainerGuid }, { 5, ItemField::CreatorGuid }, { 6, ItemField::GiftCreatorGuid },
            { 7, ItemField::StackCount }, { 8, ItemField::Duration }, { 9, ItemField::Flags }, { 10, ItemField::Durability },
            { 11, ItemField::MaxDurability }, { 12, ItemField::CreatePlayedTime }, { 17, ItemField::ModifierMask }
        };
        for (const Single& single : singles)
            if (changed.changed(single.field))
                mask.set(single.bit);
        for (uint8_t i = 0; i < 5; ++i)
            if (changed.changed(ItemField::SpellCharges, i))
                mask.set(22 + i);
        for (uint8_t i = 0; i < EnchantmentCount; ++i)
            if (changed.changed(ItemField::Enchantment, i))
                mask.set(28 + i);

        for (uint8_t bit = 1; bit < 21; ++bit)
            if (mask[bit])
                mask.set(0);
        for (uint8_t i = 0; i < 5; ++i)
            if (mask[22 + i])
                mask.set(21);
        for (uint8_t i = 0; i < EnchantmentCount; ++i)
            if (mask[28 + i])
                mask.set(27);
    }

    void itemFilter(ItemMask& mask, uint8_t flags)
    {
        uint32_t allowed[2] = { 0xF80A727F, 0x000001FF };
        if (flags & FlagOwner)
            allowed[0] |= 0x07F58D80;
        mask.filter(allowed);
    }

    void itemUpdate(Item* item, Context const& ctx, ItemMask const& mask)
    {
        ByteBuffer& data = ctx.data;

        mask.writeBlocks(data);
        if (mask[0])
        {
            if (mask[1])
                writeDynamicMask(data, 0);                                  // artifact powers
            if (mask[2])
                writeDynamicMask(data, 0);                                  // gems
        }
        data.flushBits();

        if (mask[0])
        {
            if (mask[3])
                ctx.guid(item->getOwnerGuid());
            if (mask[4])
                ctx.guid(item->getContainerGuid());
            if (mask[5])
                ctx.guid(item->getCreatorGuid());
            if (mask[6])
                ctx.guid(item->getGiftCreatorGuid());
            if (mask[7])
                data << uint32_t(item->getStackCount());
            if (mask[8])
                data << uint32_t(item->getDuration());
            if (mask[9])
                data << uint32_t(item->getFlags());
            if (mask[10])
                data << uint32_t(item->getDurability());
            if (mask[11])
                data << uint32_t(item->getMaxDurability());
            if (mask[12])
                data << uint32_t(item->getCreatePlayedTime());
            if (mask[13])
                data << uint8_t(0);
            if (mask[14])
                data << int64_t(0);
            if (mask[15])
                data << uint64_t(0);
            if (mask[16])
                data << uint8_t(0);
            if (mask[17])
                itemModifiersUpdate(data);
            if (mask[18])
                data << uint32_t(0);
            if (mask[19])
                itemBonusKeyCreate(item, data);
            if (mask[20])
                data << uint16_t(0);
        }
        if (mask[21])
        {
            for (uint8_t i = 0; i < 5; ++i)
                if (mask[22 + i])
                    data << int32_t(item->getSpellCharges(i));
        }
        if (mask[27])
        {
            for (uint8_t i = 0; i < EnchantmentCount; ++i)
            {
                if (!mask[28 + i])
                    continue;

                data.writeBits(0x1F, 5);
                data.flushBits();
                itemEnchantmentCreate(item, i, data);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // ContainerData
    using ContainerMask = ChangeMask<101>;

    // 12.x carries 98 slots, the server layout 36: the remaining slots are always empty
    constexpr uint8_t ContainerSlotCount = 98;
    constexpr uint8_t LayoutContainerSlotCount = 36;

    uint64_t containerSlot(Container* container, uint8_t slot)
    {
        return slot < LayoutContainerSlotCount ? container->getSlot(slot) : 0;
    }

    void containerCreate(Container* container, Context const& ctx)
    {
        for (uint8_t i = 0; i < ContainerSlotCount; ++i)
            ctx.guid(containerSlot(container, i));
        ctx.data << uint32_t(container->getSlotCount());
    }

    void containerCollect(ChangedFields const& changed, ContainerMask& mask)
    {
        if (changed.changed(ContainerField::SlotCount))
        {
            mask.set(0);
            mask.set(1);
        }
        for (uint8_t i = 0; i < LayoutContainerSlotCount; ++i)
        {
            if (changed.changed(ContainerField::ItemSlot, i))
            {
                mask.set(2);
                mask.set(3 + i);
            }
        }
    }

    void containerUpdate(Container* container, Context const& ctx, ContainerMask const& mask)
    {
        mask.writeBlocks(ctx.data);
        ctx.data.flushBits();
        if (mask[0] && mask[1])
            ctx.data << uint32_t(container->getSlotCount());
        if (mask[2])
        {
            for (uint8_t i = 0; i < ContainerSlotCount; ++i)
                if (mask[3 + i])
                    ctx.guid(containerSlot(container, i));
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // GameObjectData
    using GameObjectMask = ChangeMask<27>;

    void gameObjectCreate(GameObject* gameobject, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;

        data << int32_t(gameobject->getDisplayId());
        data << uint32_t(0);                                                // spell visual
        data << uint32_t(0);                                                // state spell visual
        data << uint32_t(0);                                                // spawn tracking state anim
        data << uint32_t(0);                                                // spawn tracking state anim kit
        data << uint32_t(0);                                                // state world effects
        data << uint32_t(0);                                                // state world effects quest objective
        ctx.guid(gameobject->getCreatedByGuid());
        data << WoWGuid128();                                               // guild
        data << uint32_t(gameobject->getClientFlags());
        data << uint32_t(0);                                                // flags b
        for (uint8_t i = 0; i < 4; ++i)
            data << float(gameobject->getParentRotation(i));
        data << int32_t(gameobject->getFactionTemplate());
        data << int8_t(gameobject->getClientState());
        data << int8_t(gameobject->getGoType());
        data << uint8_t(gameobject->getField<uint8_t>(GameObjectField::Bytes1Health));
        data << uint32_t(gameobject->getArtKit());
        data << uint32_t(0);                                                // doodad sets
        data << uint32_t(0);                                                // custom parameter
        data << int32_t(gameobject->getLevel());
        data << uint32_t(0);                                                // anim group instance
        data << uint32_t(0);                                                // ui widget item
        data << uint32_t(0);                                                // ui widget item quality
        data << uint32_t(0);                                                // ui widget item count
        data << uint32_t(0);                                                // world effects
        data.writeBits(0, 1);                                               // has assist action data
        data.flushBits();
    }

    void gameObjectCollect(ChangedFields const& changed, GameObjectMask& mask)
    {
        if (changed.changed(GameObjectField::DisplayId))
            mask.set(4);
        if (changed.changed(GameObjectField::ObjectFieldCreatedBy))
            mask.set(10);
        if (changed.changed(GameObjectField::Flags))
            mask.set(12);
        for (uint8_t i = 0; i < 4; ++i)
            if (changed.changed(GameObjectField::Rotation, i))
                mask.set(14);
        if (changed.changed(GameObjectField::FactionTemplate))
            mask.set(15);
        if (changed.changed(GameObjectField::Bytes1))
            mask.setRange(16, 4);
        if (changed.changed(GameObjectField::Level))
            mask.set(21);
        if (mask.any())
            mask.set(0);
    }

    void gameObjectUpdate(GameObject* gameobject, Context const& ctx, GameObjectMask const& mask)
    {
        ByteBuffer& data = ctx.data;

        mask.writeSmall(data);
        if (mask[0] && mask[1])
            data.writeBits(0, 32);                                          // state world effects
        data.flushBits();
        if (mask[0])
        {
            if (mask[2])
                writeDynamicMask(data, 0);                                  // doodad sets
            if (mask[3])
                writeDynamicMask(data, 0);                                  // world effects
        }
        data.flushBits();

        if (mask[0])
        {
            if (mask[4])
                data << int32_t(gameobject->getDisplayId());
            for (uint8_t bit = 5; bit <= 9; ++bit)
                if (mask[bit])
                    data << uint32_t(0);
            if (mask[10])
                ctx.guid(gameobject->getCreatedByGuid());
            if (mask[11])
                data << WoWGuid128();
            if (mask[12])
                data << uint32_t(gameobject->getClientFlags());
            if (mask[13])
                data << uint32_t(0);                                        // flags b
            if (mask[14])
                for (uint8_t i = 0; i < 4; ++i)
                    data << float(gameobject->getParentRotation(i));
            if (mask[15])
                data << int32_t(gameobject->getFactionTemplate());
            if (mask[16])
                data << int8_t(gameobject->getClientState());
            if (mask[17])
                data << int8_t(gameobject->getGoType());
            if (mask[18])
                data << uint8_t(gameobject->getField<uint8_t>(GameObjectField::Bytes1Health));
            if (mask[19])
                data << uint32_t(gameobject->getArtKit());
            if (mask[20])
                data << uint32_t(0);
            if (mask[21])
                data << int32_t(gameobject->getLevel());
            for (uint8_t bit = 22; bit <= 25; ++bit)
                if (mask[bit])
                    data << uint32_t(0);
            data.writeBits(0, 1);                                           // has assist action data
            data.flushBits();
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // DynamicObjectData
    using DynamicObjectMask = ChangeMask<7>;

    void dynamicObjectCreate(DynamicObject* dynamicObject, Context const& ctx)
    {
        ctx.guid(dynamicObject->getCasterGuid());
        ctx.data << uint8_t(dynamicObject->getDynamicType());
        ctx.data << int32_t(0) << int32_t(0);                               // spell visual: x spell visual, script visual
        ctx.data << int32_t(dynamicObject->getSpellId());
        ctx.data << float(dynamicObject->getRadius());
        ctx.data << uint32_t(dynamicObject->getCastTime());
    }

    void dynamicObjectCollect(ChangedFields const& changed, DynamicObjectMask& mask)
    {
        if (changed.changed(DynamicObjectField::CasterGuid))
            mask.set(1);
        if (changed.changed(DynamicObjectField::DynamicobjectBytes))
            mask.set(2);
        if (changed.changed(DynamicObjectField::SpellId))
            mask.set(4);
        if (changed.changed(DynamicObjectField::Radius))
            mask.set(5);
        if (changed.changed(DynamicObjectField::CastTime))
            mask.set(6);
        if (mask.any())
            mask.set(0);
    }

    void dynamicObjectUpdate(DynamicObject* dynamicObject, Context const& ctx, DynamicObjectMask const& mask)
    {
        mask.writeSmall(ctx.data);
        ctx.data.flushBits();
        if (mask[0])
        {
            if (mask[1])
                ctx.guid(dynamicObject->getCasterGuid());
            if (mask[2])
                ctx.data << uint8_t(dynamicObject->getDynamicType());
            if (mask[3])
                ctx.data << int32_t(0) << int32_t(0);
            if (mask[4])
                ctx.data << int32_t(dynamicObject->getSpellId());
            if (mask[5])
                ctx.data << float(dynamicObject->getRadius());
            if (mask[6])
                ctx.data << uint32_t(dynamicObject->getCastTime());
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // CorpseData
    using CorpseMask = ChangeMask<33>;

    constexpr uint8_t CorpseItemCount = 19;

    // the class of the owner while online, the corpse stores none
    static uint8_t corpseOwnerClass(Corpse* corpse)
    {
        const auto* owner = corpse->getWorldMapPlayer(corpse->getOwnerGuid());
        return owner != nullptr ? static_cast<uint8_t>(owner->getClass()) : 0;
    }

    // the look bytes of the corpse as customization choices
    static std::vector<CharacterCustomizations::ChrCustomizationChoice> corpseCustomizations(Corpse* corpse)
    {
        CharacterCustomizations::LegacyLook look;
        look.skin = corpse->getSkinColor();
        look.face = corpse->getFace();
        look.hairStyle = corpse->getHairStyle();
        look.hairColor = corpse->getHairColor();
        look.facialStyle = corpse->getFacialFeatures();
        return CharacterCustomizations::convertLegacyLook(corpse->getRace(), corpseOwnerClass(corpse), corpse->getGender(), look);
    }

    void corpseCreate(Corpse* corpse, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;

        data << uint32_t(corpse->getViewerDynamicFlags(ctx.target));
        ctx.guid(corpse->getOwnerGuid());
        ctx.guid(corpse->getField<uint64_t>(CorpseField::PartyGuid));
        ctx.guildGuid(corpse->getField<uint32_t>(CorpseField::Guild));
        data << uint32_t(corpse->getDisplayId());
        for (uint8_t i = 0; i < CorpseItemCount; ++i)
            data << uint32_t(corpse->getItem(i));
        data << uint8_t(corpse->getRace());
        data << uint8_t(corpse->getGender());
        const auto customizations = corpseCustomizations(corpse);
        data << uint8_t(corpseOwnerClass(corpse));
        data << uint32_t(customizations.size());
        data << uint32_t(corpse->getFlags());
        data << int32_t(0);                                                 // faction template
        data << uint32_t(0);                                                // state spell visual kit
        for (const auto& customization : customizations)
            data << customization.optionId << customization.choiceId;
    }

    void corpseCollect(ChangedFields const& changed, CorpseMask& mask)
    {
        if (changed.changed(CorpseField::DynamicFlags))
            mask.set(2);
        if (changed.changed(CorpseField::OwnerGuid))
            mask.set(3);
        if (changed.changed(CorpseField::PartyGuid))
            mask.set(4);
        if (changed.changed(CorpseField::Guild))
            mask.set(5);
        if (changed.changed(CorpseField::DisplayId))
            mask.set(6);
        if (changed.changed(CorpseField::CorpseBytes1))
            mask.setRange(7, 2);
        if (changed.changed(CorpseField::CorpseBytes1) || changed.changed(CorpseField::CorpseBytes2))
            mask.set(1);
        if (changed.changed(CorpseField::CorpseFlags))
            mask.set(10);
        for (uint8_t i = 0; i < CorpseItemCount; ++i)
            if (changed.changed(CorpseField::Item, i))
                mask.set(14 + i);

        for (uint8_t bit = 1; bit < 13; ++bit)
            if (mask[bit])
                mask.set(0);
        for (uint8_t i = 0; i < CorpseItemCount; ++i)
            if (mask[14 + i])
                mask.set(13);
    }

    void corpseUpdate(Corpse* corpse, Context const& ctx, CorpseMask const& mask)
    {
        ByteBuffer& data = ctx.data;

        const auto customizations = mask[0] && mask[1] ? corpseCustomizations(corpse) : std::vector<CharacterCustomizations::ChrCustomizationChoice>();

        mask.writeBlocks(data);
        if (mask[0] && mask[1])
            writeDynamicMask(data, static_cast<uint32_t>(customizations.size()));
        data.flushBits();
        if (mask[0])
        {
            if (mask[1])
                for (const auto& customization : customizations)
                    data << customization.optionId << customization.choiceId;
            if (mask[2])
                data << uint32_t(corpse->getViewerDynamicFlags(ctx.target));
            if (mask[3])
                ctx.guid(corpse->getOwnerGuid());
            if (mask[4])
                ctx.guid(corpse->getField<uint64_t>(CorpseField::PartyGuid));
            if (mask[5])
                ctx.guildGuid(corpse->getField<uint32_t>(CorpseField::Guild));
            if (mask[6])
                data << uint32_t(corpse->getDisplayId());
            if (mask[7])
                data << uint8_t(corpse->getRace());
            if (mask[8])
                data << uint8_t(corpse->getGender());
            if (mask[9])
                data << uint8_t(corpseOwnerClass(corpse));
            if (mask[10])
                data << uint32_t(corpse->getFlags());
            if (mask[11])
                data << int32_t(0);
            if (mask[12])
                data << uint32_t(0);
        }
        if (mask[13])
        {
            for (uint8_t i = 0; i < CorpseItemCount; ++i)
                if (mask[14 + i])
                    data << uint32_t(corpse->getItem(i));
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // AreaTriggerData: the caster, the duration, the spell and a sphere of the scale without a path
    constexpr int32_t AreaTriggerPathNone = 2;
    constexpr uint8_t AreaTriggerShapeSphere = 0;

    // a scale curve: start time offset, two points, parameter curve, override active
    void areaTriggerScaleCurveCreate(ByteBuffer& data)
    {
        data << uint32_t(0) << float(0) << float(0) << float(0) << float(0) << uint32_t(0);
        data.writeBit(false);
        data.flushBits();
    }

    // the visual anim: the animation data bit, the decay bit, anim kit, server time
    void areaTriggerVisualAnimCreate(ByteBuffer& data)
    {
        data.writeBits(0, 1);
        data.writeBit(false);
        data.flushBits();
        data << uint32_t(0) << uint32_t(0);
        data.flushBits();
    }

    // the action set period modifier: unknown value and factor
    void areaTriggerPeriodModifierCreate(ByteBuffer& data)
    {
        data << int32_t(0) << float(0.0f);
    }

    void areaTriggerCreate(Object* object, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;
        const float scale = object->getField<float>(AreaTriggerField::Scale);

        areaTriggerScaleCurveCreate(data);                                  // override scale curve
        areaTriggerScaleCurveCreate(data);                                  // extra scale curve
        areaTriggerScaleCurveCreate(data);                                  // override move curve x
        areaTriggerScaleCurveCreate(data);                                  // override move curve y
        areaTriggerScaleCurveCreate(data);                                  // override move curve z
        areaTriggerScaleCurveCreate(data);                                  // override shape curve
        ctx.guid(object->getField<uint64_t>(AreaTriggerField::CasterGuid));
        data << uint32_t(object->getField<uint32_t>(AreaTriggerField::Duration));
        data << uint32_t(0);                                                // time to target
        data << uint32_t(0);                                                // time to target scale
        data << uint32_t(0);                                                // time to target extra scale
        data << uint32_t(0);                                                // time to target position
        data << uint32_t(0);                                                // time to target shape
        data << int32_t(object->getField<uint32_t>(AreaTriggerField::SpellId));
        data << int32_t(object->getField<uint32_t>(AreaTriggerField::SpellId));   // spell for visuals
        data << int32_t(object->getField<uint32_t>(AreaTriggerField::SpellVisualId));
        data << int32_t(0);                                                 // script visual
        data << scale;                                                      // bounds radius
        data << uint32_t(0);                                                // decal properties
        data << WoWGuid128();                                               // creating effect
        data << WoWGuid128();                                               // orbit path target
        data << float(0) << float(0) << float(0);                           // roll, pitch, yaw
        data << int32_t(0);                                                 // positional sound kit
        data << uint32_t(0);                                                // movement start time
        data << uint32_t(0);                                                // creation time
        data << float(0);                                                   // z offset
        data << uint32_t(0);                                                // flags
        areaTriggerVisualAnimCreate(data);
        data << uint32_t(0);                                                // scale curve
        data << uint32_t(0);                                                // facing curve
        data << uint32_t(0);                                                // morph curve
        data << uint32_t(0);                                                // move curve
        data << float(object->GetOrientation());                            // facing
        data << AreaTriggerPathNone;
        data << AreaTriggerShapeSphere;
        areaTriggerPeriodModifierCreate(data);
        data << scale << scale;                                             // sphere: radius, radius target
        data.writeBits(0, 1);                                               // has a target roll, pitch, yaw
        data.writeBits(0, 1);                                               // has a forced position and rotation
        data.flushBits();
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    Context makeContext(Object* object, Player* target, ByteBuffer& data)
    {
        return Context{ data, target, worldConfig.realm.id, target != nullptr ? target->GetMapId() : object->GetMapId(), fieldFlags(object, target) };
    }
}

namespace ObjectUpdateMidnight
{
    uint8_t wireObjectTypeId(Object const* object, Player const* target)
    {
        switch (object->getObjectTypeId())
        {
            case TYPEID_ITEM: return WireTypeItem;
            case TYPEID_CONTAINER: return WireTypeContainer;
            case TYPEID_UNIT: return WireTypeUnit;
            case TYPEID_PLAYER: return object == target ? WireTypeActivePlayer : WireTypePlayer;
            case TYPEID_GAMEOBJECT: return WireTypeGameObject;
            case TYPEID_DYNAMICOBJECT: return WireTypeDynamicObject;
            case TYPEID_CORPSE: return WireTypeCorpse;
            case TYPEID_AREATRIGGER: return WireTypeAreaTrigger;
            default: return WireTypeObject;
        }
    }

    // 12.x: every object is a list of entity fragments. The update field structures are the contents of the
    // indirect fragment 3, the object types are tags (200 and up); the list is sorted and closed by 255.
    namespace EntityFragment
    {
        constexpr uint8_t Object = 3;
        constexpr uint8_t Vendor = 18;
        constexpr uint8_t TagItem = 200;
        constexpr uint8_t TagContainer = 201;
        constexpr uint8_t TagUnit = 204;
        constexpr uint8_t TagPlayer = 205;
        constexpr uint8_t TagGameObject = 206;
        constexpr uint8_t TagDynamicObject = 207;
        constexpr uint8_t TagCorpse = 208;
        constexpr uint8_t TagAreaTrigger = 209;
        constexpr uint8_t End = 255;

        // the contents mask of a value update: bit 0 announces the indirect object fragment, bit 1 its changes
        constexpr uint8_t ObjectPresent = 0x01;
        constexpr uint8_t ObjectChanged = 0x02;
    }

    // the vendor fragment of a creature: its vendor types and the petition flag, 0 without
    static int32_t vendorFragmentFlags(Object* object)
    {
        if (object->getObjectTypeId() != TYPEID_UNIT)
            return 0;

        const uint64_t npcFlags = static_cast<Unit*>(object)->getNpcFlags();
        int32_t flags = static_cast<int32_t>((npcFlags & 0xF80) >> 7);          // generic, ammo, food, poison, reagent
        if (npcFlags & UNIT_NPC_FLAG_CHARTERGIVER)
            flags |= 0x20;
        return flags;
    }

    static void writeEntityFragments(Object* object, ByteBuffer& data)
    {
        data << uint8_t(EntityFragment::Object);
        switch (object->getObjectTypeId())
        {
            case TYPEID_ITEM:
                data << uint8_t(EntityFragment::TagItem);
                break;
            case TYPEID_CONTAINER:
                data << uint8_t(EntityFragment::TagItem);
                data << uint8_t(EntityFragment::TagContainer);
                break;
            case TYPEID_UNIT:
                if (vendorFragmentFlags(object) != 0)
                    data << uint8_t(EntityFragment::Vendor);
                data << uint8_t(EntityFragment::TagUnit);
                break;
            case TYPEID_PLAYER:
                data << uint8_t(EntityFragment::TagUnit);
                data << uint8_t(EntityFragment::TagPlayer);
                break;
            case TYPEID_GAMEOBJECT:
                data << uint8_t(EntityFragment::TagGameObject);
                break;
            case TYPEID_DYNAMICOBJECT:
                data << uint8_t(EntityFragment::TagDynamicObject);
                break;
            case TYPEID_CORPSE:
                data << uint8_t(EntityFragment::TagCorpse);
                break;
            case TYPEID_AREATRIGGER:
                data << uint8_t(EntityFragment::TagAreaTrigger);
                break;
            default:
                break;
        }
        data << uint8_t(EntityFragment::End);
    }

    void writeCreateValues(Object* object, Player* target, ByteBuffer& data)
    {
        const Context ctx = makeContext(object, target, data);

        const size_t sizePosition = data.wpos();
        data << uint32_t(0);
        data << uint8_t(ctx.flags);
        writeEntityFragments(object, data);
        data << uint8_t(1);                                 // the indirect object fragment is active

        objectCreate(object, ctx);

        switch (object->getObjectTypeId())
        {
            case TYPEID_ITEM:
                itemCreate(static_cast<Item*>(object), ctx);
                break;
            case TYPEID_CONTAINER:
                itemCreate(static_cast<Item*>(object), ctx);
                containerCreate(static_cast<Container*>(object), ctx);
                break;
            case TYPEID_UNIT:
                unitCreate(static_cast<Unit*>(object), ctx);
                break;
            case TYPEID_PLAYER:
                unitCreate(static_cast<Unit*>(object), ctx);
                playerCreate(static_cast<Player*>(object), ctx);
                if (object == target)
                    activePlayerCreate(static_cast<Player*>(object), ctx);
                break;
            case TYPEID_GAMEOBJECT:
                gameObjectCreate(static_cast<GameObject*>(object), ctx);
                break;
            case TYPEID_DYNAMICOBJECT:
                dynamicObjectCreate(static_cast<DynamicObject*>(object), ctx);
                break;
            case TYPEID_CORPSE:
                corpseCreate(static_cast<Corpse*>(object), ctx);
                break;
            case TYPEID_AREATRIGGER:
                areaTriggerCreate(object, ctx);
                break;
            default:
                break;
        }

        // the vendor fragment follows the object fragment
        if (const int32_t vendorFlags = vendorFragmentFlags(object); vendorFlags != 0)
            data << int32_t(vendorFlags);

        data.put<uint32_t>(sizePosition, static_cast<uint32_t>(data.wpos() - sizePosition - sizeof(uint32_t)));
    }

    void writeUpdateValues(Object* object, Player* target, UpdateMask const& mask, ByteBuffer& data)
    {
        const Context ctx = makeContext(object, target, data);
        const ChangedFields changed(&mask);

        const size_t sizePosition = data.wpos();
        data << uint32_t(0);
        data << uint8_t(ctx.owner() ? 1 : 0);
        data << uint8_t(0);                                 // the fragment list is unchanged
        data << uint8_t(EntityFragment::ObjectPresent | EntityFragment::ObjectChanged);
        const size_t typeMaskPosition = data.wpos();
        data << uint32_t(0);

        uint32_t typeMask = 0;

        ObjectMask objectMask;
        objectCollect(changed, objectMask, object);
        if (objectMask.any())
        {
            typeMask |= 1u << WireTypeObject;
            objectUpdate(object, ctx, objectMask);
        }

        switch (object->getObjectTypeId())
        {
            case TYPEID_ITEM:
            case TYPEID_CONTAINER:
            {
                ItemMask itemMask;
                itemCollect(changed, itemMask);
                itemFilter(itemMask, ctx.flags);
                if (itemMask.any())
                {
                    typeMask |= 1u << WireTypeItem;
                    itemUpdate(static_cast<Item*>(object), ctx, itemMask);
                }

                if (object->getObjectTypeId() == TYPEID_CONTAINER)
                {
                    ContainerMask containerMask;
                    containerCollect(changed, containerMask);
                    if (containerMask.any())
                    {
                        typeMask |= 1u << WireTypeContainer;
                        containerUpdate(static_cast<Container*>(object), ctx, containerMask);
                    }
                }
            } break;
            case TYPEID_UNIT:
            case TYPEID_PLAYER:
            {
                Unit* unit = static_cast<Unit*>(object);

                UnitMask unitMask;
                unitCollect(changed, unitMask);
                unitFilter(unitMask, ctx.flags);
                if (unitMask.any())
                {
                    typeMask |= 1u << WireTypeUnit;
                    unitUpdate(unit, ctx, unitMask);
                }

                if (object->getObjectTypeId() == TYPEID_PLAYER)
                {
                    Player* player = static_cast<Player*>(object);

                    PlayerMask playerMask;
                    playerCollect(changed, playerMask);
                    playerFilter(playerMask, ctx.flags);
                    if (playerMask.any())
                    {
                        typeMask |= 1u << WireTypePlayer;
                        playerUpdate(player, ctx, playerMask);
                    }

                    if (object == target)
                    {
                        ActivePlayerMask activeMask;
                        activePlayerCollect(changed, activeMask);
                        if (activeMask.any())
                        {
                            typeMask |= 1u << WireTypeActivePlayer;
                            activePlayerUpdate(player, ctx, activeMask);
                        }
                    }
                }
            } break;
            case TYPEID_GAMEOBJECT:
            {
                GameObjectMask gameObjectMask;
                gameObjectCollect(changed, gameObjectMask);
                if (gameObjectMask.any())
                {
                    typeMask |= 1u << WireTypeGameObject;
                    gameObjectUpdate(static_cast<GameObject*>(object), ctx, gameObjectMask);
                }
            } break;
            case TYPEID_DYNAMICOBJECT:
            {
                DynamicObjectMask dynamicObjectMask;
                dynamicObjectCollect(changed, dynamicObjectMask);
                if (dynamicObjectMask.any())
                {
                    typeMask |= 1u << WireTypeDynamicObject;
                    dynamicObjectUpdate(static_cast<DynamicObject*>(object), ctx, dynamicObjectMask);
                }
            } break;
            case TYPEID_CORPSE:
            {
                CorpseMask corpseMask;
                corpseCollect(changed, corpseMask);
                if (corpseMask.any())
                {
                    typeMask |= 1u << WireTypeCorpse;
                    corpseUpdate(static_cast<Corpse*>(object), ctx, corpseMask);
                }
            } break;
            default:
                break;
        }

        data.put<uint32_t>(typeMaskPosition, typeMask);
        data.put<uint32_t>(sizePosition, static_cast<uint32_t>(data.wpos() - sizePosition - sizeof(uint32_t)));
    }
}
