/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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

#ifndef _EQUIPMENTSETMGR_H
#define _EQUIPMENTSETMGR_H

#define EQUIPMENTSET_SLOTS 10

namespace Arcemu
{
    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note   struct EquipmentSet   - World of Warcraft Equipment Set structure
    ///         Contains the name, icon, Id, and item lowguids of the EquipmentSet
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    struct EquipmentSet
    {
        uint32 SetGUID;
        uint32 SetID;
        std::string SetName;
        std::string IconName;
        std::tr1::array<uint32, 19> ItemGUID;

        EquipmentSet()
        {
            SetGUID = 0;
            SetID = 0;
            SetName = "";
            IconName = "";

            for (uint32 i = 0; i < ItemGUID.size(); ++i)
                ItemGUID[i] = 0;
        }
    };


    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note EquipmentSetStorage   - Storage for world of warcraft equipment set structures
    ///
    /// Key     - uint32 -  GUID of the set
    /// Value   - EquipmentSet*  - pointer to an EquipmentSet structure
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    typedef std::map<uint32, EquipmentSet*> EquipmentSetStorage;


    //////////////////////////////////////////////////////////////////////////////////////////
    /// class EquipmentSetMgr
    /// \note   Class that manages World of Warcraft Equipment Sets
    ///         Manages set storage, indexing and serialization/deserialization
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    class EquipmentSetMgr
    {
        public:

            EquipmentSetMgr() { ownerGUID = 1; }
            EquipmentSetMgr(uint32 ownerGUID) { this->ownerGUID = ownerGUID; }
            ~EquipmentSetMgr();

            //////////////////////////////////////////////////////////////////////////////////////////
            /// \note EquipmentSet* GetEquipmentSet - Looks up the set in storage (if any) and returns it
            ///
            /// \param uint32 GUID  -  GUID of the equipment set
            ///
            /// \returns a pointer to an EquipmentSet structure on success, NULL on failure.
            ///
            //////////////////////////////////////////////////////////////////////////////////////////
            EquipmentSet* GetEquipmentSet(uint32 GUID);


            //////////////////////////////////////////////////////////////////////////////////////////
            /// \note bool AddEquipmentSet  - Stores and equipment set in storage
            ///
            /// \param uint32 setGUID    -  GUID of the equipment set
            /// \param EquipmentSet* set -  pointer to an equipment set structure
            ///
            /// \returns true on success, false on failure.
            ///
            //////////////////////////////////////////////////////////////////////////////////////////
            bool AddEquipmentSet(uint32 setGUID, EquipmentSet* set);


            //////////////////////////////////////////////////////////////////////////////////////////
            /// \note bool DeleteEquipmentSet- Removes an equipment set with the given GUID from storage
            ///
            /// \param uint32 setGUID  -  GUID of the equipment set
            ///
            /// \returns true on success, false on failure.
            ///
            //////////////////////////////////////////////////////////////////////////////////////////
            bool DeleteEquipmentSet(uint32 setGUID);


            //////////////////////////////////////////////////////////////////////////////////////////
            /// \note bool LoadfromDB   - Deserializes the contents of the class from the database
            ///
            /// \param QueryResult* result  -  pointer to a QueryResult structure that contains the data to be loaded
            ///
            /// \returns true on success, false on failure.
            ///
            //////////////////////////////////////////////////////////////////////////////////////////
            bool LoadfromDB(QueryResult* result);


            //////////////////////////////////////////////////////////////////////////////////////////
            /// \note bool SavetoDB - Serializes the contents of the class and saves them to the database.
            ///
            /// \param QueryBuffer* buf  -  Pointer to a QueryBuffer structure that can store the datas of the class
            ///
            /// \returns true on success, false on failure.
            ///
            //////////////////////////////////////////////////////////////////////////////////////////
            bool SavetoDB(QueryBuffer* buf);


            //////////////////////////////////////////////////////////////////////////////////////////
            /// \note void FillEquipmentSetListPacket   - Fills the SMSG_EQUIPMENT_SET_LIST packet with data from the class.
            ///
            /// \param WorldPacket& data  -  Reference to a WorldPacket structure that can store the data to be sen
            ///
            /// \returns none
            ///
            //////////////////////////////////////////////////////////////////////////////////////////
            void FillEquipmentSetListPacket(WorldPacket & data);

        private:

            EquipmentSetMgr(EquipmentSetMgr & other) {}
            EquipmentSetMgr & operator=(EquipmentSetMgr & other) { return *this; }

            /// GUID of the owner (player) of the equipment sets
            uint32 ownerGUID;

            EquipmentSetStorage EquipmentSets;
    };

}

#endif // _EQUIPMENTSETMGR_H
