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

#ifndef TAXIMGR_H
#define TAXIMGR_H

//MIT

namespace TaxiNodeError
{
    enum
    {
        Ok = 0,
        UnspecificError = 1,
        NoDirectPath = 2,
        NotEnoughMoney = 3
    };
};

#include <map>

#define TAXI_TRAVEL_SPEED 32
#include "Singleton.h"
#include <unordered_map>

class Player;

struct TaxiNode
{
    uint32_t id;
    float x, y, z;
    uint32_t mapid;
    uint32_t hordeMount;
    uint32_t allianceMount;
};

struct TaxiPathNode
{
    float x, y, z;
    uint32_t mapid;
};

class TaxiPath
{
        friend class TaxiMgr;

    public:
        TaxiPath()
        {
            m_price = 0;
            m_id = 0;
            m_length1 = 0;
            m_map1 = 0;
            m_length2 = 0;
            m_map2 = 0;
            m_to = 0;
            m_from = 0;
        }

        ~TaxiPath()
        {
            while(!m_pathNodes.empty())
            {
                TaxiPathNode* pn = m_pathNodes.begin()->second;
                m_pathNodes.erase(m_pathNodes.begin());
                delete pn;
            }
        }

        void ComputeLen();
        void SetPosForTime(float & x, float & y, float & z, uint32_t time, uint32_t* lastNode, uint32_t mapid);
        inline uint32_t GetID() { return m_id; }
        void SendMoveForTime(Player* riding, Player* to, uint32_t time);
        void AddPathNode(uint32_t index, TaxiPathNode* pn) { m_pathNodes[index] = pn; }
        inline size_t GetNodeCount() { return m_pathNodes.size(); }
        TaxiPathNode* GetPathNode(uint32_t i);
        inline uint32_t getPrice() { return m_price; }
        inline uint32_t getSourceNode() { return m_from; }

    protected:

        std::map<uint32_t, TaxiPathNode*> m_pathNodes;

        float m_length1;
        uint32_t m_map1;

        float m_length2;
        uint32_t m_map2;

		uint32_t m_id;
		uint32_t m_to;
		uint32_t m_from;
		uint32_t m_price;
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
            while(!m_taxiPaths.empty())
            {
                TaxiPath* p = m_taxiPaths.begin()->second;
                m_taxiPaths.erase(m_taxiPaths.begin());
                delete p;
            }
            while(!m_taxiNodes.empty())
            {
                TaxiNode* n = m_taxiNodes.begin()->second;
                m_taxiNodes.erase(m_taxiNodes.begin());
                delete n;
            }
        }

        TaxiPath* GetTaxiPath(uint32_t path);
        TaxiPath* GetTaxiPath(uint32_t from, uint32 to);
        TaxiNode* GetTaxiNode(uint32_t node);

        //MIT
        uint32_t getNearestNodeForPlayer(Player* player);

        uint32 GetNearestTaxiNode(float x, float y, float z, uint32 mapid);
        bool GetGlobalTaxiNodeMask(uint32_t curloc, uint32_t* mask);


    private:
        void _LoadTaxiNodes();
        void _LoadTaxiPaths();

        std::unordered_map<uint32_t, TaxiNode*> m_taxiNodes;
        std::unordered_map<uint32_t, TaxiPath*> m_taxiPaths;
};

#define sTaxiMgr TaxiMgr::getSingleton()

#endif 
