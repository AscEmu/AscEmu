/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Management/TaxiMgr.h"
#include "Server/Packets/Opcode.h"
#include "Units/Players/Player.h"

initialiseSingleton(TaxiMgr);


void TaxiPath::ComputeLen()
{
    m_length1 = 0;
    m_length2 = 0;
    m_map1 = 0;
    m_map2 = 0;
    float* curptr = &m_length1;

    if (!m_pathNodes.size())
        return;

    std::map<uint32, TaxiPathNode*>::iterator itr;
    itr = m_pathNodes.begin();

    float x = itr->second->x;
    float y = itr->second->y;
    float z = itr->second->z;
    uint32 curmap = itr->second->mapid;
    m_map1 = curmap;

    ++itr;

    while (itr != m_pathNodes.end())
    {
        if (itr->second->mapid != curmap)
        {
            curptr = &m_length2;
            m_map2 = itr->second->mapid;
            curmap = itr->second->mapid;
        }

        *curptr += sqrt((itr->second->x - x) * (itr->second->x - x) +
                        (itr->second->y - y) * (itr->second->y - y) +
                        (itr->second->z - z) * (itr->second->z - z));

        x = itr->second->x;
        y = itr->second->y;
        z = itr->second->z;
        ++itr;
    }
}

void TaxiPath::SetPosForTime(float & x, float & y, float & z, uint32 time, uint32* last_node, uint32 mapid)
{
    if (!time)
        return;

    float length;
    if (mapid == m_map1)
        length = m_length1;
    else
        length = m_length2;

    float traveled_len = (time / (length * TAXI_TRAVEL_SPEED)) * length;
    uint32 len = 0;

    x = 0;
    y = 0;
    z = 0;

    if (!m_pathNodes.size())
        return;

    std::map<uint32, TaxiPathNode*>::iterator itr;
    itr = m_pathNodes.begin();

    float nx, ny, nz;
    nx = ny = nz = 0.0f;
    bool set = false;
    uint32 nodecounter = 0;

    while (itr != m_pathNodes.end())
    {
        if (itr->second->mapid != mapid)
        {
            ++itr;
            ++nodecounter;
            continue;
        }

        if (!set)
        {
            nx = itr->second->x;
            ny = itr->second->y;
            nz = itr->second->z;
            set = true;
            continue;
        }

        len = (uint32)sqrt((itr->second->x - nx) * (itr->second->x - nx) +
                           (itr->second->y - ny) * (itr->second->y - ny) +
                           (itr->second->z - nz) * (itr->second->z - nz));

        if (len >= traveled_len)
        {
            x = (itr->second->x - nx) * (traveled_len / len) + nx;
            y = (itr->second->y - ny) * (traveled_len / len) + ny;
            z = (itr->second->z - nz) * (traveled_len / len) + nz;
            *last_node = nodecounter;
            return;
        }
        else
        {
            traveled_len -= len;
        }

        nx = itr->second->x;
        ny = itr->second->y;
        nz = itr->second->z;
        ++itr;
        ++nodecounter;
    }

    x = nx;
    y = ny;
    z = nz;
}

TaxiPathNode* TaxiPath::GetPathNode(uint32 i)
{
    if (m_pathNodes.find(i) == m_pathNodes.end())
        return nullptr;

    return m_pathNodes.find(i)->second;
}

void TaxiPath::SendMoveForTime(Player* riding, Player* plrTo, uint32 pTime)
{
    if (!pTime)
        return;

    float length;
    uint32 mapid = riding->GetMapId();
    if (mapid == m_map1)
        length = m_length1;
    else
        length = m_length2;

    float traveled_len = (pTime / (length * TAXI_TRAVEL_SPEED)) * length;
    uint32 len = 0;
    float x = 0, y = 0, z = 0;

    if (!m_pathNodes.size())
        return;

    std::map<uint32, TaxiPathNode*>::iterator itr;
    itr = m_pathNodes.begin();

    float nx, ny, nz;
    nx = ny = nz = 0.0f;
    bool set = false;
    uint32 nodecounter = 1;

    while (itr != m_pathNodes.end())
    {
        if (itr->second->mapid != mapid)
        {
            ++itr;
            ++nodecounter;
            continue;
        }

        if (!set)
        {
            nx = itr->second->x;
            ny = itr->second->y;
            nz = itr->second->z;
            set = true;
            continue;
        }

        len = (uint32)sqrt((itr->second->x - nx) * (itr->second->x - nx) +
                           (itr->second->y - ny) * (itr->second->y - ny) +
                           (itr->second->z - nz) * (itr->second->z - nz));

        if (len >= traveled_len)
        {
            x = (itr->second->x - nx) * (traveled_len / len) + nx;
            y = (itr->second->y - ny) * (traveled_len / len) + ny;
            z = (itr->second->z - nz) * (traveled_len / len) + nz;
            break;
        }
        else
        {
            traveled_len -= len;
        }

        nx = itr->second->x;
        ny = itr->second->y;
        nz = itr->second->z;
        ++itr;
    }

    if (itr == m_pathNodes.end())
        return;

    WorldPacket* data = new WorldPacket(SMSG_MONSTER_MOVE, 2000);
    size_t pos;

    *data << riding->GetNewGUID();
    *data << uint8(0);                  //VLack: usual uint8 after new style guid
    *data << riding->GetPositionX();
    *data << riding->GetPositionY();
    *data << riding->GetPositionZ();
    *data <<Util::getMSTime();
    *data << uint8(0);
    *data << uint32(0x00003000);
    *data << uint32(uint32((length * TAXI_TRAVEL_SPEED) - pTime));
    *data << uint32(nodecounter);
    pos = data->wpos();
    *data << nx;
    *data << ny;
    *data << nz;

    while (itr != m_pathNodes.end())
    {
        TaxiPathNode* pn = itr->second;
        if (pn->mapid != mapid)
            break;

        *data << pn->x;
        *data << pn->y;
        *data << pn->z;
        ++itr;
        ++nodecounter;
    }

    *(uint32*)&(data->contents()[pos]) = nodecounter;
    plrTo->getUpdateMgr().queueDelayedPacket(data);
}

