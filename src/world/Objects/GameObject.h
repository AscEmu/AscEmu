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

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Object.hpp"
#include "Data/WoWGameObject.hpp"
#include "GameObjectDefines.hpp"
#include "Management/Loot/Loot.hpp"
#include "Management/Loot/LootDefines.hpp"
#include "Server/UpdateFieldInclude.h"

namespace MySQLStructure
{
    struct GameobjectSpawn;
}

struct QuestProperties;
struct GameObjectProperties;
struct GameObjectValue;
enum LootState : uint8_t;
class Player;
class GameObjectAIScript;
class GameObjectModel;
class GameEvent;
struct QuestRelation;

class SERVER_DECL GameObject : public Object
{
    // MIT Start
public:
    GameObject(uint64_t guid);
    ~GameObject();

    bool loadFromDB(MySQLStructure::GameobjectSpawn* spawn, WorldMap* map, bool addToWorld);
    void saveToDB(bool newSpawn = false);
    void deleteFromDB();
    bool create(uint32_t entry, WorldMap* map, uint32_t phase, LocationVector const& position, QuaternionData const&  rotation, GameObject_State state, uint32_t spawnId = 0);

    uint32_t getSpawnId() const { return m_spawnId; }
    void setSpawnId(uint32_t spawnId) { m_spawnId = spawnId; }

    void despawn(uint32_t delay /*milliseconds*/, uint32_t forceRespawntime /*seconds*/);
    void expireAndDelete();
    void RemoveFromWorld(bool free_guid);

    void setRespawnTime(int32_t respawn);
    time_t getRespawnTime() const { return m_respawnTime; }
    void saveRespawnTime(uint32_t forceDelay = 0);
    void respawn();
    uint32_t getRespawnDelay() const { return m_respawnDelayTime; }
    bool isSpawned() const
    {
        return m_respawnDelayTime == 0 ||
            (m_respawnTime > 0 && !m_spawnedByDefault) ||
            (m_respawnTime == 0 && m_spawnedByDefault);
    }

    bool isSpawnedByDefault() const { return m_spawnedByDefault; }
    void setSpawnedByDefault(bool b) { m_spawnedByDefault = b; }

    void setSpellId(uint32_t id)
    {
        m_spawnedByDefault = false;                     // all summoned object is despawned after delay
        m_spellId = id;
    }
    uint32_t getSpellId() const { return m_spellId; }

    GameObject* getLinkedTrap();
    void setLinkedTrap(GameObject* linkedTrap) { m_linkedTrap = linkedTrap->getGuid(); }

    void addUniqueUse(Player* player);
    void addUse() { ++m_usetimes; }
    uint32_t getUseCount() const { return m_usetimes; }
    uint32_t getUniqueUseCount() const { return static_cast<uint32_t>(m_unique_users.size()); }
    bool isUseable();
    void switchDoorOrButton(bool activate, bool alternative = false);
    void useDoorOrButton(uint32_t time_to_restore = 0, bool alternative = false, Unit* user = nullptr);
    void resetDoorOrButton();
    void triggerLinkedGameObject(uint32_t trapEntry, Unit* target);

    void setOwnerGuid(uint64_t owner);

    LootState getLootState() const;
    void setLootState(LootState state, Unit* unit = nullptr);

    void setLocalRotationAngles(float z_rot, float y_rot, float x_rot);
    void setLocalRotation(float qx, float qy, float qz, float qw);
    void setParentRotation(QuaternionData const& rotation);
    QuaternionData const& getLocalRotation() const;
    int64_t getPackedLocalRotation() const;
    QuaternionData getWorldRotation() const;

    void enableCollision(bool enable);
    std::unique_ptr<GameObjectModel> m_model;

    Transporter* ToTransport();
    Transporter const* ToTransport() const;
    void updateModelPosition();

    MySQLStructure::GameobjectSpawn* m_spawn = nullptr;

    GameObjectValue const* getGOValue() const;

private:
    void updatePackedRotation();

    //////////////////////////////////////////////////////////////////////////////////////////
    // WoWData
    WoWGameObject* gameObjectData() const { return reinterpret_cast<WoWGameObject*>(wow_data); }

public:
    uint64_t getCreatedByGuid() const;
    void setCreatedByGuid(uint64_t guid);

