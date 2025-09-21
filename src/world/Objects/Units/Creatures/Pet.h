/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#ifndef PET_H
#define PET_H

#include "PetDefines.hpp"
#include "Summons/Summon.hpp"
#include "Utilities/utf8String.hpp"
#include "Macros/PetMacros.hpp"

// MIT START

struct AI_Spell;
class CreatureAISpells;
struct PetCache;
enum SpellCastResult : uint8_t;

namespace WDB::Structures
{
    struct SummonPropertiesEntry;
}

using PetSpellMap = std::unordered_map<uint32_t, uint8_t>;

class SERVER_DECL Pet : public Summon
{
public:
    Pet(uint64_t guid, WDB::Structures::SummonPropertiesEntry const* properties);
    ~Pet();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Essential functions

    void Update(unsigned long /*time_passed*/) override;    // overrides function Summon::Update
    // void AddToWorld();                                   // not used
    // void AddToWorld(WorldMap* pMapMgr);                  // not used
    // void PushToWorld(WorldMap*);                         // not used
    // void RemoveFromWorld(bool free_guid);                // not used
    // void OnPrePushToWorld();                             // not used
    void OnPushToWorld() override;                          // overrides function Summon::OnPushToWorld
    // void OnPreRemoveFromWorld();                         // not used
    // void OnRemoveFromWorld();                            // not used

    // Override superclass method that returns false
    bool isPet() const override { return true; }

    // Override from Creature class
    void PrepareForRemove() override;
    // Override from Creature class
    void SafeDelete() override;

    // Override from Creature class
    void sendSpellsToController(Unit* controller, uint32_t duration) override;

    // TODO: make it virtual -Appled
    Group* getGroup();

    // Override from Creature class
    void setDeathState(DeathState s) override;

    //////////////////////////////////////////////////////////////////////////////////////////
    // General functions

    // Override from Summon class
    // NOTE: Pet class should not call this directly, use createAsSummon instead
    void load(CreatureProperties const* creatureProperties, Unit* unitOwner, LocationVector const& position, uint32_t duration, uint32_t spellId) override;

    // Function excepts there is NO existing summon in same summon slot
    // Returns false if an error occured. The caller MUST delete us.
    bool createAsSummon(CreatureProperties const* creatureProperties, Creature* createdFromCreature, Unit* unitOwner, LocationVector const& position, uint32_t duration, SpellInfo const* createdBySpell, uint8_t effIndex, PetTypes type);
    // Function excepts there is NO existing summon in same summon slot
    // Returns false if an error occured. The caller MUST delete us.
    bool loadFromDB(Player* owner, PetCache const* petCache);

    // Updates player pet cache in player class i.e. before save to db
    void updatePetInfo(bool setPetOffline) const;

    // Permanently removes pet from player
    void abandonPet();
    // Unsummons pet but does not set it offline and keeps it active
    void unSummonTemporarily();
    // Override from Summon class
    void unSummon() override;

    // Refers to PetCache::number
    uint8_t getPetId() const;

    void rename(utf8_string const& newName);
    utf8_string const& getName() const;

    bool isHunterPet() const;

    void giveXp(uint32_t xp);
    bool canGainXp() const;
    uint32_t getNextLevelXp(uint32_t level) const;

    void applyStatsForLevel();

    void setPetAction(PetCommands action);
    PetCommands getPetAction() const;

    // Hunter pet related
#if VERSION_STRING < Cata
    HappinessStates getHappinessState() const;
    float_t getHappinessDamageMod() const;
#endif
    uint8_t getPetDiet() const;

private:
    // Prepares pet and pushes it to world
    bool _preparePetForPush(PetCache const* petCache);

    void _setNameForEntry(uint32_t entry, SpellInfo const* createdBySpell);
    void _setPetDiet();

    utf8_string m_petName;
    // Refers to PetCache::number
    uint8_t m_petId = 0;
    PetTypes m_petType = PET_TYPE_NONE;
    PetCommands m_petAction = PET_ACTION_FOLLOW;
    // If pet has duration
    bool m_petExpires = false;

    bool m_isScheduledForDeletion = false;
    bool m_isScheduledForTemporaryUnsummon = false;

    // Hunter pet related
#if VERSION_STRING < Cata
    uint16_t m_happinessTimer = PET_HAPPINESS_UPDATE_TIMER;
#endif
    uint8_t m_petDiet = PET_FOOD_NONE;

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Spells

    PetSpellMap const& getSpellMap() const;

    bool hasSpell(uint32_t spellId) const;
    void addSpell(SpellInfo const* spellInfo, bool silently = false);
    bool removeSpell(uint32_t spellId, bool silently = false);
    SpellCastResult canLearnSpell(SpellInfo const* spellInfo) const;

    void updateSpellList(bool onSummon = false);

    uint8_t getSpellState(uint32_t spellId) const;
    void setSpellState(uint32_t spellId, uint8_t state, std::optional<uint32_t> actionSlotId = std::nullopt);

    void setActionBarSlot(uint8_t slot, uint32_t spellId, uint8_t state);

#if VERSION_STRING == WotLK || VERSION_STRING == Cata
    uint8_t getTotalTalentPoints() const;
    uint8_t getSpentTalentPoints() const;
#endif

#if VERSION_STRING < Mop
    // Talent reset cost in wotlk and cata
    // Beast training reset cost in classic and tbc
    uint32_t getUntrainCost();
#endif

private:
    PetSpellMap mSpells;

    void _addSpell(SpellInfo const* spellInfo, bool silently, uint8_t spellState);
    // Returns true/false if removed spell and its spell state
    std::tuple<bool, uint8_t> _removeSpell(uint32_t spellId, bool silently = false, bool removingPreviousRanks = false);

    CreatureAISpells* _createAISpell(SpellInfo const* spellInfo);
    void _removeAISpell(uint32_t spellId);

    void _setDefaultActionBar();
    std::array<PetActionButtonData, PET_MAX_ACTION_BAR_SLOT> m_actionBar = { 0 };

#if VERSION_STRING < Mop
    time_t m_talentResetTime = 0;
    uint32_t m_talentResetCost = 0;
#endif

public:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Packets

    void sendActionFeedback(PetActionFeedback feedback);
    void sendPetCastFailed(uint32_t spellId, uint8_t reason);

public:
    // MIT END

        void ApplySummonLevelAbilities();
        void ApplyPetLevelAbilities();
        void UpdateAP();
        void LoadPetAuras(int32_t id);

        void WipeTalents();

#if VERSION_STRING == WotLK || VERSION_STRING == Cata
        // talents
        void SendTalentsToOwner();                                                                              // Send talentpoints and talent spells to owner
#endif

        void HandleAutoCastEvent(AutoCastEvents Type);
        AI_Spell* HandleAutoCastEvent();

        void die(Unit* pAttacker, uint32_t damage, uint32_t spellid);

    protected:
        uint32_t GetAutoCastTypeForSpell(SpellInfo const* ent);

    std::list<AI_Spell*> m_autoCastSpells[AUTOCAST_EVENT_COUNT];
};

#endif // _PET_H
