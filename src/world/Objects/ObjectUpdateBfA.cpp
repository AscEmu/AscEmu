/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ObjectUpdateBfA.hpp"

#if VERSION_STRING == BfA

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
    // object types of 8.x
    enum WireTypeId : uint8_t
    {
        WireTypeObject = 0,
        WireTypeItem = 1,
        WireTypeContainer = 2,
        WireTypeUnit = 5,
        WireTypePlayer = 6,
        WireTypeActivePlayer = 7,
        WireTypeGameObject = 8,
        WireTypeDynamicObject = 9,
        WireTypeCorpse = 10,
        WireTypeAreaTrigger = 11
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
    using UnitMask = ChangeMask<192>;

    constexpr UnitField PowerFields[6] = { UnitField::Power1, UnitField::Power2, UnitField::Power3, UnitField::Power4, UnitField::Power5, UnitField::Power6 };
    constexpr UnitField MaxPowerFields[6] = { UnitField::MaxPower1, UnitField::MaxPower2, UnitField::MaxPower3, UnitField::MaxPower4, UnitField::MaxPower5, UnitField::MaxPower6 };

    // the visible item of a unit: entry, appearance modifier and visual from the slot display and its info
    void unitVisibleItem(Unit* unit, uint8_t slot, int32_t& itemId, uint16_t& appearanceModId, uint16_t& itemVisual)
    {
        itemId = static_cast<int32_t>(unit->getField<uint32_t>(UnitField::VirtualItemSlotDisplay, slot * 2));
        const uint32_t info = unit->getField<uint32_t>(UnitField::VirtualItemSlotDisplay, slot * 2 + 1);
        appearanceModId = static_cast<uint16_t>(info & 0xFFFF);
        itemVisual = static_cast<uint16_t>(info >> 16);
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

        data << int32_t(viewerDisplayId(unit, ctx.target));
        data << uint32_t(unit->getField<uint32_t>(UnitField::NpcFlags, 0, 0));
        data << uint32_t(unit->getField<uint32_t>(UnitField::NpcFlags, 0, 4));
        data << uint32_t(0);                                                // state spell visual
        data << uint32_t(0);                                                // state anim
        data << uint32_t(0);                                                // state anim kit
        data << uint32_t(0);                                                // state world effects
        data << uint32_t(0);                                                // state world effects quest objective
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
        data << int32_t(unit->getChannelSpellId());                         // channel data
        data << int32_t(0);
        data << uint32_t(0);                                                // summoned by home realm
        data << uint8_t(unit->getRace());
        data << uint8_t(unit->getClass());
        data << uint8_t(unit->getClass());                                  // player class
        data << uint8_t(unit->getGender());
        data << uint8_t(unit->getField<uint8_t>(UnitField::FieldBytes0PowerType));
        data << uint32_t(unit->getField<uint32_t>(UnitField::OverrideDisplayPowerId));
        data << int64_t(unit->getHealth());

        for (uint8_t i = 0; i < 6; ++i)
        {
            data << int32_t(unit->getField<uint32_t>(PowerFields[i]));
            data << int32_t(unit->getField<uint32_t>(MaxPowerFields[i]));
        }

        if (ctx.ownerOrUnitAll())
        {
            for (uint8_t i = 0; i < 6; ++i)
            {
                data << unit->getField<float>(UnitField::PowerRegenFlatModifier, i);
                data << unit->getField<float>(UnitField::PowerRegenInterruptedFlatModifier, i);
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
        {
            int32_t itemId;
            uint16_t appearanceModId;
            uint16_t itemVisual;
            unitVisibleItem(unit, i, itemId, appearanceModId, itemVisual);
            data << itemId << appearanceModId << itemVisual;
        }

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
        data << unit->getField<float>(UnitField::ModCastHaste);
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
            for (uint8_t i = 0; i < 7; ++i)
            {
                data << int32_t(unit->getField<uint32_t>(UnitField::ResistanceBuffModPositive, i)) - int32_t(unit->getField<uint32_t>(UnitField::ResistanceBuffModNegative, i));
                data << int32_t(unit->getPowerCostModifier(i));
                data << float(unit->getPowerCostMultiplier(i));
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
            data << float(0.0f);                                            // mana cost modifier modifier
            data << unit->getField<float>(UnitField::MaxHealthModifier);
        }

        data << float(unit->getHoverHeight());
        data << int32_t(0);                                                 // min item level cutoff
        data << int32_t(unit->getField<uint32_t>(UnitField::MinItemLevel));
        data << int32_t(unit->getField<uint32_t>(UnitField::MaxItemLevel));
        data << int32_t(0);                                                 // azerite item level
        data << int32_t(unit->getField<uint32_t>(UnitField::WildBattlePetLevel));
        data << uint32_t(unit->getField<uint32_t>(UnitField::BattlePetCompanionNameTimestamp));
        data << int32_t(unit->getField<uint32_t>(UnitField::InteractSpellId));
        data << int32_t(0);                                                 // scale duration
        data << int32_t(0);                                                 // spell override name
        data << int32_t(0);                                                 // looks like mount
        data << int32_t(0);                                                 // looks like creature
        data << int32_t(0);                                                 // look at controller
        data << int32_t(0);                                                 // taxi nodes
        ctx.guildGuid(unitGuildId(unit));
        data << uint32_t(0);                                                // passive spells
        data << uint32_t(0);                                                // world effects
        data << uint32_t(channelObject != 0 ? 1 : 0);                       // channel objects
        data << WoWGuid128();                                               // skinning owner
        if (channelObject != 0)
            ctx.guid(channelObject);
    }

    // bits of the fields that changed in the server layout
    void unitCollect(ChangedFields const& changed, UnitMask& mask)
    {
        struct Single { uint8_t bit; UnitField field; };
        static constexpr Single singles[] =
        {
            { 5, UnitField::DisplayId }, { 10, UnitField::CharmGuid }, { 11, UnitField::SummonGuid }, { 12, UnitField::CritterGuid },
            { 13, UnitField::CharmedByGuid }, { 14, UnitField::SummonedByGuid }, { 15, UnitField::CreatedByGuid }, { 16, UnitField::DemonCreatorGuid },
            { 18, UnitField::TargetGuid }, { 19, UnitField::BattlePetCompanionGuid }, { 21, UnitField::ChannelSpell }, { 22, UnitField::SummonedByHomeRealm },
            { 27, UnitField::DisplayPower }, { 28, UnitField::OverrideDisplayPowerId }, { 29, UnitField::Health }, { 30, UnitField::MaxHealth },
            { 31, UnitField::Level }, { 33, UnitField::EffectiveLevel }, { 41, UnitField::FactionTemplate }, { 42, UnitField::UnitFlags },
            { 43, UnitField::UnitFlags2 }, { 45, UnitField::AuraState }, { 47, UnitField::BoundingRadius }, { 48, UnitField::CombatReach },
            { 50, UnitField::NativeDisplayId }, { 52, UnitField::MountDisplayId }, { 54, UnitField::MinimumDamage }, { 55, UnitField::MaximumDamage },
            { 56, UnitField::MinimumOffhandDamage }, { 57, UnitField::MaximumOffhandDamage }, { 62, UnitField::PetNumber }, { 63, UnitField::PetNameTimestamp },
            { 65, UnitField::PetExperience }, { 66, UnitField::PetNextLevelExperience }, { 67, UnitField::ModCastSpeed }, { 68, UnitField::ModCastHaste },
            { 69, UnitField::ModHaste }, { 70, UnitField::ModRangedHaste }, { 71, UnitField::ModHasteRegen }, { 73, UnitField::CreatedBySpellId },
            { 74, UnitField::NpcEmoteState }, { 75, UnitField::BaseMana }, { 76, UnitField::BaseHealth }, { 81, UnitField::AttackPower },
            { 82, UnitField::AttackPowerModPos }, { 83, UnitField::AttackPowerModNeg }, { 84, UnitField::AttackPowerMultiplier }, { 85, UnitField::RangedAttackPower },
            { 86, UnitField::RangedAttackPowerModsPos }, { 87, UnitField::RangedAttackPowerModsNeg }, { 88, UnitField::RangedAttackPowerMultiplier },
            { 94, UnitField::MinimumRangedDamage }, { 95, UnitField::MaximumRangedDamage }, { 98, UnitField::MaxHealthModifier }, { 99, UnitField::HoverHeight },
            { 101, UnitField::MinItemLevel }, { 102, UnitField::MaxItemLevel }, { 104, UnitField::WildBattlePetLevel }, { 105, UnitField::BattlePetCompanionNameTimestamp },
            { 106, UnitField::InteractSpellId }
        };

        for (const Single& single : singles)
            if (changed.changed(single.field))
                mask.set(single.bit);

        if (changed.changed(UnitField::ChannelObjectGuid))
            mask.set(4);
        if (changed.changed(UnitField::FieldBytes0))
            mask.setRange(23, 4);
        if (changed.changed(UnitField::FieldBytes1))
            mask.setRange(58, 4);
        if (changed.changed(UnitField::FieldBytes2))
            mask.setRange(77, 4);
        if (changed.changed(PlayerField::GuildId))
            mask.set(113);
        if (changed.changed(UnitField::NpcFlags))
            mask.setRange(116, 2);

        for (uint8_t i = 0; i < 6; ++i)
        {
            if (changed.changed(PowerFields[i]))
                mask.set(119 + i);
            if (changed.changed(MaxPowerFields[i]))
                mask.set(125 + i);
            if (changed.changed(UnitField::PowerRegenFlatModifier, i))
                mask.set(131 + i);
            if (changed.changed(UnitField::PowerRegenInterruptedFlatModifier, i))
                mask.set(137 + i);
        }

        for (uint8_t i = 0; i < 3; ++i)
            if (changed.changed(UnitField::VirtualItemSlotDisplay, i * 2) || changed.changed(UnitField::VirtualItemSlotDisplay, i * 2 + 1))
                mask.set(144 + i);

        for (uint8_t i = 0; i < 2; ++i)
            if (changed.changed(UnitField::BaseAttackTime, i))
                mask.set(148 + i);

        for (uint8_t i = 0; i < 4; ++i)
        {
            if (changed.changed(UnitField::Stat, i))
                mask.set(151 + i);
            if (changed.changed(UnitField::PositiveStat, i))
                mask.set(155 + i);
            if (changed.changed(UnitField::NegativeStat, i))
                mask.set(159 + i);
        }

        for (uint8_t i = 0; i < 7; ++i)
        {
            if (changed.changed(UnitField::Resistance, i))
                mask.set(164 + i);
            if (changed.changed(UnitField::ResistanceBuffModPositive, i) || changed.changed(UnitField::ResistanceBuffModNegative, i))
                mask.set(171 + i);
            if (changed.changed(UnitField::PowerCostModifier, i))
                mask.set(178 + i);
            if (changed.changed(UnitField::PowerCostMultiplier, i))
                mask.set(185 + i);
        }

        // the group bits
        static constexpr uint8_t groups[] = { 0, 32, 64, 96 };
        for (const uint8_t group : groups)
            for (uint8_t bit = group + 1; bit < group + 32; ++bit)
                if (mask[bit])
                    mask.set(group);
        for (uint8_t i = 0; i < 2; ++i)
            if (mask[116 + i])
                mask.set(115);
        for (uint8_t i = 0; i < 24; ++i)
            if (mask[119 + i])
                mask.set(118);
        for (uint8_t i = 0; i < 3; ++i)
            if (mask[144 + i])
                mask.set(143);
        for (uint8_t i = 0; i < 2; ++i)
            if (mask[148 + i])
                mask.set(147);
        for (uint8_t i = 0; i < 12; ++i)
            if (mask[151 + i])
                mask.set(150);
        for (uint8_t i = 0; i < 28; ++i)
            if (mask[164 + i])
                mask.set(163);
    }

    // the fields a receiver may see
    void unitFilter(UnitMask& mask, uint8_t flags)
    {
        uint32_t allowed[6] = { 0xFFFFEFFF, 0xFC3FBFFF, 0x0001EFFF, 0xFFBFFFF9, 0x003F8007, 0x00000000 };
        if (flags & FlagOwner)
        {
            const uint32_t owner[6] = { 0x00001000, 0x03C04000, 0xFFFE1000, 0x00400006, 0xFFC07FF8, 0xFFFFFFFF };
            for (size_t i = 0; i < 6; ++i)
                allowed[i] |= owner[i];
        }
        if (flags & FlagUnitAll)
        {
            allowed[3] |= 0x00400000;
            allowed[4] |= 0x00007FF8;
        }
        if (flags & FlagEmpath)
        {
            allowed[1] |= 0x03C00000;
            allowed[5] |= 0x000007F8;
        }
        mask.filter(allowed);
    }

    void unitUpdate(Unit* unit, Context const& ctx, UnitMask const& mask)
    {
        ByteBuffer& data = ctx.data;
        const uint64_t channelObject = unit->getChannelObjectGuid();

        mask.writeBlocks(data);
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
            if (mask[6])
                data << uint32_t(0);
            if (mask[7])
                data << uint32_t(0);
            if (mask[8])
                data << uint32_t(0);
            if (mask[9])
                data << uint32_t(0);
            if (mask[10])
                ctx.guid(unit->getCharmGuid());
            if (mask[11])
                ctx.guid(unit->getSummonGuid());
            if (mask[12])
                ctx.guid(unit->getCritterGuid());
            if (mask[13])
                ctx.guid(unit->getCharmedByGuid());
            if (mask[14])
                ctx.guid(unit->getSummonedByGuid());
            if (mask[15])
                ctx.guid(unit->getCreatedByGuid());
            if (mask[16])
                ctx.guid(unit->getField<uint64_t>(UnitField::DemonCreatorGuid));
            if (mask[17])
                data << WoWGuid128();
            if (mask[18])
                ctx.guid(unit->getTargetGuid());
            if (mask[19])
                data << WoWGuid128();
            if (mask[20])
                data << uint64_t(0);
            if (mask[21])
                data << int32_t(unit->getChannelSpellId()) << int32_t(0);
            if (mask[22])
                data << uint32_t(0);
            if (mask[23])
                data << uint8_t(unit->getRace());
            if (mask[24])
                data << uint8_t(unit->getClass());
            if (mask[25])
                data << uint8_t(unit->getClass());
            if (mask[26])
                data << uint8_t(unit->getGender());
            if (mask[27])
                data << uint8_t(unit->getField<uint8_t>(UnitField::FieldBytes0PowerType));
            if (mask[28])
                data << uint32_t(unit->getField<uint32_t>(UnitField::OverrideDisplayPowerId));
            if (mask[29])
                data << int64_t(unit->getHealth());
            if (mask[30])
                data << int64_t(unit->getMaxHealth());
            if (mask[31])
                data << int32_t(unit->getLevel());
        }

        if (mask[32])
        {
            if (mask[33])
                data << int32_t(unit->getField<uint32_t>(UnitField::EffectiveLevel));
            for (uint8_t bit = 34; bit <= 40; ++bit)
                if (mask[bit])
                    data << int32_t(0);
            if (mask[41])
                data << int32_t(unit->getFactionTemplate());
            if (mask[42])
                data << uint32_t(viewerUnitFlags(unit, ctx.target));
            if (mask[43])
                data << uint32_t(unit->getUnitFlags2());
            if (mask[44])
                data << uint32_t(0);
            if (mask[45])
                data << uint32_t(unit->getAuraState());
            if (mask[46])
                data << uint32_t(2000);
            if (mask[47])
                data << float(unit->getBoundingRadius());
            if (mask[48])
                data << float(unit->getCombatReach());
            if (mask[49])
                data << float(1.0f);
            if (mask[50])
                data << int32_t(unit->getNativeDisplayId());
            if (mask[51])
                data << float(1.0f);
            if (mask[52])
                data << int32_t(unit->getMountDisplayId());
            if (mask[53])
                data << int32_t(0);
            if (mask[54])
                data << float(unit->getMinDamage());
            if (mask[55])
                data << float(unit->getMaxDamage());
            if (mask[56])
                data << float(unit->getMinOffhandDamage());
            if (mask[57])
                data << float(unit->getMaxOffhandDamage());
            if (mask[58])
                data << uint8_t(unit->getStandState());
            if (mask[59])
                data << uint8_t(unit->getPetTalentPoints());
            if (mask[60])
                data << uint8_t(unit->getStandStateFlags());
            if (mask[61])
                data << uint8_t(unit->getAnimationFlags());
            if (mask[62])
                data << uint32_t(unit->getPetNumber());
            if (mask[63])
                data << uint32_t(unit->getField<uint32_t>(UnitField::PetNameTimestamp));
        }

        if (mask[64])
        {
            if (mask[65])
                data << uint32_t(unit->getField<uint32_t>(UnitField::PetExperience));
            if (mask[66])
                data << uint32_t(unit->getField<uint32_t>(UnitField::PetNextLevelExperience));
            if (mask[67])
                data << float(unit->getModCastSpeed());
            if (mask[68])
                data << unit->getField<float>(UnitField::ModCastHaste);
            if (mask[69])
                data << unit->getField<float>(UnitField::ModHaste);
            if (mask[70])
                data << unit->getField<float>(UnitField::ModRangedHaste);
            if (mask[71])
                data << unit->getField<float>(UnitField::ModHasteRegen);
            if (mask[72])
                data << float(1.0f);
            if (mask[73])
                data << int32_t(unit->getCreatedBySpellId());
            if (mask[74])
                data << int32_t(unit->getEmoteState());
            if (mask[75])
                data << int32_t(unit->getBaseMana());
            if (mask[76])
                data << int32_t(unit->getBaseHealth());
            if (mask[77])
                data << uint8_t(unit->getSheathType());
            if (mask[78])
                data << uint8_t(unit->getPvpFlags());
            if (mask[79])
                data << uint8_t(unit->getPetFlags());
            if (mask[80])
                data << uint8_t(unit->getShapeShiftForm());
            if (mask[81])
                data << int32_t(unit->getAttackPower());
            if (mask[82])
                data << int32_t(unit->getField<uint32_t>(UnitField::AttackPowerModPos));
            if (mask[83])
                data << int32_t(unit->getField<uint32_t>(UnitField::AttackPowerModNeg));
            if (mask[84])
                data << float(unit->getAttackPowerMultiplier());
            if (mask[85])
                data << int32_t(unit->getRangedAttackPower());
            if (mask[86])
                data << int32_t(unit->getField<uint32_t>(UnitField::RangedAttackPowerModsPos));
            if (mask[87])
                data << int32_t(unit->getField<uint32_t>(UnitField::RangedAttackPowerModsNeg));
            if (mask[88])
                data << float(unit->getRangedAttackPowerMultiplier());
            for (uint8_t bit = 89; bit <= 92; ++bit)
                if (mask[bit])
                    data << int32_t(0);
            if (mask[93])
                data << float(0.0f);
            if (mask[94])
                data << float(unit->getMinRangedDamage());
            if (mask[95])
                data << float(unit->getMaxRangedDamage());
        }

        if (mask[96])
        {
            if (mask[97])
                data << float(0.0f);
            if (mask[98])
                data << unit->getField<float>(UnitField::MaxHealthModifier);
            if (mask[99])
                data << float(unit->getHoverHeight());
            if (mask[100])
                data << int32_t(0);
            if (mask[101])
                data << int32_t(unit->getField<uint32_t>(UnitField::MinItemLevel));
            if (mask[102])
                data << int32_t(unit->getField<uint32_t>(UnitField::MaxItemLevel));
            if (mask[103])
                data << int32_t(0);
            if (mask[104])
                data << int32_t(unit->getField<uint32_t>(UnitField::WildBattlePetLevel));
            if (mask[105])
                data << uint32_t(unit->getField<uint32_t>(UnitField::BattlePetCompanionNameTimestamp));
            if (mask[106])
                data << int32_t(unit->getField<uint32_t>(UnitField::InteractSpellId));
            for (uint8_t bit = 107; bit <= 112; ++bit)
                if (mask[bit])
                    data << int32_t(0);
            if (mask[113])
                ctx.guildGuid(unitGuildId(unit));
            if (mask[114])
                data << WoWGuid128();
        }

        if (mask[115])
        {
            for (uint8_t i = 0; i < 2; ++i)
                if (mask[116 + i])
                    data << uint32_t(unit->getField<uint32_t>(UnitField::NpcFlags, 0, i * 4));
        }

        if (mask[118])
        {
            for (uint8_t i = 0; i < 6; ++i)
            {
                if (mask[119 + i])
                    data << int32_t(unit->getField<uint32_t>(PowerFields[i]));
                if (mask[125 + i])
                    data << int32_t(unit->getField<uint32_t>(MaxPowerFields[i]));
                if (mask[131 + i])
                    data << unit->getField<float>(UnitField::PowerRegenFlatModifier, i);
                if (mask[137 + i])
                    data << unit->getField<float>(UnitField::PowerRegenInterruptedFlatModifier, i);
            }
        }

        if (mask[143])
        {
            for (uint8_t i = 0; i < 3; ++i)
            {
                if (!mask[144 + i])
                    continue;

                // the visible item with its own mask: every field
                int32_t itemId;
                uint16_t appearanceModId;
                uint16_t itemVisual;
                unitVisibleItem(unit, i, itemId, appearanceModId, itemVisual);
                data.writeBits(0xF, 4);
                data.flushBits();
                data << itemId << appearanceModId << itemVisual;
            }
        }

        if (mask[147])
        {
            for (uint8_t i = 0; i < 2; ++i)
                if (mask[148 + i])
                    data << uint32_t(unit->getBaseAttackTime(i));
        }

        if (mask[150])
        {
            for (uint8_t i = 0; i < 4; ++i)
            {
                if (mask[151 + i])
                    data << int32_t(unit->getStat(i));
                if (mask[155 + i])
                    data << int32_t(unit->getPosStat(i));
                if (mask[159 + i])
                    data << int32_t(unit->getNegStat(i));
            }
        }

        if (mask[163])
        {
            for (uint8_t i = 0; i < 7; ++i)
            {
                if (mask[164 + i])
                    data << int32_t(unit->getResistance(i));
                if (mask[171 + i])
                    data << int32_t(unit->getField<uint32_t>(UnitField::ResistanceBuffModPositive, i)) - int32_t(unit->getField<uint32_t>(UnitField::ResistanceBuffModNegative, i));
                if (mask[178 + i])
                    data << int32_t(unit->getPowerCostModifier(i));
                if (mask[185 + i])
                    data << float(unit->getPowerCostMultiplier(i));
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // PlayerData
    using PlayerMask = ChangeMask<192>;

    constexpr uint8_t QuestLogCount = 125;
    constexpr uint8_t VisibleItemCount = 19;

    // the quest of a log slot: id, state flags, end time, accept time, objective progress
    void playerQuestLogCreate(Player* player, uint8_t slot, ByteBuffer& data)
    {
        const QuestLogEntry* questLog = slot < MAX_QUEST_LOG_SIZE ? player->getQuestLogBySlotId(slot) : nullptr;
        data << int32_t(questLog != nullptr ? questLog->getQuestProperties()->id : 0);
        data << uint32_t(0);                                                // state flags
        data << uint32_t(0);                                                // end time
        data << uint32_t(0);                                                // accept time
        data << uint32_t(0);
        for (uint8_t i = 0; i < 24; ++i)
            data << int16_t(questLog != nullptr && i < 4 ? questLog->getMobCountByIndex(i) : 0);
    }

    void playerVisibleItemCreate(Player* player, uint8_t slot, ByteBuffer& data)
    {
        data << int32_t(player->getVisibleItemEntry(slot));
        data << uint16_t(0);                                                // appearance modifier
        data << uint16_t(player->getVisibleItemEnchantment(slot, 0));       // visual: the permanent enchantment
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
        data << uint8_t(player->getSkinColor());
        data << uint8_t(player->getFace());
        data << uint8_t(player->getHairStyle());
        data << uint8_t(player->getHairColor());
        data << uint8_t(0) << uint8_t(0) << uint8_t(0);                     // custom display
        data << uint8_t(player->getFacialFeatures());
        data << uint8_t(0);                                                 // party type
        data << uint8_t(player->getPlayerGender());
        data << uint8_t(player->getDrunkValue());
        data << uint8_t(player->getPvpRank());
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
        data << int32_t(player->getField<uint32_t>(PlayerField::Inebriation));
        data << uint32_t(player->getField<uint32_t>(PlayerField::VirtualPlayerRealm));
        data << uint32_t(player->getCurrentSpecId());
        data << int32_t(player->getField<uint32_t>(PlayerField::TaxiMountAnimKitId));
        for (uint8_t i = 0; i < 4; ++i)
            data << float(0.0f);                                            // average item level
        data << uint8_t(player->getField<uint8_t>(PlayerField::CurrentBattlePetBreedQuality));
        data << int32_t(0);                                                 // honor level
        data << uint32_t(0);                                                // arena cooldowns
        data << int32_t(0);
        data << int32_t(0);
        data << WoWGuid128();
        data << int32_t(0);

        if (ctx.partyMember())
            data.writeBit(false);                                           // has a quest session
        data.writeBit(false);                                               // has a level link
        data.flushBits();
    }

    void playerCollect(ChangedFields const& changed, PlayerMask& mask)
    {
        struct Single { uint8_t bit; PlayerField field; };
        static constexpr Single singles[] =
        {
            { 5, PlayerField::DuelArbiter }, { 8, PlayerField::PlayerFlags }, { 10, PlayerField::GuildRank }, { 11, PlayerField::GuildDeleteDate },
            { 12, PlayerField::GuildLevel }, { 23, PlayerField::DuelTeam }, { 24, PlayerField::GuildTimestamp }, { 25, PlayerField::ChosenTitle },
            { 26, PlayerField::Inebriation }, { 27, PlayerField::VirtualPlayerRealm }, { 28, PlayerField::CurrentSpecId }, { 29, PlayerField::TaxiMountAnimKitId },
            { 30, PlayerField::CurrentBattlePetBreedQuality }
        };

        for (const Single& single : singles)
            if (changed.changed(single.field))
                mask.set(single.bit);

        if (changed.changed(PlayerField::PlayerBytes))
            mask.setRange(13, 4);
        if (changed.changed(PlayerField::PlayerBytes2))
            mask.setRange(17, 2);
        if (changed.changed(PlayerField::PlayerBytes3))
            mask.setRange(19, 4);

        for (uint8_t i = 0; i < VisibleItemCount; ++i)
            if (changed.changed(PlayerField::VisibleItemsEntry, i) || changed.changed(PlayerField::VisibleItemsEnchantment, i))
                mask.set(168 + i);

        for (uint8_t bit = 1; bit < 32; ++bit)
            if (mask[bit])
                mask.set(0);
        for (uint8_t i = 0; i < VisibleItemCount; ++i)
            if (mask[168 + i])
                mask.set(167);
    }

    void playerFilter(PlayerMask& mask, uint8_t flags)
    {
        uint32_t allowed[6] = { 0xFFFFFFF5, 0x000001FF, 0, 0, 0, 0xFFFFFF80 };
        if (flags & FlagPartyMember)
        {
            const uint32_t party[6] = { 0x0000000A, 0xFFFFFE00, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x0000007F };
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
                writeDynamicMask(data, 0);                                  // quest session quest log
            if (mask[4])
                writeDynamicMask(data, 0);                                  // arena cooldowns
        }
        data.flushBits();

        if (mask[0])
        {
            if (mask[5])
                ctx.guid(player->getDuelArbiter());
            if (mask[6])
                data << WoWGuid128::global(HighGuid128::WowAccount, player->getSession() != nullptr ? player->getSession()->GetAccountId() : 0);
            if (mask[7])
                data << WoWGuid128();
            if (mask[8])
                data << uint32_t(player->getPlayerFlags());
            if (mask[9])
                data << uint32_t(0);
            if (mask[10])
                data << uint32_t(player->getGuildRank());
            if (mask[11])
                data << uint32_t(player->getField<uint32_t>(PlayerField::GuildDeleteDate));
            if (mask[12])
                data << int32_t(player->getField<uint32_t>(PlayerField::GuildLevel));
            if (mask[13])
                data << uint8_t(player->getSkinColor());
            if (mask[14])
                data << uint8_t(player->getFace());
            if (mask[15])
                data << uint8_t(player->getHairStyle());
            if (mask[16])
                data << uint8_t(player->getHairColor());
            if (mask[17])
                data << uint8_t(player->getFacialFeatures());
            if (mask[18])
                data << uint8_t(0);
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
                data << int32_t(player->getChosenTitle());
            if (mask[26])
                data << int32_t(player->getField<uint32_t>(PlayerField::Inebriation));
            if (mask[27])
                data << uint32_t(player->getField<uint32_t>(PlayerField::VirtualPlayerRealm));
            if (mask[28])
                data << uint32_t(player->getCurrentSpecId());
            if (mask[29])
                data << int32_t(player->getField<uint32_t>(PlayerField::TaxiMountAnimKitId));
            if (mask[30])
                data << uint8_t(player->getField<uint8_t>(PlayerField::CurrentBattlePetBreedQuality));
            if (mask[31])
                data << int32_t(0);
        }

        if (mask[32])
        {
            if (mask[33])
                data << int32_t(0);
            if (mask[34])
                data << int32_t(0);
            if (mask[35])
                data << WoWGuid128();
            if (mask[36])
                data << int32_t(0);
        }

        if (mask[37])
        {
            for (uint8_t i = 0; i < 3; ++i)
                if (mask[38 + i])
                    data << uint8_t(0);
        }

        if (mask[41])
        {
            for (uint8_t i = 0; i < QuestLogCount; ++i)
            {
                if (!mask[42 + i])
                    continue;

                // the quest log entry with its own mask: every field
                data.writeBits(1, 1);
                data.writeBits(0x7FFFFFFF, 32);
                data.flushBits();
                playerQuestLogCreate(player, i, data);
            }
        }

        if (mask[167])
        {
            for (uint8_t i = 0; i < VisibleItemCount; ++i)
            {
                if (!mask[168 + i])
                    continue;

                data.writeBits(0xF, 4);
                data.flushBits();
                playerVisibleItemCreate(player, i, data);
            }
        }

        if (mask[187])
        {
            for (uint8_t i = 0; i < 4; ++i)
                if (mask[188 + i])
                    data << float(0.0f);
        }

        data.flushBits();
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // ActivePlayerData: the player itself
    using ActivePlayerMask = ChangeMask<1494>;

    constexpr uint16_t InvSlotCount = 199;
    constexpr uint16_t ExploredZoneCount = 192;
    constexpr uint16_t SkillCount = 256;
    constexpr uint16_t LegacySkillCount = 128;
    constexpr uint16_t QuestCompletedCount = 875;
    constexpr uint8_t KnownTitleCount = 6;

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
        data << int32_t(player->getTrackCreature());
        data << uint32_t(player->getTrackResource());
        data << uint32_t(0);
        activeStatsCreate(player, data);

        for (uint16_t i = 0; i < ExploredZoneCount; ++i)
            data << activeExploredZone(player, i);

        // rest info: xp and honor
        data << uint32_t(player->getRestStateXp()) << uint8_t(player->getRestState());
        data << uint32_t(0) << uint8_t(0);

        for (uint8_t i = 0; i < 7; ++i)
        {
            data << int32_t(player->getModDamageDonePositive(i));
            data << int32_t(player->getModDamageDoneNegative(i));
            data << float(player->getModDamageDonePct(i));
        }

        data << int32_t(player->getModHealingDone());
        data << player->getField<float>(PlayerField::FieldModHealingPct);
        data << player->getField<float>(PlayerField::FieldModHealingDonePct);
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
        data << int32_t(0);                                                 // local flags
        data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytesRafLevel));
        data << uint8_t(player->getEnabledActionBars());
        data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytesMaxPvpRank));
        data << uint8_t(0);                                                 // respecs
        data << uint32_t(player->getField<uint32_t>(PlayerField::FieldPvpMedals));

        for (uint8_t i = 0; i < 12; ++i)
        {
            data << player->getField<uint32_t>(PlayerField::FieldBuyBackPrice, i);
            data << player->getField<uint32_t>(PlayerField::FieldBuyBackTimestamp, i);
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
        data << uint8_t(0);                                                 // local regen flags
        data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytes2AuraVision));
        data << uint8_t(16);                                                // backpack slots
        data << int32_t(player->getField<uint32_t>(PlayerField::OverrideSpellId));
        data << int32_t(player->getField<uint32_t>(PlayerField::LfgBonusFactionId));
        data << uint16_t(player->getField<uint16_t>(PlayerField::LootSpecId));
        data << uint32_t(player->getField<uint32_t>(PlayerField::OverrideZonePvpType));

        for (uint8_t i = 0; i < 4; ++i)
            data << uint32_t(0);                                            // bag slot flags
        for (uint8_t i = 0; i < 7; ++i)
            data << uint32_t(0);                                            // bank bag slot flags

        for (uint16_t i = 0; i < QuestCompletedCount; ++i)
            data << uint64_t(0);

        data << int32_t(0);                                                 // honor
        data << int32_t(0);                                                 // honor next level
        data << int32_t(0);                                                 // pvp reward achieved
        data << int32_t(0);                                                 // pvp tier max from wins
        data << int32_t(0);                                                 // pvp last weeks reward achieved
        data << int32_t(0);                                                 // pvp last weeks tier max from wins
        data << int32_t(0);                                                 // pvp last weeks reward claimed
        data << uint8_t(player->getBankSlots());

        // the dynamic fields: research sites, research site progress, daily quests, quest lines, heirlooms,
        // heirloom flags, toys, toy flags, transmog, conditional transmog, self res spells, character restrictions,
        // spell modifiers by label
        for (uint8_t i = 0; i < 14; ++i)
            data << uint32_t(0);
        data << uint32_t(0);                                                // research
        data << uint32_t(0);                                                // replayed quests
        data << uint32_t(0);                                                // disabled spells

        for (uint8_t i = 0; i < KnownTitleCount; ++i)
            data << uint64_t(player->getKnownTitles(i));

        // pvp info: six brackets
        for (uint8_t i = 0; i < 6; ++i)
        {
            for (uint8_t j = 0; j < 8; ++j)
                data << uint32_t(0);
            data.writeBits(0, 1);
            data.flushBits();
        }

        data.writeBit(false);                                               // backpack auto sort disabled
        data.writeBit(false);                                               // bank auto sort disabled
        data.writeBit(false);                                               // sort bags right to left
        data.writeBit(false);                                               // insert items left to right
        data.writeBit(false);                                               // has a quest session
        data.flushBits();
    }

    void activePlayerCollect(ChangedFields const& changed, ActivePlayerMask& mask)
    {
        struct Single { uint16_t bit; PlayerField field; };
        static constexpr Single singles[] =
        {
            { 24, PlayerField::FarsightGuid }, { 25, PlayerField::SummonedBattlePetGuid }, { 26, PlayerField::FieldCoinage }, { 27, PlayerField::Xp },
            { 28, PlayerField::NextLevelXp }, { 31, PlayerField::CharacterPoints1 }, { 32, PlayerField::MaxTalentTiers }, { 33, PlayerField::TrackCreatures },
            { 35, PlayerField::Expertise }, { 36, PlayerField::OffhandExpertise }, { 37, PlayerField::RangedExpertise }, { 38, PlayerField::CombatRatingExpertise },
            { 39, PlayerField::BlockPct }, { 40, PlayerField::DodgePct }, { 42, PlayerField::ParryPct }, { 44, PlayerField::CritPct },
            { 45, PlayerField::RangedCritPct }, { 46, PlayerField::OffhandCritPct }, { 47, PlayerField::SpellCritPct }, { 48, PlayerField::ShieldBlock },
            { 49, PlayerField::ShieldBlockCritPct }, { 50, PlayerField::Mastery }, { 56, PlayerField::PvpPowerDamage }, { 57, PlayerField::PvpPowerHealing },
            { 58, PlayerField::FieldModHealingDone }, { 59, PlayerField::FieldModHealingPct }, { 60, PlayerField::FieldModHealingDonePct },
            { 61, PlayerField::FieldModPeriodicHealingDonePct }, { 62, PlayerField::ModSpellPowerPct }, { 63, PlayerField::ModResiliencePct },
            { 64, PlayerField::OverrideSpellPowerByApPct }, { 65, PlayerField::OverrideApBySpellPowerPct }, { 67, PlayerField::FieldModTargetResistance },
            { 68, PlayerField::FieldModTargetPhysicalResistance }, { 74, PlayerField::FieldPvpMedals }, { 77, PlayerField::FieldLifetimeHonorableKills },
            { 78, PlayerField::FieldWatchedFactionIdx }, { 79, PlayerField::FieldMaxLevel }, { 82, PlayerField::PetSpellPower }, { 83, PlayerField::UiHitMod },
            { 84, PlayerField::UiHitSpellMod }, { 85, PlayerField::UiHomeRealmTimeOffset }, { 86, PlayerField::ModPetHaste }, { 90, PlayerField::OverrideSpellId },
            { 91, PlayerField::LfgBonusFactionId }, { 92, PlayerField::LootSpecId }, { 93, PlayerField::OverrideZonePvpType }
        };

        for (const Single& single : singles)
            if (changed.changed(single.field))
                mask.set(single.bit);

        if (changed.changed(PlayerField::FieldSkillInfo))
            mask.set(30);
        if (changed.changed(PlayerField::PlayerFieldBytes))
            mask.setRange(70, 3);
        if (changed.changed(PlayerField::FieldKills))
            mask.setRange(75, 2);
        if (changed.changed(PlayerField::PlayerFieldBytes2))
            mask.set(88);
        if (changed.changed(PlayerField::PlayerBytes2))
            mask.set(102);

        for (uint16_t i = 0; i < InvSlotCount; ++i)
            if (activeInvSlotChanged(changed, i))
                mask.set(105 + i);
        if (changed.changed(PlayerField::TrackResources))
            mask.set(305);
        for (uint16_t i = 0; i < ExploredZoneCount; ++i)
            if (changed.changed(PlayerField::ExploredZones, i * 2) || changed.changed(PlayerField::ExploredZones, i * 2 + 1))
                mask.set(308 + i);
        if (changed.changed(PlayerField::RestStateXp) || changed.changed(PlayerField::PlayerBytes2))
            mask.set(501);
        for (uint8_t i = 0; i < 7; ++i)
        {
            if (changed.changed(PlayerField::FieldModDamageDonePositive, i))
                mask.set(504 + i);
            if (changed.changed(PlayerField::FieldModDamageDoneNegative, i))
                mask.set(511 + i);
            if (changed.changed(PlayerField::FieldModDamageDonePct, i))
                mask.set(518 + i);
        }
        for (uint8_t i = 0; i < 3; ++i)
            if (changed.changed(PlayerField::WeaponDmgMultiplier, i))
                mask.set(526 + i);
        for (uint8_t i = 0; i < 12; ++i)
        {
            if (changed.changed(PlayerField::FieldBuyBackPrice, i))
                mask.set(533 + i);
            if (changed.changed(PlayerField::FieldBuyBackTimestamp, i))
                mask.set(545 + i);
        }
        for (uint8_t i = 0; i < 32; ++i)
            if (changed.changed(PlayerField::FieldCombatRating, i))
                mask.set(558 + i);
        for (uint8_t i = 0; i < 4; ++i)
            if (changed.changed(PlayerField::NoReagentCost, i))
                mask.set(598 + i);
        for (uint8_t i = 0; i < 2; ++i)
            if (changed.changed(PlayerField::ProfessionSkillLine, i))
                mask.set(603 + i);

        // the group bits
        static constexpr uint16_t groups[] = { 0, 34, 66, 98 };
        for (const uint16_t group : groups)
            for (uint16_t bit = group + 1; bit < group + 32; ++bit)
                if (mask[bit])
                    mask.set(group);
        struct Array { uint16_t parent; uint16_t first; uint16_t count; };
        static constexpr Array arrays[] =
        {
            { 104, 105, InvSlotCount }, { 304, 305, 2 }, { 307, 308, ExploredZoneCount }, { 500, 501, 2 }, { 503, 504, 21 },
            { 525, 526, 6 }, { 532, 533, 24 }, { 557, 558, 32 }, { 597, 598, 4 }, { 602, 603, 2 }
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
            for (uint8_t bit = 5; bit <= 19; ++bit)
                if (mask[bit])
                    writeDynamicMask(data, 0);
        }
        if (mask[22] && mask[23])
            writeDynamicMask(data, 0);
        data.flushBits();
        if (mask[0])
        {
            if (mask[20])
                writeDynamicMask(data, 0);
            if (mask[21])
                writeDynamicMask(data, 0);
        }
        data.flushBits();

        if (mask[0])
        {
            if (mask[24])
                ctx.guid(player->getFarsightGuid());
            if (mask[25])
                data << WoWGuid128();
            if (mask[26])
                data << uint64_t(player->getCoinage());
            if (mask[27])
                data << int32_t(player->getXp());
            if (mask[28])
                data << int32_t(player->getNextLevelXp());
            if (mask[29])
                data << int32_t(0);
            if (mask[30])
                activeSkillUpdate(player, data);
            if (mask[31])
                data << int32_t(player->getField<uint32_t>(PlayerField::CharacterPoints1));
            if (mask[32])
                data << int32_t(player->getField<uint32_t>(PlayerField::MaxTalentTiers));
            if (mask[33])
                data << int32_t(player->getTrackCreature());
        }

        if (mask[34])
        {
            // every stat of the block: the create order is the bit order
            ByteBuffer stats;
            activeStatsCreate(player, stats);
            size_t position = 0;
            for (uint16_t bit = 35; bit <= 57; ++bit)
            {
                if (mask[bit])
                    data.append(stats.contents() + position, 4);
                position += 4;
            }
            if (mask[58])
                data << int32_t(player->getModHealingDone());
            if (mask[59])
                data << player->getField<float>(PlayerField::FieldModHealingPct);
            if (mask[60])
                data << player->getField<float>(PlayerField::FieldModHealingDonePct);
            if (mask[61])
                data << player->getField<float>(PlayerField::FieldModPeriodicHealingDonePct);
            if (mask[62])
                data << player->getField<float>(PlayerField::ModSpellPowerPct);
            if (mask[63])
                data << player->getField<float>(PlayerField::ModResiliencePct);
            if (mask[64])
                data << player->getField<float>(PlayerField::OverrideSpellPowerByApPct);
            if (mask[65])
                data << player->getField<float>(PlayerField::OverrideApBySpellPowerPct);
        }

        if (mask[66])
        {
            if (mask[67])
                data << int32_t(player->getField<uint32_t>(PlayerField::FieldModTargetResistance));
            if (mask[68])
                data << int32_t(player->getField<uint32_t>(PlayerField::FieldModTargetPhysicalResistance));
            if (mask[69])
                data << int32_t(0);
            if (mask[70])
                data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytesRafLevel));
            if (mask[71])
                data << uint8_t(player->getEnabledActionBars());
            if (mask[72])
                data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytesMaxPvpRank));
            if (mask[73])
                data << uint8_t(0);
            if (mask[74])
                data << uint32_t(player->getField<uint32_t>(PlayerField::FieldPvpMedals));
            if (mask[75])
                data << player->getField<uint16_t>(PlayerField::FieldKillsKillsToday);
            if (mask[76])
                data << player->getField<uint16_t>(PlayerField::FieldKillsKillsYesterday);
            if (mask[77])
                data << player->getField<uint32_t>(PlayerField::FieldLifetimeHonorableKills);
            if (mask[78])
                data << int32_t(player->getWatchedFaction());
            if (mask[79])
                data << int32_t(player->getMaxLevel());
            if (mask[80])
                data << int32_t(0);
            if (mask[81])
                data << int32_t(0);
            if (mask[82])
                data << int32_t(player->getField<uint32_t>(PlayerField::PetSpellPower));
            if (mask[83])
                data << player->getField<float>(PlayerField::UiHitMod);
            if (mask[84])
                data << player->getField<float>(PlayerField::UiHitSpellMod);
            if (mask[85])
                data << int32_t(player->getField<uint32_t>(PlayerField::UiHomeRealmTimeOffset));
            if (mask[86])
                data << player->getField<float>(PlayerField::ModPetHaste);
            if (mask[87])
                data << uint8_t(0);
            if (mask[88])
                data << uint8_t(player->getField<uint8_t>(PlayerField::PlayerFieldBytes2AuraVision));
            if (mask[89])
                data << uint8_t(16);
            if (mask[90])
                data << int32_t(player->getField<uint32_t>(PlayerField::OverrideSpellId));
            if (mask[91])
                data << int32_t(player->getField<uint32_t>(PlayerField::LfgBonusFactionId));
            if (mask[92])
                data << uint16_t(player->getField<uint16_t>(PlayerField::LootSpecId));
            if (mask[93])
                data << uint32_t(player->getField<uint32_t>(PlayerField::OverrideZonePvpType));
            for (uint8_t bit = 94; bit <= 97; ++bit)
                if (mask[bit])
                    data << int32_t(0);
        }

        if (mask[98])
        {
            for (uint8_t bit = 99; bit <= 101; ++bit)
                if (mask[bit])
                    data << int32_t(0);
            if (mask[102])
                data << uint8_t(player->getBankSlots());
        }
        data.flushBits();
        if (mask[98])
        {
            data.writeBit(false);                                           // has a quest session
        }

        if (mask[104])
        {
            for (uint16_t i = 0; i < InvSlotCount; ++i)
                if (mask[105 + i])
                    ctx.guid(activeInvSlot(player, i));
        }
        if (mask[304])
        {
            if (mask[305])
                data << uint32_t(player->getTrackResource());
            if (mask[306])
                data << uint32_t(0);
        }
        if (mask[307])
        {
            for (uint16_t i = 0; i < ExploredZoneCount; ++i)
                if (mask[308 + i])
                    data << activeExploredZone(player, i);
        }
        if (mask[500])
        {
            if (mask[501])
            {
                data.writeBits(0x7, 3);
                data.flushBits();
                data << uint32_t(player->getRestStateXp()) << uint8_t(player->getRestState());
            }
            if (mask[502])
            {
                data.writeBits(0x7, 3);
                data.flushBits();
                data << uint32_t(0) << uint8_t(0);
            }
        }
        if (mask[503])
        {
            for (uint8_t i = 0; i < 7; ++i)
            {
                if (mask[504 + i])
                    data << int32_t(player->getModDamageDonePositive(i));
                if (mask[511 + i])
                    data << int32_t(player->getModDamageDoneNegative(i));
                if (mask[518 + i])
                    data << float(player->getModDamageDonePct(i));
            }
        }
        if (mask[525])
        {
            for (uint8_t i = 0; i < 3; ++i)
            {
                if (mask[526 + i])
                    data << player->getField<float>(PlayerField::WeaponDmgMultiplier, i);
                if (mask[529 + i])
                    data << float(1.0f);
            }
        }
        if (mask[532])
        {
            for (uint8_t i = 0; i < 12; ++i)
            {
                if (mask[533 + i])
                    data << player->getField<uint32_t>(PlayerField::FieldBuyBackPrice, i);
                if (mask[545 + i])
                    data << player->getField<uint32_t>(PlayerField::FieldBuyBackTimestamp, i);
            }
        }
        if (mask[557])
        {
            for (uint8_t i = 0; i < 32; ++i)
                if (mask[558 + i])
                    data << int32_t(player->getCombatRating(i));
        }
        if (mask[597])
        {
            for (uint8_t i = 0; i < 4; ++i)
                if (mask[598 + i])
                    data << player->getField<uint32_t>(PlayerField::NoReagentCost, i);
        }
        if (mask[602])
        {
            for (uint8_t i = 0; i < 2; ++i)
                if (mask[603 + i])
                    data << int32_t(player->getProfessionSkillLine(i));
        }
        data.flushBits();
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // ItemData
    using ItemMask = ChangeMask<40>;

    constexpr uint8_t EnchantmentCount = 13;

    void itemEnchantmentCreate(Item* item, uint8_t index, ByteBuffer& data)
    {
        data << int32_t(item->getEnchantmentId(index));
        data << uint32_t(item->getEnchantmentDuration(index));
        data << int16_t(item->getEnchantmentCharges(index));
        data << uint16_t(0);                                                // inactive
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
        if (ctx.owner())
            data << uint32_t(item->getField<uint32_t>(ItemField::ModifierMask));
        data << int32_t(0);                                                 // context
        if (ctx.owner())
        {
            data << uint64_t(0);                                            // artifact xp
            data << uint8_t(0);                                             // item appearance modifier
        }
        data << uint32_t(0);                                                // modifiers
        data << uint32_t(0);                                                // artifact powers
        data << uint32_t(0);                                                // gems
        if (ctx.owner())
            data << uint32_t(0);                                            // dynamic flags 2
    }

    void itemCollect(ChangedFields const& changed, ItemMask& mask)
    {
        struct Single { uint8_t bit; ItemField field; };
        static constexpr Single singles[] =
        {
            { 5, ItemField::OwnerGuid }, { 6, ItemField::ContainerGuid }, { 7, ItemField::CreatorGuid }, { 8, ItemField::GiftCreatorGuid },
            { 9, ItemField::StackCount }, { 10, ItemField::Duration }, { 11, ItemField::Flags }, { 12, ItemField::Durability },
            { 13, ItemField::MaxDurability }, { 14, ItemField::CreatePlayedTime }, { 15, ItemField::ModifierMask }
        };
        for (const Single& single : singles)
            if (changed.changed(single.field))
                mask.set(single.bit);
        for (uint8_t i = 0; i < 5; ++i)
            if (changed.changed(ItemField::SpellCharges, i))
                mask.set(21 + i);
        for (uint8_t i = 0; i < EnchantmentCount; ++i)
            if (changed.changed(ItemField::Enchantment, i))
                mask.set(27 + i);

        for (uint8_t bit = 1; bit < 20; ++bit)
            if (mask[bit])
                mask.set(0);
        for (uint8_t i = 0; i < 5; ++i)
            if (mask[21 + i])
                mask.set(20);
        for (uint8_t i = 0; i < EnchantmentCount; ++i)
            if (mask[27 + i])
                mask.set(26);
    }

    void itemFilter(ItemMask& mask, uint8_t flags)
    {
        uint32_t allowed[2] = { 0xFC0149FF, 0x000000FF };
        if (flags & FlagOwner)
            allowed[0] |= 0x03FEB600;
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
            for (uint8_t bit = 2; bit <= 4; ++bit)
                if (mask[bit])
                    writeDynamicMask(data, 0);
        }
        data.flushBits();

        if (mask[0])
        {
            if (mask[5])
                ctx.guid(item->getOwnerGuid());
            if (mask[6])
                ctx.guid(item->getContainerGuid());
            if (mask[7])
                ctx.guid(item->getCreatorGuid());
            if (mask[8])
                ctx.guid(item->getGiftCreatorGuid());
            if (mask[9])
                data << uint32_t(item->getStackCount());
            if (mask[10])
                data << uint32_t(item->getDuration());
            if (mask[11])
                data << uint32_t(item->getFlags());
            if (mask[12])
                data << uint32_t(item->getDurability());
            if (mask[13])
                data << uint32_t(item->getMaxDurability());
            if (mask[14])
                data << uint32_t(item->getCreatePlayedTime());
            if (mask[15])
                data << uint32_t(item->getField<uint32_t>(ItemField::ModifierMask));
            if (mask[16])
                data << int32_t(0);
            if (mask[17])
                data << uint64_t(0);
            if (mask[18])
                data << uint8_t(0);
            if (mask[19])
                data << uint32_t(0);
        }
        if (mask[20])
        {
            for (uint8_t i = 0; i < 5; ++i)
                if (mask[21 + i])
                    data << int32_t(item->getSpellCharges(i));
        }
        if (mask[26])
        {
            for (uint8_t i = 0; i < EnchantmentCount; ++i)
            {
                if (!mask[27 + i])
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
    using GameObjectMask = ChangeMask<20>;

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
        data << int32_t(gameobject->getLevel());
        data << int8_t(gameobject->getState());
        data << int8_t(gameobject->getGoType());
        data << uint8_t(gameobject->getField<uint8_t>(GameObjectField::Bytes1Health));
        data << uint32_t(gameobject->getArtKit());
        data << uint32_t(0);                                                // doodad sets
        data << uint32_t(0);                                                // custom parameter
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
        if (changed.changed(GameObjectField::Level))
            mask.set(14);
        if (changed.changed(GameObjectField::Bytes1))
            mask.setRange(15, 4);
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
                data << int32_t(gameobject->getLevel());
            if (mask[15])
                data << int8_t(gameobject->getState());
            if (mask[16])
                data << int8_t(gameobject->getGoType());
            if (mask[17])
                data << uint8_t(gameobject->getField<uint8_t>(GameObjectField::Bytes1Health));
            if (mask[18])
                data << uint32_t(gameobject->getArtKit());
            if (mask[19])
                data << uint32_t(0);
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // DynamicObjectData
    using DynamicObjectMask = ChangeMask<7>;

    void dynamicObjectCreate(DynamicObject* dynamicObject, Context const& ctx)
    {
        ctx.guid(dynamicObject->getCasterGuid());
        ctx.data << int32_t(0);                                             // spell visual
        ctx.data << int32_t(dynamicObject->getSpellId());
        ctx.data << float(dynamicObject->getRadius());
        ctx.data << uint32_t(dynamicObject->getCastTime());
        ctx.data << uint8_t(dynamicObject->getDynamicType());
    }

    void dynamicObjectCollect(ChangedFields const& changed, DynamicObjectMask& mask)
    {
        if (changed.changed(DynamicObjectField::CasterGuid))
            mask.set(1);
        if (changed.changed(DynamicObjectField::SpellId))
            mask.set(3);
        if (changed.changed(DynamicObjectField::Radius))
            mask.set(4);
        if (changed.changed(DynamicObjectField::CastTime))
            mask.set(5);
        if (changed.changed(DynamicObjectField::DynamicobjectBytes))
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
                ctx.data << int32_t(0);
            if (mask[3])
                ctx.data << int32_t(dynamicObject->getSpellId());
            if (mask[4])
                ctx.data << float(dynamicObject->getRadius());
            if (mask[5])
                ctx.data << uint32_t(dynamicObject->getCastTime());
            if (mask[6])
                ctx.data << uint8_t(dynamicObject->getDynamicType());
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // CorpseData
    using CorpseMask = ChangeMask<40>;

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
        data << uint8_t(0);
        data << uint8_t(corpse->getRace());
        data << uint8_t(corpse->getGender());
        data << uint8_t(corpse->getSkinColor());
        data << uint8_t(corpse->getFace());
        data << uint8_t(corpse->getHairStyle());
        data << uint8_t(corpse->getHairColor());
        data << uint8_t(corpse->getFacialFeatures());
        data << uint32_t(corpse->getFlags());
        data << int32_t(0);                                                 // faction template
        data << uint8_t(0) << uint8_t(0) << uint8_t(0);                     // custom display
    }

    void corpseCollect(ChangedFields const& changed, CorpseMask& mask)
    {
        if (changed.changed(CorpseField::DynamicFlags))
            mask.set(1);
        if (changed.changed(CorpseField::OwnerGuid))
            mask.set(2);
        if (changed.changed(CorpseField::PartyGuid))
            mask.set(3);
        if (changed.changed(CorpseField::Guild))
            mask.set(4);
        if (changed.changed(CorpseField::DisplayId))
            mask.set(5);
        if (changed.changed(CorpseField::CorpseBytes1))
            mask.setRange(7, 3);
        if (changed.changed(CorpseField::CorpseBytes2))
            mask.setRange(10, 4);
        if (changed.changed(CorpseField::CorpseFlags))
            mask.set(14);
        for (uint8_t i = 0; i < CorpseItemCount; ++i)
            if (changed.changed(CorpseField::Item, i))
                mask.set(17 + i);

        for (uint8_t bit = 1; bit < 16; ++bit)
            if (mask[bit])
                mask.set(0);
        for (uint8_t i = 0; i < CorpseItemCount; ++i)
            if (mask[17 + i])
                mask.set(16);
    }

    void corpseUpdate(Corpse* corpse, Context const& ctx, CorpseMask const& mask)
    {
        ByteBuffer& data = ctx.data;

        mask.writeBlocks(data);
        data.flushBits();
        if (mask[0])
        {
            if (mask[1])
                data << uint32_t(viewerDynamicFlags(corpse, ctx.target));
            if (mask[2])
                ctx.guid(corpse->getOwnerGuid());
            if (mask[3])
                ctx.guid(corpse->getField<uint64_t>(CorpseField::PartyGuid));
            if (mask[4])
                ctx.guildGuid(corpse->getField<uint32_t>(CorpseField::Guild));
            if (mask[5])
                data << uint32_t(corpse->getDisplayId());
            if (mask[6])
                data << uint8_t(0);
            if (mask[7])
                data << uint8_t(corpse->getRace());
            if (mask[8])
                data << uint8_t(corpse->getGender());
            if (mask[9])
                data << uint8_t(corpse->getSkinColor());
            if (mask[10])
                data << uint8_t(corpse->getFace());
            if (mask[11])
                data << uint8_t(corpse->getHairStyle());
            if (mask[12])
                data << uint8_t(corpse->getHairColor());
            if (mask[13])
                data << uint8_t(corpse->getFacialFeatures());
            if (mask[14])
                data << uint32_t(corpse->getFlags());
            if (mask[15])
                data << int32_t(0);
        }
        if (mask[16])
        {
            for (uint8_t i = 0; i < CorpseItemCount; ++i)
                if (mask[17 + i])
                    data << uint32_t(corpse->getItem(i));
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // AreaTriggerData: the caster, the duration and the spell
    void areaTriggerCreate(Object* object, Context const& ctx)
    {
        ByteBuffer& data = ctx.data;

        // override scale curve: start time offset, two points, parameter curve, override active
        data << uint32_t(0) << float(0) << float(0) << float(0) << float(0) << uint32_t(0);
        data.writeBit(false);
        data.flushBits();
        ctx.guid(object->getField<uint64_t>(AreaTriggerField::CasterGuid));
        data << uint32_t(object->getField<uint32_t>(AreaTriggerField::Duration));
        data << uint32_t(0);                                                // time to target
        data << uint32_t(0);                                                // time to target scale
        data << uint32_t(0);                                                // time to target extra scale
        data << int32_t(object->getField<uint32_t>(AreaTriggerField::SpellId));
        data << int32_t(object->getField<uint32_t>(AreaTriggerField::SpellId));
        data << int32_t(object->getField<uint32_t>(AreaTriggerField::SpellVisualId));
        data << float(object->getField<float>(AreaTriggerField::Scale));
        data << uint32_t(0);                                                // decal properties
        data << WoWGuid128();                                               // creating effect
        // extra scale curve
        data << uint32_t(0) << float(0) << float(0) << float(0) << float(0) << uint32_t(0);
        data.writeBit(false);
        data.flushBits();
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    Context makeContext(Object* object, Player* target, ByteBuffer& data)
    {
        return Context{ data, target, worldConfig.battleNetComm.realmId, target != nullptr ? target->GetMapId() : object->GetMapId(), fieldFlags(object, target) };
    }
}

namespace ObjectUpdateBfA
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
