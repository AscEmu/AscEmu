/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ObjectUpdateShadowlands.hpp"

#if VERSION_STRING == Shadowlands

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
#include "Server/WorldConfig.h"
#include "Storage/MySQLStructures.h"
#include "Version/ObjectLayout.hpp"
#include "WoWGuid.hpp"

#include <algorithm>
#include <array>
#include <cstring>

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
    // object types of 9.x
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
        WireTypeConversation = 13
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

        // the large header of more than 32 blocks: the first mask word as value, the second as bits
        void writeLarge(ByteBuffer& data) const
        {
            data << uint32_t(blocksMask(0));
            data.writeBits(blocksMask(1), static_cast<uint32_t>(BlockCount - 32));
            for (size_t i = 0; i < BlockCount; ++i)
                if (m_blocks[i] != 0)
                    data.writeBits(m_blocks[i], 32);
        }

    private:
        std::array<uint32_t, BlockCount> m_blocks{};
    };

    // the size of a dynamic field with every element changed
    void writeDynamicMask(ByteBuffer& data, uint32_t size)
    {
        data.writeBits(size, 32);
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
    // values that depend on the receiver
    uint32_t viewerDynamicFlags(Object* object, Player* target)
    {
        if (Creature* creature = object->ToCreature())
        {
            uint32_t dynamicFlags = creature->getDynamicFlags() & ~(U_DYN_FLAG_TAGGED_BY_OTHER | U_DYN_FLAG_TAPPED_BY_PLAYER);
            if (target != nullptr && creature->getTaggerGuid())
            {
                dynamicFlags |= U_DYN_FLAG_TAGGED_BY_OTHER;
                if (creature->isTaggedByPlayerOrItsGroup(target))
                    dynamicFlags |= U_DYN_FLAG_TAPPED_BY_PLAYER;
            }
            return dynamicFlags;
        }

        if (Unit* unit = object->ToUnit())
            return unit->getDynamicFlags();

        if (GameObject* gameobject = static_cast<GameObject*>(object); object->isGameObject())
        {
            // flags in the low half, the path progress of a transport in the high half
            uint32_t dynamicFlags = gameobject->getDynamicFlags() & ~(GO_DYN_FLAG_INTERACTABLE | GO_DYN_FLAG_SPARKLE);
            const bool isMovingTransport = gameobject->getGoType() == GAMEOBJECT_TYPE_TRANSPORT || gameobject->getGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT;
            const uint16_t pathProgress = isMovingTransport ? static_cast<uint16_t>(gameobject->getDynamicPathProgress()) : 0;

            if (target != nullptr)
            {
                const auto gobProperties = gameobject->GetGameObjectProperties();
                bool activeObject = false;

                for (const auto& questPair : gobProperties->goMap)
                {
                    if (auto* const questLog = target->getQuestLogByQuestId(questPair.first->id))
                    {
                        const auto quest = questLog->getQuestProperties();
                        if (quest->count_required_mob == 0)
                            continue;

                        for (uint8_t i = 0; i < 4; ++i)
                        {
                            if (quest->required_mob_or_go[i] == static_cast<int32_t>(gameobject->getEntry()) && questLog->getMobCountByIndex(i) < quest->required_mob_or_go_count[i])
                            {
                                activeObject = true;
                                break;
                            }
                        }
                    }

                    if (activeObject)
                        break;
                }

                if (!activeObject)
                {
                    for (const auto& questItemData : gobProperties->itemMap)
                    {
                        for (const auto& itemPair : questItemData.second)
                        {
                            if (target->getQuestLogByQuestId(questItemData.first->id) == nullptr)
                                continue;

                            if (target->getItemInterface()->GetItemCount(itemPair.first) < itemPair.second)
                            {
                                activeObject = true;
                                break;
                            }
                        }

                        if (activeObject)
                            break;
                    }
                }

                if (activeObject)
                    dynamicFlags |= GO_DYN_FLAG_INTERACTABLE | GO_DYN_FLAG_SPARKLE;

                if (!(dynamicFlags & GO_DYN_FLAG_INTERACTABLE) && gameobject->isQuestGiver())
                {
                    auto* const objectQuestGiver = dynamic_cast<GameObject_QuestGiver*>(gameobject);
                    if (objectQuestGiver != nullptr && objectQuestGiver->HasQuests())
                    {
                        for (const auto& questRelation : objectQuestGiver->getQuestList())
                        {
                            if (questRelation == nullptr || questRelation->qst == nullptr)
                                continue;

                            const auto questProperties = questRelation->qst;
                            if ((questRelation->type & QUESTGIVER_QUEST_START && !target->hasQuestInQuestLog(questProperties->id)
                                && sQuestMgr.CalcQuestStatus(gameobject, target, questRelation.get()) >= QuestStatus::AvailableChat) ||
                                (questRelation->type & QUESTGIVER_QUEST_END && target->hasQuestInQuestLog(questProperties->id)))
                            {
                                dynamicFlags |= GO_DYN_FLAG_INTERACTABLE;
                                break;
                            }
                        }
                    }
                }
            }

            return (dynamicFlags & 0xFFFF) | (uint32_t(pathProgress) << 16);
        }

        if (object->isCorpse())
        {
            auto* const corpse = static_cast<Corpse*>(object);
            uint32_t dynamicFlags = corpse->getDynamicFlags() & ~(U_DYN_FLAG_LOOTABLE | U_DYN_FLAG_TAPPED_BY_PLAYER);
            if (!corpse->loot.isLooted())
                dynamicFlags |= U_DYN_FLAG_LOOTABLE | U_DYN_FLAG_TAPPED_BY_PLAYER;
            return dynamicFlags;
        }

        return object->getField<uint32_t>(ObjectField::DynamicField);
    }

    uint32_t viewerDisplayId(Unit* unit, Player* target)
    {
        if (Creature* creature = unit->ToCreature(); creature != nullptr && target != nullptr && target->isGMFlagSet())
        {
            if (creature->GetCreatureProperties()->isTriggerNpc)
                return creature->GetCreatureProperties()->getVisibleModelForTriggerNpc();
        }

        return unit->getDisplayId();
    }

    uint32_t viewerUnitFlags(Unit* unit, Player* target)
    {
        uint32_t unitFlags = unit->getUnitFlags();
        if (unit->isCreature() && target != nullptr && target->isGMFlagSet())
            unitFlags &= ~UNIT_FLAG_NOT_SELECTABLE;
        return unitFlags;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // ObjectData: entry, dynamic flags, scale
    using ObjectMask = ChangeMask<4>;

    void objectCreate(Object* object, Context const& ctx)
    {
        ctx.data << int32_t(object->getEntry());
        ctx.data << uint32_t(viewerDynamicFlags(object, ctx.target));
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
                ctx.data << uint32_t(viewerDynamicFlags(object, ctx.target));
            if (mask[3])
                ctx.data << float(object->getScale());
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // UnitData
    using UnitMask = ChangeMask<194>;

    // 9.x carries seven power slots, the server layout six: the seventh is always empty
    constexpr uint8_t PowerCount = 7;
    constexpr uint8_t LayoutPowerCount = 6;
    constexpr UnitField PowerFields[LayoutPowerCount] = { UnitField::Power1, UnitField::Power2, UnitField::Power3, UnitField::Power4, UnitField::Power5, UnitField::Power6 };
    constexpr UnitField MaxPowerFields[LayoutPowerCount] = { UnitField::MaxPower1, UnitField::MaxPower2, UnitField::MaxPower3, UnitField::MaxPower4, UnitField::MaxPower5, UnitField::MaxPower6 };

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

    // the visible item of a unit: entry, appearance modifier and visual from the slot display and its info
    void unitVisibleItem(Unit* unit, uint8_t slot, int32_t& itemId, uint16_t& appearanceModId, uint16_t& itemVisual)
    {
        itemId = static_cast<int32_t>(unit->getField<uint32_t>(UnitField::VirtualItemSlotDisplay, slot * 2));
        const uint32_t info = unit->getField<uint32_t>(UnitField::VirtualItemSlotDisplay, slot * 2 + 1);
        appearanceModId = static_cast<uint16_t>(info & 0xFFFF);
        itemVisual = static_cast<uint16_t>(info >> 16);
    }

    // the visible item structure: item, secondary modified appearance, appearance modifier, visual
    void unitVisibleItemCreate(Unit* unit, uint8_t slot, ByteBuffer& data)
    {
        int32_t itemId;
        uint16_t appearanceModId;
        uint16_t itemVisual;
        unitVisibleItem(unit, slot, itemId, appearanceModId, itemVisual);
        data << itemId;
        data << int32_t(0);                                                 // secondary item modified appearance
        data << appearanceModId << itemVisual;
    }

    // the channel: spell, spell visual (x spell visual, script visual)
    void unitChannelCreate(Unit* unit, ByteBuffer& data)
    {
        data << int32_t(unit->getChannelSpellId());
        data << int32_t(0) << int32_t(0);
    }

    // family and type of a creature from its properties, none for players
    void unitCreatureInfo(Unit* unit, int32_t& family, int32_t& type)
    {
        family = 0;
        type = 0;
        if (Creature* creature = unit->ToCreature())
        {
            family = int32_t(creature->GetCreatureProperties()->Family);
            type = int32_t(creature->GetCreatureProperties()->Type);
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
        int32_t creatureType;
        unitCreatureInfo(unit, creatureFamily, creatureType);

        data << int32_t(viewerDisplayId(unit, ctx.target));
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
        unitChannelCreate(unit, data);
        data << uint32_t(0);                                                // summoned by home realm
        data << uint8_t(unit->getRace());
        data << uint8_t(unit->getClass());
        data << uint8_t(unit->getClass());                                  // player class
        data << uint8_t(unit->getGender());
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
        data << int32_t(unit->getLevel());
        data << int32_t(unit->getField<uint32_t>(UnitField::EffectiveLevel));
        data << int32_t(0);                                                 // content tuning
        data << int32_t(0);                                                 // scaling level min
        data << int32_t(0);                                                 // scaling level max
        data << int32_t(0);                                                 // scaling level delta
        data << int32_t(0);                                                 // scaling faction group
        data << int32_t(0);                                                 // scaling health item level curve
        data << int32_t(0);                                                 // scaling damage item level curve
        data << int32_t(unit->getFactionTemplate());

        for (uint8_t i = 0; i < 3; ++i)
            unitVisibleItemCreate(unit, i, data);

        data << uint32_t(viewerUnitFlags(unit, ctx.target));
        data << uint32_t(unit->getUnitFlags2());
        data << uint32_t(0);                                                // flags 3
        data << uint32_t(unit->getAuraState());
        data << uint32_t(unit->getBaseAttackTime(0));
        data << uint32_t(unit->getBaseAttackTime(1));
        if (ctx.owner())
            data << uint32_t(2000);                                         // ranged attack round base time
        data << float(unit->getBoundingRadius());
        data << float(unit->getCombatReach());
        data << float(1.0f);                                                // display scale
        data << creatureFamily;
        data << creatureType;
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
            for (uint8_t i = 0; i < 4; ++i)
            {
                data << int32_t(unit->getStat(i));
                data << int32_t(unit->getPosStat(i));
                data << int32_t(unit->getNegStat(i));
            }
        }

        if (ctx.ownerOrEmpath())
        {
            for (uint8_t i = 0; i < 7; ++i)
                data << int32_t(unit->getResistance(i));
        }

        if (ctx.owner())
        {
            // bonus resistance and mana cost modifier per school, the power cost multiplier is one value in 9.x
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
            data << int32_t(unit->getRangedAttackPower());
            data << int32_t(unit->getField<uint32_t>(UnitField::RangedAttackPowerModsPos));
            data << int32_t(unit->getField<uint32_t>(UnitField::RangedAttackPowerModsNeg));
            data << float(unit->getRangedAttackPowerMultiplier());
            data << int32_t(0);                                             // main hand weapon attack power
            data << int32_t(0);                                             // off hand weapon attack power
            data << int32_t(0);                                             // ranged weapon attack power
            data << int32_t(0);                                             // set attack speed aura
            data << float(0.0f);                                            // lifesteal
            data << float(unit->getMinRangedDamage());
            data << float(unit->getMaxRangedDamage());
            data << float(0.0f);                                            // mana cost multiplier, no server value
            data << unit->getField<float>(UnitField::MaxHealthModifier);
        }

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
        data << int32_t(0);                                                 // taxi nodes
        ctx.guildGuid(unitGuildId(unit));
        data << uint32_t(0);                                                // passive spells
        data << uint32_t(0);                                                // world effects
        data << uint32_t(channelObject != 0 ? 1 : 0);                       // channel objects
        data << uint32_t(0);                                                // silenced school mask
        data << WoWGuid128();                                               // nameplate attach to
        if (channelObject != 0)
            ctx.guid(channelObject);
    }

    // bits of the fields that changed in the server layout
    void unitCollect(ChangedFields const& changed, UnitMask& mask)
    {
        struct Single { uint8_t bit; UnitField field; };
        static constexpr Single singles[] =
        {
            { 5, UnitField::DisplayId }, { 11, UnitField::CharmGuid }, { 12, UnitField::SummonGuid }, { 13, UnitField::CritterGuid },
            { 14, UnitField::CharmedByGuid }, { 15, UnitField::SummonedByGuid }, { 16, UnitField::CreatedByGuid }, { 17, UnitField::DemonCreatorGuid },
            { 19, UnitField::TargetGuid }, { 20, UnitField::BattlePetCompanionGuid }, { 22, UnitField::ChannelSpell }, { 23, UnitField::SummonedByHomeRealm },
            { 28, UnitField::DisplayPower }, { 29, UnitField::OverrideDisplayPowerId }, { 30, UnitField::Health }, { 31, UnitField::MaxHealth },
            { 33, UnitField::Level }, { 34, UnitField::EffectiveLevel }, { 42, UnitField::FactionTemplate }, { 43, UnitField::UnitFlags },
            { 44, UnitField::UnitFlags2 }, { 46, UnitField::AuraState }, { 48, UnitField::BoundingRadius }, { 49, UnitField::CombatReach },
            { 53, UnitField::NativeDisplayId }, { 55, UnitField::MountDisplayId }, { 57, UnitField::MinimumDamage }, { 58, UnitField::MaximumDamage },
            { 59, UnitField::MinimumOffhandDamage }, { 60, UnitField::MaximumOffhandDamage }, { 66, UnitField::PetNumber }, { 67, UnitField::PetNameTimestamp },
            { 68, UnitField::PetExperience }, { 69, UnitField::PetNextLevelExperience }, { 70, UnitField::ModCastSpeed }, { 72, UnitField::ModCastHaste },
            { 73, UnitField::ModHaste }, { 74, UnitField::ModRangedHaste }, { 75, UnitField::ModHasteRegen }, { 77, UnitField::CreatedBySpellId },
            { 78, UnitField::NpcEmoteState }, { 79, UnitField::BaseMana }, { 80, UnitField::BaseHealth }, { 85, UnitField::AttackPower },
            { 86, UnitField::AttackPowerModPos }, { 87, UnitField::AttackPowerModNeg }, { 88, UnitField::AttackPowerMultiplier }, { 89, UnitField::RangedAttackPower },
            { 90, UnitField::RangedAttackPowerModsPos }, { 91, UnitField::RangedAttackPowerModsNeg }, { 92, UnitField::RangedAttackPowerMultiplier },
            { 99, UnitField::MinimumRangedDamage }, { 100, UnitField::MaximumRangedDamage }, { 102, UnitField::MaxHealthModifier }, { 103, UnitField::HoverHeight },
            { 105, UnitField::MinItemLevel }, { 106, UnitField::MaxItemLevel }, { 108, UnitField::WildBattlePetLevel }, { 110, UnitField::BattlePetCompanionNameTimestamp },
            { 111, UnitField::InteractSpellId }
        };

        for (const Single& single : singles)
            if (changed.changed(single.field))
                mask.set(single.bit);

        if (changed.changed(UnitField::ChannelObjectGuid))
            mask.set(4);
        if (changed.changed(UnitField::FieldBytes0))
            mask.setRange(24, 4);
        if (changed.changed(UnitField::FieldBytes1))
        {
            // stand state, pet talent points, vis flags close the second block, the anim tier opens the third
            mask.setRange(61, 3);
            mask.set(65);
        }
        if (changed.changed(UnitField::FieldBytes2))
            mask.setRange(81, 4);
        if (changed.changed(PlayerField::GuildId))
            mask.set(117);
        if (changed.changed(UnitField::NpcFlags))
            mask.setRange(121, 2);

        for (uint8_t i = 0; i < LayoutPowerCount; ++i)
        {
            if (changed.changed(PowerFields[i]))
                mask.set(124 + i);
            if (changed.changed(MaxPowerFields[i]))
                mask.set(131 + i);
            if (changed.changed(UnitField::PowerRegenFlatModifier, i))
                mask.set(138 + i);
            if (changed.changed(UnitField::PowerRegenInterruptedFlatModifier, i))
                mask.set(145 + i);
        }

        for (uint8_t i = 0; i < 3; ++i)
            if (changed.changed(UnitField::VirtualItemSlotDisplay, i * 2) || changed.changed(UnitField::VirtualItemSlotDisplay, i * 2 + 1))
                mask.set(153 + i);

        for (uint8_t i = 0; i < 2; ++i)
            if (changed.changed(UnitField::BaseAttackTime, i))
                mask.set(157 + i);

        for (uint8_t i = 0; i < 4; ++i)
        {
            if (changed.changed(UnitField::Stat, i))
                mask.set(160 + i);
            if (changed.changed(UnitField::PositiveStat, i))
                mask.set(164 + i);
            if (changed.changed(UnitField::NegativeStat, i))
                mask.set(168 + i);
        }

        for (uint8_t i = 0; i < 7; ++i)
        {
            if (changed.changed(UnitField::Resistance, i))
                mask.set(173 + i);
            if (changed.changed(UnitField::ResistanceBuffModPositive, i) || changed.changed(UnitField::ResistanceBuffModNegative, i))
                mask.set(180 + i);
            if (changed.changed(UnitField::PowerCostModifier, i))
                mask.set(187 + i);
            if (changed.changed(UnitField::PowerCostMultiplier, i))
                mask.set(101);
        }

        // the group bits
        static constexpr uint8_t groups[] = { 0, 32, 64, 96 };
        for (const uint8_t group : groups)
            for (uint8_t bit = group + 1; bit < group + 32; ++bit)
                if (mask[bit])
                    mask.set(group);
        for (uint8_t i = 0; i < 2; ++i)
            if (mask[121 + i])
                mask.set(120);
        for (uint8_t i = 0; i < 28; ++i)
            if (mask[124 + i])
                mask.set(123);
        for (uint8_t i = 0; i < 3; ++i)
            if (mask[153 + i])
                mask.set(152);
        for (uint8_t i = 0; i < 2; ++i)
            if (mask[157 + i])
                mask.set(156);
        for (uint8_t i = 0; i < 12; ++i)
            if (mask[160 + i])
                mask.set(159);
        for (uint8_t i = 0; i < 21; ++i)
            if (mask[173 + i])
                mask.set(172);
    }

    // the fields a receiver may see
    void unitFilter(UnitMask& mask, uint8_t flags)
    {
        uint32_t allowed[7] = { 0xFFFFDFFF, 0xE1FF7FFF, 0x001EFFFF, 0xFFFFFF81, 0x7F0003FF, 0x00000000, 0x00000000 };
        if (flags & FlagOwner)
        {
            const uint32_t owner[7] = { 0x00002000, 0x1E008000, 0xFFE10000, 0x0800007E, 0x80FFFC00, 0xFFFFFFFF, 0x00000003 };
            for (size_t i = 0; i < 7; ++i)
                allowed[i] |= owner[i];
        }
        if (flags & FlagUnitAll)
        {
            allowed[3] |= 0x08000000;
            allowed[4] |= 0x00FFFC00;
        }
        if (flags & FlagEmpath)
        {
            allowed[1] |= 0x1E000000;
            allowed[5] |= 0x000FF000;
        }
        mask.filter(allowed);
    }

    void unitUpdate(Unit* unit, Context const& ctx, UnitMask const& mask)
    {
        ByteBuffer& data = ctx.data;
        const uint64_t channelObject = unit->getChannelObjectGuid();
        int32_t creatureFamily;
        int32_t creatureType;
        unitCreatureInfo(unit, creatureFamily, creatureType);

        mask.writeBlocks(data);
        if (mask[0] && mask[1])
            data.writeBits(0, 32);                                          // state world effects
        data.flushBits();

        if (mask[0])
        {
            // the dynamic fields: passive spells, world effects, channel objects
            if (mask[2])
                writeDynamicMask(data, 0);
            if (mask[3])
                writeDynamicMask(data, 0);
            if (mask[4])
                writeDynamicMask(data, channelObject != 0 ? 1 : 0);
        }
        data.flushBits();

        if (mask[0])
        {
            if (mask[4] && channelObject != 0)
                ctx.guid(channelObject);
            if (mask[5])
                data << int32_t(viewerDisplayId(unit, ctx.target));
            for (uint8_t bit = 6; bit <= 9; ++bit)
                if (mask[bit])
                    data << uint32_t(0);
            if (mask[10])
                data << int32_t(0);
            if (mask[11])
                ctx.guid(unit->getCharmGuid());
            if (mask[12])
                ctx.guid(unit->getSummonGuid());
            if (mask[13])
                ctx.guid(unit->getCritterGuid());
            if (mask[14])
                ctx.guid(unit->getCharmedByGuid());
            if (mask[15])
                ctx.guid(unit->getSummonedByGuid());
            if (mask[16])
                ctx.guid(unit->getCreatedByGuid());
            if (mask[17])
                ctx.guid(unit->getField<uint64_t>(UnitField::DemonCreatorGuid));
            if (mask[18])
                data << WoWGuid128();
            if (mask[19])
                ctx.guid(unit->getTargetGuid());
            if (mask[20])
                data << WoWGuid128();
            if (mask[21])
                data << uint64_t(0);
            if (mask[22])
                unitChannelCreate(unit, data);
            if (mask[23])
                data << uint32_t(0);
            if (mask[24])
                data << uint8_t(unit->getRace());
            if (mask[25])
                data << uint8_t(unit->getClass());
            if (mask[26])
                data << uint8_t(unit->getClass());
            if (mask[27])
                data << uint8_t(unit->getGender());
            if (mask[28])
                data << uint8_t(unit->getField<uint8_t>(UnitField::FieldBytes0PowerType));
            if (mask[29])
                data << uint32_t(unit->getField<uint32_t>(UnitField::OverrideDisplayPowerId));
            if (mask[30])
                data << int64_t(unit->getHealth());
            if (mask[31])
                data << int64_t(unit->getMaxHealth());
        }

        if (mask[32])
        {
            if (mask[33])
                data << int32_t(unit->getLevel());
            if (mask[34])
                data << int32_t(unit->getField<uint32_t>(UnitField::EffectiveLevel));
            for (uint8_t bit = 35; bit <= 41; ++bit)
                if (mask[bit])
                    data << int32_t(0);
            if (mask[42])
                data << int32_t(unit->getFactionTemplate());
            if (mask[43])
                data << uint32_t(viewerUnitFlags(unit, ctx.target));
            if (mask[44])
                data << uint32_t(unit->getUnitFlags2());
            if (mask[45])
                data << uint32_t(0);
            if (mask[46])
                data << uint32_t(unit->getAuraState());
            if (mask[47])
                data << uint32_t(2000);
            if (mask[48])
                data << float(unit->getBoundingRadius());
            if (mask[49])
                data << float(unit->getCombatReach());
            if (mask[50])
                data << float(1.0f);
            if (mask[51])
                data << creatureFamily;
            if (mask[52])
                data << creatureType;
            if (mask[53])
                data << int32_t(unit->getNativeDisplayId());
            if (mask[54])
                data << float(1.0f);
            if (mask[55])
                data << int32_t(unit->getMountDisplayId());
            if (mask[56])
                data << int32_t(0);
            if (mask[57])
                data << float(unit->getMinDamage());
            if (mask[58])
                data << float(unit->getMaxDamage());
            if (mask[59])
                data << float(unit->getMinOffhandDamage());
            if (mask[60])
                data << float(unit->getMaxOffhandDamage());
            if (mask[61])
                data << uint8_t(unit->getStandState());
            if (mask[62])
                data << uint8_t(unit->getPetTalentPoints());
            if (mask[63])
                data << uint8_t(unit->getStandStateFlags());
        }

        if (mask[64])
        {
            if (mask[65])
                data << uint8_t(unit->getAnimationFlags());
            if (mask[66])
                data << uint32_t(unit->getPetNumber());
            if (mask[67])
                data << uint32_t(unit->getField<uint32_t>(UnitField::PetNameTimestamp));
            if (mask[68])
                data << uint32_t(unit->getField<uint32_t>(UnitField::PetExperience));
            if (mask[69])
                data << uint32_t(unit->getField<uint32_t>(UnitField::PetNextLevelExperience));
            if (mask[70])
                data << float(unit->getModCastSpeed());
            if (mask[71])
                data << float(0.0f);
            if (mask[72])
                data << unit->getField<float>(UnitField::ModCastHaste);
            if (mask[73])
                data << unit->getField<float>(UnitField::ModHaste);
            if (mask[74])
                data << unit->getField<float>(UnitField::ModRangedHaste);
            if (mask[75])
                data << unit->getField<float>(UnitField::ModHasteRegen);
            if (mask[76])
                data << float(1.0f);
            if (mask[77])
                data << int32_t(unit->getCreatedBySpellId());
            if (mask[78])
                data << int32_t(unit->getEmoteState());
            if (mask[79])
                data << int32_t(unit->getBaseMana());
            if (mask[80])
                data << int32_t(unit->getBaseHealth());
            if (mask[81])
                data << uint8_t(unit->getSheathType());
            if (mask[82])
                data << uint8_t(unit->getPvpFlags());
            if (mask[83])
                data << uint8_t(unit->getPetFlags());
            if (mask[84])
                data << uint8_t(unit->getShapeShiftForm());
            if (mask[85])
                data << int32_t(unit->getAttackPower());
            if (mask[86])
                data << int32_t(unit->getField<uint32_t>(UnitField::AttackPowerModPos));
            if (mask[87])
                data << int32_t(unit->getField<uint32_t>(UnitField::AttackPowerModNeg));
            if (mask[88])
                data << float(unit->getAttackPowerMultiplier());
            if (mask[89])
                data << int32_t(unit->getRangedAttackPower());
            if (mask[90])
                data << int32_t(unit->getField<uint32_t>(UnitField::RangedAttackPowerModsPos));
            if (mask[91])
                data << int32_t(unit->getField<uint32_t>(UnitField::RangedAttackPowerModsNeg));
            if (mask[92])
                data << float(unit->getRangedAttackPowerMultiplier());
            for (uint8_t bit = 93; bit <= 95; ++bit)
                if (mask[bit])
                    data << int32_t(0);
        }

        if (mask[96])
        {
            if (mask[97])
                data << int32_t(0);
            if (mask[98])
                data << float(0.0f);
            if (mask[99])
                data << float(unit->getMinRangedDamage());
            if (mask[100])
                data << float(unit->getMaxRangedDamage());
            if (mask[101])
                data << float(0.0f);
            if (mask[102])
                data << unit->getField<float>(UnitField::MaxHealthModifier);
            if (mask[103])
                data << float(unit->getHoverHeight());
            if (mask[104])
                data << int32_t(0);
            if (mask[105])
                data << int32_t(unit->getField<uint32_t>(UnitField::MinItemLevel));
            if (mask[106])
                data << int32_t(unit->getField<uint32_t>(UnitField::MaxItemLevel));
            if (mask[107])
                data << int32_t(0);
            if (mask[108])
                data << int32_t(unit->getField<uint32_t>(UnitField::WildBattlePetLevel));
            if (mask[109])
                data << int32_t(0);
            if (mask[110])
                data << uint32_t(unit->getField<uint32_t>(UnitField::BattlePetCompanionNameTimestamp));
            if (mask[111])
                data << int32_t(unit->getField<uint32_t>(UnitField::InteractSpellId));
            for (uint8_t bit = 112; bit <= 116; ++bit)
                if (mask[bit])
                    data << int32_t(0);
            if (mask[117])
                ctx.guildGuid(unitGuildId(unit));
            if (mask[118])
                data << uint32_t(0);
            if (mask[119])
                data << WoWGuid128();
        }

        if (mask[120])
        {
            for (uint8_t i = 0; i < 2; ++i)
                if (mask[121 + i])
                    data << uint32_t(unit->getField<uint32_t>(UnitField::NpcFlags, 0, i * 4));
        }

        if (mask[123])
        {
            for (uint8_t i = 0; i < PowerCount; ++i)
            {
                if (mask[124 + i])
                    data << unitPower(unit, i);
                if (mask[131 + i])
                    data << unitMaxPower(unit, i);
                if (mask[138 + i])
                    data << unitPowerRegen(unit, UnitField::PowerRegenFlatModifier, i);
                if (mask[145 + i])
                    data << unitPowerRegen(unit, UnitField::PowerRegenInterruptedFlatModifier, i);
            }
        }

        if (mask[152])
        {
            for (uint8_t i = 0; i < 3; ++i)
            {
                if (!mask[153 + i])
                    continue;

                // the visible item with its own mask: every field
                data.writeBits(0x1F, 5);
                data.flushBits();
                unitVisibleItemCreate(unit, i, data);
            }
        }

        if (mask[156])
        {
            for (uint8_t i = 0; i < 2; ++i)
                if (mask[157 + i])
                    data << uint32_t(unit->getBaseAttackTime(i));
        }

        if (mask[159])
        {
            for (uint8_t i = 0; i < 4; ++i)
            {
                if (mask[160 + i])
                    data << int32_t(unit->getStat(i));
                if (mask[164 + i])
                    data << int32_t(unit->getPosStat(i));
                if (mask[168 + i])
                    data << int32_t(unit->getNegStat(i));
            }
        }

        if (mask[172])
        {
            for (uint8_t i = 0; i < 7; ++i)
            {
                if (mask[173 + i])
                    data << int32_t(unit->getResistance(i));
                if (mask[180 + i])
                    data << int32_t(unit->getField<uint32_t>(UnitField::ResistanceBuffModPositive, i)) - int32_t(unit->getField<uint32_t>(UnitField::ResistanceBuffModNegative, i));
                if (mask[187 + i])
                    data << int32_t(unit->getPowerCostModifier(i));
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // PlayerData
    using PlayerMask = ChangeMask<188>;

    constexpr uint8_t QuestLogCount = 125;
    constexpr uint8_t VisibleItemCount = 19;
    constexpr uint8_t AvgItemLevelCount = 6;

    // the quest of a log slot: id, state flags, end time, accept time, objective flags, objective progress
    void playerQuestLogCreate(Player* player, uint8_t slot, ByteBuffer& data)
    {
        const QuestLogEntry* questLog = slot < MAX_QUEST_LOG_SIZE ? player->getQuestLogBySlotId(slot) : nullptr;
        data << int32_t(questLog != nullptr ? questLog->getQuestProperties()->id : 0);
        data << uint32_t(0);                                                // state flags
        data << uint32_t(0);                                                // end time
        data << uint32_t(0);                                                // accept time
        data << uint32_t(0);                                                // objective flags
        for (uint8_t i = 0; i < 24; ++i)
            data << int16_t(questLog != nullptr && i < 4 ? questLog->getMobCountByIndex(i) : 0);
    }

    void playerVisibleItemCreate(Player* player, uint8_t slot, ByteBuffer& data)
    {
        data << int32_t(player->getVisibleItemEntry(slot));
        data << int32_t(0);                                                 // secondary item modified appearance
        data << uint16_t(0);                                                // appearance modifier
        data << uint16_t(player->getVisibleItemEnchantment(slot, 0));       // visual: the permanent enchantment
    }

    // the content tuning options: condition mask, unknown, expansion level mask
    void playerCtrOptionsCreate(ByteBuffer& data)
    {
        data << int32_t(0) << uint32_t(0) << uint32_t(0);
    }

    // the dungeon score summary: two scores, no runs
    void playerDungeonScoreSummaryCreate(ByteBuffer& data)
    {
        data << float(0.0f) << float(0.0f) << uint32_t(0);
    }

    void playerCreate(Player* player, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;

        ctx.guid(player->getDuelArbiter());
        data << WoWGuid128::global(HighGuid128::WowAccount, player->getSession() != nullptr ? player->getSession()->GetAccountId() : 0);
        data << WoWGuid128();                                               // loot target
        data << uint32_t(player->getPlayerFlags());
        data << uint32_t(0);                                                // player flags ex
        data << uint32_t(player->getGuildRank());
        data << uint32_t(player->getField<uint32_t>(PlayerField::GuildDeleteDate));
        data << int32_t(player->getField<uint32_t>(PlayerField::GuildLevel));
        data << uint32_t(0);                                                // customizations: the look bytes of the server layout have no 9.x list
        data << uint8_t(0);                                                 // party type
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
            data << uint32_t(0);                                            // quest session quest log
        }

        for (uint8_t i = 0; i < VisibleItemCount; ++i)
            playerVisibleItemCreate(player, i, data);

        data << int32_t(player->getChosenTitle());
        data << int32_t(player->getField<uint32_t>(PlayerField::Inebriation)); // fake inebriation
        data << uint32_t(player->getField<uint32_t>(PlayerField::VirtualPlayerRealm));
        data << uint32_t(player->getCurrentSpecId());
        data << int32_t(player->getField<uint32_t>(PlayerField::TaxiMountAnimKitId));
        for (uint8_t i = 0; i < AvgItemLevelCount; ++i)
            data << float(0.0f);                                            // average item level
        data << uint8_t(player->getField<uint8_t>(PlayerField::CurrentBattlePetBreedQuality));
        data << int32_t(0);                                                 // honor level
        data << uint32_t(0);                                                // arena cooldowns
        data << int32_t(0);                                                 // field b0
        data << int32_t(0);                                                 // field b4
        playerCtrOptionsCreate(data);
        data << int32_t(0);                                                 // covenant
        data << int32_t(0);                                                 // soulbind

        if (ctx.partyMember())
            data.writeBit(false);                                           // has a quest session
        data.writeBit(false);                                               // has a level link
        data.flushBits();
        playerDungeonScoreSummaryCreate(data);
    }

    void playerCollect(ChangedFields const& changed, PlayerMask& mask)
    {
        struct Single { uint8_t bit; PlayerField field; };
        static constexpr Single singles[] =
        {
            { 6, PlayerField::DuelArbiter }, { 9, PlayerField::PlayerFlags }, { 11, PlayerField::GuildRank }, { 12, PlayerField::GuildDeleteDate },
            { 13, PlayerField::GuildLevel }, { 19, PlayerField::DuelTeam }, { 20, PlayerField::GuildTimestamp }, { 21, PlayerField::ChosenTitle },
            { 22, PlayerField::Inebriation }, { 23, PlayerField::VirtualPlayerRealm }, { 24, PlayerField::CurrentSpecId }, { 25, PlayerField::TaxiMountAnimKitId },
            { 26, PlayerField::CurrentBattlePetBreedQuality }
        };

        for (const Single& single : singles)
            if (changed.changed(single.field))
                mask.set(single.bit);

        // the look bytes (PlayerBytes, PlayerBytes2) have no 9.x field: the customization list stays empty
        if (changed.changed(PlayerField::PlayerBytes3))
            mask.setRange(15, 4);

        for (uint8_t i = 0; i < VisibleItemCount; ++i)
            if (changed.changed(PlayerField::VisibleItemsEntry, i) || changed.changed(PlayerField::VisibleItemsEnchantment, i))
                mask.set(162 + i);

        for (uint8_t bit = 1; bit < 32; ++bit)
            if (mask[bit])
                mask.set(0);
        for (uint8_t i = 0; i < VisibleItemCount; ++i)
            if (mask[162 + i])
                mask.set(161);
    }

    void playerFilter(PlayerMask& mask, uint8_t flags)
    {
        uint32_t allowed[6] = { 0xFFFFFFED, 0x00000007, 0, 0, 0, 0x0FFFFFFE };
        if (flags & FlagPartyMember)
        {
            const uint32_t party[6] = { 0x00000012, 0xFFFFFFF8, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001 };
            for (size_t i = 0; i < 6; ++i)
                allowed[i] |= party[i];
        }
        mask.filter(allowed);
    }

    void playerUpdate(Player* player, Context const& ctx, PlayerMask const& mask)
    {
        ByteBuffer& data = ctx.data;

        mask.writeBlocks(data);
        data.writeBit(false);                                               // the quest log keeps its change masks
        if (mask[0])
        {
            if (mask[1])
                data.writeBit(false);                                       // has a quest session
            if (mask[2])
                data.writeBit(false);                                       // has a level link
            if (mask[3])
                writeDynamicMask(data, 0);                                  // customizations
            if (mask[4])
                writeDynamicMask(data, 0);                                  // quest session quest log
            if (mask[5])
                writeDynamicMask(data, 0);                                  // arena cooldowns
        }
        data.flushBits();

        if (mask[0])
        {
            if (mask[6])
                ctx.guid(player->getDuelArbiter());
            if (mask[7])
                data << WoWGuid128::global(HighGuid128::WowAccount, player->getSession() != nullptr ? player->getSession()->GetAccountId() : 0);
            if (mask[8])
                data << WoWGuid128();
            if (mask[9])
                data << uint32_t(player->getPlayerFlags());
            if (mask[10])
                data << uint32_t(0);
            if (mask[11])
                data << uint32_t(player->getGuildRank());
            if (mask[12])
                data << uint32_t(player->getField<uint32_t>(PlayerField::GuildDeleteDate));
            if (mask[13])
                data << int32_t(player->getField<uint32_t>(PlayerField::GuildLevel));
            if (mask[14])
                data << uint8_t(0);
            if (mask[15])
                data << uint8_t(player->getPlayerGender());
            if (mask[16])
                data << uint8_t(player->getDrunkValue());
            if (mask[17])
                data << uint8_t(player->getPvpRank());
            if (mask[18])
                data << uint8_t(player->getArenaFaction());
            if (mask[19])
                data << uint32_t(player->getDuelTeam());
            if (mask[20])
                data << int32_t(player->getGuildTimestamp());
            if (mask[21])
                data << int32_t(player->getChosenTitle());
            if (mask[22])
                data << int32_t(player->getField<uint32_t>(PlayerField::Inebriation));
            if (mask[23])
                data << uint32_t(player->getField<uint32_t>(PlayerField::VirtualPlayerRealm));
            if (mask[24])
                data << uint32_t(player->getCurrentSpecId());
            if (mask[25])
                data << int32_t(player->getField<uint32_t>(PlayerField::TaxiMountAnimKitId));
            if (mask[26])
                data << uint8_t(player->getField<uint8_t>(PlayerField::CurrentBattlePetBreedQuality));
            if (mask[27])
                data << int32_t(0);
            if (mask[28])
                data << int32_t(0);
            if (mask[29])
                data << int32_t(0);
            if (mask[30])
                playerCtrOptionsCreate(data);
            if (mask[31])
                data << int32_t(0);
        }

        if (mask[32])
        {
            if (mask[33])
                data << int32_t(0);
            if (mask[34])
                playerDungeonScoreSummaryCreate(data);
        }

        if (mask[35])
        {
            for (uint8_t i = 0; i < QuestLogCount; ++i)
            {
                if (!mask[36 + i])
                    continue;

                // the quest log entry with its own mask: every field
                data.writeBits(1, 1);
                data.writeBits(0x7FFFFFFF, 32);
                data.flushBits();
                playerQuestLogCreate(player, i, data);
            }
        }

        if (mask[161])
        {
            for (uint8_t i = 0; i < VisibleItemCount; ++i)
            {
                if (!mask[162 + i])
                    continue;

                data.writeBits(0x1F, 5);
                data.flushBits();
                playerVisibleItemCreate(player, i, data);
            }
        }

        if (mask[181])
        {
            for (uint8_t i = 0; i < AvgItemLevelCount; ++i)
                if (mask[182 + i])
                    data << float(0.0f);
        }

        data.flushBits();
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // ActivePlayerData: the player itself
    using ActivePlayerMask = ChangeMask<1554>;

    constexpr uint16_t InvSlotCount = 199;
    constexpr uint16_t ExploredZoneCount = 240;
    constexpr uint16_t SkillCount = 256;
    constexpr uint16_t LegacySkillCount = 128;
    constexpr uint16_t QuestCompletedCount = 875;
    constexpr uint8_t KnownTitleCount = 6;
    constexpr uint8_t PvpInfoCount = 6;
    constexpr uint8_t PvpInfoValueCount = 11;

    // the inventory of the server layout in the order of the client slots
    uint64_t activeInvSlot(Player* player, uint16_t slot)
    {
        struct Range { PlayerField field; uint16_t count; };
        static constexpr Range ranges[] =
        {
            { PlayerField::InventorySlot, 23 }, { PlayerField::PackSlot, 16 }, { PlayerField::BankSlot, 28 },
            { PlayerField::BankBagSlot, 7 }, { PlayerField::VendorBuyBackSlot, 12 }
        };

        for (const Range& range : ranges)
        {
            if (slot < range.count)
                return player->getField<uint64_t>(range.field, slot);
            slot -= range.count;
        }

        return 0;
    }

    bool activeInvSlotChanged(ChangedFields const& changed, uint16_t slot)
    {
        struct Range { PlayerField field; uint16_t count; };
        static constexpr Range ranges[] =
        {
            { PlayerField::InventorySlot, 23 }, { PlayerField::PackSlot, 16 }, { PlayerField::BankSlot, 28 },
            { PlayerField::BankBagSlot, 7 }, { PlayerField::VendorBuyBackSlot, 12 }
        };

        for (const Range& range : ranges)
        {
            if (slot < range.count)
                return changed.changed(range.field, slot);
            slot -= range.count;
        }

        return false;
    }

    // the explored zones as 64 bit words, the server layout ends after 160 of the 240 words
    uint64_t activeExploredZone(Player* player, uint16_t index)
    {
        return uint64_t(player->getExploredZone(index * 2)) | (uint64_t(player->getExploredZone(index * 2 + 1)) << 32);
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

    // the skills with their own mask: the whole structure
    void activeSkillUpdate(Player* player, ByteBuffer& data)
    {
        ChangeMask<1793> mask;
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

    // the pvp info of a bracket: eleven counters and the disqualified bit
    void activePvpInfoCreate(ByteBuffer& data)
    {
        for (uint8_t i = 0; i < PvpInfoValueCount; ++i)
            data << uint32_t(0);
        data.writeBit(false);
        data.flushBits();
    }

    // the dungeon score: no seasons, no runs
    void activeDungeonScoreCreate(ByteBuffer& data)
    {
        data << uint32_t(0) << int32_t(0);
    }

    WoWGuid128 activeBnetAccount(Player* player)
    {
        return WoWGuid128::global(HighGuid128::BNetAccount, player->getSession() != nullptr ? player->getSession()->GetAccountId() : 0);
    }

    void activePlayerCreate(Player* player, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;

        for (uint16_t i = 0; i < InvSlotCount; ++i)
            ctx.guid(activeInvSlot(player, i));
        ctx.guid(player->getFarsightGuid());
        data << WoWGuid128();                                               // summoned battle pet
        data << uint32_t(KnownTitleCount);
        data << uint64_t(player->getCoinage());
        data << int32_t(player->getXp());
        data << int32_t(player->getNextLevelXp());
        data << int32_t(0);                                                 // trial xp
        activeSkillCreate(player, data);
        data << int32_t(player->getField<uint32_t>(PlayerField::CharacterPoints1));
        data << int32_t(player->getField<uint32_t>(PlayerField::MaxTalentTiers));
        data << uint32_t(player->getTrackCreature());
        activeStatsCreate(player, data);

        for (uint16_t i = 0; i < ExploredZoneCount; ++i)
            data << activeExploredZone(player, i);

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
        data << uint32_t(0);                                                // local flags
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

        data << int32_t(player->getMaxLevel());
        data << int32_t(0);                                                 // scaling player level delta
        data << int32_t(0);                                                 // max creature scaling level

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
        data << uint8_t(16);                                                // backpack slots
        data << int32_t(player->getField<uint32_t>(PlayerField::OverrideSpellId));
        data << uint16_t(player->getField<uint16_t>(PlayerField::LootSpecId));
        data << uint32_t(player->getField<uint32_t>(PlayerField::OverrideZonePvpType));
        data << activeBnetAccount(player);
        data << uint64_t(0);                                                // guild club member

        for (uint8_t i = 0; i < 4; ++i)
            data << uint32_t(0);                                            // bag slot flags
        for (uint8_t i = 0; i < 7; ++i)
            data << uint32_t(0);                                            // bank bag slot flags

        for (uint16_t i = 0; i < QuestCompletedCount; ++i)
            data << uint64_t(0);

        data << int32_t(0);                                                 // honor
        data << int32_t(0);                                                 // honor next level
        data << uint8_t(player->getBankSlots());

        // the dynamic fields: research sites, research site progress, daily quests, quest lines, heirlooms,
        // heirloom flags, toys, toy flags, transmog, conditional transmog, self res spells, runeforge powers,
        // transmog illusions, character restrictions, spell modifiers by label
        for (uint8_t i = 0; i < 16; ++i)
            data << uint32_t(0);
        data << uint32_t(0);                                                // research
        data << uint32_t(0);                                                // maw powers
        data << uint32_t(0);                                                // multi floor exploration
        data << uint32_t(0);                                                // recipe progression
        data << uint32_t(0);                                                // replayed quests
        data << uint32_t(0);                                                // disabled spells
        data << int32_t(0);                                                 // ui chromie time expansion
        data << int32_t(0);                                                 // transport server time
        data << uint32_t(0);                                                // weekly rewards period since origin
        data << int16_t(0);                                                 // soulbind conduit rank

        for (uint8_t i = 0; i < KnownTitleCount; ++i)
            data << uint64_t(player->getKnownTitles(i));

        for (uint8_t i = 0; i < PvpInfoCount; ++i)
            activePvpInfoCreate(data);

        data.flushBits();
        data.writeBit(false);                                               // backpack auto sort disabled
        data.writeBit(false);                                               // bank auto sort disabled
        data.writeBit(false);                                               // sort bags right to left
        data.writeBit(false);                                               // insert items left to right
        data.writeBits(0, 1);                                               // has a quest session
        data.flushBits();
        data << WoWGuid128();                                               // field 1410: guid and value
        data << int32_t(0);
        activeDungeonScoreCreate(data);
        data.flushBits();
    }

    void activePlayerCollect(ChangedFields const& changed, ActivePlayerMask& mask)
    {
        struct Single { uint16_t bit; PlayerField field; };
        static constexpr Single singles[] =
        {
            { 5, PlayerField::FieldKnownTitles }, { 29, PlayerField::FarsightGuid }, { 30, PlayerField::SummonedBattlePetGuid }, { 31, PlayerField::FieldCoinage },
            { 32, PlayerField::Xp }, { 33, PlayerField::NextLevelXp }, { 36, PlayerField::FieldSkillInfo }, { 37, PlayerField::CharacterPoints1 },
            { 38, PlayerField::MaxTalentTiers }, { 39, PlayerField::TrackCreatures }, { 40, PlayerField::Expertise }, { 41, PlayerField::OffhandExpertise },
            { 42, PlayerField::RangedExpertise }, { 43, PlayerField::CombatRatingExpertise }, { 44, PlayerField::BlockPct }, { 45, PlayerField::DodgePct },
            { 47, PlayerField::ParryPct }, { 49, PlayerField::CritPct }, { 50, PlayerField::RangedCritPct }, { 51, PlayerField::OffhandCritPct },
            { 52, PlayerField::SpellCritPct }, { 53, PlayerField::ShieldBlock }, { 54, PlayerField::ShieldBlockCritPct }, { 55, PlayerField::Mastery },
            { 61, PlayerField::PvpPowerDamage }, { 62, PlayerField::PvpPowerHealing }, { 63, PlayerField::FieldModHealingDone }, { 64, PlayerField::FieldModHealingPct },
            { 65, PlayerField::FieldModPeriodicHealingDonePct }, { 67, PlayerField::ModSpellPowerPct }, { 68, PlayerField::ModResiliencePct },
            { 69, PlayerField::OverrideSpellPowerByApPct }, { 70, PlayerField::OverrideApBySpellPowerPct }, { 71, PlayerField::FieldModTargetResistance },
            { 72, PlayerField::FieldModTargetPhysicalResistance }, { 78, PlayerField::FieldPvpMedals }, { 81, PlayerField::FieldLifetimeHonorableKills },
            { 82, PlayerField::FieldWatchedFactionIdx }, { 83, PlayerField::FieldMaxLevel }, { 86, PlayerField::PetSpellPower }, { 87, PlayerField::UiHitMod },
            { 88, PlayerField::UiHitSpellMod }, { 89, PlayerField::UiHomeRealmTimeOffset }, { 90, PlayerField::ModPetHaste }, { 94, PlayerField::PlayerFieldBytes2 },
            { 96, PlayerField::OverrideSpellId }, { 97, PlayerField::LootSpecId }, { 99, PlayerField::OverrideZonePvpType }, { 104, PlayerField::PlayerBytes2 }
        };

        for (const Single& single : singles)
            if (changed.changed(single.field))
                mask.set(single.bit);

        if (changed.changed(PlayerField::PlayerFieldBytes))
            mask.setRange(74, 3);
        if (changed.changed(PlayerField::FieldKills))
            mask.setRange(79, 2);

        for (uint16_t i = 0; i < InvSlotCount; ++i)
            if (activeInvSlotChanged(changed, i))
                mask.set(113 + i);
        for (uint16_t i = 0; i < ExploredZoneCount; ++i)
            if (changed.changed(PlayerField::ExploredZones, i * 2) || changed.changed(PlayerField::ExploredZones, i * 2 + 1))
                mask.set(313 + i);
        if (changed.changed(PlayerField::RestStateXp) || changed.changed(PlayerField::PlayerBytes2))
            mask.set(554);
        for (uint8_t i = 0; i < 7; ++i)
        {
            if (changed.changed(PlayerField::FieldModDamageDonePositive, i))
                mask.set(557 + i);
            if (changed.changed(PlayerField::FieldModDamageDoneNegative, i))
                mask.set(564 + i);
            if (changed.changed(PlayerField::FieldModDamageDonePct, i))
                mask.set(571 + i);
            if (changed.changed(PlayerField::FieldModHealingDonePct))
                mask.set(578 + i);
        }
        for (uint8_t i = 0; i < 3; ++i)
            if (changed.changed(PlayerField::WeaponDmgMultiplier, i))
                mask.set(586 + i);
        for (uint8_t i = 0; i < 12; ++i)
        {
            if (changed.changed(PlayerField::FieldBuyBackPrice, i))
                mask.set(593 + i);
            if (changed.changed(PlayerField::FieldBuyBackTimestamp, i))
                mask.set(605 + i);
        }
        for (uint8_t i = 0; i < 32; ++i)
            if (changed.changed(PlayerField::FieldCombatRating, i))
                mask.set(618 + i);
        for (uint8_t i = 0; i < 4; ++i)
            if (changed.changed(PlayerField::NoReagentCost, i))
                mask.set(658 + i);
        for (uint8_t i = 0; i < 2; ++i)
            if (changed.changed(PlayerField::ProfessionSkillLine, i))
                mask.set(663 + i);

        // the group bits
        static constexpr uint16_t groups[] = { 0, 34, 66, 98 };
        for (const uint16_t group : groups)
            for (uint16_t bit = group + 1; bit < group + 32; ++bit)
                if (mask[bit])
                    mask.set(group);
        struct Array { uint16_t parent; uint16_t first; uint16_t count; };
        static constexpr Array arrays[] =
        {
            { 27, 28, 1 }, { 112, 113, InvSlotCount }, { 312, 313, ExploredZoneCount }, { 553, 554, 2 }, { 556, 557, 28 },
            { 585, 586, 6 }, { 592, 593, 24 }, { 617, 618, 32 }, { 650, 651, PvpInfoCount }, { 657, 658, 4 }, { 662, 663, 2 },
            { 665, 666, 4 }, { 670, 671, 7 }, { 678, 679, QuestCompletedCount }
        };
        for (const Array& array : arrays)
            for (uint16_t i = 0; i < array.count; ++i)
                if (mask[array.first + i])
                    mask.set(array.parent);
    }

    void activePlayerUpdate(Player* player, Context const& ctx, ActivePlayerMask const& mask)
    {
        ByteBuffer& data = ctx.data;

        mask.writeLarge(data);
        if (mask[0])
        {
            for (uint8_t bit = 1; bit <= 4; ++bit)
                if (mask[bit])
                    data.writeBit(false);
            if (mask[5])
                writeDynamicMask(data, KnownTitleCount);
            for (uint8_t bit = 6; bit <= 21; ++bit)
                if (mask[bit])
                    writeDynamicMask(data, 0);
        }
        if (mask[27] && mask[28])
            writeDynamicMask(data, 0);                                      // research
        data.flushBits();
        if (mask[0])
        {
            for (uint8_t bit = 22; bit <= 26; ++bit)
                if (mask[bit])
                    writeDynamicMask(data, 0);
        }
        data.flushBits();

        if (mask[0])
        {
            if (mask[5])
                for (uint8_t i = 0; i < KnownTitleCount; ++i)
                    data << uint64_t(player->getKnownTitles(i));
            if (mask[29])
                ctx.guid(player->getFarsightGuid());
            if (mask[30])
                data << WoWGuid128();
            if (mask[31])
                data << uint64_t(player->getCoinage());
            if (mask[32])
                data << int32_t(player->getXp());
            if (mask[33])
                data << int32_t(player->getNextLevelXp());
        }

        if (mask[34])
        {
            if (mask[35])
                data << int32_t(0);
            if (mask[36])
                activeSkillUpdate(player, data);
            if (mask[37])
                data << int32_t(player->getField<uint32_t>(PlayerField::CharacterPoints1));
            if (mask[38])
                data << int32_t(player->getField<uint32_t>(PlayerField::MaxTalentTiers));
            if (mask[39])
                data << uint32_t(player->getTrackCreature());

            // every stat of the block: the create order is the bit order
            ByteBuffer stats;
            activeStatsCreate(player, stats);
            size_t position = 0;
            for (uint16_t bit = 40; bit <= 62; ++bit)
            {
                if (mask[bit])
                    data.append(stats.contents() + position, 4);
                position += 4;
            }
            if (mask[63])
                data << int32_t(player->getModHealingDone());
            if (mask[64])
                data << player->getField<float>(PlayerField::FieldModHealingPct);
            if (mask[65])
                data << player->getField<float>(PlayerField::FieldModPeriodicHealingDonePct);
        }

        if (mask[66])
        {
            if (mask[67])
                data << player->getField<float>(PlayerField::ModSpellPowerPct);
            if (mask[68])
                data << player->getField<float>(PlayerField::ModResiliencePct);
            if (mask[69])
                data << player->getField<float>(PlayerField::OverrideSpellPowerByApPct);
            if (mask[70])
                data << player->getField<float>(PlayerField::OverrideApBySpellPowerPct);
            if (mask[71])
                data << int32_t(player->getField<uint32_t>(PlayerField::FieldModTargetResistance));
            if (mask[72])
                data << int32_t(player->getField<uint32_t>(PlayerField::FieldModTargetPhysicalResistance));
            if (mask[73])
                data << uint32_t(0);
            if (mask[74])
                data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytesRafLevel));
            if (mask[75])
                data << uint8_t(player->getEnabledActionBars());
            if (mask[76])
                data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytesMaxPvpRank));
            if (mask[77])
                data << uint8_t(0);
            if (mask[78])
                data << uint32_t(player->getField<uint32_t>(PlayerField::FieldPvpMedals));
            if (mask[79])
                data << player->getField<uint16_t>(PlayerField::FieldKillsKillsToday);
            if (mask[80])
                data << player->getField<uint16_t>(PlayerField::FieldKillsKillsYesterday);
            if (mask[81])
                data << player->getField<uint32_t>(PlayerField::FieldLifetimeHonorableKills);
            if (mask[82])
                data << int32_t(player->getWatchedFaction());
            if (mask[83])
                data << int32_t(player->getMaxLevel());
            if (mask[84])
                data << int32_t(0);
            if (mask[85])
                data << int32_t(0);
            if (mask[86])
                data << int32_t(player->getField<uint32_t>(PlayerField::PetSpellPower));
            if (mask[87])
                data << player->getField<float>(PlayerField::UiHitMod);
            if (mask[88])
                data << player->getField<float>(PlayerField::UiHitSpellMod);
            if (mask[89])
                data << int32_t(player->getField<uint32_t>(PlayerField::UiHomeRealmTimeOffset));
            if (mask[90])
                data << player->getField<float>(PlayerField::ModPetHaste);
            if (mask[91])
                data << int8_t(0);
            if (mask[92])
                data << int8_t(0);
            if (mask[93])
                data << uint8_t(0);
            if (mask[94])
                data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytes2AuraVision));
            if (mask[95])
                data << uint8_t(16);
            if (mask[96])
                data << int32_t(player->getField<uint32_t>(PlayerField::OverrideSpellId));
            if (mask[97])
                data << uint16_t(player->getField<uint16_t>(PlayerField::LootSpecId));
        }

        if (mask[98])
        {
            if (mask[99])
                data << uint32_t(player->getField<uint32_t>(PlayerField::OverrideZonePvpType));
            if (mask[100])
                data << activeBnetAccount(player);
            if (mask[101])
                data << uint64_t(0);
            if (mask[102])
                data << int32_t(0);
            if (mask[103])
                data << int32_t(0);
            if (mask[104])
                data << uint8_t(player->getBankSlots());
            if (mask[107])
                data << int32_t(0);
            if (mask[108])
                data << int32_t(0);
            if (mask[109])
                data << uint32_t(0);
            if (mask[110])
                data << int16_t(0);
        }
        if (mask[98])
        {
            data.writeBits(0, 1);                                           // has a quest session
            if (mask[106])
            {
                // field 1410 with its own mask: nothing changed
                data.writeBits(0, 3);
                data.flushBits();
            }
            if (mask[111])
            {
                data.flushBits();
                activeDungeonScoreCreate(data);
            }
        }
        data.flushBits();

        if (mask[112])
        {
            for (uint16_t i = 0; i < InvSlotCount; ++i)
                if (mask[113 + i])
                    ctx.guid(activeInvSlot(player, i));
        }
        if (mask[312])
        {
            for (uint16_t i = 0; i < ExploredZoneCount; ++i)
                if (mask[313 + i])
                    data << activeExploredZone(player, i);
        }
        if (mask[553])
        {
            for (uint8_t i = 0; i < 2; ++i)
            {
                if (!mask[554 + i])
                    continue;

                data.writeBits(0x7, 3);
                data.flushBits();
                activeRestInfoCreate(player, i, data);
            }
        }
        if (mask[556])
        {
            for (uint8_t i = 0; i < 7; ++i)
            {
                if (mask[557 + i])
                    data << int32_t(player->getModDamageDonePositive(i));
                if (mask[564 + i])
                    data << int32_t(player->getModDamageDoneNegative(i));
                if (mask[571 + i])
                    data << float(player->getModDamageDonePct(i));
                if (mask[578 + i])
                    data << player->getField<float>(PlayerField::FieldModHealingDonePct);
            }
        }
        if (mask[585])
        {
            for (uint8_t i = 0; i < 3; ++i)
            {
                if (mask[586 + i])
                    data << player->getField<float>(PlayerField::WeaponDmgMultiplier, i);
                if (mask[589 + i])
                    data << float(1.0f);
            }
        }
        if (mask[592])
        {
            for (uint8_t i = 0; i < 12; ++i)
            {
                if (mask[593 + i])
                    data << player->getField<uint32_t>(PlayerField::FieldBuyBackPrice, i);
                if (mask[605 + i])
                    data << int64_t(player->getField<uint32_t>(PlayerField::FieldBuyBackTimestamp, i));
            }
        }
        if (mask[617])
        {
            for (uint8_t i = 0; i < 32; ++i)
                if (mask[618 + i])
                    data << int32_t(player->getCombatRating(i));
        }
        if (mask[657])
        {
            for (uint8_t i = 0; i < 4; ++i)
                if (mask[658 + i])
                    data << player->getField<uint32_t>(PlayerField::NoReagentCost, i);
        }
        if (mask[662])
        {
            for (uint8_t i = 0; i < 2; ++i)
                if (mask[663 + i])
                    data << int32_t(player->getProfessionSkillLine(i));
        }
        if (mask[665])
        {
            for (uint8_t i = 0; i < 4; ++i)
                if (mask[666 + i])
                    data << uint32_t(0);
        }
        if (mask[670])
        {
            for (uint8_t i = 0; i < 7; ++i)
                if (mask[671 + i])
                    data << uint32_t(0);
        }
        if (mask[678])
        {
            for (uint16_t i = 0; i < QuestCompletedCount; ++i)
                if (mask[679 + i])
                    data << uint64_t(0);
        }
        if (mask[650])
        {
            // the pvp info follows the quest completed array, every bracket with its own mask
            for (uint8_t i = 0; i < PvpInfoCount; ++i)
            {
                if (!mask[651 + i])
                    continue;

                data.writeBits(0x1FFF, 13);
                data.writeBit(false);                                       // disqualified
                data.flushBits();
                for (uint8_t j = 0; j < PvpInfoValueCount; ++j)
                    data << uint32_t(0);
                data.flushBits();
            }
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

    // the modifier list: no entries, the size takes six bits
    void itemModifiersCreate(ByteBuffer& data)
    {
        data.writeBits(0, 6);
        data.flushBits();
    }

    void itemModifiersUpdate(ByteBuffer& data)
    {
        data.writeBits(1, 1);
        data.writeBits(0, 6);
        data.flushBits();
    }

    void itemCreate(Item* item, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;

        data << uint32_t(0);                                                // bonus list ids
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
        data << int32_t(0);                                                 // context
        data << int64_t(0);                                                 // create time, no server value
        if (ctx.owner())
        {
            data << uint64_t(0);                                            // artifact xp
            data << uint8_t(0);                                             // item appearance modifier
        }
        data << uint32_t(0);                                                // artifact powers
        data << uint32_t(0);                                                // gems
        if (ctx.owner())
        {
            data << uint32_t(0);                                            // dynamic flags 2
            data << uint16_t(0);                                            // debug item level
        }
        itemModifiersCreate(data);
    }

    void itemCollect(ChangedFields const& changed, ItemMask& mask)
    {
        struct Single { uint8_t bit; ItemField field; };
        static constexpr Single singles[] =
        {
            { 4, ItemField::OwnerGuid }, { 5, ItemField::ContainerGuid }, { 6, ItemField::CreatorGuid }, { 7, ItemField::GiftCreatorGuid },
            { 8, ItemField::StackCount }, { 9, ItemField::Duration }, { 10, ItemField::Flags }, { 11, ItemField::Durability },
            { 12, ItemField::MaxDurability }, { 13, ItemField::CreatePlayedTime }, { 18, ItemField::ModifierMask }
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
        uint32_t allowed[2] = { 0xF804E4FF, 0x000001FF };
        if (flags & FlagOwner)
            allowed[0] |= 0x07FB1B00;
        mask.filter(allowed);
    }

    void itemUpdate(Item* item, Context const& ctx, ItemMask const& mask)
    {
        ByteBuffer& data = ctx.data;

        mask.writeBlocks(data);
        if (mask[0] && mask[1])
            data.writeBits(0, 32);                                          // bonus list ids
        data.flushBits();
        if (mask[0])
        {
            if (mask[2])
                writeDynamicMask(data, 0);                                  // artifact powers
            if (mask[3])
                writeDynamicMask(data, 0);                                  // gems
        }
        data.flushBits();

        if (mask[0])
        {
            if (mask[4])
                ctx.guid(item->getOwnerGuid());
            if (mask[5])
                ctx.guid(item->getContainerGuid());
            if (mask[6])
                ctx.guid(item->getCreatorGuid());
            if (mask[7])
                ctx.guid(item->getGiftCreatorGuid());
            if (mask[8])
                data << uint32_t(item->getStackCount());
            if (mask[9])
                data << uint32_t(item->getDuration());
            if (mask[10])
                data << uint32_t(item->getFlags());
            if (mask[11])
                data << uint32_t(item->getDurability());
            if (mask[12])
                data << uint32_t(item->getMaxDurability());
            if (mask[13])
                data << uint32_t(item->getCreatePlayedTime());
            if (mask[14])
                data << int32_t(0);
            if (mask[15])
                data << int64_t(0);
            if (mask[16])
                data << uint64_t(0);
            if (mask[17])
                data << uint8_t(0);
            if (mask[19])
                data << uint32_t(0);
            if (mask[20])
                data << uint16_t(0);
            if (mask[18])
                itemModifiersUpdate(data);
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
    using ContainerMask = ChangeMask<39>;

    constexpr uint8_t ContainerSlotCount = 36;

    void containerCreate(Container* container, Context const& ctx)
    {
        for (uint8_t i = 0; i < ContainerSlotCount; ++i)
            ctx.guid(container->getSlot(i));
        ctx.data << uint32_t(container->getSlotCount());
    }

    void containerCollect(ChangedFields const& changed, ContainerMask& mask)
    {
        if (changed.changed(ContainerField::SlotCount))
        {
            mask.set(0);
            mask.set(1);
        }
        for (uint8_t i = 0; i < ContainerSlotCount; ++i)
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
                    ctx.guid(container->getSlot(i));
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // GameObjectData
    using GameObjectMask = ChangeMask<21>;

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
        data << uint32_t(gameobject->getFlags());
        for (uint8_t i = 0; i < 4; ++i)
            data << float(gameobject->getParentRotation(i));
        data << int32_t(gameobject->getFactionTemplate());
        data << int8_t(gameobject->getState());
        data << int8_t(gameobject->getGoType());
        data << uint8_t(gameobject->getField<uint8_t>(GameObjectField::Bytes1Health));
        data << uint32_t(gameobject->getArtKit());
        data << uint32_t(0);                                                // doodad sets
        data << uint32_t(0);                                                // custom parameter
        data << int32_t(gameobject->getLevel());
        data << uint32_t(0);                                                // anim group instance
    }

    void gameObjectCollect(ChangedFields const& changed, GameObjectMask& mask)
    {
        if (changed.changed(GameObjectField::DisplayId))
            mask.set(3);
        if (changed.changed(GameObjectField::ObjectFieldCreatedBy))
            mask.set(9);
        if (changed.changed(GameObjectField::Flags))
            mask.set(11);
        for (uint8_t i = 0; i < 4; ++i)
            if (changed.changed(GameObjectField::Rotation, i))
                mask.set(12);
        if (changed.changed(GameObjectField::FactionTemplate))
            mask.set(13);
        if (changed.changed(GameObjectField::Bytes1))
            mask.setRange(14, 4);
        if (changed.changed(GameObjectField::Level))
            mask.set(19);
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
        if (mask[0] && mask[2])
            writeDynamicMask(data, 0);                                      // doodad sets
        data.flushBits();

        if (mask[0])
        {
            if (mask[3])
                data << int32_t(gameobject->getDisplayId());
            for (uint8_t bit = 4; bit <= 8; ++bit)
                if (mask[bit])
                    data << uint32_t(0);
            if (mask[9])
                ctx.guid(gameobject->getCreatedByGuid());
            if (mask[10])
                data << WoWGuid128();
            if (mask[11])
                data << uint32_t(gameobject->getFlags());
            if (mask[12])
                for (uint8_t i = 0; i < 4; ++i)
                    data << float(gameobject->getParentRotation(i));
            if (mask[13])
                data << int32_t(gameobject->getFactionTemplate());
            if (mask[14])
                data << int8_t(gameobject->getState());
            if (mask[15])
                data << int8_t(gameobject->getGoType());
            if (mask[16])
                data << uint8_t(gameobject->getField<uint8_t>(GameObjectField::Bytes1Health));
            if (mask[17])
                data << uint32_t(gameobject->getArtKit());
            if (mask[18])
                data << uint32_t(0);
            if (mask[19])
                data << int32_t(gameobject->getLevel());
            if (mask[20])
                data << uint32_t(0);
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

    void corpseCreate(Corpse* corpse, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;

        data << uint32_t(viewerDynamicFlags(corpse, ctx.target));
        ctx.guid(corpse->getOwnerGuid());
        ctx.guid(corpse->getField<uint64_t>(CorpseField::PartyGuid));
        ctx.guildGuid(corpse->getField<uint32_t>(CorpseField::Guild));
        data << uint32_t(corpse->getDisplayId());
        for (uint8_t i = 0; i < CorpseItemCount; ++i)
            data << uint32_t(corpse->getItem(i));
        data << uint8_t(corpse->getRace());
        data << uint8_t(corpse->getGender());
        data << uint8_t(0);                                                 // class, no server value
        data << uint32_t(0);                                                // customizations: the look bytes of the server layout have no 9.x list
        data << uint32_t(corpse->getFlags());
        data << int32_t(0);                                                 // faction template
        data << uint32_t(0);                                                // state spell visual kit
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

        mask.writeBlocks(data);
        if (mask[0] && mask[1])
            writeDynamicMask(data, 0);                                      // customizations
        data.flushBits();
        if (mask[0])
        {
            if (mask[2])
                data << uint32_t(viewerDynamicFlags(corpse, ctx.target));
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
                data << uint8_t(0);
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
    // AreaTriggerData: the caster, the duration and the spell
    // a scale curve: start time offset, two points, parameter curve, override active
    void areaTriggerScaleCurveCreate(ByteBuffer& data)
    {
        data << uint32_t(0) << float(0) << float(0) << float(0) << float(0) << uint32_t(0);
        data.writeBit(false);
        data.flushBits();
    }

    void areaTriggerCreate(Object* object, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;

        areaTriggerScaleCurveCreate(data);                                  // override scale curve
        ctx.guid(object->getField<uint64_t>(AreaTriggerField::CasterGuid));
        data << uint32_t(object->getField<uint32_t>(AreaTriggerField::Duration));
        data << uint32_t(0);                                                // time to target
        data << uint32_t(0);                                                // time to target scale
        data << uint32_t(0);                                                // time to target extra scale
        data << int32_t(object->getField<uint32_t>(AreaTriggerField::SpellId));
        data << int32_t(object->getField<uint32_t>(AreaTriggerField::SpellId));   // spell for visuals
        data << int32_t(object->getField<uint32_t>(AreaTriggerField::SpellVisualId));
        data << int32_t(0);                                                 // script visual
        data << float(object->getField<float>(AreaTriggerField::Scale));    // bounds radius
        data << uint32_t(0);                                                // decal properties
        data << WoWGuid128();                                               // creating effect
        data << uint32_t(0);                                                // field 80
        data << uint32_t(0);                                                // field 84
        data << WoWGuid128();                                               // field 88
        areaTriggerScaleCurveCreate(data);                                  // extra scale curve
        // visual anim: animation data, anim kit, anim progress, the flag bit
        data << uint32_t(0) << uint32_t(0) << uint32_t(0);
        data.writeBit(false);
        data.flushBits();
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    Context makeContext(Object* object, Player* target, ByteBuffer& data)
    {
        return Context{ data, target, worldConfig.battleNetComm.realmId, target != nullptr ? target->GetMapId() : object->GetMapId(), fieldFlags(object, target) };
    }
}

namespace ObjectUpdateShadowlands
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

    void writeCreateValues(Object* object, Player* target, ByteBuffer& data)
    {
        const Context ctx = makeContext(object, target, data);

        const size_t sizePosition = data.wpos();
        data << uint32_t(0);
        data << uint8_t(ctx.flags);

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

        data.put<uint32_t>(sizePosition, static_cast<uint32_t>(data.wpos() - sizePosition - sizeof(uint32_t)));
    }

    void writeUpdateValues(Object* object, Player* target, UpdateMask const& mask, ByteBuffer& data)
    {
        const Context ctx = makeContext(object, target, data);
        const ChangedFields changed(&mask);

        const size_t sizePosition = data.wpos();
        data << uint32_t(0);
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

#endif