    uint32_t getDisplayId() const;
    void setDisplayId(uint32_t id);

    uint32_t getFlags() const;
    void setFlags(uint32_t flags);
    void addFlags(uint32_t flags);
    void removeFlags(uint32_t flags);
    bool hasFlags(uint32_t flags) const;

    float getParentRotation(uint8_t type) const;

#if VERSION_STRING < WotLK
    uint32_t getDynamicFlags() const;
    void setDynamicFlags(uint32_t dynamicFlags);
#elif VERSION_STRING < Mop
    uint16_t getDynamicFlags() const;
    int16_t getDynamicPathProgress() const;
    void setDynamicFlags(uint16_t dynamicFlags);
    void setDynamicPathProgress(int16_t pathProgress);
#endif

    uint32_t getFactionTemplate() const;
    void setFactionTemplate(uint32_t id);

    uint32_t getLevel() const;
    void setLevel(uint32_t level);

    //bytes1
    uint8_t getState() const;
    void setState(uint8_t state);

    uint8_t getGoType() const;
    void setGoType(uint8_t type);

    uint8_t getArtKit() const;
    void setArtKit(uint8_t artkit);

    uint8_t getAnimationProgress() const;
    void setAnimationProgress(uint8_t progress);

    virtual uint32_t getTransportPeriod() const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Type helper
    bool isQuestGiver() const;
    bool isFishingNode() const;

    // Trap objects
    bool invisible = false;
    bool inStealth = false;
    uint8_t invisibilityFlag = INVIS_FLAG_NORMAL;
    uint8_t stealthFlag = STEALTH_FLAG_NORMAL;

    // Returns unit owner
    Unit* getUnitOwner() override;
    // Returns unit owner
    Unit const* getUnitOwner() const override;
    // Returns player owner
    Player* getPlayerOwner() override;
    // Returns player owner
    Player const* getPlayerOwner() const override;

    // MIT End

    GameEvent* mEvent = nullptr;

    GameObjectProperties const* GetGameObjectProperties() const;
        void SetGameObjectProperties(GameObjectProperties const* go_prop);

        virtual bool IsLootable() { return false; }

        virtual void Use(uint64_t /*GUID*/) {}
        void CastSpell(uint64_t TargetGUID, SpellInfo const* sp);
        void CastSpell(uint64_t TargetGUID, uint32_t SpellID);

        void Update(unsigned long time_passed);

        //void _EnvironmentalDamageUpdate();
        // Serialization
        void SaveToFile(std::stringstream & name);

        virtual void InitAI();

        void CallScriptUpdate();

        GameObjectAIScript* GetScript();

        void OnPushToWorld();
        void onRemoveInRangeObject(Object* pObj);

        uint32_t GetGOReqSkill();
        MapCell* m_respawnCell = nullptr;

        void SetOverrides(uint32_t go_overrides) { m_overrides = go_overrides; }
        uint32_t GetOverrides() { return m_overrides; }

        //\todo serverdie faction can be handled in update.
        void SetFaction(uint32_t id)
        {
            setFactionTemplate(id);
            setServersideFaction();
        }

    protected:
        bool m_summonedGo = false;
        bool m_deleted = false;
        GameObjectProperties const* gameobject_properties = nullptr;

        GameObjectAIScript* myScript = nullptr;
        uint32_t _fields[getSizeOfStructure(WoWGameObject)];

        uint32_t m_overrides = 0;             //See enum GAMEOBJECT_OVERRIDES!

    //MIT
    public:
        void sendGameobjectCustomAnim(uint32_t anim = 0);
        virtual void onUse(Player* /*player*/);

    protected:
        std::unique_ptr<GameObjectModel> createModel();
        void updateModel();

        virtual void _internalUpdateOnState(unsigned long timeDiff);

        uint32_t m_spellId = 0;

        uint32_t m_spawnId = 0;                 // temporary GameObjects have 0
        time_t m_respawnTime = 0;               // seconds
        uint32_t m_respawnDelayTime = 300;      // seconds
        uint32_t m_despawnDelay = 0;            // milliseconds
        uint32_t m_despawnRespawnTime = 0;      // seconds
        LootState m_lootState = GO_NOT_READY;
        uint64_t m_lootStateUnitGUID = 0;
        bool m_spawnedByDefault = true;
        time_t m_cooldownTime = 0;              // milliseconds
        GameObject_State m_prevGoState = GO_STATE_OPEN; // What state to set whenever resetting

