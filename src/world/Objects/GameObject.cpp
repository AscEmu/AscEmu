/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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
 *
 */

#include "StdAfx.h"
#include "Management/GameEvent.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include <G3D/Quat.h>
#include "Map/MapCell.h"
#include "Map/MapMgr.h"
#include "Faction.h"
#include "Spell/Definitions/ProcFlags.h"
#include "Spell/Definitions/SpellEffectTarget.h"
#include "Spell/Customization/SpellCustomizations.hpp"

GameObject::GameObject(uint64 guid)
{
    m_objectType |= TYPE_GAMEOBJECT;
    m_objectTypeId = TYPEID_GAMEOBJECT;

#if VERSION_STRING != Cata
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_POSITION | UPDATEFLAG_ROTATION);
#else
    m_updateFlag = (UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);
#endif

    m_valuesCount = GAMEOBJECT_END;
    m_uint32Values = _fields;
    std::fill(m_uint32Values, &m_uint32Values[GAMEOBJECT_END], 0);
    m_updateMask.SetCount(GAMEOBJECT_END);
    setUInt32Value(OBJECT_FIELD_TYPE, TYPE_GAMEOBJECT | TYPE_OBJECT);
    SetGUID(guid);
    SetAnimProgress(100);
    m_wowGuid.Init(guid);
    SetScale(1);
    m_summonedGo = false;
    invisible = false;
    invisibilityFlag = INVIS_FLAG_NORMAL;
    m_summoner = NULL;
    charges = -1;
    gameobject_properties = nullptr;
    myScript = NULL;
    m_spawn = 0;
    m_deleted = false;
    m_respawnCell = NULL;
    m_rotation = 0;
    m_overrides = 0;

    m_model = NULL;
}

GameObject::~GameObject()
{
    sEventMgr.RemoveEvents(this);

    if (myScript != NULL)
    {
        myScript->Destroy();
        myScript = NULL;
    }

    uint32 guid = getUInt32Value(OBJECT_FIELD_CREATED_BY);
    if (guid)
    {
        Player* plr = objmgr.GetPlayer(guid);
        if (plr && plr->GetSummonedObject() == this)
            plr->SetSummonedObject(NULL);

        if (plr == m_summoner)
            m_summoner = 0;
    }

    if (m_respawnCell != NULL)
        m_respawnCell->_respawnObjects.erase(this);

    if (m_summonedGo && m_summoner)
        for (uint8 i = 0; i < 4; i++)
            if (m_summoner->m_ObjectSlots[i] == GetLowGUID())
                m_summoner->m_ObjectSlots[i] = 0;
}

bool GameObject::CreateFromProto(uint32 entry, uint32 mapid, float x, float y, float z, float ang, float r0, float r1, float r2, float r3, uint32 overrides)
{
    gameobject_properties = sMySQLStore.getGameObjectProperties(entry);
    if (gameobject_properties == nullptr)
    {
        LOG_ERROR("Something tried to create a GameObject with invalid entry %u", entry);
        return false;
    }

    Object::_Create(mapid, x, y, z, ang);
    SetEntry(entry);

    m_overrides = overrides;
    SetRotationQuat(r0, r1, r2, r3);
    SetPosition(x, y, z, ang);
    //UpdateRotation();
    SetAnimProgress(0);
    SetState(1);
    SetDisplayId(gameobject_properties->display_id);
    SetType(static_cast<uint8>(gameobject_properties->type));
    InitAI();

    return true;
}

void GameObject::Update(unsigned long time_passed)
{
    if (m_event_Instanceid != m_instanceId)
    {
        event_Relocate();
        return;
    }

    if (!IsInWorld())
        return;

    if (m_deleted)
        return;

    _UpdateSpells(time_passed);
}

void GameObject::Spawn(MapMgr* m)
{
    PushToWorld(m);
}