void TaxiMgr::_LoadTaxiNodes()
{
    for (uint32 i = 0; i < sTaxiNodesStore.GetNumRows(); i++)
    {
        auto taxi_nodes = sTaxiNodesStore.LookupEntry(i);
        if (taxi_nodes)
        {
            TaxiNode* n = new TaxiNode;
            n->id = taxi_nodes->id;
            n->mapid = taxi_nodes->mapid;
            n->alliance_mount = taxi_nodes->alliance_mount;
            n->horde_mount = taxi_nodes->horde_mount;
            n->x = taxi_nodes->x;
            n->y = taxi_nodes->y;
            n->z = taxi_nodes->z;

            this->m_taxiNodes.insert(std::map<uint32, TaxiNode*>::value_type(n->id, n));
        }
    }

    ///\todo load mounts
}

void TaxiMgr::_LoadTaxiPaths()
{
    for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); i++)
    {
        auto taxi_path = sTaxiPathStore.LookupEntry(i);
        if (taxi_path)
        {
            TaxiPath* p = new TaxiPath;
            p->from = taxi_path->from;
            p->to = taxi_path->to;
            p->id = taxi_path->id;
            p->price = taxi_path->price;

            //Load Nodes
            for (uint32 j = 0; j < sTaxiPathNodeStore.GetNumRows(); j++)
            {
                auto taxi_path_node = sTaxiPathNodeStore.LookupEntry(j);
                if (taxi_path_node)
                {
                    if (taxi_path_node->path == p->id)
                    {
                        TaxiPathNode* pn = new TaxiPathNode;
                        pn->x = taxi_path_node->x;
                        pn->y = taxi_path_node->y;
                        pn->z = taxi_path_node->z;
                        pn->mapid = taxi_path_node->mapid;
                        p->AddPathNode(taxi_path_node->seq, pn);
                    }
                }
            }
            p->ComputeLen();
            this->m_taxiPaths.insert(std::map<uint32, TaxiPath*>::value_type(p->id, p));
        }
    }
}

TaxiPath* TaxiMgr::GetTaxiPath(uint32 path)
{
    std::unordered_map<uint32, TaxiPath*>::iterator itr;

    itr = this->m_taxiPaths.find(path);
    if (itr == m_taxiPaths.end())
        return NULL;
    else
        return itr->second;
}

TaxiPath* TaxiMgr::GetTaxiPath(uint32 from, uint32 to)
{
    std::unordered_map<uint32, TaxiPath*>::iterator itr;

    for (itr = m_taxiPaths.begin(); itr != m_taxiPaths.end(); ++itr)
        if ((itr->second->to == to) && (itr->second->from == from))
            return itr->second;

    return NULL;
}

TaxiNode* TaxiMgr::GetTaxiNode(uint32 node)
{
    std::unordered_map<uint32, TaxiNode*>::iterator itr;

    itr = this->m_taxiNodes.find(node);

    if (itr == m_taxiNodes.end())
        return NULL;
    else
        return itr->second;
}

//MIT
uint32_t TaxiMgr::getNearestNodeForPlayer(Player* player)
{
    return GetNearestTaxiNode(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId());
}

uint32 TaxiMgr::GetNearestTaxiNode(float x, float y, float z, uint32 mapid)
{
    uint32 nearest = 0;
    float distance = -1;
    float nx, ny, nz, nd;

    std::unordered_map<uint32, TaxiNode*>::iterator itr;

    for (itr = m_taxiNodes.begin(); itr != m_taxiNodes.end(); ++itr)
    {
        if (itr->second->mapid == mapid)
        {
            nx = itr->second->x - x;
            ny = itr->second->y - y;
            nz = itr->second->z - z;
            nd = nx * nx + ny * ny + nz * nz;
            if (nd < distance || distance < 0)
            {
                distance = nd;
                nearest = itr->second->id;
            }
        }
    }
    return nearest;
}

bool TaxiMgr::GetGlobalTaxiNodeMask(uint32 /*curloc*/, uint32* Mask)
{
    std::unordered_map<uint32, TaxiPath*>::iterator itr;
    uint8 field;

    for (itr = m_taxiPaths.begin(); itr != m_taxiPaths.end(); ++itr)
    {
        /*if (itr->second->from == curloc)
        {*/
        field = (uint8)((itr->second->to - 1) / 32);
        if (field >= 12)    //The DBC can contain negative TO values??? That'll be 255 here (because we store everything unsigned), skip them!
            continue;
        Mask[field] |= 1 << ((itr->second->to - 1) % 32);
        //}
    }
    return true;
}
