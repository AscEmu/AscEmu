/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

#ifndef TAXIMGR_H
#define TAXIMGR_H

#include <map>

#define TAXI_TRAVEL_SPEED 32
#include "Singleton.h"
#include <unordered_map>

class Player;

struct TaxiNode
{
    uint32 id;
    float x, y, z;
    uint32 mapid;
    uint32 horde_mount;
    uint32 alliance_mount;
};

struct TaxiPathNode
{
    float x, y, z;
    uint32 mapid;
};

class TaxiPath
{
        friend class TaxiMgr;

    public:
        TaxiPath()
        {
            price = 0;
            id = 0;
            m_length1 = 0;
            m_map1 = 0;
            m_length2 = 0;
            m_map2 = 0;
            to = 0;
            from = 0;
        }

        ~TaxiPath()
        {
            while(m_pathNodes.size())
            {
                TaxiPathNode* pn = m_pathNodes.begin()->second;
                m_pathNodes.erase(m_pathNodes.begin());
                delete pn;
            }
        }

        void ComputeLen();
        void SetPosForTime(float & x, float & y, float & z, uint32 time, uint32* lastnode, uint32 mapid);
        inline uint32 GetID() { return id; }
        void SendMoveForTime(Player* riding, Player* to, uint32 time);
        void AddPathNode(uint32 index, TaxiPathNode* pn) { m_pathNodes[index] = pn; }
        inline size_t GetNodeCount() { return m_pathNodes.size(); }
        TaxiPathNode* GetPathNode(uint32 i);
        inline uint32 GetPrice() { return price; }
        inline uint32 GetSourceNode() { return from; }

    protected:

        std::map<uint32, TaxiPathNode*> m_pathNodes;

        float m_length1;
        uint32 m_map1;

        float m_length2;
        uint32 m_map2;
        uint32 id, to, from, price;
};


class SERVER_DECL TaxiMgr :  public Singleton< TaxiMgr >
{
    public:
        TaxiMgr()
        {
            _LoadTaxiNodes();
            _LoadTaxiPaths();
        }

        ~TaxiMgr()
        {
            while(m_taxiPaths.size())
            {
                TaxiPath* p = m_taxiPaths.begin()->second;
                m_taxiPaths.erase(m_taxiPaths.begin());
                delete p;
            }
            while(m_taxiNodes.size())
            {
                TaxiNode* n = m_taxiNodes.begin()->second;
                m_taxiNodes.erase(m_taxiNodes.begin());
                delete n;
            }
        }

        TaxiPath* GetTaxiPath(uint32 path);
        TaxiPath* GetTaxiPath(uint32 from, uint32 to);
        TaxiNode* GetTaxiNode(uint32 node);

        uint32 GetNearestTaxiNode(float x, float y, float z, uint32 mapid);
        bool GetGlobalTaxiNodeMask(uint32 curloc, uint32* Mask);


    private:
        void _LoadTaxiNodes();
        void _LoadTaxiPaths();

        std::unordered_map<uint32, TaxiNode*> m_taxiNodes;
        std::unordered_map<uint32, TaxiPath*> m_taxiPaths;
};

#define sTaxiMgr TaxiMgr::getSingleton()

#endif 