        int64_t m_packedRotation = 0;
        QuaternionData m_localRotation;
        GameObjectValue m_goValue;

        uint64_t m_linkedTrap = 0;

        std::set<uint64_t> m_unique_users;
        uint32_t m_usetimes = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Abstract Base Class for lootables (fishing node, fishing hole, and chests)
class GameObject_Lootable : public GameObject
{
public:
    GameObject_Lootable(uint64_t GUID) : GameObject(GUID) {}
    ~GameObject_Lootable() {}

    virtual bool HasLoot() = 0;

    uint16_t getLootMode() const;
    bool hasLootMode(uint16_t lootMode) const;
    void setLootMode(uint16_t lootMode);
    void addLootMode(uint16_t lootMode);
    void removeLootMode(uint16_t lootMode);
    void resetLootMode();
    void setLootGenerationTime();
    uint32_t getLootGenerationTime() const;

    time_t getRestockTime() const;
    void setRestockTime(time_t time);

    void getFishLoot(Player* loot_owner, bool getJunk = false);

    Loot loot;

protected:
    time_t m_restockTime = 0;                // seconds

    uint16_t m_LootMode = LOOT_MODE_DEFAULT; // bitmask, determines what loot will be lootable used for Hardmodes example Ulduar Bosses
    uint32_t m_lootGenerationTime = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Implements Type 0 (DOOR) GameObjects
class GameObject_Door : public GameObject
{
public:
    GameObject_Door(uint64_t GUID);
    ~GameObject_Door();

    void InitAI();

    void onUse(Player* player) override;

protected:
    void _internalUpdateOnState(unsigned long timeDiff) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Implements Type 1 (BUTTON) GameObjects
class GameObject_Button : public GameObject
{
public:
    GameObject_Button(uint64_t GUID);
    ~GameObject_Button();

    void InitAI();

    void onUse(Player* player) override;

protected:
    void _internalUpdateOnState(unsigned long timeDiff) override;

private:
    SpellInfo const* spell;
};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing Type 2 (QUESTGIVER) GameObjects
class GameObject_QuestGiver : public GameObject
{
public:
    GameObject_QuestGiver(uint64_t GUID);
    ~GameObject_QuestGiver();

    void InitAI();

    bool HasQuests();

    void onUse(Player* player) override;

    uint32_t NumOfQuests();

    void AddQuest(std::unique_ptr<QuestRelation> Q);

    void DeleteQuest(QuestRelation const* Q);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Searches for a QuestRelation in the GO and if found, returns the Quest
    // \param uint32_t quest_id  -  Identifier of the Quest
    // \param uint8_t quest_relation  -  QuestRelation type
    // \return the Quest on success NULL on failure
    QuestProperties const* FindQuest(uint32_t quest_id, uint8_t quest_relation);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Finds the Quest with quest_id in the GO, and returns it's QuestRelation type
    // \param uint32_t quest_id  -  Identifier of the Quest
    // \return Returns the QuestRelation type on success, 0 on failure
    uint16_t GetQuestRelation(uint32_t quest_id);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Returns an iterator to the GO's QuestRelation list beginning
    // \return an iterator to the QuestRelation list's beginning
    std::list<std::unique_ptr<QuestRelation>>::iterator QuestsBegin();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Returns an iterator to the GO's QuestRelation list end
    // \return an iterator to the QuestRelation list's end
    std::list<std::unique_ptr<QuestRelation>>::iterator QuestsEnd();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Initializes the QuestRelation list with another
    // \param std::list< std::unique_ptr<QuestRelation> >* qst_lst  -  pointer to the other list
    void SetQuestList(std::list<std::unique_ptr<QuestRelation>>* qst_lst);

    std::list<std::unique_ptr<QuestRelation>>& getQuestList() const;

private:
    //////////////////////////////////////////////////////////////////////////////////////////
    // Loads the QuestRelations from QuestMgr for this GO
    void LoadQuests();

