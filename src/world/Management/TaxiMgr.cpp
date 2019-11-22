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

void TaxiPath::ComputeLen()
{
    m_length1 = 0;
    m_length2 = 0;
    m_map1 = 0;
    m_map2 = 0;
    float* curptr = &m_length1;

    if (m_pathNodes.empty())
        return;

    auto itr = m_pathNodes.begin();

    float x = itr->second->x;
    float y = itr->second->y;
    float z = itr->second->z;
    uint32_t curmap = itr->second->mapid;
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

void TaxiPath::SetPosForTime(float & x, float & y, float & z, uint32_t time, uint32_t* lastNode, uint32_t mapid)
{
    if (!time)
        return;

    float length;
    if (mapid == m_map1)
        length = m_length1;
    else
        length = m_length2;

    float traveledLen = (time / (length * TAXI_TRAVEL_SPEED)) * length;

    x = 0;
    y = 0;
    z = 0;

    if (m_pathNodes.empty())
        return;

    auto itr = m_pathNodes.begin();

    float nx = 0.0f;
    float ny = 0.0f;
    float nz = 0.0f;
    bool set = false;
    uint32_t nodecounter = 0;

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

        auto len = static_cast<uint32_t>(sqrt((itr->second->x - nx) * (itr->second->x - nx) +
            (itr->second->y - ny) * (itr->second->y - ny) +
            (itr->second->z - nz) * (itr->second->z - nz)));

        if (len >= traveledLen)
        {
            x = (itr->second->x - nx) * (traveledLen / len) + nx;
            y = (itr->second->y - ny) * (traveledLen / len) + ny;
            z = (itr->second->z - nz) * (traveledLen / len) + nz;
            *lastNode = nodecounter;
            return;
        }
        else
        {
            traveledLen -= len;
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

TaxiPathNode* TaxiPath::GetPathNode(uint32_t i)
{
    if (m_pathNodes.find(i) == m_pathNodes.end())
        return nullptr;

    return m_pathNodes.find(i)->second;
}

void TaxiPath::SendMoveForTime(Player* riding, Player* to, uint32_t time)
{
    if (!time)
        return;

    float length;
    uint32_t mapid = riding->GetMapId();
    if (mapid == m_map1)
        length = m_length1;
    else
        length = m_length2;

    float traveledLen = (time / (length * TAXI_TRAVEL_SPEED)) * length;

    if (m_pathNodes.empty())
        return;

    auto itr = m_pathNodes.begin();

    float nx = 0.0f;
    float ny = 0.0f;
    float nz = 0.0f;
    bool set = false;
    uint32_t nodecounter = 1;

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

        auto len = static_cast<uint32_t>(sqrt((itr->second->x - nx) * (itr->second->x - nx) +
            (itr->second->y - ny) * (itr->second->y - ny) +
            (itr->second->z - nz) * (itr->second->z - nz)));

        if (len >= traveledLen)
        {
            float x = (itr->second->x - nx) * (traveledLen / len) + nx;
            float y = (itr->second->y - ny) * (traveledLen / len) + ny;
            float z = (itr->second->z - nz) * (traveledLen / len) + nz;
            break;
        }

        traveledLen -= len;

        nx = itr->second->x;
        ny = itr->second->y;
        nz = itr->second->z;
        ++itr;
    }

    if (itr == m_pathNodes.end())
        return;

    auto* data = new WorldPacket(SMSG_MONSTER_MOVE, 2000);

    *data << riding->GetNewGUID();
    *data << uint8_t(0);                  //VLack: usual uint8 after new style guid
    *data << riding->GetPositionX();
    *data << riding->GetPositionY();
    *data << riding->GetPositionZ();
    *data << Util::getMSTime();
    *data << uint8_t(0);
    *data << uint32_t(0x00003000);
    *data << uint32_t(uint32_t((length * TAXI_TRAVEL_SPEED) - time));
    *data << uint32_t(nodecounter);
    size_t pos = data->wpos();
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

    *reinterpret_cast<uint32_t*>(&(data->contents()[pos])) = nodecounter;
    to->getUpdateMgr().queueDelayedPacket(data);
}

void TaxiMgr::_LoadTaxiNodes()
{
    for (uint32_t i = 0; i < sTaxiNodesStore.GetNumRows(); i++)
    {
        auto taxiNodes = sTaxiNodesStore.LookupEntry(i);
        if (taxiNodes)
        {
            auto* n = new TaxiNode;
            n->id = taxiNodes->id;
            n->mapid = taxiNodes->mapid;
            n->allianceMount = taxiNodes->alliance_mount;
            n->hordeMount = taxiNodes->horde_mount;
            n->x = taxiNodes->x;
            n->y = taxiNodes->y;
            n->z = taxiNodes->z;

            this->m_taxiNodes.insert(std::map<uint32, TaxiNode*>::value_type(n->id, n));
        }
    }

    ///\todo load mounts
}

void TaxiMgr::_LoadTaxiPaths()
{
    for (uint32_t i = 0; i < sTaxiPathStore.GetNumRows(); i++)
    {
        auto taxiPath = sTaxiPathStore.LookupEntry(i);
        if (taxiPath)
        {
            auto* p = new TaxiPath;
            p->m_from = taxiPath->from;
            p->m_to = taxiPath->to;
            p->m_id = taxiPath->id;
            p->m_price = taxiPath->price;

            //Load Nodes
            for (uint32_t j = 0; j < sTaxiPathNodeStore.GetNumRows(); j++)
            {
                auto taxiPathNode = sTaxiPathNodeStore.LookupEntry(j);
                if (taxiPathNode)
                {
                    if (taxiPathNode->path == p->m_id)
                    {
                        auto* pn = new TaxiPathNode;
                        pn->x = taxiPathNode->x;
                        pn->y = taxiPathNode->y;
                        pn->z = taxiPathNode->z;
                        pn->mapid = taxiPathNode->mapid;
                        p->AddPathNode(taxiPathNode->seq, pn);
                    }
                }
            }
            p->ComputeLen();
            this->m_taxiPaths.insert(std::map<uint32, TaxiPath*>::value_type(p->m_id, p));
        }
    }
}

TaxiPath* TaxiMgr::GetTaxiPath(uint32_t path)
{
    auto itr = this->m_taxiPaths.find(path);
    if (itr == m_taxiPaths.end())
        return nullptr;
    
    return itr->second;
}

TaxiPath* TaxiMgr::GetTaxiPath(uint32_t from, uint32_t to)
{
    for (auto& taxiPath : m_taxiPaths)
        if (taxiPath.second->m_to == to && taxiPath.second->m_from == from)
            return taxiPath.second;

    return nullptr;
}

TaxiNode* TaxiMgr::GetTaxiNode(uint32_t node)
{
    auto itr = this->m_taxiNodes.find(node);

    if (itr == m_taxiNodes.end())
        return nullptr;

    return itr->second;
}

//MIT
uint32_t TaxiMgr::getNearestNodeForPlayer(Player* player)
{
    return GetNearestTaxiNode(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId());
}

uint32_t TaxiMgr::GetNearestTaxiNode(float x, float y, float z, uint32_t mapid)
{
    uint32_t nearest = 0;
    float distance = -1;

    for (auto& taxiNode : m_taxiNodes)
    {
        if (taxiNode.second->mapid == mapid)
        {
            float nx = taxiNode.second->x - x;
            float ny = taxiNode.second->y - y;
            float nz = taxiNode.second->z - z;
            float nd = nx * nx + ny * ny + nz * nz;
            if (nd < distance || distance < 0)
            {
                distance = nd;
                nearest = taxiNode.second->id;
            }
        }
    }
    return nearest;
}

bool TaxiMgr::GetGlobalTaxiNodeMask(uint32_t /*curloc*/, uint32_t* mask)
{
    for (auto& taxiPath : m_taxiPaths)
    {
        /*if (itr->second->from == curloc)
        {*/
        auto field = static_cast<uint8_t>((taxiPath.second->m_to - 1) / 32);
        if (field >= 12) // The DBC can contain negative TO values??? That'll be 255 here (because we store everything unsigned), skip them!
            continue;
        mask[field] |= 1 << ((taxiPath.second->m_to - 1) % 32);
        //}
    }
    return true;
}