void GameObject::Despawn(uint32 delay, uint32 respawntime)
{
    if (delay)
    {
        sEventMgr.AddEvent(this, &GameObject::Despawn, (uint32)0, respawntime, EVENT_GAMEOBJECT_EXPIRE, delay, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        return;
    }

    if (!IsInWorld())
        return;

    //This is for go get deleted while looting
    if (m_spawn)
    {
        SetState(static_cast<uint8>(m_spawn->state));
        SetFlags(m_spawn->flags);
    }

    CALL_GO_SCRIPT_EVENT(this, OnDespawn)();

    if (respawntime)
    {
        /* Get our originating mapcell */
        MapCell* pCell = GetMapCell();
        ARCEMU_ASSERT(pCell != NULL);
        pCell->_respawnObjects.insert(this);
        sEventMgr.RemoveEvents(this);
        sEventMgr.AddEvent(m_mapMgr, &MapMgr::EventRespawnGameObject, this, pCell->GetPositionX(), pCell->GetPositionY(), EVENT_GAMEOBJECT_ITEM_SPAWN, respawntime, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        Object::RemoveFromWorld(false);
        m_respawnCell = pCell;
    }
    else
    {
        Object::RemoveFromWorld(true);
        ExpireAndDelete();
    }
}

void GameObject::SaveToDB()
{
    if (m_spawn == NULL)
    {
        // Create spawn instance
        m_spawn = new GameobjectSpawn;
        m_spawn->entry = GetEntry();
        m_spawn->id = objmgr.GenerateGameObjectSpawnID();
        m_spawn->map = GetMapId();
        m_spawn->position_x = GetPositionX();
        m_spawn->position_y = GetPositionY();
        m_spawn->position_z = GetPositionZ();
        m_spawn->orientation = GetOrientation();
        m_spawn->rotation_0 = GetParentRotation(0);
        m_spawn->rotation_1 = GetParentRotation(1);
        m_spawn->rotation_2 = GetParentRotation(2);
        m_spawn->rotation_3 = GetParentRotation(3);
        m_spawn->state = GetState();
        m_spawn->flags = GetFlags();
        m_spawn->faction = GetFaction();
        m_spawn->scale = GetScale();
        //m_spawn->stateNpcLink = 0;
        m_spawn->phase = GetPhase();
        m_spawn->overrides = GetOverrides();

        uint32 cx = GetMapMgr()->GetPosX(GetPositionX());
        uint32 cy = GetMapMgr()->GetPosY(GetPositionY());

        GetMapMgr()->GetBaseMap()->GetSpawnsListAndCreate(cx, cy)->GameobjectSpawns.push_back(m_spawn);
    }
    std::stringstream ss;

    ss << "DELETE FROM gameobject_spawns WHERE id = ";
    ss << m_spawn->id;
    ss << ";";

    WorldDatabase.ExecuteNA(ss.str().c_str());

    ss.rdbuf()->str("");

    ss << "INSERT INTO gameobject_spawns VALUES("
        << m_spawn->id << ","
        << GetEntry() << ","
        << GetMapId() << ","
        << GetPositionX() << ","
        << GetPositionY() << ","
        << GetPositionZ() << ","
        << GetOrientation() << ","
        << GetParentRotation(0) << ","
        << GetParentRotation(1) << ","
        << GetParentRotation(2) << ","
        << GetParentRotation(3) << ","
        << "0,"              // initial state
        << GetFlags() << ","
        << GetFaction() << ","
        << GetScale() << ","
        << "0,"
        << m_phase << ","
        << m_overrides << ")";
    WorldDatabase.Execute(ss.str().c_str());
}

void GameObject::SaveToFile(std::stringstream & name)
{

    std::stringstream ss;

    ss << "INSERT INTO gameobject_spawns VALUES("
        << ((m_spawn == NULL) ? 0 : m_spawn->id) << ","
        << GetEntry() << ","
        << GetMapId() << ","
        << GetPositionX() << ","
        << GetPositionY() << ","
        << GetPositionZ() << ","
        << GetOrientation() << ","
        << GetParentRotation(0) << ","
        << GetParentRotation(1) << ","
        << GetParentRotation(2) << ","
        << GetParentRotation(3) << ","
        << uint32(GetState()) << ","
        << GetFlags() << ","
        << GetFaction() << ","
        << GetScale() << ","
        << "0,"
        << m_phase << ","
        << m_overrides << ")";

    FILE* OutFile;

    OutFile = fopen(name.str().c_str(), "wb");
    if (!OutFile) return;
    fwrite(ss.str().c_str(), 1, ss.str().size(), OutFile);
    fclose(OutFile);

}

void GameObject::InitAI()
{
    if (myScript == NULL)
        myScript = sScriptMgr.CreateAIScriptClassForGameObject(GetEntry(), this);
}

bool GameObject::Load(GameobjectSpawn* go_spawn)
{
    if (!CreateFromProto(go_spawn->entry, 0, go_spawn->position_x, go_spawn->position_y, go_spawn->position_z, go_spawn->orientation, go_spawn->rotation_0, go_spawn->rotation_1, go_spawn->rotation_2, go_spawn->rotation_3, go_spawn->overrides))
        return false;

    m_spawn = go_spawn;
    m_phase = go_spawn->phase;
    SetFlags(go_spawn->flags);
    SetState(static_cast<uint8>(go_spawn->state));
    if (go_spawn->faction)
    {
        SetFaction(go_spawn->faction);
    }
    SetScale(go_spawn->scale);

    return true;
}

void GameObject::DeleteFromDB()
{
    if (m_spawn != NULL)
        WorldDatabase.Execute("DELETE FROM gameobject_spawns WHERE id=%u", m_spawn->id);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Summoned Go's
//////////////////////////////////////////////////////////////////////////////////////////
void GameObject::_Expire()
{
    sEventMgr.RemoveEvents(this);

    if (IsInWorld())
        RemoveFromWorld(true);

    //sEventMgr.AddEvent(World::getSingletonPtr(), &World::DeleteObject, ((Object*)this), EVENT_DELETE_TIMER, 1000, 1);
    delete this;
    //this = NULL;
}

void GameObject::ExpireAndDelete()
{
    if (m_deleted)
        return;

    m_deleted = true;

    // remove any events
    sEventMgr.RemoveEvents(this);
    if (IsInWorld())
        sEventMgr.AddEvent(this, &GameObject::_Expire, EVENT_GAMEOBJECT_EXPIRE, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    else
        delete this;
}

void GameObject::CallScriptUpdate()
{
    ARCEMU_ASSERT(myScript != NULL);
    myScript->AIUpdate();
}

void GameObject::OnPushToWorld()
{
    Object::OnPushToWorld();
    if (mEvent != nullptr)
    {
        if (mEvent->mEventScript != nullptr)
        {
            mEvent->mEventScript->OnGameObjectPushToWorld(mEvent, this);
        }
    }

    CALL_GO_SCRIPT_EVENT(this, OnCreate)();
    CALL_GO_SCRIPT_EVENT(this, OnSpawn)();
    CALL_INSTANCE_SCRIPT_EVENT(m_mapMgr, OnGameObjectPushToWorld)(this);

    if (gameobject_properties->type == GAMEOBJECT_TYPE_CHEST)
    {
        //restock on respwn
        static_cast<GameObject_Lootable*>(this)->resetLoot();

        //close if open (happenes after respawn)
        if (this->GetState() == GO_STATE_OPEN)
            this->SetState(GO_STATE_CLOSED);

        // set next loot reset time
        time_t lootResetTime = 60 * 1000;
        if (gameobject_properties->chest.restock_time > 60)
            lootResetTime = gameobject_properties->chest.restock_time * 1000;

        sEventMgr.AddEvent(static_cast<GameObject_Lootable*>(this), &GameObject_Lootable::resetLoot, EVENT_GO_CHEST_RESTOCK, lootResetTime, 0, 0);
    }
}

void GameObject::OnRemoveInRangeObject(Object* pObj)
{
    Object::OnRemoveInRangeObject(pObj);
    if (m_summonedGo && m_summoner == pObj)
    {
        for (uint8 i = 0; i < 4; i++)
            if (m_summoner->m_ObjectSlots[i] == GetLowGUID())
                m_summoner->m_ObjectSlots[i] = 0;

        m_summoner = 0;
        ExpireAndDelete();
    }
}
// Remove gameobject from world, using their despawn animation.
void GameObject::RemoveFromWorld(bool free_guid)
{
    WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
    data << uint64(GetGUID());
    SendMessageToSet(&data, true);

    sEventMgr.RemoveEvents(this);
    Object::RemoveFromWorld(free_guid);
}

uint32 GameObject::GetGOReqSkill()
{
    // Here we check the SpellFocus table against the dbcs
    auto lock = sLockStore.LookupEntry(GetGameObjectProperties()->raw.parameter_0);
    if (!lock)
        return 0;

    for (uint8 i = 0; i < LOCK_NUM_CASES; i++)
    {
        if (lock->locktype[i] == 2 && lock->minlockskill[i])
            return lock->minlockskill[i];
    }
    return 0;
}

using G3D::Quat;
struct QuaternionCompressed
{
    QuaternionCompressed() : m_raw(0) {}
    QuaternionCompressed(int64 val) : m_raw(val) {}
    QuaternionCompressed(const Quat& quat) { Set(quat); }

    enum
    {
        PACK_COEFF_YZ = 1 << 20,
        PACK_COEFF_X = 1 << 21,
    };

    void Set(const Quat& quat)
    {
        int8 w_sign = (quat.w >= 0 ? 1 : -1);
        int64 X = int32(quat.x * PACK_COEFF_X) * w_sign & ((1 << 22) - 1);
        int64 Y = int32(quat.y * PACK_COEFF_YZ) * w_sign & ((1 << 21) - 1);
        int64 Z = int32(quat.z * PACK_COEFF_YZ) * w_sign & ((1 << 21) - 1);
        m_raw = Z | (Y << 21) | (X << 42);
    }

    Quat Unpack() const
    {
        double x = (double)(m_raw >> 42) / (double)PACK_COEFF_X;
        double y = (double)(m_raw << 22 >> 43) / (double)PACK_COEFF_YZ;
        double z = (double)(m_raw << 43 >> 43) / (double)PACK_COEFF_YZ;
        double w = 1 - (x * x + y * y + z * z);
        ARCEMU_ASSERT(w >= 0);
        w = sqrt(w);

        return Quat(float(x), float(y), float(z), float(w));
    }

    int64 m_raw;
};

void GameObject::SetRotationQuat(float qx, float qy, float qz, float qw)
{
    Quat quat(qx, qy, qz, qw);
    // Temporary solution for gameobjects that has no rotation data in DB:
    if (qz == 0 && qw == 0)
        quat = Quat::fromAxisAngleRotation(G3D::Vector3::unitZ(), GetOrientation());

    quat.unitize();
    m_rotation = QuaternionCompressed(quat).m_raw;
    setFloatValue(GAMEOBJECT_PARENTROTATION+0, quat.x);
    setFloatValue(GAMEOBJECT_PARENTROTATION+1, quat.y);
    setFloatValue(GAMEOBJECT_PARENTROTATION+2, quat.z);
    setFloatValue(GAMEOBJECT_PARENTROTATION+3, quat.w);
}

void GameObject::SetRotationAngles(float z_rot, float y_rot, float x_rot)
{
    Quat quat(G3D::Matrix3::fromEulerAnglesZYX(z_rot, y_rot, x_rot));
    SetRotationQuat(quat.x, quat.y, quat.z, quat.w);
}

void GameObject::CastSpell(uint64 TargetGUID, SpellInfo* sp)
{
    Spell* s = new Spell(this, sp, true, NULL);

    SpellCastTargets tgt(TargetGUID);

    // TODO: Is this meant to be set source?
    tgt.setDestination(GetPosition());

    s->prepare(&tgt);
}

void GameObject::CastSpell(uint64 TargetGUID, uint32 SpellID)
{
    SpellInfo* sp = sSpellCustomizations.GetSpellInfo(SpellID);
    if (sp == nullptr)
    {
        LogError("GameObject %u tried to cast a non-existing Spell %u.", gameobject_properties->entry, SpellID);
        return;
    }

    CastSpell(TargetGUID, sp);
}

//MIT
void GameObject::SetCustomAnim(uint32_t anim)
{
    WorldPacket data(SMSG_GAMEOBJECT_CUSTOM_ANIM, 12);
    data << uint64_t(GetGUID());
    data << uint32_t(anim);
    SendMessageToSet(&data, false, false);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Door
GameObject_Door::GameObject_Door(uint64 GUID) : GameObject(GUID)
{ }

GameObject_Door::~GameObject_Door()
{ }

void GameObject_Door::InitAI()
{
    GameObject::InitAI();

    if (gameobject_properties->door.start_open != 0)
        SetState(GO_STATE_OPEN);
    else
        SetState(GO_STATE_CLOSED);
}

void GameObject_Door::Open()
{
    SetState(GO_STATE_OPEN);
    if (gameobject_properties->door.auto_close_time != 0)
        sEventMgr.AddEvent(this, &GameObject_Door::Close, 0, gameobject_properties->door.auto_close_time, 1, 0);
}

void GameObject_Door::Close()
{
    sEventMgr.RemoveEvents(this, EVENT_GAMEOBJECT_CLOSE);
    SetState(GO_STATE_CLOSED);
}

void GameObject_Door::SpecialOpen()
{
    SetState(GO_STATE_ALTERNATIVE_OPEN);
}

void GameObject_Door::Use(uint64 GUID)
{
    if (GetState() == GO_STATE_CLOSED)
        Open();
    else
        Close();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Button
GameObject_Button::GameObject_Button(uint64 GUID) : GameObject(GUID)
{
    spell = nullptr;
}

GameObject_Button::~GameObject_Button()
{ }

void GameObject_Button::InitAI()
{
    GameObject::InitAI();

    if (gameobject_properties->button.start_open != 0)
        SetState(GO_STATE_OPEN);

    if (gameobject_properties->button.linked_trap_id != 0)
    {
        GameObjectProperties const* gameobject_info = sMySQLStore.getGameObjectProperties(gameobject_properties->button.linked_trap_id);

        if (gameobject_info != nullptr)
        {
            if (gameobject_info->trap.spell_id != 0)
                spell = sSpellCustomizations.GetSpellInfo(gameobject_info->trap.spell_id);
        }
    }
}

void GameObject_Button::Open()
{
    SetState(GO_STATE_OPEN);
    if (gameobject_properties->button.auto_close_time != 0)
        sEventMgr.AddEvent(this, &GameObject_Button::Close, EVENT_GAMEOBJECT_CLOSE, gameobject_properties->button.auto_close_time, 1, 0);
}

void GameObject_Button::Close()
{
    sEventMgr.RemoveEvents(this, EVENT_GAMEOBJECT_CLOSE);
    SetState(GO_STATE_CLOSED);
}

void GameObject_Button::Use(uint64 GUID)
{
    if (GetState() == GO_STATE_CLOSED)
    {
        Open();

        if (spell != NULL)
            CastSpell(GUID, spell);
    }
    else
    {
        Close();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_QuestGiver
GameObject_QuestGiver::GameObject_QuestGiver(uint64 GUID) : GameObject(GUID)
{
    m_quests = NULL;
}

GameObject_QuestGiver::~GameObject_QuestGiver()
{ }

void GameObject_QuestGiver::InitAI()
{
    LoadQuests();
    GameObject::InitAI();
}

void GameObject_QuestGiver::DeleteQuest(QuestRelation* Q)
{
    for (std::list<QuestRelation*>::iterator itr = m_quests->begin(); itr != m_quests->end(); ++itr)
    {
        QuestRelation* qr = *itr;

        if ((qr->type == Q->type) && (qr->qst == Q->qst))
        {
            delete qr;
            m_quests->erase(itr);
            break;
        }
    }
}

QuestProperties const* GameObject_QuestGiver::FindQuest(uint32 quest_id, uint8 quest_relation)
{
    for (std::list<QuestRelation*>::iterator itr = m_quests->begin(); itr != m_quests->end(); ++itr)
    {
        QuestRelation* qr = *itr;

        if ((qr->qst->id == quest_id) && ((qr->type & quest_relation) != 0))
        {
            return qr->qst;
        }
    }
    return nullptr;
}

uint16 GameObject_QuestGiver::GetQuestRelation(uint32 quest_id)
{
    uint16 quest_relation = 0;

    for (std::list<QuestRelation*>::iterator itr = m_quests->begin(); itr != m_quests->end(); ++itr)
    {
        QuestRelation* qr = *itr;

        if ((qr != nullptr) && (qr->qst->id == quest_id))
            quest_relation |= qr->type;
    }

    return quest_relation;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Chest
GameObject_Chest::GameObject_Chest(uint64 GUID) : GameObject_Lootable(GUID)
{
    spell = nullptr;
}

GameObject_Chest::~GameObject_Chest()
{ }

void GameObject_Chest::InitAI()
{
    GameObject::InitAI();

    if (gameobject_properties->chest.linked_trap_id != 0)
    {
        GameObjectProperties const* gameobject_info = sMySQLStore.getGameObjectProperties(gameobject_properties->chest.linked_trap_id);

        if (gameobject_info != nullptr)
        {
            if (gameobject_info->trap.spell_id != 0)
                spell = sSpellCustomizations.GetSpellInfo(gameobject_info->trap.spell_id);
        }
    }
}

bool GameObject_Chest::HasLoot()
{
    if (loot.gold > 0)
        return true;

    for (std::vector< __LootItem >::iterator itr = loot.items.begin(); itr != loot.items.end(); ++itr)
    {
        if ((itr->item.itemproto->Bonding == ITEM_BIND_QUEST) || (itr->item.itemproto->Bonding == ITEM_BIND_QUEST2))
            continue;

        if (itr->iItemsCount > 0)
            return true;
    }
    return false;
}

void GameObject_Chest::Open()
{
    SetState(GO_STATE_OPEN);
}

void GameObject_Chest::Close()
{
    SetState(GO_STATE_CLOSED);
}

void GameObject_Chest::Use(uint64 GUID)
{
    if (GetState() == GO_STATE_CLOSED)
    {
        Open();

        if (spell != NULL)
            CastSpell(GUID, spell);
    }
    else
    {
        Close();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Trap
GameObject_Trap::GameObject_Trap(uint64 GUID) : GameObject(GUID)
{
    spell = NULL;
    targetupdatetimer = 0;
    maxdistance = 0.0f;
    cooldown = 0;
}

GameObject_Trap::~GameObject_Trap()
{ }

void GameObject_Trap::InitAI()
{
    ///\brief prevent traps from casting periodic spells. This can be removed until proper event handling.
    // e.g. BootyBay
    switch (gameobject_properties->entry)
    {
        case 171941:
        case 175791:
        case 176214:
        case 179557:
        case 179560:
        case 180391:
            return;
    }

    spell = sSpellCustomizations.GetSpellInfo(gameobject_properties->trap.spell_id);
    charges = gameobject_properties->trap.charges;

    if (gameobject_properties->trap.stealthed != 0)
    {
        invisible = true;
        invisibilityFlag = INVIS_FLAG_TRAP;
    }

    cooldown = gameobject_properties->trap.cooldown * 1000;
    if (cooldown < 1000)
        cooldown = 1000;

    maxdistance = sqrt(float(gameobject_properties->trap.radius));
    if (maxdistance == 0.0f)
        maxdistance = 1.0f;

    GameObject::InitAI();
}

void GameObject_Trap::Update(unsigned long time_passed)
{
    if (m_deleted)
        return;

    if (m_event_Instanceid != m_instanceId)
    {
        event_Relocate();
        return;
    }

    if (!IsInWorld())
        return;

    if (spell == NULL)
        return;

    if (GetState() == 1)
    {
        targetupdatetimer += time_passed;

        if (targetupdatetimer > cooldown)   // Update targets only if cooldown finished
            targetupdatetimer = 0;

        if (targetupdatetimer != 0)
            return;

        for (std::set<Object*>::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
        {
            float dist;

            Object* o = *itr;

            if (!o->IsUnit())
                continue;

            if ((m_summoner != NULL) && (o->GetGUID() == m_summoner->GetGUID()))
                continue;

            dist = getDistanceSq(o);

            if (dist <= maxdistance)
            {

                if (m_summonedGo)
                {
                    if (!m_summoner)
                    {
                        ExpireAndDelete();
                        return;
                    }

                    if (!isAttackable(m_summoner, o))
                        continue;
                }

                CastSpell(o->GetGUID(), spell);

                if (m_summoner != NULL)
                    m_summoner->HandleProc(PROC_ON_TRAP_TRIGGER, reinterpret_cast<Unit*>(o), spell);

                if (charges != 0)
                    charges--;

                if (m_summonedGo && gameobject_properties->trap.charges != 0 && charges == 0)
                {
                    ExpireAndDelete();
                    return;
                }

                if (spell->getEffectImplicitTargetA(0) == EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT || spell->getEffectImplicitTargetA(0) == EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT)
                {
                    return;	 // on area don't continue.
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_SpellFocus
GameObject_SpellFocus::GameObject_SpellFocus(uint64 GUID) : GameObject(GUID)
{ }

GameObject_SpellFocus::~GameObject_SpellFocus()
{ }

void GameObject_SpellFocus::OnPushToWorld()
{
    GameObject::OnPushToWorld();
    SpawnLinkedTrap();
}

void GameObject_SpellFocus::SpawnLinkedTrap()
{
    uint32 trapid = gameobject_properties->spell_focus.linked_trap_id;
    if (trapid == 0)
        return;

    GameObject* go = m_mapMgr->CreateGameObject(trapid);
    if (go == nullptr)
    {
        LogError("Failed to create linked trap (entry: %u) for GameObject %u ( %s ). Missing GOProperties!", trapid, gameobject_properties->entry, gameobject_properties->name.c_str());
        return;
    }

    if (!go->CreateFromProto(trapid, m_mapId, m_position.x, m_position.y, m_position.z, m_position.o))
    {
        LogError("Failed CreateFromProto for linked trap of GameObject %u ( %s ).", gameobject_properties->entry, gameobject_properties->name.c_str());
        return;
    }

    go->SetFaction(GetFaction());
    go->setUInt64Value(OBJECT_FIELD_CREATED_BY, GetGUID());
    go->PushToWorld(m_mapMgr);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Goober
GameObject_Goober::GameObject_Goober(uint64 GUID) : GameObject(GUID)
{
    spell = NULL;
}

GameObject_Goober::~GameObject_Goober()
{ }

void GameObject_Goober::InitAI()
{
    GameObject::InitAI();

    if (gameobject_properties->goober.linked_trap_id != 0)
    {
        GameObjectProperties const* gameobject_info = sMySQLStore.getGameObjectProperties(gameobject_properties->goober.linked_trap_id);
        if (gameobject_info != nullptr)
        {
            if (gameobject_info->trap.spell_id != 0)
                spell = sSpellCustomizations.GetSpellInfo(gameobject_info->trap.spell_id);
        }
    }
}

void GameObject_Goober::Open()
{
    SetState(GO_STATE_OPEN);
    if (gameobject_properties->goober.auto_close_time != 0)
        sEventMgr.AddEvent(this, &GameObject_Goober::Close, EVENT_GAMEOBJECT_CLOSE, gameobject_properties->goober.auto_close_time, 1, 0);
}

void GameObject_Goober::Close()
{
    sEventMgr.RemoveEvents(this, EVENT_GAMEOBJECT_CLOSE);
    SetState(GO_STATE_CLOSED);
}

void GameObject_Goober::Use(uint64 GUID)
{
    if (GetState() == GO_STATE_CLOSED)
    {
        Open();

        if (spell != NULL)
        {
            CastSpell(GUID, spell);
        }
    }
    else
    {
        Close();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_FishingNode
GameObject_FishingNode::GameObject_FishingNode(uint64 GUID) : GameObject_Lootable(GUID)
{
    FishHooked = false;
}

GameObject_FishingNode::~GameObject_FishingNode()
{ }

void GameObject_FishingNode::OnPushToWorld()
{
    uint32 zone = 0; // GetArea(GetPositionX(), GetPositionY(), GetPositionZ());
    if (zone == 0)
        zone = GetZoneId();

    // Only set a 'splash' if there is any loot in this area / zone
    if (lootmgr.IsFishable(zone))
    {
        uint32 seconds[] = { 0, 4, 10, 14 };
        uint32 rnd = RandomUInt(3);
        sEventMgr.AddEvent(this, &GameObject_FishingNode::EventFishHooked, EVENT_GAMEOBJECT_FISH_HOOKED, seconds[rnd] * 1000, 1, 0);

    }
    sEventMgr.AddEvent(this, &GameObject_FishingNode::EndFishing, true, EVENT_GAMEOBJECT_END_FISHING, 17 * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

bool GameObject_FishingNode::UseNode()
{
    sEventMgr.RemoveEvents(this);

    // Clicking on the bobber before something is hooked
    if (!FishHooked)
    {
        EndFishing(true);
        return false;
    }
    return true;
}

void GameObject_FishingNode::EndFishing(bool abort)
{
    if (!abort)
        sEventMgr.AddEvent(static_cast<GameObject*>(this), &GameObject::ExpireAndDelete, EVENT_GAMEOBJECT_EXPIRE, 10 * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    else
        ExpireAndDelete();
}

void GameObject_FishingNode::EventFishHooked()
{
    SetCustomAnim();
    FishHooked = true;
}

bool GameObject_FishingNode::HasLoot()
{
    for (std::vector<__LootItem>::iterator itr = loot.items.begin(); itr != loot.items.end(); ++itr)
    {
        if ((itr->item.itemproto->Bonding == ITEM_BIND_QUEST) || (itr->item.itemproto->Bonding == ITEM_BIND_QUEST2))
            continue;

        if (itr->iItemsCount > 0)
            return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Ritual
GameObject_Ritual::GameObject_Ritual(uint64 GUID) : GameObject(GUID)
{
    Ritual = NULL;;
}

GameObject_Ritual::~GameObject_Ritual()
{
    delete Ritual;
    Ritual = NULL;
}

void GameObject_Ritual::InitAI()
{
    Ritual = new CRitual(gameobject_properties->summoning_ritual.req_participants);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_SpellCaster
GameObject_SpellCaster::GameObject_SpellCaster(uint64 GUID) : GameObject(GUID)
{
    spell = nullptr;
}

GameObject_SpellCaster::~GameObject_SpellCaster()
{ }

void GameObject_SpellCaster::InitAI()
{
    charges = gameobject_properties->spell_caster.charges;

    spell = sSpellCustomizations.GetSpellInfo(gameobject_properties->spell_caster.spell_id);
    if (spell == nullptr)
        LogError("GameObject %u ( %s ) has a nonexistant spellID in the database.", gameobject_properties->entry, gameobject_properties->name.c_str());
}

void GameObject_SpellCaster::Use(uint64 GUID)
{
    if (spell == nullptr)
        return;

    CastSpell(GUID, spell);

    if ((charges > 0) && (--charges == 0))
        ExpireAndDelete();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_FishingHole
GameObject_FishingHole::GameObject_FishingHole(uint64 GUID) : GameObject_Lootable(GUID)
{
    usage_remaining = 0;
}

GameObject_FishingHole::~GameObject_FishingHole()
{ }

void GameObject_FishingHole::InitAI()
{
    CalcFishRemaining(true);
}

bool GameObject_FishingHole::CanFish()
{
    if (usage_remaining > 0)
        return true;
    else
        return false;
}

void GameObject_FishingHole::CatchFish()
{
    ASSERT(usage_remaining > 0);
    usage_remaining--;
    if (usage_remaining == 0)
        sEventMgr.AddEvent(static_cast<GameObject*>(this), &GameObject::Despawn, uint32(0), (1800000 + RandomUInt(3600000)), EVENT_GAMEOBJECT_EXPIRE, 10000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT); // respawn in 30 - 90 minutes
}

void GameObject_FishingHole::CalcFishRemaining(bool force)
{
    if (force || (usage_remaining == 0))
        usage_remaining = gameobject_properties->fishinghole.min_success_opens + RandomUInt(gameobject_properties->fishinghole.max_success_opens - gameobject_properties->fishinghole.min_success_opens) - 1;
}

bool GameObject_FishingHole::HasLoot()
{
    for (std::vector<__LootItem>::iterator itr = loot.items.begin(); itr != loot.items.end(); ++itr)
    {
        if (itr->item.itemproto->Bonding == ITEM_BIND_QUEST || itr->item.itemproto->Bonding == ITEM_BIND_QUEST2)
            continue;

        if (itr->iItemsCount > 0)
            return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Class functions for GameObject_Destructible
GameObject_Destructible::GameObject_Destructible(uint64 GUID) : GameObject(GUID)
{
    hitpoints = 0;
    maxhitpoints = 0;
}

GameObject_Destructible::~GameObject_Destructible()
{ }

void GameObject_Destructible::InitAI()
{
    GameObject::InitAI();
    Rebuild();
}

void GameObject_Destructible::Damage(uint32 damage, uint64 AttackerGUID, uint64 ControllerGUID, uint32 SpellID)
{
    // If we are already destroyed there's nothing to damage!
    if (hitpoints == 0)
        return;

    if (damage >= hitpoints)
    {
        // Instant destruction
        hitpoints = 0;

        SetFlags(GO_FLAG_DESTROYED);
        SetFlags(GetFlags() & ~GO_FLAG_DAMAGED);
        SetDisplayId(gameobject_properties->destructible_building.destroyed_display_id);   // destroyed display id

        CALL_GO_SCRIPT_EVENT(this, OnDestroyed)();

    }
    else
    {
        // Simply damaging
        hitpoints -= damage;

        if (!HasFlags(GO_FLAG_DAMAGED))
        {
            // Intact  ->  Damaged

            // Are we below the intact-damaged transition treshold?
            if (hitpoints <= (maxhitpoints - gameobject_properties->destructible_building.intact_num_hits))
            {
                SetFlags(GO_FLAG_DAMAGED);
                SetDisplayId(gameobject_properties->destructible_building.damaged_display_id); // damaged display id
            }
        }
        else
        {
            if (hitpoints == 0)
            {
                SetFlags(GetFlags() & ~GO_FLAG_DAMAGED);
                SetFlags(GO_FLAG_DESTROYED);
                SetDisplayId(gameobject_properties->destructible_building.destroyed_display_id);
            }
        }

        CALL_GO_SCRIPT_EVENT(this, OnDamaged)(damage);
    }

    uint8 animprogress = static_cast<uint8>(std::round(hitpoints / float(maxhitpoints)) * 255);
    SetAnimProgress(animprogress);
    SendDamagePacket(damage, AttackerGUID, ControllerGUID, SpellID);
}

void GameObject_Destructible::SendDamagePacket(uint32 damage, uint64 AttackerGUID, uint64 ControllerGUID, uint32 SpellID)
{
#if VERSION_STRING > TBC
    WorldPacket data(SMSG_DESTRUCTIBLE_BUILDING_DAMAGE, 29);

    data << WoWGuid(GetNewGUID());
    data << WoWGuid(AttackerGUID);
    data << WoWGuid(ControllerGUID);
    data << uint32(damage);
    data << uint32(SpellID);
    SendMessageToSet(&data, false, false);
#endif
}

void GameObject_Destructible::Rebuild()
{
    SetFlags(GetFlags() & uint32(~(GO_FLAG_DAMAGED | GO_FLAG_DESTROYED)));
    SetDisplayId(gameobject_properties->display_id);
    maxhitpoints = gameobject_properties->destructible_building.intact_num_hits + gameobject_properties->destructible_building.damaged_num_hits;
    hitpoints = maxhitpoints;
}