    std::list<std::unique_ptr<QuestRelation>>* m_quests;
};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing type 3 (CHEST) GameObjects
class GameObject_Chest : public GameObject_Lootable
{
public:
    GameObject_Chest(uint64_t GUID);
    ~GameObject_Chest();

    void InitAI();

    bool IsLootable() { return true; }
    bool HasLoot();

    void onUse(Player* player) override;

    void Open();
    void Close();

protected:
    void _internalUpdateOnState(unsigned long timeDiff) override;

private:
    SpellInfo const* spell;
};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing Type 6 (TRAP) GameObjects
class GameObject_Trap : public GameObject
{
public:
    GameObject_Trap(uint64_t GUID);
    ~GameObject_Trap();

    void InitAI();

    void onUse(Player* player) override;

protected:
    void _internalUpdateOnState(unsigned long timeDiff) override;

private:
    SpellInfo const* spell;
};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing Type 7 (GAMEOBJECT_TYPE_CHAIR) GameObjects
class GameObject_Chair : public GameObject
{
public:
    GameObject_Chair(uint64_t GUID) : GameObject(GUID){}
    ~GameObject_Chair(){};

    void onUse(Player* player) override;

};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing Type 8 (SPELL_FOCUS) GameObjects
class GameObject_SpellFocus : public GameObject
{
public:
    GameObject_SpellFocus(uint64_t GUID);
    ~GameObject_SpellFocus();

    void OnPushToWorld();
};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing Type 10 (GOOBER) GameObjects
class GameObject_Goober : public GameObject
{
public:
    GameObject_Goober(uint64_t GUID);
    ~GameObject_Goober();

    void InitAI();

    void onUse(Player* player) override;

protected:
    void _internalUpdateOnState(unsigned long timeDiff) override;

private:
    SpellInfo const* spell;
};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing Type 11 (TRANSPORT) GameObjects
class GameObject_Transport : public GameObject
{
public:
    GameObject_Transport(uint64_t guid) : GameObject(guid) {}
    ~GameObject_Transport() {}

protected:
    void _internalUpdateOnState(unsigned long timeDiff) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing Type 13 (CAMERA) GameObjects
class GameObject_Camera : public GameObject
{
public:
    GameObject_Camera(uint64_t GUID) : GameObject(GUID) {}
    ~GameObject_Camera() {}

    void onUse(Player* player) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// implements Type 17 (FISHINGNODE) GameObjects
class GameObject_FishingNode : public GameObject_Lootable
{
public:
    GameObject_FishingNode(uint64_t GUID);
    ~GameObject_FishingNode();

    void onUse(Player* player) override;

    bool HasLoot();

    bool IsLootable() override { return true; }

protected:
    void _internalUpdateOnState(unsigned long timeDiff) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Class implementing Type 18 (SUMMONING_RITUAL) GameObjects
class GameObject_Ritual : public GameObject
{
public:
    GameObject_Ritual(uint64_t GUID);
    ~GameObject_Ritual();

    void InitAI();

    void onUse(Player* player) override;

    struct RitualStruct
    {
        uint64_t CasterGUID = 0;
        uint64_t TargetGUID = 0;
        uint32_t SpellID = 0;
        uint32_t CurrentMembers = 0;
        uint32_t MaxMembers = 0;
        std::vector<uint64_t> Members;

        RitualStruct(uint32_t members) : MaxMembers(members), Members(members) {}

        void Setup(uint64_t caster_guid, uint64_t target_guid, uint32_t spell_id)
        {
            CasterGUID = caster_guid;
            TargetGUID = target_guid;
            SpellID = spell_id;

            AddMember(caster_guid);
        }

        uint64_t GetCasterGUID() { return CasterGUID; }
        uint64_t GetTargetGUID() { return TargetGUID; }
        uint32_t GetSpellID() { return SpellID; }

        bool AddMember(uint64_t GUID)
        {
            uint32_t i = 0;
            for (; i < MaxMembers; i++)
                if (Members[i] == 0)
                    break;

            if (i == MaxMembers)
                return false;

            Members[i] = GUID;
            CurrentMembers++;

            return true;
        }

        bool RemoveMember(uint64_t GUID)
        {
            uint32_t i = 0;
            for (; i < MaxMembers; i++)
            {
                if (Members[i] == GUID)
                {
                    Members[i] = 0;
                    CurrentMembers--;
                    return true;
                }
            }

            return false;
        }

        bool HasMember(uint64_t GUID)
        {
            for (uint32_t i = 0; i < MaxMembers; i++)
                if (Members[i] == GUID)
                    return true;

            return false;
        }

        uint64_t GetMemberGUIDBySlot(uint32_t Slot) { return Members[Slot]; }

        bool HasFreeSlots()
        {
            if (CurrentMembers < MaxMembers)
                return true;

            return false;
        }

        uint32_t GetMaxMembers() { return MaxMembers; }

        void Finish() { SpellID = 0; }

        bool IsFinished()
        {
            if (SpellID == 0)
                return true;

            return false;
        }
    };

    RitualStruct* GetRitual() const { return Ritual.get(); }

private:
    std::unique_ptr<RitualStruct> Ritual;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Implements Type 22 (SPELLCASTER) GameObjects
class GameObject_SpellCaster : public GameObject
{
public:
    GameObject_SpellCaster(uint64_t GUID);
    ~GameObject_SpellCaster();

    void InitAI();

    void onUse(Player* player) override;

private:
    SpellInfo const* spell;
};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing Type 23 (MEETINGSTONE) GameObjects
class GameObject_Meetingstone : public GameObject
{
public:
    GameObject_Meetingstone(uint64_t GUID) : GameObject(GUID) {}
    ~GameObject_Meetingstone() {}

    void onUse(Player* player) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing Type 24 (FLAGSTAND) GameObjects
class GameObject_FlagStand : public GameObject
{
public:
    GameObject_FlagStand(uint64_t GUID) : GameObject(GUID) {}
    ~GameObject_FlagStand() {}

    void onUse(Player* player) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing Type 25 (FISHINGHOLE) GameObjects
class GameObject_FishingHole : public GameObject_Lootable
{
public:
    GameObject_FishingHole(uint64_t GUID);
    ~GameObject_FishingHole();

    void InitAI();

    void onUse(Player* player) override;

    bool IsLootable() { return true; }
    bool HasLoot();

    uint32_t getMaxOpen() const { return maxOpens; }
    void setMaxOpen(uint32_t max) { maxOpens = max; }

protected:
    void _internalUpdateOnState(unsigned long timeDiff) override;

private:
    uint32_t maxOpens = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing Type 26 (FLAGDROP) GameObjects
class GameObject_FlagDrop : public GameObject
{
public:
    GameObject_FlagDrop(uint64_t GUID) : GameObject(GUID) {}
    ~GameObject_FlagDrop() {}

    void onUse(Player* player) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// implementing Type 32 (BARBER_CHAIR) GameObjects
class GameObject_BarberChair : public GameObject
{
public:
    GameObject_BarberChair(uint64_t GUID) : GameObject(GUID) {}
    ~GameObject_BarberChair() {}

    void onUse(Player* player) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Implements Type 33 (DESTRUCTIBLE) GameObjects
class SERVER_DECL GameObject_Destructible : public GameObject
{
public:
    GameObject_Destructible(uint64_t GUID);
    ~GameObject_Destructible();

    void InitAI();

    void Damage(uint32_t damage, uint64_t AttackerGUID, uint64_t ControllerGUID, uint32_t SpellID);

    void Rebuild();
    void setDestructibleState(GameObjectDestructibleState state, bool setHealth = false);
    GameObjectDestructibleState GetDestructibleState() const
    {
        if (hasFlags(GO_FLAG_DESTROYED))
            return GO_DESTRUCTIBLE_DESTROYED;
        if (hasFlags(GO_FLAG_DAMAGED))
            return GO_DESTRUCTIBLE_DAMAGED;
        return GO_DESTRUCTIBLE_INTACT;
    }

    uint32_t GetHP() { return hitpoints; }
    void setHP(uint32_t hp) { hitpoints = hp; }

    uint32_t GetMaxHP() { return maxhitpoints; }
    void setMaxHP(uint32_t maxHp) { maxhitpoints = maxHp; }

private:
    void SendDamagePacket(uint32_t damage, uint64_t AttackerGUID, uint64_t ControllerGUID, uint32_t SpellID);

    uint32_t hitpoints;
    uint32_t maxhitpoints;
};

#endif // GAMEOBJECT_H
